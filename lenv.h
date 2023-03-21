#pragma once

#include <map>

#include "lval.h"

typedef std::map<std::string, Lval*> Lval_map;

class Lenv {
public:
    Lval_map* buildins;

    void lenv_def(string, Lval&);
    // get by keyVal
    // ret valVal if exists.(ret errVal if not exists!)
    Lval& lenv_get(Lval&);

    void init_buildins();
    void lenv_delete();

    static Lenv& New_Lenv();
private:
    void buildin_funcs(string, Lval::Lval_Func);
    Lenv() {}
    ~Lenv() {}
};
