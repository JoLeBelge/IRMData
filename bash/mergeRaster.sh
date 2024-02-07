for var in "BHE" "TG" "MBRRS"
do
for period in "2021-2050" "2051-2080" "2081-2100"
do
otbcli_BandMathX -il /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MPI-ESM1-2-HR-ssp585/MAR-MultiY/raster/MARv3.13-MPI-ESM1-2-HR-ssp585-"$period""$var"cor.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-NorESM2-MM-ssp585/MAR-MultiY/raster/MARv3.13-NorESM2-MM-ssp585-"$period""$var"cor.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-13-MIROC6-ssp585/MAR-MultiY/raster/MARv3.13-MIROC6-ssp585-"$period""$var"cor.tif -out /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/raster/ssp585-"$period""$var"cor.tif -exp '(im1b1+im2b1+im3b1)/3' -ram 4000

gdalwarp -overwrite -tr 100 100 -r cubicspline /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/raster/ssp585-"$period""$var"cor.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/raster/ssp585-"$period""$var"cor100mGSD.tif

gdalwarp -overwrite -of GTiff -cutline /home/gef/Documents/limitWal/limiteWal.shp -cl limiteWal -crop_to_cutline /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/raster/ssp585-"$period""$var"cor100mGSD.tif /media/gef/598c5e48-4601-4dbf-ae86-d15388a3dffa/MAR-all/raster/ssp585-"$period""$var"cor100mGSDClip.tif

done
done

