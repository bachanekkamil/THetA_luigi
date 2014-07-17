import ilog.concert.IloException;
import ilog.concert.IloLinearNumExpr;
import ilog.concert.IloNumExpr;
import ilog.concert.IloNumVar;
import ilog.concert.IloObjective;
import ilog.cplex.IloCplex;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * 2012, 2013 Brown University, Providence, RI.
 *
 *                       All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose other than its incorporation into a
 * commercial product is hereby granted without fee, provided that the
 * above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Brown University not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.

 * BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * http://cs.brown.edu/people/braphael/software.html
 * 
 * @author Layla oesper, Anna Ritz, Sarah Aerni, Ryan Drebin and Benjamin Raphael
 *
 */


public class runPREGO 
{
	
	// PUBLIC VARIABLES
	public String INTERVAL_FILE;
	public String VARIANT_FILE;
	public String OUTPUT_PREFIX;
	public int TIMEOUT = 3600; // 1 hour
	public int EXPECTED_COPY=2;
	public boolean HANDLE_TELOS = false;;
	
	// INTERNAL VARIABLES
	
	//OTHER VARIABLES
	public boolean USE_NORMAL = true;
	public ArrayList<Interval> intervals;
	public long referenceLength;
	public long numTumorReads;
	public long numNormalReads;
	
	public double objValue;
	public boolean isFeasible;
	public IntervalAdjacency graph;

	/**
	 * @param args
	 */
	public static void main(String[] args) 
	{
		//Step 1: Load in parameters
		runPREGO prego = new runPREGO(args);
		
		//Step 2: Run ILP
		prego.solveILP();
		
		//Step 3: Save Solution to file
		prego.printSolution();
		prego.graph.printGraphML(prego.OUTPUT_PREFIX + ".graphml");

	}
	
	
	// CONSTRUCTOR
	
