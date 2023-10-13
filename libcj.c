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

#include <ctype.h> // TODO: Remove this dependancy
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h> // TODO: Remove this dependancy

#include "libcj.h"

// https://cplusplus.com/reference/cstdio/scanf/

// TODO: strtod, strstr, strchr, strlen, isspace, sscanf
// TODO: Implement some kind of generics in C using preprocessor
// TODO: Try to make it fast and optimized
// TODO: We should use macro like CASE() to simplify parsing of codes in __snprintf


// The length modifiers j, z and t are not implemented
enum Length_Modifier {
    Modifier_None,
    Modifier_char, Modifier_short, Modifier_long, Modifier_longlong,
    Modifier_longdouble
};

enum Fmt_Specifier {
    Fmt_d, Fmt_i, Fmt_u, Fmt_o, Fmt_x,
    Fmt_f, Fmt_e, Fmt_g, Fmt_a,
    Fmt_c, Fmt_s, Fmt_p, Fmt_n, Fmt_percent,
    Fmt_unknown
};

enum Fmt_Flags {
    Flag_None  = 0x00,
    Flag_Minus = 0x01,
    Flag_Plus  = 0x02,
    Flag_Space = 0x04,
    Flag_Hash  = 0x08,
    Flag_Zero  = 0x10
};

// Parses a integer number using base 8, 10 or 16
static int str_to_int(
    const char *fmt,
    const unsigned int base,
    long int *const value
)
{
    int index = 0;
    long int number = 0;
    for (; fmt[index] != '\0'; index++) {
        const char c = fmt[index];
        int digit = 0;
        if (isdigit(c)) {
            digit = c - '0';
        } else if ((base == 16) && isxdigit(c)) {
            digit = tolower(c) - 'a' + 10;
        } else {
            break;
        }
        if (number <= LONG_MAX / base) {
            number = base * (number) + digit;
        } else {
            // Overflow detected
            // In this case we return the maximum positive number that a long int can store
            // We continue the loop, in order to parse all the number
            number = LONG_MAX;
        }
    }
    if (index > 0) {
        *value = number;
    }
    return index;
}

#define VALUE_TO_CHAR(value, uppercase) (((value) < 10) ? ((value) + '0') : (value - 10 + ((uppercase) ? 'A' : 'a')))

#define PUTCHAR(c)              \
    do {                        \
        if (sz == NULL) {       \
            **buf = (c);        \
            (*buf)++;           \
        } else if (*sz > 1) {   \
            **buf = (c);        \
            (*buf)++;           \
            (*sz)--;            \
        }                       \
    } while (0)

static int                      // Return the amount of characters that would have been written if we had enough size
int_to_str(
    char **buf,                 // Buffer in which we must write the output string
    size_t *const sz,           // Size of the buffer
    const int width,
    const int precision,
    const enum Fmt_Flags flags,
    const bool uppercase,       // The hexadecimal characters should be in upper case?
    const unsigned int base,    // Integer base (8, 10 or 16)
    const bool sign,            // The integer value informed is signed?
    const intmax_t value        // Integer value to be converted to string
)
{
    char str[32];
    const bool negative = sign && (value < 0);
    uintmax_t x = (uintmax_t)(negative ? (-value) : value);
    int written = 0;
    do {
        str[written++] = VALUE_TO_CHAR((char)(x % base), uppercase);
        x /= base;
    } while (x > 0);
    // Flag_Zero is ignored if precision is informed
    int zeros = (precision >= 0) ? precision : ((flags & Flag_Zero) ? width : 0);
    if ((flags & Flag_Zero) && (flags & Flag_Hash)) {
        if (base == 8) {
            zeros -= 1;
        } else if (base == 16) {
            zeros -= 2;
        }
    }
    while (written < zeros) {
        str[written++] = '0';
    }
    if (flags & Flag_Hash) {
        if (base == 8) {
            str[written++] = '0';
        } else if (base == 16) {
            str[written++] = uppercase ? 'X' : 'x';
            str[written++] = ('0');
        }
    }
    if ((base != 8) && (negative || (flags & Flag_Plus))) {
        if (!sign) {
            str[written++] = (flags & Flag_Hash) ? '+' : ' ';
        } else {
            str[written++] = negative ? '-' : '+';
        }
    }
    const bool left_justify = flags & Flag_Minus;
    while (!left_justify && written < width) {
        str[written++] = ' ';
    }
    for (int index = written; index > 0; index--) {
        PUTCHAR(str[index-1]);
    }
    while (left_justify && written < width) {
        PUTCHAR(' ');
        written++;
    }
    // This function doesn't need to introduce null termination to the buffer
    // This is responsability of its caller
    return written;
}

