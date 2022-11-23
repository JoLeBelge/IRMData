#include "utils.h"

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



void setGeoTMAR(std::string aRasterIn){
    std::cout << "set geotransform et spatialRef : rotation pour raster grille MAR et définition du src\n"
                 "prend en input le résultat de :  gdal_translate -of GTiff NETCDF:'MAR.nc':var1 var1_MAR.tif\n" << std::endl;

    GDALAllRegister();

    if (boost::filesystem::exists(aRasterIn)){

    const char *inputPath=aRasterIn.c_str();
    GDALDataset *pInputRaster = (GDALDataset*) GDALOpen(inputPath, GA_Update);


    double transform[6], tr[6];

    // tr1 c'est la définition de la grille de MAR pour BE, en 5km - pas loin de la définition dans netcdf mais avec quelque variantes tout de même
    tr[0]=-220;
    tr[1]=5;
    tr[2]=0;
    //tr[3]=-120; # gdal inverse toute les colonnes à par rapport au repère de la grille dans netcdf
     tr[3]=-120+5*49.5;
     // coin un demi-pixel plus loin
     tr[0]=-220-2.5;
    tr[4]=0;
    tr[5]=-5;

    // appliquer une rotation de 30 degré, je fais ça via la geotransform
    /* [0] x-coordinate of upper-left raster corner
    [1] cos(θ) * x_resolution
    [2] -sin(θ) * x_resolution
    [3] y-coordinate of upper-left raster corner
    [4] sin(θ) * y_resolution
    [5] cos(θ) * y_resolution*/

    double rot(30);
    rot=rot*M_PI/180;
    transform[1] = cos(rot)*tr[1];
    transform[2] = -sin(rot)*tr[1];
    transform[4] = sin(rot)*tr[5];
    transform[5] = cos(rot)*tr[5];
    //transform[0] = cos(-rot)*tr[0]+sin(-rot)*tr[3];
    transform[0] = cos(-rot)*tr[0]-sin(-rot)*tr[3];
    //transform[3] = -sin(-rot)*tr[0]+cos(-rot)*tr[3];
    transform[3] = -sin(rot)*tr[0]+cos(-rot)*tr[3];

    pInputRaster->SetGeoTransform( transform );

    // définition du src- un Stereographic Oblique Projection dont l'origine est LONLAT(5,50.5) positionné sur le pixel UV(45,25)
    std::string proj4("+proj=stere +lat_0=50.5 +lon_0=5 +k=1 +x0=0 +y0=0 +a=6371229 +b=6371229 +units=km +no_defs");
    OGRSpatialReference oSRS;
    oSRS.importFromProj4(proj4.c_str());
    pInputRaster->SetSpatialRef(&oSRS);
    GDALClose(pInputRaster);
    } else {

        std::cout << " setGeoTMAR : input n'existe pas " << aRasterIn << std::endl;
    }
}
