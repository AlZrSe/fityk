// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+

#ifndef FITYK_WX_FRAME_H_
#define FITYK_WX_FRAME_H_

#include <list>
#include <wx/spinctrl.h>

#include <wx/filename.h>
#include "cmn.h"  // enums
#include "../ui.h" // UserInterface::Style

class ApplicationLogic;
class FDXLoadDlg;
class PlotPane;
class MainPlot;
class IOPane;
class SideBar;
class ProportionalSplitter;
class DataEditorDlg;
class PrintManager;
class FStatusBar;
class DataAndModel;
struct RealRange;

class Ftk;
extern Ftk *ftk;

/// Toolbar bar in Fityk
class FToolBar : public wxToolBar
{
public:
    FToolBar (wxFrame *parent, wxWindowID id);
    void update_peak_type(int nr, std::vector<std::string> const* peak_types=0);

    void OnPeakChoice (wxCommandEvent& event);
    void OnChangeMouseMode (wxCommandEvent& event);
    void OnSwitchSideBar (wxCommandEvent& event);
    void OnClickTool (wxCommandEvent& event);

private:
    wxChoice *peak_choice;

    DECLARE_EVENT_TABLE()
};



/// Fityk-GUI main window
class FFrame: public wxFrame
{
    friend class FToolBar;
    friend class FApp;
public:
    FFrame(wxWindow *parent, const wxWindowID id, const wxString& title,
            const long style);
    ~FFrame();
    //void OnSize (wxSizeEvent& event);

