#include <iostream>
//#include <netcdfcpp.h>

#include <sqlite3.h>
#include "irmdata.h"


std::string mBDpath("/home/lisein/Documents/Scolyte/Data/owsf/owsf_scolyte.db");

//std::string irmDataFile("/home/lisein/Documents/Scolyte/Data/climat/IRM/pdg1133.csv");

// il y a dans pdg1147-moyTrentenaire la date jour et mois mais pas année, pas meme structure que l'autre csv de l'IRM
std::string irmDataFile("/home/lisein/Documents/Scolyte/Data/climat/IRM/pdg1147-moyTrentenaire.csv");
// ça c'est le chemin d'accès vers le raster "template" de l'IRM qui contient pour chaque pixel le numéro identifiant du pixel.
std::string IRMRasterTemplatepath("/home/lisein/Documents/Scolyte/Data/climat/IRM/pixel_id.tif");
// dossier ou l'on va sauver tout nos résultats
std::string pathOut("/home/lisein/Documents/Scolyte/Data/climat/IRM/irmCarte/");


// pour extraire une bande netcdf au format raster ; gdal_translate -ot Int16 NETCDF:qq_ens_mean_0.1deg_reg_2011-2020_v22.0e.nc -b 1 -unscale "t1_01.tif"

//std::vector<int> timesForOneMonth(year_month ym, NcVar *timeVar, int nrec);
//int timeFromYMD(year_month_day ymd, NcVar *timeVar, int nrec);
void projectRaster(std::string inPath, std::string outPath);

void projectRasterCallgdal(std::string inPath, std::string outPath);

int calculCarteMensuel();
void exportCarteUneDate(year_month_day ymd, std::string varAccro="qq");

void calculDJPourDate();
void exportCarteDJUneDate(year_month_day ymd);
//std::vector<int> timesForDateForDJ(year_month_day ymd, NcVar *timeVar, int nrec);

year_month_day ymdFromString(std::string date);

int getValue(std::string pathTif, double x, double y);

year_month_day baseymd = 1950_y/1/1;

int NLON;
int NLAT;
// Return this in event of a problem.
static const int NC_ERR = 2;

extern std::vector<int> vYears;
extern std::vector<int> vMonths;

// pour calcul de degré jours
bool DG(1);
double baseDJ(8.3); // baier

void processIRMData();

// pour avoir ensemble une date, un identifiant de site et une position de site
class dateSite;

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
};

