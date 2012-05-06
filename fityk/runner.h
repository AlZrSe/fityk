// This file is part of fityk program. Copyright 2009 Marcin Wojdyr.
// Licence: GNU General Public License ver. 2+

#ifndef FITYK_RUNNER_H_
#define FITYK_RUNNER_H_

#include <vector>
#include "lexer.h" // Token
#include "eparser.h"
#include "view.h" //RealRange

namespace fityk {

class Ftk;
struct Statement;
struct Command;
class DataAndModel;

RealRange args2range(const Token& t1, const Token& t2);

class Runner
{
public:
    Runner(Ftk* F) : F_(F), ep_(F) {}

    // Execute the last parsed string.
    // Throws ExecuteError, ExitRequestedException.
    // The statement is not const, because expressions in it can be re-parsed 
    // when executing for multiple datasets.
    void execute_statement(Statement& st);

private:
    Ftk* F_;
    std::vector<VMData>* vdlist_;
    ExpressionParser ep_;

    void execute_command(Command& c, int ds);
    void command_set(const std::vector<Token>& args);
    void command_delete(const std::vector<Token>& args);
    void command_delete_points(const std::vector<Token>& args, int ds);
    void command_exec(TokenType tt, const std::string& str);
    void command_fit(const std::vector<Token>& args, int ds);
    void command_guess(const std::vector<Token>& args, int ds);
    void command_plot(const std::vector<Token>& args, int ds);
    void command_undefine(const std::vector<Token>& args);
    void command_load(const std::vector<Token>& args);
    void command_dataset_tr(const std::vector<Token>& args);
    void command_name_func(const std::vector<Token>& args, int ds);
    void command_all_points_tr(const std::vector<Token>& args, int ds);
    void command_point_tr(const std::vector<Token>& args, int ds);
    void command_resize_p(const std::vector<Token>& args, int ds);
    void command_assign_param(const std::vector<Token>& args, int ds);
    void command_assign_all(const std::vector<Token>& args, int ds);
    void command_name_var(const std::vector<Token>& args);
    void command_change_model(const std::vector<Token>& args, int ds);
    void read_dms(std::vector<Token>::const_iterator first,
                  std::vector<Token>::const_iterator last,
                  std::vector<DataAndModel*>& dms);
    void recalculate_command(Command& c, int ds, Statement& st);
    int make_func_from_template(const std::string& name,
                                const std::vector<Token>& args, int pos);
    VMData* get_vm_from_token(const Token& t) const;
};

} // namespace fityk
#endif //FITYK_RUNNER_H_
