/*
 * creature.cpp
 *
 *  Created on: Mar 20, 2017
 *      Author: sjelli
 */

#include <climits>
#include <cstdlib>
#include <iostream>
#include <string>
#include <ncurses.h>

#include "dungeon.h"
#include "creature.h"
#include "charParser.h"
#include "creatureTemplate.h"
#include "dice.h"
#include "item.h"

//TODO Need to add error messages when items are not properly handled

//generates a creature with random attributes or a "null" monster
using namespace std;

int typeToInt(item* i)
{
	if(i->type[0] == "WEAPON" || (i->type.size() > 1 && i->type[1] == "WEAPON"))
	{
		return 0;
	}
	if(i->type[0] == "OFFHAND")
	{
		return 1;
	}
	if(i->type[0] == "RANGED")
	{
		return 2;
	}
	if(i->type[0] == "ARMOR")
	{
		return 3;
	}
	if(i->type[0] == "HELMET")
	{
		return 4;
	}
	if(i->type[0] == "CLOAK")
	{
		return 5;
	}
	if(i->type[0] == "GLOVES")
	{
		return 6;
	}
	if(i->type[0] == "BOOTS")
	{
		return 7;
	}
	if(i->type[0] == "RING")
	{
		return 8;
	}
	if(i->type[0] == "AMULET")
	{
		return 10;
	}
	if(i->type[0] == "LIGHT")
	{
		return 11;
	}
	if(i->type[0] == "CONTAINER")
	{
		return 12;
	}

	return -1;
}

int canDrop(unsigned char posx, unsigned char posy)
{
	item*** itemMap = getItemMap();
	cell_t** dun = getDungeon();
	int j;

	unsigned int offsets[] = {
			(unsigned int)posx, (unsigned int)posy, //on the creature
			(unsigned int)posx + 1, (unsigned int)posy, //1 to the right
			(unsigned int)posx - 1, (unsigned int)posy, //1 to the left
			(unsigned int)posx, (unsigned int)posy + 1, //1 below
			(unsigned int)posx, (unsigned int)posy - 1, //1 above
			(unsigned int)posx + 1, (unsigned int)posy + 1, //bottom right
			(unsigned int)posx - 1, (unsigned int)posy + 1, //bottom left
			(unsigned int)posx + 1, (unsigned int)posy - 1, //top right
			(unsigned int)posx - 1, (unsigned int)posy - 1, //top left
	};

	for(j = 0; j < 9; j += 2)
	{
		//the dungeon tile is open and no item is on the space
		if(dun[offsets[j + 1]][offsets[j]].hardness == 0 &&
				itemMap[offsets[j + 1]][offsets[j]]->name == "")
		{
			free(itemMap);
			free(dun);
			return 0;
		}
	}
	free(itemMap);
	free(dun);
	return 1;
}

creature::creature(int isNotNull, int prty) //constructor
{
	if(!isNotNull) //null creature
	{
		characteristics = 0;
		armor = 0;
		dodge = 0;
		acc = 0;
		hitpoints = 0;
		isDead = 1; //possibly the "null" identifier
		nextMove = 0;
		posx = 255;
		posy = 255;
		priority = INT_MAX;
		speed = 0;
		damage = 0;
		type = EMPTY;
		los = 0;
		color.push_back("WHITE");
		int i;
		for(i = 0; i < 20; i++)
		{
			inv[i] = 0;
		}
		for(i = 0; i < 13; i++)
		{
			equip[i] = 0;
		}
	}

	else
	{
		characteristics = rand();
		armor = 0;
		dodge = 0;
		acc = 65;
		hitpoints = rand() % 9 + 1; //1-10
		isDead = 0;
		nextMove = 0;
		posx = 255;
		posy = 255;
		priority = prty;
		speed = (rand() % 16) + 5; //5-20
		damage = new dice((unsigned int)(rand() % 10), (unsigned int)(rand() % 4), (unsigned int)(rand() % 8));
		los = (rand() % 10) + 1;
		type = 'p';
		color.push_back("WHITE");
		int i;
		for(i = 0; i < 20; i++)
		{
			inv[i] = 0;
		}
		for(i = 0; i < 13; i++)
		{
			equip[i] = 0;
		}
	}
}

