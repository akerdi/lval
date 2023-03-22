#include <set>
// #include <algorithm>

using namespace std;

#include "lenv.h"

Lenv& Lenv::New_Lenv() {
    Lenv* env = new Lenv;
    env->buildins = new Lval_map;
    env->parent = NULL;
    return *env;
}
Lval& Lenv::lenv_get(Lval& keyVal) {
    Lval_map::iterator pos = buildins->find(keyVal.symbol);
    if (pos != buildins->end()) return pos->second->lval_copy();

    if (parent) return parent->lenv_get(keyVal);

    return Lval::lval_err("Unbound Function for %s", keyVal.symbol.c_str());
}
Lenv& Lenv::lenv_copy() {
    Lenv& env = Lenv::New_Lenv();
    env.buildins = new Lval_map;
    for (Lval_map::iterator it = buildins->begin(); it != buildins->end(); it++) {
        env.lenv_put((*it).first, (*it).second->lval_copy());
    }
    if (parent) env.parent = parent;
    return env;
}
void Lenv::lenv_put(const string key, Lval& val) {
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
void Lenv::lenv_def(string key, Lval& val) {
    Lenv* that = this;
    while (that->parent) { that = that->parent; }
    that->lenv_put(key, val);
}

void Lenv::buildin_funcs(string key, Lval::Lval_Func func) {
    Lval& funcVal = Lval::lval_func(func);
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
    buildin_funcs("def", &Lval::buildin_def);
    buildin_funcs("=", &Lval::buildin_put);
    buildin_funcs("\\", &Lval::buildin_lambda);
}
void Lenv::lenv_delete() {
    if (buildins) delete buildins; buildins = NULL;
    if (parent) parent = NULL;
    delete this;
}
