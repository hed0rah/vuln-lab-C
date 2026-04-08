#!/bin/bash
# harden_test.sh - compile each vulnerability with different hardening flags
# and show which protections detect which bugs.
#
# usage:
#   ./harden_test.sh                          full matrix (all examples, all columns)
#   ./harden_test.sh asan ubsan               specific columns only
#   ./harden_test.sh --only heap_overflow     single example against all columns
#   ./harden_test.sh --custom "-O2 -fsanitize=address,undefined"
#   ./harden_test.sh --compile-check          which examples fail to build with strict warnings
#   ./harden_test.sh --sysinfo                show current system hardening state
#
# environment:
#   CC=clang ./harden_test.sh msan            use a different compiler

CC="${CC:-gcc}"
BASEDIR=$(cd "$(dirname "$0")" && pwd)
TMPD=$(mktemp -d)
trap 'rm -rf "$TMPD"' EXIT

# --- flag sets ---
declare -A FLAGS
FLAGS[stack]="-fstack-protector-all"
FLAGS[fortify]="-D_FORTIFY_SOURCE=2 -O2"
FLAGS[asan]="-fsanitize=address -O1"
FLAGS[ubsan]="-fsanitize=undefined"
FLAGS[tsan]="-fsanitize=thread -O1"
FLAGS[msan]="-fsanitize=memory -O1"
FLAGS[full]="-fstack-protector-all -D_FORTIFY_SOURCE=2 -O2 -pie -fPIE -Wl,-z,relro,-z,now"

DEFAULT_COLUMNS=(stack fortify asan ubsan tsan full)

# --- tests: "source trigger_arg [trigger_arg ...]" ---
# omitted: cmd_injection (logic bug, not a memory error -- no sanitizer catches it)
#           toctou (requires an external process to race against)
declare -a TESTS=(
    "buffer_overflow     AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "heap_overflow       AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "stack_off_by_one    AAAAAAAAAA"
    "off_by_one_challenge AAAAAAAAAA"
    "format_string       %x.%x.%x.%x"
    "double_free"
    "use_after_free"
    "use_after_free2"
    "dangling_pointer"
    "int_to_buffer_overflow -1"
    "integer_overflow"
    "null_ptr_deref"
    "buf_over_read       hello 200"
    "uninitialized_var"
    "uninitialized_var2"
    "type_confusion"
    "race_condition"
)

# tests where tsan results are meaningless (single-threaded programs)
declare -A SKIP_TSAN
for t in buffer_overflow heap_overflow stack_off_by_one off_by_one_challenge \
         format_string double_free use_after_free use_after_free2 \
         dangling_pointer int_to_buffer_overflow integer_overflow \
         null_ptr_deref buf_over_read uninitialized_var uninitialized_var2 \
         type_confusion; do
    SKIP_TSAN[$t]=1
done

# msan requires clang and full instrumentation; skip on gcc
# and skip for threaded programs (msan + threads is fragile)
declare -A SKIP_MSAN
SKIP_MSAN[race_condition]=1

# --- colors (disabled when not a tty) ---
if [[ -t 1 ]]; then
    GRN=$'\033[0;32m'
    RED=$'\033[0;31m'
    YLW=$'\033[0;33m'
    DIM=$'\033[2m'
    CYN=$'\033[0;36m'
    BLD=$'\033[1m'
    RST=$'\033[0m'
else
    GRN='' RED='' YLW='' DIM='' CYN='' BLD='' RST=''
fi

# --- helpers ---

compile_with() {
    local src="$1"; shift
    local flags="$*"
    local slug; slug=$(echo "$flags" | tr ' /=-' '____' | tr -s '_')
    local out="$TMPD/$(basename "$src" .c)_${slug}"
    local ldflags=""
    local pieflags="-no-pie -fno-pie"
    [[ "$src" == *race_condition* ]] && ldflags="-lpthread"
    # tsan and msan need PIE; -no-pie causes fatal memory mapping errors
    [[ "$flags" == *-fsanitize=thread* || "$flags" == *-fsanitize=memory* ]] && pieflags=""
    # shellcheck disable=SC2086
    if $CC -g $pieflags $flags "$src" -o "$out" $ldflags 2>/dev/null; then
        echo "$out"
    fi
}

# compile and capture diagnostics (for --compile-check)
compile_strict() {
    local src="$1"
    local ldflags=""
    [[ "$src" == *race_condition* ]] && ldflags="-lpthread"
    local out="$TMPD/$(basename "$src" .c)_strict"
    # shellcheck disable=SC2086
    $CC -g -Wall -Wextra -Wformat=2 -Wformat-security -Werror \
        -Wuse-after-free -Warray-bounds=2 \
        "$src" -o "$out" $ldflags 2>&1
    return $?
}

