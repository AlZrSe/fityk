/* This file is part of fityk program. Copyright (C) Marcin Wojdyr */

/* file for bison. */

%{
#include "common.h"
RCSID("$Id$")
#include <string>
#include <stdlib.h>
#include <vector>
#include "v_fit.h"
#include "crystal.h"
#include "sum.h"
#include "data.h"
#include "v_IO.h"
#include "manipul.h"
#include "other.h"

using namespace std;
int iperror (char *s);
int iplex ();
struct yy_buffer_state;
yy_buffer_state *yy_scan_string(const char *str);
void ip_delete_buffer (yy_buffer_state *b);
vector<int> ivec, ivec2;
vector <double> fvec;
vector<Pag> pgvec;

bool new_line = false;

inline void imsg(std::string s) {if (verbosity >= 2 && !s.empty()) gmessage(s);}

void replot()
{
    if (new_line)
        new_line = false;
    else
        return; //do not replot
    if (auto_plot >= 2 && my_core->was_changed()) {
        my_IO->plot();
	my_core->was_plotted();
    }
}

#define YYERROR_VERBOSE 1

%}


%union 
{
    bool bol;
    int i;
    char c;
    double f;
    Pre_string s;
    pre_Hkl pl;
    struct { int ph; pre_Hkl hkl; } ph_n_pl;
    pre_Domain pre_domain;
    struct { double l, r; } range;
    Pre_Pag pre_pag;
    struct { One_of_fzg fzg; char c; } fzg_type_type;
    struct { One_of_fzg fzg; int i; } fzg_num_type;
}

%token <c> SET
%token D_ACTIVATE
%token D_LOAD D_BACKGROUND D_CALIBRATE D_RANGE D_DEVIATION D_INFO D_EXPORT
%token F_RUN F_CONTINUE F_METHOD F_INFO 
%token S_ADD S_FREEZE S_HISTORY S_INFO S_REMOVE S_CHANGE S_VALUE S_EXPORT 
%token C_WAVELENGTH C_ADD C_INFO C_REMOVE C_FIND 
%token O_PLOT O_LOG O_INCLUDE O_WAIT O_DUMP
%token M_FINDPEAK
%token QUIT 
%token PLUS_MINUS TWO_COLONS
%token SEP
%token <s> FILENAME DASH_STRING EQ_STRING 
%token <c> LOWERCASE G_TYPE F_TYPE Z_TYPE PH_TYPE
%token <i> UINt UI_DASH UI_SLASH INt A_NUM G_NUM F_NUM Z_NUM PH_NUM
%token <f> FLOAt P_NUM NEW_A
/* %token <pl> PLANE */
%token <s> LEX_ERROR
%type <i> inr opt_uint opt_uint_1 a_num dl_merge dload_arg 
%type <f> flt opt_flt
%type <c> sign opt_lcase opt_proc
%type <range> range sim_range bracket_range
%type <ph_n_pl> p_plane
%type <pl> plane
%type <pre_pag> pag
%type <pre_domain> domain
%type <fzg_type_type> fzg_type
%type <fzg_num_type> fzg_num
%type <bol> opt_asterix opt_plus

%%

input:	 /*empty*/
    | input SEP { replot(); }
    | input exp { replot(); }
    | input LEX_ERROR error SEP {
    	warn ("Syntax error at the beginning of command."); 
	yyerrok; yyclearin; }
    | input error LEX_ERROR error SEP { 
    	warn ("Syntax error near unknown token: `" + $3.str() + "'"); 
	yyerrok; yyclearin; }
    | input error SEP { warn("Parse error."); 
      yyerrok; yyclearin;}
    ;

