/**
 * Copyright 2012 Benjamin Raphael, Suzanne Sindi, Anthony Cannistra, Hsin-Ta Wu, Luke Peng, Selim Onal
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
 *
 */

#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <vector>		   
#include <cstdlib>
#include <cstring>

using namespace std;

//itoa: Convert an integer to a string;
string itoa(int number)
{
    if (number == 0)
        return "0";
    string temp="";
    string returnvalue="";
    while (number>0)
    {
        temp+=number%10+48;
        number/=10;
    }
    for (int i=0;i<temp.length();i++)
        returnvalue+=temp[temp.length()-i-1];
    return returnvalue;
}

//String Tokenizer: Creates an array of strings from tokenized string
int split( vector<string> & theStringVector, const  string  & theString, const  string  & theDelimiter){
	int numFields = 0;
	size_t  start = 0, end = 0;
	
    while ( end != string::npos){
        end = theString.find( theDelimiter, start);
		
        // If at end, use length=maxLength.  Else use length=end-start.
        theStringVector.push_back( theString.substr( start, (end == string::npos) ? string::npos : end - start));
		
        // If at end, use start=maxSize.  Else use start=end+delimiter.
        start = (   ( end > (string::npos - theDelimiter.size()) )?  string::npos  :  end + theDelimiter.size());
		numFields++;
    }
	return numFields;
}

//String Tokenizer: Creates an array of integers from tokenized string
int splitToIntegers( vector<int> & theIntegerVector, const  string  & theString, const  string  & theDelimiter){
	int numFields = 0;
	size_t  start = 0, end = 0;
	
    while ( end != string::npos){
        end = theString.find( theDelimiter, start);
		
        // If at end, use length=maxLength.  Else use length=end-start.
		string tmpString = theString.substr( start, (end == string::npos) ? string::npos : end - start);
        theIntegerVector.push_back( atoi(tmpString.c_str()) );
		
        // If at end, use start=maxSize.  Else use start=end+delimiter.
        start = (   ( end > (string::npos - theDelimiter.size()) )?  string::npos  :  end + theDelimiter.size());
		numFields++;
    }
	return numFields;
}

/****************/
/* MAIN PROGRAM */
/****************/

