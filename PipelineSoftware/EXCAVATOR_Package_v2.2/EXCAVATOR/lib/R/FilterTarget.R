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


MyTarget <- x
MyChr <- unique.chrs


dir.create(file.path(Program_Folder,"data","targets",assembly))
dir.create(file.path(Program_Folder,"data","targets",assembly,target.name))
dir.create(file.path(Program_Folder,"data","targets",assembly,target.name,"GCC"))
dir.create(file.path(Program_Folder,"data","targets",assembly,target.name,"MAP"))
write.table(data.frame(rbind(MyChr)),file.path(Program_Folder,"data","targets",assembly,target.name,paste(target.name,"_chromosome.txt",sep="")),col.names=F,row.names=F,quote=F)
save(MyTarget,file=file.path(Program_Folder,"data","targets",assembly,target.name,paste(target.name,".RData",sep="")))
write.table(data.frame(MyTarget),file.path(Program_Folder,"data","targets",assembly,target.name,paste("Filtered.txt",sep="")),col.names=F,row.names=F,quote=F)

