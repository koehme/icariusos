/**
 * @file string.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdarg.h>
#include <stdbool.h>

#include "string.h"
#include "status.h"
#include "vga.h"

extern VGADisplay vga_display;

bool is_alpha(const char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
};

size_t slen(const char *str)
{
    size_t i;

    for (i = 0; str[i] != '\0'; ++i)
    {
    };
    return i;
};

char *sptr(char *str, const char c)
{
    while (*str != '\0')
    {
        if (*str == c)
        {
            return str;
        };
        str++;
    }
    return 0x0;
};

bool scmp(const char *s1, const char *s2)
{
    size_t s1_len = slen(s1);
    size_t s2_len = slen(s2);

    if (s1_len != s2_len)
    {
        return false;
    };
    size_t i = 0;

    while (s1[i] != '\0' && s2[i] != '\0')
    {
        if (s1[i] == s2[i])
        {
            i++;
        }
        else
        {
            return false;
        };
    };
    return true;
};

void sreverse(char *str, const size_t length)
{
    size_t start = 0;
    size_t end = length - 1;

    while (start < end)
    {
        uint8_t buffer = str[start];
        str[start] = str[end];
        str[end] = buffer;
        start++;
        end--;
    };
    return;
};

/**
 * @brief Converts an integer to a string.
 * @param num The integer to be converted.
 * @param str The character array buffer to store the result.
 * @param base The base for the conversion (e.g., 10 for decimal conversion).
 * @return A pointer to the resulting string.
 */
char *itoa(uint32_t num, char *str, int32_t base)
{
    size_t i = 0;
    bool is_negative = false;

    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    };

    if (num < 0 && base == 10)
    {
        is_negative = true;
        num = -num;
    };
    // Extract digits from the number and add them to the string in reverse order
    while (num != 0)
    {
        // Extract in base 10 each digit for example 1024 % 10 = 4
        //                                              ^
        const int32_t rem = num % base;
        // Convert the remainder to the corresponding character and add it to the string buffer
        // If the remainder is greater than 9, use 'a' to 'f'
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        //                          ^                   ^
        //                 floats +  base 16           base 10
        // Get the next digit
        num = num / base;
    };

    if (is_negative)
    {
        str[i++] = '-';
    };
    str[i] = '\0';
    sreverse(str, i);
    return str;
};

int32_t kprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt != '\0')
    {
        if (*fmt == '%')
        {
            fmt++;

            switch (*fmt)
            {
            case 'c':
            {
                const char ch = va_arg(args, int32_t);
                vga_print_ch(&vga_display, ch, VGA_COLOR_LIGHT_GREEN);
                break;
            };
            case 's':
            {
                const char *str = va_arg(args, const char *);
                vga_print(&vga_display, str, VGA_COLOR_LIGHT_GREEN);
                break;
            };
            case 'd':
            {
                char buffer[1024] = {};
                const uint64_t num = va_arg(args, int32_t);
                itoa(num, buffer, 10);
                vga_print(&vga_display, buffer, VGA_COLOR_LIGHT_GREEN);
                break;
            };
            case 'x':
            {
                char buffer[1024] = {};
                const uint64_t value = va_arg(args, uint64_t);
                itoa(value, buffer, 16);
                vga_print(&vga_display, buffer, VGA_COLOR_LIGHT_GREEN);
                break;
            };
            default:
            {
                break;
            };
            };
        }
        else
        {
            vga_print_ch(&vga_display, *fmt, VGA_COLOR_LIGHT_GREEN);
        };
        fmt++;
    };
    va_end(args);
    return 0;
};

char *scat(char *dest, const char *src)
{
    while (*dest != '\0')
    {
        dest++;
    };

    while (*src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
    };
    *dest = '\0';
    return dest;
}