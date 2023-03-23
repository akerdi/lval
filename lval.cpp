#include <iostream>
using namespace std;

#include <errno.h>

#include "lval.h"
#include "lenv.h"

#define NewLval Lval* lval = new Lval
#define LVAL_ASSERT(x, that, ...) if (!(x)) { Lval& err = Lval::lval_err(__VA_ARGS__); that.lval_delete(); return err; }
#define LVAL_ASSERT_NUM(func, x, expect) LVAL_ASSERT(x.cells->size() == expect, x, \
    "Function '%s', Pass Invalid Count of Args. Expect %d, Got %d", func, expect, x.cells->size() \
    )
#define LVAL_ASSERT_TYPE(func, x, index, expect) LVAL_ASSERT(expect == x.cells->at(index)->type, x, \
    "Function '%s', Pass Invalid Type. Expect %s, Got %s", func, Lval::lval_type2name(expect), Lval::lval_type2name(x.cells->at(index)->type) \
    )
#define LVAL_ASSERT_NOT_EMPTY(func, x, index) LVAL_ASSERT(0 != x.cells->at(index)->cells->size(), x, \
    "Function '%s', Pass '{}'.", func \
    )
#define LVAL_FATAL_ERROR(func, msg) cout << __func__ << "::" << func << msg << endl; exit(EXIT_FAILURE);

#pragma mark - Functions

