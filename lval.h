#pragma once

#include <iostream>
#include <vector>

typedef enum {
    LVAL_TYPE_ERR,
    LVAL_TYPE_NUM,
    LVAL_TYPE_SYM,
    LVAL_TYPE_SEXPR,
    LVAL_TYPE_QEXPR,
} LVAL_TYPE;

class Lval;
typedef std::vector<Lval*> Lvalv;

class Lval {
public:
    LVAL_TYPE type;
    std::string error;
    std::string symbol;
    int num;
    Lvalv* cells;

    Lval& lval_add(Lval&);
    Lval& lval_pop(uint32_t);
    Lval& lval_take(uint32_t);
    Lval& lval_eval();
    Lval& buildin(Lval&);
    Lval& buildin_head();
    Lval& buildin_tail();
    Lval& buildin_list();
    Lval& buildin_eval();
    Lval& buildin_concat();
    void lval_println();
    void lval_delete();

    static Lval& lval_err(std::string, ...);
    static Lval& lval_sym(std::string);
    static Lval& lval_check_num(std::string);
    static Lval& lval_num(int);
    static Lval& lval_sexpr(void);
    static Lval& lval_qexpr(void);
private:
    Lval() {};
    ~Lval() {};

    Lval& buildin_op(Lval&);
    static char* lval_type2name(LVAL_TYPE);
    Lval& lval_expr_eval();

    void lval_print();
    void lval_expr_print(char, char);
    void lval_expr_delete();
};
