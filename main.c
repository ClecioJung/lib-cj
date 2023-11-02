// MIT License

// Copyright (c) 2023 CLECIO JUNG <clecio.jung@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//------------------------------------------------------------------------------
// SOURCE
//------------------------------------------------------------------------------

#ifdef USE_LIB_CJ
#include "libcj.h"

// Definitions from stdio.h, so that we don't need to include it.
// That header does some macro magic so that sscanf calls get redirected to
// __isoc99_sscanf, which is not what we want here.
typedef struct FILE FILE;
extern FILE *stderr;
extern int fprintf (FILE *stream, const char *format, ...);

#else // USE_LIB_CJ
#define _GNU_SOURCE
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#endif // USE_LIB_CJ

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#define TEST_IMPLEMENTATION
#include "test.h"

//------------------------------------------------------------------------------
// CTYPE.H
//------------------------------------------------------------------------------

// Test all the code classification functions defined in ctype.h
// Actually, in my libc, this functions are defined as macros
static void check_code_fns(void)
{
    // Check all of the ASCII characters
    for (int c = 0x0; c <= 0x7F; c++) {
        if (((0x00 <= c) && (c <= 0x08)) || ((0x0E <= c) && (c <= 0x1F)) || (c == 0x7F)) {
            EXPECT_TRUE(iscntrl(c));
            EXPECT_FALSE(isblank(c));
            EXPECT_FALSE(isspace(c));
            EXPECT_FALSE(isupper(c));
            EXPECT_FALSE(islower(c));
            EXPECT_FALSE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_FALSE(isxdigit(c));
            EXPECT_FALSE(isalnum(c));
            EXPECT_FALSE(ispunct(c));
            EXPECT_FALSE(isgraph(c));
            EXPECT_FALSE(isprint(c));
        } else if (c == 0x09) { // Tab: '\t'
            EXPECT_TRUE(iscntrl(c));
            EXPECT_TRUE(isblank(c));
            EXPECT_TRUE(isspace(c));
            EXPECT_FALSE(isupper(c));
            EXPECT_FALSE(islower(c));
            EXPECT_FALSE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_FALSE(isxdigit(c));
            EXPECT_FALSE(isalnum(c));
            EXPECT_FALSE(ispunct(c));
            EXPECT_FALSE(isgraph(c));
            EXPECT_FALSE(isprint(c));
        } else if ((0x0A <= c) && (c <= 0x0D)) { // white-space control codes: '\f','\v','\n','\r'
            EXPECT_TRUE(iscntrl(c));
            EXPECT_FALSE(isblank(c));
            EXPECT_TRUE(isspace(c));
            EXPECT_FALSE(isupper(c));
            EXPECT_FALSE(islower(c));
            EXPECT_FALSE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_FALSE(isxdigit(c));
            EXPECT_FALSE(isalnum(c));
            EXPECT_FALSE(ispunct(c));
            EXPECT_FALSE(isgraph(c));
            EXPECT_FALSE(isprint(c));
        } else if (c == 0x20) { // Space: ' '
            EXPECT_FALSE(iscntrl(c));
            EXPECT_TRUE(isblank(c));
            EXPECT_TRUE(isspace(c));
            EXPECT_FALSE(isupper(c));
            EXPECT_FALSE(islower(c));
            EXPECT_FALSE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_FALSE(isxdigit(c));
            EXPECT_FALSE(isalnum(c));
            EXPECT_FALSE(ispunct(c));
            EXPECT_FALSE(isgraph(c));
            EXPECT_TRUE(isprint(c));
        } else if ((0x21 <= c) && (c <= 0x2F)) { // !"#$%&'()*+,-./
            EXPECT_FALSE(iscntrl(c));
            EXPECT_FALSE(isblank(c));
            EXPECT_FALSE(isspace(c));
            EXPECT_FALSE(isupper(c));
            EXPECT_FALSE(islower(c));
            EXPECT_FALSE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_FALSE(isxdigit(c));
            EXPECT_FALSE(isalnum(c));
            EXPECT_TRUE(ispunct(c));
            EXPECT_TRUE(isgraph(c));
            EXPECT_TRUE(isprint(c));
        } else if ((0x30 <= c) && (c <= 0x39)) { // 0123456789
            EXPECT_FALSE(iscntrl(c));
            EXPECT_FALSE(isblank(c));
            EXPECT_FALSE(isspace(c));
            EXPECT_FALSE(isupper(c));
            EXPECT_FALSE(islower(c));
            EXPECT_FALSE(isalpha(c));
            EXPECT_TRUE(isdigit(c));
            EXPECT_TRUE(isxdigit(c));
            EXPECT_TRUE(isalnum(c));
            EXPECT_FALSE(ispunct(c));
            EXPECT_TRUE(isgraph(c));
            EXPECT_TRUE(isprint(c));
        } else if (
            ((0x3A <= c) && (c <= 0x40)) || // :;<=>?@
            ((0x5B <= c) && (c <= 0x60)) || // [\]^_`
            ((0x7B <= c) && (c <= 0x7E))) { // {|}~
            EXPECT_FALSE(iscntrl(c));
            EXPECT_FALSE(isblank(c));
            EXPECT_FALSE(isspace(c));
            EXPECT_FALSE(isupper(c));
            EXPECT_FALSE(islower(c));
            EXPECT_FALSE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_FALSE(isxdigit(c));
            EXPECT_FALSE(isalnum(c));
            EXPECT_TRUE(ispunct(c));
            EXPECT_TRUE(isgraph(c));
            EXPECT_TRUE(isprint(c));
        } else if ((0x41 <= c) && (c <= 0x46)) { // ABCDEF
            EXPECT_FALSE(iscntrl(c));
            EXPECT_FALSE(isblank(c));
            EXPECT_FALSE(isspace(c));
            EXPECT_TRUE(isupper(c));
            EXPECT_FALSE(islower(c));
            EXPECT_TRUE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_TRUE(isxdigit(c));
            EXPECT_TRUE(isalnum(c));
            EXPECT_FALSE(ispunct(c));
            EXPECT_TRUE(isgraph(c));
            EXPECT_TRUE(isprint(c));
        } else if ((0x47 <= c) && (c <= 0x5A)) { // GHIJKLMNOPQRSTUVWXYZ
            EXPECT_FALSE(iscntrl(c));
            EXPECT_FALSE(isblank(c));
            EXPECT_FALSE(isspace(c));
            EXPECT_TRUE(isupper(c));
            EXPECT_FALSE(islower(c));
            EXPECT_TRUE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_FALSE(isxdigit(c));
            EXPECT_TRUE(isalnum(c));
            EXPECT_FALSE(ispunct(c));
            EXPECT_TRUE(isgraph(c));
            EXPECT_TRUE(isprint(c));
        } else if ((0x61 <= c) && (c <= 0x66)) { // abcdef
            EXPECT_FALSE(iscntrl(c));
            EXPECT_FALSE(isblank(c));
            EXPECT_FALSE(isspace(c));
            EXPECT_FALSE(isupper(c));
            EXPECT_TRUE(islower(c));
            EXPECT_TRUE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_TRUE(isxdigit(c));
            EXPECT_TRUE(isalnum(c));
            EXPECT_FALSE(ispunct(c));
            EXPECT_TRUE(isgraph(c));
            EXPECT_TRUE(isprint(c));
        } else if ((0x67 <= c) && (c <= 0x7A)) { // ghijklmnopqrstuvwxyz
            EXPECT_FALSE(iscntrl(c));
            EXPECT_FALSE(isblank(c));
            EXPECT_FALSE(isspace(c));
            EXPECT_FALSE(isupper(c));
            EXPECT_TRUE(islower(c));
            EXPECT_TRUE(isalpha(c));
            EXPECT_FALSE(isdigit(c));
            EXPECT_FALSE(isxdigit(c));
            EXPECT_TRUE(isalnum(c));
            EXPECT_FALSE(ispunct(c));
            EXPECT_TRUE(isgraph(c));
            EXPECT_TRUE(isprint(c));
        } else {
            fprintf(stderr, "%s:%u [TEST FAILED] unhandled character '%c' (%#x) in %s\n", __FILE__, __LINE__, c, c, __func__);
            exit(EXIT_FAILURE);
        }
    }
}

void check_conversion_fns(void)
{
    // tolower
    EXPECT_INT(tolower(' '), ' ');
    EXPECT_INT(tolower('0'), '0');
    EXPECT_INT(tolower('a'), 'a');
    EXPECT_INT(tolower('A'), 'a');
    EXPECT_INT(tolower('z'), 'z');
    EXPECT_INT(tolower('Z'), 'z');
    // toupper
    EXPECT_INT(toupper(' '), ' ');
    EXPECT_INT(toupper('0'), '0');
    EXPECT_INT(toupper('a'), 'A');
    EXPECT_INT(toupper('A'), 'A');
    EXPECT_INT(toupper('z'), 'Z');
    EXPECT_INT(toupper('Z'), 'Z');
}

static void check_ctype(void)
{
    check_code_fns();
    check_conversion_fns();
}

//------------------------------------------------------------------------------
// STRING.H
//------------------------------------------------------------------------------

static void check_memcpy(void)
{
    char dst[32];
    const char *const hello_world = "Hello World!";
    const char *const test = "Test";
    EXPECT_PTR(memcpy(dst, hello_world, 13), dst);
    EXPECT_STR(dst, "Hello World!");
    EXPECT_PTR(memcpy(dst, test, 4), dst);
    EXPECT_SIZED_STR(dst, "Test", 4);
}

static void check_memmove(void)
{
    char dst[32];
    const char *const hello_world = "Hello World!";
    EXPECT_PTR(memmove(dst, hello_world, 13), dst);
    EXPECT_STR(dst, "Hello World!");
    EXPECT_PTR(memmove(dst, dst, 13), dst);
    EXPECT_STR(dst, "Hello World!");
    EXPECT_PTR(memmove(&dst[2], dst, 13), &dst[2]);
    EXPECT_STR(dst, "HeHello World!");
    EXPECT_PTR(memmove(dst, &dst[2], 13), dst);
    EXPECT_STR(dst, "Hello World!");
}

static void check_strcpy(void)
{
    char dst[32];
    const char *const hello_world = "Hello World!";
    EXPECT_PTR(strcpy(dst, hello_world), dst);
    EXPECT_STR(dst, "Hello World!");
}

static void check_strncpy(void)
{
    char dst[32];
    const char *const hello_world = "Hello World!";
    const char *const test = "Testing";
    EXPECT_PTR(strncpy(dst, hello_world, sizeof(dst)), dst);
    EXPECT_STR(dst, "Hello World!");
    for (size_t i = strlen(dst); i < sizeof(dst); i++) {
        EXPECT_CHAR(dst[i], '\0');
    }
    EXPECT_PTR(strncpy(dst, test, 4), dst);
    EXPECT_SIZED_STR(dst, "Test", 4);
    EXPECT_CHAR_NOT(dst[5], '\0');
}

