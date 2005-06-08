// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$


// wxwindows headers, see wxwindows samples for description
#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif


#include <istream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <wx/valtext.h>
#include <wx/bmpbuttn.h>
#include <wx/grid.h>
#include <wx/statline.h>
#include <wx/splitter.h>
#include "common.h"
#include "wx_dlg.h"
#include "wx_common.h"
#include "data.h"
#include "sum.h"
#include "ffunc.h"
#include "gfunc.h"
#include "v_fit.h"
#include "pcore.h"
#include "ui.h"

//bitmaps for buttons
#include "img/up_arrow.xpm"
#include "img/down_arrow.xpm"

using namespace std;


enum {
    ID_BRO_TREE             = 26100,
    ID_BRO_A_WHAT                  ,
    ID_BRO_A_TYPE                  ,
    ID_BRO_A_ADD                   ,
    ID_BRO_A_NRB            = 26110, // and next 10
    ID_BRO_A_TPC            = 26125, // and next 2
    ID_BRO_A_NTC            = 26130,
    ID_BRO_C_LL                    ,
    ID_BRO_C_L                     ,
    ID_BRO_C_R                     ,
    ID_BRO_C_RR                    ,
    ID_BRO_C_TXT                   ,
    ID_BRO_C_SETCB                 ,
    ID_BRO_C_CSETCB                ,
    ID_BRO_C_RB0                   ,
    ID_BRO_C_RB1                   ,
    ID_BRO_C_APPL                  ,
    ID_BRO_D_DEL                   ,
    ID_BRO_D_RCRM                  ,
    ID_BRO_F_FT                    ,
    ID_BRO_F_FA                    ,
    ID_BRO_F_TA                    ,
    ID_BRO_V_BTN                   ,
    ID_BRO_MENU_EXP                , 
    ID_BRO_MENU_COL                , 
    ID_BRO_MENU_BUT                , 
    ID_BRO_MENU_RST                , 
    
    ID_DXLOAD_FTYPE_RB             ,
    ID_DXLOAD_STDDEV_CB            ,
  
    ID_SHIST_LC                    ,
    ID_SHIST_UP                    ,
    ID_SHIST_DOWN                  ,
    ID_SHIST_TSAV                  ,
    ID_SHIST_CWSSR                 ,
    ID_SHIST_V              = 26300, // and next 2
    ID_DE_GRID              = 26310,
    ID_DE_RESET                    ,
    ID_DE_CODE                     ,
    ID_DE_EXAMPLES
};

//======================== FuncBrowserDlg ===========================

BEGIN_EVENT_TABLE(FuncBrowserDlg, wxDialog)
    EVT_TREE_SEL_CHANGED (ID_BRO_TREE,     FuncBrowserDlg::OnSelChanged)
    EVT_CHOICE           (ID_BRO_A_WHAT,   FuncBrowserDlg::OnAddWhatChoice)
    EVT_CHOICE           (ID_BRO_A_TYPE,   FuncBrowserDlg::OnAddTypeChoice)
    EVT_COMMAND_RANGE    (ID_BRO_A_NRB, ID_BRO_A_NRB + 9, 
                          wxEVT_COMMAND_RADIOBUTTON_SELECTED, 
                                           FuncBrowserDlg::OnParNumChosen)
    EVT_COMMAND_RANGE    (ID_BRO_A_TPC,  ID_BRO_A_TPC + 2,
                          wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                                           FuncBrowserDlg::OnAddTNCRadio)
    EVT_TEXT             (ID_BRO_A_NTC,    FuncBrowserDlg::OnAddValText)
    EVT_BUTTON           (ID_BRO_A_ADD,    FuncBrowserDlg::OnAddAddButton)
    EVT_BUTTON           (ID_BRO_C_APPL,   FuncBrowserDlg::OnChangeButton)
    EVT_COMMAND_RANGE    (ID_BRO_C_LL,ID_BRO_C_RR, wxEVT_COMMAND_BUTTON_CLICKED,
                                           FuncBrowserDlg::OnArrowButton)
    EVT_CHECKBOX         (ID_BRO_C_SETCB,  FuncBrowserDlg::OnSetDomCheckBox)
    EVT_RADIOBUTTON      (ID_BRO_C_RB0,    FuncBrowserDlg::OnSetDomCheckBox)
    EVT_RADIOBUTTON      (ID_BRO_C_RB1,    FuncBrowserDlg::OnSetDomCheckBox)
    EVT_CHECKBOX         (ID_BRO_C_CSETCB, FuncBrowserDlg::OnSetDomCtrCheckBox)
    EVT_BUTTON           (ID_BRO_D_DEL,    FuncBrowserDlg::OnDeleteButton)
    EVT_CHECKBOX         (ID_BRO_D_RCRM,   FuncBrowserDlg::OnRRCheckBox)
    EVT_BUTTON           (ID_BRO_F_FT,     FuncBrowserDlg::OnFreezeButton)
    EVT_BUTTON           (ID_BRO_F_FA,     FuncBrowserDlg::OnFreezeAllButton)
    EVT_BUTTON           (ID_BRO_F_TA,     FuncBrowserDlg::OnFreezeAllButton)
    EVT_BUTTON           (ID_BRO_V_BTN,    FuncBrowserDlg::OnValueButton)
END_EVENT_TABLE()

