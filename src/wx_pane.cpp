// This file is part of fityk program. Copyright (C) Marcin Wojdyr

// wxwindows headers, see wxwindows samples for description
#ifdef __GNUG__
#pragma implementation
#endif
#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "common.h"
RCSID ("$Id$")

#include <wx/colordlg.h>
#include <wx/fontdlg.h>
#include <wx/treectrl.h>
#include "wx_pane.h" 
#include "wx_gui.h" 
#include "wx_plot.h" 
#include "data.h" 
#include "pcore.h" 
#include "other.h" 
#include "ui.h" 

using namespace std;

enum { 
    ID_COMBO            = 47001,
    ID_OUTPUT_TEXT             ,

    ID_OUTPUT_C_BG             ,
    ID_OUTPUT_C_IN             ,
    ID_OUTPUT_C_OU             ,
    ID_OUTPUT_C_QT             ,
    ID_OUTPUT_C_WR             ,
    ID_OUTPUT_C                ,
    ID_OUTPUT_P_FONT           ,
    ID_OUTPUT_P_CLEAR          ,
    ID_DATAPANE_TREE           ,
    ID_DPT_POPUP_APPEND_DATA   ,
    ID_DPT_POPUP_APPEND_PLOT   ,
    ID_DPT_POPUP_REMOVE_DATA   ,
    ID_DPT_POPUP_REMOVE_PLOT
};


//===============================================================
//                            PlotPane
//===============================================================

BEGIN_EVENT_TABLE(PlotPane, ProportionalSplitter)
END_EVENT_TABLE()

PlotPane::PlotPane(wxWindow *parent, wxWindowID id)
    : ProportionalSplitter(parent, id, 0.75), plot_shared()
{
    plot = new MainPlot(this, plot_shared);
    aux_split = new ProportionalSplitter(this, -1, 0.5);
    SplitHorizontally(plot, aux_split);

    aux_plot[0] = new AuxPlot(aux_split, plot_shared, "0");
    aux_plot[1] = new AuxPlot(aux_split, plot_shared, "1");
    aux_plot[1]->Show(false);
    aux_split->Initialize(aux_plot[0]);
}

void PlotPane::zoom_forward()
{
    const int max_length_of_zoom_history = 10;
    zoom_hist.push_back(my_core->view.str());
    if (size(zoom_hist) > max_length_of_zoom_history)
        zoom_hist.erase(zoom_hist.begin());
}

string PlotPane::zoom_backward(int n)
{
    if (n < 1 || zoom_hist.empty()) return "";
    int pos = zoom_hist.size() - n;
    if (pos < 0) pos = 0;
    string val = zoom_hist[pos];
    zoom_hist.erase(zoom_hist.begin() + pos, zoom_hist.end());
    return val;
}

void PlotPane::save_settings(wxConfigBase *cf) const
{
    cf->SetPath("/PlotPane");
    cf->Write("PlotPaneProportion", GetProportion());
    cf->Write("AuxPlotsProportion", aux_split->GetProportion());
    cf->Write("ShowAuxPane0", aux_visible(0));
    cf->Write("ShowAuxPane1", aux_visible(1));
    plot->save_settings(cf);
    aux_plot[0]->save_settings(cf);
    aux_plot[1]->save_settings(cf);
}

void PlotPane::read_settings(wxConfigBase *cf)
{
    cf->SetPath("/PlotPane");
    SetProportion(from_config_read_double(cf, "PlotPaneProportion", 0.75));
    aux_split->SetProportion(from_config_read_double(cf, "AuxPlotsProportion", 
                                                         0.5));
    show_aux(0, from_config_read_bool(cf, "ShowAuxPane0", true));
    show_aux(1, from_config_read_bool(cf, "ShowAuxPane1", false));
    plot->read_settings(cf);
    aux_plot[0]->read_settings(cf);
    aux_plot[1]->read_settings(cf);
}

