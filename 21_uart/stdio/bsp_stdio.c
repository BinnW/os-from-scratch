#include "bsp_stdio.h"
#include "bsp_uart.h"
#include <stdarg.h>
#include <stdbool.h>

/**
 * _write - Write to file descriptor (redirected to UART)
 * @fd: file descriptor (1 = stdout, 2 = stderr)
 * @ptr: buffer containing data to write
 * @len: number of bytes to write
 * 
 * Returns: number of bytes written, or -1 on error
 */
int _write(int fd, char *ptr, size_t len)
{
    size_t i;
    
    /* Only handle stdout (fd=1) and stderr (fd=2) */
    if (fd != 1 && fd != 2) {
        return -1;
    }
    
    /* Write each character to UART */
    for (i = 0; i < len; i++) {
        uart_putchar(ptr[i]);
        
        /* Handle newline: add carriage return */
        if (ptr[i] == '\n') {
            uart_putchar('\r');
        }
    }
    
    return len;
}

/**
 * _read - Read from file descriptor (redirected to UART)
 * @fd: file descriptor (0 = stdin)
 * @ptr: buffer to store read data
 * @len: maximum number of bytes to read
 * 
 * Returns: number of bytes read, or -1 on error
 */
int _read(int fd, char *ptr, size_t len)
{
    size_t i;
    
    /* Only handle stdin (fd=0) */
    if (fd != 0) {
        return -1;
    }
    
    /* Read characters from UART until buffer is full or newline received */
    for (i = 0; i < len; i++) {
        char ch = uart_getchar();
        
        /* Echo the character */
        uart_putchar(ch);
        
        /* Handle carriage return: convert to newline */
        if (ch == '\r') {
            uart_putchar('\n');
            ptr[i] = '\n';
            return i + 1;
        }
        
        ptr[i] = ch;
        
        /* Return on newline */
        if (ch == '\n') {
            return i + 1;
        }
    }
    
    return len;
}

/* Forward declaration for simple strlen */
static size_t simple_strlen(const char *s);

/**
 * print_char - Print a single character
 */
static void print_char(char c)
{
    if (c == '\n') {
        uart_putchar('\r');
    }
    uart_putchar(c);
}

/**
 * print_string - Print a string
 */
static void print_string(const char *s)
{
    while (*s) {
        print_char(*s++);
    }
}

/**
 * print_number - Print a signed integer in decimal
 */
static void print_number(int num, bool negative)
{
    char buffer[16];
    char *p = buffer + sizeof(buffer) - 1;
    unsigned int u;
    
    if (negative) {
        u = (unsigned int)(-(num + 1)) + 1;
    } else {
        u = (unsigned int)num;
    }
    
    if (u == 0) {
        print_char('0');
        return;
    }
    
    *p = '\0';
    while (u > 0) {
        *--p = '0' + (u % 10);
        u /= 10;
    }
    
    if (negative) {
        print_char('-');
    }
    
    while (*p) {
        print_char(*p++);
    }
}

/**
 * print_hex - Print unsigned integer in hexadecimal
 */
static void print_hex(unsigned int num, bool uppercase)
{
    char buffer[16];
    char *p = buffer + sizeof(buffer) - 1;
    static const char hex_lc[] = "0123456789abcdef";
    static const char hex_uc[] = "0123456789ABCDEF";
    const char *hex = uppercase ? hex_uc : hex_lc;
    
    if (num == 0) {
        print_string("0x0");
        return;
    }
    
    *p = '\0';
    while (num > 0) {
        *--p = hex[num % 16];
        num /= 16;
    }
    
    print_string("0x");
    while (*p) {
        print_char(*p++);
    }
}

/**
 * printf - Minimal printf implementation
 * Supports: %d, %u, %x, %X, %c, %s, %%
 */
int printf(const char *format, ...)
{
    va_list args;
    const char *p;
    int count = 0;
    
    va_start(args, format);
    
    for (p = format; *p != '\0'; p++) {
        if (*p != '%') {
            print_char(*p);
            count++;
            continue;
        }
        
        p++;
        if (*p == '\0') break;
        
        switch (*p) {
            case '%':
                print_char('%');
                count++;
                break;
                
            case 'c': {
                char c = (char)va_arg(args, int);
                print_char(c);
                count++;
                break;
            }
            
            case 's': {
                const char *s = va_arg(args, const char *);
                if (s == NULL) {
                    print_string("(null)");
                } else {
                    print_string(s);
                    count += simple_strlen(s);
                }
                break;
            }
            
            case 'd':
            case 'i': {
                int num = va_arg(args, int);
                bool neg = (num < 0);
                print_number(num, neg);
                count++;
                break;
            }
            
            case 'u': {
                unsigned int num = va_arg(args, unsigned int);
                print_number((int)num, false);
                count++;
                break;
            }
            
            case 'x': {
                unsigned int num = va_arg(args, unsigned int);
                print_hex(num, false);
                count++;
                break;
            }
            
            case 'X': {
                unsigned int num = va_arg(args, unsigned int);
                print_hex(num, true);
                count++;
                break;
            }
            
            default:
                print_char(*p);
                count++;
                break;
        }
    }
    
    va_end(args);
    
    return count;
}

