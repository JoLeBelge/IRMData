#ifndef MAR_H
#define MAR_H

#include "boost/filesystem.hpp"
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <vector>
#include "utils.h"

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <netcdfcpp.h>

std::string exec(const char* cmd);

enum typeAggreg {sommeY
               ,meanY
               ,sommeYmon
               ,meanYmon
                   };




// une classe pour passer de l'horaire à du journalier puis du mensuel
class MAR
{
public:
    MAR(std::string aWd, std::string aZbioNc, typeGrid aGrid=SOP,bool fromDaily=0);

    void rechunk();// car cdo est extrèmement lent si les données ne sont pas découpées celon l'axe des timestep ; cas des netcdf de la grille IRM
    void hourly2daily(bool overwrite=0);
    void daily2monthly(bool overwrite=0);
    void multiY(int y1,int y2);
    void multiYStat(int y1,int y2);
    void multiYStatTable(int y1,int y2,std::string post="");

    void multiYCorrection(int y1, int y2, MAR * era5, MAR * GCMhisto);
    void varInterannuelle(int y1, int y2, std::string aVar, std::string aFileIn, std::string aVarOut, typeAggreg agr);
     std::string nameMultiY(int y1,int y2, std::string post="");
     std::string nameRastMultiY(int y1,int y2, std::string post);
    std::string dailyFile(int y);
    std::string monthlyFile(int y);
    std::string chunkedFile(int y);
    std::map<int,std::string> mYearNcdf;
    std::string mOutDaily,mOutMonthly,mOutMY,mWd, mBaseName, zbioNc;
    bool mFromDaily;
    //,mOutChunk

    void moyenneMobile();

    typeGrid mTypeGrid;

    //MY = multi-year
};

#endif // MAR_H
