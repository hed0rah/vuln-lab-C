#include <limits.h>
#include <stdio.h>

/* integer overflow examples.

   unsigned overflow: wraps to zero (defined behavior in C).
   commonly exploited to bypass size checks: if (a + b > MAX) looks safe,
   but if a+b wraps, the sum appears small and the check passes.

   signed overflow: undefined behavior in C. compilers may assume it never
   happens and optimize away the check entirely, creating silent logic bugs. */

void unsigned_overflow(void)
{
    unsigned int a = 0xFFFFFFFF;
    unsigned int b = 1;
    unsigned int sum = a + b;      /* wraps to 0 */
    printf("unsigned: 0x%x + 0x%x = 0x%x\n", a, b, sum);

    /* allocation size check that appears to guard against overflow */
    unsigned int size = 0xFFFFFFF0;
    unsigned int extra = 0x20;
    if (size + extra > 0xFFFFFFFF) {
        printf("allocation would overflow (check fired)\n");
    } else {
        printf("allocation size: 0x%x  (check silently bypassed)\n", size + extra);
    }
}

void signed_overflow(void)
{
    int x = INT_MAX;
    int y = x + 1;                 /* UB: signed overflow */
    printf("signed:   INT_MAX + 1 = %d  (UB; result is implementation-defined)\n", y);

    /* compilers exploiting signed overflow UB: the check below may be
       optimized away entirely because the compiler assumes signed overflow
       never happens, making (x + 1 > x) always true */
    if (x + 1 > x) {
        printf("check passed -- compiler may have assumed no overflow\n");
    }
}

int main(void)
{
    unsigned_overflow();
    signed_overflow();
    return 0;
}
