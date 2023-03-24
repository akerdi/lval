#include <iostream>

using namespace std;

#include <stdio.h>
#include "config.h"
#include "lval.h"
#include "lenv.h"

void readline(string& input) {
    getline(cin, input);
}

int main(int argc, char** argv) {
    printf("%s VERSION: %d.%d\n", argv[0], APP_VERSION_MAJOR, APP_VERSION_MINOR);
#ifdef USE_MY_COMPILER
    Lenv& env = Lenv::New_Lenv();
    env.init_buildins();
    while (true) {
        cout << "> ";
        string input;
        readline(input);
        AStruct &program = AKCompiler::compiler(input);
        if (Ast_Type_Error == program.type) {
            program.print();
            program.deleteNode();
            continue;
        }
        Lval& expr = Lval::readAst(program);
        program.deleteNode();

        Lval& res = expr.lval_eval(env);

        res.lval_println();
        res.lval_delete();
    }
    env.lenv_delete();
#else
    printf("not implement compiler yet!\n");
    return 1;
#endif
    return 0;
}
