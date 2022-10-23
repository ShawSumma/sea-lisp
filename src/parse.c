#include "parse.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "strbuf.h"

size_t vm_sea_parse_strip(const char **psrc) {
    size_t ret = 0;
    for (;;) {
        if (**psrc == ' ') {
            ret += 1;
            *psrc += 1;
            continue;
        }
        if (**psrc == '\n') {
            ret = 0;
            *psrc += 1;
            continue;
        }
        return ret;
    }
}

static char *vm_sea_strdup(const char *name) {
    size_t len = strlen(name);
    char *ret = malloc(sizeof(char) * (len + 1));
    memcpy(ret, name, sizeof(char) * (len + 1));
    return ret;
}

vm_sea_ast_t vm_sea_parse(const char *src) {
    vm_sea_ast_t ret = vm_sea_ast_call(1, vm_sea_ast_ident(vm_sea_strdup("do")));
    vm_sea_ast_t *indents[256];
    indents[0] = &ret;
    while (*src != '\0') {
        size_t indent = vm_sea_parse_strip(&src);
        vm_sea_ast_call_t *cur = &indents[indent]->call;
        if (*src == '\0') {
            break;
        }
        for (;;) {
            if ('0' <= *src && *src <= '9') {
                ptrdiff_t n = 0;
                while ('0' <= *src && *src <= '9') {
                    n *= 10;
                    n += *src - '0';
                    src += 1;
                }
                vm_sea_ast_call_add(cur, vm_sea_ast_num(n));
                break;
            } else if (*src == ':') {
                src += 1;
                vm_sea_strbuf_t buf = vm_sea_strbuf_new();
                while (*src != '\n' && *src != '\0') {
                    vm_sea_strbuf_putchar(&buf, *src);
                    src += 1;
                }
                vm_sea_ast_call_add(cur, vm_sea_ast_str(buf.buf));
            } else if (*src == '\\') {
                src += 1;
                vm_sea_ast_call_add(cur, vm_sea_ast_num(*src));
                src += 1;
            } else {
                size_t slen = 0;
                size_t spaces = 0;
                const char *src0 = src;
                while (*src != ' ' && *src != '\n' && *src != '\0') {
                    slen += 1;
                    src += 1;
                }
                while (*src == ' ') {
                    spaces += 1;
                    src += 1;
                }
                char *dupd = malloc(sizeof(char) * (slen + 1));
                for (size_t i = 0; i < slen; i++) {
                    dupd[i] = src0[i];
                }
                dupd[slen] = '\0';
                if (dupd[0] == '|' && dupd[1] == '\0') {
                    indent += slen + spaces;
                    vm_sea_ast_t tree = vm_sea_ast_call(0);
                    vm_sea_ast_call_add(cur, tree);
                    indents[indent] = &cur->args[cur->nargs - 1];
                    cur = &indents[indent]->call;
                } else if (spaces == 0) {
                    vm_sea_ast_call_add(cur, vm_sea_ast_ident(dupd));
                    break;
                } else {
                    indent += slen + spaces;
                    vm_sea_ast_t tree = vm_sea_ast_call(1, vm_sea_ast_ident(dupd));
                    vm_sea_ast_call_add(cur, tree);
                    indents[indent] = &cur->args[cur->nargs - 1];
                    cur = &indents[indent]->call;
                }
            }
        }
    }
    return ret;
}
