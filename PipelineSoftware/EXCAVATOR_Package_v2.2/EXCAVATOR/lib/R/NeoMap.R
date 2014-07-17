vars.tmp <- commandArgs()
vars <- vars.tmp[length(vars.tmp)]
split.vars <- unlist(strsplit(vars,","))
chrsel <- split.vars[1]
Program_Folder <- split.vars[2]
TargetFile <- split.vars[3]
DataTemp <- split.vars[4]
MAPPath <- split.vars[5]


dyn.load(paste(Program_Folder,"/lib/F77/F4R.so",sep=""))

load(TargetFile)
chrTarget <- as.character(MyTarget[,1])
indT <- which(chrTarget==chrsel)
X <- as.numeric(MyTarget[indT,2])
Y <- as.numeric(MyTarget[indT,3])

filecon <- file(DataTemp,open="r")

jex <- 1
stepread <- 100000
logit <- 1
PASFLAG <- 0
PSCORE <- 0. 
NEX <- length(X)
MapMed <- rep(0,NEX)

while (logit!=0) 
	{
		tt <- readLines(filecon,n=stepread)
		if (length(tt) > 0) 
		{
			pari <- seq(2,length(tt)*3, by = 3)
			dispari <- seq(1,length(tt)*3, by = 3)
			trispari <- seq(3,length(tt)*3, by = 3)
			
	    	A <- as.integer(unlist(strsplit(tt,"\t"))[dispari])
	    	B <- as.integer(unlist(strsplit(tt,"\t"))[pari])
	    	CC <- as.integer(unlist(strsplit(tt,"\t"))[trispari])
	    	Nchunk <- length(A)	    

			out <- .Fortran("NEOMAP",as.integer(A),as.integer(B),as.double(CC),as.integer(Nchunk),as.integer(X),as.integer(Y),as.integer(NEX),as.integer(jex),as.double(PSCORE),as.integer(PASFLAG),as.double(MapMed))
		
			jex <- out[[8]]
			PSCORE <- out[[9]]
			PASFLAG <- out[[10]]
			MapMed <- out[[11]]
		} else 
		{
			Nchunk <- 11
		}
		if (Nchunk < stepread)
		{
			logit=0
		}
	}

close.connection(filecon)
save(MapMed,file=file.path(MAPPath,paste("Map.",chrsel,".RData",sep="")))