// Computes x * base^exponent
static double scale_radix_exp(double x, const unsigned int radix, int exponent) {
    if (x == 0.0) {
        return x;
    }
    if (exponent < 0) {
        while (exponent++ != 0) {
            x /= radix;
        }
    } else {
        while (exponent-- != 0) {
            if (x < -DBL_MAX / radix) {
                return -HUGE_VAL;
            } else if (DBL_MAX / radix < x) {
                return HUGE_VAL;
            }
            x *= radix;
        }
    }
    return x;
}

static long div_remainder(const double dividend, const double divisor, const bool round_up)
{
    const long quotient = (long)(dividend / divisor);
    const double remainder = dividend - (divisor * (double)quotient);
    // TODO: The rest of the code is ugly. But this is simply disgusting!!!
    if (round_up && (div_remainder(remainder*divisor, divisor, false) >= divisor/2)) {
        return (long)(remainder+1);
    }
    return (long)remainder;
}

// value must be positive
static long int float_exponent_form(double *const value, const unsigned int base)
{
    long int exponent = 0;
    if ((int)(*value/base) > 0) {
        while ((int)(*value/base) > 0) {
            *value = scale_radix_exp(*value, base, -1);
            exponent++;
        }
    } else {
        while ((int)(*value) == 0) {
            *value = scale_radix_exp(*value, base, 1);
            exponent--;
        }
    }
    return exponent;
}

#define ABS(x)  (((x) >= 0) ? (x) : -(x))

// TODO: Implement a sort of generics with this functions
// The current version will be just to slow in embedded

static int                      // Return the amount of characters that would have been written if we had enough size
float_to_str(
    char **buf,                 // Buffer in which we must write the output string
    size_t *const sz,           // Size of the buffer
    const int width,
    int precision,
    const enum Fmt_Flags flags,
    bool exponent_form,         // Use exponent form?
    const bool short_form,      // Use short form?
    const bool uppercase,       // The hexadecimal characters should be in upper case?
    const unsigned int base,    // Integer base (10 or 16)
    const double value          // Float value to be converted to string
)
{
    char str[32];
    (void)width;
    (void)flags;
    //const bool pad_with_zeros = flags & Flag_Zero;
    //const bool left_justify = flags & Flag_Minus;
    //const bool must_add_sign = flags & Flag_Plus;
    const bool negative = value < 0.0;
    double x = negative ? (-value) : value;
    int written = 0;
    long int exponent = 0;
    const double original_x = x;
    if (exponent_form | short_form) {
        exponent = float_exponent_form(&x, base == 16 ? 2 : base);
    }
    if (short_form) {
        exponent_form = (ABS(exponent) >= 5);
        if (precision < 0) {
            precision = exponent_form ? 5 : (int)(5L - exponent);
        }
        if (!exponent_form) {
            x = original_x;
        }
    }
    if (precision < 0) { // Default precision
        precision = (base == 16) ? 13 : 6;
    }
    for (int exp = precision; exp > 0; exp--) {
        const double y = scale_radix_exp(x, base, exp);
        const char c = (char)div_remainder(y, base, exp == precision);
        if ((!short_form) || (c != 0) || (written != 0)) {
            str[written++] = VALUE_TO_CHAR(c, uppercase);
        }
    }
    if ((!short_form) || (written > 0)) {
        str[written++] = '.';
    }
    char c = (char)div_remainder(x, base, false);
    do {
        str[written++] = VALUE_TO_CHAR(c, uppercase);
        x = scale_radix_exp(x, base, -1);
        c = (char)div_remainder(x, base, false);
    } while (c > 0);
    if (negative) {
        str[written++] = '-';
    }
    for (int index = written; index > 0; index--) {
        PUTCHAR(str[index-1]);
    }
    if (exponent_form) {
        if (base == 16) {
            PUTCHAR(uppercase ? 'P' : 'p');
        } else {
            PUTCHAR(uppercase ? 'E' : 'e');
        }
        written++;
        written += int_to_str(buf, sz, -1, ((base == 10) ? 2 : 1), Flag_Plus, uppercase, 10, true, exponent);
    }
    // This function doesn't need to introduce null termination to the buffer
    // This is responsability of its caller
    return written;
}

