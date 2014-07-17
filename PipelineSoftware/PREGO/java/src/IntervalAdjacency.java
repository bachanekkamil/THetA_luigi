import java.io.BufferedWriter;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

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

public class IntervalAdjacency 
{

	//VARIABLES
	public Map<String, IntAdjNode> nodes;
	public List<IntAdjNode> listOfNodes;
	public List<IntAdjNode> nonEndNodes;
	public List<IntAdjEdge> intervalEdges;
	public List<IntAdjEdge> variantEdges;
	public List<IntAdjEdge> refEdges;
	public List<IntAdjNode> telomereNodes;
	
	private final double ratioCutoff = 2;
	
	
	//CONSTRUCTORS
	public IntervalAdjacency()
	{
		nodes = new HashMap<String, IntAdjNode>();
        listOfNodes = new ArrayList<IntAdjNode>();
        nonEndNodes = new ArrayList<IntAdjNode>();
        intervalEdges = new ArrayList<IntAdjEdge>();
        variantEdges = new ArrayList<IntAdjEdge>();
        refEdges = new ArrayList<IntAdjEdge>();
        telomereNodes = new ArrayList<IntAdjNode>();
	}
	
	
	//METHODS
	public void updateTelomereNodes()
	{
        for (int i = 0; i < nonEndNodes.size(); i++)
        {
            IntAdjNode curNode = nonEndNodes.get(i);
            
            //Need to get interval edge across reference edge
            IntAdjEdge curRef = curNode.refEdge;
            IntAdjNode nodeOppRef = curRef.getOther(curNode);
            IntAdjEdge intOppCurNode = nodeOppRef.intEdge;
            IntAdjNode intEdgeOppNode = intOppCurNode.getOther(nodeOppRef);
            
            IntAdjEdge curInterval = curNode.intEdge;
            double curRatio = curInterval.getCoverageRatio();
            double compareRatio = intOppCurNode.getCoverageRatio();
            double ratioOfRatios = curRatio/compareRatio;
            
            //Check if this is an end node and ratio
            if (!nonEndNodes.contains(intEdgeOppNode) && (ratioOfRatios > ratioCutoff))
            {
            	telomereNodes.add(curNode);
                    
            }
        }//end loop on all nonEndNodes
	}//end updateTelomereNodes
	
