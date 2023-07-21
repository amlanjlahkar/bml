#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parsing.h"

#define TASSERT(args, cond, err) \
    if (!(cond)) {               \
        tval_del(args);          \
        return tval_err(err);    \
    }

tval* tval_num(long num) {
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_NUM;
    v->num = num;
    return v;
}

tval* tval_err(const char* err) {
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_ERR;
    v->err = malloc(strlen(err) + 1);
    strcpy(v->err, err);
    return v;
}

tval* tval_sym(const char* sym) {
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_SYM;
    v->sym = malloc(strlen(sym) + 1);
    strcpy(v->sym, sym);
    return v;
}

tval* tval_sexpr(void) {
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_SEXPR;
    v->count = 0;
    v->list = NULL;
    return v;
}

tval* tval_qexpr(void) {
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_QEXPR;
    v->count = 0;
    v->list = NULL;
    return v;
}

void tval_del(tval* v) {
    switch (v->type) {
        case TVAL_NUM: break;
        case TVAL_ERR: free(v->err); break;
        case TVAL_SYM: free(v->sym); break;
        case TVAL_QEXPR:
        case TVAL_SEXPR:
            for (size_t i = 0; i < v->count; i++)
                tval_del(v->list[i]);
            free(v->list);
            break;
    }
    free(v);
}

tval* tval_read_num(mpc_ast_t* t) {
    errno = 0;
    long num = strtol(t->contents, NULL, 10);
    return errno != ERANGE
               ? tval_num(num)
               : tval_err("Couldn't parse s-expression(invalid number)!");
}

tval* tval_read_ast(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) return tval_read_num(t);
    if (strstr(t->tag, "symbol")) return tval_sym(t->contents);

    tval* v = NULL;
    if (strcmp(t->tag, ">") == 0) v = tval_sexpr();
    if (strstr(t->tag, "sexpr")) v = tval_sexpr();
    if (strstr(t->tag, "qexpr")) v = tval_qexpr();

    for (size_t i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "{") == 0) continue;
        if (strcmp(t->children[i]->contents, "}") == 0) continue;
        if (strcmp(t->children[i]->contents, "(") == 0) continue;
        if (strcmp(t->children[i]->contents, ")") == 0) continue;
        if (strcmp(t->children[i]->tag, "regex") == 0) continue;
        v = tval_add(v, tval_read_ast(t->children[i]));
    }

    return v;
}

tval* tval_add(tval* vcurr, tval* vnew) {
    vcurr->count++;
    vcurr->list = realloc(vcurr->list, sizeof(tval*) * vcurr->count);
    vcurr->list[vcurr->count - 1] = vnew;
    return vcurr;
}

tval* tval_pop(tval* v, size_t pos) {
    tval* x = v->list[pos];

    memmove(&v->list[pos], &v->list[pos + 1],
            sizeof(tval*) * (v->count - pos - 1));

    v->count--;
    v->list = realloc(v->list, sizeof(tval*) * v->count);
    return x;
}

tval* tval_throw(tval* v, size_t pos) {
    tval* x = tval_pop(v, pos);
    tval_del(v);
    return x;
}

tval* tval_join(tval* dest, tval* src) {
    while (src->count)
        dest = tval_add(dest, tval_pop(src, 0));

    tval_del(src);
    return dest;
}

tval* op_list(tval* v) {
    v->type = TVAL_QEXPR;
    return v;
}

tval* op_head(tval* q) {
    TASSERT(q, q->count == 1,
            "Couldn't evaluate q-expr('head' only takes single argument!)");
    TASSERT(q, q->list[0]->type == TVAL_QEXPR,
            "Incorrect type passed to 'head'(must be a qexpr!)");
    TASSERT(q, q->list[0]->count > 0, "Got empty qexpr!");

    tval* v = tval_throw(q, 0);
    while (v->count > 1)
        tval_del(tval_pop(v, 1));
    return v;
}

tval* op_tail(tval* q) {
    TASSERT(q, q->count == 1,
            "Couldn't evaluate q-expr('tail' only takes single argument!)");
    TASSERT(q, q->list[0]->type == TVAL_QEXPR,
            "Incorrect type passed to 'tail'(must be a qexpr!)");
    TASSERT(q, q->list[0]->count > 0, "Got empty qexpr!");

    tval* v = tval_throw(q, 0);
    tval_del(tval_pop(v, 0));
    return v;
}

tval* op_join(tval* v) {
    for (size_t i = 0; i < v->count; i++) {
        TASSERT(v, v->list[i]->type == TVAL_QEXPR,
                "Incorrect type passed to 'join'(must be a qexpr!)");
    }

    tval* x = tval_pop(v, 0);
    while (v->count)
        x = tval_join(x, tval_pop(v, 0));

    return x;
}

