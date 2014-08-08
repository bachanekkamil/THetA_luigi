import sys
import json
import re
import os

"""
David Liu
7/30/14
"""

###
#Pre-process cgquery output into JSON.
###

def get_info(line):
	line = line.strip().split(":")
	return line[1].strip()

def get_url(line):
	line = line.strip().split(":")
	return line[1].strip() + ":" + line[2].strip()

samples = {}

with open("seq.txt", "r") as seq_info:
	for line in seq_info:
		if "legacy_sample_id" in line:
			current_sample = get_info(line)
			samples[current_sample] = {}
			continue
		elif "disease_abbr" in line:
			disease_abbr = get_info(line)
			samples[current_sample]["name"] = disease_abbr + "-" + current_sample
		elif "analysis_data_uri" in line:
			aurid = get_url(line)
			samples[current_sample]["aurid"] = aurid
		elif "refassem" in line:
			ref_assem = get_info(line)
			samples[current_sample]["ref_assem"] = ref_assem
		elif "sample_type" in line:
			code = int(re.sub("[^0-9]", "", get_info(line)))
			if code < 10:
				sample_type = "TUMOR"
			elif code >=10:
				sample_type = "NORMAL"
			samples[current_sample]["sample_type"] = sample_type
		else:
			continue

###
#Pair samples into pairs.
###

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

output = {}

def prefixize(name):
	parts = name.split("-")
	return parts[0] + "-" + parts[1] + "-" + parts[2]

#Split into normal and tumor
tumors = []
normal = []
for key, info in samples.iteritems():
	if info["sample_type"] == "TUMOR":
		tumors.append({prefixize(key):info})
	else:
		normal.append({prefixize(key):info})

def find_normal_match(prefix):
	for normal_item in normal:
		if normal_item.keys()[0] == prefix:
			return normal_item

def parse_refassem(s):
	s = int(re.sub("[^0-9]", "", s))
	if s == 19 or s == 37:
		return "hg19"
	elif s == 18 or s == 36:
		return "hg18"
	else:
		raise Exception("Unsupported assembly")


#Combine
for tumor in tumors:
	try:
		prefix = tumor.keys()[0]
		tumor_item = tumor.values()[0]
		normal_item = find_normal_match(prefix).values()[0]
		output[prefix] = {
			'norm_prefix' : normal_item['name'],
			'tumor_prefix': tumor_item['name'],
			'ref_assem' : parse_refassem(tumor_item['ref_assem']),
			'norm_aurid' : normal_item['aurid'],
			'tumor_aurid' : tumor_item['aurid'],
			# 'norm_download_dir': "",
			# 'tumor_download_dir': ""
		}
	except:
		#There was not a match between the tumor and the normal samples.
		pass

open(os.path.abspath("../tumor_sample_info.json"), "w").write(json.dumps(output, indent = 3))