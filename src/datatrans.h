// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#ifndef FITYK__DATATRANS__H__
#define FITYK__DATATRANS__H__


#include <boost/spirit/core.hpp>

#include "common.h"
using namespace boost::spirit;

struct Point;

std::vector<Point> transform_data(std::string const& str, 
                                  std::vector<Point> const& old_points);

bool validate_transformation(std::string const& str); 
fp get_transform_expression_value(std::string const &s, Data const* data);


struct DataExpressionGrammar : public grammar<DataExpressionGrammar>
{
  template <typename ScannerT>
  struct definition
  {
    definition(DataExpressionGrammar const& /*self*/);

    rule<ScannerT> rprec1, rbool_or, rbool_and, rbool_not, rbool,
                   rprec2, rprec3, rprec4, rprec5; 

    rule<ScannerT> const& start() const { return rprec1; }
  };
};

extern DataExpressionGrammar DataExpressionG;

struct DataTransformGrammar : public grammar<DataTransformGrammar>
{
  template <typename ScannerT>
  struct definition
  {
    definition(DataTransformGrammar const& /*self*/);

    rule<ScannerT> assignment, statement, range, order;

    rule<ScannerT> const& start() const { return statement; }
  };
};

extern DataTransformGrammar DataTransformG;




#endif 
