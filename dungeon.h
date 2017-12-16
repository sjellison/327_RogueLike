/*
 * DungeonGenerator.h
 *
 *  Created on: Jan 18, 2017
 *      Author: Sean Jellison
 */

#ifndef DUNGEONGENERATOR_H
#define DUNGEONGENERATOR_H

#include "ai.h"
#include "creature.h"

#ifdef __cplusplus
extern "C" {
#endif

//shapes of cells or blocks
#define EMPTY '`' //This is an empty block that MUST be changed.
#define EDGE 'H' //The border columns of the board matrix.
#define ROOM ':' //These characters represent a room space with nothing special about it.
//#define PATH1 '-' //A horizontal path
//#define PATH2 '|' //A vertical path
#define PATH3 '#' //A generic path
//#define STONE 'r' //A simple rock
#define ROCK 'R' //A larger rock
//#define ROCK ' ' //Useful for debugging
//#define BOULDER 'B' //A really large rock
//#define WATER1 'w' //Small rising wave water tile
//#define WATER2 'W' //Large rising wave water tile
//#define WATER3 'M' //Large crashing wave water tile
//#define WATER4 'm' //Small crashing wave water tile
//#define WATER5 '~' //Calm/river water tile
#define STAIRSUP '<'
#define STAIRSDOWN '>'

//fine! player will be an @ symbol
#define PLAYER '@' //might use >, <, v, and ^ to represent the player and their facing later. For now we just need to represent the player on the map

//some other "suggeste" tiles to use, pft!
#define WEAPON '|'
#define OFFHAND ')'
#define RANGED '}'
#define ARMOR '['
#define HELMET ']'
#define CLOAK '('
#define GLOVES '{'
#define BOOTS '\\'
#define RING '='
#define AMULET '\"'
#define LIGHT '_'
#define SCROLL '~'
#define BOOK '?'
#define FLASK '!'
#define GOLD '$'
#define AMMUNITION '/'
#define FOOD ','
#define WAND '-'
#define CONTAINER '%'
#define ITEMSTACK '&'

//A simplified block that is 1x1
typedef struct cell
{
	int visited; //used for path finding
	int hardness; //a difficulty to break the cell
	char tile; //the "shape" of the cell
	char visible; //determines if the cell is visible in the fog of war (NOT if its in LOS of the player)
}cell_t;

void setFrame(int);
void displayDungeon();
void loadDungeon(unsigned char*, unsigned char*, int);
void generateDungeon(int, int, int);
void getRoomOne(int*);
void setMonster(creature*);
void setItem(item*, int, int);
void nullMonster(int, int);
void nullItem(int, int);
void releaseMonster(int, int);
void killScreen();
void moveScreen(int);
void newDungeon();
void updateScreen();
void moveScreen(int);
creature* getMonster(int, int);
item* getItem(int, int);
item*** getItemMap();
int isMonsterDead(int, int);
int getHardness(unsigned char*);
int getRooms(unsigned char*);
int getNumOfRooms();
int isOnStairs(creature*);
int getFloorNum();
cell_t** getDungeon();


#ifdef __cplusplus
}
#endif
//A block is a set of characters that represent an object, such as a bridge or large boulder.
//A block is minimum 1x1 unit wide and long, and maximum 4x4.
//typedef struct block
//{
//	int width; //Number of units wide a block is, inclusive.
//	int length; //Number of units long a block is, inclusive.
//	int walkable; //Boolean value for whether the block is walkable or not. 0: Not walkable 1: Walkable
//	int mineable; //Boolean value for whether the block can be destroyed by the player.
//	int hardness; //A value for mineable objects to give a relative difficulty to remove. Values provided here are the base and can be changed.
//	char units[4][4]; //The definition of the block.
//}block_t;

#endif /* DUNGEONGENERATOR_H_ */
