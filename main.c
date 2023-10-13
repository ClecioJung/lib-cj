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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#ifdef USE_LIB_CJ
#include "libcj.h"
#endif

#define BUFFER_SIZE 1024

// TODO: Introduce a function to escape special chars
#define EXPECT_STR(value, to_equal) expect_str(__FILE__, __LINE__, value, to_equal)
void expect_str(const char *const  file, const unsigned int line, const char *const value, const char *const to_equal)
{
    if (strcmp(value, to_equal) != 0) {
        fprintf(stderr, "%s:%u [TEST FAILED] expected\n    %s\n  but got\n    %s\n", file, line, to_equal, value);
        exit(EXIT_FAILURE);
    }
}

#define EXPECT_INT(value, to_equal) expect_int(__FILE__, __LINE__, value, to_equal)
void expect_int(const char *const  file, const unsigned int line, const int value, const int to_equal)
{
    if (value != to_equal) {
        fprintf(stderr, "%s:%u [TEST FAILED] expected\n    %d\n  but got\n    %d\n", file, line, to_equal, value);
        exit(EXIT_FAILURE);
    }
}

#define TEST_SNPRINTF(expected, ...) \
    do { \
        char buffer[BUFFER_SIZE]; \
        const int ret = snprintf(buffer, sizeof(buffer), __VA_ARGS__); \
        EXPECT_STR(buffer, expected); \
        EXPECT_INT(ret, strlen(expected)); \
    } while (0) \

