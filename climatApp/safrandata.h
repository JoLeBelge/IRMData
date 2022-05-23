#ifndef SAFRANDATA_H
#define SAFRANDATA_H
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
#include "irmdata.h"
#include <iostream>
#include <fstream>

#include "irmdata.h"

using namespace date;

class dataOnePix; // classe partagée
class dataOneDate;
class irmData;
//class safdataOneDate;
class safranData;


/*
class safdataOneDate
{
public:
    safdataOneDate(year_month_day ymd);

    void addOnePix(std::vector<std::string> & aLigne);
    void addOneDate(safdataOneDate * dod);
   // void addOneDateDJ(safdataOneDate * dod, double aSeuilDJ);
    void getMax(safdataOneDate * dod);

    year_month_day getDate(){return mDate;}

    void divide(int nb);
    void exportMap(std::string aOut, std::string aVar);

    //double getValForPix(int pixel_id,std::string aVar);

    // key ; pixel id. val : toutes les données.
    std::map<std::tuple<int,int>,dataOnePix> mVData;
    private:
    year_month_day mDate;
};
*/

/*
class safdataOnePix
{
public:
    safdataOnePix():Tmean(0.0),Tmax(0),Tmin(0),R(0),ETP(0){}
    safdataOnePix(std::vector<std::string> & aLigne);
    safdataOnePix(safdataOnePix * dop, double aSeuilDJ);

    void addOneDate(safdataOnePix * dop);
    //void addOneDateDJ(safdataOnePix * dop, double aSeuilDJ);
    void divide(int nb);
//private:
    double Tmean,Tmax,Tmin,R,ETP, P, WS;
};*/

class safranData : public irmData
{
public:
    safranData(std::string aFileIn);
    /*safdataOneDate dataMensuel(year y, month m);
    // cumul de DJ pour un mois

    safdataOneDate getMax(year y, month m,bool allY=true);
    safdataOneDate getMax(std::vector<year_month_day> aYMDs);
    */

private:
    //std::map<year_month_day,safdataOneDate> mVAllDates;

};

#endif // SAFRANDATA_H
