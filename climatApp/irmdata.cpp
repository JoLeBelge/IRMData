#include "irmdata.h"
std::vector<int> vYears={2016,2017,2018,2019,2020}; // pour les données IRM j'ajoute l'année 2020, mais pour ECAD je pense que je n'ai pas les données 2020
std::vector<int> vMonths={3,4,5,6,7,8,9};

extern double baseDJ;
extern std::string input2;
extern std::string pathOut;
// la colonne dans laquelle est stoqué l'info - varie d'un fichier à un autre.
int colTmean(0),colTmax(0),colTmin(0),colR(0),colETP(0),colP(0), colWS(0),colP2(0),colRH(0);

year_month_day baseymd = 1950_y/1/1;

double NoD(-999);

irmData::irmData(std::string aFileIRM):mInPutFile(aFileIRM)
{
    std::cout << " création du catalogue irm " << std::endl;
    std::vector<std::vector<std::string>> d=parseCSV2V_quick(aFileIRM); // un peu plus rapide
    //std::vector<std::vector<std::string>> d=parseCSV2V(aFileIRM,';');
    std::cout << " fichier texte parsé " << std::endl;

    // détermine les numéro de colonnes pour les variables
    std::vector<std::string> headerline=d.at(0);
    for (int c(0);c<headerline.size();c++){
        std::string header=headerline.at(c);
        std::cout << header << " est la colonne " << c << std::endl;
        if (header.find("GLOBAL RADIATION")!=std::string::npos){colR=c; mVVars.push_back(std::make_pair("R",std::make_pair("QQ","GLOBAL RADIATION (kWh/m2/day)")));}
        if (header.find("TEMPERATURE AVG")!=std::string::npos){colTmean=c;mVVars.push_back(std::make_pair("Tmean",std::make_pair("TG","TEMPERATURE AVG")));}
        if (header.find("TEMPERATURE MAX")!=std::string::npos){colTmax=c;mVVars.push_back(std::make_pair("Tmax",std::make_pair("TX","TEMPERATURE MAX")));}
        if (header.find("TEMPERATURE MIN")!=std::string::npos){colTmin=c;mVVars.push_back(std::make_pair("Tmin",std::make_pair("TN","TEMPERATURE MIN")));}
        //if (header=="PRESSURE (hPa)"){colP=c;}
        if (header.find("PRECIPITATION (mm)")!=std::string::npos){colP=c;mVVars.push_back(std::make_pair("P",std::make_pair("RF","PRECIPITATION (mm)")));}
        if (header.find("ET")!=std::string::npos){colETP=c;mVVars.push_back(std::make_pair("ETP",std::make_pair("ETP", "Evapotranspiration Potentielle ET0 ")));}
        if (header.find("WIND SPEED")!=std::string::npos){colWS=c;mVVars.push_back(std::make_pair("WS",std::make_pair("WS","WIND SPEED (m/s)")));}
        if (header.find("RELATIVE HUMIDITY")!=std::string::npos){colRH=c;mVVars.push_back(std::make_pair("RH",std::make_pair("RH","RELATIVE HUMIDITY (%)")));}

    }
    std::cout << "Radiation colonne " << colR << " T mean colonne " << colTmean << " T max colonne " << colTmax << " T min colonne " << colTmin << "\n" << " Précipitation colonne " << colP << " ET0 colonne " << colETP << " , Wind speed colonne " << colWS << std::endl;

    std::string curDate("");
    for (int c(1); c<d.size();c++){
        std::vector<std::string> line=d.at(c);
        std::string aDate=line.at(0);
        aDate.erase(boost::remove_if(aDate, boost::is_any_of("\"")), aDate.end());

        // fonctionnement polyvalent si moyenne trentenaire - en fonction de la taille du champ date il définit le mode de lecture des données
        int d(1),m(1),y(1);
        //std::cout << "aDate " << aDate << std::endl;
        if (aDate.size()==5){
            d=std::stoi(aDate.substr(3,5));
            m=std::stoi(aDate.substr(0,2));
            y=1991;
            //std::cout << " day " << d << " , " << " month " << m << std::endl;
        }else{
            // une nouvelle date
            d=std::stoi(aDate.substr(8,9));
            m=std::stoi(aDate.substr(5,6));
            y=std::stoi(aDate.substr(0,4));
        }

        year_month_day ymd(year{y},month{m},day{d});
        if ( curDate!=aDate){
            //std::cout << " création des données irm pour une date , y " << y << " m " << m << " d " << d << std::endl;
            mVAllDates.emplace(std::make_pair(ymd,dataOneDate(ymd)));
            mVAllDates.at(ymd).addOnePix(line);
            curDate=aDate;
        } else {
            mVAllDates.at(ymd).addOnePix(line);
        }
    }
}

