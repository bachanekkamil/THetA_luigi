vars.tmp <- commandArgs()
vars <- vars.tmp[length(vars.tmp)]
split.vars <- unlist(strsplit(vars,","))


###  Setting input paths for normalized read count and experimental design ###
DataFolder <- split.vars[1]
TargetFolder <- split.vars[2]
ExperimentalFile <- split.vars[3]
ExperimentalDesign <- split.vars[4]
TargetName <- split.vars[5]
ProgramFolder <- split.vars[6]
Assembly <- split.vars[7]


### Load and set experimental design ###
ExperimentalTable <- read.table(ExperimentalFile,sep=",",quote="",header=F)
ExpName <- as.character(ExperimentalTable[,1])
LabelName <- as.character(ExperimentalTable[,2])


### Create the vector for the experimental design ###
if (ExperimentalDesign=="pooling")
{
  indC <- grep("C",LabelName)
  indT <- grep("T",LabelName)
  ExpTest <- c(ExpName[indT])
  ExpControl <- rep("Control",length(indT))
}

if (ExperimentalDesign=="somatic")
{
  indC <- grep("C",LabelName)
  indT <- grep("T",LabelName)
  LabelNameC <- LabelName[indC]
  LabelNameT <- LabelName[indT]
  ExpNameC <- ExpName[indC]
  ExpNameT <- ExpName[indT]
  NumC <- as.numeric(substr(LabelNameC, 2, 100000))
  NumT <- as.numeric(substr(LabelNameT, 2, 100000))
  indCS <- sort(NumC,index.return=T)$ix
  indTS <- sort(NumT,index.return=T)$ix
  ExpTest <- ExpNameT[indTS]
  ExpControl <- ExpNameC[indCS]
}


### Loading target chromosomes ###
TargetChrom <- file.path(TargetFolder,paste(TargetName,"_chromosome.txt",sep=""))
CHR <- scan(TargetChrom,what="character")
unique.chrom <- paste(CHR,".",sep="")


source(file.path(ProgramFolder,"/lib/R/FastCallSeq.R"))


### Loading centromere file ### 
CentromereTable <- read.table(file.path(ProgramFolder,paste("data/centromere/CentromerePosition_",Assembly,".txt",sep="")),sep="\t",quote="",header=T)
CentroChr <- as.character(CentromereTable[,1])
if (nchar(CHR[1])<4)
{
  CentroChr<-substr(CentroChr, 4, 100000)
}
CentroStart <- as.numeric(CentromereTable[,2])
CentroEnd <- as.numeric(CentromereTable[,3])



