// This file is part of fityk program. Copyright (C) Marcin Wojdyr

// wxwindows headers, see wxwindows samples for description
#include <wx/wxprec.h>
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "common.h"
RCSID ("$Id$")

#include <wx/laywin.h>
#include <wx/sashwin.h>
#include <wx/minifram.h>

#include "v_IO.h"
#include "wx_plot.h"
#include "wx_gui.h"
#include "data.h"
#include "sum.h"

int my_sleep (int seconds) 
{
    wxSleep (seconds);
    return 0;
}

void interrupt_handler (int /*signum*/)
{
    user_interrupt = true;
}

void sys_depen_init() 
{
#ifdef SIGINT
#ifdef SIG_IGN
    if (signal (SIGINT, interrupt_handler) == SIG_IGN) 
        signal (SIGINT, SIG_IGN);
#endif
#endif
}


bool gui_IO::start (const char* /*parameter*/)
{
    return true;
}

void gui_IO::message (const char *s)
{
    Output_style_enum style = os_ty_normal;
    if (s && s[0] == '!')  //warning
        style = os_ty_warn;
    else if (s && isdigit(s[0])) { //quoted input
        int i = 0;
        while (isdigit (s[i]))
            i++;
        if (s[i] == '>')
            style = os_ty_quot;
    }
    frame->output_win->append_text (wxString(s) + "\n", style);
}

void gui_IO::plot ()
{
    if (my_sum->was_changed() || my_data->was_changed())
        clear_buffered_sum();
    a_copy4plot.clear();
    frame->plot->Refresh(false);
    frame->diff_plot->Refresh(false);
}

void gui_IO::plot_now (const std::vector<fp>& a)
{
    clear_buffered_sum();
    a_copy4plot = a;
    frame->plot->Refresh(false);
    frame->plot->Update();
    frame->diff_plot->Refresh(false);
    frame->diff_plot->Update();
}

