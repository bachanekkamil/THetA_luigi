### define a function to read in outpus of Joe's program
read.bic = function(file)
	{ con = file(file,open="r")
	  flag = T
	  while(flag)
		{ lne = readLines(con,n=1)
		  tmp = substr(lne,1,1)
		  if(tmp!="I"&&tmp!="M"&&tmp!="\t"&&tmp!="c"&&tmp!="l") { flag = F; pushBack(lne,con)}
		}
	  tab.bic = read.table(con)
	  close(con)
	  return(tab.bic)
	}



getSummary = function(segs, correction=TRUE,cutoff=0.3){
	totalCase = sum(as.numeric(segs$caseRead))
	totalControl = sum(as.numeric(segs$controlRead))
	adjust = totalCase/totalControl
	genomeSize = max(sum(as.numeric(segs$end - segs$start + 1)),1)
	
	ratios = log2(segs$caseRead/segs$controlRead) - log2(adjust)
	ind.rm = which(abs(ratios)>cutoff)
	if(length(ind.rm)>0){
		totalCase.tmp = totalCase -  sum(as.numeric(segs$caseRead[ind.rm]))
		totalControl.tmp = totalControl - sum(as.numeric(segs$controlRead[ind.rm]))

		if(totalCase.tmp>100&&totalControl.tmp>100){
			totalCase = totalCase.tmp
			totalControl = totalControl.tmp
			}
		}
	adjust = totalCase/totalControl
	ratios = log2(segs$caseRead/segs$controlRead) - log2(adjust)

	prob = totalCase/(totalCase+totalControl)
	pvalue = rep(1,length(ratios))
	for(k in c(1:length(pvalue))){
		ttInBin  = (segs$caseRead[k]+segs$controlRead[k])
		p = segs$caseRead[k]/ttInBin
		if(p<=prob){
			p = (segs$caseRead[k]+0.5)/ttInBin
			pvalue[k] = pnorm((p-prob)*sqrt(ttInBin),sd=sqrt(prob*(1-prob)),log.p = TRUE)
			}else{
			probN = 1- prob
			p = (segs$controlRead[k] + 0.5)/ttInBin
			pvalue[k] = pnorm((p-probN)*sqrt(ttInBin),sd=sqrt(probN*(1-probN)),log.p = TRUE)
			}
		}
	pvalue = pvalue/log(10)

	if(correction!=FALSE){
		pvalue = pvalue + log10(genomeSize)
		ind.tmp = which(pvalue>=0.0)
		if(length(ind.tmp)>0){pvalue[ind.tmp] = 0.0}
		}
	pvalue = round(pvalue,4)

	segs.summary = data.frame(chrom=segs$chrom,start=segs$start,end=segs$end,caseRead=segs$caseRead,controlRead=segs$controlRead,log2.copyRatio=round(ratios,4),log10.pvalue=pvalue)
	return(segs.summary)
	}


### define new post processing function (use different formula to calculate the copy ratio)
### The first argument of this function should be a list of BIC-seq results (from chrm1 - chrm X or Y)
### files.in: a list of BIC-seq files
### chrm: the chromosome names corresponds to each file in files.in
### file.out: the output file


post_proc = function(files.in,chrm, file.out)
		{ colnames = c("chrom","start","end","caseRead","controlRead","log2.copyRatio","log10.pvalue")
		  bin = NULL
		  chromosome = NULL
		  if(length(files.in)!=length(chrm)||length(files.in)==0)
			{ stop("the length of the first argument and the second argument must be the same and positive.")}
		  for(i in c(1:length(files.in)))
	  	  	{ file = files.in[i]
			  bin.tmp = read.bic(file)
			  bin = rbind(bin,bin.tmp)
			  chromosome.tmp = rep(chrm[i],dim(bin.tmp)[1])
			  chromosome = c(unlist(chromosome),chromosome.tmp)
			  rm(bin.tmp,chromosome.tmp)
			}
		  start = bin[,4]
		  end = bin[,5]
		  caseRead = bin[,1]
		  controlRead = bin[,2] - bin[,1]
		  caseFreq = bin[,3]

		  segs = data.frame(chrom=chromosome,start=start,end=end,caseRead=caseRead,controlRead=controlRead)
		  segs = getSummary(segs)

		  #rk = order(segs$chrom,segs$start)
		  write.table(segs,file=file.out,row.names=F,sep="\t",quote=F)

		  return(segs)
		}

#### post-processing for the resampled data
post_proc_permu = function(files.in,chrm,file.out){
	#colnames = c("resample","chrom","start","end","caseRead","controlRead","caseFreq","copyRatio","log10.pvalue")
        if(length(files.in)!=length(chrm)||length(files.in)==0)
		{ stop("the length of the first argument and the second argument must be the same and positive.")}

	bin = NULL
        chromosome = NULL
        for(i in c(1:length(files.in))){ 
		file = files.in[i]
		bin.tmp = read.table(file,header=T,sep="\t")
		bin = rbind(bin,bin.tmp)
		chromosome.tmp = rep(chrm[i],dim(bin.tmp)[1])
		chromosome = c(unlist(chromosome),chromosome.tmp)
		rm(bin.tmp,chromosome.tmp)
		}
	bin = data.frame(chrom=chromosome,bin)
	
	bic = NULL
	for(i in c(1:max(bin$resample))){
		ind.rs = which(bin$resample==i)
		if(length(ind.rs)>0){
			caseRead = bin$caseRead[ind.rs]
			controlRead = bin$totalRead[ind.rs] - caseRead
			segs.tmp = data.frame(chrom=bin$chrom[ind.rs],start=bin$start[ind.rs],end=bin$end[ind.rs],caseRead=caseRead,controlRead=controlRead)
			segs.tmp = getSummary(segs.tmp)
			segs.tmp = data.frame(resample=rep(i,nrow(segs.tmp)),segs.tmp)
			bic = rbind(bic,segs.tmp)
			}
		}

	write.table(bic,file=file.out,row.names=FALSE,sep="\t",quote=FALSE)
	return(bic)
	}


