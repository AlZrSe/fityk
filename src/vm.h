// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+
// $Id: $

/// virtual machine - calculates expressions using by executing bytecode

#ifndef FITYK_VM_H_
#define FITYK_VM_H_

#include "fityk.h" // struct Point
using fityk::Point;

class Ftk;

/// operators used in VM code
enum Op
{
    // constant
    OP_NUMBER,
    // Custom symbol that has associated numeric value.
    // Like OP_NUMBER, OP_SYMBOL is followed by index,
    // but the value is stored externally.
    OP_SYMBOL,

    // ops used only in calc.cpp
    OP_X,
    OP_PUT_VAL,
    OP_PUT_DERIV,

    // functions R -> R
    OP_ONE_ARG,
    OP_NEG = OP_ONE_ARG,
    OP_EXP,
    OP_ERFC,
    OP_ERF,
    OP_SIN,
    OP_COS,
    OP_TAN,
    OP_SINH,
    OP_COSH,
    OP_TANH,
    OP_ASIN,
    OP_ACOS,
    OP_ATAN,
    OP_LOG10,
    OP_LN,
    OP_SQRT,
    OP_GAMMA,
    OP_LGAMMA,
    OP_DIGAMMA,
    OP_ABS,
    OP_ROUND,

    // functions (R, R) -> R
    OP_TWO_ARG,
    OP_POW = OP_TWO_ARG,
    OP_MUL,
    OP_DIV,
    OP_ADD,
    OP_SUB,
    OP_VOIGT,
    OP_DVOIGT_DX,
    OP_DVOIGT_DY,
    OP_MOD,
    OP_MIN2,
    OP_MAX2,
    OP_RANDNORM,
    OP_RANDU,

    // comparisons (R, R) -> 0/1
    OP_GT, OP_GE, OP_LT, OP_LE, OP_EQ, OP_NEQ,

    // boolean
    // R -> 0/1
    OP_NOT,
    // evaluate first arg and either discard it or skip the second expression
    OP_OR, OP_AFTER_OR, OP_AND, OP_AFTER_AND,
    // evaluate condition and one of two expressions
    OP_TERNARY, OP_TERNARY_MID, OP_AFTER_TERNARY,

    // functions (R, points) -> R
    OP_XINDEX,

    // properties of points
    OP_PX, OP_PY, OP_PS, OP_PA,
    OP_Px, OP_Py, OP_Ps, OP_Pa,
    OP_Pn, OP_PM,

    // changing points
    OP_ASSIGN_X, OP_ASSIGN_Y, OP_ASSIGN_S, OP_ASSIGN_A,

    // Fityk function-objects
    OP_FUNC, OP_SUM_F, OP_SUM_Z,

    // (model, R, ...) -> R
    OP_NUMAREA, OP_FINDX, OP_FIND_EXTR,

    // these two are not VM operators, but are handy to have here,
    // they and are used in implementation of shunting yard algorithm
    OP_OPEN_ROUND, OP_OPEN_SQUARE
};

/// handles VM data and provides low-level access to it
class VirtualMachineData
{
public:
    const std::vector<int>& code() const { return code_; }
    const std::vector<double>& numbers() const { return numbers_; }

    void append_code(int op) { code_.push_back(op); }
    void append_number(double d);
    void clear_data() { code_.clear(); numbers_.clear(); }
    void replace_symbols(const std::vector<double>& vv);
private:
    std::vector<int> code_;    //  VM code
    std::vector<double> numbers_;  //  VM data (numeric values)
};

std::string vm2str(const std::vector<int>& code,
                   const std::vector<double>& data);


class ExprCalculator
{
public:
    ExprCalculator(const Ftk* F) : F_(F) {}

    /// calculate value of expression that may depend on dataset
    double calculate(int n, const std::vector<Point>& points) const;

    /// calculate value of expression that does not depend on dataset
    double calculate() const { return calculate(0, std::vector<Point>()); }

    /// calculate value of expression that was parsed with custom_vars set,
    /// the values in custom_val should correspond to names in custom_vars
    double calculate_custom(const std::vector<double>& custom_val) const;

    /// transform data (X=..., Y=..., S=..., A=...)
    void transform_data(std::vector<Point>& points);

    const VirtualMachineData& vm() const { return vm_; }

protected:
    const Ftk* F_;
    VirtualMachineData vm_;
};

#endif // FITYK_VM_H_

