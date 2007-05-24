// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License version 2
// $Id$

/// In this file:
///  FPlot, the base class for MainPlot and AuxPlot 

#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/fontdlg.h>
#include <wx/numdlg.h>
#include <wx/confbase.h>

#include "plot.h"
#include "cmn.h" 
#include "gui.h" //ftk
#include "../data.h"
#include "../logic.h" 

using namespace std;

void Scale::set(fp m, fp M, int pixels)
{
    fp h = 0;
    if (logarithm) {
        M = log(max(M, 1e-1));
        m = log(max(m, 1e-1));
    }
    h = M - m;
    origin = reversed ? M : m;
    if (h == 0)
        h = 0.1;
    scale = pixels / (reversed ? -h : h);
}

//===============================================================
//                       BufferedPanel
//===============================================================

void BufferedPanel::refresh(bool now)
{
    clear_and_draw();
    Refresh(false);
    if (now) 
        Update();
}

bool BufferedPanel::resize_buffer(wxDC &dc)
{
    wxCoord w, h;
    dc.GetSize(&w, &h);
    if (!buffer.Ok()
            || w != buffer.GetWidth() || h != buffer.GetHeight()) {
        memory_dc.SelectObject(wxNullBitmap);
        buffer = wxBitmap(w, h);
        memory_dc.SelectObject(buffer);
        return true;
    }
    return false;
}

void BufferedPanel::clear_and_draw()
{
    if (!buffer.Ok())
        return;
    memory_dc.SetLogicalFunction(wxCOPY);
    memory_dc.SetBackground(wxBrush(backgroundCol));
    memory_dc.Clear();
    draw(memory_dc);
}

/// called from wxPaint event handler
void BufferedPanel::buffered_draw()
{
    wxPaintDC dc(this);
    if (resize_buffer(dc)) 
        clear_and_draw();
    dc.Blit(0, 0, buffer.GetWidth(), buffer.GetHeight(), &memory_dc, 0, 0);
}

//===============================================================
//                FPlot (plot with data and fitted curves) 
//===============================================================

void FPlot::draw_dashed_vert_line(int X, int style)
{
    if (X != INT_MIN) {
        wxClientDC dc(this);
        dc.SetLogicalFunction (wxINVERT);
        if (style == wxSHORT_DASH)
            dc.SetPen(*wxBLACK_DASHED_PEN);
        else {
            wxPen pen = *wxBLACK_DASHED_PEN;
            pen.SetStyle(style);
            dc.SetPen(pen);
        }
        int h = GetClientSize().GetHeight();
        dc.DrawLine (X, 0, X, h);
    }
}

void FPlot::draw_crosshair(int X, int Y)
{
    wxClientDC dc(this);
    dc.SetLogicalFunction (wxINVERT);
    dc.SetPen(*wxBLACK_DASHED_PEN);
    dc.CrossHair(X, Y);
}

bool FPlot::vert_line_following_cursor (MouseActEnum ma, int x, int x0)
{
    if (ma == mat_start) {
        draw_dashed_vert_line(x0);
        vlfc_prev_x0 = x0;
    }
    else {
        if (vlfc_prev_x == INT_MIN) 
            return false;
        draw_dashed_vert_line(vlfc_prev_x); //clear (or draw again) old line
    }
    if (ma == mat_move || ma == mat_start) {
        draw_dashed_vert_line(x);
        vlfc_prev_x = x;
    }
    else if (ma == mat_redraw) {
        draw_dashed_vert_line(vlfc_prev_x0);
    }
    else { // mat_stop
        draw_dashed_vert_line(vlfc_prev_x0); //clear
        vlfc_prev_x = vlfc_prev_x0 = INT_MIN;
    }
    return true;
}

void draw_line_with_style(wxDC& dc, int style, 
                          wxCoord X1, wxCoord Y1, wxCoord X2, wxCoord Y2)
{
    wxPen pen = dc.GetPen();
    int old_style = pen.GetStyle();
    pen.SetStyle(style);
    dc.SetPen(pen);
    dc.DrawLine (X1, Y1, X2, Y2);
    pen.SetStyle(old_style);
    dc.SetPen(pen);
}

