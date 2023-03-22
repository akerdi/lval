#pragma once

#include <iostream>
#include <vector>

typedef enum {
    LVAL_TYPE_ERR,
    LVAL_TYPE_NUM,
    LVAL_TYPE_SYM,
    LVAL_TYPE_SEXPR,
    LVAL_TYPE_QEXPR,
    LVAL_TYPE_FUNC,
} LVAL_TYPE;

class Lenv;
class Lval {
public:
    typedef std::vector<Lval*> Lvalv;
    typedef Lval&(Lval::*Lval_Func)(Lenv&, Lval&);
    LVAL_TYPE type;
    std::string error;
    std::string symbol;
    int num;
    Lvalv* cells;
    Lval_Func func;

    Lval& lval_add(Lval&);
    Lval& lval_pop(uint32_t);
    Lval& lval_take(uint32_t);
    Lval& lval_copy();
    Lval& lval_eval(Lenv&);
    Lval& buildin_head(Lenv&, Lval&);
    Lval& buildin_tail(Lenv&, Lval&);
    Lval& buildin_list(Lenv&, Lval&);
    Lval& buildin_eval(Lenv&, Lval&);
    Lval& buildin_concat(Lenv&, Lval&);
    Lval& buildin_add(Lenv&, Lval&);
    Lval& buildin_sub(Lenv&, Lval&);
    Lval& buildin_mul(Lenv&, Lval&);
    Lval& buildin_div(Lenv&, Lval&);
    Lval& buildin_def(Lenv&, Lval&);
    void lval_println();
    void lval_delete();

    static Lval& lval_err(std::string, ...);
    static Lval& lval_sym(std::string);
    static Lval& lval_check_num(std::string);
    static Lval& lval_num(int);
    static Lval& lval_sexpr(void);
    static Lval& lval_qexpr(void);
    static Lval& lval_func(void);
private:
    Lval() {};
    ~Lval() {};

    Lval& buildin_op(Lenv&, string);
    Lval& buildin_var(Lenv&, Lval&, string);
    static char* lval_type2name(LVAL_TYPE);
    Lval& lval_expr_eval(Lenv&);

    void lval_print();
    void lval_expr_print(char, char);
    void lval_expr_delete();
};
