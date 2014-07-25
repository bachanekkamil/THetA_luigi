import luigi
import subprocess
import glob
import os
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
		return {"RunTHetA":RunTHetA(prefix = self.prefix, out_dir = self.this_out_dir, download_dir = self.this_download_dir), 
				"virtualSNPArray": virtualSNPArray(prefix = self.prefix, out_dir = self.this_out_dir, download_dir = self.this_download_dir), 
				"intervalCountingPipeline": intervalCountingPipeline(prefix = self.prefix, out_dir = self.this_out_dir, download_dir = self.this_download_dir)}
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
#The virtualSNPArray depends on the intervalCountingPipeline and the SNP filter.
#############################################################################################################
class virtualSNPArray(luigi.Task):
	prefix = luigi.Parameter()
	out_dir = luigi.Parameter()
	download_dir = luigi.Parameter()
	this_out_dir = ""
	def requires(self):
		self.this_out_dir = os.path.join(self.out_dir, "SNP_array")
		subprocess.call(["mkdir", self.this_out_dir])
		return {'countAndFilterSNPs': countAndFilterSNPs(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir), 
				'intervalCountingPipeline': intervalCountingPipeline(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir)}
	def run(self):
		#run theta
		subprocess.call(["touch", os.path.join(self.this_out_dir, "virtualSNPArray.txt")])

	def output(self):
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "virtualSNPArray.txt"))

#############################################################################################################
#RunTheta depends on the intervalCountingPipeline and BICSeqToTHetA.
#############################################################################################################
class RunTHetA(luigi.Task):
	prefix = luigi.Parameter()
	out_dir = luigi.Parameter()
	download_dir = luigi.Parameter()
	this_out_dir = ""
	def requires(self):
		self.this_out_dir = os.path.join(theta_dir, "THetA_input")
 		subprocess.call(["mkdir", self.this_out_dir])
		self.this_out_dir = os.path.join(self.out_dir, "THetA")		
		subprocess.call(["mkdir", self.this_out_dir])
		return {'BICSeq': BICSeq(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir), 
				'intervalCountingPipeline': intervalCountingPipeline(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir)}
	def run(self):
		#Get bicseg location
		bicseq_output_loc = os.path.join(self.out_dir, "BICSeq", prefix + ".bicseg")
		subprocess.call(["./PipelineSoftware/theta/bin/RunTHetA.sh", bicseq_output_loc, self.prefix, self.this_out_dir, READ_DEPTH_FILE_LOC])




		subprocess.call(["touch", os.path.join(self.this_out_dir, self.prefix + ".pdf")])

	def output(self):
		return luigi.LocalTarget(os.path.join(self.this_out_dir, self.prefix + ".pdf"))


#############################################################################################################
#Define the main workflow dependency graph.
#############################################################################################################

#############################################################################################################
#Once the BAM file is downloaded, we run three processes. The intervalCountingPipeline, BAMtoGASV, and count SNPs
#############################################################################################################
"""
Java program that counts SNPs. Python code that filters them.
"""
class countAndFilterSNPs(luigi.Task):
	prefix = luigi.Parameter()
	out_dir = luigi.Parameter()
	download_dir = luigi.Parameter()
	this_out_dir = ""		
	def requires(self):
		snp_dir = os.path.join(self.out_dir, "SNP_array")
		subprocess.call(["mkdir", snp_dir])
		self.this_out_dir = os.path.join(snp_dir, "Processed_SNPs")
		subprocess.call(["mkdir", self.this_out_dir])
		return downloadGenome(self.prefix)	
	def run(self):
		#Write the parameters file.
		#Make the file
		#Write SNP_FILE = path/to/snp/file
		#OUTPUT_PREFIX = out_dir/output
		#Write all bam files
		# subprocess.call(["java - classpath SOMESTUFF"])
		#Filter and grouping code
		subprocess.call(["touch", os.path.join(self.this_out_dir, "countAndFilterSNPs.txt")])
	def output(self):
		# return glob.glob("OUT_DIR/output.withCounts")
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "countAndFilterSNPs.txt"))

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
		return downloadGenome(self.prefix)
	def run(self):
		#Run BAMtoGASV
		normal_dir = os.path.join(this_out_dir, "NORMAL") 
		tumor_dir = os.path.join(this_out_dir, "TUMOR")
		subprocess.call(["mkdir", normal_dir])
		subprocess.call(["mkdir", tumor_dir])
		#Run on normal
		subprocess.call(["./pipeline/scripts/runBAMtoGASV.sh", normal_dir, PATHTONORMALBAMFILE, "NORMAL"])
		#move files
		# subprocess.call(["mv", "*.gasv.in", normal_dir])
		# subprocess.call(["mv", "*.info", normal_dir])
		# subprocess.call(["mv", "*.deletion", normal_dir])
		# subprocess.call(["mv", "*.divergent", normal_dir])
		# subprocess.call(["mv", "*.insertion", normal_dir])
		# subprocess.call(["mv", "*.inversion", normal_dir])
		# subprocess.call(["mv", "*.translocation", normal_dir])
		# subprocess.call(["mv", "*.concordant", normal_dir])
		#Run on tumor
		subprocess.call(["./pipeline/scripts/runBAMtoGASV.sh", tumor_dir, PATHTOTUMORBAMFILE, "TUMOR"])
		#Move files
		# subprocess.call(["mv", "*.gasv.in", normal_dir])
		# subprocess.call(["mv", "*.info", tumor_dir])
		# subprocess.call(["mv", "*.deletion", tumor_dir])
		# subprocess.call(["mv", "*.divergent", tumor_dir])
		# subprocess.call(["mv", "*.insertion", tumor_dir])
		# subprocess.call(["mv", "*.inversion", tumor_dir])
		# subprocess.call(["mv", "*.translocation", tumor_dir])
		# subprocess.call(["mv", "*.concordant", tumor_dir])
		subprocess.call(["touch", os.path.join(self.this_out_dir, "BAMtoGASVfinished.txt")])		
	def output(self):
		# return luigi.LocalTarget("path/to/output/stuffz")
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
		normal_conc = os.path.join(out_dir, "BAMtoGASV_output", "NORMAL", prefix + ".concordant")
		tumor_conc = os.path.join(out_dir, "BAMtoGASV_output", "TUMOR", prefix + ".concordant")
		bicseq_input_loc = this_out_dir #To be created
		#Run script
		subprocess.call(["./pipeline/scripts/runBICseq.sh", self.this_out_dir, tumor_conc, normal_conc, bicseq_input_loc, self.prefix])
		#Remove input file
		subprocess.call(["rm", "-f", os.path.join(this_out_dir, "*.input")])
		#done
		subprocess.call(["touch", os.path.join(self.this_out_dir, "BICSeqDone.txt")])
	def output(self):
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "BICSeqDone.txt"))

