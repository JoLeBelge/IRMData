#include "mar.h"

std::vector vVarsSum={"MBRR","MBRO3","MBET","MBSL", "SHF","LHF"};
std::vector vVarsGXN={"TT","ST2"};
std::vector vVarsG={"SLQC"};

MAR::MAR(std::string aWd, bool fromDaily): mOutDaily(aWd+"/MAR-daily"),mOutMonthly(aWd+"/MAR-montly"),mWd(aWd),mOutMY(aWd+"/MAR-MultiY")
{

    std::cout << "gestion fichiers mar depuis le répertoire " << aWd << std::endl;
    if(!boost::filesystem::exists(mOutDaily)){boost::filesystem::create_directory(mOutDaily);}
    if(!boost::filesystem::exists(mOutMonthly)){boost::filesystem::create_directory(mOutMonthly);}
    if(!boost::filesystem::exists(mOutMY)){boost::filesystem::create_directory(mOutMY);}

    if(!boost::filesystem::exists(mOutMY+"/raster")){boost::filesystem::create_directory(mOutMY+"/raster");}



    boost::filesystem::directory_iterator end_itr;

    if (!fromDaily){
        for (boost::filesystem::directory_iterator itr(aWd); itr != end_itr; ++itr)
        {
            std::string s=itr->path().filename().string();
            if (itr->path().filename().extension().string()==".nc"){

                std::string y = s.substr(s.size()-7,4);
                mBaseName=s.substr(0,s.size()-7);
                std::cout << "année " << y << " fichier " << s << std::endl;
                std::cout << "baseName" << mBaseName<< std::endl;
                mYearNcdf.emplace(std::make_pair(std::stoi(y),s));
            }


        }
    } else {
        // j'ai deux ordinateur, un ou j'ai les données horaires, l'autre ou je n'ai copié que les données journalière - je veux un code polyvalent

        for (boost::filesystem::directory_iterator itr(mOutDaily); itr != end_itr; ++itr)
        {
            std::cout << "création depuis les fichiers daily"<< std::endl;
            std::string s=itr->path().filename().string();
            if (itr->path().filename().extension().string()==".nc"){

                std::string y = s.substr(s.size()-7,4);
                mBaseName=s.substr(0,s.size()-7-6);//-6 pour les caractères "-daily"
                std::cout << "année " << y << " fichier " << s << std::endl;


                s=mBaseName+y+".nc";
                 std::cout << "baseName" << mBaseName<< ", filename " << s << std::endl;
                mYearNcdf.emplace(std::make_pair(std::stoi(y),s));
            }


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

        if (boost::filesystem::exists(monthlyFile(kv.first))){
            boost::filesystem::remove(monthlyFile(kv.first));
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
            aCommand += monthlyFile(kv.first);

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
            std::string aIn=monthlyFile(kv.first);
            aCommand += " " + monthlyFile(kv.first);
        }
    }
    aCommand += " " + nameMultiY(y1,y2,"monthly");
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

void MAR::multiYStat(int y1,int y2){


    // moyenne trentenaire par mois
    std::string aCommand="cdo -ymonmean -selname,MBRR,TTG "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"G");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    // somme sur l'année
    aCommand="cdo -yearsum -selvar,MBRR " + nameMultiY(y1,y2,"G") + " " + nameMultiY(y1,y2,"MBRRS");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    // export au format raster
    aCommand="gdal_translate -of GTiff NETCDF:'"+ nameMultiY(y1,y2,"MBRRS")+"':MBRR "+ nameRastMultiY(y1,y2,"MBRRS");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    setGeoTMAR(nameRastMultiY(y1,y2,"MBRRS"));

    // moyenne sur l'année
    aCommand="cdo -yearmean -selvar,TTG " + nameMultiY(y1,y2,"G") + " " + nameMultiY(y1,y2,"TTG");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    // export au format raster
    aCommand="gdal_translate -of GTiff NETCDF:'"+ nameMultiY(y1,y2,"TTG")+"':TTG "+ nameRastMultiY(y1,y2,"TTG");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    setGeoTMAR(nameRastMultiY(y1,y2,"TTG"));

    // min et max pour TT
    aCommand="cdo -yearmin -ymonmin -selname,TTN "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"TTN");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    aCommand="cdo -yearmax -ymonmax -selname,TTX "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"TTX");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    aCommand="gdal_translate -of GTiff NETCDF:'"+ nameMultiY(y1,y2,"TTX")+"':TTX "+ nameRastMultiY(y1,y2,"TTX");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    setGeoTMAR(nameRastMultiY(y1,y2,"TTX"));
    aCommand="gdal_translate -of GTiff NETCDF:'"+ nameMultiY(y1,y2,"TTN")+"':TTN "+ nameRastMultiY(y1,y2,"TTN");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    setGeoTMAR(nameRastMultiY(y1,y2,"TTN"));

    // somme des précipitation de avril à septembre
    aCommand="cdo -yearsum -selmonth,4/9 -selvar,MBRR " + nameMultiY(y1,y2,"G") + " " + nameMultiY(y1,y2,"m4_7MBRRS");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    // export au format raster
    aCommand="gdal_translate -of GTiff NETCDF:'"+ nameMultiY(y1,y2,"m4_7MBRRS")+"':MBRR "+ nameRastMultiY(y1,y2,"m4_7MBRRS");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    setGeoTMAR(nameRastMultiY(y1,y2,"m4_7MBRRS"));

   // bilan hydrique estival :" MBRO3-MBET-MBSL" je commence avec cette approche car c'est les variables que j'ai sur mon portable
   // par contre pour une des variables j'ai les secteurs à choisir... c'est ce qui est nommé levels par cdo, donc sellevidx -sellevidx,1 ou alors vertmean. mais vu que c'est une moyenne pondérée par classe de végétation je dois utiliser la grille FRV

   // let VAR2 = (VAR[k=1]*FRV[k=1]+VAR[k=2]*FRV[k=2]+VAR[k=3]*FRV[k=3])/100
   //aCommand="cdo expr,'BHE=MBRO3-MBET-MBSL;' -vertmean -yearsum -selmonth,4/9 -ymonmean -selname,MBRO3,MBET,MBSL "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"BHE");

   aCommand="cdo -expr,'BHE=BHE/100;' -vertmean -expr,'BHE=MBRO3*FRV-MBET*FRV-MBSL*FRV;' -merge -yearsum -selmonth,4/9 -ymonmean -selname,MBRO3,MBET,MBSL "+ nameMultiY(y1,y2,"monthly")+" -selvar,FRV /home/jo/Documents/climat_MAR/data/ICE.195001.b01.nc "+ nameMultiY(y1,y2,"BHE");

   std::cout << aCommand << std::endl;
   system(aCommand.c_str());
   aCommand="gdal_translate -of GTiff NETCDF:'"+ nameMultiY(y1,y2,"BHE")+"':BHE "+ nameRastMultiY(y1,y2,"BHE");
   std::cout << aCommand << std::endl;
    system(aCommand.c_str());
   setGeoTMAR(nameRastMultiY(y1,y2,"BHE"));

    // création du tableau
   std::string aTable(mOutMY+"/table1990-2020ZBIO.csv");
   std::string zbioNc("/home/jo/app/climat/doc/zbio_MAR-SOP.nc");
   std::string delimCmd("echo ';' >>"+aTable);
    aCommand="echo 'ZBIO;MBRR;TTG;TTX;TTN;m4_7MBRR;BHE\n' >> "+aTable;
    system(aCommand.c_str());
   for (int zbio : {1,2,10}){
    aCommand="echo '"+std::to_string(zbio)+";' >> "+aTable;
    system(aCommand.c_str());
    aCommand="cdo -outputf,%8.6g,80 -fldmean -ifthen -eqc,"+std::to_string(zbio)+ " "+ zbioNc+ " "+nameMultiY(y1,y2,"MBRRS") + " >> " +aTable;
    system(aCommand.c_str());
    system(delimCmd.c_str());
    aCommand="cdo -outputf,%8.6g,80 -fldmean -ifthen -eqc,"+std::to_string(zbio)+ " "+ zbioNc+ " "+nameMultiY(y1,y2,"TTG") + " >> " +aTable;
    system(aCommand.c_str());
    system(delimCmd.c_str());
    aCommand="cdo -outputf,%8.6g,80 -fldmax -ifthen -eqc,"+std::to_string(zbio)+ " "+ zbioNc+ " "+nameMultiY(y1,y2,"TTX") + " >> " +aTable;
    system(aCommand.c_str());
    system(delimCmd.c_str());
    aCommand="cdo -outputf,%8.6g,80 -fldmin -ifthen -eqc,"+std::to_string(zbio)+ " "+ zbioNc+ " "+nameMultiY(y1,y2,"TTN") + " >> " +aTable;
    system(aCommand.c_str());
    system(delimCmd.c_str());
    aCommand="cdo -outputf,%8.6g,80 -fldmean -ifthen -eqc,"+std::to_string(zbio)+ " "+ zbioNc+ " "+nameMultiY(y1,y2,"m4_7MBRRS") + " >> " +aTable;
    system(aCommand.c_str());
    system(delimCmd.c_str());
    aCommand="cdo -outputf,%8.6g,80 -fldmean -ifthen -eqc,"+std::to_string(zbio)+ " "+ zbioNc+ " "+nameMultiY(y1,y2,"BHE") + " >> " +aTable;
    system(aCommand.c_str());
    aCommand="echo '\n' >> "+aTable;
     system(aCommand.c_str());
   }
}

std::string MAR::dailyFile(int y){
    return mOutDaily+"/"+mBaseName+"daily-"+std::to_string(y)+".nc";
}
std::string MAR::monthlyFile(int y){
    return mOutMonthly+"/"+mBaseName+"monthly-"+std::to_string(y)+".nc";
}
std::string MAR::nameMultiY(int y1,int y2, std::string post){
    return mOutMY+"/"+mBaseName+std::to_string(y1)+"-"+ std::to_string(y2)+post+".nc";
}
std::string MAR::nameRastMultiY(int y1,int y2, std::string post){
    return mOutMY+"/raster/"+mBaseName+std::to_string(y1)+"-"+ std::to_string(y2)+post+".tif";
}