	/**
	 * Constructor - parses arguments and sets parameters.
	 * @param args - see usage information for arguments
	 */
	public runPREGO(String[] args)
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
	}
	
	/**
	 * Parses input arguments
	 * @param args - see usage instructions.
	 * @return true if there are no errors, false otherwise.
	 */
	public boolean parseArguments(String[] args)
	{
		//Check the number of arguments is correct (always even and positive)
		if (args.length % 2 != 0 || args.length == 0)
		{
			System.err.println("Error! Incorrect number of arguments.");
			return false;
		}
		
		//Initialize IntervalAdjacency Graph which will be filled out
		graph = new IntervalAdjacency();
		
		// Parse interval file
		INTERVAL_FILE = args[0];
		
		OUTPUT_PREFIX=INTERVAL_FILE;
		
		
		//For each pair of arguments (flag and value) set parameters
		for (int i=2; i< args.length; i+=2)
		{
			
			if (args[i].equalsIgnoreCase("-TIMEOUT"))
			{
				try
				{
					TIMEOUT = Integer.parseInt(args[i+1]);
					if (TIMEOUT <= 0)
						throw new Exception();
				}
				catch (Exception e)
				{
					System.out.println("Error! TIMEOUT must be a non-negative integer.");
					return false;
				}
			}//End TIMEOUT Flag
			
			if (args[i].equalsIgnoreCase("-EXPECTED_COPY"))
			{
				try
				{
					EXPECTED_COPY = Integer.parseInt(args[i+1]);
					if (EXPECTED_COPY <= 0)
						throw new Exception();
				}
				catch (Exception e)
				{
					System.out.println("Error! EXPECTED_COPY must be a non-negative integer.");
					return false;
				}
			}//End EXPECTED_COPY
			
			if (args[i].equalsIgnoreCase("-OUTPUT_PREFIX"))
			{
				OUTPUT_PREFIX = args[i+1];
			}
			
			if (args[i].equalsIgnoreCase("-USE_NORMAL"))
			{
				try
				{
					USE_NORMAL = Boolean.parseBoolean(args[i+1]);
				}
				catch (Exception e)
				{
					System.out.println("Error!  USE_NORMAL must be a valid boolean (true/false).");
					return false;
				}
			}
			
			if (args[i].equalsIgnoreCase("-HANDLE_TELOS"))
			{
				try
				{
					HANDLE_TELOS = Boolean.parseBoolean(args[i+1]);
				}
				catch (Exception e)
				{
					System.out.println("Error!  HANDLE_TELOS must be a valid boolean (true/false).");
					return false;
				}
			}
		} //End optional parameters
		
		if (!(new File(INTERVAL_FILE).exists()))
		{
			System.err.println("Error! " + INTERVAL_FILE + " does not exist.");
			return false;
		}
		else
		{
			boolean success = loadIntervalFile();
			if (!success)
				return success;
			
			buildGraphIntervals();

		}
		
		// Parse interval file
		VARIANT_FILE = args[1];
		
		if (!(new File(VARIANT_FILE).exists()))
		{
			System.err.println("Error! " + VARIANT_FILE + " does not exist.");
			return false;
		}
		else
		{
			boolean success = loadVariantFile();
			if(!success)
				return success;
		}
		
		objValue = -1;
		isFeasible = false;
		
		return true;
	}//end parseArguments
	
	/**
	 * Prints the usage information.
	 */
	public void printUsage()
	{
		System.out.println("\nProgram: runPREGO");
		System.out.println("USAGE: ./runPREGO <INTERVAL_FILE> <VARIANT_FILE> [Options]\n" +
				"USAGE: bash runPREGO <INTERVAL_FILE> <VARIANT_VFILE> [Options]\n" +
				//"USAGE (jar): java -jar runPREGO <INTERVAL_FILE> <VARIANT_FILE> [Options]\n" + 
				"<INTERVAL_FILE> [String]\n" +
				"\t An input file containing id chrm, start, end, tumorCt, normalCt (optional) \n" +
				"<VARIANT_FILE> [String]\n" +
				"\t An input file containing id1 chrm1 bp1 id2 chrm2 bp2 [optional columns] \n" +
				"Options are:\n" + 
				"-USE_NORMAL [Boolean] (Default: True)\n" +
				"\t Whether or not to use the normal read depth to set expected. \n" +
				"-OUTPUT_PREFIX [String] (Default: <INTERVAL_FILE>)\n" +
				"\t Prefix for all files created. \n" +
				"-TIMEOUT [Integer] (Default: 3600)\n" +
				"\t The number of seconds for which the ILP will timeout if it has not found a solution.\n" + 
				"-EXPECTED_COPY [Integer] (Default: 2)\n" +
				"\t The number of copies of each chromosome in a normal genome.\n" +
				"-HANDLE_TELOS [Boolean] (Default: false)\n" +
				"\t Allows telomeres to be deleted in solution.");
	}
	
	/**
	 * Prints the arguments that are set.
	 */
	public void printArguments()
	{
		System.out.println("\n=====================================");
		System.out.println("Arguments are:");
		System.out.println("   INTERVAL_FILE  = " + INTERVAL_FILE);
		System.out.println("   VARIANT_FILE  = " + VARIANT_FILE);
		System.out.println("   OUTPUT_PREFIX = " + OUTPUT_PREFIX);
		System.out.println("   USE_NORMAL = " + USE_NORMAL);
		System.out.println("   TIMEOUT = " + TIMEOUT);
		System.out.println("   EXPECTED_COPY = " + EXPECTED_COPY);
		System.out.println("   HANDLE_TELOS = " + HANDLE_TELOS);
		System.out.println("\n=====================================");
	}
	
	public boolean loadIntervalFile()
	{
		intervals = new ArrayList<Interval>();
		referenceLength = 0;
		numTumorReads = 0;
		numNormalReads = 0;
		
		try
		{
            FileInputStream fis = new FileInputStream(INTERVAL_FILE);
            InputStreamReader isr = new InputStreamReader(fis);
            BufferedReader br = new BufferedReader(isr);
            
            String curLineValue = null;
            
            while ((curLineValue=br.readLine()) != null)
            {
            	//Skip header line
            	if (curLineValue.substring(0,1).equals("#"))
            		continue;
            	
            	Interval curInt = null;
            	
            	String[] spaceSplit = curLineValue.split("\\s+");
            	
            	if (!spaceSplit[0].contains(":")) //old format
            	{
            		try
            		{
            			curInt = parseOldLine(spaceSplit);
            		}
            		catch (NumberFormatException e)
            		{
            			System.out.println("Error! File contains invalid input: " + INTERVAL_FILE);
            			return false;
            		}
            		catch (Exception e)
            		{
            			System.out.println("Error! File not formatted correctly: " + INTERVAL_FILE);
            			return false;
            		}
            	}
            	else
            	{
            		try
            		{
            			curInt = parseNewLine(spaceSplit);
            		}
            		catch (NumberFormatException e)
            		{
            			System.out.println("Error! File contains invalid input: " + INTERVAL_FILE);
            			return false;
            		}
            		catch (Exception e)
            		{
            			System.out.println("Error! File not formatted correctly: " + INTERVAL_FILE);
            			return false;
            		}
            	}
            	
            	intervals.add(curInt);
            	numTumorReads+=curInt.numTumor;
            	numNormalReads+=curInt.numNormal;
            	referenceLength+=curInt.getLength();
            }//end iterate through file
            
            //Close files
            br.close();
            isr.close();
            fis.close();
		}
		catch (FileNotFoundException e)
		{
			System.out.println("Error!  File not found: " + INTERVAL_FILE);
			return false;
		}
		catch (IOException e)
		{
			System.out.println("Error!  Unable to read file: " + INTERVAL_FILE);
			return false;
		}
		
		//Normalize normal counts
		normalizeCounts();
		
		//Sort intervals
		Collections.sort(intervals);
		
		return true;
		
	}
	
	
	public boolean loadVariantFile()
	{
		try
		{
            FileInputStream fis = new FileInputStream(VARIANT_FILE);
            InputStreamReader isr = new InputStreamReader(fis);
            BufferedReader br = new BufferedReader(isr);
            
            String curLineValue = null;
            
            while ((curLineValue=br.readLine()) != null)
            {
            	//Skip header line
            	if (curLineValue.substring(0,1).equals("#"))
            		continue;
            	
            	
            	String[] spaceSplit = curLineValue.split("\\s+");
            	
            	String n1 = null;
            	String n2 = null;
            	
            	//Use the clusters starts with c to determine new format
            	if (spaceSplit[0].contains("_") && !spaceSplit[0].startsWith("c")) //old format
            	{

        			n1 = spaceSplit[0].substring(0,spaceSplit[0].length()-1);
        			n2 = spaceSplit[1];
            	}
            	else // new format
            	{
            		n1 = spaceSplit[0];
            		n2 = spaceSplit[3];
            	}
            	
    			IntAdjNode node1 = graph.nodes.get(n1);
    			IntAdjNode node2 = graph.nodes.get(n2);
    			

    				System.out.println("Error!  Variant node: " + node1 + " not found in interval file.");
    				return false;
    			}
    			
    			if (node2 == null )
    			{
    				System.out.println("Error!  Variant node: " + node2 + " not found in interval file.");
    				return false;
    			}
    			
    			//Update datastructures
    			IntAdjEdge curVariant = new IntAdjEdge(node1, node2);
    			graph.variantEdges.add(curVariant);
    			node1.varEdge.add(curVariant);
    			node2.varEdge.add(curVariant);
            	

            }//end iterate through file
            
            //Close files
            br.close();
            isr.close();
            fis.close();
		}
		catch (FileNotFoundException e)
		{
			System.out.println("Error!  File not found: " + INTERVAL_FILE);
			return false;
		}
		catch (IOException e)
		{
			System.out.println("Error!  Unable to read file: " + INTERVAL_FILE);
			return false;
		}
		
		return true;
	}
	
	public void normalizeCounts()
	{
		for (int i=0; i< intervals.size(); i++)
		{
			Interval curInt = intervals.get(i);
			
			if (!USE_NORMAL) 
			{
				// set as uniform distribution over length of interval compared to normal genome
				curInt.numNormal = Math.round(
						((double) curInt.getLength() * this.numTumorReads)/ this.referenceLength);
			}
			else
			{
				//Normalize distribution of normal to same number of reads as tumor sample
				curInt.numNormal = Math.round(
						((double) curInt.numNormal * this.numTumorReads)/this.numNormalReads);
			}
		}
	}
	
	public Interval parseOldLine(String[] vals) throws Exception
	{

		//Check for enough pieces
		if (vals.length < 3)
			throw new Exception();
		
		String firstPiece = vals[0];
		String[] firstPieceSplit = firstPiece.split("_");
		if (firstPieceSplit.length != 3)
			throw new Exception();
		
		//Deal with first piece
		String chrmStr = firstPieceSplit[1];
		int chrm1 = Integer.parseInt(chrmStr);
		String startStr = firstPieceSplit[2];
		startStr = startStr.substring(0,startStr.length() - 1); //remove comma
		Long start = Long.parseLong(startStr);
		
		//Deal with second piece
		String secondPiece = vals[1];
		String[] secondPieceSplit = secondPiece.split("_");
		if (secondPieceSplit.length != 3)
			throw new Exception();
		
		chrmStr = secondPieceSplit[1];
		int chrm2 = Integer.parseInt(chrmStr);
		if (chrm1 != chrm2)
			throw new Exception();
		String endStr = secondPieceSplit[2];
		Long end = Long.parseLong(endStr);
		
		String ID = firstPiece.substring(0,firstPiece.length() - 1) + ":" + secondPiece;
		
		Interval curInt = new Interval(chrm1, start, end, ID);
		
		curInt.numTumor = Long.parseLong(vals[2]); // get tumor
		
		//Check for normal component
		if (vals.length >= 4) 
			curInt.numNormal = Long.parseLong(vals[3]);
		else
		{
			this.USE_NORMAL=false;
			curInt.numNormal = 0;
		}
		
		return curInt;
	
	}
	
	public Interval parseNewLine(String[] vals) throws Exception
	{
		//Check for enough pieces
		if (vals.length < 5)
			throw new Exception();
		
		String ID = vals[0];
		int chrm = Integer.parseInt(vals[1]);
		long start = Long.parseLong(vals[2]);
		long end = Long.parseLong(vals[3]);
		long numTumor = Long.parseLong(vals[4]);
		
		Interval curInt = new Interval(chrm, start, end, ID);
		curInt.numTumor = numTumor;
		
		if (vals.length >= 6)
		{
			curInt.numNormal = Long.parseLong(vals[5]);
		}
		else
		{
			this.USE_NORMAL=false;
			curInt.numNormal = 0;
		}
		
		return curInt;
	}
	
	public void buildGraphIntervals()
	{
		//For checking end of chrms
		int lastChrm = 0;
		IntAdjNode lastNode = new IntAdjNode();
		
		
		//Build graph now that intervals are sorted - make sure that they partition genome.
		for (int i=0; i < intervals.size(); i++)
		{
			Interval curInt = intervals.get(i);
			int chrm = curInt.chrm;
			long start = curInt.start;
			long end = curInt.end;
			String ID = curInt.ID;
			String[] vals = ID.split(":");
			String ID1 = vals[0];
			String ID2 = vals[1];
			
			//Update data structures in graph
			IntAdjNode node1 = new IntAdjNode(ID1,chrm,start);
			IntAdjNode node2 = new IntAdjNode(ID2,chrm,end);
			graph.nodes.put(ID1, node1);
			graph.nodes.put(ID2, node2);
			graph.listOfNodes.add(node1);
			graph.listOfNodes.add(node2);
			
			IntAdjEdge newInt = new IntAdjEdge(node1, node2);
			node1.intEdge = newInt;
			node2.intEdge = newInt;
			newInt.tumorCoverage=curInt.numTumor;
			newInt.normalCoverage = curInt.numNormal;
			graph.intervalEdges.add(newInt);
			
			//Check for chromosome end
			if (lastChrm == chrm)
			{
				graph.nonEndNodes.add(lastNode);
				graph.nonEndNodes.add(node1);
				
				//Create a Reference Edge
				IntAdjEdge newRef = new IntAdjEdge(lastNode, node1);
				lastNode.refEdge = newRef;
				node1.refEdge = newRef;
				graph.refEdges.add(newRef);
			}
			
			//Update Variables we check
			lastNode = node2;
			lastChrm = chrm;
		}//end while loop on all intervals
		
		if (HANDLE_TELOS)
			graph.updateTelomereNodes();
	}
	
	public void solveILP()
	{
		//Get all variables we may need
		int numIntervals = intervals.size();
		List<IntAdjNode> nonEndNodes = graph.nonEndNodes;
        List<IntAdjEdge> intervalEdges = graph.intervalEdges;
        List<IntAdjEdge> referenceEdges = graph.refEdges;
        List<IntAdjEdge> variantEdges = graph.variantEdges;
        List<IntAdjNode> telomereNodes = graph.telomereNodes;
        
        int numInts = intervalEdges.size();
        int numRefs = referenceEdges.size();
        int numVars = variantEdges.size();
        int numNonEndNodes = nonEndNodes.size();
        int numTelomereNodes = telomereNodes.size();
        
        //Build Cplex program
        try
        {
        	IloCplex cplex = new IloCplex();
        	
        	//Variables for each edge type
            IloNumVar[] intVars = cplex.intVarArray(numInts, 0, Integer.MAX_VALUE);
            IloNumVar[] refVars = cplex.intVarArray(numRefs, 0, Integer.MAX_VALUE);
            IloNumVar[] varVars = cplex.intVarArray(numVars, 0, Integer.MAX_VALUE);
            IloNumVar[] teloVars = cplex.intVarArray(numTelomereNodes, 0, Integer.MAX_VALUE);
            
            //OBJECTIVE STUFF GOES HERE
            IloNumExpr[] objExpressions = new IloNumExpr[numIntervals]; // hold all piecewise constraints
            double[] breakpoints = {1, 2, 3, 4, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
            double[] xVals = {0.5, 1, 2, 3, 4, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50 ,55};
            double[] yVals = new double[xVals.length];
            double[] slopes = new double[breakpoints.length + 1];
            
            
            //Loop through all intervals
            for (int i=0; i < numIntervals; i++)
            {
                //For objective
                IloLinearNumExpr intervalCount = cplex.linearNumExpr();
                intervalCount.addTerm(intVars[i], 1.0);
                
                Interval curInt = intervals.get(i);
                long tumorCount = curInt.numTumor;
                long normalCount = curInt.numNormal;
                long length = curInt.getLength();
                                

                double frac = 1.0 / EXPECTED_COPY;
                double objRatio = (frac * normalCount);
                
                //OLD WAY
                double ratio = (frac * length * numTumorReads) / (double) referenceLength;
                
                //Build yVals
                for (int k = 0; k < xVals.length; k++)
                {
                        
                	double val = objRatio*xVals[k] - tumorCount * Math.log(xVals[k]);
                	//double val = ratio*xVals[k] - tumorCount * Math.log(xVals[k]);

                    yVals[k] = val;
                        
                }
                
                //Build Slopes
                for (int k = 0; k < slopes.length; k++)
                {
                        double x1 = xVals[k];
                        double y1 = yVals[k];
                        double x2 = xVals[k+1];
                        double y2 = yVals[k+1];
                        
                        double curSlope = (y2 - y1)/(x2 - x1);
                        slopes[k] = curSlope;
                        
                }
                
                //Anchor at copy count of 2
                double anchor = xVals[2];
                double fAnchor = yVals[2];
                
                //New objective function expression
                IloNumExpr objectivePiece = cplex.piecewiseLinear(intervalCount, breakpoints, slopes, anchor, fAnchor);
                objExpressions[i] = objectivePiece;
                
            }
            
            //CONSTRAINTS
            //Add one constraint per non end node in the IA graph - that balances that node
            for (int i=0; i < numNonEndNodes; i++)
            {
                IloLinearNumExpr balancedExpr = cplex.linearNumExpr();
                IntAdjNode curNode = nonEndNodes.get(i);
                
                //Retrieve the interval edge
                IntAdjEdge curInterval = curNode.intEdge;
                int intervalIdx = intervalEdges.indexOf(curInterval);
                balancedExpr.addTerm(intVars[intervalIdx], 1.0);
                
                //Retrieve the reference edge (there must be one since node is non end)
                IntAdjEdge curReference = curNode.refEdge;
                int referenceIdx = referenceEdges.indexOf(curReference);
                balancedExpr.addTerm(refVars[referenceIdx], -1.0);
                
                
                //Retrieve any variant edges
                List<IntAdjEdge> variants = curNode.varEdge;
                for (int j=0; j < variants.size(); j++)
                {
                    IntAdjEdge curVariant = variants.get(j);
                    int variantIdx = variantEdges.indexOf(curVariant);
                    balancedExpr.addTerm(varVars[variantIdx], -1.0);
                }
                
                //Check for telomere node
                if (graph.isTelomereNode(curNode) && (HANDLE_TELOS))
                {       
                        //get Index
                        int teloIdx = telomereNodes.indexOf(curNode);
                        balancedExpr.addTerm(teloVars[teloIdx], -1.0);
                }
                
                //Add constraint to the model
                cplex.addEq(balancedExpr, 0);
            }//end loop on non End Nodes
            
            
            //Take sum of all objective expressions
            IloNumExpr sumObjective = cplex.sum(objExpressions);
            
            
            IloObjective obj = cplex.minimize(sumObjective);
            cplex.add(obj);
            
            //Set Default Timeout to 1 hour
            cplex.setParam(IloCplex.DoubleParam.TiLim, this.TIMEOUT);
            cplex.setParam(IloCplex.DoubleParam.BarObjRng,1e70);
            //cplex.setParam(IloCplex.IntParam.PreDual, 1);
            //cplex.setParam(IloCplex.IntParam.RootAlg, 4);
            
            
            //Solve the ILP
            this.isFeasible = cplex.solve();
            
            System.out.println("\n=================================================\n");
            
            IloCplex.Status status = cplex.getStatus();
            System.out.println("Solution Status: " + status.toString());
            
            
            double objVal = cplex.getObjValue();
            System.out.println("Objective Value: " + objVal);
            this.objValue = objVal;
            
            
            if (!status.equals(IloCplex.Status.Feasible ) && !status.equals(IloCplex.Status.Optimal))
            {
                this.objValue = -1;
                System.err.println("Solution is not feasible or Optimal:");
                System.exit(-1);
            }
            else
            {
                //Retrieve results and store in correct object
                double[] solnIntervals = cplex.getValues(intVars);
                double[] solnReferences = cplex.getValues(refVars);
                double[] solnVariants = cplex.getValues(varVars);
                
                
                for (int i = 0; i < solnIntervals.length; i++)
                {
                    long val = Math.round(solnIntervals[i]);
                    IntAdjEdge curInterval = intervalEdges.get(i);
                    curInterval.count = (int)val;
                }
                
                for (int i = 0; i < solnReferences.length; i++)
                {
                    long val = Math.round(solnReferences[i]);
                    IntAdjEdge curRef = referenceEdges.get(i);
                    curRef.count = (int) val;
                }
                
                for (int i = 0; i < solnVariants.length; i++)
                {
                    long val = Math.round(solnVariants[i]);
                    IntAdjEdge curVariant = variantEdges.get(i);
                    curVariant.count = (int) val;
                }
                
                if (HANDLE_TELOS)
                {
                    double[] solnSources = cplex.getValues(teloVars);
                    for (int i = 0; i < solnSources.length; i++)
                    {
                        long val = Math.round(solnSources[i]);
                        IntAdjNode curNode = telomereNodes.get(i);
                        
                        if (val > 0)
                        	curNode.unbalanced = true;
                     
                    }
                }    
                    
            }
        }//end try
        catch (IloException e)
        {
        	System.out.println("Concert exception caught: " + e);
        	e.printStackTrace();
        	System.exit(-1);
        }
		
	}
	
	public void printSolution()
	{
		String fileName = OUTPUT_PREFIX + ".results";
		graph.printSolution(fileName, this.HANDLE_TELOS);
	}
	
	
	class Interval implements Comparable<Interval>
	{
		//VARIABLES
		int chrm;
		long start;
		long end;
		String ID;
		
		long numTumor = 0;
		long numNormal = 0;
		
		public Interval(int aChrm, long aStart, long anEnd, String anID)
		{
			chrm = aChrm;
			start = aStart;
			end = anEnd;
			ID = anID;
		}
		
		public long getLength()
		{
			return end - start + 1;
		}

		@Override
		/**
		 * Sort on: 1. chrm, 2. start, 3. end.
		 */
		public int compareTo(Interval other) 
		{
			if (chrm < other.chrm)
				return -1;
			else if (chrm > other.chrm)
				return 1;
			else if (start < other.start)
				return -1;
			else if (start > other.start)
				return 1;
			else if (end < other.end)
				return -1;
			else if (end > other.end)
				return 1;
			else
				return 0;
		}
	}
		

}
