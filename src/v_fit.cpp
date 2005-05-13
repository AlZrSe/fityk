// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#include "common.h"
#include "v_fit.h"
#include <algorithm>
#include <sstream>
#include <time.h>
#include <math.h>
#include "other.h"
#include "pcore.h"
#include "sum.h"
#include "data.h"
#include "ui.h"
#include "LMfit.h"
#include "GAfit.h"
#include "NMfit.h"

using namespace std;

v_fit *my_fit;
FitMethodsContainer *fitMethodsContainer;

v_fit::v_fit (char symb, string m)  
    : symbol(symb), method(m), 
      default_max_iterations(50), output_one_of(1), random_seed(-1),
      max_evaluations(0), evaluations(0), iter_nr (0), na(0), nf(0) 
{
    irpar ["output-one-of"] = IntRange (&output_one_of, 1, 999);
    irpar["pseudo-random-seed"] = IntRange (&random_seed, -1, 999999999);
    ipar["default-max-iterations"] = &default_max_iterations;
    ipar["max-wssr-evaluations"] = &max_evaluations;
    Distrib_enum ['u'] = "uniform";
    Distrib_enum ['g'] = "gauss";
    Distrib_enum ['l'] = "lorentz";
    Distrib_enum ['b'] = "bound";
}

string v_fit::getInfo (int mode)
{
    //n_m = number of points - degrees of freedom (parameters)
    int n_m = - AL->pars()->count_a();
    for (int i = 0; i < AL->get_core_count(); i++) 
        n_m += AL->get_core(i)->get_active_data()->get_n(); 

    string s = "Current WSSR = " + S(compute_wssr()) 
                + " (expected: " + S(n_m) + "); SSR = " 
                + S(compute_wssr(fp_v0, false));
    if (mode == 1 || mode == 2) {
        if (na + nf != AL->pars()->count_a() 
                            || nf != AL->pars()->count_frozen() || na == 0) 
            return s + "\nTo show covariance matrix or errors, method must be "
                        "initialized (f.run).";
        vector<fp> alpha(na*na), beta(na);
        a_orig = all2fitted (AL->pars()->values());
        compute_derivatives(a_orig, alpha, beta);
        reverse_matrix (alpha, na);
        for (vector<fp>::iterator i = alpha.begin(); i != alpha.end(); i++)
            (*i) *= 2;//FIXME: is it right? (S.Brandt, Analiza danych (10.17.4))
        if (mode == 1) {
            s += "\nSymetric errors: ";
            for (int i = 0; i < na; i++) {
                int nr = fitted2all(i);
                s += " @" + S(nr) + "=" + S(AL->pars()->get_a(nr)) + "+-" 
                    + S(sqrt(alpha[i * na + i]));
            }
        }
        else if (mode == 2)
            s += "\n" + print_matrix (alpha /*reversed*/, na, na, 
                                      "(co)variance matrix");
    }
    return s;
}

fp v_fit::compute_wssr (const vector<fp>& A, bool weigthed)
{
    evaluations++;
    const vector<fp>& all = (nf == 0 || A.empty() ? A : fitted2all(A));
    fp wssr = 0;
    for (int i = 0; i < AL->get_core_count(); i++) {
        const PlotCore *core = AL->get_core(i);
        core->get_sum()->use_param_a_for_value(all);
        wssr += compute_wssr_for_data(core->get_active_data(), core->get_sum(),
                                      weigthed);
    }
    return wssr;
}

fp v_fit::compute_wssr_for_data(const Data* data, const Sum *sum, bool weigthed)
{
    // pre: sum->precompute() called
    fp wssr = 0;
    int n = data->get_n();
    for (int j = 0; j < n; j++) {
        fp y = sum->value (data->get_x(j));
        fp weigth = weigthed ? data->get_sigma(j) : 1.;
        fp dy_sig = (data->get_y(j) - y) / weigth;
        wssr += dy_sig * dy_sig;
    }  
    return wssr;
}

//results in alpha and beta 
void v_fit::compute_derivatives(const vector<fp>& A, 
                                vector<fp>& alpha, vector<fp>& beta)
{
    assert (size(A) == na && size(alpha) == na * na && size(beta) == na);
    fill (alpha.begin(), alpha.end(), 0.0);
    fill (beta.begin(), beta.end(), 0.0);

    for (int i = 0; i < AL->get_core_count(); i++) {
        const PlotCore *core = AL->get_core(i);
        compute_derivatives_for(core->get_active_data(), core->get_sum(),
                                A, alpha, beta);
    }
    // filling second half of alpha[] 
    for (int j = 1; j < na; j++)
        for (int k = 0; k < j; k++)
            alpha[na * k + j] = alpha[na * j + k]; 
}

