/*
 * itemTemplate.h
 *
 *  Created on: Apr 1, 2017
 *      Author: sjelli
 */

#ifndef ITEMTEMPLATE_H_
#define ITEMTEMPLATE_H_

#include <string>
#include <vector>

#include "dice.h"

using namespace std;

class itemTemplate {

	public:
		string name;
		vector<string> type;
		string color;
		dice* hit;
		dice* dam;
		dice* dodge;
		dice* def;
		dice* weight;
		dice* attr;
		dice* value;
		dice* speed;

		itemTemplate();
		~itemTemplate();
};


#endif /* ITEMTEMPLATE_H_ */
