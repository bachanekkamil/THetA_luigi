#!/bin/python
import csv
import argparse
"""
Filters out non-heterozygous SNPs. BAF outside of the lower and upper bound are filtered out.
"""

def parse_arguments(silent = False):
	parser = argparse.ArgumentParser()
	parser.add_argument("SNP_FILE")
	parser.add_argument("--LOWER_BOUND", required=False, default = 0.2)
	parser.add_argument("--UPPER_BOUND", required=False, default = 0.8)

	args = parser.parse_args()

	snp_file = args.SNP_FILE
	lower_bound = args.LOWER_BOUND
	upper_bound = args.UPPER_BOUND

	return snp_file, lower_bound, upper_bound

def main():
	snp_file, lower_bound, upper_bound = parse_arguments()
	parts = snp_file.strip().split()
	prefix = parts[0]
	with open(snp_file, 'rb') as fin, open(prefix + ".heterozygous_BAFs.tsv", 'wb') as fout:
		freader = csv.reader(fin, delimiter = '\t')
		fwriter = csv.writer(fout, delimiter = '\t')
		#Skip the header
		next(freader)
		for line in freader:
			BAF = float(line[12])
			if BAF > lower_bound and BAF < upper_bound:
				fwriter.writerow(line)
			else:
				continue

if __name__ == "__main__":
	main()