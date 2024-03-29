# ----------------------------------------
# Definitions
# ----------------------------------------

ifndef CC
	CC := gcc
endif
CFLAGS := -W -Wall -Wextra -pedantic \
          -Wconversion -Wswitch-enum \
          -Wno-nonnull -Wno-nonnull-compare -Wno-format \
          -flto -std=c11 -O0

# List of functions linked from libcj or libc
FUNCTIONS := tolower toupper \
             memcpy memmove strcpy strncpy strcat strncat \
			 memcmp strcmp strncmp \
			 memchr strchr strcspn strpbrk strchr strspn \
			 strstr strtok memset strlen \
			 atoi atol atoll strtol strtoll strtoul strtoull \
			 atof strtof strtod strtold \
             snprintf sscanf

# Function that uses nm to check if a function name (2nd argument) is undefined in a object file (1st argument)
is_undefined = $(if $(shell nm -uj "$(1)" | grep "$(2)"),undefined)

# Function to check if FUNCTIONS are defined or undefined in an object file (1st argument). 
# The second argument determines the linking behavior: "defined" requires all functions to be defined locally,
# while "undefined" mandates that all functions should be linked from libc.
define check_functions
	$(foreach fn,$(FUNCTIONS),\
		$(if $(call is_undefined,$(1),$(fn)),\
			$(if $(findstring "undefined",$(2)), $(error "$(1) shouldn't link with $(fn) from libc")),\
            $(if $(findstring "defined",$(2)), $(error "$(1) should link with $(fn) from libc"))\
		)\
	)
endef

# ----------------------------------------
# Compilation rules
# ----------------------------------------

all: test_libc test_libcj

test_libc: main.c test.h Makefile
	$(CC) $(CFLAGS) $(filter %.c %.s %.o,$^) -o $@

test_libcj: main.c libcj.c libcj.h test.h Makefile
	$(CC) $(CFLAGS) $(filter %.c %.s %.o,$^) -o $@ -DUSE_LIB_CJ

# ----------------------------------------
# Script rules
# ----------------------------------------

test: test_libc test_libcj
	$(call check_functions,test_libc,"defined")
	$(call check_functions,test_libcj,"undefined")
	./test_libc
	./test_libcj
	@ echo "Congrats! All tests passed!"

clean:
	rm -rf test_lib* *.o

.PHONY: all test clean

# ----------------------------------------