/// draw x axis tics
void FPlot::draw_xtics (wxDC& dc, View const &v, bool set_pen)
{
    if (set_pen) {
        dc.SetPen(wxPen(xAxisCol));
        dc.SetTextForeground(xAxisCol);
    }
    dc.SetFont(ticsFont);
    // get tics text height
    wxCoord h;
    dc.GetTextExtent(wxT("1234567890"), 0, &h);

    vector<double> minors;
    vector<double> x_tics = scale_tics_step(v.left, v.right, x_max_tics, 
                                            minors, xs.logarithm);

    //if x axis is visible tics are drawed at the axis, 
    //otherwise tics are drawed at the bottom edge of the plot
    int Y = GetClientSize().GetHeight() - h;
    if (x_axis_visible && !ys.logarithm && ys.px(0) >= 0 && ys.px(0) < Y)
        Y = ys.px(0);
    for (vector<double>::const_iterator i = x_tics.begin(); 
                                                    i != x_tics.end(); ++i) {
        int X = xs.px(*i);
        dc.DrawLine (X, Y, X, Y - x_tic_size);
        wxString label = s2wx(S(*i));
        if (label == wxT("-0")) 
            label = wxT("0");
        wxCoord w;
        dc.GetTextExtent (label, &w, 0);
        dc.DrawText (label, X - w/2, Y + 1);
        if (x_grid) {
            draw_line_with_style(dc, wxDOT, X,0, X,Y);
            draw_line_with_style(dc, wxDOT, X, Y+1+h, 
                                            X, GetClientSize().GetHeight());
        }
    }
    //draw minor tics
    if (xminor_tics_visible)
        for (vector<double>::const_iterator i = minors.begin(); 
                                                    i != minors.end(); ++i) {
            int X = xs.px(*i);
            dc.DrawLine (X, Y, X, Y - x_tic_size);
        }
}

/// draw y axis tics
void FPlot::draw_ytics (wxDC& dc, View const &v, bool set_pen)
{
    if (set_pen) {
        dc.SetPen(wxPen(xAxisCol));
        dc.SetTextForeground(xAxisCol);
    }
    dc.SetFont(ticsFont);


    //if y axis is visible, tics are drawed at the axis, 
    //otherwise tics are drawed at the left hand edge of the plot
    int X = 0;
    if (y_axis_visible && xs.px(0) > 0 
            && xs.px(0) < GetClientSize().GetWidth()-10)
        X = xs.px(0);
    vector<double> minors;
    vector<double> y_tics = scale_tics_step(v.bottom, v.top, y_max_tics, 
                                            minors, ys.logarithm);
    for (vector<double>::const_iterator i = y_tics.begin(); 
                                                    i != y_tics.end(); ++i) {
        int Y = ys.px(*i);
        dc.DrawLine (X, Y, X + y_tic_size, Y);
        wxString label = s2wx(S(*i));
        if (label == wxT("-0"))
            label = wxT("0");
        if (x_axis_visible && label == wxT("0")) 
            continue;
        wxCoord w, h;
        dc.GetTextExtent (label, &w, &h);
        dc.DrawText (label, X + y_tic_size + 1, Y - h/2);
        if (y_grid) {
            draw_line_with_style(dc, wxDOT, 0,Y, X,Y);
            draw_line_with_style(dc, wxDOT, X + y_tic_size + 1 + w + 1, Y,
                                            GetClientSize().GetWidth(), Y);
        }
    }
    //draw minor tics
    if (yminor_tics_visible)
        for (vector<double>::const_iterator i = minors.begin(); 
                                                    i != minors.end(); ++i) {
            int Y = ys.px(*i);
            dc.DrawLine (X, Y, X + y_tic_size, Y);
        }
}

double FPlot::get_max_abs_y (double (*compute_y)(vector<Point>::const_iterator, 
                                                 Sum const*),
                         vector<Point>::const_iterator first,
                         vector<Point>::const_iterator last,
                         Sum const* sum)
{
    double max_abs_y = 0;
    for (vector<Point>::const_iterator i = first; i < last; i++) {
        if (i->is_active) {
            double y = fabs(((*compute_y)(i, sum)));
            if (y > max_abs_y) max_abs_y = y;
        }
    }
    return max_abs_y;
}

