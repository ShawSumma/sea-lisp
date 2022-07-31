#include <vm/asm.h>
#include <vm/ir/ir.h>
#include <vm/ir/opt.h>
#include <vm/ir/toir.h>
#include <vm/ir/build.h>
#include <vm/ir/be/jit.h>

#include "strbuf.h"
#include "parse.h"
#include "lower.h"

int main(int argc, char **argv) {
    const char *dump = NULL;
    const char *filename = NULL;
    size_t runs = 1;
    size_t jit = 1;
    size_t dumpasm = 0;
    size_t jitdumpir = 0;
    size_t jitdumpopt = 0;
    while (true)
    {
        if (argc < 2)
        {
            if (filename == NULL)
            {
                fprintf(stderr, "too few args\n");
                return 1;
            }
            else
            {
                break;
            }
        }
        if (!strcmp(argv[1], "-o") || !strcmp(argv[1], "--output"))
        {
            argv += 1;
            argc -= 1;
            dump = argv[1];
            argv += 1;
            argc -= 1;
            continue;
        }
        if (!strcmp(argv[1], "-n"))
        {
            argv += 1;
            argc -= 1;
            size_t n = 0;
            char *ptr = argv[1];
            while (*ptr != '\0')
            {
                n *= 10;
                n += *ptr - '0';
                ptr += 1;
            }
            argv += 1;
            argc -= 1;
            runs = n;
            continue;
        }
        if (argv[1][0] == '-' && argv[1][1] == 'j')
        {
            char *tmp = argv[1] + 2;
            argv += 1;
            argc -= 1;
            if (!strcmp(tmp, "on"))
            {
                jit = 1;
            }
            else if (!strcmp(tmp, "off"))
            {
                jit = 0;
            }
            else if (!strcmp(tmp, "dump=asm"))
            {
                dumpasm = 1;
            }
            else if (!strcmp(tmp, "dump=ir"))
            {
                jitdumpir = 1;
            }
            else if (!strcmp(tmp, "dump=opt"))
            {
                jitdumpopt = 1;
            }
            else
            {
                fprintf(stderr, "unknown -j option: -j%s\n", tmp);
                return 1;
            }
            continue;
        }
        if (filename != NULL)
        {
            fprintf(stderr, "cannot handle multiple files at cli\n");
            return 1;
        }
        else
        {
            filename = argv[1];
            argv += 1;
            argc -= 1;
        }
    }
    if (!jit && (jitdumpopt || jitdumpir))
    {
        fprintf(stderr, "cannot use -jdump with out jit turned on (-jon vs -joff)");
        return 1;
    }
        for (size_t i = 0; i < runs; i++)
    {
        vm_sea_strbuf_t srcbuf = vm_sea_strbuf_read_file(filename);
        if (srcbuf.len == 0)
        {
            fprintf(stderr, "main(): error: no such file, or empty file: %s\n", filename);
            return 1;
        }
        char *srcstr = vm_sea_strbuf_to_string(&srcbuf);
        if (srcstr == NULL)
        {
            fprintf(stderr, "could not read file\n");
            return 1;
        }
        vm_sea_ast_t ast = vm_sea_parse(srcstr);
        vm_free(srcstr);
        char *asmstr = vm_sea_lower(ast);
        if (dumpasm)
        {
            fprintf(stderr, "%s\n", asmstr);
        }
        vm_bc_buf_t buf = vm_asm(asmstr);
        if (jit)
        {
            size_t nblocks = buf.nops;
            vm_ir_block_t *blocks = vm_ir_parse(nblocks, buf.ops);
            vm_free(buf.ops);
            if (jitdumpir)
            {
                vm_ir_print_blocks(stderr, nblocks, blocks);
            }
            vm_ir_opt_all(&nblocks, &blocks);
            if (jitdumpopt)
            {
                vm_ir_print_blocks(stderr, nblocks, blocks);
            }
            vm_ir_be_jit(nblocks, blocks);
            vm_ir_blocks_free(nblocks, blocks);
        }
        else
        {
            if (buf.nops == 0)
            {
                fprintf(stderr, "could not assemble file\n");
                return 1;
            }
            if (dump)
            {
                void *out = fopen(dump, "wb");
                fwrite(buf.ops, sizeof(vm_opcode_t), buf.nops, out);
                fclose(out);
            }
            else
            {
                int res = vm_run_arch_int(buf.nops, buf.ops);
                if (res != 0)
                {
                    fprintf(stderr, "could not run asm\n");
                    return 1;
                }
            }
            vm_free(buf.ops);
        }
    }
    return 0;
}