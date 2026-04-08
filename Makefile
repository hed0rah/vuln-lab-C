CC     = gcc
CFLAGS = -g -fno-stack-protector -z execstack -no-pie -fno-pie -Wall

# disabled protections:
#   -fno-stack-protector  stack canaries off
#   -z execstack          executable stack (for shellcode demos)
#   -no-pie / -fno-pie    fixed load address (simplifies ROP/overflow demos)

SOURCES = \
    buffer_overflow.c    \
    buf_over_read.c      \
    cmd_injection.c      \
    dangling_pointer.c   \
    double_free.c        \
    format_string.c      \
    heap_overflow.c      \
    integer_overflow.c   \
    int_to_buffer_overflow.c \
    null_ptr_deref.c     \
    off_by_one_challenge.c \
    race_condition.c     \
    stack_off_by_one.c   \
    toctou.c             \
    type_confusion.c     \
    uninitialized_var.c  \
    uninitialized_var2.c \
    use_after_free.c     \
    use_after_free2.c

TARGETS = $(SOURCES:.c=)

all: $(TARGETS)

# race_condition needs pthreads
race_condition: race_condition.c
	$(CC) $(CFLAGS) $< -o $@ -lpthread

# default rule for everything else
%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)
