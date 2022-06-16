
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
        for (size_t i = 0; i < state->nlocals; i++)
        {
            if (!strcmp(state->locals[i].name, ast.str))
            {
                return state->locals[i].reg;
            }
        }
        fprintf(stderr, "unknown identifier: %s\n", ast.str);
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
            else if (!strcmp(name, "let"))
            {
                size_t len0 = state->nlocals;
                for (size_t i = 1; i < ast.call.nargs - 1; i++)
                {
                    vm_sea_ast_t sets = ast.call.args[i];
                    if (sets.type == VM_SEA_AST_TYPE_CALL && sets.call.nargs == 2)
                    {
                        const char *name = sets.call.args[0].str;
                        size_t varreg = vm_sea_lower_state(state, sets.call.args[1]);
                        if (state->nlocals + 4 >= state->locals_alloc)
                        {
                            state->locals_alloc = (state->nlocals + 4) * 4;
                            state->locals = realloc(state->locals, sizeof(vm_sea_lower_locals_t) * state->locals_alloc);
                        }
                        state->locals[state->nlocals++] = (vm_sea_lower_locals_t) {
                            .name = name,
                            .reg = varreg,
                        };
                    }
                }
                size_t ret = vm_sea_lower_state(state, ast.call.args[ast.call.nargs - 1]);
                state->nlocals = len0;
                return ret;
            }
            else if (!strcmp(name, "+"))
            {
                size_t ret = state->nregs++;
                vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "r%u <- int 0\n", ret);
                for (size_t i = 1; i < ast.call.nargs; i++)
                {
                    size_t tmp = vm_sea_lower_state(state, ast.call.args[i]);
                    vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "r%u <- add r%u r%u\n", ret, ret, tmp);
                }
                return ret;
            }
        }
        vm_sea_ast_print_z(stderr, ast);
        fprintf(stderr, "\n");
        __builtin_trap();
    }
    default:
    {
        vm_sea_ast_print_z(stderr, ast);
        fprintf(stderr, "\n");
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
        .locals = NULL,
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
    free(state.locals);

    return vm_sea_strbuf_to_string(&state.endbuf);
}