creature::creature(creatureTemplate* c, int prty)
{
	characteristics = c->abil;
	armor = 0;
	dodge = 0;
	acc = 75;
	hitpoints = c->hp->roll();
	isDead = 0;
	nextMove = 0;
	posx = 255;
	posy = 255;
	priority = prty;
	speed = c->speed->roll();
	damage = c->dmg;
	type = c->type;
	los = (rand() % 10) + 1;
	color = c->colors;
	int i;
	for(i = 0; i < 20; i++)
	{
		inv[i] = 0;
	}
	for(i = 0; i < 13; i++)
	{
		equip[i] = 0;
	}
}

creature::~creature()
{
//	cout << "Attempting to delete creature" << endl;
	delete(damage);
//	cout << "Deleted damage dice" << endl;
	int i;
	for(i = 0; i < 32; i++)
	{
		if(inv[i] != 0)
		{
//			cout << "Attempting to delete inventory item" << endl;
			delete(inv[i]);
//			cout << "Deleted inventory item" << endl;
		}
		if(i < 13 && equip[i] != 0)
		{
//			cout << "Attempting to delete equipped item" << endl;
			delete(equip[i]);
//			cout << "Deleted equipped item" << endl;
		}
	}
//	cout << "Attempting to free inventory" << endl;
//	free(inv);
//	cout << "Freed inventory" << endl;
//	cout << "Attempting to free equipment" << endl;
//	free(equip);
//	cout << "Freed equipment" << endl;
}

//adds an item to the creature's inventory
int creature::addItem(item* i)
{
	item* ni = new item();
	dice* dam = new dice();
	int j, invSize, invPos;

	if(equip[12] != 0)
	{
		invSize = 20;
	}
	else
	{
		invSize = 10;
	}

	for(j = 0; j < invSize; j++)
	{
		if(inv[j] == 0)
		{
			invPos = j;
			break;
		}
	}

	//no room for item
	if(j == invSize)
	{
		mvprintw(22, 0, "Cannot pick up item. Inventory is full.");
		refresh();
		delete(ni);
		delete(dam);
		return -1;
	}

	dam->base = i->damBonus->base;
	dam->numDice = i->damBonus->numDice;
	dam->sidesPerDie = i->damBonus->sidesPerDie;

	ni->color = i->color;
	ni->damBonus = dam;
	ni->defenseBonus = i->defenseBonus;
	ni->dodgeBonus = i->dodgeBonus;
	ni->hitChanceBonus = i->hitChanceBonus;
	ni->name = i->name;
	ni->special = i->special;
	ni->speedBonus = i->speedBonus;
	ni->type = i->type;
	ni->value = i->value;
	ni->weight = i->weight;

	inv[invPos] = ni;
	mvprintw(22, 0, "Added %s to inventory.", ni->name.c_str());
	refresh();
	return invPos;
}

int creature::isInLOS(int x, int y)
{
	if((x >= this->posx - (unsigned char)this->los) && (x <= this->posx + (unsigned char)this->los))
	{
		if((y >= this->posy - (unsigned char)this->los) && (y <= this->posy + (unsigned char)this->los))
		{
			return 1;
		}
	}
	return 0;
}