run_bin() {
    local bin="$1"; shift
    local combined exit_code

    combined=$(timeout 8 "$bin" "$@" 2>&1)
    exit_code=$?

    [[ $exit_code -eq 124 ]] && echo "TIMEOUT" && return

    if echo "$combined" | grep -qiE \
        'AddressSanitizer|MemorySanitizer|runtime error|stack smashing|double free|heap-use-after-free|ThreadSanitizer|illegal instruction|SIGILL|ERROR:|buffer overflow detected|memory corruption|invalid pointer'; then
        echo "CAUGHT"
        return
    fi

    if [[ $exit_code -gt 128 || $exit_code -eq 134 || $exit_code -eq 139 ]]; then
        echo "CRASH"
        return
    fi

    echo "MISS"
}

render() {
    case "$1" in
        CAUGHT)   printf "%s%-7s%s" "$GRN" "CAUGHT"   "$RST" ;;
        MISS)     printf "%s%-7s%s" "$RED" "miss"     "$RST" ;;
        CRASH)    printf "%s%-7s%s" "$YLW" "crash"    "$RST" ;;
        TIMEOUT)  printf "%s%-7s%s" "$YLW" "timeout"  "$RST" ;;
        BUILD)    printf "%s%-7s%s" "$DIM" "build?"   "$RST" ;;
        NA)       printf "%s%-7s%s" "$CYN" "n/a"     "$RST" ;;
        REJECTED) printf "%s%-7s%s" "$GRN" "REJECT"   "$RST" ;;
        BUILT)    printf "%s%-7s%s" "$RED" "built"    "$RST" ;;
    esac
}

usage() {
    cat <<'EOF'
usage: ./harden_test.sh [options] [columns...]

modes:
  (default)                   run full matrix
  --compile-check             show which examples are rejected by strict warnings
  --sysinfo                   display current system hardening settings
  --help                      this message

options:
  --only <example>            test a single example against all columns
  --custom "<flags>"          add a custom column with the given compiler flags
                              (can be repeated)

columns:
  stack fortify asan ubsan tsan msan full
  or any name defined via --custom

environment:
  CC=clang ./harden_test.sh msan    use clang (required for msan)

examples:
  ./harden_test.sh asan ubsan
  ./harden_test.sh --only heap_overflow
  ./harden_test.sh --custom "-fsanitize=address,undefined -O2"
  ./harden_test.sh --only double_free --custom "-D_FORTIFY_SOURCE=3 -O2"
  CC=clang ./harden_test.sh msan
EOF
    exit 0
}

# --- --sysinfo mode ---

cmd_sysinfo() {
    printf "${BLD}system hardening state${RST}\n\n"

    # ASLR
    local aslr
    if [[ -r /proc/sys/kernel/randomize_va_space ]]; then
        aslr=$(cat /proc/sys/kernel/randomize_va_space)
        case "$aslr" in
            0) printf "ASLR:           ${RED}OFF${RST} (randomize_va_space = 0)\n" ;;
            1) printf "ASLR:           ${YLW}partial${RST} (randomize_va_space = 1, stack/mmap/vdso only)\n" ;;
            2) printf "ASLR:           ${GRN}full${RST} (randomize_va_space = 2)\n" ;;
            *) printf "ASLR:           %s (randomize_va_space = %s)\n" "$aslr" "$aslr" ;;
        esac
    else
        printf "ASLR:           unknown (cannot read /proc/sys/kernel/randomize_va_space)\n"
    fi

    # mmap_min_addr
    if [[ -r /proc/sys/vm/mmap_min_addr ]]; then
        local mma
        mma=$(cat /proc/sys/vm/mmap_min_addr)
        if [[ "$mma" -eq 0 ]]; then
            printf "mmap_min_addr:  ${RED}0${RST} (page zero mappable -- null deref exploitable)\n"
        else
            printf "mmap_min_addr:  ${GRN}%s${RST}\n" "$mma"
        fi
    fi

    # NX support
    if grep -q ' nx ' /proc/cpuinfo 2>/dev/null; then
        printf "NX bit:         ${GRN}supported${RST}\n"
    else
        printf "NX bit:         ${YLW}not detected in /proc/cpuinfo${RST}\n"
    fi

    # kernel hardening
    if [[ -r /proc/sys/kernel/dmesg_restrict ]]; then
        local dm
        dm=$(cat /proc/sys/kernel/dmesg_restrict)
        printf "dmesg_restrict: %s\n" "$([[ $dm -eq 1 ]] && echo "restricted" || echo "open")"
    fi

    if [[ -r /proc/sys/kernel/kptr_restrict ]]; then
        local kp
        kp=$(cat /proc/sys/kernel/kptr_restrict)
        printf "kptr_restrict:  %s\n" "$kp"
    fi

    # compiler
    printf "\ncompiler:       %s\n" "$($CC --version 2>/dev/null | head -1)"

    # check for sanitizer support
    printf "\nsanitizer support:\n"
    for san in address undefined thread memory; do
        local test_src="$TMPD/san_test.c"
        echo 'int main(void){return 0;}' > "$test_src"
        if $CC -fsanitize="$san" "$test_src" -o "$TMPD/san_test" 2>/dev/null; then
            printf "  -fsanitize=%-12s ${GRN}available${RST}\n" "$san"
        else
            printf "  -fsanitize=%-12s ${RED}not available${RST}\n" "$san"
        fi
    done

    exit 0
}

