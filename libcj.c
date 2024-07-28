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

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libcj.h"

//------------------------------------------------------------------------------
// DEFINITIONS
//------------------------------------------------------------------------------

#ifdef __GNUC__ 
#define INLINE_FUNCTION inline
#elif defined(_MSC_VER)
#define INLINE_FUNCTION __inline
#else
#define INLINE_FUNCTION
#endif

#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define THREAD_LOCAL thread_local
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define THREAD_LOCAL _Thread_local
#elif defined(_MSC_VER)
#define THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__)
#define THREAD_LOCAL __thread
#else
#error "No support for thread-local storage."
#endif

// Modifier for the local private functions
#define LIBCJ_FN static INLINE_FUNCTION

// Local buffer size for functions that handle strings
#define LOCAL_BUFFER_SIZE 1024 // This should be enough for everyone

#define VALUE_TO_CHAR(value, uppercase) \
    (((value) < 10) ? ((value) + '0') : (value - 10 + ((uppercase) ? 'A' : 'a')))

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

#define SKIP_WHITESPACES(buf)   \
    do {                        \
        while (isspace(*buf)) { \
            buf++;              \
        }                       \
    } while (0)

#define ADVANCE_CURSOR(cursor)  \
    do {                        \
        if (*cursor != '\0') {  \
            cursor++;           \
        }                       \
    } while (0)

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define ABS(x)    (((x) >= 0)  ? (x) : -(x))

//------------------------------------------------------------------------------
// CUSTOM TYPES
//------------------------------------------------------------------------------

// TODO: The length modifiers j, z and t are not implemented
enum Length_Modifier {
    Modifier_None,
    Modifier_char, Modifier_short, Modifier_long, Modifier_llong,
    Modifier_ldouble
};

enum Fmt_Specifier {
    Fmt_d, Fmt_i, Fmt_u, Fmt_o, Fmt_x,
    Fmt_f, Fmt_e, Fmt_g, Fmt_a,
    Fmt_c, Fmt_s, Fmt_p, Fmt_n, Fmt_percent,
    Fmt_unknown
};

// sprintf flags
enum Fmt_Flags {
    Flag_None  = 0x00,
    Flag_Minus = 0x01,
    Flag_Plus  = 0x02,
    Flag_Space = 0x04,
    Flag_Hash  = 0x08,
    Flag_Zero  = 0x10,
    Flag_Upper = 0x20,
    Flag_Exp   = 0x40, // Float number exponential form
    Flag_Short = 0x80, // Float number short form
};

//------------------------------------------------------------------------------
// SOURCE
//------------------------------------------------------------------------------

// Receives a scanset string (not containing the initial open bracket '[')
// and returns true if the character is in accordance with that scanset
// If the scanset string ends before a closing bracket ']' is found,
// false is returned
// This functions is used by vsscanf
LIBCJ_FN bool char_in_scanset(const char *scanset, const char c)
{
    bool excluded_set = false;
    if (*scanset == '^') {
        excluded_set = true;
        scanset++;
    }
    bool found = excluded_set;
    if (*scanset == ']') {
        if (c == *scanset) {
            found = !excluded_set;
        }
        scanset++;
    }
    for (; *scanset != ']'; scanset++) {
        if (*scanset == '\0') {
            return false; // Didn't found closing bracket
        }
        if ((*(scanset+1) == '-') && (*(scanset+2) != ']')) {
            const char range_begin = *scanset;
            scanset += 2;
            const char range_end = *scanset;
            if ((range_begin <= c) && (c <= range_end)) {
                found = !excluded_set;
            }
        } else if (c == *scanset) {
            found = !excluded_set;
        }
    }
    return found;
}

// Important: Our library currently contains a significant amount of redundancy
// and duplicated code, particularly within the functions responsible for converting
// strings into numerical values. Each of these conversion functions possesses
// distinct characteristics, and the nature of the C language presents challenges
// in crafting highly efficient code without resorting to repetition. Given this
// context, I have opted to reimplement these features individually, despite the
// redundancy, to achieve a higher level of code efficiency.

// Parses a string into a natural number (integer greather than zero)
// Overflows aren't handled
// If no integer was parsed, the value is kept at its original value
// Returns the amount of characters that were consumed
LIBCJ_FN int str_to_natural(const char *const str, int *const value)
{
    const int base = 10;
    int index = 0;
    int number = 0;
    for (; str[index] != '\0'; index++) {
        if (!isdigit(str[index])) {
            break;
        }
        const int digit = str[index] - '0';
        number = base * number + digit;
    }
    if (index > 0) {
        *value = number;
    }
    return index;
}

// Parses a string into a sign integer
// The number may be preceded by optional sign
// When a positive overflow occurs, the 'value' is set to INT_MAX, and conversely,
// in the case of negative overflow, 'value' is assigned to INT_MIN
// If width is greather than zero, it parses at most width characters
// Returns the amount of characters that were consumed
LIBCJ_FN int str_to_sign_integer(const char *const str, const int width, int *const value)
{
    const int base = 10;
    int index = 0;
    bool negative = false;
    *value = 0;
    if ((str[index] == '+') || (str[index] == '-')) {
        negative = str[index] == '-';
        index++;
    }
    for (; ((width < 0) || (index < width)) && (str[index] != '\0'); index++) {
        if (!isdigit(str[index])) {
            break;
        }
        const int digit = str[index] - '0';
        if (negative) {
            if (*value >= INT_MIN / base) {
                *value = base * (*value) - digit;
            } else {
                *value = INT_MIN;
            }
        } else {
            if (*value <= INT_MAX / base) {
                *value = base * (*value) + digit;
            } else {
                *value = INT_MAX;
            }
        }
    }
    return index;
}

