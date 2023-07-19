#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "parsing.h"

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

void tval_del(tval* v) {
    switch (v->type) {
        case TVAL_NUM: break;
        case TVAL_ERR: free(v->err); break;
        case TVAL_SYM: free(v->sym); break;
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
    return errno != ERANGE ? tval_num(num) : tval_err("Invalid number");
}

tval* tval_read_ast(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) return tval_read_num(t);
    if (strstr(t->tag, "symbol")) return tval_sym(t->contents);

    tval* v = NULL;
    if (strcmp(t->tag, ">")) v = tval_sexpr();
    if (strcmp(t->tag, "sexpr")) v = tval_sexpr();

    for (size_t i = 0; i < t->children_num; i++) {
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

tval* operate(tval* v, const char* sym) {
    for (size_t i = 0; i < v->count; i++) {
        if (v->list[i]->type != TVAL_NUM) {
            tval_del(v);
            return tval_err("Can not operate on non number!");
        }
    }

    tval* current = tval_pop(v, 0);
    while (v->count > 0) {
        tval* next = tval_pop(v, 0);

        if (strcmp(sym, "+") == 0) current->num += next->num;
        if (strcmp(sym, "-") == 0) current->num -= next->num;
        if (strcmp(sym, "*") == 0) current->num *= next->num;
        if (strcmp(sym, "^") == 0) current->num = pow(current->num, next->num);
        if (strcmp(sym, "/") == 0) {
            if (next->num == 0) {
                tval_del(next), tval_del(current);
                current = tval_err("Division by zero isn't allowed!");
                break;
            }
            current->num /= next->num;
        }

        if (strcmp(sym, "min") == 0)
            current->num =
                (current->num < next->num) ? current->num : next->num;

        if (strcmp(sym, "max") == 0)
            current->num =
                (current->num > next->num) ? current->num : next->num;

        tval_del(next);
    }

    tval_del(v);
    return current;
}

tval* tval_eval_sexpr(tval* v) {
    if (v->count == 0) return v;
    if (v->count == 1) return tval_throw(v, 0);

    for (size_t i = 0; i < v->count; i++)
        v->list[i] = tval_eval(v->list[i]);

    for (size_t i = 0; i < v->count; i++)
        if (v->list[i]->type == TVAL_ERR) return tval_throw(v, i);

    tval* init = tval_pop(v, 0);
    if (init->type != TVAL_SYM) {
        tval_del(init), tval_del(v);
        return tval_err("S-Expression doesn't start with a symbol!");
    }
    tval* result = operate(v, init->sym);
    tval_del(init);
    return result;
}

tval* tval_eval(tval* v) {
    if (v->type == TVAL_SEXPR) return tval_eval_sexpr(v);
    return v;
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
    }
}

void tval_println(tval* v) { tval_print(v), putchar('\n'); }