//equips the item from the inventory
void creature::equipItem(int i)
{
	//no item at inventory location
	if(inv[i] == 0 || i < 0 || i > 19)
	{
		return;
	}
	else
	{
		int t = typeToInt(inv[i]);

		//TODO Need special case for weapons, offhands, and rings

		//2h weapon
		if((t == 0 || t == 1) && inv[i]->type.size() > 1)
		{
			//no previous weapon or offhand, just equip the item
			if(equip[0] == 0 && equip[1] == 0)
			{
				equip[0] = inv[i];
				equip[1] = inv[i];
				inv[i] = 0;
				mvprintw(22, 0, "Equipped %s.", equip[0]->name.c_str());
				refresh();
			}
			//previous weapon, no off
			else if(equip[1] == 0)
			{
				item* temp = inv[i];
				inv[i] = equip[0];
				equip[0] = temp;
				equip[1] = temp;

				mvprintw(22, 0, "Un-equipped %s and equipped %s.", inv[i]->name.c_str(), equip[0]->name.c_str());
				refresh();

				//remove stats from the old equipment
				this->damage->base = this->damage->base - inv[i]->damBonus->base;
				this->damage->numDice = this->damage->numDice - inv[i]->damBonus->numDice;
				if(inv[i]->damBonus->numDice > 0)
				{
					this->damage->sidesPerDie = this->damage->sidesPerDie - inv[i]->damBonus->sidesPerDie;
				}
				this->armor = this->armor - inv[i]->defenseBonus;
				this->dodge = this->dodge - inv[i]->dodgeBonus;
				this->acc = this->acc - inv[i]->hitChanceBonus;
				this->speed = this->speed - inv[i]->speedBonus;
				this->los = this->los - inv[i]->special;
			}
			//previous offhand, no weap
			else if(equip[0] == 0)
			{
				item* temp = inv[i];
				inv[i] = equip[1];
				equip[0] = temp;
				equip[1] = temp;

				mvprintw(22, 0, "Un-equipped %s and equipped %s.", inv[i]->name.c_str(), equip[0]->name.c_str());
				refresh();

				//remove stats from the old equipment
				this->damage->base = this->damage->base - inv[i]->damBonus->base;
				this->damage->numDice = this->damage->numDice - inv[i]->damBonus->numDice;
				if(inv[i]->damBonus->numDice > 0)
				{
					this->damage->sidesPerDie = this->damage->sidesPerDie - inv[i]->damBonus->sidesPerDie;
				}
				this->armor = this->armor - inv[i]->defenseBonus;
				this->dodge = this->dodge - inv[i]->dodgeBonus;
				this->acc = this->acc - inv[i]->hitChanceBonus;
				this->speed = this->speed - inv[i]->speedBonus;
				this->los = this->los - inv[i]->special;
			}
			//previous 2h
			else if(equip[0] != 0 && equip[0] == equip[1])
			{
				item* temp = inv[i];
				inv[i] = equip[0];
				equip[0] = temp;
				equip[1] = temp;

				mvprintw(22, 0, "Un-equipped %s and equipped %s.", inv[i]->name.c_str(), equip[0]->name.c_str());
				refresh();

				//remove stats from the old equipment
				this->damage->base = this->damage->base - inv[i]->damBonus->base;
				this->damage->numDice = this->damage->numDice - inv[i]->damBonus->numDice;
				if(inv[i]->damBonus->numDice > 0)
				{
					this->damage->sidesPerDie = this->damage->sidesPerDie - inv[i]->damBonus->sidesPerDie;
				}
				this->armor = this->armor - inv[i]->defenseBonus;
				this->dodge = this->dodge - inv[i]->dodgeBonus;
				this->acc = this->acc - inv[i]->hitChanceBonus;
				this->speed = this->speed - inv[i]->speedBonus;
				this->los = this->los - inv[i]->special;
			}
			//1h and offhand
			else if(equip[0] != 0 && equip[1] != 0 && equip[0] != equip[1])
			{
				//TODO
			}
		}//end 2h handle
		//Handle rings
		else if(t == 8)
		{
			//ring 1 is open
			if(equip[t] == 0)
			{
				equip[t] = inv[i];
				inv[i] = 0;
				mvprintw(22, 0, "Equipped %s.", equip[t]->name.c_str());
				refresh();
			}
			//ring 1 is not open but ring 2 is
			else if(equip[t + 1] == 0)
			{
				t++;
				equip[t] = inv[i];
				inv[i] = 0;
				mvprintw(22, 0, "Equipped %s.", equip[t]->name.c_str());
				refresh();
			}
			//TODO Need to handle when a ring slot is not open
		}
		//the item slot is free, just put the item on
		else if(equip[t] == 0)
		{
			equip[t] = inv[i];
			inv[i] = 0;
			mvprintw(22, 0, "Equipped %s.", equip[t]->name.c_str());
			refresh();
		}
		//an item is already equipped in slot, switch it with the inventory
		else
		{
			//TODO Need to handle removing 2h weapons
			item* temp = inv[i];
			inv[i] = equip[t];
			equip[t] = temp;

			mvprintw(22, 0, "Un-equipped %s and equipped %s.", inv[i]->name.c_str(), equip[t]->name.c_str());
			refresh();

			//remove stats from the old equipment
			this->damage->base = this->damage->base - inv[i]->damBonus->base;
			this->damage->numDice = this->damage->numDice - inv[i]->damBonus->numDice;
			if(inv[i]->damBonus->numDice > 0)
			{
				this->damage->sidesPerDie = this->damage->sidesPerDie - inv[i]->damBonus->sidesPerDie;
			}
			this->armor = this->armor - inv[i]->defenseBonus;
			this->dodge = this->dodge - inv[i]->dodgeBonus;
			this->acc = this->acc - inv[i]->hitChanceBonus;
			this->speed = this->speed - inv[i]->speedBonus;
			this->los = this->los - inv[i]->special;
		}

		//add stats from the new equipment
		this->damage->base = this->damage->base + equip[t]->damBonus->base;
		this->damage->numDice = this->damage->numDice + equip[t]->damBonus->numDice;
		if(equip[t]->damBonus->numDice > 0)
		{
			this->damage->sidesPerDie = this->damage->sidesPerDie + equip[t]->damBonus->sidesPerDie;
		}
		this->armor = this->armor + equip[t]->defenseBonus;
		this->dodge = this->dodge + equip[t]->dodgeBonus;
		this->acc = this->acc + equip[t]->hitChanceBonus;
		this->speed = this->speed + equip[t]->speedBonus;
		this->los = this->los + equip[t]->special;

	}
}//end equipItem