// Generic function to convert string into integers in bases 0, 8, 10 or 16
// Returns the amount of characters that were consumed
// If width is greather than zero, it parses at most width characters
static int str_to_int(const char *const str, int width, int base, intmax_t *const value)
{
    int index = 0;
    bool negative = false;
    *value = (intmax_t)0;
    if ((str[index] == '+') || (str[index] == '-')) {
        negative = str[index] == '-';
        index++;
    }
    if (str[index] == '0') {
        if (str[index+1] == 'x') {
            if ((base == 0) || (base == 16)) {
                base = 16;
                index += 2;
            }
        } else {
            if ((base == 0) || (base == 8)) {
                base = 8;
                index++;
            }
        }
    }
    if (base == 0) { // Default base
        base = 10;
    }
    for (; ((width < 0) || (index < width)) && (str[index] != '\0'); index++) {
        const char c = str[index];
        int digit = 0;
        if (isdigit(c)) {
            digit = c - '0';
        } else if ((base > 10) && isalpha(c)) {
            digit = tolower(c) - 'a' + 10;
        } else {
            break;
        }
        if (digit >= base) {
            break;
        }
        *value = base * (*value) + ((negative) ? -digit : digit);
    }
    return index;
}

// Generic function to convert integer numbers to string
// Return the amount of characters that would have been written if we had enough size
static int int_to_str(char **buf, size_t *const sz,
    const int width, const int precision, const enum Fmt_Flags flags, const int base,
    const bool sign, const intmax_t value)
{
    char str[LOCAL_BUFFER_SIZE]; 
    const bool left_justify = (flags & Flag_Minus) != 0;
    const bool negative = sign && (value < 0);
    const bool uppercase = (flags & Flag_Upper) != 0;
    // Pointers are the only unsigned integer that can have the plus flag
    const bool include_sign = negative || ((flags & Flag_Plus) != 0);
    const bool show_base_padding = ((flags & Flag_Hash) != 0) && (base != 10) && (value != 0);
    const int base_padding = show_base_padding ? (base / 8) : 0;
    // Padding used for sign and base specifiers
    const int left_padding = base_padding + (include_sign ? 1 : 0);
    const bool use_precision = (precision >= 0) || ((flags & Flag_Minus) != 0);
    // Flag_Zero is ignored if precision or Flag_Minus are informed
    const int left_pad_zeros = (flags & Flag_Zero) ? (width-left_padding) : 0;
    const int zeros = use_precision ? precision : left_pad_zeros;
    uintmax_t x = (uintmax_t)(negative ? (-value) : value);
    int written = 0;
    do {
        str[written++] = VALUE_TO_CHAR((char)(x % (unsigned int)base), uppercase);
        x /= (unsigned int)base;
    } while (x > 0);
    while (written < zeros) {
        str[written++] = '0';
    }
    if (base_padding) {
        switch (base) {
        case 8:
            if ((!use_precision || ((written > 0) && (str[written-1] != '0')))) {
                str[written++] = '0';
            }
            break;
        case 16:
            str[written++] = uppercase ? 'X' : 'x';
            str[written++] = '0';
            break;
        }
    }
    if (include_sign) {
        str[written++] = negative ? '-' : '+';
    }
    if (!left_justify) {
        while (written < width) {
            str[written++] = ' ';
        }
    }
    for (int index = written; index > 0; index--) {
        PUTCHAR(str[index-1]);
    }
    if (left_justify) {
        while (written < width) {
            PUTCHAR(' ');
            written++;
        }
    }
    // This function doesn't need to introduce null termination to the buffer
    // This is responsability of its caller
    return written;
}

// Computes x * radix^exponent
static double scale_radix_exp(double x, const int radix, int exponent)
{
    if (x == 0.0) {
        return x;
    }
    if (exponent < 0) {
        while (exponent++ != 0) {
            x /= radix;
        }
    } else {
        while (exponent-- != 0) {
            if (x < -DBL_MAX/radix) {
                return -HUGE_VAL;
            } else if (DBL_MAX/radix < x) {
                return HUGE_VAL;
            }
            x *= radix;
        }
    }
    return x;
}

LIBCJ_FN long div_remainder_double(const double dividend, const double divisor)
{
    const long quotient = (long)(dividend / divisor);
    const double remainder = dividend - (divisor * (double)quotient);
    return (long)remainder;
}

