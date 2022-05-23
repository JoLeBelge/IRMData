#include "safrandata.h"

extern int colP,colTmean,colTmax,colTmin, colP2, colETP;
int colX(0),colY(1),colDate(2);

std::string fileXY("/home/lisein/Documents/Scolyte/Data/climat/SAFRAN/safranUniqueXY.csv");

safranData::safranData(std::string aFileIn):irmData()
{

    std::map<std::tuple<int,int>, int> mXYtoID;
    std::vector<std::vector<std::string>> d2=parseCSV2V(fileXY,';');
    for (int c(1); c<d2.size();c++){
        std::vector<std::string> line=d2.at(c);
        int x=std::stoi(line.at(1));
        int y=std::stoi(line.at(2));
        int id=std::stoi(line.at(0))+1;
        mXYtoID.emplace(std::make_tuple(x,y),id);
    }

    for (int ay : {2012,2013,2014,2015,2016,2017,2018,2019,2020}){
    //for (int ay : {2018}){
        aFileIn="/home/jo/Documents/Scolyte/Data/climat/SAFRAN/Safran_params_"+std::to_string(ay)+".csv";

        std::cout << " création du catalogue Safran pour fichier " << aFileIn << std::endl;
        std::vector<std::vector<std::string>> d=parseCSV2V(aFileIn,';');
        std::cout << " fichier texte parsé " << std::endl;

        // détermine les numéro de colonnes pour les variables
        std::vector<std::string> headerline=d.at(0);
        int nbItem=headerline.size();
        for (int c(0);c<headerline.size();c++){
            std::string header=headerline.at(c);
            std::cout << header << " est la colonne " << c << std::endl;
            //if (header.find("GLOBAL RADIATION")!=std::string::npos){colR=c;}

            // attention, ECOULEMENT_Q contient T_Q!!
            if (header=="T_Q"){colTmean=c;}

            if (header.find("TSUP_H_Q")!=std::string::npos){colTmax=c;}
            if (header.find("TINF_H_Q")!=std::string::npos){colTmin=c;}
            //if (header=="PRESSURE (hPa)"){colP=c;}
            // PRECIPITATION LIQUIDE - IL FAUDRAI AJOUTER LES SOLIDES
            if (header.find("PRELIQ_Q")!=std::string::npos){colP=c;}
            if (header.find("PRENEI_Q")!=std::string::npos){colP2=c;}
            if (header.find("ETP_Q")!=std::string::npos){colETP=c;}
            //if (header.find("ET")!=std::string::npos){colETP=c;}
            //if (header.find("WIND SPEED")!=std::string::npos){colWS=c;}
        }
        std::cout << "\n T mean colonne " << colTmean << " T max colonne " << colTmax << " T min colonne " << colTmin << "\n" << " Précipitation colonne " << colP << "\n précipitation neige " << colP2 << " ET0 colonne " << colETP << std::endl;

        for (int c(1); c<d.size();c++){
            std::vector<std::string> line=d.at(c);
            if (line.size()==nbItem){
                std::string aDate=line.at(colDate);
                int d(1),m(1),y(1);
                //"20120113"
                //std::cout << " aDate " << aDate << std::endl;

                // std::cout << " y " << aDate.substr(0,4) << " m " << aDate.substr(4,2) << " "<< aDate.substr(6,2) << std::endl;
                d=std::stoi(aDate.substr(6,2));
                m=std::stoi(aDate.substr(4,2));
                y=std::stoi(aDate.substr(0,4));

                year_month_day ymd(year{y},month{m},day{d});

                // determine l'id du pixel
                int id(0);
                std::tuple<int,int> t = std::make_tuple(std::stoi(line.at(colX)),std::stoi(line.at(colY)));
                if (mXYtoID.find(t)!=mXYtoID.end()){
                    id=mXYtoID.at(t);
                } else {
                    std::cout << "problème avec pix  " << line.at(colX) << ", " << line.at(colY) << std::endl;
                }

               /* if (id==7260){//6939
            for (std::string s : line){
                std::cout<< s<< ";" ;
            }

            std::cout<< "Tmean " <<std::stod(line.at(colTmean)) << std::endl;
            std::cout<<std::endl;
                }*/

                if ( mVAllDates.find(ymd)==mVAllDates.end()){
                   // std::cout << " création des données SAFRAN pour une date , y " << y << " m " << m << " d " << d << std::endl;
                    mVAllDates.emplace(std::make_pair(ymd,dataOneDate(ymd)));
                    mVAllDates.at(ymd).addOnePix(line,id);
                } else {
                    mVAllDates.at(ymd).addOnePix(line,id);
                }
            }else {
                std::cout << "pas assez d'élément dans la ligne !! " << std::endl;
                for (std::string s : line){
                    std::cout<< s<< ";" ;
                }
                std::cout<<std::endl;
            }
            //std::cout << "done " << std::endl;
        }
        std::cout << "done, file " << aFileIn << std::endl;
    }



    // print all xy position in a file to have an id associated with each cell and to create the template raster.
    /*if(0){
    std::ofstream aOut;
    aOut.open(fileXY,std::ios::out);
    aOut << "id;X;Y;X2;Y2\n" ;
    for (auto kv : mVAllDates){
        int c(0);
        safdataOneDate * d= & kv.second;
        for (auto kv2 : d->mVData){
            std::tuple<int,int> t = kv2.first;
            // X et Y sont en hm, donc je dois faire x100 pour avoir la vrai position
            aOut << c<<";" << std::get<0>(t) << ";"<< std::get<1>(t) << ";" << 100*std::get<0>(t) << ";"<< 100*std::get<1>(t)<< "\n";
            c++;
        }
        break;
    }
    aOut.close();
    }*/




}

/*

safdataOneDate::safdataOneDate(year_month_day ymd){
    //std::cout << " création des données irm pour une date , " << ymd << std::endl;
    mDate=ymd;
}

void safdataOneDate::addOnePix(std::vector<std::string> & aLigne){

    mVData.emplace(std::make_pair(std::make_tuple(std::stoi(aLigne.at(colX)),std::stoi(aLigne.at(colY))),dataOnePix(aLigne)));
}

void safdataOneDate::divide(int nb){
    for (auto & kv : mVData){
        kv.second.divide(nb);
    }
}

safdataOneDate safranData::dataMensuel(year y, month m){
    std::vector<safdataOneDate*> aVD;
    // selection des dates qui font parties du mois
    for (auto & kv : mVAllDates){
        year_month_day curYmd=kv.first;
        if (curYmd.year()==y && curYmd.month()==m){
            aVD.push_back(& kv.second);
        }
    }
    // création d'un dataOneDate pour y mettre les valeurs mensuelles
    year_month_day ymd(y,m,day{1});
    safdataOneDate mensuel(ymd);
    for ( safdataOneDate * dod : aVD){
        // je dois faire l'addition pour tout les pixels
        mensuel.addOneDate(dod);
    }
    // pour avoir la moyenne ; je divise par le nombre d'observations.
    mensuel.divide(aVD.size());
    return mensuel;
}

void safdataOneDate::addOneDate(safdataOneDate * dod){
    for (auto & kv : dod->mVData){
        if (mVData.find(kv.first)!=mVData.end()){
            mVData.at(kv.first).addOneDate(&kv.second);
        } else {
            mVData.emplace(kv);
        }
    }
}
*/

