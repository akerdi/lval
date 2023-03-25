#pragma once

#include <iostream>
#include <deque>

#include "config.h"

#ifdef USE_MY_COMPILER
#include <compiler.h>
#endif

typedef enum {
    LVAL_TYPE_ERR,
    LVAL_TYPE_NUM,
    LVAL_TYPE_SYM,
    LVAL_TYPE_SEXPR,
    LVAL_TYPE_QEXPR,
    LVAL_TYPE_FUNC,
    LVAL_TYPE_STR,
} LVAL_TYPE;

class Lenv;
class Lval {
public:
    typedef std::deque<Lval*> Lvaldq;
    typedef Lval&(Lval::*Lval_Func)(Lenv&, Lval&);
    LVAL_TYPE type;
    std::string error;
    std::string symbol;
    int num;
    Lvaldq* cells;
    Lval_Func func;

    Lenv* env;
    Lval* formals;
    Lval* body;
    std::string str;

    Lval& lval_add(Lval&);
    Lval& lval_pop(uint32_t);
    Lval& lval_pop_front();
    Lval& lval_pop_back();
    Lval& lval_take(uint32_t);
    Lval& lval_copy();
    Lval& lval_eval(Lenv&);
    Lval& lval_call(Lenv&, Lval&);
    void lval_println();
    void lval_delete();

    Lval& buildin_head(Lenv&, Lval&);
    Lval& buildin_tail(Lenv&, Lval&);
    Lval& buildin_list(Lenv&, Lval&);
    Lval& buildin_eval(Lenv&, Lval&);
    Lval& buildin_concat(Lenv&, Lval&);
    Lval& buildin_add(Lenv&, Lval&);
    Lval& buildin_sub(Lenv&, Lval&);
    Lval& buildin_mul(Lenv&, Lval&);
    Lval& buildin_div(Lenv&, Lval&);
    Lval& buildin_put(Lenv&, Lval&);
    Lval& buildin_def(Lenv&, Lval&);
    Lval& buildin_lambda(Lenv&, Lval&);
    Lval& buildin_gt(Lenv&, Lval&);
    Lval& buildin_gte(Lenv&, Lval&);
    Lval& buildin_lt(Lenv&, Lval&);
    Lval& buildin_lte(Lenv&, Lval&);
    Lval& buildin_eq(Lenv&, Lval&);
    Lval& buildin_neq(Lenv&, Lval&);
    Lval& buildin_if(Lenv&, Lval&);
    Lval& buildin_print(Lenv&, Lval&);
    Lval& buildin_load(Lenv&, Lval&);

    bool operator==(const Lval&);

    static Lval& readAst(AStruct& t);
    static Lval& lval_err(std::string, ...);
    static Lval& lval_sym(std::string);
    static Lval& lval_check_num(std::string);
    static Lval& lval_num(int);
    static Lval& lval_sexpr(void);
    static Lval& lval_qexpr(void);
    static Lval& lval_func(Lval_Func);
    static Lval& lval_lambda(Lval&, Lval&);
    static Lval& lval_check_string(const std::string);
    static Lval& lval_string(const std::string);

private:
    Lval& lval_expr_eval(Lenv&);
    bool lval_compare(Lval&);

    Lval& buildin_op(Lenv&, std::string);
    Lval& buildin_var(Lenv&, Lval&, std::string);
    Lval& buildin_order(Lenv&, Lval &, const char*);
    Lval& buildin_compare(Lenv&, Lval&, const char*);

    void lval_print();
    void lval_expr_print(char, char);
    void lval_expr_delete();

    static char* lval_type2name(LVAL_TYPE);
};
