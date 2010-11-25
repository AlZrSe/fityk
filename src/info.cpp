// This file is part of fityk program. Copyright 2009 Marcin Wojdyr.
// Licence: GNU General Public License ver. 2+
// $Id: $

/// Functions to execute commands: info, debug, print.

#include "info.h"
#include <string>
#include <vector>
#include <ctype.h>
#include <string.h>
//#include <iostream>

#include <xylib/xylib.h> //get_version()
#include <boost/version.hpp> // BOOST_VERSION

#include "logic.h"
#include "func.h"
#include "data.h"
#include "fit.h"
#include "ast.h"
#include "model.h"
#include "guess.h"
#include "cparser.h"
#include "eparser.h"
#include "lexer.h"
#include "ui.h"
#include "runner.h" // args2range

using namespace std;

string info_compiler()
{
    return
        "Build system type: "
#ifdef CONFIGURE_BUILD
        CONFIGURE_BUILD
#else
        "UNKNOWN"
#endif

        "\nConfigured with: "
#ifdef CONFIGURE_ARGS
        CONFIGURE_ARGS
#else
        "UNKNOWN"
#endif

        "\nCompiler: "
#if defined(__GNUC__)
        "GCC"
#elif defined(_MSC_VER)
        "MS VC++"
#else
        "UNKNOWN"
#endif

        "\nCompiler version: "
#ifdef __VERSION__
        __VERSION__
#else
        "UNKNOWN"
#endif

        "\nCompilation date: " __DATE__
        "\nBoost version: " + S(BOOST_VERSION / 100000)
                      + "." + S(BOOST_VERSION / 100 % 1000)
                      + "." + S(BOOST_VERSION % 100)
        + "\nxylib version: " + xylib_get_version();
}

void info_types(string& result)
{
    result += "Defined function types:";
    vector<string> const& tt = Function::get_all_types();
    vector_foreach (string, i, tt)
        result += "\n" + Function::get_formula(*i);
}


void info_functions(const Ftk* F, const string& name, string& result)
{
    if (!contains_element(name, '*')) {
        const Function *f = F->find_function(name);
        result += f->get_basic_assignment();
    }
    else {
        vector_foreach (Function*, i, F->functions())
            if (match_glob((*i)->name.c_str(), name.c_str()))
                result += (result.empty() ? "" : "\n")
                          + (*i)->get_basic_assignment();
    }
}

void info_variables(const Ftk* F, const string& name, string& result)
{
    if (!contains_element(name, '*')) {
        const Variable* var = F->find_variable(name);
        result += F->get_variable_info(var);
    }
    else {
        vector_foreach (Variable*, i, F->variables())
            if (match_glob((*i)->name.c_str(), name.c_str()))
                result += (result.empty() ? "" : "\n")
                          + F->get_variable_info(*i);
    }
}

void info_func_type(const string& functype, string& result)
{
    string m = Function::get_formula(functype);
    if (m.empty())
        result += "undefined";
    else {
        result += m;
        if (m.find(" where ") != string::npos)
            result += "\n = " + Function::get_rhs_from_formula(m);
    }
}

void info_history(const Ftk* F, const Token& t1, const Token& t2,
                  string& result)
{
    const vector<Commands::Cmd>& cmds = F->get_ui()->get_commands().get_cmds();
    int from = 0, to = cmds.size();
    if (t1.type == kTokenExpr) {
        from = iround(t1.value.d);
        if (from < 0)
            from += cmds.size();
    }
    if (t2.type == kTokenExpr) {
        to = iround(t2.value.d);
        if (to < 0)
            to += cmds.size();
    }
    if (from < 0 || to > (int) cmds.size())
        throw ExecuteError("wrong history range");
    for (int i = from; i < to; ++i)
        result += cmds[i].str() + "\n";
}