dataOneDate irmData::dataMensuel(year y, month m){
    std::vector<dataOneDate*> aVD;

    // selection des dates qui font parties du mois
    for (auto & kv : mVAllDates){
        year_month_day curYmd=kv.first;
        if (curYmd.year()==y && curYmd.month()==m){
            aVD.push_back(& kv.second);
        }
    }
    // création d'un dataOneDate pour y mettre les valeurs mensuelles
    year_month_day ymd(y,m,day{1});
    dataOneDate mensuel(ymd);
    for ( dataOneDate * dod : aVD){
        // je dois faire l'addition pour tout les pixels

        //  La méthode ne fonctionne pas pour sélectionner le maximum ou le minimum.
        mensuel.addOneDate(dod);
    }
    // pour avoir la moyenne ; je divise par le nombre d'observations.
    mensuel.divide(aVD.size());




    return mensuel;
}

dataOneDate irmData::dataAnnuel(year y){
    std::vector<dataOneDate*> aVD;

    // selection des dates qui font parties du mois
    for (auto & kv : mVAllDates){
        year_month_day curYmd=kv.first;
        if (curYmd.year()==y){
            aVD.push_back(& kv.second);
        }
    }
    // création d'un dataOneDate pour y mettre les valeurs mensuelles
    year_month_day ymd(y,month{1},day{1});
    dataOneDate annuel(ymd);
    for ( dataOneDate * dod : aVD){
        // je dois faire l'addition pour tout les pixels
        annuel.addOneDate(dod);
    }

    std::cout << " calcul valeur annuelle ; nombre de jours est de " << aVD.size() << std::endl;
    // pour avoir la moyenne ; je divise par le nombre d'observations.
    //annuel.divide(aVD.size());
    // je veux P et ETP donc je ne divise pas car je souhaite la somme annuelle
    return annuel;
}

dataOneDate irmData::moyAll(){
    std::vector<dataOneDate*> aVD;


    for (auto & kv : mVAllDates){
        aVD.push_back(& kv.second);
    }
    // création d'un dataOneDate pour y mettre les valeurs mensuelles
    year_month_day ymd(year{1},month{1},day{1});
    dataOneDate mensuel(ymd);
    for ( dataOneDate * dod : aVD){
        // je dois faire l'addition pour tout les pixels
        mensuel.addOneDate(dod);
    }
    // pour avoir la moyenne ; je divise par le nombre d'observations.
    mensuel.divide(aVD.size());
    return mensuel;
}

dataOneDate irmData::calculCarteMensuelTrentenaire(year y1, year y2, month m){
    std::vector<dataOneDate*> aVD;
    // selection des dates qui font parties du mois
    for (auto & kv : mVAllDates){
        year_month_day curYmd=kv.first;
        if (curYmd.month()==m){
            aVD.push_back(& kv.second);
        }
    }
    // création d'un dataOneDate pour y mettre les valeurs mensuelles
    year_month_day ymd(year{1},month{m},day{1});
    dataOneDate mensuel(ymd);
    for ( dataOneDate * dod : aVD){
        // je dois faire l'addition pour tout les pixels
        mensuel.addOneDate(dod);
    }
    int nby =((int) y2-(int)y1)+1;

    std::cout << "number of year " << nby << std::endl;
    // pour avoir la moyenne ; je divise par le nombre d'observations.
    // mais c'est faux pour les variables pour lesquelles je veux la somme ; reste à diviser par le nombre de mois
    mensuel.divide(aVD.size(),nby);
    return mensuel;

}


