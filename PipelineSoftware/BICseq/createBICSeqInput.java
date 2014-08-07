import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.TreeMap;

/**
 * This class creates the input needed for BIC-seq from a provided pair of tumor and 
 * normal concordant files.
 * @author layla
 *
 */
public class createBICSeqInput 
{

	//VARIABLES
	ArrayList<TreeMap<Integer,String>> files; //0 is tumor, 1 is normal
	ArrayList<TreeMap<Integer,Boolean>> writtenYet;
	String parameterFile;
	HashMap<Integer, ArrayList<Long>> startLocs;
	
	String OUTPUT_PREFIX;
	Boolean ALL_CHRMS = true;
	int SINGLE_CHRM = 1;
	String TUMOR_CONC;
	String NORMAL_CONC;
	
	long MAX_NUMBER = 50000;
	
	/**
	 * @param args
	 */
	public static void main(String[] args) 
	{
	
		//Step 1. Validate and store input
		createBICSeqInput bic = new createBICSeqInput(args);
		
		//Step 2. Load data and write individual files
		bic.load(bic.TUMOR_CONC, 0);
		bic.load(bic.NORMAL_CONC, 1);
		
		//Step 3. Write config file for all
		bic.writeConfig();
		
	}
	
	public createBICSeqInput(String[] args)
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
	
	public boolean parseArguments(String[] args)
	{
		//Check for 2 or 3 input arguments
		if (args.length < 2)
		{
			System.err.println("Error! Incorrect number of arguments.");
			return false;
		}
		
		TUMOR_CONC=args[0];
		NORMAL_CONC=args[1];
		
		//For each pair of arguments (flag and value) set parameters
		for (int i=2; i< args.length; i+=2)
		{
			if (args[i].equalsIgnoreCase("-CHRM_ONLY"))
			{
				try
				{
					SINGLE_CHRM = Integer.parseInt(args[i+1]);
					ALL_CHRMS=false;
					
					if (SINGLE_CHRM <= 0)
						throw new Exception();
				}
				catch (Exception e)
				{
					System.out.println("Error! -CHRM_ONLY must be a non-negative integer.");
					return false;
				}
			}//end -CHRM_ONLY
			
			if (args[i].equalsIgnoreCase("-OUTPUT_PREFIX"))
			{
				OUTPUT_PREFIX = args[i+1];
			}//end -OUTPUT_PREFIX
		}//END Optional parameters
		
		//Initialize other data structures
		files = new ArrayList<TreeMap<Integer, String>>();
		files.add(new TreeMap<Integer, String>()); //tumor
		files.add(new TreeMap<Integer, String>()); //normal
		
		writtenYet = new ArrayList<TreeMap<Integer, Boolean>>();
		writtenYet.add(new TreeMap<Integer, Boolean>()); //tumor
		writtenYet.add(new TreeMap<Integer, Boolean>()); //normal
		
		startLocs = new HashMap<Integer, ArrayList<Long>>();
		
		return true;
	}
	
	/**
	 * Prints the usage information.
	 */
	public void printUsage()
	{
		System.out.println("\nProgram: createBICSeqInput");
		System.out.println("USAGE (src): java createBICSeqInput <TUMOR_FILE> <NORMAL_FILE> [Options]\n" +
				"USAGE (jar): java -jar createBICSeqInput <TUMOR_FILE> <NORMAL_FILE> [Options]\n" + 
				"<TUMOR_FILE> [String]\n" +
				"\t A concordant file for the tumor genome.\n" +
				"<NORMAL_FILE> [String]\n" +
				"\t A concordant file for the normal genome.\n" + 
				"-SINGLE_CHRM [Integer] \n" +
				"\t A single chromosome to consider.\n" +
				"-OUTPUT_PREFIX [String] \n" +
				"\t The output prefix to append to output files.");
	}
	
	
	/**
	 * Prints the arguments that are set.
	 */
	public void printArguments()
	{
		System.out.println("\n=====================================");
		System.out.println("Arguments are:");
		System.out.println("   TUMOR_FILE  = " + TUMOR_CONC);
		System.out.println("   NORMAL_FILE = " + NORMAL_CONC);
		System.out.println("   OUTPUT_PREFIX = " + OUTPUT_PREFIX);
		if (!ALL_CHRMS)
		{
			System.out.println("   SINGLE_CHRM =" + SINGLE_CHRM);
		}
		
		System.out.println("\n=====================================");
	}
	
