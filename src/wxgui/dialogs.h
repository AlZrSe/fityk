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
    std::string get_cmd() const;
private:
    wxRadioBox* data_rb;
    wxChoice* method_c;
    wxCheckBox* initialize_cb, *autoplot_cb;
    SpinCtrl *maxiter_sc, *maxeval_sc;
    wxStaticText *nomaxeval_st, *nomaxiter_st;
    std::vector<int> sel; // indices of selected datasets

    void OnSpinEvent(wxSpinEvent &) { update_unlimited(); }
    void OnChangeDsOrMethod(wxCommandEvent&) { update_allow_continue(); }
    void update_unlimited();
    void update_allow_continue();
    DECLARE_EVENT_TABLE()
};


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


// similar to wxTextEntryDialog, but uses wxComboBox instead of wxTextCtrl
class TextComboDlg : public wxDialog
{
public:
    TextComboDlg(wxWindow *parent, const wxString& message,
                 const wxString& caption);
    wxComboBox *combo;
};

#endif

