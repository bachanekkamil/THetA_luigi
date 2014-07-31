import luigi
import subprocess
import glob
import os
import sys
from time import strftime, gmtime

#############################################################################################################
#This pipeline automates genome downloading and processing from CGHub. The bottom of the file is the head of the graph.
#############################################################################################################
__author__ = "David Liu"
__version__ = 1.0


"""
Global variables
"""
#Configure things here.
output_dir = os.path.abspath("./all_outputs")
download_dir = os.path.abspath("./all_downloads")

subprocess.call(["mkdir", output_dir])
subprocess.call(["mkdir", download_dir])

pipeline_output = luigi.Parameter(is_global = True, default = output_dir)
pipeline_downloads = luigi.Parameter(is_global = True, default = download_dir)


#############################################################################################################
#Deletes the BAM files after everything is finished.
#############################################################################################################
class deleteBAMFiles(luigi.Task):
	prefix = luigi.Parameter()
	pipeline_output = pipeline_output
	pipeline_downloads = pipeline_downloads
	time_began = strftime("Time began: %a, %d %b %Y %H:%M:%S", gmtime())
	this_download_dir = ""
	this_out_dir = ""
	def requires(self):
		self.this_out_dir = os.path.join(self.pipeline_output, self.prefix)
		self.this_download_dir = os.path.join(self.pipeline_downloads, self.prefix)
		subprocess.call(["mkdir", self.this_out_dir])
		subprocess.call(["mkdir", self.this_download_dir])
		return {"RunTHetA":RunTHetA(prefix = self.prefix, out_dir = self.this_out_dir, download_dir = self.this_download_dir)}
	def run(self):
		# subprocess.call(["rm", "-rf", self.this_download_dir])
		file = open(os.path.join(self.this_out_dir, "job_summary.txt"), "w")
		file.write("Sample barcode: " + self.prefix + "\n")
		file.write("Sample analysis_uri_id " + prefix_to_id[self.prefix] + "\n")		
		file.write(self.time_began + "\n")
		file.write(strftime("Time finished: %a, %d %b %Y %H:%M:%S", gmtime()))
		file.close()
	def output(self):
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "job_summary.txt"))


#############################################################################################################
#RunTheta depends on the intervalCountingPipeline and BICSeqToTHetA.
#############################################################################################################
class RunTHetA(luigi.Task):
	prefix = luigi.Parameter()
	out_dir = luigi.Parameter()
	download_dir = luigi.Parameter()
	this_out_dir = ""
	def requires(self):
		self.this_out_dir = os.path.join(self.out_dir, "THetA")		
		subprocess.call(["mkdir", self.this_out_dir])
		# theta_input_dir = os.path.join(self.this_out_dir, "THetA_input")
 	# 	subprocess.call(["mkdir", theta_input_dir])	
		return {'BICSeq': BICSeq(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir),
				'intervalCountingPipeline': intervalCountingPipeline(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir)}
	def run(self):
		#Get bicseg location
		bicseq_output_loc = os.path.join(self.out_dir, "BICSeq/output", self.prefix + ".bicseg")
		if subprocess.call(["./pipeline/scripts/runTHetA.sh", bicseq_output_loc, self.prefix, self.this_out_dir, "/gpfs/main/research/compbio/users/ddliu/luigi_pipeline/all_outputs/a_genome/intervalPipeline/intervals.txt_processed"]) != 0:
			sys.exit()
		subprocess.call(["touch", os.path.join(self.this_out_dir, "THetA_complete.txt")])
	def output(self):
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "THetA_complete.txt"))

#############################################################################################################
#Define the main workflow dependency graph.
#############################################################################################################

"""
The C++ code that counts the 50kb bins.
"""
class intervalCountingPipeline(luigi.Task):
	prefix = luigi.Parameter()
	out_dir = luigi.Parameter()
	download_dir = luigi.Parameter()	
	this_out_dir = ""
	def requires(self):
		return BAMtoGASV(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir)
	def run(self):
		self.this_out_dir = os.path.join(self.out_dir, "intervalPipeline")
		subprocess.call(["mkdir", self.this_out_dir])
		parameter_file_path = os.path.abspath(os.path.join(self.this_out_dir, "parameters.txt"))
		normal_conc = os.path.join(self.out_dir, "BAMtoGASV_output", "NORMAL", "NORMAL_" + "TCGA-BF-A1PZ-10A-01D-A18Z" + ".concordant")
		tumor_conc = os.path.join(self.out_dir, "BAMtoGASV_output", "TUMOR", "TUMOR_" + "TCGA-BF-A1PZ-01A-11D-A18Z" + ".concordant")
		#with open(parameter_file_path, "w") as parameter_file:
		#	parameter_file.write("IntervalFile: "+ os.path.abspath("all_outputs/a_genome/intervalPipeline/intervals.txt") + "\n")
		#	parameter_file.write("Software: PREGO" + "\n")
		#	parameter_file.write("ConcordantFile: " + "/gpfs/main/research/compbio/users/ddliu/luigi_pipeline/all_outputs/a_genome/BAMtoGASV_output/NORMAL/NORMAL_TCGA-BF-A1PZ-10A-01D-A18Z.concordant" + "\n")
		#	parameter_file.write("ConcordantFile: " + "/gpfs/main/research/compbio/users/ddliu/luigi_pipeline/all_outputs/a_genome/BAMtoGASV_output/TUMOR/TUMOR_TCGA-BF-A1PZ-01A-11D-A18Z.concordant")
		#if subprocess.call(["./pipeline/scripts/runIntervalPipeline.sh", self.this_out_dir, parameter_file_path]) != 0:
		#	sys.exit()
	#	subprocess.call(["touch", os.path.join(self.this_out_dir, "intervalsDone.txt")])
	def output(self):
		#????????????????
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "intervalsDone.txt"))
	# def complete(self):
	# 	return True

