#include "strbuf.h"

#include <stdarg.h>
#include <stdlib.h>

vm_sea_strbuf_t vm_sea_strbuf_new(void)
{
    return (vm_sea_strbuf_t){
        .buf = NULL,
        .len = 0,
        .alloc = 0,
    };
}

void vm_sea_strbuf_del(vm_sea_strbuf_t buf)
{
    free(buf.buf);
}

void vm_sea_strbuf_putchar(vm_sea_strbuf_t *buf, char chr)
{
    if (buf->len + 4 >= buf->alloc)
    {
        buf->alloc = (buf->len + 1) * 2;
        buf->buf = realloc(buf->buf, sizeof(char) * buf->alloc);
    }
    buf->buf[buf->len++] = chr;
}

void vm_sea_strbuf_eof(vm_sea_strbuf_t *buf)
{
    vm_sea_strbuf_putchar(buf, '\0');
}

char *vm_sea_strbuf_to_string(vm_sea_strbuf_t *buf)
{
    vm_sea_strbuf_eof(buf);
    buf->alloc = 0;
    buf->len = 0;
    return buf->buf;
}

void vm_sea_strbuf_puts(vm_sea_strbuf_t *buf, const char *str)
{
    while (*str != '\0')
    {
        vm_sea_strbuf_putchar(buf, *str);
        str += 1;
    }
}

void vm_sea_strbuf_putnum(vm_sea_strbuf_t *buf, size_t num) 
{
    if (num >= 10)
    {
        vm_sea_strbuf_putnum(buf, num / 10);
    }
    vm_sea_strbuf_putchar(buf, num % 10 + '0');
}

void vm_sea_strbuf_printf(vm_sea_strbuf_t *buf, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    while (*fmt != '\0')
    {
        char chr = *fmt++;
        if (chr == '%')
        {
            char op = *fmt++;
            switch (op)
            {
            case '%':
                vm_sea_strbuf_putchar(buf, '%');
                break;
            case 's':
                vm_sea_strbuf_puts(buf, va_arg(va, const char *));
                break;
            case 'u':
                vm_sea_strbuf_putnum(buf, va_arg(va, size_t));
                break;
            }
        }
        else
        {
            vm_sea_strbuf_putchar(buf, chr);
        }
    }
    va_end(va);
}