void PlotPane::refresh_plots(bool update)
{
    vector<FPlot*> vp = get_visible_plots();
    for (vector<FPlot*>::const_iterator i = vp.begin(); i != vp.end(); ++i) {
        (*i)->Refresh(false);
        if (update) (*i)->Update();
    }
}

void PlotPane::set_mouse_mode(Mouse_mode_enum m) 
{ 
    plot->set_mouse_mode(m); 
}

void PlotPane::update_mouse_hints() 
{ 
    plot->update_mouse_hints();
}

bool PlotPane::is_background_white() 
{ 
    //have all visible plots white background?
    vector<FPlot*> vp = get_visible_plots();
    for (vector<FPlot*>::const_iterator i = vp.begin(); i != vp.end(); ++i) 
        if ((*i)->get_bg_color() != *wxWHITE)
            return false;
    return true;
}

const std::vector<FPlot*> PlotPane::get_visible_plots() const
{
    vector<FPlot*> visible;
    visible.push_back(plot);
    for (int i = 0; i < 2; ++i)
        if (aux_visible(i))
            visible.push_back(aux_plot[i]);
    return visible;
}

bool PlotPane::aux_visible(int n) const
{
    return IsSplit() && (aux_split->GetWindow1() == aux_plot[n]
                         || aux_split->GetWindow2() == aux_plot[n]);
}

void PlotPane::show_aux(int n, bool show)
{
    if (aux_visible(n) == show) return;

    if (show) {
        if (!IsSplit()) { //both where invisible
            SplitHorizontally(plot, aux_split);
            aux_split->Show(true);
            assert(!aux_split->IsSplit());
            if (aux_split->GetWindow1() == aux_plot[n])
                ;
            else {
                aux_split->SplitHorizontally(aux_plot[0], aux_plot[1]);
                aux_plot[n]->Show(true);
                aux_split->Unsplit(aux_plot[n==0 ? 1 : 0]);
            }
        }
        else {//one was invisible
            aux_split->SplitHorizontally(aux_plot[0], aux_plot[1]);
            aux_plot[n]->Show(true);
        }
    }
    else { //hide
        if (aux_split->IsSplit()) //both where visible
            aux_split->Unsplit(aux_plot[n]);
        else // only one was visible
            Unsplit(); //hide whole aux_split
    }
}


//===============================================================
//                            IOPane
//===============================================================

BEGIN_EVENT_TABLE(IOPane, wxPanel)
END_EVENT_TABLE()

IOPane::IOPane(wxWindow *parent, wxWindowID id)
    : wxPanel(parent, id), output_win(0), input_combo(0)
{
    wxBoxSizer *io_sizer = new wxBoxSizer (wxVERTICAL);

    // wxTextCtrl which displays output of commands
    output_win = new Output_win (this, ID_OUTPUT_TEXT);
    io_sizer->Add (output_win, 1, wxEXPAND);

    // FCombo - wxComboBox used for user keybord input
    //wxString input_choices[] = { /*"help"*/ };
    input_combo = new FCombo (this, ID_COMBO, "",
                               wxDefaultPosition, wxDefaultSize, 
                               0, 0,//input_choices, 
                               wxCB_DROPDOWN|wxWANTS_CHARS|
                               wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB);
    io_sizer->Add (input_combo, 0, wxEXPAND);

    SetAutoLayout (true);
    SetSizer (io_sizer);
    io_sizer->Fit (this);
    io_sizer->SetSizeHints (this);
}

void IOPane::save_settings(wxConfigBase *cf) const
{
    output_win->save_settings(cf);
}

void IOPane::read_settings(wxConfigBase *cf)
{
    output_win->read_settings(cf);
}


//===============================================================
//                            DataPane
//===============================================================

BEGIN_EVENT_TABLE(DataPane, wxPanel)
END_EVENT_TABLE()

    DataPane::DataPane(wxWindow *parent, wxWindowID id)
