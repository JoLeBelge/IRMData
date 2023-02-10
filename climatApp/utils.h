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
#include <boost/range/algorithm/remove_if.hpp>

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split

#include "csv.h"

// différent moyen d'aggréger les valeurs par mois
enum typeGrid {SOP
               ,irm
               ,SOP75
               ,irmO
                   };

namespace qi = boost::spirit::qi;

std::vector<std::vector<std::string>> parseCSV2V(std::string aFileIn, char aDelim);

std::vector<std::vector<std::string>> parseCSV2V_quick(std::string aFileIn);


void setGeoTMAR(std::string aRasterIn, typeGrid aGrid);

void exportRaster(std::string aNetCdfIn, std::string aRasterOut, typeGrid mode);


#endif // UTILS_H