exp:  SET DASH_STRING EQ_STRING SEP { 
	set_class_p($1)->setp ($2.str(), $3.str());
      }
    | SET DASH_STRING SEP          { set_class_p($1)->getp ($2.str()); }
    | SET SEP                      { imsg (set_class_p($1)->print_usage($1)); }
    | D_ACTIVATE opt_uint_1 TWO_COLONS opt_uint_1 SEP 
                                        { my_core->activate_data(/*$2,*/ $4); }
    | D_LOAD opt_lcase dload_arg FILENAME SEP { 
                                   my_data->load($4.str(), $2, ivec, ivec2, $3);
				   my_core->set_view (Rect()); }
    | D_RANGE sign range SEP  { my_data->change_range ($3.l, $3.r, $2 == '+'); }
    | D_RANGE range SEP            { 
                                     my_data->change_range (-INF, +INF, false);
                                     my_data->change_range ($2.l, $2.r);
	                           }
    | D_RANGE SEP                  { 
		    imsg("Active data range: " + my_data->range_as_string()); }
    | D_RANGE '*' flt flt SEP    { my_data->auto_range ($3, $4); }
    | D_BACKGROUND '*' UINt flt flt SEP { /*experimental auto-background*/
                 my_data->auto_background ($3, $4, false, $5, false); }
    | D_BACKGROUND '*' UINt flt '%' flt SEP { 
                  my_data->auto_background ($3, $4, true, $6, false); }
    | D_BACKGROUND '*' UINt flt flt '%' SEP { 
                  my_data->auto_background ($3, $4, false, $5, true); }
    | D_BACKGROUND '*' UINt flt '%' flt '%' SEP {
                  my_data->auto_background ($3, $4, true, $6, true); }
    | D_BACKGROUND flt flt SEP {my_data->add_background_point($2, $3, bgc_bg);}
    | D_BACKGROUND '!' flt SEP   { my_data->rm_background_point($3, bgc_bg); }
    | D_BACKGROUND SEP      { imsg (my_data->background_info(bgc_bg)); }
    | D_BACKGROUND '.' SEP    { my_data->recompute_background(bgc_bg); }
    | D_BACKGROUND '!' SEP        { my_data->clear_background(bgc_bg); }
    | D_CALIBRATE flt flt SEP {my_data->add_background_point($2, $3, bgc_cl);}
    | D_CALIBRATE '!' flt SEP{my_data->rm_background_point($3,bgc_cl);}
    | D_CALIBRATE SEP              { imsg (my_data->background_info(bgc_cl)); }
    | D_CALIBRATE '.' SEP          { my_data->recompute_background(bgc_cl); }
    | D_CALIBRATE '!' SEP          { my_data->clear_background(bgc_cl); }
    | D_DEVIATION LOWERCASE opt_flt SEP  { my_data->change_sigma($2, $3); }
    | D_DEVIATION SEP              { imsg (my_data->print_sigma()); }
    | D_INFO SEP                   { imsg (my_data->info()); } 
    | D_EXPORT opt_lcase FILENAME opt_plus SEP 
                                   { my_data->export_to_file($3.str(), $4, $2);}
    | F_RUN UINt SEP               { my_fit->fit(true, $2); }
    | F_RUN SEP                    { my_fit->fit(true, -1); }
    | F_CONTINUE UINt SEP          { my_fit->fit(false, $2); }
    | F_CONTINUE SEP               { my_fit->fit(false, -1); }
    | F_METHOD SEP     { imsg (fitMethodsContainer->print_current_method ()); }
    | F_METHOD LOWERCASE SEP       { fitMethodsContainer->change_method ($2); }
    | F_INFO SEP                   { imsg (my_fit->info(0)); }
    | F_INFO '*' SEP               { imsg (my_fit->info(1)); }
    | F_INFO '*' '*' SEP           { imsg (my_fit->info(2)); }
    | S_ADD fzg_type pags_vec SEP  { my_sum->add_fzg ($2.fzg, $2.c, pgvec); }
    | S_ADD a_num SEP              {/*nothing, @ is added inside a_num*/}
    | S_CHANGE a_num flt opt_proc SEP  { my_sum->change_a ($2, $3, $4); }
    | S_CHANGE a_num flt opt_proc domain SEP { my_sum->change_a ($2, $3, $4); 
                                       my_sum->change_domain ($2, Domain($5)); }
    | S_CHANGE F_NUM F_TYPE SEP    { my_sum->change_f ($2, $3); }
    | S_CHANGE F_NUM '[' uints ']' pags_vec SEP
			           { my_sum->change_in_f ($2, ivec, pgvec); }
    | S_FREEZE SEP                 { imsg (my_sum->frozen_info ()); }
    | S_FREEZE a_num SEP           { my_sum->freeze ($2, true); }
    | S_FREEZE '!' a_num SEP       { my_sum->freeze ($3, false); }
    | S_HISTORY SEP                { imsg (my_sum->print_history()); }
    | S_HISTORY INt SEP           { my_sum->move_in_history ($2, true); }
    | S_HISTORY UINt SEP          { my_sum->move_in_history ($2, false); }
    | S_HISTORY '*' opt_uint SEP  { my_sum->toggle_history_item_saved ($3); }
    | S_HISTORY uint_slashes SEP   { imsg (my_sum->history_diff (ivec)); }
    | S_INFO a_num SEP             { imsg (my_sum->info_a ($2)); }
    | S_INFO fzg_num SEP           { imsg (my_sum->info_fzg ($2.fzg, $2.i)); }
    | S_INFO '$' SEP               { imsg (V_fzg::print_type_info (gType, 0)); }
    | S_INFO '^' SEP               { imsg (V_fzg::print_type_info (fType, 0)); }
    | S_INFO '<' SEP               { imsg (V_fzg::print_type_info (zType, 0)); }
    | S_INFO fzg_type SEP       { imsg (V_fzg::print_type_info ($2.fzg, $2.c));}
    | S_INFO SEP                   { imsg (my_sum->general_info()); }
    | S_REMOVE a_num SEP           { my_sum->rm_a ($2); }
    | S_REMOVE fzg_num SEP         { my_sum->rm_fzg ($2.fzg, $2.i); }
    | S_REMOVE '*' '*' SEP         { my_sum->rm_all(); }
    | S_VALUE flt opt_asterix SEP { imsg (my_sum->print_sum_value ($2, $3)); }
    | S_VALUE fzg_num flt opt_asterix SEP 
		       { imsg (my_sum->print_fzg_value($2.fzg, $2.i, $3, $4)); }
    | S_VALUE G_NUM SEP            {imsg (my_sum->print_fzg_value (gType, $2));}
    | S_VALUE a_num SEP            { imsg (my_sum->info_a ($2)); }
    | S_EXPORT peaks_to_sum opt_lcase FILENAME opt_plus SEP { 
                             my_sum->export_to_file ($4.str(), $5, $3, ivec); }
    | M_FINDPEAK flt opt_flt SEP { 
                            imsg (my_manipul->print_simple_estimate ($2, $3)); }
    | M_FINDPEAK                   {imsg (my_manipul->print_global_peakfind());}
    | O_PLOT SEP                   { my_IO->plot_now ();}
    | O_PLOT range SEP          { my_core->set_view (Rect($2.l, $2.r), true); }
    | O_PLOT bracket_range bracket_range SEP  { 
                            my_core->set_view (Rect($2.l, $2.r, $3.l, $3.r)); }
    | O_PLOT '.' range SEP         { my_core->set_view_v ($3.l, $3.r); }
    | O_PLOT range '.' SEP         { my_core->set_view_h ($2.l, $2.r); }
    | O_PLOT '.' SEP               { imsg (my_core->view_info()); } 
    | O_PLOT '.' '.' SEP           { imsg (my_core->view_info()); } 
    | O_PLOT sign SEP              { my_core->set_plus_background($2 == '+'); }
    | O_LOG opt_lcase FILENAME SEP {
			      my_other->start_logging_to_file ($3.str(), $2); }
    | O_LOG '!' SEP                { my_other->stop_logging_to_file (); }
    | O_LOG                        { imsg (my_other->logging_info()); }
    | O_INCLUDE FILENAME rows SEP  { my_other->include_file ($2.str(), ivec);}
    | O_INCLUDE '!' FILENAME rows SEP { AL->reset_all(); 
                                       my_other->include_file ($3.str(), ivec);}
    | O_INCLUDE '!' SEP            { AL->reset_all(); }
    | O_WAIT UINt SEP             { my_other->sleep ($2); }
    | O_DUMP FILENAME SEP          { AL->dump_all_as_script ($2.str()); }
    | QUIT SEP                     { YYABORT;}
    /***/
    | C_WAVELENGTH pags_vec SEP    { my_crystal->xrays.add (pgvec); }
    | C_WAVELENGTH '!' SEP         { my_crystal->xrays.clear(); }
    | C_WAVELENGTH SEP             { imsg (my_crystal->wavelength_info()); }
    | C_ADD PH_TYPE pags_vec SEP   { my_crystal->add_phase ($2, pgvec); }
    | C_ADD p_plane SEP            { my_crystal->add_plane ($2.ph, $2.hkl); }
    | C_ADD p_plane f_num_vec SEP  { 
                          my_crystal->add_plane_as_f ($2.ph, $2.hkl, ivec); }
    | C_INFO '%' SEP               { imsg (my_crystal->phase_type_info()); }
    | C_INFO PH_TYPE SEP           { imsg (my_crystal->phase_type_info()); }
    | C_INFO PH_NUM SEP            { imsg (my_crystal->phase_info ($2) + "\n"
    				     + my_crystal->list_planes_in_phase ($2)); }
    | C_INFO p_plane SEP      { imsg (my_crystal->plane_info ($2.ph, $2.hkl)); }
    | C_INFO SEP                   { imsg (my_crystal->wavelength_info() + "\n"
                                            + my_crystal->phase_info (-1)); }
    | C_REMOVE PH_NUM SEP          { my_crystal->rm_phase ($2); }
    | C_REMOVE p_plane SEP         { my_crystal->rm_plane ($2.ph, $2.hkl); }
    | C_FIND p_plane opt_flt SEP  { 
                       imsg (my_crystal->print_estimate ($2.ph, $2.hkl, $3)); }
    /***/
    ;