: wxPanel(parent, id), tree(0)
{
    wxBoxSizer *sizer = new wxBoxSizer (wxVERTICAL);
    tree = new DataPaneTree(this, ID_DATAPANE_TREE);
    sizer->Add(tree, 1, wxEXPAND);

    SetAutoLayout (true);
    SetSizer (sizer);
    sizer->Fit (this);
    sizer->SetSizeHints (this);
}
//===============================================================
//                            DataPaneTree
//===============================================================

BEGIN_EVENT_TABLE(DataPaneTree, wxTreeCtrl)
    EVT_IDLE (DataPaneTree::OnIdle)
    EVT_TREE_SEL_CHANGING (ID_DATAPANE_TREE, DataPaneTree::OnSelChanging)
    EVT_TREE_SEL_CHANGED (ID_DATAPANE_TREE, DataPaneTree::OnSelChanged)
    EVT_RIGHT_DOWN (DataPaneTree::OnPopupMenu)
    EVT_MENU (ID_DPT_POPUP_APPEND_DATA, DataPaneTree::OnMenuItem)
    EVT_MENU (ID_DPT_POPUP_APPEND_PLOT, DataPaneTree::OnMenuItem)
    EVT_MENU (ID_DPT_POPUP_REMOVE_DATA, DataPaneTree::OnMenuItem)
    EVT_MENU (ID_DPT_POPUP_REMOVE_PLOT, DataPaneTree::OnMenuItem)
    EVT_KEY_DOWN (                      DataPaneTree::OnKeyDown)
END_EVENT_TABLE()

DataPaneTree::DataPaneTree(wxWindow *parent, wxWindowID id)
    : wxTreeCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
                 wxTR_HIDE_ROOT|wxTR_HAS_BUTTONS|wxTR_TWIST_BUTTONS)
{
    AddRoot("root");
}

void DataPaneTree::OnIdle(wxIdleEvent &event)
{
    if (!IsShown()) return;

    const wxTreeItemId& root = GetRootItem();
    //correct number of plots,
    int diff = AL->get_core_count() - GetChildrenCount(root, false);
    if (diff > 0)
        for (int i = 0; i < diff; i++)
            AppendItem(root, "");
    else if (diff < 0)
        for (int i = 0; i < -diff; i++)
            Delete(GetLastChild(root));
    //  ... "plot n" labels, and data files
    long cookie, cookie2;
    int counter = 0;
    wxString label;
    for (wxTreeItemId i = GetFirstChild(root, cookie); i.IsOk(); 
                                   i = GetNextChild(root, cookie), counter++) {
        label.Printf("plot %d", counter);
        if (GetItemText(i) != label)
            SetItemText(i, label);
        update_tree_datalabels(AL->get_core(counter), i); 
        // if active plot, select active data item
        if (AL->get_active_core_position() == counter) {
            wxTreeItemId active_item = GetFirstChild(i, cookie2);
            for (int j = 0; j < my_core->get_active_data_position(); j++)
                active_item = GetNextChild(i, cookie2);
            if (!IsSelected(active_item))
                SelectItem(active_item);
        }
    }
    event.Skip();
}

void DataPaneTree::update_tree_datalabels(const PlotCore *pcore, 
                                          const wxTreeItemId &plot_item)
{
    vector<string> new_labels = pcore->get_data_titles(); 
    vector<string> old_labels;
    long cookie;
    for (wxTreeItemId i = GetFirstChild(plot_item, cookie); i.IsOk(); 
                                    i = GetNextChild(plot_item, cookie)) 
        old_labels.push_back(GetItemText(i).c_str());

    if (new_labels != old_labels) {
        DeleteChildren(plot_item);
        for (vector<string>::const_iterator i = new_labels.begin();
                                                 i != new_labels.end(); i++)
            AppendItem(plot_item, (*i).c_str());
        Expand(plot_item);
    }
}

void DataPaneTree::OnSelChanging(wxTreeEvent &event)
{
    const wxTreeItemId &id = event.GetItem();
    if (id == GetRootItem() || GetItemParent(id) == GetRootItem()) 
        event.Veto();
}

