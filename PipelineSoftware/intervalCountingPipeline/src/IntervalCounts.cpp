/**
 * Copyright 2012 Anthony Cannistra
 *
 *  This file is part of the GASVPro code distribution.
 * 
 *  GASVPro is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  GASVPro is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with gasv.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "IntervalTree.h"
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

struct CHelper{
    string interval_id;
	//int coverage;
	int local_chr;
	bool divergent;
	bool point;
	vector<int> coverage;
};


int main(int argc, char* argv[])
{
	string INTERVALFILE;
    int MAXCHROMOSOME = 100;
	bool GASVPro = false;
	bool PREGO = false;
    bool checkIntervals = true;
    bool checkPoints = true;
	bool overlap_mode = true;
	bool contained_mode = false;
    vector<string> concordantFiles;
    
	vector<Interval<CHelper*> > intervals;
	IntervalTree<CHelper*> concordanceTree;

	vector<pair<int,int> > beRDIntervals;
	vector<pair<int,int> > relevantGenome;



	if(argc != 2){cout << "IntervalCounts.\tUsage: ./IntervalCounts </path/to/parametersfile>\n"; exit(1);}
	string temp;
	ifstream p_file(argv[1], ios::in);
	cout << "\n\n|||INPUT PARAMETERS|||" << endl;
	if(p_file.is_open()){cout << "\tParameters File found." << endl;}
	else{cout << "ERROR: Parameters file \"" << argv[1] << "\" cannot be opened." << endl; exit(1);}
	while(getline(p_file, temp))
	{
		if(temp[0] == '#')
			continue;
		else
		{
			string term;//BEGIN OVERLAP MODE
		
		//END OVERLAP MODE
			string value;
			int spacePos = temp.find(' ');
            if(spacePos == -1)
            {
                spacePos = temp.find(':');
                value = temp.substr(spacePos+1);
                term = temp.substr(0,spacePos+1);
            }
            else{
			  	value = temp.substr(spacePos+1);
			  	term = temp.substr(0,spacePos);
		      	
                if(term == "IntervalFile:"){
                    INTERVALFILE = value;
                    cout << "\tIntervals File: " << INTERVALFILE << endl;
                }
                if(term == "Software:"){
                    if(value == "GASVPro")
                    {
                        GASVPro = true;
                        checkPoints = true;
                        checkIntervals = true;
                        overlap_mode = true;
                        cout << "\tGASVPro mode active." << endl;}
                    if(value == "PREGO")
                    {
                        PREGO = true;
                        checkPoints = false;
                        checkIntervals = true;
                        contained_mode = true;
                        overlap_mode = false;
                        cout << "\tPREGO mode active." << endl;}
                }
                if(term == "Comparison:"){
                    if(value == "overlap" || value == "Overlap"){overlap_mode = true; contained_mode = false; cout << "\tOverlap mode active." << endl;}
                    if(value == "Containment" || value == "containment"){contained_mode = true; overlap_mode = false; cout << "\tContainment mode active." << endl;}
                }
                if(term == "ConcordantFile:"){
                    concordantFiles.push_back(value);
                }
                if(term == "MapabilityTrack:"){
                    cout << "\tWARNING: MAPABILITY/UNIQUENESS NOT SUPPORTED. (yet)" << endl;
                    //ADD UNIQUENESS
                }
                if(term == "GCTrack:"){
                    cout << "\tWARNING: MAPABILITY/UNIQUENESS NOT SUPPORTED. (yet)" << endl;
                    //ADD GC
                }
                if(term == "MaxChrNumber:"){
                    MAXCHROMOSOME = atoi(value.c_str());
                }
                
            }	    
        }
    }
    cout << "|||INPUT PARAMETERS|||\n" << endl;

    
    ifstream interval_file(INTERVALFILE.c_str(), ios::in);
    if(!(interval_file.is_open())){cout << "\tError! Cannot open interval file \"" << INTERVALFILE << "\". Aborting." << endl; exit(1);}
    interval_file.close();
    
    cout << "STEP 1: Storing intervals and points" << endl;
    
    //BEGIN Temporary Variables
    string interval_id;
    int chr;
    int start;
    int end;
    int sorted_start;
    int sorted_end;
    
    interval_file.open(INTERVALFILE.c_str(), ios::in);
    interval_file.ignore(200, '\n');
    while((interval_file >> interval_id >> chr >> start >> end))
    {
        sorted_start = min(start, end);
        sorted_end = max(start,end);
        if(interval_id[0] == 'P' && checkPoints){
            //looking at POINTS-BASED interval
            for(int i = sorted_start; i <= sorted_end; i++)
            {
                CHelper* temp = new CHelper;
                temp->interval_id = interval_id;
				temp->local_chr = chr;
                for(int j = 0; j < concordantFiles.size(); j++)
				{
						temp->coverage.push_back(0);
				}
				temp->point = true;
                intervals.push_back(Interval<CHelper*>(i, i, temp));
            }
        }
        else
        {
            CHelper* temp = new CHelper;
            temp->interval_id = interval_id;
			temp->local_chr = chr;
            for(int j = 0; j < concordantFiles.size(); j++)
			{
					temp->coverage.push_back(0);
			}
			temp->point = false;
            intervals.push_back(Interval<CHelper*>(sorted_start,sorted_end,temp));
        }
        
        
        
        
        
    }
    cout << "\tBuilding intervalTree structure...";
	concordanceTree = IntervalTree<CHelper*>(intervals);
	cout << "done." << endl;
    cout << "\tTotal : " << intervals.size() << endl;
    interval_file.close();
    
	//BEGIN concordance calculation
    cout << "STEP 2: Analyzing Concordance" << endl;
	
	//Temporary Variables
	int tmpChr, tmpStart, tmpEnd;
    
    for(int f = 0; f < concordantFiles.size(); f++)
    {
		ifstream ccFile(concordantFiles[f].c_str(), ios::in);
		int lineCount = 0;
		cout << "\tReading concordant file \"" << concordantFiles[f] << "\"" << flush;		
		if(!ccFile.is_open()){cout << "\tError! Concordant file \"" << concordantFiles[f] << "\" cannot be opened." << endl; continue;}
		//ccFile.ignore(255, '\n'); //ignore first header line. DANGER if there's no header.
		
		//Overlap Mode
		if(overlap_mode && (!contained_mode))
		{
			
			
			while(ccFile >> tmpChr >> tmpStart >> tmpEnd)
			{
// 				vector<Interval<CHelper*>* > test;
// 				concordanceTree.findOverlapping(tmpStart, tmpEnd+1, test);
// 				int OVER = test.size();
// 				if(test.size() != 0) cout << "\tOVERLAP (" << tmpStart << " " << tmpEnd << ") NUM: " << test.size() << endl;
// 				for(int i = 0; i < test.size(); i++)
// 				{
// 						cout << "\t\t" << test[i]->start << " " << test[i]->stop << endl;
// 				}
// 				test.clear();
// 				concordanceTree.findContained(tmpStart, tmpEnd, test);
// 				int CONT = test.size();
// 				if(test.size() != 0) cout << "\tCONTAIN (" << tmpStart << " " << tmpEnd << ")NUM: " << test.size() << endl;
// 				for(int i = 0; i < test.size(); i++)
// 				{
// 						cout << "\t\t" << test[i]->start << " " << test[i]->stop << endl;
// 				}
// 				if(OVER != 0 && CONT != 0 && OVER > CONT){cout << "NOTICE" << endl << endl;}
				lineCount++;
				vector<Interval<CHelper*>* > overlap;
				concordanceTree.findOverlapping(tmpStart, tmpEnd, overlap);
				for(int o = 0; o < overlap.size(); o++)
				{
					//cout << "Overlap: " << overlap[o]->value->coverage[f] << endl;
					if(tmpChr == overlap[o]->value->local_chr){overlap[o]->value->coverage[f]++;}
					//cout << overlap[o]->value->coverage[f] << "\n\n" << flush;
				}
			}
		}
		
		//Contained Mode
		else if(contained_mode && (!overlap_mode))
		{
            while(ccFile >> tmpChr >> tmpStart >> tmpEnd)
			{
				//vector<Interval<CHelper*>* > contained;
				lineCount++;
				vector<Interval<CHelper*>* > contained;
				concordanceTree.findOverlapping(tmpStart, tmpEnd, contained);
				for(int c = 0; c < contained.size(); c++)
				{
					//cout << contained[c]->value->coverage[f] << endl;
					if(tmpChr == contained[c]->value->local_chr && 
                       contained[c]->start <= tmpStart &&
                       contained[c]->stop >= tmpEnd)
                    {contained[c]->value->coverage[f]++;}
					//cout << contained[c]->value->coverage[f] << "\n\n" << flush;
				}
			}	
		}
		else{cout << "\nFatal Error! Overlap/Contained mode not set properly. Choose one." << endl; exit(1);}
		cout << "...found " << lineCount << " lines." << endl << flush;

    }
    
    //END concordance calculation 
    
    cout << "STEP 3: Write concordance." << endl;
	
	string outFileName = INTERVALFILE+"_processed";
	ofstream outFile(outFileName.c_str(), ios::out);
    interval_file.open(INTERVALFILE.c_str(), ios::in);
	interval_file.ignore(255, '\n');
	outFile << "#IntervalID\tChr\tStart\tEnd";
    for(int f = 0; f < concordantFiles.size(); f++)
    {
        outFile << "\tCoverage:" << concordantFiles[f];
    }
    outFile << endl;
	while(interval_file >> interval_id >> chr >> start >> end)
	{
		vector<Interval<CHelper*>* > search;
		int real_start = min(start, end);
		int real_end = max(start, end);
		if(checkPoints)
        {
            if(interval_id[0] == 'P')
            {
                concordanceTree.findContained(real_start, real_end, search);
                outFile << interval_id << "\t" << chr << "\t" << start << "\t" << end << "\t";
                for(int f = 0; f < concordantFiles.size(); f++)
                {
                    for(int s = 0; s < search.size(); s++)
                    {
                        if(search[s]->value->interval_id == interval_id &&
                           search[s]->value->local_chr == chr)
                        {
                            outFile << search[s]->value->coverage[f] << ":";
							
                        }
                    }
                    long pos = outFile.tellp();
                    outFile.seekp(pos-1);
                    outFile.write(" ", 1);
                    outFile << "\t";
                }
                outFile << endl;
            }
            
        }
        if(checkIntervals && checkPoints)
        {
            if(interval_id[0] == 'I')
            {
                concordanceTree.findContained(real_start, real_end, search);
                for(int s = 0; s < search.size(); s++)
                {
                    if(search[s]->value->interval_id == interval_id &&
                       search[s]->value->local_chr == chr &&
                       search[s]->start == real_start &&
                       search[s]->stop == real_end)
                    {
						outFile << interval_id << "\t" << chr << "\t" << start << "\t" << end << "\t";
						for(int f = 0; f < concordantFiles.size(); f++)
						{
							outFile << search[s]->value->coverage[f] << "\t";
						}
						outFile << endl;
                    }
                }
            }
        }
        if(checkIntervals && !checkPoints)
        {
            concordanceTree.findContained(real_start, real_end, search);
            for(int s = 0; s < search.size(); s++)
            {
                if(search[s]->value->interval_id == interval_id &&
                   search[s]->value->local_chr == chr &&
                   search[s]->start == real_start &&
                   search[s]->stop == real_end)
                {
                    outFile << interval_id << "\t" << chr << "\t" << start << "\t" << end << "\t";
                    for(int f = 0; f < concordantFiles.size(); f++)
                    {
                        outFile << search[s]->value->coverage[f] << "\t";
                    }
                    outFile << endl;
                }
            }
        }
        
		
	}
	cout << "\tDone. Filename: " << INTERVALFILE+"_processed" << endl;
	
	cout << "IntervalCounts Complete.\n\n" << endl;
    
    /*for(int current_chr = 0; current_chr <= MAXCHROMOSOME; current_chr++)
    {
        interval_file.open(INTERVALFILE.c_str(), ios::in);
        interval_file.ignore(200, '\n');
        while((interval_file >> interval_id >> chr >> start >> end))
        {
            if(chr != current_chr){continue;}
            sorted_start = min(start, end);
            sorted_end = max(start,end);

            if(interval_id[0] == 'P'){
                //looking at POINTS-BASED interval
                for(int i = sorted_start; i <= sorted_end; i++)
                {
                    CHelper* temp = new CHelper;
                    temp->interval_id = interval_id;
                    temp->coverage = 0;
                    intervals.push_back(Interval<CHelper*>(i, i, temp));
                }
            }
            else
            {
                CHelper* temp = new CHelper;
                temp->interval_id = interval_id;
                temp->coverage = 0;
                intervals.push_back(Interval<CHelper*>(sorted_start,sorted_end,temp));
            }
            
            
            
            
            
        }
        cout << "Intervals after chr " << current_chr << ": " << intervals.size() << endl;
        interval_file.close();
    }*/
    
    
    

    
    
}
