// This file is part of fityk program. Copyright (C) 2005 Marcin Wojdyr
// $Id$

//  Based on ast_calc example from Boost::Spirit by Daniel Nuffer

//TODO:
// CSE in tree (or in VM code?)
// new op: SQR? DUP? STORE/WRITE?
// AST -> VM code
// output VM code for tests
// constant-merge (merge identical constants)



#include <boost/spirit/core.hpp>
#include <boost/spirit/tree/ast.hpp>

#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <cmath>

#include "common.h"
#include "calc.h"
#include "var.h"
#include "datatrans.h"

////////////////////////////////////////////////////////////////////////////
using namespace std;
using namespace boost::spirit;

typedef char const*         iterator_t;
typedef tree_match<iterator_t> parse_tree_match_t;
typedef parse_tree_match_t::tree_iterator iter_t;
typedef parse_tree_match_t::const_tree_iterator const_iter_t;

////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////


OpTree::OpTree(int n, OpTree *arg1) : op(n), c1(arg1), c2(0), val(0.) 
                              { assert(OP_ONE_ARG < n && n < OP_TWO_ARG); }
OpTree::OpTree(int n, OpTree *arg1, OpTree *arg2) 
    : op(n), c1(arg1), c2(arg2), val(0.)   { assert(n > OP_TWO_ARG); }

string OpTree::str(const vector<string> *vars)
{
    if (op < 0) {
        int v_nr = -op-1;
        return vars->empty() ? "var"+S(v_nr) : "$"+(*vars)[v_nr];
    }
    switch (op) {
        case 0:       return S(val);
        case OP_NEG:  return "-" + c1->str_b(c1->op >= OP_POW, vars);
        case OP_EXP:  return "exp(" + c1->str(vars) + ")";
        case OP_SIN:  return "sin(" + c1->str(vars) + ")";
        case OP_COS:  return "cos(" + c1->str(vars) + ")";
        case OP_ATAN: return "atan("+ c1->str(vars) + ")";
        case OP_TAN:  return "tan(" + c1->str(vars) + ")";
        case OP_ASIN: return "asin("+ c1->str(vars) + ")";
        case OP_ACOS: return "acos("+ c1->str(vars) + ")";
        case OP_LOG10:return "log10("+c1->str(vars) + ")";
        case OP_LN:   return "ln("  + c1->str(vars) + ")";
        case OP_SQRT: return "sqrt("+ c1->str(vars) + ")";
        case OP_POW:  return c1->str_b(c1->op >= OP_POW, vars) 
                             + "^" + c2->str_b(c2->op >= OP_POW, vars);
        case OP_ADD:  return c1->str(vars) + "+" + c2->str(vars);
        case OP_SUB:  return c1->str(vars) + "-" 
                                         + c2->str_b(c2->op >= OP_ADD, vars);
        case OP_MUL:  return c1->str_b(c1->op >= OP_ADD, vars) 
                             + "*" + c2->str_b(c2->op >= OP_ADD, vars);
        case OP_DIV:  return c1->str_b(c1->op >= OP_ADD, vars) 
                             + "/" + c2->str_b(c2->op >= OP_MUL, vars);
        default: assert(0); return "";
    }
}

string OpTree::ascii_tree(int width, int start, const vector<string> *vars)
{
    string node = "???";
    if (op < 0) {
        int v_nr = -op-1;
        node = vars->empty() ? "var"+S(v_nr) : (*vars)[v_nr];
    }
    else
        switch (op) {
            case 0:       node = S(val); break;
            case OP_NEG:  node = "NEG";  break;
            case OP_EXP:  node = "EXP";  break;
            case OP_SIN:  node = "SIN";  break;
            case OP_COS:  node = "COS";  break;
            case OP_ATAN: node = "ATAN"; break;
            case OP_TAN:  node = "TAN";  break;
            case OP_ASIN: node = "ASIN"; break;
            case OP_ACOS: node = "ACOS"; break;
            case OP_LOG10:node = "LOG";  break;
            case OP_LN:   node = "LN";   break;
            case OP_SQRT: node = "SQRT"; break;
            case OP_POW:  node = "POW";  break;
            case OP_ADD:  node = "ADD";  break;
            case OP_SUB:  node = "SUB";  break;
            case OP_MUL:  node = "MUL";  break;
            case OP_DIV:  node = "DIV";  break;
        }
    int n = (int(node.size()) < width ? start + (width-node.size())/2
                                 : start);
    node = string(n, ' ') + node + "\n";
    if (c1)
        node += c1->ascii_tree(width/2, start, vars);
    if (c2)
        node += c2->ascii_tree(width/2, start+width/2, vars);
    return node;
}

