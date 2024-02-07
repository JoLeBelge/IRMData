
for ssp in "245" "585"
do

/home/gef/app/climat/build-climatApp//climatApp --outil 8 --input /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp${ssp}/ --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --input3 "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-histo/" --years 1 1

/home/gef/app/climat/build-climatApp//climatApp --outil 8 --input /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp${ssp}/ --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --input3 "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorEsm2-MM-histo/" --years 1 1

/home/gef/app/climat/build-climatApp//climatApp --outil 8 --input /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp${ssp}/ --input2 "/home/gef/app/climat/doc/grilleIRMGDL.nc" --input3 "/media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-histo/" --years 1 1

done


mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp245/MAR-MultiY/tableMoyMobileZBIOcor.csv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/moyMob/tableMoyMobileZBIO-MPI-ESM1-2-HR-ssp245.csv

mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp585/MAR-MultiY/tableMoyMobileZBIOcor.csv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/moyMob/tableMoyMobileZBIO-MPI-ESM1-2-HR-ssp585.csv

mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp245/MAR-MultiY/tableMoyMobileZBIOcor.csv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/moyMob/tableMoyMobileZBIO-NorESM2-MM-ssp245.csv

mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp585/MAR-MultiY/tableMoyMobileZBIOcor.csv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/moyMob/tableMoyMobileZBIO-NorESM2-MM-ssp585.csv

mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp245/MAR-MultiY/tableMoyMobileZBIOcor.csv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/moyMob/tableMoyMobileZBIO-MIROC6-ssp245.csv

mv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp585/MAR-MultiY/tableMoyMobileZBIOcor.csv /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/moyMob/tableMoyMobileZBIO-MIROC6-ssp585.csv
