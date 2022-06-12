#if !defined(VM_SEA_HEADER_AST)
#define VM_SEA_HEADER_AST

#include <stddef.h>
#include <stdio.h>

enum vm_sea_ast_type_t;

struct vm_sea_ast_t;
typedef struct vm_sea_ast_t vm_sea_ast_t;

struct vm_sea_ast_call_t;
typedef struct vm_sea_ast_call_t vm_sea_ast_call_t;

enum vm_sea_ast_type_t
{
    VM_SEA_AST_TYPE_NUMBER,
    VM_SEA_AST_TYPE_IDENT,
    VM_SEA_AST_TYPE_STRING,
    VM_SEA_AST_TYPE_CALL,
};

struct vm_sea_ast_call_t
{
    vm_sea_ast_t *args;
    size_t nargs;
    size_t alloc;
};

struct vm_sea_ast_t
{
    size_t type;
    union 
    {
        ptrdiff_t num;
        const char *str;
        vm_sea_ast_call_t *call;
    };
};

vm_sea_ast_t vm_sea_ast_call(size_t n, ...);
vm_sea_ast_t vm_sea_ast_num(ptrdiff_t n);
vm_sea_ast_t vm_sea_ast_str(const char *str);
vm_sea_ast_t vm_sea_ast_ident(const char *str);

void vm_sea_ast_call_add(vm_sea_ast_call_t *out, vm_sea_ast_t ast);

void vm_sea_ast_print_s(FILE *out, vm_sea_ast_t ast);
void vm_sea_ast_print_zi(FILE *out, vm_sea_ast_t ast, size_t depth);
void vm_sea_ast_print_z(FILE *out, vm_sea_ast_t ast);

#endif
