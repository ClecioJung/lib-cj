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
#define LOCAL_BUFFER_SIZE 32 // TODO: Check for boundaries

#define VALUE_TO_CHAR(value, uppercase) \
    (((value) < 10) ? ((value) + '0') : (value - 10 + ((uppercase) ? 'A' : 'a')))

// Helper macro used to simplify code in vsscanf
// Overflow in this situation is undefined behavior according to C-standard
#define SCANF_HANDLE_NUMBER(type, str_to_type_fn, base, save_arg_fn) \
    do {                                                             \
        type value;                                                  \
        parsed_chars = str_to_type_fn(buf_cursor, base, &value);     \
        if (parsed_chars > 0) {                                      \
            if (!assignment_suppression) {                           \
                save_arg_fn(args, modifier, value);                  \
                count++;                                             \
            }                                                        \
            buf_cursor += parsed_chars;                              \
        }                                                            \
    } while (0)

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
#define ABS(x)    (((x) >= 0) ? (x) : -(x))

//------------------------------------------------------------------------------
// CUSTOM TYPES
//------------------------------------------------------------------------------

// The length modifiers j, z and t are not implemented
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

// Parses a string into a natural number (integer greather than zero)
// Overflows aren't handled
// If no integer was parsed, the value is kept at its original value
// It returns the amount of characters consumed
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
// It returns the amount of characters consumed
LIBCJ_FN int str_to_sign_integer(const char *const str, int *const value)
{
    const int base = 10;
    int index = 0;
    bool negative = false;
    *value = 0;
    if ((str[index] == '+') || (str[index] == '-')) {
        negative = str[index] == '-';
        index++;
    }
    for (; str[index] != '\0'; index++) {
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

// This macro defines a function that parses a sign integer
// using bases from 0 to 36
// Leading spaces are ignored
// It returns the amount of characters consumed
#define CREATE_STR_TO_INT_FN(name, type, zero, max, min) \
    LIBCJ_FN int name( \
        const char *const str, \
        int base, \
        type *const value) \
    { \
        int index = 0; \
        bool negative = false; \
        *value = zero; \
        if ((base < 0) || (base > 36)) { \
            return 0; \
        } \
        while (isspace(str[index])) { \
            index++; \
        } \
        if ((str[index] == '+') || (str[index] == '-')) { \
            negative = str[index] == '-'; \
            index++; \
        } \
        if (str[index] == '0') { \
            if (str[index+1] == 'x') { \
                if ((base == 0) || (base == 16)) { \
                    base = 16; \
                    index += 2; \
                } \
            } else { \
                if ((base == 0) || (base == 8)) { \
                    base = 8; \
                    index++; \
                } \
            } \
        } \
        if (base == 0) { \
            base = 10; \
        } \
        for (; str[index] != '\0'; index++) { \
            const char c = str[index]; \
            int digit = 0; \
            if (isdigit(c)) { \
                digit = c - '0'; \
            } else if ((base > 10) && isalpha(c)) { \
                digit = tolower(c) - 'a' + 10; \
            } else { \
                break; \
            } \
            if (digit >= base) { \
                break; \
            } \
            if (negative) { \
                if (*value >= (min) / (type)base) { \
                    *value = (type)base * (*value) - (type)digit; \
                } else { \
                    *value = (min); \
                } \
            } else { \
                if (*value <= (max) / (type)base) { \
                    *value = (type)base * (*value) + (type)digit; \
                } else { \
                    *value = (max); \
                } \
            } \
        } \
        return index; \
    }

// Some sort of generics in C using the preprocessor
CREATE_STR_TO_INT_FN(str_to_int, int, 0, INT_MAX, INT_MIN)
CREATE_STR_TO_INT_FN(str_to_long, long, 0L, LONG_MAX, LONG_MIN)
CREATE_STR_TO_INT_FN(str_to_llong, long long, 0LL, LLONG_MAX, LLONG_MIN)
CREATE_STR_TO_INT_FN(str_to_intmax, intmax_t, (intmax_t)0, INTMAX_MAX, INTMAX_MIN)
CREATE_STR_TO_INT_FN(str_to_uint, unsigned int, (unsigned int)0, UINT_MAX, (unsigned int)0)
CREATE_STR_TO_INT_FN(str_to_ulong, unsigned long, 0UL, ULONG_MAX, 0UL)
CREATE_STR_TO_INT_FN(str_to_ullong, unsigned long long, 0ULL, ULLONG_MAX, 0ULL)
CREATE_STR_TO_INT_FN(str_to_ptr, uintptr_t, (uintptr_t)0, UINTPTR_MAX, (uintptr_t)0)
CREATE_STR_TO_INT_FN(str_to_uintmax, uintmax_t, (uintmax_t)0, UINTMAX_MAX, (uintmax_t)0)

// Generic function to convert integer numbers to string
// Return the amount of characters that would have been written if we had enough size
static int int_to_str(char **buf, size_t *const sz,
    const int width, const int precision, const enum Fmt_Flags flags, const int base,
    const bool sign, const intmax_t value)
{
    char str[LOCAL_BUFFER_SIZE]; 
    const bool left_justify = flags & Flag_Minus;
    const bool negative = sign && (value < 0);
    const bool uppercase = (flags & Flag_Upper) != 0;
    // Pointers are the only unsigned integer that can have the plus flag
    const bool include_sign = negative || ((flags & Flag_Plus) != 0);
    const int base_padding = ((flags & Flag_Hash) && (base != 10) && (value != 0)) ? (base / 8) : 0;
    // Padding using for sign and base specifiers
    const int left_padding = (int)(include_sign ? (base_padding+1) : base_padding);
    const bool use_precision = (precision >= 0) || (flags & Flag_Minus);
    // Flag_Zero is ignored if precision or Flag_Minus are informed
    const int zeros = use_precision ? precision : ((flags & Flag_Zero) ? (width-left_padding) : 0);
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
#define CREATE_SCALE_RADIX_EXP_FN(name, type, zero, max) \
    static type name(type x, const int radix, int exponent) { \
        if (x == zero) { \
            return x; \
        } \
        if (exponent < 0) { \
            while (exponent++ != 0) { \
                x /= (type)radix; \
            } \
        } else { \
            while (exponent-- != 0) { \
                if (x < -(type)max/(type)radix) { \
                    return -HUGE_VAL; \
                } else if ((type)max/(type)radix < x) { \
                    return HUGE_VAL; \
                } \
                x *= (type)radix; \
            } \
        } \
        return x; \
    }

CREATE_SCALE_RADIX_EXP_FN(scale_radix_exp_float, float, 0.0f, FLT_MAX)
CREATE_SCALE_RADIX_EXP_FN(scale_radix_exp_double, double, 0.0, DBL_MAX)
CREATE_SCALE_RADIX_EXP_FN(scale_radix_exp_ldouble, long double, 0.0L, LDBL_MAX)

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
            *value = scale_radix_exp_double(*value, base, -1);
            exponent++;
        }
    } else {
        while ((int)(*value) == 0) {
            *value = scale_radix_exp_double(*value, base, 1);
            exponent--;
        }
    }
    return exponent;
}