a_num: A_NUM                     { $$ = $1; }
    |  NEW_A                     { $$ = my_sum->add_a ($1, Domain()) .a(); } 
    |  NEW_A domain              { $$ = my_sum->add_a ($1, Domain($2)) .a(); } 
    ;

domain: '[' ']'             { $$.set = $$.ctr_set = false; }
    | '[' flt ':' flt ']' { $$.set = true; $$.ctr_set = true; 
                              $$.ctr = ($2 + $4) / 2; $$.sigma = ($4 - $2) / 2;}
    | '[' PLUS_MINUS flt ']' { $$.set=true; $$.ctr_set=false; $$.sigma = $3; }
    | '[' flt PLUS_MINUS flt ']' { $$.set = true; $$.ctr_set = true; 
				     $$.ctr = $2; $$.sigma = $4; }
    | '[' flt PLUS_MINUS flt '%' ']' { $$.set = true; $$.ctr_set = true; 
					 $$.ctr = $2; $$.sigma = $4 * $2 / 100;}
    ;

opt_proc: /*empty*/         { $$ = '='; }  
    |     '%'               { $$ = '%'; }  
    ;

sim_range:  ':'     { $$.l = -INF; $$.r = +INF; }
    | flt ':'      { $$.l = $1; $$.r = +INF; }
    | ':' flt      { $$.l = -INF; $$.r = $2; }
    ;

