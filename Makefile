CC = gcc
CFLAGS = -g -fno-stack-protector -z execstack -no-pie -fno-pie

SOURCES = buffer_overflow.c cmd_injection.c double_free.c format_string.c \
          heap_overflow.c integer_overflow.c int_to_buffer_overflow.c null_ptr_deref.c \
          off_by_one_challenge.c race_condition.c stack_off_by_one.c type_confusion.c \
          uninitialized_var.c uninitialized_var2.c use_after_free.c use_after_free2.c

TARGETS = $(SOURCES:.c=)

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
