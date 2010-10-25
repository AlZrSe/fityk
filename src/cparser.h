// This file is part of fityk program. Copyright 2009 Marcin Wojdyr.
// Licence: GNU General Public License ver. 2+
// $Id: $

/// This parser is not used yet.
/// In the future it will replace the current parser (cmd* files)
/// Command parser.

#ifndef FITYK_CPARSER_H_
#define FITYK_CPARSER_H_

#include <string>
#include "ui.h"
#include "lexer.h"

struct Statement;
class StatementList;
class DataAndModel;

class Parser
{
public:
    Parser(Ftk* F);
    ~Parser();

    // Parses the string. Throws SyntaxError.
    void parse(const std::string& str);

    // Execute the last parsed string.
    // Throws ExecuteError, ExitRequestedException.
    void execute();

    // Calls parse() and execute(), catches exceptions and returns status.
    Commands::Status parse_and_execute(const std::string& str);

    // The same as parse(), but it doesn't throw. Returns true on success.
    bool check_command_syntax(const std::string& str);

    // for debugging only
    std::string get_statements_repr() const;

private:
    Ftk* F_;
    std::string str_;
    bool ok_;
    StatementList *sts_;

    void parse_set_args(Lexer& lex, std::vector<Token>& args);

    void execute_command_set(const std::vector<Token>& args);
    void execute_command_define(const std::vector<Token>& args);
    void execute_command_delete(const std::vector<Token>& args);
    void execute_command_delete_points(const Statement& st);
    void execute_command_exec(const std::vector<Token>& args);
    void execute_command_fit(const std::vector<Token>& args);
    void execute_command_guess(const std::vector<Token>& args);
    void execute_command_info(const std::vector<Token>& args);
    void execute_command_plot(const std::vector<Token>& args);
    void execute_command_undefine(const std::vector<Token>& args);
    void execute_command_load(const std::vector<Token>& args);
    void execute_command_dataset_tr(const std::vector<Token>& args);
    void execute_command_name_func(const std::vector<Token>& args);

    std::vector<DataAndModel*>
        get_datasets_from_indata(const std::vector<int>& ds);
    std::vector<int> expand_dataset_indices(const std::vector<int>& ds);
};

#endif //FITYK_CPARSER_H_
