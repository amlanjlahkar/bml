#include <editline/readline.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/mpc/mpc.c"

long eval (mpc_ast_t*);
long eval_op (long, char*, long);

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
            operator : '+' | '-' | '*' | '/' | '%' | '^' ;      \
            expr     : <number> | '(' <operator> <expr>+ ')' ;  \
            toosty   : /^/ <operator> <expr>+ /$/ ;             \
        ",
        Number, Operator, Expr, Toosty);
    // clang-format on

    while (true)
        {
            char* ibuffer = readline ("toosty> ");
            add_history (ibuffer);

            mpc_result_t r;

            if (strcmp (ibuffer, "exit") == 0)
                {
                    free (ibuffer);
                    break;
                }

            if (mpc_parse ("<stdin>", ibuffer, Toosty, &r))
                {
                    // mpc_ast_print (r.output);
                    long result = eval (r.output);
                    printf ("%li\n", result);
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

long
eval (mpc_ast_t* t)
{
    if (strstr (t->tag, "number")) return strtol (t->contents, NULL, 10);

    char* op = t->children[1]->contents;

    long x = eval (t->children[2]);

    size_t i = 3;
    while (strstr (t->children[i]->tag, "expr"))
        {
            x = eval_op (x, op, eval (t->children[i]));
            i++;
        }

    if (!(i ^ 3) && strcmp (op, "-") == 0) return --x;
    if (!(i ^ 3) && strcmp (op, "+") == 0) return ++x;

    return x;
}

long
eval_op (long x, char* op, long y)
{
    if (strcmp (op, "+") == 0) { return x + y; }
    if (strcmp (op, "-") == 0) { return x - y; }
    if (strcmp (op, "*") == 0) { return x * y; }
    if (strcmp (op, "/") == 0) { return x / y; }
    if (strcmp (op, "%") == 0) { return x % y; }
    if (strcmp (op, "^") == 0) { return pow (x, y); }
    return 0;
}
