// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+
// $Id: $

#include "udf.h"
#include "ast.h"

using namespace std;


CompoundFunction::CompoundFunction(const Ftk* F,
                                   const string &name,
                                   const Tplate::Ptr tp,
                                   const vector<string> &vars)
    : Function(F, name, tp, vars),
      vmgr_(F)
{
}

void CompoundFunction::init()
{
    Function::init();
    /*TODO
    vector<string> rf = UdfContainer::get_cpd_rhs_components(type_formula,true);
    init_components(rf);
    */
}

void CompoundFunction::init_components(vector<string>& rf)
{
    vmgr_.silent = true;
    for (int j = 0; j != nv(); ++j)
        vmgr_.assign_variable(varnames[j], ""); // mirror variables

    for (vector<string>::iterator i = rf.begin(); i != rf.end(); ++i) {
        for (int j = 0; j != nv(); ++j) {
            replace_words(*i, tp_->pars[j], vmgr_.get_variable(j)->xname);
        }
        /*TODO
        vmgr_.assign_func("", get_typename_from_formula(*i),
                              get_varnames_from_formula(*i));
        */
    }
}

void CompoundFunction::set_var_idx(vector<Variable*> const& variables)
{
    VariableUser::set_var_idx(variables);
    for (int i = 0; i < nv(); ++i)
        vmgr_.get_variable(i)->set_original(variables[get_var_idx(i)]);
}

/// vv_ was changed, but not variables, mirror variables in vmgr_ must be frozen
void CompoundFunction::precomputations_for_alternative_vv()
{
    vector<Variable const*> backup(nv());
    for (int i = 0; i < nv(); ++i) {
        //prevent change in use_parameters()
        backup[i] = vmgr_.get_variable(i)->freeze_original(vv_[i]);
    }
    vmgr_.use_parameters();
    for (int i = 0; i < nv(); ++i) {
        vmgr_.get_variable(i)->set_original(backup[i]);
    }
}

void CompoundFunction::more_precomputations()
{
    vmgr_.use_parameters();
}

void CompoundFunction::calculate_value_in_range(vector<fp> const &xx,
                                                vector<fp> &yy,
                                                int first, int last) const
{
    vector_foreach (Function*, i, vmgr_.functions())
        (*i)->calculate_value_in_range(xx, yy, first, last);
}

void CompoundFunction::calculate_value_deriv_in_range(
                                             vector<fp> const &xx,
                                             vector<fp> &yy, vector<fp> &dy_da,
                                             bool in_dx,
                                             int first, int last) const
{
    vector_foreach (Function*, i, vmgr_.functions())
        (*i)->calculate_value_deriv_in_range(xx, yy, dy_da, in_dx, first, last);
}

string CompoundFunction::get_current_formula(string const& x) const
{
    string t;
    vector_foreach (Function*, i, vmgr_.functions()) {
        if (i != vmgr_.functions().begin())
            t += "+";
        t += (*i)->get_current_formula(x);
    }
    return t;
}

bool CompoundFunction::get_center(fp* a) const
{
    vector<Function*> const& ff = vmgr_.functions();
    bool r = ff[0]->get_center(a);
    if (!r)
        return false;
    for (size_t i = 1; i < ff.size(); ++i) {
        fp b;
        r = ff[i]->get_center(&b);
        if (!r || is_neq(*a, b))
            return false;
    }
    return true;
}

/// if consists of >1 functions and centers are in the same place
///  height is a sum of heights
bool CompoundFunction::get_height(fp* a) const
{
    vector<Function*> const& ff = vmgr_.functions();
    if (ff.size() == 1)
        return ff[0]->get_height(a);
    fp ctr;
    if (!get_center(&ctr))
        return false;
    fp sum = 0;
    for (size_t i = 0; i < ff.size(); ++i) {
        if (!ff[i]->get_height(a))
            return false;
        sum += *a;
    }
    *a = sum;
    return true;
}

bool CompoundFunction::get_fwhm(fp* a) const
{
    vector<Function*> const& ff = vmgr_.functions();
    if (ff.size() == 1)
        return ff[0]->get_fwhm(a);
    return false;
}

bool CompoundFunction::get_area(fp* a) const
{
    vector<Function*> const& ff = vmgr_.functions();
    fp sum = 0;
    for (size_t i = 0; i < ff.size(); ++i)
        if (ff[i]->get_area(a))
            sum += *a;
        else
            return false;
    *a = sum;
    return true;
}

bool CompoundFunction::get_nonzero_range(fp level, fp& left, fp& right) const
{
    vector<Function*> const& ff = vmgr_.functions();
    if (ff.size() == 1)
        return ff[0]->get_nonzero_range(level, left, right);
    else
        return false;
}

///////////////////////////////////////////////////////////////////////