// value must be positive
LIBCJ_FN long int exponent_form_double(double *const value, const int base)
{
    long int exponent = 0;
    if (*value == 0.0) {
        return 0;
    }
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

LIBCJ_FN double rouding_double(const double value, const int base, const int decimal_places)
{
    const double y = scale_radix_exp(value, base, decimal_places);
    const double remainder = y - (long)y;
    if (remainder >= 0.5) {
        const double value_to_add = scale_radix_exp(1.0, base, -decimal_places);
        return (value + value_to_add);
    }
    return value;
}

// Generic function to convert floating point numbers to string
// Return the amount of characters that would have been written if we had enough size
static int double_to_str(char **buf, size_t *const sz,
    const int width, int precision, enum Fmt_Flags flags, const int base,
    double value)
{
    char str[LOCAL_BUFFER_SIZE];
    const bool negative = value < 0.0;
    const bool uppercase = (flags & Flag_Upper) != 0;
    const bool left_justify = (flags & Flag_Minus) != 0;
    const bool include_sign = negative || ((flags & Flag_Plus) != 0);
    const bool right_fill_with_zeros = ((base != 16) || precision > 0) && ((flags & Flag_Short) == 0);
    const int base_padding = (base == 16) ? 2 : 0;
    const int left_padding = base_padding + (include_sign ? 1 : 0);
    // Flag_Zero is ignored if Flag_Minus is informed
    const bool pad_with_zeros = !(flags & Flag_Minus) && (flags & Flag_Zero);
    const int zeros = pad_with_zeros ? (width-left_padding) : 0;
    value = negative ? (-value) : value;
    int written = 0;
    long int exponent = 0;
    const double original_value = value;
    if (precision < 0) { // Default precision
        precision = (base == 16) ? 13 : 6;
    }
    if (flags & (Flag_Exp | Flag_Short)) {
        exponent = exponent_form_double(&value, base == 16 ? 2 : base);
    }
    if (flags & Flag_Short) {
        if (ABS(exponent) >= 5) {
            flags |= Flag_Exp;
        }
        precision = flags & Flag_Exp ? (precision - 1) : (int)(precision - 1 - exponent);
        if ((flags & Flag_Exp) == 0) {
            value = original_value;
        }
    }
    if (flags & Flag_Exp) {
        const int exponent_base = 10;
        const int exponent_length = (base == 10) ? 2 : 1;
        const bool exponent_negative = exponent < 0;
        exponent = exponent_negative ? (-exponent) : exponent;
        while ((exponent > 0) || (written < exponent_length)) {
            str[written++] = VALUE_TO_CHAR((char)(exponent % exponent_base), uppercase);
            exponent /= exponent_base;
        }
        str[written++] = exponent_negative ? '-' : '+';
        if (base == 16) {
            str[written++] = uppercase ? 'P' : 'p';
        } else {
            str[written++] = uppercase ? 'E' : 'e';
        }
    }
    bool has_decimal_chars = false;
    value = rouding_double(value, base, precision);
    for (int exp = precision; exp > 0; exp--) {
        const double y = scale_radix_exp(value, base, exp);
        char c = (char)div_remainder_double(y, base);
        if (right_fill_with_zeros || has_decimal_chars || (c != 0)) {
            str[written++] = VALUE_TO_CHAR(c, uppercase);
            has_decimal_chars = true;
        }
    }
    if (has_decimal_chars) {
        str[written++] = '.';
    }
    char c = (char)div_remainder_double(value, base);
    do {
        str[written++] = VALUE_TO_CHAR(c, uppercase);
        value = scale_radix_exp(value, base, -1);
        c = (char)div_remainder_double(value, base);
    } while (c > 0);
    while (written < zeros) {
        str[written++] = '0';
    }
    if (base_padding) {
        str[written++] = uppercase ? 'X' : 'x';
        str[written++] = '0';
    }
    if (include_sign) {
        str[written++] = negative ? '-' : '+';
    }
    if (flags & Flag_Space) {
        str[written++] = ' ';
    }
    if (!left_justify) {
        while (written < width) {
            str[written++] = ' ';
        }
    }
    for (int index = written; index > 0; index--) {
        PUTCHAR(str[index-1]);
    }
    if (left_justify) {
        while (written < width) {
            PUTCHAR(' ');
            written++;
        }
    }
    // This function doesn't need to introduce null termination to the buffer
    // This is responsability of its caller
    return written;
}

// Generic function to convert string into double in base 10
// Returns the amount of characters that were consumed
// If width is greather than zero, it parses at most width characters
LIBCJ_FN int str_to_double(const char *const str, int width, double *const value) {
    const int base = 10;
    bool dotted = false;
    int exponent = 0;
    int index = 0;
    bool negative = false;
    *value = 0.0;
    if (width >= 0) {
        width += index;
    }
    if ((str[index] == '+') || (str[index] == '-')) {
        negative = str[index] == '-';
        index++;
    }
    for (; ((width < 0) || (index < width)) && (str[index] != '\0'); index++) {
        if (isdigit(str[index])) {
            const int digit = str[index] - '0';
            if (*value <= DBL_MAX / base) {
                *value = *value * base + digit;
            } else {
                exponent++;
            }
            if (dotted) {
                exponent--;
            }
        } else if (str[index] == '.') {
            if (dotted) {
                break;
            }
            dotted = true;
        } else {
            break;
        }
    }
    if (tolower(str[index]) == 'e') {
        int exp;
        const int parsed = str_to_sign_integer(&str[index+1], (width-index-1), &exp);
        if (parsed > 0) {
            index += parsed + 1;
            if (exponent > 0) {
                exponent = ((INT_MAX - exponent) < exp) ? INT_MAX : (exponent + exp);
            } else {
                exponent = (exp < (INT_MIN - exponent)) ? INT_MIN : (exponent + exp);
            }
        }
    }
    *value = scale_radix_exp(negative ? -(*value) : *value, base, exponent);
    return index;
}

static int put_string(char **buf, size_t *const sz, const int width, int precision, const bool left_justify, const char *string)
{
    int written = 0;
    if (string == NULL) {
        string = "(null)";
    }
    if ((!left_justify) && (width > 0)) {
        const int len = (int)strlen(string);
        int spaces = width - ((precision > 0) ? MIN(len, precision) : len);
        while (spaces > 0) {
            PUTCHAR(' ');
            written++;
            spaces--;
        }
    }
    while ((*string != '\0') && (precision != 0)) {
        PUTCHAR(*string);
        string++;
        written++;
        precision--;
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

LIBCJ_FN int parse_fmt_flags(const char *const fmt, enum Fmt_Flags *const flags)
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

LIBCJ_FN int parse_width_precision(const char *const fmt, va_list args, int *const width, int *const precision)
{
    int index = 0;
    if (fmt[index] == '*') {
        index++;
        *width = va_arg(args, int);
    } else {
        index += str_to_natural(&fmt[index], width);
    }
    if (fmt[index] == '.') {
        index++;
        if (fmt[index] == '*') {
            index++;
            *precision = va_arg(args, int);
        } else {
            index += str_to_natural(&fmt[index], precision);
        }
    }
    return index;
}

LIBCJ_FN int parse_fmt_specifier(const char *const fmt, enum Fmt_Specifier *const specifier, enum Length_Modifier *const modifier, bool *const uppercase)
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
            *modifier = Modifier_llong;
        } else {
            *modifier = Modifier_long;
        }
    } else if (fmt[index] == 'L') {
        index++;
        *modifier = Modifier_ldouble;
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
    if (uppercase != NULL) {
        *uppercase = isupper(fmt[index]);
    }
    // Return the amount of characters parsed
    return (index + 1);
}

// Helper macros used to simplify code in __vsnprintf
#define PRINTF_HANDLE_INT(base)                                                                            \
    do {                                                                                                   \
        switch (modifier) {                                                                                \
        case Modifier_char:                                                                                \
            written += int_to_str(buf, sz, width, precision, flags, base, true, (char)va_arg(args, int));  \
            break;                                                                                         \
        case Modifier_short:                                                                               \
            written += int_to_str(buf, sz, width, precision, flags, base, true, (short)va_arg(args, int)); \
            break;                                                                                         \
        case Modifier_long:                                                                                \
            written += int_to_str(buf, sz, width, precision, flags, base, true, va_arg(args, long));       \
            break;                                                                                         \
        case Modifier_llong:                                                                               \
            written += int_to_str(buf, sz, width, precision, flags, base, true, va_arg(args, long long));  \
            break;                                                                                         \
        case Modifier_None:                                                                                \
        case Modifier_ldouble:                                                                             \
        default:                                                                                           \
            written += int_to_str(buf, sz, width, precision, flags, base, true, va_arg(args, int));        \
            break;                                                                                         \
        }                                                                                                  \
    } while (0)

#define PRINTF_HANDLE_UINT(base)                                                                                              \
    do {                                                                                                                      \
        flags = flags & (enum Fmt_Flags)~Flag_Plus; /* This flag isn't supported for unsigned numbers */                      \
        switch (modifier) {                                                                                                   \
        case Modifier_char:                                                                                                   \
            written += int_to_str(buf, sz, width, precision, flags, base, false, (unsigned char)va_arg(args, unsigned int));  \
            break;                                                                                                            \
        case Modifier_short:                                                                                                  \
            written += int_to_str(buf, sz, width, precision, flags, base, false, (unsigned short)va_arg(args, unsigned int)); \
            break;                                                                                                            \
        case Modifier_long:                                                                                                   \
            written += int_to_str(buf, sz, width, precision, flags, base, false, (intmax_t)va_arg(args, unsigned long));      \
            break;                                                                                                            \
        case Modifier_llong:                                                                                                  \
            written += int_to_str(buf, sz, width, precision, flags, base, false, (intmax_t)va_arg(args, unsigned long long)); \
            break;                                                                                                            \
        case Modifier_None:                                                                                                   \
        case Modifier_ldouble:                                                                                                \
        default:                                                                                                              \
            written += int_to_str(buf, sz, width, precision, flags, base, false, va_arg(args, unsigned int));                 \
            break;                                                                                                            \
        }                                                                                                                     \
    } while (0)

// TODO: long double is not currently supported
#define PRINTF_HANDLE_FLOAT(base, special_flag)                                                                                   \
    do {                                                                                                                          \
        switch (modifier) {                                                                                                       \
        case Modifier_ldouble:                                                                                                    \
            written += double_to_str(buf, sz, width, precision, (flags | special_flag), base, (double)va_arg(args, long double)); \
            break;                                                                                                                \
        case Modifier_None:                                                                                                       \
        case Modifier_char:                                                                                                       \
        case Modifier_short:                                                                                                      \
        case Modifier_long:                                                                                                       \
        case Modifier_llong:                                                                                                      \
        default:                                                                                                                  \
            written += double_to_str(buf, sz, width, precision, (flags | special_flag), base, va_arg(args, double));              \
            break;                                                                                                                \
        }                                                                                                                         \
    } while (0)

static int __vsnprintf(char **buf, size_t *const sz, const char *fmt, va_list args)
{
    enum Fmt_Flags flags;
    enum Length_Modifier modifier;
    bool uppercase;
    int written = 0;
    const char *cursor = fmt;
    if (fmt == NULL) {
        return -1;
    }
    while (*cursor != '\0') {
        enum Fmt_Specifier specifier = Fmt_unknown;
        int width = -1, precision = -1;
        if (*cursor == '%') {
            va_list copied_args;
            va_copy(copied_args, args);
            int parsed_chars = 1; // Start with one character parsed ('%')
            // snprintf format specifier follows this pattern:
            // %[flags][width][.precision][length]specifier
            // https://cplusplus.com/reference/cstdio/printf/
            parsed_chars += parse_fmt_flags((cursor+parsed_chars), &flags);
            parsed_chars += parse_width_precision((cursor+parsed_chars), copied_args, &width, &precision);
            parsed_chars += parse_fmt_specifier((cursor+parsed_chars), &specifier, &modifier, &uppercase);
            // If the format specifier was fully parsed, update the cursor position and args
            if (specifier != Fmt_unknown) {
                args = copied_args;
                cursor += parsed_chars;
            }
            if (uppercase) {
                flags |= Flag_Upper;
            }
        }
        switch (specifier) {
        case Fmt_d:
        case Fmt_i: // Signed integer
            PRINTF_HANDLE_INT(10);
            break;
        case Fmt_u: // Unsigned integer
            PRINTF_HANDLE_UINT(10);
            break;
        case Fmt_o: // Unsigned integer in octal form
            PRINTF_HANDLE_UINT(8);
            break;
        case Fmt_x: // Unsigned integer in hexadecimal form
            PRINTF_HANDLE_UINT(16);
            break;
        case Fmt_f: // Decimal floating point
            PRINTF_HANDLE_FLOAT(10, 0);
            break;
        case Fmt_e: // Decimal floating point in exponent form
            PRINTF_HANDLE_FLOAT(10, Flag_Exp);
            break;
        case Fmt_g: // Decimal floating point in shortest form
            PRINTF_HANDLE_FLOAT(10, Flag_Short);
            break;
        case Fmt_a: // Decimal floating point in hexadecimal form
            PRINTF_HANDLE_FLOAT(16, Flag_Exp);
            break;
        case Fmt_c: // Character
            PUTCHAR((char)va_arg(args, int));
            written++;
            break;
        case Fmt_s: // String
            written += put_string(buf, sz, width, precision, (flags & Flag_Minus), va_arg(args, char *));
            break;
        case Fmt_p: { // Pointer
            const void *ptr = va_arg(args, void *);
            if (ptr == NULL) {
                written += put_string(buf, sz, width, precision, (flags & Flag_Minus), "(nil)");
            } else {
                written += int_to_str(buf, sz, width, precision, (flags | Flag_Hash), 16, false, (intmax_t)ptr);
            }
        } break;
        case Fmt_n: { // Return the number of characters written so far
            // TODO: Length modifiers are not implemented for %n
            int *ptr = va_arg(args, int *);
            *ptr = written;
        } break;
        case Fmt_percent:
            PUTCHAR('%');
            written++;
            break;
        case Fmt_unknown:
            PUTCHAR(*cursor);
            cursor++;
            written++;
            break;
        }
    }
    // null termination
    if ((sz == NULL) || (*sz > 0)) {
        **buf = '\0';
    }
    return written;
}

// Temporary buffer print function
char *tprintf(char *fmt, ...) {
    static THREAD_LOCAL char buffer[4096];
    va_list args;
    va_start(args, fmt);
    const int written = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    if (written >= 0) {
        return buffer;
    }
    return NULL;
}

//------------------------------------------------------------------------------
// CTYPE.H
//------------------------------------------------------------------------------

// Check if character is alphanumeric
int isalnum(int c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9');
}

// Check if character is alphabetic
int isalpha(int c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

// Check if character is blank
int isblank(int c)
{
    return c == ' ' || c == '\t';
}

// Check if character is a control character
int iscntrl(int c)
{
    // Control characters are those between ASCII codes 0x00 (NUL) and 0x1f (US), plus 0x7f (DEL).
    return (0x0 <= c && c <= 0x1f) || (c == 0x7f);
}

// Check if character is decimal digit
int isdigit(int c)
{
    return '0' <= c && c <= '9';
}

// Check if character has graphical representation
int isgraph(int c)
{
    // The characters with graphical representation are all those characters than can be printed
    // (as determined by isprint) except the space character (' ')
    return (c > 0x1f) && (c != 0x7f) && (c != ' ');
}

// Check if character is lowercase letter
int islower(int c)
{
    return ('a' <= c && c <= 'z');
}

// Check if character is printable
int isprint(int c)
{
    // Printing characters are all with an ASCII code greater than 0x1f (US), except 0x7f (DEL)
    return (c > 0x1f) && (c != 0x7f);
}

// Check if character is a punctuation character
int ispunct(int c)
{
    // Punctuation characters are all graphic characters (as in isgraph) that are not alphanumeric (as in isalnum).
    return isgraph(c) && !isalnum(c);
}

// Check if character is a white-space
int isspace(int c)
{
    return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\v') || (c == '\f') || (c == '\r');
}

// Check if character is uppercase letter
int isupper(int c)
{
    return ('A' <= c && c <= 'Z');
}

// Check if character is hexadecimal digit
int isxdigit(int c)
{
    return ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F') || ('0' <= c && c <= '9');
}

// Convert uppercase letter to lowercase
int tolower(int c)
{
    if ('A' <= c && c <= 'Z') {
        return (c+'a'-'A');
    }
    return c;
}

// Convert lowercase letter to uppercase
int toupper(int c)
{
    if ('a' <= c && c <= 'z') {
        return (c+'A'-'a');
    }
    return c;
}

//------------------------------------------------------------------------------
// STRING.H
//------------------------------------------------------------------------------

// Copy block of memory
void *memcpy(void *dst, const void *src, size_t sz)
{
    uint8_t *dst8 = (uint8_t *)dst;
    uint8_t *src8 = (uint8_t *)src;
    for (size_t i = 0; i < sz; i++) {
        dst8[i] = src8[i];
    }
    return dst;
}

// Move block of memory
void *memmove(void *dst, const void *src, size_t sz)
{
    uint8_t *dst8 = (uint8_t *)dst;
    uint8_t *src8 = (uint8_t *)src;
    if (src8 > dst8) {
        for (size_t i = 0; i < sz; i++) {
            dst8[i] = src8[i];
        }
    } else if (src8 < dst8) {
        for (size_t i = (sz-1); i < sz; i--) {
            dst8[i] = src8[i];
        }
    }
    return dst;
}

// Copy string
char *strcpy(char *dst, const char *src)
{
    size_t i = 0;
    for (; src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    // Add null-terminator to dst
    dst[i] = '\0';
    return dst;
}

// Copy characters from string
char *strncpy(char *dst, const char *src, size_t sz)
{
    size_t i = 0;
    for (; (src[i] != '\0') && (i < sz); i++) {
        dst[i] = src[i];
    }
    // If the end of the src is found before num characters have been copied,
    // destination is padded with zeros until a total of sz characters have
    // been written to it.
    for (; i < sz; i++) {
        dst[i] = '\0';
    }
    // No null-character is implicitly appended at the end of destination
    // if src is longer than sz
    return dst;
}

// Concatenate strings
char *strcat(char *dst, const char *src)
{
    size_t i = 0;
    // Find the end of dst
    while (dst[i] != '\0') {
        i++;
    }
    // Copy src to the end of dst
    for (size_t j = 0; src[j] != '\0'; i++, j++) {
        dst[i] = src[j];
    }
    // Add null-terminator to dst
    dst[i] = '\0';
    return dst;
}

// Append characters from string
char *strncat(char *dst, const char *src, size_t sz)
{
    size_t i = 0;
    // Find the end of dst
    while (dst[i] != '\0') {
        i++;
    }
    // Copy src to the end of dst
    for (size_t j = 0; (src[j] != '\0') && (j < sz); i++, j++) {
        dst[i] = src[j];
    }
    // Add null-terminator to dst
    dst[i] = '\0';
    return dst;
}

// Compare two blocks of memory
int memcmp(const void *a, const void *b, size_t sz)
{
    uint8_t *a8 = (uint8_t *)a;
    uint8_t *b8 = (uint8_t *)b;
    for (size_t i = 0; i < sz; i++) {
        int res = (int)(a8[i] - b8[i]);
        if (res != 0) {
            return res;
        }
    }
    return 0;
}

// Compare two strings
int strcmp(const char *a, const char *b)
{
    size_t i = 0;
    for (; (a[i] != '\0') && (b[i] != '\0'); i++) {
        int res = (int)(a[i] - b[i]);
        if (res != 0) {
            return res;
        }
    }
    return (int)(a[i] - b[i]);
}

// Compare characters of two strings
int strncmp(const char *a, const char *b, size_t sz)
{
    size_t i = 0;
    for (; (a[i] != '\0') && (b[i] != '\0') && (i < sz); i++) {
        int res = (int)(a[i] - b[i]);
        if (res != 0) {
            return res;
        }
    }
    if (i == sz) {
        return 0;
    }
    return (int)(a[i] - b[i]);
}

// Locate character in block of memory
void *memchr(const void *haystack, int needle, size_t sz)
{
    uint8_t *hay8 = (uint8_t *)haystack;
    uint8_t ndl8 = (uint8_t)needle;
    for (size_t i = 0; i < sz; i++) {
        if (hay8[i] == ndl8) {
            return &hay8[i];
        }
    }
    return NULL;
}

// Locate first occurrence of character in string
char *strchr(const char *str, int c)
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == (char)c) {
            return (char *)&str[i];
        }
    }
    return NULL;
}

