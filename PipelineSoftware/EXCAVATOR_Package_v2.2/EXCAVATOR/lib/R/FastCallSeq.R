######## Starting Condition Fucntion ########

StartCond<-function(mdata,thru,thrd)

{
muvec<- c(-2,-1,0,0.58,1)
sdvec<-c(0.01,0.01,0.01,0.01,0.01)
lvec<-c(-50,-1.3,-thrd,thru,0.9)
uvec<-c(-1.3,-thrd,thru,0.9,50)

for (i in 1:5)
{
u<-uvec[i]
l<-lvec[i]
ind<-which(mdata<=u & mdata>=l)
if (length(ind)==0)
{
muvec[i]<-muvec[i]
sdvec[i]<-sdvec[i]
}
if (length(ind)==1)
{
muvec[i]<-mdata[ind]
sdvec[i]<-sdvec[i]
}
if (length(ind)>1)
{
muvec[i]<-mean(mdata[ind])
sdvec[i]<-sd(mdata[ind])
}
}
sdvec[which(sdvec<0.001)]<-0.001
Result<-list()
Result$muvec<-muvec
Result$sdvec<-sdvec
Result
}

################### EM Algorithm ##############

EMFastCall<-function(mdata,thru,thrd)

{
        
        StartPar<-StartCond(mdata,thru,thrd)
        muvec<-StartPar$muvec
        sdvec<-StartPar$sdvec
	prior<-c(0.05,0.1,0.7,0.1,0.05)
	lvec<-c(-20,-1.3,-thrd,thru,0.9)
	uvec<-c(-1.3,-thrd,thru,0.9,20)
        LikeliNew<-sum(PosteriorP(mdata,muvec,sdvec,prior)*prior)

	threshold<-1e-05
	for (i in 1:1000)
	{
		muvecold<-muvec
                LikeliOld<-LikeliNew
		taux<-EStep(mdata,muvec,sdvec,prior,lvec,uvec)
		MResult<-MStep(mdata,taux,muvec,sdvec)
		muvec<-MResult$mu
		sdvec<-MResult$sdev
		prior<-MResult$prior
                #MResult2<-MStep2(muvec,sdvec,lvec,uvec)
                #muvec<-MResult2$muvec
                #sdvec<-MResult2$sdev
		#truncResult<-truncUp(muvec,sdvec,thr0,Factor)
		#lvec<-truncResult$l
		#uvec<-truncResult$u
		LikeliNew<-sum(PosteriorP(mdata,muvec,sdvec,prior)*prior)
		if (abs(LikeliNew-LikeliOld)<threshold)
		{	
			break
		}
		
	}
Results<-list()
Results$muvec<-muvec
Results$sdvec<-sdvec
Results$prior<-prior
Results$iter<-i
Results$bound<-cbind(lvec,uvec)
Results
}




######## GFCT Function ##########
gfct<-function(x,moy,sdev,l,u)
{
	(dnorm(x,mean=moy,sd=sdev)*(x<=u)*(x>=l))/(pnorm(u,mean=moy,sd=sdev)-pnorm(l,mean=moy,sd=sdev))
}


######## Posterior Probability Function ##########
PosteriorP<-function(mdata,muvec,sdvec,prior)
{
		Ncount<-length(muvec)
		Posterior <- matrix(0,nrow=length(mdata),ncol=length(muvec))
		deno <- 0
		normaldataMat<-c()

		for (j in 1:Ncount)
		{
			moy<-muvec[j]
			sdev<-sdvec[j]
			normaldataMat<-cbind(normaldataMat,dnorm(mdata,mean=moy,sd=sdev))
		}
		tauMat<-c()
		for (j in 1:Ncount)
		{
			tauMat<-cbind(tauMat,c(prior[j]*(t(normaldataMat[,j]))))		}
			
		deno <- rowSums(tauMat)
		Posterior<- tauMat/deno
		Posterior
}



####### Expectation Step #############
EStep<-function(mdata,muvec,sdvec,prior,lvec,uvec)
{
	Ncount<-length(muvec)
	taux <- matrix(0,nrow=length(mdata),ncol=Ncount)
	deno <- 0
	normaldataMat<-c()
	for (g in 1:Ncount)
	{
		mu<-muvec[g]
		sdev<-sdvec[g]
		l<-lvec[g]
		u<-uvec[g]
		if (sum((mdata<=u)*(mdata>=l))!=0)
		{
		normaldataVec<-c(gfct(t(mdata),mu,sdev,l,u))
		normaldataVec[which(normaldataVec==Inf)]<-100
		normaldataMat<-cbind(normaldataMat,normaldataVec)
		} else
		{
		normaldataMat<-cbind(normaldataMat,rep(0,length(mdata)))
		}
	}
	tauMat<-c()
	for (j in 1:Ncount)
	{
		tauMat<-cbind(tauMat,c(prior[j]*(t(normaldataMat[,j]))))
	}
	deno <- rowSums(tauMat)
  ind0<-which(deno==0)
  if (length(ind0)!=0)
  {
    for (k in 1:length(ind0))
    {
      indmin<-which.min(abs(muvec-mdata[ind0[k]]))
      tauMat[ind0[k],indmin]<-1
    }
    deno <- rowSums(tauMat)
  }
	taux<- tauMat/deno
taux
}

