"""
David Liu, 7/3/14
"""


with open("WGS_results.txt", "r+") as info:
	with open("analysis_URIS.txt", "w") as uris:
		for line in info:
			if "analysis_data_uri" in line:
				index = line.index('download')
				try:
					uri = line[index+9:].strip()
					uris.write(uri + "\t")
				except:
					print "download string not found."
			if "filename" in line and ".bam.bai" not in line:
				index = line.index(":")
				stopIndex = line.index(".bam")
				try:
					name = line[index + 2: stopIndex]
					uris.write(name + "\n")
				except:
					print "error getting filename"