# 2023 01 - j'ai plusieurs tableau synthétiques du climat (passé et projection futur) et je voudrai les fusionner pour une zone bioclimatique donnée

library(tidyverse)


path <- "/home/jo/Documents/climat_MAR/all"

path <- "/home/jo/Documents/climat_MAR/all-20230206"

setwd(path)
dirs <- list.dirs(path = path, recursive = FALSE)

df <- data.frame(date=as.integer(),model=as.character(),MBRR=as.double(),m4_9MBRR=as.double())

ds <-1
for (dir in dirs) {
  
  dirName <- str_split(dir, '/')[[1]]
  dirName <- dirName[length(dirName)]

  for (f in list.files(dir)){
    
    timeName <- substr(f,6,14)
    timeCentrale <- as.integer(substr(timeName,1,4))+ (as.integer(substr(timeName,6,9))-as.integer(substr(timeName,1,4)))/2
    #lecture du dataframe
    d <- read.table(paste0(dir,"/",f),sep=";", header=T) 
    
    cond <- d$ZBIO %in% c("Oesling", "Gutland")
    cond <- d$ZBIO %in% c("Ardenne")
    
    df <- df %>% add_row(date = timeCentrale, model = dirName, MBRR=mean(d[cond,"MBRR"]), m4_9MBRR=mean(d[cond,"m4_9MBRR"]))
    colnames(d) <- paste0(dirName,"-",timeName,"-",colnames(d))
    
    # je garde que les précipitations pour commencer
    d <- cbind (d[,1], d[,grepl( "MBRR",colnames(d))])
    
    if (ds==1){ds=d}else{ ds<- cbind(ds,d[,2:ncol(d)])}
  }
  
}

write.table(ds,"synthese.csv",row.names = F)


# Libraries
library(ggplot2)
library(babynames) # provide the dataset: a dataframe called babynames
library(dplyr)



# Plot
don %>%
  ggplot( aes(x=year, y=n, group=name, color=name)) +
  geom_line()

# ajout de deux lignes, correspondant à la moyenne des modèles globaux pour un scénario donné

df <- df %>% add_row(date = 2060, model = "ssp245", MBRR=mean(df$MBRR[grepl("ssp245",df$model)]), m4_9MBRR=mean(df$m4_9MBRR[grepl("ssp245",df$model)]))

df %>%
  ggplot( aes(x=date, y=MBRR, group=model, color=model)) +
  geom_line() + geom_point()

df %>%
  ggplot( aes(x=date, y=m4_9MBRR, group=model, color=model)) +
  geom_line() + geom_point()

#-----------------------------------------------
# je recommence mais avec toutes les variables
df <- read.table(paste0(dirs[1],"/",list.files(dirs[1])[1]), sep=";", header=T)
df <- df[0,]

first <- TRUE
for (dir in dirs) {
  
  dirName <- str_split(dir, '/')[[1]]
  dirName <- dirName[length(dirName)]
  
  for (f in list.files(dir)){
    
    model <- substr(dirName,1,nchar(dirName)-6)
    ssp <- substr(dirName,nchar(dirName)-5,nchar(dirName))
    
    timeName <- substr(f,6,14)
    timeCentrale <- as.integer(substr(timeName,1,4))+ (as.integer(substr(timeName,6,9))-as.integer(substr(timeName,1,4)))/2
    #lecture du dataframe
    d <- read.table(paste0(dir,"/",f),sep=";", header=T) 
    d <- d[d$ZBIO=="Ardenne",]
    if (first){
    df <- rbind(df,d) 
    df$periode[nrow(df)] <- timeName
    df$timeCentral[nrow(df)] <- timeCentrale
    df$ssp[nrow(df)] <- ssp
    df$model[nrow(df)] <- model
    first<- FALSE
    } else {
      d$periode[nrow(d)] <- timeName
      d$timeCentral[nrow(d)] <- timeCentrale
      d$ssp[nrow(d)] <- ssp
      d$model[nrow(d)] <- model
    
    df <- rbind(df,d)
    }
  }
}

write.table(df,"syntheseMAR3.13-ArdenneAllModels.csv",row.names = F)
#moyenne maintenant
summary <- df %>% group_by(ssp,periode) %>% summarise(nombreModels=n(),MBRR=mean(MBRR),TTG=mean(TTG),TTX=max(TTX),TTN=min(TTN),m4_9MBRR=mean(m4_9MBRR),BHE=mean(BHE),BHE2=mean(BHE2),GSL=mean(GSL.6.8.),SD30=mean(SD30),SD40=mean(SD40),SDG20=mean(SDG20), SDG25=mean(SDG25),FD=mean(FD),FDG=mean(FDG), HPD=mean(HPD))

write.table(summary,"syntheseMAR3.13-Ardenne.csv",row.names = F)

