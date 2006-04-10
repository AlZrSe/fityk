// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include "common.h"
#include "sum.h"
#include "func.h"
#include "var.h"
#include "data.h"  //used in export_as...() methods
#include "guess.h" //estimate_peak_parameters() in guess_f()
#include "ui.h"

using namespace std;

Sum::Sum(VariableManager *mgr_, Data const* data_) 
    : mgr(*mgr_), data(data_)
{
    mgr.register_sum(this);
}

Sum::~Sum()
{
    mgr.unregister_sum(this);
}


void Sum::do_find_function_indices(vector<string> &names,
                                   vector<int> &idx)
{
    idx.clear();
    for (int i = 0; i < size(names); ){
        int k = mgr.find_function_nr(names[i]);
        if (k == -1)
            names.erase(names.begin() + i);
        else {
            idx.push_back(k);
            ++i;
        }
    }
}

void Sum::find_function_indices()
{
    do_find_function_indices(ff_names, ff_idx);
    do_find_function_indices(zz_names, zz_idx);
}

/// checks if sum depends on variable with index idx
bool Sum::is_dependent_on_var(int idx) const
{
    std::vector<Variable*> const& vv = mgr.get_variables();
    for (vector<int>::const_iterator i = ff_idx.begin(); i != ff_idx.end(); i++)
        if (mgr.get_function(*i)->is_dependent_on(idx, vv))
            return true;
    for (vector<int>::const_iterator i = zz_idx.begin(); i != zz_idx.end(); i++)
        if (mgr.get_function(*i)->is_dependent_on(idx, vv))
            return true;
    return false;
}

void Sum::add_function_to(string const &name, char add_to)
{
    string only_name = !name.empty() && name[0]=='%' ? string(name,1) : name;
    // first remove function if it is already in ff or zz
    for (int i = 0; i < size(ff_names); ++i)
        if (ff_names[i] == only_name) {
            ff_names.erase(ff_names.begin() + i);
            ff_idx.erase(ff_idx.begin() + i);
        }
    for (int i = 0; i < size(zz_names); ++i)
        if (zz_names[i] == only_name) {
            zz_names.erase(zz_names.begin() + i);
            zz_idx.erase(zz_idx.begin() + i);
        }
    int idx = mgr.find_function_nr(only_name);
    if (idx == -1)
        throw ExecuteError("function %" + only_name + " not found.");
    if (add_to == 'F') {
        ff_names.push_back(only_name);
        ff_idx.push_back(idx);
    }
    else if (add_to == 'Z') {
        zz_names.push_back(only_name);
        zz_idx.push_back(idx);
    }
    else if (add_to == 'N')
        ;
    else
        throw ExecuteError("don't know how to add function to: " + S(add_to));
}

void Sum::remove_all_functions_from(char rm_from)
{
    if (rm_from == 'F') {
        ff_names.clear();
        ff_idx.clear();
    }
    else if (rm_from == 'Z') {
        zz_names.clear();
        zz_idx.clear();
    }
    else
        throw ExecuteError("don't know how to clear : " + S(rm_from));
}

fp Sum::value(fp x) const
{
    x += zero_shift(x);
    fp y = 0;
    for (vector<int>::const_iterator i = ff_idx.begin(); i != ff_idx.end(); i++)
        y += mgr.get_function(*i)->calculate_value(x);
    return y;
}

fp Sum::zero_shift(fp x) const
{
    fp z = 0;
    for (vector<int>::const_iterator i = zz_idx.begin(); i != zz_idx.end(); i++)
        z += mgr.get_function(*i)->calculate_value(x);
    return z;
}

void Sum::calculate_sum_value(vector<fp> &x, vector<fp> &y) const
{
    // add zero-shift to x
    for (vector<int>::const_iterator i = zz_idx.begin(); i != zz_idx.end(); i++)
        mgr.get_function(*i)->calculate_value(x, x);
    // add y-value to y
    for (vector<int>::const_iterator i = ff_idx.begin(); i != ff_idx.end(); i++)
        mgr.get_function(*i)->calculate_value(x, y);
}

