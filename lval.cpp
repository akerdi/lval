
using namespace std;

#include <errno.h>

#include "lval.h"

#define NewLval Lval* lval = new Lval

Lval& Lval::lval_err(string err) {
    NewLval;
    lval->type = LVAL_TYPE_ERR;
    lval->error = err;
    return *lval;
}
Lval& Lval::lval_sym(string sym) {
    NewLval;
    lval->type = LVAL_TYPE_SYM;
    lval->symbol = sym;
    return *lval;
}
Lval& Lval::lval_check_num(std::string numStr) {
    errno = 0;
    int num = strtol(numStr.c_str(), NULL, 10);
    return errno > 0 ? lval_err("Invalid number!") : lval_num(num);
}
Lval& Lval::lval_num(uint32_t num) {
    NewLval;
    lval->type = LVAL_TYPE_NUM;
    lval->num = num;
    return *lval;
}
Lval& Lval::lval_sexpr(void) {
    NewLval;
    lval->type = LVAL_TYPE_SEXPR;
    lval->cells = new Lvalv;
    return *lval;
}

Lval& Lval::lval_add(Lval& a) {
    this->cells->push_back(&a);
    return *this;
}
void Lval::lval_expr_print(char open, char close) {
    putchar(open);
    for (Lvalv::iterator it = cells->begin(); it != cells->end();) {
        Lval*node = *it;
        node->lval_print();
        it++;
        if (it != cells->end()) putchar(' ');
    }
    putchar(close);
}
void Lval::lval_print() {
    switch (type) {
        case LVAL_TYPE_ERR: {
            cout << error;
            return;
        }
        case LVAL_TYPE_NUM: {
            cout << num;
            return;
        }
        case LVAL_TYPE_SYM: {
            cout << symbol;
            return;
        }
        case LVAL_TYPE_SEXPR: {
            return lval_expr_print('(', ')');
        }
    }
}
void Lval::lval_println() {
    lval_print();
    putchar('\n');
}

void Lval::lval_expr_delete() {
    for (Lvalv::iterator it = cells->begin(); it != cells->end(); it++) {
        Lval* node = *it;
        node->lval_delete();
    }
    cells->clear();
    delete cells;
}
void Lval::lval_delete() {
    switch (type) {
        case LVAL_TYPE_SEXPR:
            return lval_expr_delete();
        case LVAL_TYPE_ERR:
        case LVAL_TYPE_NUM:
        case LVAL_TYPE_SYM:
            break;
    }
    delete this;
}