CustomFunction::CustomFunction(const Ftk* F,
                               const string &name,
                               const Tplate::Ptr tp,
                               const vector<string> &vars)
    : Function(F, name, tp, vars),
      // don't use nv() here, it's not set until init()
      derivatives_(vars.size()+1),
      afo_(tp->op_trees, value_, derivatives_)
{
}


void CustomFunction::set_var_idx(vector<Variable*> const& variables)
{
    VariableUser::set_var_idx(variables);
    afo_.tree_to_bytecode(var_idx.size());
}


void CustomFunction::more_precomputations()
{
    afo_.prepare_optimized_codes(vv_);
}

void CustomFunction::calculate_value_in_range(vector<fp> const &xx,
                                              vector<fp> &yy,
                                              int first, int last) const
{
    for (int i = first; i < last; ++i) {
        yy[i] += afo_.run_vm_val(xx[i]);
    }
}

void CustomFunction::calculate_value_deriv_in_range(
                                           vector<fp> const &xx,
                                           vector<fp> &yy, vector<fp> &dy_da,
                                           bool in_dx,
                                           int first, int last) const
{
    int dyn = dy_da.size() / xx.size();
    for (int i = first; i < last; ++i) {
        afo_.run_vm_der(xx[i]);

        if (!in_dx) {
            yy[i] += value_;
            vector_foreach (Multi, j, multi_)
                dy_da[dyn*i+j->p] += derivatives_[j->n] * j->mult;
            dy_da[dyn*i+dyn-1] += derivatives_.back();
        }
        else {
            vector_foreach (Multi, j, multi_)
                dy_da[dyn*i+j->p] += dy_da[dyn*i+dyn-1]
                                       * derivatives_[j->n] * j->mult;
        }
    }
}

///////////////////////////////////////////////////////////////////////

SplitFunction::SplitFunction(const Ftk* F,
                             const string &name,
                             const Tplate::Ptr tp,
                             const vector<string> &vars)
    : CompoundFunction(F, name, tp, vars)
{
}

void SplitFunction::init()
{
    Function::init();
    vector<string> rf;
    /*TODO
    vector<string> rf = UdfContainer::get_if_then_else_parts(type_formula,true);
    */
    string split_expr = rf[0].substr(rf[0].find('<') + 1);
    rf.erase(rf.begin());
    init_components(rf);
    for (int j = 0; j != nv(); ++j)
        replace_words(split_expr, tp_->pars[j],
                                  vmgr_.get_variable(j)->xname);
    vmgr_.assign_variable("", split_expr);
}

void SplitFunction::calculate_value_in_range(vector<fp> const &xx,
                                             vector<fp> &yy,
                                             int first, int last) const
{
    double xsplit = vmgr_.variables().back()->get_value();
    int t = lower_bound(xx.begin(), xx.end(), xsplit) - xx.begin();
    vmgr_.get_function(0)->calculate_value_in_range(xx, yy, first, t);
    vmgr_.get_function(1)->calculate_value_in_range(xx, yy, t, last);
}

void SplitFunction::calculate_value_deriv_in_range(
                                          vector<fp> const &xx,
                                          vector<fp> &yy, vector<fp> &dy_da,
                                          bool in_dx,
                                          int first, int last) const
{
    double xsplit = vmgr_.variables().back()->get_value();
    int t = lower_bound(xx.begin(), xx.end(), xsplit) - xx.begin();
    vmgr_.get_function(0)->
        calculate_value_deriv_in_range(xx, yy, dy_da, in_dx, first, t);
    vmgr_.get_function(1)->
        calculate_value_deriv_in_range(xx, yy, dy_da, in_dx, t, last);
}

string SplitFunction::get_current_formula(string const& x) const
{
    double xsplit = vmgr_.variables().back()->get_value();
    return "x < " + S(xsplit)
        + " ? " + vmgr_.get_function(0)->get_current_formula(x)
        + " : " + vmgr_.get_function(1)->get_current_formula(x);
}

bool SplitFunction::get_height(fp* a) const
{
    vector<Function*> const& ff = vmgr_.functions();
    fp h2;
    return ff[0]->get_height(a) && ff[1]->get_height(&h2) && is_eq(*a, h2);
}

bool SplitFunction::get_center(fp* a) const
{
    vector<Function*> const& ff = vmgr_.functions();
    fp c2;
    return ff[0]->get_center(a) && ff[1]->get_center(&c2) && is_eq(*a, c2);
}

bool SplitFunction::get_nonzero_range(fp level, fp& left, fp& right) const
{
    vector<Function*> const& ff = vmgr_.functions();
    fp dummy;
    return ff[0]->get_nonzero_range(level, left, dummy) &&
           ff[0]->get_nonzero_range(level, dummy, right);
}