#### plot functions
highlightChrom <- function(adjustments, min, max){
  for(index in 1:(length(adjustments)-1)){
    if(index %% 2 == 1){
      polygon(c(adjustments[index], adjustments[index + 1],
                adjustments[index + 1], adjustments[index]),
              c(min, min, max, max), col = "gray", border = "white")
    }
  }
  return(invisible())
}



markChrom <- function(adjustments, min,chromNames,height=10){
  chromLocs <- NULL
  ind = NULL
  for(i in 1:(length(adjustments) - 1)){
    if(i %% 2 == 1){
      chromLocs <- c(chromLocs, mean(c(adjustments[i], adjustments[i + 1])))
      ind = c(ind,i)
    }
  }
  if(length(chromLocs)!=length(chromNames[ind])){stop("Incorrect number of chromsome names")}
  text(chromLocs, rep(min - 0.125, length(chromLocs)), chromNames[ind], cex = 1.2)
  return(invisible())
}


markboundary <- function(adjustments,min,max){
    if(length(adjustments)<3) return;
    for(i in c(2:(length(adjustments-1)))){
        lines(rep(adjustments[i],2),c(min,max))
        }
}





args <- commandArgs(TRUE)
if(length(args)!=4&&length(args)!=3)
	{ print("Usage: \'R --slave --args <BIC files> <Chrom names> <OutDir> <description> < BIC-postprocessing.R\'")
	  print("or")
	  print("       \'R --slave --args <segment files for the resampled data> <Chrom names> <output> < BIC-postprocessing.R\'")
	  q(status=1)
	}


if(length(args)==4){
	files.in = unlist(strsplit(args[[1]],","))
	chrm = unlist(strsplit(args[[2]],","))
	out.dir = args[[3]]
	#file.out = args[[3]]
	#wig.file.name = args[[4]]
	description = args[[4]]
	len.outdir = nchar(out.dir)
	if(substring(out.dir,len.outdir,len.outdir)!="/") {out.dir = paste(out.dir,"/",sep="")}
	file.out = paste(out.dir,description,".bicseg",sep="")
	wig.file.name = paste(out.dir,description,".wig",sep="")
	pngfile = paste(out.dir,description,".png",sep="")

	segs = post_proc(files.in,chrm,file.out)

	wig.header.template = "track type=wiggle_0 name=\"Bed Format\" description=\"TBA\" visibility=dense color=200,100,0 altColor=0,100,200 priority=20"
	wig.header = sub("TBA",description,wig.header.template)

	wig.file = file(wig.file.name,open = "w")

	write(wig.header,file=wig.file)
	write.table(segs[,c(1:3,6)],file = wig.file,row.names=F,quote=F,col.names=F)
	close(wig.file)

	### now start plotting
	
	png(filename=pngfile,width=1200,height=500)
	chrom.code = aggregate(1:nrow(segs),by=list(segs$chrom),min) ### make sure the chromosomes are ordered as loaded
	rk = order(chrom.code[,2])
	chrom.length = aggregate(as.numeric(segs$end),by=list(segs$chrom),max)
	chrom.length = chrom.length[rk,]
	num.segs.chrom = aggregate(segs$end,by=list(segs$chrom),length)
	num.segs.chrom = num.segs.chrom[rk,]
	chrom.boundary = c(0,cumsum(chrom.length[,2]))
	plot(0, 0, type = "n", main = description, xlab = "Chromosome",
         ylab = " Log2 ratio", ylim = c(-5, 5), axes = FALSE,
         xlim = c(0, max(chrom.boundary) + 10))
	
	highlightChrom(chrom.boundary,-5,5)
	lines(c(min(chrom.boundary), max(chrom.boundary)), rep(0, 2), lwd = 1, col = "blue")
        lines(c(min(chrom.boundary), max(chrom.boundary)), rep(-1, 2), lwd = 1, col = "blue")
        lines(c(min(chrom.boundary), max(chrom.boundary)), rep(1, 2), lwd = 1, col = "blue")
	axis(2)
	box()

	adjust = rep(chrom.boundary[-length(chrom.boundary)],num.segs.chrom[,2])
	loc.start = segs$start + adjust
	loc.end = segs$end + adjust
	for(k in c(1:length(loc.start))){
		lines(c(loc.start[k],loc.end[k]),rep(segs$log2.copyRatio[k],2),col="red",lwd=2)
		}
	markChrom(chrom.boundary,-5,chrom.length[,1])
	dev.off()

	}

if(length(args)==3){
	files.in = unlist(strsplit(args[[1]],","))
	chrm = unlist(strsplit(args[[2]],","))
	file.out = args[[3]]

	seg.bic = post_proc_permu(files.in,chrm,file.out)
	}









