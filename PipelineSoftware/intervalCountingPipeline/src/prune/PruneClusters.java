package prune;

import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.EOFException;
import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

public class PruneClusters {

	public static long minIntervalSize = 8000;
	public static long maxVariantSize = 20000000;
	public static int minClusterSize = 3;
	public static long cytoBuffer = 1000000;
	public static String cytoBandsFile = null;
	public static String intervalsFile = null;
	public static boolean removeXY = false;
	public static int intervalUncertainty = 0;
	public static boolean makeLoops = false;
	public static long maxLoopSize = 8000;
	public static boolean verbose = false;
	public static String clustersFile1 = null;
	public static String clustersFile2 = null;
	public static RandomAccessFile onlyClustersFile = null;
	public static String outputFileName = null;
	public static RandomAccessFile outputFile = null;
	public static boolean removeSmallInversions = false;
	
	
	public static int count = 0;
	public static int locCount = 0;
	public static int supCount = 0;
	public static int cytoCount = 0;
	public static int interCount = 0;
	public static int smallCount = 0;
	public static int largeCount = 0;
	public static int loopCount = 0;
	public static int invCount = 0;
	public static int sexCount = 0;
	public static int keepCount = 0;
	
	public PruneClusters() {
	}
	
	private static void printUsage() {
		System.out.println("Usage1: apply filters to a single set of clusters");
		System.out.println("\tjava PruneClusters <ClustersFile> <Options>");
		System.out.println("Usage2: apply filters to a pair of sets of clusters, and filter the first file by the second file");
		System.out.println("\tjava PruneClusters <TumorClustersFile> <NormalClustersFile> <Options>");
		System.out.println("Options:");
		System.out.println("--minLength int : minimum length allowed between breakpoints (default 8000)");
		System.out.println("--maxLength int : maximum length allowed between breakpoints (default 20000000)");
		System.out.println("--minClusterSize int : minimum cluster support requirement (default 3)");
		System.out.println("--cytoBuffer int : buffer size around centromere and telomere (default 1000000)");
		System.out.println("--cytoBandsFile /.../file.txt : full path of file used to determine locations of centromeres and telomeres. Filter will not be applied if file is not provided.");
		System.out.println("--intervalsFile /.../file.txt : full path of file of intervals. If given, clusters in these intervals will be removed. Format should be:");
		System.out.println("<id>\t<chr>\t<leftPos>\t<rightPos>");
		System.out.println("--removeXY : select whether to remove clusters located on sex chrms");
		System.out.println("--intervalUncertainty int : amount of uncertainty in the comparison of a pair of clusters files, in bp. Only used when a pair of clusters files is specified. (default 0)");
		System.out.println("--maxLoopSize int : maximum length for an interval to be designated a self-loop (default 8000)");
		System.out.println("--removeSmallInversions : remove small inversions in which the two breakpoint ranges overlap");
		System.out.println("--makeLoops : make self-loops out of small non-reciprocal inversions");
		System.out.println("--PREGO : sets all options to be appropriate for PREGO and related pipelines");
		System.out.println("--GASVPro : sets all options to be appropriate for GASVPro and related pipelines");
		System.out.println("--verbose : prints progress/debugging info to terminal");
		
	}
	
