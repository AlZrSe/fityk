// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$
#ifndef WX_COMMON__H__
#define WX_COMMON__H__

enum Mouse_mode_enum { mmd_zoom, mmd_bg, mmd_add, mmd_range, mmd_peak };

struct Plot_shared
{
    Plot_shared() : xUserScale(1.), xLogicalOrigin(0.), plot_y_scale(1e3) {}
    int x2X (fp x) {return static_cast<int>((x - xLogicalOrigin) * xUserScale);}
    fp X2x (int X) { return X / xUserScale + xLogicalOrigin; }
    int dx2dX (fp dx) { return static_cast<int>(dx * xUserScale); }
    fp dX2dx (int dX) { return dX / xUserScale; }

    fp xUserScale, xLogicalOrigin; 
    std::vector<std::vector<fp> > buf;
    fp plot_y_scale;
    std::vector<wxPoint> peaktops;
};

//because only wxString and long types can be read conveniently from wxConfig
class wxConfigBase;
class wxString;
bool from_config_read_bool(wxConfigBase *cf, const wxString& key, bool def_val);
double from_config_read_double(wxConfigBase *cf, const wxString& key, 
                               double def_val);
//dummy events -- useful when calling event handler functions
extern wxMouseEvent dummy_mouse_event;
extern wxCommandEvent dummy_cmd_event;

#endif // WX_COMMON__H__
