// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#ifndef FITYK__WX_MPLOT__H__
#define FITYK__WX_MPLOT__H__

#include "wx_plot.h"
#include "numfuncs.h" // B_point definition


/// it cares about visualization of spline / polyline background 
/// which can be set by selecting points on Plot

struct t_xy { fp x, y; };

class BgManager
{
public:
    BgManager(PlotShared &x_calc_) : x_calc(x_calc_), min_dist(8), 
                                     spline_bg(true) {}
    void add_background_point(fp x, fp y);
    void rm_background_point(fp x);
    void clear_background();
    void strip_background();
    bool bg_empty() const { return bg.empty(); }
    void set_spline_bg(bool s) { spline_bg=s; recompute_bgline(); }
protected:
    PlotShared &x_calc;
    int min_dist; //minimal distance in X between bg points
    bool spline_bg;
    typedef std::vector<B_point>::iterator bg_iterator;
    typedef std::vector<B_point>::const_iterator bg_const_iterator;
    std::vector<B_point> bg;
    std::vector<t_xy> bgline;

    void recompute_bgline();
};


/// main plot, single in application, displays data, fitted peaks etc. 
class MainPlot : public FPlot, public BgManager
{
public:
    MainPlot (wxWindow *parent, PlotShared &shar); 
    ~MainPlot() {}
    void OnPaint(wxPaintEvent &event);
    void Draw(wxDC &dc);
    void OnLeaveWindow (wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent &event);
    void OnButtonDown (wxMouseEvent &event);
    void OnLeftDClick (wxMouseEvent& WXUNUSED(event)) { PeakInfo(); }
    void OnButtonUp (wxMouseEvent &event);
    void OnKeyDown (wxKeyEvent& event);
    void set_scale();

    void OnPopupShowXX (wxCommandEvent& event);
    void OnPopupColor (wxCommandEvent& event);
    void OnInvertColors (wxCommandEvent& event);
    void OnPeakLabel (wxCommandEvent& event);
    void OnPlabelFont (wxCommandEvent& event);
    void OnTicsFont (wxCommandEvent& WXUNUSED(event)) { change_tics_font(); }
    void OnPopupRadius (wxCommandEvent& event);
    void OnZoomAll (wxCommandEvent& event);
    void PeakInfo ();
    void OnPeakInfo (wxCommandEvent& WXUNUSED(event)) { PeakInfo(); }
    void OnPeakDelete (wxCommandEvent& event);
    void OnPeakGuess(wxCommandEvent &event);
    void cancel_mouse_press();
    void save_settings(wxConfigBase *cf) const;
    void read_settings(wxConfigBase *cf);
    void update_mouse_hints();
    void set_mouse_mode(MouseModeEnum m);
    MouseModeEnum get_mouse_mode() const { return mode; }
    wxColour const& get_data_color(int n) const
        { return dataColour[n % max_data_pens]; }
    wxColour const& get_func_color(int n) const
        { return peakPen[n % max_peak_pens].GetColour(); }
    void set_data_color(int n, wxColour const& col) 
        { dataColour[n % max_data_pens] = col; }
    void set_func_color(int n, wxColour const& col) 
        { peakPen[n % max_peak_pens].SetColour(col); }

private:
    MouseModeEnum basic_mode, 
                    mode;  //actual mode -- either basic_mode or mmd_peak
    static const int max_group_pens = 8;
    static const int max_peak_pens = 32;
    static const int max_data_pens = 32;
    static const int max_radius = 4; //size of data point
    bool peaks_visible, groups_visible, sum_visible, data_visible, 
         plabels_visible; 
    wxFont plabelFont;
    std::string plabel_format;
    std::vector<std::string> plabels;
    wxPen sumPen, bg_pointsPen;
    wxPen groupPen[max_group_pens], peakPen[max_peak_pens];
    wxColour dataColour[max_data_pens];
    int pressed_mouse_button;
    bool ctrl;
    int over_peak;

    void draw_x_axis (wxDC& dc);
    void draw_background(wxDC& dc); 
    void draw_sum (wxDC& dc, Sum const* sum);
    void draw_groups (wxDC& dc, Sum const* sum);
    void draw_peaks (wxDC& dc, Sum const* sum);
    void draw_peaktops (wxDC& dc, Sum const* sum);
    void draw_peaktop_selection(wxDC& dc, Sum const* sum);
    void draw_plabels (wxDC& dc, Sum const* sum);
    void draw_dataset(wxDC& dc, int n);
    void prepare_peaktops(Sum const* sum);
    void prepare_peak_labels(Sum const* sum);
    void look_for_peaktop (wxMouseEvent& event);
    void show_popup_menu (wxMouseEvent &event);
    void show_peak_menu (wxMouseEvent &event);
    void peak_draft (Mouse_act_enum ma, wxMouseEvent &event =dummy_mouse_event);
    void move_peak (Mouse_act_enum ma, wxMouseEvent &event = dummy_mouse_event);
    void draw_peak_draft (int X_mid, int X_hwhm, int Y, float Shape=0./*, 
                                                  const f_names_type *f=0*/);
    bool rect_zoom (Mouse_act_enum ma, wxMouseEvent &event = dummy_mouse_event);
    void draw_rect (int X1, int Y1, int X2, int Y2);
    bool has_mod_keys(const wxMouseEvent& event); 
    void change_peak_parameters(const std::vector<fp> &peak_hcw);
    bool visible_peaktops(MouseModeEnum mode);

    DECLARE_EVENT_TABLE()
};

#endif 
