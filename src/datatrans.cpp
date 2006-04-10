// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

// the idea of VM is based on one of boost::spirit samples - vmachine_calc

// this file can be compiled to stand-alone test program:
// $ g++ -I../3rdparty -DSTANDALONE_DATATRANS datatrans.cpp numfuncs.cpp -o dt
// $ ./dt 

// right hand variables: 
//       arrays (index in square brackets can be ommitted and n is assumed):
//              x[] - x coordinate of point before transformation (old x)  
//              X[] - x coordinate after transformation (new x),
//              y[], Y[] - y coordinate of point (before/after transformation)
//              s[], S[]  -  std. dev. of y, 
//              a[], A[] - active point, either 0 or 1
//       scalars:
//              M - size of arrays, 
//              n - current index (index of currently transformed point)
// upper-case variables are assignable 
// assignment syntax: 
//        or: assignable_var = expression     --> executed for all points
//        or: assignable_var[k] = expression  --> executed for k-th point
//        or: assignable_var[k...m] = expression  
//        or: assignable_var[k...] = expression  
//        or: assignable_var[...k] = expression  
//   where k and m are integers (negative are counted from the end:
//                               -1 is last, -2 one before last, and so on)
//                               FIXME: should x[-1] also mean x[M-1]?
//     k...m means that expression is executed for k, k+1, ... m-1. Not for m.
//
// Assignments are executed for n = 0, ..., M-1 (if assignments are joined 
// with '&', all are executed for n=0, then all for n=1, and so on).
// Before transformation the new point coordinates are a copy of the old ones.
//
// There is a special function sum(), which can be used as a real value. 
// and returns a sum over all points of value of expression 
// eg. sum(n > 0 ? (x[n] - x[n-1]) * (y[n-1] + y[n])/2 : 0) -> area.
// The value of sum() is computed before transformation.
//
//
// There are statements, that are executed only once: 
//    M=..., 
//    order=...
// and assignments T[k]=..., if these statements are joined with other 
// assignments using '&', they are executed first, before iteration over all
// indices.
//
//        M=length (eg. M=M+5 or M=100) changes number of points: 
//        either appends new data points with x=y=sigma=0, is_active=false,
//        or deletes last points.
//        order=[-]coordinate (eg. order=x, order=-x) 
//        sort data points; sorting is stable. After finishing transform 
//        command, points are sorted using x.
//        delete[k] (or delete[k...m])
//        delete(condition)
//           deletion and change of M is postponed to the end of computation
// 

// operators: binary +, -, *, /, % , ^  
// ternary operator:       condition ? expression1 : expression2 
// 
//functions: sqrt exp log10 ln sin cos tan atan asin acos abs min2 max2 round
// boolean: AND, OR, >, >=, <, <=, = (or ==), != (or <>), TRUE, FALSE, NOT
//
//parametrized functions: spline, interpolate //TODO->polyline
// The general syntax is: pfunc[param1, param2,...](expression), 
//  eg. spline[22.1, 37.9, 48.1, 17.2, 93.0, 20.7](x)
// spline - cubic spline interpolation, parameters are x1, y1, x2, y2, ...
// interpolate - polyline interpolation, parameters are x1, y1, x2, y2, ...
//
// All computations are performed using real numbers, but using round for
// comparisions should not be neccessary. Two numbers that differ less
// than epsilon=1e-9 ie. abs(a-b)<epsilon, are considered equal. 
// Indices are also computed in real number domain, 
// and if they are not integers, interpolation of two values
// is taken (i.e of values with indices floor(idx) and ceil(idx) 
//     
//
// Syntax examples:
//    set standard deviation as sqrt(max2(1,y))
//                         s = sqrt(max2(1,y))
//         or (the same):  s = sqrt(max2(1,y[n]))
//                    or:  S = sqrt(max2(1,y[n]))
//                    or:  S = sqrt(max2(1,Y[n]))
//                    or:  S = sqrt(max2(1,Y))
//
//    integration:   Y[1...] = Y[n-1] + y[n]  
//
//    swaping x and y axes: y=x & x=y & s=sqrt(Y) 
//                      or: y=x & x=y & s=sqrt(x)
//
//    smoothing: Y[1...-1] = (y[n-1] + y[n+1])/2  
//
//    reducing: order = x, # not neccessary, points are always in this order
//              x[...-1] = (x[n]+x[n+1])/2, 
//              y[...-1] = y[n]+y[n+1],
//              delete(n%2==1)    
//
//    delete inactive points:    delete(not a)  
//    normalize area: 
//           y = y / sum(n > 0 ? (x[n] - x[n-1]) * (y[n-1] + y[n])/2 : 0) 
//
//    substract spline baseline given by points (22.17, 37.92), (48.06, 17.23),
//                                              (93.03, 20.68).
//    y = y - spline[22.17 37.92  48.06 17.23  93.03 20.68](x) 
//

