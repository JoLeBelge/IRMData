for y in "2021-2050" "2051-2080" "2081-2100"
do

otbcli_BandMathX -il /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp585/MAR-MultiY/raster/MARv3.13-MPI-ESM1-2-HR-ssp585-${y}BHE.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp585/MAR-MultiY/raster/MARv3.13-NorESM2-MM-ssp585-${y}BHE.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp585/MAR-MultiY/raster/MARv3.13-MIROC6-ssp585-${y}BHE.tif -out /media/gef/DataLisein/r/ssp585-${y}BHE.tif -exp '(im1b1+im2b1+im3b1)/3' -ram 4000

gdalwarp -overwrite -tr 100 100 -r cubicspline /media/gef/DataLisein/r/ssp585-${y}BHE.tif /media/gef/DataLisein/r/ssp585-${y}BHE_GSD100.tif

gdalwarp -overwrite -of GTiff -cutline /media/gef/DataLisein/limiteWal.shp -cl limiteWal -crop_to_cutline /media/gef/DataLisein/r/ssp585-${y}BHE_GSD100.tif /media/gef/DataLisein/r/ssp585-${y}BHE_GSD100Clip.tif

otbcli_BandMathX -il /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp585/MAR-MultiY/raster/MARv3.13-MPI-ESM1-2-HR-ssp585-${y}TTG.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp585/MAR-MultiY/raster/MARv3.13-NorESM2-MM-ssp585-${y}TTG.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp585/MAR-MultiY/raster/MARv3.13-MIROC6-ssp585-${y}TTG.tif -out /media/gef/DataLisein/r/ssp585-${y}TTG.tif -exp '(im1b1+im2b1+im3b1)/3' -ram 4000

gdalwarp -overwrite -tr 100 100 -t_srs EPSG:31370 -r cubicspline /media/gef/DataLisein/r/ssp585-${y}TTG.tif /media/gef/DataLisein/r/ssp585-${y}TTG_GSD100.tif

gdalwarp -overwrite -of GTiff -cutline /media/gef/DataLisein/limiteWal.shp -cl limiteWal -crop_to_cutline /media/gef/DataLisein/r/ssp585-${y}TTG_GSD100.tif /media/gef/DataLisein/r/ssp585-${y}TTG_GSD100Clip.tif

otbcli_BandMathX -il /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp585/MAR-MultiY/raster/MARv3.13-MPI-ESM1-2-HR-ssp585-${y}MBRRS.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp585/MAR-MultiY/raster/MARv3.13-NorESM2-MM-ssp585-${y}MBRRS.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp585/MAR-MultiY/raster/MARv3.13-MIROC6-ssp585-${y}MBRRS.tif -out /media/gef/DataLisein/r/ssp585-${y}MBRRS.tif -exp '(im1b1+im2b1+im3b1)/3' -ram 4000

gdalwarp -overwrite -tr 100 100 -t_srs EPSG:31370 -r cubicspline /media/gef/DataLisein/r/ssp585-${y}MBRRS.tif /media/gef/DataLisein/r/ssp585-${y}MBRRS_GSD100.tif

gdalwarp -overwrite -of GTiff -cutline /media/gef/DataLisein/limiteWal.shp -cl limiteWal -crop_to_cutline /media/gef/DataLisein/r/ssp585-${y}MBRRS_GSD100.tif /media/gef/DataLisein/r/ssp585-${y}MBRRS_GSD100Clip.tif
done

