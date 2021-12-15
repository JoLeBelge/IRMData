#include <iostream>
#include <sqlite3.h>
#include "irmdata.h"
//#include "ecaddata.h"
#include "boost/program_options.hpp"

namespace po = boost::program_options;
std::string mBDpath("/home/lisein/Documents/Scolyte/Data/owsf/owsf_scolyte.db");

std::string irmDataFile("/home/lisein/Documents/Scolyte/Data/climat/IRM/pdg1133.csv");

// il y a dans pdg1147-moyTrentenaire la date jour et mois mais pas année, pas meme structure que l'autre csv de l'IRM
//std::string irmDataFile("/home/lisein/Documents/Scolyte/Data/climat/IRM/pdg1147-moyTrentenaire.csv");
// ça c'est le chemin d'accès vers le raster "template" de l'IRM qui contient pour chaque pixel le numéro identifiant du pixel.
std::string IRMRasterTemplatepath("/home/lisein/Documents/Scolyte/Data/climat/IRM/pixel_id.tif");
// dossier ou l'on va sauver tout nos résultats
std::string pathOut("/home/lisein/Documents/Scolyte/Data/climat/IRM/irmCarte/");


extern std::vector<int> vYears;
extern std::vector<int> vMonths;

// pour calcul de degré jours

double baseDJ(8.3); // baier

void processIRMData();

void processECADData();

// pour avoir ensemble une date, un identifiant de site et une position de site
/*class dateSite;

class dateSite{
public:
    dateSite(){}
    dateSite(std::string aId,std::string aPose,std::string aReleve,double aX,double aY):id(aId),X(aX),Y(aY),pose(ymdFromString(aPose)),releve(ymdFromString(aReleve)),datePose(aPose),dateRel(aReleve){
        // std::cout <<"création d'un dateSite" << std::endl;
        djpose=0;
        djrel=0;
    }
    std::string cat(){return std::string(id+";"+datePose+";"+std::to_string(djpose)+";"+dateRel+";"+std::to_string(djrel));}
    //~dateSite();
    std::string id, datePose,dateRel;
    year_month_day pose, releve;
    double X,Y;
    int djpose,djrel;
};*/

int main(int argc, char *argv[])
{
    GDALAllRegister();

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("outils", po::value<int>(), "choix de l'outil à utiliser. 1: calcul carte mensuelle ECAD. 2: export carte une date ECAD. 3: calcul de DJ pour une série de date donnée ECAD. 4 IRM création des cartes mensuelles et autres")
            ("date", po::value< std::string>(), "date pour outil 2. Format yyyy/mm/dd")
            ("accro", po::value< std::string>(), "accro pour carte climatique ; qq, tg, ...")
            ("inputIRMFile", po::value< std::string>(), "chemin d'accès et nom du fichier csv contenant les données de l'IRM")
            ("inputIRMRT", po::value< std::string>(), "chemin d'accès et nom du fichier raster contenant les identifiant de chaque pixel= Raster Template")
            ("outDir", po::value< std::string>(), "dossier ou sera écrit les résultats")
            ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    GDALAllRegister();
    std::string aInTuile,aInArbre,aOut;

    if (vm.count("outils")) {

        int mode(vm["outils"].as<int>());
        switch (mode) {
        /*
         *
         * ECAD
         *
         */
        // calcul carte mensuelle
        case 1:{
            processECADData();
            break;
        }

        case 2:{
            if (vm.count("date")) {
                std::string date(vm["date"].as<std::string>());
                int y=std::stoi(date.substr(0,4));
                std::cout << "y " << std::endl;
                int m=std::stoi(date.substr(5,6));
                std::cout << "m " << std::endl;
                int d=std::stoi(date.substr(8,9));
                std::cout << "d" << std::endl;

                std::cout << "y " << y << " m " << m << " d " << d << std::endl;
                year_month_day ymd(year{y},month{m},day{d});
                std::cout << "date " << ymd << std::endl;
                if (vm.count("accro")) {
                    //exportCarteUneDate(ymd,vm["accro"].as<std::string>());} else {
                    //exportCarteUneDate(ymd);
                }
            } else {
                std::cout << " argument date obligatoire " << std::endl;
            }
            break;
        }
        case 3:{
            std::cout << " calculDJPourDate()" << std::endl;
            // calculDJPourDate();
            break;
        }
            /*
             *
             * IRM
             *
             */
        case 4:{
            std::cout << " create map for IRM" << std::endl;

            if (vm.count("inputIRMFile")) {irmDataFile=vm["inputIRMFile"].as<std::string>();
                std::cout << " je vais utiliser le fichier de donnée IRM " << irmDataFile << std::endl;
            }
            if (vm.count("inputIRMRT")) {IRMRasterTemplatepath=vm["inputIRMRT"].as<std::string>();
                std::cout << " je vais utiliser le raster template de l'IRM " << IRMRasterTemplatepath << std::endl;}
            if (vm.count("outDir")) {pathOut=vm["outDir"].as<std::string>();}
            std::cout << " j'écrirai les résutats dans le dossier " << pathOut << std::endl;

            if (boost::filesystem::exists(irmDataFile)){

                if (boost::filesystem::exists(IRMRasterTemplatepath)){

                    processIRMData();
                } else {
                    std::cout << " je ne peux rien faire car " << IRMRasterTemplatepath << " n'existe pas..." <<std::endl;}
            } else {
                std::cout << " je ne peux rien faire car " << irmDataFile << " n'existe pas..." <<std::endl;
            }
            break;
        }
        }
    }
    std::cout << "done" << std::endl;
    return 0;
}

