#include "mar.h"

std::vector vVarsSum={"MBRR","MBRO3","MBET","MBSL", "SHF","LHF"};
std::vector vVarsGXN={"TT","ST2"};
std::vector vVarsG={"SLQC"};

MAR::MAR(std::string aWd, std::string aZbioNc, bool fromDaily): mOutDaily(aWd+"/MAR-daily"),mOutMonthly(aWd+"/MAR-monthly"),mWd(aWd),mOutMY(aWd+"/MAR-MultiY"),zbioNc(aZbioNc)
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

            // création depuis les fichiers hourly
            std::string s=itr->path().filename().string();
            if (itr->path().filename().extension().string()==".nc"){

                std::string y = s.substr(s.size()-7,4);
                mBaseName=s.substr(0,s.size()-7-7);// -7 pour -hourly
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

            for (std::string var : vVarsG){
                aCommand+= "-monmean -selname,"+var+" "+aIn +" ";
            }

            aCommand += monthlyFile(kv.first);

            std::cout << aCommand << std::endl;
            system(aCommand.c_str());
            //break;

        }
        else {  std::cout << "daily to monthly : pas d'input pour y " << kv.first << " input " << dailyFile(kv.first) <<  std::endl;}

    }
}

void MAR::multiY(int y1,int y2){

     std::cout << "MAR::multiY" << std::endl;
    std::string aCommand="cdo cat ";
    int nbY(0);
    for (auto kv : mYearNcdf){
        if (kv.first>y1-1 && kv.first < y2+1){
            std::string aIn=monthlyFile(kv.first);
            aCommand += " " + monthlyFile(kv.first);
            nbY++;
        }
    }
    std::cout << " nombre d'années : " << std::to_string(nbY) << std::endl;
    aCommand += " " + nameMultiY(y1,y2,"monthly");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    // pour le moment, juste pour TTG car sinon fait des fichiers trop volumineux qui font bugger cdo ydaymean, en tout cas si période = trentenaire
    aCommand="cdo cat ";
    for (auto kv : mYearNcdf){
        if (kv.first>y1-1 && kv.first < y2+1){
            std::string aIn=dailyFile(kv.first);
            aCommand += " -selvar,TTG " + dailyFile(kv.first);
        }
    }
    aCommand += " " + nameMultiY(y1,y2,"daily");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
}