static void check_strcat(void)
{
    char dst[32] = {0};
    const char *const hello = "Hello";
    const char *const world = " World";
    const char *const exclamation = "!";
    EXPECT_PTR(strcat(dst, hello), dst);
    EXPECT_STR(dst, "Hello");
    EXPECT_PTR(strcat(dst, world), dst);
    EXPECT_STR(dst, "Hello World");
    EXPECT_PTR(strcat(dst, exclamation), dst);
    EXPECT_STR(dst, "Hello World!");
}

static void check_strncat(void)
{
    char dst[32] = {0};
    const char *const hello_world = "Hello World!";
    const char *const world = " World";
    const char *const exclamation = "!";
    EXPECT_PTR(strncat(dst, hello_world, 5), dst);
    EXPECT_STR(dst, "Hello");
    EXPECT_PTR(strncat(dst, world, 10), dst);
    EXPECT_STR(dst, "Hello World");
    EXPECT_PTR(strncat(dst, exclamation, 1), dst);
    EXPECT_STR(dst, "Hello World!");
}

static void check_memcmp(void)
{
    const char *const hello = "Hello";
    const char *const hello_world = "Hello World!";
    const char *const test = "Test";
    EXPECT_INT(memcmp(hello, test, 4), -12);
    EXPECT_INT(memcmp(hello, hello_world, 5), 0);
    EXPECT_INT(memcmp(hello_world, hello, 6), 32);
}

static void check_strcmp(void)
{
    const char *const hello = "Hello";
    const char *const hello_world = "Hello World!";
    const char *const test = "Test";
    EXPECT_INT(strcmp(hello, test), -12);
    EXPECT_INT(strcmp(hello_world, hello), 32);
    EXPECT_INT(strcmp("Hello World!", hello_world), 0);
}

static void check_strncmp(void)
{
    const char *const hello = "Hello";
    const char *const hello_world = "Hello World!";
    const char *const test = "Test";
    EXPECT_INT(strncmp(hello, test, 4), -12);
    EXPECT_INT(strncmp("Testing", test, 4), 0);
    EXPECT_INT(strncmp(hello_world, hello, 5), 0);
    EXPECT_INT(strncmp(hello_world, hello, 13), 32);
    EXPECT_INT(strncmp("Hello World!", hello_world, 32), 0);
}

static void check_memchr(void)
{
    const char *const hello = "Hello\0world";
    EXPECT_PTR(memchr(hello, 't', 5), NULL);
    EXPECT_PTR(memchr(hello, 'H', 5), hello);
    EXPECT_PTR(memchr(hello, 'o', 5), &hello[4]);
    EXPECT_PTR(memchr(hello, 'w', 12), &hello[6]);
}

static void check_strchr(void)
{
    const char *const hello = "Hello";
    EXPECT_PTR(strchr(hello, 't'), NULL);
    EXPECT_PTR(strchr(hello, 'H'), hello);
    EXPECT_PTR(strchr(hello, 'o'), &hello[4]);
    EXPECT_PTR(strchr(hello, 'l'), &hello[2]);
}

static void check_strcspn(void)
{
    const char *const hello = "Hello";
    EXPECT_SIZE(strcspn(hello, "t"), 5);
    EXPECT_SIZE(strcspn(hello, "H"), 0);
    EXPECT_SIZE(strcspn(hello, "lo"), 2);
    EXPECT_SIZE(strcspn(hello, "ol"), 2);
    EXPECT_SIZE(strcspn(hello, "oe"), 1);
}

static void check_strpbrk(void)
{
    const char *const hello = "Hello";
    EXPECT_PTR(strpbrk(hello, "t"), NULL);
    EXPECT_PTR(strpbrk(hello, "H"), hello);
    EXPECT_PTR(strpbrk(hello, "lo"), &hello[2]);
    EXPECT_PTR(strpbrk(hello, "ol"), &hello[2]);
    EXPECT_PTR(strpbrk(hello, "oe"), &hello[1]);
}

static void check_strrchr(void)
{
    const char *const hello = "Hello";
    EXPECT_PTR(strrchr(hello, 't'), NULL);
    EXPECT_PTR(strrchr(hello, 'H'), hello);
    EXPECT_PTR(strrchr(hello, 'o'), &hello[4]);
    EXPECT_PTR(strrchr(hello, 'l'), &hello[3]);
}

static void check_strspn(void)
{
    const char *const hello = "Hello";
    EXPECT_SIZE(strspn(hello, "t"), 0);
    EXPECT_SIZE(strspn(hello, "H"), 1);
    EXPECT_SIZE(strspn(hello, "lo"), 0);
    EXPECT_SIZE(strspn(hello, "He"), 2);
    EXPECT_SIZE(strspn(hello, "lHe"), 4);
    EXPECT_SIZE(strspn(hello, "lHeoe"), 5);
}

static void check_strstr(void)
{
    const char *const hello_world = "Hello World!";
    EXPECT_PTR(strstr(hello_world, "Hello"), hello_world);
    EXPECT_PTR(strstr(hello_world, "Help"), NULL);
    EXPECT_PTR(strstr(hello_world, "World"), &hello_world[6]);
    EXPECT_PTR(strstr(hello_world, "World!2"), NULL);
}

static void check_strtok(void)
{
    char hello_world[] = "Hello World!";
    EXPECT_STR(strtok(hello_world, " !"), "Hello");
    EXPECT_STR(strtok(NULL, " !"), "World");
    EXPECT_PTR(strtok(NULL, " !"), NULL);
    EXPECT_PTR(strtok(NULL, " !"), NULL);
    char example[] = "!A simple, but effective, test sentence!";
    EXPECT_STR(strtok(example, " !,"), "A");
    EXPECT_STR(strtok(NULL, " !,"), "simple");
    EXPECT_STR(strtok(NULL, " !,"), "but");
    EXPECT_STR(strtok(NULL, " !,"), "effective");
    EXPECT_STR(strtok(NULL, " !,"), "test");
    EXPECT_STR(strtok(NULL, " !,"), "sentence");
    EXPECT_PTR(strtok(NULL, " !,"), NULL);
    EXPECT_PTR(strtok(NULL, " !,"), NULL);
}

static void check_memset(void)
{
    char buf[32];
    EXPECT_PTR(memset(buf, 'S', sizeof(buf)), buf);
    for (size_t i = 0; i < sizeof(buf); i++) {
        EXPECT_CHAR(buf[i], 'S');
    }
    EXPECT_PTR(memset(buf, 0, sizeof(buf)), buf);
    for (size_t i = 0; i < sizeof(buf); i++) {
        EXPECT_CHAR(buf[i], 0);
    }
}

static void check_strlen(void)
{
    const char *const empty_str = "";
    const char *const hello_world = "Hello World!";
    const char *const test = "Test";
    EXPECT_SIZE(strlen(empty_str), 0);
    EXPECT_SIZE(strlen(hello_world), 12);
    EXPECT_SIZE(strlen(test), 4);
}

static void check_strtok_r(void)
{
    char hello_world[] = "Hello World!";
    char *ptr;
    EXPECT_STR(strtok_r(hello_world, " !", &ptr), "Hello");
    EXPECT_STR(strtok_r(NULL, " !", &ptr), "World");
    EXPECT_PTR(strtok_r(NULL, " !", &ptr), NULL);
    EXPECT_PTR(strtok_r(NULL, " !", &ptr), NULL);
    char example[] = "!A simple, but effective, test sentence!";
    EXPECT_STR(strtok_r(example, " !,", &ptr), "A");
    EXPECT_STR(strtok_r(NULL, " !,", &ptr), "simple");
    EXPECT_STR(strtok_r(NULL, " !,", &ptr), "but");
    EXPECT_STR(strtok_r(NULL, " !,", &ptr), "effective");
    EXPECT_STR(strtok_r(NULL, " !,", &ptr), "test");
    EXPECT_STR(strtok_r(NULL, " !,", &ptr), "sentence");
    EXPECT_PTR(strtok_r(NULL, " !,", &ptr), NULL);
    EXPECT_PTR(strtok_r(NULL, " !,", &ptr), NULL);
}

static void check_strsep(void)
{
    char hello_world[] = "Hello World!";
    char *ptr = NULL;
    EXPECT_PTR(strsep(&ptr, " "), NULL);
    EXPECT_PTR(ptr, NULL);
    ptr = hello_world;
    EXPECT_STR(strsep(&ptr, " !"), "Hello");
    EXPECT_CHAR(*ptr, 'W');
    EXPECT_STR(strsep(&ptr, " !"), "World");
    EXPECT_CHAR(*ptr, '\0');
    EXPECT_STR(strsep(&ptr, " !"), "");
    EXPECT_PTR(ptr, NULL);
    EXPECT_PTR(strsep(&ptr, " !"), NULL);
    char example[] = "!A simple, but effective, test sentence!";
    ptr = example;
    EXPECT_STR(strsep(&ptr, " !,"), "");
    EXPECT_STR(strsep(&ptr, " !,"), "A");
    EXPECT_STR(strsep(&ptr, " !,"), "simple");
    EXPECT_STR(strsep(&ptr, " !,"), "");
    EXPECT_STR(strsep(&ptr, " !,"), "but");
    EXPECT_STR(strsep(&ptr, " !,"), "effective");
    EXPECT_STR(strsep(&ptr, " !,"), "");
    EXPECT_STR(strsep(&ptr, " !,"), "test");
    EXPECT_STR(strsep(&ptr, " !,"), "sentence");
    EXPECT_STR(strsep(&ptr, " !,"), "");
    EXPECT_PTR(strsep(&ptr, " !,"), NULL);
    EXPECT_PTR(strsep(&ptr, " !,"), NULL);
}

static void check_cstring(void)
{
    check_memcpy();
    check_memmove();
    check_strcpy();
    check_strncpy();
    check_strcat();
    check_strncat();
    check_memcmp();
    check_strcmp();
    check_strncmp();
    check_memchr();
    check_strchr();
    check_strcspn();
    check_strpbrk();
    check_strrchr();
    check_strspn();
    check_strstr();
    check_strtok();
    check_memset();
    check_strlen();
    check_strtok_r();
    check_strsep();
}

//------------------------------------------------------------------------------
// STDLIB.H
//------------------------------------------------------------------------------

static void check_atoi(void)
{
    EXPECT_INT(atoi("  85"), 85);
    EXPECT_INT(atoi("256"), 256);
    EXPECT_INT(atoi("+23"), 23);
    EXPECT_INT(atoi(" -375"), -375);
    EXPECT_INT(atoi("0"), 0);
    EXPECT_INT(atoi("+0"), 0);
    EXPECT_INT(atoi("-0"), 0);
    EXPECT_INT(atoi("-1"), -1);
    EXPECT_INT(atoi("123alpha"), 123);
    EXPECT_INT(atoi("12b3"), 12);
    EXPECT_INT(atoi("alpha123"), 0);
    EXPECT_INT(atoi("2147483647"), INT_MAX);
    EXPECT_INT(atoi("2147483646"), 2147483646);
    EXPECT_INT(atoi("-2147483648"), INT_MIN);
    EXPECT_INT(atoi("-2147483647"), -2147483647);
}

