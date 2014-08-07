package prune;

import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.EOFException;
import java.io.File;
import java.util.ArrayList;

/**
 * This is an ALTERED version of ComparePolygonFiles for use within the PruneClusters program.
 * The main method now operates as a normal method to be called by PruneClusters.
 */

public class ComparePolygonFiles {

	private static String OUTPUTFILE = null;
	private static int MIN_CLUSTER_SIZE = 4;
	private static RandomAccessFile file1 = null; 
	private static RandomAccessFile file2 = null; 
	private static boolean file1IsClusters = false;
	private static boolean file2IsClusters = false;
	//private static ArrayList<String> list1 = new ArrayList<String>();
	//private static ArrayList<String> list2 = new ArrayList<String>();
	private static ArrayList<String> list1 = null;
	private static ArrayList<String> list2 = null;
	private static RandomAccessFile matchesFile1 = null; 
	private static RandomAccessFile matchesFile2 = null; 
	private static final int NUM_INTERVAL_COLS = 6;
	private static int intervalUncertainty_ = 0;
	private static String suffix_ = "";
	private static boolean verbose = false;
	private static boolean invert = false;
	private static int chrX_ = -1;
	private static int chrY_ = -1;
	private static boolean chrSpecified_ = false;

	//private static boolean saveMemory = false;
	private static ArrayList[][] list1Array; //= new ArrayList<String>[24][24];
	private static ArrayList[][] list2Array; //= new ArrayList<String>[24][24];
	public ComparePolygonFiles(){

	}

