vars.tmp <- commandArgs()
vars <- vars.tmp[length(vars.tmp)]
split.vars <- unlist(strsplit(vars,","))
File_In <- split.vars[1]
Working_Folder <- split.vars[2]
chr <- split.vars[3]

GCContent <- scan(file=File_In)
save(GCContent,file=file.path(Working_Folder,paste("GCC.",chr,".RData",sep="")))
