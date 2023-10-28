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
#endif

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Introduce a function to escape special chars
#define EXPECT_STR(value, to_equal) expect_str(__FILE__, __LINE__, value, to_equal)
void expect_str(const char *const file, const unsigned int line, const char *const value, const char *const to_equal)
{
    if (strcmp(value, to_equal) != 0) {
        fprintf(stderr, "%s:%u [TEST FAILED] expected\n    \"%s\"\n  but got\n    \"%s\"\n", file, line, to_equal, value);
        exit(EXIT_FAILURE);
    }
}

#define EXPECT_INT(value, to_equal) expect_int(__FILE__, __LINE__, value, to_equal)
void expect_int(const char *const file, const unsigned int line, const int value, const int to_equal)
{
    if (value != to_equal) {
        fprintf(stderr, "%s:%u [TEST FAILED] expected\n    %d\n  but got\n    %d\n", file, line, to_equal, value);
        exit(EXIT_FAILURE);
    }
}

void check_strlen(void)
{
    const char *const empty_str = "";
    const char *const hello_world = "Hello World!";
    const char *const test = "Test";
    EXPECT_INT((int)strlen(empty_str), 0);
    EXPECT_INT((int)strlen(hello_world), 12);
    EXPECT_INT((int)strlen(test), 4);
}

#define BUFFER_SIZE 1024

#define TEST_SNPRINTF(expected, ...) \
    do { \
        char buffer[BUFFER_SIZE]; \
        const int ret = snprintf(buffer, sizeof(buffer), __VA_ARGS__); \
        EXPECT_STR(buffer, expected); \
        EXPECT_INT(ret, strlen(expected)); \
    } while (0) \

void check_snprintf(void)
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
}

int main(void)
{
    check_strlen();
    check_snprintf();
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