//removes the item from the equipment slot
void creature::unequipItem(int i)
{
	int j, k, invSize, invPos;

	//unequiping a container
	if(i == 12 && equip[i] != 0)
	{
		for(j = 0; j < 10; j++)
		{
			if(inv[j + 10] != 0)
			{
				for(k = 0; k < 10; k++)
				{
					if(inv[k] == 0)
					{
						inv[k] = inv[j + 10];
						inv[j + 10] = 0;
						break; //should end the k loop
					}
				}
				if(k == 10)
				{
					if(canDrop(posx, posy))
					{
						mvprintw(22, 0, "Inventory is full, dropped %s.", inv[j + 10]->name.c_str());
						refresh();
						dropItem(j + 10);
					}
					else
					{
						//no room in inventory, no space to drop the item
						//items may have gotten shuffled and some may have been dropped
						//but not everything that was needed
						mvprintw(22, 0, "Inventory is full, cannot find place to drop %s.", inv[j + 10]->name.c_str());
						refresh();
						return;
					}
				}
			}
		}
		//feels redundant to do this again
		//place the container into the inventory
		for(j = 0; j < 10; j++)
		{
			if(inv[j] == 0)
			{
				mvprintw(22, 0, "Un-equipped %s.", equip[i]->name.c_str());
				refresh();
				inv[j] = equip[i];
				equip[i] = 0;

				//remove stats from the old equipment, just incase the bag gives stats
				this->damage->base = this->damage->base - inv[j]->damBonus->base;
				this->damage->numDice = this->damage->numDice - inv[j]->damBonus->numDice;
				if(inv[j]->damBonus->numDice > 0)
				{
					this->damage->sidesPerDie = this->damage->sidesPerDie - inv[j]->damBonus->sidesPerDie;
				}
				this->armor = this->armor - inv[j]->defenseBonus;
				this->dodge = this->dodge - inv[j]->dodgeBonus;
				this->acc = this->acc - inv[j]->hitChanceBonus;
				this->speed = this->speed - inv[j]->speedBonus;
				this->los = this->los - inv[j]->special;

				return; //container has been emptied and placed into inventory
			}
		}
	}

	//check to see if the creature has extended inventory space
	if(equip[12] != 0)
	{
		invSize = 20;
	}
	else
	{
		invSize = 10;
	}

	//find an empty inventory slot
	for(j = 0; j < invSize; j++)
	{
		if(inv[j] == 0)
		{
			invPos = j;
			break;
		}
	}

	if(j != invSize)
	{
		mvprintw(22, 0, "Un-equipped %s.", equip[i]->name.c_str());
		refresh();

		inv[invPos] = equip[i];
		equip[i] = 0;

		//remove stats from the old equipment
		this->damage->base = this->damage->base - inv[invPos]->damBonus->base;
		this->damage->numDice = this->damage->numDice - inv[invPos]->damBonus->numDice;
		if(inv[invPos]->damBonus->numDice > 0)
		{
			this->damage->sidesPerDie = this->damage->sidesPerDie - inv[invPos]->damBonus->sidesPerDie;
		}
		this->armor = this->armor - inv[invPos]->defenseBonus;
		this->dodge = this->dodge - inv[invPos]->dodgeBonus;
		this->acc = this->acc - inv[invPos]->hitChanceBonus;
		this->speed = this->speed - inv[invPos]->speedBonus;
		this->los = this->los - inv[invPos]->special;
	}
}