	private static void printStats() {
		System.out.println("In the first "+count+" clusters:");
		if (locCount > 0) {
			System.out.println(locCount+" removed for a localization of -1");
		}
		if (supCount > 0) {
			System.out.println(supCount+" removed for too little support");
		}
		if (interCount > 0) {
			System.out.println(interCount+" removed for being on the provided intervals");
		}
		if (cytoCount > 0) {
			System.out.println(cytoCount+" removed for being on the centro/telomeres");
		}
		if (sexCount > 0) {
			System.out.println(sexCount+" removed for being on X/Y chromosomes");
		}
		if (invCount > 0) {
			System.out.println(invCount+" removed as small inversions");
		}
		if (smallCount > 0) {
			System.out.println(smallCount+" removed for being too small");
		}
		if (largeCount > 0) {
			System.out.println(largeCount+" removed for being too large");
		}
		if (loopCount > 0) {
			System.out.println(loopCount+" turned into self-loops");
		}
		if (keepCount > 0) {
			System.out.println(keepCount+" are kept");
		}
		System.out.println();
	}
	
	
	public static void main(String[] args) throws java.io.IOException {
		
		//parsing flags
		if (args == null) {
			System.out.println("Error: must have at least one argument.");
			printUsage();
			return;
		} else {
			for (int i = 0; i < args.length; i++) {
				if (args[i].equalsIgnoreCase("--minLength")) {
					i++;
					if (i < args.length) {
						try {
							minIntervalSize = Long.parseLong(args[i]);
						} catch (Exception e) {
							System.out.println("Could not parse --minLength " + args[i]);
							return;
						} 
					} else {
						System.out.println("Require int argument for --minLength option");
						return;
					
					}
				} else if (args[i].equalsIgnoreCase("--maxLength")) {
					i++;
					if (i < args.length) {
						try {
							maxVariantSize = Long.parseLong(args[i]);
						} catch (Exception e) {
							System.out.println("Could not parse --maxLength " + args[i]);
							return;
						}
					} else {
						System.out.println("Require int argument for --maxLength option");
						return;
					
					}
				} else if (args[i].equalsIgnoreCase("--minClusterSize")) {
					i++;
					if (i < args.length) {
						try {
							minClusterSize = Integer.parseInt(args[i]);
						} catch (Exception e) {
							System.out.println("Could not parse --minClusterSize " + args[i]);
							return;
						}
					} else {
						System.out.println("Require int argument for --minClusterSize option");
						return;
						
					}
				} else if (args[i].equalsIgnoreCase("--cytoBuffer")) {
					i++;
					if (i < args.length) {
						try {
							cytoBuffer = Long.parseLong(args[i]);
						} catch (Exception e) {
							System.out.println("Could not parse --cytoBuffer " + args[i]);
							return;
						}
					} else {
						System.out.println("Require int argument for --cytoBuffer option");
						return;
					
					}
				} else if (args[i].equalsIgnoreCase("--maxLoopSize")) {
					i++;
					if (i < args.length) {
						try {
							maxLoopSize = Long.parseLong(args[i]);
						} catch (Exception e) {
							System.out.println("Could not parse --maxLoopSize " + args[i]);
							return;
						}
					} else {
						System.out.println("Require int argument for --maxLoopSize option");
						return;
					
					}
				} else if (args[i].equalsIgnoreCase("--cytoBandsFile")) {
					i++;
					if (i < args.length) {
						if (args[i].contains("txt")) {
							cytoBandsFile = args[i];
						} else {
							System.out.println("Require a .txt file for --cytoBandsFile argument");
							return;
						}
					} else {
						System.out.println("Require string argument for --cytoBandsFile option");
						return;
					}
				} else if (args[i].equalsIgnoreCase("--intervalsFile")) {
					i++;
					if (i < args.length) {
						if (args[i].contains("txt")) {
							intervalsFile = args[i];
						} else {
							System.out.println("Require a .txt file for --removeIntervalsFile argument");
							return;
						}
					} else {
						System.out.println("Require string argument for --removeIntervalsFile option");
						return;
					}
				} else if (args[i].equalsIgnoreCase("--removeXY")) {
					removeXY = true;
					
				} else if (args[i].equalsIgnoreCase("--intervalUncertainty")) {
					i++;
					if (i < args.length) {
						try {
							intervalUncertainty = Integer.parseInt(args[i]);
						} catch (Exception e) {
							System.out.println("Could not parse --intervalUncertainty " + args[i]);
							return;
						}
					} else {
						System.out.println("Require int argument for --intervalUncertainty option");
						return;
					
					}
				} else if (args[i].equalsIgnoreCase("--verbose")) {
					
					verbose = true;
					
				} else if (args[i].equalsIgnoreCase("--removeSmallInversions")) {
					removeSmallInversions = true;
					
				} else if (args[i].equalsIgnoreCase("--makeLoops")) {
					makeLoops = true;
					
				} else if (args[i].equalsIgnoreCase("--PREGO")) {
					removeSmallInversions = false;
					makeLoops = true;
					
					
				} else if (args[i].equalsIgnoreCase("--GASVPro")) {
					removeSmallInversions = true;
					makeLoops = false;
					
					
				} else if (args[i].contains("clusters")) {
					if (clustersFile1 == null) {
						clustersFile1 = args[i];
						outputFileName = clustersFile1 + ".final";
					} else if (clustersFile2 == null) {
						clustersFile2 = args[i];
					} else {
						System.out.println("You have more than two .clusters files. Try again.");
						printUsage();
						return;
					}
				} else {
					System.out.println("Invalid argument: " + args[i]);
					printUsage();
					return;
				}
			}
		}
	
				
		if (clustersFile1 == null) {
			
			System.out.println("Error: You must have at least one valid .clusters file as an argument.");
			printUsage();
			return;
		}
		
		//only do this if there are two .clusters files to compare
		if (clustersFile1 != null && clustersFile2 != null) {
			ComparePolygonFiles cpf = new ComparePolygonFiles();
			if (verbose) {
				System.out.println("Comparing first .clusters file by second .clusters file:");
				String[] compareCall = {clustersFile1,clustersFile2,Integer.toString(minClusterSize),"--intervalUncertainty",Integer.toString(intervalUncertainty),"--invert","--verbose"};
				cpf.compareFiles(compareCall);
			} else {
				String[] compareCall = {clustersFile1,clustersFile2,Integer.toString(minClusterSize),"--intervalUncertainty",Integer.toString(intervalUncertainty),"--invert"};
				cpf.compareFiles(compareCall);
			}
			
			
			onlyClustersFile = new RandomAccessFile(clustersFile1 + ".filtered", "r");
		} else {
			onlyClustersFile = new RandomAccessFile(clustersFile1, "r");
		}
		
		File deleteFile = new File(outputFileName);
		deleteFile.delete();
		outputFile = new RandomAccessFile(outputFileName, "rw");
		
		//reading cytoBandsFile
		long[] chrEnds = new long[24];
		long[] chrMids = new long[24];
		HashMap<Integer, ArrayList<Long>> chrToLeftCentroPos = new HashMap<Integer, ArrayList<Long>>(); 
		HashMap<Integer, ArrayList<Long>> chrToRightCentroPos = new HashMap<Integer, ArrayList<Long>>(); 
		for (int i =1; i< 25; ++i) {
			chrToLeftCentroPos.put(new Integer(i), new ArrayList<Long>());
			chrToRightCentroPos.put(new Integer(i), new ArrayList<Long>());
		}
		if (cytoBandsFile != null) {
			try {
				RandomAccessFile cytoFile = new RandomAccessFile(cytoBandsFile, "r");
				String cytoLine = cytoFile.readLine();
				while (cytoLine != null) {
					
					//Skip if it is a comment
					if (cytoLine.startsWith("#"))
					{
						cytoLine = cytoFile.readLine();
						continue;
					}
					String[] line = cytoLine.split("\\s+");
					int chr = parseChr(line[0].substring(3));
					long leftPos = Long.parseLong(line[1]);
					long rightPos = Long.parseLong(line[2]);
					if (leftPos == 0) {
						//already know to ignore first 1MB of each chr

					} else if (line[4].equals("acen")) {

						chrToLeftCentroPos.get(new Integer(chr)).add(new Long(leftPos - cytoBuffer)); 
						chrToRightCentroPos.get(new Integer(chr)).add(new Long(rightPos + cytoBuffer));
						chrMids[chr-1] = leftPos;
					} else {
						//this is the end of the chr
						chrEnds[chr-1] = rightPos;
					}
					cytoLine = cytoFile.readLine();
				}
			} catch (Exception e) {
				System.out.println("Error while reading cytoBandsFile");
				e.printStackTrace();
				return;
			}
		}
		
		
		//reading intervals file
		HashMap<Integer, ArrayList<Long>> chrToLeftIntPos = new HashMap<Integer, ArrayList<Long>>(); 
		HashMap<Integer, ArrayList<Long>> chrToRightIntPos = new HashMap<Integer, ArrayList<Long>>(); 
		for (int i =1; i< 25; ++i) {
			chrToLeftIntPos.put(new Integer(i), new ArrayList<Long>());
			chrToRightIntPos.put(new Integer(i), new ArrayList<Long>());
		}
		if (intervalsFile != null) {
			try {
				RandomAccessFile intFile = new RandomAccessFile(intervalsFile, "r");
				String intLine = intFile.readLine();
				while (intLine != null) {
					String[] intLineSplit = intLine.split("\\s+");
					chrToLeftIntPos.get(Integer.parseInt(intLineSplit[1])).add(Long.parseLong(intLineSplit[2]));
					chrToRightIntPos.get(Integer.parseInt(intLineSplit[1])).add(Long.parseLong(intLineSplit[3]));
				}
			} catch (Exception e) {
				System.out.println("Error while reading intervalsFile");
				e.printStackTrace();
				return;
			}
		}
		
		
		
		
		
		//Looking at clusters starts here!!!
		onlyClustersFile.seek(0);
		String nextLine = onlyClustersFile.readLine();
		
		
		while (nextLine != null) {
			if (count % 10000 == 0 && verbose) {
				printStats();
			}
			if (nextLine.startsWith("#")) {
				outputFile.writeBytes(nextLine + "\n");
				nextLine = onlyClustersFile.readLine();
				continue;
			}
			
			String[] line = nextLine.split("\\s+");
			int chrM = -1;
			int chrN = -1;
			boolean printLine = true;
			boolean isLoop = false;
			count++;
			//assume regions format
			int numPES = Integer.parseInt(line[1]);
			double localization = Double.parseDouble(line[2]);
			String varType = line[3];
			
			String xCoord;// = line[len-2];
			String yCoord;// = line[len-1];
			long minX = Long.MAX_VALUE;
			long maxX = 0;
			long minY = Long.MAX_VALUE;
			long maxY = 0;
			
			// only look at cliques and at clusters of minimum size
			if (localization == -1) {
				printLine = false;
// 				if (verbose) {
// 					System.out.println("Cluster "+line[0]+" removed because of -1 localization.");
// 				}
				locCount++;
				nextLine = onlyClustersFile.readLine();
				continue;
			}
			if (numPES < minClusterSize) {
				printLine = false;
// 				if (verbose) {
// 					System.out.println("Cluster "+line[0]+" removed because of innsufficient support.");
// 				}
				supCount++;
				nextLine = onlyClustersFile.readLine();
				continue;
			}

			//skip the list of reads which come next (there are "numPES" of these)
			//	and are followed by the chromosome numbers
			chrM = parseChr(line[4+numPES]);
			chrN = parseChr(line[5+numPES]);

			
			//put the list of coordinates into a list of points
			int len = line.length;
			for (int i=(6+numPES); i<len; i+=2) {
				xCoord = line[i];
				yCoord = line[i+1];
				xCoord = xCoord.substring(0, xCoord.length()-1);
				if (i < (len -2)) {
					yCoord = yCoord.substring(0, yCoord.length()-1);
				}
				try { 
					long x = Long.parseLong(xCoord); 
					long y = Long.parseLong(yCoord);
					if (x > maxX) {
						maxX = x;
					}
					if (y > maxY) {
						maxY = y;
					}
					if (x < minX) {
						minX = x;
					}
					if (y < minY) {
						minY = y;
					}
				} catch (Exception ex) {
					System.out.println("Caught Exception: " + ex + " for cluster: "
							+ line[0] + " ...probably incomplete, but just continue");
					break;
				}
			}
			
				
			//applying filters before writing clusters to .final files
			
			
			//this is the centromere/telomere filter
			ArrayList<Long> leftIntervals = chrToLeftCentroPos.get(chrM);
			ArrayList<Long> rightIntervals = chrToRightCentroPos.get(chrM);
			
			for (int i = 0; i < leftIntervals.size(); i++) {
				if (chrM == chrN) {
					if ((leftIntervals.get(i) < minX && rightIntervals.get(i) > minX) ||
						(leftIntervals.get(i) < maxY && rightIntervals.get(i) > maxY)) {
						
						printLine = false;
					}
				} else {
					if ((leftIntervals.get(i) < minX && rightIntervals.get(i) > minX) ||
						(leftIntervals.get(i) < maxX && rightIntervals.get(i) > maxX)) {
						printLine = false;
					}
				}
			}
			
			leftIntervals = chrToLeftCentroPos.get(chrN);
			rightIntervals = chrToRightCentroPos.get(chrN);
			
			for (int i = 0; i < leftIntervals.size(); i++) {
				if (chrM == chrN) {
					if ((leftIntervals.get(i) < minX && rightIntervals.get(i) > minX) ||
						(leftIntervals.get(i) < maxY && rightIntervals.get(i) > maxY)) {
						
						printLine = false;
					}
				} else {
					if ((leftIntervals.get(i) < minY && rightIntervals.get(i) > minY) ||
						(leftIntervals.get(i) < maxY && rightIntervals.get(i) > maxY)) {
						printLine = false;
					}
				}
			}
			
			if (!printLine) {
// 				if (verbose) {
// 					System.out.println("Cluster "+line[0]+" removed because it was located in the centromere.");
// 				}
				nextLine = onlyClustersFile.readLine();
				cytoCount++;
				continue;
			}
					
			
			if (cytoBandsFile != null && (minX < cytoBuffer || minY < cytoBuffer || 
				chrEnds[chrN - 1] - cytoBuffer < maxY || chrEnds[chrM - 1] - cytoBuffer < maxX ||
				(chrM == chrN && minX < chrMids[chrM - 1] && maxY > chrMids[chrM - 1]))) {
// 				if (verbose) {
// 					System.out.println("Cluster "+line[0]+" removed because it was located in the telomeres or centromere.");
// 				}
				nextLine = onlyClustersFile.readLine();
				cytoCount++;
				continue;
			}
			
			
			//this is the intervals file filter
			leftIntervals = chrToLeftIntPos.get(chrM);
			rightIntervals = chrToRightIntPos.get(chrM);
			
			for (int i = 0; i < leftIntervals.size(); i++) {
				if (chrM == chrN) {
					if ((leftIntervals.get(i) < minX && rightIntervals.get(i) > minX) ||
						(leftIntervals.get(i) < maxY && rightIntervals.get(i) > maxY)) {
						
						printLine = false;
					}
				} else {
					if ((leftIntervals.get(i) < minX && rightIntervals.get(i) > minX) ||
						(leftIntervals.get(i) < maxX && rightIntervals.get(i) > maxX)) {
						printLine = false;
					}
				}
			}
			
			leftIntervals = chrToLeftIntPos.get(chrN);
			rightIntervals = chrToRightIntPos.get(chrN);
			
			for (int i = 0; i < leftIntervals.size(); i++) {
				if (chrM == chrN) {
					if ((leftIntervals.get(i) < minX && rightIntervals.get(i) > minX) ||
						(leftIntervals.get(i) < maxY && rightIntervals.get(i) > maxY)) {
						
						printLine = false;
					}
				} else {
					if ((leftIntervals.get(i) < minY && rightIntervals.get(i) > minY) ||
						(leftIntervals.get(i) < maxY && rightIntervals.get(i) > maxY)) {
						printLine = false;
					}
				}
			}
			
			if (!printLine) {
// 				if (verbose) {
// 					System.out.println("Cluster "+line[0]+" removed because it was located in a given interval.");
// 				}
				nextLine = onlyClustersFile.readLine();
				interCount++;
				continue;
			}
			
			
			
			//making loops
			if (makeLoops && chrM == chrN && maxY - minX < maxLoopSize && (varType.equals("I+") || varType.equals("I-"))) {
				isLoop = true;
// 				if (verbose) {
// 					System.out.println("Cluster "+line[0]+" transformed into self-loop.");
// 				}
				loopCount++;
				
			}
			
			//removing small variants if it's not a loop
			if (chrM == chrN && (maxY - minX < minIntervalSize && !isLoop)) {
// 				if (verbose) {
// 					System.out.println("Cluster "+line[0]+" removed because the interval was too small.");
// 				}
				nextLine = onlyClustersFile.readLine();
				smallCount++;
				continue;
			}
			
			//removing large variants
			if (chrM == chrN && maxY - minX > maxVariantSize) {
// 				if (verbose) {
// 					System.out.println("Cluster "+line[0]+" removed because the interval was too large.");
// 				}
				nextLine = onlyClustersFile.readLine();
				largeCount++;
				continue;
			}
			
			
			//removing small inversions
			if (removeSmallInversions && maxX > minY) {
// 				if (verbose) {
// 					System.out.println("Cluster "+line[0]+" removed because it was a small inversion.");
// 				}
				nextLine = onlyClustersFile.readLine();
				invCount++;
				continue;
			}
			
			
			//removing stuff on sex chrms
			if (removeXY) {
				if (chrM == 23 || chrM == 24 || chrN == 23 || chrN == 24) {
// 					if (verbose) {
// 						System.out.println("Cluster "+line[0]+" removed because it was located on a sex chromosome.");
// 					}
					nextLine = onlyClustersFile.readLine();
					sexCount++;
					continue;
				}
			}
			
			
			//printing to the file if everything's still okay
			if (printLine) {
				if (isLoop) {
					
					//lko - 5/28/13 - Fix loop handling
					if(varType.equals("I-"))
					{
						String newLine = nextLine.replaceAll("(I\\-)", "L-");
						outputFile.writeBytes(newLine + "\n");
					}
					else
					{
						String newLine = nextLine.replaceAll("(I\\+)", "L+");
						outputFile.writeBytes(newLine + "\n");
					}
					//String newLine = nextLine.replaceAll("(I\\+|I\\-)", "L");
					//outputFile.writeBytes(newLine + "\n");
				} else {
					outputFile.writeBytes(nextLine + "\n");
				}
				keepCount++;
			}
			nextLine = onlyClustersFile.readLine();
			
		}
		printStats();
		
	}
	
	
	
	private static int parseChr(String chrStr) {
		if (chrStr.startsWith("chr") || chrStr.startsWith("Chr")) {
			chrStr = chrStr.substring(3, chrStr.length());
		}
		if (chrStr.equalsIgnoreCase("x")) {
			chrStr = "23";
		} else if (chrStr.equalsIgnoreCase("y")) {
			chrStr = "24";
		}
		return Integer.parseInt(chrStr);
	}
}