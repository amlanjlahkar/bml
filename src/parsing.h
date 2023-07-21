#include <stdlib.h>

#include "../lib/mpc/mpc.h"

typedef struct tval {
    size_t type;
    long num;
    char* err;
    char* sym;
    size_t count;
    struct tval** list;
} tval;

enum tval_type { TVAL_NUM, TVAL_ERR, TVAL_SYM, TVAL_SEXPR, TVAL_QEXPR };
// enum tval_type_err { TERR_DIV_ZERO = 1, TERR_BAD_OP = 2, TERR_BAD_NUM = 3 };

/*
** Constructors
*/
tval* tval_num(long new_num);
tval* tval_err(const char* new_err);
tval* tval_sym(const char* new_sym);
tval* tval_sexpr(void);
tval* tval_qexpr(void);

/*
** Helpers
*/
void tval_del(tval*);
tval* tval_read_num(mpc_ast_t*);
tval* tval_read_ast(mpc_ast_t*);
tval* tval_add(tval* vcurr, tval* vnew);
tval* tval_pop(tval*, size_t pos);
tval* tval_throw(tval*, size_t pos);
tval* tval_join(tval* dest, tval* src);

/*
** Operators
*/
tval* op_list(tval*);
tval* op_head(tval*);
tval* op_tail(tval*);
tval* op_join(tval*);
tval* op_eval(tval*);
tval* op_min(tval*);
tval* op_max(tval*);
tval* op_arith(tval*, const char* symbol);
tval* operate(tval*, const char* op);

/*
** Evaluators
*/
tval* tval_eval_sexpr(tval*);
tval* tval_eval(tval*);

void tval_print_expr(tval*, const char sym_open, const char sym_close);
void tval_print(tval*);
void tval_println(tval*);