//-----------------------------------------------------------------
// how it works:
//   * parse string and prepare VM code (list of operators) 
//     and data (list of numbers). Actually there are two VM codes and
//     data arrays - one for do-once operations, and second 
//     for for-many-points operations. 
//  
//   * execute do-once operations (eg. order=x, x[3]=4) 
//     and computes value of sum()
//
//   * execute all assignments for every point (from first to last),
//     unless the point is outside of specified range.
//


//#define BOOST_SPIRIT_DEBUG


//#include "datatrans.h"
#include "datatrans2.h"
#include "sum.h"
//#include "common.h"
//#include "data.h"
//#include "var.h"
//#include "numfuncs.h"
//#include "logic.h"
//#include <boost/spirit/core.hpp>

//using namespace std;
//using namespace boost::spirit;
using namespace datatrans;

#ifdef STANDALONE_DATATRANS

#include <iostream>  
bool dt_verbose = false;
#define DT_DEBUG(x) if (dt_verbose) std::cout << (x) << std::endl;
#define DT_DEBUG_N(x) if (dt_verbose) std::cout << (x); 
//-------------------------  Main program  ---------------------------------
#include <string.h>

int main(int argc, char **argv)
{
    cout << "DataTransformVMachine test started...\n";
    if (argc > 1 && !strcmp(argv[1], "-v")) {
        cout << "[verbose mode]" << endl;
        dt_verbose=true;
    }
    cout << "==> ";

    string str;
    while (getline(cin, str))
    {
        vector<Point> points;

        points.push_back(Point(0.1, 4, 1));
        points.push_back(Point(0.2, 2, 1));
        points.push_back(Point(0.3, -2, 1));
        points.push_back(Point(0.4, 3, 1));
        points.push_back(Point(0.5, 3, 1));

        try {
            vector<Point> transformed_points = transform_data(str, points);
            int len = (int) points.size();
            for (int n = 0; n != (int) transformed_points.size(); n++) 
                cout << "point " << n << ": " 
                    << (n < len ? points[n] : Point()).str()
                    << " -> " << transformed_points[n].str() << endl;
        } catch (ExecuteError& e) {
            cout << e.what() << endl;
        }
        cout << "==> ";
    }
    return 0;
}

#else 

#    define DT_DEBUG(x) ;
#    define DT_DEBUG_N(x) ;

#endif //STANDALONE_DATATRANS

//----------------------------  grammar  ----------------------------------
template <typename ScannerT>
DataTransformGrammar::definition<ScannerT>::definition(
                                          DataTransformGrammar const& /*self*/)
{
    range
        =  '[' >> 
              ( eps_p(int_p >> ']')[push_op(OP_DO_ONCE)] 
                >> int_p [push_double()] [push_op(OP_INDEX)]
              | (
                  ch_p('n') [push_the_double(0.)] [push_the_double(0.)]
                | (
                    ( int_p [push_double()]
                    | eps_p [push_the_double(0.)]
                    )
                     >> (str_p("...") | "..")
                     >> ( int_p [push_double()]
                        | eps_p [push_the_double(0)]
                        )
                   )
                ) [push_op(OP_RANGE)] 
              )  
            >> ']'
        ;

    order 
        =  ('-' >> as_lower_d["x"])        [push_the_double(-1.)]
        |  (!ch_p('+') >> as_lower_d["x"]) [push_the_double(+1.)]
        |  ('-' >> as_lower_d["y"])        [push_the_double(-2.)]
        |  (!ch_p('+') >> as_lower_d["y"]) [push_the_double(+2.)]
        |  ('-' >> as_lower_d["s"])        [push_the_double(-3.)]
        |  (!ch_p('+') >> as_lower_d["s"]) [push_the_double(+3.)]
        ;


    assignment //not only assignments
        =  (as_lower_d["x"] >> !range >> '=' >> DataExpressionG) 
                                                     [push_op(OP_ASSIGN_X)]
        |  (as_lower_d["y"] >> !range >> '=' >> DataExpressionG) 
                                                     [push_op(OP_ASSIGN_Y)]
        |  (as_lower_d["s"] >> !range >> '=' >> DataExpressionG) 
                                                     [push_op(OP_ASSIGN_S)]
        |  (as_lower_d["a"] >> !range >> '=' >> DataExpressionG) 
                                                     [push_op(OP_ASSIGN_A)]
        |  ((ch_p('M') >> '=') [push_op(OP_DO_ONCE)]  
           >> DataExpressionG) [push_op(OP_RESIZE)]  
        |  ((as_lower_d["order"] >> '=') [push_op(OP_DO_ONCE)]  
           >> order) [push_op(OP_ORDER)] 
        |  (as_lower_d["delete"] >> eps_p('[') [push_op(OP_DO_ONCE)]  
            >> range) [push_op(OP_DELETE)]
        |  (as_lower_d["delete"] >> '(' >> DataExpressionG >> ')') 
                                                   [push_op(OP_DELETE_COND)]
        ;

    statement 
        = (eps_p[push_op(OP_BEGIN)] >> assignment >> eps_p[push_op(OP_END)])
                                                             % ch_p(',') 
        ;
}