// Get span until character in string
size_t strcspn(const char *str, const char *needles)
{
    size_t i = 0;
    for (; str[i] != '\0'; i++) {
        for (size_t j = 0; needles[j] != '\0'; j++) {
            if (str[i] == needles[j]) {
                return i;
            }
        }
    }
    return i;
}

// Locate characters in string
char *strpbrk(const char *str, const char *needles)
{
    size_t i = 0;
    for (; str[i] != '\0'; i++) {
        for (size_t j = 0; needles[j] != '\0'; j++) {
            if (str[i] == needles[j]) {
                return (char *)&str[i];
            }
        }
    }
    return NULL;
}

// Locate last occurrence of character in string
char *strrchr(const char *str, int c)
{
    // Find the end of the string
    size_t i = 0;
    while (str[i] != '\0') {
        i++;
    }
    // The terminating null-character is considered part of the C string.
    // Therefore, it can also be located to retrieve a pointer to the end of a string.
    for (; i != SIZE_MAX; i--) {
        if (str[i] == c) {
            return (char *)&str[i];
        }
    }
    return NULL;
}

// Get span of character set in string
size_t strspn(const char *str, const char *needles)
{
    size_t i = 0;
    for (; str[i] != '\0'; i++) {
        bool found_any = false;
        for (size_t j = 0; needles[j] != '\0'; j++) {
            if (str[i] == needles[j]) {
                found_any = true;
            }
        }
        if (!found_any) {
            return i;
        }
    }
    return i;
}

