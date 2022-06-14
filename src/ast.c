#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void vm_sea_ast_del(vm_sea_ast_t ast)
{
    if (ast.type == VM_SEA_AST_TYPE_CALL)
    {
        for (size_t i = 0; i < ast.call.nargs; i++)
        {
            vm_sea_ast_del(ast.call.args[i]);
        }
        free(ast.call.args);
    }
    if (ast.type == VM_SEA_AST_TYPE_STRING || ast.type == VM_SEA_AST_TYPE_IDENT)
    {
        free((void*) ast.str);
    }
}

vm_sea_ast_t vm_sea_ast_call(size_t n, ...)
{
    va_list va;
    va_start(va, n);
    vm_sea_ast_t *args = malloc(sizeof(vm_sea_ast_t) * n);
    for (size_t i = 0; i < n; i++) {
        args[i] = va_arg(va, vm_sea_ast_t);
    }
    va_end(va);
    return (vm_sea_ast_t) {
        .type = VM_SEA_AST_TYPE_CALL,
        .call = (vm_sea_ast_call_t) {
            .args = args,
            .nargs = n,
            .alloc = n,
        },
    };
}

vm_sea_ast_t vm_sea_ast_num(ptrdiff_t n)
{
    return (vm_sea_ast_t) {
        .type = VM_SEA_AST_TYPE_NUMBER,
        .num = n,
    };
}

vm_sea_ast_t vm_sea_ast_str(const char *str)
{
    return (vm_sea_ast_t) {
        .type = VM_SEA_AST_TYPE_STRING,
        .str = str,
    };
}

vm_sea_ast_t vm_sea_ast_ident(const char *str)
{
    return (vm_sea_ast_t) {
        .type = VM_SEA_AST_TYPE_IDENT,
        .str = str,
    };
}

vm_sea_ast_t vm_sea_ast_keyword(const char *str)
{
    return (vm_sea_ast_t) {
        .type = VM_SEA_AST_TYPE_KEYWORD,
        .str = str,
    };
}

void vm_sea_ast_call_add(vm_sea_ast_call_t *out, vm_sea_ast_t ast)
{
    if (out->nargs + 1 >= out->alloc)
    {
        out->alloc = (out->nargs + 1) * 2;
        out->args = realloc(out->args, sizeof(vm_sea_ast_t) * out->alloc);
    }
    out->args[out->nargs++] = ast;
}

void vm_sea_ast_print_s(FILE *out, vm_sea_ast_t ast) 
{
    switch (ast.type)
    {
    case VM_SEA_AST_TYPE_NUMBER:
        fprintf(out, "%zi", ast.num);
        break;
    case VM_SEA_AST_TYPE_KEYWORD:
    case VM_SEA_AST_TYPE_IDENT:
        fprintf(out, "%s", ast.str);
        break;
    case VM_SEA_AST_TYPE_STRING:
        fprintf(out, "\"%s\"", ast.str);
        break;
    case VM_SEA_AST_TYPE_CALL:
        fprintf(out, "(");
        for (size_t i = 0; i < ast.call.nargs; i++)
        {
            if (i != 0)
            {
                fprintf(out, " ");
            }
            vm_sea_ast_print_s(out, ast.call.args[i]);
        }
        fprintf(out, ")");
        break;
    default:
        fprintf(out, "[type=%i]", (int) ast.type);
        break;
    }
}

void vm_sea_ast_print_zi(FILE *out, vm_sea_ast_t ast, size_t depth)
{
    switch(ast.type)
    {
    case VM_SEA_AST_TYPE_NUMBER:
    {
        fprintf(out, "%zi", ast.num);
        break;
    }
    case VM_SEA_AST_TYPE_KEYWORD:
    case VM_SEA_AST_TYPE_IDENT:
    {
        fprintf(out, "%s", ast.str);
        break;
    }
    case VM_SEA_AST_TYPE_STRING:
    {
        fprintf(out, "\"%s\"", ast.str);
        break;
    }
    case VM_SEA_AST_TYPE_CALL:
    {
        if (ast.call.args[0].type == VM_SEA_AST_TYPE_IDENT || ast.call.args[0].type == VM_SEA_AST_TYPE_KEYWORD)
        {
            const char *fname = ast.call.args[0].str;
            size_t indent = strlen(fname) + 1;
            fprintf(out, "%s ", fname);
            for (size_t i = 1; i < ast.call.nargs; i++)
            {
                if (i != 1) {
                    fprintf(out, "\n%*c", (int) (depth + indent), ' ');
                }
                vm_sea_ast_print_zi(out, ast.call.args[i], depth + indent);
            }
        }
        else
        {
            const char *fname = "|";
            size_t indent = strlen(fname) + 1;
            fprintf(out, "%s ", fname);
            for (size_t i = 0; i < ast.call.nargs; i++)
            {
                if (i != 1) {
                    fprintf(out, "\n%*c", (int) (depth + indent), ' ');
                }
                vm_sea_ast_print_zi(out, ast.call.args[i], depth + indent);
            }
        }
        break;
    }
    default:
    {
        fprintf(out, "[type=%i]", (int) ast.type);
        break;
    }
    }
}

void vm_sea_ast_print_z(FILE *out, vm_sea_ast_t ast)
{
    vm_sea_ast_print_zi(out, ast, 0);
}
