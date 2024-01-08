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

/**
 * @brief Calculate the length of a null-terminated C string.
 * This function takes a pointer to a null-terminated C string and iterates through
 * each character until the null terminator '\0' is encountered. It then returns
 * the number of characters in the string, excluding the null terminator.
 * @param str A pointer to the null-terminated C string whose length is to be determined.
 * @return size_t The length of the C string, excluding the null terminator.
 * @note The function assumes that the input string is null-terminated. If the input
 * string is not null-terminated, the behavior is undefined.
 * @note This function has a time complexity of O(n), where n is the length of the input string.
 */
size_t slen(const char *str)
{
    size_t i;

    for (i = 0; str[i] != '\0'; ++i)
    {
    };
    return i;
};

char *itoa(int num, char *str, int base)
{
    int i = 0;
    bool is_neg = false;

    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    };

    if (num < 0 && base == 10)
    {
        is_neg = true;
        num = -num;
    };

    while (num != 0)
    {
        const int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    };

    if (is_neg)
    {
        str[i++] = '-';
    };
    str[i] = '\0';

    int start = 0;
    int end = i - 1;

    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    };
    return str;
};

int kprintf(const char *fmt, ...)
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
                const char ch = va_arg(args, int);
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
                const int num = va_arg(args, int);
                const char *str = itoa(num, buffer, 10);
                vga_print(&vga_display, str, VGA_COLOR_LIGHT_GREEN);
                break;
            };
            case 'x':
            {
                char buffer[3] = {0x0, 0x0, '\0'};
                const char value = va_arg(args, int);
                const char *hex_chars = "0123456789ABCDEF";
                const uint8_t upper_nibble = (value >> 4) & 0b00001111;
                const uint8_t lower_nibble = value & 0b00001111;
                buffer[0] = hex_chars[upper_nibble];
                buffer[1] = hex_chars[lower_nibble];
                vga_print(&vga_display, buffer, VGA_COLOR_LIGHT_GREEN);
            };
            default:
            {
                ksleep(KDEBUG_SLOW_DOWN);
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