
#include "edit.h"
#include "ast.h"
#include "parse.h"
#include "strbuf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <termios.h>

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
    switch (ast.type)
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
        if (isselected)
        {
            fprintf(stdout, ":");
        }
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
            fprintf(stdout, "%s%c", fname, isselected ? ':' : ' ');
            for (size_t i = 1; i < ast.call.nargs; i++)
            {
                if (i != 1)
                {
                    fprintf(stdout, "\r\n%*c", (int)(depth + indent), ' ');
                }
                state->cur.path[state->cur.len++] = i;
                vm_sea_edit_print_zi(ast.call.args[i], state, depth + indent);
                state->cur.len--;
            }
        }
        else
        {
            fprintf(stdout, "{BAD AST}");
        }
        break;
    }
    default:
    {
        fprintf(stdout, "[type=%i]", (int)ast.type);
        break;
    }
    }
    if (isselected)
    {
        fprintf(stdout, "\x1b[22m");
    }
}

void vm_sea_edit_print_prog(vm_sea_ast_t prog, vm_sea_edit_query_t query)
{
    if (prog.type != VM_SEA_AST_TYPE_CALL)
    {
        return;
    }
    vm_sea_edit_print_state_t state = (vm_sea_edit_print_state_t){
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
    fprintf(stdout, "\x1b[H");
    fprintf(stdout, "\x1b[1;1H");
    vm_sea_edit_print_prog(ast, query);
    fflush(stdout);
}

int vm_sea_edit_get_key(void)
{
    char c;
    read(STDIN_FILENO, &c, 1);
    if (c == '\x1b')
    {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';
        if (seq[0] == '[')
        {
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return '\x1b';
                if (seq[2] == '~')
                {
                    switch (seq[1])
                    {
                    case '1':
                        return VM_SEA_EDIT_KEY_HOME;
                    case '3':
                        return VM_SEA_EDIT_KEY_DEL;
                    case '4':
                        return VM_SEA_EDIT_KEY_END;
                    case '5':
                        return VM_SEA_EDIT_KEY_PGUP;
                    case '6':
                        return VM_SEA_EDIT_KEY_PGDN;
                    case '7':
                        return VM_SEA_EDIT_KEY_HOME;
                    case '8':
                        return VM_SEA_EDIT_KEY_END;
                    }
                }
            }
            else
            {
                switch (seq[1])
                {
                case 'A':
                    return VM_SEA_EDIT_KEY_UP;
                case 'B':
                    return VM_SEA_EDIT_KEY_DOWN;
                case 'C':
                    return VM_SEA_EDIT_KEY_RIGHT;
                case 'D':
                    return VM_SEA_EDIT_KEY_LEFT;
                case 'H':
                    return VM_SEA_EDIT_KEY_HOME;
                case 'F':
                    return VM_SEA_EDIT_KEY_END;
                }
            }
        }
        else if (seq[0] == 'O')
        {
            switch (seq[1])
            {
            case 'H':
                return VM_SEA_EDIT_KEY_HOME;
            case 'F':
                return VM_SEA_EDIT_KEY_END;
            }
        }
        return '\x1b';
    }
    else
    {
        return c;
    }
}

bool vm_sea_edit_isnumber(const char *src)
{
    while (*src != '\0')
    {
        if ('0' <= *src && *src <= '9')
        {
            src += 1;   
        }
        else
        {
            return false;
        }
    }
    return true;
}

void vm_sea_edit_boot(void)
{
    vm_sea_edit_enter_raw_mode();

    vm_sea_strbuf_t buf = vm_sea_strbuf_read_file("out.sea");

    vm_sea_ast_t ast;

    if (buf.len != 0)
    {
        char *src = vm_sea_strbuf_to_string(&buf);

        ast = vm_sea_parse(src);

        free(src);
    }
    else
    {
        ast = vm_sea_parse("");
    }
    vm_sea_edit_query_t query = {
        .len = 0,
    };

    vm_sea_edit_redraw(ast, query);

    for (;;)
    {
        FILE *out = fopen("out.sea", "w");
        for (size_t i = 1; i < ast.call.nargs; i++)
        {
            vm_sea_ast_print_z(out, ast.call.args[i]);
            fprintf(out, "\n");
        }
        fclose(out);
        int key = vm_sea_edit_get_key();
    redo:;
        if (key == VM_SEA_EDIT_KEY_LEFT)
        {
            if (query.len > 0)
            {
                query.len -= 1;
            }
        }
        else if (key == VM_SEA_EDIT_KEY_RIGHT)
        {
            query.path[query.len++] = 1;
            vm_sea_ast_t *node = vm_sea_edit_query_get(query, &ast);
            if (node == NULL)
            {
                query.len -= 1;
            }
        }
        else if (key == VM_SEA_EDIT_KEY_UP)
        {
            if (query.len > 0 && query.path[query.len - 1] > 1)
            {
                query.path[query.len - 1] -= 1;
            }
            else if (query.len > 1)
            {
                query.len -= 1;
                vm_sea_ast_t *parent = vm_sea_edit_query_get(query, &ast);
                query.len += 1;
                query.path[query.len - 1] = parent->call.nargs - 1;
            }
        }
        else if (key == VM_SEA_EDIT_KEY_DOWN)
        {
            if (query.len > 0)
            {
                query.path[query.len - 1] += 1;
                vm_sea_ast_t *node = vm_sea_edit_query_get(query, &ast);
                if (node == NULL)
                {
                    query.path[query.len - 1] = 1;
                }
            }
        }
        else if (key < 128 && isprint(key) && key != '\n' && key != '\r' && key != ' ')
        {
            size_t len = 0;
            char *name = calloc(256, sizeof(char));
            name[len++] = (char) key;
            name[len + 0] = '?';
            name[len + 1] = '\0';
            vm_sea_ast_t *target = vm_sea_edit_query_get(query, &ast);
            if (target == NULL)
            {
                goto end;
            }
            if (target->type != VM_SEA_AST_TYPE_CALL)
            {
                *target = vm_sea_ast_call(1, *target);
            }
            vm_sea_ast_call_add(&target->call, vm_sea_ast_ident(name));
            for (;;)
            {
                vm_sea_edit_redraw(ast, query);
                int chr = vm_sea_edit_get_key();
                if (chr == '\x7F')
                {
                    if (len > 0)
                    {
                        len -= 1;
                        name[len + 0] = '?';
                        name[len + 1] = '\0';
                    }
                }
                else if (chr == '\n' || chr == '\r' || chr >= 128)
                {
                    if (len == 0)
                    {
                        target->call.nargs -= 1;
                    }
                    else
                    {
                        if (vm_sea_edit_isnumber(name))
                        {
                            target->call.nargs -= 1;
                            size_t n = 0;
                            sscanf(name, "%zu", &n);
                            vm_sea_ast_call_add(&target->call, vm_sea_ast_num(n));
                        }
                        else
                        {
                            name[len] = '\0';
                        }
                    }
                    key = chr;
                    goto redo;
                }
                else if (chr == ' ')
                {
                    if (len == 0)
                    {
                        target->call.nargs -= 1;
                    }
                    else
                    {
                        name[len] = '\0';
                        query.path[query.len++] = target->call.nargs - 1;
                    }
                    vm_sea_edit_redraw(ast, query);
                    goto end;
                }
                else
                {
                    name[len++] = chr;
                    name[len + 0] = '?';
                    name[len + 1] = '\0';
                }
            }
        }
    end:;
        vm_sea_edit_redraw(ast, query);
    }
}