void DataPaneTree::OnSelChanged(wxTreeEvent &event)
{
    const wxTreeItemId &id = event.GetItem();
    if (id == GetRootItem() || GetItemParent(id) == GetRootItem()) 
        return;
    else { //dataset
        int p = count_previous_siblings(GetItemParent(id));
        int d = count_previous_siblings(id);
        if (p != AL->get_active_core_position() 
                                || d != my_core->get_active_data_position())
        exec_command("d.activate " + S(p) + "::" + S(d));
    }
}

int DataPaneTree::count_previous_siblings(const wxTreeItemId &id)
{
    int counter = 0;
    for (wxTreeItemId i = id; i.IsOk(); i = GetPrevSibling(i))
        counter++;
    return counter-1;
}

void DataPaneTree::OnPopupMenu(wxMouseEvent &event)
{
    int flags;
    wxTreeItemId id = HitTest(event.GetPosition(), flags);

    //find title for menu
    wxString what = "data pane";
    if (id.IsOk()) 
        what = GetItemText(id);
    if (what.Length() > 20)
        what = "..." + what.Right(18);
    wxMenu popup_menu ("Menu for " + what);

    if (id.IsOk() && GetItemParent(id).IsOk()) { 
        if (GetItemParent(id) == GetRootItem()) { //plot
            pmenu_p = count_previous_siblings(id);
            pmenu_d = -1;
            popup_menu.Append (ID_DPT_POPUP_APPEND_DATA, 
                                                "&Append slot for dataset");
            popup_menu.Append (ID_DPT_POPUP_REMOVE_PLOT, 
                                            "&Remove plot (with datasets)");
            popup_menu.Append (ID_DPT_POPUP_APPEND_PLOT, "&New plot");
        }
        else { //data
            pmenu_p = count_previous_siblings(GetItemParent(id));
            pmenu_d = count_previous_siblings(id);
            popup_menu.Append (ID_DPT_POPUP_APPEND_DATA, 
                                                "&Append slot for dataset");
            popup_menu.Append (ID_DPT_POPUP_REMOVE_DATA, "&Remove dataset");
        }
    }
    else { //no item
        popup_menu.Append (ID_DPT_POPUP_APPEND_PLOT, "&New plot");
    }
    
    PopupMenu (&popup_menu, event.GetX(), event.GetY());
}

void DataPaneTree::OnMenuItem(wxCommandEvent &event)
{
    int eid = event.GetId();
    string cmd = "d.activate ";
    if (eid == ID_DPT_POPUP_APPEND_DATA)
        cmd += S(pmenu_p) + "::*";
    else if (eid == ID_DPT_POPUP_APPEND_PLOT)
        cmd += "*::";
    else if (eid == ID_DPT_POPUP_REMOVE_DATA)
        cmd += "! " + S(pmenu_p) + "::" + S(pmenu_d);
    else if (eid == ID_DPT_POPUP_REMOVE_PLOT)
        cmd += "! " + S(pmenu_p) + "::";
    exec_command(cmd);
}

void DataPaneTree::OnKeyDown(wxKeyEvent& event)
{
    if (event.GetKeyCode() == ' ' || event.GetKeyCode() == WXK_TAB) 
        frame->focus_input();
    else
        event.Skip();
}


//===============================================================
//                            Output_win
//===============================================================

BEGIN_EVENT_TABLE(Output_win, wxTextCtrl)
    EVT_RIGHT_DOWN (                      Output_win::OnRightDown)
    EVT_MENU_RANGE (ID_OUTPUT_C_BG, ID_OUTPUT_C_WR, Output_win::OnPopupColor)
    EVT_MENU       (ID_OUTPUT_P_FONT    , Output_win::OnPopupFont)
    EVT_MENU       (ID_OUTPUT_P_CLEAR   , Output_win::OnPopupClear)
    EVT_KEY_DOWN   (                      Output_win::OnKeyDown)
