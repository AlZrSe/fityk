// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#ifndef FITYK__GNUPLOT__H__
#define FITYK__GNUPLOT__H__
#include "common.h"
#include <stdio.h>
#include <vector>

class GnuPlot
{
public:
    GnuPlot();
    ~GnuPlot();
    int plot();
    void raw_command(char *command);// no syntax checking
    static char path_to_gnuplot[] ; 

private:
    FILE *gnuplot_pipe;

    void fork_and_make_pipe ();
    bool gnuplot_pipe_ok();
};

#endif 
