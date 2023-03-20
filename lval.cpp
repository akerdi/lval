#include <iostream>
using namespace std;

#include <errno.h>

#include "lval.h"

#define NewLval Lval* lval = new Lval
#define Lval_Assert(x, ...) a

#pragma mark - Methods

Lval& Lval::lval_add(Lval& a) {
    this->cells->push_back(&a);
    return *this;
}
Lval& Lval::lval_pop(uint32_t index) {
    Lval* node = cells->at(index);
    cells->erase(cells->begin()+index);
    return *node;
}
Lval& Lval::lval_take(uint32_t index) {
    Lval& node = lval_pop(index);
    cells->clear();
    return node;
}

Lval& Lval::buildin_op(Lval& sym) {
    for (Lvalv::iterator it = cells->begin(); it != cells->end(); it++) {
        if (LVAL_TYPE_NUM != (*it)->type) {
            Lval& err = Lval::lval_err(
                "Operation must be number! Expect %s, got %s!",
                Lval::lval_type2name(LVAL_TYPE_NUM),
                Lval::lval_type2name((*it)->type)
            );
            this->lval_delete();
            return err;
        }
    }
    Lval* x = &lval_pop(0);
    // .e.g. `- 3` / `- 5`
    if (cells->size() == 0 && "-" == sym.symbol) x->num = -x->num;

    while (cells->size()) {
        Lval& y = lval_pop(0);
        if ("+" == sym.symbol) x->num += y.num;
        else if ("-" == sym.symbol) x->num -= y.num;
        else if ("*" == sym.symbol) x->num *= y.num;
        else if ("/" == sym.symbol) {
            if (0 == y.num) {
                x->lval_delete();
                y.lval_delete();
                x = &lval_err("Division by zero!");
                break;
            }
            x->num /= y.num;
        }
        y.lval_delete();
    }
    lval_delete();
    return *x;
}

Lval& Lval::lval_expr_eval() {
    Lvalv::iterator it;
    for (it = cells->begin(); it != cells->end(); it++) {
        Lval* node = *it;
        *it = &(node->lval_eval());
    }
    it = cells->begin();
    for (int i = 0 ; i < cells->size(); i++) {
        Lval* node = *it;
        if (LVAL_TYPE_ERR == node->type) {
            Lval& node = lval_pop(i);
            this->lval_delete();
            return node;
        }
        it++;
    }
    if (!cells->size()) return *this;
    if (cells->size() == 1) return lval_take(0);
    Lval& sym = lval_pop(0);
    if (LVAL_TYPE_SYM != sym.type) {
        Lval& err = lval_err(
            "SExpr must start by a symbol! Expect %s, got %s",
                Lval::lval_type2name(LVAL_TYPE_SYM),
                Lval::lval_type2name(sym.type)
            );
        this->lval_delete();
        sym.lval_delete();
        return err;
    }
    Lval& res = buildin_op(sym);
    sym.lval_delete();

    return res;
}

Lval& Lval::lval_eval() {
    if (LVAL_TYPE_SEXPR == type) return lval_expr_eval();

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
        case LVAL_TYPE_QEXPR: {
            return lval_expr_print('{', '}');
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
    if (cells) delete cells;
}
void Lval::lval_delete() {
    switch (type) {
        case LVAL_TYPE_QEXPR:
        case LVAL_TYPE_SEXPR:
            return lval_expr_delete();
        case LVAL_TYPE_ERR:
        case LVAL_TYPE_NUM:
        case LVAL_TYPE_SYM:
            break;
    }
    delete this;
}

#pragma mark - Static

Lval& Lval::lval_err(string fmt, ...) {
    NewLval;
    lval->type = LVAL_TYPE_ERR;
    static char buf[255] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt.c_str(), ap);
    va_end(ap);
    lval->error = string(buf);

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
    return errno > 0 ? lval_err("Invalid number: %s", strerror(errno)) : lval_num(num);
}
Lval& Lval::lval_num(int num) {
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
Lval& Lval::lval_qexpr(void) {
    NewLval;
    lval->type = LVAL_TYPE_QEXPR;
    lval->cells = new Lvalv;
    return *lval;
}

char* Lval::lval_type2name(LVAL_TYPE type) {
    switch (type) {
        case LVAL_TYPE_ERR: return "Error";
        case LVAL_TYPE_NUM: return "Number";
        case LVAL_TYPE_SYM: return "Symbol";
        case LVAL_TYPE_SEXPR: return "SEXPR";
        case LVAL_TYPE_QEXPR: return "QEXPR";
        default: "Unknown Type: " + type;
    }
}
