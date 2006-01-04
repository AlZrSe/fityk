// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#include "common.h"
#include "manipul.h"
#include <algorithm>
#include "data.h"
#include "sum.h"
#include "logic.h"
#include "ui.h"
#include "settings.h"

using namespace std;

fp VirtPeak::get_approx_y(fp x) const
{
    if (fabs(x - center) < fwhm) {
        fp dist_in_fwhm = fabs((x - center) / fwhm);
        if (dist_in_fwhm < 0.5)
            return height;
        else // 0.5 < dist_in_fwhm < 1.0
            return height * 2. * (1. - dist_in_fwhm);
    }
    else
        return 0;
}

namespace {

fp my_y (DataWithSum const* ds, int n, EstConditions const* ec=0);
fp data_area (DataWithSum const* ds, int from, int to, 
              EstConditions const* ec=0);
int max_data_y_pos (DataWithSum const* ds, int from, int to, 
                    EstConditions const* ec=0);
fp compute_data_fwhm (DataWithSum const* ds, 
                      int from, int max_pos, int to, fp level,
                      EstConditions const* ec=0);
void parse_range(DataWithSum const* ds, std::vector<std::string> const& range, 
                 fp& range_from, fp& range_to);


fp my_y(DataWithSum const* ds, int n, EstConditions const* ec) 
{
    //pre: sum->use_param_a_for_value();
    fp x = ds->get_data()->get_x(n);
    fp y = ds->get_data()->get_y(n);

    if (!ec)
        return y - ds->get_sum()->value(x);

    for (vector<VirtPeak>::const_iterator i = ec->virtual_peaks.begin();
                                             i != ec->virtual_peaks.end(); i++)
        y -= i->get_approx_y(x);
    for (vector<int>::const_iterator i = ec->real_peaks.begin();
                                             i != ec->real_peaks.end(); i++)
        y -= AL->get_functions()[*i]->calculate_value(x); 
    return y;
}

fp data_area(DataWithSum const* ds, int from, int to, 
             EstConditions const* ec) 
{
    fp area = 0;
    fp x_prev = ds->get_data()->get_x(from);
    fp y_prev = my_y(ds, from, ec);
    for (int i = from + 1; i <= to; i++) {
        fp x =  ds->get_data()->get_x(i);
        fp y =  my_y(ds, i, ec);
        area += (x - x_prev) * (y_prev + y) / 2;
        x_prev = x;
        y_prev = y;
    }
    return area;
}

int max_data_y_pos(DataWithSum const* ds, int from, int to, 
                   EstConditions const* ec) 
{
    assert (from < to);
    int pos = from;
    fp maxy = my_y(ds, from, ec);
    for (int i = from + 1; i < to; i++) {
        fp y = my_y(ds, i, ec);
        if (y > maxy) {
            maxy = y;
            pos = i;
        }
    }
    return pos;
}

fp compute_data_fwhm(DataWithSum const* ds, 
                     int from, int max_pos, int to, fp level,
                     EstConditions const* ec) 
{
    assert (from <= max_pos && max_pos <= to);
    const fp hm = my_y(ds, max_pos, ec) * level;
    const int limit = 3; 
    int l = from, r = to, counter = 0;
    for (int i = max_pos; i >= from; i--) { //going down (and left) from maximum
        fp y = my_y(ds, i, ec);
        if (y > hm) {
            if (counter > 0) //previous point had y < hm
                counter--;  // compensating it; perhaps it was only fluctuation
        }
        else {
            counter++;     //this point is below half (if level==0.5) width
            if (counter >= limit) { // but i want `limit' points below to be
                l = min (i + counter, max_pos);// sure that it's not fuctuation
                break;
            }
        }
    }
    for (int i = max_pos; i <= to; i++) { //the same for right half of peak
        fp y = my_y(ds, i, ec);
        if (y > hm) {
            if (counter > 0)
                counter--;
        }
        else {
            counter++;
            if (counter >= limit) {
                r = max (i - counter, max_pos);
                break;
            }
        }
    }
    fp fwhm = ds->get_data()->get_x(r) - ds->get_data()->get_x(l);
    return max (fwhm, EPSILON);
}

void parse_range(DataWithSum const* ds, vector<string> const& range,
                 fp& range_from, fp& range_to)
{
    assert (range.size() == 2);
    string le = range[0];
    string ri = range[1];
    if (le.empty())
        range_from = ds->get_data()->get_x_min();
    else if (le == ".") 
        range_from = AL->view.left;
    else
        range_from = strtod(le.c_str(), 0);
    if (ri.empty())
        range_to = ds->get_data()->get_x_max();
    else if (ri == ".") 
        range_to = AL->view.right;
    else
        range_to = strtod(ri.c_str(), 0);
}

} // anonymous namespace


