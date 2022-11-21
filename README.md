une application pour le traitement de données climatiques.

Concu pour les besoins de la thèse de Arthur Gilles en 2021, l'outil était destiné dans un premier temps à la lecture des données de l'ECAD.
Par la suite, l'IRM nous ayant transmis les données que nous attendions, l'outil est développé pour la lecture et le traitement des données reçue par l'IRM qui sont dans un format custom, bien entendu.

l'outil à utiliser est donc l'outil numéro 4, car les 3 premiers concernent le traitements des données ECAD et sont donc moins intéressant.

Exemple d'utilisation;

./climatApp --outils 4 --inputIRMFile /home/lisein/Documents/Scolyte/Data/climat/IRM/pdg1147-moyTrentenaire.csv --inputIRMRT ../pixel_id.tif --outDir=toto/

Installation ; netcdfcpp
wget ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-cxx-4.2.tar.gz
tar -xzf netcdf-cxx-4.2.tar.gz
cd netcdf-cxx-4.2
./configure 
make 
sudo make install

ou depuis package : sudo apt install libnetcdf-c++4 libnetcdf-cxx-legacy-dev

cloner dépot date aussi.

git submodule init
git submodule update






