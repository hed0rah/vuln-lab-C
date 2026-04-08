#!/bin/bash
# harden_test.sh - compile each vulnerability with different hardening flags
# and show which protections detect which bugs.
#
# usage:
#   ./harden_test.sh              run all columns
#   ./harden_test.sh asan ubsan   run specific columns only

CC=gcc
BASEDIR=$(cd "$(dirname "$0")" && pwd)
TMPD=$(mktemp -d)
trap 'rm -rf "$TMPD"' EXIT

# --- flag sets ---
declare -A FLAGS
FLAGS[stack]="-fstack-protector-all"
FLAGS[fortify]="-D_FORTIFY_SOURCE=2 -O2"
FLAGS[asan]="-fsanitize=address -O1"
FLAGS[ubsan]="-fsanitize=undefined -fsanitize-undefined-trap-on-error"
FLAGS[tsan]="-fsanitize=thread -O1"
FLAGS[full]="-fstack-protector-all -D_FORTIFY_SOURCE=2 -O2 -pie -fPIE -Wl,-z,relro,-z,now"

COLUMN_ORDER=(stack fortify asan ubsan tsan full)

# --- tests: "source trigger_arg [trigger_arg ...]" ---
# use a single field; args are shell-quoted inside each entry
declare -a TESTS=(
    "buffer_overflow     AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "heap_overflow       AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "format_string       %x.%x.%x.%x"
    "double_free"
    "use_after_free"
    "int_to_buffer_overflow -1"
    "integer_overflow"
    "null_ptr_deref"
    "race_condition"
    "buf_over_read       hello 200"
    "dangling_pointer"
)

# --- colors (disabled when not a tty) ---
if [[ -t 1 ]]; then
    GRN=$'\033[0;32m'
    RED=$'\033[0;31m'
    YLW=$'\033[0;33m'
    DIM=$'\033[2m'
    RST=$'\033[0m'
else
    GRN='' RED='' YLW='' DIM='' RST=''
fi

# compile src.c with extra flags; print binary path or nothing on failure
compile_with() {
    local src="$1"; shift
    local flags="$*"
    local slug; slug=$(echo "$flags" | tr ' /=-' '____' | tr -s '_')
    local out="$TMPD/$(basename "$src" .c)_${slug}"
    local ldflags=""
    [[ "$src" == *race_condition* ]] && ldflags="-lpthread"
    # shellcheck disable=SC2086
    if $CC -g -no-pie -fno-pie $flags "$src" -o "$out" $ldflags 2>/dev/null; then
        echo "$out"
    fi
}

# run binary; return CAUGHT / MISS / CRASH / TIMEOUT
run_bin() {
    local bin="$1"; shift
    local combined exit_code

    combined=$(timeout 8 "$bin" "$@" 2>&1)
    exit_code=$?

    [[ $exit_code -eq 124 ]] && echo "TIMEOUT" && return

    if echo "$combined" | grep -qiE \
        'AddressSanitizer|runtime error|stack smashing|double free|heap-use-after-free|ThreadSanitizer|illegal instruction|SIGILL|ERROR:|buffer overflow detected|memory corruption|invalid pointer'; then
        echo "CAUGHT"
        return
    fi

    # killed by signal without a sanitizer message
    if [[ $exit_code -gt 128 || $exit_code -eq 134 || $exit_code -eq 139 ]]; then
        echo "CRASH"
        return
    fi

    echo "MISS"
}

render() {
    local pad="       "
    case "$1" in
        CAUGHT)  printf "%s%-7s%s" "$GRN" "CAUGHT"  "$RST" ;;
        MISS)    printf "%s%-7s%s" "$RED" "miss"    "$RST" ;;
        CRASH)   printf "%s%-7s%s" "$YLW" "crash"   "$RST" ;;
        TIMEOUT) printf "%s%-7s%s" "$YLW" "timeout" "$RST" ;;
        BUILD)   printf "%s%-7s%s" "$DIM" "build?"  "$RST" ;;
    esac
}

# pick columns
if [[ $# -gt 0 ]]; then
    COLUMN_ORDER=("$@")
fi

# header
printf "%-28s" ""
for col in "${COLUMN_ORDER[@]}"; do
    printf "%-8s" "$col"
done
printf "\n"
printf "%-28s" ""
for col in "${COLUMN_ORDER[@]}"; do
    printf "%-8s" "-------"
done
printf "\n"

# run tests
for entry in "${TESTS[@]}"; do
    read -ra words <<< "$entry"
    src="${words[0]}"
    args=("${words[@]:1}")   # may be empty

    printf "%-28s" "$src"

    for col in "${COLUMN_ORDER[@]}"; do
        flags="${FLAGS[$col]}"
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