dataOneDate irmData::dataMensuelDJ(year y, month m){
    std::vector<dataOneDate*> aVD;

    // selection de toutes les dates qui sont antérieure dans l'année et qui font partie du mois
    for (auto & kv : mVAllDates){
        year_month_day curYmd=kv.first;
        if (curYmd.year()==y && curYmd.month()<=m){
            aVD.push_back(& kv.second);
        }
    }
    std::cout << " irmData::dataDJMensuel " << y << " , " <<m <<" , nombre de dates ;" << aVD.size() << std::endl;
    // création d'un dataOneDate pour y mettre les valeurs mensuelles
    year_month_day ymd(y,m,day{1});
    dataOneDate mensuel(ymd);

    for ( dataOneDate * dod : aVD){
        // je dois faire l'addition pour tout les pixels
        mensuel.addOneDateDJ(dod,baseDJ);
    }

    return mensuel;
}

dataOneDate irmData::getMax(year y, month m, bool allY){
    std::vector<dataOneDate*> aVD;

    for (auto & kv : mVAllDates){
        year_month_day curYmd=kv.first;
        // selection de toutes les dates qui sont antérieure dans l'année et qui font partie du mois
        if (allY){
            if (curYmd.year()==y && curYmd.month()<=m){
                aVD.push_back(& kv.second);
            }
        }else{
            // selection de toutes les dates de ce mois uniquement (maximum mensuel donc)
            if (curYmd.year()==y && curYmd.month()==m){
                aVD.push_back(& kv.second);
            }
        }
    }
    std::cout << " irmData::getMax " << y << " , " <<m <<" , nombre de dates ;" << aVD.size() << std::endl;
    // création d'un dataOneDate pour y mettre les valeurs maximum
    year_month_day ymd(y,m,day{1});
    dataOneDate max(ymd);

    for ( dataOneDate * dod : aVD){
        // je dois faire l'addition pour tout les pixels
        max.getMax(dod);
    }

    return max;
}


dataOneDate irmData::getMax(std::vector<year_month_day> aYMDs){

    std::vector<dataOneDate*> aVD;

    for (year_month_day ymd : aYMDs){
        if (mVAllDates.find(ymd)!=mVAllDates.end()){

            aVD.push_back(& mVAllDates.at(ymd));

        }else{
            std::cout << " attention, je n'ai pas trouvé la date " << ymd << " dans le catalogue irmData " << std::endl;
        }
    }
    std::cout << " irmData::getMax pour un vecteur de dates, nombre de dates trouvées;" << aVD.size() << std::endl;
    // création d'un dataOneDate pour y mettre les valeurs maximum
    year_month_day ymd(aYMDs.at(0));
    dataOneDate max(ymd);

    for ( dataOneDate * dod : aVD){
        // je dois faire l'addition pour tout les pixels
        max.getMax(dod);
    }

    return max;
}

void dataOneDate::addOneDate(dataOneDate * dod){
    for (auto & kv : dod->mVData){
        if (mVData.find(kv.first)!=mVData.end()){
            mVData.at(kv.first).addOneDate(&kv.second);
        } else {
            mVData.emplace(kv);
        }
    }
}

void dataOneDate::addOneDateDJ(dataOneDate * dod, double aSeuilDJ){
    for (auto & kv : dod->mVData){
        if (mVData.find(kv.first)!=mVData.end()){
            mVData.at(kv.first).addOneDateDJ(&kv.second,aSeuilDJ);
        } else {
            // creation d'un nouveau dataOnePix
            dataOnePix djDop(&kv.second,aSeuilDJ);
            mVData.emplace(std::make_pair(kv.first,djDop));
        }
    }
}

void dataOneDate::getMax(dataOneDate * dod){
    for (auto & kv : dod->mVData){
        if (mVData.find(kv.first)!=mVData.end()){

            dataOnePix dop = kv.second;

            if (dop.Tmax>mVData.at(kv.first).Tmax){
                mVData.at(kv.first).Tmax=dop.Tmax;
            }

            if (dop.Tmean>mVData.at(kv.first).Tmean){
                mVData.at(kv.first).Tmean=dop.Tmean;
            }

            if (dop.Tmin>mVData.at(kv.first).Tmin){
                mVData.at(kv.first).Tmin=dop.Tmin;
            }
            if (dop.WS>mVData.at(kv.first).WS){
                mVData.at(kv.first).WS=dop.WS;
            }

        } else {
            // creation d'un nouveau dataOnePix
            mVData.emplace(std::make_pair(kv.first,kv.second));
        }
    }
}

