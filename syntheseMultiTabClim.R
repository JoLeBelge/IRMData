# 2023 01 - j'ai plusieurs tableau synthétiques du climat (passé et projection futur) et je voudrai les fusionner pour une zone bioclimatique donnée

library(tidyverse)


path <- "/home/jo/Documents/climat_MAR/all"

path <- "/home/jo/Documents/climat_MAR/all-20230215"
path <- "/home/jo/Documents/climat_MAR/all-20230306"

path <- "/home/jo/Documents/climat_MAR/all-20230327"

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
    
    model <- substr(dirName,1,nchar(dirName)-7)
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

write.table(df,"syntheseMAR3.13-ArdenneAllModelsCor.csv",row.names = F)
#moyenne maintenant
summary <- df %>% group_by(ssp,periode) %>% summarise(nombreModels=n(),MBRR=mean(MBRR),TTG=mean(TTG),TTX=max(TTX),TTN=min(TTN),m4_9MBRR=mean(m4_9MBRR),m4_9TTG=mean(m4_9TTG),BHE=mean(BHE),BHE2=mean(BHE2),GSL=mean(GSL.6.8.),SD30=mean(SD30),SD35=mean(SD35),SD40=mean(SD40),SDG20=mean(SDG20), SDG25=mean(SDG25),FD=mean(FD),FDG=mean(FDG), HPD=mean(HPD),SF=mean(SF))

summary <- df %>% group_by(ssp,periode) %>% summarise(nombreModels=n(),MBRR=mean(MBRR),TG=mean(TG),TX=max(TX),TN=min(TN),m4_9MBRR=mean(m4_9MBRR),m4_9TG=mean(m4_9TG),BHE=mean(BHE),BHE2=mean(BHE2),GSL=mean(GSL.6.8.),SD30=mean(SD30),SD35=mean(SD35),SD40=mean(SD40),SDG20=mean(SDG20), SDG25=mean(SDG25),FD=mean(FD),FDG=mean(FDG), HPD=mean(HPD),SF=mean(SF))


write.table(summary,"syntheseMAR3.13-ArdenneCor.csv",row.names = F)

#######################################################################################################

# synthèse des moyenne mobile de température annuelle par zbio
path <- "/home/jo/Documents/carteApt/Andyne_catalogues/climat/moyMob"
setwd(path)
test <- 1
for (f in list.files(getwd())){
  
    model <- substr(f,20,nchar(f)-4-7)
    ssp <-  substr(f,nchar(f)-4-5,nchar(f)-4)
    d <- read.table(f,sep=";", header=T) 
    d$model <- model
    d$ssp <- ssp
    if(test){dall <- d
    test <- 0}else{dall <- rbind(dall,d)}
}
summary <- dall[!dall$ZBIO %in% c("Ardenne","NordSM"),] %>% group_by(ssp,Decennie,ZBIO) %>% summarise(TTG=mean(TTG))

#write.table(summary,"evolTTG.csv",row.names = F)

d585 <- summary[summary$ssp=="ssp585",]

d585 %>%
  ggplot( aes(x=Decennie+5, y=TTG, group=ZBIO, color=ZBIO)) +
  geom_line() +   scale_fill_manual(values=c("#E69F00", "#56B4E9","#999999"))



plot(d585$Decennie[d585$ZBIO=="HA"]+5,d585$TTG[d585$ZBIO=="HA"],ylim=c(8,13),xlim=c(2020,2100),xlab="Années",ylab="Température moyenne annuelle [C°]", type="l",lwd=2,col="#5c8944")
lines(d585$Decennie[d585$ZBIO=="BMA"]+5,d585$TTG[d585$ZBIO=="BMA"], type = "l", lwd = 2,col="#cdf57a")
lines(d585$Decennie[d585$ZBIO=="HCO"]+5,d585$TTG[d585$ZBIO=="HCO"], type = "l", lwd = 2,col="#89cd66")
