/*
This program is designed to convert the .txt file generated by the web crawler into a formatted .csv file for use by the analyzer.

The program will have multiple stages:

1. Take in and read all data from a .txt file

2. Write data out as a formatted .csv

3. Present menu to allow different stages of data cleaning before writing to .csv

4. Write function that will parse data from full URL to domain URL when corresponding choice is made

5. Write function that will remove tier listings from data vector when corresponding choice is made 

6. Write separate function to write just to text file with data as-is

7. Add option to menu to print size of data set

8. Ask before removing blank lines

9. Fix write_csv to work for links that don't start with http

10. Make write_html function to allow data to be written as raw html file for browsing links

11. Make function to convert duplicates/tocrawl files to csv with occurrence counters

12. Make function to get occurrences of links in duplicates/tocrawl files

13. Make function to check if link already counted

14. Rewrite write_csv() for new format need for analyzer

15. FIx bug B1


Current Status:
1. Complete
2. Complete
3. Complete
4. Cancelled
5. Cancelled
6. Complete
7. Complete
8. Complete
9. Complete
10. Complete
11. Complete
12. Complete
13. Complete
14. Complete (?)
15. Queued

BUGS:
	B1: Currently prints data backwards due to the way data is inserted into vector
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>

using namespace std;

//global variables

//vector<string> type for crawled conversion
typedef vector<string> line;

//prototypes

//This function will read all data from the input file and store it in a vector of strings
void read(ifstream&, vector <string>&);

//This function will check if link has already been counted
bool find_link(string&, vector<string>&);

//function to find if tier line has been printed as link and determine depth to lines belonging to the tier
int find_tier(vector<line>&, string&, int&);

//function to fill empty spots in line with NA strings
void fill_lines(vector<line>&);

//This function will write all data in the vector of strings to the .csv file
void write_csv(ofstream&, vector <string>&);

//This function will write all data to a text file
void write_txt(ofstream&, vector <string>&);

//This function will write all data to a html file
void write_html(ofstream&, vector<string>&);

//This function will convert duplicates/tocrawl files to csv (call count function first)
void convert_csv(ofstream&, vector<string>&, vector<int>&);

//This function with get occurrence counters for duplicate/tocrawl files
void get_occur(vector<string>&, vector<string>&, vector<int>&);

//This function will remove all empty newlines from data set
void remove_blanks(vector <string>&);

//Debug function to print all in the vector
void print(vector <string>&);

int main()
{
	ifstream input;
	ofstream output;

	string ifile; //string to hold name of input file
	string ofile; //string to hold name of output file

	vector<string> data;
	vector<int> occur;
	vector<string> counted_data;

	int choice = 0;
	char decide;
	
	int file_type = -1;

	cout << "Converter program for Web Crawler output data." << endl;
	cout << "Program will ask for an input file generated by the web crawler (typically links.txt)." << endl;
	cout << "A menu will be presented asking what type of file is being converted to .csv." << endl;
	cout << "If a crawled file, blank lines will be removed before conversion." << endl << endl;


	//get name of input file and open
	cout << "Enter name of input file: ";
	cin >> ifile;
	
	input.open(ifile.c_str());

	if (!input)
	{
		cout << "Could not open file" << endl;
		exit(2);
	}
	////////


	read(input, data); //read data
	
	cout << "Which file is this? " << endl;
	cout << "1. Crawled file" << endl;
	cout << "2. Duplicates/Tocrawl file" << endl;
	cout << "Choice: ";
	cin >> file_type;
	
	switch(file_type)
	{
		case 1: //if file contains list of crawled links
		{
			remove_blanks(data); //basic cleaning
			write_csv(output, data);
			break;
		}
		case 2: //for duplicate or tocrawl files
		{
			cout << "Converting to .csv file with occurrence counters." << endl;
			get_occur(data, counted_data, occur);
			convert_csv(output, counted_data, occur);
			break;
		}
		default:
		{
			cout << "Not a choice." << endl;
			break;
		}
		
	}//end file_type switch
	
//close files
	input.close();
	output.close();
	cout << "Files closed." << endl;
//pause for review
	cin.ignore();
	cin.ignore();
	return 0;
}

//function read all data from file into vector of strings
void read(ifstream& input, vector<string> &data)
{
	string t;
	getline(input, t); //prime
	while(!input.eof()) //loop until end of file
	{
		data.push_back(t); //add line to vector
		getline(input, t);  //gets each line and stores into data vector
	}
}

int find_tier(vector<line> &lines, string &key, int &insert_pos)
{
	string link;
	int pos;
	bool space = false;
	insert_pos = -1;
	
	//separate actual link from tier line by identifying spaces
	for (int i = 0; i < key.size(); i++)
	{
		if (key[i] == ' ' && space == false) //if first space is found, set flag
		{
			space = true;
		}
		else if (key[i] == ' ' && space == true) //if second space is found, link has been found
		{
			pos = i + 1;
			space = false;
			break; //no need to look more
		}
	}
	
	int NA_count = 1;
	
	//take link substring from tier line
	link = key.substr(pos, (key.size() - pos));
	
	for (int i = 0; i < lines.size(); i++) //for all vector in lines vector
	{
		for (int j = 0; j < lines[i].size(); j++) //for all strings in current subvector
		{
			if (lines[i][j].find(link) != string::npos) //if key link is found
			{
				insert_pos = i + 1;
				for ( int k = 0; k < lines[i][j].size(); k++) //for all strings in current subvector
				{
					if (lines[i][k] == "NA,") //if string is NA
					{
						NA_count++; //increment count of NA strings
					}
				}
			}
		}
	}
	
	return NA_count;
}

//function writes data stored in vector to formatted csv file
void write_csv(ofstream& output, vector<string> &data)
{
	string ofile;
	
	//get name of output file and open
	cout << "Enter name of output file (name.csv): ";
	cin >> ofile;

	output.open(ofile.c_str());
	
	
	vector<line> lines;
	
	string amount;
	int true_amount = 0; //holds int value of lines to write based on tier
	int NA_count = 1; //holds number of NA strings to print
	int insert_pos; //holds value of where to insert into lines vector
	
	for (int i = 0; i < data.size(); i++)
	{
		if (i == 0) //currently at origin, write as is
		{
			line origin;
			origin.push_back(data[i]);
			lines.push_back(origin);
		}
		else if (isdigit(data[i][0]) && (data[i].find(" from ") != string::npos)) //if at a tier line
		{
			for (int j = 0; j < data[i].size(); j++)
			{
				if (data[i][j] != ' ')
				{
					amount.push_back(data[i][j]); //get numerical value for tier
				}
				else
				{
					break; //once a space is found, break from loop
				}
			} //end for
			
			NA_count = find_tier(lines, data[i], insert_pos); //get amount of NA strings to print before data value
			
			true_amount = atoi(amount.c_str()); //cast found value from string to int
			amount.clear(); //clear the string for next tier
		} //end else if
		else //if a data line
		{
			if (true_amount != 0) //only if at least one line for the tier
			{
				int j;
				for (j = i; j < (i + true_amount); j++) //get the next set of data lines
				{
					if (isdigit(data[j][0]) && (data[j].find(" from ") != string::npos)) //if another tier line is found
					{
						break; //leave the loop
					}
					else //found a data line
					{
						line l;
						//print number of NA strings determined by find_tier()
						for (int n = 0; n < NA_count; n ++)
						{
						l.push_back("NA,");
						}
						//print data value
						l.push_back(data[j]);
						//insert line based on insert_pos value
						if (insert_pos == -1) //for no tier line (should never trigger)
						{
						lines.push_back(l);
						}
						else //for lines with a tier
						{
							lines.insert(lines.begin() + insert_pos, l);
						}
					}
				}
				i = j - 1; // needed to prevent duplicates in output
			}//end if
		} //end else
	} //end for
	
	fill_lines(lines);
	
	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = 0; j < lines[i].size(); j++)
		{
			output << lines[i][j];
		}
		if (i != (lines.size() -1)) //only output new line if not at last line
		{
			output << endl;
		}
	}
	
	cout << "Write successful." << endl;
}

//function removes blank lines from data set
void remove_blanks(vector<string> &data)
{
	cout << "Removing blank lines from data." << endl;

	bool removed = false;
	int lines= 0;
//search data vector for blank lines and remove them until none are found
	do
	{
		removed = false;
		for(int i = 0; i < data.size(); i++)
		{
			if(data[i] == "")
			{
				//cout << "Found" << endl;
				data.erase(data.begin() + i);
				removed = true;
				lines++;
			}
		}
	}while(removed);

	//data.erase(data.begin() + (data.size() - 1)); //removes trailing line
//output how many blank lines were removed (no real purpose, just fyi)
	cout << "Blank lines removed: " << lines << endl;
}

//function to print data set to screen (for debugging purposes)
void print(vector<string> &data)
{
	for (int i = 0; i < data.size(); i++)
	{
		cout << data[i] << endl;
	}
}

//function to determine if current link has already been tallied
bool find_link(string &to_find, vector<string> &check)
{
	for (int i = 0; i < check.size(); i++)
	{
		if (to_find == check[i]) //if link is already in counted vector...
		{
			return true;
		}
	}
	return false; //if link is not already in counted vector...
}

//function to get the occurrence numbers of each link in the file
void get_occur(vector<string> &data, vector<string> &counted, vector<int> &occur)
{
	bool found = false;
	int c = 0;
	string t;
	
	for (int i = 0; i < data.size(); i++)
	{
		found = find_link(data[i], counted); //check if current link has already been counted
		
		if (!found) //if not counted yet
		{
			counted.push_back(data[i]); //append link to vector
			t = data[i]; //store current link
			for (int j = 0; j < data.size(); j++)
			{
				if (data[j] == t) //total up each occurrence of the link
				{
					c++;
				} //close if
			} //close for
		}//close if
		
		if (c != 0) //if at least once instance is present...
		{
			occur.push_back(c); //append link counter to vector
			c = 0; //reset temp counter
		}
	}//close for
}//close occur

//This function takes in the vector of unique links and the occurrence of each to write to a csv file
void convert_csv(ofstream &output, vector<string> &data, vector<int> &occur)
{
	string fname;
	int comma_pos = string::npos; //initialize position for locating url commas
	
	cout << "Enter file name: (name.csv)";
	cin >> fname;
	
	output.open(fname.c_str());
	
	output << "Link," << "Occurrence" << endl; //write column headers
	for (int i = 0; i < data.size(); i++)
	{
		do
		{
			comma_pos = data[i].find(','); //see if there is a comma in the URL
			
			if (comma_pos != string::npos) //If there is...
			{
				data[i].erase(data[i].begin() + comma_pos); //remove the comma
			}
		}while (comma_pos != string::npos);
		
		output << data[i] << ',' << occur[i] << endl; //write index number, URL, and occurrence count to file
	}
	
	cout << "Write Successful." << endl;
}

void fill_lines(vector<line> &lines)
{
	int max_size = 0;
	//determine size of longest line
	for (int i = 0; i < lines.size(); i++)
	{
		if (lines[i].size() > max_size)
		{
			max_size = lines[i].size();
		}
	}
	
	//for each line shorter than longest line, fill in with NA strings
	for (int i = 0; i < lines.size(); i++)
	{
		while (lines[i].size() < max_size)
		{
			if (lines.size() == max_size - 1) //if at the needed size, don't add an extra comma
			{
				lines[i].push_back("NA");
			}
			else //otherwise, add a comma,NA string
			{
				lines[i].push_back(",NA");
			}
		}
	}
}

