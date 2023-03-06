
if false; then
/home/gef/app/climat/build-climatApp//climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp245/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2
/home/gef/app/climat/build-climatApp//climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp585/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2
/home/gef/app/climat/build-climatApp//climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp245/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2
/home/gef/app/climat/build-climatApp//climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp585/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2
/home/gef/app/climat/build-climatApp//climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp245/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2
/home/gef/app/climat/build-climatApp//climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp585/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2
fi;

# pour les réanalyse : miroc 6 et noresm qui ont le climat passé
if true; then
/home/gef/app/climat/build-climatApp//climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp585/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2
/home/gef/app/climat/build-climatApp//climatApp --outil 7 --input "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp585/" --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --mode 2


mkdir /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/
mkdir /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/MPI-ESM1-2-HR-ssp245/
mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp245/MAR-MultiY/table* /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/MPI-ESM1-2-HR-ssp245/

mkdir /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/MPI-ESM1-2-HR-ssp585/
mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp585/MAR-MultiY/table* /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/MPI-ESM1-2-HR-ssp585/

mkdir /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/NorESM2-MM-ssp245/
mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp245/MAR-MultiY/table* /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/NorESM2-MM-ssp245/

mkdir /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/NorESM2-MM-ssp585/
mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp585/MAR-MultiY/table* /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/NorESM2-MM-ssp585/

mkdir /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/MIROC6-ssp245/
mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp245/MAR-MultiY/table* /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/MIROC6-ssp245/

mkdir /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/MIROC6-ssp585/
mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp585/MAR-MultiY/table* /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/MIROC6-ssp585/

fi;
