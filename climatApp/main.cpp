#include <iostream>
#include <sqlite3.h>
#include "irmdata.h"
#include "ecaddata.h"
#include "safrandata.h"
#include "boost/program_options.hpp"
#include "../date/include/date/date.h"
#include "mar.h"


using namespace date;
using namespace std;
namespace po = boost::program_options;
std::string mBDpath("/home/lisein/Documents/Scolyte/Data/owsf/owsf_scolyte.db");

std::string input("/home/lisein/Documents/Scolyte/Data/climat/IRM/pdg1133.csv");

// il y a dans pdg1147-moyTrentenaire la date jour et mois mais pas année, pas meme structure que l'autre csv de l'IRM
//std::string irmDataFile("/home/lisein/Documents/Scolyte/Data/climat/IRM/pdg1147-moyTrentenaire.csv");
// ça c'est le chemin d'accès vers le raster "template" de l'IRM qui contient pour chaque pixel le numéro identifiant du pixel.
std::string input2("/home/lisein/Documents/Scolyte/Data/climat/IRM/pixel_id.tif");
// dossier ou l'on va sauver tout nos résultats
std::string pathOut("/home/lisein/Documents/Scolyte/Data/climat/IRM/irmCarte/");

std::string input3("");

extern std::vector<int> vYears;
extern std::vector<int> vMonths;

// pour calcul de degré jours

double baseDJ(8.3); // baier

int mode(1);
typeGrid grid;

void processIRMData();
void processSAFRAN();
void processECADData();

void correctionPrevMar(int y1,int y2);

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
            ("input", po::value< std::string>(), "chemin d'accès et nom du fichier csv contenant les données de l'IRM")
            ("input2", po::value< std::string>(), "chemin d'accès et nom du fichier raster contenant les identifiant de chaque pixel= Raster Template")
            ("input3", po::value< std::string>(), "chemin d'accès reanalyse GCM-MAR pour corriger indice climatique")
            ("outDir", po::value< std::string>(), "dossier ou sera écrit les résultats")
            ("mode", po::value<int>(), "mode pour export netcdf to raster : 1 grille rotationnée SOP 5km de résolution, 2 grille IRM; 3 grille rotationnée résolution 7.5km")
            ("years",  po::value<std::vector<int> >()->multitoken(), "année 1 et année 2 pour calcul d'indices climatiques sur une période de plusieurs années" )

            ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    GDALAllRegister();
    //std::string aInTuile,aInArbre,aOut;
    if (vm.count("mode")) {mode=vm["mode"].as<int>();
        if (mode==1){grid=SOP;}
        if (mode==2){grid=irm;}
        if (mode==3){grid=SOP75;}
        if (mode==4){grid=irmO;}
    }

    std::vector<int> y;
    if (!vm["years"].empty() && (vm["years"].as<vector<int> >()).size() == 2) {
        y=vm["years"].as<vector<int> >();
        std::cout << " période de référence entre " << y.at(0) << " et " << y.at(1) <<std::endl;
    }

    if (vm.count("input")) {input=vm["input"].as<std::string>();
        std::cout << " je vais utiliser le fichier de donnée IRM " << input << std::endl;
    }
    if (vm.count("input2")) {input2=vm["input2"].as<std::string>();
        std::cout << " je vais utiliser le raster template de l'IRM " << input2 << std::endl;}
    if (vm.count("outDir")) {pathOut=vm["outDir"].as<std::string>();
        std::cout << " j'écrirai les résutats dans le dossier/fichier " << pathOut << std::endl;}


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



            if (boost::filesystem::exists(input)){

                //if (boost::filesystem::exists(input2)){

                processIRMData();
                /*} else {
                    std::cout << " je ne peux rien faire car " << input2 << " n'existe pas..." <<std::endl;}*/
            } else {
                std::cout << " je ne peux rien faire car " << input << " n'existe pas..." <<std::endl;
            }
            break;
        }
            /*
             *
             * SAFRAN
             *
             */
        case 5:{
            std::cout << " create map for SAFRAN data" << std::endl;
            processSAFRAN();
            break;
        }
        case 6:{
            // ./climatApp --outil 6 --input "/home/gef/app/climat/doc/grilleIRMGDL.nc:ZBIO" --outDir ./grilleIRMZBIO.tif --mode 2

            std::cout << " export raster from netcdf " << input << " to " << pathOut << std::endl;
            //setGeoTMAR(input);
            exportRaster(input,pathOut,grid);
            break;
        }
        case 7:{
             int y1(y.at(0)), y2(y.at(1));
            // ./climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR/reanalyse-IRMgrid/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2
            std::cout << " MAR netcdf : passage de l'horaire au journalier" << std::endl;
            MAR mar(input,input2,grid,0);
            mar.hourly2daily();
            mar.daily2monthly();
            //mar.moyenneMobile();

            mar.multiY(y1,y2);
            mar.multiYStat(y1,y2);
           /* mar.multiY(2021,2050);
            mar.multiYStat(2021,2050);
            mar.multiY(2051,2080);
            mar.multiYStat(2051,2080);
            mar.multiY(2081,2100);
            mar.multiYStat(2081,2100);*/

            break;
        }

            // correction pour les valeurs simulées pour le futur ; je crée un total de 3 objets MAR pour pouvoir cal
        case 8:{

            if (vm.count("input3")) {input3=vm["input3"].as<std::string>();}
            int y1(y.at(0)), y2(y.at(1));   // période de simulation
            correctionPrevMar(y1,y2);
            // ./climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR/reanalyse-IRMgrid/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2

            break;
        }

        case 40:{
            // travail sur un fichier netcdf de l'IRM, créé par l'outil 4 - il faut structurer tout ça comme les sorties MAR ; je dois donc faire des fichiers individuel pour chaque année.

            // maintenant je peux utiliser l'outil 7 pour faire tout cela, redondant..
            MAR mar(input,input2,irmO,0);
            //mar.hourly2daily();
            //mar.daily2monthly();
            if(1){
                mar.multiY(1961,1990);
                mar.multiYStat(1961,1990);

                mar.multiY(1991,2020);
                mar.multiYStat(1991,2020);
            }
            if(0){

                // comme VanderPerre et al., sauf que eux ont utilisé une grille de 500mètres.
                /*mar.multiY(1986,2005);
                    mar.multiYStat(1986,2005);

                    mar.multiY(2011,2020);
                    mar.multiYStat(2011,2020);
                     */

                //mar.multiY(1981,2010);
                //mar.multiYStat(1981,2010);
                //mar.multiY(1991,2020);
                //mar.multiYStat(1991,2020);

            }

            break;
        }

        }
    }
    std::cout << "done" << std::endl;
    return 0;
}