// x is changed to x+Z
void Sum::calculate_sum_value_deriv(vector<fp> &x, vector<fp> &y,
                                    vector<fp> &dy_da) const
{
    assert(y.size() == x.size());
    if (x.empty())
        return;
    fill (dy_da.begin(), dy_da.end(), 0);
    // add zero-shift to x
    for (vector<int>::const_iterator i = zz_idx.begin(); i != zz_idx.end(); i++)
        mgr.get_function(*i)->calculate_value(x, x);
    // calculate value and derivatives
    for (vector<int>::const_iterator i = ff_idx.begin(); i != ff_idx.end(); i++)
        mgr.get_function(*i)->calculate_value_deriv(x, y, dy_da, false);
    for (vector<int>::const_iterator i = zz_idx.begin(); i != zz_idx.end(); i++)
        mgr.get_function(*i)->calculate_value_deriv(x, y, dy_da, true);
}

vector<fp> 
Sum::get_symbolic_derivatives(fp x) const
{
    int n = mgr.get_parameters().size();
    vector<fp> dy_da(n+1);
    vector<fp> xx(1, x);
    vector<fp> yy(1);
    calculate_sum_value_deriv(xx, yy, dy_da);
    dy_da.resize(n); //throw out last item (dy/dx)
    return dy_da;
}

vector<fp> 
Sum::get_numeric_derivatives(fp x, fp numerical_h) const
{
    std::vector<fp> av_numder = mgr.get_parameters();
    int n = av_numder.size();
    vector<fp> dy_da(n);
    const fp small_number = 1e-10; //it only prevents h==0
    for (int k = 0; k < n; k++) {
        fp acopy = av_numder[k];
        fp h = max (fabs(acopy), small_number) * numerical_h;
        av_numder[k] -= h;
        mgr.use_external_parameters(av_numder);
        fp y_aless = value(x);
        av_numder[k] = acopy + h;
        mgr.use_external_parameters(av_numder);
        fp y_amore = value(x);
        dy_da[k] = (y_amore - y_aless) / (2 * h);
        av_numder[k] = acopy;
    }
    mgr.use_parameters();
    return dy_da;
}

// estimate max. value in given range (probe at peak centers and between)
fp Sum::approx_max(fp x_min, fp x_max)
{
    mgr.use_parameters();
    fp x = x_min;
    fp y_max = value(x);
    vector<fp> xx; 
    for (vector<int>::const_iterator i=ff_idx.begin(); i != ff_idx.end(); i++) {
        fp ctr = mgr.get_function(*i)->center();
        if (x_min < ctr && ctr < x_max)
            xx.push_back(ctr);
    }
    xx.push_back(x_max);
    sort(xx.begin(), xx.end());
    for (vector<fp>::const_iterator i = xx.begin(); i != xx.end(); i++) {
        fp x_between = (x + *i)/2.;
        x = *i;
        fp y = max(value(x_between), value(x));
        if (y > y_max) 
            y_max = y;
    }
    return y_max;
}

void Sum::export_as_script (ostream& os) const
{
    os << "### sum exported as script -- begin" << endl;
    //TODO @n.
    os << join_vector(ff_names, ", ") << " -> F" << endl;
    os << join_vector(zz_names, ", ") << " -> Z" << endl;
    os << "### end of exported sum" << endl;
}


void Sum::export_as_dat (ostream& os) 
{
    int smooth_limit = 0;
    int n = data->get_n();
    if (n < 2) {
        warn ("At least two data points should be active when"
                " exporting sum as points");
        return;
    }
    mgr.use_parameters();
    int k = smooth_limit / n + 1;
    for (int i = 0; i < n - 1; i++) 
        for (int j = 0; j < k; j++) {
            fp x = ((k - j) * data->get_x(i) + j * data->get_x(i+1)) / k;
            fp y = value(x);
            os << x << "\t" << y << endl; 
        }
    fp x = data->get_x(n - 1);
    fp y = value(x);
    os << x << "\t" << y << endl; 
}

