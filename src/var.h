// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+

#ifndef FITYK__VAR__H__
#define FITYK__VAR__H__

#include "common.h"
#include "vm.h"

struct OpTree;
class Variable;
class Function;
class Sum;

class VariableUser
{
public:
    const std::string name;
    const std::string prefix;

    VariableUser(const std::string &name_, std::string const &prefix_,
              const std::vector<std::string> &vars = std::vector<std::string>())
        : name(name_), prefix(prefix_), varnames(vars) {}
    virtual ~VariableUser() {}
    bool is_auto_delete() const { return name.size() > 0 && name[0] == '_'; }

    bool is_dependent_on(int idx, const std::vector<Variable*> &variables)const;
    bool is_directly_dependent_on(int idx) const
                                  { return contains_element(var_idx, idx); }

    virtual void set_var_idx(const std::vector<Variable*>& variables);
    int get_var_idx(int n) const
             { assert(n >= 0 && n < size(var_idx)); return var_idx[n]; }
    int get_max_var_idx();
    int get_vars_count() const { return varnames.size(); }
    const std::vector<std::string>& get_varnames() const { return varnames; }
    std::string get_var_name(int n) const
             { assert(n >= 0 && n < size(varnames)); return varnames[n]; }
    void substitute_param(int n, const std::string &new_p)
             { assert(n >= 0 && n < size(varnames)); varnames[n] = new_p; }
    std::string get_debug_idx_info() const;

protected:
    std::vector<std::string> varnames; // variable names
    /// var_idx is set after initialization (in derived class)
    /// and modified after variable removal or change
    std::vector<int> var_idx;
};

/// the variable is either simple-variable and nr_ is the index in vector
/// of parameters, or it is "compound variable" and has nr_==-1.
/// third special case: nr_==-2 - it is mirror-variable (such variable
///        is not recalculated but copied)
/// In the second case, the value and derivatives are calculated:
/// -  string is parsed by eparser to VMData representation,
///    and then it is transformed to AST (struct OpTree), calculating derivates
///    at the same time (calculate_deriv()).
/// -  set_var_idx() finds positions of variables in variables vector
///    (references to variables are kept using names) and creates bytecode
///    (VMData, again) that will be used to calculate value and derivatives.
/// -  recalculate() calculates (using run_code_for_variable()) value
///    and derivatives for current parameter value
class Variable : public VariableUser
{
public:
    RealRange domain;

    struct ParMult { int p; realt mult; };
    Variable(const std::string &name_, int nr_);
    Variable(const std::string &name_, const std::vector<std::string> &vars_,
             const std::vector<OpTree*> &op_trees_);
    ~Variable();
    void recalculate(const std::vector<Variable*> &variables,
                     const std::vector<realt> &parameters);

    int get_nr() const { return nr_; };
    void erased_parameter(int k);
    realt get_value() const { return value_; };
    std::string get_formula(const std::vector<realt> &parameters) const;
    bool is_visible() const { return true; } //for future use
    void set_var_idx(const std::vector<Variable*> &variables);
    const std::vector<ParMult>& recursive_derivatives() const
                                            { return recursive_derivatives_; }
    bool is_simple() const { return nr_ != -1; }
    bool is_constant() const;

    std::vector<OpTree*> const& get_op_trees() const { return op_trees_; }
    void set_original(const Variable* orig) { assert(nr_==-2); original_=orig; }
    realt get_derivative(int n) const { return derivatives_[n]; }

private:
    int nr_; /// see description of this class in .h
    realt value_;
    std::vector<realt> derivatives_;
    std::vector<ParMult> recursive_derivatives_;
    std::vector<OpTree*> op_trees_;
    VMData vm_;
    Variable const* original_;
};

#endif
