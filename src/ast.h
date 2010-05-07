// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+
// $Id$

#ifndef FITYK__AST__H__
#define FITYK__AST__H__

#include <boost/spirit/core.hpp>
#include <boost/spirit/tree/ast.hpp>
using namespace boost::spirit;

/// used for functions and variables
/// there is a different set of opcodes for data transformation
enum
{
    OP_CONSTANT=0,
    OP_VARIABLE,
    OP_X,
    OP_PUT_VAL,
    OP_PUT_DERIV,
    OP_ONE_ARG,
    OP_NEG = OP_ONE_ARG,
    OP_EXP,
    OP_ERFC,
    OP_ERF,
    OP_SINH,
    OP_COSH,
    OP_TANH,
    OP_SIN,
    OP_COS,
    OP_TAN,
    OP_ASIN,
    OP_ACOS,
    OP_ATAN,
    OP_LOG10,
    OP_LN,
    OP_SQRT,
    OP_LGAMMA,
    OP_DIGAMMA,
    OP_ABS,
    OP_TWO_ARG,
    OP_POW = OP_TWO_ARG,
    OP_MUL,
    OP_DIV,
    OP_ADD,
    OP_SUB,
    OP_VOIGT,
    OP_DVOIGT_DX,
    OP_DVOIGT_DY,
    OP_END
};


/// Node in abstract syntax tree (AST)
struct OpTree
{
    int op;   /// op < 0: variable (n=-op-1)
              /// op == 0: constant
              /// op > 0: operator
    OpTree *c1,
           *c2;
    fp val;

    explicit OpTree(fp v) : op(0), c1(0), c2(0), val(v) {}
    explicit OpTree(int n, const std::string &/*s*/)
                            : op(-n-1), c1(0), c2(0), val(0.) {}
    explicit OpTree(int n, OpTree *arg1);
    explicit OpTree(int n, OpTree *arg1, OpTree *arg2);

    ~OpTree() { delete c1; delete c2; }
    std::string str(const std::vector<std::string> *vars=NULL);
    std::string str_b(bool b, const std::vector<std::string> *vars)
                            { return b ? "(" + str(vars) + ")" : str(vars); }
    std::string ascii_tree(int width=64, int start=0,
                           const std::vector<std::string> *vars=NULL);
    OpTree* copy() const;
    //void swap_args() { assert(c1 && c2); OpTree *t=c1; c1=c2; c2=t; }
    OpTree* remove_c1() { OpTree *t=c1; c1=0; return t; }
    OpTree* remove_c2() { OpTree *t=c2; c2=0; return t; }
    void change_op(int op_) { op=op_; }
    bool operator==(const OpTree &t) {
        return op == t.op && val == t.val
               && (c1 == t.c1 || (c1 && t.c1 && *c1 == *t.c1))
               && (c2 == t.c2 || (c2 && t.c2 && *c2 == *t.c2));
    }
};

std::vector<std::string>
find_tokens_in_ptree(int tokenID, const tree_parse_info<> &info);

typedef tree_match<char const*>::const_tree_iterator const_tm_iter_t;
std::vector<OpTree*> calculate_deriv(const_tm_iter_t const &i,
                                     std::vector<std::string> const &vars);
fp get_constant_value(std::string const &s);
size_t get_derivatives_str(const char* formula, std::string& result);

#endif

