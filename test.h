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
// HEADER
//------------------------------------------------------------------------------

#ifndef __TEST_H
#define __TEST_H

// Header only library
// Define the macro TEST_IMPLEMENTATION in one of your C/C++ files to include
// the implementation of the functions here defined
// You may want to include the headers math.h, stdio.h and stdlib.h in order to
// use this library.

#define EXPECT_STR(value, to_equal) expect_str(__FILE__, __LINE__, value, to_equal)
#define EXPECT_SIZED_STR(value, to_equal, size) expect_sized_str(__FILE__, __LINE__, value, to_equal, size)
#define EXPECT_TRUE(value) expect_true(__FILE__, __LINE__, value)
#define EXPECT_FALSE(value) expect_false(__FILE__, __LINE__, value)

#define EXPECT_PTR(value, to_equal) expect_ptr(__FILE__, __LINE__, value, to_equal)
#define EXPECT_CHAR(value, to_equal) expect_char(__FILE__, __LINE__, value, to_equal)
#define EXPECT_SHORT(value, to_equal) expect_short(__FILE__, __LINE__, value, to_equal)
#define EXPECT_INT(value, to_equal) expect_int(__FILE__, __LINE__, value, to_equal)
#define EXPECT_LONG(value, to_equal) expect_long(__FILE__, __LINE__, value, to_equal)
#define EXPECT_LLONG(value, to_equal) expect_llong(__FILE__, __LINE__, value, to_equal)
#define EXPECT_UCHAR(value, to_equal) expect_uchar(__FILE__, __LINE__, value, to_equal)
#define EXPECT_USHORT(value, to_equal) expect_ushort(__FILE__, __LINE__, value, to_equal)
#define EXPECT_UINT(value, to_equal) expect_uint(__FILE__, __LINE__, value, to_equal)
#define EXPECT_ULONG(value, to_equal) expect_ulong(__FILE__, __LINE__, value, to_equal)
#define EXPECT_ULLONG(value, to_equal) expect_ullong(__FILE__, __LINE__, value, to_equal)
#define EXPECT_SIZE(value, to_equal) expect_size(__FILE__, __LINE__, value, to_equal)
#define EXPECT_FLOAT(value, to_equal) expect_float(__FILE__, __LINE__, value, to_equal)
#define EXPECT_DOUBLE(value, to_equal) expect_double(__FILE__, __LINE__, value, to_equal)
#define EXPECT_LDOUBLE(value, to_equal) expect_ldouble(__FILE__, __LINE__, value, to_equal)
#define EXPECT_FLOAT_PREC(value, to_equal, precision) expect_float_prec(__FILE__, __LINE__, value, to_equal, precision)
#define EXPECT_DOUBLE_PREC(value, to_equal, precision) expect_double_prec(__FILE__, __LINE__, value, to_equal, precision)
#define EXPECT_LDOUBLE_PREC(value, to_equal, precision) expect_ldouble_prec(__FILE__, __LINE__, value, to_equal, precision)

#define EXPECT_PTR_NOT(value, to_equal) expect_ptr_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_CHAR_NOT(value, to_equal) expect_char_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_INT_NOT(value, to_equal) expect_int_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_LONG_NOT(value, to_equal) expect_long_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_LLONG_NOT(value, to_equal) expect_llong_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_UCHARNOT(value, to_equal) expect_uchar_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_UINTNOT(value, to_equal) expect_uint_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_ULONGNOT(value, to_equal) expect_ulong_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_ULLONGNOT(value, to_equal) expect_ullong_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_SIZE_NOT(value, to_equal) expect_size_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_FLOAT_NOT(value, to_equal) expect_float_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_DOUBLE_NOT(value, to_equal) expect_double_not(__FILE__, __LINE__, value, to_equal)
#define EXPECT_LDOUBLE_NOT(value, to_equal) expect_ldouble_not(__FILE__, __LINE__, value, to_equal)

#endif  // __TEST_H

//------------------------------------------------------------------------------
// SOURCE
//------------------------------------------------------------------------------

#ifdef TEST_IMPLEMENTATION

#define CREATE_EXPECT_NUMBER_FN(name, type, fmt) \
    void name( \
        const char *const file, const unsigned int line, \
        const type value, const type to_equal) \
    { \
        if (value != to_equal) { \
            fprintf(stderr, "%s:%u [TEST FAILED] expected " # type "\n    " fmt "\n  but got\n    " fmt "\n", \
                file, line, to_equal, value); \
            exit(EXIT_FAILURE); \
        } \
    } \
    void name ## _not( \
        const char *const file, const unsigned int line, \
        const type value, const type to_equal) \
    { \
        if (value == to_equal) { \
            fprintf(stderr, "%s:%u [TEST FAILED] expected " # type "to be different from\n    " fmt "\n  but got\n    " fmt "\n", \
                file, line, to_equal, value); \
            exit(EXIT_FAILURE); \
        } \
    }

