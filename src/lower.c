
#include "lower.h"
#include "strbuf.h"

#include <string.h>
#include <stdlib.h>

vm_sea_strbuf_t *vm_sea_lower_state_buffer(vm_sea_lower_t *state)
{
    return &state->bufs.bufs[state->bufs.nbufs-1];
}

void vm_sea_lower_state_push(vm_sea_lower_t *state)
{
    if (state->bufs.nbufs + 1 >= state->bufs.alloc)
    {
        state->bufs.alloc = (state->bufs.nbufs + 1) * 2;
        state->bufs.bufs = realloc(state->bufs.bufs, sizeof(struct vm_sea_strbuf_t) * state->bufs.alloc);
    }
    state->bufs.bufs[state->bufs.nbufs++] = vm_sea_strbuf_new();
}

void vm_sea_lower_state_pop(vm_sea_lower_t *state)
{
    char *str = vm_sea_strbuf_to_string(&state->bufs.bufs[--state->bufs.nbufs]);
    char *str0 = str;
    while (*str != '\0')
    {
        vm_sea_strbuf_putchar(&state->endbuf, *str);
        str += 1;
    }
    free(str0);
}

vm_sea_lower_res_t vm_sea_lower_state(vm_sea_lower_t *state, vm_sea_ast_t ast)
{
    switch (ast.type)
    {
    case VM_SEA_AST_TYPE_KEYWORD:
    {
        __builtin_trap();
    }
    case VM_SEA_AST_TYPE_IDENT:
    {
        __builtin_trap();
    }
    case VM_SEA_AST_TYPE_STRING:
    {
        size_t reg = state->nregs;
        vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "r%u <- str :%s\n", reg, ast.str);
        state->nregs += 1;
        return reg;
    }
    case VM_SEA_AST_TYPE_NUMBER:
    {
        size_t reg = state->nregs;
        vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "r%u <- int %i\n", reg, ast.num);
        state->nregs += 1;
        return reg;
    }
    case VM_SEA_AST_TYPE_CALL:
    {
        if (ast.call.args[0].type == VM_SEA_AST_TYPE_KEYWORD || ast.call.args[0].type == VM_SEA_AST_TYPE_IDENT)
        {
            const char *name = ast.call.args[0].str;
            if (!strcmp(name, "do"))
            {
                size_t ret = 0;
                for (size_t index = 1; index < ast.call.nargs; index++)
                {
                    ret = vm_sea_lower_state(state, ast.call.args[index]);
                }
                return ret;
            }
            else if (!strcmp(name, "out"))
            {
                for (size_t index = 1; index < ast.call.nargs; index++)
                {
                    size_t charreg = vm_sea_lower_state(state, ast.call.args[index]);
                    vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "putchar r%u\n", charreg);
                }
                return 0;
            }
        }
        __builtin_trap();
    }
    }
}

static void vm_sea_lower_enter_func(vm_sea_strbuf_t *buf, char *funcname)
{
    vm_sea_strbuf_printf(buf, "func %s\n", funcname);
}

static void vm_sea_lower_exit_func(vm_sea_strbuf_t *buf)
{
    vm_sea_strbuf_printf(buf, "r0 <- int 0\n");
    vm_sea_strbuf_printf(buf, "ret r0\n");
    vm_sea_strbuf_printf(buf, "end\n");
}

static void vm_sea_lower_prelude(vm_sea_strbuf_t *buf)
{
    vm_sea_strbuf_printf(buf, "@__entry\n");
    vm_sea_strbuf_printf(buf, "  r0 <- call toplevel\n");
    vm_sea_strbuf_printf(buf, "  exit\n");
}

char *vm_sea_lower(vm_sea_ast_t ast)
{
    vm_sea_lower_t state = (vm_sea_lower_t) {
        .nregs = 1,
        .bufs = (vm_sea_lower_bufs_t) {
            .bufs = NULL,
            .nbufs = 0,
            .alloc = 0,
        },
        .endbuf = vm_sea_strbuf_new(),
    };

    vm_sea_lower_state_push(&state);
    vm_sea_lower_prelude(vm_sea_lower_state_buffer(&state));

    vm_sea_lower_state_push(&state);
    vm_sea_lower_enter_func(vm_sea_lower_state_buffer(&state), "toplevel");
    vm_sea_lower_state(&state, ast);
    vm_sea_lower_exit_func(vm_sea_lower_state_buffer(&state));
    vm_sea_lower_state_pop(&state);
    
    vm_sea_lower_state_pop(&state);
    
    free(state.bufs.bufs);

    return vm_sea_strbuf_to_string(&state.endbuf);
}
