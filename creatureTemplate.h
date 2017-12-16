/*
 * creatureTemplate.h
 *
 *  Created on: Mar 29, 2017
 *      Author: sjelli
 */

#ifndef CREATURETEMPLATE_H_
#define CREATURETEMPLATE_H_

#include <string>
#include <vector>

#include "dice.h"

using namespace std;

class creatureTemplate{
	public:
		string name;
		vector<string> colors;
		unsigned int abil;
		unsigned char type;
		dice* hp;
		dice* dmg;
		dice* speed;
		creatureTemplate();
		~creatureTemplate();
};



#endif /* CREATURETEMPLATE_H_ */