OpTree* OpTree::copy()
{ 
    OpTree *t = new OpTree(*this); 
    if (c1) t->c1 = c1->copy();
    if (c2) t->c2 = c2->copy();
    return t;
}

////////////////////////////////////////////////////////////////////////////

void do_find_tokens(int tokenID, const_iter_t const &i, vector<string> &vars)
{
    for (const_iter_t j = i->children.begin(); j != i->children.end(); ++j) {
        if (j->value.id() == tokenID) {
            string v(j->value.begin(), j->value.end());
            if (find(vars.begin(), vars.end(), v) == vars.end())
                vars.push_back(v);
        }
        else
            do_find_tokens(tokenID, j, vars);
    }
}

vector<string> find_tokens(int tokenID, const tree_parse_info<> &info)
{
    vector<string> vars;
    const_iter_t const &root = info.trees.begin();
    if (root->value.id() == tokenID) 
        vars.push_back(string(root->value.begin(), root->value.end()));
    else
        do_find_tokens(tokenID, root, vars);
    return vars;
}


////////////////////////////////////////////////////////////////////////////
OpTree* simplify_terms(OpTree *a);
OpTree* do_multiply(OpTree *a, OpTree *b);

OpTree* do_neg(OpTree *a)
{
    if (a->op == 0) {
        double val = - a->val;
        delete a;
        return new OpTree(val);
    }
    else if (a->op == OP_NEG) {
        OpTree *t = a->c1->copy();
        delete a;
        return t;
    }
    else
        return new OpTree(OP_NEG, a);
}

OpTree* do_add(int op, OpTree *a, OpTree *b)
{
    if (a->op == 0 && a->val == 0.) { // 0 + t
        delete a;
        if (op == OP_ADD)
            return b;
        else
            return do_neg(b);
    }
    else if (b->op == 0 && b->val == 0.) { // t + 0
        delete b;
        return a;
    }
    else if (a->op == 0 && b->op == 0) { // p + q
        double val = (op == OP_ADD ? a->val + b->val : a->val - b->val);
        delete a;
        delete b;
        return new OpTree(val);
    }
    else if (b->op == OP_NEG) { // t + -u
        OpTree *t = b->remove_c1();
        delete b;
        return do_add(op == OP_ADD ? OP_SUB : OP_ADD, a, t);
    }
    else if ((b->op == OP_MUL || b->op == OP_DIV)  
             && b->c1->op == 0  && b->c1->val < 0) { // t + -p*v  
        b->c1->val = - b->c1->val;
        return do_add(op == OP_ADD ? OP_SUB : OP_ADD, a, b);
    }
    else if (*a == *b) { 
        delete b;
        if (op == OP_ADD) // t + t
            return do_multiply(new OpTree(2.), a);
        else { // t - t
            delete a;
            return new OpTree(0.);
        }
    }
    else
        return new OpTree(op, a, b);
}

OpTree* do_add(OpTree *a, OpTree *b)
{
    return do_add(OP_ADD, a, b);
}

OpTree* do_sub(OpTree *a, OpTree *b)
{
    return do_add(OP_SUB, a, b);
}

OpTree* do_multiply(OpTree *a, OpTree *b)
{
    if ((a->op == 0 && b->op == 0)
        || (a->op == 0 && a->val == 0.)
        || (b->op == 0 && b->val == 0.))
    {
        double val = a->val * b->val;
        delete a;
        delete b;
        return new OpTree(val);
    }
    else if (a->op == 0 && a->val == 1.) {
        delete a;
        return b;
    }
    else if (b->op == 0 && b->val == 1.) {
        delete b;
        return a;
    }
    else if (a->op == 0 && a->val == -1.) {
        delete a;
        return do_neg(b);
    }
    else if (b->op == 0 && b->val == -1.) {
        delete b;
        return do_neg(a);
    }
    else {
        return new OpTree(OP_MUL, a, b);
    }
}