/*
void Ftk::dump_all_as_script(string const &filename)
{
    FILE* f = fopen(filename.c_str(), "w");
    if (!f) {
        warn ("Can't open file: " + filename);
        return;
    }
    fprintf(f, "%s\n", fityk_version_line);
    fprintf(f, "## dumped at: %s\n", time_now().c_str());
    fprintf(f, "set verbosity = quiet #the rest of the file is not shown\n");
    fprintf(f, "set autoplot = never\n");
    fprintf(f, "reset\n");
    fprintf(f, "# ------------  settings  ------------\n");
    fprintf(f, "%s\n", get_settings()->set_script().c_str());
    fprintf(f, "# ------------  variables and functions  ------------\n");
    // We define here also auto-removed variables and functions,
    // so the script can't trigger VariableManager::remove_unreferred()
    // nor VariableManager::auto_remove_functions() until all references
    // are reproduced.
    vector_foreach (Variable*, i, variables_)
        fprintf(f, "%s = %s\n", (*i)->xname.c_str(),
                                (*i)->get_formula(parameters_).c_str());
    fprintf(f, "\n");
    vector<UdfContainer::UDF> const& udfs = UdfContainer::get_udfs();
    vector_foreach (UdfContainer::UDF, i, udfs)
        if (!i->builtin)
            fprintf(f, "define %s\n", i->formula.c_str());
    fprintf(f, "\n");
    vector_foreach (Function*, i, functions_) {
        if ((*i)->has_outdated_type()) {
            string new_formula = Function::get_formula((*i)->type_name);
            if (!new_formula.empty())
                fprintf(f, "undefine %s\n", (*i)->type_name.c_str());
            fprintf(f, "define %s\n", (*i)->type_formula.c_str());
            fprintf(f, "%s\n", (*i)->get_basic_assignment().c_str());
            fprintf(f, "undefine %s\n", (*i)->type_name.c_str());
            if (!new_formula.empty())
                fprintf(f, "define %s\n", new_formula.c_str());
        }
        else
            fprintf(f, "%s\n", (*i)->get_basic_assignment().c_str());
    }
    fprintf(f, "\n");
    fprintf(f, "# ------------  datasets and models  ------------\n");
    for (int i = 0; i != get_dm_count(); ++i) {
        Data const* data = get_data(i);
        if (i != 0)
            fprintf(f, "@+ = 0\n");
        if (!data->get_title().empty())
            fprintf(f, "set @%d.title = '%s'\n", i, data->get_title().c_str());
        int m = data->points().size();
        fprintf(f, "M=%d in @%d\n", m, i);
        fprintf(f, "X=%.12g in @%d # =max(x), prevents sorting.\n",
                   data->get_x_max(), i);
        for (int j = 0; j != m; ++j) {
            Point const& p = data->points()[j];
            fprintf(f, "X[%d]=%.12g, Y[%d]=%.12g, S[%d]=%g, A[%d]=%d in @%d\n",
                       j, p.x, j, p.y, j, p.sigma, j, (int) p.is_active, i);
        }
        fprintf(f, "\n");
        Model const* model = get_model(i);
        vector<string> const& ff = model->get_ff().names;
        if (!ff.empty()) {
            fprintf(f, "@%d.F = %%%s", i, ff[0].c_str());
            for (size_t j = 1; j < ff.size(); ++j)
                fprintf(f, " + %%%s", ff[j].c_str());
            fprintf(f, "\n");
        }
        vector<string> const& zz = model->get_zz().names;
        if (!zz.empty()) {
            fprintf(f, "@%d.Z = %%%s", i, zz[0].c_str());
            for (size_t j = 1; j < zz.size(); ++j)
                fprintf(f, " + %%%s", zz[j].c_str());
            fprintf(f, "\n");
        }
        fprintf(f, "\n");
    }
    fprintf(f, "plot %s\n", view.str().c_str());
    // TODO set default dataset "use %s"
    fprintf(f, "set autoplot = %s\n", get_settings()->getp("autoplot").c_str());
    fprintf(f, "set verbosity = %s\n",
               get_settings()->getp("verbosity").c_str());
    fclose(f);
}
*/