void Sum::export_as_peaks(std::ostream& os) const
{
    os << "# data file: " << data->get_filename() << endl;  
    os << "# Peak Type     Center  Height  Area    FWHM    "
                                        "parameters...\n"; 
    for (vector<int>::const_iterator i=ff_idx.begin(); i != ff_idx.end(); i++){
        Function const* p = mgr.get_function(*i);
        os << p->xname << "  " << p->type_name  
           << "  " << p->center() << " " << p->height() << " " << p->area() 
           << " " << p->fwhm() << "  ";
        for (int j = 0; j < p->get_vars_count(); ++j) 
            os << " " << p->get_var_value(j);
        os << endl;
    }
}

/*
void Sum::export_as_xfit (std::ostream& os) const
{
    const char* header = "     File        Peak     Th2        "
                            "Code      Constr      Area        Err     \r\n";
    const char *line_template = "                PV                @       "
                                    "   1.000                  0.0000     \r\n";

    os << header;
    string filename = data->get_filename();
    size_t last_slash = filename.find_last_of("/\\");
    if (last_slash != string::npos) filename = filename.substr(last_slash+1);
    if (filename.size() > 16)
        filename = filename.substr(filename.size() - 16, 16); 
    for (vector<int>::const_iterator i=ff_idx.begin(); i != ff_idx.end(); i++){
        Function const* p = mgr.get_function(*i);
        if (!p->is_peak()) 
            continue;
        string line = line_template;
        if (i == ff_idx.begin()) 
            line.replace (0, filename.size(), filename);
        string ctr = S(p->center());
        line.replace (23, ctr.size(), ctr);
        string area = S(p->area());
        line.replace (56, area.size(), area);
        os << line;
    }
}
*/

void Sum::export_to_file (string filename, bool append, char filetype) 
{
    if (!filetype) {
        //guessing filetype
        int dot = filename.rfind('.');
        if (dot > 0 && dot < static_cast<int>(filename.length()) - 1) {
            string exten(filename.begin() + dot, filename.end());
            if (exten == ".dat" || exten == ".xy")
                filetype = 'd';
            else if (exten == ".peaks")
                filetype = 'p';
            else if (exten == ".txt" && filename.size() >= 8 &&
                 filename.substr(filename.size()-8).compare("xfit.txt")==0)
                filetype = 'x';
        }
        if (!filetype) {
            info ("exporting as formula");
            filetype = 'f';
        }
    }
    ofstream os(filename.c_str(), ios::out | (append ? ios::app : ios::trunc));
    if (!os) {
        throw ExecuteError("Can't open file: " + filename);
    }
    switch (filetype) {
        case 'f': 
            os << get_formula() << endl;
            break;
        case 'd': 
            export_as_dat(os);
            break;
        case 'p': 
            export_as_peaks(os);
            break;
/*
        case 'x': 
            export_as_xfit(os);
            break;
*/
        default:
            warn ("Unknown filetype letter: " + S(filetype));
    }
}


string Sum::get_formula(bool simplify) const
{
    if (ff_names.empty())
        return "0";
    string shift;
    for (vector<int>::const_iterator i = zz_idx.begin(); i != zz_idx.end(); i++)
        shift += "+(" + mgr.get_function(*i)->get_current_formula() + ")";
    string x = "(x" + shift + ")";
    string formula;
    for (vector<int>::const_iterator i = ff_idx.begin(); i != ff_idx.end(); i++)
        formula += (i==ff_idx.begin() ? "" : "+") 
                   + mgr.get_function(*i)->get_current_formula(x); 
    if (simplify) {
        formula = simplify_formula(formula);
    }
    return formula;
}

fp Sum::numarea(fp x1, fp x2, int nsteps) const
{
    x1 += zero_shift(x1);
    x2 += zero_shift(x2);
    fp a = 0;
    for (vector<int>::const_iterator i = ff_idx.begin(); i != ff_idx.end(); i++)
        a += mgr.get_function(*i)->numarea(x1, x2, nsteps);
    return a;
}

