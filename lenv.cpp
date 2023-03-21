#include <map>
#include <set>

using namespace std;

#include "lenv.h"

Lenv& Lenv::New_Lenv() {
    Lenv* env = new Lenv;
    env->buildins = new Lval_map;
    return *env;
}
Lval& Lenv::lenv_get(Lval& keyVal) {
    Lval_map::iterator pos = buildins->find(keyVal.symbol);
    if (pos != buildins->end()) {
        return pos->second->lval_copy();
    }
    return Lval::lval_err("Unbound Function for %s", keyVal.symbol.c_str());
}

void Lenv::lenv_def(string key, Lval& val) {
    Lval_map::iterator pos = buildins->find(key);
    if (pos != buildins->end()) {
        buildins->erase(key);
        buildins->insert(make_pair(key, &val));
        return;
    }
    pair<Lval_map::iterator, bool > ret = buildins->insert(make_pair(key, &val));
    if (!ret.second) {
        cout << __func__ << "::Tried to insert op to an exists pair map!" << endl;
    }
}

void Lenv::buildin_funcs(string key, Lval::Lval_Func func) {
    Lval& funcVal = Lval::lval_func();
    funcVal.func = func;
    lenv_def(key, funcVal);
}
void Lenv::init_buildins() {
    buildin_funcs("+", &Lval::buildin_add);
    buildin_funcs("-", &Lval::buildin_sub);
    buildin_funcs("*", &Lval::buildin_mul);
    buildin_funcs("/", &Lval::buildin_div);
    buildin_funcs("head", &Lval::buildin_head);
    buildin_funcs("tail", &Lval::buildin_tail);
    buildin_funcs("list", &Lval::buildin_list);
    buildin_funcs("eval", &Lval::buildin_eval);
    buildin_funcs("concat", &Lval::buildin_concat);
    Lval& funcVal = Lval::lval_func();

}
void Lenv::lenv_delete() {
    if (buildins) {
        delete buildins;
    }
    delete this;
}