int eval_one_info_arg(const Ftk* F, int ds, const vector<Token>& args, int n,
                      string& result)
{
    int ret = 0;
    if (args[n].type == kTokenLname) {
        const string word = args[n].as_string();

        // no args
        if (word == "version")
            result += "Fityk " VERSION;
        else if (word == "compiler")
            result += info_compiler();
        else if (word == "variables")
            for (size_t i = 0; i < F->variables().size(); ++i)
                result += (i > 0 ? " " : "") + F->get_variable(n)->xname;
        else if (word == "types")
            result += join_vector(Function::get_all_types(), " ");
        else if (word == "types_full")
            info_types(result);
        else if (word == "functions")
            for (size_t i = 0; i < F->functions().size(); ++i)
                result += (i > 0 ? " " : "") + F->get_function(n)->xname;
        else if (word == "dataset_count")
            result += S(F->get_dm_count());
        else if (word == "datasets") {
            for (int i = 0; i < F->get_dm_count(); ++i)
                result += (i > 0 ? "\n@" : "@") + S(i) + ": "
                        + F->get_data(i)->get_title();
        }
        else if (word == "view")
            result += F->view.str();
        else if (word == "set")
            result += F->get_settings()->print_usage();
        else if (word == "fit_history")
            result += F->get_fit_container()->param_history_info();
        else if (word == "filename") {
            result += F->get_data(ds)->get_filename();
        }
        else if (word == "title") {
            result += F->get_data(ds)->get_title();
        }
        else if (word == "data") {
            result += F->get_data(ds)->get_info();
        }
        else if (word == "formula") {
            bool gnuplot =
                F->get_settings()->get_e("formula_export_style") == 1;
            result += F->get_model(ds)->get_formula(false, gnuplot);
        }
        else if (word == "simplified_formula") {
            bool gnuplot =
                F->get_settings()->get_e("formula_export_style") == 1;
            result += F->get_model(ds)->get_formula(true, gnuplot);
        }
        else if (word == "state") {
            //TODO F->save_state(result);
            //F->dump_all_as_script
        }
        else if (word == "peaks") {
            vector<fp> errors;
            result += F->get_model(ds)->get_peak_parameters(errors);
        }
        else if (word == "peaks_err") {
            //FIXME: assumes the dataset was fitted separately
            DataAndModel* dm = const_cast<DataAndModel*>(F->get_dm(ds));
            vector<DataAndModel*> dms(1, dm);
            vector<fp> errors = F->get_fit()->get_standard_errors(dms);
            result += F->get_model(ds)->get_peak_parameters(errors);
        }
        else if (word == "history_summary")
            result += F->get_ui()->get_commands().get_history_summary();

        // optional range
        else if (word == "history") {
            info_history(F, args[n+1], args[n+2], result);
            ret += 2;
        }
        else if (word == "guess") {
            RealRange range = args2range(args[n], args[n+1]);
            if (range.from >= range.to)
                result += "invalid range";
            else {
                int lb = F->get_data(ds)->get_lower_bound_ac(range.from);
                int rb = F->get_data(ds)->get_upper_bound_ac(range.to);
                Guess g(F->get_settings());
                g.initialize(F->get_dm(ds), lb, rb, -1);
                g.get_guess_info(result);
            }
            ret += 2;
        }

        // optionally takes datasets as args
        else if (word == "fit" || word == "errors" || word == "cov") {
            vector<DataAndModel*> v;
            ++n;
            while (args[n].type == kTokenDataset) {
                int k = args[n].value.i;
                DataAndModel* dm = const_cast<DataAndModel*>(F->get_dm(k));
                v.push_back(dm);
                ++n;
                ++ret;
            }
            if (v.empty()) {
                DataAndModel* dm = const_cast<DataAndModel*>(F->get_dm(ds));
                v.push_back(dm);
            }
            if (word == "fit")
                result += F->get_fit()->get_goodness_info(v);
            else if (word == "errors")
                result += F->get_fit()->get_error_info(v);
            else //if (word == "cov")
                result += F->get_fit()->get_cov_info(v);
        }

        // one arg: $var
        else if (word == "refs") {
            string name = Lexer::get_string(args[n]);
            vector<string> refs = F->get_variable_references(name);
            result += join_vector(refs, ", ");
            ++ret;
        }

        // one arg: %func
        else if (word == "par") {
            string name = Lexer::get_string(args[n]);
            result += F->find_function(name)->get_par_info(F);
            ++ret;
        }
    }

    // FuncType
    else if (args[n].type == kTokenCname)
        info_func_type(args[n].as_string(), result);

    // %func
    else if (args[n].type == kTokenFuncname)
        info_functions(F, Lexer::get_string(args[n]), result);

    // $var
    else if (args[n].type == kTokenVarname)
        info_variables(F, Lexer::get_string(args[n]), result);

    // handle [@n.]F/Z['['expr']']
    else if ((args[n].type == kTokenUletter &&
                             (*args[n].str == 'F' || *args[n].str == 'Z'))
             || args[n].type == kTokenDataset) {
        int k = ds;
        if (args[n].type == kTokenDataset) {
            k = args[n].value.i;
            ++ret;
        }
        const Model* model = F->get_model(k);
        char fz = *args[n].str;
        if (is_index(n+1, args) && args[n+1].type == kTokenExpr) {
            ++ret;
            int idx = iround(args[n].value.d);
            const string& name = model->get_func_name(fz, idx);
            const Function *f = F->find_function(name);
            result += f->get_basic_assignment();
        }
        else {
            result += join_vector(model->get_fz(fz).names, ", ");
        }
    }
    ++ret;
    return ret;
}

