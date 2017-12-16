/*
 * charParser.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: Sean Jellison
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>

#include "creature.h"
#include "creatureTemplate.h"
#include "charParser.h"

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

void static handleName(creatureTemplate* c, string s)
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

void static handleSymbol(creatureTemplate* c, string s)
{
	//This assumes very strict adherence to formatting in the text file
	//cout << s << endl;
	c->type = s.at(5); //should be only one character left and it will set c->type to it
//	//cout << "Creature type: " << c->type << endl;
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

void static handleStat(creatureTemplate* c, string s)
{
	//format for stats, i->e-> speed, damage, and hp
	//STATNAME 'base'+'numDice'd'sides'

	//cout << s << endl;

	vector<string> split = splitString(s, "+d ");

	unsigned int base = atoi(split.at(1).c_str());
	unsigned int numDice = atoi(split.at(2).c_str());
	unsigned int sides = atoi(split.at(3).c_str());

	dice* d = new dice(base, numDice, sides);
//	//cout << s << ": base: " << base << " numDice: " << numDice << " sides: " << sides << endl;
	if(!split.at(0).compare("SPEED"))
	{
//		//cout << "Speed Pre: " << c->speed->base << endl;
		delete(c->speed);
		c->speed = d;
//		//cout << "Speed Post: " << c->speed->base << endl;
	}
	else if(!split.at(0).compare("DAM"))
	{
//		//cout << "Damage Pre: " << c->dmg->base << endl;
		delete(c->dmg);
		c->dmg = d;
//		//cout << "Damage Post: " << c->dmg->base << endl;
	}
	else if(!split.at(0).compare("HP"))
	{
//		//cout << "HP Pre: " << c->hp->base << endl;
		delete(c->hp);
		c->hp = d;
//		//cout << "HP Post: " << c->hp->base << endl;
	}

}

void static handleColor(creatureTemplate *c, string s)
{
	//cout << s << endl;

	vector<string> split = splitString(s, " ");
	unsigned int i;

	//don't want the time
	for(i = 1; i < split.size(); i++)
	{
		c->colors.push_back(split.at(i));
	}

}

void static handleAbilities(creatureTemplate *c, string s)
{
	//cout << s << endl;

	vector<string> split = splitString(s, " ");

	c->abil = 0;
	unsigned int i;
	if(split.size() > 1)
	{
		for(i = 1; i < split.size(); i++)
		{
//			//cout << split.at(i) << endl;
			if(!split.at(i).compare("SMART")) //intelligent monster
			{
				c->abil += INTELLIGENT;
//				//cout << "Creature is intelligent" << endl;
			}
			else if(!split.at(i).compare("PASS")) //ghostly?
			{
				c->abil += GHOST;
//				//cout << "Creature is a ghost" << endl;
			}
			else if(!split.at(i).compare("TELE")) //telepathic
			{
				c->abil += TELEPATH;
//				//cout << "Creature is telepathic" << endl;
			}
			else if(!split.at(i).compare("TUNNEL")) //can tunnel
			{
				c->abil += TUNNELER;
//				//cout << "Creature is a tunneler" << endl;
			}
			else if(!split.at(i).compare("ERRATIC")) //eratic
			{
				c->abil += ERATIC;
//				//cout << "Creature is erratic" << endl;
			}
			else if(!split.at(i).compare("DESTROY"))
			{
				c->abil += DESTITEMS;
//				//cout << "Creature destroys items" << endl;
			}
			else if(!split.at(i).compare("PICKUP"))
			{
				c->abil += PICKITEMS;
//				//cout << "Creature picks up items" << endl;
			}
		}
	}

}

//creature.cpp is going to be calling this so it has a list of creature templates it can make
int getCreatureTemplates(vector<creatureTemplate*>* creatures)
{
	string line, type;
	ifstream file;

	fileName += "/.rlg327/monster_desc.txt";
	file.open(fileName.c_str()); //file1 object that is input only

	getline(file, line);

	//returns 0 if the strings are the same
	if(line.compare("RLG327 MONSTER DESCRIPTION 1"))
	{
		//wrong file1
		cout << "Wrong file, no RLG327 MONSTER DESCRIPTION 1. File name: " << fileName << endl;
		return 1;
	}

	getline(file, line); //currently holds RLG327... i.e. the header

	//reads till the end of file
	while(getline(file, line))
	{
		lineNum++;
		//line now contains a line of text from the file

		//read the object
		if(!line.compare("BEGIN MONSTER"))
		{
			bool namecheck = false;
			bool symbcheck = false;
			bool desccheck = false;
			bool hpcheck = false;
			bool abilcheck = false;
			bool speedcheck = false;
			bool colorcheck = false;
			bool damagecheck = false;

			creatureTemplate* c = new creatureTemplate();
			//cout << endl;
			//cout << "Found monster " << lineNum << endl;

			getline(file, line);
			while(line.compare("END"))
			{
				if(line != "")
				{
					/*
					 * Possible line starters:
					 * 	N //this is something that has no relevance to the gameplay, but it needs to be handled
					 * 	SY
					 * 	C
					 * 	DE //we don't care about the description, but it must be handled
					 * 	SP
					 * 	DA
					 * 	H
					 * 	A
					 *
					 * 	We can just look at the first 1 or 2 characters in a line for simplicity
					 */

					istringstream iss(line); //allows the line to be split with space delimiter
					iss >> type;

					if(type.at(0) == 'D' || type.at(0) == 'S')
					{
						if(type.at(1) == 'Y')
						{
							if(!symbcheck)
							{
								handleSymbol(c, line);
								symbcheck = true;
							}
							else
							{
								break;
							}

						}
						else if(type.at(1) == 'E')
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
						else if(type.at(1) == 'A')
						{
							if(!damagecheck)
							{
								handleStat(c, line);
								damagecheck = true;
							}
							else
							{
								break;
							}
						}
						else if(type.at(1) == 'P')
						{
							if(!speedcheck)
							{
								handleStat(c, line);
								speedcheck = true;
							}
							else
							{
								break;
							}
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
							handleColor(c, line);
							colorcheck = true;
						}
						else
						{
							break;
						}

					}
					else if(type.at(0) == 'H')
					{
						if(!hpcheck)
						{
							handleStat(c, line);
							hpcheck = true;
						}
						else
						{
							break;
						}
					}
					else if(type.at(0) == 'A')
					{
						if(!abilcheck)
						{
							handleAbilities(c, line);
							abilcheck = true;
						}
						else
						{
							break;
						}

					}
					else if(type.at(0) == 'N')
					{
						if(!namecheck)
						{
							handleName(c, line);
							namecheck = true;
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

			if(namecheck && abilcheck && hpcheck && colorcheck && speedcheck && damagecheck && desccheck && symbcheck)
			{
				creatures->push_back(c);
			}
			else
			{
				cout << "Error with creature, discarding" << endl;
				cout << namecheck << abilcheck << hpcheck << colorcheck << speedcheck << damagecheck << desccheck << symbcheck << endl;
				delete(c);
			}
		}
	}

	file.close();

	return 0;
}