#define CREATE_EXPECT_NUMBER_PREC_FN(name, type, fmt, abs_fn) \
    void name( \
        const char *const file, const unsigned int line, \
        const type value, const type to_equal, const type precision) \
    { \
        if (abs_fn(value - to_equal) > precision) { \
            fprintf(stderr, "%s:%u [TEST FAILED] expected " # type " (precision = " fmt ")\n    " fmt "\n  but got\n    " fmt "\n", \
                file, line, precision, to_equal, value); \
            exit(EXIT_FAILURE); \
        } \
    } \
    void name ## _not( \
        const char *const file, const unsigned int line, \
        const type value, const type to_equal, const type precision) \
    { \
        if (abs_fn(value - to_equal) < precision) { \
            fprintf(stderr, "%s:%u [TEST FAILED] expected " # type "to be different from (precision = " fmt ")\n    " fmt "\n  but got\n    " fmt "\n", \
                file, line, precision, to_equal, value); \
            exit(EXIT_FAILURE); \
        } \
    }

CREATE_EXPECT_NUMBER_FN(expect_ptr, void *, "%p")
CREATE_EXPECT_NUMBER_FN(expect_int, int, "%d")
CREATE_EXPECT_NUMBER_FN(expect_char, char, "%hhd")
CREATE_EXPECT_NUMBER_FN(expect_short, short, "%hd")
CREATE_EXPECT_NUMBER_FN(expect_long, long, "%ld")
CREATE_EXPECT_NUMBER_FN(expect_llong, long long, "%lld")
CREATE_EXPECT_NUMBER_FN(expect_uint, unsigned int, "%u")
CREATE_EXPECT_NUMBER_FN(expect_uchar, unsigned char, "%hhu")
CREATE_EXPECT_NUMBER_FN(expect_ushort, unsigned short, "%hu")
CREATE_EXPECT_NUMBER_FN(expect_ulong, unsigned long, "%lu")
CREATE_EXPECT_NUMBER_FN(expect_ullong, unsigned long long, "%llu")
CREATE_EXPECT_NUMBER_FN(expect_size, size_t, "%zu")
CREATE_EXPECT_NUMBER_FN(expect_float, float, "%f")
CREATE_EXPECT_NUMBER_FN(expect_double, double, "%f")
CREATE_EXPECT_NUMBER_FN(expect_ldouble, long double, "%Lf")

CREATE_EXPECT_NUMBER_PREC_FN(expect_float_prec, float, "%f", fabsf)
CREATE_EXPECT_NUMBER_PREC_FN(expect_double_prec, double, "%f", fabs)
CREATE_EXPECT_NUMBER_PREC_FN(expect_ldouble_prec, long double, "%f", fabsl)

void expect_str(const char *const file, const unsigned int line, const char *const value, const char *const to_equal)
{
    if (strcmp(value, to_equal) != 0) {
        // TODO: Introduce a function to escape special chars
        fprintf(stderr, "%s:%u [TEST FAILED] expected string\n    \"%s\"\n  but got\n    \"%s\"\n", file, line, to_equal, value);
        exit(EXIT_FAILURE);
    }
}

void expect_sized_str(const char *const file, const unsigned int line, const char *const value, const char *const to_equal, const size_t size)
{
    if (strncmp(value, to_equal, size) != 0) {
        fprintf(stderr, "%s:%u [TEST FAILED] expected sized string\n    \"%.*s\"\n  but got\n    \"%.*s\"\n", file, line, size, to_equal, size, value);
        exit(EXIT_FAILURE);
    }
}

void expect_true(const char *const file, const unsigned int line, const int value)
{
    if (value == 0) {
        fprintf(stderr, "%s:%u [TEST FAILED] expected true but got\n    %d\n", file, line, value);
        exit(EXIT_FAILURE);
    }
}

void expect_false(const char *const file, const unsigned int line, const int value)
{
    if (value != 0) {
        fprintf(stderr, "%s:%u [TEST FAILED] expected false but got\n    %d\n", file, line, value);
        exit(EXIT_FAILURE);
    }
}

#endif  // TEST_IMPLEMENTATION

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