// Locate substring
char *strstr(const char *haystack, const char *needle)
{
    for (size_t i = 0; haystack[i] != '\0'; i++) {
        size_t j = 0;
        for (; needle[j] != '\0'; j++) {
            if (haystack[i+j] != needle[j]) {
                break;
            }
        }
        if (needle[j] == '\0') {
            return (char *)&haystack[i];
        }
    }
    return NULL;
}

// Split string into tokens
char *strtok(char *str, const char *delimiters)
{
    static THREAD_LOCAL char *old_str = NULL;
    return strtok_r(str, delimiters, &old_str);
}

// Fill block of memory
void *memset(void *ptr, int value, size_t sz)
{
    uint8_t *ptr8 = (uint8_t *)ptr;
    for (size_t i = 0; i < sz; i++) {
        ptr8[i] = (uint8_t)value;
    }
    return ptr;
}

// Get string length
size_t strlen(const char *str)
{
    size_t i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

// Reentrant version of the strtok function
char *strtok_r(char *str, const char *delimiters, char **saveptr)
{
    if (str == NULL) {
        // Use the saved pointer
        str = *saveptr;
    }
    if (str == NULL) {
        return NULL;
    }
    // Find the beginning of the token
    char *const tok_begin = str + strspn(str, delimiters);
    if (*tok_begin == '\0') {
        // Didn't found any token
        *saveptr = NULL;
        return NULL;
    }
    // Find the end of the token
    char *const tok_end = strpbrk(tok_begin, delimiters);
    if (tok_end != NULL) {
        *saveptr = *tok_end != '\0' ? (tok_end+1) : NULL;
        *tok_end = '\0';
    }
    return tok_begin;
}

// Function defined in POSIX-compliant librarys
char *strsep(char **str, const char *delimiters)
{
    if ((str == NULL) || (*str == NULL)) {
        return NULL;
    }
    char *const tok_begin = *str;
    // Find the end of the token
    char *const tok_end = strpbrk(tok_begin, delimiters);
    if (tok_end != NULL) {
        *str = *tok_end != '\0' ? (tok_end+1) : NULL;
        *tok_end = '\0';
    } else {
        *str = NULL;
    }
    return tok_begin;
}

//------------------------------------------------------------------------------
// STDLIB.H
//------------------------------------------------------------------------------

// Convert string to integer
int atoi(const char *str)
{
    return strtoi(str, NULL, 10);
}

// Convert string to long integer
long atol(const char *str)
{
    return strtol(str, NULL, 10);
}

// Convert string to long long integer
long long atoll(const char *str)
{
    return strtoll(str, NULL, 10);
}

// Convert string to integer
#define CREATE_STRTOI_FN(name, type, zero, max, min) \
    type name(const char *str, char **endptr, int base) \
    { \
        bool negative = false; \
        type value = zero; \
        if ((base < 0) || (base > 36)) { \
            return zero; \
        } \
        SKIP_WHITESPACES(str); \
        if ((*str == '+') || (*str == '-')) { \
            negative = *str == '-'; \
            str++; \
        } \
        if (*str == '0') { \
            if (*(str+1) == 'x') { \
                if ((base == 0) || (base == 16)) { \
                    base = 16; \
                    str += 2; \
                } \
            } else { \
                if ((base == 0) || (base == 8)) { \
                    base = 8; \
                    str++; \
                } \
            } \
        } \
        if (base == 0) { \
            base = 10; \
        } \
        for (; *str != '\0'; str++) { \
            int digit = 0; \
            if (isdigit(*str)) { \
                digit = *str - '0'; \
            } else if ((base > 10) && isalpha(*str)) { \
                digit = tolower(*str) - 'a' + 10; \
            } else { \
                break; \
            } \
            if (digit >= base) { \
                break; \
            } \
            if (negative) { \
                if (value >= (min) / (type)base) { \
                    value = (type)base * (value) - (type)digit; \
                } else { \
                    value = (min); \
                } \
            } else { \
                if (value <= (max) / (type)base) { \
                    value = (type)base * (value) + (type)digit; \
                } else { \
                    value = (max); \
                } \
            } \
        } \
        if (endptr != NULL) { \
            *endptr = (char *)str; \
        } \
        return value; \
    }

// Some sort of generics in C using the preprocessor
CREATE_STRTOI_FN(strtoi, int, 0, INT_MAX, INT_MIN)
CREATE_STRTOI_FN(strtol, long, 0L, LONG_MAX, LONG_MIN)
CREATE_STRTOI_FN(strtoll, long long, 0LL, LLONG_MAX, LLONG_MIN)
CREATE_STRTOI_FN(strtou, unsigned int, 0U, UINT_MAX, 0U)
CREATE_STRTOI_FN(strtoul, unsigned long, 0UL, ULONG_MAX, 0UL)
CREATE_STRTOI_FN(strtoull, unsigned long long, 0ULL, ULLONG_MAX, 0ULL)

// Convert string to double
double atof(const char *str)
{
    return strtod(str, NULL);
}

// Convert string to floating point number
#define CREATE_STRTOF_FN(name, type, zero, max) \
    type name(const char *str, char **endptr) \
    { \
        const int base = 10; \
        bool dotted = false; \
        int exponent = 0; \
        bool negative = false; \
        type value = zero; \
        SKIP_WHITESPACES(str); \
        if ((*str == '+') || (*str == '-')) { \
            negative = *str == '-'; \
            str++; \
        } \
        for (; *str != '\0'; str++) { \
            if (isdigit(*str)) { \
                const int digit = *str - '0'; \
                if (value <= max / (type)base) { \
                    value = value * (type)base + (type)digit; \
                } else { \
                    exponent++; \
                } \
                if (dotted) { \
                    exponent--; \
                } \
            } else if (*str == '.') { \
                if (dotted) { \
                    break; \
                } \
                dotted = true; \
            } else { \
                break; \
            } \
        } \
        if (tolower(*str) == 'e') { \
            char *endint = NULL; \
            const int exp = strtoi((str+1), &endint, 10); \
            if ((endint != NULL) && ((endint-str) > 1)) { \
                str = endint; \
                if (exponent > 0) { \
                    exponent = ((INT_MAX - exponent) < exp) ? INT_MAX : (exponent + exp); \
                } else { \
                    exponent = (exp < (INT_MIN - exponent)) ? INT_MIN : (exponent + exp); \
                } \
            } \
        } \
        if (endptr != NULL) { \
            *endptr = (char *)str; \
        } \
        if (value == zero) { \
            return value; \
        } \
        if (negative) { \
            value = -value; \
        } \
        if (exponent < 0) { \
            while (exponent++ != 0) { \
                value /= (type)base; \
            } \
        } else { \
            while (exponent-- != 0) { \
                if (value < -(type)max/(type)base) { \
                    return -HUGE_VAL; \
                } else if ((type)max/(type)base < value) { \
                    return HUGE_VAL; \
                } \
                value *= (type)base; \
            } \
        } \
        return value; \
    }

// Some sort of generics in C using the preprocessor
CREATE_STRTOF_FN(strtof, float, 0.0f, FLT_MAX)
CREATE_STRTOF_FN(strtod, double, 0.0, DBL_MAX)
CREATE_STRTOF_FN(strtold, long double, 0.0L, LDBL_MAX)

//------------------------------------------------------------------------------
// STDIO.H
//------------------------------------------------------------------------------

// Write formatted data to string
int sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const int written = __vsnprintf(&buf, NULL, fmt, args);
    va_end(args);
    return written;
}

