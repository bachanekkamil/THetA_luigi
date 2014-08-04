#THetA pipeline

This pipeline uses [Luigi](https://github.com/spotify/luigi) to automate the download, processing, and data analysis of THetA. THetA is a software suite that uses cancer genomic structural variation to determine tumor heterogeneity in a sample. 

THetA was developed and written by the [Raphael Lab](http://compbio.cs.brown.edu/) at Brown University's CCMB.

#Structure

Use `./install` to ensure the correct dependencies, download the SNP files, and to generate the `.class` files.

The pipeline code is in `luigi_pipeline.py`. 

The executables for each software component are in `/pipelineSoftware`.

The shell scripts for running each step of the pipeline are in `/pipeline`. 

##Included software
+ CGHub
+ intervalCountingPipeline (for read depth)
+ BAMtoGASV, BICSeq, BICSeqtoTHetA
+ THetA (as a submodule, since updates are still occurring.)
+ Virtual SNP Array
	+ SNP files hosted on S3, downloaded in install script.

##Dependency chart

![Dependency Chart](PipelineSoftware/dep_chart.png)

##Output structure

	all_downloads
	`-- a_genome
	    |-- NORMAL
	    |   `-- downloadHash
	    |       |-- normal_genome.bam
	    |       `-- normal_genome.bam.bai
	    `-- TUMOR
	        `-- downloadHash
	            |-- tumor_genome.bam
	            `-- tumor_genome.bam.bai
	            
	all_outputs
	`-- a_genome
	    |-- BAMtoGASV_output
	    |   |-- NORMAL
	    |   |   |-- NORMAL.a_genome-lib1.concordant
	    |   |   |-- NORMAL.a_genome-lib1.deletion
	    |   |   |-- NORMAL.a_genome-lib1.divergent
	    |   |   |-- NORMAL.a_genome-lib1.inversion
	    |   |   `-- NORMAL.a_genome-lib1.translocation
	    |   `-- TUMOR
	    |       `-- same_as_NORMAL
	    |-- BICSeq
	    |   |-- a_genome.bicseg
	    |   `-- bicseq_outputs
	    |-- intervalPipeline
	    |   `-- a_genome.txt.processed
	    |-- THetA
	    |   |-- a_genome.n2.pdf
	    |   |-- a_genome.n2.results
	    |   `-- a_genome.N_processed
	    `-- virtualSNPArray
	        `-- a_genome.BAlleleFreqs

#Execution

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
+ `--workers X` : Runs the pipeline with X workers. By default, it runs with 16 workers.

While the pipeline is running, go to `http://localhost:8082` to get a dynamic overview of the pipeline's excecution.






