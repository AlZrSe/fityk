// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#ifndef FITYK__WX_DLG__H__
#define FITYK__WX_DLG__H__

#include <wx/spinctrl.h>
#include <wx/listctrl.h> 
//#include "wx_common.h"

class SumHistoryDlg : public wxDialog
{
public:
    SumHistoryDlg (wxWindow* parent, wxWindowID id);
    void OnUpButton           (wxCommandEvent& event);
    void OnDownButton         (wxCommandEvent& event);
    void OnComputeWssrButton  (wxCommandEvent& event);
    void OnSelectedItem       (wxListEvent&    event);
    void OnActivatedItem      (wxListEvent&    event); 
    void OnViewSpinCtrlUpdate (wxSpinEvent&    event); 
    void OnClose (wxCommandEvent& ) { close_it(this); }
protected:
    int view[4], view_max;
    wxListCtrl *lc;
    wxBitmapButton *up_arrow, *down_arrow;
    wxButton *compute_wssr_button;

    void initialize_lc();
    void update_selection();
    void add_item_to_lc(int pos, std::vector<fp> const& item);
    DECLARE_EVENT_TABLE()
};


class FitRunDlg : public wxDialog
{
public:
    FitRunDlg(wxWindow* parent, wxWindowID id, bool initialize);
    void OnOK(wxCommandEvent& event);
    void OnSpinEvent(wxSpinEvent &) { update_unlimited(); }
    void OnChangeDsOrMethod(wxCommandEvent&) { update_allow_continue(); }
private:
    wxRadioBox* ds_rb;
    wxChoice* method_c;
    wxCheckBox* initialize_cb;
    SpinCtrl *maxiter_sc, *maxeval_sc;
    wxStaticText *nomaxeval_st, *nomaxiter_st;

    void update_unlimited();
    void update_allow_continue();
    DECLARE_EVENT_TABLE()
};


bool export_data_dlg(wxWindow *parent, bool load_exported=false);

class DataExportDlg : public wxDialog
{
public:
    DataExportDlg(wxWindow* parent, wxWindowID id, std::string const& ds);
    void OnRadioChanged(wxCommandEvent&) { on_widget_change(); }
    void OnInactiveChanged(wxCommandEvent&) { on_widget_change(); }
    void OnTextChanged(wxCommandEvent&);
    void OnOk(wxCommandEvent& event);
    void on_widget_change();
    std::string get_columns() { return wx2s(text->GetValue()); }
protected:
    wxRadioBox *rb;
    wxCheckBox *inactive_cb;
    wxTextCtrl *text;
    wxArrayString cv;
    DECLARE_EVENT_TABLE()
};


/// dialog Help->About
class AboutDlg : public wxDialog
{
public:
    AboutDlg(wxWindow* parent);
    void OnTextURL(wxTextUrlEvent& event);
private:
    wxTextCtrl *txt;
    DECLARE_EVENT_TABLE()
};


#endif