	private static void printUsage() {
		System.out.println("Usage:");
	 	System.out.println("java ComparePolygonFiles [FileToCompare1] [FileToCompare2] [MIN_CLUSTER_SIZE] [OPTIONS]"); 
		System.out.println("\nExample:\njava ComparePolygonFiles mydeletions.clusters mydata.intervals 4 --verbose");
		System.out.println("\nOutput files:\n[FileToCompare1].matches \tContains the datapoints in [FileToCompare1] that match any datapoints in [FileToCompare2]");
		System.out.println("[FileToCompare2].matches \tContains the datapoints in [FileToCompare2] that match any datapoints in [FileToCompare1]");
		System.out.println("\nOptions: ");
		System.out.println("--verbose \t Print out extra debugging info ");
		System.out.println("--invert \t Report lines that don't match rather than lines that do match. Only one output file in this case: [FileToCompare1].filtered");
		System.out.println("--intervalUncertainty [Val] \t Uncertainty inherent in any interval files. Will grow the sides of each interval rectangle by +Val and -Val (for total length increase of 2*Val bp for each side)");
		System.out.println("--suffix [suffix] \t Append an additional suffix to the output filenames (after .matches or .filtered)");
		System.out.println("--chr [chr1] [chr2] \t Filter only by specified chromosomes. So only need to look at the chromosomes in questions, should be much faster.");

		System.out.println("\nOther Notes:");
		System.out.println("- Acceptable input formats include the standard GASV clusters format as well as an interval format.");
		System.out.println("- Clusters files must have a filename containing the word 'clusters', otherwise the interval format is expected.");
		System.out.println("- Standard interval format is: ");
		System.out.println("chr1    int1_start      int1_end        chr2    int2_start      int2_end");
	 	System.out.println("- The [MIN_CLUSTER_SIZE] should be an integer and applies to any/all clusters files only");
	}
	/**
	 *
	 */
	public void compareFiles(String[] args) throws java.io.IOException {

		if (args == null || args.length < 3) {
			System.err.println("ERROR: incorrect arguments!");
			printUsage();
			return;
		} else {
			//parse any extra options:
			if (args.length > 3) {
				for (int i=3; i < args.length; ++i) {
					if (args[i].equals("--verbose")) {
						verbose = true;
					} else if (args[i].equals("--invert")) {
						//invert the results, so report lines that don't match rather than the lines that do match!
						invert = true;
					//} else if (args[i].equals("--saveMemory")) {
						//saveMemory = true;
					} else if (args[i].equals("--intervalUncertainty")) {
						++i;
						if (args.length > i) {
							try {
								intervalUncertainty_ = Integer.parseInt(args[i]);
							} catch (Exception ex) {
								System.out.println("Couldn't parse uncertainty int value: " + args[i]);
								return;
							}
						} else {
							System.out.println("Require uncertainty int argument for --intervalUncertainty option");
							return;
						}
					} else if (args[i].equals("--suffix")) {
						++i;
						if (args.length > i) {
							suffix_ = args[i];
						} else {
							System.out.println("Require suffix to be specified for --suffix option");
							return;
						}
					
					} else if (args[i].equals("--chr")) {
						++i;
						if (args.length > i) {
							chrX_ = parseChr(args[i]);
						} else {
							System.out.println("Require two chr's to be specified for --chr option");
							return;
						}
						++i;
						if (args.length > i) {
							chrY_ = parseChr(args[i]);
						} else {
							System.out.println("Require two chr's to be specified for --chr option");
							return;
						}
						chrSpecified_ = true;

					} else {
						System.err.println("Extra arg: " + args[i] + " is not recognized.");
						printUsage();
						return;
					}
				}
			}
			//if (!inputIsIntervals && !args[0].contains(".clusters")) {
				//System.out.println("ERROR: currently only comparison w/GASV cluster output supported, filename must contain '.clusters'");
				//printUsage();
				//return;
			//}
		}
		
		list1Array = new ArrayList[24][24];
		list2Array = new ArrayList[24][24];
		for (int i=0; i<24; ++i) {
			for (int j=0; j<24; ++j) {
				if (i <= j) {
					list1Array[i][j] = new ArrayList<String>();
					list2Array[i][j] = new ArrayList<String>();
				}
			}
		}

		MIN_CLUSTER_SIZE = Integer.parseInt(args[2]);
		file1IsClusters = args[0].contains("clusters");
		file2IsClusters = args[1].contains("clusters");
		file1 = new RandomAccessFile(args[0], "r");
		file2 = new RandomAccessFile(args[1], "r");
		//readFileIntoArrayList(file1, list1);
		//if (saveMemory) {
		readFileIntoArrayList(file1, list1Array, file1IsClusters);
		readFileIntoArrayList(file2, list2Array, file2IsClusters);
		//} else {
			//readFileIntoArrayList(file2, list2);
		//}
		
		String fn1 = args[0] + ".matches" + suffix_;
		String fn2 = args[1] + ".matches" + suffix_;
		if (invert) {
			fn1 = args[0] + ".filtered" + suffix_;
			fn2 = args[1] + ".filtered" + suffix_;
		}

		File newFileToDelete = new File(fn1);
		newFileToDelete.delete();
		matchesFile1 = new RandomAccessFile(fn1, "rw");

		//if inverting results, don't need second output file
		if (!invert) {
			newFileToDelete = new File(fn2);
			newFileToDelete.delete();
			matchesFile2 = new RandomAccessFile(fn2, "rw");
		}
		ArrayList<String> matchesFile1List = new ArrayList<String>();

		ArrayList<String> matchesFile2List = new ArrayList<String>();

		int lineNum = 0;
		for (int f=0; f<24; ++f) {
		for (int g=0; g<24; ++g) {
		if (f<=g) {
		list1 = list1Array[f][g];
		for (int i=0; i<list1.size(); ++i) {
			String nextLine = list1.get(i);
			//skip comments
			if (nextLine.startsWith("#")) {
				continue;
			}
			lineNum++;
			if (verbose && lineNum%1000==0) {
				System.out.println("On line " + (lineNum) + " of first file.");
				//System.out.println("On line " + i + " of the first file.");
			}
			String[] line = nextLine.split("\\s+");
			int[] chrs1 = new int[2];
			Poly p = null;
			if (file1IsClusters) { 
				p = getPolyFromCluster(line, chrs1);
			} else {
				if (line.length != NUM_INTERVAL_COLS) {
					System.err.println("ERROR: Encountered improperly formatted line of interval data in file " 
							+ fn1 + "(if this is a clusters file, file name must contain 'clusters'! ");
					System.err.println("Bad line was: \n" + nextLine);
					return;
				}

				p = getPolyFromIntervals(line, chrs1);
			}
		
			if (p == null) {
				//p probably null because cluster was -1 localization or didn't meet MIN_CLUSTER_SIZE
				// just ignore and continue
			} else {
				boolean matchFound = false;

				//list2 = list2Array[chrs1[0]-1][chrs1[1]-1];
				list2 = list2Array[f][g];
				for (int j=0; j < list2.size(); ++j) {
					String nextLine2 = list2.get(j);
					//skip comments
					if (nextLine2.startsWith("#")) {
						continue;
					}
					String[] line2 = nextLine2.split("\\s+");
					Poly p2 = null;
					int[] chrs2 = new int[2];
					if (file2IsClusters) { 
						p2 = getPolyFromCluster(line2, chrs2);
					} else {
						if (line2.length != NUM_INTERVAL_COLS) {
							System.err.println("ERROR: Encountered improperly formatted line of interval data in file " 
									+ fn2 + "(if this is a clusters file, file name must contain 'clusters'! ");
							System.err.println("Bad line was: \n" + nextLine2);
							return;
						}

						p2 = getPolyFromIntervals(line2, chrs2);
					}

					if (p2 == null) {
						//p2 probably null because cluster was -1 localization or didn't meet MIN_CLUSTER_SIZE
						// just ignore and continue
					} else {
						//make sure chromosomes match first
						if ((chrs1[0] == chrs2[0]) && (chrs1[1] == chrs2[1])) {
							//then test if they intersect in 2D
							PolyDefault res = (PolyDefault) p.intersection(p2);
							if (res.getArea()>0) {
								if (verbose) {
									//System.out.println(fn1 + " variant:\n " + nextLine + "\n matches "
											//+ fn2 + " variant:\n " + nextLine2 + "\n");
								}
								matchFound = true;
								if (!invert) {
									if (!matchesFile1List.contains(nextLine)) {
										matchesFile1List.add(nextLine);
									}
									if (!matchesFile2List.contains(nextLine2)) {
										matchesFile2List.add(nextLine2);
									}
								} else {
									//once we know there's a match, we don't have to look anymore
									//since we're only reporting non-matches in --invert mode
									break;

								}
							}
						}
					}
				}//end for loop
				if (invert && !matchFound) {
					//matchesFile1List will actually contain the non-matches!
					if (!matchesFile1List.contains(nextLine)) {
						matchesFile1List.add(nextLine);
					}
				}
			}
		}//end for loop
		}//end if f<=g
		}//end for loop
		}//end for loop
		for (int i=0; i<matchesFile1List.size(); ++i) {
			matchesFile1.writeBytes(matchesFile1List.get(i) + "\n");
		}
		if (!invert) {
			for (int i=0; i<matchesFile2List.size(); ++i) {
				matchesFile2.writeBytes(matchesFile2List.get(i) + "\n");
			}
		}
	}

