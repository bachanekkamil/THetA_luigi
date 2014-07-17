vars.tmp <- commandArgs()
vars <- vars.tmp[length(vars.tmp)]
split.vars <- unlist(strsplit(vars,","))


### Setting input paths for RC, mappability, GC-content and target ###
DataFolder <- split.vars[1]
TargetFolder <- split.vars[2]
ExperimentalFile <- split.vars[3]
ExperimentalDesign <- split.vars[4]
TargetName <- split.vars[5]
ProgramFolder <- split.vars[6]


### Load and set experimental design ###
ExperimentalTable <- read.table(ExperimentalFile,sep=",",quote="",header=F)
ExpName <- as.character(ExperimentalTable[,1])
LabelName <- as.character(ExperimentalTable[,2])


### Setting paths for RC, mappability, GC-content e target ###
PathGC <- file.path(TargetFolder,"GCC")
PathMAP <- file.path(TargetFolder,"MAP")
GCFiles <- list.files(PathGC)
MAPFiles <- list.files(PathMAP)


### Loading target chromosomes ###
TargetChrom <- file.path(TargetFolder,paste(TargetName,"_chromosome.txt",sep=""))
CHR<-readLines(con = TargetChrom, n = -1L, ok = TRUE, warn = TRUE,encoding = "unknown")
unique.chrom<-strsplit(CHR," ")[[1]]



Path2ExomeRC <- file.path(ProgramFolder,"lib","R","LibraryExomeRC.R")
source(Path2ExomeRC)


### Create the RC matrix with all the Experiments ### 
RCTMatrix <- c()

for (i in 1:length(ExpName))
{
PathRC <- file.path(DataFolder,"Data",ExpName[i],"RC")
RCFiles <- list.files(PathRC)
RCTMatrix <- rbind(RCTMatrix,loadRC(PathRC,RCFiles,unique.chrom))
}


### Create the RC matrix pooling the control samples ###
if (ExperimentalDesign=="pooling")
{
indTest <- grep("T",LabelName)
indControl <- grep("C",LabelName)

RCTControl <- colSums(rbind(RCTMatrix[indControl,]))
RCTMatrix <- rbind(RCTMatrix[indTest,],RCTControl)
ExpName <- c(ExpName[indTest],"Control")

### Create the Directory for pooled Control ###
dir.create(file.path(DataFolder,"Data","Control"))
dir.create(file.path(DataFolder,"Data","Control","RC"))
dir.create(file.path(DataFolder,"Data","Control","RCNorm"))
dir.create(file.path(DataFolder,"Data","Control","Images"))
}


### Loading target, mappability and GC content Files ###
TargetOut <- loadTarget(TargetFolder,unique.chrom,TargetName)
MyTarget <- TargetOut[[1]]
GCContentT <- TargetOut[[2]]
MAPT <- TargetOut[[3]]


start <- as.integer(MyTarget[,2])
end <- as.integer(MyTarget[,3])
chrom <- as.character(MyTarget[,1])
GeneName <- as.character(MyTarget[,4])
L <- end-start
Position <- (start+end)/2


### Filtering RC as a function of enrichment efficiency threshold ###
#RCTMatrix <- t(t(RCTMatrix)/L)
MatrixControl <- matrix(0,nrow=nrow(RCTMatrix),ncol=ncol(RCTMatrix))
thrF <- 20
for (z in 1:nrow(RCTMatrix))
{
#thrF <- 0.01*median(RCTMatrix[z,])
MatrixControl[z,which(RCTMatrix[z,]>thrF)] <- 1
}

sumM <- colSums(MatrixControl)
indF <- which(sumM!=0)

RCTMatrix <- RCTMatrix[,indF]
start <- start[indF]
end <- end[indF]
chrom <- chrom[indF]
L <- L[indF]
GCContentT <- GCContentT[indF]
MAPT <- MAPT[indF]
Position <- Position[indF]
GeneName <- GeneName[indF]


### Small Exons Filtering ###
indF <- which(L>50)
RCTMatrix <- RCTMatrix[,indF]
start <- start[indF]
end <- end[indF]
chrom <- chrom[indF]
L <- L[indF]
GCContentT <- GCContentT[indF]
MAPT <- MAPT[indF]
Position <- Position[indF]
GeneName <- GeneName[indF]

#RCTMatrixL <- RCTMatrix
RCTMatrixL <- t(t(RCTMatrix)/L)
GCContentL <- (GCContentT/L)*100


