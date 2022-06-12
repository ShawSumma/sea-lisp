#include <vm/asm.h>

#include "strbuf.h"
#include "parse.h"

int main(int argc, const char **argv) {
    if (argc < 2)
    {
        fprintf(stderr, "main(): error: too few args\n");
        return 1;
    }
    vm_sea_strbuf_t srcbuf = vm_sea_strbuf_read_file(argv[1]);
    if (srcbuf.len == 0)
    {
        fprintf(stderr, "main(): error: no such file, or empty file: %s\n", argv[1]);
        return 1;
    }
    char *srcstr = vm_sea_strbuf_to_string(&srcbuf);
    vm_sea_ast_t ast = vm_sea_parse(srcstr);
    free(srcstr);
    vm_sea_ast_print_s(stdout, ast);
    fprintf(stdout, "\n");
    // const char *src = "@__entry; exit;";
    // vm_sea_strbuf_t strbuf = vm_sea_strbuf_new();
    // vm_sea_strbuf_printf(&strbuf, "@__entry\n");
    // vm_sea_strbuf_printf(&strbuf, "  r0 <- call toplevel\n");
    // vm_sea_strbuf_printf(&strbuf, "  exit\n");
    // char *str = vm_sea_strbuf_to_string(&strbuf);
    // vm_asm_buf_t buf = vm_asm(str);
    // free(str);
    // if (buf.nops == 0) {
    //     fprintf(stderr, "vm_asm(): error\n");
    //     return 1;
    // }
    // int res = vm_run_arch_int(buf.nops, buf.ops);
    // if (res) {
    //     fprintf(stderr, "vm_run_arch_int(): error #%i\n", res);
    //     return 1;
    // }
    return 0;
}