/*
 * item.cpp
 *
 *  Created on: Apr 1, 2017
 *      Author: sjelli
 */

#include <string>

#include "itemTemplate.h"
#include "item.h"
#include "dice.h"

using namespace std;

item::item()
{
	name = "";
	color = "";
	hitChanceBonus = 0;
	damBonus = new dice();
	dodgeBonus = 0;
	defenseBonus = 0;
	weight = 0;
	speedBonus = 0;
	special = 0;
	value = 0;
}

item::item(itemTemplate *p)
{
	name = p->name;
	color = p->color;
	type = p->type;
	hitChanceBonus = p->hit->roll();
	damBonus = p->dam;
	dodgeBonus = p->dodge->roll();
	defenseBonus = p->def->roll();
	weight = p->weight->roll();
	speedBonus = p->speed->roll();
	special = p->attr->roll();
	value = p->value->roll();
}

item::~item()
{
	delete damBonus;
}
