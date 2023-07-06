#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lib/mpc/mpc.c"
#include <editline/readline.h>

int
main (int argc, char** argv)
{
    mpc_parser_t* Number   = mpc_new ("number");
    mpc_parser_t* Operator = mpc_new ("operator");
    mpc_parser_t* Expr     = mpc_new ("expr");
    mpc_parser_t* Toosty   = mpc_new ("toosty");

    // clang-format off
    mpca_lang (MPCA_LANG_DEFAULT,
        "                                                       \
            number   : /-?[0-9]+/ ;                             \
            operator : '+' | '-' | '*' | '/' ;                  \
            expr     : <number> | '(' <operator> <expr>+ ')' ;  \
            toosty   : /^/ <operator> <expr>+ /$/ ;             \
        ",
        Number, Operator, Expr, Toosty);
    // clang-format on

    puts ("Toosty version 0.0.1");

    while (true)
        {
            char* ibuffer = readline ("toosty> ");
            add_history (ibuffer);

            mpc_result_t r;
            if (mpc_parse ("<stdin>", ibuffer, Toosty, &r))
                {
                    mpc_ast_print (r.output);
                    mpc_ast_delete (r.output);
                }
            else
                {
                    mpc_err_print (r.error);
                    mpc_err_delete (r.error);
                }

            free (ibuffer);
        }

    mpc_cleanup (4, Number, Operator, Expr, Toosty);

    return EXIT_SUCCESS;
}