    void OnShowHelp(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnOnline(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);

    void OnDataRevertUpdate (wxUpdateUIEvent& event);
    void OnDataExportUpdate (wxUpdateUIEvent& event);
    void OnDataQLoad     (wxCommandEvent& event);
    void OnDataXLoad     (wxCommandEvent& event);
    void OnDataRecent    (wxCommandEvent& event);
    void OnDataRevert    (wxCommandEvent& event);
    void OnDataTable     (wxCommandEvent& event);
    void OnDataEditor    (wxCommandEvent& event);
    void OnSavedDT       (wxCommandEvent& event);
    void OnDataMerge     (wxCommandEvent&);
    void OnDataCalcShirley (wxCommandEvent&);
    void OnDataRmShirley (wxCommandEvent&);
    void OnDataExport    (wxCommandEvent&);

    void OnDefinitionMgr (wxCommandEvent&);
    void OnSGuess        (wxCommandEvent& event);
    void OnSPFInfo       (wxCommandEvent& event);
    void OnAutoFreeze    (wxCommandEvent& event);
    void OnSExport       (wxCommandEvent& event);

    void OnFMethodUpdate (wxUpdateUIEvent& event);
    void OnMenuFitRunUpdate (wxUpdateUIEvent& event);
    void OnMenuFitUndoUpdate (wxUpdateUIEvent& event);
    void OnMenuFitRedoUpdate (wxUpdateUIEvent& event);
    void OnMenuFitHistoryUpdate (wxUpdateUIEvent& event);
    void OnFOneOfMethods (wxCommandEvent& event);
    void OnFRun          (wxCommandEvent& event);
    void OnFInfo         (wxCommandEvent& event);
    void OnFUndo         (wxCommandEvent& event);
    void OnFRedo         (wxCommandEvent& event);
    void OnFHistory      (wxCommandEvent& event);

    void OnPowderDiffraction (wxCommandEvent&);

    void OnMenuLogStartUpdate (wxUpdateUIEvent& event);
    void OnMenuLogStopUpdate (wxUpdateUIEvent& event);
    void OnMenuLogOutputUpdate (wxUpdateUIEvent& event);
    void OnLogStart      (wxCommandEvent& event);
    void OnLogStop       (wxCommandEvent& event);
    void OnLogWithOutput (wxCommandEvent& event);
    void OnSaveHistory   (wxCommandEvent& event);
    void OnInclude      (wxCommandEvent& event);
    void OnReInclude    (wxCommandEvent& event);
    void OnShowEditor   (wxCommandEvent&) { show_editor(wxT("")); }
    void show_editor (wxString const& path);
    void OnReset       (wxCommandEvent&);
    void OnDump         (wxCommandEvent&);
    void OnSettings      (wxCommandEvent&);
    void OnEditInit      (wxCommandEvent&);
    void OnPrintPreview  (wxCommandEvent&);
    void OnPageSetup     (wxCommandEvent&);
    void OnPrint         (wxCommandEvent&);
    void OnPrintPSFile   (wxCommandEvent&);
    void OnPrintToClipboard (wxCommandEvent&);
    void OnSaveAsImage (wxCommandEvent&);
    void OnChangeMouseMode (wxCommandEvent&);
    void OnChangePeakType(wxCommandEvent& event);
    void OnMenuBgStripUpdate(wxUpdateUIEvent& event);
    void OnMenuBgUndoUpdate(wxUpdateUIEvent& event);
    void OnMenuBgClearUpdate(wxUpdateUIEvent& event);
    void OnStripBg       (wxCommandEvent& event);
    void OnUndoBg        (wxCommandEvent& event);
    void OnClearBg       (wxCommandEvent& event);
    void OnRecentBg      (wxCommandEvent& event);
    void OnConvexHullBg  (wxCommandEvent& event);
    void OnSplineBg      (wxCommandEvent& event);
    void GViewAll();
    void OnGViewAll      (wxCommandEvent&) { GViewAll(); }
    void OnGFitHeight    (wxCommandEvent& event);
    void OnGScrollLeft   (wxCommandEvent& event);
    void OnGScrollRight  (wxCommandEvent& event);
    void OnGScrollUp     (wxCommandEvent& event);
    void OnGExtendH      (wxCommandEvent& event);
    void OnPreviousZoom  (wxCommandEvent& event);
    void OnConfigBuiltin (wxCommandEvent& event);
    void OnConfigX (wxCommandEvent& event);
    void OnSaveDefaultConfig(wxCommandEvent& event);
    void OnSaveConfigAs(wxCommandEvent&);
    void OnMenuShowAuxUpdate(wxUpdateUIEvent& event);
    void SwitchSideBar(bool show);
    void OnSwitchSideBar(wxCommandEvent& ev) {SwitchSideBar(ev.IsChecked());}
    void OnSwitchAuxPlot(wxCommandEvent& ev);
    void SwitchIOPane(bool show);
    void OnSwitchIOPane(wxCommandEvent& ev) {SwitchIOPane(ev.IsChecked());}
    void SwitchToolbar(bool show);
    void OnSwitchToolbar(wxCommandEvent& ev) {SwitchToolbar(ev.IsChecked());}
    void SwitchStatbar(bool show);
    void OnSwitchStatbar(wxCommandEvent& ev) {SwitchStatbar(ev.IsChecked());}
    void SwitchCrosshair(bool show);
    void OnShowPrefDialog(wxCommandEvent& ev);
    void OnConfigureStatusBar(wxCommandEvent& event);
    void OnConfigureOutputWin(wxCommandEvent&);
    void OnSwitchCrosshair(wxCommandEvent& e) {SwitchCrosshair(e.IsChecked());}
    void OnSwitchFullScreen(wxCommandEvent& event);
    void OnGShowY0(wxCommandEvent& e);
    void save_config_as(wxString const& name);
    void read_config(wxString const& name);
    void save_all_settings(wxConfigBase *cf) const;
    void save_settings(wxConfigBase *cf) const;
    void read_all_settings(wxConfigBase *cf);
    void read_settings(wxConfigBase *cf);
    const FToolBar* get_toolbar() const { return toolbar; }
    std::string get_peak_type() const;
    void set_status_text(std::string const& text);
    void set_status_coords(fp x, fp y, PlotTypeEnum pte);
    void clear_status_coords();
    void output_text(UserInterface::Style style, std::string const& str);
    void change_zoom(const RealRange& h, const RealRange& v);
    void scroll_view_horizontally(fp step);
    void refresh_plots(bool now, WhichPlot which_plot);
    void update_crosshair(int X, int Y);
    void focus_input(wxKeyEvent& event);
    void edit_in_input(std::string const& s);
    void after_cmd_updates();
    void update_toolbar();
    void update_config_menu(wxMenu *menu);
    int get_focused_data_index();
    std::vector<int> get_selected_data_indices();
    std::vector<DataAndModel*> get_selected_dms();
    std::string get_datasets();
    std::string get_guess_string(const std::string& name);
    MainPlot* get_main_plot();
    MainPlot const* get_main_plot() const;
    void update_data_pane();
    SideBar const* get_sidebar() const { return sidebar; }
    SideBar* get_sidebar() { return sidebar; }
    void activate_function(int n);
    void update_app_title();
    void add_recent_data_file(std::string const& filename);
    void update_menu_functions();
    void update_menu_saved_transforms();
    void update_menu_recent_baselines();
    void update_menu_previous_zooms();
    // overridden from wxFrameBase, to show help in our status bar replacement
    void DoGiveHelp(const wxString& help, bool show);

private:
    ProportionalSplitter *main_pane;
    PlotPane *plot_pane;
    IOPane *io_pane;
    SideBar *sidebar;
    FStatusBar *status_bar;

    int peak_type_nr;
    std::vector<std::string> peak_types;
    FToolBar *toolbar;
    ProportionalSplitter *v_splitter;
    PrintManager* print_mgr;
    std::string last_include_path;
    std::list<wxFileName> recent_data_files;
    wxMenu *data_menu_recent, *data_ft_menu, *func_type_menu;
    wxString script_dir_, data_dir_, export_dir_;

    void place_plot_and_io_windows(wxWindow *parent);
    void create_io_panel(wxWindow *parent);
    void set_menubar();
    void update_peak_type_list();
    void read_recent_data_files();
    void write_recent_data_files();
    void change_mouse_mode(MouseModeEnum mode);

    DECLARE_EVENT_TABLE()
};

extern FFrame *frame;

#endif // FITYK_WX_FRAME_H_