	public boolean isTelomereNode(IntAdjNode aNode)
	{
		if (telomereNodes.contains(aNode))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	public void printSolution(String fileName, boolean handleTelos)
	{
		try
		{
            FileOutputStream fos = new FileOutputStream(fileName);
            Writer aWriter = new OutputStreamWriter(fos);
            BufferedWriter out = new BufferedWriter(aWriter);
            
            out.write("interval edges:\n");
            
            //Iterate through intervals
            
            for (int i = 0; i < intervalEdges.size(); i++)
            {
                    
                IntAdjEdge curEdge = intervalEdges.get(i);
                IntAdjNode node1 = curEdge.start;
                int chr1 = node1.chrm;
                long bp1 = node1.bp;
                IntAdjNode node2 = curEdge.end;
                int chr2 = node2.chrm;
                long bp2 = node2.bp;
                int count = curEdge.count;
                //out.append(node1.ID + " " + node2.ID + " " + count + "\n");
                out.append(node1.ID + "\t" + chr1 + "\t" + bp1 + "\t" + node2.ID + "\t" + chr2 + "\t" + bp2 + "\t" + count + "\n");
            }
            
            //Another new line
            out.newLine();
            out.append("ref edges:\n");
            
            //Iterate through reference edges
            for (int i = 0; i < refEdges.size(); i++)
            {
                IntAdjEdge curEdge = refEdges.get(i);
                IntAdjNode node1 = curEdge.start;
                IntAdjNode node2 = curEdge.end;
                long count = curEdge.count;
                //out.append(node1.ID + " " + node2.ID + " " + count + "\n");
                out.append(node1.ID + "\t" + node1.chrm + "\t" + node1.bp + "\t" + node2.ID + "\t" + node2.chrm + "\t" + node2.bp + "\t" + count + "\n");
               
            }
            
            //Another new line
            out.newLine();
            out.append("variant edges:\n");
            
            //Iterate through variant edges
            for (int i = 0; i < variantEdges.size(); i++)
            {
                IntAdjEdge curEdge = variantEdges.get(i);
                IntAdjNode node1 = curEdge.start;
                IntAdjNode node2 =  curEdge.end;
                long count = curEdge.count;
                //out.append(node1.ID + " " + node2.ID + " " + count + "\n");
                out.append(node1.ID + "\t" + node1.chrm + "\t" + node1.bp + "\t" + node2.ID + "\t" + node2.chrm + "\t" + node2.bp + "\t" + count + "\n");

            }
            
            //5-22/2013 - Remove source edges from output unless handle telos used
            
            if (handleTelos)
            {
	            //Another new line
	            out.newLine();
	            out.append("source edges:\n");
	            
	            
	            //Iterate through all nodes and print source
	            for (int i = 0; i < listOfNodes.size(); i++)
	            {
	            	IntAdjNode curNode = listOfNodes.get(i);
	            	int count = 0;
	            	if ( curNode.unbalanced)
	            		count = 1;
	                out.append(curNode.ID + "\t" + curNode.chrm + "\t" + curNode.bp + "\t" + count + "\n");
	            }
            }
            
            out.close();
            aWriter.close();
            fos.close();

		}//end try
		catch (IOException e)
		{
			System.out.println("Error! Unable to write to file: " + fileName);
			e.printStackTrace();
			System.exit(-1);
		}
	}
	
	public void printGraphML(String fileName)
	{
		String[] nodeColors = {"#FFC0CB","#FF1493","#9400D3","#7A67EE","#6495ED","#C6E2FF","#1E90FF","#00B2EE","#00E5EE","#00FA9A","#43CD80","#00FF00","#6B8E23","#C0FF3E","#CDCDC1","#FAFAD2","#FFFF00","#FFD700","#B8860B","#FFA500","#FF6103","#B22222","#FF3030","#FFC0FF"};
		
		try
		{
            FileOutputStream fos = new FileOutputStream(fileName);
            Writer aWriter = new OutputStreamWriter(fos);
            BufferedWriter out = new BufferedWriter(aWriter);
            
            out.write(getHeader());
            
            //Iterate through and draw nodes
            int pos = 0;
            int lastChrm = 0;
            for (IntAdjNode aNode : listOfNodes)
            {
            	int chrm = aNode.chrm;
            	long bp = aNode.bp;
            	
            	if (lastChrm != chrm)
            	{
            		pos = 0; //reset at the beginning of every chromosome
            	}
            	
            	//Determine node type
            	String type = null;
            	if (!nonEndNodes.contains(aNode))
            		type = "telomeric";
            	else if (telomereNodes.contains(aNode) && aNode.unbalanced)
            		type = "unbalanced";
            	else
            		type = "internal";
            	
            	String nodeGraphML = getNodeGraphML(chrm, bp, type, pos);
            	out.write(nodeGraphML);
            	pos++;
            	lastChrm = chrm;
            }
            
            //Iterate through interval edges
            for (IntAdjEdge anEdge : intervalEdges)
            {
            	String edgeGraphML = drawEdgeGraphML(anEdge, "interval");
            	if (anEdge.count !=0)
            		out.write(edgeGraphML);
            }
            
            //Iterate through reference edges
            for (IntAdjEdge anEdge : refEdges)
            {
            	String edgeGraphML = drawEdgeGraphML(anEdge, "reference");
            	if (anEdge.count != 0)
            		out.write(edgeGraphML);
            }
            
            //Iterate through variant edges
            for (IntAdjEdge anEdge : variantEdges)
            {
            	String edgeGraphML = drawEdgeGraphML(anEdge, "variant");
            	if (anEdge.count !=0)
            		out.write(edgeGraphML);
            }
            
            
            out.write(getFooter());
            
            
            out.close();
            aWriter.close();
            fos.close();

		}//end try
		catch (IOException e)
		{
			System.out.println("Error! Unable to write to file: " + fileName);
			e.printStackTrace();
			System.exit(-1);
		}
	}
	
	public static String getHeader()
	{
		StringBuffer sb = new StringBuffer();
		sb.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		sb.append("<!-- Test file -->\n");
		sb.append("<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns/graphml\"\n");
		sb.append("\txmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n\t");
		sb.append("xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns/graphml\n");
		sb.append("\thttp://www.yworks.com/xml/schema/graphml/1.0/ygraphml.xsd\"\n");
		sb.append("\txmlns:y=\"http://www.yworks.com/xml/graphml\">\n");
		sb.append("\t<key id=\"d0\" for=\"node\" yfiles.type=\"nodegraphics\"/>\n");
		sb.append("\t<key id=\"d1\" for=\"node\" attr.name=\"BooleanValue\" ");
		sb.append("attr.type=\"boolean\"/>\n");
		sb.append("\t<key id=\"d2\" for=\"edge\" yfiles.type=\"edgegraphics\"/>\n");
		sb.append("\t<key id=\"d3\" for=\"edge\" attr.name=\"IntValue\" attr.type=");
		sb.append("\"int\"/>\n");
		sb.append("\t<key id=\"d4\" for=\"graph\" attr.name=\"StringValue\"");
		sb.append(" attr.type=\"string\"/>\n");
		sb.append("\t<graph id=\"Main\" edgedefault=\"directed\">\n");
		
		return sb.toString();
	}
	
	
	public static String getNodeGraphML(int chrm, long bp, String type, int pos)
	{
		String[] nodeColors = {"#FFC0CB","#FF1493","#9400D3","#7A67EE","#6495ED","#C6E2FF","#1E90FF","#00B2EE","#00E5EE","#00FA9A","#43CD80","#00FF00","#6B8E23","#C0FF3E","#CDCDC1","#FAFAD2","#FFFF00","#FFD700","#B8860B","#FFA500","#FF6103","#B22222","#FF3030","#FFC0FF"};
		
		String shape = null;
		if (type.equals("internal"))
			shape = "rectangle";
		else if (type.equals("telomeric"))
			shape = "trapezoid";
		else 
			shape = "oval";
		
		StringBuilder sb = new StringBuilder();
		String nodeLabel = chrm + ":" + bp;
		String nodeName = "nodeLoc" + nodeLabel;
		
		sb.append("\t\t<data key=\"d4\">nodes</data>\n");
		sb.append("\t\t<node id=\""+ nodeName  + "\">\n");
		sb.append("\t\t\t<data key=\"d0\">\n");
		sb.append("\t\t\t\t<y:ShapeNode>\n");
		sb.append("\t\t\t\t\t<y:Geometry x=\"");
		sb.append(pos*200);
		sb.append(".0\" y=\"");
		sb.append(chrm*100);
		sb.append(".0\" width=\"100.0\" height=\"20.0\"/>\n");
		sb.append("\t\t\t\t\t<y:Fill color=\"");
		sb.append(nodeColors[chrm-1]);
		sb.append("\" transparent=\"false\"/>\n");
		sb.append("\t\t\t\t\t<y:BorderStyle type=\"line\" width=\"1.0\"");
		sb.append(" color=\"#000000\"/>\n");
		sb.append("\t\t\t\t\t<y:NodeLabel x=\"-20\" y=\"18\">");
		sb.append(nodeLabel);
		sb.append("</y:NodeLabel>\n");
		sb.append("\t\t\t\t\t<y:Shape type=\"");
		sb.append(shape);
		sb.append("\"/>\n");
		sb.append("\t\t\t\t</y:ShapeNode>\n");
		sb.append("\t\t\t</data>\n");
		sb.append("\t\t</node>\n");
		
		return sb.toString();
	}
	
	
	public static String drawEdgeGraphML(IntAdjEdge anEdge, String type)
	{

		StringBuilder sb = new StringBuilder();
		String color = null;
		
		if (type.equals("interval"))
			color = "#FF0000";
		else if (type.equals("reference"))
			color = "#000000";
		else
			color = "#0000FF";
		
		String n1 = anEdge.start.chrm + ":" + anEdge.start.bp;
		String n2 = anEdge.end.chrm + ":" + anEdge.end.bp;
		
		//String ID = n1 + ":" + n2;
		//lko - 5/28/2013 Make sure that ID is unique so we don't overwrite
		String ID = type + ":" + n1 + ":" + n2;
		
		sb.append("\t\t<edge id=\"");
		sb.append(ID);
		sb.append("\" source=\"nodeLoc");
		sb.append(n1);
		sb.append("\" target=\"nodeLoc");
		sb.append(n2);
		sb.append("\">\n");
		sb.append("\t\t\t<data key=\"d2\">\n");
		
		if (type.equals("variant") && (!n1.equals(n2)))
			sb.append("\t\t\t\t<y:ArcEdge>\n");
		else
			sb.append("\t\t\t\t<y:PolyLineEdge>\n");
		sb.append("\t\t\t\t\t<y:LineStyle type=\"line\" width=\"1.0\" ");
		sb.append("color=\"");
		sb.append(color);
		sb.append("\"/>\n");
		sb.append("\t\t\t\t\t<y:Path sx=\"0.0\" sy=\"0");
		sb.append(".0\" tx=\"0.0\" ty=\"0");
		sb.append(".0\"/>\n");
		sb.append("\t\t\t\t\t<y:BendStyle smoothed=\"true\"/>\n");
		sb.append("\t\t\t\t\t<y:Arrows source=\"none\" target=\"");
		sb.append("none");
		sb.append("\"/>\n");
		sb.append("\t\t\t\t\t<y:EdgeLabel ");
		sb.append("y=\"4\" ");
		sb.append("visible=\"true\"");
		sb.append(" preferredPlacement=");
		sb.append("\"target\">");
		sb.append(anEdge.count);
		sb.append("</y:EdgeLabel>\n");
		
		if (type.equals("variant") && (!n1.equals(n2)))
			sb.append("\t\t\t\t</y:ArcEdge>\n");
		else
			sb.append("\t\t\t\t</y:PolyLineEdge>\n");
		sb.append("\t\t\t</data>\n");
		sb.append("\t\t</edge>\n");
			
		return sb.toString();
		
	}
	
	public static String getFooter()
	{
		StringBuilder sb = new StringBuilder();
		sb.append("\t</graph>\n");
		sb.append("</graphml>\n");
		
		return sb.toString();
		
	}
	
	
}



/**
 * This class represents a single node in the interval adjacency graph.
 * @author layla
 *
 */
class IntAdjNode
{
	//VARIABLES
	public String ID;
	public int chrm;
	public long bp;
	
