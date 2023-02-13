#include "mar.h"

std::vector vVarsSum={"MBRR","MBRO3","MBET","MBSL", "SHF","LHF"};
std::vector vVarsGXN={"TT","ST2"};
std::vector vVarsG={"SLQC"};

std::string Tvar("TT");
std::string Pvar("MBRR");

MAR::MAR(std::string aWd, std::string aZbioNc, typeGrid aGrid, bool fromDaily): mOutDaily(aWd+"/MAR-daily"),mOutMonthly(aWd+"/MAR-monthly")
  ,mWd(aWd),mOutMY(aWd+"/MAR-MultiY")
  ,zbioNc(aZbioNc)
  ,mTypeGrid(aGrid)
  ,mFromDaily(fromDaily)
  // ,mOutChunk(aWd+"/Chunked")
{

    int nbCharToRm(0);
    if (mTypeGrid==SOP75){nbCharToRm=14;}
    if (mTypeGrid==SOP75){nbCharToRm=5;}// SOP75 avec ER5

    std::cout << "gestion fichiers mar depuis le répertoire " << aWd << std::endl;
    if(!boost::filesystem::exists(mOutDaily)){boost::filesystem::create_directory(mOutDaily);}
    if(!boost::filesystem::exists(mOutMonthly)){boost::filesystem::create_directory(mOutMonthly);}
    if(!boost::filesystem::exists(mOutMY)){boost::filesystem::create_directory(mOutMY);}
    if(!boost::filesystem::exists(mOutMY+"/raster")){boost::filesystem::create_directory(mOutMY+"/raster");}
    //  if(!boost::filesystem::exists(mOutChunk)){boost::filesystem::create_directory(mOutChunk);}

    boost::filesystem::directory_iterator end_itr;

    if (!fromDaily){
        for (boost::filesystem::directory_iterator itr(aWd); itr != end_itr; ++itr)
        {

            // création depuis les fichiers hourly
            std::string s=itr->path().filename().string();
            if (itr->path().filename().extension().string()==".nc"){

                int year;
                try
                {
                    if (mTypeGrid==SOP75){nbCharToRm=s.size()-13-3;}// -3 c'est pour .nc
                    std::string y = s.substr(s.size()-7-nbCharToRm,4);

                    std::string tmp;
                    tmp=s.substr(0,s.size()-7-nbCharToRm);

                    if (tmp.substr(tmp.size()-7,7)=="-hourly"){
                        tmp.substr(0, tmp.size()-7);
                    }
                    if (tmp.substr(tmp.size()-8,8)!="Chunked-"){

                        mBaseName=tmp;

                        // mBaseName=s.substr(0,s.size()-7-7);// -7 pour -hourly
                        std::cout << "année " << y << " fichier " << s << std::endl;
                        std::cout << "baseName  " << mBaseName<< std::endl;
                        year=std::stoi(y);
                        mYearNcdf.emplace(std::make_pair(year,s));
                    }

                }catch (...) {
                    // Block of code to handle errors
                }
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

    switch(mTypeGrid){
    case irm:{
        vVarsSum={"RF","RO3","ET","SL","SF"};
        vVarsGXN={"T2m","ST"};
        vVarsG={"SQC"};
        Tvar="T2m";
        Pvar="RF";

        //rechunk();// renomme aussi les fichiers de base, donc je dois le faire,- mais pas pour MAR 13
        break; // si pas le break, problème!
    }

    case SOP75:{
        vVarsSum={"RF","RO3","ET","SL"};
        vVarsGXN={"TT","ST"};
        vVarsG={"SLQ"};
        Tvar="TT";// varie avec atmlay, il y en a 3 dans ces fichiers là..
        Pvar="RF";
        // bon je n'ai pas de température minimum ou maximum dans ces sorties MAR. Donc ça vaut bien la peine. Qu'est-ce que cela implique?

        //mFromDaily=1;
        mFromDaily=0; // je vais recréer des daily et dupliquer la variable de températeur en X, N et G pour rendre compatible le code.
        break;
    }

    }

}

void MAR::rechunk(){

    std::cout << " réorganise les chunks sinon cdo va pas fonctionner .. " << std::endl;
    for (auto kv : mYearNcdf){
        if (!boost::filesystem::exists(mWd+"/"+chunkedFile(kv.first))){

            std::string aIn=mWd+"/"+kv.second;
            //std::string aCommand="nccopy -k nc4 -d 0 "+aIn +" "+chunkedFile(kv.first);

            std::string aCommand="ncks --cnk_dmn y,55 --cnk_dmn x,66 --cnk_dmn time,1 "+aIn +" "+mWd+"/" +chunkedFile(kv.first);
            std::cout << aCommand << std::endl;
            system(aCommand.c_str());

        }
        mYearNcdf.at(kv.first)=chunkedFile(kv.first);
    }
}

void MAR::hourly2daily(bool overwrite){

    for (auto kv : mYearNcdf){


        if (boost::filesystem::exists(dailyFile(kv.first)) & overwrite){
            boost::filesystem::remove(dailyFile(kv.first));
        }

        if (!boost::filesystem::exists(dailyFile(kv.first))){
            std::string aIn=mWd+"/"+kv.second;


        // check compression ; si compression, on décompresse.
        //const char * p=aIn.c_str();
        //NcFile in(p,NcFile::FileMode::ReadOnly,NULL,0,NcFile::FileFormat::Netcdf4);

        std::string aCommand="cdo merge "
                             "-selname,LON,LAT " +aIn + " ";

        std::string sub ="-daysum -selname";

        for (std::string var : vVarsSum){

            sub+=","+var;
        }

        sub+= " " + aIn + " ";
        aCommand +=sub;

        for (std::string var : vVarsG){
            aCommand+= "-daymean -selname,"+var+" "+aIn +" ";
        }


        // pour la grille en 7.5 ; c'est des input daily mais sans Tmax ou Tmin,donc je triche ici
        /*   switch(mTypeGrid){
        case SOP75:{



        }
        default :{*/

        for (std::string var : vVarsGXN){

            // températeur qui dépends de ATMLAY pour MAR 7.5 km ; je choisis le premier level vertical
            aCommand+= "-daymean -setvar,"+var+"G -sellevidx,1 -selname,"+var+" "+aIn +" ";
            aCommand+= "-daymax -setvar,"+var+"X -sellevidx,1 -selname,"+var+" "+aIn+" ";
            aCommand+= "-daymin -setvar,"+var+"N -sellevidx,1 -selname,"+var+" "+aIn+" ";
        }

        //  }
        //     }

        aCommand += dailyFile(kv.first);

        std::cout << aCommand << std::endl;

        system(aCommand.c_str());
        //break;
    }

}

}

void MAR::daily2monthly(bool overwrite){

    for (auto kv : mYearNcdf){

        if (boost::filesystem::exists(monthlyFile(kv.first)) & overwrite){
            boost::filesystem::remove(monthlyFile(kv.first));
        }

        if (!boost::filesystem::exists(monthlyFile(kv.first))){
        std::string aIn=dailyFile(kv.first);

        if (boost::filesystem::exists(dailyFile(kv.first))){
            // -b F64 sinon overflow pour ET , grille IRM
            std::string aCommand="cdo -b F64 merge "
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
            //  break;

        }
        else {  std::cout << "daily to monthly : pas d'input pour y " << kv.first << " input " << dailyFile(kv.first) <<  std::endl;}
        }

    }
}

void MAR::multiY(int y1,int y2){

    std::cout << "MAR::multiY" << std::endl;
    std::string aCommand="cdo copy ";
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

    // j'y ajoute TTN et TTX - non il rale,  cat (Abort): Input streams have different number of variables per timestep! sois-disant. attention, pas cat mais copy!

    aCommand="cdo copy ";
    for (auto kv : mYearNcdf){
        if (kv.first>y1-1 && kv.first < y2+1){
            std::string aIn=dailyFile(kv.first);
            aCommand += " -selvar,"+Tvar+"G " + dailyFile(kv.first);
        }
    }
    aCommand += " " + nameMultiY(y1,y2,"dailyTG");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    aCommand="cdo copy ";
    for (auto kv : mYearNcdf){
        if (kv.first>y1-1 && kv.first < y2+1){
            std::string aIn=dailyFile(kv.first);
            aCommand += " -selvar,"+Tvar+"N " + dailyFile(kv.first);
        }
    }
    aCommand += " " + nameMultiY(y1,y2,"dailyTN");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    aCommand="cdo copy ";
    for (auto kv : mYearNcdf){
        if (kv.first>y1-1 && kv.first < y2+1){
            std::string aIn=dailyFile(kv.first);
            aCommand += " -selvar,"+Tvar+"X " + dailyFile(kv.first);
        }
    }
    aCommand += " " + nameMultiY(y1,y2,"dailyTX");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
}

void MAR::multiYStat(int y1,int y2){
    std::string aCommand;
    bool compute(1);
    if (compute){
        // bug sur TS de 30 ans. Le faire par petites étapes? par tranches de 10ans? non résoudre le prblem, en gardant la taille des fichiers pas trop élevée (moins de variables dedans)
        aCommand="cdo -ydaymean "+ nameMultiY(y1,y2,"dailyTG")+" "+ nameMultiY(y1,y2,"ydaymeanTG");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        aCommand="cdo -ydaymean "+ nameMultiY(y1,y2,"dailyTX")+" "+ nameMultiY(y1,y2,"ydaymeanTX");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        aCommand="cdo -ydaymean "+ nameMultiY(y1,y2,"dailyTN")+" "+ nameMultiY(y1,y2,"ydaymeanTN");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        /* aCommand="cdo -ydaymin "+ nameMultiY(y1,y2,"daily")+" "+ nameMultiY(y1,y2,"ydaymin");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
   aCommand="cdo -ydaymax "+ nameMultiY(y1,y2,"daily")+" "+ nameMultiY(y1,y2,"ydaymax");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());*/

        // moyenne trentenaire par mois
        aCommand="cdo -ymonmean -selname,"+Pvar+","+Tvar+"G "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"G");
        std::cout << aCommand <<  "\n" <<std::endl;
        system(aCommand.c_str());

        // somme sur l'année
        aCommand="cdo -yearsum -selvar,"+Pvar+" " + nameMultiY(y1,y2,"G") + " " + nameMultiY(y1,y2,"MBRRS");
        std::cout << aCommand << "\n" << std::endl;
        system(aCommand.c_str());

        exportRaster( "'" +nameMultiY(y1,y2,"MBRRS")+"':"+Pvar,nameRastMultiY(y1,y2,"MBRRS"),mTypeGrid);

        // moyenne sur l'année
        aCommand="cdo -yearmean -selvar,"+Tvar+"G " + nameMultiY(y1,y2,"G") + " " + nameMultiY(y1,y2,"TTG");
        std::cout << aCommand << "\n" << std::endl;
        system(aCommand.c_str());
        exportRaster("'" +nameMultiY(y1,y2,"TTG")+"':"+Tvar+"G",nameRastMultiY(y1,y2,"TTG"),mTypeGrid);

        // min et max pour TT
        aCommand="cdo -yearmin -ymonmin -selname,"+Tvar+"N "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"TTN");
        std::cout << aCommand <<  "\n" << std::endl;
        system(aCommand.c_str());
        aCommand="cdo -yearmax -ymonmax -selname,"+Tvar+"X "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"TTX");
        std::cout << aCommand <<  "\n" <<std::endl;
        system(aCommand.c_str());

        exportRaster("'" +nameMultiY(y1,y2,"TTX")+"':"+Tvar+"X",nameRastMultiY(y1,y2,"TTX"),mTypeGrid);
        exportRaster("'" +nameMultiY(y1,y2,"TTN")+"':"+Tvar+"N",nameRastMultiY(y1,y2,"TTN"),mTypeGrid);


        // somme des précipitation de avril à septembre
        aCommand="cdo -yearsum -selmonth,4/9 -selvar,"+Pvar+" " + nameMultiY(y1,y2,"G") + " " + nameMultiY(y1,y2,"m4_9MBRRS");

        std::cout << " toto \n\n\n" << std::endl;
        std::cout << aCommand  <<  "\n" <<std::endl;
          std::cout << " toto \n\n\n" << std::endl;
        system(aCommand.c_str());

        // export au format raster
        exportRaster("'" +nameMultiY(y1,y2,"m4_9MBRRS")+"':"+Pvar+"",nameRastMultiY(y1,y2,"m4_9MBRRS"),mTypeGrid);

        // bilan hydrique estival :" MBRO3-MBET-MBSL" je commence avec cette approche car c'est les variables que j'ai sur mon portable
        // par contre pour une des variables j'ai les secteurs à choisir... c'est ce qui est nommé levels par cdo, donc sellevidx -sellevidx,1 ou alors vertmean. mais vu que c'est une moyenne pondérée par classe de végétation je dois utiliser la grille FRV

        // let VAR2 = (VAR[k=1]*FRV[k=1]+VAR[k=2]*FRV[k=2]+VAR[k=3]*FRV[k=3])/100
        //aCommand="cdo expr,'BHE=MBRO3-MBET-MBSL;' -vertmean -yearsum -selmonth,4/9 -ymonmean -selname,MBRO3,MBET,MBSL "+ nameMultiY(y1,y2,"monthly")+" "+ nameMultiY(y1,y2,"BHE");


        switch (mTypeGrid){
        case SOP: aCommand="cdo -expr,'BHE=BHE/100;' -vertmean -expr,'BHE=MBRO3*FRV-MBET*FRV-MBSL*FRV;' -merge -yearsum -selmonth,4/9 -ymonmean -selname,MBRO3,MBET,MBSL "+ nameMultiY(y1,y2,"monthly")+" -selvar,FRV "+ mWd+"/"+mYearNcdf.begin()->second +" "+  nameMultiY(y1,y2,"BHE"); break;
        case irm: aCommand="cdo -b F64 -expr,'BHE=RO3-ET-SL;' -yearsum -selmonth,4/9 -ymonmean -selname,RO3,ET,SL "+ nameMultiY(y1,y2,"monthly")+" "+  nameMultiY(y1,y2,"BHE"); break;
        case SOP75: aCommand="cdo -b F64 -expr,'BHE=RO3-ET-SL;' -yearsum -selmonth,4/9 -ymonmean -selname,RO3,ET,SL "+ nameMultiY(y1,y2,"monthly")+" "+  nameMultiY(y1,y2,"BHE");break;
        }
        std::cout << aCommand <<  "\n" << std::endl;
        system(aCommand.c_str());

        exportRaster("'" +nameMultiY(y1,y2,"BHE")+"':BHE",nameRastMultiY(y1,y2,"BHE"),mTypeGrid);

        // suivi de l'humidité du sol comme indice de bilan hydrique estival

        switch (mTypeGrid){
        case SOP:    aCommand="cdo -expr,'BHE2=BHE2/100;' -vertmean -expr,'BHE2=SLQC*FRV;' -merge -yearsum -selmonth,4/9 -ymonmean -selname,SLQC "+ nameMultiY(y1,y2,"monthly")+" -selvar,FRV "+ mWd+"/"+mYearNcdf.begin()->second +" "+  nameMultiY(y1,y2,"BHE2");break;
        case irm: aCommand="cdo -expr,'BHE2=SQC/6000;' -yearsum -selmonth,4/9 -ymonmean -selname,SQC "+ nameMultiY(y1,y2,"monthly")+" "+  nameMultiY(y1,y2,"BHE2");break;
            // pour une moyenne en tonne (unitée de base: kg d'humidité). Pourquoi 6000  --> 6 moi x 1000 kg par tonne
        case SOP75: aCommand="cdo -expr,'BHE2=SLQ/6000;' -yearsum -selmonth,4/9 -ymonmean -selname,SLQ "+ nameMultiY(y1,y2,"monthly")+" "+  nameMultiY(y1,y2,"BHE2");break;

        }
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        exportRaster("'" +nameMultiY(y1,y2,"BHE2")+"':BHE2",nameRastMultiY(y1,y2,"BHE2"),mTypeGrid);

        // growing season length
        aCommand="cdo -setvar,GSL -eca_gsl,6,8 -expr,'TG="+Tvar+"G+274.15;' -vertmean -selvar,"+Tvar+"G "+nameMultiY(y1,y2,"ydaymeanTG") +" -gec,0 "+zbioNc + " "+ nameMultiY(y1,y2,"GSL");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        exportRaster("'" +nameMultiY(y1,y2,"GSL")+"':GSL",nameRastMultiY(y1,y2,"GSL"),mTypeGrid);

        //nombre de jours de gel : on va pas fonctionner comme GSL, car sinon ça fait des résultas trop restrictifs
        //cdo copy -eca_fd -expr,'TN=T2mN+274.15;' /home/jo/Documents/climat_MAR/MAR-IRM/MAR-daily/MARv3.12-ERA5-UZhourly-daily-1980.nc -eca_fd -expr,'TN=T2mN+274.15;' /home/jo/Documents/climat_MAR/MAR-IRM/MAR-daily/MARv3.12-ERA5-UZhourly-daily-1981.nc tmp.nc
        aCommand="cdo -setvar,FD -eca_fd -expr,'TN="+Tvar+"N+274.15;' -vertmean -selvar,"+Tvar+"N "+nameMultiY(y1,y2,"ydaymeanTN") +" "+ nameMultiY(y1,y2,"FD");
        aCommand="cdo copy ";
        for (auto kv : mYearNcdf){
            if (kv.first>y1-1 && kv.first < y2+1){
                std::string aIn=dailyFile(kv.first);
                aCommand += " -setvar,FD -eca_fd -expr,'TN="+Tvar+"N+274.15;' " + dailyFile(kv.first);
            }
        }
        aCommand += " " + nameMultiY(y1,y2,"FD");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        // autant de bandes que d'année!
        exportRaster(nameMultiY(y1,y2,"FD")+":FD",nameRastMultiY(y1,y2,"FD"),mTypeGrid);

        // même chose mais sur température moyenne
        aCommand="cdo copy ";
        for (auto kv : mYearNcdf){
            if (kv.first>y1-1 && kv.first < y2+1){
                std::string aIn=dailyFile(kv.first);
                aCommand += " -setvar,FD -eca_fd -expr,'TN="+Tvar+"G+274.15;' " + dailyFile(kv.first);
            }
        }
        aCommand += " " + nameMultiY(y1,y2,"FDG");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        // autant de bandes que d'année!
        exportRaster(nameMultiY(y1,y2,"FDG")+":FD",nameRastMultiY(y1,y2,"FDG"),mTypeGrid);

        // nb jours qui dépassent un seuil max de température max
        aCommand="cdo copy ";
        for (auto kv : mYearNcdf){
            if (kv.first>y1-1 && kv.first < y2+1){
                std::string aIn=dailyFile(kv.first);
                aCommand += " -setvar,SD -eca_su,30 -expr,'TX="+Tvar+"X+274.15;' " + dailyFile(kv.first);
            }
        }
        aCommand += " " + nameMultiY(y1,y2,"SD30");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        // autant de bandes que d'année!
        exportRaster(nameMultiY(y1,y2,"SD30")+":SD",nameRastMultiY(y1,y2,"SD30"),mTypeGrid);

        aCommand="cdo copy ";
        for (auto kv : mYearNcdf){
            if (kv.first>y1-1 && kv.first < y2+1){
                std::string aIn=dailyFile(kv.first);
                aCommand += " -setvar,SD -eca_su,40 -expr,'TX="+Tvar+"X+274.15;' " + dailyFile(kv.first);
            }
        }
        aCommand += " " + nameMultiY(y1,y2,"SD40");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        // autant de bandes que d'année!
        exportRaster(nameMultiY(y1,y2,"SD40")+":SD",nameRastMultiY(y1,y2,"SD40"),mTypeGrid);


        // nb jours qui dépassent un seuil max de température moy ----------------------------
        aCommand="cdo copy ";
        for (auto kv : mYearNcdf){
            if (kv.first>y1-1 && kv.first < y2+1){
                std::string aIn=dailyFile(kv.first);
                aCommand += " -setvar,SD -eca_su,20 -expr,'TX="+Tvar+"G+274.15;' " + dailyFile(kv.first);
            }
        }
        aCommand += " " + nameMultiY(y1,y2,"SDG20");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        // autant de bandes que d'année!
        exportRaster(nameMultiY(y1,y2,"SDG20")+":SD",nameRastMultiY(y1,y2,"SDG20"),mTypeGrid);

        aCommand="cdo copy ";
        for (auto kv : mYearNcdf){
            if (kv.first>y1-1 && kv.first < y2+1){
                std::string aIn=dailyFile(kv.first);
                aCommand += " -setvar,SD -eca_su,25 -expr,'TX="+Tvar+"G+274.15;' " + dailyFile(kv.first);
            }
        }
        aCommand += " " + nameMultiY(y1,y2,"SDG25");
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());
        // autant de bandes que d'année!
        exportRaster(nameMultiY(y1,y2,"SDG25")+":SD",nameRastMultiY(y1,y2,"SDG25"),mTypeGrid);


        //eca_r10mm Heavy precipitation days
        aCommand="cdo copy ";
        for (auto kv : mYearNcdf){
            if (kv.first>y1-1 && kv.first < y2+1){
                std::string aIn=dailyFile(kv.first);
                aCommand += " -setvar,HPD -eca_r10mm -selvar,"+Pvar+ " " + dailyFile(kv.first);
            }
        }
        aCommand += " " + nameMultiY(y1,y2,"HPD");
        system(aCommand.c_str());
        // autant de bandes que d'année!
        exportRaster(nameMultiY(y1,y2,"HPD")+":HPD",nameRastMultiY(y1,y2,"HPD"),mTypeGrid);

    }

    // création du tableau
    if (1){
        std::string aTable(mOutMY+"/table"+std::to_string(y1)+"-"+std::to_string(y2)+"ZBIO.csv");

        std::ofstream ofs (aTable, std::ofstream::out);
        ofs << "ZBIO;MBRR;TTG;TTX;TTN;m4_9MBRR;BHE;BHE2;GSL(6,8);SD30;SD40;SDG20;SDG25;FD;FDG;HPD\n";
        // system(aCommand.c_str());
        std::vector<std::string> vZbio={"Ardenne","HA et HCO", "HA", "HCO", "BMA","Oesling", "Gutland"};
        int j(0);
        for (std::string zbio : {"mask=(zbio==10)+(zbio==1)+(zbio==2);","mask=(zbio==10)?1:(zbio==1);","mask=zbio==10;","mask=zbio==1;","mask=zbio==2;","mask=(zbio==11)+(zbio==12)+(zbio==13);","mask=(zbio==14)+(zbio==15)+(zbio==16)+(zbio==17);"}){

            // ofs <<std::to_string(zbio) << ";";
            //ofs <<"'"<<zbio << "';";
            ofs <<vZbio.at(j) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"MBRRS") ;//+ " >> " +aTable;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo  -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"TTG") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmax -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"TTX") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmin -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"TTN") ;
            std::cout << aCommand << std::endl;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"m4_9MBRRS") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"BHE") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"BHE2") ;
            ofs <<exec(aCommand.c_str()) << ";";
            // plusieur date parfois dans le netcdf, je prend la dernière 1990-12-31
            aCommand="cdo -s -outputf,%8.6g,80 -selvar,GSL -seldate,"+std::to_string(y2)+"-12-31 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " "+nameMultiY(y1,y2,"GSL") ;
            //std::cout << aCommand << std::endl;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " -timmean "+nameMultiY(y1,y2,"SD30") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " -timmean "+nameMultiY(y1,y2,"SD40") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " -timmean "+nameMultiY(y1,y2,"SDG20") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " -timmean "+nameMultiY(y1,y2,"SDG25") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " -timmean "+nameMultiY(y1,y2,"FD") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " -timmean "+nameMultiY(y1,y2,"FDG") ;
            ofs <<exec(aCommand.c_str()) << ";";
            aCommand="cdo -s -outputf,%8.6g,80 -fldmean -ifthen -expr,'"+zbio+ "' "+ zbioNc+ " -timmean "+nameMultiY(y1,y2,"HPD") ;
            ofs <<exec(aCommand.c_str()) << "\n";
            j++;
        }
    }

}

std::string MAR::dailyFile(int y){

    std::string aRes=mOutDaily+"/"+mBaseName+"daily-"+std::to_string(y)+".nc";
    if(mFromDaily){ aRes=mWd+"/"+mYearNcdf.at(y);}
    return aRes;
}

std::string MAR::chunkedFile(int y){
    return mBaseName+"Chunked-"+std::to_string(y)+".nc";
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
    boost::erase_all(result, " ");
    boost::erase_all(result, "\n");


    if (result.find('.')!=std::string::npos){
        //std::cout <<" result " <<  result << "\n\n" << std::endl;
        std::string r=result.substr(0,result.find('.'))+ result.substr(result.find('.'),3);
        //if (result.find('-')!=std::string::npos && r.find('-')==std::string::npos){r="-"+r;}
        return  r;
    }


    else {return result;}
}
