#include <vm/asm.h>

#include "strbuf.h"
#include "parse.h"
#include "lower.h"

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
    // vm_sea_ast_print_s(stdout, ast);
    // fprintf(stdout, "\n");
    char *asmstr = vm_sea_lower(ast);
    vm_sea_ast_del(ast);
    vm_asm_buf_t bcbuf = vm_asm(asmstr);
    free(asmstr);
    if (bcbuf.nops == 0) {
        fprintf(stderr, "vm_asm(): error\n");
        return 1;
    }
    int res = vm_run_arch_int(bcbuf.nops, bcbuf.ops);
    free(bcbuf.ops);
    if (res) {
        fprintf(stderr, "vm_run_arch_int(): error #%i\n", (int) res);
        return 1;
    }
    return 0;
}