//results in alpha and beta 
//it computes only half of alpha matrix
void v_fit::compute_derivatives_for(const Data* data, const Sum *sum,
                                const vector<fp>& A, 
                                vector<fp>& alpha, vector<fp>& beta)
{
    static vector<fp> tmp, tmp2;
    if (size(tmp) != na)
        tmp.resize (na);
    if (nf > 0) { //there are frozen @a
        sum->use_param_a_for_value(fitted2all(A));
        tmp2.resize(na + nf);
    }
    else
        sum->use_param_a_for_value(A);
    
    int ndata = data->get_n ();
    for (int i = 0; i < ndata; i++) {
        fp y;
        if (nf > 0) { //there are frozen @a
            y = sum->value_and_put_deriv (data->get_x(i), tmp2);
            tmp = all2fitted (tmp2);
        }
        else //no frozen ...
            y = sum->value_and_put_deriv (data->get_x(i), tmp);
        fp inv_sig = 1.0 / data->get_sigma(i);
        fp dy_sig = (data->get_y(i) - y) * inv_sig;
        for (vector<fp>::iterator j = tmp.begin(); j != tmp.end(); j++) 
            *j *= inv_sig;
        for (int j = 0; j < na; j++) {
            for (int k = 0; k <= j; k++)       //half of alpha[]
                alpha[na * j + k] += tmp[j] * tmp[k];
            beta[j] += dy_sig * tmp[j]; 
        }
    }   
}

string v_fit::print_matrix (const vector<fp>& vec, int m, int n, char *name)
    //m rows, n columns
{ 
    assert (size(vec) == m * n);
    if (m < 1 || n < 1)
        warn ("In `print_matrix': It is not a matrix.");
    ostringstream h;
    h << name << "={ ";
    if (m == 1) { // vector 
        for (int i = 0; i < n; i++)
            h << vec[i] << (i < n - 1 ? ", " : " }") ;
    }
    else { //matrix 
        std::string blanks (strlen (name) + 1, ' ');
        for (int j = 0; j < m; j++){
            if (j > 0)
                h << blanks << "  ";
            for (int i = 0; i < n; i++) 
                h << vec[j * n + i] << ", ";
            h << endl;
        }
        h << blanks << "}";
    }
    return h.str();
}

bool v_fit::post_fit (const std::vector<fp>& aa, fp chi2)
{
    bool no_move = (chi2 >= wssr_before);
    string comment = method + (chi2 < wssr_before ? "" : " (worse)");
    if (nf > 0) {
        vector<fp> aaa = fitted2all(aa);
        AL->pars()->write_avec (aaa, method, no_move);
    }
    else
        AL->pars()->write_avec (aa, method, no_move);
    if (chi2 < wssr_before) {
        info ("Better fit found (WSSR = " + S(chi2) + ", was " + S(wssr_before)
                + ", " + S((chi2 - wssr_before) / wssr_before * 100) + "%).");
        return true;
    }
    else {
        if (chi2 > wssr_before) {
            info ("Better fit NOT found (WSSR = " + S(chi2)
                    + ", was " + S(wssr_before) + ").\nParameters NOT changed");
        }
        iteration_plot(fp_v0); //reverting to old plot
        return false;
    }
}

fp v_fit::draw_a_from_distribution (int nr, char distribution, fp mult)
{
    assert (nr >= 0 
            && nr < AL->pars()->count_a() - AL->pars()->count_frozen());
    if (nf > 0)
        nr = fitted2all (nr);
    fp dv = 0;
    switch (distribution) {
        case 'g':
            dv = rand_gauss();
            break;
        case 'l':
            dv = rand_cauchy();
            break;
        case 'b':
            dv = rand_bool() ? -1 : 1;
            break;
        default: // 'u' - uniform
            dv = rand_1_1();
            break;
    }
    return AL->pars()->variation_of_a (nr, dv * mult);
}

