# Hardening reference

Quick reference for compiler, linker, and system-level hardening options
relevant to the vulnerability examples in this repo. Use `harden_test.sh` to
see which flags catch which bugs empirically.

## Compiler flags (GCC and Clang)

### Stack protection

| flag | what it does |
|------|-------------|
| `-fstack-protector` | inserts canary before return address on functions with char arrays >= 8 bytes |
| `-fstack-protector-strong` | broader heuristic: covers functions with any array, address-taken locals, or struct with arrays |
| `-fstack-protector-all` | instruments every function unconditionally; higher overhead but no gaps |

Canaries detect sequential stack overwrites (e.g., `buffer_overflow.c`). They do
not help with heap corruption, format strings, or non-linear overwrites.

### Source fortification

| flag | what it does |
|------|-------------|
| `-D_FORTIFY_SOURCE=1` | compile-time checks on `strcpy`, `memcpy`, `sprintf`, etc. where buffer size is known; requires `-O1` or higher |
| `-D_FORTIFY_SOURCE=2` | adds runtime checks for the above functions; aborts on overflow |
| `-D_FORTIFY_SOURCE=3` | (GCC 12+ / Clang 15+) extends `=2` with `__builtin_dynamic_object_size` for variable-length buffer checks |

Catches `strcpy`/`memcpy`-based overflows when the compiler can determine the
destination size. Does not help with custom copy loops or logic bugs.

### Sanitizers

| flag | what it does | catches | notes |
|------|-------------|---------|-------|
| `-fsanitize=address` (ASan) | shadow memory tracking for heap/stack/global bounds | heap overflow, stack overflow, use-after-free, double free, buffer over-read | ~2x slowdown; incompatible with TSan |
| `-fsanitize=undefined` (UBSan) | inserts runtime checks for undefined behavior | signed overflow, null deref, shift errors, misaligned access | low overhead; can trap or print diagnostics |
| `-fsanitize=thread` (TSan) | tracks memory accesses across threads | data races, race conditions | ~5-15x slowdown; incompatible with ASan |
| `-fsanitize=memory` (MSan) | tracks uninitialized memory reads | uninitialized variables (stack and heap) | clang-only; requires entire program (including libc) to be instrumented for full accuracy |
| `-fsanitize=cfi` | control-flow integrity checks at indirect call sites | type confusion, vtable hijacking | clang-only; requires `-flto` |

ASan is the single most effective runtime tool. MSan fills the one gap ASan
cannot cover (uninitialized reads). TSan is only useful for threaded code.

### Integer overflow

| flag | what it does |
|------|-------------|
| `-ftrapv` | traps (SIGILL) on signed integer overflow; uses a library call (`__addvsi3`), measurable overhead |
| `-fsanitize=signed-integer-overflow` | same detection via UBSan; uses inline checks (`jo` + `ud2`), typically faster than `-ftrapv` |
| `-fwrapv` | makes signed overflow defined (wraps like unsigned); prevents compiler from optimizing based on overflow UB, but does not detect bugs |

`-ftrapv` and UBSan signed-overflow catch the same class of bug. UBSan is
preferred on modern toolchains for both performance and diagnostic quality.

### Control-flow protection

| flag | what it does |
|------|-------------|
| `-fcf-protection=full` | (GCC 8+ / Intel CET) inserts `endbr64` at branch targets and shadow stack for return addresses; hardware-assisted on supported CPUs |
| `-fcf-protection=branch` | indirect branch tracking only (no shadow stack) |
| `-fcf-protection=return` | shadow stack only (no branch tracking) |
| `-fsanitize=cfi` | (clang-only) software CFI; checks that indirect call targets match expected type signatures |

Relevant to `type_confusion.c` where a function pointer is called through the
wrong struct type.

### Umbrella flags

| flag | what it does |
|------|-------------|
| `-fhardened` | (GCC 14+) enables `-fstack-protector-strong`, `-D_FORTIFY_SOURCE=3`, `-fstack-clash-protection`, `-fcf-protection=full`, PIE, and RELRO in one flag |

Convenient but opaque. Use `harden_test.sh` to see exactly what it catches.

