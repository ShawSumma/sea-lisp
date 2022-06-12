
#include "lower.h"
#include "strbuf.h"

#include <string.h>

static size_t vm_sea_lower_to(vm_sea_ast_t ast, size_t *nregs, vm_sea_strbuf_t *buf)
{
    switch (ast.type)
    {
    case VM_SEA_AST_TYPE_IDENT:
    {
        __builtin_trap();
    }
    case VM_SEA_AST_TYPE_STRING:
    {
        size_t reg = *nregs;
        vm_sea_strbuf_printf(buf, "r%u <- str :%s\n", reg, ast.str);
        *nregs += 1;
        return reg;
    }
    case VM_SEA_AST_TYPE_NUMBER:
    {
        size_t reg = *nregs;
        vm_sea_strbuf_printf(buf, "r%u <- int %i\n", reg, ast.num);
        *nregs += 1;
        return reg;
    }
    case VM_SEA_AST_TYPE_CALL:
    {
        if (ast.call->args[0].type == VM_SEA_AST_TYPE_IDENT)
        {
            const char *name = ast.call->args[0].str;
            if (!strcmp(name, "do"))
            {
                size_t ret = 0;
                for (size_t index = 1; index < ast.call->nargs; index++)
                {
                    ret = vm_sea_lower_to(ast.call->args[index], nregs, buf);
                }
                return ret;
            }
            else if (!strcmp(name, "out"))
            {
                for (size_t index = 1; index < ast.call->nargs; index++)
                {
                    size_t charreg = vm_sea_lower_to(ast.call->args[index], nregs, buf);
                    vm_sea_strbuf_printf(buf, "putchar r%u\n", charreg);
                }
                return 0;
            }
        }
        __builtin_trap();
    }
    }
}

static void vm_sea_lower_prelude(vm_sea_strbuf_t *buf)
{
    vm_sea_strbuf_printf(buf, "@__entry\n");
    vm_sea_strbuf_printf(buf, "  r0 <- call toplevel\n");
    vm_sea_strbuf_printf(buf, "  exit\n");
}

char *vm_sea_lower(vm_sea_ast_t ast)
{
    vm_sea_strbuf_t buf = vm_sea_strbuf_new();
    vm_sea_lower_prelude(&buf);
    size_t nregs = 1;
    vm_sea_strbuf_printf(&buf, "func toplevel\n");
    vm_sea_lower_to(ast, &nregs, &buf);
    vm_sea_strbuf_printf(&buf, "end\n");
    return vm_sea_strbuf_to_string(&buf);
}
