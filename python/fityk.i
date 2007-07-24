// This file is part of fityk program. 
// Licence: GNU General Public License version 2
// $Id: fityk.h 272 2007-03-17 19:03:07Z wojdyr $

%module fityk
%feature("autodoc", "1");

%{
#include "fityk.h"
%}
%include "std_string.i"
%include "std_vector.i"
%include "exception.i"
namespace std {
        %template(PointVector) vector<fityk::Point>;
        %template(DoubleVector) vector<double>;
}

%rename(__str__) str();

%ignore set_show_message;

%include "fityk.h"

%extend fityk::ExecuteError 
{
        const char* __str__() { return $self->what(); }
};

%extend fityk::SyntaxError 
{
        const char* __str__() { return ""; }
};


