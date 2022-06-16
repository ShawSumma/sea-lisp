
#if !defined(VM_SEA_HEADER_LOWER)
#define VM_SEA_HEADER_LOWER

#include "ast.h"

typedef size_t vm_sea_lower_res_t;

struct vm_sea_lower_locals_t;
typedef struct vm_sea_lower_locals_t vm_sea_lower_locals_t;

struct vm_sea_lower_bufs_t;
typedef struct vm_sea_lower_bufs_t vm_sea_lower_bufs_t;

struct vm_sea_lower_t;
typedef struct vm_sea_lower_t vm_sea_lower_t;

#include "strbuf.h"

struct vm_sea_lower_locals_t
{
    const char *name;
    size_t reg;
};

struct vm_sea_lower_bufs_t
{
    vm_sea_strbuf_t *bufs;
    size_t nbufs;
    size_t alloc;
};

struct vm_sea_lower_t 
{
    vm_sea_lower_locals_t *locals;
    size_t nlocals;
    size_t locals_alloc;
    size_t nregs;
    vm_sea_lower_bufs_t bufs;
    vm_sea_strbuf_t endbuf;
};

char *vm_sea_lower(vm_sea_ast_t ast);
vm_sea_lower_res_t vm_sea_lower_state(vm_sea_lower_t *state, vm_sea_ast_t ast);

void vm_sea_lower_state_push(vm_sea_lower_t *state);
void vm_sea_lower_state_pop(vm_sea_lower_t *state);
vm_sea_strbuf_t *vm_sea_lower_state_buffer(vm_sea_lower_t *state);

#endif
