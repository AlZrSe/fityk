// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id $

#ifndef FITYK__WX_FDLG__H__
#define FITYK__WX_FDLG__H__

#include <wx/treectrl.h>
#include <wx/spinctrl.h>
#include <wx/dirctrl.h>
#include <wx/print.h>
#include "wx_common.h"


class PreviewPlot;
class PlotPane;

class FDXLoadDlg : public wxDialog
{
public:
    FDXLoadDlg (wxWindow* parent, wxWindowID id, int n, Data* data);

protected:
    int data_nr;
    ProportionalSplitter *splitter, *right_splitter;
    wxGenericDirCtrl *dir_ctrl;
    wxTextCtrl *filename_tc, *text_preview;
    wxSpinCtrl *x_column, *y_column, *s_column;
    wxPanel *left_panel, *rupper_panel, *rbottom_panel, *columns_panel;
    PreviewPlot *plot_preview;
    wxCheckBox *std_dev_cb, *auto_text_cb, *auto_plot_cb;
    wxButton *open_here, *open_new;
    bool initialized;

    std::string get_command_tail();
    std::string get_filename();
    void OnStdDevCheckBox (wxCommandEvent& event);
    void OnAutoTextCheckBox (wxCommandEvent& event);
    void OnAutoPlotCheckBox (wxCommandEvent& event);
    void OnColumnChanged (wxSpinEvent& event);
    void OnOpenHere (wxCommandEvent& event);
    void OnOpenNew (wxCommandEvent& event);
    void OnClose (wxCommandEvent& event);
    void on_path_change();
    void on_filter_change();
    void OnPathSelectionChanged(wxTreeEvent &WXUNUSED(event)){on_path_change();}
    void update_text_preview();
    void update_plot_preview();
    DECLARE_EVENT_TABLE()
};

bool export_data_dlg(wxWindow *parent, bool load_exported=false);


class PrintManager
{
public:
    bool landscape;
    bool colors;
    int scale;
    bool keep_ratio; 
    bool plot_aux[2], plot_borders;
    PlotPane const* plot_pane;

    PrintManager(PlotPane const* pane);
    ~PrintManager();
    wxPrintData& get_print_data();
    wxPageSetupDialogData& get_page_data();
    void print();
    void print_to_psfile();
    void printPreview();
    void pageSetup(); 
    void save_settings(wxConfigBase *cf) const;
    void read_settings(wxConfigBase *cf);
private:
    wxPrintData *print_data;
    wxPageSetupDialogData *page_setup_data;
};


class PageSetupDialog: public wxDialog
{
public:
    PageSetupDialog(wxWindow *parent, PrintManager *print_mgr);
    void OnOk(wxCommandEvent& event);
protected:
    PrintManager *pm;
    wxRadioBox *orientation, *colors;
    wxComboBox *papers;
    wxCheckBox *keep_ratio, *plot_aux[2], *plot_borders;
    wxSpinCtrl *left_margin, *right_margin, *top_margin, *bottom_margin,
               *scale;
    DECLARE_EVENT_TABLE()
};


class FPrintout: public wxPrintout
{
public:
    FPrintout(PrintManager const* print_manager);
    bool HasPage(int page) { return (page == 1); }
    bool OnPrintPage(int page);
    void GetPageInfo(int *minPage,int *maxPage,int *selPageFrom,int *selPageTo)
        { *minPage = *maxPage = *selPageFrom = *selPageTo = 1; }
private:
    PrintManager const* pm;
};

void do_print_plots(wxDC *dc, PrintManager const* pm);

#endif