// Write formatted output to sized buffer
int snprintf(char *buf, size_t sz, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const int written = __vsnprintf(&buf, &sz, fmt, args);
    va_end(args);
    return written;
}

// Write formatted data from variable argument list to string
int vsprintf(char *buf, const char *fmt, va_list args)
{
    return __vsnprintf(&buf, NULL, fmt, args);
}

// Write formatted data from variable argument list to sized buffer
int vsnprintf(char *buf, size_t sz, const char *fmt, va_list args)
{
    return __vsnprintf(&buf, &sz, fmt, args);
}

// Read formatted data from string
int sscanf(const char *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const int count = vsscanf(buf, fmt, args);
    va_end(args);
    return count;
}

// Helper macros used to simplify code in vsscanf
// Overflow in this situation is undefined behavior according to C-standard
#define SCANF_HANDLE_INT(base)                                                \
    do {                                                                      \
        intmax_t value;                                                       \
        SKIP_WHITESPACES(buf_cursor);                                         \
        const int parsed_chars = str_to_int(buf_cursor, width, base, &value); \
        if (parsed_chars == 0) {                                              \
            goto vsscanf_error;                                               \
        }                                                                     \
        if (!assignment_suppression) {                                        \
            void *ptr = va_arg(args, void *);                                 \
            switch (modifier) {                                               \
            case Modifier_char:                                               \
                *(char *)ptr = (char)value;                                   \
                break;                                                        \
            case Modifier_short:                                              \
                *(short *)ptr = (short)value;                                 \
                break;                                                        \
            case Modifier_long:                                               \
                *(long *)ptr = (long)value;                                   \
                break;                                                        \
            case Modifier_llong:                                              \
                *(long long *)ptr = (long long)value;                         \
                break;                                                        \
            case Modifier_None:                                               \
            case Modifier_ldouble:                                            \
            default:                                                          \
                *(int *)ptr = (int)value;                                     \
                break;                                                        \
            }                                                                 \
            count++;                                                          \
        }                                                                     \
        buf_cursor += parsed_chars;                                           \
    } while (0)

