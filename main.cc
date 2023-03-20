#include <iostream>

using namespace std;

#include <stdio.h>
#include "config.h"
#include "lval.h"

#ifdef USE_MY_COMPILER
#include <compiler.h>
#endif

void readline(string& input) {
    getline(cin, input);
}

Lval& readAst2Lval(AStruct& t) {
    if (Ast_Type_Number == t.type) return Lval::lval_check_num(t.content);
    if (Ast_Type_Symbol == t.type) return Lval::lval_sym(t.content);
    Lval *x;
    if (Ast_Type_Program == t.type) x = &Lval::lval_sexpr();
    else if (Ast_Type_SExpr == t.type) x = &Lval::lval_sexpr();

    for (ASTptrVector::iterator it = t.children->begin(); it != t.children->end(); it++) {
        AStruct *child_t = *it;
        if (Ast_Type_Semi == child_t->type) continue;
        if (Ast_type_Quote == child_t->type) continue;
        if (Ast_Type_Comment == child_t->type) continue;
        Lval& child = readAst2Lval(*child_t);
        x->lval_add(child);
    }
    return *x;
}

int main(int argc, char** argv) {
    printf("%s VERSION: %d.%d\n", argv[0], APP_VERSION_MAJOR, APP_VERSION_MINOR);
#ifdef USE_MY_COMPILER
    while (true) {
        cout << "> ";
        string input;
        readline(input);
        AStruct &program = AKCompiler::compiler(input);
        Lval& expr = readAst2Lval(program);
        program.deleteNode();

        Lval& res = expr.lval_eval();
        res.lval_println();
        res.lval_delete();
    }
#else
    printf("not implement compiler yet!\n");
    return 1;
#endif
    return 0;
}