// TODO: We must have tests for multiple flags!
int main(void)
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
        EXPECT_INT(ret, strlen(fmt));
    }
    // Signed decimal integer
    TEST_SNPRINTF("10 -10 -2147483648 2147483647", "%d %d %d %d", 10, -10, INT_MIN, INT_MAX);
    TEST_SNPRINTF("Out of bounds test: -2147483648 -1", "Out of bounds test: %d %d", (1L + INT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3   +4  0016 0025", "Testing flags: %4d % 3d %04d %-3d %+2d %5.4d %.4d", 10, 1, 2, 3, 4, 16, 25);
    // Unsigned decimal integer
    TEST_SNPRINTF("128 0 4294967295", "%u %u %u", 128, 0, UINT_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 4294967295", "Out of bounds test: %u %u", (1L + UINT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3    4  0016 0025", "Testing flags: %4u % 3u %04u %-3u %+2u %5.4u %.4u", 10, 1, 2, 3, 4, 16, 25);
    // Signed char
    TEST_SNPRINTF("23 -82 -128 127", "%hhd %hhd %hhd %hhd", (char)23, (char)-82, CHAR_MIN, CHAR_MAX);
    TEST_SNPRINTF("Out of bounds test: -128 -1", "Out of bounds test: %hhd %hhd", (1L + CHAR_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3   +4  0016 0025", "Testing flags: %4hhd % 3hhd %04hhd %-3hhd %+2hhd %5.4hhd %.4hhd", 10, 1, 2, 3, 4, 16, 25);
    // Unsigned char
    TEST_SNPRINTF("233 0 255", "%hhu %hhu %hhu", (unsigned char)233, (unsigned char)0, UCHAR_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 255", "Out of bounds test: %hhu %hhu", (1L + UCHAR_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3    4  0016 0025", "Testing flags: %4hhu % 3hhu %04hhu %-3hhu %+2hhu %5.4hhu %.4hhu", 10, 1, 2, 3, 4, 16, 25);
    // Signed short integer
    TEST_SNPRINTF("15 -82 -32768 32767", "%hd %hd %hd %hd", (short)15, (short)-82, SHRT_MIN, SHRT_MAX);
    TEST_SNPRINTF("Out of bounds test: -32768 -1", "Out of bounds test: %hd %hd", (1L + SHRT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3   +4  0016 0025", "Testing flags: %4hd % 3hd %04hd %-3hd %+2hd %5.4hd %.4hd", 10, 1, 2, 3, 4, 16, 25);
    // Unsigned short integer
    TEST_SNPRINTF("128 0 65535", "%hu %hu %hu", (unsigned short)128, (unsigned short)0, USHRT_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 65535", "Out of bounds test: %hu %hu", (1L + USHRT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3    4  0016 0025", "Testing flags: %4hu % 3hu %04hu %-3hu %+2hu %5.4hu %.4hu", 10, 1, 2, 3, 4, 16, 25);
    // Signed long integer
    TEST_SNPRINTF("100 -100 -9223372036854775808 9223372036854775807", "%ld %ld %ld %ld", (long)100, (long)-100, LONG_MIN, LONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3   +4  0016 0025", "Testing flags: %4ld % 3ld %04ld %-3ld %+2ld %5.4ld %.4ld", 10, 1, 2, 3, 4, 16, 25);
    // Unsigned long integer
    TEST_SNPRINTF("128 0 18446744073709551615", "%lu %lu %lu", (unsigned long)128, (unsigned long)0, ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   10   1 0002 3    4  0016 0025", "Testing flags: %4lu % 3lu %04lu %-3lu %+2lu %5.4lu %.4lu", 10, 1, 2, 3, 4, 16, 25);
    // Character
    TEST_SNPRINTF("Characters: A c 7", "Characters: %c %c %c", 65, 'c', '7');
    // Percent character
    TEST_SNPRINTF("Percent character: %", "Percent character: %%");
    // Unsigned octal
    TEST_SNPRINTF("200 0 37777777777", "%o %o %o", 128, 0, UINT_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 37777777777", "Out of bounds test: %o %o", (1L + UINT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:   12  13 0014 15  16  0017 020", "Testing flags: %4o % 3o %04o %-3o %+2o %5.4o %#o", 10, 11, 12, 13, 14, 15, 16);
    // Unsigned hexadecimal
    TEST_SNPRINTF("0xfa 0 ffffffff 4E 0XFFFFFFFF", "%#x %x %x %X %#X", 250, 0, UINT_MAX, 78, UINT_MAX);
    TEST_SNPRINTF("Out of bounds test: 0 ffffffff", "Out of bounds test: %x %x", (1L + UINT_MAX), ULONG_MAX);
    TEST_SNPRINTF("Testing flags:    a   b 000c d    e  000f 0x10", "Testing flags: %4x % 3x %04x %-3x %+2x %5.4x %#x", 10, 11, 12, 13, 14, 15, 16);
    TEST_SNPRINTF("Testing flags:    A   B 000C D    E  000F 0X10", "Testing flags: %4X % 3X %04X %-3X %+2X %5.4X %#X", 10, 11, 12, 13, 14, 15, 16);
    // Pointer address
    TEST_SNPRINTF("Pointer addresses: 0x456789ab 0x6789ab (nil)", "Pointer addresses: %p %p %p", (void*)0x456789AB, (void*)0x006789AB, NULL);
    TEST_SNPRINTF("Testing flags:    (nil)    (nil)    (nil) (nil)       (nil) (nil)", "Testing flags: %8p % 8p %08p %-8p %+8p %2.6p", NULL, NULL, NULL, NULL, NULL, NULL);
    TEST_SNPRINTF("Testing flags:      0xa      0xb 0x00000c 0xd          +0xe 0x00000f", "Testing flags: %8p % 8p %08p %-8p %+8p %2.6p", (void*)10, (void*)11, (void*)12, (void*)13, (void*)14, (void*)15);
    // Decimal floating point
    TEST_SNPRINTF("392.567800 0.001000 0.10 0.001000", "%f %f %.2f %F", 392.5678, 1e-3, 0.1, 1e-3);
    // Scientific notation (mantissa/exponent)
    TEST_SNPRINTF("3.925678e+02 1.000000e-03 1.00e-01 1.000000E-03", "%e %e %.2e %E", 392.5678, 1e-3, 0.1, 1e-3);
    // Hexadecimal floating point
    TEST_SNPRINTF("0x1.88915b573eab3p+8 0x1.b7cdfd9d7bdbbp-34 0x1.9ap-4 0X1.B7CDFD9D7BDBBP-34", "%a %a %.2a %A", 392.5678, 1e-10, 0.1, 1e-10);
    // Use the shortest representation:
    // TODO: Add better tests, with negative numbers and double limits
    TEST_SNPRINTF("392.568 1e-10 0.1 1E-10", "%g %g %.2g %G", 392.5678, 1e-10, 0.1, 1e-10);
    TEST_SNPRINTF("0.1 0.001 0.00123457 0.000123457 1.23457e-05", "%g %g %g %g %g", 0.1, 1e-3, 1.234567e-3, 1.234567e-4, 1.234567e-5);
    // String of characters
    TEST_SNPRINTF("Some null string of chars: (null)", "%s %.4s%7s %-3s%.*ss: %s", "Some", "null pointer", "string", "of", 4, "character", (char *)NULL);
    // Nothing printed. The corresponding argument must be a pointer to a signed int.
    // The number of characters written so far is stored in the pointed location
    {
        const char expected[] = "Testing characters written";
        const int expected_size = strlen(expected);
        char buffer[BUFFER_SIZE];
        int begin, middle, end;
        const int ret = snprintf(buffer, sizeof(buffer), "%nTesting %n%s%n", &begin, &middle, "characters written", &end);
        EXPECT_STR(buffer, expected);
        EXPECT_INT(ret, expected_size);
        EXPECT_INT(begin, 0);
        EXPECT_INT(middle, 8);
        EXPECT_INT(end, expected_size);
    }
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