void v_fit::fit (bool ini, int max_iter)
{
    if (AL->pars()->count_a() == 0) {
        warn ("What am I to fit?");
        return;
    }
    if (ini) {
        user_interrupt = false;
        iter_nr = 0;
        nf = AL->pars()->count_frozen();
        na = AL->pars()->count_a() - nf;
        a_orig = all2fitted (AL->pars()->values());
        int rs = random_seed >= 0 ? random_seed : time(0);
        srand (rs);
        verbose ("Seed for a sequence of pseudo-random numbers: " + S(rs));
        init();
    }
    //was init() callled ?
    else if (na + nf != AL->pars()->count_a() 
                                  || nf != AL->pars()->count_frozen()) {
        warn (method + " method should be initialized first. Canceled");
        return;
    }
    user_interrupt = false;
    evaluations = 0;
    max_iterations = max_iter >= 0 ? max_iter : default_max_iterations;
    autoiter();
}

bool v_fit::common_termination_criteria(int iter)
{
    bool stop = false;
    if (user_interrupt) {
        user_interrupt = false;
        info ("Fitting stopped manually.");
        stop = true;
    }
    if (max_iterations >= 0 && iter >= max_iterations) {
        info("Maximum iteration number reached.");
        stop = true;
    }
    if (max_evaluations > 0 && evaluations >= max_evaluations) {
        info("Maximum evaluations number reached.");
        stop = true;
    }
    return stop;
}

vector<fp> v_fit::fitted2all (const vector<fp>& A)
{
    assert (size(A) == na);
    vector<fp> r(na + nf);
    vector<fp>::const_iterator ai = A.begin(); 
    for (int i = 0; i < na + nf; i++)
        if (!AL->pars()->is_frozen(i)) {
            r[i] = *ai;
            ai++;
        }
        else
            r[i] = AL->pars()->get_a(i);
    assert (ai == A.end());
    return r;
}

int v_fit::fitted2all (int nr)
{
    int f = 0, a = 0;
    while (f != nr) {
        if (!AL->pars()->is_frozen(a))
            f++;
        a++;
    }
    return a;
}

vector<fp> v_fit::all2fitted (const vector<fp>& A)
{
    assert (size(A) == na + nf);
    vector<fp> r(na);
    vector<fp>::iterator ri = r.begin(); 
    for (int i = 0; i < na + nf; i++)
        if (!AL->pars()->is_frozen(i)) {
            *ri = A[i];
            ri++;
        }
    assert (ri == r.end());
    return r;
}

void v_fit::iteration_plot (const vector<fp>& a)
{
    getUI()->drawPlot(3, true, nf > 0 && !a.empty() ? fitted2all(a) : a);
}

FitMethodsContainer::FitMethodsContainer()
{
    v_fit *f = new LMfit;
    methods.push_back(f); 
    f = new NMfit;
    methods.push_back(f); 
    f = new GAfit;
    methods.push_back(f); 
    my_fit = methods[0]; 
}

FitMethodsContainer::~FitMethodsContainer()
{
    for (vector<v_fit*>::iterator i = methods.begin(); i != methods.end(); i++)
        delete *i;
}

string FitMethodsContainer::list_available_methods()
{
    string s = "Available methods: ";
    for (vector<v_fit*>::iterator i = methods.begin(); i != methods.end(); i++)
        s += S(i == methods.begin() ? " [" : ",  [") + (*i)->symbol + "] " 
            + (*i)->method;
    return s;
}

void FitMethodsContainer::change_method (char c)
{
    if (my_fit->symbol == c) {
        info ("Fitting method already was: " + my_fit->method);
        return;
    }
    for (vector<v_fit*>::iterator i = methods.begin(); i != methods.end(); i++)
        if ((*i)->symbol == c) {
            my_fit = *i;
            info ("Fitting method changed to: " + my_fit->method);
            return;
        }
    // if we are here, symbol c was not found in vector methods
    warn ("Unknown symbol for fitting method: " + S(c));
    info (list_available_methods());
}

string FitMethodsContainer::print_current_method()
{
    return "Current fitting method: " + my_fit->method + "\n" + 
                                                list_available_methods();
}

int FitMethodsContainer::current_method_number()
{
    int n = find(methods.begin(), methods.end(), my_fit) - methods.begin();
    assert (n < size(methods));
    return n;
}

void FitMethodsContainer::export_methods_settings_as_script(std::ostream& os)
{
    os << "### Settings of all method\n";
    for (vector<v_fit*>::iterator i = methods.begin(); i != methods.end(); i++)
        os << "f.method " << (*i)->symbol << " ### " << (*i)->method
            << endl << (*i)->set_script('f');
}