int eval_info_args(const Ftk* F, int ds, const vector<Token>& args,
                   string& result)
{
    int len = args.size();
    if (len > 2 && (args[len-2].type == kTokenGT ||
                    args[len-2].type == kTokenAppend))
        len -= 2;
    int n = 0;
    while (n < len) {
        if (!result.empty())
            result += "\n";
        n += eval_one_info_arg(F, ds, args, n, result);
    }
    return n;
}

void eval_one_print_arg(const Ftk* F, int ds, const Token& t, string& result)
{
    if (t.type == kTokenString)
        result += Lexer::get_string(t);
    else if (t.type == kTokenExpr)
        result += F->get_settings()->format_double(t.value.d);
    else if (t.as_string() == "filename")
        result += F->get_data(ds)->get_filename();
    else if (t.as_string() == "title")
        result += F->get_data(ds)->get_title();
    else
        assert(0);
}

int eval_print_args(const Ftk* F, int ds, const vector<Token>& args,
                    string& result)
{
    // args: condition (expr|string|"filename"|"title")+
    int len = args.size();
    if (len > 2 && (args[len-2].type == kTokenGT ||
                    args[len-2].type == kTokenAppend))
        len -= 2;
    string sep = " ";
    if (args[0].type == kTokenNop) {
        for (int n = 1; n < len; ++n) {
            if (n != 1)
                result += sep;
            eval_one_print_arg(F, ds, args[n], result);
        }
    }
    else {
        vector<ExpressionParser> expr_parsers(args.size() + 1, F);
        for (int i = 0; i < len; ++i)
            if (args[i].type == kTokenExpr) {
                Lexer lex(args[i].str);
                expr_parsers[i].parse_expr(lex, ds);
            }
        const vector<Point>& points = F->get_data(ds)->points();
        for (int k = 0; k != (int) points.size(); ++k) {
            if (args[0].type == kTokenExpr) {
                double cond = expr_parsers[0].calculate(k, points);
                if (fabs(cond) < 0.5)
                    continue;
            }
            if (!result.empty())
                result += "\n";
            for (int n = 1; n < len; ++n) {
                if (n != 1)
                    result += sep;
                if (args[n].type == kTokenExpr) {
                    double value = expr_parsers[n].calculate(k, points);
                    result += F->get_settings()->format_double(value);
                }
                else
                    eval_one_print_arg(F, ds, args[n], result);
            }
        }
    }
    return len;
}

string get_info_string(Ftk const* F, string const& args)
{
    Lexer lex(args.c_str());
    Parser cp(const_cast<Ftk*>(F));
    vector<Token> tt;
    cp.parse_info_args(lex, tt);
    if (lex.peek_token().type != kTokenNop)
        lex.throw_syntax_error("unexpected token");
    string result;
    eval_info_args(F, -1, tt, result);
    return result;
}

void run_info(const Ftk* F, int ds,
              CommandType cmd, const std::vector<Token>& args)
{
    string info;
    int n;
    if (cmd == kCmdInfo)
        n = eval_info_args(F, ds, args, info);
    else // cmd == kCmdPrint
        n = eval_print_args(F, ds, args, info);
    if (n == (int) args.size()) { // no redirection to file
        int max_screen_info_length = 2048;
        int more = (int) info.length() - max_screen_info_length;
        if (more > 0) {
            info.resize(max_screen_info_length);
            info += "\n[... " + S(more) + " characters more...]";
        }
        F->rmsg(info);
    }
    else {
        assert(n == (int) args.size() - 2);
        assert(args[n].type == kTokenGT || args[n].type == kTokenAppend);
        assert(args[n+1].type == kTokenFilename);
        ios::openmode mode = (args[n].type == kTokenGT ? ios::trunc : ios::app);
        string filename = args[n+1].as_string();
        ofstream os(filename.c_str(), ios::out | mode);
        if (!os)
            throw ExecuteError("Can't open file: " + filename);
        os << info << endl;
    }
}

