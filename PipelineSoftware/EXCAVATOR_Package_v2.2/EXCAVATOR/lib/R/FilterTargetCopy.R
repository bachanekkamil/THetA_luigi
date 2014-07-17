vars.tmp <- commandArgs()
vars <- vars.tmp[length(vars.tmp)]
split.vars <- unlist(strsplit(vars,","))
target.path <- split.vars[1]
Program_Folder <- split.vars[2]
target.name <- split.vars[3]
assembly <- split.vars[4]


x <- read.table(target.path,sep="\t",header=F,quote="")
chr <- as.character(x[,1])
start <- as.numeric(x[,2])
end <- as.numeric(x[,3])
gene <- as.character(x[,4])
unique.chrs <- unique(chr)


MyTarget <- c()
MyChr <- c()
for (i in 1:length(unique.chrs)) 
{
	ix.chr <- which(chr==unique.chrs[i])
	start1 <- start[ix.chr]
	end1 <- end[ix.chr]
	gene1 <- gene[ix.chr]
	indS <- sort(start1,index.return=T)$ix
	startS <- start1[indS]
	endS <- end1[indS]
	geneS <- gene1[indS]
	indF <- unique(which(duplicated(startS)),which(duplicated(endS)))
	if (length(indF)==0)
	{
		startF <- startS
		endF <- endS
		geneF <- geneS	
	}
	if (length(indF)!=0)
	{
		startF <- startS[-indF]
		endF <- endS[-indF]
		geneF <- geneS[-indF]	
	}
	exondist <- startF[-1]-endF[-length(endF)]
	exondist <- c(1,exondist)
	ind <- which(exondist>0)
	MyTarget<- rbind(MyTarget,cbind(rep(unique.chrs[i],length(ind)),startF[ind],endF[ind],geneF[ind]))
	MyChr[i] <- unique.chrs[i]
}
dir.create(file.path(Program_Folder,"data","targets",assembly))
dir.create(file.path(Program_Folder,"data","targets",assembly,target.name))
dir.create(file.path(Program_Folder,"data","targets",assembly,target.name,"GCC"))
dir.create(file.path(Program_Folder,"data","targets",assembly,target.name,"MAP"))
write.table(data.frame(rbind(MyChr)),file.path(Program_Folder,"data","targets",assembly,target.name,paste(target.name,"_chromosome.txt",sep="")),col.names=F,row.names=F,quote=F)
save(MyTarget,file=file.path(Program_Folder,"data","targets",assembly,target.name,paste(target.name,".RData",sep="")))
write.table(data.frame(MyTarget),file.path(Program_Folder,"data","targets",assembly,target.name,paste("Filtered.txt",sep="")),col.names=F,row.names=F,quote=F)

