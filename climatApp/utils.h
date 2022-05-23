#ifndef UTILS_H
#define UTILS_H
#include <string.h>
#include <vector>
#include <numeric>
#include <boost/spirit/include/qi.hpp>
#include "iostream"
#include <fstream>

namespace qi = boost::spirit::qi;

std::vector<std::vector<std::string>> parseCSV2V(std::string aFileIn, char aDelim);


#endif // UTILS_H
