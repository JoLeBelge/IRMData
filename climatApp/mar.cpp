#include "mar.h"

std::vector vVarsSum={"MBRR","MBRO3","MBET","MBSL", "SHF","LHF"};
std::vector vVarsGXN={"TT","ST2"};
std::vector vVarsG={"SLQC"};

MAR::MAR(std::string aWd): mOutDaily(aWd+"/MAR-daily"),mOutMonthly(aWd+"/MAR-montly"),mWd(aWd),mOutMY(aWd+"/MAR-MultiY")
{

    std::cout << "gestion fichiers mar depuis le répertoire " << aWd << std::endl;
    if(!boost::filesystem::exists(mOutDaily)){boost::filesystem::create_directory(mOutDaily);}
    if(!boost::filesystem::exists(mOutMonthly)){boost::filesystem::create_directory(mOutMonthly);}
    if(!boost::filesystem::exists(mOutMY)){boost::filesystem::create_directory(mOutMY);}

    boost::filesystem::directory_iterator end_itr;

    // cycle through the directory
    for (boost::filesystem::directory_iterator itr(aWd); itr != end_itr; ++itr)
    {
        std::string s=itr->path().filename().string();
        if (itr->path().filename().extension().string()==".nc"){

            std::string y = s.substr(s.size()-7,4);

            std::cout << "année " << y << " fichier " << s << std::endl;
            mYearNcdf.emplace(std::make_pair(std::stoi(y),s));
        }


    }




}

void MAR::hourly2daily(){

    for (auto kv : mYearNcdf){

        if (boost::filesystem::exists(dailyFile(kv.first))){
            boost::filesystem::remove(dailyFile(kv.first));
        }
        std::string aIn=mWd+"/"+kv.second;
        std::string aCommand="cdo merge "
                             "-selname,LON,LAT " +aIn + " ";

        std::string sub ="-daysum -selname";

        for (std::string var : vVarsSum){

            sub+=","+var;
        }

        sub+= " " + aIn + " ";
        aCommand +=sub;


        for (std::string var : vVarsGXN){
            aCommand+= "-daymean -setvar,"+var+"G -selname,"+var+" "+aIn +" ";
            aCommand+= "-daymax -setvar,"+var+"X -selname,"+var+" "+aIn+" ";
            aCommand+= "-daymin -setvar,"+var+"N -selname,"+var+" "+aIn+" ";
        }
        for (std::string var : vVarsG){
            aCommand+= "-daymean -selname,"+var+" "+aIn +" ";
        }

        aCommand += dailyFile(kv.first);

        std::cout << aCommand << std::endl;

        system(aCommand.c_str());
        //break;

    }

}


void MAR::daily2monthly(){

    for (auto kv : mYearNcdf){

        if (boost::filesystem::exists(montlyFile(kv.first))){
            boost::filesystem::remove(montlyFile(kv.first));
        }
        std::string aIn=dailyFile(kv.first);

        if (boost::filesystem::exists(dailyFile(kv.first))){

            std::string aCommand="cdo merge "
                                 "-selname,LON,LAT " +aIn + " ";

            std::string sub ="-monsum -selname";
            for (std::string var : vVarsSum){
                sub+=","+var;
            }

            sub+= " " + aIn + " ";
            aCommand +=sub;

            for (std::string var : vVarsGXN){
                aCommand+= "-monmean -selname,"+var+"G "+aIn +" ";
                aCommand+= "-monmax  -selname,"+var+"X "+aIn+" ";
                aCommand+= "-monmin  -selname,"+var+"N "+aIn+" ";
            }
            aCommand += montlyFile(kv.first);

            std::cout << aCommand << std::endl;
            system(aCommand.c_str());
            //break;

        }
        else {  std::cout << "daily to monthly : pas d'input pour y " << kv.first << std::endl;}

    }
}

void MAR::multiY(int y1,int y2){
    std::string aCommand="cdo cat ";
    for (auto kv : mYearNcdf){
        if (kv.first>y1 && kv.first < y2+1){
        std::string aIn=montlyFile(kv.first);
        aCommand += " " + montlyFile(kv.first);
        }
    }
    aCommand += " " + nameMultiY(y1,y2,"montly");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    aCommand="cdo cat ";
    for (auto kv : mYearNcdf){
        if (kv.first>y1 && kv.first < y2+1){
        std::string aIn=dailyFile(kv.first);
        aCommand += " " + dailyFile(kv.first);
        }
    }
    aCommand += " " + nameMultiY(y1,y2,"daily");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
}

std::string MAR::dailyFile(int y){
    return mOutDaily+"/MARv3.12.2-ERA5-daily-"+std::to_string(y)+".nc";
}
std::string MAR::montlyFile(int y){
    return mOutMonthly+"/MARv3.12.2-ERA5-montly-"+std::to_string(y)+".nc";
}
std::string MAR::nameMultiY(int y1,int y2, std::string post){
     return mOutMY+"/MARv3.12.2-ERA5-"+std::to_string(y1)+"-"+ std::to_string(y2)+post+".nc";
}
