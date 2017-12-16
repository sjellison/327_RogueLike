/*
 * item.h
 *
 *  Created on: Apr 1, 2017
 *      Author: sjelli
 */

#ifndef ITEM_H_
#define ITEM_H_

#include <string>
#include <vector>

#include "itemTemplate.h"
#include "dice.h"

using namespace std;

class item {

	public:
		string name;
		string color;
		vector<string> type;
		unsigned int hitChanceBonus;
		dice* damBonus;
		unsigned int dodgeBonus;
		unsigned int defenseBonus;
		unsigned int weight;
		unsigned int speedBonus;
		unsigned int special;
		unsigned int value;
		item();
		item(itemTemplate*);
		~item();
};



#endif /* ITEM_H_ */