	public IntAdjEdge intEdge; //interval edge containing this node
	public List<IntAdjEdge> varEdge; //variant edges containing this node
	public IntAdjEdge refEdge; //reference edge containing this node (if it exists)
	
	boolean internal = true;
	boolean unbalanced = false;
	
	/**
	 * Constructor
	 * @param anID
	 * @param aChrm
	 * @param aBP
	 */
	public IntAdjNode(String anID, int aChrm, long aBP)
	{
		ID = anID;
		chrm = aChrm;
		bp =aBP;
		
		intEdge = null;
		varEdge = new ArrayList<IntAdjEdge>();
		refEdge = null;
	}
	
	public IntAdjNode()
	{
		ID = null;
		chrm = 0;
		bp =0;
		
		intEdge = null;
		varEdge = new ArrayList<IntAdjEdge>();
		refEdge = null;
	}
	
	/**
	 * Returns true if the specified IntAdjNode is equal (same chrm, bp and ID) 
	 * as this IntAdjNode.
	 * @param aNode
	 * @return
	 */
	public boolean isEqual(IntAdjNode aNode)
	{
		
		if (this.ID != aNode.ID)
		{
			return false;
		}
		
		if (this.chrm != aNode.chrm)
		{
			return false;
		}
		
		if (this.bp != aNode.bp)
		{
			return false;
		}
		
		return true;
	}
}//End IntAdjNode




class IntAdjEdge
{
	//Variables
	public IntAdjNode start;
	public IntAdjNode end;
	public int count; //Number of copies of edge
	public String type; //only used for variant edges
	public String cluster; //only used for variant edges
	public int numPES; //only used for variant edges
	public double localization; //only used for variant edges
	public long tumorCoverage; //only used for interval edges
	public long normalCoverage;
	
	public IntAdjEdge(IntAdjNode n1, IntAdjNode n2)
	{
		start = n1;
		end = n2;
		
		count = 0;
		type = null;
		cluster = null;
		numPES = 0;
		localization = 0;
		tumorCoverage = 0;
		normalCoverage = 0;
	}
	
	
	//METHODS
	public IntAdjNode getOther(IntAdjNode aNode)
	{
		IntAdjNode other = null;
		if(aNode.equals(start))
		{
			other = end;
		}
		
		if (aNode.equals(end))
		{
			other = start;
		}
		
		return other;
	}
	
	public long getLength()
	{
		if (start.chrm == end.chrm)
		{
			return end.bp - start.bp + 1;
		}
		else
		{
			return 0;
		}
	}
	
	
	public double getCoverageRatio()
	{
		return (double)tumorCoverage/normalCoverage;
		
	}
	
}