//places the item in the dungeon relative to the creature position
int creature::dropItem(int i)
{
	if(this->inv[i] != 0)
	{
		item*** itemMap = getItemMap();
		cell_t** dun = getDungeon();
		int j;

		unsigned int offsets[] = {
				(unsigned int)posx, (unsigned int)posy, //on the creature
				(unsigned int)posx + 1, (unsigned int)posy, //1 to the right
				(unsigned int)posx - 1, (unsigned int)posy, //1 to the left
				(unsigned int)posx, (unsigned int)posy + 1, //1 below
				(unsigned int)posx, (unsigned int)posy - 1, //1 above
				(unsigned int)posx + 1, (unsigned int)posy + 1, //bottom right
				(unsigned int)posx - 1, (unsigned int)posy + 1, //bottom left
				(unsigned int)posx + 1, (unsigned int)posy - 1, //top right
				(unsigned int)posx - 1, (unsigned int)posy - 1, //top left
		};

		for(j = 0; j < 9; j += 2)
		{
			//the dungeon tile js open and no item is on the space
			if(dun[offsets[j + 1]][offsets[j]].hardness == 0 &&
					itemMap[offsets[j + 1]][offsets[j]]->name == "")
			{
				delete(itemMap[offsets[j + 1]][offsets[j]]);
				itemMap[offsets[j + 1]][offsets[j]] = this->inv[i];
				mvprintw(22, 0, "Dropped %s.", this->inv[i]->name.c_str());
				refresh();
				this->inv[i] = 0;
				free(itemMap);
				free(dun);
				return 0;
			}
		}
		mvprintw(22, 0, "No room to drop item");
		refresh();
		free(itemMap);
		free(dun);
		return 1;
	}
	return 1;
}

//destroys the item
//DOES NOT HANDLE INVENTORY SPACE!
void creature::destroyItem(item* i)
{
	mvprintw(22, 0, "%s destroyed!", i->name.c_str());
	refresh();
	delete(i);
}