### Normalization for GC Content, mappability and exon size ###
library(Hmisc)
for (i in 1:length(ExpName))
{
RCTL <- c(RCTMatrixL[i,])

PathExp <- file.path(DataFolder,"Data",ExpName[i])
PathNorm <- file.path(PathExp,"RCNorm")
PathImages <- file.path(PathExp,"Images")
FileOut <- file.path(PathNorm,paste(ExpName[i],".NRC.RData",sep=""))


### Exon size normalization ###
step <- 5
RCLNormList <- CorrectSize(RCTL,L,step)
RCLNorm <- RCLNormList$RCNorm


### Mappability normalization ###
step <- 5
RCMAPNormList <- CorrectMAP(RCLNorm,MAPT,step)
RCMAPNorm <- RCMAPNormList$RCNorm


### GC-content Normalization ###
step <- 5
RCGCNormList <- CorrectGC(RCMAPNorm,GCContentL,step)
RCGCNorm <- RCGCNormList$RCNorm


### Images generation ###
### Quantiles for exon size ###
step <- 5
QuantileLList <- QuantileLength(RCTL,L,step)
MedianVecL <- QuantileLList$Median
QuantileVecL <- QuantileLList$Quantile

SizeBias <- file.path(PathImages,"SizeBias.pdf")
pdf(SizeBias,height=10,width=15)
yup <- median(RCTL,na.rm=T)*3
#par(mfrow=c(3,5),oma=c(2,0,2,0))
stepseq <- seq(0,max(L),by=step)
xstep <- c(1:(length(stepseq)-1))
labelx <- "Exon Length"
labely <- "Read Count"
errbar(xstep,MedianVecL,QuantileVecL[,2],QuantileVecL[,1],axes=F,frame.plot=TRUE,xlab=labelx,ylab=labely,add=F,xlim=c(0,160),ylim=c(0,yup),lty=3,cex.axis=1.2,cex.lab=1.3)
#title("Mappability Normalization",cex.main=2.7,outer=T)
title("Exon Length Bias",cex.main=2)
text((length(stepseq)-3),2000,"a",cex=3)
ax.x <- stepseq[2:length(stepseq)]
axis(1,xstep,labels=formatC(ax.x, format="fg"))
axis(2)
dev.off()

### Quantiles for mappability ###
step <- 5
QuantileMapList <- QuantileMAP(RCTL,MAPT,step)
MedianVecMAP <- QuantileMapList$Median
QuantileVecMAP <- QuantileMapList$Quantile

MAPBias <- file.path(PathImages,"MAPBias.pdf")
pdf(MAPBias,height=10,width=15)
yup <- median(RCTL,na.rm=T)*3
#par(mfrow=c(3,5),oma=c(2,0,2,0))
stepseq <- seq(0,100,by=step)
xstep <- c(1:(length(stepseq)-1))
labelx <- "Mappability"
labely <- "Read Count"
errbar(xstep,MedianVecMAP,QuantileVecMAP[,2],QuantileVecMAP[,1],axes=F,frame.plot=TRUE,xlab=labelx,ylab=labely,add=F,ylim=c(0,yup),lty=3,cex.axis=1.2,cex.lab=1.3)
#title("Mappability Normalization",cex.main=2.7,outer=T)
title("Mappability Bias",cex.main=2)
text((length(stepseq)-3),2000,"a",cex=3)
ax.x <- stepseq[2:length(stepseq)]
axis(1,xstep,labels=formatC(ax.x, format="fg"))
axis(2)
dev.off()

#### Quantiles for GC-content ###
step <- 5
QuantileGCList <- QuantileGC(RCTL,GCContentL,step)
MedianVecGC <- QuantileGCList$Median
QuantileVecGC <- QuantileGCList$Quantile

GCBias <- file.path(PathImages,"GCBias.pdf")
pdf(GCBias,height=10,width=15)
yup <- median(RCTL,na.rm=T)*2
#par(mfrow=c(3,5),oma=c(2,0,2,0))
stepseq <- seq(0,100,by=step)
xstep <- c(1:(length(stepseq)-1))
labelx <- "GC-Content"
labely <- "Read Count"
errbar(xstep,MedianVecGC,QuantileVecGC[,2],QuantileVecGC[,1],axes=F,frame.plot=TRUE,xlab=labelx,ylab=labely,add=F,ylim=c(0,yup),lty=3,cex.axis=1.2,cex.lab=1.3)
#title("Mappability Normalization",cex.main=2.7,outer=T)
title("GC-Content Bias",cex.main=2)
text((length(stepseq)-3),2000,"a",cex=3)
ax.x <- stepseq[2:length(stepseq)]
axis(1,xstep,labels=formatC(ax.x, format="fg"))
axis(2)
dev.off()

### Normalized data ###
### Quantile of normalized exon length ###
step <- 5
QuantileLListN <- QuantileLength(RCLNorm,L,step)
MedianVecLN <- QuantileLListN$Median
QuantileVecLN <- QuantileLListN$Quantile

SizeBiasNorm <- file.path(PathImages,"SizeBiasNorm.pdf")
pdf(SizeBiasNorm,height=10,width=15)
yup <- median(RCLNorm,na.rm=T)*2
#par(mfrow=c(3,5),oma=c(2,0,2,0))
stepseq <- seq(0,max(L),by=step)
xstep <- c(1:(length(stepseq)-1))
labelx <- "Exon Length"
labely <- "Read Count"
errbar(xstep,MedianVecLN,QuantileVecLN[,2],QuantileVecLN[,1],axes=F,frame.plot = TRUE,xlab=labelx,ylab=labely,add=F,xlim=c(0,160),ylim=c(0,yup),lty=3,cex.axis=1.2,cex.lab=1.3)
#title("Mappability Normalization",cex.main=2.7,outer=T)
title("Normalized Read Count Exon Length Bias",cex.main=2)
text((length(stepseq)-3),2000,"a",cex=3)
ax.x <- stepseq[2:length(stepseq)]
axis(1,xstep,labels=formatC(ax.x, format="fg"))
axis(2)
dev.off()


### Quantile of normalized mapping ###
step <- 5
QuantileMapListN <- QuantileMAP(RCMAPNorm,MAPT,step)
MedianVecMAPN <- QuantileMapListN$Median
QuantileVecMAPN <- QuantileMapListN$Quantile
MAPBiasNorm <- file.path(PathImages,"MAPBiasNorm.pdf")
pdf(MAPBiasNorm,height=10,width=15)
yup <- median(RCMAPNorm,na.rm=T)*2
#par(mfrow=c(3,5),oma=c(2,0,2,0))
stepseq <- seq(0,100,by=step)
xstep <- c(1:(length(stepseq)-1))
labelx <- "Mappability"
labely <- "Read Count"
errbar(xstep,MedianVecMAPN,QuantileVecMAPN[,2],QuantileVecMAPN[,1],axes=F,frame.plot = TRUE,xlab=labelx,ylab=labely,add=F,ylim=c(0,yup),lty=3,cex.axis=1.2,cex.lab=1.3)
#title("Mappability Normalization",cex.main=2.7,outer=T)
title("Normalized Read Count Mappability Bias",cex.main=2)
text((length(stepseq)-3),2000,"a",cex=3)
ax.x <- stepseq[2:length(stepseq)]
axis(1,xstep,labels=formatC(ax.x, format="fg"))
axis(2)
dev.off()


#### Quantile of normalized GC-content ###
step <- 5
QuantileGCListN <- QuantileGC(RCGCNorm,GCContentL,step)
MedianVecGCN <- QuantileGCListN$Median
QuantileVecGCN <- QuantileGCListN$Quantile

GCBiasNorm <- file.path(PathImages,"GCBiasNorm.pdf")
pdf(GCBiasNorm,height=10,width=15)

yup <- median(RCGCNorm,na.rm=T)*2
#par(mfrow=c(3,5),oma=c(2,0,2,0))
stepseq <- seq(0,100,by=step)
xstep <- c(1:(length(stepseq)-1))
labelx <- "GC-Content"
labely <- "Read Count"
errbar(xstep,MedianVecGCN,QuantileVecGCN[,2],QuantileVecGCN[,1],axes=F,frame.plot = TRUE,xlab=labelx,ylab=labely,add=F,ylim=c(0,yup),lty=3,cex.axis=1.2,cex.lab=1.3)
#title("Mappability Normalization",cex.main=2.7,outer=T)
title("Normalizard Read Count GC-Content Bias",cex.main=2)
text((length(stepseq)-3),2000,"a",cex=3)
ax.x <- stepseq[2:length(stepseq)]
axis(1,xstep,labels=formatC(ax.x, format="fg"))
axis(2)
dev.off()

##### Saving normalized RC file ###
RCGCNorm[which(RCGCNorm==0)] <- min(RCGCNorm[which(RCGCNorm!=0)])
RCNorm <- RCGCNorm
MatrixNorm <- cbind(chrom,Position,start,end,GeneName,RCNorm)
save(MatrixNorm,file=FileOut)
}