END_EVENT_TABLE()

Output_win::Output_win (wxWindow *parent, wxWindowID id, 
                        const wxPoint& pos, const wxSize& size)
    : wxTextCtrl(parent, id, "", pos, size,
                 wxTE_MULTILINE|wxTE_RICH|wxNO_BORDER|wxTE_READONLY)
{
    //GetFont().SetFamily(wxMODERN);
    fancy_dashes();
    //SetScrollbar (wxVERTICAL, 0, 5, 50);
}

void Output_win::fancy_dashes() {
    for (int i = 0; i < 16; i++) {
        SetDefaultStyle (wxTextAttr (wxColour(i * 16, i * 16, i * 16)));
        AppendText ("-");
    }
    AppendText ("\n");
}

void Output_win::read_settings(wxConfigBase *cf)
{
    cf->SetPath("/OutputWin/Colors");
    text_color[os_normal] = read_color_from_config(cf, "normal", 
                                                      wxColour(150, 150, 150));
    text_color[os_warn] = read_color_from_config(cf, "warn", 
                                                    wxColour(200, 0, 0));
    text_color[os_quot] = read_color_from_config(cf, "quot", 
                                                    wxColour(50, 50, 255));
    text_color[os_input] = read_color_from_config(cf, "input", 
                                                     wxColour(0, 200, 0));
    bg_color = read_color_from_config(cf, "bg", wxColour(20, 20, 20));

    SetDefaultStyle (wxTextAttr(text_color[os_quot], bg_color));
    if (frame->IsShown()) // this "if" is needed on GTK 1.2 (I don't know why)
        SetBackgroundColour(bg_color); //if it is called before window is shown,
    Refresh();                         //it doesn't work and it is impossible
}                                      // to change the background later.

void Output_win::save_settings(wxConfigBase *cf) const
{
    cf->SetPath("/OutputWin/Colors");
    write_color_to_config (cf, "normal", text_color[os_normal]);  
    write_color_to_config (cf, "warn", text_color[os_warn]); 
    write_color_to_config (cf, "quot", text_color[os_quot]); 
    write_color_to_config (cf, "input", text_color[os_input]); 
    write_color_to_config (cf, "bg", bg_color); 
}

void Output_win::append_text (OutputStyle style, const wxString& str)
{
    SetDefaultStyle (wxTextAttr (text_color[style]));
    AppendText (str);
}

void Output_win::OnPopupColor (wxCommandEvent& event)
{
    int n = event.GetId();
    wxColour *col;
    if (n == ID_OUTPUT_C_BG)
        col = &bg_color;
    else if (n == ID_OUTPUT_C_OU)
        col = &text_color[os_normal];
    else if (n == ID_OUTPUT_C_QT)
        col = &text_color[os_quot];
    else if (n == ID_OUTPUT_C_WR)
        col = &text_color[os_warn];
    else if (n == ID_OUTPUT_C_IN)
        col = &text_color[os_input];
    else 
        return;
    wxColourData col_data;
    col_data.SetCustomColour (0, *col);
    col_data.SetColour (*col);
    wxColourDialog dialog (this, &col_data);
    if (dialog.ShowModal() == wxID_OK) {
        *col = dialog.GetColourData().GetColour();
        SetBackgroundColour (bg_color);
        SetDefaultStyle (wxTextAttr(wxNullColour, bg_color));
        Refresh();
    }
}

void Output_win::OnPopupFont (wxCommandEvent& WXUNUSED(event))
{
    wxFontData data; 
    data.SetInitialFont (GetDefaultStyle().GetFont());
    wxFontDialog dlg (this, &data);
    int r = dlg.ShowModal();
    if (r == wxID_OK) {
        wxFont f = dlg.GetFontData().GetChosenFont();
        SetDefaultStyle (wxTextAttr (wxNullColour, wxNullColour, f));
        Refresh();
    }
}

