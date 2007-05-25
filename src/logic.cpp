// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License version 2
// $Id$

#include "common.h"
#include "logic.h"
#include <stdio.h>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include "data.h"
#include "sum.h"
#include "ui.h"
#include "fit.h"
#include "guess.h"
#include "settings.h"
#include "mgr.h"
#include "func.h"

using namespace std;

DataWithSum::DataWithSum(Fityk *F, Data* data_)
    : data(data_ ? data_ : new Data(F)), sum(new Sum(F))  
{}

bool DataWithSum::has_any_info() const
{
    return get_data()->has_any_info() || get_sum()->has_any_info(); 
}


Fityk::Fityk()
    : VariableManager(this),
      default_relative_domain_width(0.1)
{
    ui = new UserInterface(this);
    fit_container = new FitMethodsContainer(this);
    // Settings ctor is using FitMethodsContainer 
    settings = new Settings(this);
    view = View(0, 180, -50, 1e3);
    append_ds();
    activate_ds(0);
    get_settings()->do_srand();
    UdfContainer::initialize_udfs();
    AL = this;
}

Fityk::~Fityk() 
{
    dsds.clear();
    VariableManager::do_reset();
    delete fit_container;
    delete settings;
    delete ui;
}

void Fityk::activate_ds(int d)
{
    if (d < 0 || d >= size(dsds))
        throw ExecuteError("there is no such dataset: @" + S(d));
    active_ds = d;
}

int Fityk::append_ds(Data *data)
{
    DataWithSum* ds = new DataWithSum(this, data);
    dsds.push_back(ds); 
    return dsds.size() - 1; 
}

void Fityk::remove_ds(int d)
{
    if (d < 0 || d >= size(dsds))
        throw ExecuteError("there is no such dataset: @" + S(d));
    delete dsds[d];
    dsds.erase(dsds.begin() + d);
    if (dsds.empty())
        append_ds();
    if (active_ds == d)
        activate_ds( d==size(dsds) ? d-1 : d );
}

const Function* Fityk::find_function_any(string const &fstr) const
{
    if (fstr.empty())
        return 0;
    return VariableManager::find_function(find_function_name(fstr));
}

string Fityk::find_function_name(string const &fstr) const
{
    if (fstr[0] == '%' || islower(fstr[0]))
        return fstr;
    int pos = 0;
    int pref = -1;
    if (fstr[0] == '@') {
        pos = fstr.find(".") + 1;
        pref = strtol(fstr.c_str()+1, 0, 10);
    }
    vector<string> const &names = get_ds(pref)->get_sum()->get_names(fstr[pos]);
    int idx_ = strtol(fstr.c_str()+pos+2, 0, 10);
    int idx = (idx_ >= 0 ? idx_ : idx_ + names.size());
    if (!is_index(idx, names))
        throw ExecuteError("There is no item with index " + S(idx_));
    return names[idx];
}


void Fityk::dump_all_as_script(string const &filename)
{
    ofstream os(filename.c_str(), ios::out);
    if (!os) {
        warn ("Can't open file: " + filename);
        return;
    }
    os << fityk_version_line << "## dumped at: " << time_now() << endl;
    os << "set verbosity = only-warnings #the rest of the file is not shown\n"; 
    os << "set autoplot = never\n";
    os << "reset\n";
    os << "# ------------  settings  ------------\n";
    os << get_settings()->set_script() << endl;
    os << "# ------------  variables and functions  ------------\n";
    for (vector<Variable*>::const_iterator i = variables.begin();
            i != variables.end(); ++i)
        os << (*i)->xname << " = " << (*i)->get_formula(parameters) << endl;
    os << endl;
    vector<UdfContainer::UDF> const& udfs = UdfContainer::get_udfs();
    for (vector<UdfContainer::UDF>::const_iterator i = udfs.begin();
            i != udfs.end(); ++i)
        if (!i->is_builtin)
            os << "define " << i->formula << endl;
    os << endl;
    for (vector<Function*>::const_iterator i = functions.begin();
            i != functions.end(); ++i) {
        if ((*i)->has_outdated_type()) {
            string new_formula = Function::get_formula((*i)->type_name);
            if (!new_formula.empty())
                os << "undefine " << (*i)->type_name << endl;
            os << "define " << (*i)->type_formula << endl;
            os << (*i)->get_basic_assignment() << endl;
            os << "undefine " << (*i)->type_name << endl;
            if (!new_formula.empty())
                os << "define " << new_formula << endl;
        }
        else
            os << (*i)->get_basic_assignment() << endl;
    }
    os << endl;
    os << "# ------------  datasets and sums  ------------\n";
    for (int i = 0; i != get_ds_count(); ++i) {
        Data const* data = get_data(i);
        if (i != 0)
            os << "@+\n";
        if (!data->get_title().empty())
            os << "@" << i << ".title = '" << data->get_title() << "'\n";
        int m = data->points().size();
        os << "M=" << m << " in @" << i << endl;
        os << "X=" << data->get_x_max() << " in @" << i 
            << " # =max(x), prevents sorting." << endl;
        for (int j = 0; j != m; ++j) {
            Point const& p = data->points()[j];
            os << "X[" << j << "]=" << p.x << ", Y[" << j << "]=" << p.y 
                << ", S[" << j << "]=" << p.sigma 
                << ", A[" << j << "]=" << (p.is_active ? 1 : 0) 
                << " in @" << i << endl;
        }
        os << endl;
        Sum const* sum = get_sum(i);
        if (!sum->get_ff_names().empty())
            os << "@" << i << ".F = " 
                << join_vector(concat_pairs("%", sum->get_ff_names()), " + ") 
                << endl;
        if (!sum->get_zz_names().empty())
            os << "@" << i << ".Z = " 
                << join_vector(concat_pairs("%", sum->get_zz_names()), " + ") 
                << endl;
        os << endl;
    }
    os << "plot " << view.str() << " in @" << active_ds << endl;
    os << "set autoplot = " << get_settings()->getp("autoplot") << endl;
    os << "set verbosity = " << get_settings()->getp("verbosity") << endl;
}