int
main(int argc, char *argv[])
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

    if (vm.count("outil")) {

        int mode(vm["outil"].as<int>());
        switch (mode) {
        /*
         *
         * ECAD
         *
         */
        // calcul carte mensuelle
        case 1:{
            //calculCarteMensuel();
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

/*
int calculCarteMensuel(){
    //std::vector<std::string> accro{"qq", "tn", "tx", "pp", "rr", "tg"};
    // pp c'est la pression qui est de 1000 par jour donc somme par mois est de 30000 et ça fonctionne pas
    //std::vector<std::string> accro{ "tn", "tx", "rr", "tg","qq"};
    std::vector<std::string> accro{"tn","tx","tg"};
    //std::string varAccro("pp");

    for (std::string varAccro : accro){
        std::cout << " traitement variable climatique " << varAccro << std::endl;

        // je regarde sous qgis les scale factor ; qq = 1, rr = 0.1, temp = 0.0099999998
        double scaleF(1);
        if (varAccro=="tn" |varAccro=="tx" | varAccro=="tg"){
            scaleF=0.0099999998;
        }
        if (varAccro=="rr"){
            scaleF=0.1;
        }

        //std::string ncpath("/home/lisein/Documents/Scolyte/Data/climat/01deg/qq_ens_mean_0.1deg_reg_2011-2020_v22.0e.nc");
        std::string ncpath("/home/lisein/Documents/Scolyte/Data/climat/01deg/"+varAccro+"_ens_mean_0.1deg_reg_2011-2020_v22.0e.nc");
        boost::filesystem::path ap(ncpath);
        //std::string varAccro=ap.stem().string().substr(0,2);
        std::string tifModel("/home/lisein/Documents/Scolyte/Data/climat/01deg/template_"+varAccro+".tif");

        const char * p=ncpath.c_str();

        NcFile in(p,NcFile::FileMode::ReadOnly,NULL,0,NcFile::FileFormat::Netcdf4);
        if (!in.is_valid())
        {
            std::cout << "Couldn't open file!\n";
            return NC_ERR;
        } else{


            // NcError err(NcError::silent_nonfatal);

            std::cout << "input File has " << in.num_dims() << " dimensions and " <<in.num_vars() << " vars and "  << in.num_atts() << " atts" << std::endl ;

            std::cout << "determine lat et long ncVar" << std::endl;
            NcVar *latVar, *lonVar;
            if (!(latVar = in.get_var("latitude")))
                return NC_ERR;
            if (!(lonVar = in.get_var("longitude")))
                return NC_ERR;

            std::cout << "latVar->name() " << latVar->name() << " " << latVar->id() << " id " <<std::endl;
            std::cout << "lonVar->name() " << lonVar->name() << " " << lonVar->id() << " id "<<std::endl;

            NcVar *inVar,*timeVar;
            //timeVar=in.get_var(0);
            timeVar=in.get_var("time");
            std::cout << "timevar->name() " <<timeVar->name() << " " << timeVar->id() << " id " <<std::endl;

            NcDim * time=in.get_dim("time");
            std::cout << " size of dim " << time->name() << " " << time->size() << std::endl;
            int NREC = time->size();
            NcDim * l=in.get_dim("longitude");
            std::cout << " size of dim " << l->name() << " " << l->size() << std::endl;
            int NLON = l->size();
            NcDim * lati=in.get_dim("latitude");
            std::cout << " size of dim " << lati->name() << " " << lati->size() << std::endl;
            int NLAT = lati->size();

            float var_in[NLAT][NLON];

            inVar=in.get_var(3);
            std::cout << "invar->name() " <<inVar->name() << std::endl;

            for (int y : vYears){
                for (int m : vMonths){
                    std::cout << " calcul valeur mensuelles pour " << y << "/" << m << std::endl;
                    //year_month ym(2018_y/5);
                    year_month ym(year{y}/m);
                    std::vector<int> v= timesForOneMonth(ym, timeVar, NREC);
                    int ndayInM = v.size();

                    GDALDriver *pDriver,*pDriverMEM;
                    GDALDataset *pRaster=NULL,*pTemplateRaster=NULL,*pMensuelRaster=NULL;

                    const char *inputTemplate=tifModel.c_str();

                    const char *pszFormat = "GTiff";

                    std::cout << "lecture tiff template et création mem raster avec autant de bandes que de jours " << std::endl;
                    pDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
                    pTemplateRaster = (GDALDataset*) GDALOpen(inputTemplate, GA_ReadOnly);

                    const char *pszFormat2 = "MEM";

                    pDriverMEM = GetGDALDriverManager()->GetDriverByName(pszFormat2);
                    std::string output("/vsimem/tmp.tif");
                    //std::string output("/home/lisein/Documents/tmp.tif");
                    const char *out=output.c_str();
                    pRaster = pDriverMEM->Create(out,  NLON,NLAT, v.size(), GDT_Int16, NULL);
                    pRaster->SetSpatialRef(pTemplateRaster->GetSpatialRef());
                    double transform[6];
                    pTemplateRaster->GetGeoTransform(transform);
                    pRaster->SetGeoTransform( transform );

                    // le tif qui synthétisera les info mensuelles---------------------------------------
                    std::string aName(varAccro+"_"+std::to_string(int (ym.year()))+"_"+ym.month().getM());
                    if (DG){aName=aName+"_DG";}

                    std::string output2("/home/lisein/Documents/Scolyte/Data/climat/01deg/mensuel/"+aName+".tif");
                    const char *out2=output2.c_str();
                    std::string output3="/home/lisein/Documents/Scolyte/Data/climat/01deg/mensuelBL72/"+aName+"_BL72.tif";
                    const char *out3=output3.c_str();

                    //pMensuelRaster = pDriver->Create(out2,  NLON,NLAT,1, GDT_Float32, NULL);
                    if (varAccro=="tn" |varAccro=="tx" | varAccro=="tg" | varAccro=="rr"){
                        pMensuelRaster = pDriver->Create(out2,  NLON,NLAT,1, GDT_Float32, NULL);
                    } else {// qq
                        pMensuelRaster = pDriver->Create(out2,  NLON,NLAT,1, GDT_Int16, NULL);
                    }
                    pMensuelRaster->SetGeoTransform( transform );
                    pRaster->SetSpatialRef(pTemplateRaster->GetSpatialRef());
                    GDALClose(pTemplateRaster);

                    int c(1);
                    float *varbuf;
                    varbuf = (float *) CPLMalloc( sizeof( float ) * 1 );
                    //std::cout << "loop " << std::endl;
                    for (int rec : v){
                        // ou stoquer les bandes de une date en attendant leur utilisation? quelles structures de donnée?
                        //for (int rec = 0; rec < 3; rec++)//NREC; rec++)

                        //std::cout << "lecture des données pour time " << rec << std::endl;

                        if (!timeVar->set_cur(rec))
                            return NC_ERR;
                        float time_in;
                        if (!timeVar->get(&time_in, 1))
                            return NC_ERR;
                        year_month_day ymd = sys_days{baseymd} + days{(int (time_in))};
                        //std::cout << "date :" << ymd << std::endl;

                        // Read the data one record at a time.
                        //std::cout << "invar set cur \n" << std::endl;
                        if (!inVar->set_cur(rec, 0, 0))
                            return NC_ERR;

                        //std::cout << "getvarin \n" << std::endl;
                        if (!inVar->get(&var_in[0][0], 1, NLAT, NLON))
                            return NC_ERR;

                        for (int lat = 0; lat < NLAT; lat++){
                            for (int lon = 0; lon < NLON; lon++){
                                varbuf= &var_in[lat][lon] ;
                                //std::cout << " value= "<< var_in[lat][lon] << std::endl;
                                pRaster->GetRasterBand(c)->RasterIO(GF_Write, lon, NLAT-(lat+1), 1,1, varbuf,1, 1,GDT_Float32, 0, 0 );
                            }
                        }
                        c++;


                    }
                    // CPLFree(varbuf); // invalid pointer, pas de free donc

                    // on rassemble toutes les couches en une seule..
                    for (int x(0); x < pRaster->GetRasterXSize(); x++){
                        for (int y(0); y < pRaster->GetRasterYSize(); y++){
                            // lecture de toute les bandes
                            std::vector<float> vVar;
                            float *varbuf=(float *) CPLMalloc( sizeof( float ) * 1 );
                            for (int date(1); date < pRaster->GetRasterCount()+1;date++){

                                pRaster->GetRasterBand(date)->RasterIO(GF_Read, x,y, 1,1, varbuf,1, 1,GDT_Float32, 0, 0 );
                                float v=varbuf[0];
                                // enlever les no data -9999

                                if (v!=-9999){v= v*scaleF;
                                    //std::cout << " valeur pour " << date << " est de " << v << std::endl;
                                    if (DG){if (v>baseDJ){v=v-baseDJ;} else {v=0;}}
                                    vVar.push_back(v);
                                }
                            }

                            float sum_of_elems = std::accumulate(vVar.begin(), vVar.end(), 0);
                            //std::cout << " valeur finale de " << sum_of_elems << std::endl;
                            // moyenne de temp
                            if (!DG){
                                if (varAccro=="tn" |varAccro=="tx" | varAccro=="tg" ){
                                    sum_of_elems=sum_of_elems/ndayInM;
                                }}
                            // somme pour précipitation et radiation

                            varbuf=&sum_of_elems;
                            pMensuelRaster->GetRasterBand(1)->RasterIO(GF_Write, x,y, 1,1, varbuf,1, 1,GDT_Float32, 0, 0 );
                        }
                    }

                    GDALClose(pRaster);
                    GDALClose(pMensuelRaster);

                    projectRasterCallgdal(output2,output3);

            // je projecte le raster mensuel:
            GDALDatasetH hSrcDS  = GDALOpenEx( out2, GDAL_OF_VECTOR | GDAL_OF_READONLY, NULL, NULL, NULL );
            //GDALDataset  * DS = (GDALDataset *) GDALOpen( out2, GA_ReadOnly );
            char** papszArgv = nullptr;
            papszArgv = CSLAddString(papszArgv, "-t_srs"); // target src with reprojection
            papszArgv = CSLAddString(papszArgv, "EPSG:31370");
            papszArgv = CSLAddString(papszArgv, "-te");
            papszArgv = CSLAddString(papszArgv, "42250.0");
            papszArgv = CSLAddString(papszArgv, "21170.0");
            papszArgv = CSLAddString(papszArgv, "295170.0");
            papszArgv = CSLAddString(papszArgv, "167700.0");
            papszArgv = CSLAddString(papszArgv, "-te_srs");
            papszArgv = CSLAddString(papszArgv, "EPSG:31370");

             GDALWarpOptions * option =GDALWarpOptions(papszArgv);
             if (option){

                 GDALDatasetH hOutDS = GDALWarp(out3,nullptr,1,DS,option,nullptr);

                 GDALClose(hOutDS);
                GDALDestroyWarpOptions(option);
             }


            //GDALVectorTranslateOptions * option = GDALVectorTranslateOptionsNew(papszArgv, nullptr);
             GDALTranslateOptions * option = GDALTranslateOptionsNew(papszArgv,nullptr);
             GDALTranslateOptionsFree(option);
             if (option){

                 GDALDatasetH hOutDS = GDALTranslate(out3,DS,option,nullptr);
                 GDALClose(hOutDS);
                 GDALTranslateOptionsFree(option);
             }
          if( option ){
                GDALDatasetH hOutDS = GDALVectorTranslate(out3,nullptr,1,&hSrcDS,option,nullptr);
                if( hOutDS ){
                    GDALClose(hOutDS);
                } else {  std::cout << "hOutDS null" << std::endl;}
            }else{
                std::cout << "no valid option for gdal translate" << std::endl;
            }
            GDALVectorTranslateOptionsFree(option);

                    // GDALClose(hSrcDS);

                }
            }


        }
    }
}

int timeFromYMD(year_month_day ymd, NcVar *timeVar, int nrec){
    int aRes(0);
    // déterminer la date de la première bande
    if (!timeVar->set_cur((int (0))))
        return aRes;
    float time_in;
    if (!timeVar->get(&time_in, 1))
        return aRes;
    year_month_day ymdRec0 = sys_days{baseymd} + days{(int (time_in))};

    if (!timeVar->set_cur(nrec-1))
        return aRes;
    if (!timeVar->get(&time_in, 1))
        return aRes;

    days d=sys_days{ymd}-sys_days{ymdRec0};

    if ((d>=days(0))){
        aRes=d.count()+1; // j'ai testé manuellement et en fait il faut ajouter 1, donc faire pareil pour fonction timesForOneMonth!!!!! attention!
    } else {
        std::cout << "timefrom ymd ; date pas dans la série tempo "<< std::endl;
    }
    return aRes;
}

std::vector<int> timesForOneMonth(year_month ym, NcVar *timeVar, int nrec){
    std::vector<int> aRes;
    // déterminer la date de la première bande
    if (!timeVar->set_cur((int (0))))
        return aRes;
    float time_in;
    if (!timeVar->get(&time_in, 1))
        return aRes;
    year_month_day ymdRec0 = sys_days{baseymd} + days{(int (time_in))};
    if (!timeVar->set_cur(nrec-1))
        return aRes;
    if (!timeVar->get(&time_in, 1))
        return aRes;
    year_month_day ymdRecLast=sys_days{baseymd} + days{(int (time_in))};
    year_month_day ymdIn(ym.year(),ym.month(),day(1));
    days d=sys_days{ymdIn}-sys_days{ymdRec0};
    days d2=sys_days{ymdIn}-sys_days{ymdRecLast};
    if ((d>days(0)) && d2<days(0)){
        // d c'est l'id du premier record pour le moi
        int c(1);
        while (ym.month()==ymdIn.month()){
            //std::cout << "jour " << c << std::endl;
            d=sys_days{ymdIn}-sys_days{ymdRec0};
            //std::cout << " sys day " << d.count() << std::endl;
            //int daynum=sys_days{d};
            aRes.push_back(d.count()+1);
            ymdIn=sys_days{ymdIn}+days(1);
            c++;
        }

    } else {
        std::cout << "timesForOneMonth ; moi pas dans la série tempo "<< std::endl;
    }
    //std::cout << aRes.size() <<" bandes sélectionnées "<< std::endl;
    return aRes;

}
std::vector<int> timesForDateForDJ(year_month_day ymd, NcVar *timeVar, int nrec){
    std::vector<int> aRes;
    // déterminer la date de la première bande
    if (!timeVar->set_cur((int (0))))
        return aRes;
    float time_in;
    if (!timeVar->get(&time_in, 1))
        return aRes;
    year_month_day ymdRec0 = sys_days{baseymd} + days{(int (time_in))};
    if (!timeVar->set_cur(nrec-1))
        return aRes;
    if (!timeVar->get(&time_in, 1))
        return aRes;
    year_month_day ymdRecLast=sys_days{baseymd} + days{(int (time_in))};

    // date à partir de laquelle on considère le calcul des DJ. 1 mars?
    year_month_day ymdDebut(ymd.year(),month(2),day(15));

    days d=sys_days{ymdDebut}-sys_days{ymdRec0};
    days d2=sys_days{ymdDebut}-sys_days{ymdRecLast};
    days d3=sys_days{ymd}-sys_days{ymdDebut};

    if ((d>days(0)) && d2<days(0) && d3>days(0)){
        // d c'est l'id du premier record pour le mois
        int c(1);
        while (ymd!=ymdDebut){

            d=sys_days{ymdDebut}-sys_days{ymdRec0};
            aRes.push_back(d.count()+1);
            ymdDebut=sys_days{ymdDebut}+days(1);
            c++;
        }

    } else {
        std::cout << "timesForDateForDJ ; problème avec date "<< std::endl;
    }
    std::cout << aRes.size() <<" bandes sélectionnées "<< std::endl;
    return aRes;
}

// fonctionne pas probablement car c'est du code pour reprojecter, et le src in est wgs84 donc sans projection..
void projectRaster(std::string inPath, std::string outPath){
    GDALDriverH hDriver;
    GDALDataType eDT;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;

    // Open the source file.
    hSrcDS = GDALOpen( inPath.c_str(), GA_ReadOnly );
    CPLAssert( hSrcDS != NULL );

    // Create output with same datatype as first input band.
    eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1));

    // Get output driver (GeoTIFF format)
    hDriver = GDALGetDriverByName( "GTiff" );
    CPLAssert( hDriver != NULL );

    // Get Source coordinate system.
    char *pszSrcWKT=NULL;
    char  *pszDstWKT = NULL;

    OGRSpatialReference oSRS;
    OGRErr err=oSRS.importFromEPSG(31370);
    oSRS.exportToWkt( &pszDstWKT );
    std::cout << " pszDstWKT " << pszDstWKT << std::endl;

    //pszSrcWKT = GDALGetProjectionRef( hSrcDS );
    //CPLAssert( pszSrcWKT != NULL && strlen(pszSrcWKT) > 0 );
    //std::cout << " pszDstWKT " << pszSrcWKT << std::endl;


    OGRSpatialReference oSRSIn;
    err=oSRSIn.importFromEPSG(4326);
    oSRSIn.exportToWkt( &pszSrcWKT );
    std::cout << " pszSrcWKT " << pszSrcWKT << std::endl;

    //std::cout << "OGR error setwellKnownGeog : " << err << std::endl;

    // Create a transformer that maps from source pixel/line coordinates
    // to destination georeferenced coordinates (not destination
    // pixel line).  We do that by omitting the destination dataset
    // handle (setting it to NULL).
    void *hTransformArg;
    hTransformArg =
            GDALCreateGenImgProjTransformer( hSrcDS, pszSrcWKT, NULL, pszDstWKT,
                                             FALSE, 0, 1 );
    CPLAssert( hTransformArg != NULL );

    // Get approximate output georeferenced bounds and resolution for file.
    double adfDstGeoTransform[6];
    int nPixels=0, nLines=0;
    CPLErr eErr;
    eErr = GDALSuggestedWarpOutput( hSrcDS,
                                    GDALGenImgProjTransform, hTransformArg,
                                    adfDstGeoTransform, &nPixels, &nLines );
    CPLAssert( eErr == CE_None );
    GDALDestroyGenImgProjTransformer( hTransformArg );

    // Create the output file.
    hDstDS = GDALCreate( hDriver, outPath.c_str(), nPixels, nLines,
                         GDALGetRasterCount(hSrcDS), eDT, NULL );
    CPLAssert( hDstDS != NULL );

    // Write out the projection definition.
    GDALSetProjection( hDstDS, pszDstWKT );
    GDALSetGeoTransform( hDstDS, adfDstGeoTransform );

    // Setup warp options.
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->hSrcDS = hSrcDS;
    psWarpOptions->hDstDS = hDstDS;
    psWarpOptions->nBandCount = 1;
    psWarpOptions->panSrcBands =
            (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panSrcBands[0] = 1;
    psWarpOptions->panDstBands =
            (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panDstBands[0] = 1;
    psWarpOptions->pfnProgress = GDALTermProgress;

    // Establish reprojection transformer.
    psWarpOptions->pTransformerArg =
            GDALCreateGenImgProjTransformer( hSrcDS,
                                             GDALGetProjectionRef(hSrcDS),
                                             hDstDS,
                                             GDALGetProjectionRef(hDstDS),
                                             FALSE, 0.0, 1 );
    psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

    // Initialize and execute the warp operation.
    GDALWarpOperation oOperation;
    oOperation.Initialize( psWarpOptions );
    oOperation.ChunkAndWarpImage( 0, 0,
                                  GDALGetRasterXSize( hDstDS ),
                                  GDALGetRasterYSize( hDstDS ) );
    GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
    GDALDestroyWarpOptions( psWarpOptions );
    GDALClose( hDstDS );
    GDALClose( hSrcDS );

}

void projectRasterCallgdal(std::string inPath, std::string outPath){

    std::string aCommand= std::string("gdalwarp -s_srs EPSG:4326 -t_srs EPSG:31370 -te 32250.0 11170.0 305170.0 177700.0 -tr 5000 5000 -r average -overwrite ")
            + inPath +" "+outPath+" ";
    //std::cout << aCommand << "\n";
    system(aCommand.c_str());
}

// fonctionne pas encore car pas de facteur multiplicatif lors de l'export !!

void exportCarteUneDate(year_month_day ymd, std::string varAccro){
    double scaleF(1);
    if (varAccro=="tn" |varAccro=="tx" | varAccro=="tg"){
        scaleF=0.0099999998;
    }
    if (varAccro=="rr"){
        scaleF=0.1;
    }
    std::string ncpath("/home/lisein/Documents/Scolyte/Data/climat/01deg/"+varAccro+"_ens_mean_0.1deg_reg_2011-2020_v22.0e.nc");
    boost::filesystem::path ap(ncpath);

    const char * p=ncpath.c_str();

    NcFile in(p,NcFile::FileMode::ReadOnly,NULL,0,NcFile::FileFormat::Netcdf4);
    if (!in.is_valid())
    {
        std::cout << "Couldn't open file!\n";

    } else{

        std::cout << "input File has " << in.num_dims() << " dimensions and " <<in.num_vars() << " vars and "  << in.num_atts() << " atts" << std::endl ;

        NcVar *timeVar;
        timeVar=in.get_var("time");
        std::cout << "timevar->name() " <<timeVar->name() << " " << timeVar->id() << " id " <<std::endl;
        NcDim * time=in.get_dim("time");
        std::cout << " size of dim " << time->name() << " " << time->size() << std::endl;
        int NREC = time->size();

        int rec= timeFromYMD(ymd, timeVar, NREC);

        std::string parentp(ap.parent_path().c_str());
        std::string aOut1( parentp+"/"+varAccro+"_date.tiff");
        std::string aOut2( parentp+"/"+varAccro+"_date_BL72.tiff");
        std::string aCommand= std::string("gdal_translate -of GTiff -b "+std::to_string(rec)+ " " +ncpath.c_str()+ " "+aOut1 );
        std::cout << aCommand << std::endl;
        system(aCommand.c_str());

        projectRasterCallgdal(aOut1,aOut2);

    }

}

void calculDJPourDate(){
    std::cout << "début" ;
    // connection avec BD sqlite pour récupérer liste de date
    int rc;
    sqlite3 *db_;

    std::vector<dateSite> vDatesSites;
    std::cout << "ouvre connexion avec BD observation OWSF ..." ;

    rc = sqlite3_open_v2(mBDpath.c_str(), &db_,SQLITE_OPEN_READONLY,NULL);
    // The 31 result codes are defined in sqlite3.h
    //SQLITE_ERROR (1)
    if( rc!=0) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db_));
        std::cout << " mBDpath " << mBDpath << std::endl;
        std::cout << "result code : " << rc << std::endl;
    } else {

        std::cout << "connexion ok ..." ;
        sqlite3_stmt * stmt;
        //std::string SQLstring="SELECT DISTINCT pose, releve FROM observation_site WHERE pose is not null;";

        std::string SQLstring="SELECT DISTINCT observation_site.site, observation_site.pose, observation_site.releve,  owsf_piegeSco_loca.X,owsf_piegeSco_loca.Y FROM observation_site INNER JOIN owsf_piegeSco_loca ON observation_site.site=owsf_piegeSco_loca.SITE WHERE observation_site.pose is not null;";
        sqlite3_prepare_v2( db_, SQLstring.c_str(), -1, &stmt, NULL );//preparing the statement
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            if (sqlite3_column_type(stmt, 0)!=SQLITE_NULL && sqlite3_column_type(stmt, 1)!=SQLITE_NULL){
                //std::cout <<"lecture observation" << std::endl;
                std::string id=std::string( (char *)sqlite3_column_text( stmt, 0 ) );
                std::string pose=std::string( (char *)sqlite3_column_text( stmt, 1 ) );
                std::string releve=std::string( (char *)sqlite3_column_text( stmt, 2 ) );
                double X=sqlite3_column_double(stmt, 3 );
                double Y=sqlite3_column_double(stmt, 4 );

                dateSite ds(id,pose,releve,X,Y);
                vDatesSites.push_back(ds);
            }
        }
        sqlite3_finalize(stmt);
        std::cout <<"fin requete db" << std::endl;
    }

    sqlite3_close_v2(db_);
    std::vector<year_month_day> vDates;
    for (dateSite ds : vDatesSites){

        bool test(1);
        for (year_month_day ymd : vDates){
            if (ymd==ds.pose){test=0;}
        }
        if (test){vDates.push_back(ds.pose);}
        test=1;
        for (year_month_day ymd : vDates){
            if (ymd==ds.releve){test=0;}
        }
        if (test){vDates.push_back(ds.releve);}

    }


    int c(0);
    for (year_month_day ymd : vDates){

        // if (c>98){
        std::cout <<ymd <<  " date " << c << " sur total de " << vDates.size() << std::endl;

        year_month_day ymdDebut(ymd.year(),month(2),day(15));
        days d=sys_days{ymd}-sys_days{ymdDebut};

        // la limite du dataset est 30 juin 2020
        year_month_day ymdFin(year(2020),month(6),day(29));
        days d2=sys_days{ymdFin}-sys_days{ymd};

        if (d>days(0) & d2>days(0)){
            exportCarteDJUneDate(ymd);

            for (dateSite & ds : vDatesSites){
                if (ds.pose==ymd){
                    std::cout << "-" << std::endl;
                    ds.djpose= getValue("/home/lisein/Documents/Scolyte/Data/climat/01deg/tmp2.tif",ds.X,ds.Y);
                }
                if (ds.releve==ymd){
                    std::cout << "-" << std::endl;
                    ds.djrel= getValue("/home/lisein/Documents/Scolyte/Data/climat/01deg/tmp2.tif",ds.X,ds.Y);
                }

            }
        }
        //   }
        c++;
    }
    // sauve les stats dans un fichier texte
    std::ofstream aOut("/home/lisein/Documents/Scolyte/Data/owsfDJtx.csv");
    aOut.precision(3);

    for (dateSite ds : vDatesSites){

        aOut << ds.cat() ; aOut << "\n";
    }
    aOut.close();

}

void exportCarteDJUneDate(year_month_day ymd){
    //std::cout << " exportCarteDJUneDate " << ymd << std::endl;
    double scaleF(0.0099999998);
    // std::string varAccro("tg"); moyenne
    std::string varAccro("tx"); // temperature max

    std::string ncpath("/home/lisein/Documents/Scolyte/Data/climat/01deg/"+varAccro+"_ens_mean_0.1deg_reg_2011-2020_v22.0e.nc");
    boost::filesystem::path ap(ncpath);
    std::string tifModel("/home/lisein/Documents/Scolyte/Data/climat/01deg/template_"+varAccro+".tif");

    const char * p=ncpath.c_str();

    NcFile in(p,NcFile::FileMode::ReadOnly,NULL,0,NcFile::FileFormat::Netcdf4);
    if (!in.is_valid())
    {
        std::cout << "Couldn't open file!\n";

    } else{

        NcVar *latVar, *lonVar;
        latVar = in.get_var("latitude");
        lonVar = in.get_var("longitude");
        NcVar *inVar,*timeVar;
        timeVar=in.get_var("time");
        NcDim * time=in.get_dim("time");
        int NREC = time->size();
        NcDim * l=in.get_dim("longitude");
        int NLON = l->size();
        NcDim * lati=in.get_dim("latitude");
        int NLAT = lati->size();
        float var_in[NLAT][NLON];
        inVar=in.get_var(3);

        // revoie tout les "time" à prendre en compte pour le calcul des DJ pour cette date.
        std::vector<int> v= timesForDateForDJ(ymd, timeVar, NREC);
        int ndayInM = v.size();

        GDALDriver *pDriver,*pDriverMEM;
        GDALDataset *pRaster=NULL,*pTemplateRaster=NULL,*pMensuelRaster=NULL;

        const char *inputTemplate=tifModel.c_str();
        const char *pszFormat = "GTiff";

        //std::cout << "lecture tiff template et création mem raster avec autant de bandes que de jours " << std::endl;
        pDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
        pTemplateRaster = (GDALDataset*) GDALOpen(inputTemplate, GA_ReadOnly);

        const char *pszFormat2 = "MEM";
        pDriverMEM = GetGDALDriverManager()->GetDriverByName(pszFormat2);
        std::string output("/vsimem/tmp.tif");
        //std::string output("/home/lisein/Documents/tmp.tif");
        const char *out=output.c_str();
        pRaster = pDriverMEM->Create(out,  NLON,NLAT, v.size(), GDT_Int16, NULL);
        pRaster->SetSpatialRef(pTemplateRaster->GetSpatialRef());
        double transform[6];
        pTemplateRaster->GetGeoTransform(transform);
        pRaster->SetGeoTransform( transform );

        // le tif qui synthétisera les info mensuelles---------------------------------------
        std::string output2("/home/lisein/Documents/Scolyte/Data/climat/01deg/tmp1.tif");
        const char *out2=output2.c_str();
        std::string output3="/home/lisein/Documents/Scolyte/Data/climat/01deg/tmp2.tif";
        const char *out3=output3.c_str();

        pMensuelRaster = pDriver->Create(out2,  NLON,NLAT,1, GDT_Float32, NULL);
        pMensuelRaster->SetGeoTransform( transform );
        pRaster->SetSpatialRef(pTemplateRaster->GetSpatialRef());
        GDALClose(pTemplateRaster);
        int c(1);
        float *varbuf;
        varbuf = (float *) CPLMalloc( sizeof( float ) * 1 );
        //std::cout << "loop " << std::endl;
        for (int rec : v){
            timeVar->set_cur(rec);

            float time_in;
            timeVar->get(&time_in, 1);

            year_month_day ymd = sys_days{baseymd} + days{(int (time_in))};

            // Read the data one record at a time.
            inVar->set_cur(rec, 0, 0);
            inVar->get(&var_in[0][0], 1, NLAT, NLON);

            for (int lat = 0; lat < NLAT; lat++){
                for (int lon = 0; lon < NLON; lon++){
                    varbuf= &var_in[lat][lon] ;
                    pRaster->GetRasterBand(c)->RasterIO(GF_Write, lon, NLAT-(lat+1), 1,1, varbuf,1, 1,GDT_Float32, 0, 0 );
                }
            }
            c++;
        }
        // on rassemble toutes les couches en une seule..
        for (int x(0); x < pRaster->GetRasterXSize(); x++){
            for (int y(0); y < pRaster->GetRasterYSize(); y++){
                // lecture de toute les bandes
                std::vector<float> vVar;
                float *varbuf=(float *) CPLMalloc( sizeof( float ) * 1 );
                for (int date(1); date < pRaster->GetRasterCount()+1;date++){
                    pRaster->GetRasterBand(date)->RasterIO(GF_Read, x,y, 1,1, varbuf,1, 1,GDT_Float32, 0, 0 );
                    float v=varbuf[0];
                    // enlever les no data -9999
                    if (v!=-9999){v= v*scaleF;
                        // calcul degré jour pour ce jour là
                        if (v>baseDJ){v=v-baseDJ;} else {v=0;}
                        vVar.push_back(v);
                    }
                }
                float sum_of_elems = std::accumulate(vVar.begin(), vVar.end(), 0);
                varbuf=&sum_of_elems;
                pMensuelRaster->GetRasterBand(1)->RasterIO(GF_Write, x,y, 1,1, varbuf,1, 1,GDT_Float32, 0, 0 );
            }
        }
        GDALClose(pRaster);
        GDALClose(pMensuelRaster);
        projectRasterCallgdal(output2,output3);
    }
}

// converti le format qui est dans la bd owsf vers le type year_month_day
year_month_day ymdFromString(std::string date){
    int d=std::stoi(date.substr(0,2));
    int m=std::stoi(date.substr(3,4));
    int y=std::stoi(date.substr(6,9));

    //std::cout << "y " << y << " m " << m << " d " << d << std::endl;
    year_month_day ymd(year{y},month{m},day{d});
    return ymd;
}

int getValue(std::string pathTif, double x, double y){

    int aRes(0);
    GDALDataset  * mGDALDat = (GDALDataset *) GDALOpen( pathTif.c_str(), GA_ReadOnly );
    if( mGDALDat == NULL )
    {
        std::cout << "je n'ai pas lu l'image " << pathTif << std::endl;
    } else {
        GDALRasterBand * mBand = mGDALDat->GetRasterBand( 1 );

        double transform[6];
        mGDALDat->GetGeoTransform(transform);
        double xOrigin = transform[0];
        double yOrigin = transform[3];
        double pixelWidth = transform[1];
        double pixelHeight = -transform[5];

        int col = int((x - xOrigin) / pixelWidth);
        int row = int((yOrigin - y ) / pixelHeight);

        if (col<mBand->GetXSize() && row < mBand->GetYSize()){
            float *scanPix;
            scanPix = (float *) CPLMalloc( sizeof( float ) * 1 );
            // lecture du pixel
            mBand->RasterIO( GF_Read, col, row, 1, 1, scanPix, 1,1, GDT_Float32, 0, 0 );
            aRes=scanPix[0];
            CPLFree(scanPix);
            GDALClose( mGDALDat );
            mBand=NULL;
        }
    }

    return aRes;
}
*/


void processIRMData(){

    irmData d(irmDataFile);
    if (0){
    for (int y : vYears){
        for (int m : vMonths){
            std::cout << " calcul valeur mensuelles pour " << y << "/" << m << std::endl;
            if (0){
            dataOneDate mens=d.dataMensuel(year{y},month{m});
            mens.exportMap("Tmean_"+std::to_string(y)+"_"+std::to_string(m),"Tmean");
            mens.exportMap("Tmax_"+std::to_string(y)+"_"+std::to_string(m),"Tmax");
            mens.exportMap("Tmin_"+std::to_string(y)+"_"+std::to_string(m),"Tmin");
            mens.exportMap("ETP_"+std::to_string(y)+"_"+std::to_string(m),"ETP");
            mens.exportMap("P_"+std::to_string(y)+"_"+std::to_string(m),"P");
            mens.exportMap("R_"+std::to_string(y)+"_"+std::to_string(m),"R");
            }
            dataOneDate mens=d.dataMensuelDJ(year{y},month{m});
            mens.exportMap("DJ_"+std::to_string(y)+"_"+std::to_string(m),"Tmean");
            //break;
        }
    }
    std::cout << " calcul valeur moyenne sur toute les données" << std::endl;
    dataOneDate moy=d.moyAll();
    moy.exportMap("Tmean_moy","Tmean");
    moy.exportMap("ETP_moy","ETP");
    moy.exportMap("P_moy","P");
    moy.exportMap("R_moy","R");
    }

    // 2021 06 22 on recois P et ET0 trentenaire, je les traite ici

    // carte annuelle:
    dataOneDate da= d.dataAnnuel(year{1});
    da.exportMap("ETP_30aire","ETP");
    da.exportMap("P_30aire","P");
    for (int m : {1,2,3,4,5,6,7,8,9,10,11,12}){
        std::cout << " calcul valeur mensuelles trentenaire "<< std::endl;

        dataOneDate mens=d.dataMensuel(year{1},month{m});
        mens.exportMap("ETP_30aire_"+std::to_string(m),"ETP");
        mens.exportMap("P_30aire_"+std::to_string(m),"P");
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