#define SCANF_HANDLE_UINT(base)                                               \
    do {                                                                      \
        intmax_t value;                                                       \
        SKIP_WHITESPACES(buf_cursor);                                         \
        const int parsed_chars = str_to_int(buf_cursor, width, base, &value); \
        if (parsed_chars == 0) {                                              \
            goto vsscanf_error;                                               \
        }                                                                     \
        if (!assignment_suppression) {                                        \
            void *ptr = va_arg(args, void *);                                 \
            switch (modifier) {                                               \
            case Modifier_char:                                               \
                *(unsigned char *)ptr = (unsigned char)value;                 \
                break;                                                        \
            case Modifier_short:                                              \
                *(unsigned short *)ptr = (unsigned short)value;               \
                break;                                                        \
            case Modifier_long:                                               \
                *(unsigned long *)ptr = (unsigned long)value;                 \
                break;                                                        \
            case Modifier_llong:                                              \
                *(unsigned long long *)ptr = (unsigned long long)value;       \
                break;                                                        \
            case Modifier_None:                                               \
            case Modifier_ldouble:                                            \
            default:                                                          \
                *(unsigned int *)ptr = (unsigned int)value;                   \
                break;                                                        \
            }                                                                 \
            count++;                                                          \
        }                                                                     \
        buf_cursor += parsed_chars;                                           \
    } while (0)

