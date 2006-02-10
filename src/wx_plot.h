// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#ifndef FITYK__WX_PLOT__H__
#define FITYK__WX_PLOT__H__

#define INVALID -1234565 //i know it is ugly 

#include <map>
#include <limits.h>
#include "wx_common.h" // MouseModeEnum

// wxFULL_REPAINT_ON_RESIZE is defined only in wxWidgets >= 2.5
#ifndef wxFULL_REPAINT_ON_RESIZE
    #define wxFULL_REPAINT_ON_RESIZE 0
#endif 

class wxConfigBase;

struct Point;
class MainPlot;
class View;
struct f_names_type;

enum Mouse_act_enum  { mat_start, mat_stop, mat_move, mat_cancel,
                       mat_redraw };

enum Aux_plot_kind_enum 
{ 
    apk_empty, 
    apk_diff, 
    apk_diff_stddev, 
    apk_diff_y_proc,
    apk_cum_chi2
};


// convention here: lowercase coordinates of point are real values,
// and uppercase ones are coordinates of point on screen (integers).
// eg. x2X() - convert point x coordinate to pixel position on screen 
//

/// This class has no instances, MainPlot and AuxPlot are derived from it
class FPlot : public wxPanel
{
public:
    FPlot (wxWindow *parent, PlotShared &shar)
       : wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, 
                 wxNO_BORDER|wxFULL_REPAINT_ON_RESIZE),
         yUserScale(1.), yLogicalOrigin(0.), 
         shared(shar), mouse_press_X(INVALID), mouse_press_Y(INVALID), 
         vlfc_prev_x(INVALID), vlfc_prev_x0(INVALID)   {}
         
    ~FPlot() {}
    wxColour get_bg_color() const { return backgroundBrush.GetColour(); }
    void FPlot::draw_crosshair(int X, int Y);
    virtual void save_settings(wxConfigBase *cf) const;
    virtual void read_settings(wxConfigBase *cf);
    virtual void Draw(wxDC &dc) = 0;

protected:
    wxBrush backgroundBrush;
    wxPen activeDataPen, inactiveDataPen;
    wxPen xAxisPen;
    wxFont ticsFont;
    wxColour colourTextForeground, colourTextBackground;
    int point_radius;
    bool line_between_points;
    bool x_axis_visible, y_axis_visible, xtics_visible, ytics_visible;
    int x_max_tics, y_max_tics, x_tic_size, y_tic_size;
    fp yUserScale, yLogicalOrigin; 
    PlotShared &shared;
    int mouse_press_X, mouse_press_Y;
    int vlfc_prev_x, vlfc_prev_x0;

    void draw_dashed_vert_lines (int x1);
    bool vert_line_following_cursor(Mouse_act_enum ma, int x=0, int x0=INVALID);
    void draw_xtics (wxDC& dc, View const& v);
    void draw_ytics (wxDC& dc, View const &v);
    fp get_max_abs_y (fp (*compute_y)(std::vector<Point>::const_iterator,
                                      Sum const*),
                         std::vector<Point>::const_iterator first,
                         std::vector<Point>::const_iterator last,
                         Sum const* sum);
    void draw_data (wxDC& dc, 
                    fp (*compute_y)(std::vector<Point>::const_iterator, 
                                    Sum const*),
                    Data const* data, 
                    Sum const* sum, 
                    wxColour const& color = wxNullColour,
                    int Y_offset = 0,
                    bool cumulative=false);
    void change_tics_font();
    int y2Y (fp y) {  fp t = (y - yLogicalOrigin) * yUserScale;
                      return (fabs(t) < SHRT_MAX ? static_cast<int>(t) 
                                                 : t > 0 ? SHRT_MAX : SHRT_MIN);
                   }
    fp Y2y (int Y) { return Y / yUserScale + yLogicalOrigin; }
    int x2X (fp x) { return shared.x2X(x); }
    fp X2x (int X) { return shared.X2x(X); }

    DECLARE_EVENT_TABLE()
};


//Auxiliary plot, usually shows residuals or peak positions
class AuxPlot : public FPlot
{
public:
    AuxPlot (wxWindow *parent, PlotShared &shar, std::string name_) 
        : FPlot (parent, shar), name(s2wx(name_)), 
          y_zoom(1.), y_zoom_base(1.), fit_y_once(false) {}
    ~AuxPlot() {}
    void OnPaint(wxPaintEvent &event);
    void Draw(wxDC &dc);
    void OnLeaveWindow (wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent &event);
    void OnLeftDown (wxMouseEvent &event);
    void OnLeftUp (wxMouseEvent &event);
    void OnRightDown (wxMouseEvent &event);
    void OnMiddleDown (wxMouseEvent &event);
    void OnKeyDown (wxKeyEvent& event);
    bool cancel_mouse_left_press();
    void set_scale();
    bool is_zoomable(); //false if kind is eg. empty or peak-position
    void OnPopupPlot (wxCommandEvent& event);
    void OnPopupPlotCtr (wxCommandEvent& event);
    void OnPopupColor (wxCommandEvent& event);
    void OnPopupYZoom (wxCommandEvent& event);
    void OnPopupYZoomFit (wxCommandEvent& event);
    void OnPopupYZoomAuto (wxCommandEvent& event);
    void OnTicsFont (wxCommandEvent& WXUNUSED(event)) { change_tics_font(); }
    void save_settings(wxConfigBase *cf) const;
    void read_settings(wxConfigBase *cf);

private:
    wxString name;
    Aux_plot_kind_enum kind;
    bool mark_peak_ctrs;
    fp y_zoom, y_zoom_base;
    bool auto_zoom_y;
    bool fit_y_once;
    wxCursor cursor;
    static const int move_plot_margin_width = 20;

    void draw_diff (wxDC& dc, std::vector<Point>::const_iterator first,
                                std::vector<Point>::const_iterator last);
    void draw_zoom_text(wxDC& dc);
    void fit_y_zoom(Data const* data, Sum const* sum);

    DECLARE_EVENT_TABLE()
};

// utilities

inline wxColour invert_colour(const wxColour& col)
{ return wxColour(255 - col.Red(), 255 - col.Green(), 255 - col.Blue()); }

fp scale_tics_step (fp beg, fp end, int max_tics);

#endif 