static int put_string(
    char **buf,
    size_t *const sz,
    const int width,
    const int precision,
    const bool left_justify, // Right justification is the default
    const char *string
)
{
    int written = 0;
    if (string == NULL) {
        string = "(null)";
    }
    if ((!left_justify) && (width > 0)) {
        int spaces = width - (int)strlen(string);
        while (spaces > 0) {
            PUTCHAR(' ');
            written++;
            spaces--;
        }
    }
    while ((*string != '\0') && (precision < 0 || (written < precision))) {
        PUTCHAR(*string);
        string++;
        written++;
    }
    if ((left_justify) && (width > 0)) {
        int spaces = width - written;
        while (spaces > 0) {
            PUTCHAR(' ');
            written++;
            spaces--;
        }
    }
    return written;
}

static int parse_fmt_flags(
    const char *const fmt,
    enum Fmt_Flags *const flags
)
{
    *flags = Flag_None;
    int index = 0;
    for (; fmt[index] != '\0'; index++) {
        switch (fmt[index]) {
        case '-': *flags |= Flag_Minus; break;
        case '+': *flags |= Flag_Plus;  break;
        case ' ': *flags |= Flag_Space; break;
        case '#': *flags |= Flag_Hash;  break;
        case '0': *flags |= Flag_Zero;  break;
        default: return index;
        }
    }
    return index;
}

// TODO: scanf does not have precision
// TODO: scanf has a special "flag" asterisk
// TODO: In printf, the width can be replaced by asterisk instead of using numbers
static int parse_fmt_specifier(
    const char *const fmt,
    enum Fmt_Specifier *const specifier,
    enum Length_Modifier *const modifier,
    bool *const uppercase
)
{
    int index = 0;
    *specifier = Fmt_unknown;
    // Parse the length modifier
    *modifier = Modifier_None;
    if (fmt[index] == 'h') {
        index++;
        if (fmt[index] == 'h') {
            index++;
            *modifier = Modifier_char;
        } else {
            *modifier = Modifier_short;
        }
    } else if (fmt[index] == 'l') {
        index++;
        if (fmt[index] == 'l') {
            index++;
            *modifier = Modifier_longlong;
        } else {
            *modifier = Modifier_long;
        }
    } else if (fmt[index] == 'L') {
        index++;
        *modifier = Modifier_longdouble;
    }
    // Parse the format specifier
    switch (fmt[index]) {
    // Integers
    case 'd': *specifier = Fmt_d; break;
    case 'i': *specifier = Fmt_i; break;
    case 'u': *specifier = Fmt_u; break;
    case 'o': *specifier = Fmt_o; break;
    case 'x':
    case 'X': *specifier = Fmt_x; break;
    // Floating points
    case 'f':
    case 'F': *specifier = Fmt_f; break;
    case 'e':
    case 'E': *specifier = Fmt_e; break;
    case 'g':
    case 'G': *specifier = Fmt_g; break;
    case 'a':
    case 'A': *specifier = Fmt_a; break;
    // Others
    case 'c': *specifier = Fmt_c; break;
    case 's': *specifier = Fmt_s; break;
    case 'p': *specifier = Fmt_p; break;
    case 'n': *specifier = Fmt_n; break;
    case '%': *specifier = Fmt_percent; break;
    }
    if (*specifier == Fmt_unknown) {
        return 0;
    }
    *uppercase = isupper(fmt[index]);
    // Return the amount of characters parsed
    return (index + 1);
}

static intmax_t next_int_arg(va_list args, enum Length_Modifier modifier)
{
    switch (modifier) {
    case Modifier_char: return (char)va_arg(args, int);
    case Modifier_short: return (short)va_arg(args, int);
    case Modifier_long: return va_arg(args, long);
    case Modifier_longlong: return va_arg(args, long long);
    case Modifier_None:
    case Modifier_longdouble:
    default: return va_arg(args, int);
    }
}

static intmax_t next_uint_arg(va_list args, enum Length_Modifier modifier)
{
    switch (modifier) {
    case Modifier_char: return (unsigned char)va_arg(args, unsigned int);
    case Modifier_short: return (unsigned short)va_arg(args, unsigned int);
    case Modifier_long: return (intmax_t)va_arg(args, unsigned long);
    case Modifier_longlong: return (intmax_t)va_arg(args, unsigned long long);
    case Modifier_None:
    case Modifier_longdouble:
    default: return va_arg(args, unsigned int);
    }
}

static double next_float_arg(va_list args, enum Length_Modifier modifier)
{
    switch (modifier) {
    case Modifier_longdouble: return (double)va_arg(args, long double);
    case Modifier_None:
    case Modifier_char:
    case Modifier_short:
    case Modifier_long:
    case Modifier_longlong:
    default: return va_arg(args, double);
    }
}