LIBCJ_FN double rouding_double(const double value, const int base, const int decimal_places)
{
    const double y = scale_radix_exp_double(value, base, decimal_places);
    const double remainder = y - (long)y;
    if (remainder >= 0.5) {
        const double value_to_add = scale_radix_exp_double(1.0, base, -decimal_places);
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
    const int left_padding = (int)(include_sign ? (base_padding+1) : base_padding);
    // Flag_Zero is ignored if Flag_Minus is informed
    const int zeros = (!(flags & Flag_Minus) && (flags & Flag_Zero)) ? (width-left_padding) : 0;
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
        const double y = scale_radix_exp_double(value, base, exp);
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
        value = scale_radix_exp_double(value, base, -1);
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

// It returns the amount of characters consumed
#define CREATE_STR_TO_FLOAT_FN(name, type, zero, max, fn) \
    static int name(const char *const str, const bool ignore_whitespaces, type *const value) { \
        const int base = 10; \
        bool dotted = false; \
        int exponent = 0; \
        int index = 0; \
        bool negative = false; \
        *value = zero; \
        if (ignore_whitespaces) { \
            while (isspace(str[index])) { \
                index++; \
            } \
        } \
        if ((str[index] == '+') || (str[index] == '-')) { \
            negative = str[index] == '-'; \
            index++; \
        } \
        for (; str[index] != '\0'; index++) { \
            if (isdigit(str[index])) { \
                const int digit = str[index] - '0'; \
                if (*value <= max / (type)base) { \
                    *value = *value * (type)base + (type)digit; \
                } else { \
                    exponent++; \
                } \
                if (dotted) { \
                    exponent--; \
                } \
            } else if (str[index] == '.') { \
                if (dotted) { \
                    break; \
                } \
                dotted = true; \
            } else { \
                break; \
            } \
        } \
        if (tolower(str[index]) == 'e') { \
            int exp; \
            const int parsed = str_to_sign_integer(&str[index+1], &exp); \
            if (parsed > 0) { \
                index += parsed + 1; \
                if (exponent > 0) { \
                    exponent = ((INT_MAX - exponent) < exp) ? INT_MAX : (exponent + exp); \
                } else { \
                    exponent = (exp < (INT_MIN - exponent)) ? INT_MIN : (exponent + exp); \
                } \
            } \
        } \
        *value = fn(negative ? -(*value) : *value, base, exponent); \
        return index; \
    }

CREATE_STR_TO_FLOAT_FN(str_to_float, float, 0.0f, FLT_MAX, scale_radix_exp_float)
CREATE_STR_TO_FLOAT_FN(str_to_double, double, 0.0, DBL_MAX, scale_radix_exp_double)
CREATE_STR_TO_FLOAT_FN(str_to_ldouble, long double, 0.0L, LDBL_MAX, scale_radix_exp_ldouble)

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

LIBCJ_FN void save_int_arg(va_list args, enum Length_Modifier modifier, intmax_t value)
{
    void *ptr = va_arg(args, void *);
    switch (modifier) {
    case Modifier_char:
        *(char *)ptr = (char)value;
        break;
    case Modifier_short:
        *(short *)ptr = (short)value;
        break;
    case Modifier_long:
        *(long *)ptr = (long)value;
        break;
    case Modifier_llong:
        *(long long *)ptr = (long long)value;
        break;
    case Modifier_None:
    case Modifier_ldouble:
    default:
        *(int *)ptr = (int)value;
        break;
    }
}

LIBCJ_FN void save_uint_arg(va_list args, enum Length_Modifier modifier, uintmax_t value)
{
    void *ptr = va_arg(args, void *);
    switch (modifier) {
    case Modifier_char:
        *(unsigned char *)ptr = (unsigned char)value;
        break;
    case Modifier_short:
        *(unsigned short *)ptr = (unsigned short)value;
        break;
    case Modifier_long:
        *(unsigned long *)ptr = (unsigned long)value;
        break;
    case Modifier_llong:
        *(unsigned long long *)ptr = (unsigned long long)value;
        break;
    case Modifier_None:
    case Modifier_ldouble:
    default:
        *(unsigned int *)ptr = (unsigned int)value;
        break;
    }
}

LIBCJ_FN void save_ptr_arg(va_list args, enum Length_Modifier modifier, uintptr_t value)
{
    (void)modifier;
    void **ptr = va_arg(args, void *);
    *ptr = (void *)value;
}

LIBCJ_FN void save_float_arg(va_list args, enum Length_Modifier modifier, double value)
{
    // TODO: long double is not currently supported
    void *ptr = va_arg(args, void *);
    switch (modifier) {
    case Modifier_long:
        *(double *)ptr = (double)value;
        break;
    case Modifier_ldouble:
        *(long double *)ptr = (long double)value;
        break;
    case Modifier_char:
    case Modifier_short:
    case Modifier_llong:
    case Modifier_None:
    default:
        *(float *)ptr = (float)value;
        break;
    }
}

LIBCJ_FN intmax_t next_int_arg(va_list args, enum Length_Modifier modifier)
{
    switch (modifier) {
    case Modifier_char:  return (char)va_arg(args, int);
    case Modifier_short: return (short)va_arg(args, int);
    case Modifier_long:  return va_arg(args, long);
    case Modifier_llong: return va_arg(args, long long);
    case Modifier_None:
    case Modifier_ldouble:
    default: return va_arg(args, int);
    }
}

LIBCJ_FN intmax_t next_uint_arg(va_list args, enum Length_Modifier modifier)
{
    switch (modifier) {
    case Modifier_char:  return (unsigned char)va_arg(args, unsigned int);
    case Modifier_short: return (unsigned short)va_arg(args, unsigned int);
    case Modifier_long:  return (intmax_t)va_arg(args, unsigned long);
    case Modifier_llong: return (intmax_t)va_arg(args, unsigned long long);
    case Modifier_None:
    case Modifier_ldouble:
    default: return va_arg(args, unsigned int);
    }
}

LIBCJ_FN double next_float_arg(va_list args, enum Length_Modifier modifier)
{
    // TODO: long double is not currently supported
    switch (modifier) {
    case Modifier_ldouble: return (double)va_arg(args, long double);
    case Modifier_None:
    case Modifier_char:
    case Modifier_short:
    case Modifier_long:
    case Modifier_llong:
    default: return va_arg(args, double);
    }
}

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
            written += int_to_str(buf, sz, width, precision, flags, 10, true, next_int_arg(args, modifier));
            break;
        case Fmt_u: // Unsigned integer
            flags = flags & (enum Fmt_Flags)~Flag_Plus; // This flag isn't supported for unsigned numbers
            written += int_to_str(buf, sz, width, precision, flags, 10, false, next_uint_arg(args, modifier));
            break;
        case Fmt_o: // Unsigned integer in octal form
            flags = flags & (enum Fmt_Flags)~Flag_Plus; // This flag isn't supported for octal numbers
            written += int_to_str(buf, sz, width, precision, flags, 8, false, next_uint_arg(args, modifier));
            break;
        case Fmt_x: // Unsigned integer in hexadecimal form
            flags = flags & (enum Fmt_Flags)~Flag_Plus; // This flag isn't supported for hexadecimal numbers
            written += int_to_str(buf, sz, width, precision, flags, 16, false, next_uint_arg(args, modifier));
            break;
        case Fmt_f: // Decimal floating point
            written += double_to_str(buf, sz, width, precision, flags, 10, next_float_arg(args, modifier));
            break;
        case Fmt_e: // Decimal floating point in exponent form
            written += double_to_str(buf, sz, width, precision, (flags | Flag_Exp), 10, next_float_arg(args, modifier));
            break;
        case Fmt_g: // Decimal floating point in shortest form
            written += double_to_str(buf, sz, width, precision, (flags | Flag_Short), 10, next_float_arg(args, modifier));
            break;
        case Fmt_a: // Decimal floating point in hexadecimal form
            written += double_to_str(buf, sz, width, precision, (flags | Flag_Exp), 16, next_float_arg(args, modifier));
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
            // Length modifiers are not implemented for %n
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
char *tprint(char *fmt, ...) {
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
#define CREATE_ATOI_FN(name, type, fn) \
    type name(const char *str) \
    { \
        type value; \
        fn(str, 10, &value); \
        return value; \
    }

CREATE_ATOI_FN(atoi, int, str_to_int)
CREATE_ATOI_FN(atol, long, str_to_long)
CREATE_ATOI_FN(atoll, long long, str_to_llong)

// Convert string to integer
#define CREATE_STRTOI_FN(name, type, fn) \
    type name(const char *str, char **endptr, int base) \
    { \
        type value; \
        const int parsed = fn(str, base, &value); \
        if (endptr != NULL) { \
            *endptr = (char *)str + parsed; \
        } \
        return value; \
    }

CREATE_STRTOI_FN(strtoi, int, str_to_int)
CREATE_STRTOI_FN(strtol, long, str_to_long)
CREATE_STRTOI_FN(strtoll, long long, str_to_llong)
CREATE_STRTOI_FN(strtou, unsigned int, str_to_uint)
CREATE_STRTOI_FN(strtoul, unsigned long, str_to_ulong)
CREATE_STRTOI_FN(strtoull, unsigned long long, str_to_ullong)

// Convert string to double
double atof(const char *str)
{
    double value;
    str_to_double(str, true, &value);
    return value;
}

// Convert string to floating point number
#define CREATE_STRTOF_FN(name, type, fn) \
    type name(const char *str, char **endptr) \
    { \
        type value; \
        const int parsed = fn(str, true, &value); \
        if (endptr != NULL) { \
            *endptr = (char *)str + parsed; \
        } \
        return value; \
    }

CREATE_STRTOF_FN(strtof, float, str_to_float)
CREATE_STRTOF_FN(strtod, double, str_to_double)
CREATE_STRTOF_FN(strtold, long double, str_to_ldouble)

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
            bool assignment_suppression = false;
            int width = -1; //  TODO: handle maximum width
            int parsed_chars = 1; // Start with one character parsed ('%')
            // sscanf format specifier follows this pattern:
            // %[*][width][length]specifier
            // https://cplusplus.com/reference/cstdio/scanf/
            if (*(cursor+parsed_chars) == '*') {
                assignment_suppression = true;
                parsed_chars++;
            }
            parsed_chars += str_to_natural((cursor+parsed_chars), &width);
            parsed_chars += parse_fmt_specifier((cursor+parsed_chars), &specifier, &modifier, NULL);
            // If the format specifier was fully parsed, update the cursor position and args
            if (specifier != Fmt_unknown) {
                cursor += parsed_chars;
            }
            switch (specifier) {
            case Fmt_d: // Signed integer in decimal base
                SCANF_HANDLE_NUMBER(intmax_t, str_to_intmax, 10, save_int_arg);
                break;
            case Fmt_i: // Signed integer in decimal, hexadecimal or octal base
                SCANF_HANDLE_NUMBER(intmax_t, str_to_intmax, 0, save_int_arg);
                break;
            case Fmt_u: // Unsigned integer
                SCANF_HANDLE_NUMBER(uintmax_t, str_to_uintmax, 10, save_uint_arg);
                break;
            case Fmt_o: // Unsigned integer in octal form
                SCANF_HANDLE_NUMBER(uintmax_t, str_to_uintmax, 8, save_uint_arg);
                break;
            case Fmt_x: // Unsigned integer in hexadecimal form
                SCANF_HANDLE_NUMBER(uintmax_t, str_to_uintmax, 16, save_uint_arg);
                break;
            case Fmt_f: // Floating point
            case Fmt_e:
            case Fmt_g:
            case Fmt_a: {
                double value;
                parsed_chars = str_to_double(buf_cursor, true, &value);
                if (parsed_chars > 0) {
                    if (!assignment_suppression) {
                        save_float_arg(args, modifier, value);
                        count++;
                    }
                    buf_cursor += parsed_chars;
                }
            } break;
            case Fmt_c: { // Character
                if (!assignment_suppression) {
                    char *str = va_arg(args, char *);
                    *str = *buf_cursor;
                    count++;
                }
                ADVANCE_CURSOR(buf_cursor);
            } break;
            case Fmt_s: { // String
                SKIP_WHITESPACES(buf_cursor);
                char *str = NULL;
                if (!assignment_suppression) {
                    str = va_arg(args, char *);
                }
                while ((*buf_cursor != '\0') && !isspace(*buf_cursor)) {
                    if (!assignment_suppression) {
                        *str = *buf_cursor;
                        str++;
                    }
                    ADVANCE_CURSOR(buf_cursor);
                }
                if (!assignment_suppression) {
                    *str = '\0';
                    count++;
                }
            } break;
            case Fmt_p: // Pointer
                SCANF_HANDLE_NUMBER(uintptr_t, str_to_ptr, 16, save_ptr_arg);
                break;
            case Fmt_n: { // Return the number of characters consumed so far
                // Assignment suppression and length modifiers are not implemented for %n
                int *consumed = va_arg(args, int *);
                *consumed = (int)(buf_cursor-buf);
            } break;
            // TODO: Implement scansets. Example: %[abc], %[^0-9]
            case Fmt_percent:
                SKIP_WHITESPACES(buf_cursor);
                if (*buf_cursor != '%') {
                    goto vsscanf_end;
                }
                ADVANCE_CURSOR(buf_cursor);
                break;
            case Fmt_unknown:
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