	private static void readFileIntoArrayList(RandomAccessFile file, ArrayList<String> array) throws java.io.IOException {
		file.seek(0);
		String nextLine = file.readLine();
		while (nextLine != null) {
			//skip comments
			if (nextLine.startsWith("#")) {
				nextLine = file.readLine();
				continue;
			}
			array.add(nextLine);
			nextLine = file.readLine();
		}
	}

	private static void readFileIntoArrayList(RandomAccessFile file, ArrayList<String>[][] array, boolean isClusters) throws java.io.IOException {
		file.seek(0);
		String nextLine = file.readLine();
		while (nextLine != null) {
			//skip comments
			if (nextLine.startsWith("#")) {
				nextLine = file.readLine();
				continue;
			}
			String[] line = nextLine.split("\\s+");
			int chrM = -1;
			int chrN = -1;
			if (isClusters) {
				if (line[2].contains(",")) {
					chrM = parseChr(line[1]);
					chrN = parseChr(line[3]);
				} else {
					int numPES = Integer.parseInt(line[1]);
					//need to offset indices by 1 if using the new format
					// ASSUME: NEW FORMAT, for now
					int newFormatOffset = 1;
					if (line[3].contains(",") || line[3].length() > 4) { 
						newFormatOffset = 0;
					}
					chrM = parseChr(line[3+numPES+newFormatOffset]);
					chrN = parseChr(line[4+numPES+newFormatOffset]);
				}
			} else {
				chrM = parseChr(line[0]);
				chrN = parseChr(line[3]);
			}

			array[chrM-1][chrN-1].add(nextLine);
			nextLine = file.readLine();
		}
		if (verbose) {
			System.out.println("Finished reading a file into arraylist[][]");
		}
	}
	/**
	  * int[] chrs parameter will be filled in with the chromosomes parsed from the line
	  */
	private static Poly getPolyFromCluster(String[] line, int[] chrs) {
		int len = line.length;
		Poly p = null;
		String cid = line[0];
		if (line[2].contains(",")) {
			//--ouput reads or --output standard format
			int numPES = Integer.parseInt(line[5]);
			double localization = Double.parseDouble(line[6]);
			if ((numPES >= MIN_CLUSTER_SIZE) && (localization > -1)) {
				int chrM = parseChr(line[1]);
				chrs[0] = chrM;
				int chrN = parseChr(line[3]);
				chrs[1] = chrN;
				String leftBP[] = line[2].split(",");
				String rightBP[] = line[4].split(",");
				int x1 = Integer.parseInt(leftBP[0]);
				int x2 = Integer.parseInt(leftBP[1]);
				int y1 = Integer.parseInt(rightBP[0]);
				int y2 = Integer.parseInt(rightBP[1]);

				p = new PolyDefault();
				p.add(x1, y2);
				p.add(x2, y2);
				p.add(x2, y1);
				p.add(x1, y1);

			}

		} else {
			//otherwise it's old format or --output regions
			int numPES = Integer.parseInt(line[1]);
			double localization = Double.parseDouble(line[2]);
			//need to offset indices by 1 if using the new format
			int newFormatOffset = 0;
			if (line[3].equals("TR+") || line[3].equals("TR-") || line[3].equals("TN+1") || line[3].equals("TN-1") 
					|| line[3].equals("TN+2") || line[3].equals("TN-2") || line[3].equals("IR")
					|| line[3].equals("DV") || line[3].equals("D") || line[3].equals("V")
					|| line[3].equals("I+") || line[3].equals("I-") || line[3].equals("T")) {

				newFormatOffset = 1;
					}	
			String xCoord;// = line[len-2];
			String yCoord;// = line[len-1];
			// only look at cliques and at clusters of minimum size
			if ((numPES >= MIN_CLUSTER_SIZE) && (localization > -1)) {

				//skip the list of reads which come next (there are "numPES" of these)
				//	and are followed by the chromosome numbers
				int chrM = parseChr(line[3+numPES+newFormatOffset]);
				chrs[0] = chrM;
				int chrN = parseChr(line[4+numPES+newFormatOffset]);
				chrs[1] = chrN;

				p = new PolyDefault();
				long minX = Long.MAX_VALUE;
				long maxX = 0;
				long minY = Long.MAX_VALUE;
				long maxY = 0;
				//put the list of coordinates into a list of points
				for (int i=(5+numPES+newFormatOffset); i<len; i+=2) {
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
						p.add(x, y);
					} catch (Exception ex) {
						System.out.println("Caught Exception: " + ex + " for cluster: "
								+ line[0] + " ...probably incomplete, but just continue");
						break;
					}
				}
			}
		}
		return p;
	}

	/**
	  * int[] chrs parameter will be filled in with the chromosomes parsed from the line
	  */
	private static Poly getPolyFromIntervals(String[] line, int[] chrs) {

		//System.out.println("chr1    int1_start      int1_end        chr2    int2_start      int2_end");
		//otherwise handle case where input file is just intervals
		int chr1 = parseChr(line[0]);
		chrs[0] = chr1;
		long xStart = Long.parseLong(line[1]);
		xStart -= intervalUncertainty_;
		long xEnd = Long.parseLong(line[2]);
		xEnd += intervalUncertainty_;
		int chr2 = parseChr(line[3]);
		chrs[1] = chr2;
		long yStart = Long.parseLong(line[4]);
		long yEnd = Long.parseLong(line[5]);
		yStart -= intervalUncertainty_;
		yEnd += intervalUncertainty_;

		Poly p = new PolyDefault();
		p.add(xStart, yEnd);
		p.add(xEnd, yEnd);
		p.add(xEnd, yStart);
		p.add(xStart, yStart);
		return p;
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
