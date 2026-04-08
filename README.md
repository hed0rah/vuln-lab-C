# C vulnerability examples

Minimal, self-contained C programs demonstrating common memory and logic
vulnerabilities. Each file is one bug, with comments explaining what goes
wrong and why it matters.

## Build

```sh
make        # build all examples
make clean
```

Compiled with protections deliberately disabled:

| flag | what it disables |
|------|-----------------|
| `-fno-stack-protector` | stack canaries |
| `-z execstack` | NX (non-executable stack) |
| `-no-pie -fno-pie` | ASLR / position-independent executable |

`-Wall` is left on. Many warnings point directly at the bugs -- that is intentional.

## Examples

| file | vulnerability | notes |
|------|--------------|-------|
| `buffer_overflow.c` | stack buffer overflow | `strcpy` into fixed buffer via `argv` |
| `buf_over_read.c` | buffer over-read | Heartbleed-style: claimed length > actual length |
| `cmd_injection.c` | command injection | unsanitized input passed to `system()` |
| `dangling_pointer.c` | dangling pointer | stale pointer aliases new heap allocation |
| `double_free.c` | double free | `free()` called twice on same pointer |
| `format_string.c` | format string | user input passed directly as `printf` format |
| `heap_overflow.c` | heap overflow | `strcpy` into heap buffer with no bounds check |
| `integer_overflow.c` | integer overflow | unsigned wrap and signed UB; bypasses size checks |
| `int_to_buffer_overflow.c` | signedness confusion | negative length passes signed check, wraps in `memcpy` |
| `null_ptr_deref.c` | null pointer dereference | direct dereference of NULL |
| `off_by_one_challenge.c` | off-by-one (CTF) | off-by-one overwrites `target`; calls `win()` if triggered |
| `race_condition.c` | race condition | unsynchronized read-modify-write across two threads |
| `stack_off_by_one.c` | off-by-one (stack) | `strlen <= N` allows N+1 byte write into N-byte buffer |
| `toctou.c` | TOCTOU | `access()` check and `open()` use separated by a race window |
| `type_confusion.c` | type confusion | struct cast mismatch; function pointer called from wrong type |
| `uninitialized_var.c` | uninitialized variable (stack) | uninitialized buffer may contain prior stack data |
| `uninitialized_var2.c` | uninitialized variable (heap) | freed heap data may appear in later uninitialized reads |
| `use_after_free.c` | use-after-free | read through pointer after `free()` |
| `use_after_free2.c` | use-after-free (reuse) | shows allocator reusing the freed address |

## Quick demos

```sh
# buf_over_read: claim 40 bytes but only provide 5
./buf_over_read hello 40

# signedness: -1 passes the > 64 check, wraps in memcpy
./int_to_buffer_overflow -1

# race condition: expected 200, gets less
./race_condition

# TOCTOU: run the program, replace the file in the 2-second window
./toctou /tmp/testfile &
# in another shell: ln -sf /etc/passwd /tmp/testfile
```

## Hardening flag test matrix

`harden_test.sh` recompiles each example with different protection flag sets and
shows which ones detect which bugs. Requires gcc with sanitizer support (libasan,
libubsan, libtsan).

```sh
./harden_test.sh              # all columns
./harden_test.sh asan ubsan   # specific columns only
```

Columns: `stack` (-fstack-protector-all), `fortify` (-D_FORTIFY_SOURCE=2),
`asan` (AddressSanitizer), `ubsan` (UBSan), `tsan` (ThreadSanitizer),
`full` (stack + fortify + PIE + RELRO).

Results show `CAUGHT` (protection fired), `miss` (not detected), or `crash`
(raw signal without a protection message -- still broken, just not cleanly caught).