int Fityk::check_ds_number(int n) const
{
    if (n == -1) {
        if (get_ds_count() == 1)
            return 0;
        else
            throw ExecuteError("Dataset must be specified.");
    }
    if (n < 0 || n >= get_ds_count())
        throw ExecuteError("There is no dataset @" + S(n));
    return n;
}

/// Send warning to UI. 
void Fityk::warn(std::string const &s) const
{ 
    get_ui()->output_message(os_warn, s); 
}

/// Send implicitely requested message to UI. 
void Fityk::rmsg(std::string const &s) const
{ 
    get_ui()->output_message(os_normal, s);
}

/// Send message to UI. 
void Fityk::msg(std::string const &s) const
{ 
    if (get_ui()->get_verbosity() >= 0)
         get_ui()->output_message(os_normal, s); 
}

/// Send verbose message to UI. 
void Fityk::vmsg(std::string const &s) const
{ 
    if (get_ui()->get_verbosity() >= 0)
         get_ui()->output_message(os_normal, s); 
}

int Fityk::get_verbosity() const 
{ 
    return settings->get_e("verbosity"); 
}

/// execute command(s) from string
Commands::Status Fityk::exec(std::string const &s) 
{ 
    return get_ui()->exec_and_log(s); 
}

Fit* Fityk::get_fit() 
{ 
    int nr = get_settings()->get_e("fitting-method");
    return get_fit_container()->get_method(nr); 
}

void Fityk::import_dataset(int slot, string const& filename, 
                            string const& type, vector<int> const& cols)
{
    const int new_dataset = -1;
    if (slot == new_dataset) {
        vector<string> next_files;
        if (this->get_ds_count() != 1 || this->get_data(0)->has_any_info()
                                    || this->get_sum(0)->has_any_info()) {
            // load data into new slot
            auto_ptr<Data> data(new Data(this));
            next_files = data->load_file(filename, type, cols); 
            slot = this->append_ds(data.release());
        }
        else { // there is only one and empty slot -- load data there
            next_files = this->get_data(-1)->load_file(filename, type, cols); 
            this->view.set_datasets(vector1(this->get_ds(0)));
            this->view.fit();
            slot = 0;
        }
        for (vector<string>::const_iterator i = next_files.begin(); 
                                            i != next_files.end(); ++i) 
            this->import_dataset(new_dataset, *i, type, cols);
    }
    else { // slot number was specified -- load data there
        this->get_data(slot)->load_file(filename, type, cols); 
        if (this->get_ds_count() == 1) {
            this->view.set_datasets(vector1(this->get_ds(0)));
            this->view.fit();
        }
    }
    this->activate_ds(slot);
}

//==================================================================


const fp View::relative_x_margin = 1./20.;
const fp View::relative_y_margin = 1./20.;

string View::str() const
{ 
    return "[" + (left!=right ? S(left) + ":" + S(right) : string(" "))
        + "] [" + (bottom!=top ? S(bottom) + ":" + S(top) 
                                           : string (" ")) + "]";
}

void View::fit(int flag)
{
    if (flag&fit_left || flag&fit_right) {
        fp x_min=0, x_max=0;
        get_x_range(x_min, x_max);
        if (x_min == x_max) {
            x_min -= 0.1; 
            x_max += 0.1;
        }
        fp x_margin = (x_max - x_min) * relative_x_margin;
        if (flag&fit_left)
            left = x_min - x_margin;
        if (flag&fit_right)
            right = x_max + x_margin;
    }

    if (flag&fit_top || flag&fit_bottom) {
        fp y_min=0, y_max=0;
        get_y_range(y_min, y_max);
        if (y_min == y_max) {
            y_min -= 0.1; 
            y_max += 0.1;
        }
        fp y_margin = (y_max - y_min) * relative_y_margin;
        if (flag&fit_bottom)
            bottom = y_min - y_margin;
        if (flag&fit_top)
            top = y_max + y_margin;
    }

}