dataOneDate::dataOneDate(year_month_day ymd){
    //std::cout << " création des données irm pour une date , " << ymd << std::endl;
    mDate=ymd;
}

void dataOneDate::addOnePix(std::vector<std::string> & aLigne){

    mVData.emplace(std::make_pair(std::stoi(aLigne.at(1)),dataOnePix(aLigne)));
}

void dataOneDate::addOnePix(std::vector<std::string> & aLigne, int id){
    mVData.emplace(std::make_pair(id,dataOnePix(aLigne)));
}

void dataOneDate::divide(int nb, int nbMois){
    for (auto & kv : mVData){
        kv.second.divide(nb,nbMois);
    }
}

void dataOneDate::exportMap(std::string aOut,std::string aVar){
    // ouverture du template, copie de la carte, remplacer chaque pixel_id par sa valeur.
    const char *pszFormat = "GTiff";
    GDALDriver *pDriver= GetGDALDriverManager()->GetDriverByName(pszFormat);

    GDALDataset * temp=(GDALDataset *) GDALOpen( input2.c_str(), GA_ReadOnly );
    std::string out=pathOut+"/"+aOut+".tif";
    const char * ch=out.c_str();

    // créer une copie ; pas la bonne démarche, car c'est du 8 bits le template et moi je dois stoquer en float 32
    // histoire d'aller très vite, je crée un template en float 32 et je l'ouvre en // au pixel_id.tif
    GDALDataset * DSRes= pDriver->CreateCopy( ch,temp,FALSE, NULL,NULL, NULL );
    GDALClose( temp );

    int x=DSRes->GetRasterBand(1)->GetXSize();
    int y=DSRes->GetRasterBand(1)->GetYSize();

    float *scanline;
    scanline = (float *) CPLMalloc( sizeof( float ) * x );

    // boucle sur les pixels
    for ( int row = 0; row < y; row++ )
    {
        DSRes->GetRasterBand(1)->RasterIO( GF_Read, 0, row, x, 1, scanline, x,1, GDT_Float32, 0, 0 );

        // iterate on pixels in row
        for (int col = 0; col < x; col++)
        {
            if ( scanline[ col ] !=0){
                scanline[ col ] = getValForPix(scanline[col],aVar);
            }
        }

        DSRes->GetRasterBand(1)->RasterIO( GF_Write, 0, row, x, 1, scanline, x, 1,GDT_Float32, 0, 0 );
    }
    CPLFree(scanline);
    if( DSRes != NULL ){ GDALClose( DSRes );}

}

double dataOneDate::getValForPix(int pixel_id,std::string aVar){
    double aRes(-999);
    if (mVData.find(pixel_id)!=mVData.end()){
        if (aVar=="Tmean"){
            aRes=mVData.at(pixel_id).Tmean;
        } else if (aVar=="Tmax"){
            aRes=mVData.at(pixel_id).Tmax;
        } else if (aVar=="Tmin"){
            aRes=mVData.at(pixel_id).Tmin;
        } else if (aVar=="Tmean"){
            aRes=mVData.at(pixel_id).Tmean;
        } else if (aVar=="ETP"){
            aRes=mVData.at(pixel_id).ETP;
        } else if (aVar=="R"){
            aRes=mVData.at(pixel_id).R;
        } else if (aVar=="P"){
            aRes=mVData.at(pixel_id).P;
        } else if (aVar=="WS"){
            aRes=mVData.at(pixel_id).WS;
        } else if (aVar=="TminMin"){
            aRes=mVData.at(pixel_id).TminMin;
        }
    }
    return aRes;
}

void dataOnePix::divide(int nb, int nbMois){
    if (nb!=0){
        Tmean/=nb;
        Tmax/=nb;
        Tmin/=nb;
        P/=nbMois;
        R/=nb;
        WS/=nb;
        ETP/=nb;
    }
}