tval* op_eval(tval* q) {
    TASSERT(q, q->count == 1,
            "Couldn't evaluate q-expr('eval' only takes single argument!)");
    TASSERT(q, q->list[0]->type == TVAL_QEXPR,
            "Incorrect type passed to 'eval'(must be a qexpr!)");

    tval* v = tval_throw(q, 0);
    v->type = TVAL_SEXPR;
    return tval_eval(v);
}

tval* op_min(tval* q) {
    TASSERT(q, q->count == 1,
            "Couldn't evaluate q-expr('min' only takes single argument!)");
    TASSERT(q, q->list[0]->type == TVAL_QEXPR,
            "Incorrect type passed to 'min'(must be a qexpr!)");

    tval* v = tval_throw(q, 0);

    while (v->count > 1) {
        if (v->list[0]->num < v->list[1]->num) {
            tval_del(tval_pop(v, 1));
        } else {
            tval_del(tval_pop(v, 0));
        }
    }

    return v;
}

tval* op_max(tval* q) {
    TASSERT(q, q->count == 1,
            "Couldn't evaluate q-expr('max' only takes single argument!)");
    TASSERT(q, q->list[0]->type == TVAL_QEXPR,
            "Incorrect type passed to 'max'(must be a qexpr!)");

    tval* v = tval_throw(q, 0);

    while (v->count > 1) {
        if (v->list[0]->num > v->list[1]->num) {
            tval_del(tval_pop(v, 1));
        } else {
            tval_del(tval_pop(v, 0));
        }
    }

    return v;
}

tval* op_arith(tval* v, const char* sym) {
    for (size_t i = 0; i < v->count; i++) {
        if (v->list[i]->type != TVAL_NUM) {
            tval_del(v);
            return tval_err("Couldn't evaluate s-expression"
                            "(can not operate on non-number)!");
        }
    }

    tval* curr = tval_pop(v, 0);
    while (v->count > 0) {
        tval* next = tval_pop(v, 0);

        if (strcmp(sym, "+") == 0) curr->num += next->num;
        if (strcmp(sym, "-") == 0) curr->num -= next->num;
        if (strcmp(sym, "*") == 0) curr->num *= next->num;
        if (strcmp(sym, "^") == 0) curr->num = pow(curr->num, next->num);
        if (strcmp(sym, "/") == 0) {
            if (next->num == 0) {
                tval_del(next), tval_del(curr);
                curr = tval_err("Couldn't evaluate s-expression"
                                "(division by zero isn't allowed)!");
                break;
            }
            curr->num /= next->num;
        }
        tval_del(next);
    }

    tval_del(v);
    return curr;
}

tval* operate(tval* v, const char* op) {
    if (strcmp("list", op) == 0) return op_list(v);
    if (strcmp("head", op) == 0) return op_head(v);
    if (strcmp("tail", op) == 0) return op_tail(v);
    if (strcmp("join", op) == 0) return op_join(v);
    if (strcmp("eval", op) == 0) return op_eval(v);
    if (strcmp("min", op) == 0) return op_min(v);
    if (strcmp("max", op) == 0) return op_max(v);
    if (strstr("+-/*^", op)) return op_arith(v, op);

    tval_del(v);
    return tval_err("Undefined function!");
}

tval* tval_eval_sexpr(tval* v) {
    for (size_t i = 0; i < v->count; i++)
        v->list[i] = tval_eval(v->list[i]);

    for (size_t i = 0; i < v->count; i++)
        if (v->list[i]->type == TVAL_ERR) return tval_throw(v, i);

    if (v->count == 0) return v;
    if (v->count == 1) return tval_throw(v, 0);

    tval* init = tval_pop(v, 0);
    if (init->type != TVAL_SYM) {
        tval_del(init), tval_del(v);
        return tval_err(
            "Couldn't parse s-expression(must start with a symbol!)");
    }
    tval* result = operate(v, init->sym);
    tval_del(init);
    return result;
}

tval* tval_eval(tval* v) {
    return (v->type == TVAL_SEXPR) ? tval_eval_sexpr(v) : v;
}

void tval_print_expr(tval* v, const char sym_open, const char sym_close) {
    putchar(sym_open);
    for (size_t i = 0; i < v->count; i++) {
        tval_print(v->list[i]);
        if (i != (v->count - 1)) putchar(' ');
    }
    putchar(sym_close);
}

void tval_print(tval* v) {
    switch (v->type) {
        case TVAL_NUM: printf("%li", v->num); break;
        case TVAL_ERR: printf("Error: %s", v->err); break;
        case TVAL_SYM: printf("%s", v->sym); break;
        case TVAL_SEXPR: tval_print_expr(v, '(', ')'); break;
        case TVAL_QEXPR: tval_print_expr(v, '{', '}'); break;
    }
}

void tval_println(tval* v) { tval_print(v), putchar('\n'); }
