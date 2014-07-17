package prego;

/**
 * Copyright 2012 Benjamin Raphael, Layla Oesper, Anna Ritz, Ryan Drebin, Sarah Aerni
 *
 *  This file is part of PREGO.
 * 
 *  PREGO is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  PREGO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with PREGO.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Random;
import java.util.TreeMap;


public class buildIntervalsPREGO 
{
	//VARIABLES
	public String BANDS_FILE = null;
	public HashMap<Integer,Long> CHRM_ENDS = null;
	public HashMap<Integer,Integer> CHRM_EXCLUDE = null;
	public int MIN_INTERVAL = 8000;
	public String CLUSTER_FILE = null;
	public String EXCLUDE_FILE = null;
	public Long START = new Long(1);
	public String OUTPUT_PREFIX = null;
	public String TUMOR_CONCORDANT = null;
	public String NORMAL_CONCORDANT = null;
	
	public HashMap<String,Cluster> clusters;
	public HashMap<String, breakEnd[]> allEnds;
	public TreeMap<Integer, ArrayList<breakEnd>> bps;
	public Random lotto;
	
	
	
	public static void main(String[] args) 
	{
		//Step 1. Validate and store input
		buildIntervalsPREGO prego = new buildIntervalsPREGO(args);
		
		//Step 2: load in exclusion values
		prego.loadExclusion();
		
		//Step 3: load in chrm ends and intialize bp lists
		prego.loadChrmEnds();
		prego.initializeBPLists();
		
		//Step 4: load in clusters file
		prego.loadClusters();
		
		//Step 5: remove clusters that form short intervals
		prego.removeShortIntervals();
		
		//Step 6: Build all breakEnds and update clusters correctly
		prego.initializeBPLists();
		prego.buildGraph();
		
		//Step 7: Print files
		prego.printIntervals();
		prego.printVariants();
		prego.printParameters();
		
	}
	
	/**
	 * Constructor - parses arguments and sets parameters.
	 * @param args - see usage information for arguments
	 */
	public buildIntervalsPREGO(String[] args)
	{
		
		// Parse and print arguments.  If arguments didn't parse correctly, 
		//then print the usage and exit.
		boolean success = parseArguments(args);
		if (!success )
		{
			printUsage();
			System.exit(-1);
		}
		printArguments();
		
		CHRM_ENDS = new HashMap<Integer, Long>();
		CHRM_EXCLUDE = new HashMap<Integer, Integer>();
		clusters = new HashMap<String,Cluster>();
		lotto = new Random();
		allEnds = new HashMap<String,breakEnd[]>();
	}

	
	/**
	 * Parses input arguments
	 * @param args - see usage instructions.
	 * @return true if there are no errors, false otherwise.
	 */
	private boolean parseArguments(String[] args) 
	{
		//Check the number of arguments is correct (always even)
		if (args.length % 2 != 0 || args.length < 2)
		{
			System.err.println("Error! Incorrect number of arguments.");
			return false;
		}
		
		CLUSTER_FILE = args[0];
		
		// Parse Clusters file
		CLUSTER_FILE = args[0];
		
		if (!(new File(CLUSTER_FILE).exists()))
		{
			System.err.println("Error! " + CLUSTER_FILE + " does not exist.");
			return false;
		}
		
		BANDS_FILE = args[1];
		
		if (!(new File(BANDS_FILE).exists()))
		{
			System.err.println("Error! " + BANDS_FILE + " does not exist.");
			return false;
		}
		
		//Default is clusters file name
		OUTPUT_PREFIX = CLUSTER_FILE;

		
		//For each pair of arguments (flag and value) set parameters
		for (int i=2; i< args.length; i+=2)
		{
			if (args[i].equalsIgnoreCase("-MIN_INTERVAL"))
			{
				try
				{
					MIN_INTERVAL = Integer.parseInt(args[i+1]);
					if (MIN_INTERVAL <= 0)
						throw new Exception();
				}
				catch (Exception e)
				{
					System.out.println("Error! MIN_INTERVAL must be a non-negative integer.");
					return false;
				}
			}//End MIN_INTERVAL
			
			if (args[i].equalsIgnoreCase("-EXCLUDE_FILE"))
			{
				EXCLUDE_FILE = args[i+1];
				
				if (!(new File(EXCLUDE_FILE).exists()))
				{
					System.err.println("Error! " + EXCLUDE_FILE + " does not exist.");
					return false;
				}
			}//End EXCLUDE_FILE	
			
			if (args[i].equalsIgnoreCase("-OUTPUT_PREFIX"))
			{
				OUTPUT_PREFIX = args[i+1];
			}
			
			if (args[i].equalsIgnoreCase("-TUMOR_CONCORDANT"))
			{
				TUMOR_CONCORDANT = args[i+1];
			}
			
			if (args[i].equalsIgnoreCase("-NORMAL_CONCORDANT"))
			{
				NORMAL_CONCORDANT=args[i+1];
			}
		}//End of optional parameters
			
		return true;
	}//end parseArguments
	
	/**
	 * Prints the usage information.
	 */
	public void printUsage()
	{
		System.out.println("\nProgram: buildIntervalsPREGO");
		System.out.println("USAGE (src): java buildIntervalsPREGO <CLUSTERS_FILE> <BANDS_FILE> [Options]\n" +
				"USAGE (jar): java -jar buildIntervalsPREGO <CLUSTERS_FILE> <BANDS_FILE> [Options]\n" + 
				"<CLUSTERS_FILE> [String]\n" +
				"\t A clusters file output by GASV.\n" + 
				"<BANDS_FILE> [String]\n" +
				"\t A cytogenetic bands file.\n" + 
				"\nOptional Parameters: \n" +
				"-EXCLUDE_FILE [STRING] (Default: none)\n" +
				"\t A file that includes a list of chromosomes to ignore in the output. \n" +
				"-MIN_INTERVAL [Integer] (Default: 8000)\n" +
				"\t A minimum size that an interval can be.\n" +
				"-OUTPUT_PREFIX [STRING] (Default: CLUSTERS_FILE)\n" +
				"\t Prefix for all output files.\n" +
				"-TUMOR_CONCORDANT [STRING] (Default: none)\n" +
				"\t Tumor concordants file for interval counting parameters file.\n" +
				"-NORMAL_CONCORDANT [STRING] (Default: none)\n" +
				"\t Normal concordants file for interval counting parameters file.");
	}
	
	
	/**
	 * Prints the arguments that are set.
	 */
	public void printArguments()
	{
		System.out.println("\n=====================================");
		System.out.println("Arguments are:");
		System.out.println("   CLUSTERS_FILE  = " + CLUSTER_FILE);
		System.out.println("   BANDS_FILE  = " + BANDS_FILE);
		if (EXCLUDE_FILE != null)
		{
			System.out.println("   EXCLUDE_FILE = " + EXCLUDE_FILE);
		}
		System.out.println("   MIN_INTERVAL = " + MIN_INTERVAL);
		System.out.println("   OUTPUT_PREFIX = " + OUTPUT_PREFIX);
		if (TUMOR_CONCORDANT != null)
		{
			System.out.println("   TUMOR_CONCORDANT = " + TUMOR_CONCORDANT);
		}
		if (NORMAL_CONCORDANT != null)
		{
			System.out.println("   NORMAL_CONCORDANT = " + NORMAL_CONCORDANT);
		}
		System.out.println("\n=====================================");
	}
	
	
	/**
	 * Reads in the file of chromosomes to exclude and adds them to the list.
	 */
	private void loadExclusion()
	{
		if (EXCLUDE_FILE==null)
			return;
		
		try
		{
			FileInputStream fis = new FileInputStream(EXCLUDE_FILE);
			InputStreamReader isr = new InputStreamReader(fis);
			BufferedReader br = new BufferedReader(isr);
			
			String curLine;
			
			while ((curLine = br.readLine()) != null)
			{
				//Check for header by looking for #
				if (curLine.contains("#"))
					continue;
				
				//remove any extra spaces just in case
				String[] vals = curLine.split("\\s+");
				
				
				//Check for X and Y explicitly
				if (vals[0].equalsIgnoreCase("X"))
				{
					CHRM_EXCLUDE.put(23, 23);
				}
				
				else if (vals[0].equalsIgnoreCase("Y"))
				{
					CHRM_EXCLUDE.put(24,24);
				}
				else
				{
					int chrm = Integer.parseInt(vals[0]);
					CHRM_EXCLUDE.put(chrm,chrm);
				}
			}
			
			br.close();
			isr.close();
			fis.close();
		}
		catch (FileNotFoundException e)
		{
			System.err.println("Error! File not found: " + EXCLUDE_FILE);
			System.exit(-1);
		}
		catch (NumberFormatException e)
		{
			System.err.println("Error! Chromosomes to exclude must either be numeric or X, Y");
			System.exit(-1);
		} catch (IOException e) 
		{
			System.err.println("Error! IOException encounterd while reading file: " + EXCLUDE_FILE);
			e.printStackTrace();
			System.exit(-1);
		}
	}
	
	
	
	/**
	 * Reads in the cytogenetic bands file and stores the ends of chromosomes
	 * information in a HashMap
	 */
	private void loadChrmEnds()
	{
		try
		{
			FileInputStream fis = new FileInputStream(BANDS_FILE);
			InputStreamReader isr = new InputStreamReader(fis);
			BufferedReader br = new BufferedReader(isr);
			
			String curLine;
			
			int prevChrm = -1;
			long prevBP = 0;
			
			while ((curLine = br.readLine()) != null)
			{
				//Check for header by looking for #
				if (curLine.contains("#"))
					continue;
				
				//remove any extra spaces just in case
				String[] vals = curLine.split("\\s+");
				
				int chrm = parseChr(vals[0]);
				long bp = Long.parseLong(vals[2]);
				
				//Then last was the end of the chromosome
				if (prevChrm != -1 && prevChrm != chrm)
				{
					CHRM_ENDS.put(prevChrm, prevBP);
				}
				
				prevChrm = chrm;
				prevBP = bp;
			}
			
			br.close();
			isr.close();
			fis.close();
			
			//Add final chrm
			CHRM_ENDS.put(prevChrm, prevBP);
		}
		catch (FileNotFoundException e)
		{
			System.err.println("Error! File not found: " + BANDS_FILE);
			System.exit(-1);
		}
		catch (NumberFormatException e)
		{
			System.err.println("Error! Chromosome ends must be longs.");
			System.exit(-1);
		} catch (IOException e) 
		{
			System.err.println("Error! IOException encounterd while reading file: " + BANDS_FILE);
			e.printStackTrace();
			System.exit(-1);
		}
	}
	
	
	/**
	 * Reads in the clusters file and loads the data into the internal data structures.
	 */
	private void loadClusters()
	{
		try
		{
			FileInputStream fis = new FileInputStream(CLUSTER_FILE);
			InputStreamReader isr = new InputStreamReader(fis);
			BufferedReader br = new BufferedReader(isr);
			
			String curLine;
			
			while ((curLine = br.readLine()) != null)
			{
				//Check for header by looking for #
				if (curLine.substring(0,1).equals("#"))
					continue;
				
				Cluster curCluster = parseCluster(curLine);
				String id = curCluster.clusterID;
				
				//Ignore type DV
				if (curCluster.type.equals("DV"))
					continue;
				
				//Check for exclusions and that we have all chrms
				if (!isExcluded(curCluster) && haveChrms(curCluster))
					clusters.put(id,curCluster);
				else
					continue;
				
				if (curCluster.type.equals("L+"))
				{
					breakEnd right = new breakEnd(curCluster.rightChrm, curCluster.rightBP, "internal", curCluster.clusterID);
					bps.get(curCluster.rightChrm).add(right);
					breakEnd[] newEnds = new breakEnd[1];
					newEnds[0] = right;
					allEnds.put(id, newEnds);
				}
				else if (curCluster.type.equals("L-"))
				{
					breakEnd left = new breakEnd(curCluster.leftChrm, curCluster.leftBP, "internal", curCluster.clusterID);
					bps.get(curCluster.rightChrm).add(left);
					breakEnd[] newEnds = new breakEnd[1];
					newEnds[0] = left;
					allEnds.put(id, newEnds);
				}
				else
				{
					//Create two breakEnd objects
					breakEnd left = new breakEnd(curCluster.leftChrm, curCluster.leftBP, "internal", curCluster.clusterID);
					breakEnd right = new breakEnd(curCluster.rightChrm, curCluster.rightBP, "internal", curCluster.clusterID);
					
					//Check if exists????
					bps.get(curCluster.leftChrm).add(left);
					bps.get(curCluster.rightChrm).add(right);
					
					breakEnd[] newEnds = new breakEnd[2];
					newEnds[0] = left;
					newEnds[1] = right;
					allEnds.put(id, newEnds);
				}
				
			}
			
			br.close();
			isr.close();
			fis.close();
		}
		catch (FileNotFoundException e)
		{
			System.err.println("Error! File not found: " + CLUSTER_FILE);
			System.exit(-1);
		}
		catch (NumberFormatException e)
		{
			System.err.println("Error! Number formatting issues in: " + CLUSTER_FILE);
			System.exit(-1);
		} catch (IOException e) 
		{
			System.err.println("Error! IOException encounterd while reading file: " + CLUSTER_FILE);
			e.printStackTrace();
			System.exit(-1);
		}
	}
	
	private boolean isExcluded(Cluster aCluster)
	{
		int leftChr = aCluster.leftChrm;
		int rightChr = aCluster.rightChrm;
		
		if (CHRM_EXCLUDE.containsKey(leftChr) || CHRM_EXCLUDE.containsKey(rightChr))
			return true;
		
		return false;
	}
	
	private boolean haveChrms(Cluster aCluster)
	{
		int leftChr = aCluster.leftChrm;
		int rightChr = aCluster.rightChrm;
		
		if (CHRM_ENDS.containsKey(leftChr) && CHRM_ENDS.containsKey(rightChr))
			return true;
		
		return false;
	}
	
	private void initializeBPLists()
	{
		bps = new TreeMap<Integer, ArrayList<breakEnd>>();
		
		//Loop on Chrms we have ends for check excusion to add start and end to priority queue
		for (int key : CHRM_ENDS.keySet())
		{
			if (!CHRM_EXCLUDE.containsKey(key))
			{
				breakEnd start = new breakEnd(key,START,"start","start");
				breakEnd terminal = new breakEnd(key,CHRM_ENDS.get(key),"end","end");
				
				ArrayList<breakEnd> list = new ArrayList<breakEnd>();
				list.add(start);
				list.add(terminal);
				
				bps.put(key,list);
			}
		}

	}
	
	
	private void removeShortIntervals()
	{
		for (int key : bps.keySet())
		{
			ArrayList<breakEnd> curList = bps.get(key);
			Collections.sort(curList);
			
			int idx = 1;
			
			while (idx < curList.size())
			{
				breakEnd prev = curList.get(idx-1);
				breakEnd next = curList.get(idx);
				
				if (next.bp - prev.bp >= MIN_INTERVAL)
				{
					idx++;
				}
				else //Need to remove a cluster
				{
					
					breakEnd toRemove = null;
					
					//Never remove an end
					if (prev.type.equals("start"))
						toRemove = next;
					
					if (next.type.equals("end"))
						toRemove = prev;
					
					//Short interval, can't remove anything
					if (prev.type.equals("start") && next.type.equals("end"))
					{
						idx++;
						continue;
					}
					
					if (!prev.type.equals("start") && !next.type.equals("end"))
					{
						//Remove based on numPES
						Cluster prevCluster = clusters.get(prev.cid);
						Cluster nextCluster = clusters.get(next.cid);
						
						if (prevCluster.numPR < nextCluster.numPR)
							toRemove = prev;
						else if (prevCluster.numPR > nextCluster.numPR)
							toRemove = next;
						else
						{
							double val = lotto.nextDouble();
							if (val < 0.5)
								toRemove = prev;
							else
								toRemove = next;
						}
					}
					
					String cid = toRemove.cid;
					
					//Remove from cluster list
					clusters.remove(cid);
					
					//Remove both breakEnds
					breakEnd[] ends = allEnds.get(cid);
					for (int i = 0; i < ends.length; i++)
					{
						breakEnd curEnd = ends[i];
						int chrm = curEnd.chrm;
						
						bps.get(chrm).remove(curEnd);
					}
					
					allEnds.remove(cid);
					
					//Reset idx
					idx = 1;
				}
			}
		}
	}
	
	
	private void buildGraph()
	{
		//Iterate through the list of clusters remaining
		for (String key : clusters.keySet())
		{
			Cluster curCluster = clusters.get(key);
			String type = curCluster.type;
			
			//Do different things based on type
			if (type.equals("D"))
				addDeletion(curCluster);
			
			if (type.equals("V"))
				addDivergent(curCluster);

			if (type.equals("I") || type.equals("I-") || type.equals("I+"))
				addInversion(curCluster, type);
			
			//Handle both old and new types
			if (type.equals("TR+") || type.equals("TN+1") || type.equals("TN+2") ||
					type.equals("TR-") || type.equals("TN-1") || type.equals("TN-2") ||
					type.equals("TR") || type.equals("TNR-") || type.equals("TNR+") ||
					type.equals("TO") || type.equals("TNO-") || type.equals("TNO+"))
				addTrans(curCluster,type);
			
			if (type.equals("L+") || type.equals("L-"))
				addLoop(curCluster, type);
		}
		
		
		//Sort
		for (int key : bps.keySet())
		{
			ArrayList<breakEnd> curList = bps.get(key);
			Collections.sort(curList);
		}
	}
	
	private void addDeletion(Cluster aCluster)
	{
		//Create break end nodes
		int leftChrm = aCluster.leftChrm;
		int rightChrm = aCluster.rightChrm;
		
		breakEnd leftLeft = new breakEnd(leftChrm, aCluster.leftBP - 1, "internal", aCluster.clusterID);
		breakEnd leftRight = new breakEnd(leftChrm, aCluster.leftBP, "internal", aCluster.clusterID);
		breakEnd rightLeft = new breakEnd(rightChrm, aCluster.rightBP - 1, "internal", aCluster.clusterID);
		breakEnd rightRight = new breakEnd(rightChrm, aCluster.rightBP, "internal", aCluster.clusterID);
		
		aCluster.leftLeft = leftLeft;
		aCluster.leftRight = leftRight;
		aCluster.rightLeft = rightLeft;
		aCluster.rightRight = rightRight;
		
		breakEnd[] edge = new breakEnd[2];
		edge[0] = leftLeft;
		edge[1] = rightRight;
		
		aCluster.edges.add(edge);
		
		bps.get(leftChrm).add(leftLeft);
		bps.get(leftChrm).add(leftRight);
		bps.get(rightChrm).add(rightLeft);
		bps.get(rightChrm).add(rightRight);
	}
	
	private void addDivergent(Cluster aCluster)
	{
		//Create break end nodes
		int leftChrm = aCluster.leftChrm;
		int rightChrm = aCluster.rightChrm;
		
		breakEnd leftLeft = new breakEnd(leftChrm, aCluster.leftBP - 1, "internal", aCluster.clusterID);
		breakEnd leftRight = new breakEnd(leftChrm, aCluster.leftBP, "internal", aCluster.clusterID);
		breakEnd rightLeft = new breakEnd(rightChrm, aCluster.rightBP - 1, "internal", aCluster.clusterID);
		breakEnd rightRight = new breakEnd(rightChrm, aCluster.rightBP, "internal", aCluster.clusterID);
		
		aCluster.leftLeft = leftLeft;
		aCluster.leftRight = leftRight;
		aCluster.rightLeft = rightLeft;
		aCluster.rightRight = rightRight;
		
		breakEnd[] edge = new breakEnd[2];
		edge[0] = leftRight;
		edge[1] = rightLeft;
		
		aCluster.edges.add(edge);
		
		bps.get(leftChrm).add(leftLeft);
		bps.get(leftChrm).add(leftRight);
		bps.get(rightChrm).add(rightLeft);
		bps.get(rightChrm).add(rightRight);
	}
	
	private void addInversion(Cluster aCluster, String type)
	{
		//Create break end nodes
		int leftChrm = aCluster.leftChrm;
		int rightChrm = aCluster.rightChrm;
		
		breakEnd leftLeft = new breakEnd(leftChrm, aCluster.leftBP - 1, "internal", aCluster.clusterID);
		breakEnd leftRight = new breakEnd(leftChrm, aCluster.leftBP, "internal", aCluster.clusterID);
		breakEnd rightLeft = new breakEnd(rightChrm, aCluster.rightBP - 1, "internal", aCluster.clusterID);
		breakEnd rightRight = new breakEnd(rightChrm, aCluster.rightBP, "internal", aCluster.clusterID);
		
		aCluster.leftLeft = leftLeft;
		aCluster.leftRight = leftRight;
		aCluster.rightLeft = rightLeft;
		aCluster.rightRight = rightRight;
		
		bps.get(leftChrm).add(leftLeft);
		bps.get(leftChrm).add(leftRight);
		bps.get(rightChrm).add(rightLeft);
		bps.get(rightChrm).add(rightRight);
		
		if (type.equals("IR") || type.equals("I+"))
		{
			breakEnd[] edge = new breakEnd[2];
			edge[0] = leftRight;
			edge[1] = rightRight;
			
			aCluster.edges.add(edge);
		}
		
		if (type.equals("IR") || type.equals("I-"))
		{
			breakEnd[] edge = new breakEnd[2];
			edge[0] = leftLeft;
			edge[1] = rightLeft;
			
			aCluster.edges.add(edge);
		}
	}
	
	
	private void addTrans(Cluster aCluster, String type)
	{
		//Create break end nodes
		int leftChrm = aCluster.leftChrm;
		int rightChrm = aCluster.rightChrm;
		
		breakEnd leftLeft = new breakEnd(leftChrm, aCluster.leftBP - 1, "internal", aCluster.clusterID);
		breakEnd leftRight = new breakEnd(leftChrm, aCluster.leftBP, "internal", aCluster.clusterID);
		breakEnd rightLeft = new breakEnd(rightChrm, aCluster.rightBP - 1, "internal", aCluster.clusterID);
		breakEnd rightRight = new breakEnd(rightChrm, aCluster.rightBP, "internal", aCluster.clusterID);
		
		aCluster.leftLeft = leftLeft;
		aCluster.leftRight = leftRight;
		aCluster.rightLeft = rightLeft;
		aCluster.rightRight = rightRight;
		
		bps.get(leftChrm).add(leftLeft);
		bps.get(leftChrm).add(leftRight);
		bps.get(rightChrm).add(rightLeft);
		bps.get(rightChrm).add(rightRight);
		
		if (type.equals("TR+") || type.equals("TR") || type.equals("TN+1") || type.equals("TNR+"))
		{
			breakEnd[] edge = new breakEnd[2];
			edge[0] = leftLeft;
			edge[1] = rightRight;
			
			aCluster.edges.add(edge);
		}
		
		if (type.equals("TR+") || type.equals("TR") || type.equals("TN+2") || type.equals("TNR-"))
		{
			breakEnd[] edge = new breakEnd[2];
			edge[0] = leftRight;
			edge[1] = rightLeft;
			
			aCluster.edges.add(edge);
		}
		
		if (type.equals("TR-") || type.equals("TO") || type.equals("TN-1") || type.equals("TNO+"))
		{
			breakEnd[] edge = new breakEnd[2];
			edge[0] = leftLeft;
			edge[1] = rightLeft;
			
			aCluster.edges.add(edge);
		}
		
		if (type.equals("TR-") || type.equals("TO") || type.equals("TN-2") || type.equals("TNO-"))
		{
			breakEnd[] edge = new breakEnd[2];
			edge[0] = leftRight;
			edge[1] = rightRight;
			
			aCluster.edges.add(edge);
		}
	}
	
	private void addLoop(Cluster aCluster, String type)
	{
		if (type.equals("L+"))
		{
			int rightChrm = aCluster.rightChrm;
			
			breakEnd rightLeft = new breakEnd(rightChrm, aCluster.rightBP - 1, "internal", aCluster.clusterID);
			breakEnd rightRight = new breakEnd(rightChrm, aCluster.rightBP, "internal", aCluster.clusterID);
			
			aCluster.rightLeft = rightLeft;
			aCluster.rightRight = rightRight;
			
			bps.get(rightChrm).add(rightLeft);
			bps.get(rightChrm).add(rightRight);
			
			breakEnd[] edge = new breakEnd[2];
			edge[0] = rightLeft;
			edge[1] = rightLeft;
			
			aCluster.edges.add(edge);
		}
		
		if (type.equals("L-"))
		{
			int leftChrm = aCluster.leftChrm;
			
			breakEnd leftLeft = new breakEnd(leftChrm, aCluster.leftBP - 1, "internal", aCluster.clusterID);
			breakEnd leftRight = new breakEnd(leftChrm, aCluster.leftBP, "internal", aCluster.clusterID);
			
			aCluster.leftLeft = leftLeft;
			aCluster.leftRight = leftRight;
			
			bps.get(leftChrm).add(leftLeft);
			bps.get(leftChrm).add(leftRight);
			
			breakEnd[] edge = new breakEnd[2];
			edge[0] = leftLeft;
			edge[1] = leftLeft;
			
			aCluster.edges.add(edge);
		}
	}
	
	private void printIntervals()
	{
		String intFile = OUTPUT_PREFIX + ".intervals";
		
		String header = "#ID" + "\t" + "chrm" + "\t" + "start" + "\t" + "end";
		
		try
		{
			FileOutputStream fos = new FileOutputStream(intFile);
			Writer aWriter = new OutputStreamWriter(fos);
			BufferedWriter out = new BufferedWriter(aWriter);
			
			out.write(header);
			out.newLine();
			
			for (int key : bps.keySet())
			{
				ArrayList<breakEnd> list = bps.get(key);
				
				for (int i=0; i < list.size(); i+=2)
				{
					breakEnd first = list.get(i);
					breakEnd second = list.get(i+1);
					
					String ID = first.getID() + ":" + second.getID();
					
					long start = first.bp;
					long end = second.bp;
					
					String line = ID + "\t" + key + "\t" + start + "\t" + end;
					out.write(line);
					out.newLine();
				}//end of all in chromosome
			}//end key iteration
			out.close();
			aWriter.close();
			fos.close();
		}//end of try
		catch (IOException e)
		{
			System.err.println("Error!  Cannot write to output directory.");
			e.printStackTrace();
			System.exit(-1);
		}
	}
	
	
	private void printVariants()
	{
		String intFile = OUTPUT_PREFIX + ".variants";
		
		String header = "#ID" + "\t" + "chrm" + "\t" + "bp" + "\t" + "ID" + "\t" + "chrm" + "\t" + "bp" + "\t" + "type" + "\t" + "Localization";
		
		try
		{
			FileOutputStream fos = new FileOutputStream(intFile);
			Writer aWriter = new OutputStreamWriter(fos);
			BufferedWriter out = new BufferedWriter(aWriter);
			
			out.write(header);
			out.newLine();
			
			for (String key : clusters.keySet())
			{
				Cluster c = clusters.get(key);
				ArrayList<breakEnd[]> edges = c.edges;
				String type  = c.type;
				double local = c.localization;
				
				for (int i = 0; i < edges.size(); i++)
				{
					breakEnd[] cur = edges.get(i);
					
					breakEnd first = cur[0];
					breakEnd second = cur[1];
					
					int chrm1 = first.chrm;
					long bp1 = first.bp;
					String id1 = first.getID();
					int chrm2 = second.chrm;
					long bp2 = second.bp;
					String id2 = second.getID();
					
					String line = id1 + "\t" + chrm1 + "\t" + bp1 + "\t" + id2 + "\t" + chrm2 + "\t" + bp2 + "\t" + type + "\t" + local;
					out.write(line);
					out.newLine();
				}//end of all in chromosome
			}//end key iteration
			out.close();
			aWriter.close();
			fos.close();
		}//end of try
		catch (IOException e)
		{
			System.err.println("Error!  Cannot write to output directory.");
			e.printStackTrace();
			System.exit(-1);
		}
	}
	
	public void printParameters()
	{
		String params = OUTPUT_PREFIX + ".parameters";
		
		try
		{
			FileOutputStream fos = new FileOutputStream(params);
			Writer aWriter = new OutputStreamWriter(fos);
			BufferedWriter out = new BufferedWriter(aWriter);
			
			out.write("IntervalFile: ");
			out.write(OUTPUT_PREFIX + ".intervals");
			out.newLine();
			out.write("Software: PREGO");
			out.newLine();
			
			if (TUMOR_CONCORDANT != null)
			{
				out.write("ConcordantFile: " + TUMOR_CONCORDANT);
				out.newLine();
			}
			
			if (NORMAL_CONCORDANT != null)
			{
				out.write("ConcordantFile: " + NORMAL_CONCORDANT);
				out.newLine();
			}
			
			out.close();
			aWriter.close();
			fos.close();
		}//end of try
		catch (IOException e)
		{
			System.err.println("Error!  Cannot write to output directory.");
			e.printStackTrace();
			System.exit(-1);
		}
		
	}
	
	
	/**
	 * Parses a string from a cytogenetic bands file to return the integer
	 * version of the chromosome number.
	 * @param chrStr
	 * @return
	 */
	private static int parseChr(String chrStr) {

		int chr = 0;
		if (chrStr.equalsIgnoreCase("chrX") || chrStr.equalsIgnoreCase("X")) {
			chr = 23;
		} else if (chrStr.equalsIgnoreCase("chrY") || chrStr.equalsIgnoreCase("Y")) {
			chr = 24;
		} else {
			if (chrStr.startsWith("chr")) {
				chrStr = chrStr.substring(3);
			}
			chr = Integer.parseInt(chrStr);
		}
		return chr;
	}
	

	/**
	 * Parses a line from a clusters file into the corresponding Cluster
	 * object.  Handles both regions and standard GASV output.
	 * @param clusterLine
	 * @return
	 */
	private static Cluster parseCluster(String clusterLine)
	{
		String[] line = clusterLine.split("\\s+");
	
		//Get cluster ID
		String cid = line[0];
		int numPES = 0;
		
		//Handle localization
		double localization = -2;
		
		//If in standard format
		if (line[2].contains(",")) 
		{
			numPES = Integer.parseInt(line[5]);
			localization = Double.parseDouble(line[6]);
		} 
		else 
		{
			//In regions format
			numPES = Integer.parseInt(line[1]);
			localization = Double.parseDouble(line[2]);
		}
		
		int len = line.length;
		int leftChr = -1;
		int rightChr = -1;

		//use the average coordinates as the breakpoints
		long x = 0;
		long y = 0;
		long x1, x2, y1, y2;
		String type = null;
		
		if (line[2].contains(",")) {
			String[] xVal = line[2].split(",");
			String[] yVal = line[4].split(",");
			x1 = Integer.parseInt(xVal[0]);
			x2 = Integer.parseInt(xVal[1]);
			y1 = Integer.parseInt(yVal[0]);
			y2 = Integer.parseInt(yVal[1]);
			x = (x1 + x2) / 2;
			y = (y1 + y2) / 2;
			leftChr = parseChr(line[1]);
			rightChr = parseChr(line[3]);
			type = line[7];
		}
		else
		{
			int typeOffset = 1;
			type = line[3];
			leftChr = parseChr(line[3+numPES+typeOffset]);
			rightChr = parseChr(line[4+numPES+typeOffset]);
			
			int coordCount = 0;
			String xCoord;
			String yCoord;
			
			//find the average of the x, y coordinates
			//used to just find the extremes
			for (int i=(5+numPES+typeOffset); i<len; i+=2) {
				xCoord = line[i];
				yCoord = line[i+1];
				xCoord = xCoord.substring(0, xCoord.length()-1);
				if (i < (len -2)) {
					yCoord = yCoord.substring(0, yCoord.length()-1);
				}
				x += Long.parseLong(xCoord);
				//if (Long.parseLong(xCoord) < x) {
					//x = Long.parseLong(xCoord);
				//}
				y += Long.parseLong(yCoord);
				//if (Long.parseLong(yCoord) > y) {
				//	y = Long.parseLong(yCoord);
				//}
				coordCount++;
			}
			x = x / coordCount;
			y = y / coordCount;
		}//end if in regions format
		
		Cluster newCluster = new Cluster(cid, type, localization, leftChr, rightChr, x, y, numPES);
		return newCluster;
	}
}