FuncBrowserDlg::FuncBrowserDlg (wxWindow* parent, wxWindowID id, int tab) 
    :  wxDialog(parent, id, wxString("Functions Browser"), 
            wxDefaultPosition, wxSize(200, 500), 
            wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
       add_what(0), initialized(false)
{
    wxBoxSizer *top_sizer = new wxBoxSizer (wxVERTICAL);

    //tree
    func_tree = new FuncTree(this, ID_BRO_TREE);
    top_sizer->Add (func_tree, 1, wxEXPAND);

    //notebook
    wxNotebook *notebook = new wxNotebook(this, -1 );
    wxNotebookSizer *nbsizer = new wxNotebookSizer(notebook);
    top_sizer->Add (nbsizer, 0, wxEXPAND);

    //page "Info"
    wxPanel *p_info = new wxPanel(notebook, -1);
    wxSizer *panelsizer_i = new wxBoxSizer (wxVERTICAL);
    info_text = new wxTextCtrl (p_info, -1, my_sum->general_info().c_str(), 
                                wxDefaultPosition, wxSize(-1, 100), 
                                wxTE_MULTILINE|wxTE_READONLY);
    panelsizer_i->Add (info_text, 1, wxEXPAND);
    p_info->SetSizer(panelsizer_i);
    notebook->AddPage (p_info, "info");

    //page "Add"
    p_add = new wxPanel(notebook, -1);
    wxSizer *panelsizer_a = new wxBoxSizer (wxVERTICAL);
    wxSizer *ah_sizer = new wxBoxSizer (wxHORIZONTAL);
    //TODO: change these ^/$/< names
    wxString fzg_choices[] = { "^function", "$parameter", "<zero-shift" };
    fzg_choice = new wxChoice (p_add, ID_BRO_A_WHAT, 
                               wxDefaultPosition, wxDefaultSize,
                               3, fzg_choices);
    fzg_choice->SetSelection(0);
    ah_sizer->Add (fzg_choice, 0, wxALL, 5);
    type_choice = new wxChoice (p_add, ID_BRO_A_TYPE);
    ah_sizer->Add (type_choice, 1, wxALL|wxEXPAND, 5);
    panelsizer_a->Add (ah_sizer, 0, wxEXPAND); 
    wxSizer *ah2_sizer = new wxBoxSizer (wxHORIZONTAL);
    add_preview_tc = new wxTextCtrl (p_add, -1, "", 
                                     wxDefaultPosition, wxDefaultSize, 
                                     wxTE_READONLY);
    add_preview_tc->Enable (false);
    ah2_sizer->Add (add_preview_tc, 1);
    add_add_button = new wxButton (p_add, ID_BRO_A_ADD, "Add");
    ah2_sizer->Add (add_add_button, 0, wxALIGN_RIGHT);
    //ah2_sizer added to panelsizer_a below 
    ah3_sizer = new wxBoxSizer (wxHORIZONTAL);
    ah3_sizer->Add (new wxStaticText (p_add, -1, "Parameter"), 
                    0, wxALIGN_CENTER|wxALL, 5);
    panelsizer_a->Add (ah3_sizer, 0, wxEXPAND); 
    add_box = new wxStaticBox (p_add, -1, "");
    wxStaticBoxSizer *ahs_sizer = new wxStaticBoxSizer (add_box, wxVERTICAL);
    tpc_rb[0] = new wxRadioButton (p_add, ID_BRO_A_TPC + 0, "",
                                 wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    ahs_sizer->Add (tpc_rb[0], 0, wxLEFT|wxEXPAND, 5);
    wxSizer *ah4_sizer = new wxBoxSizer (wxHORIZONTAL);
    wxSizer *av1_sizer = new wxBoxSizer (wxVERTICAL);
    tpc_rb[1] = new wxRadioButton (p_add, ID_BRO_A_TPC + 1,"new parameter");
    av1_sizer->Add (tpc_rb[1], 0, wxLEFT, 5);
    tpc_rb[2] = new wxRadioButton (p_add, ID_BRO_A_TPC + 2, "constant");
    av1_sizer->Add (tpc_rb[2], 0, wxLEFT, 5);
    ah4_sizer->Add (av1_sizer, 0);
    ah4_sizer->Add (new wxStaticText(p_add, -1, "value:"), 
                    0, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    add_p_val_tc = new wxTextCtrl (p_add, ID_BRO_A_NTC, "", 
                                   wxDefaultPosition, wxSize(50, -1), 0,
                                   wxTextValidator(wxFILTER_NUMERIC, 0));
    ah4_sizer->Add (add_p_val_tc, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    ahs_sizer->Add (ah4_sizer, 0);
    panelsizer_a->Add (ahs_sizer, 0, wxEXPAND); 
    panelsizer_a->Add (ah2_sizer, 0, wxEXPAND|wxTOP, 5); 

    p_add->SetSizer(panelsizer_a);
    set_list_of_fzg_types();
    notebook->AddPage (p_add, "add");

    //page "Change"
    p_change = new wxPanel(notebook, -1);
    wxSizer *panelsizer_c = new wxBoxSizer (wxVERTICAL);
    ch_label = new wxStaticText (p_change, -1, "Select parameter.");
    panelsizer_c->Add (ch_label, 0, wxALIGN_CENTER|wxADJUST_MINSIZE);
    panelsizer_c->Add (new wxStaticLine(p_change, -1), 
                       0, wxEXPAND|wxLEFT|wxRIGHT, 10);
    wxSizer *hsizer = new wxBoxSizer (wxHORIZONTAL);
    hsizer->Add (new wxButton (p_change, ID_BRO_C_LL, "<<", 
                               wxDefaultPosition, wxSize(25, -1), 
                               wxBU_EXACTFIT), 
                 0, wxALIGN_CENTER);
    hsizer->Add (new wxButton (p_change, ID_BRO_C_L, " < ", 
                               wxDefaultPosition, wxSize(25, -1), 
                               wxBU_EXACTFIT), 
                 0, wxALIGN_CENTER);
    ch_edit = new wxTextCtrl (p_change, ID_BRO_C_TXT, "", 
                              wxDefaultPosition, wxSize(70, -1), 
                              0, wxTextValidator (wxFILTER_NUMERIC, 0));
    hsizer->Add (ch_edit, 0, wxALIGN_CENTER);
    hsizer->Add (new wxButton (p_change, ID_BRO_C_R, " > ", 
                               wxDefaultPosition, wxSize(25, -1), 
                               wxBU_EXACTFIT), 
                 0, wxALIGN_CENTER);
    hsizer->Add (new wxButton (p_change, ID_BRO_C_RR, ">>", 
                               wxDefaultPosition, wxSize(25, -1), 
                               wxBU_EXACTFIT), 
                 0, wxALIGN_CENTER);
    panelsizer_c->Add (hsizer, 0, wxALIGN_CENTER); 
    panelsizer_c->Add (new wxStaticLine(p_change, -1), 
                       0, wxEXPAND|wxLEFT|wxRIGHT, 10);
    dom_set_cb = new wxCheckBox (p_change, ID_BRO_C_SETCB, 
                                 "s&et domain of parameter");
    panelsizer_c->Add (dom_set_cb, 0, wxALIGN_LEFT);
    wxSizer *hsizer2 = new wxBoxSizer (wxHORIZONTAL);
    ch_dom_rb[0] = new wxRadioButton (p_change, ID_BRO_C_RB0, "");
    hsizer2->Add (ch_dom_rb[0], 0, wxLEFT, 10);
    dom_ctr_set_cb = new wxCheckBox (p_change, ID_BRO_C_CSETCB, "");
    hsizer2->Add (dom_ctr_set_cb, 0);
    ch_ctr = new wxTextCtrl (p_change, -1, "", 
                             wxDefaultPosition, wxSize(60, -1),
                             0, wxTextValidator (wxFILTER_NUMERIC, 0));
    hsizer2->Add (ch_ctr, 0);
    ch_dom_label[0] = new wxStaticText (p_change, -1, "+-");
    hsizer2->Add (ch_dom_label[0], 0, wxALL, 5);
    ch_sigma = new wxTextCtrl (p_change, -1, "", 
                               wxDefaultPosition, wxSize(60, -1),
                               0, wxTextValidator (wxFILTER_NUMERIC, 0));
    hsizer2->Add (ch_sigma, 0);
    panelsizer_c->Add (hsizer2, 0, wxALIGN_LEFT); 
    wxSizer *hsizer3 = new wxBoxSizer (wxHORIZONTAL);
    ch_dom_rb[1] = new wxRadioButton (p_change, ID_BRO_C_RB1, "");
    hsizer3->Add (ch_dom_rb[1], 0, wxLEFT, 10);
    ch_dom_label[1] = new wxStaticText (p_change, -1, "[");
    hsizer3->Add (ch_dom_label[1], 0, wxALL, 5);
    ch_left_b = new wxTextCtrl (p_change, -1, "", 
                                wxDefaultPosition, wxSize(60, -1),
                                0, wxTextValidator (wxFILTER_NUMERIC, 0));
    hsizer3->Add (ch_left_b, 0);
    ch_dom_label[2] = new wxStaticText (p_change, -1, ";");
    hsizer3->Add (ch_dom_label[2], 0, wxALL, 5);
    ch_right_b = new wxTextCtrl (p_change, -1, "", 
                                 wxDefaultPosition, wxSize(60, -1),
                                 0, wxTextValidator (wxFILTER_NUMERIC, 0));
    hsizer3->Add (ch_right_b, 0);
    ch_dom_label[3] = new wxStaticText (p_change, -1, "]");
    hsizer3->Add (ch_dom_label[3], 0, wxALL, 5);
    panelsizer_c->Add (hsizer3, 0, wxALIGN_LEFT); 
    wxSizer *hsizer4 = new wxBoxSizer (wxHORIZONTAL);
    panelsizer_c->Add (hsizer4, 0, wxALIGN_LEFT); 
    panelsizer_c->Add (new wxStaticLine(p_change, -1), 
                       0, wxEXPAND|wxLEFT|wxRIGHT, 10);
    wxSizer *hsizer5 = new wxBoxSizer (wxHORIZONTAL);
    hsizer5->Add (new wxStaticText(p_change, -1, "default domain width"),
                  0, wxALL|wxALIGN_CENTER, 5);
    ch_def_dom_w = new wxTextCtrl (p_change, -1, "", 
                                   wxDefaultPosition, wxSize(60, -1),
                                   0, wxTextValidator (wxFILTER_NUMERIC, 0));
    hsizer5->Add (ch_def_dom_w, 0, wxALIGN_CENTER);
    hsizer5->Add (new wxStaticText(p_change, -1, "%"),
                  0, wxALL|wxALIGN_CENTER, 5);
    panelsizer_c->Add (hsizer5, 0, wxALIGN_CENTER); 
    panelsizer_c->Add (new wxStaticLine(p_change, -1), 
                       0, wxEXPAND|wxLEFT|wxRIGHT, 10);
    panelsizer_c->Add (new wxButton (p_change, ID_BRO_C_APPL, "&Apply"),
                       0, wxALL|wxALIGN_RIGHT, 5);

    p_change->SetSizer(panelsizer_c);
    notebook->AddPage (p_change, "change");
    p_change->Enable(false);

    //page "Delete/Freeze"
    wxPanel *p_delete = new wxPanel(notebook, -1);
    wxSizer *panelsizer_d = new wxBoxSizer (wxVERTICAL);
    panelsizer_d->Add (1, 1, 1);
    del_label = new wxStaticText (p_delete, -1, "");
    panelsizer_d->Add (del_label, 0, wxALIGN_LEFT|wxADJUST_MINSIZE);
    del_button = new wxButton (p_delete, ID_BRO_D_DEL, "Delete [none]"); 
    panelsizer_d->Add (del_button, 0, wxALL|wxALIGN_CENTER, 5);
    del_button->Enable(false);

    wxCheckBox *rr_cb = new wxCheckBox (p_delete, ID_BRO_D_RCRM, 
                                        "recursive-remove");
    string value;
    my_sum->getp_core ("recursive-remove", value);
    assert (isdigit(value[0]));
    rr_cb->SetValue (value[0] - '0');
    panelsizer_d->Add (rr_cb, 0);
    panelsizer_d->Add (1, 1, 1);
    panelsizer_d->Add (new wxStaticLine(p_delete, -1), 0, 
                       wxEXPAND|wxLEFT|wxRIGHT, 10);
    panelsizer_d->Add (1, 1, 1);
    //---
    panelsizer_d->Add (new wxStaticText (p_delete, -1, 
                                         "Frozen parameters:"), 
                       0, wxALIGN_CENTER);
    frozen_tc = new wxTextCtrl (p_delete, -1, "", 
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_READONLY);
    update_frozen_tc();
    panelsizer_d->Add (frozen_tc, 0, wxALL|wxEXPAND, 5);
    freeze_button = new wxButton (p_delete, ID_BRO_F_FT, "&Freeze [none]");
    freeze_button->Enable(false);
    panelsizer_d->Add (freeze_button, 0, wxALL|wxALIGN_CENTER, 5);
    wxBoxSizer *fh_sizer = new wxBoxSizer (wxHORIZONTAL);
    fh_sizer->Add (new wxButton(p_delete, ID_BRO_F_FA, "Freeze All"), 
                   0, wxALL, 5);
    fh_sizer->Add (5, 5, 1);
    fh_sizer->Add (new wxButton(p_delete, ID_BRO_F_TA, "Unfreeze All"), 
                   0, wxALL, 5);
    panelsizer_d->Add (fh_sizer, 0, wxEXPAND);

    panelsizer_d->Add (1, 1, 1);
    p_delete->SetSizer(panelsizer_d);
    notebook->AddPage (p_delete, "delete/freeze");
    //page "Value"
    wxPanel *p_value = new wxPanel(notebook, -1);
    wxSizer *panelsizer_v = new wxBoxSizer (wxVERTICAL);
    wxSizer *vh_sizer = new wxBoxSizer (wxHORIZONTAL);
    vh_sizer->Add (new wxStaticText (p_value, -1, "value of "), 
                   0, wxALIGN_CENTER);
    value_label = new wxStaticText (p_value, -1, "sum    ");
    vh_sizer->Add (value_label, 0, wxALIGN_CENTER);
    vh_sizer->Add (new wxStaticText (p_value, -1, " at "), 0, wxALIGN_CENTER);
    value_at_tc = new wxTextCtrl (p_value, -1, "40.0", 
                                  wxDefaultPosition, wxSize(50, -1)); 
    vh_sizer->Add (value_at_tc, 0, wxALIGN_CENTER);
    panelsizer_v->Add (vh_sizer, 0, wxEXPAND|wxALL, 5);
    panelsizer_v->Add (new wxButton (p_value, ID_BRO_V_BTN, "Compute"),
                       0, wxALIGN_RIGHT);
    value_output_tc = new wxTextCtrl (p_value, -1, "", 
                                      wxDefaultPosition, wxDefaultSize, 
                                      wxTE_MULTILINE|wxTE_READONLY);
    panelsizer_v->Add (value_output_tc, 1, wxEXPAND);
    p_value->SetSizer(panelsizer_v);
    notebook->AddPage (p_value, "value");

    notebook->SetSelection(tab);
    wxButton *button = new wxButton (this, wxID_CANCEL, "&Close");
    top_sizer->Add (button, 0, wxALIGN_CENTER); 
    initialized = true;

    SetSizer (top_sizer);
    top_sizer->SetSizeHints(this);
}

void FuncBrowserDlg::OnSelChanged (wxTreeEvent& WXUNUSED(event))
{
    if (!initialized)
        return;
    wxTreeItemId id = func_tree->GetSelection();
    if (id <= 0) {
        info_text->SetValue ("Select item to see info");
        return;
    }
    string txt = func_tree->GetItemText(id).c_str();
    sel_fun = S(txt[0]);
    for (string::iterator i =txt.begin() + 1; i !=txt.end() && isdigit(*i); ++i)
        sel_fun.push_back (*i);
    char type = sel_fun[0];
    int nr = atoi (sel_fun.c_str() + 1);
    info_text->SetValue (txt.c_str());
    if (type == '^') {
        const V_f* f = my_sum->get_f(nr);
        info_text->AppendText (("\n" + f->extra_description()).c_str());
    }
    else if (type == 's')
        info_text->SetValue (my_sum->general_info().c_str());
    if (type == '@' && my_sum->refs_to_ag(Pag(0., nr)) 
            || type == '$' && my_sum->refs_to_ag(Pag((V_g*)0, nr))
            || type == '^' && my_sum->refs_to_f(nr)) {
        del_label->SetLabel("Can not be removed -- has references");
        del_button->Enable (false);
        del_button->SetLabel ("Delete " + wxString(sel_fun.c_str()));
    }
    else if (type == 's') {
        del_label->SetLabel("Remove everything without references.");
        del_button->Enable (true);
        del_button->SetLabel ("Delete all");
    }
    else {
        del_label->SetLabel("Removing can not be canceled.");
        del_button->Enable (true);
        del_button->SetLabel ("Delete " + wxString(sel_fun.c_str()));
    }
    
    if (type == '@') {
        p_change->Enable (true);
        set_change_initials(nr);
        freeze_button->Enable (true);
        update_freeze_button_label();
    }
    else { //not '@'
        set_change_initials(-1);
        p_change->Enable (false);
        freeze_button->Enable (false);
    }
    value_label->SetLabel (!sel_fun.empty() && sel_fun[0] == 's' 
                           ? "sum" : sel_fun.c_str());
    if (type == '$' || type == '@') {
        par_descr[current_add_p_number].from_tree = sel_fun;
        tpc_rb[0]->SetLabel (("parameter from tree: " + sel_fun).c_str());
        if (par_descr[current_add_p_number].option == 0)
            update_add_preview();
    }
}

void FuncBrowserDlg::set_change_initials(int n)
{
        ch_label->SetLabel (("Parameter @" + S(n) + ":").c_str());
        fp a_value = (n >= 0 ? my_sum->pars()->get_a (n) : 0);
        const Domain &dom = (n >= 0 ? my_sum->pars()->get_domain(n) : Domain());
        string label = (n >= 0 ? ("Parameter @" + S(n) + ":") 
                               : S("Select @parameter."));
        ch_label->SetLabel (label.c_str());
        ch_edit->SetValue (S(a_value).c_str());
        dom_set_cb->SetValue(dom.is_set());
        ch_dom_rb[0]->SetValue(true);
        ch_dom_rb[1]->SetValue(false);
        dom_ctr_set_cb->SetValue(dom.is_ctr_set());
        ch_ctr->SetValue (dom.is_ctr_set() ? S(dom.Ctr()).c_str() : "value");
        fp defrel = my_sum->get_def_rel_domain_width();
        fp ctr = dom.is_ctr_set() ? dom.Ctr() : a_value;
        fp sig = dom.is_set() ? dom.Sigma() : defrel * a_value;
        ch_sigma->SetValue (S(sig).c_str());
        ch_left_b->SetValue (S(ctr - sig).c_str());
        ch_right_b->SetValue (S(ctr + sig).c_str());
        ch_def_dom_w->SetValue (S(defrel * 100).c_str());
        change_domain_enable();
}

void FuncBrowserDlg::set_list_of_fzg_types()
{
    const wxString label = fzg_choice->GetStringSelection();
    if (label[0] == add_what)
        return;
    add_what = label[0];
    type_choice->Clear();
    One_of_fzg fzg = V_fzg::type_of_symbol(add_what);
    all_t = V_fzg::all_types(fzg);
    for (vector<const z_names_type*>::const_iterator i = all_t.begin(); 
                                                        i != all_t.end(); i++)
        type_choice->Append ((*i)->name.c_str());
    assert (type_choice->GetCount() != 0);
    type_choice->SetSelection (0);
    type_was_chosen();
}

void FuncBrowserDlg::type_was_chosen()
{
    //clear
    for (vector<par_descr_type>::iterator i = par_descr.begin();
            i != par_descr.end(); i++) {
        ah3_sizer->Remove(i->radio);
        i->radio->Destroy();
    }
    par_descr.clear();

    //add
    One_of_fzg fzg = V_fzg::type_of_symbol(add_what);
    int n_sel = type_choice->GetSelection();
    assert (n_sel < size(all_t));
    char type = all_t[n_sel]->type;
    int n = all_t[n_sel]->psize; 

    for (int i = 0; i < n; i++) {
        par_descr_type d;
        d.radio = new wxRadioButton (p_add, ID_BRO_A_NRB + i, S(i).c_str(),
                                     wxDefaultPosition, wxDefaultSize,
                                     i == 0 ? wxRB_GROUP : 0);
        if (fzg == fType) {
            string s = static_cast<const f_names_type*> (V_fzg::type_info(fzg, 
                                                             type))->pnames[i];
            d.name = s.substr(0, s.find(':'));
        }
        else
            d.name =  "a" + S(i);
        d.option = 1;//new parameter
        d.from_tree = "none", d.new_value = "";
        ah3_sizer->Add (d.radio, 0);
        par_descr.push_back (d);
    }
    ah3_sizer->Layout();
    par_descr[0].radio->SetValue (true);
    parameter_number_was_chosen();
}

void FuncBrowserDlg::parameter_number_was_chosen()
{
    vector<par_descr_type>::const_iterator i;
    for (i = par_descr.begin(); i != par_descr.end(); i++) 
        if (i->radio->GetValue() == true)
            break;
    current_add_p_number = i - par_descr.begin();
    add_box->SetLabel (i->name.c_str());
    tpc_rb[0]->SetLabel (("parameter from tree: " + i->from_tree).c_str());
    add_p_val_tc->SetValue (i->new_value.c_str());
    change_tpc_radio (i->option);
}

void FuncBrowserDlg::OnAddTNCRadio (wxCommandEvent& event)
{
    change_tpc_radio (event.GetId() - ID_BRO_A_TPC);
}

void FuncBrowserDlg::change_tpc_radio (int nradio)
{
    assert (nradio >= 0 && nradio < 3);
    par_descr[current_add_p_number].option = nradio;
    tpc_rb[nradio]->SetValue(true);
    for (int i = 0; i < 3; i++) //on MSW previously selected radio has to be
        tpc_rb[i]->SetValue(i == nradio); //unselected 
    add_p_val_tc->Enable (nradio == 1 || nradio == 2);
    update_add_preview();
}

void FuncBrowserDlg::update_add_preview()
{
    bool ok = true;
    string s = S(add_what) + S(all_t[type_choice->GetSelection()]->type) + "  ";
    for (vector<par_descr_type>::iterator i = par_descr.begin();
            i != par_descr.end(); i++) {
        if (i->option == 0 && i->from_tree[0] == 'n' 
                || i->option != 0 && (i->new_value.empty() 
                                      || !is_double(i->new_value))) {
            s += " ?  ";
            ok = false;
        }
        else if (i->option == 0) 
            s += i->from_tree + " ";
        else if (i->option == 1) 
            s += "~" + i->new_value + " ";
        else if (i->option == 2) 
            s += "_" + i->new_value + " ";
    }
    add_preview_tc->SetValue (s.c_str()); 
    add_add_button->Enable (ok);
}

void FuncBrowserDlg::OnAddValText (wxCommandEvent& WXUNUSED(event))
{
    par_descr[current_add_p_number].new_value =add_p_val_tc->GetValue().c_str();
    update_add_preview();
}

void FuncBrowserDlg::OnAddAddButton (wxCommandEvent& WXUNUSED(event))
{
    exec_command (("s.add " + add_preview_tc->GetValue()).c_str());
    func_tree->reset_funcs_in_root();
}

void FuncBrowserDlg::OnChangeButton (wxCommandEvent& WXUNUSED(event))
{
    string com = "s.change " + sel_fun + " " + ch_edit->GetValue().c_str() 
                 + " [";
    if (dom_set_cb->GetValue()) {
        if (ch_dom_rb[0]->GetValue()) {
            if (dom_ctr_set_cb->GetValue())
                com += S(ch_ctr->GetValue().c_str());
            com += " +- " + S(ch_sigma->GetValue().c_str());
        }
        else if (ch_dom_rb[1]->GetValue()) {
             com += S(ch_left_b->GetValue().c_str()) + " : " 
                    + ch_right_b->GetValue().c_str();
        }
        else assert(0);
    }
    com += "]";
    exec_command (com);
    
    string dd_was;
    my_sum->getp_core ("default-relative-domain-width", dd_was);
    string dd_is_proc = ch_def_dom_w->GetValue().c_str();
    if (!is_double(dd_is_proc))
        return;
    string dd_is = S(strtod(dd_is_proc.c_str(), 0) / 100.);
    if (dd_is != dd_was) 
        exec_command ("s.set default-relative-domain-width = " + dd_is);
    func_tree->update_labels (sel_fun);
}

void FuncBrowserDlg::OnArrowButton  (wxCommandEvent& event)
{
    int n = event.GetId();
    fp a = atof (ch_edit->GetValue().c_str());
    if (n == ID_BRO_C_LL)
        ch_edit->SetValue (S(0.5 * a).c_str());
    else if (n == ID_BRO_C_L)
        ch_edit->SetValue (S(0.95 * a).c_str());
    else if (n == ID_BRO_C_R) 
        ch_edit->SetValue (S(1.05 * a).c_str());
    else if (n == ID_BRO_C_RR)
        ch_edit->SetValue (S(1.5 * a).c_str());
}

void FuncBrowserDlg::OnSetDomCheckBox    (wxCommandEvent& WXUNUSED(event))
{
    change_domain_enable ();
}

void FuncBrowserDlg::OnSetDomCtrCheckBox (wxCommandEvent& WXUNUSED(event))
{
    if (dom_ctr_set_cb->GetValue())
        ch_ctr->SetValue(ch_edit->GetValue());
    else
        ch_ctr->SetValue("value");
    change_domain_enable ();
}

void FuncBrowserDlg::change_domain_enable()
{
    bool de = dom_set_cb->GetValue();
    bool h0 = false, h1 = false;
    if (de) {
        if (ch_dom_rb[0]->GetValue()) h0 = true;
        else if (ch_dom_rb[1]->GetValue()) h1 = true;
        else assert(0);
    }
    ch_dom_rb[0]->Enable(de);
    ch_dom_rb[1]->Enable(de);
    dom_ctr_set_cb->Enable(h0);
    ch_ctr->Enable(h0 && dom_ctr_set_cb->GetValue());
    ch_dom_label[0]->Enable(h0);
    ch_sigma->Enable(h0);
    ch_left_b->Enable(h1);
    ch_right_b->Enable(h1);
    for (int i = 1; i < 4; i++)
        ch_dom_label[i]->Enable(h1);
}

void FuncBrowserDlg::OnDeleteButton (wxCommandEvent& event)
{
    exec_command ("s.remove " + (!sel_fun.empty() && sel_fun[0] == 's' 
                                  ? S("**") : sel_fun));
    func_tree->OnPopupReset(event);
    update_frozen_tc();

}

void FuncBrowserDlg::OnRRCheckBox   (wxCommandEvent& event)
{
    exec_command (S("s.set recursive-remove = ") 
                   + (event.IsChecked() ? "1" : "0"));
}

void FuncBrowserDlg::OnFreezeButton    (wxCommandEvent& WXUNUSED(event))
{
    const wxString label = freeze_button->GetLabel();
    bool frozen = (label[0] == 'U'); //[U]nfreeze
    exec_command ("s.freeze " + S(frozen ? "! " : "") + sel_fun);
    update_frozen_tc();
    func_tree->update_labels (sel_fun);
    update_freeze_button_label();
}

void FuncBrowserDlg::OnFreezeAllButton (wxCommandEvent& event)
{
    //"freeze all" or "unfreeze all" button 
    string cmd = "s.freeze " + S(event.GetId() == ID_BRO_F_TA ? "! " : "") 
                 + "@*";
    exec_command (cmd);
    update_frozen_tc();
    func_tree->update_labels ("@");
    update_freeze_button_label();
}

void FuncBrowserDlg::update_frozen_tc()
{
    string f_i = my_sum->pars()->frozen_info(); 
    frozen_tc->SetValue (f_i.empty() ? "none" : f_i.c_str());
}

void FuncBrowserDlg::update_freeze_button_label()
{
    string freeze_button_label;
    int a_nr = atoi (sel_fun.c_str() + 1);
    if (my_sum->pars()->is_frozen(a_nr))
        freeze_button_label = "Unfreeze " + sel_fun;
    else
        freeze_button_label = "Freeze " + sel_fun;
    freeze_button->SetLabel (freeze_button_label.c_str());
}

void FuncBrowserDlg::OnValueButton (wxCommandEvent& WXUNUSED(event))
{
    string s = value_at_tc->GetValue().c_str();
    if (!is_double(s))
        return;
    fp at = strtod(s.c_str(), 0);
    string output;
    if (sel_fun.empty() || sel_fun[0] == 's') //sum
        output = my_sum->print_sum_value(at);
    else {
        char c = sel_fun[0]; 
        int n = atoi (sel_fun.c_str() + 1);
        if (c == '@')
            output = my_sum->pars()->info_a(n);
        else // ^,$,<
            output = my_sum->print_fzg_value(V_fzg::type_of_symbol(c), n, at);
    }
    value_output_tc->SetValue (output.c_str()); 
}

void FuncBrowserDlg::show_expanded (int item_nr, int subitem_nr)
{
    wxTreeItemIdValue cookie;
    wxTreeItemId id = func_tree->GetFirstChild(func_tree->GetRootItem(),cookie);
    for (int i = 1; i <= item_nr && id.IsOk(); i++)
        id = func_tree->GetNextChild(func_tree->GetRootItem(), cookie);
    if (!id.IsOk()) return;
    wxTreeItemId show_id = id;
    if (subitem_nr >= 0) {
        wxTreeItemId s_id = func_tree->GetFirstChild(id, cookie);
        for (int i = 1; i <= subitem_nr && s_id.IsOk(); i++)
            s_id = func_tree->GetNextChild(id, cookie);
        if (!s_id.IsOk()) return;
        show_id = s_id;
    }
    func_tree->EnsureVisible(show_id);
    func_tree->SelectItem(show_id);
    func_tree->ExpandAll(show_id);
}

//======================== FuncTree ===========================

BEGIN_EVENT_TABLE (FuncTree, wxTreeCtrl)
    EVT_RIGHT_DOWN (               FuncTree::OnRightDown         )
    EVT_MENU (ID_BRO_MENU_EXP,     FuncTree::OnPopupExpandAll    )
    EVT_MENU (ID_BRO_MENU_COL,     FuncTree::OnPopupCollapseAll  )
    EVT_MENU (ID_BRO_MENU_BUT,     FuncTree::OnPopupToggleButton )
    EVT_MENU (ID_BRO_MENU_RST,     FuncTree::OnPopupReset        )
END_EVENT_TABLE()

FuncTree::FuncTree (wxWindow *parent, const wxWindowID id)
    :  wxTreeCtrl (parent, id, wxDefaultPosition, wxSize(-1, 200), 
                   wxTR_HAS_BUTTONS|wxTR_TWIST_BUTTONS|wxTR_ROW_LINES)
{
    reset_funcs_in_root();
}

void FuncTree::reset_funcs_in_root()
{
    DeleteAllItems();
    a_ids.clear();
    g_ids.clear();
    f_ids.clear();
    z_ids.clear();
    wxTreeItemId root = AddRoot("sum");
    a_ids.resize(my_sum->pars()->count_a());
    g_ids.resize(my_sum->fzg_size(gType));
    f_ids.resize(my_sum->fzg_size(fType));
    z_ids.resize(my_sum->fzg_size(zType));
    //f-functions
    for (int i = 0; i < my_sum->fzg_size(fType); ++i) 
        add_fzg_to_tree (root, fType, i);
    //zero-shifts
    for (int i = 0; i < my_sum->fzg_size(zType); ++i) 
        add_fzg_to_tree (root, zType, i);
    // g and a not used by f and zs
    for (int i = 0; i < size(g_ids); i++)
        if (g_ids[i].empty())
            add_fzg_to_tree (root, gType, i);
    for (int i = 0; i < size(a_ids); i++)
        if (a_ids[i].empty())
            add_pags_to_tree (root, vector1(Pag(0., i)));
    Expand(root);
}

void FuncTree::add_pags_to_tree(wxTreeItemId p_id, const vector<Pag>& pags)
{
    for (vector<Pag>::const_iterator i = pags.begin(); i != pags.end(); i++)
        if (i->is_g()) 
            add_fzg_to_tree (p_id, gType, i->g());
        else if (i->is_a()) {
            wxTreeItemId id = AppendItem (p_id, 
                                       my_sum->pars()->info_a(i->a()).c_str());
            a_ids[i->a()].push_back(id);
            if (my_sum->pars()->is_frozen(i->a()) == false)
                SetItemTextColour (id, wxColour(0, 100, 0));
            else
                SetItemTextColour (id, wxColour(0, 200, 0));
        }
}

void FuncTree::add_fzg_to_tree (wxTreeItemId p_id, One_of_fzg fzg, int n)
{
        wxTreeItemId id = AppendItem (p_id, my_sum->info_fzg(fzg, n).c_str());
        wxColour text_col;
        if      (fzg == fType) {
            text_col = *wxBLACK;
            f_ids[n].push_back(id);
        }
        else if (fzg == zType) {
            text_col = wxColour(100, 0, 0);
            z_ids[n].push_back(id);
        }
        else if (fzg == gType) {
            text_col = *wxBLUE;
            g_ids[n].push_back(id);
        }
        add_pags_to_tree (id, my_sum->get_fzg(fzg, n)->copy_of_pags());
        SetItemTextColour (id, text_col);
        //SetItemBackgroundColour (id, *wxBLACK);
}

void FuncTree::OnRightDown (wxMouseEvent& event)
{
    wxMenu popup_menu ("browser menu");

    popup_menu.Append (ID_BRO_MENU_EXP, "&Expand all");
    popup_menu.Append (ID_BRO_MENU_COL, "&Collapse all");
    //popup_menu.Append (ID_BRO_MENU_BUT, "Toggle &buttons");
    popup_menu.Append (ID_BRO_MENU_RST, "&Reset");
    PopupMenu (&popup_menu, event.GetX(), event.GetY());
}

void FuncTree::OnPopupExpandAll     (wxCommandEvent& WXUNUSED(event))
{
    ExpandAll (GetRootItem());
}

//ExpandAll method is copied from wxGenericTreeCtrl,
// because it doesn't exists in MSW treectrl
void FuncTree::ExpandAll(const wxTreeItemId& item)
{
    if ( !HasFlag(wxTR_HIDE_ROOT) || item != GetRootItem())
    {
        Expand(item);
        if ( !IsExpanded(item) )
            return;
    }
    wxTreeItemIdValue cookie;
    wxTreeItemId child = GetFirstChild(item, cookie);
    while ( child.IsOk() )
    {
        ExpandAll(child);
        
        child = GetNextChild(item, cookie); 
    }   
}   

void FuncTree::OnPopupCollapseAll   (wxCommandEvent& WXUNUSED(event))
{
    reset_funcs_in_root();
}

void FuncTree::OnPopupToggleButton  (wxCommandEvent& WXUNUSED(event))
{
    SetWindowStyle (GetWindowStyle() ^ wxTR_TWIST_BUTTONS);
}

void FuncTree::OnPopupReset         (wxCommandEvent& WXUNUSED(event))
{
    reset_funcs_in_root();
}

wxTreeItemId FuncTree::next_item (const wxTreeItemId& item)
{
    if (ItemHasChildren(item)) {
        wxTreeItemIdValue cookie;
        return GetFirstChild (item, cookie);
    }
    else 
        return next_item_but_not_child (item);
}

wxTreeItemId FuncTree::next_item_but_not_child (const wxTreeItemId& item)
{
    if (GetNextSibling(item).IsOk())
        return GetNextSibling(item);  
    else if (item == GetRootItem())
        return wxTreeItemId(); //invalid item 
    else 
        return next_item_but_not_child (GetItemParent (item));
}

int FuncTree::update_labels (const string& beginning)
{
    assert (!beginning.empty());
    int count = 0;
    for  (wxTreeItemId item = GetRootItem(); 
          item.IsOk(); 
          item = next_item(item)) {
        string item_text = GetItemText(item).c_str();
        if (! item_text.substr(0, beginning.length()).compare(beginning)) {
            char symb = GetItemText(item).c_str()[0];
            int nr = atoi (GetItemText(item).c_str() + 1);
            string new_label;
            if (symb == '@') {
                new_label = my_sum->pars()->info_a(nr); 
                if (my_sum->pars()->is_frozen(nr) == false)
                    SetItemTextColour (item, wxColour(0, 100, 0));
                else
                    SetItemTextColour (item, wxColour(0, 200, 0));
            }
            else // $,^,<
                new_label = my_sum->info_fzg (V_fzg::type_of_symbol(symb), nr);
            SetItemText (item, new_label.c_str());
            count++;
        }
    }
    return count;
}



//=====================   data->LoadFile(Custom) dialog  ==================

// first helper class: LoadDataDirCtrl
BEGIN_EVENT_TABLE(LoadDataDirCtrl, wxGenericDirCtrl)
    EVT_TREE_SEL_CHANGED(wxID_TREECTRL, LoadDataDirCtrl::OnPathSelectionChanged)
END_EVENT_TABLE()

LoadDataDirCtrl::LoadDataDirCtrl(FDXLoadDlg* parent)
    : wxGenericDirCtrl(parent, -1, wxDirDialogDefaultFolderStr,
                       wxDefaultPosition, wxSize(-1, 300),
                       wxDIRCTRL_SHOW_FILTERS,
                       // multiple wildcards, eg. 
                       // |*.dat;*.DAT;*.xy;*.XY;*.fio;*.FIO
                       // are not supported by wxGenericDirCtrl  
                       "all files (*)|*"
                       "|ASCII x y files (*)|*" 
                       "|rit files (*.rit)|*.rit"
                       "|cpi files (*.cpi)|*.cpi"
                       "|mca files (*.mca)|*.mca"
                       "|Siemens/Bruker (*.raw)|*.raw"),
      load_dlg(parent)
{}

void LoadDataDirCtrl::OnPathSelectionChanged(wxTreeEvent &WXUNUSED(event))
{
    load_dlg->on_path_change();
}

void LoadDataDirCtrl::SetFilterIndex(int n)
{
    wxGenericDirCtrl::SetFilterIndex(n);
    load_dlg->on_filter_change();
}


BEGIN_EVENT_TABLE(FDXLoadDlg, wxDialog)
    EVT_CHECKBOX    (ID_DXLOAD_STDDEV_CB, FDXLoadDlg::OnStdDevCheckBox)
END_EVENT_TABLE()

FDXLoadDlg::FDXLoadDlg (wxWindow* parent, wxWindowID id)
    : wxDialog(parent, id, "Data load (custom)", 
               wxDefaultPosition, wxSize(-1, 500), 
               wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) 
{
    wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);

    dir_ctrl = new LoadDataDirCtrl(this);
    string path = my_data->get_filename();
    //TODO it segfaults, don't know why
    //dir_ctrl->SetPath(path.c_str());
    top_sizer->Add(dir_ctrl, 1, wxALL|wxEXPAND, 5);

    filename_tc = new wxTextCtrl (this, -1, path.c_str(), 
                                  wxDefaultPosition, wxDefaultSize,
                                  wxTE_READONLY);
    top_sizer->Add (filename_tc, 0, wxALL|wxEXPAND, 5);

    //selecting columns
    columns_panel = new wxPanel (this, -1);
    wxStaticBox *cbox = new wxStaticBox (columns_panel, -1, "Select columns:");
    wxStaticBoxSizer *h2a_sizer = new wxStaticBoxSizer (cbox, wxHORIZONTAL);
    h2a_sizer->Add (new wxStaticText (columns_panel, -1, "x"), 
                    0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    x_column = new wxSpinCtrl (columns_panel, -1, "1", 
                               wxDefaultPosition, wxSize(70, -1), 
                               wxSP_ARROW_KEYS, 1, 99, 1);
    h2a_sizer->Add (x_column, 0, wxALL|wxALIGN_LEFT, 5);
    h2a_sizer->Add (new wxStaticText (columns_panel, -1, "y"), 
                    0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    y_column = new wxSpinCtrl (columns_panel, -1, "2",
                               wxDefaultPosition, wxSize(70, -1), 
                               wxSP_ARROW_KEYS, 1, 99, 2);
    h2a_sizer->Add (y_column, 0, wxALL|wxALIGN_LEFT, 5);
    std_dev_cb = new wxCheckBox(columns_panel, ID_DXLOAD_STDDEV_CB, "std.dev.");
    std_dev_cb->SetValue (false);
    h2a_sizer->Add (std_dev_cb, 0, wxALL|wxALIGN_LEFT, 5);
    s_column = new wxSpinCtrl (columns_panel, -1, "3",
                               wxDefaultPosition, wxSize(70, -1), 
                               wxSP_ARROW_KEYS, 1, 99, 3);
    h2a_sizer->Add (s_column, 0, wxALL|wxALIGN_LEFT, 5);
    columns_panel->SetSizerAndFit(h2a_sizer);
    top_sizer->Add (columns_panel, 0, wxALL|wxEXPAND, 5);
    OnStdDevCheckBox (dummy_cmd_event);

    append_cb = new wxCheckBox(this, -1, 
                               "Append data from file to already loaded data");
    append_cb->Enable(!my_data->is_empty());
    top_sizer->Add(append_cb, 0, wxALL, 5);

    top_sizer->Add (new wxStaticLine(this, -1), 0, wxEXPAND|wxLEFT|wxRIGHT, 5);
    top_sizer->Add (CreateButtonSizer (wxOK|wxCANCEL), 
                    0, wxALL|wxALIGN_CENTER, 5);
    SetSizerAndFit(top_sizer);
    on_filter_change();
}

void FDXLoadDlg::OnStdDevCheckBox (wxCommandEvent& WXUNUSED(event))
{
    s_column->Enable(std_dev_cb->GetValue());
}

void FDXLoadDlg::on_filter_change()
{
    int idx = dir_ctrl->GetFilterIndex();
    if (idx == 0) // all files
        on_path_change();
    else
        columns_panel->Enable(idx == 1); // enable if ASCII 
}

void FDXLoadDlg::on_path_change()
{
    wxString path = dir_ctrl->GetFilePath();
    filename_tc->SetValue(path);
    if (dir_ctrl->GetFilterIndex() == 0) { // all files
        columns_panel->Enable(!path.IsEmpty() 
                              && Data::guess_file_type(path.c_str()) == 'd'); 
    }
    //TODO enable/disable OK button?
}

string FDXLoadDlg::get_filename()
{
    return filename_tc->GetValue().c_str();
}

string FDXLoadDlg::get_command()
{
    string cols;
    if (columns_panel->IsEnabled()) { // a:b[:c]
        cols = " " + S(x_column->GetValue()) + ":" + S(y_column->GetValue());
        if (std_dev_cb->GetValue())
            cols += S(":") + S(s_column->GetValue());
    }
    //TODO opt_lcase;
    string s = "d.load" + cols + " '" + get_filename() + "'";
    if (append_cb->GetValue())
        s += " +";
    return s;
}

//=====================   data->history dialog  ==================

BEGIN_EVENT_TABLE(SumHistoryDlg, wxDialog)
    EVT_BUTTON      (ID_SHIST_UP,     SumHistoryDlg::OnUpButton)
    EVT_BUTTON      (ID_SHIST_DOWN,   SumHistoryDlg::OnDownButton)
    EVT_BUTTON      (ID_SHIST_TSAV,   SumHistoryDlg::OnToggleSavedButton)
    EVT_BUTTON      (ID_SHIST_CWSSR,  SumHistoryDlg::OnComputeWssrButton)
    EVT_LIST_ITEM_SELECTED  (ID_SHIST_LC, SumHistoryDlg::OnSelectedItem)
    EVT_LIST_ITEM_ACTIVATED (ID_SHIST_LC, SumHistoryDlg::OnActivatedItem)
    EVT_SPINCTRL    (ID_SHIST_V+0,    SumHistoryDlg::OnViewSpinCtrlUpdate)
    EVT_SPINCTRL    (ID_SHIST_V+1,    SumHistoryDlg::OnViewSpinCtrlUpdate)
    EVT_SPINCTRL    (ID_SHIST_V+2,    SumHistoryDlg::OnViewSpinCtrlUpdate)
END_EVENT_TABLE()

SumHistoryDlg::SumHistoryDlg (wxWindow* parent, wxWindowID id)
    : wxDialog(parent, id, "Parameters History", 
               wxDefaultPosition, wxDefaultSize, 
               wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER), 
      lc(0)
{
    wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *hsizer = new wxBoxSizer(wxHORIZONTAL);
    initialize_lc(); //wxListCtrl
    hsizer->Add (lc, 1, wxEXPAND);

    wxBoxSizer *arrows_sizer = new wxBoxSizer(wxVERTICAL);
    up_arrow = new wxBitmapButton (this, ID_SHIST_UP, wxBitmap (up_arrow_xpm));
    arrows_sizer->Add (up_arrow, 0);
    arrows_sizer->Add (10, 10, 1);
    down_arrow = new wxBitmapButton (this, ID_SHIST_DOWN, 
                                     wxBitmap (down_arrow_xpm));
    arrows_sizer->Add (down_arrow, 0);
    hsizer->Add (arrows_sizer, 0, wxALIGN_CENTER);
    top_sizer->Add (hsizer, 1, wxEXPAND);

    wxBoxSizer *buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
    buttons_sizer->Add (new wxButton (this, ID_SHIST_TSAV, "Toggle saved"), 
                        0, wxALL, 5);
    buttons_sizer->Add (10, 10, 1);
    compute_wssr_button = new wxButton (this, ID_SHIST_CWSSR,"Compute WSSRs");
    buttons_sizer->Add (compute_wssr_button, 0, wxALL, 5);
    buttons_sizer->Add (10, 10, 1);
    buttons_sizer->Add (new wxStaticText (this, -1, "View @:"), 
                        0, wxALL|wxALIGN_CENTER, 5);
    for (int i = 0; i < 3; i++)
        buttons_sizer->Add (new wxSpinCtrl (this, ID_SHIST_V + i, 
                                            S(view[i]).c_str(),
                                            wxDefaultPosition, wxSize(40, -1),
                                            wxSP_ARROW_KEYS, 0, view_max),
                            0, wxALL, 5);
    buttons_sizer->Add (10, 10, 1);
    buttons_sizer->Add (new wxButton (this, wxID_CANCEL, "Close"), 
                        0, wxALL, 5);
    top_sizer->Add (buttons_sizer, 0, wxALIGN_CENTER);

    SetSizer (top_sizer);
    top_sizer->SetSizeHints (this);

    update_selection();
}

void SumHistoryDlg::initialize_lc()
{
    assert (lc == 0);
    view_max = my_sum->pars()->count_a() - 1;
    assert (view_max != -1);
    for (int i = 0; i < 3; i++)
        view[i] = min (i, view_max);
    lc = new wxListCtrl (this, ID_SHIST_LC, 
                         wxDefaultPosition, wxSize(450, 250), 
                         wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_HRULES|wxLC_VRULES
                             |wxSIMPLE_BORDER);
    lc->InsertColumn(0, "Saved");
    lc->InsertColumn(1, "No.");
    lc->InsertColumn(2, "Changed by");
    lc->InsertColumn(3, "WSSR");
    for (int i = 0; i < 3; i++)
        lc->InsertColumn(4 + i, ("@" + S(view[i])).c_str()); 

    for (int i = 0; i != my_sum->pars()->history_size(); ++i) {
        const HistoryItem& item = my_sum->pars()->history_item(i);
        lc->InsertItem(i, item.saved ? "   *   " : "       ");
        lc->SetItem (i, 1, ("  " + S(i+1) + "  ").c_str());
        lc->SetItem (i, 2, item.comment.c_str());
        lc->SetItem (i, 3, "      ?      ");
        for (int j = 0; j < 3; j++)
            lc->SetItem (i, 4 + j, S(item.a[view[j]]).c_str());
    }
    for (int i = 0; i < 7; i++)
        lc->SetColumnWidth(i, wxLIST_AUTOSIZE);
}

void SumHistoryDlg::update_selection()
{
    int index = my_sum->pars()->history_position();
    lc->SetItemState (index, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED, 
                             wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
    up_arrow->Enable (index != 0);
    down_arrow->Enable (index != my_sum->pars()->history_size() - 1);
}

void SumHistoryDlg::OnUpButton       (wxCommandEvent& WXUNUSED(event))
{
    exec_command ("s.history -1");
    update_selection();
}

void SumHistoryDlg::OnDownButton     (wxCommandEvent& WXUNUSED(event))
{
    exec_command ("s.history +1");
    update_selection();
}

void SumHistoryDlg::OnToggleSavedButton (wxCommandEvent& WXUNUSED(event))
{
    exec_command ("s.history *");
    int idx = my_sum->pars()->history_position();
    lc->SetItemText (idx, my_sum->pars()->history_item(idx).saved ? "   *   " 
                                                          : "       ");
}

void SumHistoryDlg::OnComputeWssrButton (wxCommandEvent& WXUNUSED(event))
{
    for (int i = 0; i != my_sum->pars()->history_size(); ++i) {
        const HistoryItem& item = my_sum->pars()->history_item(i);
        my_sum->use_param_a_for_value (item.a);
        fp wssr = v_fit::compute_wssr_for_data (my_data, my_sum, true);
        lc->SetItem (i, 3, S(wssr).c_str());
    }
    lc->SetColumnWidth(3, wxLIST_AUTOSIZE);
    compute_wssr_button->Enable(false);
}
void SumHistoryDlg::OnSelectedItem (wxListEvent& WXUNUSED(event))
{
    update_selection();
}

void SumHistoryDlg::OnActivatedItem (wxListEvent& event)
{
    int n = event.GetIndex();
    exec_command ("s.history " + S(n+1));
    update_selection();
}

void SumHistoryDlg::OnViewSpinCtrlUpdate (wxSpinEvent& event) 
{
    int v = event.GetId() - ID_SHIST_V;
    assert (0 <= v && v < 3);
    int n = event.GetPosition();
    assert (0 <= n && n <= view_max);
    view[v] = n;
    //update header in wxListCtrl
    wxListItem li;
    li.SetMask (wxLIST_MASK_TEXT);
    li.SetText (("@" + S(view[v])).c_str());
    lc->SetColumn(4 + v, li); 
    //update data in wxListCtrl
    for (int i = 0; i != my_sum->pars()->history_size(); ++i) {
        const HistoryItem& item = my_sum->pars()->history_item(i);
        lc->SetItem (i, 4 + v, S(item.a[view[v]]).c_str());
    }
}


//=====================   data->editor dialog  ==================

class DataTable: public wxGridTableBase
{
public:
    DataTable(Data *data_, wxGrid *grid_) : wxGridTableBase(), 
                                            data(data_), grid(grid_) {}
    int GetNumberRows() { return data->points().size(); }
    int GetNumberCols() { return 4; }
    bool IsEmptyCell(int WXUNUSED(row), int WXUNUSED(col)) { return false; }

    wxString GetValue(int row, int col) 
        {  return S(col == 0 ? double(GetValueAsBool(row,col))
                             : GetValueAsDouble(row,col)).c_str(); }

    void SetValue(int, int, const wxString&) { assert(0); }

    wxString GetTypeName(int WXUNUSED(row), int col)
        { return col == 0 ? wxGRID_VALUE_BOOL : wxGRID_VALUE_FLOAT; }

    bool CanGetValueAs(int row, int col, const wxString& typeName)
        { return typeName == GetTypeName(row, col); }

    double GetValueAsDouble(int row, int col)
    { 
        const Point &p = data->points()[row]; 
        switch (col) {
            case 1: return p.x;      
            case 2: return p.y;      
            case 3: return p.sigma;  
            default: assert(0);
        }
    }

    bool GetValueAsBool(int row, int col)
        { assert(col==0); return data->points()[row].is_active; }

    void SetValueAsDouble(int row, int col, double value) 
    { 
        string t;
        switch (col) {
            case 1: t = "x";  break;
            case 2: t = "y";  break;
            case 3: t = "s";  break;
            default: assert(0);
        }
        exec_command("d.transform " + t + "[" + S(row)+"]=" + S(value));
        if (col == 1) // order of items can be changed
            grid->ForceRefresh();
    }

    void SetValueAsBool(int row, int col, bool value) 
    { 
        assert(col==0); 
        exec_command("d.transform a[" + S(row)+"]=" + (value ?"true":"false")); 
    }

    wxString GetRowLabelValue(int row) { return S(row).c_str(); }

    wxString GetColLabelValue(int col) 
    { 
        switch (col) {
            case 0: return "active"; 
            case 1: return "x";      
            case 2: return "y";      
            case 3: return "sigma";  
            default: assert(0);
        }
    }

private:
    Data *data;
    wxGrid *grid;
};


// ';' will be replaced by line break
static const char *default_examples = 

"integrate|useful|Integrate data numerically and adjust std. dev.;"
"in other words it produces cumulative area"
"|Y[1...] = Y[n-1] + y[n];S = sqrt(max(1,y))|Y\n"

"differentiate|useful|compute numerical derivative f'(x)"
"|Y[...-1] = y[n+1]-y[n];X[...-1] = (x[n+1]+x[n])/2;"
"M=M-1;S = sqrt(max(1,y))|Y\n"

"normalize area|useful|divide all Y (and std. dev.) values;"
"by the current data area; it produces unit area"
"|S = s / sum(n > 0 ? (x[n] - x[n-1]) * (y[n-1] + y[n])/2 : 0);" 
"Y = y / sum(n > 0 ? (x[n] - x[n-1]) * (y[n-1] + y[n])/2 : 0)|Y\n" 

"zero negative y|useful|zero the Y value; of points with negative Y"
"|Y=max(y,0)|Y\n"

"clear inactive|useful|delete inactive points"
"|delete(not a)|Y\n"

"swap axes|example|Swap X and Y axes and adjust std. dev."
"|Y=x & X=y & S=sqrt(max(1,Y))|N\n"

"generate sinusoid|example|replaces current data with sinusoid"
"|M=2000; x=n/100; y=sin(x); s=1|N\n"

"invert|example|inverts y value of points"
"|Y=-y|N\n"

"activate all|example|activate all data points"
"|a=true|N\n"

"Q -> 2theta(Cu)|example|rescale X axis;in powder diffraction pattern"
"|X = asin(x/(4*pi)*1.54051) * 2*180/pi|N\n"

"2theta(Cu) -> Q|example|rescale X axis;in powder diffraction pattern"
"|X = 4*pi * sin(x/2*pi/180) / 1.54051|N\n"
;

DataTransExample::DataTransExample(string line)
     : in_menu(false) 
{
    replace_all(line, ";", "\n");
    string::size_type pos=0;
    for (int cnt = 0; cnt <= 4; ++cnt) {
        string::size_type new_pos = line.find('|', pos);
        string sub = string(line, pos, new_pos-pos);
        if (cnt == 0) 
            name = sub;
        else if (cnt == 1)
            category = sub;
        else if (cnt == 2)
            description = sub;
        else if (cnt == 3)
            code = sub;
        else if (cnt == 4)
            in_menu = (sub == "Y");
        if (new_pos == string::npos)
            break;
        pos = new_pos + 1;
    }
}

string DataTransExample::as_fileline() const
{
    string s = name + "|" + category + "|" + description + "|" + code 
               + "|" + (in_menu ? "Y" : "N");
    replace_all(s, "\n", ";");
    return s;
}


BEGIN_EVENT_TABLE(DataEditorDlg, wxDialog)
    EVT_BUTTON      (wxID_REVERT_TO_SAVED,  DataEditorDlg::OnRevert)
    EVT_BUTTON      (wxID_SAVEAS,           DataEditorDlg::OnSaveAs)
    EVT_BUTTON      (wxID_ADD,              DataEditorDlg::OnAdd)
    EVT_BUTTON      (wxID_REMOVE,           DataEditorDlg::OnRemove)
    EVT_BUTTON      (wxID_UP,               DataEditorDlg::OnUp)
    EVT_BUTTON      (wxID_DOWN,             DataEditorDlg::OnDown)
    EVT_BUTTON      (wxID_SAVE,             DataEditorDlg::OnSave)
    EVT_BUTTON      (ID_DE_RESET,           DataEditorDlg::OnReset)
    EVT_BUTTON      (wxID_APPLY,            DataEditorDlg::OnApply)
    EVT_BUTTON      (wxID_HELP,             DataEditorDlg::OnHelp)
    EVT_BUTTON      (wxID_CLOSE,            DataEditorDlg::OnClose)
    EVT_TEXT        (ID_DE_CODE,            DataEditorDlg::OnCodeText)
    EVT_LIST_ITEM_SELECTED(ID_DE_EXAMPLES,  DataEditorDlg::OnESelected)
    EVT_LIST_ITEM_ACTIVATED(ID_DE_EXAMPLES, DataEditorDlg::OnEActivated)
END_EVENT_TABLE()

DataEditorDlg::DataEditorDlg (wxWindow* parent, wxWindowID id, Data *data_)
    : wxDialog(parent, id, "Data Editor", 
               wxDefaultPosition, wxSize(500, 500), 
               wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
      data(0)
{
    wxSplitterWindow *splitter = new wxSplitterWindow(this);

    // left side of the dialog
    wxPanel *left_panel = new wxPanel(splitter); 
    wxBoxSizer *left_sizer = new wxBoxSizer(wxVERTICAL);
    left_sizer->Add(new wxStaticText(left_panel, -1, "Original filename:"));
    filename_label = new wxStaticText(left_panel, -1, "");
    left_sizer->Add(filename_label, 0, wxADJUST_MINSIZE);
    wxBoxSizer *two_btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    revert_btn = new wxButton(left_panel, wxID_REVERT_TO_SAVED, 
                              "Revert to Saved");
    two_btn_sizer->Add(revert_btn, 0, wxALL|wxALIGN_CENTER, 5);
    save_as_btn = new wxButton(left_panel, wxID_SAVEAS, 
                               "Save &As...");
    two_btn_sizer->Add(save_as_btn, 0, wxALL|wxALIGN_CENTER, 5);
    left_sizer->Add(two_btn_sizer, 0, wxALIGN_CENTER);
    left_sizer->Add(new wxStaticText(left_panel, -1, "Data title: "),
                    0, wxLEFT|wxRIGHT|wxTOP, 5);
    title_label = new wxStaticText(left_panel, -1, "");
    left_sizer->Add(title_label, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE,5);
    grid = new wxGrid(left_panel, ID_DE_GRID, 
                      wxDefaultPosition, wxSize(-1, 350));
    left_sizer->Add(grid, 1, wxEXPAND);
    left_panel->SetSizerAndFit(left_sizer);

    // right side of the dialog
    wxPanel *right_panel = new wxPanel(splitter); 
    wxBoxSizer *right_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *example_sizer = new wxBoxSizer(wxHORIZONTAL);
    example_list = new wxListCtrl(right_panel, ID_DE_EXAMPLES, 
                                  wxDefaultPosition, wxDefaultSize,
                                  wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_HRULES);
    example_list->InsertColumn(0, "transformation");
    example_list->InsertColumn(1, "in menu");
    example_sizer->Add(example_list, 1, wxEXPAND|wxALL, 5);
    wxBoxSizer *example_button_sizer = new wxBoxSizer(wxVERTICAL);
    add_btn = new wxButton(right_panel, wxID_ADD, "Add");
    example_button_sizer->Add(add_btn, 0, wxALL, 5);
    remove_btn = new wxButton(right_panel, wxID_REMOVE, "Remove");
    example_button_sizer->Add(remove_btn, 0, wxALL, 5);
    up_btn = new wxButton(right_panel, wxID_UP, "&Up");
    example_button_sizer->Add(up_btn, 0, wxALL, 5);
    down_btn = new wxButton(right_panel, wxID_DOWN, "&Down");
    example_button_sizer->Add(down_btn, 0, wxALL, 5);
    save_btn = new wxButton(right_panel, wxID_SAVE, "&Save");
    example_button_sizer->Add(save_btn, 0, wxALL, 5);
    reset_btn = new wxButton(right_panel, ID_DE_RESET, "&Reset");
    example_button_sizer->Add(reset_btn, 0, wxALL, 5);
    example_sizer->Add(example_button_sizer, 0);
    right_sizer->Add(example_sizer, 0, wxEXPAND);
    description = new wxStaticText(right_panel, -1, "\n\n\n\n", 
                                   wxDefaultPosition, wxDefaultSize,
                                   wxALIGN_LEFT);
    right_sizer->Add(description, 0, wxEXPAND|wxALL|wxADJUST_MINSIZE, 5);
    code = new wxTextCtrl(right_panel, ID_DE_CODE, "", 
                          wxDefaultPosition, wxDefaultSize,
                          wxTE_MULTILINE|wxHSCROLL|wxVSCROLL);
    right_sizer->Add(code, 1, wxEXPAND|wxALL, 5);
    wxBoxSizer *apply_help_sizer = new wxBoxSizer(wxHORIZONTAL);
    apply_help_sizer->Add(1, 1, 1);
    apply_btn = new wxButton(right_panel, wxID_APPLY, "&Apply");
    apply_help_sizer->Add(apply_btn, 0, wxALIGN_CENTER|wxALL, 5);
    apply_help_sizer->Add(1, 1, 1);
    help_btn = new wxButton(right_panel, wxID_HELP, "&Help");
    apply_help_sizer->Add(help_btn, 0, wxALIGN_RIGHT|wxALL, 5);
    right_sizer->Add(apply_help_sizer, 0, wxEXPAND);
    right_panel->SetSizerAndFit(right_sizer);

    // setting column sizes and a bit of logic
    update_data(data_);
    grid->SetEditable(true);
    grid->SetColumnWidth(0, 40);
    grid->SetRowLabelSize(60);
    initialize_examples();
    for (int i = 0; i < 2; i++)
        example_list->SetColumnWidth(i, wxLIST_AUTOSIZE);
    apply_btn->Enable(false);

    // finishing layout
    splitter->SplitVertically(left_panel, right_panel);
    wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
    top_sizer->Add(splitter, 1, wxEXPAND, 1);
    top_sizer->Add (new wxStaticLine(this, -1), 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
    top_sizer->Add(new wxButton(this, wxID_CLOSE, "&Close"), 
                   0, wxALIGN_CENTER|wxALL, 5);
    SetSizerAndFit(top_sizer);
    Centre();
}

std::vector<DataTransExample> DataEditorDlg::examples;

void DataEditorDlg::read_examples(bool reset)
{
    examples.clear();
    // this item should be always present
    examples.push_back(DataTransExample("custom", "builtin", 
                                        "Custom transformation.\n"
                                        "You can type eg. Y=log10(y).\n"
                                        "See Help for details.",
                                        "", false));
    //TODO last transformation item
    wxString transform_path = get_user_conffile("transform");
    string t_line;
    if (wxFileExists(transform_path) && !reset) {
        ifstream f(transform_path.c_str());
        while (getline(f, t_line))
            examples.push_back(DataTransExample(t_line));
    }
    else {
        istringstream f(default_examples);
        while (getline(f, t_line))
            examples.push_back(DataTransExample(t_line));
    }
}

void DataEditorDlg::initialize_examples(bool reset)
{
    if (reset)
        read_examples(reset);
    example_list->DeleteAllItems();
    for (int i = 0; i < size(examples); ++i) 
        insert_example_list_item(i);
    select_example(0);
}

void DataEditorDlg::insert_example_list_item(int n)
{
    const DataTransExample& ex = examples[n];
    example_list->InsertItem(n, ex.name.c_str());
    example_list->SetItem(n, 1, (ex.in_menu ? "Yes" : "No"));
}

void DataEditorDlg::select_example(int item)
{
    if (item >= example_list->GetItemCount())
        return;
    example_list->SetItemState (item, 
                                wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                                wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
    // ESelected();
}

int DataEditorDlg::get_selected_item()
{
    return example_list->GetNextItem(-1,wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

void DataEditorDlg::update_data(Data *data_)
{
    data = data_;
    filename_label->SetLabel(data->get_filename().c_str());
    title_label->SetLabel(data->get_title().c_str());
    grid->SetTable(new DataTable(data, grid), true, wxGrid::wxGridSelectRows);
    grid->ForceRefresh();
    grid->AdjustScrollbars();
    Show();
}

void DataEditorDlg::OnRevert (wxCommandEvent& WXUNUSED(event))
{
    exec_command(data->get_load_cmd());
    grid->ForceRefresh();
    grid->AdjustScrollbars();
}

void DataEditorDlg::OnSaveAs (wxCommandEvent& WXUNUSED(event))
{
    bool ok = export_data_dlg(this /*GetParent()*/, true);
    if (ok) {
        filename_label->SetLabel(data->get_filename().c_str());
    }
}

void DataEditorDlg::OnAdd (wxCommandEvent& WXUNUSED(event))
{
    DataTransExample new_example("new", "useful",
                                 "", code->GetValue().c_str());
    ExampleEditorDlg dlg(this, -1, new_example, examples, -1);
    if (dlg.ShowModal() == wxID_OK) {
        int pos = get_selected_item() + 1;
        examples.insert(examples.begin() + pos, new_example);
        insert_example_list_item(pos);
        select_example(pos);
    }
}

void DataEditorDlg::OnRemove (wxCommandEvent& WXUNUSED(event))
{
    int item = get_selected_item();
    if (item == -1 || examples[item].category == "builtin")
        return;
    examples.erase(examples.begin() + item);
    example_list->DeleteItem(item);
    select_example(item > 0 ? item-1 : 0);
}

void DataEditorDlg::OnUp (wxCommandEvent& WXUNUSED(event))
{
    int item = get_selected_item();
    if (item == 0)
        return;
    // swap item-1 and item
    DataTransExample ex = examples[item-1];
    examples.erase(examples.begin() + item - 1);
    example_list->DeleteItem(item-1);
    examples.insert(examples.begin() + item, ex);
    insert_example_list_item(item);
    up_btn->Enable(item-1 > 0);
    down_btn->Enable(true);
}

void DataEditorDlg::OnDown (wxCommandEvent& WXUNUSED(event))
{
    int item = get_selected_item();
    if (item >= size(examples) - 1)
        return;
    // swap item+1 and item
    DataTransExample ex = examples[item+1];
    examples.erase(examples.begin() + item + 1);
    example_list->DeleteItem(item+1);
    examples.insert(examples.begin() + item, ex);
    insert_example_list_item(item);
    up_btn->Enable(true);
    down_btn->Enable(item+1 < example_list->GetItemCount() - 1);
}

void DataEditorDlg::OnSave (wxCommandEvent& WXUNUSED(event))
{
    wxString transform_path = get_user_conffile("transform");
    ofstream f(transform_path.c_str());
    for (vector<DataTransExample>::const_iterator i = examples.begin();
            i != examples.end(); ++i)
        if (i->category != "builtin")
            f << i->as_fileline() << endl;
}

void DataEditorDlg::OnReset (wxCommandEvent& WXUNUSED(event))
{
    initialize_examples(true);
}

void DataEditorDlg::OnApply (wxCommandEvent& WXUNUSED(event))
{
    execute_tranform(code->GetValue().Trim().c_str());
    grid->ForceRefresh();
    grid->AdjustScrollbars();
}

void DataEditorDlg::execute_tranform(string code)
{
    replace_all(code, "\n", ";   d.transform ");
    exec_command("d.transform " + code);
}

void DataEditorDlg::OnHelp (wxCommandEvent& WXUNUSED(event))
{
    getUI()->displayHelpTopic("Data transformations");
}

void DataEditorDlg::OnClose (wxCommandEvent& event)
{
    OnCancel(event);
}

void DataEditorDlg::CodeText()
{
    wxString text = code->GetValue().Trim();
    apply_btn->Enable(!text.IsEmpty());
}

void DataEditorDlg::ESelected()
{
    int item = get_selected_item();
    if (item == -1) {
        item = 0;
        select_example(0);
        return;
    }
    const DataTransExample& ex = examples[item];
    // to avoid frequent resizing, description should have >= 3 lines
    string desc = ex.description;
    for (int i = count(desc.begin(), desc.end(), '\n') + 1; i < 3; ++i)
        desc += "\n";
    description->SetLabel(desc.c_str());
    Layout(); // to resize description
    code->SetValue(ex.code.c_str());

    up_btn->Enable(item > 0);
    down_btn->Enable(item < example_list->GetItemCount() - 1);
    remove_btn->Enable(ex.category != "builtin");
    CodeText();
}

void DataEditorDlg::OnEActivated (wxListEvent& event)
{
    int n = event.GetIndex();
    if (examples[n].category == "builtin")
        return;
    ExampleEditorDlg dlg(this, -1, examples[n], examples, n);
    if (dlg.ShowModal() == wxID_OK) {
        example_list->DeleteItem(n);
        insert_example_list_item(n);
        select_example(n);
    }
}


BEGIN_EVENT_TABLE(ExampleEditorDlg, wxDialog)
    EVT_BUTTON  (wxID_OK,    ExampleEditorDlg::OnOK)
END_EVENT_TABLE()

ExampleEditorDlg::ExampleEditorDlg(wxWindow* parent, wxWindowID id, 
                                   DataTransExample& ex_,
                                   const vector<DataTransExample>& examples_,
                                   int pos_)
    : wxDialog(parent, id, "Example Editor", 
               wxDefaultPosition, wxDefaultSize, 
               wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
      ex(ex_), examples(examples_), pos(pos_)
{
    name_tc = new wxTextCtrl(this, -1, ex.name.c_str());
    description_tc = new wxTextCtrl(this, -1, ex.description.c_str(),
                                    wxDefaultPosition, wxSize(-1, 80),
                                    wxTE_MULTILINE|wxHSCROLL|wxVSCROLL);
    wxString choices[] = {"useful", "example", "other"};
    category_c = new wxComboBox(this, -1, ex.category.c_str(),
                                wxDefaultPosition, wxDefaultSize,
                                3, choices,
                                wxCB_READONLY);
    code_tc = new wxTextCtrl(this, -1, ex.code.c_str(), 
                             wxDefaultPosition, wxSize(-1, 100),
                             wxTE_MULTILINE|wxHSCROLL|wxVSCROLL);
    inmenu_cb = new wxCheckBox(this, -1, "show item in Data->Fast_DT menu");
    inmenu_cb->SetValue(ex.in_menu);

    wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer *flexsizer = new wxFlexGridSizer(2);
    flexsizer->Add(new wxStaticText(this, -1, "name"), 0, wxALL, 5);
    flexsizer->Add(name_tc, 0, wxALL|wxEXPAND, 5);
    flexsizer->Add(new wxStaticText(this, -1, "category"), 0, wxALL, 5);
    flexsizer->Add(category_c, 0, wxALL|wxEXPAND, 5);
    flexsizer->Add(new wxStaticText(this, -1, "description"), 0, wxALL, 5);
    flexsizer->Add(description_tc, 0, wxALL|wxEXPAND, 5);
    flexsizer->Add(new wxStaticText(this, -1, "code"), 0, wxALL, 5);
    flexsizer->Add(code_tc, 0, wxALL|wxEXPAND, 5);
    flexsizer->AddGrowableRow(2); // description
    flexsizer->AddGrowableRow(3); // code
    flexsizer->AddGrowableCol(1);
    top_sizer->Add(flexsizer, 0, wxEXPAND);
    top_sizer->Add(inmenu_cb, 0, wxALL, 5);
    top_sizer->Add (new wxStaticLine(this, -1), 0, wxEXPAND|wxLEFT|wxRIGHT, 5);
    top_sizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL, 5);
    SetSizerAndFit(top_sizer);
    Centre();
}

void ExampleEditorDlg::OnOK(wxCommandEvent &event)
{
    string new_name = name_tc->GetValue().Trim().c_str();
    for (int i = 0; i < size(examples); ++i) 
            if (i != pos && examples[i].name == new_name) {//name is not unique
                name_tc->SetFocus();
                name_tc->SetSelection(-1, -1);
                return;
            }
    // we are here -- name is unique
    ex.name = new_name;
    ex.category = category_c->GetValue().c_str();
    ex.description = description_tc->GetValue().Trim().c_str();
    ex.code = code_tc->GetValue().Trim().c_str();
    ex.in_menu = inmenu_cb->GetValue();
    wxDialog::OnOK(event);
}

/// get path ~/.fityk/filename and create ~/.fityk/ dir if not exists
wxString get_user_conffile(const wxString &filename)
{
    wxString fityk_dir = wxGetHomeDir() + wxFILE_SEP_PATH + config_dirname;
    if (!wxDirExists(fityk_dir))
        wxMkdir(fityk_dir);
    return fityk_dir + wxFILE_SEP_PATH + filename;
}


/// show "Export data" dialog
bool export_data_dlg(wxWindow *parent, bool load_exported)
{
    static wxString dir = ".";
    wxFileDialog fdlg (parent, "Export data to file", dir, "",
                       "x y data (*.dat, *.xy)|*.dat;*.DAT;*.xy;*.XY",
                       wxSAVE | wxOVERWRITE_PROMPT);
    dir = fdlg.GetDirectory();
    if (fdlg.ShowModal() == wxID_OK) {
        string path = fdlg.GetPath().c_str();
        exec_command("d.export '" + path + "'");
        if (load_exported)
            exec_command("d.load '" + path + "'");
        return true;
    }
    else
        return false;
}


