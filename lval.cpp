#include <iostream>
#include <algorithm>
using namespace std;

#include <errno.h>

#include "lval.h"
#include "lenv.h"

#define NewLval Lval* lval = new Lval
#define LVAL_ASSERT(x, ...) if (!(x)) { Lval& err = Lval::lval_err(__VA_ARGS__); lval_delete(); return err; }
#define LVAL_ASSERT_NUM(func, x, expect) LVAL_ASSERT(x.cells->size() == expect, \
    "Function '%s', Pass Invalid Count of Args. Expect %d, Got %d", func, expect, x.cells->size() \
    )
#define LVAL_ASSERT_TYPE(func, x, index, expect) LVAL_ASSERT(expect == x.cells->at(index)->type, \
    "Function '%s', Pass Invalid Type. Expect %s, Got %s", func, Lval::lval_type2name(expect), Lval::lval_type2name(x.cells->at(index)->type) \
    )
#define LVAL_ASSERT_NOT_EMPTY(func, x, index) LVAL_ASSERT(0 != x.cells->at(index)->cells->size(), \
    "Function '%s', Pass '{}'.", func \
    )

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
Lval& Lval::lval_copy() {
    NewLval;
    lval->type = type;
    switch (lval->type) {
        case LVAL_TYPE_NUM:
            lval->num = num;
            break;
        case LVAL_TYPE_ERR:
            lval->error = error;
            break;
        case LVAL_TYPE_SYM:
            lval->symbol = symbol;
            break;
        case LVAL_TYPE_FUNC:
            lval->func = func;
            break;
        case LVAL_TYPE_QEXPR:
        case LVAL_TYPE_SEXPR: {
            lval->cells = new Lvalv;
            lval->cells->resize(cells->size());
            copy(cells->begin(), cells->end(), lval->cells->begin());
        }
            break;
    }
    return *lval;
}

Lval& Lval::buildin_op(Lenv&, string op) {
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
    if (cells->size() == 0 && "-" == op) x->num = -x->num;

    while (cells->size()) {
        Lval& y = lval_pop(0);
        if ("+" == op) x->num += y.num;
        else if ("-" == op) x->num -= y.num;
        else if ("*" == op) x->num *= y.num;
        else if ("/" == op) {
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
Lval& Lval::buildin_head(Lenv& env, Lval& expr) {
    LVAL_ASSERT_NUM("head", (expr), 1);
    LVAL_ASSERT_TYPE("head", (expr), 0, LVAL_TYPE_QEXPR);
    LVAL_ASSERT_NOT_EMPTY("head", (expr), 0);

    Lval& node = expr.lval_take(0);
    expr.lval_delete();

    Lval& res = node.lval_take(0);
    node.lval_delete();

    return res;
}
Lval& Lval::buildin_tail(Lenv& env, Lval& expr) {
    LVAL_ASSERT_NUM("tail", (expr), 1);
    LVAL_ASSERT_TYPE("tail", (expr), 0, LVAL_TYPE_QEXPR);
    LVAL_ASSERT_NOT_EMPTY("tail", (expr), 0);

    Lval& node = expr.lval_take(0);
    expr.lval_delete();

    Lval& res = node.lval_pop(0);
    res.lval_delete();

    return node;
}
Lval& Lval::buildin_list(Lenv& env, Lval& expr) {
    expr.type = LVAL_TYPE_QEXPR;
    return expr;
}
Lval& Lval::buildin_eval(Lenv& env, Lval& expr) {
    LVAL_ASSERT_NUM("eval", (expr), 1);
    LVAL_ASSERT_TYPE("eval", (expr), 0, LVAL_TYPE_QEXPR);
    LVAL_ASSERT_NOT_EMPTY("eval", (expr), 0);

    Lval& node = expr.lval_take(0);
    expr.lval_delete();

    node.type = LVAL_TYPE_SEXPR;
    return node.lval_eval(env);
}
Lval& Lval::buildin_concat(Lenv& env, Lval& expr) {
    LVAL_ASSERT_NUM("concat", (expr), 2);
    LVAL_ASSERT_TYPE("concat", (expr), 0, LVAL_TYPE_QEXPR);
    LVAL_ASSERT_TYPE("concat", (expr), 1, LVAL_TYPE_QEXPR);

    Lval& qexprX = expr.lval_pop(0);
    Lval& qexprY = expr.lval_pop(0);
    expr.lval_delete();
    while (qexprY.cells->size()) {
        Lval& a = qexprY.lval_pop(0);
        qexprX.lval_add(a);
    }
    qexprY.lval_delete();

    return qexprX;
}
Lval& Lval::buildin_add(Lenv& env, Lval& expr) {
    return expr.buildin_op(env, "+");
}
Lval& Lval::buildin_sub(Lenv& env, Lval& expr) {
    return expr.buildin_op(env, "-");
}
Lval& Lval::buildin_mul(Lenv& env, Lval& expr) {
    return expr.buildin_op(env, "*");
}
Lval& Lval::buildin_div(Lenv& env, Lval& expr) {
    return expr.buildin_op(env, "/");
}
Lval& Lval::lval_expr_eval(Lenv& env) {
    Lvalv::iterator it;
    for (it = cells->begin(); it != cells->end(); it++) {
        Lval* node = *it;
        *it = &(node->lval_eval(env));
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

    Lval& funcVal = lval_pop(0);
    if (LVAL_TYPE_FUNC != funcVal.type) {
        Lval& err = lval_err(
            "SExpr must start by Function type! Expect %s, got %s",
                Lval::lval_type2name(LVAL_TYPE_FUNC),
                Lval::lval_type2name(funcVal.type)
            );
        this->lval_delete();
        funcVal.lval_delete();
        return err;
    }
    // funvVal.*func is a pointer that canot run property
    // Declare afunc type is `Lval_Func`, to prevent call a function pointer and leet to Exception
    Lval_Func afunc = funcVal.func;
    Lval& res = (funcVal.*afunc)(env, *this);

    funcVal.lval_delete();

    return res;
}

Lval& Lval::lval_eval(Lenv& env) {
    if (LVAL_TYPE_SYM == type) {
        Lval& res = env.lenv_get(*this);

        this->lval_delete();
        return res;
    }
    if (LVAL_TYPE_SEXPR == type) return lval_expr_eval(env);

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
        case LVAL_TYPE_FUNC: {
            cout << &func;
            return;
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
    if (cells) delete cells; cells = NULL;
}
void Lval::lval_delete() {
    switch (type) {
        case LVAL_TYPE_QEXPR:
        case LVAL_TYPE_SEXPR:
            return lval_expr_delete();
        case LVAL_TYPE_FUNC:
            func = NULL;
            break;
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
    Lval& lval = Lval::lval_sexpr();
    lval.type = LVAL_TYPE_QEXPR;
    return lval;
}
Lval& Lval::lval_func(void) {
    NewLval;
    lval->type = LVAL_TYPE_FUNC;
    lval->func = NULL;
    return *lval;
}

char* Lval::lval_type2name(LVAL_TYPE type) {
    switch (type) {
        case LVAL_TYPE_ERR:     return "Error";
        case LVAL_TYPE_NUM:     return "Number";
        case LVAL_TYPE_SYM:     return "Symbol";
        case LVAL_TYPE_SEXPR:   return "SEXPR";
        case LVAL_TYPE_QEXPR:   return "QEXPR";
        case LVAL_TYPE_FUNC:    return "Function";
        default: "Unknown Type: " + type;
    }
}