void Output_win::OnPopupClear (wxCommandEvent& WXUNUSED(event))
{
    Clear();
    fancy_dashes();
}

    
void Output_win::OnRightDown (wxMouseEvent& event)
{
    wxMenu popup_menu ("output text menu");

    wxMenu *color_menu = new wxMenu;
    color_menu->Append (ID_OUTPUT_C_BG, "&Background");
    color_menu->Append (ID_OUTPUT_C_IN, "&Input");
    color_menu->Append (ID_OUTPUT_C_OU, "&Output");
    color_menu->Append (ID_OUTPUT_C_QT, "&Quotation");
    color_menu->Append (ID_OUTPUT_C_WR, "&Warning");
    popup_menu.Append  (ID_OUTPUT_C   , "&Color", color_menu);

    popup_menu.Append  (ID_OUTPUT_P_FONT, "&Font");
    popup_menu.Append  (ID_OUTPUT_P_CLEAR, "Clea&r");

    PopupMenu (&popup_menu, event.GetX(), event.GetY());
}

void Output_win::OnKeyDown (wxKeyEvent& event)
{
    if (event.GetKeyCode() == ' ' || event.GetKeyCode() == WXK_TAB) {
        IOPane *parent = static_cast<IOPane*>(GetParent()); //to not use RTTI
        parent->focus_input();
    }
    else
        event.Skip();
}

//===============================================================
//                            combo
//===============================================================

BEGIN_EVENT_TABLE(FCombo, wxComboBox)
    EVT_KEY_DOWN (FCombo::OnKeyDown)
END_EVENT_TABLE()

void FCombo::OnKeyDown (wxKeyEvent& event)
{
    const int n_list_items = 15;
    if (event.m_keyCode == WXK_RETURN || event.m_keyCode == WXK_NUMPAD_ENTER) {
        wxString s = GetValue().Trim();
        if (s.IsEmpty())
            return;
        frame->SetStatusText (s);
        // changing drop-down list
        vector<wxString> list;
        list.push_back(s);
        int n = std::min (n_list_items, GetCount() + 1);
        for (int i = 0; i < n - 1; i++)
            list.push_back(GetString(i));
        Clear();
        for (vector<wxString>::iterator i = list.begin(); i != list.end(); i++)
            Append (*i);
        SetValue("");

        //displaying and executing command
        exec_command (s.c_str());
    }
    else if (event.m_keyCode == WXK_TAB) {
        IOPane *parent = static_cast<IOPane*>(GetParent()); //to not use RTTI
        parent->focus_output();
    }
    else
        event.Skip();
}

//===============================================================
//                            FPrintout
//===============================================================

FPrintout::FPrintout(const PlotPane *p_pane) 
    : wxPrintout(my_data->get_filename().c_str()), pane(p_pane) 
{}

bool FPrintout::OnPrintPage(int page)
{
    if (page != 1) return false;
    if (my_data->is_empty()) return false;
    wxDC *dc = GetDC();
    if (!dc) return false;

    // Set the scale and origin
    const int space = 20; //vertical space between plots
    const int marginX = 50, marginY = 50; //page margins
    //width is the same for all plots
    int width = pane->plot->GetClientSize().GetWidth(); 
    vector<FPlot*> vp = pane->get_visible_plots();
    int height = -space;  //height = sum of all heights + (N-1)*space
    for (vector<FPlot*>::const_iterator i = vp.begin(); i != vp.end(); ++i) 
        height += (*i)->GetClientSize().GetHeight() + space;
    int w, h;
    dc->GetSize(&w, &h);
    fp scaleX = w / (width + 2.*marginX);
    fp scaleY = h / (height + 2.*marginY);
    fp actualScale = min (scaleX, scaleY);
    dc->SetUserScale (actualScale, actualScale);

    const int posX = iround((w - width * actualScale) / 2.);
    int posY = iround((h - height * actualScale) / 2.);

    //drawing all visible plots, every at different posY
    for (vector<FPlot*>::const_iterator i = vp.begin(); i != vp.end(); ++i) {
        dc->SetDeviceOrigin (posX, posY);
        (*i)->Draw(*dc);
        posY += iround(((*i)->GetClientSize().GetHeight()+space) * actualScale);
    }
    return true;
}