void run_debug(const Ftk* F, int ds, const Token& key, const Token& rest)
{
    // args: any-token rest-of-line
    string r;
    string word = key.as_string();

    if (word == "parse") {
        Parser parser(const_cast<Ftk*>(F));
        try {
            Lexer lex(rest.str);
            while (parser.parse_statement(lex))
                r += parser.get_statements_repr();
        }
        catch (fityk::SyntaxError& e) {
            r += string("ERR: ") + e.what();
        }
    }

    else if (word == "lex") {
        Lexer lex(rest.str);
        for (Token t = lex.get_token(); t.type != kTokenNop; t =lex.get_token())
            r += token2str(t) + "\n";
    }

    else if (word == "expr") {
        Lexer lex(rest.str);
        try {
            ExpressionParser parser(F);
            parser.parse_expr(lex, -1);
            r += parser.list_ops();
        }
        catch (fityk::SyntaxError& e) {
            r += "ERROR at " + S(lex.scanned_chars()) + ": " + e.what();
        }
        if (lex.peek_token().type != kTokenNop)
            r += "\nnext token: " + token2str(lex.peek_token());
    }

    else if (word == "der") {
        get_derivatives_str(rest.str, r);
    }

    // show values of derivatives for all variables
    else if (word == "rd") {
        for (int i = 0; i < size(F->variables()); ++i) {
            Variable const* var = F->get_variable(i);
            r += var->xname + ": ";
            vector_foreach (Variable::ParMult, i, var->recursive_derivatives())
                r += "p" + S(i->p) + "="
                    + F->find_variable_handling_param(i->p)->xname
                    + " *" + S(i->mult) + "    ";
            r += "\n";
        }
    }

    // show varnames and var_idx from VariableUser
    else if (word == "idx") {
        for (size_t i = 0; i != F->functions().size(); ++i)
            r += S(i) + ": " + F->get_function(i)->get_debug_idx_info() + "\n";
        for (size_t i = 0; i != F->variables().size(); ++i)
            r += S(i) + ": " + F->get_variable(i)->get_debug_idx_info() + "\n";
    }

    // compares numeric and symbolic derivatives
    else if (word == "df") {
        Lexer lex(rest.str);
        ExpressionParser ep(F);
        ep.parse_expr(lex, ds);
        double x = ep.calculate();
        Model const* model = F->get_model(ds);
        vector<fp> symb = model->get_symbolic_derivatives(x);
        vector<fp> num = model->get_numeric_derivatives(x, 1e-4);
        assert (symb.size() == num.size());
        r += "F(" + S(x) + ")=" + S(model->value(x));
        for (int i = 0; i < size(num); ++i) {
            if (is_neq(symb[i], 0) || is_neq(num[i], 0))
                r += "\ndF / d " + F->find_variable_handling_param(i)->xname
                    + " = (symb.) " + S(symb[i]) + " = (num.) " + S(num[i]);
        }
    }

    // show %function's bytecode
    else if (key.type == kTokenFuncname) {
        Function const* f = F->find_function(Lexer::get_string(key));
        r += f->get_bytecode();
    }

    // show derivatives of $variable
    else if (key.type == kTokenVarname) {
        Variable const* v = F->find_variable(Lexer::get_string(key));
        vector<string> vn = concat_pairs("$", v->get_varnames());
        for (int i = 0; i < v->get_vars_count(); ++i) {
            string formula = v->get_op_trees()[i]->str(&vn);
            double value = v->get_derivative(i);
            if (i != 0)
                r += "\n";
            r += "d(" + v->xname + ")/d($" + v->get_var_name(i) + "): "
              + formula + " == " + F->get_settings()->format_double(value);
        }
    }

    // tests the match_glob()
    else if (word == "glob") {
        Lexer lex(rest.str);
        string pattern = lex.get_filename_token().as_string();
        Token t;
        while ((t = lex.get_filename_token()).type != kTokenNop) {
            string s = t.as_string();
            if (match_glob(s.c_str(), pattern.c_str()))
                r += s + " ";
        }
    }

    else
        r += "unexpected arg: " + word;
    F->rmsg(r);
}

