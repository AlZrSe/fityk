// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License version 2
// $Id$

#ifndef FITYK__WX_FANCYRC__H__
#define FITYK__WX_FANCYRC__H__

#include <string>

class SideBar;
class KFTextCtrl;

class FancyRealCtrl : public wxPanel
{
public:
    FancyRealCtrl(wxWindow* parent, wxWindowID id, 
                  double value, std::string const& tc_name, bool locked_,
                  SideBar const* draw_handler_);
    void change_value(double factor);
    void on_stop_changing();
    void set(double value, std::string const& tc_name);
    double get_value() const;
    std::string get_name() const { return name; }
    bool get_locked() const { return locked; }
    void set_temporary_value(double value); 
    void toggle_lock(bool exec); 
    void connect_to_onkeydown(wxObjectEventFunction function, 
                              wxEvtHandler* sink);
private:
    double initial_value;
    std::string name;
    bool locked;
    SideBar const* draw_handler;
    KFTextCtrl *tc;
    wxBitmapButton *lock_btn;
    wxWindow *vch; //ValueChangingWidget

    wxBitmap get_lock_bitmap() const;
    void OnTextEnter(wxCommandEvent &) { on_stop_changing(); }
    void OnLockButton(wxCommandEvent&) { toggle_lock(true); }

    DECLARE_EVENT_TABLE()
};

#endif