bracket_range: '[' ']'      { $$.l = -INF; $$.r = +INF; }
    | '[' sim_range ']'     { $$ = $2 }
    | flt ':' flt         { $$.l = $1; $$.r = $3; }
    | '[' flt ':' flt ']' { $$.l = $2; $$.r = $4; }
    ;

range: sim_range      { $$ = $1; }
     | bracket_range  { $$ = $1; }
     ;

p_plane:  PH_NUM plane {$$.ph = $1; $$.hkl=$2; }
    |     plane        {$$.ph = 0;  $$.hkl=$1; }
    ;

plane:  '(' UINt ')'  { if ($2 < 1 /*1==001*/ || $2 > 999) { 
                          $$.h = $$.k =$$.l = 0;
                          warn("Please separate Miller's indices with spaces.");
			 }
                           else $$.h = $2/100, $$.k=$2%100/10, $$.l=$2%10;   
		       }
    | '(' inr inr inr ')' { $$.h = $2; $$.k = $3; $$.l = $4; }
    ;

pag:  a_num            { $$.c = 'a'; $$.n = $1; }
    | G_NUM            { $$.c = 'g'; $$.n = $1; }
    | P_NUM            { $$.c = 'p'; $$.p = $1; }
    ;

fzg_type: F_TYPE       { $$.fzg = fType; $$.c = $1; }
    |     Z_TYPE       { $$.fzg = zType; $$.c = $1; }
    |     G_TYPE       { $$.fzg = gType; $$.c = $1; }
    ;

