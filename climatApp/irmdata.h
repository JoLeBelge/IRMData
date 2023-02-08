#ifndef IRMDATA_H
#define IRMDATA_H
#include <netcdfcpp.h>
#include "date.h"
#include <string.h>
#include <vector>
#include <boost/filesystem.hpp>
#include <numeric>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <gdalwarper.h>
#include "ogr_spatialref.h"
#include <string>
#include "utils.h"

#include <boost/range/algorithm/remove_if.hpp>
using namespace date;

class dataOnePix;
class dataOneDate;
class irmData;

class irmData
{
public:
    irmData(){}
    irmData(std::string aFileIRM);
    // résumer les données sur un mois complet.
    dataOneDate dataMensuel(year y, month m);
    // cumul de DJ pour un mois
    dataOneDate dataMensuelDJ(year y, month m);
    dataOneDate getMax(year y, month m,bool allY=true);
    dataOneDate getMax(std::vector<year_month_day> aYMDs);
    dataOneDate dataAnnuel(year y);
    // Aurélien veux des données climatiques moyennes mais je n'ai pas encore les moyennes trentenaires, donc je fait la moyenne sur toute ces données là.
    dataOneDate moyAll();
    dataOneDate calculCarteMensuelTrentenaire(year y1, year y2, month m);

    // pour pouvoir utiliser cdo et toute la puissance des netcdf
    void saveNetCDF(std::string aOut);

protected:
    std::map<year_month_day,dataOneDate> mVAllDates;
    std::vector<std::string> mVVars;// contient les variables qui sont lues du fichier csv
    std::string mInPutFile;
};

class dataOneDate
{
public:
    dataOneDate(year_month_day ymd);

    void addOnePix(std::vector<std::string> & aLigne);
    void addOnePix(std::vector<std::string> & aLigne, int id);
    void addOneDate(dataOneDate * dod);
    void addOneDateDJ(dataOneDate * dod, double aSeuilDJ);
    void getMax(dataOneDate * dod);



    year_month_day getDate(){return mDate;}

    void divide(int nb, int nbMois=1);
    void exportMap(std::string aOut, std::string aVar);
    double getValForPix(int pixel_id,std::string aVar);
private:
    // key ; pixel id. val : toutes les données.
    std::map<int,dataOnePix> mVData;
    year_month_day mDate;
};

class dataOnePix
{
public:
    dataOnePix():Tmean(0),Tmax(0),Tmin(0),R(0),ETP(0),TminMin(100){}
    dataOnePix(std::vector<std::string> & aLigne);
    dataOnePix(dataOnePix * dop, double aSeuilDJ);

    void addOneDate(dataOnePix * dop);
    void addOneDateDJ(dataOnePix * dop, double aSeuilDJ);
    void divide(int nb, int nbMois=1);
    void cat(){std::cout << "Tmean " << Tmean << " Tmax " << Tmax << " Tmin " << Tmin << " P " << P << std::endl;}
//private:
    double Tmean,Tmax,Tmin,R,ETP, P, WS, TminMin;
};

#endif // IRMDATA_H
