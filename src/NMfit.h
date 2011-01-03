// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+

#ifndef FITYK__NMFIT__H__
#define FITYK__NMFIT__H__

#include <vector>
#include "common.h"
#include "fit.h"

/// Vertex used in Nelder-Mead method
struct Vertex
{
    std::vector<fp> a;
    bool computed;
    fp wssr;

    Vertex() : a(0), computed(false), wssr(0.) {}
    Vertex(int n) : a(n), computed(false), wssr(0.) {}
    Vertex (std::vector<fp>& a_) : a(a_), computed(false), wssr(0.) {}
};

///              Nelder-Mead simplex method
class NMfit : public Fit
{
public:
    NMfit(Ftk* F);
    ~NMfit();
    fp init(); // called before autoiter()
    void autoiter();
private:
    int iteration;
    std::vector<Vertex> vertices;
    std::vector<Vertex>::iterator best, s_worst /*second worst*/, worst;
    std::vector<fp> coord_sum;
    fp volume_factor;

    void find_best_worst();
    void change_simplex();
    fp try_new_worst (fp f);
    void compute_coord_sum();
    bool termination_criteria (int iter, fp convergence);
    void compute_v (Vertex& v);
};

#endif

