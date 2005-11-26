// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#include "common.h"
#include "manipul.h"
#include <algorithm>
#include "data.h"
#include "sum.h"
#include "logic.h"
#include "ui.h"

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

Manipul::Manipul()
    : search_width (1.), cancel_peak_out_of_search(true),
      height_correction(1.), fwhm_correction(1.) 
{
    fpar ["search-width"] = &search_width;   
    bpar ["cancel-peak-out-of-search"] = &cancel_peak_out_of_search;
    fpar ["height-correction"] = &height_correction;
    fpar ["fwhm-correction"] = &fwhm_correction;
}

fp Manipul::my_y(int n, const EstConditions *ec) const
{
    //pre: sum->use_param_a_for_value();
    fp x = my_data->get_x(n);
    fp y = my_data->get_y(n);

    if (!ec)
        return y - my_sum->value(x);

    for (vector<VirtPeak>::const_iterator i = ec->virtual_peaks.begin();
                                             i != ec->virtual_peaks.end(); i++)
        y -= i->get_approx_y(x);
    for (vector<int>::const_iterator i = ec->real_peaks.begin();
                                             i != ec->real_peaks.end(); i++)
        y -= AL->get_functions()[*i]->calculate_value(x); 
    return y;
}

fp Manipul::data_area(int from, int to, const EstConditions *ec) const
{
    fp area = 0;
    fp x_prev = my_data->get_x (from);
    fp y_prev = my_y(from, ec);
    for (int i = from + 1; i <= to; i++) {
        fp x =  my_data->get_x (i);
        fp y =  my_y(i, ec);
        area += (x - x_prev) * (y_prev + y) / 2;
        x_prev = x;
        y_prev = y;
    }
    return area;
}

int Manipul::max_data_y_pos(int from, int to, const EstConditions *ec) const 
{
    assert (from < to);
    int pos = from;
    fp maxy = my_y(from, ec);
    for (int i = from + 1; i < to; i++) {
        fp y = my_y(i, ec);
        if (y > maxy) {
            maxy = y;
            pos = i;
        }
    }
    return pos;
}

fp Manipul::compute_data_fwhm(int from, int max_pos, int to, fp level,
                              const EstConditions *ec) const
{
    assert (from <= max_pos && max_pos <= to);
    const fp hm = my_y(max_pos, ec) * level;
    const int limit = 3; 
    int l = from, r = to, counter = 0;
    for (int i = max_pos; i >= from; i--) { //going down (and left) from maximum
        fp y = my_y(i, ec);
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
        fp y = my_y(i, ec);
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
    fp fwhm = my_data->get_x(r) - my_data->get_x(l);
    return max (fwhm, EPSILON);
}

bool Manipul::estimate_peak_parameters(fp approx_ctr, fp ctrplusmin,
                            fp *center, fp *height, fp *area, fp *fwhm,
                            const EstConditions *ec) const
{
    AL->use_parameters();
    if (my_data->get_n() <= 0) {
        warn ("No active data.");
        return false;
    }

    if (ctrplusmin < 0)
        ctrplusmin = search_width;
    int l_bor = max (my_data->get_lower_bound_ac (approx_ctr - ctrplusmin), 0);
    int r_bor = min (my_data->get_upper_bound_ac (approx_ctr + ctrplusmin), 
                     my_data->get_n() - 1);
    if (l_bor >= r_bor){
        warn ("Searching peak outside of data points range. Abandoned."
              " Tried at " + S(approx_ctr) + " +- " + S(ctrplusmin));
        return false;
    }
    int max_y_pos = max_data_y_pos(l_bor, r_bor, ec);
    if (max_y_pos == l_bor || max_y_pos == r_bor - 1) {
        string s = "Estimating peak parameters: peak outside of search scope."
              " Tried at " + S(approx_ctr) + " +- " + S(ctrplusmin);
        if (cancel_peak_out_of_search) {
            warn (s + " Canceled.");
            return false;
        }
        else
            info (s);
    }
    fp h = my_y(max_y_pos, ec);
    if (height) 
        *height = h * height_correction;
    if (center)
        *center = my_data->get_x(max_y_pos);
    if (fwhm)
        *fwhm = compute_data_fwhm(l_bor, max_y_pos, r_bor, 0.5, ec) 
                                                            * fwhm_correction;
    if (area) 
        *area = data_area(l_bor, r_bor, ec); //FIXME: how to find peak borders? 
                                            // t * FWHM would be better? t=??
    return true;
}

string Manipul::print_simple_estimate (fp center, fp w) const
{
    if (w <= 0)
        w = search_width;
    fp c = 0, h = 0, a = 0, fwhm = 0;
    int r = estimate_peak_parameters(center, w, &c, &h, &a, &fwhm); 
    if (r < 0)
        return "";
    return "Peak center: " + S(c) + " (expected: " + S(center) 
            + "), height: " + S(h) + ", area: " + S(a) + ", FWHM: " + S(fwhm);
}

string Manipul::print_global_peakfind () 
{
    string s;
    EstConditions estc;
    estc.real_peaks = my_sum->get_ff_idx();
    for (int i = 1; i <= 4; i++) {
        fp c = 0., h = 0., a = 0., fwhm = 0.;
        estimate_peak_parameters(0., +INF, &c, &h, &a, &fwhm, &estc);
        estc.virtual_peaks.push_back(VirtPeak(c, h, fwhm));
        if (h == 0.) 
            break;
        s += S(i != 1 ? "\n" : "") + "Peak #" + S(i) + " - center: " + S(c) 
            + ", height: " + S(h) + ", area: " + S(a) + ", FWHM: " + S(fwhm);
    }
    return s;
}

void Manipul::guess_and_add(string const& name, string const& function,
                            bool in_range, fp range_from, fp range_to,
                            vector<string> vars)
{
    fp c = 0., h = 0., a = 0., fwhm = 0.;
    EstConditions estc;
    estc.real_peaks = my_sum->get_ff_idx();
    if (in_range)
        estimate_peak_parameters((range_from+range_to)/2, 
                                 (range_to-range_from)/2, 
                                 &c, &h, &a, &fwhm, &estc);
    else
        estimate_peak_parameters(0., +INF, &c, &h, &a, &fwhm, &estc);
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
    AL->get_active_ds()->get_sum()->add_function_to(real_name, 'F');
}


Manipul *my_manipul;

