// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#ifndef FITYK__WX_PANE__H__
#define FITYK__WX_PANE__H__

#include <wx/config.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <list>
#include <utility>
#include "wx_common.h"  //for MouseModeEnum, OutputStyle

class PlotPane;
class IOPane;
class MainPlot;
class AuxPlot;
class FPlot;
class PlotCore;
class BgManager;
class FancyRealCtrl;
class Variable;
class Function;
class SideBar;
class GradientDlg;


class InputField : public wxTextCtrl
{
public:
    InputField(wxWindow *parent, wxWindowID id,
               const wxString& value = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0)
    : wxTextCtrl(parent, id, value, pos, size, style), 
      history(1), h_pos(history.begin()), spin_button(0) {} 
    void set_spin_button(wxSpinButton *sb) { spin_button = sb; }
    void history_up();
    void history_down();
protected:
    void OnKeyDown (wxKeyEvent& event);

    std::list<wxString> history;
    std::list<wxString>::iterator h_pos;
    wxSpinButton *spin_button;

    DECLARE_EVENT_TABLE()
};


class OutputWin : public wxTextCtrl
{
public:
    OutputWin (wxWindow *parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition, 
                const wxSize& size = wxDefaultSize);
    void append_text (OutputStyle style, const wxString& str);
    void OnRightDown (wxMouseEvent& event);
    void OnPopupColor  (wxCommandEvent& event);       
    void OnPopupFont   (wxCommandEvent& event);  
    void OnPopupClear  (wxCommandEvent& event); 
    void OnKeyDown     (wxKeyEvent& event);
    void save_settings(wxConfigBase *cf) const;
    void read_settings(wxConfigBase *cf);
    void fancy_dashes();

private:
    wxColour text_color[4]; 
    wxColour bg_color;

    DECLARE_EVENT_TABLE()
};


class IOPane : public wxPanel
{
public:
    IOPane(wxWindow *parent, wxWindowID id=-1);
    void append_text (OutputStyle style, const wxString& str) 
                                      { output_win->append_text(style, str); }
    void save_settings(wxConfigBase *cf) const;
    void read_settings(wxConfigBase *cf);
    void focus_input(int key);
    void focus_output() { output_win->SetFocus(); }
    void show_fancy_dashes() { output_win->fancy_dashes(); }
    void show_popup_menu(wxMouseEvent& ev) { output_win->OnRightDown(ev); }
    void edit_in_input(std::string const& s);
    void OnSpinButtonUp(wxSpinEvent &) { input_field->history_up(); }
    void OnSpinButtonDown(wxSpinEvent &) { input_field->history_down(); }
private:
    OutputWin *output_win;
    InputField *input_field;
    wxSpinButton *spin_button;

    DECLARE_EVENT_TABLE()
};


class PlotPane : public ProportionalSplitter
{
    friend class FPrintout;
public:
    PlotPane(wxWindow *parent, wxWindowID id=-1);
    void zoom_forward();
    std::string zoom_backward(int n=1);
    void save_settings(wxConfigBase *cf) const;
    void read_settings(wxConfigBase *cf);
    void refresh_plots(bool update, bool only_main=false);
    void set_mouse_mode(MouseModeEnum m);
    void update_mouse_hints();
    bool is_background_white();
    std::vector<std::string> const& get_zoom_hist() const { return zoom_hist; }
    MainPlot const* get_plot() const { return plot; }
    MainPlot* get_plot() { return plot; }
    BgManager* get_bg_manager(); 
    std::vector<FPlot*> const get_visible_plots() const;
    AuxPlot* get_aux_plot(int n) const 
                     { assert(n>=0 && n<2); return aux_plot[n]; }
    void show_aux(int n, bool show); 
    bool aux_visible(int n) const;
    void draw_crosshair(int X, int Y);

    bool crosshair_cursor;
private:
    PlotShared plot_shared;
    MainPlot *plot;
    ProportionalSplitter *aux_split;
    AuxPlot *aux_plot[2];
    std::vector<std::string> zoom_hist;

    void do_draw_crosshair(int X, int Y);