static const fp TINY = 1e-12;

fp rand_gauss()
{
    static bool is_saved = false;
    static fp saved;
    if (!is_saved) {
        fp rsq = 0, x1, x2;
        while (rsq < TINY || rsq >= 1) {
            x1 = rand_1_1();
            x2 = rand_1_1();
            rsq = x1 * x1 + x2 * x2;
        }
        fp f = sqrt (-2. * log(rsq) / rsq);
        saved = x1 * f;
        is_saved = true;
        return x2 * f;
    }
    else {
        is_saved = false;
        return saved;
    }
}

fp rand_cauchy()
{
    fp rsq = 0, x1, x2;
    while (rsq < TINY || rsq >= 1) {
        x1 = rand_1_1(); 
        x2 = rand_1_1();
        rsq = x1 * x1 + x2 * x2;
    }
    if (fabs(x1) < TINY) //bad luck
        return rand_cauchy(); //try again
    return (x2 / x1);
}


int v_fit::Jordan(vector<fp>& A, vector<fp>& b, int n) 
{
    
    /* This function solves a set of linear algebraic equations using
     * Jordan elimination with partial pivoting.
     * (PL:metoda eliminacji Jordana z cz�ciowym wyborem elementu podstawowego)
     *
     * A * x = b
     * 
     * A is n x n matrix, fp A[n*n]
     * b is vector b[n],   
     * Function returns vector x[] in b[], and 1-matrix in A[].
     * 
     */
    assert (size(A) == n*n && size(b) == n);
//#define DISABLE_PIVOTING   /*don't do it*/

#ifndef DISABLE_PIVOTING
    int maxnr;
    fp amax;
#endif
    for (int i = 0; i < n; i++) {
#ifndef DISABLE_PIVOTING 
        amax = 0; maxnr = -1;                           // looking for a pivot
        for (int j = i; j < n; j++)                     // element
            if (fabs (A[n*j+i]) > amax){
                maxnr = j;
                amax = fabs (A[n * j + i]);
            }
        if (maxnr == -1) {    // singular matrix
            // it's not part of Jordan's method. i-th column has only zeros. 
            // If it's the same about i-th row, and b[i]==0, let x[i]==0. 
            // If not, warn and return
            for (int j = i; j < n; j++)
                if (A[n * i + j] || b[i]) {
                    verbose (print_matrix(A, n, n, "A"));
                    info (print_matrix(b, 1, n, "b"));
                    warn ("Inside Jordan elimination: singular matrix.");
                    verbose ("Column " + S(i) + " is zeroed."
                            " Jordan method with partial pivoting used.");
                    return -i - 1;
                }
            verbose ("In L-M method: @" + S(i) + " not changed.");
            continue; // x[i]=b[i], b[i]==0
        }
        if (maxnr != i) {                            // interchanging rows
            for (int j=i; j<n; j++)
                Swap (A[n*maxnr+j], A[n*i+j]);
            Swap (b[i], b[maxnr]);
        }
#else
        if (A[i*n+i] == 0) {
            warn ("Inside Jordan elimination method with "
                    "_disabled_ pivoting: 0 on diagonal row=column=" + S(i));
            return -i - 1;
        }
#endif
        register fp foo = 1.0 / A[i*n+i];
        for (int j = i; j < n; j++)
            A[i*n+j] *= foo;
        b[i] *= foo;
        for (int k = 0; k < n; k++)
            if (k != i) {
                foo = A[k * n + i];
                for (int j = i; j < n; j++)
                    A[k * n + j] -= A[i * n + j] * foo;
                b[k] -= b[i] * foo;
            }
    }
    return 0;
}

int v_fit::reverse_matrix (vector<fp>&A, int n) 
    //returns A^(-1) in A
{
    assert (size(A) == n*n);    //it's slow, but there is no need 
    vector<fp> A_result(n*n);   // to optimize it
    for (int i = 0; i < n; i++) {
        vector<fp> A_copy = A;      
        vector<fp> v(n, 0);
        v[i] = 1;
        int r = Jordan(A_copy, v, n);
        if (r != 0)
            return r;
        for (int j = 0; j < n; j++) 
            A_result[j * n + i] = v[j];
    }
    A = A_result;
    return 0;
}


