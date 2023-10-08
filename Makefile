CC 		:=  gcc
CFLAGS  := 	-W -Wall -Wextra -pedantic \
			-Wconversion -Wswitch-enum \
			-Wno-nonnull -Wno-nonnull-compare -Wno-format \
			-flto -std=c11

EXEC = main

all: $(EXEC)

%: %.c Makefile
	$(CC) $(CFLAGS) $(filter %.c %.s %.o,$^) -o $@

run: $(EXEC)
	./$(EXEC)

clean:
	rm -rf $(EXEC) *.o

.PHONY: all clean run