//===============================================================

wxColour read_color_from_config(const wxConfigBase *config, const wxString& key,
                                 const wxColour& default_value)
{
    return wxColour (config->Read (key + "/Red", default_value.Red()), 
                     config->Read (key + "/Green", default_value.Green()), 
                     config->Read (key + "/Blue", default_value.Blue()));
}

void write_color_to_config (wxConfigBase *config, const wxString& key,
                            const wxColour& value)
{
    config->Write (key + "/Red", value.Red());
    config->Write (key + "/Green", value.Green());
    config->Write (key + "/Blue", value.Blue());
}


//===============================================================
//                            ProportionalSplitter
//===============================================================

ProportionalSplitter::ProportionalSplitter(wxWindow* parent, wxWindowID id, 
                                           float proportion, const wxSize& size,
                                           long style, const wxString& name) 
    : wxSplitterWindow(parent, id, wxDefaultPosition, size, style, name),
      m_proportion(proportion), m_firstpaint(true)
{
    wxASSERT(m_proportion >= 0. && m_proportion <= 1.);
    SetMinimumPaneSize(20);
    ResetSash();
    Connect(GetId(), wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,
                (wxObjectEventFunction) &ProportionalSplitter::OnSashChanged);
    Connect(GetId(), wxEVT_SIZE, 
                     (wxObjectEventFunction) &ProportionalSplitter::OnReSize);
    //hack to set sizes on first paint event
    Connect(GetId(), wxEVT_PAINT, 
                      (wxObjectEventFunction) &ProportionalSplitter::OnPaint);
}

bool ProportionalSplitter::SplitHorizontally(wxWindow* win1, wxWindow* win2,
                                             float proportion) 
{
    if (proportion >= 0. && proportion <= 1.)
        m_proportion = proportion;
    int height = GetClientSize().GetHeight();
    int h = iround(height * m_proportion);
    //sometimes there is a strange problem without it (why?)
    if (h < GetMinimumPaneSize() || h > height-GetMinimumPaneSize())
        h = 0; 
    return wxSplitterWindow::SplitHorizontally(win1, win2, h);
}

bool ProportionalSplitter::SplitVertically(wxWindow* win1, wxWindow* win2,
                                           float proportion) 
{
    if (proportion >= 0. && proportion <= 1.)
        m_proportion = proportion;
    int width = GetClientSize().GetWidth();
    int w = iround(width * m_proportion);
    if (w < GetMinimumPaneSize() || w > width-GetMinimumPaneSize())
        w = 0;
    return wxSplitterWindow::SplitVertically(win1, win2, w);
}

int ProportionalSplitter::GetExpectedSashPosition()
{
    return iround(GetWindowSize() * m_proportion);
}

void ProportionalSplitter::ResetSash()
{
    SetSashPosition(GetExpectedSashPosition());
}

void ProportionalSplitter::OnReSize(wxSizeEvent& event)
{
    // We may need to adjust the sash based on m_proportion.
    ResetSash();
    event.Skip();
}

void ProportionalSplitter::OnSashChanged(wxSplitterEvent &event)
{
    // We'll change m_proportion now based on where user dragged the sash.
    const wxSize& s = GetSize();
    int t = GetSplitMode() == wxSPLIT_HORIZONTAL ? s.GetHeight() : s.GetWidth();
    m_proportion = float(GetSashPosition()) / t;
    event.Skip();
}

void ProportionalSplitter::OnPaint(wxPaintEvent &event)
{
    if (m_firstpaint) {
        if (GetSashPosition() != GetExpectedSashPosition())
            ResetSash();
        m_firstpaint = false;
    }
    event.Skip();
}