int main(int argc, char* argv[]){


     if(argc != 3 && argc != 2){
                //                 0        1              2                      3              4       5         6       7        8       9           // 10
				cout << "\nbuildIntervalsGASVPro: Convert a set of GASV clusters (regions format) to intervals" << endl;
				cout << "Version: 1.0" << endl << endl;
				cout << "Usage: ./exe {clusterfile} {parameters (optional)} " << endl;
				cout << "\n";
                cout << "Parameters Considered:\n";
				cout << "           Limit: {value}    (Default: 1000)\n";
				cout << "  Translocations: {boolean}  (Default: FALSE)\n";
				cout << "  TransOnly:      {boolean}  (Default: FALSE)\n";
		 
				cout << "\n";
				cout << "Output: {clusterfile}.GASVPro.intervals\n";
				exit(-1);
	 }

	
	//SET DEFAULTS HERE
	int Limit = 1000;
	bool TRANSLOCATIONS_ON = false;
	bool TRANS_ONLY = false;
	
	//Step 0:
	cout << "Step 0: Processing Command Line Arguments.\n";
		
	if(argc == 3){
		cout << "|||INPUT PARAMETERS|||" << endl;
		string temp;
		ifstream p_file(argv[2], ios::in);
		if(p_file.is_open()){cout << "   Parameters file found." << endl;}
		else{cout << "ERROR: Parameters file \"" << argv[1] << "\" cannot be opened." << endl; exit(1);}
		while(getline(p_file, temp)){
			if(temp[0] == '#')
				continue;
			else
			{
				string term;
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
				}
				
				if(term == "Limit:"){
						Limit = atoi(value.c_str());
						cout << "   Limit: " << Limit << endl;
						continue;
				}
				if(term == "Translocations:"){
					if(value == "true" || value == "True" || value == "TRUE"){
						TRANSLOCATIONS_ON = true;
						cout << "   Translocations will be output." << endl;
					}
					else
						TRANSLOCATIONS_ON = false;
				}
				if(term == "TransOnly:"){
					if(value == "true" || value == "True" || value == "TRUE"){
						TRANS_ONLY = true;
						cout << "***WARNING: Only translocations will be output.***" << endl;
					}
				}

			}
		}
	}

	if(Limit <= 0){ cerr << "\n\t\tERROR: Limit must be positive\n";}
	cout << "\tSuccessfully Finished.\n" << flush;

	//Step 1: Process Clusters File;
	cout << "Step 1: Processing Clusters File.\n";
	
	int translocationCount = 0;
	int divergentCount = 0;	
	int inversionCount = 0;
	int deletionCount = 0;
	int numIgnored = 0;
	int numConsidered = 0;
	int numNegativeLocalization = 0;
	int numCorrectlyProcessed = 0;
	
	//Input File:
	string CLUSTERFILE = argv[1];
	ifstream clusterFile(CLUSTERFILE.c_str(),ios::in);
			
	string BPOUT = CLUSTERFILE + ".GASVPro.intervals";
	ofstream outFile(BPOUT.c_str(),ios::out);
	
	outFile << "#IntervalID\tChromosome\tStart\tEnd" << endl << flush;
		
	string clusterLine;
	string clusterID;
	int chrL;
	int chrR;
	int start; 
	int end;
	double localization;
	string type;	
	int localType; //Tracks which model
	
	bool tooManyFieldsWarning = false;
	bool processCluster;
	int numLines = 0;
	
	int numCasesBeforePrint = 1000; //print every numCasesBeforePrint cluster ids
	
	//Example Line:
	//0     1     2     3          4            5   6            7
	//c1	1	204.2	D	SRR004856.7363154	1	1	746128, 748510, 746333, 748510, 746027, 748204, 746027, 748409
	while(getline(clusterFile,clusterLine) ){
		
		/////////////////////////
		// Read and Parse Line //
		/////////////////////////
		
		vector<string> v;

		int numFields = split( v, clusterLine.c_str(), "\t" );	
		if(numFields > 8 && !tooManyFieldsWarning){
			cout << "\tWarning: There are more fields than expected in a regions output. Assuming extra columns.\n";
			tooManyFieldsWarning = true;
		}
		if(numFields < 8){
			cout << "\tError: GASVPro requires clusters are in regions format. Clusters file has only " << numFields << " columns instead of 8.\n";
			exit(-1);
		}
		
		if(v[0].substr(0,1) == "#")
			continue;
		
		clusterID    = v[0];
		localization = atof(v[2].c_str());
		type         = v[3];
		chrL         = atoi(v[5].c_str());
		chrR         = atoi(v[5].c_str());
		
		if(numLines%numCasesBeforePrint == 0 && numLines>0 ){ 
			cout << "\t\t\tProcessing Cluster " << numLines << "\t"  << clusterID << endl;
			time_t now;
			struct tm *current;
			time(&now);
			current = localtime(&now);
			printf("\t\t\tTime:\t%i:%i:%i\n", current->tm_hour, current->tm_min, current->tm_sec);
		}
	
		///////////////////////////////////////
		// Determining If we Process Cluster //
		///////////////////////////////////////
		
		//Q: What type of variant is this cluster?
		localType = -1;
		if(strcmp(type.c_str(),"D") == 0){ localType = 0; deletionCount++; }
		else if(strcmp(type.c_str(),"I+") == 0 || strcmp(type.c_str(),"I-") == 0 || strcmp(type.c_str(),"IR") == 0){ localType = 1; inversionCount++; }
		else if(strcmp(type.c_str(), "V") == 0) { localType = 2; divergentCount++;  } //Default ignoring divergents;
		else if(type.substr(0,1) == "T"){ localType = 3; translocationCount++;  } //Default: ignore translocations;
		
		//Q: Do we process this cluster?
		if(localType == 0 || localType == 1){ processCluster = true;}
		else if(localType == 2){ processCluster = false; } //Skip divergents totally;
		else if(localType == 3){ 
			if(TRANSLOCATIONS_ON){ processCluster = true; }
			else{ processCluster = false;}
		}
		else{ 
			cout << "\tError: Invalid local type in cluster file; exiting program.\n";
			cout << clusterLine << endl;
			exit(-1);
		}

		//If translocation only selected! All 
		if(TRANS_ONLY == true && localType!= 3 ){ processCluster = false; }		

		//Skip clusters we don't want to process; 
		if(processCluster == false){
			numIgnored++;
			continue;
		}
				
		//Skip clusters with negative localization;
		numConsidered++;
		if(localization < 0){ 
			numNegativeLocalization++;
			continue;
		}
		
		////////////////////////
		// Processing Cluster //
		////////////////////////
		
		numCorrectlyProcessed++;
		
		vector <int> bothCoordinates;
		int totalCoords = splitToIntegers(bothCoordinates,v[7],",");
		int minX, maxX, minY, maxY;
		int numCoords = totalCoords/2;
		minX = maxX = bothCoordinates[0];
		minY = maxY = bothCoordinates[1];
		for(int k = 2; k<totalCoords; k++){
			if(k%2 == 0){
				if(bothCoordinates[k]<minX){ minX = bothCoordinates[k];}
				if(bothCoordinates[k]>maxX){ maxX = bothCoordinates[k];}
			}
			else{
				if(bothCoordinates[k]<minY){ minY = bothCoordinates[k];}
				if(bothCoordinates[k]>maxY){ maxY = bothCoordinates[k];}
			}
		}
		
		//Output an interval IF we are 
		if( localType == 0 && (minY - maxX) >= Limit){
			string intervalID = "I_" + itoa(numLines) + "_" + clusterID; //ID is *both* the line number and cluster ID;
			outFile << intervalID << "\t" << chrL << "\t" << maxX << "\t" << minY << endl;
		}
		//We are doing breakendReadDepth
		else{
			string intervalID = "P_" + itoa(numLines) + "_" + clusterID;
			outFile << intervalID << "\t" << chrL << "\t" << minX << "\t" << maxX << endl;
			outFile << intervalID << "\t" << chrR << "\t" << maxY << "\t" << minY << endl;
			
			//for(int X = minX; X<=maxX; X++){
			//	outFile << intervalID << "\t" << chrL << "\t" << X << "\t" << X << endl;
			//}
			//for(int Y = minY; Y<=maxY; Y++){
			//	outFile << intervalID << "\t" << chrR << "\t" << Y << "\t" << Y << endl;
			//}
		}
		
		numLines++;

	}
	
	clusterFile.close();
	outFile.close();
	cout << "\tSuccessfully Finished.\n";
	
	cout << endl;
	cout << "Summary Statistics\n";
	cout << "-------------------------------------\n";
	cout << "Deletion Clusters       := " << deletionCount << endl;
	cout << "Inversion Clusters      := " << inversionCount << endl;
	cout << "Divergent Clusters      := " << divergentCount << endl;
	cout << "Translocation Clusters  := " << translocationCount << endl;

	cout << endl;
	cout << "TotalClustersIgnored    := " << numIgnored << endl;
	cout << "TotalClustersConsidered := " << numConsidered << endl;
	cout << endl; 
	cout << "TotalWithNegativeLoc    := " << numNegativeLocalization << endl;
	cout << "TotalCorrectlyProcessed := " << numCorrectlyProcessed << endl;
	cout << endl;
		
	return 0;	
}