void estimate_peak_parameters(DataWithSum const* ds, fp range_from, fp range_to,
                              fp *center, fp *height, fp *area, fp *fwhm,
                              EstConditions const* ec) 
{
    AL->use_parameters();
    if (ds->get_data()->get_n() <= 0) 
        throw ExecuteError("No active data.");

    int l_bor = max (ds->get_data()->get_lower_bound_ac (range_from), 0);
    int r_bor = min (ds->get_data()->get_upper_bound_ac (range_to), 
                     ds->get_data()->get_n() - 1);
    if (l_bor >= r_bor)
        throw ExecuteError("Searching peak outside of data points range. "
                           "Abandoned. Tried at [" + S(range_from) + " : " 
                           + S(range_to) + "]");
    int max_y_pos = max_data_y_pos(ds, l_bor, r_bor, ec);
    if (max_y_pos == l_bor || max_y_pos == r_bor - 1) {
        string s = "Estimating peak parameters: peak outside of search scope."
                  " Tried at [" + S(range_from) + " : " + S(range_to) + "]";
        if (getSettings()->get_b("cancel-peak-out-of-search")) 
            throw ExecuteError(s + " Canceled.");
        info (s);
    }
    fp h = my_y(ds, max_y_pos, ec);
    if (height) 
        *height = h * getSettings()->get_f("height-correction");
    if (center)
        *center = ds->get_data()->get_x(max_y_pos);
    if (fwhm)
        *fwhm = compute_data_fwhm(ds, l_bor, max_y_pos, r_bor, 0.5, ec) 
                                    * getSettings()->get_f("width-correction");
    if (area) 
        *area = data_area(ds, l_bor, r_bor, ec); 
        //FIXME: how to find peak borders?  t * FWHM would be better? t=??
}

string print_simple_estimate(DataWithSum const* ds,
                             fp range_from, fp range_to) 
{
    fp c = 0, h = 0, a = 0, fwhm = 0;
    estimate_peak_parameters(ds, range_from, range_to, &c, &h, &a, &fwhm); 
    return "Peak center: " + S(c) 
            + " (searched in [" + S(range_from) + ":" + S(range_to) + "])" 
            + " height: " + S(h) + ", area: " + S(a) + ", FWHM: " + S(fwhm);
}

string print_multiple_peakfind(DataWithSum const* ds,
                               int n, vector<string> const& range) 
{
    fp range_from, range_to;
    parse_range(ds, range, range_from, range_to);
    string s;
    EstConditions estc;
    estc.real_peaks = ds->get_sum()->get_ff_idx();
    for (int i = 1; i <= n; i++) {
        fp c = 0., h = 0., a = 0., fwhm = 0.;
        estimate_peak_parameters(ds, range_from, range_to, 
                                 &c, &h, &a, &fwhm, &estc);
        estc.virtual_peaks.push_back(VirtPeak(c, h, fwhm));
        if (h == 0.) 
            break;
        s += S(i != 1 ? "\n" : "") + "Peak #" + S(i) + " - center: " + S(c) 
            + ", height: " + S(h) + ", area: " + S(a) + ", FWHM: " + S(fwhm);
    }
    return s;
}


void guess_and_add(DataWithSum* ds, 
                   string const& name, string const& function,
                   vector<string> const& range, vector<string> vars)
{
    fp range_from, range_to;
    parse_range(ds, range, range_from, range_to);
    fp c = 0., h = 0., a = 0., fwhm = 0.;
    EstConditions estc;
    estc.real_peaks = ds->get_sum()->get_ff_idx();
    estimate_peak_parameters(ds, range_from, range_to, 
                             &c, &h, &a, &fwhm, &estc);
    vector<string> vars_lhs(vars.size());
    for (int i = 0; i < size(vars); ++i)
        vars_lhs[i] = string(vars[i], 0, vars[i].find('='));
    if (find(vars_lhs.begin(), vars_lhs.end(), "center") == vars_lhs.end())
        vars.push_back("center=~"+S(c));
    if (find(vars_lhs.begin(), vars_lhs.end(), "height") == vars_lhs.end())
        vars.push_back("height=~"+S(h));
    if (find(vars_lhs.begin(), vars_lhs.end(), "hwhm") == vars_lhs.end())
        vars.push_back("hwhm=~"+S(fwhm/2));
    if (find(vars_lhs.begin(), vars_lhs.end(), "area") == vars_lhs.end())
        vars.push_back("area=~"+S(a));
    string real_name = AL->assign_func(name, function, vars);
    ds->get_sum()->add_function_to(real_name, 'F');
}


