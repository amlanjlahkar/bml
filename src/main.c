#include <editline/readline.h>

#include "../lib/mpc/mpc.c"
#include "parsing.h"

int main(void) {
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Toosty = mpc_new("toosty");

    // clang-format off
    mpca_lang (MPCA_LANG_DEFAULT,
        "                                                                       \
            number   : /-?[0-9]+/ ;                                             \
            symbol   : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\"    \
                     | \"cmpr\" | \"list\" | \"head\" | \"tail\" | \"join\"     \
                     | \"eval\" ;                                               \
            sexpr    : '(' <expr>* ')' ;                                        \
            qexpr    : '{' <expr>* '}' ;                                        \
            expr     : <number> | <symbol> | <sexpr> | <qexpr> ;                \
            toosty   : /^/ <expr>* /$/ ;                                        \
        ",
        Number, Symbol, Sexpr, Qexpr, Expr, Toosty);
    // clang-format on

    while (1) {
        char* ibuffer = readline("toosty> ");
        add_history(ibuffer);

        mpc_result_t result;

        if (strcmp(ibuffer, "exit") == 0) {
            free(ibuffer);
            break;
        }

        if (mpc_parse("<stdin>", ibuffer, Toosty, &result)) {
            void* ast = result.output;
            // mpc_ast_print(ast);
            tval* vres = tval_eval(tval_read_ast(ast));
            tval_println(vres);
            tval_del(vres);
            mpc_ast_delete(ast);
        } else {
            mpc_err_print(result.error);
            mpc_err_delete(result.error);
        }

        free(ibuffer);
    }

    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Toosty);

    return 0;
}
