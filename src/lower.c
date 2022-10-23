
#include "lower.h"

#include <stdlib.h>
#include <string.h>

#include "strbuf.h"

vm_sea_strbuf_t *vm_sea_lower_state_buffer(vm_sea_lower_t *state) {
    return &state->bufs.bufs[state->bufs.nbufs - 1];
}

void vm_sea_lower_state_push(vm_sea_lower_t *state) {
    if (state->bufs.nbufs + 1 >= state->bufs.alloc) {
        state->bufs.alloc = (state->bufs.nbufs + 1) * 2;
        state->bufs.bufs = realloc(state->bufs.bufs, sizeof(struct vm_sea_strbuf_t) * state->bufs.alloc);
    }
    state->bufs.bufs[state->bufs.nbufs++] = vm_sea_strbuf_new();
}

void vm_sea_lower_state_pop(vm_sea_lower_t *state) {
    char *str = vm_sea_strbuf_to_string(&state->bufs.bufs[--state->bufs.nbufs]);
    char *str0 = str;
    while (*str != '\0') {
        vm_sea_strbuf_putchar(&state->endbuf, *str);
        str += 1;
    }
    free(str0);
}

void vm_sea_lower_state(vm_sea_lower_t *state, vm_sea_ast_t ast) {
    switch (ast.type) {
        case VM_SEA_AST_TYPE_IDENT: {
            vm_sea_lower_locals_t *locals = state->locals;
            while (locals != NULL) {
                if (!strcmp(locals->name, ast.str)) {
                    vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "  move $%u $%u\n", state->depth, locals->values);
                    goto done;
                }
                locals = locals->next;
            }
            fprintf(stderr, "no such local: `%s`\n", ast.str);
            __builtin_trap();
        }
        case VM_SEA_AST_TYPE_STRING: {
            vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "  move $%u \"%s\"\n", state->depth, ast.str);
            break;
        }
        case VM_SEA_AST_TYPE_NUMBER: {
            vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "  move $%u %u\n", state->depth, (int)ast.num);
            break;
        }
        case VM_SEA_AST_TYPE_CALL: {
            if (ast.call.args[0].type == VM_SEA_AST_TYPE_IDENT) {
                const char *name = ast.call.args[0].str;
                if (!strcmp(name, "do")) {
                    size_t ret = 0;
                    for (size_t index = 1; index < ast.call.nargs; index++) {
                        vm_sea_lower_state(state, ast.call.args[index]);
                    }
                    goto done;
                } else if (!strcmp(name, "dlopen")) {
                    vm_sea_lower_state(state, ast.call.args[1]);
                    vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "  dlopen $%u $%u\n", state->depth, state->depth);
                    goto done;
                } else if (!strcmp(name, "dlsym")) {
                    vm_sea_lower_state(state, ast.call.args[1]);
                    state->depth += 1;
                    vm_sea_lower_state(state, ast.call.args[2]);
                    state->depth -= 1;
                    vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "  dlsym $%u $%u $%u", state->depth, state->depth, state->depth + 1);
                    for (size_t index = 3; index < ast.call.nargs; index++) {
                        vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), " %s", ast.call.args[index].str);
                    }
                    vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "\n");
                    goto done;
                } else if (!strcmp(name, "out")) {
                    for (size_t index = 1; index < ast.call.nargs; index++) {
                        vm_sea_lower_state(state, ast.call.args[index]);
                        vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "  out $%u\n", state->depth);
                    }
                    goto done;
                } else if (!strcmp(name, "add") || !strcmp(name, "sub") || !strcmp(name, "mul") || !strcmp(name, "div") || !strcmp(name, "mod")) {
                    vm_sea_lower_state(state, ast.call.args[1]);
                    for (size_t index = 2; index < ast.call.nargs; index++) {
                        state->depth += 1;
                        vm_sea_lower_state(state, ast.call.args[index]);
                        state->depth -= 1;
                        vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "  %s $%u $%u $%u\n", name, state->depth, state->depth, state->depth + 1);
                    }
                    goto done;
                } else if (!strcmp(name, "let")) {
                    state->locals = &(vm_sea_lower_locals_t){
                        .next = state->locals,
                        .name = ast.call.args[1].call.args[0].str,
                        .values = state->depth,
                    };
                    vm_sea_lower_state(state, ast.call.args[1].call.args[1]);
                    state->depth += 1;
                    for (size_t index = 2; index < ast.call.nargs; index++) {
                        vm_sea_lower_state(state, ast.call.args[index]);
                    }
                    state->depth -= 1;
                    state->locals = state->locals->next;
                    vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "  move $%u $%u\n", state->depth, state->depth + 1);
                    goto done;
                }
                vm_sea_lower_locals_t *locals = state->locals;
                while (locals != NULL) {
                    if (!strcmp(locals->name, name)) {
                        size_t initdepth = state->depth;
                        for (size_t index = 1; index < ast.call.nargs; index++) {
                            vm_sea_lower_state(state, ast.call.args[index]);
                            state->depth += 1;
                        }
                        state->depth = initdepth;
                        vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "  call $%u $%u", state->depth, locals->values);
                        initdepth = state->depth;
                        for (size_t index = 1; index < ast.call.nargs; index++) {
                            vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), " $%u", state->depth);
                            state->depth += 1;
                        }
                        vm_sea_strbuf_printf(vm_sea_lower_state_buffer(state), "\n");
                        state->depth = initdepth;
                        goto done;
                    }
                    locals = locals->next;
                }
                fprintf(stderr, "unknown call to: %s\n", name);
            }
            __builtin_trap();
        }
        default: {
            fprintf(stderr, "unknown ast type\n");
            __builtin_trap();
        }
    }
done:;
}

static void vm_sea_lower_enter_func(vm_sea_strbuf_t *buf, char *funcname) {
    vm_sea_strbuf_printf(buf, "@%s:\n", funcname);
}

static void vm_sea_lower_exit_func(vm_sea_strbuf_t *buf) {
    vm_sea_strbuf_printf(buf, "  ret 0\n");
}

static void vm_sea_lower_prelude(vm_sea_strbuf_t *buf) {
    vm_sea_strbuf_printf(buf, "@:\n");
    vm_sea_strbuf_printf(buf, "  call $0 [toplevel]\n");
    vm_sea_strbuf_printf(buf, "  exit\n");
}

char *vm_sea_lower(vm_sea_ast_t ast) {
    vm_sea_lower_t state = (vm_sea_lower_t){
        .depth = 0,
        .locals = NULL,
        .bufs = (vm_sea_lower_bufs_t){
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
