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
		self.this_out_dir = os.path.join(self.out_dir, "THetA")		
		subprocess.call(["mkdir", self.this_out_dir])
		return {'BICSeqToTheta': BICSeqToTHetA(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir), 
				'intervalCountingPipeline': intervalCountingPipeline(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir)}
	def run(self):
		#run theta
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
		self.this_out_dir = os.path.join(self.out_dir, "GASV")
		subprocess.call(["mkdir", self.this_out_dir])		
		return downloadGenome(self.prefix)
	def run(self):
		#Run BAMtoGASV
		# subprocess.call(["./pipeline/scripts/runGASVNEW.sh", os.path.join(out, )])
		subprocess.call(["touch", os.path.join(self.this_out_dir, "BAMtoGASV.txt")])		
	def output(self):
		# return luigi.LocalTarget("path/to/output/stuffz")
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "BAMtoGASV.txt"))

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
		subprocess.call(["touch", os.path.join(self.this_out_dir, "BICSeq.txt")])
	def output(self):
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "BICSeq.txt"))

#Returns the interval files 
class BICSeqToTHetA(luigi.Task):
	prefix = luigi.Parameter()
	out_dir = luigi.Parameter()
	download_dir = luigi.Parameter()
	this_out_dir = ""
	def requires(self):
		theta_dir = os.path.join(self.out_dir, "THetA")
		self.this_out_dir = os.path.join(theta_dir, "THetA_input")
		subprocess.call(["mkdir", self.this_out_dir])
		return BICSeq(prefix = self.prefix, out_dir = self.out_dir, download_dir = self.download_dir)
	def run(self):
		subprocess.call(["touch", os.path.join(self.this_out_dir, "BICSeqToTHetA.txt")])
	def output(self):
		return luigi.LocalTarget(os.path.join(self.this_out_dir, "BICSeqToTHetA.txt"))

"""
The C++ code that counts the 50kb bins.
"""
class intervalCountingPipeline(luigi.Task):
	prefix = luigi.Parameter()
	out_dir = luigi.Parameter()
	download_dir = luigi.Parameter()	
	this_out_dir = ""
	def requires(self):
		return downloadGenome(self.prefix)
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
	global prefix_to_id
	prefix = luigi.Parameter()
	pipeline_downloads = pipeline_downloads
	download_dir = ""
	def run(self):
		global prefix_to_id
		analysis_uri_id = prefix_to_id[self.prefix]
		self.download_dir = os.path.join(self.pipeline_downloads, self.prefix)
		# subprocess.call(["./CGHub/runGeneTorrentNew.bash", analysis_uri_id, self.download_dir])
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

with open("analysis_URIS.txt", "r") as URI_file:
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

# print tasks_to_run

luigi.build([TriggerAll()], workers=2)