// explicit template instantiations 
template DataTransformGrammar::definition<scanner<char const*, scanner_policies<skipper_iteration_policy<iteration_policy>, match_policy, no_actions_action_policy<action_policy> > > >::definition(DataTransformGrammar const&);

template DataTransformGrammar::definition<scanner<char const*, scanner_policies<skipper_iteration_policy<iteration_policy>, match_policy, no_actions_action_policy<no_actions_action_policy<action_policy> > > > >::definition(DataTransformGrammar const&);

DataTransformGrammar DataTransformG;


namespace datatrans {

fp find_idx_in_sorted(vector<Point> const& pp, fp x)
{
    if (x <= pp.front().x)
        return 0;
    else if (x >= pp.back().x)
        return pp.size() - 1;
    vector<Point>::const_iterator i = lower_bound(pp.begin(), pp.end(), x);
    assert (i > pp.begin() && i < pp.end());
    if (is_eq(x, i->x))
            return i - pp.begin();
    else
        return i - pp.begin() - (i->x - x) / (i->x - (i-1)->x);
}


//------------------------  Virtual Machine  --------------------------------

vector<int> code;        //  VM code 
vector<double> numbers;  //  VM data 
vector<ParameterizedFunction*> parameterized; // also used by VM 
const int stack_size = 8192;  //should be enough, 
                              //there are no checks for stack overflow  

void clear_parse_vecs()
{
    code.clear();
    numbers.clear();
    //TODO shared_ptr
    purge_all_elements(parameterized);
}

// code vector contains not only operators, but also indices that
// points locations in numbers or parameterized vectors
bool is_operator(vector<int>::iterator i, DataTransformVMOperator op)
{
    assert(code.begin() <= i && i < code.end());
    return (*i == op && (i != code.begin() 
                        || *(i-1) != OP_NUMBER && *(i-1) != OP_PARAMETERIZED
                           && *(i-1) != OP_FUNC && *(i-1) != OP_SUM_F
                           && *(i-1) != OP_SUM_Z));
}

vector<int>::const_iterator 
skip_code(vector<int>::const_iterator i, int start_op, int finish_op)
{
    int counter = 1;
    while (counter) {
        ++i;
        if (*i == finish_op) counter--;
        else if (*i == start_op)  counter++;
    }
    return i;
}

void skip_to_end(vector<int>::const_iterator &i)
{
    DT_DEBUG("SKIPing")
    while (*i != OP_END)
        ++i;
}

template<typename T>
fp get_var_with_idx(fp idx, vector<Point> points, T Point::*t)
{
    if (idx < 0 && idx > points.size()-1)
        return 0.;
    else if (is_eq(idx, iround(idx)))
        return points[iround(idx)].*t;
    else {
        int flo = int(floor(idx));
        fp fra = idx - flo;
        return (1-fra) * fp(points[flo].*t) + fra * fp(points[flo+1].*t);
    }
}

// Return value: 
//  if once==true (one-time pass).
//   true means: it is neccessary to call this function for every point.
//  if iterative pass:
//   true means: delete this point.
// n: index of point
// M: number of all points (==new_points.size())
bool execute_code(int n, int &M, vector<double>& stack,
                  vector<Point> const& old_points, vector<Point>& new_points,
                  vector<int> const& code)  
{
    DT_DEBUG("executing code; n=" + S(n) + " M=" + S(M))
    assert(M == size(new_points));
    bool once = (n == M);
    bool return_value=false; 
    vector<double>::iterator stackPtr = stack.begin() - 1;//will be ++'ed first
    for (vector<int>::const_iterator i=code.begin(); i != code.end(); i++) {
        DT_DEBUG("NOW op " + S(*i))
        switch (*i) {
            //unary-operators
            case OP_NEG:
                *stackPtr = - *stackPtr;
                break;
            case OP_SQRT:
                *stackPtr = sqrt(*stackPtr);
                break;
            case OP_EXP:
                *stackPtr = exp(*stackPtr);
                break;
            case OP_LOG10:
                *stackPtr = log10(*stackPtr); 
                break;
            case OP_LN:
                *stackPtr = log(*stackPtr); 
                break;
            case OP_SIN:
                *stackPtr = sin(*stackPtr);
                break;
            case OP_COS:
                *stackPtr = cos(*stackPtr);
                break;
            case OP_TAN:
                *stackPtr = tan(*stackPtr); 
                break;
            case OP_ATAN:
                *stackPtr = atan(*stackPtr); 
                break;
            case OP_ASIN:
                *stackPtr = asin(*stackPtr); 
                break;
            case OP_ACOS:
                *stackPtr = acos(*stackPtr); 
                break;
            case OP_ABS:
                *stackPtr = fabs(*stackPtr);
                break;
            case OP_ROUND:
                *stackPtr = floor(*stackPtr + 0.5);
                break;

            case OP_x_IDX:
                *stackPtr = find_idx_in_sorted(old_points, *stackPtr);
                break;

            case OP_FUNC:
                i++;
                *stackPtr = AL->get_function(*i)->calculate_value(*stackPtr);
                break;
            case OP_SUM_F:
                i++;
                *stackPtr = AL->get_sum(*i)->value(*stackPtr);
                break;
            case OP_SUM_Z:
                i++;
                *stackPtr = AL->get_sum(*i)->zero_shift(*stackPtr);
                break;
            case OP_NUMAREA:
                i+=2;
                stackPtr-=2;
                if (*(i-1) == OP_FUNC) {
                    *stackPtr = AL->get_function(*i)->numarea(*stackPtr, 
                                        *(stackPtr+1), iround(*(stackPtr+2)));
                }
                else if (*(i-1) == OP_SUM_F) {
                    *stackPtr = AL->get_sum(*i)->numarea(*stackPtr, 
                                        *(stackPtr+1), iround(*(stackPtr+2)));
                }
                else // OP_SUM_Z
                    throw ExecuteError("numarea(Z,...) is not implemented."
                                       "Does anyone need it?"); 
                break;

            case OP_FINDX:
                i+=2;
                stackPtr-=2;
                if (*(i-1) == OP_FUNC) {
                    *stackPtr = AL->get_function(*i)->find_x_with_value(
                                      *stackPtr, *(stackPtr+1), *(stackPtr+2));
                }
                else if (*(i-1) == OP_SUM_F) {
                    throw ExecuteError("findx(F,...) is not implemented. "
                                       "Does anyone need it?"); 
                }
                else // OP_SUM_Z
                    throw ExecuteError("findx(Z,...) is not implemented. "
                                       "Does anyone need it?"); 
                break;

            case OP_FIND_EXTR:
                i+=2;
                stackPtr-=1;
                if (*(i-1) == OP_FUNC) {
                    *stackPtr = AL->get_function(*i)->find_extremum(*stackPtr,
                                                                *(stackPtr+1));
                }
                else if (*(i-1) == OP_SUM_F) {
                    throw ExecuteError("extremum(F,...) is not implemented. "
                                       "Does anyone need it?"); 
                }
                else // OP_SUM_Z
                    throw ExecuteError("extremum(Z,...) is not implemented. "
                                       "Does anyone need it?"); 
                break;

            case OP_PARAMETERIZED:
                i++;
                *stackPtr = parameterized[*i]->calculate(*stackPtr);
                break;

            //binary-operators
            case OP_MIN:
                stackPtr--;
                *stackPtr = min(*stackPtr, *(stackPtr+1));
                break;
            case OP_MAX:
                stackPtr--;
                *stackPtr = max(*stackPtr, *(stackPtr+1));
                break;
            case OP_RANDU:
                stackPtr--;
                *stackPtr = rand_uniform(*stackPtr, *(stackPtr+1));
                break;
            case OP_RANDNORM:
                stackPtr--;
                *stackPtr += rand_gauss() * *(stackPtr+1);
                break;
            case OP_ADD:
                stackPtr--;
                *stackPtr += *(stackPtr+1);
                break;
            case OP_SUB:
                stackPtr--;
                *stackPtr -= *(stackPtr+1);
                break;
            case OP_MUL:
                stackPtr--;
                *stackPtr *= *(stackPtr+1);
                break;
            case OP_DIV:
                stackPtr--;
                *stackPtr /= *(stackPtr+1);
                break;
            case OP_MOD:
                stackPtr--;
                *stackPtr -= floor(*stackPtr / *(stackPtr+1)) * *(stackPtr+1);
                break;
            case OP_POW:
                stackPtr--;
                *stackPtr = pow(*stackPtr, *(stackPtr+1));
                break;

            // putting-number-to-stack-operators
            // stack overflow not checked
            case OP_NUMBER:
                stackPtr++;
                i++;
                *stackPtr = numbers[*i];
                break;
            case OP_VAR_n:
                stackPtr++;
                *stackPtr = static_cast<double>(n);
                break;
            case OP_VAR_M:
                stackPtr++;
                *stackPtr = static_cast<double>(new_points.size());
                break;
            case OP_VAR_x:
                *stackPtr = get_var_with_idx(*stackPtr, old_points, &Point::x);
                break;
            case OP_VAR_y:
                *stackPtr = get_var_with_idx(*stackPtr, old_points, &Point::y);
                break;
            case OP_VAR_s:
                *stackPtr = get_var_with_idx(*stackPtr, old_points, 
                                             &Point::sigma);
                break;
            case OP_VAR_a:
                *stackPtr = bool(iround(get_var_with_idx(*stackPtr, old_points, 
                                                         &Point::is_active)));
                break;
            case OP_VAR_X:
                *stackPtr = get_var_with_idx(*stackPtr, new_points, &Point::x);
                break;
            case OP_VAR_Y:
                *stackPtr = get_var_with_idx(*stackPtr, new_points, &Point::y);
                break;
            case OP_VAR_S:
                *stackPtr = get_var_with_idx(*stackPtr, new_points, 
                                             &Point::sigma);
                break;
            case OP_VAR_A:
                *stackPtr = bool(iround(get_var_with_idx(*stackPtr, new_points, 
                                                         &Point::is_active)));
                break;

            //assignment-operators
            case OP_ASSIGN_X:
                new_points[n].x = *stackPtr;
                stackPtr--; 
                break;
            case OP_ASSIGN_Y:
                new_points[n].y = *stackPtr;
                stackPtr--; 
                break;
            case OP_ASSIGN_S:
                new_points[n].sigma = *stackPtr;
                stackPtr--; 
                break;
            case OP_ASSIGN_A:
                new_points[n].is_active = is_neq(*stackPtr, 0.);
                stackPtr--; 
                break;

            // logical; can skip part of VM code !
            case OP_AND:
                if (*stackPtr)    //return second
                    stackPtr--; 
                else              // return first
                    i = skip_code(i, OP_AND, OP_AFTER_AND);
                break;

            case OP_OR:
                if (*stackPtr)    //return first
                    i = skip_code(i, OP_OR, OP_AFTER_OR);
                else              // return second
                    stackPtr--; 
                break;

            case OP_NOT:
                *stackPtr = is_eq(*stackPtr, 0.);
                break;

            case OP_TERNARY:
                if (! *stackPtr) 
                    i = skip_code(i, OP_TERNARY, OP_TERNARY_MID);
                stackPtr--; 
                break;
            case OP_TERNARY_MID:
                //if we are here, condition was true. Skip.
                i = skip_code(i, OP_TERNARY_MID, OP_AFTER_TERNARY);
                break;

            case OP_AFTER_AND: //do nothing
            case OP_AFTER_OR:
            case OP_AFTER_TERNARY:
                break;

            case OP_DELETE_COND:
                if (*stackPtr)
                    return_value = true;
                stackPtr--;
                break;

            // comparisions
            case OP_LT:
                stackPtr--;
                *stackPtr = is_lt(*stackPtr, *(stackPtr+1));
                break;
            case OP_GT:
                stackPtr--;
                *stackPtr = is_gt(*stackPtr, *(stackPtr+1));
                break;
            case OP_LE:
                stackPtr--;
                *stackPtr = is_le(*stackPtr, *(stackPtr+1));
                break;
            case OP_GE:
                stackPtr--;
                *stackPtr = is_ge(*stackPtr, *(stackPtr+1));
                break;
            case OP_EQ:
                stackPtr--;
                *stackPtr = is_eq(*stackPtr, *(stackPtr+1));
                break;
            case OP_NEQ:
                stackPtr--;
                *stackPtr = is_neq(*stackPtr, *(stackPtr+1));
                break;

                // next comparision hack, see rbool rule for more...
            case OP_NCMP_HACK:
                stackPtr++;                // put number, that is accidentally 
                *stackPtr = *(stackPtr+1); // in unused part of the stack
                break;
            
            //transformation condition 
            case OP_INDEX:
                assert(once);  //x[n]= or delete[n]
                n = iround(*stackPtr);//changing n(!!) for use in OP_ASSIGN_
                stackPtr--;
                if (n < 0)
                    n += M;
                if (n < 0 || n >= M)
                    skip_to_end(i);
                if (*(i+1) == OP_DELETE) {
                    new_points.erase(new_points.begin() + n);
                    M--;
                    skip_to_end(i);
                }
                break;
            case OP_RANGE: 
              {
                //x[i...j]= or delete[i...j]
                int right = iround(*stackPtr); //Last In First Out
                stackPtr--;                    
                if (right <= 0)
                    right += M;
                int left = iround(*stackPtr);
                stackPtr--;
                if (left < 0)
                    left += M;
                if (*(i+1) == OP_DELETE) {
                    if (0 < left && left < right && right <= M) {
                        new_points.erase(new_points.begin()+left, 
                                         new_points.begin()+right);
                        M = size(new_points);
                    }
                    skip_to_end(i);
                }
                else { 
                    //if n not in [i...j] then skip to prevent OP_ASSIGN_.
                    bool n_between = (left <= n && n < right);
                    if (!n_between) 
                        skip_to_end(i);
                }
                break;
              }
            case OP_BEGIN:
              {
                bool next_op_once = (*(i+1) == OP_DO_ONCE);
                if (next_op_once != once) {
                    skip_to_end(i);
                    if (once)
                        return_value=true;
                }
                break;
              }
            case OP_END: //nothing -- it is used only for skipping assignment
                break;
            // once - operators
            case OP_DO_ONCE: //do nothing, it is only a flag
                assert(once); 
                break;
            case OP_RESIZE:
                assert(once);
                M = iround(*stackPtr);
                stackPtr--;
                new_points.resize(M);
                break;
            case OP_ORDER:
              {
                assert(once);
                int ord = iround(*stackPtr);
                stackPtr--;
                DT_DEBUG("in OP_ORDER with " + S(ord))
                if (ord == 1) {
                    DT_DEBUG("sort x_lt")
                    stable_sort(new_points.begin(), new_points.end(), x_lt);
                }
                else if(ord == -1) {
                    DT_DEBUG("sort x_gt")
                    stable_sort(new_points.begin(), new_points.end(), x_gt);
                }
                else if(ord == 2) {
                    DT_DEBUG("sort y_lt")
                    stable_sort(new_points.begin(), new_points.end(), y_lt);
                }
                else if(ord == -2) {
                    DT_DEBUG("sort y_gt")
                    stable_sort(new_points.begin(), new_points.end(), y_gt);
                }
                break;
              }
            case OP_DELETE:
                assert(0); //OP_DELETE is processed in OP_INDEX OR OP_RANGE
                break;
            case OP_SUM: //called from replace_sums() only
                assert(!once);
                assert(stackPtr == stack.begin());
                return false;
            case OP_IGNORE:
                break;
            default:
                DT_DEBUG("Unknown operator in VM code: " + S(*i))
        }
    }
    assert(stackPtr == stack.begin() - 1 //DataTransformGrammar
            || (stackPtr == stack.begin() && once)); //DataExpressionGrammar
    return return_value;
}

//change  BEGIN X ... X SUM END  with  NUMBER INDEX IGNORE ... IGNORE END
void replace_sums(int M, vector<double>& stack, vector<Point> const& old_points)
{
    DT_DEBUG_N("code before replace:");
    for (vector<int>::const_iterator i=code.begin(); i != code.end(); i++) 
        DT_DEBUG_N(" " + S(*i));
    DT_DEBUG("")
    bool nested = false;
    bool in_sum = false;
    vector<int>::iterator sum_end;
    for (vector<int>::iterator i = code.end()-1; i != code.begin(); i--) {
        if (is_operator(i, OP_SUM)) {
            if (in_sum)
                nested = true;
            sum_end = i;
            in_sum = true;
        }
        else if (in_sum && is_operator(i, OP_BEGIN)) {
            double sum = 0.;
            vector<Point> fake_new_points(M);
            vector<int> sum_code(i, sum_end+1);
            for (int n = 0; n != M; n++) {
                execute_code(n, M, stack, old_points, fake_new_points,sum_code);
                DT_DEBUG_N("n=" + S(n) + " on stack: " + S(stack.front()));
                sum += stack.front();
                DT_DEBUG(" sum:" + S(sum))
            }
            *i = OP_NUMBER;
            *(i+1) = size(numbers);
            numbers.push_back(sum);
            for (vector<int>::iterator j=i+2; j < sum_end+1; j++) 
                *j = OP_IGNORE; //FIXME: why not code.erase() ?
            in_sum = false;
        }
    }
    //in rare case of nested sum() we need:
    if (nested)
        replace_sums(M, stack, old_points);
    DT_DEBUG_N("code after replace:");
    for (vector<int>::const_iterator i=code.begin(); i != code.end(); i++) 
        DT_DEBUG_N(" " + S(*i));
    DT_DEBUG("")
}

//-------------------------------------------------------------------------


void execute_vm_code(const vector<Point> &old_points, vector<Point> &new_points)
{
    vector<double> stack(stack_size);
    int M = (int) new_points.size();
    replace_sums(M, stack, old_points);
    // first execute one-time operations: sorting, x[15]=3, etc. 
    // n==M => one-time op.
    bool t = execute_code(M, M, stack, old_points, new_points, code);
    if (!t) 
        return;
    vector<int> to_be_deleted;
    for (int n = 0; n != M; n++) {
        bool r = execute_code(n, M, stack, old_points, new_points, code);
        if (r)
            to_be_deleted.push_back(n);
    }
    if (!to_be_deleted.empty())
        for (vector<int>::const_iterator i = to_be_deleted.end() - 1; 
                                             i >= to_be_deleted.begin(); --i)
            new_points.erase(new_points.begin() + *i);
}

} //namespace