void FPlot::draw_data (wxDC& dc, 
                       double (*compute_y)(vector<Point>::const_iterator, 
                                           Sum const*),
                       Data const* data, 
                       Sum const* sum,
                       wxColour const& color, wxColour const& inactive_color, 
                       int Y_offset,
                       bool cumulative)
{
    Y_offset *= (GetClientSize().GetHeight() / 100);
    wxPen const activePen(color.Ok() ? color : activeDataCol);
    wxPen const inactivePen(inactive_color.Ok() ? inactive_color 
                                                : inactiveDataCol);
    wxBrush const activeBrush(activePen.GetColour(), wxSOLID);
    wxBrush const inactiveBrush(inactivePen.GetColour(), wxSOLID);
    if (data->is_empty()) 
        return;
    vector<Point>::const_iterator first = data->get_point_at(ftk->view.left),
                                  last = data->get_point_at(ftk->view.right);
    //if (last - first < 0) return;
    bool active = first->is_active;
    dc.SetPen (active ? activePen : inactivePen);
    dc.SetBrush (active ? activeBrush : inactiveBrush);
    int X_ = INT_MIN, Y_ = INT_MIN;
    // first line segment -- lines should be drawed towards points 
    //                                                 that are outside of plot 
    if (line_between_points && first > data->points().begin() && !cumulative) {
        X_ = xs.px (ftk->view.left);
        int Y_l = ys.px ((*compute_y)(first - 1, sum));
        int Y_r = ys.px ((*compute_y)(first, sum));
        int X_l = xs.px ((first - 1)->x);
        int X_r = xs.px (first->x);
        if (X_r == X_l)
            Y_ = Y_r;
        else
            Y_ = Y_l + (Y_r - Y_l) * (X_ - X_l) / (X_r - X_l);
    }
    Y_ -= Y_offset;
    double y = 0;

    //drawing all points (and lines); main loop
    for (vector<Point>::const_iterator i = first; i < last; i++) {
        int X = xs.px(i->x);
        if (cumulative)
            y += (*compute_y)(i, sum);
        else
            y = (*compute_y)(i, sum);
        int Y = ys.px(y) - Y_offset;
        if (X == X_ && Y == Y_) 
            continue;

        if (i->is_active != active) {
            //half of line between points should be active and half not.
            //draw first half here and change X_, Y_; the rest will be drawed
            //as usually.
            if (line_between_points) {
                int X_mid = (X_ + X) / 2, Y_mid = (Y_ + Y) / 2;
                dc.DrawLine (X_, Y_, X_mid, Y_mid);
                X_ = X_mid, Y_ = Y_mid;
            }
            active = i->is_active;
            if (active) {
                dc.SetPen (activePen);
                dc.SetBrush (activeBrush);
            }
            else {
                dc.SetPen (inactivePen);
                dc.SetBrush (inactiveBrush);
            }
        }

        if (point_radius > 1) 
            dc.DrawCircle (X, Y, point_radius - 1);
        if (line_between_points) {
            if (X_ != INT_MIN)
                dc.DrawLine (X_, Y_, X, Y);
            X_ = X, Y_ = Y;
        }
        else {//no line_between_points
            if (point_radius == 1)
                dc.DrawPoint (X, Y);
        }

        if (draw_sigma) {
            dc.DrawLine (X, ys.px(y - i->sigma) - Y_offset,
                         X, ys.px(y + i->sigma) - Y_offset);
        }
    }

    //the last line segment, toward next point
    if (line_between_points && last < data->points().end() && !cumulative) {
        int X = xs.px (ftk->view.right);
        int Y_l = ys.px ((*compute_y)(last - 1, sum));
        int Y_r = ys.px ((*compute_y)(last, sum));
        int X_l = xs.px ((last - 1)->x);
        int X_r = xs.px (last->x);
        if (X_r != X_l) {
            int Y = Y_l + (Y_r - Y_l) * (X - X_l) / (X_r - X_l) - Y_offset;
            dc.DrawLine (X_, Y_, X, Y);
        }
    }
}

void FPlot::change_tics_font()
{
    wxFontData data;
    data.SetInitialFont(ticsFont);
    data.SetColour(xAxisCol);

    wxFontDialog dialog(0, data);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxFontData retData = dialog.GetFontData();
        ticsFont = retData.GetChosenFont();
        xAxisCol = retData.GetColour();
        refresh(false);
    }
}

void FPlot::set_scale()
{
    View const &v = ftk->view;
    xs.set(v.left, v.right, GetClientSize().GetWidth());
    ys.set(v.top, v.bottom, GetClientSize().GetHeight());
}

int FPlot::get_special_point_at_pointer(wxMouseEvent& event)
{
    // searching the closest peak-top and distance from it, d = dx + dy < 10 
    int nearest = -1;
    int min_dist = 10;
    for (vector<wxPoint>::const_iterator i = special_points.begin(); 
                                             i != special_points.end(); i++) {
        int d = abs(event.GetX() - i->x) + abs(event.GetY() - i->y);
        if (d < min_dist) {
            min_dist = d;
            nearest = i - special_points.begin();
        }
    }
    return nearest;
}

