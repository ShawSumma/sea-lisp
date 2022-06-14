
#if !defined(VM_SEA_HEADER_EDIT)
#define VM_SEA_HEADER_EDIT

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ast.h"

struct vm_sea_edit_print_state_t;
typedef struct vm_sea_edit_print_state_t vm_sea_edit_print_state_t;

struct vm_sea_edit_query_t;
typedef struct vm_sea_edit_query_t vm_sea_edit_query_t;

enum
{
    VM_SEA_EDIT_KEY_LEFT = 1 << 12,
    VM_SEA_EDIT_KEY_RIGHT,
    VM_SEA_EDIT_KEY_UP,
    VM_SEA_EDIT_KEY_DOWN,
    VM_SEA_EDIT_KEY_DEL,
    VM_SEA_EDIT_KEY_HOME,
    VM_SEA_EDIT_KEY_END,
    VM_SEA_EDIT_KEY_PGUP,
    VM_SEA_EDIT_KEY_PGDN,
};

struct vm_sea_edit_query_t
{
    uint8_t path[128];
    size_t len;
};

struct vm_sea_edit_print_state_t 
{
    vm_sea_edit_query_t cur;
    vm_sea_edit_query_t query;
    size_t hole;
};

bool vm_sea_edit_query_equal(vm_sea_edit_query_t a, vm_sea_edit_query_t b);
vm_sea_ast_t *vm_sea_edit_query_get(vm_sea_edit_query_t query, vm_sea_ast_t *ast);
void vm_sea_edit_print_zi(vm_sea_ast_t ast, vm_sea_edit_print_state_t *state, size_t depth);
void vm_sea_edit_print_prog(vm_sea_ast_t prog, vm_sea_edit_query_t query);
void vm_sea_edit_exit_raw_mode(void);
void vm_sea_edit_enter_raw_mode(void);
void vm_sea_edit_clear(void);
int vm_sea_edit_get_key(void);
void vm_sea_edit_redraw(vm_sea_ast_t ast, vm_sea_edit_query_t query);

void vm_sea_edit_boot(void);

#endif
