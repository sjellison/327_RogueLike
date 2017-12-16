/*
 * itemTemplate.cpp
 *
 *  Created on: Apr 1, 2017
 *      Author: sjelli
 */

#include <string>

#include "itemTemplate.h"

using namespace std;

itemTemplate::itemTemplate()
{
	 name = "";
	 color = "";
	 hit = new dice();
	 dam = new dice();
	 dodge = new dice();
	 def = new dice();
	 weight = new dice();
	 attr = new dice();
	 value = new dice();
	 speed = new dice();
}

itemTemplate::~itemTemplate()
{
	delete(hit);
	delete(dam);
	delete(dodge);
	delete(def);
	delete(weight);
	delete(attr);
	delete(value);
	delete(speed);
}


