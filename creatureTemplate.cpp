/*
 * creatureTemplate.cpp
 *
 *  Created on: Mar 29, 2017
 *      Author: sjelli
 */

#include <string>
#include <iostream>

#include "creatureTemplate.h"
#include "dice.h"

using namespace std;

creatureTemplate::creatureTemplate()
{
	name = "";
	abil = 0;
	type = '`';
	hp = new dice();
	speed = new dice();
	dmg = new dice();
}

creatureTemplate::~creatureTemplate()
{
	cout << "Deleting CreatureTemplate: " << this->name << endl;
	if(hp != 0)
	{
		delete(hp);
	}
	if(dmg)
	{
		delete(dmg);
	}
	if(speed)
	{
		delete(speed);
	}
}
