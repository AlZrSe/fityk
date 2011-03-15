// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+

#ifndef FITYK__API__H__
#define FITYK__API__H__

#ifndef __cplusplus
#error "This library does not have C API."
#endif

// C++ exception specifications are used by SWIG bindings
#ifdef _MSC_VER
// ignore warning "C++ exception specification ignored..."
#pragma warning( disable : 4290 )
#endif

#include <string>
#include <vector>
#include <cstdio>
#include <stdexcept>
class Ftk;


/// Public C++ API of libfityk: class Fityk and helpers.
///
/// Minimal examples of using libfityk in C++, Python, Lua and Perl are in
/// samples/hello.* files.
namespace fityk
{

/// exception thrown at run-time (when executing parsed command)
struct ExecuteError : public std::runtime_error
{
    ExecuteError(const std::string& msg) : runtime_error(msg) {}
};

/// syntax error exception
struct SyntaxError : public std::invalid_argument
{
    SyntaxError(const std::string& msg="") : invalid_argument(msg) {}
};


/// exception thrown to finish the program (on command "quit")
struct ExitRequestedException : std::exception
{
};

/// data point
struct Point
{
    double x, y, sigma;
    bool is_active;

    Point();
    Point(double x_, double y_);
    Point(double x_, double y_, double sigma_);
    std::string str() const;
};
inline bool operator< (Point const& p, Point const& q) { return p.x < q.x; }

/// type of function passed to Fityk::set_show_message()
typedef void t_show_message(std::string const& s);

/// used to get statistics for all datasets together, e.g. in Fityk::get_wssr()
const int all_datasets=-1;


/// the public API to libfityk
class Fityk
{
public:

    Fityk();
    ~Fityk();

    /// @name execute fityk commands or change data
    // @{

    /// execute command; throws exception on error
    void execute(std::string const& s) throw(SyntaxError, ExecuteError,
                                             ExitRequestedException);

    /// load data
    void load_data(int dataset,
                   std::vector<double> const& x,
                   std::vector<double> const& y,
                   std::vector<double> const& sigma,
                   std::string const& title="")  throw(ExecuteError);

    /// add one data point to dataset
    void add_point(double x, double y, double sigma, int dataset=0)
                                                     throw(ExecuteError);

    // @}

    /// @name (alternative to exceptions) handling of program errors
    // @{

    /// If set to false, does not throw exceptions. Default: true.
    void set_throws(bool state) { throws_ = state; }

    /// Return error handling mode: true if exceptions are thrown.
    bool get_throws() const { return throws_; }

    ///\brief Returns a string with last error or an empty string.
    /// Useful after calling set_throws(false). See also: clear_last_error().
    std::string const& last_error() const { return last_error_; }

    /// Clear last error message. See also: last_error().
    void clear_last_error() { last_error_.clear(); }

    // @}

    /// @name handling text output
    // @{

    /// general approach: set show message callback; cancels redir_messages()
    void set_show_message(t_show_message *func);

    /// redirect output to ...(e.g. stdout/stderr); cancels set_show_message()
    void redir_messages(std::FILE *stream);

    // @}


    /// @name get data and informations
    // @{

    /// return output of the info command
    std::string get_info(std::string const& s, int dataset=0)
                                            throw(SyntaxError, ExecuteError);

    /// return expression value, similarly to the print command
    double calculate_expr(std::string const& s, int dataset=0)
                                            throw(SyntaxError, ExecuteError);

    /// returns number of datasets n, always n >= 1
    int get_dataset_count();

    /// get data points
    std::vector<Point> const& get_data(int dataset=0)  throw(ExecuteError);

    /// returns the value of the model for a given dataset at x
    double get_model_value(double x, int dataset=0)  throw(ExecuteError);

    /// multiple point version of the get_model_value()
    std::vector<double>
    get_model_vector(std::vector<double> const& x, int dataset=0)
                                                        throw(ExecuteError);

    /// \brief returns the index of parameter hold by the variable
    /// (the same index as in get_covariance_matrix(),
    /// -1 for a compound-variable)
    int get_variable_nr(std::string const& name)  throw(ExecuteError);

    // @}

    /// @name get fit statistics
    // @{

    /// get WSSR for given dataset or for all datasets
    double get_wssr(int dataset=all_datasets)  throw(ExecuteError);

    /// get SSR for given dataset or for all datasets
    double get_ssr(int dataset=all_datasets)  throw(ExecuteError);

    /// get R-squared for given dataset or for all datasets
    double get_rsquared(int dataset=all_datasets)  throw(ExecuteError);

    /// get number of degrees-of-freedom for given dataset or for all datasets
    int get_dof(int dataset=all_datasets)  throw(ExecuteError);

    /// \brief get covariance matrix (for given dataset or for all datasets)
    /// get_variable_nr() can be used to connect variables with parameter
    /// positions
    std::vector<std::vector<double> >
    get_covariance_matrix(int dataset=all_datasets)  throw(ExecuteError);
    // @}

private:
    Ftk *ftk_;
    bool throws_;
    std::string last_error_;
};

} // namespace

#endif

