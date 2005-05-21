// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#ifndef WX_MPLOT__H__
#define WX_MPLOT__H__

#include "wx_plot.h"
#include "numfuncs.h" // B_point definition


/// it cares about visualization of spline / polyline background 
/// which can be set by selecting points on Plot

struct t_xy { fp x, y; };

class BgManager
{
public:
    BgManager() : min_bg_distance(0.5), spline_bg(true) {}
    void add_background_point(fp x, fp y);
    void rm_background_point(fp x);
    void clear_background();
    void recompute_bgline();
protected:
    typedef std::vector<B_point>::iterator bg_iterator;
    typedef std::vector<B_point>::const_iterator bg_const_iterator;
    std::vector<B_point> bg;
    std::vector<t_xy> bgline;
    fp min_bg_distance;
    bool spline_bg;
};


/// main plot, single in application, displays data, fitted peaks etc. 
class MainPlot : public FPlot, public BgManager
{
public:
    MainPlot (wxWindow *parent, Plot_shared &shar); 
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
    void OnTicsFont (wxCommandEvent& event);
    void OnPopupRadius (wxCommandEvent& event);
    void OnZoomAll (wxCommandEvent& event);
    void PeakInfo ();
    void OnPeakInfo (wxCommandEvent& WXUNUSED(event)) { PeakInfo(); }
    void OnPeakDelete (wxCommandEvent& event);
    void OnPeakShowTree (wxCommandEvent& event);
    void OnPeakGuess(wxCommandEvent &event);
    void cancel_mouse_press();
    void save_settings(wxConfigBase *cf) const;
    void read_settings(wxConfigBase *cf);
    void update_mouse_hints();
    void set_mouse_mode(Mouse_mode_enum m);
    Mouse_mode_enum get_mouse_mode() const { return mode; }

private:
    Mouse_mode_enum mode, basic_mode;
    static const int max_phase_pens = 8;
    static const int max_peak_pens = 24;
    static const int max_radius = 4; //size of data point
    bool smooth;
    bool peaks_visible, phases_visible, sum_visible, data_visible, 
         plabels_visible; 
    wxFont plabelFont;
    std::string plabel_format;
    std::vector<std::string> plabels;
    wxPen sumPen, bg_pointsPen;
    wxPen phasePen[max_phase_pens], peakPen[max_peak_pens];
    int pressed_mouse_button;
    bool ctrl;
    int over_peak;

    void draw_x_axis (wxDC& dc, std::vector<Point>::const_iterator first,
                                   std::vector<Point>::const_iterator last);
    void draw_background(wxDC& dc); 
    void draw_sum (wxDC& dc, std::vector<Point>::const_iterator first,
                   std::vector<Point>::const_iterator last);
    void draw_phases (wxDC& dc, std::vector<Point>::const_iterator first,
                      std::vector<Point>::const_iterator last);
    void draw_peaks (wxDC& dc, std::vector<Point>::const_iterator first,
                     std::vector<Point>::const_iterator last);
    void buffer_peaks (std::vector<Point>::const_iterator first,
                       std::vector<Point>::const_iterator last);
    void draw_peaktops (wxDC& dc);
    void draw_plabels (wxDC& dc);
    void prepare_peaktops();
    void prepare_peak_labels();
    void look_for_peaktop (wxMouseEvent& event);
    void show_popup_menu (wxMouseEvent &event);
    void show_peak_menu (wxMouseEvent &event);
    void peak_draft (Mouse_act_enum ma, wxMouseEvent &event =dummy_mouse_event);
    void move_peak (Mouse_act_enum ma, wxMouseEvent &event = dummy_mouse_event);
    void draw_peak_draft (int X_mid, int X_hwhm, int Y, float Shape=0., 
                                                      const f_names_type *f=0);
    bool rect_zoom (Mouse_act_enum ma, wxMouseEvent &event = dummy_mouse_event);
    void draw_rect (int X1, int Y1, int X2, int Y2);
    bool has_mod_keys(const wxMouseEvent& event); 
    void change_peak_parameters(const std::vector<fp> &peak_hcw);
    bool visible_peaktops(Mouse_mode_enum mode);

    DECLARE_EVENT_TABLE()
};

#endif //WX_MPLOT__H__
