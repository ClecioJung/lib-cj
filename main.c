#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Get the name of a type */ 
#define typename(x)         _Generic((x),               \
    _Bool:                  "_Bool",                    \
    unsigned char:          "unsigned char",            \
    char:                   "char",                     \
    signed char:            "signed char",              \
    short int:              "short int",                \
    unsigned short int:     "unsigned short int",       \
    int:                    "int",                      \
    unsigned int:           "unsigned int",             \
    long int:               "long int",                 \
    unsigned long int:      "unsigned long int",        \
    long long int:          "long long int",            \
    unsigned long long int: "unsigned long long int",   \
    float:                  "float",                    \
    double:                 "double",                   \
    long double:            "long double",              \
    char *:                 "pointer to char",          \
    void *:                 "pointer to void",          \
    int *:                  "pointer to int",           \
    default:                "other")

// https://cplusplus.com/reference/cstdio/printf/
#define MY_IMPLEMENTATION

// TODO: Implement some kind of generics in C using preprocessor
// TODO: Try to make it fast and optimized
// TODO: We should use macro like CASE() to simplify parsing of codes in __snprintf
// TODO: Check what the standard proposes in case of invalid code, exemple %b or %j

#ifdef MY_IMPLEMENTATION

// Parses a integer number using base 8, 10 or 16
static int str_to_int(
    const char *fmt,
    const unsigned int base,
    long int *const number
)
{
    int index = 0;
    *number = 0;
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
        if (*number <= LONG_MAX / base) {
            *number = base * (*number) + digit;
        } else {
            // Overflow detected
            // In this case we return the maximum positive number that a long int can store
            // We continue the loop, in order to parse all the number
            *number = LONG_MAX;
        }
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
convert_int_to_str(
    char **buf,                 // Buffer in which we must write the output string
    size_t *const sz,           // Size of the buffer
    const unsigned int base,    // Integer base (8, 10 or 16)
    const bool uppercase,       // The hexadecimal characters should be in upper case?
    const bool sign,            // The integer value informed is signed?
    const bool must_add_sign,   // Always add sign to the string?
    const int digits,           // Quantity of digits that we must complete with zeros
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
    while (written < digits) {
        str[written++] = '0';
    }
    if (must_add_sign || negative) {
        str[written++] = negative ? '-' : '+';
    }
    for (int index = written; index > 0; index--) {
        PUTCHAR(str[index-1]);
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

static int                      // Return the amount of characters that would have been written if we had enough size
convert_float_to_str(
    char **buf,                 // Buffer in which we must write the output string
    size_t *const sz,           // Size of the buffer
    const unsigned int base,    // Integer base (10 or 16)
    const bool uppercase,       // The hexadecimal characters should be in upper case?
    const bool exponent_form,   // Use exponent form?
    const int decimal_places,   // Decimal places
    const double value          // Float value to be converted to string
)
{
    char str[32];
    const bool negative = value < 0.0;
    double x = negative ? (-value) : value;
    int written = 0;
    long int exponent = 0;
    if (exponent_form) {
        exponent = float_exponent_form(&x, base == 16 ? 2 : base);
    }
    for (int exp = decimal_places; exp > 0; exp--) {
        const double y = scale_radix_exp(x, base, exp);
        const char c = (char)div_remainder(y, base, exp == decimal_places);
        str[written++] = VALUE_TO_CHAR(c, uppercase);
    }
    str[written++] = '.';
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
        written += convert_int_to_str(buf, sz, 10, uppercase, true, true, ((base == 10) ? 2 : 1), exponent);
    }
    // This function doesn't need to introduce null termination to the buffer
    // This is responsability of its caller
    return written;
}

static int put_string(
    char **buf,
    size_t *const sz,
    const int max_chars,
    const bool fill_with_spaces_before, // Fill with spaces before the beginning of the string
    const bool fill_with_spaces_after, // Fill with spaces after the end of the string
    const char *string
)
{
    int written = 0;
    if (string == NULL) {
        string = "(null)";
    }
    if (fill_with_spaces_before) {
        int spaces = max_chars - (int)strlen(string);
        while (spaces > 0) {
            PUTCHAR(' ');
            written++;
            spaces--;
        }
    }
    while (*string != '\0' && (max_chars < 0 || (written < max_chars))) {
        PUTCHAR(*string);
        string++;
        written++;
    }
    if (fill_with_spaces_after) {
        int spaces = max_chars - written;
        while (spaces > 0) {
            PUTCHAR(' ');
            written++;
            spaces--;
        }
    }
    return written;
}

static int __snprintf(char **buf, size_t *const sz, const char *fmt, va_list args)
{
    int written = 0;
    const char *cursor = fmt;
    while (*cursor != '\0') {
        if (*cursor == '%') {
            cursor++; // Escape % character
            if (isdigit(*cursor)) {
                long int number;
                const int ret = str_to_int(cursor, 10, &number);
                if (*(cursor + ret) == 's') {
                    // String
                    written += put_string(buf, sz, (int)number, true, false, va_arg(args, char *));
                    cursor += ret + 1;
                }
                continue;
            }
            switch (*cursor) {
            case 'c':
                PUTCHAR((char)va_arg(args, int));
                cursor++;
                written++;
                break;
            case '%':
                PUTCHAR('%');
                cursor++;
                written++;
                break;
            case 'd':
            case 'i':
                // Signed integer
                written += convert_int_to_str(buf, sz, 10, false, true, false, 0, va_arg(args, int));
                cursor++;
                break;
            case 'u':
                // Unsigned integer
                written += convert_int_to_str(buf, sz, 10, false, false, false, 0, va_arg(args, unsigned int));
                cursor++;
                break;
            case 'h':
                if (*(cursor+1) == 'h') {
                    if (*(cursor+2) == 'd') {
                        // Signed char
                        written += convert_int_to_str(buf, sz, 10, false, true, false, 0, (char)va_arg(args, int));
                        cursor += 3;
                    } else if (*(cursor+2) == 'u') {
                        // Unsigned char
                        written += convert_int_to_str(buf, sz, 10, false, true, false, 0, (unsigned char)va_arg(args, unsigned int));
                        cursor += 3;
                    }
                } else if (*(cursor+1) == 'u') {
                    // Unsigned short integer
                    written += convert_int_to_str(buf, sz, 10, false, false, false, 0, (unsigned short)va_arg(args, unsigned int));
                    cursor += 2;
                } else {
                    // Short integer
                    written += convert_int_to_str(buf, sz, 10, false, true, false, 0, (short int)va_arg(args, int));
                    cursor += 2;
                }
                break;
            case 'l':
                if (*(cursor+1) == 'd') {
                    // Signed long
                    written += convert_int_to_str(buf, sz, 10, false, true, false, 0, va_arg(args, long int));
                    cursor += 2;
                } else if (*(cursor+1) == 'u') {
                    // Unsigned long
                    written += convert_int_to_str(buf, sz, 10, false, false, false, 0, (intmax_t)va_arg(args, unsigned long));
                    cursor += 2;
                }
                break;
            case 'o':
                // Unsigned octal
                written += convert_int_to_str(buf, sz, 8, false, false, false, 0, va_arg(args, unsigned int));
                cursor++;
                break;
            case 'x':
                // Unsigned hexadecimal
                written += convert_int_to_str(buf, sz, 16, false, false, false, 0, va_arg(args, unsigned int));
                cursor++;
                break;
            case 'X':
                // Unsigned hexadecimal
                written += convert_int_to_str(buf, sz, 16, true, false, false, 0, va_arg(args, unsigned int));
                cursor++;
                break;
            case '#':
                if (*(cursor+1) == 'o') {
                    // Unsigned octal
                    PUTCHAR('0');
                    written++;
                    written += convert_int_to_str(buf, sz, 8, false, false, false, 0, va_arg(args, unsigned int));
                    cursor += 2;
                } else if (*(cursor+1) == 'x') {
                    // Unsigned hexadecimal
                    PUTCHAR('0');
                    PUTCHAR('x');
                    written += 2;
                    written += convert_int_to_str(buf, sz, 16, false, false, false, 0, va_arg(args, unsigned int));
                    cursor += 2;
                } else if (*(cursor+1) == 'X') {
                    // Unsigned hexadecimal
                    PUTCHAR('0');
                    PUTCHAR('X');
                    written += 2;
                    written += convert_int_to_str(buf, sz, 16, true, false, false, 0, va_arg(args, unsigned int));
                    cursor += 2;
                }
                break;
            case 'p': {
                // Pointer
                void *ptr = va_arg(args, void *);
                if (ptr == NULL) {
                    written += put_string(buf, sz, -1, false, false, "(nil)");
                } else {
                    PUTCHAR('0');
                    PUTCHAR('x');
                    written += 2;
                    written += convert_int_to_str(buf, sz, 16, false, false, false, 0, (intmax_t)ptr);
                }
                cursor++;
            } break;
            case 'f':
            case 'F':
                // Decimal floating point
                written += convert_float_to_str(buf, sz, 10, false, false, 6, va_arg(args, double));
                cursor++;
                break;
            case 'e':
                // Decimal floating point
                written += convert_float_to_str(buf, sz, 10, false, true, 6, va_arg(args, double));
                cursor++;
                break;
            case 'E':
                // Decimal floating point
                written += convert_float_to_str(buf, sz, 10, true, true, 6, va_arg(args, double));
                cursor++;
                break;
            case 'a':
                // Hexadecimal floating point
                PUTCHAR('0');
                PUTCHAR('x');
                written += 2;
                written += convert_float_to_str(buf, sz, 16, false, true, 13, va_arg(args, double));
                cursor++;
                break;
            case 'A':
                // Hexadecimal floating point
                PUTCHAR('0');
                PUTCHAR('X');
                written += 2;
                written += convert_float_to_str(buf, sz, 16, true, true, 13, va_arg(args, double));
                cursor++;
                break;
            case '.':
                if ((*(cursor + 1) == '*') && (*(cursor + 2) == 's')) {
                    // String
                    const int number = va_arg(args, int);
                    written += put_string(buf, sz, (int)number, false, false, va_arg(args, char *));
                    cursor += 3;
                } else {
                    long int number;
                    const int ret = str_to_int(cursor+1, 10, &number);
                    if (ret > 0) {
                        if (*(cursor + ret + 1) == 'f') {
                            // Decimal floating point
                            written += convert_float_to_str(buf, sz, 10, false, false, (int)number, va_arg(args, double));
                            cursor += ret + 2;
                        } else if (*(cursor + ret + 1) == 'e') {
                            // Decimal floating point
                            written += convert_float_to_str(buf, sz, 10, false, true, (int)number, va_arg(args, double));
                            cursor += ret + 2;
                        } else if (*(cursor + ret + 1) == 'E') {
                            // Decimal floating point
                            written += convert_float_to_str(buf, sz, 10, true, true, (int)number, va_arg(args, double));
                            cursor += ret + 2;
                        } else if (*(cursor + ret + 1) == 'a') {
                            // Hexadecimal floating point
                            PUTCHAR('0');
                            PUTCHAR('x');
                            written += 2;
                            written += convert_float_to_str(buf, sz, 16, false, true, (int)number, va_arg(args, double));
                            cursor += ret + 2;
                        } else if (*(cursor + ret + 1) == 'A') {
                            // Hexadecimal floating point
                            PUTCHAR('0');
                            PUTCHAR('X');
                            written += 2;
                            written += convert_float_to_str(buf, sz, 16, true, true, (int)number, va_arg(args, double));
                            cursor += ret + 2;
                        } else if (*(cursor + ret + 1) == 's') {
                            // String
                            written += put_string(buf, sz, (int)number, false, false, va_arg(args, char *));
                            cursor += ret + 2;
                        }
                    }
                }
                break;
            case 's': 
                written += put_string(buf, sz, -1, false, false, va_arg(args, char *));
                cursor++;
                break;
            case '-': {
                long int number;
                const int ret = str_to_int(cursor+1, 10, &number);
                if (ret > 0) {
                    if (*(cursor + ret + 1) == 's') {
                        // String
                        written += put_string(buf, sz, (int)number, false, true, va_arg(args, char *));
                        cursor += ret + 2;
                    }
                }
            } break;
            case 'n': {
                // Return the number of characters written so far
                int *written_ptr = va_arg(args, int *);
                *written_ptr = written;
                cursor++;
            } break;
            default: // Unhandled format specifier
                break;
            }
        } else {
            PUTCHAR(*cursor);
            cursor++;
            written++;
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
#undef PUTCHAR

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
    // Unsigned decimal integer
    TEST_SNPRINTF("128 0 4294967295", "%u %u %u", 128, 0, UINT_MAX);
    // Signed char
    TEST_SNPRINTF("23 -82 -128 127", "%hhd %hhd %hhd %hhd", (char)23, (char)-82, CHAR_MIN, CHAR_MAX);
    // Unsigned char
    TEST_SNPRINTF("233 0 255", "%hhu %hhu %hhu", (unsigned char)233, (unsigned char)0, UCHAR_MAX);
    // Signed short integer
    TEST_SNPRINTF("15 -82 -32768 32767", "%hd %hd %hd %hd", (short)15, (short)-82, SHRT_MIN, SHRT_MAX);
    // Unsigned short integer
    TEST_SNPRINTF("128 0 65535", "%hu %hu %hu", (unsigned short)128, (unsigned short)0, USHRT_MAX);
    // Signed long integer
    TEST_SNPRINTF("100 -100 -9223372036854775808 9223372036854775807", "%ld %ld %ld %ld", (long)100, (long)-100, LONG_MIN, LONG_MAX);
    // Unsigned long integer
    TEST_SNPRINTF("128 0 18446744073709551615", "%lu %lu %lu", (unsigned long)128, (unsigned long)0, ULONG_MAX);
    // Character
    TEST_SNPRINTF("Characters: A c 7", "Characters: %c %c %c", 65, 'c', '7');
    // Percent character
    TEST_SNPRINTF("Percent character: %", "Percent character: %%");
    // Unsigned octal
    TEST_SNPRINTF("0200 0 037777777777", "%#o %o %#o", 128, 0, UINT_MAX);
    // Unsigned hexadecimal
    TEST_SNPRINTF("0xfa 0 ffffffff 4E 0XFFFFFFFF", "%#x %x %x %X %#X", 250, 0, UINT_MAX, 78, UINT_MAX);
    // Pointer address
    TEST_SNPRINTF("Pointer addresses: 0x456789ab 0x6789ab (nil)", "Pointer addresses: %p %p %p", (void*)0x456789AB, (void*)0x006789AB, NULL);
    // Decimal floating point
    TEST_SNPRINTF("392.567800 0.001000 0.10 0.001000", "%f %f %.2f %F", 392.5678, 1e-3, 0.1, 1e-3);
    // Scientific notation (mantissa/exponent)
    TEST_SNPRINTF("3.925678e+02 1.000000e-03 1.00e-01 1.000000E-03", "%e %e %.2e %E", 392.5678, 1e-3, 0.1, 1e-3);
    // Hexadecimal floating point
    TEST_SNPRINTF("0x1.88915b573eab3p+8 0x1.b7cdfd9d7bdbbp-34 0x1.9ap-4 0X1.B7CDFD9D7BDBBP-34", "%a %a %.2a %A", 392.5678, 1e-10, 0.1, 1e-10);
    // Use the shortest representation:
    TEST_SNPRINTF("392.568 1e-10 0.1 1E-10", "%g %g %.2g %G", 392.5678, 1e-10, 0.1, 1e-10);
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
    printf("Congrats! All tests passed!\n");
    return EXIT_SUCCESS;
}