########### Maximization Step 1 ###################
MStep<-function(mdata,taux,muvec,sdvec)
{
	N <- ncol(taux)
	moy<-c()
	sdev<-c()
	ptmp <- colSums(taux)
	pnew<-c()
	for (j in 1:N)
		{
		if (ptmp[j]!=0)
		{
			moy[j] <- (taux[,j]%*%mdata)/ptmp[j]
			if (sqrt((taux[,j]%*%((mdata-moy[j])^2))/ptmp[j])< 1e-100)
			{ 
				sdev[j]<-sdvec[j]
			} else
			{
			sdev[j]<- sqrt((taux[,j]%*%((mdata-moy[j])^2))/ptmp[j])
			}
			pnew[j]<-ptmp[j]/length(mdata)
		} else
		{
		moy[j] <- muvec[j]
		sdev[j]<- sdvec[j]
		pnew[j]<-1e-06
		}
		}
      pnew<-pnew/sum(pnew)
	ParamResult<-list()
	ParamResult$mu<-moy
	ParamResult$sdev<-sdev
	ParamResult$prior<-pnew
ParamResult
}


########### Funzione per lo step Maximization 2 ###################
MStep2<-function(muvec,sdvec,lvec,uvec)
{

muvec1<-c()
sdvec1<-c()
for (g in length(muvec))
{

l <- lvec[g]
u <- uvec[g]
moy <- muvec[g]
sdev <- sdvec[g]
variance <- sdev^2
var.init <- pmin(pmax(variance,1e-6),1000)
moy.init <- moy
for (i in 1:1000)
{
rapport <- (dnorm(l,moy,sdev)-dnorm(u,mean=moy,sd=sdev))/(pnorm(u,mean=moy,sd=sdev)-pnorm(l,mean=moy,sd=sdev))
m.iter <- moy+var.init*rapport
a1.num <- (l-m.iter)*dnorm(l,moy,sdev)-(u-m.iter)*dnorm(u,moy,sdev)
a1.deno <- pnorm(u,moy,sdev)-pnorm(l,moy,sdev)
a1 <- a1.num/a1.deno
s2.iter <- variance/(1+a1+var.init*(rapport)^2)
if ((abs(m.iter-moy)/moy>1e-6) && (abs(s2.iter-var.init)/var.init>1e-6) )
{
moy.init <- m.iter
var.init <- s2.iter
} else
{
muvec1[g] <- m.iter
sdvec1[g] <- sqrt(s2.iter)
#break
}
}
}
Result<-list()
Result$muvec<-muvec1
Result$sdev<-sdvec1
Result
}


####### Format Data for FastCall ########
MakeData<-function(TotalTable, infoPos.StartEnd)
{
	if (infoPos.StartEnd) {
	MetaTable<-TotalTable[,1:4]
	} else
	{
	MetaTable<-TotalTable[,1:3]
	}
	if (infoPos.StartEnd) {
	NumericTable<-TotalTable[,5:dim(TotalTable)[2]]
	} else
	{
	NumericTable<-TotalTable[,4:dim(TotalTable)[2]]
	}
	NExp<-(dim(NumericTable)[2])/2
	TableSeg<-as.matrix(NumericTable[,(NExp+1):(NExp*2)])
	SummaryData<-c()

	for (i in 1:NExp)
	{
		segdata<-as.numeric(TableSeg[,i])
            NData<-NumericTable[,i]
		startseg<-c(1,(1+which(diff(segdata)!=0)))
		endseg<-c(which(diff(segdata)!=0),length(segdata))
            sdvec<-c()
            for (j in 1:length(startseg))
            {
                 sdvec<-c(sdvec,sd(NData[startseg[j]:endseg[j]]))
            }
		SummaryData<-rbind(SummaryData,cbind(rep.int(i,length(startseg)),startseg,endseg,segdata[startseg],sdvec))
	}

	DataList<-list()
	DataList$SummaryData<-SummaryData
	DataList$MetaTable<-MetaTable
	DataList
}


