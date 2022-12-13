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



// une classe pour passer de l'horaire à du journalier puis du mensuel
class MAR
{
public:
    MAR(std::string aWd, std::string aZbioNc, typeGrid aGrid=SOP,bool fromDaily=0);

    void rechunk();// car cdo est extrèmement lent si les données ne sont pas découpées celon l'axe des timestep ; cas des netcdf de la grille IRM
    void hourly2daily();
    void daily2monthly();
    void multiY(int y1,int y2);
    void multiYStat(int y1,int y2);
     std::string nameMultiY(int y1,int y2, std::string post="");
     std::string nameRastMultiY(int y1,int y2, std::string post);
    std::string dailyFile(int y);
    std::string monthlyFile(int y);
    std::string chunkedFile(int y);
    std::map<int,std::string> mYearNcdf;
    std::string mOutDaily,mOutMonthly,mOutMY,mWd, mBaseName, zbioNc;
    bool mFromDaily;
    //,mOutChunk

    typeGrid mTypeGrid;

    //MY = multi-year
};

#endif // MAR_H
