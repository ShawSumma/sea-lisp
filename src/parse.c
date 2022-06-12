#include "parse.h"
#include <stdarg.h>
#include <stdlib.h>

size_t vm_sea_parse_strip(const char **psrc)
{
    size_t ret = 0;
    for (;;)
    {
        if (**psrc == ' ')
        {
            ret += 1;
            *psrc += 1;
            continue;
        }
        if (**psrc == '\n')
        {
            ret = 0;
            *psrc += 1;
            continue;
        }
        return ret;
    }
}

vm_sea_ast_t vm_sea_parse(const char *src)
{
    vm_sea_ast_t indents[256];
    indents[0] = vm_sea_ast_call(1, vm_sea_ast_ident("do"));
    while (*src != '\0')
    {
        size_t indent = vm_sea_parse_strip(&src);
        vm_sea_ast_t cur = indents[indent];
        if (*src == '\0')
        {
            break;
        }
        for(;;)
        {
            if ('0' <= *src && *src <= '9')
            {
                ptrdiff_t n = 0;
                while ('0' <= *src && *src <= '9')
                {
                    n *= 10;
                    n += *src - '0';
                    src += 1;
                }
                vm_sea_ast_call_add(cur.call, vm_sea_ast_num(n));
                break;
            }
            else
            {
                size_t slen = 0;
                size_t spaces = 0;
                const char *src0 = src;
                while (*src != ' ' && *src != '\n' && *src != '\0')
                {
                    slen += 1;
                    src += 1;
                }
                while (*src == ' ')
                {
                    spaces += 1;
                    src += 1;
                }
                char *dupd = malloc(sizeof(char) * (slen + 1));
                for (size_t i = 0; i < slen; i++)
                {
                    dupd[i] = src0[i];
                }
                dupd[slen] = '\0';
                if (dupd[0] == '|' && dupd[1] == '\0')
                {
                    indent += slen + spaces;
                    vm_sea_ast_t tree = vm_sea_ast_call(0);
                    indents[indent] = tree;
                    vm_sea_ast_call_add(cur.call, tree);
                    cur = tree;
                }
                else if (spaces == 0)
                {
                    vm_sea_ast_call_add(cur.call, vm_sea_ast_ident(dupd));
                    break;
                }
                else
                {
                    indent += slen + spaces;
                    vm_sea_ast_t tree = vm_sea_ast_call(1, vm_sea_ast_ident(dupd));
                    indents[indent] = tree;
                    vm_sea_ast_call_add(cur.call, tree);
                    cur = tree;
                }
            }
        }
    }
    return indents[0];
}