############# Make output data #####
DataRecomp<-function(SummaryData,MetaData,out)
{
	ExpClassTotal<-c()
	ExpProbTotal<-c()
	NumExp<-length(unique(SummaryData[,1]))
	for (i in 1:NumExp)
	{
		indExp<-which(SummaryData[,1]==i)
		StartEndMat<-rbind(SummaryData[indExp,2:3])
		OutClass<-out[indExp,1]
		OutProb<-out[indExp,2]
		ExpProb<-c()
		ExpClass<-c()
		for (j in 1:length(indExp))
		{
			ExpProb[StartEndMat[j,1]:StartEndMat[j,2]]<-OutProb[j]
			ExpClass[StartEndMat[j,1]:StartEndMat[j,2]]<-OutClass[j]
		}
	ExpClassTotal<-cbind(ExpClassTotal,ExpClass)
	ExpProbTotal<-cbind(ExpProbTotal,ExpProb)
	}
TotalTableFastCall<-cbind(MetaData,ExpClassTotal,ExpProbTotal)
TotalTableFastCall
}



### Draw legends (by CGHweb team) ###
vcbar <- function(zlim, colV) 
 {
  n <- length(colV)
  plot(NA, xlim = c(0,1), ylim = c(1,n), axes = FALSE, xlab = "", ylab = "",
       main = "", xaxs = "i", yaxs = "i")
  abline(h = 1:n, col = colV, lwd = 1)
  axis(4, at = round(seq(1,n,length.out=5)), las = 2, cex.axis = 0.6,
       label = formatC(seq(zlim[1], zlim[2], length.out = 5),
         format = "f", digits = 2))
  box()
  mtext("Log-Ratio", side = 3, line = 0.5, cex = 0.7, font = 2,
        at = 0.9)
}

gllegend <-
function(Class.names)  {
if (length(Class.names) > 2)
{
  plot(NA, xlab = "", ylab = "", main = "", axes = FALSE,
       xlim = c(0,1), ylim = c(0,1), xaxs = "i", yaxs = "i")
  legend(x = "center", col = c("red","orange", "green","green4"), pch = 15, cex = 0.9,
         pt.cex = 1.2, bg = "white", legend = Class.names, bty = "n")
} else
{
 plot(NA, xlab = "", ylab = "", main = "", axes = FALSE,
       xlim = c(0,1), ylim = c(0,1), xaxs = "i", yaxs = "i")
  legend(x = "center", col = c("orange", "green"), pch = 15, cex = 0.8,
         pt.cex = 1.2, bg = "white", legend = Class.names, bty = "n")

}
}

seglegend <-
function(Method)  {
  plot(NA, xlab = "", ylab = "", main = "", axes = FALSE,
       xlim = c(0,1), ylim = c(0,1), xaxs = "i", yaxs = "i")
  legend(x = "center", col = c("grey","black"), pch=c(19,1), lty=1,lwd=c(0,1.5), cex = 0.9,
         pt.lwd = c(0.9,0), bg = "white", legend = c("data",Method), bty = "n")
}
#####################  Update limits of truncated #######
truncUp<-function(muvec,sdvec,thru,thrd,Factor)
{
	lvec<-c()
	uvec<-c()
	sigmaNeut<-Factor*sdvec[3]
	
	if (sigmaNeut<thrd)
	{
		sigmaNeut<-thrd
	}
	sigmaGain<-muvec[3]-sigmaNeut
	if (sigmaGain<0.3)
	{
		sigmaGain<-0.3
	}
	uvec4<-muvec[3]+sigmaGain
	if (uvec4>0.9)
	{
	uvec4<-0.9
	}
	lvec[1]<- -1.3
	uvec[1]<- -sigmaNeut
	lvec[2]<- -sigmaNeut
	uvec[2]<-  sigmaNeut
	lvec[3]<-  sigmaNeut
	uvec[3]<-  uvec4	
Result<-list()
Result$l<-lvec
Result$u<-uvec
Result
}


############# Assegnazione delle Label Statistiche ##########
LabelAss<-function(P0,mdata)
{
	CallResults<-rep(0,length(mdata))
	ProbResults<-c()
	
	indcall<-max.col(P0)
	CallResults[indcall==1]<- -2
	CallResults[indcall==2]<- -1
	CallResults[indcall==3]<- 0
	CallResults[indcall==4]<- 1
	CallResults[indcall==5]<- 2

	for (i in 1:length(mdata))
	{
	ProbResults<-c(ProbResults,P0[i,indcall[i]])
	}

Results<-c()
Results<-cbind(CallResults,ProbResults)
Results
}



