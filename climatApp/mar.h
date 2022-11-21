#ifndef MAR_H
#define MAR_H

#include "boost/filesystem.hpp"
#include <string>
#include <iostream>
#include <vector>

// une classe pour passer de l'horaire à du journalier puis du mensuel
class MAR
{
public:
    MAR(std::string aWd);

    void hourly2daily();
    void daily2monthly();
    void multiY(int y1,int y2);
     std::string nameMultiY(int y1,int y2, std::string post="");
    std::string dailyFile(int y);
    std::string montlyFile(int y);
    std::map<int,std::string> mYearNcdf;
    std::string mOutDaily,mOutMonthly,mOutMY,mWd;

    //MY = multi-year
};

#endif // MAR_H
