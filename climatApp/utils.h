#ifndef UTILS_H
#define UTILS_H
#include <string.h>
#include <vector>
#include <numeric>
#include <boost/spirit/include/qi.hpp>
#include "iostream"
#include <fstream>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <gdalwarper.h>
#include "ogr_spatialref.h"
#include <string>
#include "boost/filesystem.hpp"

namespace qi = boost::spirit::qi;

std::vector<std::vector<std::string>> parseCSV2V(std::string aFileIn, char aDelim);


void setGeoTMAR(std::string aRasterIn);

void exportRaster(std::string aNetCdfIn, std::string aRasterOut);


#endif // UTILS_H
