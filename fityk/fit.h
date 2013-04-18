// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+

#ifndef FITYK__FIT__H__
#define FITYK__FIT__H__
#include <vector>
#include <map>
#include <string>
#include <time.h>
#include "common.h"

namespace fityk {

class DataAndModel;
class Ftk;
class Variable;

int count_points(const std::vector<DataAndModel*>& dms);

///   interface of fitting method and implementation of common functions
class FITYK_API Fit
{
public:
    const std::string name;

    Fit(Ftk *F, const std::string& m);
    virtual ~Fit() {};
    void fit(int max_iter, const std::vector<DataAndModel*>& dms);
    std::string get_goodness_info(const std::vector<DataAndModel*>& dms);
    int get_dof(const std::vector<DataAndModel*>& dms);
    std::string get_cov_info(const std::vector<DataAndModel*>& dms);
    std::vector<realt>
        get_covariance_matrix(const std::vector<DataAndModel*>& dms);
    std::vector<realt>
        get_standard_errors(const std::vector<DataAndModel*>& dms);
    std::vector<realt>
        get_confidence_limits(const std::vector<DataAndModel*>& dms,
                              double level_percent);
    const std::vector<DataAndModel*>& get_last_dm() const { return dmdm_; }
    static realt compute_wssr_for_data (const DataAndModel* dm, bool weigthed);
    static int compute_deviates_for_data(const DataAndModel* dm,
                                         double *deviates);
    // called from GUI
    realt compute_wssr(const std::vector<realt> &A,
                       const std::vector<DataAndModel*>& dms,
                       bool weigthed=true);
    static realt compute_r_squared_for_data(const DataAndModel* dm,
                                           realt* sum_err, realt* sum_tot);
    realt compute_r_squared(const std::vector<realt> &A,
                           const std::vector<DataAndModel*>& dms);
    bool is_param_used(int n) const { return par_usage_[n]; }
protected:
    Ftk *F_;
    std::vector<DataAndModel*> dmdm_;
    int evaluations_; // zeroed in fit() initialization, ++'ed in other places
    realt initial_wssr_; // set (only) at the beginning of fit()
    std::vector<realt> a_orig_;
    int na_; ///number of fitted parameters, equal to par_usage_.size()
    // getters, see the comments below for the variables
    int max_eval() const { return max_eval_; }
    const std::vector<bool>& par_usage() const { return par_usage_; }

    virtual double run_method(std::vector<realt>* best_a) = 0;
    std::string iteration_info(realt wssr); // changes best_shown_wssr_
    bool common_termination_criteria() const;
    void compute_derivatives(const std::vector<realt> &A,
                          const std::vector<DataAndModel*>& dms,
                          std::vector<realt>& alpha, std::vector<realt>& beta);
    void compute_derivatives_mp(const std::vector<realt> &A,
                                const std::vector<DataAndModel*>& dms,
                                double **derivs, double *deviates);
    realt compute_derivatives_nl(const std::vector<realt> &A,
                                 const std::vector<DataAndModel*>& dms,
                                 double *grad);
    int compute_deviates(const std::vector<realt> &A, double *deviates);
    realt draw_a_from_distribution(int nr, char distribution = 'u',
                                   realt mult = 1.);
    void iteration_plot(const std::vector<realt> &A, realt wssr);
    void output_tried_parameters(const std::vector<realt>& a);
private:
    int max_eval_; // it is set before calling run_method()
    time_t last_refresh_time_;
    clock_t start_time_;
    std::vector<bool> par_usage_;
    realt best_shown_wssr_; // for iteration_info()

    double elapsed() const; // CPU time elapsed since the start of fit()
    void compute_derivatives_for(const DataAndModel *dm,
                                 std::vector<realt>& alpha,
                                 std::vector<realt>& beta);
    int compute_derivatives_mp_for(const DataAndModel* dm, int offset,
                                   double **derivs, double *deviates);
    realt compute_derivatives_nl_for(const DataAndModel* dm, double *grad);
    void update_par_usage(const std::vector<DataAndModel*>& dms);
};

/// handles parameter history
class FITYK_API ParameterHistoryMgr
{
public:
    ParameterHistoryMgr(Ftk *F) : F_(F), param_hist_ptr_(0) {}
    bool push_param_history(const std::vector<realt>& aa);
    void clear_param_history() { param_history_.clear(); param_hist_ptr_ = 0; }
    int get_param_history_size() const { return param_history_.size(); }
    void load_param_history(int item_nr, bool relative);
    bool has_param_history_rel_item(int rel_nr) const
        { return is_index(param_hist_ptr_ + rel_nr, param_history_); }
    bool can_undo() const;
    std::string param_history_info() const;
    const std::vector<realt>& get_item(int n) const {return param_history_[n];}
    int get_active_nr() const { return param_hist_ptr_; }
protected:
    Ftk *F_;
private:
    std::vector<std::vector<realt> > param_history_; /// old parameter vectors
    int param_hist_ptr_; /// points to the current/last parameter vector
};

/// gives access to fitting methods, enables swithing between them
/// also stores parameter history
class FITYK_API FitMethodsContainer : public ParameterHistoryMgr
{
public:
    static const char* full_method_names[][2];
    FitMethodsContainer(Ftk *F_);
    ~FitMethodsContainer();
    Fit* get_method(const std::string& name) const;
    const std::vector<Fit*>& methods() const { return methods_; }
    realt get_standard_error(const Variable* var) const;
    void outdated_error_cache() { dirty_error_cache_ = true; }

private:
    std::vector<Fit*> methods_;
    mutable std::vector<realt> errors_cache_;
    bool dirty_error_cache_;

    DISALLOW_COPY_AND_ASSIGN(FitMethodsContainer);
};

extern const char* fit_method_enum[]; // used in settings.cpp

} // namespace fityk
#endif

