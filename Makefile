CC 		:=  gcc
CFLAGS  := 	-W -Wall -Wextra -pedantic \
			-Wconversion -Wswitch-enum \
			-Wno-nonnull -Wno-nonnull-compare -Wno-format \
			-flto -std=c11

EXEC = main

all: test_libc test_libcj

test_libc: main.c Makefile
	$(CC) $(CFLAGS) $(filter %.c %.s %.o,$^) -o $@

test_libcj: main.c libcj.c libcj.h Makefile
	$(CC) $(CFLAGS) $(filter %.c %.s %.o,$^) -o $@ -DUSE_LIB_CJ

test: test_libc test_libcj
	./test_libc
	./test_libcj
	@ echo "Congrats! All tests passed!"

clean:
	rm -rf test* *.o

.PHONY: all clean run
