
#include "edit.h"
#include "ast.h"
#include "parse.h"
#include "strbuf.h"

#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <termios.h>

struct vm_sea_edit_print_state_t;
typedef struct vm_sea_edit_print_state_t vm_sea_edit_print_state_t;

struct vm_sea_edit_query_t;
typedef struct vm_sea_edit_query_t vm_sea_edit_query_t;

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

bool vm_sea_edit_query_equal(vm_sea_edit_query_t a, vm_sea_edit_query_t b)
{
    if (a.len != b.len)
    {
        return false;
    }
    for (size_t i = 0; i < a.len; i++)
    {
        if (a.path[i] != b.path[i])
        {
            return false;
        }
    }
    return true;
}

vm_sea_ast_t *vm_sea_edit_query_get(vm_sea_edit_query_t query, vm_sea_ast_t *ast)
{
    for (size_t i = 0; i < query.len && ast->type == VM_SEA_AST_TYPE_CALL; i++)
    {
        if (query.path[i] >= ast->call.nargs)
        {
            return NULL;
        }
        ast = &ast->call.args[query.path[i]];
    }
    return ast;
}

void vm_sea_edit_print_zi(vm_sea_ast_t ast, vm_sea_edit_print_state_t *state, size_t depth)
{
    bool isselected = vm_sea_edit_query_equal(state->cur, state->query);
    if (isselected)
    {
        fprintf(stdout, "\x1b[1m");
    }
    switch(ast.type)
    {
    case VM_SEA_AST_TYPE_NUMBER:
    {
        fprintf(stdout, "%zi", ast.num);
        break;
    }
    case VM_SEA_AST_TYPE_KEYWORD:
    case VM_SEA_AST_TYPE_IDENT:
    {
        fprintf(stdout, "%s", ast.str);
        break;
    }
    case VM_SEA_AST_TYPE_STRING:
    {
        fprintf(stdout, "\"%s\"", ast.str);
        break;
    }
    case VM_SEA_AST_TYPE_CALL:
    {
        if (ast.call.args[0].type == VM_SEA_AST_TYPE_IDENT || ast.call.args[0].type == VM_SEA_AST_TYPE_KEYWORD)
        {
            const char *fname = ast.call.args[0].str;
            size_t indent = strlen(fname) + 1;
            fprintf(stdout, "%s ", fname);
            for (size_t i = 1; i < ast.call.nargs; i++)
            {
                if (i != 1) {
                    fprintf(stdout, "\r\n%*c", (int) (depth + indent), ' ');
                }
                state->cur.path[state->cur.len++] = i;
                vm_sea_edit_print_zi(ast.call.args[i], state, depth + indent);
                state->cur.len--;
            }
        }
        else
        {
            const char *fname = "|";
            size_t indent = strlen(fname) + 1;
            fprintf(stdout, "%s ", fname);
            for (size_t i = 0; i < ast.call.nargs; i++)
            {
                if (i != 0) {
                    fprintf(stdout, "\r\n%*c", (int) (depth + indent), ' ');
                }
                state->cur.path[state->cur.len++] = i + 1;
                vm_sea_edit_print_zi(ast.call.args[i], state, depth + indent);
                state->cur.len--;
            }
        }
        break;
    }
    default:
    {
        fprintf(stdout, "[type=%i]", (int) ast.type);
        break;
    }
    }
    if (isselected)
    {
        fprintf(stdout, "\x1b[22m");
    }
}

void vm_sea_edit_print_z(vm_sea_ast_t ast)
{
}

void vm_sea_edit_print_prog(vm_sea_ast_t prog, vm_sea_edit_query_t query)
{
    if (prog.type != VM_SEA_AST_TYPE_CALL)
    {
        return;
    }
    vm_sea_edit_print_state_t state = (vm_sea_edit_print_state_t) {
        .hole = 1,
        .query = query,
    };
    vm_sea_edit_print_zi(prog, &state, 0);
}

struct termios orig_termios;

void vm_sea_edit_exit_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void vm_sea_edit_enter_raw_mode(void)
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(&vm_sea_edit_exit_raw_mode);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void vm_sea_edit_clear(void)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
}

void vm_sea_edit_redraw(vm_sea_ast_t ast, vm_sea_edit_query_t query)
{
    vm_sea_edit_clear();
    fprintf(stdout , "\x1b[H");
    fprintf(stdout , "\x1b[1;1H");
    vm_sea_edit_print_prog(ast, query);
    fflush(stdout);
}


void vm_sea_edit_boot(void)
{
    vm_sea_edit_enter_raw_mode();

    vm_sea_ast_t ast = vm_sea_parse("");

    vm_sea_edit_query_t query = {
        .len = 0,
    };

    vm_sea_edit_redraw(ast, query);

    char c;

    for (;;)
    {
        read(STDIN_FILENO, &c, 1);
    redo:;
        vm_sea_edit_redraw(ast, query);
        if (c == 27)
        {
            read(STDIN_FILENO, &c, 1);
            if (c == '[')
            {
                read(STDIN_FILENO, &c, 1);
                if (c == 'D' && query.len > 0)
                {
                    query.len -= 1;
                }
                if (c == 'C')
                {
                    query.path[query.len++] = 1;
                }
                if (c == 'A' && query.len > 0)
                {
                    if (query.path[query.len - 1] > 1)
                    {
                        query.path[query.len - 1] -= 1;
                    }
                    else
                    {
                        query.len -= 1;
                        vm_sea_ast_t *parent = vm_sea_edit_query_get(query, &ast);
                        query.len += 1;
                        query.path[query.len - 1] = parent->call.nargs - 1;
                    }
                }
                if (c == 'B' && query.len > 0)
                {
                    query.path[query.len - 1] += 1;
                    vm_sea_ast_t *node = vm_sea_edit_query_get(query, &ast);
                    if (node == NULL)
                    {
                        query.path[query.len - 1] = 1;
                    }
                }
            }
        }
        else if (!iscntrl(c) && isprint(c) && c != '\n' && c != '\r' && c != ' ')
        {
            size_t len = 0;
            char *name = calloc(256, sizeof(char));
            name[len++] = c;
            name[len+0] = '?';
            name[len+1] = '\0';
            vm_sea_ast_t *target = vm_sea_edit_query_get(query, &ast);
            if (target->type != VM_SEA_AST_TYPE_CALL)
            {
                *target = vm_sea_ast_call(1, *target);
            }
            vm_sea_ast_call_add(&target->call, vm_sea_ast_ident(name));
            for (;;)
            {
                vm_sea_edit_redraw(ast, query);
                read(STDIN_FILENO, &c, 1);
                if (c == '\b' && len > 0)
                {
                    len -= 1;
                    name[len+0] = '?';
                    name[len+1] = '\0';
                }
                else if (c == '\n' || c == '\r' || c == 27)
                {
                    name[len] = '\0';
                    goto redo;
                }
                else if (c == ' ')
                {
                    name[len] = '\0';
                    query.path[query.len++] = target->call.nargs - 1;
                    vm_sea_edit_redraw(ast, query);
                    goto end;
                }
                else
                {
                    name[len++] = c;
                    name[len+0] = '?';
                    name[len+1] = '\0';
                }
            }
        }
    end:;
        vm_sea_edit_redraw(ast, query);
    }
}