void FPlot::read_settings(wxConfigBase *cf)
{
    cf->SetPath(wxT("Visible"));
    x_axis_visible = cfg_read_bool (cf, wxT("xAxis"), true);  
    y_axis_visible = cfg_read_bool (cf, wxT("yAxis"), false);  
    xtics_visible = cfg_read_bool (cf, wxT("xtics"), true);
    ytics_visible = cfg_read_bool (cf, wxT("ytics"), true);
    xminor_tics_visible = cfg_read_bool (cf, wxT("xMinorTics"), true);
    yminor_tics_visible = cfg_read_bool (cf, wxT("yMinorTics"), false);
    x_grid = cfg_read_bool (cf, wxT("xgrid"), false);
    y_grid = cfg_read_bool (cf, wxT("ygrid"), false);
    cf->SetPath(wxT("../Colors"));
    xAxisCol = cfg_read_color(cf, wxT("xAxis"), wxColour(wxT("WHITE")));
    cf->SetPath(wxT(".."));
    ticsFont = cfg_read_font(cf, wxT("ticsFont"), 
                             wxFont(8, wxFONTFAMILY_DEFAULT, 
                                    wxFONTSTYLE_NORMAL, 
                                    wxFONTWEIGHT_NORMAL));
}

void FPlot::save_settings(wxConfigBase *cf) const
{
    cf->SetPath(wxT("Visible"));
    cf->Write (wxT("xAxis"), x_axis_visible);
    cf->Write (wxT("yAxis"), y_axis_visible);
    cf->Write (wxT("xtics"), xtics_visible);
    cf->Write (wxT("ytics"), ytics_visible);
    cf->Write (wxT("xMinorTics"), xminor_tics_visible);
    cf->Write (wxT("yMinorTics"), yminor_tics_visible);
    cf->Write (wxT("xgrid"), x_grid);
    cf->Write (wxT("ygrid"), y_grid);
    cf->SetPath(wxT("../Colors"));
    cfg_write_color (cf, wxT("xAxis"), xAxisCol);
    cf->SetPath(wxT(".."));
    cfg_write_font (cf, wxT("ticsFont"), ticsFont);
}


BEGIN_EVENT_TABLE(FPlot, wxPanel)
END_EVENT_TABLE()


//===============================================================
//                             utilities
//===============================================================

/// returns major and minor tics positions
vector<double> scale_tics_step (double beg, double end, int max_tics, 
                                vector<double> &minors, bool log)
{
    vector<double> result;
    minors.clear();
    if (beg >= end || max_tics <= 0)
        return result;
    if (log) {
        if (beg <= 0)
            beg = 1e-1;
        if (end <= beg)
            end = 2*beg;
        double min_logstep = (log10(end/beg)) / max_tics;
        bool with_2_5 = (min_logstep < log10(2.));
        double logstep = ceil(min_logstep);
        double step0 = pow(10, logstep * ceil(log10(beg) / logstep));
        for (int i = 2; i <= 9; ++i) {
            double v = step0/10. * i;
            if (v > beg && v < end) {
                if (with_2_5 && (i == 2 || i == 5))
                    result.push_back(v);
                else
                    minors.push_back(v);
            }
        }
        for (double t = step0; t < end; t *= pow(10,logstep)) {
            result.push_back(t);
            for (int i = 2; i <= 9; ++i) {
                double v = t * i;
                if (v > beg && v < end)
                    if (with_2_5 && (i == 2 || i == 5))
                        result.push_back(v);
                    else
                        minors.push_back(v);
            }
        }
    }
    else {
        double min_step = (end - beg) / max_tics;
        double s = pow(10, floor (log10 (min_step)));
        int minor_div = 5; //ratio of major-step to minor-step
        // now s <= min_step
        if (s >= min_step)
            ;
        else if (s * 2 >= min_step) {
            s *= 2;
            minor_div = 2;
        }
        else if (s * 2.5 >= min_step) 
            s *= 2.5;
        else if (s * 5 >=  min_step) 
            s *= 5;
        else
            s *= 10;
        double t = s * ceil(beg / s);
        for (int i = 1; i < minor_div; ++i) {
            double v = t - s * i / minor_div;
            if (v > beg && v < end)
                minors.push_back(v);
        }
        for (; t < end; t += s) {
            result.push_back(t);
            for (int i = 1; i < minor_div; ++i) {
                double v = t + s * i / minor_div;
                if (v < end)
                    minors.push_back(v);
            }
        }
    }
    return result;
}

