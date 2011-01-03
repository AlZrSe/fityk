// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+

#ifndef FITYK__WX_DLG__H__
#define FITYK__WX_DLG__H__

#include <vector>
#include <wx/spinctrl.h>

#include "cmn.h"

class FitRunDlg : public wxDialog
{
public:
    FitRunDlg(wxWindow* parent, wxWindowID id, bool initialize);
    void OnOK(wxCommandEvent& event);
    void OnSpinEvent(wxSpinEvent &) { update_unlimited(); }
    void OnChangeDsOrMethod(wxCommandEvent&) { update_allow_continue(); }
private:
    wxRadioBox* data_rb;
    wxChoice* method_c;
    wxCheckBox* initialize_cb, *autoplot_cb;
    SpinCtrl *maxiter_sc, *maxeval_sc;
    wxStaticText *nomaxeval_st, *nomaxiter_st;
    std::vector<int> sel; // indices of selected datasets

    void update_unlimited();
    void update_allow_continue();
    DECLARE_EVENT_TABLE()
};


bool export_data_dlg(wxWindow *parent, bool load_exported=false);

class DataExportDlg : public wxDialog
{
public:
    DataExportDlg(wxWindow* parent, wxWindowID id, int data_idx);
    void OnRadioChanged(wxCommandEvent&) { on_widget_change(); }
    void OnInactiveChanged(wxCommandEvent&) { on_widget_change(); }
    void OnTextChanged(wxCommandEvent&);
    void OnOk(wxCommandEvent& event);
    void on_widget_change();
    std::string get_columns() { return wx2s(text->GetValue()); }
protected:
    int data_idx_;
    wxRadioBox *rb;
    wxCheckBox *only_a_cb;
    wxTextCtrl *text;
    wxArrayString cv;

    bool is_custom() const { return rb->GetSelection() == (int) cv.GetCount(); }

    DECLARE_EVENT_TABLE()
};


class DataListPlusText;

class MergePointsDlg : public wxDialog
{
public:
    MergePointsDlg(wxWindow* parent, wxWindowID id=wxID_ANY);
    std::string get_command();
    void update_info();
    void OnCheckBox(wxCommandEvent&) { y_rb->Enable(dx_cb->GetValue()); }

private:
    int focused_data;
    wxRadioBox *y_rb, *output_rb;
    wxCheckBox *dx_cb;
    RealNumberCtrl *dx_val;
    wxTextCtrl *inf;
    DECLARE_EVENT_TABLE()
};


#endif