void processIRMData(){

    irmData d(irmDataFile);
    // donnée de vent
    /*for (int y :{2016,2017,2018,2019,2020,2021}){
        for (int m : {1,2,3,4,5,6,7,8,9,10,11,12}){
            std::cout << " calcul valeur mensuelles pour " << y << "/" << m << std::endl;
            if (1){
                // dans un premier temps, je fait une carte du vent max sur tout le mois. Ce serai peut-être plus opportun de choisir les jours les plus venteux et d'exporter pour cette date là, mais on
                // va laisser arthur travailler un peu.
                dataOneDate mens=d.getMax(year{y},month{m},false);
                mens.exportMap("XWS_"+std::to_string(y)+"_"+std::to_string(m),"WS");
            }
        }
    }*/

    //5tempêtes hiver 2017_2018:
    //10/12 -14/12 (tempête Ana 2017)
    //29/12 (tempête Carmen Moins forte ardenne touchée par des vent de70 à90km)
    // 1/01 1/03  (tempête Eleanor 2018 nord ouest 285°)
    //tempête Fionn (16_18 janvier 2018 modéré sur la wallonie)
    // 18/01  (tempête david 2018)

    std::vector<year_month_day> Ana;
    Ana.push_back(year_month_day(year{2017},month{10},day{12}));
    Ana.push_back(year_month_day(year{2017},month{10},day{13}));
    Ana.push_back(year_month_day(year{2017},month{10},day{14}));
    dataOneDate tana = d.getMax(Ana);
    tana.exportMap("XWS_Ana","WS");

    std::vector<year_month_day> Carmen;
    Carmen.push_back(year_month_day(year{2017},month{12},day{29}));
    dataOneDate tcar = d.getMax(Carmen);
    tcar.exportMap("XWS_Carmen","WS");

    std::vector<year_month_day> Eleanor;
    Eleanor.push_back(year_month_day(year{2018},month{1},day{1}));
    Eleanor.push_back(year_month_day(year{2018},month{1},day{2}));
    Eleanor.push_back(year_month_day(year{2018},month{1},day{3}));
    dataOneDate tel = d.getMax(Eleanor);
    tel.exportMap("XWS_Eleanor","WS");

    std::vector<year_month_day> Fionn;
    Eleanor.push_back(year_month_day(year{2018},month{1},day{16}));
    Eleanor.push_back(year_month_day(year{2018},month{1},day{17}));
    Eleanor.push_back(year_month_day(year{2018},month{1},day{18}));
    dataOneDate tfionn = d.getMax(Fionn);
    tfionn.exportMap("XWS_FionnDavid","WS");

    /*std::vector<year_month_day> David;
    Eleanor.push_back(year_month_day(year{2018},month{1},day{16}));
    Eleanor.push_back(year_month_day(year{2018},month{1},day{17}));
    Eleanor.push_back(year_month_day(year{2018},month{1},day{18}));
    dataOneDate tfionn = d.getMax(Fionn);
    tfionn.exportMap("XWS_Fionn","WS");*/






    if (0){

        //for (int y : vYears){
        // for (int m : vMonths){
        for (int y :{2018,2019,2020}){
            for (int m : {1,2,3,4,5,6,7,8,9,10,11,12}){
                std::cout << " calcul valeur mensuelles pour " << y << "/" << m << std::endl;
                if (1){
                    dataOneDate mens=d.dataMensuel(year{y},month{m});
                    mens.exportMap("Tmean_"+std::to_string(y)+"_"+std::to_string(m),"Tmean");
                    mens.exportMap("Tmax_"+std::to_string(y)+"_"+std::to_string(m),"Tmax");
                    mens.exportMap("Tmin_"+std::to_string(y)+"_"+std::to_string(m),"Tmin");
                    mens.exportMap("ETP_"+std::to_string(y)+"_"+std::to_string(m),"ETP");
                    mens.exportMap("P_"+std::to_string(y)+"_"+std::to_string(m),"P");
                    mens.exportMap("R_"+std::to_string(y)+"_"+std::to_string(m),"R");
                }
                //dataOneDate mens=d.dataMensuelDJ(year{y},month{m});
                //mens.exportMap("DJ_"+std::to_string(y)+"_"+std::to_string(m),"Tmean");
                //break;
            }
        }
        if (0){
            std::cout << " calcul valeur moyenne sur toute les données" << std::endl;
            dataOneDate moy=d.moyAll();
            moy.exportMap("Tmean_moy","Tmean");
            moy.exportMap("ETP_moy","ETP");
            moy.exportMap("P_moy","P");
            moy.exportMap("R_moy","R");
        }
    }


    if (0){
        for (int y : vYears){
            dataOneDate mens=d.dataAnnuel(year{y});
            mens.exportMap("P_"+std::to_string(y),"P");
            mens.exportMap("ETP_"+std::to_string(y),"ETP");
            std::cout << " calcul valeur moyenne sur toute les données" << std::endl;
        }
    }


    //dataOneDate da= d.dataAnnuel(year{1});
    //da.exportMap("P_30aire","P");

    // 2021 06 22 on recois P et ET0 trentenaire, je les traite ici

    if(0){
        // carte annuelle:
        dataOneDate da= d.dataAnnuel(year{1});
        da.exportMap("ETP_30aire","ETP");
        //da.exportMap("P_30aire","P");
        for (int m : {1,2,3,4,5,6,7,8,9,10,11,12}){
            std::cout << " calcul valeur mensuelles trentenaire "<< std::endl;

            dataOneDate mens=d.dataMensuel(year{1},month{m});
            mens.exportMap("ETP_30aire_"+std::to_string(m),"ETP");
            mens.exportMap("P_30aire_"+std::to_string(m),"P");
        }
    }
    // 2021 08 13 moy trentenaire pour R et Température
    if(0){
        // carte annuelle:
        dataOneDate da= d.dataAnnuel(year{1});
        //da.exportMap("R_30aire","R");
        da.exportMap("Tmean_30aire","Tmean");
        da.exportMap("Tmax_30aire","Tmax");
        da.exportMap("Tmin_30aire","Tmin");

        for (int m : {1,2,3,4,5,6,7,8,9,10,11,12}){
            std::cout << " calcul valeur mensuelles trentenaire "<< std::endl;

            dataOneDate mens=d.dataMensuel(year{1},month{m});
            mens.exportMap("R_30aire_"+std::to_string(m),"R");
            mens.exportMap("Tmin_30aire_"+std::to_string(m),"Tmin");
            mens.exportMap("Tmax_30aire_"+std::to_string(m),"Tmax");
            mens.exportMap("Tmean_30aire_"+std::to_string(m),"Tmean");
            //mens.exportMap("P_30aire_"+std::to_string(m),"P");
        }
    }

    // on veut la temperature max par pixel pour une année donnée
    if(0){
        for (int y : vYears){
            int m(9);
            std::cout << " calcul valeur température max pour " << y << std::endl;
            dataOneDate mens=d.getMax(year{y},month{m});
            mens.exportMap("XTX_"+std::to_string(y)+"_"+std::to_string(m),"Tmax");
            mens.exportMap("XTMean_"+std::to_string(y)+"_"+std::to_string(m),"Tmean");
            mens.exportMap("XTMin_"+std::to_string(y)+"_"+std::to_string(m),"Tmin");
        }
    }



    /*
    //gdalsrsinfo  -o wkt epsg:31370
    OGRSpatialReference src;
    src.importFromEPSG(31370);
    OGRSpatialReference srcFrom;
    srcFrom.importFromEPSG(4326);
    OGRCoordinateTransformation* coordTrans =  OGRCreateCoordinateTransformation(&srcFrom, &src);
    double x1 = 49.510014917022289;
    double y1 = 5.4191010793596979;
    int reprojected = coordTrans->Transform(1, &x1, &y1);
    */

}


void processECADData(){

    //ecadData d;
    //d.calculCarteMensuel();

}

