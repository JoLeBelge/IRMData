#include "ecaddata.h"

extern year_month_day baseymd;

bool DG(0);


ecadData::ecadData()
{

}

int ecadData::calculCarteMensuelTrentenaire(year y1, year y2, std::string varAccro, resumeMensuel type, std::string prefix){

    std::cout << " traitement variable climatique " << varAccro << ", calcul normale mensuelle entre " << y1 << " et " << y2 << std::endl;

    double scaleF=getScaleF(varAccro);
    std::string ncpath=getNamePath(varAccro);
    boost::filesystem::path ap(ncpath);
    std::string tifModel=getNameTifModel(varAccro,1);
    const char * p=ncpath.c_str();

    NcFile in(p,NcFile::FileMode::ReadOnly,NULL,0,NcFile::FileFormat::Netcdf4);
    if (!in.is_valid())
    {
        std::cout << "Couldn't open file!\n";
        return NC_ERR;
    } else{

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
        timeVar=in.get_var("time");
        std::cout << "timevar->name() " <<timeVar->name() << " id " << timeVar->id() <<std::endl;

        NcDim * time=in.get_dim("time");
        std::cout << " size of dim " << time->name() << " id " << time->size() << std::endl;
        int NREC = time->size();
        NcDim * l=in.get_dim("longitude");
        std::cout << " var" << l->name() << " id " << l->id() << std::endl;
        int NLON = l->size();
        NcDim * lati=in.get_dim("latitude");
        std::cout << " var " << lati->name() << " " << lati->id() << std::endl;
        int NLAT = lati->size();

        float var_in[NLAT][NLON];

        //inVar=in.get_var(2); // attention pour la version 24 c'est l'id 2, pas le 3!!!
        inVar=in.get_var(varAccro.c_str());
        std::cout << "invar->name() " <<inVar->name() << "\n\n\n"<< std::endl;

        int nbY=((int) y2-(int)y1)+1;

        std::cout << "nombre d'année : " << nbY << std::endl;

        // boucle sur chaque année de la période trimestrielle et ensuite sur chaque moi de cette année.
        for (int m(1) ; m <13 ; m++){
            std::cout << " calcul valeur mensuelles  trentenaire pour " << m << std::endl;
            std::string aName(varAccro+getMode(varAccro,type)+"_"+std::to_string(m)+"_"+prefix+std::to_string(nbY));
            std::string output2("/home/lisein/Documents/Scolyte/Data/climat/eobs24/trentenaire/"+aName+".tif");
            const char *out2=output2.c_str();
            std::string output3="/home/lisein/Documents/Scolyte/Data/climat/eobs24/trentenaire/"+aName+"_BL72.tif";

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
            const char *out=output.c_str();

            int ndayInMTot(0);
            for (int y(0) ; y < nbY ; y++){
                year_month ym(year{y+(int)y1}/m);
                std::vector<int> v= timesForOneMonth(ym, timeVar, NREC);
                ndayInMTot+=v.size();
            }

            std::cout << "nombre de jour total : "<< ndayInMTot << std::endl;

            pRaster = pDriverMEM->Create(out,  NLON,NLAT, ndayInMTot, GDT_Int16, NULL);
            pRaster->SetSpatialRef(pTemplateRaster->GetSpatialRef());
            double transform[6];
            pTemplateRaster->GetGeoTransform(transform);
            pRaster->SetGeoTransform( transform );


            if (varAccro=="tn" |varAccro=="tx" | varAccro=="tg" | varAccro=="rr" | varAccro=="pp"){
                pMensuelRaster = pDriver->Create(out2,  NLON,NLAT,1, GDT_Float32, NULL);
            } else {// qq et pp
                pMensuelRaster = pDriver->Create(out2,  NLON,NLAT,1, GDT_Int16, NULL);
            }
            pMensuelRaster->SetGeoTransform( transform );

            OGRSpatialReference src;
            src.importFromEPSG(4326);
            char *wgs;
            src.exportToProj4(&wgs);
            pMensuelRaster->SetProjection(wgs);
            GDALClose(pTemplateRaster);

            int c(1);
            for (int y(0) ; y < nbY ; y++){
                year_month ym(year{y+(int)y1}/m);
                std::cout << " ajout des dates de " << std::to_string((int)ym.year()) << "," << std::to_string(m) << std::endl ;

                std::vector<int> v= timesForOneMonth(ym, timeVar, NREC);
                int ndayInM = v.size();


                float *varbuf;
                varbuf = (float *) CPLMalloc( sizeof( float ) * 1 );
                //std::cout << "loop " << std::endl;
                for (int rec : v){
                    if (!timeVar->set_cur(rec))
                        return NC_ERR;
                    float time_in;
                    if (!timeVar->get(&time_in, 1))
                        return NC_ERR;
                    year_month_day ymd = sys_days{baseymd} + days{(int (time_in))};
                    //std::cout << "date :" << ymd << std::endl;
                    // Read the data one record at a time.

                    if (!inVar->set_cur(rec, 0, 0)){
                        std::cout << "invar set cur bad \n" << std::endl;
                        return NC_ERR;
                    }


                    if (!inVar->get(&var_in[0][0], 1, NLAT, NLON)){
                        std::cout << "getvarin bad\n" << std::endl;
                        return NC_ERR;
                    }

                    for (int lat = 0; lat < NLAT; lat++){
                        for (int lon = 0; lon < NLON; lon++){
                            varbuf= &var_in[lat][lon] ;
                            //std::cout << " value= "<< var_in[lat][lon] << std::endl;
                            // numéro de bande ; pas c, il faut additionner toute les bandes des autres mois
                            pRaster->GetRasterBand(c)->RasterIO(GF_Write, lon, NLAT-(lat+1), 1,1, varbuf,1, 1,GDT_Float32, 0, 0 );
                        }
                    }
                    c++;
                }

                // fin boucle sur année
            }

            std::cout << "on concatène toutes les bandes en une seule"<< std::endl;
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

                        // elever les no data -9999

                        if (v!=-9999){
                            v= v*scaleF;
                            // std::cout << " valeur de " << v << std::endl;
                            //std::cout << " valeur pour " << date << " est de " << v << std::endl;
                            vVar.push_back(v);
                        }
                    }


                    //std::cout << " valeur finale de " << sum_of_elems << std::endl;
                    int ntot=ndayInMTot;
                    if (vVar.size()>0 && vVar.size()!= ndayInMTot){
                        //std::cout << " attention, il y a des no data pour " << varAccro << ", nombre d'observation " << vVar.size() << " sur un total de " << ndayInMTot << std::endl;
                        ntot=vVar.size();
                    }

                    if (varAccro=="tn" |varAccro=="tx" | varAccro=="tg" | varAccro=="pp" ){

                    } else {
                       type=resumeMensuel::sum;
                    }

                    float sum_of_elems;
                    switch (type) {
                    case resumeMensuel::moy:{
                         sum_of_elems = std::accumulate(vVar.begin(), vVar.end(), 0);
                         sum_of_elems=sum_of_elems/ntot;
                        break;
                    }
                    case resumeMensuel::sum:{
                         sum_of_elems = std::accumulate(vVar.begin(), vVar.end(), 0);
                         sum_of_elems=sum_of_elems/((double)nbY)*(((double) ndayInMTot)/((double) ntot));
                        break;
                    }
                    case resumeMensuel::min:{

                        if (vVar.size()>0){
                        int index =std::min_element(vVar.begin(),vVar.end()) - vVar.begin();

                        sum_of_elems =vVar.at(index);}else {
                            sum_of_elems =0;
                        }
                        break;
                    }
                    case resumeMensuel::max:{
                        if (vVar.size()>0){
                        int index =std::max_element(vVar.begin(),vVar.end()) - vVar.begin();
                        sum_of_elems =vVar.at(index);}else {
                            sum_of_elems =0;
                        }
                        break;
                    }

                    default:
                        break;
                    }

                    varbuf=&sum_of_elems;
                    pMensuelRaster->GetRasterBand(1)->RasterIO(GF_Write, x,y, 1,1, varbuf,1, 1,GDT_Float32, 0, 0 );
                }
            }

            GDALClose(pRaster);
            GDALClose(pMensuelRaster);
            projectRasterCallgdal(output2,output3);

        }


    }
}