dataOnePix::dataOnePix(std::vector<std::string> & aLigne):Tmean(NoD),Tmax(NoD),Tmin(NoD),R(NoD),ETP(NoD),P(NoD),WS(NoD),TminMin(100),RelHumid(0){
    if(colTmean!=0){try{Tmean=std::stod(aLigne.at(colTmean));}catch (...) {}}
    if(colTmax!=0){try{Tmax=std::stod(aLigne.at(colTmax));}catch (...) {}}
    if(colTmin!=0){try{Tmin=std::stod(aLigne.at(colTmin));TminMin=Tmin;}catch (...) {}}
    if(colR!=0){try{R=std::stod(aLigne.at(colR));}catch (...) {}}
    if(colP!=0 && colP2==0){try{P=std::stod(aLigne.at(colP));}catch (...) {}}
    // précipitation liquide et neige , safran
    if(colP!=0 && colP2!=0){try{P=std::stod(aLigne.at(colP))+std::stod(aLigne.at(colP2));}catch (...) {}}

    if(colETP!=0){try{ETP=std::stod(aLigne.at(colETP));}catch (...) {}}
    if(colWS!=0){try{WS=std::stod(aLigne.at(colWS));}catch (...) {}}
    if(colRH!=0){try{RelHumid=std::stod(aLigne.at(colRH));}catch (...) {}}

}

void dataOnePix::addOneDate(dataOnePix * dop){
    Tmean+=dop->Tmean;
    Tmax+=dop->Tmax;
    if (TminMin>dop->Tmin){TminMin=dop->Tmin;}
    Tmin+=dop->Tmin;
    P+=dop->P;
    R+=dop->R;
    ETP+=dop->ETP;
    WS+=dop->WS;
}

dataOnePix::dataOnePix(dataOnePix * dop,double aSeuilDJ){
    double DJTmean=dop->Tmean-aSeuilDJ;
    if (DJTmean<0){DJTmean=0;}
    Tmean=dop->Tmean;
    Tmax=0;
    Tmin=0;
    P=0;
    R=0;
    ETP=0;
    WS=0;
}


void dataOnePix::addOneDateDJ(dataOnePix * dop, double aSeuilDJ){
    double DJTmean=dop->Tmean-aSeuilDJ;
    if (DJTmean>0){
        Tmean+=DJTmean;
    }
}