vector<Point> transform_data(string const& str, vector<Point> const& old_points)
{
    clear_parse_vecs();
    // First compile string...
    parse_info<> result = parse(str.c_str(), DataTransformG, space_p);
    if (!result.full) {
        throw ExecuteError("Syntax error in data transformation formula.");
    }
    // and then execute compiled code.
    vector<Point> new_points = old_points; //initial values of new_points
    execute_vm_code(old_points, new_points);
    return new_points;
}

bool validate_transformation(string const& str)
{
    clear_parse_vecs();
    // First compile string...
    parse_info<> result = parse(str.c_str(), DataTransformG, space_p);
    return (bool) result.full;
}

//TODO function which check for point-dependent ops 
// OP_VAR_n, OP_VAR_x, OP_VAR_y, OP_VAR_s, OP_VAR_a, 
//           OP_VAR_X, OP_VAR_Y, OP_VAR_S, OP_VAR_A,
//  -> because  "info 2+2" should work without "in @0"
// bool is_dependend_on_points(string const &s)


fp get_transform_expression_value(string const &s, Data const* data)
{
    vector<Point> const no_points;
    vector<Point> const& points = data ? data->points() : no_points;
    clear_parse_vecs();
    // First compile string...
    parse_info<> result = parse(s.c_str(), DataExpressionG, 
                                space_p);
    if (!result.full) {
        throw ExecuteError("Syntax error in expression: " + s);
    }
    int M = (int) points.size();
    vector<Point> new_points = points;
    vector<double> stack(stack_size);
    replace_sums(M, stack, points);
    // first execute one-time operations: sorting, x[15]=3, etc. 
    // n==M => one-time op.
    bool t = execute_code(M, M, stack, points, new_points, code);
    if (t) { 
        throw ExecuteError("Expression depends on undefined `n' index: " + s);
    }
    return stack.front();
}