	/**
	 * Loads in the data from a concordant file into the supplied
	 * datastructures.
	 * @param fileName
	 * @param idx of the fileMap (0 = tumor, 1 = normal)
	 */
	public void load(String fileName, int idx)
	{
		//clear old data
		startLocs = new HashMap<Integer, ArrayList<Long>>();
		
		TreeMap<Integer,String> fileMap = files.get(idx);
		String type = "tumor";
		String otherType = "normal";
		TreeMap<Integer, Boolean> writtenMap = writtenYet.get(idx);
		
		int otherIdx = 1;
		if (idx == 1)
		{
			otherIdx = 0;
			type = "normal";
			otherType = "tumor";
		}
		
		TreeMap<Integer,String> otherFileMap = files.get(otherIdx);
		TreeMap<Integer, Boolean> otherWrittenMap = writtenYet.get(otherIdx);
		
		try
		{
			FileInputStream fis = new FileInputStream(fileName);
			InputStreamReader isr = new InputStreamReader(fis);
			BufferedReader br = new BufferedReader(isr);
			
			//Iterate through the concordant file
			String curLine;
			while ((curLine = br.readLine()) != null)
			{
				String[] parts = curLine.split("\\s+");
				if (parts.length != 3)
				{
					System.out.println("Invalid line in file: " + fileName);
					System.out.println("Ignoring line and moving on.");
					continue;
				}
				
				int chrm = Integer.parseInt(parts[0]);
				long start = Long.parseLong(parts[1]);
				
				//Check if we can skip it
				if (!ALL_CHRMS && (chrm != SINGLE_CHRM))
					continue;
				
				//make sure both tumor and normal have chrm and set not yet written
				if (!fileMap.containsKey(chrm))
				{
					String outFile = OUTPUT_PREFIX + ".chr" + chrm + "." + type;
					fileMap.put(chrm, outFile);
					writtenMap.put(chrm, false);
				}
				
				if (!otherFileMap.containsKey(chrm))
				{
					String outFile = OUTPUT_PREFIX + ".chr" + chrm + "." + otherType;
					otherFileMap.put(chrm,outFile);
					otherWrittenMap.put(chrm, false);
				}
				
				//Add to correct type
				if (!startLocs.containsKey(chrm))
				{
					ArrayList<Long> chrmLocs = new ArrayList<Long>();
					startLocs.put(chrm, chrmLocs);
				}
				
				ArrayList<Long> curLocs = startLocs.get(chrm);
				curLocs.add(start);
				
				//Check if it is time to write to file
				if (curLocs.size() > MAX_NUMBER)
				{
					this.writeLocations(idx, false);
				}
				
			}//end iterate through lines in concordant file
			
			br.close();
			isr.close();
			fis.close();
			
		}//end try clause
		catch (IOException e)
		{
			System.err.println("Error!  Cannot read concordant file.");
			e.printStackTrace();
			System.exit(-1);
		}//end catch
		
		//Write all remaining elements
		this.writeLocations(idx, true);
		
	}//end load method
	
	
	public void writeLocations(int idx, Boolean writeAll)
	{
		for (int key : startLocs.keySet())
		{
			ArrayList<Long> curVals = startLocs.get(key);
			
			//Write if over half full
			if (curVals.size() > MAX_NUMBER/2 || writeAll)
			{
				writeLocations(idx, key);
				
				//Clear out data
				curVals = new ArrayList<Long>();
				startLocs.put(key, curVals);
			}
		}
	}
	
	public void writeLocations(int idx, int chrm)
	{
		String fileName = files.get(idx).get(chrm);
		ArrayList<Long> vals = startLocs.get(chrm);
		Boolean written = writtenYet.get(idx).get(chrm);
		
		try
		{
			//Use append by setting second parameter to true after first write
			FileOutputStream fos = new FileOutputStream(fileName, written);
			writtenYet.get(idx).put(chrm, true); //make sure set to true now
			Writer aWriter = new OutputStreamWriter(fos);
			BufferedWriter out = new BufferedWriter(aWriter);
			
			for (int i=0; i< vals.size(); i++)
			{
				out.write(Long.toString(vals.get(i)));
				out.newLine();
			}
			
			out.close();
			aWriter.close();
			fos.close();

		}
		catch (IOException e)
		{
			System.err.println("Error!  Cannot write to output directory.");
			e.printStackTrace();
			System.exit(-1);
		}
	}
	
	public void writeConfig()
	{
		String fileName = OUTPUT_PREFIX + ".config";
		
		try
		{
			//Use append by setting second parameter to true
			FileOutputStream fos = new FileOutputStream(fileName);
			Writer aWriter = new OutputStreamWriter(fos);
			BufferedWriter out = new BufferedWriter(aWriter);
			
			out.write("chrom" + "\t" + "tumor" + "\t" + "normal");
			out.newLine();
			
			for (int chrm : files.get(0).keySet())
			{
				String tumorFile = files.get(0).get(chrm);
				String normalFile = files.get(1).get(chrm);
				out.write(chrm + "\t" + tumorFile + "\t" + normalFile);
				out.newLine();
			}
			
			out.close();
			aWriter.close();
			fos.close();
		}
		catch (IOException e)
		{
			System.err.println("Error!  Cannot write to config file.");
			e.printStackTrace();
			System.exit(-1);
		}
	}

}