int ecadData::calculCarteMensuel(std::vector<int> avYears, std::string varAccro, resumeMensuel type){
    //std::vector<std::string> accro{"qq", "tn", "tx", "pp", "rr", "tg"};
    // pp c'est la pression qui est de 1000 par jour donc somme par mois est de 30000 et ça fonctionne pas
    //std::vector<std::string> accro{ "tn", "tx", "rr", "tg","qq"};
    //std::vector<std::string> accro{"tn","tx","tg"};
    std::cout << " traitement variable climatique " << varAccro << std::endl;

    // je regarde sous qgis les scale factor ; qq = 1, rr = 0.1, temp = 0.0099999998
    double scaleF=getScaleF(varAccro);
    std::string ncpath=getNamePath(varAccro);
    boost::filesystem::path ap(ncpath);
    std::string tifModel=getNameTifModel(varAccro,2);
    const char * p=ncpath.c_str();

    NcFile in(p,NcFile::FileMode::ReadOnly,NULL,0,NcFile::FileFormat::Netcdf4);
    if (!in.is_valid())
    {
        std::cout << "Couldn't open file!\n";
        return NC_ERR;
    } else{

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

        //inVar=in.get_var(2);
        inVar=in.get_var(varAccro.c_str());
        std::cout << "invar->name() " <<inVar->name() << std::endl;

        for (int y : avYears){
            for (int m(1) ; m< 13; m++){
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
                std::string aName(varAccro+getMode(varAccro,type)+"_"+std::to_string(int (ym.year()))+"_"+format("%m",ym));
                if (DG){aName=aName+"_DG";}

                std::string output2("/home/lisein/Documents/Scolyte/Data/climat/eobs24/trentenaire/"+aName+".tif");
                const char *out2=output2.c_str();
                std::string output3("/home/lisein/Documents/Scolyte/Data/climat/eobs24/trentenaire/"+aName+"_BL72.tif");


                //pMensuelRaster = pDriver->Create(out2,  NLON,NLAT,1, GDT_Float32, NULL);
                if (varAccro=="tn" |varAccro=="tx" | varAccro=="tg" | varAccro=="rr" | varAccro=="pp"){
                    pMensuelRaster = pDriver->Create(out2,  NLON,NLAT,1, GDT_Float32, NULL);
                } else {// qq et pp
                    pMensuelRaster = pDriver->Create(out2,  NLON,NLAT,1, GDT_Int16, NULL);
                }
                pMensuelRaster->SetGeoTransform( transform );
                OGRSpatialReference src;
                src.importFromEPSG(4326);
                char *wgs;
                src.exportToProj4(&wgs);
                pMensuelRaster->SetProjection(wgs);
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

                            if (v!=-9999){
                                v= v*scaleF;
                                // std::cout << " valeur de " << v << std::endl;
                                //std::cout << " valeur pour " << date << " est de " << v << std::endl;
                                if (DG){if (v>baseDJ){v=v-baseDJ;} else {v=0;}}
                                vVar.push_back(v);
                            }
                        }

                        if (varAccro=="tn" | varAccro=="tg" | varAccro=="pp" ){

                        } else if (varAccro=="tx") {
                           type=resumeMensuel::max;
                        }else {
                           type=resumeMensuel::sum;
                        }

                        float sum_of_elems;
                        switch (type) {
                        case resumeMensuel::moy:{
                             sum_of_elems = std::accumulate(vVar.begin(), vVar.end(), 0);
                             sum_of_elems=sum_of_elems/ndayInM;
                            break;
                        }
                        case resumeMensuel::sum:{
                             sum_of_elems = std::accumulate(vVar.begin(), vVar.end(), 0);
                            break;
                        }
                        case resumeMensuel::min:{
                            //std::cout << " get minimum of the variable " << varAccro << std::endl;
                            std::vector<float>::iterator result = std::min_element(vVar.begin(),vVar.end());
                            //std::cout << "min element at: " << std::distance(v.begin(), result);
                            sum_of_elems=std::distance(vVar.begin(), result);
                            break;
                        }
                        case resumeMensuel::max:{
                            if (vVar.size()>0){
                            int index =std::max_element(vVar.begin(),vVar.end()) - vVar.begin();
                            sum_of_elems =vVar.at(index);}else {
                                sum_of_elems =0;
                            }
                            break;
                        }


                        default:
                            break;
                        }


                        //if (1){
                            /*sum_of_elems = std::accumulate(vVar.begin(), vVar.end(), 0);
                            //std::cout << " valeur finale de " << sum_of_elems << std::endl;
                            // moyenne de temp et de pression
                            if (!DG){
                                if (varAccro=="tn" |varAccro=="tx" | varAccro=="tg" | varAccro=="pp" ){
                                    sum_of_elems=sum_of_elems/ndayInM;
                                }}
                            // test de calculer la variation de pression plutôt que la moyenne.
                        } else {
                            std::vector<float> vVar2;
                            for (int c(1);c<vVar.size();c++){
                                float diff= vVar.at(c-1) - vVar.at(c);
                                vVar2.push_back(std::abs(diff));
                                sum_of_elems = std::accumulate(vVar2.begin(), vVar2.end(), 0);
                            }
                        }*/
                        // somme pour précipitation et radiation

                        varbuf=&sum_of_elems;
                        pMensuelRaster->GetRasterBand(1)->RasterIO(GF_Write, x,y, 1,1, varbuf,1, 1,GDT_Float32, 0, 0 );
                    }
                }

                GDALClose(pRaster);
                GDALClose(pMensuelRaster);
                projectRasterCallgdal(output2,output3);

            }
        }

    }
}


int ecadData::timeFromYMD(year_month_day ymd, NcVar *timeVar, int nrec){
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

std::vector<int> ecadData::timesForOneMonth(year_month ym, NcVar *timeVar, int nrec){
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
std::vector<int> ecadData::timesForDateForDJ(year_month_day ymd, NcVar *timeVar, int nrec){
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

void ecadData::projectRasterCallgdal(std::string inPath, std::string outPath){

    std::string aCommand= std::string("gdalwarp -s_srs EPSG:4326 -t_srs EPSG:31370 -te 32250.0 11170.0 305170.0 177700.0 -tr 5000 5000 -r average -overwrite ")
            + inPath +" "+outPath+" ";
    std::cout << aCommand << "\n";
    system(aCommand.c_str());
}

// fonctionne pas encore car pas de facteur multiplicatif lors de l'export !!

void ecadData::exportCarteUneDate(year_month_day ymd, std::string varAccro){
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

/*
void ecadData::calculDJPourDate(){
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
*/

void ecadData::exportCarteDJUneDate(year_month_day ymd){
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


