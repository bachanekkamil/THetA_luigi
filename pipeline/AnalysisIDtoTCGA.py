#!/usr/bin/env python

#==============================================================================
#title :	AnalysisIDtoTCGA.py
#author: 	Gryte Satas									
#date:		2013-12-17						
#usage: AnalysisIDtoTCGA.py AnalysisIDFile XMLFile
#args: 
#		AnalysisIDFile: List of analysis IDs
#		XMLFile: Metadata XML file from cgquery
#description: Queries an XML file for a given analysis ID and returns the 
#				matching TCGA barcode
#==============================================================================

import sys
import xml.etree.cElementTree as et

queryfile = sys.argv[1]
xmlfile = sys.argv[2]

with open(queryfile) as f:
	queries = f.readlines()
	queries = [q.strip() for q in queries]

with open(xmlfile) as f:
	fullXML = f.read()

tree = et.fromstring(fullXML)
anIDtoTCGA = {}

for el in tree.findall('Result'):
	for ch in el.getchildren():
		if ch.tag == "analysis_id": anID = ch.text
		elif ch.tag == "legacy_sample_id": tcgaID = ch.text
	anIDtoTCGA[anID] = tcgaID

for query in queries:
	print query, anIDtoTCGA[query]


