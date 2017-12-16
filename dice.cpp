/*
 * dice.cpp
 *
 *  Created on: Mar 29, 2017
 *      Author: sjelli
 */

#include <cstdlib>
#include "dice.h"

using namespace std;

dice::dice()
{
	base = 0;
	numDice = 0;
	sidesPerDie = 0;
}

dice::dice(unsigned int b, unsigned int n, unsigned int s)
{
	base = b;
	numDice = n;
	sidesPerDie = s;
}

dice::~dice()
{
}

unsigned int dice::roll()
{
	unsigned int i;
	unsigned int res = 0;
	for(i = 0; i < numDice; i++)
	{
		res += (rand() % sidesPerDie) + 1;
	}
	return base + res;
}