Lval& lval_fast_copy(const Lval& from, Lval& to) {
    to.type = from.type;
    switch (to.type) {
        case LVAL_TYPE_NUM:
            to.num = from.num;
            break;
        case LVAL_TYPE_ERR:
            to.error = from.error;
            break;
        case LVAL_TYPE_SYM:
            to.symbol = from.symbol;
            break;
        case LVAL_TYPE_FUNC:
            if (from.func) {
                to.func = from.func;
            } else {
                to.func = NULL;
                to.formals = &from.formals->lval_copy();
                to.body = &from.body->lval_copy();
                to.env = &from.env->lenv_copy();
            }
            break;
        case LVAL_TYPE_QEXPR:
        case LVAL_TYPE_SEXPR: {
            to.cells = new Lval::Lvalv;
            for (Lval::Lvalv::iterator it = from.cells->begin(); it != from.cells->end(); it++) {
                Lval& lval = Lval::lval_sym("");
                lval_fast_copy(*(*it), lval);
                to.lval_add(lval);
            }
        }
            break;
    }
    return to;
}

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
    return lval_fast_copy(*this, *lval);
}
bool Lval::operator==(const Lval& other) {
    if (type != other.type) return false;
    switch (type) {
        case LVAL_TYPE_ERR: return error == other.error;
        case LVAL_TYPE_NUM: return num == other.num;
        case LVAL_TYPE_SYM: return symbol == other.symbol;
        case LVAL_TYPE_FUNC: {
            if (func) {
                return func == other.func;
            } else {
                if (formals->cells->size() != other.formals->cells->size()) return false;
                if (body->cells->size() != other.cells->size()) return false;
                Lvalv::iterator this_it = formals->cells->begin();
                Lvalv::iterator other_it = other.formals->cells->begin();
                Lval *thisObj, *otherObj;
                for (int i = 0; i < formals->cells->size(); i++) {
                    thisObj = *(this_it + i);
                    otherObj = *(other_it + i);
                    if (!(thisObj == otherObj)) return false;
                }
                this_it = body->cells->begin();
                other_it = other.body->cells->begin();
                for (int i = 0; i < body->cells->size(); i++) {
                    thisObj = *(this_it + i);
                    otherObj = *(other_it + i);
                    if (!(thisObj == otherObj)) return false;
                }
                return true;
            }
        }
        case LVAL_TYPE_QEXPR:
        case LVAL_TYPE_SEXPR: {
            if (cells->size() != other.cells->size()) return false;
            Lvalv::iterator this_it = cells->begin();
            Lvalv::iterator other_it = other.cells->begin();
            Lval *thisObj, *otherObj;
            for (int i = 0; i < cells->size(); i++) {
                thisObj = *(this_it + i);
                otherObj = *(other_it + i);
                if (!(thisObj == otherObj)) return false;
            }
            return true;
        }
        default: {
            LVAL_FATAL_ERROR("compare", " Unbound type: " + type);
        }
    }
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
Lval& Lval::buildin_var(Lenv& env, Lval& expr, string op) {
    LVAL_ASSERT_TYPE(op.c_str(), (expr), 0, LVAL_TYPE_QEXPR);
    LVAL_ASSERT_NOT_EMPTY(op.c_str(), (expr), 0);
    Lval* qexpr = expr.cells->at(0);
    Lval::Lvalv::iterator qit = qexpr->cells->begin();
    for (int i = 0; i < qexpr->cells->size(); i++) {
        Lval* node = *(qit+i);
        if (LVAL_TYPE_SYM != node->type) {
            Lval& err = Lval::lval_err("Function '%s' pass invalid type within args: %s. at index %d.", op.c_str(), Lval::lval_type2name(node->type), i);
            expr.lval_delete();
            return err;
        }
    }
    LVAL_ASSERT((
        qexpr->cells->size() == (expr.cells->size()-1)),
        expr,
        "Function '%s' pass count of args not equal to vals. Args: %d, Vals: %d.",
        op.c_str(),
        qexpr->cells->size(),
        (expr.cells->size()-1));

    qit = qexpr->cells->begin();
    Lval::Lvalv::iterator sit = expr.cells->begin()+1;
    for (int i = 0; i < qexpr->cells->size(); i++) {
        Lval* keyVal = *(qit+i);
        Lval& valVal = (*(sit+i))->lval_copy();
        env.lenv_def(keyVal->symbol, valVal);
        if ("def" == op) env.lenv_def(keyVal->symbol, valVal);
        else env.lenv_put(keyVal->symbol, valVal);
    }
    expr.lval_delete();

    return Lval::lval_sexpr();
}
Lval& Lval::buildin_put(Lenv& env, Lval& expr) {
    return buildin_var(env, (expr), "=");
}
Lval& Lval::buildin_def(Lenv& env, Lval& expr) {
    return buildin_var(env, (expr), "def");
}
Lval& Lval::buildin_lambda(Lenv& env, Lval& expr) {
    LVAL_ASSERT_NUM("\\", expr, 2);
    LVAL_ASSERT_TYPE("\\", expr, 0, LVAL_TYPE_QEXPR);
    LVAL_ASSERT_TYPE("\\", expr, 1, LVAL_TYPE_QEXPR);

    Lval* formals = expr.cells->at(0);
    Lval* body = expr.cells->at(1);

    Lval& lambda = Lval::lval_lambda(formals->lval_copy(), body->lval_copy());
    expr.lval_delete();

    return lambda;
}
Lval& Lval::buildin_order(Lenv& env, Lval& expr, const char* op) {
    LVAL_ASSERT_NUM(op, expr, 2);
    LVAL_ASSERT_TYPE(op, expr, 0, LVAL_TYPE_NUM);
    LVAL_ASSERT_TYPE(op, expr, 1, LVAL_TYPE_NUM);
    Lval* a = expr.cells->at(0);
    Lval* b = expr.cells->at(1);
    Lval* x;
    if (">" == op) {
        x = &Lval::lval_num(a->num > b->num ? 1 : 0);
    } else if (">=" == op) {
        x = &Lval::lval_num(a->num >= b->num ? 1 : 0);
    } else if ("<" == op) {
        x = &Lval::lval_num(a->num < b->num ? 1 : 0);
    } else if ("<=" == op) {
        x = &Lval::lval_num(a->num <= b->num ? 1 : 0);
    }
    expr.lval_delete();

    return *x;
}
Lval& Lval::buildin_gt(Lenv& env, Lval& expr) {
    return buildin_order(env, expr, ">");
}
Lval& Lval::buildin_gte(Lenv& env, Lval& expr) {
    return buildin_order(env, expr, ">=");
}
Lval& Lval::buildin_lt(Lenv& env, Lval& expr) {
    return buildin_order(env, expr, "<");
}
Lval& Lval::buildin_lte(Lenv& env, Lval& expr) {
    return buildin_order(env, expr, "<=");
}
Lval& Lval::buildin_compare(Lenv& env, Lval& expr, const char* op) {
    LVAL_ASSERT_NUM(op, expr, 2);
    Lval* a = expr.cells->at(0);
    Lval* b = expr.cells->at(1);
    bool ret = *a == *b;
    Lval* res;
    if ("==" == op) {
        res = &Lval::lval_num(ret);
    } else if ("!=" == op) {
        res = &Lval::lval_num(!ret);
    } else {
        LVAL_FATAL_ERROR(op, "Not found type: " + string(op));
    }
    expr.lval_delete();

    return *res;
}
Lval& Lval::buildin_eq(Lenv& env, Lval& expr) {
    return buildin_compare(env, expr, "==");
}
Lval& Lval::buildin_neq(Lenv& env, Lval& expr) {
    return buildin_compare(env, expr, "!=");
}
Lval& Lval::buildin_if(Lenv& env, Lval& expr) {
    LVAL_ASSERT_NUM("if", expr, 3);
    LVAL_ASSERT_TYPE("if", expr, 0, LVAL_TYPE_NUM);
    LVAL_ASSERT_TYPE("if", expr, 1, LVAL_TYPE_QEXPR);
    LVAL_ASSERT_TYPE("if", expr, 2, LVAL_TYPE_QEXPR);
    Lval& numVal = expr.lval_pop(0);
    int num = numVal.num;
    numVal.lval_delete();

    expr.cells->at(0)->type = LVAL_TYPE_SEXPR;
    expr.cells->at(1)->type = LVAL_TYPE_SEXPR;

    Lval* res;
    if (num == 0) {
        res = &expr.lval_pop(1).lval_eval(env);
    } else {
        res = &expr.lval_pop(0).lval_eval(env);
    }

    expr.lval_delete();

    return *res;
}
Lval& Lval::lval_call(Lenv& env, Lval& op) {
    if (op.func) {
        // funvVal.*func is a pointer that canot run property
        // Declare afunc type is `Lval_Func`, to prevent call a function pointer and leet to Exception
        Lval_Func afunc = op.func;
        return (op.*afunc)(env, *this);
    }
    int args_count = op.formals->cells->size();
    int params_count = cells->size();
    int i = 0;
    while (i != params_count) {
        if (!op.formals->cells->size()) {
            this->lval_delete();
            return Lval::lval_err("Function '%s' pass params too much to args. Params %d, Args %d", "call", params_count, args_count);
        }
        Lval* keyVal = op.formals->cells->at(i);
        if ("&" == keyVal->symbol) {
            if (2 != args_count-i) {
                this->lval_delete();
                return Lval::lval_err("Function '%s' symbol '&' must follow only one symbol!", "call");
            }
            keyVal = op.formals->cells->at(++i);
            Lval& qexpr = Lval::lval_qexpr();
            Lvalv::iterator it = cells->begin() + i;
            while (it != cells->end()) {
                qexpr.lval_add((*it)->lval_copy());
                it++;
            }
            op.env->lenv_put(keyVal->symbol, qexpr);
            i = args_count;
            break;
        }
        Lval* valVal = cells->at(i);
        op.env->lenv_put(keyVal->symbol, valVal->lval_copy());
        i++;
    }
    this->lval_delete();
    if ((2 == args_count-i) && ("&" == op.formals->cells->at(i)->symbol)) {
        Lval* keyVal = op.formals->cells->at(i+1);
        Lval& emptyList = Lval::lval_qexpr();
        op.env->lenv_put(keyVal->symbol, emptyList);
        i = args_count;
    }
    // cut used args
    Lvalv::iterator it = op.formals->cells->begin();
    op.formals->cells->erase(it, it+i);
    // args had fully used
    if (i >= args_count) {
        op.env->parent = &env;
        Lval& sexpr = Lval::lval_sexpr();
        sexpr.lval_add(op.body->lval_copy());
        return buildin_eval(*op.env, sexpr);
    }
    // or return a copy
    return op.lval_copy();
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

    Lval& res = lval_call(env, funcVal);

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
            if (func) {
                cout << Lval::lval_type2name(LVAL_TYPE_FUNC) << ":" << &func;
            } else {
                putchar('\\');
                putchar(' ');
                putchar('{');
                int i, size;
                Lvalv::iterator it;

                i = 0; size = formals->cells->size(); it = formals->cells->begin();
                for (; i < size; i++) {
                    (*(it + i))->lval_print();
                    if (i != (size-1)) {
                        putchar(' ');
                    }
                }
                putchar('}');
                putchar(' ');
                putchar('{');

                i = 0; size = body->cells->size(); it = body->cells->begin();
                for (i = 0; i < size; i++) {
                    (*(it + i))->lval_print();
                    if (i != (size-1)) {
                        putchar(' ');
                    }
                }
                putchar('}');
            }
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
            if (func) {
                func = NULL;
            } else {
                if (formals) formals->lval_delete();
                if (body) body->lval_delete();
                if (env) env->lenv_delete();
            }
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
Lval& Lval::lval_func(Lval_Func func) {
    NewLval;
    lval->type = LVAL_TYPE_FUNC;
    lval->func = func;
    return *lval;
}
Lval& Lval::lval_lambda(Lval& formals, Lval& body) {
    Lval& lval = lval_func(nullptr);
    lval.formals = &formals;
    lval.body = &body;
    lval.env = &Lenv::New_Lenv();
    return lval;
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