static void check_atol(void)
{
    EXPECT_LONG(atol("  85"), 85L);
    EXPECT_LONG(atol("256"), 256L);
    EXPECT_LONG(atol("+23"), 23L);
    EXPECT_LONG(atol(" -375"), -375L);
    EXPECT_LONG(atol("0"), 0L);
    EXPECT_LONG(atol("+0"), 0L);
    EXPECT_LONG(atol("-0"), 0L);
    EXPECT_LONG(atol("-1"), -1L);
    EXPECT_LONG(atol("123alpha"), 123L);
    EXPECT_LONG(atol("12b3"), 12L);
    EXPECT_LONG(atol("alpha123"), 0L);
    EXPECT_LONG(atol("9223372036854775807"), LONG_MAX);
    EXPECT_LONG(atol("9223372036854775806"), 9223372036854775806L);
    EXPECT_LONG(atol("-9223372036854775808"), LONG_MIN);
    EXPECT_LONG(atol("-9223372036854775807"), -9223372036854775807L);
}

static void check_atoll(void)
{
    EXPECT_LLONG(atoll("  85"), 85LL);
    EXPECT_LLONG(atoll("256"), 256LL);
    EXPECT_LLONG(atoll("+23"), 23LL);
    EXPECT_LLONG(atoll(" -375"), -375LL);
    EXPECT_LLONG(atoll("0"), 0LL);
    EXPECT_LLONG(atoll("+0"), 0LL);
    EXPECT_LLONG(atoll("-0"), 0LL);
    EXPECT_LLONG(atoll("-1"), -1LL);
    EXPECT_LLONG(atoll("123alpha"), 123LL);
    EXPECT_LLONG(atoll("12b3"), 12LL);
    EXPECT_LLONG(atoll("alpha123"), 0LL);
    EXPECT_LLONG(atoll("9223372036854775807"), LLONG_MAX);
    EXPECT_LLONG(atoll("9223372036854775806"), 9223372036854775806LL);
    EXPECT_LLONG(atoll("-9223372036854775808"), LLONG_MIN);
    EXPECT_LLONG(atoll("-9223372036854775807"), -9223372036854775807LL);
}

static void check_strtol(void)
{
    char *endptr;
    EXPECT_LONG(strtol("  85", NULL, 10), 85L);
    EXPECT_LONG(strtol("  85", &endptr, 10), 85L);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_LONG(strtol("256", NULL, 10), 256L);
    EXPECT_LONG(strtol("+23", NULL, 10), 23L);
    EXPECT_LONG(strtol(" -375", NULL, 10), -375L);
    EXPECT_LONG(strtol("0", NULL, 10), 0L);
    EXPECT_LONG(strtol("+0", NULL, 10), 0L);
    EXPECT_LONG(strtol("-0", NULL, 10), 0L);
    EXPECT_LONG(strtol("-1", NULL, 10), -1L);
    EXPECT_LONG(strtol("123alpha", &endptr, 10), 123L);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LONG(strtol("12b3", &endptr, 10), 12L);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_LONG(strtol("alpha123", &endptr, 10), 0L);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LONG(strtol("9223372036854775807", NULL, 10), LONG_MAX);
    EXPECT_LONG(strtol("9223372036854775806", NULL, 10), 9223372036854775806L);
    EXPECT_LONG(strtol("-9223372036854775808", NULL, 10), LONG_MIN);
    EXPECT_LONG(strtol("-9223372036854775807", NULL, 10), -9223372036854775807L);
    // Octal numbers
    EXPECT_LONG(strtol("  125", NULL, 8), 85L);
    EXPECT_LONG(strtol("  125", &endptr, 8), 85L);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_LONG(strtol("1238", &endptr, 8), 83L);
    EXPECT_CHAR(*endptr, '8');
    EXPECT_LONG(strtol("777777777777777777777", NULL, 8), LONG_MAX);
    EXPECT_LONG(strtol("777777777777777777776", NULL, 8), 9223372036854775806L);
    EXPECT_LONG(strtol("-1000000000000000000000", NULL, 8), LONG_MIN);
    EXPECT_LONG(strtol("-777777777777777777777", NULL, 8), -9223372036854775807L);
    EXPECT_LONG(strtol("0777777777777777777777", NULL, 8), LONG_MAX);
    EXPECT_LONG(strtol("073", NULL, 8), 59L);
    // Hexadecimal numbers
    EXPECT_LONG(strtol("  55", NULL, 16), 85L);
    EXPECT_LONG(strtol("  55", &endptr, 16), 85L);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_LONG(strtol("123g", &endptr, 16), 291L);
    EXPECT_CHAR(*endptr, 'g');
    EXPECT_LONG(strtol("7fffffffffffffff", NULL, 16), LONG_MAX);
    EXPECT_LONG(strtol("7ffffffffffffffe", NULL, 16), 9223372036854775806L);
    EXPECT_LONG(strtol("-8000000000000000", NULL, 16), LONG_MIN);
    EXPECT_LONG(strtol("-7fffffffffffffff", NULL, 16), -9223372036854775807L);
    EXPECT_LONG(strtol("0x7f0", NULL, 16), 2032);
    EXPECT_LONG(strtol("0x7fffffffffffffff", NULL, 16), LONG_MAX);
    // Informing zero base
    EXPECT_LONG(strtol("  85", NULL, 0), 85L);
    EXPECT_LONG(strtol("  85", &endptr, 0), 85L);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_LONG(strtol("256", NULL, 0), 256L);
    EXPECT_LONG(strtol("+23", NULL, 0), 23L);
    EXPECT_LONG(strtol(" -375", NULL, 0), -375L);
    EXPECT_LONG(strtol("0", NULL, 0), 0L);
    EXPECT_LONG(strtol("+0", NULL, 0), 0L);
    EXPECT_LONG(strtol("-0", NULL, 0), 0L);
    EXPECT_LONG(strtol("-1", NULL, 0), -1L);
    EXPECT_LONG(strtol("123alpha", &endptr, 0), 123L);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LONG(strtol("12b3", &endptr, 0), 12L);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_LONG(strtol("alpha123", &endptr, 0), 0L);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LONG(strtol("9223372036854775807", NULL, 0), LONG_MAX);
    EXPECT_LONG(strtol("9223372036854775806", NULL, 0), 9223372036854775806L);
    EXPECT_LONG(strtol("-9223372036854775808", NULL, 0), LONG_MIN);
    EXPECT_LONG(strtol("-9223372036854775807", NULL, 0), -9223372036854775807L);
    EXPECT_LONG(strtol("0x123alpha", &endptr, 0), 4666L);
    EXPECT_CHAR(*endptr, 'l');
    EXPECT_LONG(strtol("01238lpha", &endptr, 0), 83L);
    EXPECT_CHAR(*endptr, '8');
    // Custom base
    EXPECT_LONG(strtol("  zzz", NULL, 36), 46655L);
    EXPECT_LONG(strtol("  zzz]", &endptr, 36), 46655L);
    EXPECT_CHAR(*endptr, ']');
    EXPECT_LONG(strtol("  hh", NULL, 30), 527L);
    EXPECT_LONG(strtol("  hhz", &endptr, 30), 527L);
    EXPECT_CHAR(*endptr, 'z');
    // Invalid base
    EXPECT_LONG(strtol("0", NULL, 1), 0L);
    EXPECT_LONG(strtol("1", NULL, 1), 0L);
    EXPECT_LONG(strtol("123", NULL, 42), 0L);
    EXPECT_LONG(strtol("zzz", NULL, 42), 0L);
    EXPECT_LONG(strtol("0123", NULL, -2), 0L);
}

