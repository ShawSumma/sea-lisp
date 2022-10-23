#if !defined(VM_SEA_HEADER_STRBUF)
#define VM_SEA_HEADER_STRBUF

#include <stddef.h>

struct vm_sea_strbuf_t;
typedef struct vm_sea_strbuf_t vm_sea_strbuf_t;

struct vm_sea_strbuf_t {
    char *buf;
    size_t len;
    size_t alloc;
};

vm_sea_strbuf_t vm_sea_strbuf_new(void);
void vm_sea_strbuf_del(vm_sea_strbuf_t buf);

char *vm_sea_strbuf_to_string(vm_sea_strbuf_t *buf);
vm_sea_strbuf_t vm_sea_strbuf_read_file(const char *filename);

void vm_sea_strbuf_putchar(vm_sea_strbuf_t *buf, char chr);
void vm_sea_strbuf_eof(vm_sea_strbuf_t *buf);
void vm_sea_strbuf_puts(vm_sea_strbuf_t *buf, const char *str);
void vm_sea_strbuf_putnum(vm_sea_strbuf_t *buf, size_t num);
void vm_sea_strbuf_printf(vm_sea_strbuf_t *buf, const char *fmt, ...);

#endif