# #Returns the interval files 
# class BICSeqToTHetA(luigi.Task):
# 	prefix = luigi.Parameter()
# 	out_dir = luigi.Parameter()
# 	download_dir = luigi.Parameter()
# 	this_out_dir = ""
# 	def requires(self):
# 		theta_dir = os.path.join(self.out_dir, "THetA")
# 		self.this_out_dir = os.path.join(theta_dir, "THetA_input")
# 		subprocess.call(["mkdir", self.this_out_dir])
# 		return BICSeq(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir)
# 	def run(self):
# 		subprocess.call(["touch", os.path.join(self.this_out_dir, "BICSeqToTHetA.txt")])
# 	def output(self):
# 		return luigi.LocalTarget(os.path.join(self.this_out_dir, "BICSeqToTHetA.txt"))

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
		#????????????????
		self.this_out_dir = os.path.join(self.out_dir, "intervalPipeline")
		subprocess.call(["mkdir", self.this_out_dir])
		subprocess.call(["touch", os.path.join(self.this_out_dir, "interval.txt")])
	def output(self):
		#????????????????
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "interval.txt"))
	# def complete(self):
	# 	return True

#############################################################################################################
#Head of the stream. Download a file from CGHub.
#############################################################################################################

class downloadGenome(luigi.Task):
	global samples
	prefix = luigi.Parameter()
	pipeline_downloads = pipeline_downloads
	download_dir = ""
	def run(self):
		self.download_dir = os.path.join(self.pipeline_downloads, self.prefix)
		normal_dir = os.path.join(self.download_dir, "NORMAL")
		tumor_dir = os.path.join(self.download_dir, "TUMOR")
		mkdir(normal_dir)
		mkdir(tumor_dir)

		#Download normal
		# subprocess.call(["./CGHub/runGeneTorrentNew.bash", samples[prefix][normal_aurid], self.download_dir])

		#Download tumor

		# subprocess.call(["./CGHub/runGeneTorrentNew.bash", samples[prefix][tumor_aurid], self.download_dir])

		subprocess.call(["touch", os.path.join(self.download_dir, self.prefix + "downloadComplete.txt")])
	def output(self):
		return luigi.LocalTarget(os.path.join(self.download_dir, self.prefix + "downloadComplete.txt"))

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


#Create tasks_to_run from the names in the file.
tasks_to_run = []
global prefix_to_id
prefix_to_id = {}

with open("seq_info.txt", "r") as seq_file:
	count = 0
	for line in URI_file:
		count += 1
		if count == 8:
			break
		line = line.strip()
		components = line.split("\t")
		#0: id, 1:prefix
		#Add to the dictionary
		try:
			prefix_to_id[components[1]] = components[0]
			tasks_to_run.append(deleteBAMFiles(components[1]))
		except:
			continue


#Make a dictionary for reference sequences
#Dictionary of prefix -> 
# {
# 	norm_dir
# 	tumor_dir
# 	ref_assem
# 	norm_aurid
# 	tumor_aurid
# }
global samples
samples = 


# print tasks_to_run

luigi.build([TriggerAll()], workers=1)