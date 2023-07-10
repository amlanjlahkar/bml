#include <editline/readline.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/mpc/mpc.c"

typedef struct {
    int type;
    long val;
    int err;
} tval;

enum tval_type_res { TVAL_NUM, TVAL_ERR };
enum tval_type_err { TERR_DIV_ZERO = 1, TERR_BAD_OP = 2, TERR_BAD_NUM = 3 };

tval eval(mpc_ast_t*);
tval eval_op(tval, char* op, tval);
tval tval_res(long);
tval tval_err(int);
void tval_print(tval);

int main(int argc, char** argv) {
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Toosty   = mpc_new("toosty");

    // clang-format off
    mpca_lang (MPCA_LANG_DEFAULT,
        "                                                                       \
            number   : /-?[0-9]+/ ;                                             \
            operator : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\" ;  \
            expr     : <number> | '(' <operator> <expr>+ ')' ;                  \
            toosty   : /^/ <operator> <expr>+ /$/ ;                             \
        ",
        Number, Operator, Expr, Toosty);
    // clang-format on

    while (true) {
        char* ibuffer = readline("toosty> ");
        add_history(ibuffer);

        mpc_result_t r;

        if (strcmp(ibuffer, "exit") == 0) {
            free(ibuffer);
            break;
        }

        if (mpc_parse("<stdin>", ibuffer, Toosty, &r)) {
            void* ast = r.output;
            // mpc_ast_print(ast);
            tval result = eval(ast);
            tval_print(result);
            mpc_ast_delete(ast);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(ibuffer);
    }

    mpc_cleanup(4, Number, Operator, Expr, Toosty);

    return EXIT_SUCCESS;
}

tval eval(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        errno  = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? tval_res(x) : tval_err(TERR_BAD_NUM);
    }

    char* op = t->children[1]->contents;

    tval x = eval(t->children[2]);

    size_t i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    if (!(i ^ 3) && strcmp(op, "-") == 0) x.val -= 1;
    if (!(i ^ 3) && strcmp(op, "+") == 0) x.val += 1;

    return x;
}

tval eval_op(tval x, char* op, tval y) {
    if (x.type == TVAL_ERR) return x;
    if (y.type == TVAL_ERR) return y;

    if (strcmp(op, "+") == 0) return tval_res(x.val + y.val);
    if (strcmp(op, "-") == 0) return tval_res(x.val - y.val);
    if (strcmp(op, "*") == 0) return tval_res(x.val * y.val);
    if (strcmp(op, "^") == 0) return tval_res(pow(x.val, y.val));

    if (strcmp(op, "/") == 0)
        return y.val == 0 ? tval_err(TERR_DIV_ZERO) : tval_res(x.val / y.val);

    if (strcmp(op, "%") == 0)
        return y.val == 0 ? tval_err(TERR_DIV_ZERO) : tval_res(x.val % y.val);

    if (strcmp(op, "min") == 0)
        return tval_res((x.val < y.val) ? x.val : y.val);

    if (strcmp(op, "max") == 0)
        return tval_res((x.val > y.val) ? x.val : y.val);

    return tval_err(TERR_BAD_OP);
}

tval tval_res(long result) {
    tval v;
    v.type = TVAL_NUM;
    v.val  = result;
    v.err = 0;
    return v;
}

tval tval_err(int err) {
    tval v;
    v.type = TVAL_ERR;
    v.err  = err;
    return v;
}

void tval_print(tval v) {
    switch (v.type) {
        case TVAL_NUM:
            printf("%li\n", v.val);
            break;

        case TVAL_ERR:
            switch (v.err) {
                case TERR_DIV_ZERO:
                    puts("Error: Attempted division by zero!");
                    break;
                case TERR_BAD_OP:
                    puts("Error: Invalid operator!");
                    break;
                case TERR_BAD_NUM:
                    puts("Error: Invalid number!");
                    break;
            }
            break;
    }
}