    DECLARE_EVENT_TABLE()
};

class ListWithColors : public wxListView
{
public:
    ListWithColors(wxWindow *parent, wxWindowID id, 
                   std::vector<std::pair<std::string,int> > const& columns_);
    void populate(std::vector<std::string> const& data, 
                  wxImageList* image_list = 0,
                  int active = -2);
    void OnColumnMenu(wxListEvent &event);
    void OnRightDown(wxMouseEvent &event);
    void OnShowColumn(wxCommandEvent &event);
    void OnFitColumnWidths(wxCommandEvent &event);
    void OnSelectAll(wxCommandEvent &event);
    void OnKeyDown (wxKeyEvent& event);
    void set_side_bar(SideBar* sidebar_) { sidebar=sidebar_; }
    DECLARE_EVENT_TABLE()
private:
    std::vector<std::pair<std::string,int> > columns;
    std::vector<std::string> list_data;
    SideBar *sidebar;
};

class ListPlusText : public ProportionalSplitter
{
public:
    ListWithColors *list;
    wxTextCtrl* inf;

    ListPlusText(wxWindow *parent, wxWindowID id, wxWindowID list_id,
                 std::vector<std::pair<std::string,int> > const& columns_);

    void OnSwitchInfo(wxCommandEvent &event);
    void split() { SplitHorizontally(list, inf); }
    DECLARE_EVENT_TABLE()
};


class SideBar : public ProportionalSplitter
{
public:
    SideBar(wxWindow *parent, wxWindowID id=-1);
    void OnDataButtonNew (wxCommandEvent& event);
    void OnDataButtonDup (wxCommandEvent& event);
    void OnDataButtonRen (wxCommandEvent& event);
    void OnDataButtonDel (wxCommandEvent&) { delete_selected_items(); }
    void OnDataButtonCopyF (wxCommandEvent& event);
    void OnDataButtonCol (wxCommandEvent& event);
    void OnDataColorsChanged(GradientDlg *gd);
    void OnDataLookChanged (wxCommandEvent& event);
    void OnDataShiftUpChanged (wxSpinEvent& event);
    void OnFuncButtonNew (wxCommandEvent& event);
    void OnFuncButtonDel (wxCommandEvent&) { delete_selected_items(); }
    void OnFuncButtonEdit (wxCommandEvent& event);
    void OnFuncButtonChType (wxCommandEvent& event);
    void OnFuncButtonCol (wxCommandEvent& event);
    void OnVarButtonNew (wxCommandEvent& event);
    void OnVarButtonDel (wxCommandEvent&) { delete_selected_items(); }
    void OnVarButtonEdit (wxCommandEvent& event);
    void OnFuncFilterChanged (wxCommandEvent& event);
    void OnDataFocusChanged(wxListEvent &event);
    void OnFuncFocusChanged(wxListEvent &event);
    void OnVarFocusChanged(wxListEvent &event);
    void update_lists(bool nondata_changed=true);
    /// get active dataset number -- if none is focused, return first one (0)
    int get_focused_data() const
                     { int n=d->list->GetFocusedItem(); return n==-1 ? 0 : n; }
    int get_active_function() const { return active_function; }
    int get_focused_var() const;
    bool is_func_selected(int n) const { return f->list->IsSelected(n) 
                                           || f->list->GetFocusedItem() == n; }
    int set_selection(int page) { return nb->SetSelection(page); }
    void activate_function(int n);
    std::vector<std::string> get_selected_data() const;
    bool howto_plot_dataset(int n, bool& shadowed, int& offset) const;
    std::vector<std::string> get_selected_func() const;
    std::vector<std::string> get_selected_vars() const;
    void update_data_inf();
    void update_func_inf();
    void update_var_inf();
    void update_bottom_panel();
    void delete_selected_items();
    void draw_function_draft(FancyRealCtrl const* frc) const;
    void change_bp_parameter_value(int idx, fp value);
private:
    wxNotebook *nb;
    wxPanel *data_page, *func_page, *var_page, *bottom_panel;
    wxFlexGridSizer* bp_sizer;
    wxStaticText *bp_label;
    std::vector<FancyRealCtrl*> bp_frc;
    std::vector<wxStaticText*> bp_statict;
    std::vector<bool> bp_sig;
    Function const* bp_func;
    ListPlusText *d, *f, *v;
    wxChoice *data_look, *filter_ch;
    wxSpinCtrl *shiftup_sc;
    int active_function;
    std::string active_function_name;

