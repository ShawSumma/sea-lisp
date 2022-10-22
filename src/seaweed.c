#include "../minivm/vm/asm.h"
#include "../minivm/vm/be/int3.h"
#include "../minivm/vm/ir.h"
#include "lower.h"
#include "parse.h"
#include "strbuf.h"

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "main(): too few args\n");
        return 1;
    } else {
        const char *out = NULL;
        const char *file = NULL;
        for (int i = 1; i < argc; i += 1) {
            if (!strcmp(argv[i], "-o")) {
                i += 1;
                out = argv[i];
            } else if (file == NULL) {
                file = argv[i];
            } else {
                fprintf(stderr, "main(): cannot open both %s and %s\n", file, argv[i]);
            }
        }
        vm_sea_strbuf_t srcbuf = vm_sea_strbuf_read_file(file);
        if (srcbuf.len == 0) {
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
        if (out) {
            FILE *outfile = fopen(out, "w");
            fprintf(outfile, "%s", asmstr);
            fclose(outfile);
        } else {
            vm_block_t *block = vm_parse_asm(asmstr);
            vm_state_t *state = vm_state_init(1 << 16);
            vm_run(state, block);
            vm_state_deinit(state);
        }
        return 0;
    }
}