### Compile-time warnings (not runtime)

| flag | what it does |
|------|-------------|
| `-Wall -Wextra` | broad warning coverage; catches many bugs at compile time |
| `-Wformat=2` | stricter format string checks; warns about non-literal format args |
| `-Wformat-security` | warns when format string is not a string literal (subset of `-Wformat=2`) |
| `-Werror=format-security` | promotes the above to a hard error; rejects `format_string.c` at build time |
| `-Wuse-after-free` | (GCC 12+) warns about pointer use after free |
| `-Warray-bounds=2` | warns about out-of-bounds array accesses the compiler can prove |

These reject vulnerable code before it runs. The cheapest possible defense.
`harden_test.sh --compile-check` shows which examples fail to build under
strict warnings.

## Linker flags

| flag | what it does |
|------|-------------|
| `-Wl,-z,relro` | partial RELRO: makes ELF headers and GOT (lazy) read-only after relocation |
| `-Wl,-z,relro,-z,now` | full RELRO: resolves all symbols at load time, then makes entire GOT read-only; blocks GOT overwrite attacks |
| `-Wl,-z,noexecstack` | marks the stack non-executable (NX); prevents shellcode execution from stack overflows |
| `-pie -fPIE` | position-independent executable; enables full ASLR for the main binary |
| `-Wl,-z,nodump` | prevents the process from being dumped via `ptrace`; minor hardening |

The Makefile in this repo uses `-no-pie -fno-pie -z execstack` to deliberately
disable these protections so the examples demonstrate their bugs reliably.

## System-level settings

These are kernel/OS controls that affect exploit viability independent of how
the binary was compiled.

| setting | what it does |
|---------|-------------|
| `/proc/sys/kernel/randomize_va_space` | ASLR level: 0 = off, 1 = stack/mmap/VDSO, 2 = full (heap too). Default 2 on modern kernels. |
| `/proc/sys/vm/mmap_min_addr` | lowest virtual address userspace can map. Default 65536. Setting to 0 allows mapping page zero, making null pointer dereferences exploitable. |
| `MALLOC_CHECK_` (env var) | glibc heap consistency checks: 1 = warn, 2 = abort, 3 = both. Catches double-free and some heap corruption without recompilation. Try: `MALLOC_CHECK_=3 ./double_free` |
| NX bit | CPU/OS feature marking memory pages non-executable. Enabled by default on all modern x86-64 kernels. `-z execstack` overrides it. |

`harden_test.sh --sysinfo` displays the current state of these settings.

## Which flags catch which bugs

Run `./harden_test.sh` for the full matrix. Summary of expected results:

| vulnerability class | stack | fortify | asan | ubsan | msan | tsan |
|---------------------|-------|---------|------|-------|------|------|
| stack buffer overflow | yes | yes | yes | - | - | - |
| heap overflow | - | yes | yes | - | - | - |
| format string | - | - | - | - | - | - |
| double free | - | yes | yes | - | - | - |
| use-after-free | - | - | yes* | - | - | - |
| dangling pointer | - | - | yes | - | - | - |
| signedness confusion | - | - | yes | - | - | - |
| integer overflow (signed) | - | - | - | yes | - | - |
| null pointer deref | crash | crash | yes | crash | - | - |
| off-by-one | - | yes | yes | - | - | - |
| uninitialized variable | - | - | - | - | yes | - |
| type confusion | crash | crash | yes | crash | - | - |
| race condition | - | - | - | - | - | yes |
| buffer over-read | - | - | - | - | - | - |
| command injection | - | - | - | - | - | - |
| TOCTOU | - | - | - | - | - | - |

`*` ASan catches use-after-free in many cases but may miss trivial reads that
don't trigger a page fault. `yes` means the protection reliably detects the
bug. `-` means it does not. `crash` means the program dies from a raw signal
but no protection message is printed.

Format strings, command injection, and TOCTOU are logic bugs that no
memory-safety tool detects. Format strings can be rejected at compile time
with `-Werror=format-security`. Command injection requires input validation
or syscall filtering (seccomp). TOCTOU requires using file descriptors
instead of paths for security decisions.