// TODO: The zero flag is ignored with strings and when precision is specified

static int __snprintf(char **buf, size_t *const sz, const char *fmt, va_list args)
{
    enum Fmt_Specifier specifier;
    enum Fmt_Flags flags;
    enum Length_Modifier modifier;
    bool uppercase;
    int written = 0;
    const char *cursor = fmt;
    while (*cursor != '\0') {
        if (*cursor != '%') {
            PUTCHAR(*cursor);
            cursor++;
            written++;
        } else {
            cursor++; // Escape % character
            // snprintf format specifier follows this pattern:
            // %[flags][width][.precision][length]specifier
            // https://cplusplus.com/reference/cstdio/printf/
            int parsed_chars = parse_fmt_flags(cursor, &flags);
            long int width = -1, precision = -1;
            parsed_chars += str_to_int((cursor+parsed_chars), 10, &width);
            if (*(cursor+parsed_chars) == '.') {
                parsed_chars++;
                if (*(cursor+parsed_chars) == '*') {
                    parsed_chars++;
                    precision = va_arg(args, int);
                } else {
                    parsed_chars += str_to_int((cursor+parsed_chars), 10, &precision);
                }
            }
            parsed_chars += parse_fmt_specifier((cursor+parsed_chars), &specifier, &modifier, &uppercase);
            // If the format specifier was fully parsed, update the cursor position
            if (specifier != Fmt_unknown) {
                cursor += parsed_chars;
            }
            switch (specifier) {
            case Fmt_d:
            case Fmt_i: // Signed integer
                written += int_to_str(buf, sz, (int)width, (int)precision, flags, uppercase, 10, true, next_int_arg(args, modifier));
                break;
            case Fmt_u: // Unsigned integer
                written += int_to_str(buf, sz, (int)width, (int)precision, flags, uppercase, 10, false, next_uint_arg(args, modifier));
                break;
            case Fmt_o: // Unsigned integer in octal form
                written += int_to_str(buf, sz, (int)width, (int)precision, flags, uppercase, 8, false, next_uint_arg(args, modifier));
                break;
            case Fmt_x: // Unsigned integer in hexadecimal form
                written += int_to_str(buf, sz, (int)width, (int)precision, flags, uppercase, 16, false, next_uint_arg(args, modifier));
                break;
            case Fmt_f: // Decimal floating point
                written += float_to_str(buf, sz, (int)width, (int)precision, flags, false, false, uppercase, 10, next_float_arg(args, modifier));
                break;
            case Fmt_e: // Decimal floating point in exponent form
                written += float_to_str(buf, sz, (int)width, (int)precision, flags, true, false, uppercase, 10, next_float_arg(args, modifier));
                break;
            case Fmt_g: // Decimal floating point in shortest form
                written += float_to_str(buf, sz, (int)width, (int)precision, flags, false, true, uppercase, 10, next_float_arg(args, modifier));
                break;
            case Fmt_a: // Decimal floating point in hexadecimal form
                PUTCHAR('0');
                PUTCHAR(uppercase ? 'X' : 'x');
                written += 2;
                written += float_to_str(buf, sz, (int)width, (int)precision, flags, true, false, uppercase, 16, next_float_arg(args, modifier));
                break;
            case Fmt_c: // Character
                PUTCHAR((char)va_arg(args, int));
                written++;
                break;
            case Fmt_s: // String
                written += put_string(buf, sz, (int)width, (int)precision, (flags & Flag_Minus), va_arg(args, char *));
                break;
            case Fmt_p: { // Pointer
                const void *ptr = va_arg(args, void *);
                if (ptr == NULL) {
                    written += put_string(buf, sz, (int)width, (int)precision, (flags & Flag_Minus), "(nil)");
                } else {
                    written += int_to_str(buf, sz, (int)width, (int)precision, (flags | Flag_Hash), uppercase, 16, false, (intmax_t)ptr);
                }
            } break;
            case Fmt_n: { // Return the number of characters written so far
                int *ptr = va_arg(args, int *);
                *ptr = written;
            } break;
            case Fmt_percent:
            case Fmt_unknown:
                PUTCHAR('%');
                written++;
                break;
            }
        }
    }
    // null termination
    if ((sz == NULL) || (*sz > 0)) {
        **buf = '\0';
    }
    return written;
}

int snprintf(char *buf, size_t sz, const char *fmt, ...)
{
    if (fmt == NULL) {
        return -1;
    }
    va_list args;
    va_start(args, fmt);
    int written = __snprintf(&buf, &sz, fmt, args);
    va_end(args);
    return written;
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
