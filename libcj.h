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

#ifndef __LIB_CJ
#define __LIB_CJ

#include <stdarg.h>
#include <stddef.h>

#ifndef __GNUC__
#define __attribute__(a)
#endif

int isalnum(int c);
int isalpha(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);

void *memcpy(void *dst, const void *src, size_t sz);
void *memmove(void *dst, const void *src, size_t sz);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t sz);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t sz);
int memcmp(const void *a, const void *b, size_t sz);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t sz);
void *memchr(const void *haystack, int needle, size_t sz);
char *strchr(const char *str, int c);
size_t strcspn(const char *str, const char *needles);
char *strpbrk(const char *str, const char *needles);
char *strrchr(const char *str, int c);
char *strstr(const char *haystack, const char *needle);
char *strstr(const char *haystack, const char *needle);
size_t strspn(const char *str, const char *needles);
char *strtok(char *str, const char *delimiters);
void *memset(void *ptr, int value, size_t sz);
size_t strlen(const char *str);

int atoi(const char *str);
long atol(const char *str);
long long atoll(const char *str);
int strtoi(const char *str, char **endptr, int base); // This function isn't defined by standard-C
long strtol(const char *str, char **endptr, int base);
long long strtoll(const char *str, char **endptr, int base);
unsigned int strtou(const char *str, char **endptr, int base); // This function isn't defined by standard-C
unsigned long strtoul(const char *str, char **endptr, int base);
unsigned long long strtoull(const char *str, char **endptr, int base);
double atof(const char *str);
float strtof(const char *str, char **endptr);
double strtod(const char *str, char **endptr);
long double strtold(const char *str, char **endptr);

int sprintf(char *buf, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
int snprintf(char *buf, size_t sz, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));
int vsprintf(char *buf, const char *fmt, va_list args);
int vsnprintf(char *buf, size_t sz, const char *fmt, va_list args);

// Temporary buffer print function
char *tprint(char *fmt, ...)
    __attribute__((format(printf, 1, 2)));

#endif  // __LIB_CJ

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
