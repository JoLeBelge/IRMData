#ifndef IRMDATA_H
#define IRMDATA_H
#include "date.h"
#include <string.h>
#include <vector>
#include <boost/filesystem.hpp>
#include <numeric>
#include "boost/program_options.hpp"
#include <boost/spirit/include/qi.hpp>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <gdalwarper.h>
#include "ogr_spatialref.h"
using namespace date;
namespace po = boost::program_options;
namespace qi = boost::spirit::qi;

std::vector<std::vector<std::string>> parseCSV2V(std::string aFileIn, char aDelim);
class dataOnePix;
class dataOneDate;
class irmData;

class irmData
{
public:
    irmData(std::string aFileIRM);
    // résumer les données sur un mois complet.
    dataOneDate dataMensuel(year y, month m);
    // cumul de DJ pour un mois
    dataOneDate dataMensuelDJ(year y, month m);
    dataOneDate dataAnnuel(year y);
    // Aurélien veux des données climatiques moyennes mais je n'ai pas encore les moyennes trentenaires, donc je fait la moyenne sur toute ces données là.
    dataOneDate moyAll();

private:
    std::map<year_month_day,dataOneDate> mVAllDates;
};

class dataOneDate
{
public:
    dataOneDate(year_month_day ymd);
    // différent mode pour différent fichier , pas structuré pareil - pas très polyvalent comme code mais pas grave
    void addOnePix(std::vector<std::string> & aLigne, int mode=1);
    void addOneDate(dataOneDate * dod);
    void addOneDateDJ(dataOneDate * dod, double aSeuilDJ);

    void divide(int nb);
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
    dataOnePix():Tmean(0.0),Tmax(0),Tmin(0),R(0),ETP(0){}
    dataOnePix(std::vector<std::string> & aLigne, int mode=1);// différent mode, car les données irm ne sont pas structuré pareil en fonction de la demande que l'on leur adresse

    dataOnePix(dataOnePix * dop, double aSeuilDJ);

    void addOneDate(dataOnePix * dop);
    void addOneDateDJ(dataOnePix * dop, double aSeuilDJ);
    void divide(int nb);
//private:
    double Tmean,Tmax,Tmin,R,ETP, P;
};

#endif // IRMDATA_H