void View::get_x_range(fp &x_min, fp &x_max)
{
    if (datas.empty()) 
        throw ExecuteError("Can't find x-y axes ranges for plot");
    x_min = datas.front()->get_x_min();
    x_max = datas.front()->get_x_max();
    for (vector<Data*>::const_iterator i = datas.begin()+1; 
            i != datas.end(); ++i) {
        x_min = min(x_min, (*i)->get_x_min());
        x_max = max(x_max, (*i)->get_x_max());
    }
}

void View::get_y_range(fp &y_min, fp &y_max)
{
    if (datas.empty()) 
        throw ExecuteError("Can't find x-y axes ranges for plot");
    y_min = y_max = (datas.front()->get_n() > 0 ? datas.front()->get_y(0) : 0);
    bool min_max_set = false;
    for (vector<Data*>::const_iterator i = datas.begin(); i != datas.end();
            ++i) {
        vector<Point>::const_iterator f = (*i)->get_point_at(left);
        vector<Point>::const_iterator l = (*i)->get_point_at(right);
        //first we are searching for minimal and max. y in active points
        for (vector<Point>::const_iterator i = f; i < l; i++) {
            if (i->is_active && is_finite(i->y)) {
                min_max_set = true;
                if (i->y > y_max) 
                    y_max = i->y;
                if (i->y < y_min) 
                    y_min = i->y;
            }
        }
    }

    if (!min_max_set || y_min == y_max) { //none or 1 active point, so now we  
                                   // search for min. and max. y in all points 
        for (vector<Data*>::const_iterator i = datas.begin(); i != datas.end();
                ++i) {
            vector<Point>::const_iterator f = (*i)->get_point_at(left);
            vector<Point>::const_iterator l = (*i)->get_point_at(right);
            for (vector<Point>::const_iterator i = f; i < l; i++) { 
                if (!is_finite(i->y))
                    continue;
                min_max_set = true;
                if (i->y > y_max) 
                    y_max = i->y;
                if (i->y < y_min) 
                    y_min = i->y;
            }
        }
    }

    for (vector<Sum*>::const_iterator i = sums.begin(); i != sums.end(); ++i) {
        Sum *sum = *i;
        if (!sum->has_any_info())
            continue;
        // estimated sum maximum
        fp sum_y_max = sum->approx_max(left, right);
        if (sum_y_max > y_max) 
            y_max = sum_y_max;
        if (sum_y_max < y_min) 
            y_min = sum_y_max;
    }
    // include or not include zero
    const fp show_zero_factor = 0.1;
    if (y_min > 0 && y_max - y_min > show_zero_factor * y_max)
        y_min = 0;
    else if (y_max < 0 && y_max - y_min > show_zero_factor * fabs(y_min))
        y_max = 0;
}

void View::parse_and_set(std::vector<std::string> const& lrbt) 
{
    assert(lrbt.size() == 4);
    string const &left = lrbt[0];
    string const &right = lrbt[1];
    string const &bottom = lrbt[2];
    string const &top = lrbt[3];
    fp l=0., r=0., b=0., t=0.;
    int flag = 0;
    if (left.empty())
        flag |= fit_left;
    else if (left != ".") {
        flag |= change_left;
        l = strtod(left.c_str(), 0);
    }
    if (right.empty())
        flag |= fit_right;
    else if (right != ".") {
        flag |= change_right;
        r = strtod(right.c_str(), 0);
    }
    if (bottom.empty())
        flag |= fit_bottom;
    else if (bottom != ".") {
        flag |= change_bottom;
        b = strtod(bottom.c_str(), 0);
    }
    if (top.empty())
        flag |= fit_top;
    else if (top != ".") {
        flag |= change_top;
        t = strtod(top.c_str(), 0);
    }
    set(l, r, b, t, flag);
    fit(flag);
}

//TODO set_datasets() is should not be public, datasets 
// should be always passed by fit() and parse_and_set()
void View::set_datasets(vector<DataWithSum*> const& dd) 
{
    assert(!dd.empty());
    datas.clear();
    sums.clear();
    for (vector<DataWithSum*>::const_iterator i = dd.begin(); 
                                                        i != dd.end(); ++i) 
        datas.push_back((*i)->get_data());
    sums.push_back(dd.front()->get_sum());
}

// the use of this global variable in libfityk will be eliminated,
// because it's not thread safe. 
Fityk* AL = 0;