void processIRMData(){

    irmData d(input);

    // test export données trentenaires aux format netcdf - 2023 02
    d.saveNetCDF(pathOut);

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

    if (0){
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
        Fionn.push_back(year_month_day(year{2018},month{1},day{16}));
        Fionn.push_back(year_month_day(year{2018},month{1},day{17}));
        Fionn.push_back(year_month_day(year{2018},month{1},day{18}));
        dataOneDate tfionn = d.getMax(Fionn);
        tfionn.exportMap("XWS_FionnDavid","WS");
    }

    if (0){

        //for (int y : vYears){
        // for (int m : vMonths){
        for (int y :{2014,2015,2016,2017,2018,2019,2020}){
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
        //dataOneDate da= d.dataAnnuel(year{1});
        //da.exportMap("P_30aire","P");
        /*da.exportMap("R_30aire","R");
        da.exportMap("Tmean_30aire","Tmean");
        da.exportMap("Tmax_30aire","Tmax");
        da.exportMap("Tmin_30aire","Tmin");
        */

        for (int m : {1,2,3,4,5,6,7,8,9,10,11,12}){
            std::cout << " calcul valeur mensuelles trentenaire "<< std::endl;
            dataOneDate mens=d.dataMensuel(year{1},month{m});
            /*mens.exportMap("R_30aire_"+std::to_string(m),"R");
            mens.exportMap("Tmin_30aire_"+std::to_string(m),"Tmin");
            mens.exportMap("Tmax_30aire_"+std::to_string(m),"Tmax");
            mens.exportMap("Tmean_30aire_"+std::to_string(m),"Tmean");
            mens.exportMap("P_30aire_"+std::to_string(m),"P");*/
            mens.exportMap("irmTmean_"+std::to_string(m),"Tmean");
            mens.exportMap("irmTmax_"+std::to_string(m),"Tmax");
            mens.exportMap("irmTmin_"+std::to_string(m),"Tmin");
            mens.exportMap("irmTminMin_"+std::to_string(m),"TminMin");
            // mens.exportMap("irmP_"+std::to_string(m),"P");
        }
    }

    if(0){
        // carte annuelle:
        dataOneDate da= d.dataAnnuel(year{2018});
        //da.exportMap("P_2018","P");
        int y=2018;
        for (int m : {1,2,3,4,5,6,7,8,9,10,11,12}){
            std::cout << " calcul valeur mensuelles "<< std::endl;
            dataOneDate mens=d.dataMensuel(year{2018},month{m});
            mens.exportMap("irmTmean_"+std::to_string(y)+"_"+std::to_string(m),"Tmean");
            mens.exportMap("irmTmax_"+std::to_string(y)+"_"+std::to_string(m),"Tmax");
            mens.exportMap("irmTmin_"+std::to_string(y)+"_"+std::to_string(m),"Tmin");
            mens.exportMap("irmTminMin_"+std::to_string(y)+"_"+std::to_string(m),"TminMin");
            mens.exportMap("irmP_"+std::to_string(y)+"_"+std::to_string(m),"P");
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

    ecadData d;

    std::vector<int> y{2018};
    //d.calculCarteMensuel(y,"tg");
    //d.calculCarteMensuel(y,"tn");
    //d.calculCarteMensuel(y,"tx");
    //d.calculCarteMensuel(y,"rr");
    //d.calculCarteMensuelTrentenaire(year{1991},year{2020}, "tn");
    //d.calculCarteMensuelTrentenaire(year{1991},year{2020}, "tx",resumeMensuel::max);


    //d.calculCarteMensuelTrentenaire(year{1991},year{2020}, "tg");
    //d.calculCarteMensuelTrentenaire(year{1986},year{2005}, "rr",resumeMensuel::sum,"fee");
    d.calculCarteMensuelTrentenaire(year{2012},year{2019}, "rr",resumeMensuel::sum,"ecad22");
    // il manque un mois en 2011 et en 2020 il n'y a pas toute l'année.

    // je remarque que les valeurs de précipitations ne concordent pas entre ecad et IRM, et je constate que antérieuremnet à 2003, il n'y avais que 2 stations en Ardenne (Mt Rigi et St Hubert).
    // ca pourrais expliquer les différences constatées, mais malheureusement même en traitant les données postérieures à 2003, les précipitations semblent clairement sous-estimée en Ardenne.
    // test moyenne précipitation sur une période plus récente

    //d.calculCarteMensuelTrentenaire(year{2003},year{2020}, "rr",resumeMensuel::sum,"dense");
    // pour le diagramme Walter & Lieth climatic diagram, j'ai besoin de la température minimum absolue également
    //d.calculCarteMensuel(y,"tn",resumeMensuel::min);
    //d.calculCarteMensuelTrentenaire(year{1991},year{2020}, "tn",resumeMensuel::min);

}

void processSAFRAN(){
    int y (2018);
    // un fichier par année.
    safranData d(input);

    for (int m : {1,2,3,4,5,6,7,8,9,10,11,12}){
        std::cout << " calcul valeur mensuelles pour " << y << "/" << m << std::endl;

        if(0){
            dataOneDate mens=d.dataMensuel(year{y},month{m});
            mens.exportMap("safranTmean_"+std::to_string(y)+"_"+std::to_string(m),"Tmean");
            mens.exportMap("safranTmax_"+std::to_string(y)+"_"+std::to_string(m),"Tmax");
            mens.exportMap("safranTmin_"+std::to_string(y)+"_"+std::to_string(m),"Tmin");
            mens.exportMap("safranTminMin_"+std::to_string(y)+"_"+std::to_string(m),"TminMin");
            //mens.exportMap("ETP_"+std::to_string(y)+"_"+std::to_string(m),"ETP");
            mens.exportMap("safranP_"+std::to_string(y)+"_"+std::to_string(m),"P");
            //mens.exportMap("R_"+std::to_string(y)+"_"+std::to_string(m),"R");
        }
        if(1){
            dataOneDate mens=d.calculCarteMensuelTrentenaire(year{2012},year{2020},month{m});
            /*mens.exportMap("safranTmean_"+std::to_string(m),"Tmean");
        mens.exportMap("safranTmax_"+std::to_string(m),"Tmax");
        mens.exportMap("safranTmin_"+std::to_string(m),"Tmin");
        mens.exportMap("safranTminMin_"+std::to_string(m),"TminMin");
        mens.exportMap("safranP_"+std::to_string(m),"P");*/
            mens.exportMap("safranETP_"+std::to_string(m),"ETP");
        }
    }
}

void correctionPrevMar(int y1, int y2){
    std::cout << " correction pour valeurs absolues simulation climat futur" << std::endl;

    MAR era5("/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR/reanalyse-IRMgrid/",input2,typeGrid::irm,0);
    MAR GCMsimu(input,input2,typeGrid::irm,0); // prévision future pour ce GCM
    //MAR irm("/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/IRM/",input2,typeGrid::irmO,0);
    MAR GCMhisto(input3,input2,typeGrid::irm,0); // réanalyse pour ce GCM

    GCMsimu.multiYCorrection(y1,y2,& era5, & GCMhisto);

    /* calcul du biais entre observations irm et reanalyse MAR -- OLD OLD test numéro 1 qui n'est pas la bonne démarche hélas
    //aCommand="cdo -sub " + irm.nameMultiY(y1r,y2r,"TG") + " " + era5.nameMultiY(y1r,y2r,"TG") + " " + era5.nameMultiY(y1r,y2r,"TGbiais");
    // calcul de l'anomalie climatique en %
    aCommand="cdo -expr,'TGanomalie=100*TGdelta/T2mG;' -merge "+ era5.nameMultiY(y1r,y2r,"TG") + " -setvar,TGdelta -sub " + prevFutur.nameMultiY(y1,y2,"TG") + " " + era5.nameMultiY(y1r,y2r,"TG") + " " + prevFutur.nameMultiY(y1,y2,"TGcor1");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());
    // appliquer l'anomalie climatique au climat irm
    aCommand="cdo -expr,'T2mG=TG+(TG*TGanomalie)/100.0;' -merge " + irm.nameMultiY(y1r,y2r,"TG") + " " + prevFutur.nameMultiY(y1,y2,"TGcor1") + " " + prevFutur.nameMultiY(y1,y2,"TGcor");
    std::cout << aCommand << std::endl;
    system(aCommand.c_str());*/
}