class Cluster
{
	public String clusterID = null;
	public String type = null;
	public double localization;
	public int leftChrm;
	public int rightChrm;
	public long leftBP;
	public long rightBP;
	public double numPR;
	
	public breakEnd leftLeft = null;
	public breakEnd leftRight = null;
	public breakEnd rightLeft = null;
	public breakEnd rightRight = null;
	
	public ArrayList<breakEnd[]> edges = null;
	
	public Cluster(String ID, String aType, double aLocal, int c1, int c2, long lbp, long rbp, double num)
	{
		clusterID = ID;
		type = aType;
		localization = aLocal;
		leftChrm = c1;
		rightChrm = c2;
		leftBP = lbp;
		rightBP = rbp;
		numPR = num;
		
		edges = new ArrayList<breakEnd[]>();
	}
}

class breakEnd implements Comparable<breakEnd>
{
	public int chrm;
	public long bp;
	public String type; //origin, s-node, t-node, both, or end
	public String cid;
	
	public breakEnd(int aChrm, long aBP, String curType, String aCid)
	{
		chrm = aChrm;
		bp = aBP;
		type = curType;
		cid = aCid;
	}
	
	public int compareTo(breakEnd otherBE) {
		if (bp < otherBE.bp) {
			return -1;

		} else if (bp > otherBE.bp) {
			return 1;
		}
		//otherwise must be equal
		return 0;
	}
	
	public String getID()
	{
		return cid + "_" + chrm + "_" + bp;
	}
}