#define SCANF_HANDLE_PTR(base)                                                \
    do {                                                                      \
        intmax_t value;                                                       \
        SKIP_WHITESPACES(buf_cursor);                                         \
        const int parsed_chars = str_to_int(buf_cursor, width, base, &value); \
        if (parsed_chars == 0) {                                              \
            goto vsscanf_error;                                               \
        }                                                                     \
        if (!assignment_suppression) {                                        \
            void **ptr = va_arg(args, void *);                                \
            *ptr = (void *)value;                                             \
            count++;                                                          \
        }                                                                     \
        buf_cursor += parsed_chars;                                           \
    } while (0)

// TODO: long double is not currently supported
#define SCANF_HANDLE_FLOAT()                                               \
    do {                                                                   \
        double value;                                                      \
        SKIP_WHITESPACES(buf_cursor);                                      \
        const int parsed_chars = str_to_double(buf_cursor, width, &value); \
        if (parsed_chars == 0) {                                           \
            goto vsscanf_error;                                            \
        }                                                                  \
        if (!assignment_suppression) {                                     \
            void *ptr = va_arg(args, void *);                              \
            switch (modifier) {                                            \
            case Modifier_long:                                            \
                *(double *)ptr = (double)value;                            \
                break;                                                     \
            case Modifier_ldouble:                                         \
                *(long double *)ptr = (long double)value;                  \
                break;                                                     \
            case Modifier_char:                                            \
            case Modifier_short:                                           \
            case Modifier_llong:                                           \
            case Modifier_None:                                            \
            default:                                                       \
                *(float *)ptr = (float)value;                              \
                break;                                                     \
            }                                                              \
            count++;                                                       \
        }                                                                  \
        buf_cursor += parsed_chars;                                        \
    } while (0)

// Read formatted data from string into variable argument list
int vsscanf(const char *buf, const char *fmt, va_list args)
{
    enum Length_Modifier modifier;
    int count = 0;
    const char *cursor = fmt;
    const char *buf_cursor = buf;
    if (fmt == NULL) {
        return -1;
    }
    while (*cursor != '\0') {
        enum Fmt_Specifier specifier = Fmt_unknown;
        if (*cursor == '%') {
            cursor++;
            bool assignment_suppression = false;
            // sscanf format specifier follows this pattern:
            // %[*][width][length]specifier
            // https://cplusplus.com/reference/cstdio/scanf/
            if (*cursor == '*') {
                assignment_suppression = true;
                cursor++;
            }
            int width = -1;
            cursor += str_to_natural(cursor, &width);
            cursor += parse_fmt_specifier(cursor, &specifier, &modifier, NULL);
            switch (specifier) {
            case Fmt_d: // Signed integer in decimal base
                SCANF_HANDLE_INT(10);
                break;
            case Fmt_i: // Signed integer in decimal, hexadecimal or octal base
                SCANF_HANDLE_INT(0);
                break;
            case Fmt_u: // Unsigned integer
                SCANF_HANDLE_UINT(10);
                break;
            case Fmt_o: // Unsigned integer in octal form
                SCANF_HANDLE_UINT(8);
                break;
            case Fmt_x: // Unsigned integer in hexadecimal form
                SCANF_HANDLE_UINT(16);
                break;
            case Fmt_p: // Pointer
                SCANF_HANDLE_PTR(16);
                break;
            case Fmt_f: // Floating point
            case Fmt_e:
            case Fmt_g:
            case Fmt_a:
                SCANF_HANDLE_FLOAT();
                break;
            case Fmt_c: { // Character
                char *str = NULL;
                if (*buf_cursor == '\0') {
                    goto vsscanf_error;
                }
                if (!assignment_suppression) {
                    str = va_arg(args, char *);
                    count++;
                }
                if (width <= 0) {
                    width = 1; // Default width
                }
                while ((*buf_cursor != '\0') && (width-- > 0)) {
                    if (str != NULL) {
                        *str = *buf_cursor;
                        str++;
                    }
                    buf_cursor++;
                }
            } break;
            case Fmt_s: { // String
                SKIP_WHITESPACES(buf_cursor);
                char *str = NULL;
                if (*buf_cursor == '\0') {
                    goto vsscanf_error;
                }
                if (!assignment_suppression) {
                    str = va_arg(args, char *);
                    count++;
                }
                while ((*buf_cursor != '\0') && !isspace(*buf_cursor)) {
                    if (str != NULL) {
                        *str = *buf_cursor;
                        str++;
                    }
                    buf_cursor++;
                    if (width > 0) {
                        width--;
                        if (width == 0) {
                            break;
                        }
                    }
                }
                if (str != NULL) {
                    *str = '\0';
                }
            } break;
            case Fmt_n: { // Return the number of characters consumed so far
                // TODO: Assignment suppression and length modifiers are not implemented for %n
                int *consumed = va_arg(args, int *);
                *consumed = (int)(buf_cursor-buf);
            } break;
            case Fmt_percent:
                SKIP_WHITESPACES(buf_cursor);
                if (*buf_cursor != '%') {
                    goto vsscanf_error;
                }
                ADVANCE_CURSOR(buf_cursor);
                break;
            case Fmt_unknown:
                // Handle scansets
                if (*cursor == '['){
                    cursor++;
                    char *str = NULL;
                    if (!char_in_scanset(cursor, *buf_cursor)) {
                        goto vsscanf_error;
                    }
                    if (!assignment_suppression) {
                        str = va_arg(args, char *);
                        count++;
                    }
                    while (char_in_scanset(cursor, *buf_cursor)) {
                        if (str != NULL) {
                            *str = *buf_cursor;
                            str++;
                        }
                        buf_cursor++;
                        if (width > 0) {
                            width--;
                            if (width == 0) {
                                break;
                            }
                        }
                    }
                    if (str != NULL) {
                        *str = '\0';
                    }
                    // Skips the scanset
                    while (*cursor != ']') {
                        cursor++;
                    }
                }
                cursor++;
                break;
            }
        } else if (isspace(*cursor)) {
            SKIP_WHITESPACES(buf_cursor);
            cursor++;
        } else {
            if (*cursor != *buf_cursor) {
                goto vsscanf_end;
            }
            ADVANCE_CURSOR(buf_cursor);
            cursor++;
        }
    }
vsscanf_end:
    return count;
vsscanf_error:
    if ((count == 0) && (*buf_cursor == '\0')) {
        return -1;
    }
    return count;
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