    void update_data_list(bool nondata_changed);
    void update_func_list(bool nondata_changed);
    void update_var_list();
    void add_variable_to_bottom_panel(Variable const* var, 
                                      std::string const& tv_name);
    void clear_bottom_panel();
    std::vector<bool> make_bottom_panel_sig(Function const* func);
    void do_activate_function();

    DECLARE_EVENT_TABLE()
};


/// displays colors from data member from left to right (one pixel - one color)
template<typename UpdateCalleeT>
class ColorGradientDisplay : public wxPanel
{
public:
    std::vector<wxColour> data;

    ColorGradientDisplay(wxWindow *parent, 
                      UpdateCalleeT *callee, void (UpdateCalleeT::*callback)())
        : wxPanel(parent, -1), updateCallee(callee), updateCallback(callback) 
    { 
        Connect(wxEVT_PAINT, 
                  wxPaintEventHandler(ColorGradientDisplay::OnPaint)); 
    }
    void OnPaint(wxPaintEvent&);
    bool was_resized() { return GetClientSize().GetWidth() != size(data); }
private:
    UpdateCalleeT *updateCallee;
    void (UpdateCalleeT::*updateCallback)();
};

template<typename UpdateCalleeT>
void ColorGradientDisplay<UpdateCalleeT>::OnPaint(wxPaintEvent&)
{
    if (was_resized())
        (updateCallee->*updateCallback)();
    wxPaintDC dc(this);
    int height = GetClientSize().GetHeight();
    wxPen pen;
    for (size_t i = 0; i < data.size(); ++i) {
        pen.SetColour(data[i]);
        dc.SetPen(pen);
        dc.DrawLine(i, 0, i, height);
    }
}


class ColorSpinSelector : public wxPanel
{
public:
    SpinCtrl *r, *g, *b;

    ColorSpinSelector(wxWindow *parent, wxString const& title, 
                      wxColour const& col);
    void OnSelector(wxCommandEvent &);
    DECLARE_EVENT_TABLE()
};


class GradientDlg : public wxDialog
{
public:
    GradientDlg(wxWindow *parent, wxWindowID id, 
                wxColour const& first_col, wxColour const& last_col);
    void OnSpinEvent(wxSpinEvent &) { update_gradient_display(); }
    void OnRadioChanged(wxCommandEvent &) { update_gradient_display(); }
    wxColour get_value(float x);
private:
    wxRadioBox *kind_rb;
    ColorSpinSelector *from, *to;
    ColorGradientDisplay<GradientDlg> *display;

    void update_gradient_display();

    DECLARE_EVENT_TABLE()
};

template<typename calleeT>
class GradientDlgWithApply : public GradientDlg
{
public:
    GradientDlgWithApply(wxWindow *parent, wxWindowID id, 
                wxColour const& first_col, wxColour const& last_col,
                calleeT *callee_, void (calleeT::*callback_)(GradientDlg*))
        : GradientDlg(parent, id, first_col, last_col), 
          callee(callee_), callback(callback_) 
    { 
        Connect(wxID_APPLY, wxEVT_COMMAND_BUTTON_CLICKED,
                  wxCommandEventHandler(GradientDlgWithApply::OnApply)); 
        Connect(wxID_CLOSE, wxEVT_COMMAND_BUTTON_CLICKED,
                  wxCommandEventHandler(GradientDlgWithApply::OnClose)); 
    }
    void OnApply(wxCommandEvent &) { (callee->*callback)(this); }
    void OnClose(wxCommandEvent &event) { OnCancel(event); }
private:
    calleeT *callee;
    void (calleeT::*callback)(GradientDlg *);
};


#endif 