void MAR::multiYStat(int y1,int y2){

    // bug sur TS de 30 ans. Le faire par petites étapes? par tranches de 10ans? non résoudre le prblem, en gardant la taille des fichiers pas trop élevée (moins de variables dedans)
    std::string aCommand="cdo -ydaymean "+ nameMultiY(y1,y2,"daily")+" "+ nameMultiY(y1,y2,"ydaymean");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    // moyenne trentenaire par mois
    aCommand="cdo -ymonmean -selname,MBRR,TTG "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"G");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    // somme sur l'année
    aCommand="cdo -yearsum -selvar,MBRR " + nameMultiY(y1,y2,"G") + " " + nameMultiY(y1,y2,"MBRRS");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    exportRaster(nameMultiY(y1,y2,"MBRRS")+"':MBRR",nameRastMultiY(y1,y2,"MBRRS"));

    // moyenne sur l'année
    aCommand="cdo -yearmean -selvar,TTG " + nameMultiY(y1,y2,"G") + " " + nameMultiY(y1,y2,"TTG");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    exportRaster(nameMultiY(y1,y2,"TTG")+"':TTG",nameRastMultiY(y1,y2,"TTG"));

    // min et max pour TT
    aCommand="cdo -yearmin -ymonmin -selname,TTN "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"TTN");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    aCommand="cdo -yearmax -ymonmax -selname,TTX "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"TTX");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    exportRaster(nameMultiY(y1,y2,"TTX")+"':TTN",nameRastMultiY(y1,y2,"TTX"));
    exportRaster(nameMultiY(y1,y2,"TTN")+"':TTN",nameRastMultiY(y1,y2,"TTN"));

    // somme des précipitation de avril à septembre
    aCommand="cdo -yearsum -selmonth,4/9 -selvar,MBRR " + nameMultiY(y1,y2,"G") + " " + nameMultiY(y1,y2,"m4_7MBRRS");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());

    // export au format raster
    exportRaster(nameMultiY(y1,y2,"m4_7MBRRS")+"':MBRR",nameRastMultiY(y1,y2,"m4_7MBRRS"));

   // bilan hydrique estival :" MBRO3-MBET-MBSL" je commence avec cette approche car c'est les variables que j'ai sur mon portable
   // par contre pour une des variables j'ai les secteurs à choisir... c'est ce qui est nommé levels par cdo, donc sellevidx -sellevidx,1 ou alors vertmean. mais vu que c'est une moyenne pondérée par classe de végétation je dois utiliser la grille FRV

   // let VAR2 = (VAR[k=1]*FRV[k=1]+VAR[k=2]*FRV[k=2]+VAR[k=3]*FRV[k=3])/100
   //aCommand="cdo expr,'BHE=MBRO3-MBET-MBSL;' -vertmean -yearsum -selmonth,4/9 -ymonmean -selname,MBRO3,MBET,MBSL "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"BHE");

   aCommand="cdo -expr,'BHE=BHE/100;' -vertmean -expr,'BHE=MBRO3*FRV-MBET*FRV-MBSL*FRV;' -merge -yearsum -selmonth,4/9 -ymonmean -selname,MBRO3,MBET,MBSL "+ nameMultiY(y1,y2,"monthly")+" -selvar,FRV "+ mWd+"/"+mYearNcdf.begin()->second +" "+  nameMultiY(y1,y2,"BHE");

   std::cout << aCommand << std::endl;
   system(aCommand.c_str());

   exportRaster(nameMultiY(y1,y2,"BHE")+"':BHE",nameRastMultiY(y1,y2,"BHE"));

   // suivi de l'humidité du sol comme indice de bilan hydrique estival
   aCommand="cdo -expr,'BHE2=BHE2/100;' -vertmean -expr,'BHE2=SLQC*FRV;' -merge -yearsum -selmonth,4/9 -ymonmean -selname,SLQC "+ nameMultiY(y1,y2,"monthly")+" -selvar,FRV "+ mWd+"/"+mYearNcdf.begin()->second +" "+  nameMultiY(y1,y2,"BHE2");
   std::cout << aCommand << std::endl;
   system(aCommand.c_str());
   exportRaster(nameMultiY(y1,y2,"BHE2")+"':BHE2",nameRastMultiY(y1,y2,"BHE2"));

   // growing season length
   aCommand="cdo -setvar,GSL -eca_gsl,6,8 -expr,'TG=TTG+274.15;' -vertmean -selvar,TTG "+nameMultiY(y1,y2,"ydaymean") +" -gec,0 "+zbioNc + " "+ nameMultiY(y1,y2,"GSL");
   std::cout << aCommand << std::endl;
   system(aCommand.c_str());
   exportRaster(nameMultiY(y1,y2,"GSL")+"':GSL",nameRastMultiY(y1,y2,"GSL"));

    // création du tableau
   std::string aTable(mOutMY+"/table"+std::to_string(y1)+"-"+std::to_string(y2)+"ZBIO.csv");

   std::ofstream ofs (aTable, std::ofstream::out);
    ofs << "ZBIO;MBRR;TTG;TTX;TTN;m4_7MBRR;BHE;GSL(6,8)\n";
   // system(aCommand.c_str());
    std::vector<std::string> vZbio={"HA et HCO", "HA", "HCO", "BMA","Oesling", "Gutland"};
   int j(0);
   for (std::string zbio : {"mask=(zbio==10)?1:(zbio==1);","mask=zbio==10;","mask=zbio==1;","mask=zbio==2;","mask=(zbio==11)+(zbio==12)+(zbio==13);","mask=(zbio==14)+(zbio==15)+(zbio==16)+(zbio==17);"}){

   // ofs <<std::to_string(zbio) << ";";
    //ofs <<"'"<<zbio << "';";
    ofs <<vZbio.at(j) << ";";
    aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"MBRRS") ;//+ " >> " +aTable;
    ofs <<exec(aCommand.c_str()) << ";";
    aCommand="cdo  -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"TTG") ;
    ofs <<exec(aCommand.c_str()) << ";";
    aCommand="cdo -s -outputf,%8.6g,80 -fldmax -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"TTX") ;
    ofs <<exec(aCommand.c_str()) << ";";
    aCommand="cdo -s -outputf,%8.6g,80 -fldmin -ifthen --expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"TTN") ;
    std::cout << aCommand << std::endl;
    ofs <<exec(aCommand.c_str()) << ";";
    aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"m4_7MBRRS") ;
    ofs <<exec(aCommand.c_str()) << ";";
    aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"BHE") ;
    ofs <<exec(aCommand.c_str()) << ";";
    // plusieur date parfois dans le netcdf, je prend la dernière 1990-12-31
    aCommand="cdo -s -outputf,%8.6g,80 -selvar,GSL -seldate,"+std::to_string(y2)+"-12-31 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"GSL") ;
    //std::cout << aCommand << std::endl;
    ofs <<exec(aCommand.c_str()) << "\n";
    j++;
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


std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    if (result.find('.')!=std::string::npos){
    //std::cout <<" result " <<  result << "\n\n" << std::endl;
        std::string r=result.substr(1,result.find('.'))+ result.substr(result.find('.')+1,2);
    if (result.find('-')!=std::string::npos && r.find('-')==std::string::npos){r="-"+r;}
    return  r;
    }
    else {return result;}
}