OpTree* do_divide(OpTree *a, OpTree *b)
{
    if (a->op == 0 && a->val == 0.) {
        delete a;
        delete b;
        return new OpTree(0.);
    }
    else if ((a->op == 0 && b->op == 0) || (b->op == 0 && b->val == 0.))
    {
        double val = a->val / b->val;
        delete a;
        delete b;
        return new OpTree(val);
    }
    else if (b->op == 0 && b->val == 1.) {
        delete b;
        return a;
    }
    else {
        return new OpTree(OP_DIV, a, b);
    }
}

OpTree *do_sqr(OpTree *a)
{
    return do_multiply(a, a->copy());
    //return new OpTree(OP_MUL, a, a->copy());
}

OpTree *do_oneover(OpTree *a)
{
    return do_divide(new OpTree(1.), a);
}


OpTree* do_exp(OpTree *a)
{
    if (a->op == 0) {
        double val = exp(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_EXP, simplify_terms(a));
}

OpTree* do_sqrt(OpTree *a)
{
    if (a->op == 0) {
        double val = sqrt(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_SQRT, a);
}

OpTree* do_log10(OpTree *a)
{
    if (a->op == 0) {
        double val = log10(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_LOG10, simplify_terms(a));
}

OpTree* do_ln(OpTree *a)
{
    if (a->op == 0) {
        double val = log(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_LN, simplify_terms(a));
}

OpTree* do_sin(OpTree *a)
{
    if (a->op == 0) {
        double val = sin(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_SIN, simplify_terms(a));
}

OpTree* do_cos(OpTree *a)
{
    if (a->op == 0) {
        double val = cos(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_COS, simplify_terms(a));
}

OpTree* do_tan(OpTree *a)
{
    if (a->op == 0) {
        double val = tan(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_TAN, simplify_terms(a));
}

OpTree* do_atan(OpTree *a)
{
    if (a->op == 0) {
        double val = atan(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_ATAN, simplify_terms(a));
}

OpTree* do_asin(OpTree *a)
{
    if (a->op == 0) {
        double val = asin(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_ASIN, simplify_terms(a));
}

OpTree* do_acos(OpTree *a)
{
    if (a->op == 0) {
        double val = acos(a->val);
        delete a;
        return new OpTree(val);
    }
    else
        return new OpTree(OP_ACOS, simplify_terms(a));
}

OpTree* do_pow(OpTree *a, OpTree *b)
{
    if (a->op == 0 && a->val == 0.) {
        delete a;
        delete b;
        return new OpTree(0.);
    }
    if ((b->op == 0 && is_eq(b->val, 0.)) 
        || (a->op == 0 && a->val == 1.)) {
        delete a;
        delete b;
        return new OpTree(1.);
    }
    else if (b->op == 0 && is_eq(b->val, 1.)) {
        delete b;
        return a;
    }
    else if (b->op == 0 && is_eq(b->val, -1.)) {
        delete b;
        return do_oneover(a);
    }
    if (a->op == 0 && b->op == 0) {
        double val = pow(a->val, b->val);
        delete a;
        delete b;
        return new OpTree(val);
    }
    else {
        return new OpTree(OP_POW, a, simplify_terms(b));
    }
}

////////////////////////////////////////////////////////////////////////////

struct MultFactor
{
    // factor (*t)^(*e)
    OpTree *t, *e;
    MultFactor(OpTree *t_, OpTree *e_) : t(t_), e(e_) {}
    void clear() { delete t; delete e; t=e=0; }
};


/// recursively walk though OP_MUL, OP_DIV, OP_NEG, OP_SQRT, OP_POW
/// and builds list of nodes with factors, such that tree a is equal to
/// (v[0]->t)^(v[0]->e) * (v[1]->t)^(v[1]->e) * ...
void get_factors(OpTree *a, OpTree *expo, 
                 double &constant, vector<MultFactor>& v)
{
    if (a->op == OP_ADD || a->op == OP_SUB)
        a = simplify_terms(a);
    if (a->op == 0 && expo->op == 0)
        constant *= pow(a->val, expo->val);
    else if (a->op == OP_MUL) {
        get_factors(a->c1, expo, constant, v);
        get_factors(a->c2, expo, constant, v);
    }
    else if (a->op == OP_DIV) {
        get_factors(a->c1, expo, constant, v);
        OpTree *expo2 = do_neg(expo->copy());
        get_factors(a->c2, expo2, constant, v);
        delete expo2;
    }
    else if (a->op == OP_NEG) {
        get_factors(a->c1, expo, constant, v);
        get_factors(new OpTree(-1.), expo, constant, v);
    }
    else if (a->op == OP_SQRT) {
        OpTree *expo2 = do_multiply(new OpTree(0.5), expo->copy());
        get_factors(a->c1, expo2, constant, v);
        delete expo2;
    }
    else if (a->op == OP_POW) {
        OpTree *expo2 = do_multiply(a->remove_c2(), expo->copy());
        get_factors(a->c1, expo2, constant, v);
        delete expo2;
    }
    else {
        bool found = false;
        for (vector<MultFactor>::iterator i = v.begin(); i != v.end(); ++i) 
            if (*i->t == *a) {
                i->e = do_add(i->e, expo->copy());
                found = true;
                break;
            }
            if (!found) {
                v.push_back(MultFactor(a, expo->copy()));
                return; //don't delete a
            }
    }
    //we are here -- MultFactor(a,...) not created
    a->c1 = a->c2 = 0;
    delete a;
}


OpTree* simplify_factors(OpTree *a)
{
#ifdef DEBUG_SIMPLIFY
    cout << "simplify_factors() [<] " << a->str() << endl;
#endif
    vector<MultFactor> v;
    OpTree expo(1.);
    double constant = 1;
    get_factors(a, &expo, constant, v); //deletes a
#ifdef DEBUG_SIMPLIFY
    cout << "simplify_factors(): [.] {" << constant << "} ";
    for (vector<MultFactor>::iterator i = v.begin(); i != v.end(); ++i) 
        cout << "{" << i->t->str() << "|" << i->e->str() << "} ";
    cout << endl;
#endif

    // tan*cos -> sin; tan/sin -> cos
    for (vector<MultFactor>::iterator i = v.begin(); i != v.end(); ++i) 
        if (i->t && i->t->op == OP_TAN) {
            for (vector<MultFactor>::iterator j = v.begin(); j != v.end(); ++j){
                if (j->t && j->t->op == OP_COS && *j->e == *i->e) {
                    i->t->change_op(OP_SIN);
                    j->clear();
                }
                if (j->t && j->t->op == OP_SIN 
                    && ((j->e->op==0 && i->e->op==0 && j->e->val==-i->e->val)
                        || (j->e->op==OP_NEG && *j->e->c1 == *i->e) 
                        || (i->e->op==OP_NEG && *i->e->c1 == *j->e))) {
                    i->t->change_op(OP_COS);
                    j->clear();
                }
            }
        }
    // sin/cos -> tan
    for (vector<MultFactor>::iterator i = v.begin(); i != v.end(); ++i) 
        if (i->t && i->t->op == OP_SIN) {
            for (vector<MultFactor>::iterator j = v.begin(); j != v.end(); ++j){
                if (j->t && j->t->op == OP_COS 
                    && ((j->e->op==0 && i->e->op==0 && j->e->val==-i->e->val)
                        || (j->e->op==OP_NEG && *j->e->c1 == *i->e) 
                        || (i->e->op==OP_NEG && *i->e->c1 == *j->e))) {
                    i->t->change_op(OP_TAN);
                    j->clear();
                }
            }
        }

    // -> tree
    // TODO x^z * y^z -> (x*y)^z  (if z != -1,0,1)
    OpTree *tu = 0, *tb = 0; // preparing expression as (tu / tb)
    for (vector<MultFactor>::iterator i = v.begin(); i != v.end(); ++i) 
        if (i->t) {
            if ((i->e->op == 0 && i->e->val < 0) || i->e->op == OP_NEG) {
                OpTree *p = do_pow(i->t, do_neg(i->e));
                tb = (tb == 0 ? p : do_multiply(tb, p));
            }
            else {
                OpTree *p = do_pow(i->t, i->e);
                tu = (tu == 0 ? p : do_multiply(tu, p));
            }
        }
    OpTree *constant_t = new OpTree(constant);
    OpTree *ret = 0;
    if (tu) {
        if (tb)
            ret = do_multiply(constant_t, do_divide(tu, tb));
        else //tu && !tb
            ret = do_multiply(constant_t, tu);
    }
    else {
        if (tb) //!tu && tb
            ret = do_divide(constant_t, tb);
        else //!tu && !tb 
            ret = constant_t;
    }
#ifdef DEBUG_SIMPLIFY
    cout << "simplify_factors() [>] " << ret->str() << endl;
#endif
    return ret;
}

////////////////////////////////////////////////////////////////////////////

struct MultTerm
{
    OpTree *t; 
    double k;
    MultTerm(OpTree *t_, double k_) : t(t_), k(k_) {}
    void clear() { delete t; t=0; }
};

void get_terms(OpTree *a, double multiplier, vector<MultTerm> &v)
{
    if (a->op == OP_MUL || a->op == OP_DIV || a->op == OP_SQRT 
            || a->op == OP_POW)
        a = simplify_factors(a);
    if (a->op == OP_ADD) {
        get_terms(a->c1, multiplier, v);
        get_terms(a->c2, multiplier, v);
    }
    else if (a->op == OP_SUB) {
        get_terms(a->c1, multiplier, v);
        get_terms(a->c2, -multiplier, v);
    }
    else if (a->op == OP_NEG) {
        get_terms(a->c1, -multiplier, v);
    }
    else if (a->op == OP_MUL && a->c1->op == 0) {
        get_terms(a->c2, multiplier*(a->c1->val), v);
    }
    else if (a->op == OP_DIV && a->c1->op == 0 && !is_eq(a->c1->val,1.)) {
        get_terms(do_oneover(a->c2), multiplier*(a->c1->val), v);
    }
    else {
        bool found = false;
        if (a->op == 0)

        for (vector<MultTerm>::iterator i = v.begin(); i != v.end(); ++i) 
            if (i->t && *i->t == *a) {
                i->k += multiplier;
                found = true;
                break;
            }
            else if (a->op == 0 && i->t && i->t->op == 0) {
                i->k += multiplier * a->val;
                found = true;
                break;
            }
        if (!found) {
            if (a->op == 0)
                v.push_back(MultTerm(new OpTree(1.), multiplier * a->val));
            else {
                v.push_back(MultTerm(a, multiplier));
                return; //don't delete a
            }
        }
    }
    //we are here -- MultTerm(a,...) not created
    a->c1 = a->c2 = 0;
    delete a;
}

OpTree* simplify_terms(OpTree *a)
{
    // not handled:
    //        (x+y) * (x-y) == x^2 - y^2 
    //        (x+/-y)^2 == x^2 +/- 2xy + y^2
#ifdef DEBUG_SIMPLIFY
    cout << "simplify_terms() [<] " << a->str() << endl;
#endif
    vector<MultTerm> v;
    get_terms(a, 1., v); //deletes a
#ifdef DEBUG_SIMPLIFY
    cout << "simplify_terms() [.] ";
    for (vector<MultTerm>::iterator i = v.begin(); i != v.end(); ++i) 
        cout << "{" << i->t->str() << "|" << i->k << "} ";
    cout << endl;
#endif

    // sin^2(x) + cos^2(x) = 1
    double to_add = 0.;
    for (vector<MultTerm>::iterator i = v.begin(); i != v.end(); ++i) 
        if (i->t && i->t->op == OP_POW && i->t->c1->op == OP_SIN 
                && i->t->c2->op == 0 && is_eq(i->t->c2->val, 2.))
            for (vector<MultTerm>::iterator j = v.begin(); j != v.end(); ++j) 
                if (j->t && j->t->op == OP_POW && j->t->c1->op == OP_COS 
                        && j->t->c2->op == 0 && is_eq(j->t->c2->val, 2.)) {
                    double k = j->k;
                    i->k -= k;
                    j->clear();
                    to_add += k;
                }
    if (to_add)
        get_terms(new OpTree(1.), to_add, v);

    // -> tree
    OpTree *t = 0;
    for (vector<MultTerm>::iterator i = v.begin(); i != v.end(); ++i) 
        if (i->t && !is_eq(i->k, 0)) {
            if (!t)
                t = do_multiply(new OpTree(i->k), i->t);
            else if (i->k > 0)
                t = do_add(t, do_multiply(new OpTree(i->k), i->t));
            else //i->k < 0
                t = do_sub(t, do_multiply(new OpTree(-i->k), i->t));
        }
    if (!t)
        t = new OpTree(0.);
#ifdef DEBUG_SIMPLIFY
    cout << "simplify_terms() [>] " << t->str() << endl;
#endif
    return t;
}

////////////////////////////////////////////////////////////////////////////

fp get_constant_value(string const &s)
{
    if (s == "pi")
        return M_PI;
    else if (s[0] == '{') {
        assert(*(s.end()-1) == '}');
        const string expr(s.begin()+1, s.end()-1);
        return get_transform_expression_value(expr);
    }
    else
        return strtod(s.c_str(), 0);
}

/// returns array of trees, 
/// first n=vars.size() derivatives and the last tree for value
vector<OpTree*> calculate_deriv(const_iter_t const &i,
                                vector<string> const &vars)
{
    int len = vars.size();
    vector<OpTree*> results(len + 1);
    string s(i->value.begin(), i->value.end());

    if (i->value.id() == FuncGrammar::real_constID)
    {
        assert(s.size() > 0);
        for (int k = 0; k < len; ++k)
            results[k] = new OpTree(0.); 
        double v = get_constant_value(s);
        results[len] = new OpTree(v);
    }

    else if (i->value.id() == FuncGrammar::variableID)
    {
        for (int k = 0; k < len; ++k)
            if (s == vars[k]) {
                results[k] = new OpTree(1.); 
                results[len] = new OpTree(k, s);
            }
            else
                results[k] = new OpTree(0.); 
    }

    else if (i->value.id() == FuncGrammar::exptokenID)
    {
        vector<OpTree*> arg = calculate_deriv(i->children.begin(), vars);
        if (s == "-")
            for (int k = 0; k < len+1; ++k) 
                results[k] = do_neg(arg[k]);
        else {
            OpTree* (* do_op)(OpTree *) = 0;
            OpTree* der = 0;
            OpTree* larg = arg.back()->copy();
            if (s == "sqrt") {
                der = do_divide(new OpTree(0.5), do_sqrt(larg));
                do_op = do_sqrt;
            }
            else if (s == "exp") {
                der = do_exp(larg);
                do_op = do_exp;
            }
            else if (s == "log10") {
                OpTree *ln_10 = do_ln(new OpTree(10.));
                der = do_oneover(do_multiply(larg, ln_10));
                do_op = do_log10;
            }
            else if (s == "ln") {
                der = do_oneover(larg);
                do_op = do_ln;
            }
            else if (s == "sin") {
                der = do_cos(larg);
                do_op = do_sin;
            }
            else if (s == "cos") {
                der = do_neg(do_sin(larg));
                do_op = do_cos;
            }
            else if (s == "tan") {
                der = do_oneover(do_sqr(do_cos(larg)));
                do_op = do_tan;
            }
            else if (s == "atan") {
                der = do_oneover(do_add(new OpTree(1.), do_sqr(larg)));
                do_op = do_atan;
            }
            else if (s == "asin") {
                OpTree *root_arg = do_sub(new OpTree(1.), do_sqr(larg));
                der = do_oneover(do_sqrt(root_arg));
                do_op = do_asin;
            }
            else if (s == "acos") {
                OpTree *root_arg = do_sub(new OpTree(1.), do_sqr(larg));
                der = do_divide(new OpTree(-1.), do_sqrt(root_arg));
                do_op = do_acos;
            }
            else
                assert(0);
            for (int k = 0; k < len; ++k) 
                results[k] = do_multiply(der->copy(), arg[k]);
            delete der;
            results[len] = (*do_op)(arg[len]);
        }
    }

    else if (i->value.id() == FuncGrammar::factorID)
    {
        assert(s == "^");
        assert(i->children.size() == 2);
        vector<OpTree*> left = calculate_deriv(i->children.begin(), vars);
        vector<OpTree*> right = calculate_deriv(i->children.begin() + 1, vars);
        //special cases like a(x)^n are not handeled separately. Should they?
        for (int k = 0; k < len; ++k) {
            OpTree *a = left[len],
                   *b = right[len],
                   *ap = left[k],
                   *bp = right[k];
            //        b(x)            b(x)  b(x) a'(x)
            //    (a(x)   )'    = a(x)     (---------- + ln(a(x)) b'(x))
            //                                 a(x)
            OpTree *pow_a_b = do_pow(a->copy(), b->copy());
            OpTree *term1 = do_divide(do_multiply(b->copy(), ap), a->copy());
            OpTree *term2 = do_multiply(do_ln(a->copy()), bp);
            results[k] = do_multiply(pow_a_b, do_add(term1, term2));
        }
        results[len] = do_pow(left[len], right[len]);
    }

    else if (i->value.id() == FuncGrammar::termID)
    {
        assert(s == "*" || s == "/");
        assert(i->children.size() == 2);
        int op = (s == "*" ? OP_MUL : OP_DIV);
        vector<OpTree*> left = calculate_deriv(i->children.begin(), vars);
        vector<OpTree*> right = calculate_deriv(i->children.begin() + 1, vars);
        for (int k = 0; k < len; ++k) {
            OpTree *a = left[len],
                   *b = right[len],
                   *ap = left[k],
                   *bp = right[k];
            if (op == OP_MUL) { // a*b' + a'*b
                results[k] = do_add(do_multiply(a->copy(), bp),
                                    do_multiply(ap,  b->copy()));
            }
            else { //OP_DIV    (a'*b - b'*a) / (b*b)
                OpTree *upper = do_sub(do_multiply(ap, b->copy()),
                                       do_multiply(bp, a->copy()));
                results[k] = do_divide(upper, do_sqr(b->copy()));
            }
        }
        results[len] = (op == OP_MUL ? do_multiply(left[len], right[len])
                                     : do_divide(left[len], right[len]));
    }

    else if (i->value.id() == FuncGrammar::expressionID)
    {
        assert(s == "+" || s == "-");
        assert(i->children.size() == 2);
        vector<OpTree*> left = calculate_deriv(i->children.begin(), vars);
        vector<OpTree*> right = calculate_deriv(i->children.begin() + 1, vars);
        for (int k = 0; k < len+1; ++k) 
            results[k] = (s == "+" ? do_add(left[k], right[k]) 
                                   : do_sub(left[k], right[k]));
    }

    else 
        assert(0); // error

    for (int k = 0; k < len+1; ++k) 
        results[k] = simplify_terms(results[k]);

    return results;
}


////////////////////////////////////////////////////////////////////////////

void add_calc_bytecode(const OpTree* tree, const vector<int> &vmvar_idx,
                       vector<int> &vmcode, vector<fp> &vmdata)
{
    int op = tree->op;
    if (op < 0) {
        int var_idx = vmvar_idx[-op-1];
        vmcode.push_back(OP_VARIABLE);
        vmcode.push_back(var_idx);
    }
    else if (op == 0) {
        vmcode.push_back(OP_CONSTANT);
        vmcode.push_back(vmdata.size());
        vmdata.push_back(tree->val);
    }
    else if (op > OP_ONE_ARG && op < OP_TWO_ARG) { //one argument
        add_calc_bytecode(tree->c1, vmvar_idx, vmcode, vmdata);
        vmcode.push_back(op);
    }
    else if (op > OP_TWO_ARG) { //two arguments
        add_calc_bytecode(tree->c1, vmvar_idx, vmcode, vmdata);
        add_calc_bytecode(tree->c2, vmvar_idx, vmcode, vmdata);
        vmcode.push_back(op);
    }
}


