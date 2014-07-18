#THetA pipeline

This pipeline uses [Luigi](https://github.com/spotify/luigi) to automate the download, processing, and data analysis of files related to THetA. 

![Dependency chart](/path/to/image)

#Execution

To execute the pipeline, simply run `python luigi_pipeline.py`. Use `./install` to ensure the correct dependencies and to generate the `.class` files.

#Structure

##Included software
+ CGHub
+ intervalCountingPipeline (for read depth)
+ BAMtoGASV, BICSeq, BICSeqtoTHetA
+ THetA
+ Virtual SNP Array
	+ Download `all_snps.tsv` from [HERE-add link](some/link) (Or maybe generate it?)

The pipeline code is in `luigi_pipeline.py`. 

The scripts for downloading `.bam` files are in `/CGHub`.

The executables for each software component are in `/pipelineSoftware`.

The shell scripts for running each step of the pipeline are in `/pipeline`. 

##Step 1: Generating the files to download

The pipeline takes as input `analysis_URIS.txt`, which contains tab-delimted lines of *analysis_uri_id	prefix*. To generate this file, use `./cgquery "library_strategy=WGS" >> WGS_results.txt` (as an example) in `CGHub/cgquery`. Then run `parse_WGS_results.py` to generate `analysis_URIS.txt`. This may become automated as well in the future, but for now, flexibility is a priority. Move the file to the top directory level.