fzg_num:  F_NUM        { $$.fzg = fType; $$.i = $1; }
    |     Z_NUM        { $$.fzg = zType; $$.i = $1; }
    |     G_NUM        { $$.fzg = gType; $$.i = $1; }
    ;

pags_vec: pag          { pgvec = vector1 (Pag($1)); }
    | pags_vec pag     { pgvec.push_back (Pag($2)); }
    ;

f_num_vec: F_NUM       { ivec = vector1($1); }
    | f_num_vec F_NUM  { ivec.push_back($2); }
    ;

inr: INt  
    | UINt { $$ = $1; }
    ;

opt_uint: /*empty*/   { $$ = 0; }
    |  UINt           { $$ = $1; }
    ;

opt_uint_1: /*empty*/ { $$ = -1; }
    |  UINt           { $$ = $1; }
    ;

uints: UINt          { ivec = vector1($1); }
    | uints UINt     { ivec.push_back($2); }
    ;

flt: FLOAt  
    | inr { $$ = (double) $1; }
    ;

opt_flt: /*empty*/      { $$ = 0; }
    |          flt      { $$ = $1 }
    ;

opt_lcase: /*empty*/     { $$ = 0; }
    |      LOWERCASE     { $$ = $1; }
    ;

opt_asterix: /*empty*/   { $$ = false; }
    |       '*'          { $$ = true;  }
    ;
opt_plus: /*empty*/      { $$ = false; }
    |       '+'          { $$ = true;  }
    ;

peaks_to_sum:  /*empty*/      { ivec.clear(); }
    | F_NUM                   { ivec = vector1($1); }
    | PH_NUM                  { ivec = my_crystal->get_funcs_in_phase($1); }
    | p_plane         { ivec = my_crystal->get_funcs_in_plane ($1.ph, $1.hkl); }
    | peaks_to_sum '+' F_NUM  { ivec.push_back($3); }
    | peaks_to_sum '+' PH_NUM { vector<int>k=my_crystal->get_funcs_in_phase($3);
				ivec.insert (ivec.end(), k.begin(), k.end()); }
    | peaks_to_sum '+' p_plane { 
                 vector<int> k = my_crystal->get_funcs_in_plane ($3.ph, $3.hkl);
                                ivec.insert (ivec.end(), k.begin(), k.end()); }
    ;

columns: /*empty*/      { ivec.clear(); }
   |  UINt ':' UINt   { ivec = vector2 ($1, $3); }
   |  UINt ':' UINt ':' UINt  { ivec = vector3 ($1, $3, $5); }
   ;

rows_of: /*empty*/      { ivec2.clear(); }
   |  UI_DASH UINt     { ivec2 = vector2($1, $2); }
   |  UI_DASH UI_SLASH UINt { ivec2 = vector3 ($1, $2, $3); }
   |  UI_SLASH UINt    { ivec2 = vector3($1, $1, $2); }
   ;

dl_merge: /*empty*/     { $$ = 0; }
   |  '*' UINt         { $$ = -$2; }
   |  '+' '*' UINt     { $$ = $3; }
   ;

dload_arg: columns rows_of dl_merge  { $$ = $3; }
   ;

rows: /*empty*/           { ivec.clear(); }
   |  rows UI_DASH UINt  { ivec.push_back($2); ivec.push_back($3); }
   |  rows UINt          { ivec.push_back($2); ivec.push_back($2); }
   ;

sign: '+'  { $$ = '+'; } 
   |  '-'  { $$ = '-'; }
   ;

uint_slashes_: UI_SLASH               { ivec = vector1 ($1); }
   |           uint_slashes UI_SLASH  { ivec.push_back ($2); }
   ;

uint_slashes: uint_slashes_           { /*nothing*/;         }
   |          uint_slashes_ UINt     { ivec.push_back ($2); }
   |          '/'                     { ivec.clear();        }
   ;

%%

int iperror (char * /*s*/) { return 0; }

void start_of_string_parsing(const char *s);
void end_of_string_parsing();

bool parser (std::string cmd, bool from_file)
{
    if (!from_file)
        my_other->log_input (cmd);
    cmd = " " + cmd + "\n";
    start_of_string_parsing (cmd.c_str());
    int result = ipparse();
    end_of_string_parsing();
    return result == 0 ? true : false;
}