static void check_strtoll(void)
{
    char *endptr;
    EXPECT_LLONG(strtoll("  85", NULL, 10), 85LL);
    EXPECT_LLONG(strtoll("  85", &endptr, 10), 85LL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_LLONG(strtoll("256", NULL, 10), 256LL);
    EXPECT_LLONG(strtoll("+23", NULL, 10), 23LL);
    EXPECT_LLONG(strtoll(" -375", NULL, 10), -375LL);
    EXPECT_LLONG(strtoll("0", NULL, 10), 0LL);
    EXPECT_LLONG(strtoll("+0", NULL, 10), 0LL);
    EXPECT_LLONG(strtoll("-0", NULL, 10), 0LL);
    EXPECT_LLONG(strtoll("-1", NULL, 10), -1LL);
    EXPECT_LLONG(strtoll("123alpha", &endptr, 10), 123LL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LLONG(strtoll("12b3", &endptr, 10), 12LL);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_LLONG(strtoll("alpha123", &endptr, 10), 0LL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LLONG(strtoll("9223372036854775807", NULL, 10), LLONG_MAX);
    EXPECT_LLONG(strtoll("9223372036854775806", NULL, 10), 9223372036854775806LL);
    EXPECT_LLONG(strtoll("-9223372036854775808", NULL, 10), LLONG_MIN);
    EXPECT_LLONG(strtoll("-9223372036854775807", NULL, 10), -9223372036854775807LL);
    // Octal numbers
    EXPECT_LLONG(strtoll("  125", NULL, 8), 85LL);
    EXPECT_LLONG(strtoll("  125", &endptr, 8), 85LL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_LLONG(strtoll("1238", &endptr, 8), 83LL);
    EXPECT_CHAR(*endptr, '8');
    EXPECT_LLONG(strtoll("777777777777777777777", NULL, 8), LLONG_MAX);
    EXPECT_LLONG(strtoll("777777777777777777776", NULL, 8), 9223372036854775806LL);
    EXPECT_LLONG(strtoll("-1000000000000000000000", NULL, 8), LLONG_MIN);
    EXPECT_LLONG(strtoll("-777777777777777777777", NULL, 8), -9223372036854775807LL);
    EXPECT_LLONG(strtoll("0777777777777777777777", NULL, 8), LLONG_MAX);
    EXPECT_LLONG(strtoll("073", NULL, 8), 59LL);
    // Hexadecimal numbers
    EXPECT_LLONG(strtoll("  55", NULL, 16), 85LL);
    EXPECT_LLONG(strtoll("  55", &endptr, 16), 85LL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_LLONG(strtoll("123g", &endptr, 16), 291LL);
    EXPECT_CHAR(*endptr, 'g');
    EXPECT_LLONG(strtoll("7fffffffffffffff", NULL, 16), LLONG_MAX);
    EXPECT_LLONG(strtoll("7ffffffffffffffe", NULL, 16), 9223372036854775806LL);
    EXPECT_LLONG(strtoll("-8000000000000000", NULL, 16), LLONG_MIN);
    EXPECT_LLONG(strtoll("-7fffffffffffffff", NULL, 16), -9223372036854775807LL);
    EXPECT_LLONG(strtoll("0x7f0", NULL, 16), 2032);
    EXPECT_LLONG(strtoll("0x7fffffffffffffff", NULL, 16), LLONG_MAX);
    // Informing zero base
    EXPECT_LLONG(strtoll("  85", NULL, 0), 85LL);
    EXPECT_LLONG(strtoll("  85", &endptr, 0), 85LL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_LLONG(strtoll("256", NULL, 0), 256LL);
    EXPECT_LLONG(strtoll("+23", NULL, 0), 23LL);
    EXPECT_LLONG(strtoll(" -375", NULL, 0), -375LL);
    EXPECT_LLONG(strtoll("0", NULL, 0), 0LL);
    EXPECT_LLONG(strtoll("+0", NULL, 0), 0LL);
    EXPECT_LLONG(strtoll("-0", NULL, 0), 0LL);
    EXPECT_LLONG(strtoll("-1", NULL, 0), -1LL);
    EXPECT_LLONG(strtoll("123alpha", &endptr, 0), 123LL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LLONG(strtoll("12b3", &endptr, 0), 12LL);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_LLONG(strtoll("alpha123", &endptr, 0), 0LL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LLONG(strtoll("9223372036854775807", NULL, 0), LLONG_MAX);
    EXPECT_LLONG(strtoll("9223372036854775806", NULL, 0), 9223372036854775806LL);
    EXPECT_LLONG(strtoll("-9223372036854775808", NULL, 0), LLONG_MIN);
    EXPECT_LLONG(strtoll("-9223372036854775807", NULL, 0), -9223372036854775807LL);
    EXPECT_LLONG(strtoll("0x123alpha", &endptr, 0), 4666LL);
    EXPECT_CHAR(*endptr, 'l');
    EXPECT_LLONG(strtoll("01238lpha", &endptr, 0), 83LL);
    EXPECT_CHAR(*endptr, '8');
    // Custom base
    EXPECT_LLONG(strtoll("  zzz", NULL, 36), 46655LL);
    EXPECT_LLONG(strtoll("  zzz]", &endptr, 36), 46655LL);
    EXPECT_CHAR(*endptr, ']');
    EXPECT_LLONG(strtoll("  hh", NULL, 30), 527LL);
    EXPECT_LLONG(strtoll("  hhz", &endptr, 30), 527LL);
    EXPECT_CHAR(*endptr, 'z');
    // Invalid base
    EXPECT_LLONG(strtoll("0", NULL, 1), 0LL);
    EXPECT_LLONG(strtoll("1", NULL, 1), 0LL);
    EXPECT_LLONG(strtoll("123", NULL, 42), 0LL);
    EXPECT_LLONG(strtoll("zzz", NULL, 42), 0LL);
    EXPECT_LLONG(strtoll("0123", NULL, -2), 0LL);
}

static void check_strtoul(void)
{
    char *endptr;
    EXPECT_ULONG(strtoul("  85", NULL, 10), 85UL);
    EXPECT_ULONG(strtoul("  85", &endptr, 10), 85UL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULONG(strtoul("256", NULL, 10), 256UL);
    EXPECT_ULONG(strtoul("+23", NULL, 10), 23UL);
    EXPECT_ULONG(strtoul(" -375", NULL, 10), -375UL);
    EXPECT_ULONG(strtoul("0", NULL, 10), 0UL);
    EXPECT_ULONG(strtoul("+0", NULL, 10), 0UL);
    EXPECT_ULONG(strtoul("-0", NULL, 10), 0UL);
    EXPECT_ULONG(strtoul("-1", &endptr, 10), -1UL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULONG(strtoul("123alpha", &endptr, 10), 123UL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_ULONG(strtoul("12b3", &endptr, 10), 12UL);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_ULONG(strtoul("alpha123", &endptr, 10), 0UL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_ULONG(strtoul("18446744073709551615", NULL, 10), ULONG_MAX);
    EXPECT_ULONG(strtoul("18446744073709551614", NULL, 10), 18446744073709551614UL);
    // Octal numbers
    EXPECT_ULONG(strtoul("  125", NULL, 8), 85UL);
    EXPECT_ULONG(strtoul("  125", &endptr, 8), 85UL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULONG(strtoul("1238", &endptr, 8), 83UL);
    EXPECT_CHAR(*endptr, '8');
    EXPECT_ULONG(strtoul("1777777777777777777777", NULL, 8), ULONG_MAX);
    EXPECT_ULONG(strtoul("1777777777777777777776", NULL, 8), 18446744073709551614UL);
    EXPECT_ULONG(strtoul("01777777777777777777777", NULL, 8), ULONG_MAX);
    EXPECT_ULONG(strtoul("073", NULL, 8), 59UL);
    // Hexadecimal numbers
    EXPECT_ULONG(strtoul("  55", NULL, 16), 85UL);
    EXPECT_ULONG(strtoul("  55", &endptr, 16), 85UL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULONG(strtoul("123g", &endptr, 16), 291UL);
    EXPECT_CHAR(*endptr, 'g');
    EXPECT_ULONG(strtoul("ffffffffffffffff", NULL, 16), ULONG_MAX);
    EXPECT_ULONG(strtoul("fffffffffffffffe", NULL, 16), 18446744073709551614UL);
    EXPECT_ULONG(strtoul("0x7f0", NULL, 16), 2032);
    EXPECT_ULONG(strtoul("0xffffffffffffffff", NULL, 16), ULONG_MAX);
    // Informing zero base
    EXPECT_ULONG(strtoul("  85", NULL, 0), 85UL);
    EXPECT_ULONG(strtoul("  85", &endptr, 0), 85UL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULONG(strtoul("256", NULL, 0), 256UL);
    EXPECT_ULONG(strtoul("+23", NULL, 0), 23UL);
    EXPECT_ULONG(strtoul(" -375", NULL, 0), -375UL);
    EXPECT_ULONG(strtoul("0", NULL, 0), 0UL);
    EXPECT_ULONG(strtoul("+0", NULL, 0), 0UL);
    EXPECT_ULONG(strtoul("-0", NULL, 0), 0UL);
    EXPECT_ULONG(strtoul("-1", NULL, 0), -1UL);
    EXPECT_ULONG(strtoul("123alpha", &endptr, 0), 123UL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_ULONG(strtoul("12b3", &endptr, 0), 12UL);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_ULONG(strtoul("alpha123", &endptr, 0), 0UL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_ULONG(strtoul("18446744073709551615", NULL, 0), ULONG_MAX);
    EXPECT_ULONG(strtoul("18446744073709551614", NULL, 0), 18446744073709551614UL);
    EXPECT_ULONG(strtoul("0x123alpha", &endptr, 0), 4666UL);
    EXPECT_CHAR(*endptr, 'l');
    EXPECT_ULONG(strtoul("01238lpha", &endptr, 0), 83UL);
    EXPECT_CHAR(*endptr, '8');
    // Custom base
    EXPECT_ULONG(strtoul("  zzz", NULL, 36), 46655UL);
    EXPECT_ULONG(strtoul("  zzz]", &endptr, 36), 46655UL);
    EXPECT_CHAR(*endptr, ']');
    EXPECT_ULONG(strtoul("  hh", NULL, 30), 527UL);
    EXPECT_ULONG(strtoul("  hhz", &endptr, 30), 527UL);
    EXPECT_CHAR(*endptr, 'z');
    // Invalid base
    EXPECT_ULONG(strtoul("0", NULL, 1), 0UL);
    EXPECT_ULONG(strtoul("1", NULL, 1), 0UL);
    EXPECT_ULONG(strtoul("123", NULL, 42), 0UL);
    EXPECT_ULONG(strtoul("zzz", NULL, 42), 0UL);
    EXPECT_ULONG(strtoul("0123", NULL, -2), 0UL);
}

static void check_strtoull(void)
{
    char *endptr;
    EXPECT_ULLONG(strtoull("  85", NULL, 10), 85ULL);
    EXPECT_ULLONG(strtoull("  85", &endptr, 10), 85ULL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULLONG(strtoull("256", NULL, 10), 256ULL);
    EXPECT_ULLONG(strtoull("+23", NULL, 10), 23ULL);
    EXPECT_ULLONG(strtoull(" -375", NULL, 10), -375ULL);
    EXPECT_ULLONG(strtoull("0", NULL, 10), 0ULL);
    EXPECT_ULLONG(strtoull("+0", NULL, 10), 0ULL);
    EXPECT_ULLONG(strtoull("-0", NULL, 10), 0ULL);
    EXPECT_ULLONG(strtoull("-1", &endptr, 10), -1ULL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULLONG(strtoull("123alpha", &endptr, 10), 123ULL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_ULLONG(strtoull("12b3", &endptr, 10), 12ULL);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_ULLONG(strtoull("alpha123", &endptr, 10), 0ULL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_ULLONG(strtoull("18446744073709551615", NULL, 10), ULLONG_MAX);
    EXPECT_ULLONG(strtoull("18446744073709551614", NULL, 10), 18446744073709551614ULL);
    // Octal numbers
    EXPECT_ULLONG(strtoull("  125", NULL, 8), 85ULL);
    EXPECT_ULLONG(strtoull("  125", &endptr, 8), 85ULL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULLONG(strtoull("1238", &endptr, 8), 83ULL);
    EXPECT_CHAR(*endptr, '8');
    EXPECT_ULLONG(strtoull("1777777777777777777777", NULL, 8), ULLONG_MAX);
    EXPECT_ULLONG(strtoull("1777777777777777777776", NULL, 8), 18446744073709551614ULL);
    EXPECT_ULLONG(strtoull("01777777777777777777777", NULL, 8), ULLONG_MAX);
    EXPECT_ULLONG(strtoull("073", NULL, 8), 59ULL);
    // Hexadecimal numbers
    EXPECT_ULLONG(strtoull("  55", NULL, 16), 85ULL);
    EXPECT_ULLONG(strtoull("  55", &endptr, 16), 85ULL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULLONG(strtoull("123g", &endptr, 16), 291ULL);
    EXPECT_CHAR(*endptr, 'g');
    EXPECT_ULLONG(strtoull("ffffffffffffffff", NULL, 16), ULLONG_MAX);
    EXPECT_ULLONG(strtoull("fffffffffffffffe", NULL, 16), 18446744073709551614ULL);
    EXPECT_ULLONG(strtoull("0x7f0", NULL, 16), 2032);
    EXPECT_ULLONG(strtoull("0xffffffffffffffff", NULL, 16), ULLONG_MAX);
    // Informing zero base
    EXPECT_ULLONG(strtoull("  85", NULL, 0), 85ULL);
    EXPECT_ULLONG(strtoull("  85", &endptr, 0), 85ULL);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_ULLONG(strtoull("256", NULL, 0), 256ULL);
    EXPECT_ULLONG(strtoull("+23", NULL, 0), 23ULL);
    EXPECT_ULLONG(strtoull(" -375", NULL, 0), -375ULL);
    EXPECT_ULLONG(strtoull("0", NULL, 0), 0ULL);
    EXPECT_ULLONG(strtoull("+0", NULL, 0), 0ULL);
    EXPECT_ULLONG(strtoull("-0", NULL, 0), 0ULL);
    EXPECT_ULLONG(strtoull("-1", NULL, 0), -1ULL);
    EXPECT_ULLONG(strtoull("123alpha", &endptr, 0), 123ULL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_ULLONG(strtoull("12b3", &endptr, 0), 12ULL);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_ULLONG(strtoull("alpha123", &endptr, 0), 0ULL);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_ULLONG(strtoull("18446744073709551615", NULL, 0), ULLONG_MAX);
    EXPECT_ULLONG(strtoull("18446744073709551614", NULL, 0), 18446744073709551614ULL);
    EXPECT_ULLONG(strtoull("0x123alpha", &endptr, 0), 4666ULL);
    EXPECT_CHAR(*endptr, 'l');
    EXPECT_ULLONG(strtoull("01238lpha", &endptr, 0), 83ULL);
    EXPECT_CHAR(*endptr, '8');
    // Custom base
    EXPECT_ULLONG(strtoull("  zzz", NULL, 36), 46655ULL);
    EXPECT_ULLONG(strtoull("  zzz]", &endptr, 36), 46655ULL);
    EXPECT_CHAR(*endptr, ']');
    EXPECT_ULLONG(strtoull("  hh", NULL, 30), 527ULL);
    EXPECT_ULLONG(strtoull("  hhz", &endptr, 30), 527ULL);
    EXPECT_CHAR(*endptr, 'z');
    // Invalid base
    EXPECT_ULLONG(strtoull("0", NULL, 1), 0ULL);
    EXPECT_ULLONG(strtoull("1", NULL, 1), 0ULL);
    EXPECT_ULLONG(strtoull("123", NULL, 42), 0ULL);
    EXPECT_ULLONG(strtoull("zzz", NULL, 42), 0ULL);
    EXPECT_ULLONG(strtoull("0123", NULL, -2), 0ULL);
}

static void check_atof(void)
{
    EXPECT_DOUBLE(atof("  85.3"), 85.3);
    EXPECT_DOUBLE(atof("256.23"), 256.23);
    EXPECT_DOUBLE(atof("0.001"), 0.001);
    EXPECT_DOUBLE(atof("+23"), 23.0);
    EXPECT_DOUBLE(atof(" -375"), -375.0);
    EXPECT_DOUBLE(atof("0"), 0.0);
    EXPECT_DOUBLE(atof("+0"), 0.0);
    EXPECT_DOUBLE(atof("-0"), 0.0);
    EXPECT_DOUBLE(atof("-1"), -1.0);
    EXPECT_DOUBLE(atof("123alpha"), 123.0);
    EXPECT_DOUBLE(atof("12.523b3"), 12.523);
    EXPECT_DOUBLE(atof("alpha123"), 0.0);
    EXPECT_DOUBLE(atof("123e5"), 123e5);
    EXPECT_DOUBLE(atof("123e+2"), 123e+2);
    EXPECT_DOUBLE(atof("123e-2"), 123e-2);
    EXPECT_DOUBLE(atof("123E5"), 123e5);
    EXPECT_DOUBLE(atof("123E+2"), 123e+2);
    EXPECT_DOUBLE(atof("123E-2"), 123e-2);
    EXPECT_DOUBLE(atof("1e-3"), 1e-3);
    EXPECT_DOUBLE(atof("123e"), 123.0);
}

static void check_strtof(void)
{
    char *endptr;
    float precision = 1e-6f;
    EXPECT_FLOAT(strtof("  85.3", NULL), 85.3f);
    EXPECT_FLOAT(strtof("  85.3", &endptr), 85.3f);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_FLOAT(strtof("256.23", NULL), 256.23f);
    EXPECT_FLOAT_PREC(strtof("0.001", NULL), 0.001f, precision);
    EXPECT_FLOAT(strtof("+23", NULL), 23.0f);
    EXPECT_FLOAT(strtof(" -375", NULL), -375.0f);
    EXPECT_FLOAT(strtof("0", NULL), 0.0f);
    EXPECT_FLOAT(strtof("+0", NULL), 0.0f);
    EXPECT_FLOAT(strtof("-0", NULL), 0.0f);
    EXPECT_FLOAT(strtof("-1", NULL), -1.0f);
    EXPECT_FLOAT(strtof("123alpha", &endptr), 123.0f);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_FLOAT_PREC(strtof("12.523b3", &endptr), 12.523f, precision);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_FLOAT(strtof("alpha123", &endptr), 0.0f);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_FLOAT(strtof("123e5", NULL), 123e5f);
    EXPECT_FLOAT(strtof("123e+2", NULL), 123e+2f);
    EXPECT_FLOAT(strtof("123e-2", NULL), 123e-2f);
    EXPECT_FLOAT(strtof("123E5", NULL), 123e5f);
    EXPECT_FLOAT(strtof("123E+2", NULL), 123e+2f);
    EXPECT_FLOAT(strtof("123E-2", NULL), 123e-2f);
    EXPECT_FLOAT_PREC(strtof("1e-3", NULL), 1e-3f, precision);
    EXPECT_FLOAT(strtof("123e", &endptr), 123.0f);
    EXPECT_CHAR(*endptr, 'e');
}

static void check_strtod(void)
{
    char *endptr;
    EXPECT_DOUBLE(strtod("  85.3", NULL), 85.3);
    EXPECT_DOUBLE(strtod("  85.3", &endptr), 85.3);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_DOUBLE(strtod("256.23", NULL), 256.23);
    EXPECT_DOUBLE(strtod("0.001", NULL), 0.001);
    EXPECT_DOUBLE(strtod("+23", NULL), 23.0);
    EXPECT_DOUBLE(strtod(" -375", NULL), -375.0);
    EXPECT_DOUBLE(strtod("0", NULL), 0.0);
    EXPECT_DOUBLE(strtod("+0", NULL), 0.0);
    EXPECT_DOUBLE(strtod("-0", NULL), 0.0);
    EXPECT_DOUBLE(strtod("-1", NULL), -1.0);
    EXPECT_DOUBLE(strtod("123alpha", &endptr), 123.0);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_DOUBLE(strtod("12.523b3", &endptr), 12.523);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_DOUBLE(strtod("alpha123", &endptr), 0.0);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_DOUBLE(strtod("123e5", NULL), 123e5);
    EXPECT_DOUBLE(strtod("123e+2", NULL), 123e+2);
    EXPECT_DOUBLE(strtod("123e-2", NULL), 123e-2);
    EXPECT_DOUBLE(strtod("123E5", NULL), 123e5);
    EXPECT_DOUBLE(strtod("123E+2", NULL), 123e+2);
    EXPECT_DOUBLE(strtod("123E-2", NULL), 123e-2);
    EXPECT_DOUBLE(strtod("1e-3", NULL), 1e-3);
    EXPECT_DOUBLE(strtod("123e", &endptr), 123.0);
    EXPECT_CHAR(*endptr, 'e');
}

static void check_strtold(void)
{
    long double precision = 1e-18L;
    char *endptr;
    EXPECT_LDOUBLE(strtold("  85.3", NULL), 85.3L);
    EXPECT_LDOUBLE(strtold("  85.3", &endptr), 85.3L);
    EXPECT_CHAR(*endptr, '\0');
    EXPECT_LDOUBLE(strtold("256.23", NULL), 256.23L);
    EXPECT_LDOUBLE(strtold("0.001", NULL), 0.001L);
    EXPECT_LDOUBLE(strtold("+23", NULL), 23.0L);
    EXPECT_LDOUBLE(strtold(" -375", NULL), -375.0L);
    EXPECT_LDOUBLE(strtold("0", NULL), 0.0L);
    EXPECT_LDOUBLE(strtold("+0", NULL), 0.0L);
    EXPECT_LDOUBLE(strtold("-0", NULL), 0.0L);
    EXPECT_LDOUBLE(strtold("-1", NULL), -1.0L);
    EXPECT_LDOUBLE(strtold("123alpha", &endptr), 123.0L);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LDOUBLE_PREC(strtold("12.523b3", &endptr), 12.523L, precision);
    EXPECT_CHAR(*endptr, 'b');
    EXPECT_LDOUBLE(strtold("alpha123", &endptr), 0.0L);
    EXPECT_CHAR(*endptr, 'a');
    EXPECT_LDOUBLE(strtold("123e5", NULL), 123e5L);
    EXPECT_LDOUBLE(strtold("123e+2", NULL), 123e+2L);
    EXPECT_LDOUBLE(strtold("123e-2", NULL), 123e-2L);
    EXPECT_LDOUBLE(strtold("123E5", NULL), 123e5L);
    EXPECT_LDOUBLE(strtold("123E+2", NULL), 123e+2L);
    EXPECT_LDOUBLE(strtold("123E-2", NULL), 123e-2L);
    EXPECT_LDOUBLE(strtold("1e-3", NULL), 1e-3L);
    EXPECT_LDOUBLE(strtold("123e", &endptr), 123L);
    EXPECT_CHAR(*endptr, 'e');
}

static void check_stdlib(void)
{
    check_atoi();
    check_atol();
    check_atoll();
    check_strtol();
    check_strtoll();
    check_strtoul();
    check_strtoull();
    check_atof();
    check_strtof();
    check_strtod();
    check_strtold();
}

//------------------------------------------------------------------------------
// STDIO.H
//------------------------------------------------------------------------------

#define BUFFER_SIZE 1024

#define TEST_SNPRINTF(expected, ...) \
    do { \
        char buffer[BUFFER_SIZE]; \
        const int ret = snprintf(buffer, sizeof(buffer), __VA_ARGS__); \
        EXPECT_STR(buffer, expected); \
        EXPECT_INT(ret, strlen(expected)); \
    } while (0)

static void check_snprintf(void)
{
    TEST_SNPRINTF("Hello World!", "Hello World!");
    {
        char buffer[10];
        const int ret = snprintf(buffer, sizeof(buffer), NULL);
        EXPECT_INT(ret, -1);
    }
    {
        const char fmt[] = "Hello World!";
        const char expected[] = "Hello Wor";
        char buffer[10];
        const int ret = snprintf(buffer, sizeof(buffer), fmt);
        EXPECT_STR(buffer, expected);
        EXPECT_INT(ret, (int)strlen(fmt));
    }
    // Signed decimal integer
    TEST_SNPRINTF("10 -10 -2147483648 2147483647", "%d %d %d %d", 10, -10, INT_MIN, INT_MAX);
    TEST_SNPRINTF("Out of bounds test: -2147483648 -1", "Out of bounds test: %d %d", (1L + INT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3   +4  0016 0025", "Testing flags: %4d % 3d %04d %-3d %+2d %5.4d %.4d", 10, 1, 2, 3, 4, 16, 25);
    TEST_SNPRINTF("Testing flags: 0010 +01 -01 2   0055  +0", "Testing flags: %04d %+03d %03d %-03d %-05.4d %+d", 10, 1, -1, 2, 55, 0);
    TEST_SNPRINTF("Variable length:    1 2    0345   003  004", "Variable length: %*d %-*d %.*d %5.*d %*.*d", 4, 1, 4, 2, 4, 345, 3, 3, 4, 3, 4);
    // Unsigned decimal integer
    TEST_SNPRINTF("128 0 4294967295", "%u %u %u", 128, 0, UINT_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 4294967295", "Out of bounds test: %u %u", (1UL + UINT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3    4  0016 0025", "Testing flags: %4u % 3u %04u %-3u %+2u %5.4u %.4u", 10, 1, 2, 3, 4, 16, 25);
    TEST_SNPRINTF("Testing flags: 0010 001 2   0055  0", "Testing flags: %04u %+03u %-03u %-05.4u %+u", 10, 1, 2, 55, 0);
    TEST_SNPRINTF("Variable length:    1 2    0345   003  004", "Variable length: %*u %-*u %.*u %5.*u %*.*u", 4, 1, 4, 2, 4, 345, 3, 3, 4, 3, 4);
    // Signed char
    TEST_SNPRINTF("23 -82 -128 127", "%hhd %hhd %hhd %hhd", (char)23, (char)-82, CHAR_MIN, CHAR_MAX);
    TEST_SNPRINTF("Out of bounds test: -128 -1", "Out of bounds test: %hhd %hhd", (1L + CHAR_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3   +4  0016 0025", "Testing flags: %4hhd % 3hhd %04hhd %-3hhd %+2hhd %5.4hhd %.4hhd", 10, 1, 2, 3, 4, 16, 25);
    TEST_SNPRINTF("Testing flags: 0010 +01 -01 2   0055  +0", "Testing flags: %04hhd %+03hhd %03hhd %-03hhd %-05.4hhd %+hhd", 10, 1, -1, 2, 55, 0);
    TEST_SNPRINTF("Variable length:    1 2    0123   003  004", "Variable length: %*hhd %-*hhd %.*hhd %5.*hhd %*.*hhd", 4, 1, 4, 2, 4, 123, 3, 3, 4, 3, 4);
    // Unsigned char
    TEST_SNPRINTF("233 0 255", "%hhu %hhu %hhu", (unsigned char)233, (unsigned char)0, UCHAR_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 255", "Out of bounds test: %hhu %hhu", (1UL + UCHAR_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3    4  0016 0025", "Testing flags: %4hhu % 3hhu %04hhu %-3hhu %+2hhu %5.4hhu %.4hhu", 10, 1, 2, 3, 4, 16, 25);
    TEST_SNPRINTF("Testing flags: 0010 001 2   0055  0", "Testing flags: %04hhu %+03hhu %-03hhu %-05.4hhu %+hhu", 10, 1, 2, 55, 0);
    TEST_SNPRINTF("Variable length:    1 2    0123   003  004", "Variable length: %*hhu %-*hhu %.*hhu %5.*hhu %*.*hhu", 4, 1, 4, 2, 4, 123, 3, 3, 4, 3, 4);
    // Signed short integer
    TEST_SNPRINTF("15 -82 -32768 32767", "%hd %hd %hd %hd", (short)15, (short)-82, SHRT_MIN, SHRT_MAX);
    TEST_SNPRINTF("Out of bounds test: -32768 -1", "Out of bounds test: %hd %hd", (1L + SHRT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3   +4  0016 0025", "Testing flags: %4hd % 3hd %04hd %-3hd %+2hd %5.4hd %.4hd", 10, 1, 2, 3, 4, 16, 25);
    TEST_SNPRINTF("Testing flags: 0010 +01 -01 2   0055  +0", "Testing flags: %04hd %+03hd %03hd %-03hd %-05.4hd %+hd", 10, 1, -1, 2, 55, 0);
    TEST_SNPRINTF("Variable length:    1 2    0345   003  004", "Variable length: %*hd %-*hd %.*hd %5.*hd %*.*hd", 4, 1, 4, 2, 4, 345, 3, 3, 4, 3, 4);
    // Unsigned short integer
    TEST_SNPRINTF("128 0 65535", "%hu %hu %hu", (unsigned short)128, (unsigned short)0, USHRT_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 65535", "Out of bounds test: %hu %hu", (1UL + USHRT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3    4  0016 0025", "Testing flags: %4hu % 3hu %04hu %-3hu %+2hu %5.4hu %.4hu", 10, 1, 2, 3, 4, 16, 25);
    TEST_SNPRINTF("Testing flags: 0010 001 2   0055  0", "Testing flags: %04hu %+03hu %-03hu %-05.4hu %+hu", 10, 1, 2, 55, 0);
    TEST_SNPRINTF("Variable length:    1 2    0345   003  004", "Variable length: %*hu %-*hu %.*hu %5.*hu %*.*hu", 4, 1, 4, 2, 4, 345, 3, 3, 4, 3, 4);
    // Signed long integer
    TEST_SNPRINTF("100 -100 -9223372036854775808 9223372036854775807", "%ld %ld %ld %ld", 100L, -100L, LONG_MIN, LONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3   +4  0016 0025", "Testing flags: %4ld % 3ld %04ld %-3ld %+2ld %5.4ld %.4ld", 10L, 1L, 2L, 3L, 4L, 16L, 25L);
    TEST_SNPRINTF("Testing flags: 0010 +01 -01 2   0055  +0", "Testing flags: %04ld %+03ld %03ld %-03ld %-05.4ld %+ld", 10L, 1L, -1L, 2L, 55L, 0L);
    TEST_SNPRINTF("Variable length:    1 2    0345   003  004", "Variable length: %*ld %-*ld %.*ld %5.*ld %*.*ld", 4, 1L, 4, 2L, 4, 345L, 3, 3L, 4, 3, 4L);
    // Unsigned long integer
    TEST_SNPRINTF("128 0 18446744073709551615", "%lu %lu %lu", 128UL, 0UL, ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3    4  0016 0025", "Testing flags: %4lu % 3lu %04lu %-3lu %+2lu %5.4lu %.4lu", 10UL, 1UL, 2UL, 3UL, 4UL, 16UL, 25UL);
    TEST_SNPRINTF("Testing flags: 0010 001 2   0055  0", "Testing flags: %04lu %+03lu %-03lu %-05.4lu %+lu", 10UL, 1UL, 2UL, 55UL, 0UL);
    TEST_SNPRINTF("Variable length:    1 2    0345   003  004", "Variable length: %*lu %-*lu %.*lu %5.*lu %*.*lu", 4, 1UL, 4, 2UL, 4, 345UL, 3, 3UL, 4, 3, 4UL);
    // Character
    TEST_SNPRINTF("Characters: A c 7", "Characters: %c %c %c", 65, 'c', '7');
    // Percent character
    TEST_SNPRINTF("Percent character: %", "Percent character: %%");
    // Unsigned octal
    TEST_SNPRINTF("200 0 37777777777", "%o %o %o", 128, 0, UINT_MAX);
    TEST_SNPRINTF("Testing length modifiers: 1777777777777777777777 177777 377", "Testing length modifiers: %lo %ho %hho", ULONG_MAX, ULONG_MAX, ULONG_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 37777777777", "Out of bounds test: %o %o", (1UL + UINT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   12  13 0014 15  16  0017 020", "Testing flags: %4o % 3o %04o %-3o %+2o %5.4o %#o", 10, 11, 12, 13, 14, 15, 16);
    TEST_SNPRINTF("Testing flags: 0021 22   0023 0024 0050     0120 0  0 00", "Testing flags: %+04o %-04o %#04o %+#04o %#-8.4o %+#o %#o %#2o %#02o", 17, 18, 19, 20, 40, 80, 0, 0, 0);
    TEST_SNPRINTF("Variable length:    1 2    0531   003  004", "Variable length: %*o %-*o %.*o %5.*o %*.*o", 4, 1, 4, 2, 4, 345, 3, 3, 4, 3, 4);
    // Unsigned hexadecimal
    TEST_SNPRINTF("0xfa 0 ffffffff 4E 0XFFFFFFFF", "%#x %x %x %X %#X", 250, 0, UINT_MAX, 78, UINT_MAX);
    TEST_SNPRINTF("Testing length modifiers: ffffffffffffffff ffff ff", "Testing length modifiers: %lx %hx %hhx", ULONG_MAX, ULONG_MAX, ULONG_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 ffffffff", "Out of bounds test: %x %x", (1UL + UINT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:    a   b 000c d    e  000f 0x10", "Testing flags: %4x % 3x %04x %-3x %+2x %5.4x %#x", 10, 11, 12, 13, 14, 15, 16);
    TEST_SNPRINTF("Testing flags:    A   B 000C D    E  000F 0X10", "Testing flags: %4X % 3X %04X %-3X %+2X %5.4X %#X", 10, 11, 12, 13, 14, 15, 16);
    TEST_SNPRINTF("Testing flags: 0011 12   0x13 0x14 0x0028   0x50 0  0 00 0xaa1 0x0a", "Testing flags: %+04x %-04x %#04x %+#04x %#-8.4x %+#x %#x %#2x %#02x %#04x %#04x", 17, 18, 19, 20, 40, 80, 0, 0, 0, 0xaa1, 0xa);
    TEST_SNPRINTF("Testing flags: 0011 12   0X13 0X14 0X0028   0X50 0  0 00 0XAA1 0X0A", "Testing flags: %+04X %-04X %#04X %+#04X %#-8.4X %+#X %#X %#2X %#02X %#04X %#04X", 17, 18, 19, 20, 40, 80, 0, 0, 0, 0xaa1, 0xa);
    TEST_SNPRINTF("Variable length:    1 2    0159   003  004", "Variable length: %*x %-*x %.*x %5.*x %*.*x", 4, 1, 4, 2, 4, 345, 3, 3, 4, 3, 4);
    // Pointer address
    TEST_SNPRINTF("Pointer addresses: 0x456789ab 0x6789ab (nil)", "Pointer addresses: %p %p %p", (void*)0x456789AB, (void*)0x006789AB, NULL);
    TEST_SNPRINTF("Testing flags:    (nil)    (nil)    (nil) (nil)       (nil) (nil)", "Testing flags: %8p % 8p %08p %-8p %+8p %2.6p", NULL, NULL, NULL, NULL, NULL, NULL);
    TEST_SNPRINTF("Testing flags:      0xa      0xb 0x00000c 0xd          +0xe 0x00000f", "Testing flags: %8p % 8p %08p %-8p %+8p %2.6p", (void*)10, (void*)11, (void*)12, (void*)13, (void*)14, (void*)15);
    TEST_SNPRINTF("Testing flags: +0x0001a 0x1b     0x00001c +0x0001d 0x001e   ", "Testing flags: %+08p %-08p %#08p %+08p %-8.4p ", (void*)26, (void*)27, (void*)28, (void*)29, (void*)30);
    TEST_SNPRINTF("Variable length:  0x1 0x2  0x0159 0x003  0x004", "Variable length: %*p %-*p %.*p %5.*p %*.*p", 4, 1, 4, 2, 4, 345, 3, 3, 6, 3, 4);
    // Decimal floating point
    TEST_SNPRINTF("392.567810 0.001000 0.10 0.001000", "%f %f %.2f %F", 392.5678f, 1e-3f, 0.1f, 1e-3f);
    TEST_SNPRINTF("Testing flags: 10.000000  1.000000 2.000000 3.000000 +4.000000 16.0000 25.0000", "Testing flags: %4f % 3f %04f %-3f %+2f %5.4f %.4f", 10.0f, 1.0f, 2.0f, 3.0f, 4.0f, 16.0f, 25.0f);
    TEST_SNPRINTF("Testing flags: 10.000000 +1.000000 -1.000000 2.000000 55.0000 +0.000000", "Testing flags: %04f %+03f %03f %-03f %-05.4f %+f", 10.0f, 1.0f, -1.0f, 2.0f, 55.0f, 0.0f);
    TEST_SNPRINTF("Fixed length:  1.000000 2.000000  345.00  3.00  4.000", "Fixed length: %9f %-9f %.2f %5.2f %6.3f", 1.0f, 2.0f, 345.0f, 3.0f, 4.0f);
    TEST_SNPRINTF("Variable length:  1.000000 2.000000  345.00  3.00  4.000", "Variable length: %*f %-*f %.*f %5.*f %*.*f", 9, 1.0f, 9, 2.0f, 2, 345.0f, 2, 3.0f, 6, 3, 4.0f);
    // Floating point in scientific notation (mantissa/exponent)
    TEST_SNPRINTF("3.925678e+02 1.000000e-03 1.00e-01 1.000000E-03", "%e %e %.2e %E", 392.5678, 1e-3, 0.1, 1e-3);
    TEST_SNPRINTF("Testing flags: 1.000000e+01  1.000000e+00 2.000000e+00 3.000000e+00 +4.000000e+00 1.6000e+01 2.5000e+01", "Testing flags: %4e % 3e %04e %-3e %+2e %5.4e %.4e", 10.0f, 1.0f, 2.0f, 3.0f, 4.0f, 16.0f, 25.0f);
    TEST_SNPRINTF("Testing flags: 1.000000e+01 +1.000000e+00 -1.000000e+00 2.000000e+00 5.5000e+01 +0.000000e+00", "Testing flags: %04e %+03e %03e %-03e %-05.4e %+e", 10.0f, 1.0f, -1.0f, 2.0f, 55.0f, 0.0f);
    TEST_SNPRINTF("Testing flags: 1.000000E+01  1.000000E+00 2.000000E+00 3.000000E+00 +4.000000E+00 1.6000E+01 2.5000E+01", "Testing flags: %4E % 3E %04E %-3E %+2E %5.4E %.4E", 10.0f, 1.0f, 2.0f, 3.0f, 4.0f, 16.0f, 25.0f);
    TEST_SNPRINTF("Testing flags: 1.000000E+01 +1.000000E+00 -1.000000E+00 2.000000E+00 5.5000E+01 +0.000000E+00", "Testing flags: %04E %+03E %03E %-03E %-05.4E %+E", 10.0f, 1.0f, -1.0f, 2.0f, 55.0f, 0.0f);
    TEST_SNPRINTF("Testing flags: 1.012340e-05 +1.234560e-06 -1.234560e-06 2.345678e+06 8.8946e+05", "Testing flags: %08e %+08e %08e %-09e %-08.4e", 10.1234e-6f, 1.23456e-6f, -1.23456e-6f, 2.345678e+6f, 889.45678e+3f);
    TEST_SNPRINTF("Testing flags: 1.012340E-05 +1.234560E-06 -1.234560E-06 2.345678E+06 8.8946E+05", "Testing flags: %08E %+08E %08E %-09E %-08.4E", 10.1234e-6f, 1.23456e-6f, -1.23456e-6f, 2.345678e+6f, 889.45678e+3f);
    TEST_SNPRINTF("Fixed length:    1.000000e+00 2.000000e+00    3.45e+02  3.00e+00 4.000e+00", "Fixed length: %15e %-15e %.2e %9.2e %9.3e", 1.0f, 2.0f, 345.0f, 3.0f, 4.0f);
    TEST_SNPRINTF("Variable length:    1.000000e+00 2.000000e+00    3.45e+02  3.00e+00 4.000e+00", "Variable length: %*e %-*e %.*e %9.*e %*.*e", 15, 1.0f, 15, 2.0f, 2, 345.0f, 2, 3.0f, 9, 3, 4.0f);
    // Hexadecimal floating point
    TEST_SNPRINTF("0x1.88915b573eab3p+8 0x1.b7cdfd9d7bdbbp-34 0x1.9ap-4 0X1.B7CDFD9D7BDBBP-34", "%a %a %.2a %A", 392.5678, 1e-10, 0.1, 1e-10);
    TEST_SNPRINTF("Testing flags: 0x1.4p+3  0x1p+0 0x1p+1 0x1.8p+1 +0x1p+2 0x1.0000p+4 0x1.9000p+4", "Testing flags: %4a % 3a %04a %-3a %+2a %5.4a %.4a", 10.0f, 1.0f, 2.0f, 3.0f, 4.0f, 16.0f, 25.0f);
    TEST_SNPRINTF("Testing flags: 0x1.4p+3 +0x1p+0 -0x1p+0 0x1p+1 0x1.b800p+5 +0x0p+0", "Testing flags: %04a %+03a %03a %-03a %-05.4a %+a", 10.0f, 1.0f, -1.0f, 2.0f, 55.0f, 0.0f);
    TEST_SNPRINTF("Testing flags: 0X1.4P+3  0X1P+0 0X1P+1 0X1.8P+1 +0X1P+2 0X1.0000P+4 0X1.9000P+4", "Testing flags: %4A % 3A %04A %-3A %+2A %5.4A %.4A", 10.0f, 1.0f, 2.0f, 3.0f, 4.0f, 16.0f, 25.0f);
    TEST_SNPRINTF("Testing flags: 0X1.4P+3 +0X1P+0 -0X1P+0 0X1P+1 0X1.B800P+5 +0X0P+0", "Testing flags: %04A %+03A %03A %-03A %-05.4A %+A", 10.0f, 1.0f, -1.0f, 2.0f, 55.0f, 0.0f);
    TEST_SNPRINTF("Testing flags: 0x1.53af58p-17 +0x1.4b6652p-20 -0x1.4b6652p-20 0x1.1e567p+21 0x1.b24ep+19", "Testing flags: %08a %+08a %08a %-09a %-08.4a", 10.1234e-6f, 1.23456e-6f, -1.23456e-6f, 2.345678e+6f, 889.45678e+3f);
    TEST_SNPRINTF("Testing flags: 0X1.53AF58P-17 +0X1.4B6652P-20 -0X1.4B6652P-20 0X1.1E567P+21 0X1.B24EP+19", "Testing flags: %08A %+08A %08A %-09A %-08.4A", 10.1234e-6f, 1.23456e-6f, -1.23456e-6f, 2.345678e+6f, 889.45678e+3f);
    // Floating point in the shortest representation
    TEST_SNPRINTF("392.568 1e-10 0.1 1E-10", "%g %g %.2g %G", 392.5678, 1e-10, 0.1, 1e-10);
    TEST_SNPRINTF("0.1 0.001 0.00123457 0.000123457 1.23457e-05", "%g %g %g %g %g", 0.1, 1e-3, 1.234567e-3, 1.234567e-4, 1.234567e-5);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3   +4    16 25", "Testing flags: %4g % 3g %04g %-3g %+2g %5.4g %.4g", 10.0f, 1.0f, 2.0f, 3.0f, 4.0f, 16.0f, 25.0f);
    TEST_SNPRINTF("Testing flags: 0010 +01 -01 2   55    +0", "Testing flags: %04g %+03g %03g %-03g %-05.4g %+g", 10.0f, 1.0f, -1.0f, 2.0f, 55.0f, 0.0f);
    TEST_SNPRINTF("Testing flags: 010.1234 +1.23456 -1.23456 2.34568   889.5   ", "Testing flags: %08g %+08g %08g %-09g %-08.4g", 10.1234f, 1.23456f, -1.23456f, 2.345678f, 889.45678f);
    TEST_SNPRINTF("Testing flags: 1.01234e-05 +1.23456e-06 -1.23456e-06 2.34568e+06 8.895e+05", "Testing flags: %08g %+08g %08g %-09g %-08.4g", 10.1234e-6f, 1.23456e-6f, -1.23456e-6f, 2.345678e+6f, 889.45678e+3f);
    TEST_SNPRINTF("Testing flags: 1.01234E-05 +1.23456E-06 -1.23456E-06 2.34568E+06 8.895E+05", "Testing flags: %08G %+08G %08G %-09G %-08.4G", 10.1234e-6f, 1.23456e-6f, -1.23456e-6f, 2.345678e+6f, 889.45678e+3f);
    // String of characters
    TEST_SNPRINTF("Some null string of chars: (null)", "Some null %s of chars: %s", "string", (char *)NULL);
    TEST_SNPRINTF("   foo bar baz test  ", "%6s %.6s %.3s %-6s", "foo", "bar", "bazzing", "test");
    TEST_SNPRINTF("   foo bar      baz baz        test", "%+6s %-03s %08.3s %-08.3s %#6s", "foo", "bar", "bazzing", "bazzing", "test");
    TEST_SNPRINTF("Variable length:    A b    char     C   D", "Variable length: %*s %-*s %.*s %5.*s %*.*s", 4, "A", 4, "b", 4, "character", 3, "C", 3, 3, "D");
    // Nothing printed. The corresponding argument must be a pointer to a signed int.
    // The number of characters written so far is stored in the pointed location
    {
        const char expected[] = "Testing characters written";
        const int expected_size = (int)strlen(expected);
        char buffer[BUFFER_SIZE];
        int begin, middle, end;
        const int ret = snprintf(buffer, sizeof(buffer), "%nTesting %n%s%n", &begin, &middle, "characters written", &end);
        EXPECT_STR(buffer, expected);
        EXPECT_INT(ret, expected_size);
        EXPECT_INT(begin, 0);
        EXPECT_INT(middle, 8);
        EXPECT_INT(end, expected_size);
    }
    // Invalid specifiers
    TEST_SNPRINTF("%k %r %v %y %", "%k %r %v %y %");
}

static void check_sscanf(void)
{
    char char1, char2;
    int int1, int2, int3;
    unsigned int uint1, uint2, uint3;
    float float1, float2, float3;
    char str1[32], str2[32];
    void *ptr1, *ptr2;
    EXPECT_INT(sscanf("", ""), 0);
    EXPECT_INT(sscanf(" ", "  "), 0);
    EXPECT_INT(sscanf("Hello World!", "Hello  \tWorld!"), 0);
    // Signed integer in decimal base
    EXPECT_INT(sscanf("183", "%d", &int1), 1);
    EXPECT_INT(int1, 183);
    EXPECT_INT(sscanf("\t  -253", " %d", &int1), 1);
    EXPECT_INT(int1, -253);
    EXPECT_INT(sscanf("a115", "%d", &int1), 0);
    EXPECT_INT(sscanf("\t  +115", "%d", &int1), 1);
    EXPECT_INT(int1, 115);
    EXPECT_INT(sscanf("\t  15 265", " %d %d %d", &int1, &int2, &int3), 2);
    EXPECT_INT(int1, 15);
    EXPECT_INT(int2, 265);
    EXPECT_INT(sscanf("0xff  0156 69", " %d", &int1), 1);
    EXPECT_INT(int1, 0);
    EXPECT_INT(sscanf("0156 69", " %d", &int1), 1);
    EXPECT_INT(int1, 156);
    EXPECT_INT(sscanf("1,2,3", " %d,%d,%d", &int1, &int2, &int3), 3);
    EXPECT_INT(int1, 1);
    EXPECT_INT(int2, 2);
    EXPECT_INT(int3, 3);
    EXPECT_INT(sscanf("12 ,21, 31", " %d,%d,%d", &int1, &int2, &int3), 1);
    EXPECT_INT(int1, 12);
    EXPECT_INT(sscanf("12,21, 33", " %d,%d,%d", &int1, &int2, &int3), 3);
    EXPECT_INT(int1, 12);
    EXPECT_INT(int2, 21);
    EXPECT_INT(sscanf("1,2,3", " %d,%*d,%d", &int1, &int2), 2);
    EXPECT_INT(int1, 1);
    EXPECT_INT(int2, 3);
    // Signed integer in decimal, hexadecimal or octal base
    EXPECT_INT(sscanf("0xff  0156 69", " %i %i %i", &int1, &int2, &int3), 3);
    EXPECT_INT(int1, 255);
    EXPECT_INT(int2, 110);
    EXPECT_INT(int3, 69);
    EXPECT_INT(sscanf(" 0xfe, -0156, +69", " %i,%i,%i", &int1, &int2, &int3), 3);
    EXPECT_INT(int1, 254);
    EXPECT_INT(int2, -110);
    EXPECT_INT(int3, 69);
    EXPECT_INT(sscanf("1,2,3", " %i,%*i,%i", &int1, &int2), 2);
    EXPECT_INT(int1, 1);
    EXPECT_INT(int2, 3);
    // Unsigned integer
    EXPECT_INT(sscanf("165  +156 96", " %u %u %u", &uint1, &uint2, &uint3), 3);
    EXPECT_UINT(uint1, 165);
    EXPECT_UINT(uint2, 156);
    EXPECT_UINT(uint3, 96);
    EXPECT_INT(sscanf("-156", "%u", &uint1), 1);
    EXPECT_UINT(uint1, (unsigned int)-156);
    EXPECT_INT(sscanf("+6 -36", "%u %u", &uint1, &uint2), 2);
    EXPECT_UINT(uint1, 6);
    EXPECT_UINT(uint2, (unsigned int)-36);
    EXPECT_INT(sscanf("1,2,3", " %u,%*u,%u", &uint1, &uint2), 2);
    EXPECT_UINT(uint1, 1);
    EXPECT_UINT(uint2, 3);
    // Unsigned integer in octal form
    EXPECT_INT(sscanf("0777 777", " %o %o", &uint1, &uint2), 2);
    EXPECT_UINT(uint1, 511);
    EXPECT_UINT(uint2, 511);
    EXPECT_INT(sscanf("-156 +15", "%o %o", &uint1, &uint2), 2);
    EXPECT_UINT(uint1, (unsigned int)-110);
    EXPECT_UINT(uint2, 13);
    EXPECT_INT(sscanf("1,2,3", " %o,%*o,%o", &uint1, &uint2), 2);
    EXPECT_UINT(uint1, 1);
    EXPECT_UINT(uint2, 3);
    // Unsigned integer in hexadecimal form
    EXPECT_INT(sscanf("0xfff fff", " %x %x", &uint1, &uint2), 2);
    EXPECT_UINT(uint1, 4095);
    EXPECT_UINT(uint2, 4095);
    EXPECT_INT(sscanf("-156 +ab", "%x %x", &uint1, &uint2), 2);
    EXPECT_UINT(uint1, (unsigned int)-342);
    EXPECT_UINT(uint2, 171);
    EXPECT_INT(sscanf("1,2,3", " %x,%*x,%x", &uint1, &uint2), 2);
    EXPECT_UINT(uint1, 1);
    EXPECT_UINT(uint2, 3);
    // Character
    EXPECT_INT(sscanf("a", "%c", &char1), 1);
    EXPECT_CHAR(char1, 'a');
    EXPECT_INT(sscanf("  a", "%c", &char1), 1);
    EXPECT_CHAR(char1, ' ');
    EXPECT_INT(sscanf("axb", "a%cb", &char1), 1);
    EXPECT_CHAR(char1, 'x');
    EXPECT_INT(sscanf("alpha", "al%ch%c", &char1, &char2), 2);
    EXPECT_CHAR(char1, 'p');
    EXPECT_CHAR(char2, 'a');
    EXPECT_INT(sscanf("beta", "%c%*ct%c", &char1, &char2), 2);
    EXPECT_CHAR(char1, 'b');
    EXPECT_CHAR(char2, 'a');
    // Percent character
    EXPECT_INT(sscanf("  % c", "%% %c", &char1), 1);
    EXPECT_CHAR(char1, 'c');
    EXPECT_INT(sscanf("  d c", "%% %c", &char1), 0);
    // Floating point
    EXPECT_INT(sscanf(" 1.0", "%f", &float1), 1);
    EXPECT_FLOAT(float1, 1.0);
    EXPECT_INT(sscanf(" 1.01,-2.1,  3.2", "%f,%f,%f", &float1, &float2, &float3), 3);
    EXPECT_FLOAT(float1, 1.01f);
    EXPECT_FLOAT(float2, -2.1f);
    EXPECT_FLOAT(float3, 3.2f);
    EXPECT_INT(sscanf(" 1.1,2.1e3,  -3.2e-1", "%f,%f,%f", &float1, &float2, &float3), 3);
    EXPECT_FLOAT(float1, 1.1f);
    EXPECT_FLOAT(float2, 2.1e3f);
    EXPECT_FLOAT(float3, -3.2e-1f);
    EXPECT_INT(sscanf(" 1.1,2.1e3,  -3.2e-1", "%e,%e,%e", &float1, &float2, &float3), 3);
    EXPECT_FLOAT(float1, 1.1f);
    EXPECT_FLOAT(float2, 2.1e3f);
    EXPECT_FLOAT(float3, -3.2e-1f);
    EXPECT_INT(sscanf(" 1.1,2.1e3,  -3.2e-1", "%g,%g,%g", &float1, &float2, &float3), 3);
    EXPECT_FLOAT(float1, 1.1f);
    EXPECT_FLOAT(float2, 2.1e3f);
    EXPECT_FLOAT(float3, -3.2e-1f);
    EXPECT_INT(sscanf(" 1.1,2.1e3,  -3.2e-1", "%a,%a,%a", &float1, &float2, &float3), 3);
    EXPECT_FLOAT(float1, 1.1f);
    EXPECT_FLOAT(float2, 2.1e3f);
    EXPECT_FLOAT(float3, -3.2e-1f);
    EXPECT_INT(sscanf(" 1.1,2.1e3,  -3.2e-1", "%F,%F,%F", &float1, &float2, &float3), 3);
    EXPECT_FLOAT(float1, 1.1f);
    EXPECT_FLOAT(float2, 2.1e3f);
    EXPECT_FLOAT(float3, -3.2e-1f);
    EXPECT_INT(sscanf(" 1.1,2.1e3,  -3.2e-1", "%E,%E,%E", &float1, &float2, &float3), 3);
    EXPECT_FLOAT(float1, 1.1f);
    EXPECT_FLOAT(float2, 2.1e3f);
    EXPECT_FLOAT(float3, -3.2e-1f);
    EXPECT_INT(sscanf(" 1.1,2.1e3,  -3.2e-1", "%G,%G,%G", &float1, &float2, &float3), 3);
    EXPECT_FLOAT(float1, 1.1f);
    EXPECT_FLOAT(float2, 2.1e3f);
    EXPECT_FLOAT(float3, -3.2e-1f);
    EXPECT_INT(sscanf(" 1.1,2.1e3,  -3.2e-1", "%A,%A,%A", &float1, &float2, &float3), 3);
    EXPECT_FLOAT(float1, 1.1f);
    EXPECT_FLOAT(float2, 2.1e3f);
    EXPECT_FLOAT(float3, -3.2e-1f);
    EXPECT_INT(sscanf("1.1,2.2,3.3", " %f,%*f,%f", &float1, &float2), 2);
    EXPECT_FLOAT(float1, 1.1f);
    EXPECT_FLOAT(float2, 3.3f);
    // String
    EXPECT_INT(sscanf("Hello You!", "%s", str1), 1);
    EXPECT_STR(str1, "Hello");
    EXPECT_INT(sscanf("\t\t   Hello You!", " %s%s", str1, str2), 2);
    EXPECT_STR(str1, "Hello");
    EXPECT_STR(str2, "You!");
    EXPECT_INT(sscanf("Let's go there!", " %s%*s%s", str1, str2), 2);
    EXPECT_STR(str1, "Let's");
    EXPECT_STR(str2, "there!");
    // Pointer
    EXPECT_INT(sscanf("0x8000ffff ffff8000", " %p %p", &ptr1, &ptr2), 2);
    EXPECT_PTR(ptr1, (void *)0x8000ffff);
    EXPECT_PTR(ptr2, (void *)0xffff8000);
    EXPECT_INT(sscanf(" -0x80001000 +10008000", "%p %p", &ptr1, &ptr2), 2);
    EXPECT_PTR(ptr1, (void *)-0x80001000ULL);
    EXPECT_PTR(ptr2, (void *)0x10008000);
    // The number of characters consumed so far
    EXPECT_INT(sscanf("0x8000fffe -52.2e-6 string", "%n %p %f %n %s%n", &int1, &ptr1, &float1, &int2, str1, &int3), 3);
    EXPECT_PTR(ptr1, (void *)0x8000fffe);
    EXPECT_FLOAT(float1, -52.2e-6f);
    EXPECT_STR(str1, "string");
    EXPECT_INT(int1, 0);
    EXPECT_INT(int2, 20);
    EXPECT_INT(int3, 26);
}

static void check_stdio(void)
{
    check_snprintf();
    check_sscanf();
}

//------------------------------------------------------------------------------
// MAIN
//------------------------------------------------------------------------------

int main(void)
{
    check_ctype();
    check_cstring();
    check_stdlib();
    check_stdio();
    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
// END
//------------------------------------------------------------------------------

// MIT License

// Copyright (c) 2023 CLECIO JUNG <clecio.jung@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
