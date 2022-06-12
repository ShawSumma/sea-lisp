#include <vm/asm.h>

#include "strbuf.h"

int main() {
    const char *src = "@__entry; exit;";
    vm_sea_strbuf_t strbuf = vm_sea_strbuf_new();
    vm_sea_strbuf_printf(&strbuf, "@__entry\n");
    vm_sea_strbuf_printf(&strbuf, "  r0 <- call toplevel\n");
    vm_sea_strbuf_printf(&strbuf, "  exit\n");
    char *str = vm_sea_strbuf_to_string(&strbuf);
    vm_asm_buf_t buf = vm_asm(str);
    free(str);
    if (buf.nops == 0) {
        fprintf(stderr, "vm_asm(): error\n");
        return 1;
    }
    int res = vm_run_arch_int(buf.nops, buf.ops);
    if (res) {
        fprintf(stderr, "vm_run_arch_int(): error #%i\n", res);
        return 1;
    }
    return 0;
}