for (zz in 1:length(ExpTest))
{
  
  ### Loading normalized read count for reference sample ####
  FileRef <- file.path(DataFolder,"Data",ExpControl[zz],"RCNorm",paste(ExpControl[zz],".NRC.RData",sep=""))
  
  load(file=FileRef)
  
  RefMatrixNorm <- MatrixNorm
  
  Position <- as.integer(RefMatrixNorm[,2])
  chrom <- as.character(RefMatrixNorm[,1])
  DataSeqRef <- as.numeric(RefMatrixNorm[,6])
  start <- as.numeric(RefMatrixNorm[,3])
  end <- as.numeric(RefMatrixNorm[,4])
  Gene <- as.character(RefMatrixNorm[,5])
  
  
  ### Loading normalized read count for test sample ###
  FileTest <- file.path(DataFolder,"Data",ExpTest[zz],"RCNorm",paste(ExpTest[zz],".NRC.RData",sep=""))
  
  load(file=FileTest)
  
  TestMatrixNorm <- MatrixNorm
  DataSeqTest <- as.numeric(TestMatrixNorm[,6])
  
  
  ### Calculating Log2-ratio and lowess normalization ###
  
  A <- 0.5*log2(DataSeqTest*DataSeqRef)
  M <- log2(DataSeqTest/DataSeqRef)
  smoothnum <- lowess(A, M)
  LogDataNorm <- M - approx(smoothnum, xout = A)$y
  
  
  dyn.load(file.path(ProgramFolder,"/lib/F77/FastJointSLMLibraryI.so"))
  source(file.path(ProgramFolder,"/lib/R/LibraryJSLMIn.R"))
  
  ### Setting starting parameters of HSLM ### 
  ParameterVec <- as.numeric(unlist(read.table(file.path(ProgramFolder,"/ParameterFile.txt"),sep="\t",quote="",header=F,comment.char="#")))
  omega <- ParameterVec[1]
  eta <- ParameterVec[2]
  stepeta <- ParameterVec[3]
  cell <- ParameterVec[4]
  thrd <- ParameterVec[5]
  thru <- ParameterVec[6]
  FW <- ParameterVec[7]
  
  
  ### Calculating parameters of the HSLM ###
  mw <- 1
  ParamList <- ParamEstSeq(rbind(LogDataNorm),omega)
  mi <- ParamList$mi
  smu <- ParamList$smu
  sepsilon <- ParamList$sepsilon
  muk <- MukEst(rbind(LogDataNorm),mw)
  
  
  ###  Segmentation of the log2-ratio profiles with HSLM ###
  MatrixSeg <- matrix(NA,nrow=length(LogDataNorm),ncol=7)
  indCountS<-1  
  
  for (i in 1:length(CHR))
  {
    chr <- CHR[i]
    
    indchr <- which(chrom==chr)
    
    seqChrom <- LogDataNorm[indchr]
    PosChrom <- Position[indchr]
    startChrom <- start[indchr]
    endChrom <- end[indchr]
    GeneChrom <- Gene[indchr]
    
    indCountE<-indCountS+length(indchr)-1
    
    
    smooth.region=10
    outlier.SD.scale=4
    smooth.SD.scale=2
    trim=0.025
    
    seqChrom <- rbind(filterOut(seqChrom, smooth.region,outlier.SD.scale,smooth.SD.scale,trim))
    
    splitchrom1 <- CentroStart[which(CentroChr==chr)]
    splitchrom2 <- CentroEnd[which(CentroChr==chr)]
    
    splitind1 <- tail(which(PosChrom<splitchrom1),1)
    splitind2 <- which(PosChrom>splitchrom2)[1]
    ChrSeg<-c()
    if (length(splitind1)!=0)
    {
      ind1 <- c(1:splitind1)
      ind2 <- c(splitind2:length(seqChrom))
      DataSeq1 <- rbind(seqChrom[ind1])
      DataSeq2 <- rbind(seqChrom[ind2])
      Pos1 <- PosChrom[1:splitind1]
      Pos2 <- PosChrom[splitind2:length(seqChrom)]
      start1 <- startChrom[1:splitind1]
      end1 <- endChrom[1:splitind1]
      start2 <- startChrom[splitind2:length(seqChrom)]
      end2 <- endChrom[splitind2:length(seqChrom)]
      Gene1 <- GeneChrom[ind1]
      Gene2 <- GeneChrom[ind2]
      TotalPredBreak1 <- JointSegIn(DataSeq1,muk,mi,smu,sepsilon,Pos1,omega,eta,mw,stepeta)
      if (length(which(is.na(TotalPredBreak1)))==0)
      {
        TotalPredBreak1 <- FilterSeg(TotalPredBreak1,FW)
        DataSeg1 <- SegResults(DataSeq1,TotalPredBreak1,start1,end1,Gene1)
        ChrSeg<-rbind(ChrSeg,cbind(rep(chr,length(ind1)),Pos1,Gene1,start1,end1,t(DataSeq1),t(DataSeg1)))
      }
      if (length(which(is.na(TotalPredBreak1)))!=0)
      {
        print(paste("The HSLM analysis of short arm of chromosome",chr,"was aborted because the total number of EMRC is too small:",length(ind1)))
        ChrSeg<-rbind(ChrSeg,cbind(rep(chr,length(ind1)),Pos1,Gene1,start1,end1,matrix(NA,nrow=length(ind1),ncol=2)))
      }
      TotalPredBreak2 <- JointSegIn(DataSeq2,muk,mi,smu,sepsilon,Pos2,omega,eta,mw,stepeta)
      if (length(which(is.na(TotalPredBreak2)))==0)
      {
        TotalPredBreak2 <- FilterSeg(TotalPredBreak2,FW)
        
        DataSeg2 <- SegResults(DataSeq2,TotalPredBreak2,start2,end2,Gene2)
        ChrSeg<-rbind(ChrSeg,cbind(rep(chr,length(ind2)),Pos2,Gene2,start2,end2,t(DataSeq2),t(DataSeg2)))
      }
      if (length(which(is.na(TotalPredBreak2)))!=0)
      {
        print(paste("The HSLM analysis of long arm of chromosome",chr,"was aborted because the total number of EMRC is too small:",length(ind2)))
        ChrSeg<-rbind(ChrSeg,cbind(rep(chr,length(ind2)),Pos2,Gene2,start2,end2,matrix(NA,nrow=length(ind2),ncol=2)))
      }
      
    }
    
    if (length(splitind1)==0)
    {
      TotalPredBreak <- JointSegIn(seqChrom,muk,mi,smu,sepsilon,PosChrom,omega,eta,mw,stepeta)
      if (length(which(is.na(TotalPredBreak)))==0)
      {
        TotalPredBreak <- FilterSeg(TotalPredBreak,FW)
        DataSeg <- SegResults(seqChrom,TotalPredBreak,start,end,gene)
        ChrSeg <- cbind(rep(chr,length(indchr)),PosChrom,cbind(GeneChrom),startChrom,endChrom,t(seqChrom),t(DataSeg))
      }
      if (length(which(is.na(TotalPredBreak)))!=0)
      {
        print(paste("The HSLM analysis of chromosome",chr,"was aborted because the total number of EMRC is too small:",length(indchr)))
        ChrSeg <- rbind(ChrSeg,cbind(rep(chr,length(indchr)),PosChrom,cbind(GeneChrom),startChrom,endChrom,matrix(NA,nrow=length(indchr),ncol=2)))
      }
      
    }
    MatrixSeg[c(indCountS:indCountE),] <- ChrSeg
    indCountS<-indCountE+1
  }
  
  MatrixSeg1 <- MatrixSeg[,c(1,2,4,5,6,7)]
  colnames(MatrixSeg1) <- c("Chromosome","Position","Start","End","Log2R","SegMean")
  FileOutSeg <- file.path(DataFolder,"Results",ExpTest[zz],paste("HSLMResults_",ExpTest[zz],".txt",sep=""))
  write.table(MatrixSeg1,FileOutSeg,col.names=T,row.names=F,sep="\t",quote=F)

  #### Filtering Not-Segmented Data ##
  DataFilt<-as.character(MatrixSeg[,6])
  indFilt<-which(is.na(DataFilt))
  if (length(indFilt)!=0)
  {
    MatrixSeg<-MatrixSeg[-indFilt,]
  }
  
  MatrixSegFC<-MatrixSeg[,-2]
  
  ###  FastCall analysis ###
  AnalisiList <- MakeData(MatrixSegFC,infoPos.StartEnd=TRUE)
  
  MetaData <- AnalisiList$MetaTable
  SummaryData <- AnalisiList$SummaryData
  
  mdata <- SummaryData[,4]
  
  if (cell<1)
  {
    datac <- (2^(mdata)/(cell)-(1-cell)/cell)
    thrc <- 2^(-5)
    datac[which(datac<thrc)] <- thrc
    mdata <- log2(datac)
  }
  
  ResultsEM <- EMFastCall(mdata,thru,thrd)
  muvec <- ResultsEM$muvec
  sdvec <- ResultsEM$sdvec
  prior <- ResultsEM$prior
  bound <- ResultsEM$bound
  
  P0 <- PosteriorP(mdata,muvec,sdvec,prior)
  
  out <- LabelAss(P0,mdata)
  
  ### Filtering Significant Segments ###
  indSig<-which(out[,1]!=0)
  
  if (length(indSig)!=0)
  {
    outSig<-out[indSig,]
    P0Sig<-P0[indSig,]
    SummaryDataSig<-SummaryData[indSig,]
    CNFSig<-2*(2^SummaryDataSig[,4])
    CNSig<-round(CNFSig)
    
    
    #### FastCall Results in BED format #####
    OutBedSig<-cbind(MetaData[SummaryDataSig[,2],1],MetaData[SummaryDataSig[,2],3],MetaData[SummaryDataSig[,3],4],SummaryDataSig[,4],CNFSig,CNSig,outSig)
  }
  if (length(indSig)==0)
  {
    OutBedSig<-c()
  }
  HeaderBed<-c("Chromosome","Start","End","Segment","CNF","CN","Call","ProbCall")
  OutBedSig<-rbind(HeaderBed,OutBedSig)
  
  FileOutCall <- file.path(DataFolder,"Results",ExpTest[zz],paste("FastCallResults_",ExpTest[zz],".txt",sep=""))
  write.table(OutBedSig,FileOutCall,col.names=F,row.names=F,sep="\t",quote=F)
  
}

