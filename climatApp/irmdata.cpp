#include "irmdata.h"
std::vector<int> vYears={2016,2017,2018,2019,2020}; // pour les données IRM j'ajoute l'année 2020, mais pour ECAD je pense que je n'ai pas les données 2020
std::vector<int> vMonths={3,4,5,6,7,8,9};

extern double baseDJ;
extern std::string IRMRasterTemplatepath;
extern std::string pathOut;

irmData::irmData(std::string aFileIRM)
{
    std::cout << " création du catalogue irm " << std::endl;
    std::vector<std::vector<std::string>> d=parseCSV2V(aFileIRM,';');
    std::cout << " fichier texte parsé " << std::endl;
    std::string curDate("");
    for (int c(1); c<d.size();c++){
        std::vector<std::string> line=d.at(c);
        std::string aDate=line.at(0);

        // fonctionnement polyvalent si moyenne trentenaire - en fonction de la taille du champ date il définit le mode de lecture des données
        int d(1),m(1),y(1), mode(1);
        if (aDate.size()==5){
            d=std::stoi(aDate.substr(3,5));
            m=std::stoi(aDate.substr(0,2));
            //std::cout << " day " << d << " , " << " month " << m << std::endl;
            mode=2;
            //mode=3;
        }else{
            // une nouvelle date
            d=std::stoi(aDate.substr(8,9));
            m=std::stoi(aDate.substr(5,6));
            y=std::stoi(aDate.substr(0,4));
           //mode=4;
        }

        year_month_day ymd(year{y},month{m},day{d});
        if ( curDate!=aDate){
            //std::cout << " création des données irm pour une date , y " << y << " m " << m << " d " << d << std::endl;
            mVAllDates.emplace(std::make_pair(ymd,dataOneDate(ymd)));
            mVAllDates.at(ymd).addOnePix(line,mode);
            curDate=aDate;
        } else {
            mVAllDates.at(ymd).addOnePix(line,mode);
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

    // selection des dates qui font parties du mois
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

dataOneDate irmData::getMax(year y, month m){
    std::vector<dataOneDate*> aVD;

    // selection de toutes les dates qui sont antérieure dans l'année et qui font partie du mois
    for (auto & kv : mVAllDates){
        year_month_day curYmd=kv.first;
        if (curYmd.year()==y && curYmd.month()<=m){
            aVD.push_back(& kv.second);
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

void dataOneDate::addOnePix(std::vector<std::string> & aLigne, int mode){

    mVData.emplace(std::make_pair(std::stoi(aLigne.at(1)),dataOnePix(aLigne,mode)));
}

void dataOneDate::divide(int nb){
    for (auto & kv : mVData){
        kv.second.divide(nb);
    }
}

void dataOneDate::exportMap(std::string aOut,std::string aVar){
    // ouverture du template, copie de la carte, remplacer chaque pixel_id par sa valeur.
    const char *pszFormat = "GTiff";
    GDALDriver *pDriver= GetGDALDriverManager()->GetDriverByName(pszFormat);

    GDALDataset * temp=(GDALDataset *) GDALOpen( IRMRasterTemplatepath.c_str(), GA_ReadOnly );
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
    double aRes(0);
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
        }
    }
    return aRes;
}

void dataOnePix::divide(int nb){
    if (nb!=0){
        Tmean/=nb;
        Tmax/=nb;
        Tmin/=nb;
        P/=nb;
        R/=nb;
        ETP/=nb;
    }
}

dataOnePix::dataOnePix(std::vector<std::string> & aLigne, int mode){
    if (mode==1){
    Tmean=std::stod(aLigne.at(4));
    Tmax=std::stod(aLigne.at(5));
    Tmin=std::stod(aLigne.at(6));
    R=std::stod(aLigne.at(7));
    P=std::stod(aLigne.at(8));
    ETP=std::stod(aLigne.at(9));
    } else if(mode==2){
        P=std::stod(aLigne.at(4));
        ETP=std::stod(aLigne.at(5));
        Tmean=0;Tmax=0;Tmin=0;R=0;
    } else if(mode==3){
        //pdg1165-moyTrentenaire
        //"month/day";"PIXEL_ID";"PIXEL_LON_CENTER";"PIXEL_LAT_CENTER";"GLOBAL RADIATION (kWh/m2/day)";"TEMPERATURE AVG (°C)";"TEMPERATURE MAX (°C)";"TEMPERATURE MIN (°C)";"PRESSURE (hPa)"
        R=std::stod(aLigne.at(4));
        Tmean=std::stod(aLigne.at(5));
        Tmax=std::stod(aLigne.at(6));
        Tmin=std::stod(aLigne.at(7));
        P=std::stod(aLigne.at(8));


    } else if(mode==4){
        //pdg1165_2-P
        //"DATE";"PIXEL_ID";"PIXEL_LON_CENTER";"PIXEL_LAT_CENTER";"TEMPERATURE AVG(°C)";"TEMPERATURE MAX (°C)";"TEMPERATURE MIN (°C)";"GLOBAL RADIATION (kWh/m2/day)";"PRECIPITATION (mm)";"ET0 (mm)"



    }
}

void dataOnePix::addOneDate(dataOnePix * dop){
    Tmean+=dop->Tmean;
    Tmax+=dop->Tmax;
    Tmin+=dop->Tmin;
    P+=dop->P;
    R+=dop->R;
    ETP+=dop->ETP;
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
}


void dataOnePix::addOneDateDJ(dataOnePix * dop, double aSeuilDJ){
    double DJTmean=dop->Tmean-aSeuilDJ;
    if (DJTmean>0){
         Tmean+=DJTmean;
    }
}

std::vector<std::vector<std::string>> parseCSV2V(std::string aFileIn, char aDelim){
    qi::rule<std::string::const_iterator, std::string()> quoted_string = '"' >> *(qi::char_ - '"') >> '"';
    qi::rule<std::string::const_iterator, std::string()> valid_characters = qi::char_ - '"' - aDelim;
    qi::rule<std::string::const_iterator, std::string()> item = *(quoted_string | valid_characters );
    qi::rule<std::string::const_iterator, std::vector<std::string>()> csv_parser = item % aDelim;

    std::vector<std::vector<std::string>> aOut;
    std::ifstream aFichier(aFileIn.c_str());
    if(aFichier)
    {
        std::string aLine;
        while(!aFichier.eof())
        {
            getline(aFichier,aLine,'\n');
            if(aLine.size() != 0)
            {
                std::string::const_iterator s_begin = aLine.begin();
                std::string::const_iterator s_end = aLine.end();
                std::vector<std::string> result;
                bool r = boost::spirit::qi::parse(s_begin, s_end, csv_parser, result);
                assert(r == true);
                assert(s_begin == s_end);
                // ajout d'un element au vecteur de vecteur
                aOut.push_back(result);
            }
        }
        aFichier.close();
    } else {
        std::cout << "file " << aFileIn << " not found " <<std::endl;
    }
    return aOut;
}
