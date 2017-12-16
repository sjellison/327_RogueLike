/*
 * itemParser.cpp
 *
 *  Created on: April 1, 2017
 *      Author: Sean Jellison
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>

#include "item.h"
#include "itemTemplate.h"
#include "itemParser.h"

using namespace std;

static string fileName = getenv("HOME"); //i'm expecting the + operator to concatinate
static int lineNum = 0;

vector<string> static splitString(string s, string delims)
{
	vector<string> split;
	size_t prev = 0;
	size_t pos;

	while((pos = s.find_first_of(delims.c_str(), prev)) != s.npos)
	{
		if(pos > prev)
		{
			split.push_back(s.substr(prev, (pos - prev)));
			prev = pos + 1;
		}
	}
	if(prev < s.length())
	{
		split.push_back(s.substr(prev, s.npos));
	}

	return split;
}

void static handleName(itemTemplate* c, string s)
{
	//cout << s << endl;
	vector<string> split = splitString(s, " ");

	unsigned int i;
	c->name = "";
	for(i = 1; i < split.size(); i++)
	{
		c->name += split.at(i);
		if(i != split.size() - 1)
		{
			c->name += " ";
		}
	}
}

void static handleType(itemTemplate* c, string s)
{
	//cout << s << endl;
	vector<string> split = splitString(s, " ");

	unsigned int i;
	for(i = 1; i < split.size(); i++)
	{
		c->type.push_back(split.at(i));
	}
}

void static handleDescription(ifstream* f)
{
	string line;
	//cout << line << endl;
	while(getline(*f, line))
	{
		lineNum++;
		//cout << line << endl;
		if(line != "" && (line.at(0) == '.' && line.length() == 1))
		{
			break;
		}
	}
}

void static handleStat(itemTemplate* c, string s)
{
	//cout << s << endl;

	vector<string> split = splitString(s, "+d ");

	unsigned int base = atoi(split.at(1).c_str());
	unsigned int numDice = atoi(split.at(2).c_str());
	unsigned int sides = atoi(split.at(3).c_str());

	dice* d = new dice(base, numDice, sides);
	if(!split.at(0).compare("SPEED"))
	{
		delete(c->speed);
		c->speed = d;
	}
	else if(!split.at(0).compare("DAM"))
	{
		delete(c->dam);
		c->dam = d;
	}
	else if(!split.at(0).compare("HIT"))
	{
		delete(c->hit);
		c->hit = d;
	}
	else if(!split.at(0).compare("DEF"))
	{
		delete(c->def);
		c->def = d;
	}
	else if(!split.at(0).compare("WEIGHT"))
	{
		delete(c->weight);
		c->weight = d;
	}
	else if(!split.at(0).compare("VALUE"))
	{
		delete(c->value);
		c->value = d;
	}
	else if(!split.at(0).compare("DODGE"))
	{
		delete(c->dodge);
		c->dodge = d;
	}
	else if(!split.at(0).compare("ATTR"))
	{
		delete(c->attr);
		c->attr = d;
	}

}

void static handleColor(itemTemplate *c, string s)
{
	//cout << s << endl;

	vector<string> split = splitString(s, " ");

	c->color = split.at(1); //should only be one color for items
}
//creature.cpp is going to be calling this so it has a list of creature templates it can make
int getItemTemplates(vector<itemTemplate*>* items)
{
	string line, type;
	ifstream file;

	fileName += "/.rlg327/item_desc.txt";
	file.open(fileName.c_str()); //file1 object that is input only

	getline(file, line);

	//returns 0 if the strings are the same
	if(line.compare("RLG327 OBJECT DESCRIPTION 1"))
	{
		//wrong file1
		cout << "Wrong file, no RLG327 OBJECT DESCRIPTION 1. File name: " << fileName << endl;
		return 1;
	}

	getline(file, line); //currently holds RLG327... i.e. the header

	//reads till the end of file
	while(getline(file, line))
	{
		lineNum++;
		//line now contains a line of text from the file

		//read the object
		if(!line.compare("BEGIN OBJECT"))
		{
			bool namecheck = false;
			bool typecheck = false;
			bool desccheck = false;
			bool weightcheck = false;
			bool hitcheck = false;
			bool speedcheck = false;
			bool colorcheck = false;
			bool damagecheck = false;
			bool attributecheck = false;
			bool valuecheck = false;
			bool dodgecheck = false;
			bool defcheck = false;

			itemTemplate* it = new itemTemplate();
//			cout << endl;
//			cout << "Found item " << lineNum << endl;

			getline(file, line);
			while(line.compare("END"))
			{
				if(line != "")
				{
					/*
					 * Possible line starters:
					 * 	N
					 * 	T
					 * 	C
					 * 	W
					 * 	H
					 * 	DA
					 * 	A
					 * 	V
					 * 	DO
					 * 	DEF
					 * 	DES
					 * 	S
					 */

					istringstream iss(line); //allows the line to be split with space delimiter
					iss >> type;

					if(type.at(0) == 'D')
					{
						if(type.at(1) == 'A')
						{
							if(!damagecheck)
							{
								handleStat(it, line);
								damagecheck = true;
							}
							else
							{
								break;
							}

						}
						else if(type.at(1) == 'O')
						{
							if(!dodgecheck)
							{
								handleStat(it, line);
								dodgecheck = true;
							}
							else
							{
								break;
							}
						}
						else if(type.at(1) == 'E')
						{
							if(type.at(2) == 'S')
							{
								if(!desccheck)
								{
									handleDescription(&file);
									desccheck = true;
								}
								else
								{
									break;
								}
							}
							else if(type.at(2) == 'F')
							{
								if(!defcheck)
								{
									handleStat(it, line);
									defcheck = true;
								}
								else
								{
									break;
								}
							}

						} //end check for "DE"
					} //end check for 'D'
					else if(type.at(0) == 'N')
					{
						if(!namecheck)
						{
							handleName(it, line);
							namecheck = true;
						}
						else
						{
							break;
						}
					}
					else if(type.at(0) == 'C')
					{
						if(!colorcheck)
						{
							handleColor(it, line);
							colorcheck = true;
						}
						else
						{
							break;
						}

					}
					else if(type.at(0) == 'H')
					{
						if(!hitcheck)
						{
							handleStat(it, line);
							hitcheck = true;
						}
						else
						{
							break;
						}
					}
					else if(type.at(0) == 'A')
					{
						if(!attributecheck)
						{
							handleStat(it, line);
							attributecheck = true;
						}
						else
						{
							break;
						}

					}
					else if(type.at(0) == 'T')
					{
						if(!typecheck)
						{
							handleType(it, line);
							typecheck = true;
						}
						else
						{
							break;
						}
					}
					else if(type.at(0) == 'W')
					{
						if(!weightcheck)
						{
							handleStat(it, line);
							weightcheck = true;
						}
						else
						{
							break;
						}
					}
					else if(type.at(0) == 'V')
					{
						if(!valuecheck)
						{
							handleStat(it, line);
							valuecheck = true;
						}
						else
						{
							break;
						}
					}
					else if(type.at(0) == 'S')
					{
						if(!speedcheck)
						{
							handleStat(it, line);
							speedcheck = true;
						}
						else
						{
							break;
						}
					}

					getline(file, line);
					lineNum++;
				}
				else
				{
					getline(file, line);
				}
			}
			if(weightcheck && valuecheck && defcheck && namecheck && attributecheck && hitcheck && colorcheck && speedcheck && damagecheck && desccheck && typecheck)
			{
				items->push_back(it);
			}
			else
			{
				cout << "Error with item, discarding" << endl;
				cout << weightcheck << valuecheck << defcheck << namecheck << attributecheck << hitcheck << colorcheck << speedcheck << damagecheck << desccheck << typecheck << endl;
				delete(it);
			}
		}
	}

	file.close();

	return 0;
}
