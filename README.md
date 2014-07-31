#THetA pipeline

This pipeline uses [Luigi](https://github.com/spotify/luigi) to automate the download, processing, and data analysis of files related to THetA. THetA is a software suite that exploits cancer genomic structural variation to determine tumor heterogeneity in a sample. 

THetA was written by the [Raphael Lab](http://compbio.cs.brown.edu/) at Brown University.

![Sample output](/path/to/image)

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

The pipeline takes a JSON file with the names of samples and associated metadata as input. To generate this file, use `./pipeline/cgquery` with appropriate parameters.
###Procedure
1. Use a query such as `./pipeline/cgquery "library_strategy=WGS" >> seq.txt`. Make sure the output is piped to `seq.txt`.
2. In `pipeline/`, run `processCGQueryforPipeline.py`. A JSON file called `tumor_sample_info.json` should be generated in the top level directory.

##Step 2: Start up the server

Start a luigi server by running `luigid` in a seperate terminal process. This is the central scheduler which assigns tasks to workers.

If that doesn't work, try `PYTHONPATH=. python bin/luigid`. Note that you need Tornado for this to work. 

The server will run on port 8082.

##Step 3: Run the pipeline

Start the pipeline by running `python luigi_pipeline.py`. The pipeline will start running automatically if the `tumor_sample_info.json` file exists. Downloads will be written to `all_downloads` and outputs will be written to `all_outputs`. 

Some useful configurations with the command line may include:

+ `--local-scheduler` : This runs the pipeline without the central planner. Note that you can only use 1 worker if you use the local scheduler.
+ `--workers X` : Runs the pipeline with X workers.

While the pipeline is running, go to `http://localhost:8082` to get a dynamic overview of the pipeline's excecution.






