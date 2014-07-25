	# Name	 : identifyPairs.py
	# Author	 : Gryte Satas
	# Date	 : September 12, 2013
	# Usage	 : python identifyPairs.py cgQueryOuput.txt
	# Description: Takes in the raw output from cgquery (in -a mode) and 
	#			identifies pairs of entries that have the same id. 
	#			Outputs list of IDs into pairedIDs.txt
	
	import sys
	
	def getInfo(entry, start, lines, offsets):
		"""
		Quick way of getting a specific line from a result starting at line
		start. For entry, accepts everything in dict offsets
		"""
		offset = offsets[entry]
		return lines[start + offset].split(": ")[1].strip()
	
	def create_offsets(linenums, lines):
		"""
		Reads the first entry, and calculates offsets based on this
		"""
	
		offsets = {}
		for i in range(linenums[0], linenums[1]):
			code = lines[i].split(": ")[0].strip()
			if len(lines[i].split(": ")) == 2: 
			 	if code in offsets: code = code+"2"
			 	offsets[code] = i
		return offsets
	
	def find_pairs(linenums, lines, offsets, output):
		"""
		Writes out a list of IDs that have a tumor/normal pair
		"""
	
		# Iterate through entries and sort them according to sample type
		# into separate lists
		entries_T = []
		entries_N = []
		
		for val in linenums:
			sample_type = getInfo("sample_type", val, lines, offsets)
			sample_id 	= getInfo("legacy_sample_id", val, lines, offsets)
			sample_id = "-".join(sample_id.split("-")[:3])
			
			if sample_id is '': continue
	
			if int(sample_type) < 10:
				  entries_T.append(sample_id)
			elif int(sample_type) >= 10:
				  entries_N.append(sample_id)
	
		# Identify IDs that exist in both lists
		paired_entries = set()
		for entry in entries_T:
			if entry in entries_N:
				paired_entries.add(entry)
	
		f = open(output, 'w')
		for line in paired_entries:
			  line += "\n"
			  f.write(line)
		f.close()
		print "Number of pairs:", len(paired_entries)
	
	def filter_by(entry, mode, value, linenums, lines, offsets):
		"""
		Iterates through all the entries in linenums, and filters out ones
		that don't meet the given criteria.
	
		Note: All numerical values are treated as ints
	
		modes: 
			-"eq"  : == (for strings)
			-"neq" : != (for strings)
			-"eqN" : == (for numerical values)
			-"lt"  : < (value must be numerical)
			-"gt"  : > (value must be numerical)
		"""
		if mode not in ["eq", "lt", "gt"]: raise ValueError
		if entry not in offsets: raise ValueError
	
		linenumsNew = []8693056
		for val in linenums:
			if mode == "eq": 
				if getInfo(entry, val, lines, offsets) == value:
						linenumsNew.append(val)
			if mode == "neq": 
				if getInfo(entry, val, lines, offsets) != value:
					linenumsNew.append(val)
			if mode == "eqN": 
				if int(getInfo(entry, val, lines, offsets)) == value:
					linenumsNew.append(val)
			if mode == "lt": 
				if int(getInfo(entry, val, lines, offsets)) <= value:
					linenumsNew.append(val)
			if mode == "gt": 
				if int(getInfo(entry, val, lines, offsets)) > value:
					linenumsNew.append(val)
		return linenumsNew
	
	def print_entry(val, lines):
		  print "".join(lines[val:val+30])
		
	import pylab as P
	def visualize_sizes(entry, linenums, lines, offsets):
		listSizes = []
		for val in linenums:
		    size = int(getInfo(entry, val, lines, offsets))
		    listSizes.append(size)
		
		listSizes = [val for val in listSizes if val > 0]
		listSizes.sort()
		listSizes = [val * 1.0/1E9 for val in listSizes]
		
		P.hist(listSizes, 100)
		P.savefig("hist.pdf")
		P.boxplot(listSizes)
		P.savefig("boxplot.pdf")
	
	
	def main():
		filename = sys.argv[1]
		if len(sys.argv) > 2: output = sys.argv[2]
		else: output = "pairedIDs.txt"
	
		#Read in cgquery file
		f = open(filename)
		lines=f.readlines()
		f.close()
		lines = lines[11:]
		
		# Line number of the start of every result
		linenums = [i for i in range(len(lines)) if "Result" in lines[i]]
		offsets = create_offsets(linenums, lines)
	
		# Filter results
		linenums = filter_by('filesize', 'gt', 60E9, linenums, lines, offsets)
		#linenums = filter_by('center_name', 'eq', "WUGSC", linenums, lines, offsets)
	
		# Match pairs and output to file
		find_pairs(linenums, lines, offsets, output)
	
	if __name__ == '__main__':
		main()
