/*
 * creature.h
 *
 *  Created on: Mar 20, 2017
 *      Author: sjelli
 */

#ifndef CREATURE_H_
#define CREATURE_H_

#ifdef __cplusplus

#include <string>
#include <vector>

#include "creatureTemplate.h"
#include "dice.h"
#include "item.h"

using namespace std;

class creature{
	public:
		unsigned char speed;
		unsigned int priority; //making it an int gives 2^32-2 possible monsters on the map
		unsigned int armor;
		unsigned int dodge;
		unsigned int acc;
		dice* damage; //a stat for tunneling monsters that determines how quickly they destroy rocks
		unsigned char posx;
		unsigned char posy;
		int hitpoints;
		unsigned char type;
		unsigned int characteristics;
		unsigned int nextMove;
		unsigned int los;
		unsigned char isDead; //0 means living
		vector<string> color; //A list of colors the creature can take on. Multiple colors means it will change every dungeon update
		item* inv[20];
		item* equip[13];
		int addItem(item*);
		int dropItem(int);
		int isInLOS(int, int);
		void equipItem(int);
		void unequipItem(int);
		void destroyItem(item*);
		creature(creatureTemplate* c, int prty);
		creature(int, int); //constructor
		~creature(); //deconstructor
};

extern "C" {
#endif

typedef void ccreature;

#define INTELLIGENT       0b00000000000000000000000000000001 //if true, finds the shortest path to the player character, otherwise it make a "straight" line for the player
#define TELEPATH          0b00000000000000000000000000000010 //if true, the monster will always move toward the player even if the player is not in line of sight
#define TUNNELER          0b00000000000000000000000000000100 //if true, the monster can dig through rocks
#define ERATIC            0b00000000000000000000000000001000 //if true, the monster will move randomly to any possible position it can move to with 50% probability, regardless of conditions
#define GHOST             0b00000000000000000000000000010000 //if true, the monster can pass through rock unhindered
#define DESTITEMS         0b00000000000000000000000000100000 //if true, the monster will destroy items it passes over
#define PICKITEMS         0b00000000000000000000000001000000 //if true, the monster will pickup and possibly use the items it pass over

#ifdef __cplusplus
}
#endif


#endif /* CREATURE_H_ */