"""
THetA stuff
"""
class BAMtoGASV(luigi.Task):
	#Also runs GASV.
	prefix = luigi.Parameter()
	out_dir = luigi.Parameter()
	download_dir = luigi.Parameter()
	this_out_dir = ""
	def requires(self):
		self.this_out_dir = os.path.join(self.out_dir, "BAMtoGASV_output")
		subprocess.call(["mkdir", self.this_out_dir])		
	def run(self):
		#Run BAMtoGASV
		normal_dir = os.path.join(self.this_out_dir, "NORMAL") 
		tumor_dir = os.path.join(self.this_out_dir, "TUMOR")
		subprocess.call(["mkdir", normal_dir])
		subprocess.call(["mkdir", tumor_dir])
		#Run on normal
		#if subprocess.call(["./pipeline/scripts/runBAMtoGASV.sh", normal_dir, os.path.abspath(os.path.join("all_downloads/a_genome/NORMAL", 'TCGA-BF-A1PZ-10A-01D-A18Z_120612_SN590_0162_BC0VNGACXX_s_6_rg.sorted.bam')), "NORMAL"]) != 0:
		#	sys.exit(0)
		#Run on tumor
		#if subprocess.call(["./pipeline/scripts/runBAMtoGASV.sh", tumor_dir, os.path.abspath(os.path.join("all_downloads/a_genome/TUMOR", 'TCGA-BF-A1PZ-01A-11D-A18Z_120612_SN590_0162_BC0VNGACXX_s_5_rg.sorted.bam')), "TUMOR"]) != 0:
		#	sys.exit(0)
		#subprocess.call(["touch", os.path.join(self.this_out_dir, "BAMtoGASVfinished.txt")])		
	def output(self):
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "BAMtoGASVfinished.txt"))

#BICSeq
class BICSeq(luigi.Task):
	prefix = luigi.Parameter()
	out_dir = luigi.Parameter()
	download_dir = luigi.Parameter()
	this_out_dir = ""
	def requires(self):
		self.this_out_dir = os.path.join(self.out_dir, "BICSeq")
		subprocess.call(["mkdir", self.this_out_dir])		
		return BAMtoGASV(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir)
	def run(self):
		# Takes concordant files as input
		normal_conc = os.path.join(self.out_dir, "BAMtoGASV_output", "NORMAL", "NORMAL_" + "TCGA-BF-A1PZ-10A-01D-A18Z" + ".concordant")
		tumor_conc = os.path.join(self.out_dir, "BAMtoGASV_output", "TUMOR", "TUMOR_" + "TCGA-BF-A1PZ-01A-11D-A18Z" + ".concordant")
		bicseq_input_loc = self.this_out_dir #To be created
		#Run script
		#if subprocess.call(["./pipeline/scripts/runBICseq.sh", self.this_out_dir, tumor_conc, normal_conc, bicseq_input_loc, self.prefix]) != 0:
		#	sys.exit()
		#Remove input file
		#subprocess.call(["rm", "-f", os.path.join(self.this_out_dir, "*.input")])
		#done
		#subprocess.call(["touch", os.path.join(self.this_out_dir, "BICSeqDone.txt")])
	def output(self):
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "BICSeqDone.txt"))


#############################################################################################################
#Run Pipeline
#############################################################################################################

class TriggerAll(luigi.Task):
	global tasks_to_run
	def requires(self):
		for task in tasks_to_run:
			yield task
	def run(self):
		subprocess.call(["touch", "pipelineComplete.txt"])
	def output(self):
		return luigi.LocalTarget("pipelineComplete.txt")

#############################################################################################################
#Set up the internal data usage format
#############################################################################################################

#GLOBAL DATA FORMAT

# {
# 	sample_name (e.g. Ovarian Cancer X Y):{
# 		norm_prefix
# 		tumor_prefix
# 		norm_dir
# 		tumor_dir
# 		ref_assem
# 		#Optional for download: norm_aurid, tumor_aurid
# 	}
# }




#Create tasks_to_run from the names in the file.
tasks_to_run = []
global prefix_to_id
prefix_to_id = {}


tasks_to_run.append(deleteBAMFiles("a_genome"))

# with open("seq_info.txt", "r") as seq_file:
# 	count = 0
# 	for line in URI_file:
# 		count += 1
# 		if count == 8:
# 			break
# 		line = line.strip()
# 		components = line.split("\t")
# 		#0: id, 1:prefix
# 		#Add to the dictionary
# 		try:
# 			prefix_to_id[components[1]] = components[0]
# 			tasks_to_run.append(deleteBAMFiles(components[1]))
# 		except:
# 			continue


global samples
samples = []


# print tasks_to_run

luigi.build([TriggerAll()], workers=1)