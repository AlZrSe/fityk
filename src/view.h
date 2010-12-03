// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+
// $Id: logic.h 322 2007-07-24 00:17:11Z wojdyr $

#ifndef FITYK__VIEW__H__
#define FITYK__VIEW__H__

#include <string>
#include <vector>
#include <float.h>

#include "common.h"

class DataAndModel;
class Data;
class Model;
class Ftk;

struct RealRange
{
    double from, to;
    RealRange() : from(-DBL_MAX), to(DBL_MAX) {}
    bool from_inf() const { return from == -DBL_MAX; }
    bool to_inf() const { return to == DBL_MAX; }
};

struct Rect
{
    fp left, right, bottom, top;

    Rect(fp l, fp r, fp b, fp t) : left(l), right(r), bottom(b), top(t) {}
    fp width() const { return right - left; }
    fp height() const { return top - bottom; }
};

/// manages view, i.e. x and y range visible currently to the user
/// user can set view in `plot' command, using string like "[20:][-100:1000]"
/// If the visible range is to be fitted to data/model, given datasets are used.
class View: public Rect
{
public:
    static const fp relative_x_margin, relative_y_margin;

    View(Ftk const* F_)
        : Rect(0, 180., -50, 1e3), F(F_),
          log_x_(false), log_y_(false), y0_factor_(10.) {}
    std::string str() const;
    /// fit specified edges to the data range
    void change_view(const RealRange& hor, const RealRange& ver,
                     const std::vector<int>& datasets);
    void set_log_scale(bool log_x, bool log_y) { log_x_=log_x; log_y_=log_y; }
    fp y0_factor() const { return y0_factor_; }
    void set_y0_factor(fp f) { y0_factor_ = f; }
private:
    Ftk const* F;
    bool log_x_, log_y_;
    fp y0_factor_;

    void get_x_range(std::vector<Data const*> datas, fp &x_min, fp &x_max);
    void get_y_range(std::vector<Data const*> datas,
                     std::vector<Model const*> models,
                     fp &y_min, fp &y_max);
};

#endif
