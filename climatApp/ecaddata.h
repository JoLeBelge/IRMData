#ifndef ECADDATA_H
#define ECADDATA_H
#include <netcdfcpp.h>
#include "date.h"
#include <string.h>
#include <vector>
#include <boost/filesystem.hpp>
#include <numeric>
#include <boost/spirit/include/qi.hpp>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <gdalwarper.h>
#include "ogr_spatialref.h"

using namespace date;
// pour extraire une bande netcdf au format raster ; gdal_translate -ot Int16 NETCDF:qq_ens_mean_0.1deg_reg_2011-2020_v22.0e.nc -b 1 -unscale "t1_01.tif"

year_month_day ymdFromString(std::string date);

int getValue(std::string pathTif, double x, double y);

// Return this in event of a problem.
static const int NC_ERR = 2;

extern std::vector<int> vYears;
extern std::vector<int> vMonths;
extern double baseDJ;


class ecadData
{
public:
    ecadData();
    void projectRaster(std::string inPath, std::string outPath);
    void projectRasterCallgdal(std::string inPath, std::string outPath);

    int calculCarteMensuel();
    void exportCarteUneDate(year_month_day ymd, std::string varAccro="qq");

    void calculDJPourDate();
    void exportCarteDJUneDate(year_month_day ymd);

    int timeFromYMD(year_month_day ymd, NcVar *timeVar, int nrec);
    std::vector<int> timesForOneMonth(year_month ym, NcVar *timeVar, int nrec);

    std::vector<int> timesForDateForDJ(year_month_day ymd, NcVar *timeVar, int nrec);

private:
    int NLON;
    int NLAT;
};

#endif // ECADDATA_H
