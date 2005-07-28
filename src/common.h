// This file is part of fityk program. Copyright (C) Marcin Wojdyr 
// $Id$

/*
 *  various headers and definitions. Included by all files.
 */
#ifndef COMMON__H__
#define COMMON__H__

#if HAVE_CONFIG_H   
#  include <config.h>  
#endif 

#ifndef VERSION
#   define VERSION "unknown"
#endif

#define USE_XTAL 1

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <math.h>
#include <assert.h>

/// favourite floating point type 
#ifdef FP_IS_LDOUBLE
typedef long double fp ;  
#else
typedef double fp ;  
#endif

extern const fp INF;


#ifndef M_PI
# define M_PI    3.1415926535897932384626433832795029  /* pi */
#endif
#ifndef M_LN2
# define M_LN2   0.6931471805599453094172321214581766  /* log_e 2 */
#endif

extern const std::vector<fp> fp_v0; //just empty vector
extern const std::vector<int> int_v0; //just empty vector

/** idea of exp_() is taken from gnuplot:
 *  some machines have trouble with exp(-x) for large x
 *  if MINEXP is defined at compile time, use exp_(x) instead,
 *  which returns 0 for exp_(x) with x < MINEXP
 */
#ifdef MINEXP
  inline fp exp_(fp x) { return (x < (MINEXP)) ? 0.0 : exp(x); }
#else
#  define exp_(x) exp(x)
#endif

extern const char* fityk_version_line; /// it is used to put version to script

/// S() converts to string
template <typename T>
inline std::string S(T k) {
    return static_cast<std::ostringstream&>(std::ostringstream() << k).str();
}

inline std::string S(const char *k) { return std::string(k); }
inline std::string S(char *k) { return std::string(k); }
inline std::string S(const char k) { return std::string(1, k); }
inline std::string S() { return std::string(); }

/// Makes 1-element vector
template <typename T>
inline std::vector<T> vector1 (T a) { return std::vector<T>(1, a); }

/// Makes 2-element vector
template <typename T>
inline std::vector<T> vector2 (T a, T b) 
    { std::vector<T> v = std::vector<T>(2, a); v[1] = b; return v;}

/// Make 3-element vector
template <typename T>
inline std::vector<T> vector3 (T a, T b, T c) 
    { std::vector<T> v = std::vector<T>(3, a); v[1] = b; v[2] = c; return v;}

/// Make 4-element vector
template <typename T>
inline std::vector<T> vector4 (T a, T b, T c, T d) { 
    std::vector<T> v = std::vector<T>(4, a); v[1] = b; v[2] = c; v[3] = d;
    return v; 
}

/// Make (u-l)-element vector, filled by numbers: l, l+1, ..., u.
std::vector<int> range_vector (int l, int u);

/// Expression like "i<v.size()", where i is int and v is a std::vector gives: 
/// "warning: comparison between signed and unsigned integer expressions"
/// implicit cast IMHO makes code less clear than "i<size(v)":
template <typename T>
inline int size(const std::vector<T>& v) { return static_cast<int>(v.size()); }

/// Return 0 <= n < a.size()
template <typename T>
inline bool is_index (int idx, const std::vector<T>& v) 
{ 
    return idx >= 0 && idx < static_cast<int>(v.size()); 
}


template <typename T>
inline std::string join_vector(const std::vector<T>& v, const std::string& sep)
{
    if (v.empty()) 
        return "";
    std::string s = S(v[0]);
    for (typename std::vector<T>::const_iterator i = v.begin() + 1; 
            i != v.end(); i++) 
        s += sep + S(*i);
    return s;
}

/// for vector<T*> - delete object and erase pointer
template<typename T>
void purge_element(std::vector<T*> &vec, int n)
{
    assert(n >= 0 && n < size(vec));
    delete vec[n];
    vec.erase(vec.begin() + n);
}

/// delete all objects handled by pointers and clear vector
template<typename T>
void purge_all_elements(std::vector<T*> &vec)
{
    for (typename std::vector<T*>::iterator i=vec.begin(); i!=vec.end(); ++i) 
        delete *i;
    vec.clear();
}



/// Round real to integer.
inline int iround(fp d) { return static_cast<int>(floor(d+0.5)); }

/// replace all occurences of old in string s with new_
void replace_all(std::string &s, const std::string &old, 
                                 const std::string &new_);

extern int smooth_limit;

/// flag that is set to interrupt fitting (it is checked after each iteration)
extern volatile bool user_interrupt;

extern const std::string help_filename;


/// Get current date and time as formatted string
std::string time_now ();

/// True if the string contains only a real number
bool is_double (std::string s);

enum OutputStyle  { os_normal, os_warn, os_quot, os_input };


class Sum;
class Data;
class V_f;
class V_z;
class V_g;
class LMfit;
class GAfit;
class NMfit;
class Crystal;
class GnuPlot;

#endif