void irmData::saveNetCDF(std::string aOut){
    int NX(66),NY(55);
    std::string aOutTmp=aOut+"-tmp.nc";

    // GRILLE IRM ---------------------------------
    // ouvrir la grille IRM qui me sert de template (j'ai l'id du pixel dedans) -- Attention, il y a un effet de bord et plusieurs pixels ont le même id! à corriger assez vite
    std::string grillepath("/home/jo/app/climat/doc/grilleIRMGDL.nc");
    NcFile grille(grillepath.c_str(),NcFile::FileMode::ReadOnly,NULL,0,NcFile::FileFormat::Netcdf4);
    float var_in[NY][NX];

    // CREATION NOUVEAU NETCDF  ---------------------------------
    int NTIME(mVAllDates.size());
    std::cout << "total de " << NTIME << " TIMES " << std::endl;

    NcFile ncOut(aOutTmp.c_str(),NcFile::FileMode::Replace);

    ncOut.add_att("institution","Ulg - Gembloux Agro-Bio Tech");
    std::string source="IRM -gridded observations from request "+mInPutFile;
    ncOut.add_att("data_source",source.c_str());
    ncOut.add_att("gridXY","5km IRM Grid enlarged to include Grand-Duché de Luxembourg");
    ncOut.add_att("srcGridXY","+proj=lcc +lat_2=50.569898649999999 +lat_1=50.569898649999999 +lon_0=4.553615160000000 +units=m +no_defs +a=6371229.0 +es=0.0");
    ncOut.add_att("contact","Lisein Jonathan - liseinjon@hotmail.com");
    NcDim * dimTime = ncOut.add_dim("TIME");
    ncOut.add_dim("bnds",2);
    //grille de l'IRM
    NcDim * dimX = ncOut.add_dim("X",NX);
    NcDim * dimY = ncOut.add_dim("Y",NY);
    // variable TIME (pas dimension mais variable associée)
    NcVar* varTime  = ncOut.add_var("TIME",ncFloat,dimTime);
    // gdal_translate ne parviens pas à comprendre que ma variable time contient les date de la dimension time. mais cdo comprend bien, c'est ce qui compte
    varTime->add_att("units","days since 1950-01-01 00:00:00");
    varTime->add_att("standard_name","time");
    varTime->add_att("time_origin","01-JAN-1950 00:00:00");
    varTime->add_att("bounds","TIME_bnds"); // https://daac.ornl.gov/submit/netcdfrequirements/
    ncOut.add_var("TIME_bnds",ncFloat,dimTime);

    // celles là je les colles tels quels, en espérant que ce soit bon.
    NcVar *idVar=grille.get_var("LON");
    if (!idVar->get(&var_in[0][0], 1, NY, NX)){
        std::cout << "getvarin bad\n" << std::endl;
    }
    NcVar* varLON  = ncOut.add_var("LON",ncDouble,dimY,dimX);
    varLON->put(&var_in[0][0],NY,NX);

    idVar=grille.get_var("LAT");
    if (!idVar->get(&var_in[0][0], 1, NY, NX)){
        std::cout << "getvarin bad\n" << std::endl;
    }
    NcVar* varLAT  = ncOut.add_var("LAT",ncDouble,dimY,dimX);
    varLAT->put(&var_in[0][0],NY,NX);

    idVar=grille.get_var("ID");

    if (!idVar->get(&var_in[0][0], 1, NY, NX)){
        std::cout << "getvarin bad\n" << std::endl;
    }
    NcVar* varPixID  = ncOut.add_var("PIXEL_ID",ncInt,dimY,dimX);
    varPixID->put(&var_in[0][0],NY,NX);
    varPixID->add_att("name","identifiant du pixel de 5kmx5km de la grille de l'IRM");
    varPixID->add_att("_FillValue",0);

    float timebuf[NTIME];
    int t(0);
    for (auto & kv : mVAllDates){
        year_month_day ymd = kv.first;
        days d=(sys_days{ymd}-sys_days{baseymd});
        //std::cout <<"jour " <<  d.count()<< std::endl;
        timebuf[t]=d.count();
        t++;
    }
    varTime->put(&timebuf[0],NTIME);

    // create and populate the variables
    for (auto & pair : mVVars){
        std::string aVar =pair.first;
        std::cout << "ajout variables " << aVar << std::endl;
        NcVar* myvar  = ncOut.add_var(pair.second.first.c_str(),ncDouble,dimTime,dimY,dimX);
        myvar->add_att("nom",pair.second.second.c_str());

        // OLD - je n'ai toujour pas de solution pour une gestion propre des no data. Pour le moment ; tout les ND sont à -999 mais cdo les prends comme des données normale pour ses calcul
        // en fait ncdump comprends bien que ce sont des no data, et cdo aussi. c'est juste que le outputf affiche les nd comme étant -999, ça m'avais perturbé
        myvar->add_att("_FillValue",-999.0);
        //myvar->add_att("valid_min",-100.0);
        //myvar->add_att("valid_max",1000.0);
        // sur longue série temporelle, je ne parvient pas à créer un tableau si grand. il faut chunker je suppose. par temps, c'est le plus logique
        t=0;
        for (auto & kv : mVAllDates){
            //std::cout << "time " << t << std::endl;
            myvar->set_rec(dimTime,t);

            float varbuf[NY][NX];
            //float varbuf[1];
            for (int y = 0; y < NY; y++){
                for (int x = 0; x < NX; x++)
                {
                    varbuf[y][x]=kv.second.getValForPix(var_in[y][x],aVar);
                }
            }
            myvar->put_rec(dimTime,&varbuf[0][0]);
            t++;
        }

        // ça c'est pour écrire toute les données d'un seul coup - fonctionne pas si beaucoup de time..
        //varbuf[t][y][x]=kv.second.getValForPix(var_in[y][x],aVar);
        //myvar->put(&varbuf[0][0][0],NTIME, NY,NX);
    }



    ncOut.close();

    // le Y est à l'envers quand j'ouvre le netcdf dans Qgis; c'est du au fait que la grille n'est pas définie comme dans grilleIRMGDL.nc ou on donne l'extend et la résolution en Y qui est négative.
    // Ajout aussi des variales lat lon
    std::string aGrid("/home/jo/app/climat/doc/gridIRMGDL.txt");
    std::string aCommand="cdo -setgrid,"+aGrid+" " + aOutTmp + " " + aOut;


    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
}