# --- --compile-check mode ---

cmd_compile_check() {
    printf "${BLD}compile-time rejection with strict warnings${RST}\n"
    printf "flags: -Wall -Wextra -Wformat=2 -Wformat-security -Werror\n"
    printf "       -Wuse-after-free -Warray-bounds=2\n\n"
    printf "%-28s %-10s %s\n" "example" "result" "reason"
    printf "%-28s %-10s %s\n" "-------" "------" "------"

    for entry in "${TESTS[@]}"; do
        read -ra words <<< "$entry"
        src="${words[0]}"
        local srcfile="$BASEDIR/${src}.c"
        [[ -f "$srcfile" ]] || continue

        local diag
        diag=$(compile_strict "$srcfile" 2>&1)
        local rc=$?

        if [[ $rc -ne 0 ]]; then
            # extract first error line
            local reason
            reason=$(echo "$diag" | grep -m1 'error:' | sed 's/.*error: //')
            printf "%-28s " "$src"
            render REJECTED
            printf "  %s\n" "$reason"
        else
            printf "%-28s " "$src"
            render BUILT
            printf "\n"
        fi
    done

    # also check the two omitted examples
    for src in cmd_injection toctou; do
        local srcfile="$BASEDIR/${src}.c"
        [[ -f "$srcfile" ]] || continue

        local diag
        diag=$(compile_strict "$srcfile" 2>&1)
        local rc=$?

        if [[ $rc -ne 0 ]]; then
            local reason
            reason=$(echo "$diag" | grep -m1 'error:' | sed 's/.*error: //')
            printf "%-28s " "$src"
            render REJECTED
            printf "  %s\n" "$reason"
        else
            printf "%-28s " "$src"
            render BUILT
            printf "\n"
        fi
    done

    printf "\n"
    printf "${GRN}REJECT${RST}  compiler rejected the code (bug caught at build time)\n"
    printf "${RED}built${RST}   compiled successfully (bug not caught by warnings)\n"

    exit 0
}

# --- argument parsing ---

ONLY_FILTER=""
CUSTOM_COUNT=0
COLUMNS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --help|-h)
            usage
            ;;
        --sysinfo)
            cmd_sysinfo
            ;;
        --compile-check)
            cmd_compile_check
            ;;
        --only)
            shift
            ONLY_FILTER="$1"
            shift
            ;;
        --custom)
            shift
            CUSTOM_COUNT=$((CUSTOM_COUNT + 1))
            local_name="custom${CUSTOM_COUNT}"
            FLAGS[$local_name]="$1"
            COLUMNS+=("$local_name")
            shift
            ;;
        *)
            COLUMNS+=("$1")
            shift
            ;;
    esac
done

# if no columns specified, use defaults
if [[ ${#COLUMNS[@]} -eq 0 ]]; then
    COLUMNS=("${DEFAULT_COLUMNS[@]}")
fi

# --- matrix mode ---

# check if requested column is a skip for this test
should_skip() {
    local col="$1" src="$2"
    if [[ "$col" == "tsan" && "${SKIP_TSAN[$src]+set}" == "set" ]]; then
        return 0
    fi
    if [[ "$col" == "msan" && "${SKIP_MSAN[$src]+set}" == "set" ]]; then
        return 0
    fi
    # msan with gcc is not supported
    if [[ "$col" == "msan" && "$CC" != *clang* ]]; then
        return 0
    fi
    return 1
}

# header
printf "%-28s" ""
for col in "${COLUMNS[@]}"; do
    printf "%-8s" "$col"
done
printf "\n"
printf "%-28s" ""
for col in "${COLUMNS[@]}"; do
    printf "%-8s" "-------"
done
printf "\n"

# run tests
for entry in "${TESTS[@]}"; do
    read -ra words <<< "$entry"
    src="${words[0]}"
    args=("${words[@]:1}")

    # --only filter
    if [[ -n "$ONLY_FILTER" && "$src" != "$ONLY_FILTER" ]]; then
        continue
    fi

    printf "%-28s" "$src"

    for col in "${COLUMNS[@]}"; do
        if should_skip "$col" "$src"; then
            render NA
            printf " "
            continue
        fi

        flags="${FLAGS[$col]}"
        if [[ -z "$flags" ]]; then
            render BUILD
            printf " "
            continue
        fi

        # shellcheck disable=SC2086
        bin=$(compile_with "$BASEDIR/${src}.c" $flags)
        if [[ -z "$bin" ]]; then
            render BUILD
        else
            result=$(run_bin "$bin" "${args[@]}")
            render "$result"
        fi
        printf " "
    done
    printf "\n"
done

printf "\n"
printf "${GRN}CAUGHT${RST}  protection fired (sanitizer or canary aborted the program)\n"
printf "${RED}miss${RST}    bug triggered but not detected by this flag set\n"
printf "${YLW}crash${RST}   raw signal with no protection message\n"
printf "${DIM}build?${RST}  did not compile with these flags\n"
printf "${CYN}n/a${RST}     test not applicable for this flag set\n"
