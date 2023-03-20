#pragma once

#include <iostream>
#include <vector>

typedef enum {
    LVAL_TYPE_ERR,
    LVAL_TYPE_NUM,
    LVAL_TYPE_SYM,
    LVAL_TYPE_SEXPR,
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

    Lval& lval_add(Lval& a);
    Lval& lval_pop(uint32_t index);
    Lval& lval_take(uint32_t index);
    Lval& lval_eval();
    Lval& buildin_op(Lval& sym);
    void lval_println();
    void lval_delete();

    static Lval& lval_err(std::string);
    static Lval& lval_sym(std::string);
    static Lval& lval_check_num(std::string);
    static Lval& lval_num(int);
    static Lval& lval_sexpr(void);
private:
    Lval() {};
    ~Lval() {};

    Lval& lval_expr_eval();

    void lval_print();
    void lval_expr_print(char open, char close);
    void lval_expr_delete();
};
