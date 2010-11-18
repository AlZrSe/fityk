// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+
// $Id$
#ifndef FITYK__FUNC__H__
#define FITYK__FUNC__H__

#include <map>
#include "mgr.h"
#include "var.h"

class Ftk;

class Function : public VariableUser
{
public:

    enum HowDefined
    {
        kCoded, // built-in, coded in C++
        kInterpreted, // built-in formula interpreted as UDF (see default_udfs)
        kUserDefined // defined by user
    };

    struct Multi
    {
        int p; int n; fp mult;
        Multi(int n_, Variable::ParMult const& pm)
            : p(pm.p), n(n_), mult(pm.mult) {}
    };

    std::string const type_formula; //eg. Gaussian(a,b,c) = a*(...)
    std::string const type_name;
    std::string const type_rhs;

    Function(Ftk const* F_,
             std::string const &name_,
             std::vector<std::string> const &vars,
             std::string const &formula_);
    static Function* factory(Ftk const* F,
                             std::string const &name_,
                             std::string const &type_name,
                             std::vector<std::string> const &vars);
    static std::vector<std::string> get_all_types();
    static std::string get_formula(int n);
    static std::string get_formula(std::string const& type);
    static HowDefined how_defined(int n);

    static std::string get_typename_from_formula(std::string const &formula)
     {return strip_string(std::string(formula, 0, formula.find_first_of("(")));}
    static std::string get_rhs_from_formula(std::string const &formula);
    static std::vector<std::string>
      get_varnames_from_formula(std::string const& formula);
    static std::vector<std::string>
      get_defvalues_from_formula(std::string const& formula);

    /// number of variables
    int nv() const { return (int) type_params_.size(); }

    /// calculate value at x[i] and _add_ the result to y[i] (for each i)
    virtual void calculate_value_in_range(std::vector<fp> const &x,
                                          std::vector<fp> &y,
                                          int first, int last) const = 0;
    void calculate_value(std::vector<fp> const &x, std::vector<fp> &y) const;
    fp calculate_value(fp x) const; ///wrapper around array version
    /// calculate function value assuming function parameters has given values
    virtual void calculate_values_with_params(std::vector<fp> const& x,
                                              std::vector<fp>& y,
                                          std::vector<fp> const& alt_vv) const;

    virtual void calculate_value_deriv_in_range(std::vector<fp> const &x,
                                                std::vector<fp> &y,
                                                std::vector<fp> &dy_da,
                                                bool in_dx,
                                                int first, int last) const = 0;
    void calculate_value_deriv(std::vector<fp> const &x,
                               std::vector<fp> &y,
                               std::vector<fp> &dy_da,
                               bool in_dx=false) const;

    void do_precomputations(std::vector<Variable*> const &variables);
    virtual void more_precomputations() {}
    void erased_parameter(int k);
    virtual bool get_nonzero_range(fp/*level*/, fp&/*left*/, fp&/*right*/) const
                                                              { return false; }

    virtual bool has_center() const { return center_idx_ != -1; }
    virtual fp center() const
                        { return center_idx_ == -1 ? 0. : vv_[center_idx_]; }
    virtual bool has_height() const { return false; }
    virtual fp height() const { return 0; }
    virtual bool has_fwhm() const { return false; }
    virtual fp fwhm() const { return 0; }
    virtual bool has_area() const { return false; }
    virtual fp area() const { return 0; }
    bool has_iwidth() const { return this->has_area() && this->has_height(); }
    fp iwidth() const { fp h=this->height(); return h ? this->area()/h : 0.; }
    virtual std::vector<std::string> get_other_prop_names() const
                                    { return std::vector<std::string>(); }
    /// has function other properties (e.g. like Lorentzian-FWHM of Voigt)
    bool has_other_props() const { return !get_other_prop_names().empty(); }
    /// check if has "other" property named `name' defined
    bool has_other_prop(std::string const& name);
    /// get other property, first check with has_other_prop
    virtual fp other_prop(std::string const&) const { return 0; }
    /// return ready-to-display string with all other properties
    std::string other_props_str() const;
    fp get_var_value(int n) const
             { assert(n>=0 && n<size(vv_)); return vv_[n]; }
    std::vector<fp> get_var_values() const  { return vv_; }
    std::string get_par_info(VariableManager const* mgr) const;
    std::string get_basic_assignment() const;
    std::string get_current_assignment(std::vector<Variable*> const &variables,
                                       std::vector<fp> const &parameters) const;
    bool has_outdated_type() const
        { return type_formula != Function::get_formula(type_name); }
    virtual std::string get_current_formula(std::string const& x = "x") const;
    std::string const& get_param(int n) const { return type_params_[n]; }
    int get_param_nr(std::string const& param) const;
    int get_param_nr_nothrow(std::string const& param) const;
    fp get_param_value(std::string const& param) const;
    /// similar to get_param_value(), but doesn't throw exceptions and doesn't
    /// search for pseudo-parameters
    bool get_param_value_nothrow(std::string const& param, fp &value) const;
    fp numarea(fp x1, fp x2, int nsteps) const;
    fp find_x_with_value(fp x1, fp x2, fp val, int max_iter=1000) const;
    fp find_extremum(fp x1, fp x2, int max_iter=1000) const;
    virtual std::string get_bytecode() const { return "No bytecode"; }
    virtual void precomputations_for_alternative_vv()
                                            { this->more_precomputations(); }
protected:
    Ftk const* F_;
    std::vector<fp> vv_; /// current variable values
    std::vector<Multi> multi_;
    std::vector<std::string> type_params_;
    int center_idx_;

    virtual void init();

private:
    static std::vector<fp> calc_val_xx, calc_val_yy;
};

//////////////////////////////////////////////////////////////////////////

class VarArgFunction : public Function
{
    friend class Function;
protected:
    VarArgFunction(Ftk const* F_,
                   std::string const &name_,
                   std::vector<std::string> const &vars,
                   std::string const &formula_)
        : Function(F_, name_, vars, formula_) {}
    virtual void init();
};

#endif

