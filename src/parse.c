#include "parse.h"
#include <stdarg.h>
#include <stdlib.h>

vm_sea_ast_t vm_sea_parse(const char *src) {
    // return vm_sea_ast_call(4,
    //     vm_sea_ast_ident("print"),
    //     vm_sea_ast_str("Hello, "),
    //     vm_sea_ast_call(3,
    //         vm_sea_ast_ident("+"),
    //         vm_sea_ast_call(3,
    //             vm_sea_ast_ident("*"),
    //             vm_sea_ast_num(49),
    //             vm_sea_ast_num(100)
    //         ),
    //         vm_sea_ast_num(85)
    //     ),
    //     vm_sea_ast_str("!")
    // );
    return vm_sea_ast_call(2,
        vm_sea_ast_ident("print"),
        vm_sea_ast_call(3,
            vm_sea_ast_ident("+"),
            vm_sea_ast_call(3,
                vm_sea_ast_ident("*"),
                vm_sea_ast_num(1),
                vm_sea_ast_num(2)
            ),
            vm_sea_ast_call(3,
                vm_sea_ast_ident("*"),
                vm_sea_ast_num(3),
                vm_sea_ast_num(4)
            )
        )
    );
}
