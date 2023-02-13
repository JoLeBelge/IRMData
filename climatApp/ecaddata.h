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


// différent moyen d'aggréger les valeurs par mois
enum resumeMensuel {moy
                    ,min
                    ,sum
                    ,max
                   };


class ecadData
{
public:
    ecadData();

    void projectRasterCallgdal(std::string inPath, std::string outPath);

    int calculCarteMensuel(std::vector<int> avYears, std::string varAccro, resumeMensuel type=resumeMensuel::moy);
    // pas spécialement trentenaire mais moyenne de plusieurs années.
    int calculCarteMensuelTrentenaire(year y1, year y2, std::string varAccro="tg", resumeMensuel type=resumeMensuel::moy,std::string prefix="");

    void exportCarteUneDate(year_month_day ymd, std::string varAccro="qq");

    double getScaleF(std::string var){
        // je regarde sous qgis les scale factor ; qq = 1, rr = 0.1, temp = 0.0099999998
        double scaleF(1);
        if (var=="tn" |var=="tx" | var=="tg"){
            scaleF=0.0099999998;
        }
        if (var=="rr" | var=="pp"){
            scaleF=0.1;
        }
        return scaleF;
    }

    std::string getMode(std::string varAccro,resumeMensuel type){
        std::string mode("");
       if (type== resumeMensuel::sum && varAccro!="rr"){
             mode="sum";
        }
       if (type== resumeMensuel::min){
             mode="min";
        }
       if (type== resumeMensuel::max){
             mode="max";
        }
       return mode;
    }

    std::string getNamePath(std::string var){return "/home/lisein/Documents/Scolyte/Data/climat/01deg/"+var+"_ens_mean_0.1deg_reg_2011-2020_v22.0e.nc";}
    //std::string getNamePath(std::string var){return "/home/lisein/Documents/Scolyte/Data/climat/eobs24/"+var+"_ens_mean_0.25deg_reg_v24.0e.nc";}


    // il sert à définir la transformation ce template, et à définir le src je pense.
   std::string getNameTifModel(std::string var, int resol=1){
        var="tg";
        std::string aRes("/home/lisein/Documents/Scolyte/Data/climat/01deg/template_"+var+".tif");
        if (resol==2){
             aRes="/home/lisein/Documents/Scolyte/Data/climat/eobs24/template_"+var+".tif";
        } else {
            aRes="/home/lisein/Documents/Scolyte/Data/climat/01deg/template.tif";
        }
        return aRes;
    }

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