/**
 * parse_number - Parse a number from a string
 * Returns: number of characters consumed
 */
static int parse_number(const char *s, int *result)
{
    int value = 0;
    int count = 0;
    bool negative = false;
    
    if (*s == '-') {
        negative = true;
        s++;
        count++;
    }
    
    if (*s < '0' || *s > '9') {
        return 0;
    }
    
    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        s++;
        count++;
    }
    
    *result = negative ? -value : value;
    return count;
}

/**
 * scanf - Minimal scanf implementation
 * Supports: %d, %u, %x, %c, %s
 */
int scanf(const char *format, ...)
{
    va_list args;
    const char *p;
    int count = 0;
    char input_buffer[64];
    int input_len = 0;
    char ch;
    
    /* Read a line of input */
    while (1) {
        ch = uart_getchar();
        uart_putchar(ch);  /* Echo */
        
        if (ch == '\r' || ch == '\n') {
            uart_putchar('\n');
            break;
        }
        
        if (input_len < (int)(sizeof(input_buffer) - 1)) {
            input_buffer[input_len++] = ch;
        }
    }
    input_buffer[input_len] = '\0';
    
    /* Parse the input */
    va_start(args, format);
    
    const char *input_p = input_buffer;
    
    for (p = format; *p != '\0' && *input_p != '\0'; p++) {
        if (*p == '%') {
            p++;
            
            switch (*p) {
                case 'd':
                case 'i': {
                    int *num_ptr = va_arg(args, int *);
                    int parsed = 0;
                    int consumed = parse_number(input_p, &parsed);
                    if (consumed > 0) {
                        *num_ptr = parsed;
                        count++;
                        input_p += consumed;
                    }
                    break;
                }
                
                case 'u': {
                    unsigned int *num_ptr = va_arg(args, unsigned int *);
                    int parsed = 0;
                    int consumed = parse_number(input_p, &parsed);
                    if (consumed > 0) {
                        *num_ptr = (unsigned int)parsed;
                        count++;
                        input_p += consumed;
                    }
                    break;
                }
                
                case 'x': {
                    unsigned int *num_ptr = va_arg(args, unsigned int *);
                    /* Simple hex parsing */
                    unsigned int value = 0;
                    while ((*input_p >= '0' && *input_p <= '9') ||
                           (*input_p >= 'a' && *input_p <= 'f') ||
                           (*input_p >= 'A' && *input_p <= 'F')) {
                        char c = *input_p;
                        unsigned int digit;
                        if (c >= '0' && c <= '9') digit = c - '0';
                        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
                        else digit = c - 'A' + 10;
                        value = value * 16 + digit;
                        input_p++;
                    }
                    *num_ptr = value;
                    if (value != 0) count++;
                    break;
                }
                
                case 'c': {
                    char *char_ptr = va_arg(args, char *);
                    if (*input_p != '\0') {
                        *char_ptr = *input_p++;
                        count++;
                    }
                    break;
                }
                
                case 's': {
                    char *str_ptr = va_arg(args, char *);
                    while (*input_p != '\0' && *input_p != ' ' && 
                           *input_p != '\n' && *input_p != '\r') {
                        *str_ptr++ = *input_p++;
                    }
                    *str_ptr = '\0';
                    if (str_ptr != va_arg(args, char *)) {
                        count++;
                    }
                    break;
                }
                
                default:
                    break;
            }
        } else if (*p == *input_p) {
            input_p++;
        } else {
            /* Skip whitespace in format */
            while (*p == ' ' || *p == '\t' || *p == '\n') {
                p++;
            }
            if (*p == *input_p) {
                input_p++;
            }
        }
    }
    
    va_end(args);
    
    return count;
}

/**
 * simple_strlen - Simple string length function
 */
static size_t simple_strlen(const char *s)
{
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}
