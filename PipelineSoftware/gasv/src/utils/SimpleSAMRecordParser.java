/**
 * Copyright 2010,2012 Benjamin Raphael, Suzanne Sindi, Hsin-Ta Wu, Anna Ritz, Luke Peng, Layla Oesper
 *
 *  This file is part of GASV.
 * 
 *  gasv is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  GASV is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with gasv.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import net.sf.samtools.SAMFileReader;
import net.sf.samtools.SAMRecord;

import java.util.Iterator;
import net.sf.samtools.SAMRecord.SAMTagAndValue;

public class SimpleSAMRecordParser {
	public static void main(String[] args) throws IOException {
		
		if(args.length != 2) {
			printUsage();
			return;
		}
		String bamfile = args[0];
		String outfile = args[1];
		
		int count = 0;
		BufferedWriter writer = new BufferedWriter(new FileWriter(outfile));
		SAMFileReader in = new SAMFileReader(new File(bamfile));
		
		writer.write("Name\tSAMFlag\tChr\tStart\tEnd\tNegativeStrand?\tQuality\tCIGAR\tNM\n");
		for (SAMRecord s : in) {
			/*
			Iterator<SAMTagAndValue> iter = s.getAttributes().iterator();
			while(iter.hasNext()) {
				SAMTagAndValue val = iter.next();
				System.out.println(val.tag+" " +val.value);
			}
			*/
		
			writer.write(s.getReadName()+"\t"+s.getFlags()+"\t"+s.getReferenceName()+"\t"+
					s.getAlignmentStart()+"\t"+s.getAlignmentEnd()+"\t"+
					s.getReadNegativeStrandFlag()+"\t"+s.getMappingQuality()+"\t"+
					s.getCigarString()+"\t"+s.getAttribute("NM")+"\n");
			if(count % 500000 == 0) 
				System.out.println("  record "+count+"...");
			count++;
		}
		in.close();
		writer.close();
	}
	
	public static void printUsage() {
		System.out.println("USAGE: java -jar SimpleSAMRecordParser.jar <input_bam_file> <output_file>");
		System.out.println("\t<input_bam_file> is the input file");
		System.out.println("\t<output_file> is a tab-delimited file with the following columns:");
		System.out.println("\t\t<name>\tRecord name");
		System.out.println("\t\t<flag>\tSAM Flag");
		System.out.println("\t\t<chr>\tChomosome (or reference) name");
		System.out.println("\t\t<start>\talignment start (softclipped, according to Picard)");
		System.out.println("\t\t<end>\talignment end (softclipped, according to Picard)");
		System.out.println("\t\t<neg>\ttrue if the alignment is on the negative strand, false otherwise");
		System.out.println("\t\t<qual>\tmapping quality");
		System.out.println("\t\t<cigar>\tCIGAR string");
		System.out.println("\t\t<NM>\tEdit distance to the reference (excluding clipping)");
		
	}
}
