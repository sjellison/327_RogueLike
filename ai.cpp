/*
 * ai.c
 *
 *  Created on: Feb 4, 2017
 *      Author: Sean Jellison
 */
//Implements path finding and other ai related functions
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ncurses.h>
#include <unistd.h>
#include <iostream>

#include "dungeon.h"
#include "heap.h"
#include "main.h"
#include "ai.h"
#include "creature.h"
#include "player.h"
#include "charParser.h"
#include "creatureTemplate.h"
#include "itemTemplate.h"
#include "itemParser.h"

using namespace std;

typedef void (*Movements)(creature*); //Used for movement look up table
typedef unsigned int (*Attacks)(creature*); //Used to perform an attack, using a look up table
typedef int (*Looks)(creature*); //Used for looking before moving table

typedef struct path{
    heap_node_t * node;
    unsigned char xpos;
    unsigned char ypos;
    int dist;
}path_t;

static int dungeon_x = 160;
static int dungeon_y = 105;
static int numMonsters = 1;
static int floorNum = 1;
static path_t** tunnelerPath;
static path_t** nonTunnelerPath;
static cell_t** dungeon;
static item*** itemMap;
static heap_t eventQ;
static long gameTime; //number of frames that have gone by since the game started
static creature* player;
static vector<creatureTemplate*> creatureTemplates;
static vector<itemTemplate*> itemTemplates;

void kill(enum MV_t, creature*);
void nonTunnelerMovement(creature*);
void tunnelerMovement(creature*);
void move_left(creature*);
void move_right(creature*);
void move_up(creature*);
void move_down(creature*);
void move_upleft(creature*);
void move_downright(creature*);
void move_upright(creature*);
void move_downleft(creature*);
void move_creature(int, creature*);
unsigned int attack_left(creature*);
unsigned int attack_right(creature*);
unsigned int attack_up(creature*);
unsigned int attack_down(creature*);
unsigned int attack_upleft(creature*);
unsigned int attack_downright(creature*);
unsigned int attack_upright(creature*);
unsigned int attack_downleft(creature*);
void getTunnelerPath(path_t**, cell_t**, int, int);
void getNonTunnelerPath(path_t**, cell_t**, int, int);
void freePaths();

int weightedHardness(int);
int max(int, int); //why is this not in the standard library?
int playerInLOS(creature*);
int look_left(creature*);
int look_right(creature*);
int look_up(creature*);
int look_down(creature*);
int look_upleft(creature*);
int look_downright(creature*);
int look_upright(creature*);
int look_downleft(creature*);

int numPlayerMissedQuips = 6;
int numCreatureMissedQuips = 5;
int numHitQuips = 10;
string missed[] = {
		"Swing and a miss!", "He felt the air behind that one!", "Over there dummy!",
		"CRITICAL! miss...", "The creature laughs at you.", "You start to rethink you career choice.",
		"Dodged that one!", "To close!", "Phew, it missed!", "Kek!", "critical, MISS!"
};

string hit[] = {
		"Hwack!", "Thock!", "Shink!", "Bam!", "Kapow!", "Wham!",
		"Bazinga!", "Boom shackalacka!", "Tink!", "Fwoom!"
};

Movements move_table[8] = {
    move_left,
    move_right,
    move_up,
    move_down,
    move_upleft,
    move_downright,
    move_upright,
    move_downleft
};

Looks look_table[8] = {
    look_left,
    look_right,
    look_up,
    look_down,
    look_upleft,
    look_downright,
    look_upright,
    look_downleft
};

Attacks attack_table[8] = {
		attack_left,
		attack_right,
		attack_up,
		attack_down,
		attack_upleft,
		attack_downright,
		attack_upright,
		attack_downleft
};

unsigned int custMin(unsigned int val1, unsigned int val2)
{
	int i1 = val1;
	int i2 = val2;
	return i1 < i2 ? val1 : val2;
}

unsigned int custMax(unsigned int val1, unsigned int val2)
{
	int i1 = val1;
	int i2 = val2;
	return i1 > i2 ? val1 : val2;
}


//compare's 2 cells by their hardness
static int cmp(const void *val1, const void *val2)
{
    return ((path_t *) val1)->dist - ((path_t *) val2)->dist;
}

//updates the pathing maps. just needs the player character's position
//I could hard code a function to get the player character's position, but leaving it like this allows me to make a pathing map with a different focus
void updatePath(int xfrom, int yfrom, int floor)
{
	static int currFloor = 0;
    if(floor != currFloor)
    {
    	currFloor = floor;
        if(!(tunnelerPath = (path**)malloc(dungeon_y * sizeof(path_t*))))
        {
        	cout << "Error allocating space for tunnelerPath" << endl;
        	return;
        }

        if(!(nonTunnelerPath = (path**)malloc(dungeon_y * sizeof(path_t*))))
		{
        	cout << "Error allocating space for nontunnelerPath\n" << endl;
        	return;
        }

        int i;
        for(i = 0; i < dungeon_y; i++)
        {
            tunnelerPath[i] = (path*)malloc(sizeof(path_t) * dungeon_x);
            nonTunnelerPath[i] = (path*)malloc(sizeof(path_t) * dungeon_x);
        }
    }
    dungeon = getDungeon();
    getTunnelerPath(tunnelerPath, dungeon, xfrom, yfrom);
    getNonTunnelerPath(nonTunnelerPath, dungeon, xfrom, yfrom);

}

//helper function to convert hardness to a normalized value
int weightedHardness(int hardness)
{
    if(hardness < 85)
    {
        return 1;
    }
    if(hardness >84 && hardness < 171)
    {
        return 2;
    }
    if(hardness < 255)
    {
        return 3;
    }
    return INT_MAX; //found an edge block. Don't want these getting added to the queue.
}

void getNonTunnelerPath(path_t** dist, cell_t** d, int xfrom, int yfrom)
{
    heap_t h;

    heap_init(&h, cmp, NULL);

    path_t* currNode;

    int y, x;
//    int changed = 0;
    for(y = 0; y < dungeon_y; y++)
    {
        for(x = 0; x < dungeon_x; x++)
        {
            dist[y][x].xpos = x;
            dist[y][x].ypos = y;

            dist[y][x].node = heap_insert(&h, &dist[y][x]);

            if(x != xfrom || y != yfrom)
            {
                dist[y][x].dist = INT_MAX;
            }
            else
            {
                dist[y][x].dist = 0;
            }
        }
    }

//    changed = (int)dist[100][156].node;

    while((currNode = (path_t*)heap_remove_min(&h)))
    {
       currNode->node = NULL; //not sure why nulling this is a good thing, but ok

       unsigned char neighbors[8*2] = {
                            currNode->xpos, (unsigned char)(currNode->ypos + 1),
							(unsigned char)(currNode->xpos + 1), (unsigned char)(currNode->ypos + 1),
							(unsigned char)(currNode->xpos + 1), currNode->ypos,
							(unsigned char)(currNode->xpos + 1), (unsigned char)(currNode->ypos - 1),
                            currNode->xpos, (unsigned char)(currNode->ypos - 1),
							(unsigned char)(currNode->xpos - 1), (unsigned char)(currNode->ypos - 1),
							(unsigned char)(currNode->xpos - 1), currNode->ypos,
							(unsigned char)(currNode->xpos - 1), (unsigned char)(currNode->ypos + 1),
       };

       int i;
       unsigned int alt = 0;
       //for every neighbor
       for(i = 0; i < 8*2; i += 2)
       {
           //alt distance = distance at the current node + distance to the neighbor (distance in this case is hardness)
           if(((int)neighbors[i] > 0) && ((int)neighbors[i] < dungeon_x) && ((int)neighbors[i + 1] > 0) && ((int)neighbors[i + 1] < dungeon_y))
           {
               int hard = d[(int)neighbors[i+1]][(int)neighbors[i]].hardness; //distance to the neighboring node from the current node
               if(hard == 0)
               {
                   unsigned int dis = (unsigned int)dist[(int)currNode->ypos][(int)currNode->xpos].dist; //distance from the starting node to the current node
                   unsigned int dis2 = (unsigned int)dist[((int)neighbors[i+1])][((int)neighbors[i])].dist; //current distance from the starting node to the neighboring node (always INT_MAX until updated)
                   int wh = weightedHardness(hard); //weighted distance to the neighboring node


                   alt = dis + wh; //total distance to the neighboring node
                   if(alt < dis2 && (dist[(int)neighbors[i+1]][(int)neighbors[i]].node != NULL))
                   {
                       dist[(int)neighbors[i+1]][(int)neighbors[i]].dist = alt;
                       heap_decrease_key_no_replace(&h, dist[(int)neighbors[i+1]][(int)neighbors[i]].node);
                   }
               }
           }
       }
    }
}

//prints out a pathing map. 1 for tunneler map
void displayPath(char t)
{
    int x, y;
    printf("\n");
//    cout << endl;
    for(y = 1; y < dungeon_y - 1; y++)
    {
        printf("%5d ", y+1);
        for(x = 1; x < dungeon_x - 1; x++)
        {
            if(tunnelerPath[y][x].dist == 0)
            {
                //printf(BOLDBLACK "%c" RESET, PLAYER);
            	printf("%c", PLAYER);
            }
            else if(t == 1)
            {
                printf("%d", tunnelerPath[y][x].dist % 10);
            }
            else if(nonTunnelerPath[y][x].dist < INT_MAX)
            {
                printf("%d", nonTunnelerPath[y][x].dist % 10);
            }
            else
            {
                printf(" ");
            }
        }
        printf("\n");
    }
}

//this doesn't really account for INTELLIGENT since a straight line is the shortest path
void getTunnelerPath(path_t** dist, cell_t** d, int xfrom, int yfrom)
{
    heap_t h;

    heap_init(&h, cmp, NULL);
    path_t* currNode;
    int y, x;
    for(y = 0; y < dungeon_y; y++)
    {
        for(x = 0; x < dungeon_x; x++)
        {
            dist[y][x].xpos = x;
            dist[y][x].ypos = y;

            dist[y][x].node = heap_insert(&h, &dist[y][x]);

            if(x != xfrom || y != yfrom)
            {
                dist[y][x].dist = INT_MAX;
            }
            else
            {
                dist[y][x].dist = 0;
            }
        }
    }

    while((currNode = (path_t*)heap_remove_min(&h)))
    {
       currNode->node = NULL; //not sure why nulling this is a good thing, but ok

       unsigned char neighbors[8*2] = {
                            currNode->xpos, (unsigned char)(currNode->ypos + 1),
							(unsigned char)(currNode->xpos + 1), (unsigned char)(currNode->ypos + 1),
							(unsigned char)(currNode->xpos + 1), currNode->ypos,
							(unsigned char)(currNode->xpos + 1), (unsigned char)(currNode->ypos - 1),
                            currNode->xpos, (unsigned char)(currNode->ypos - 1),
							(unsigned char)(currNode->xpos - 1), (unsigned char)(currNode->ypos - 1),
							(unsigned char)(currNode->xpos - 1), currNode->ypos,
							(unsigned char)(currNode->xpos - 1), (unsigned char)(currNode->ypos + 1),
       };

       int i;
       unsigned int alt = 0;
       //for every neighbor
       for(i = 0; i < 8*2; i += 2)
       {
           //alt distance = distance at the current node + distance to the neighbor (distance in this case is hardness)
           if(((int)neighbors[i] > 0) && ((int)neighbors[i] < dungeon_x) && ((int)neighbors[i + 1] > 0) && ((int)neighbors[i + 1] < dungeon_y) && dist[(int)neighbors[i+1]][(int)neighbors[i]].node != NULL)
           {
               unsigned int dis = (int)dist[(unsigned int)currNode->ypos][(int)currNode->xpos].dist;
               unsigned int dis2 = (unsigned int)dist[((int)neighbors[i+1])][((int)neighbors[i])].dist;
               int hard = d[(int)neighbors[i+1]][(int)neighbors[i]].hardness;
               int wh = weightedHardness(hard);

               alt = dis + wh;
               if(alt < dis2)
               {
                   dist[(int)neighbors[i+1]][(int)neighbors[i]].dist = alt;
                   if(dist[(int)neighbors[i+1]][(int)neighbors[i]].node == NULL)
                   {
                       printf("%d , %d", (int)neighbors[i+1], (int)neighbors[i]);
                   }
                   heap_decrease_key_no_replace(&h, dist[(int)neighbors[i+1]][(int)neighbors[i]].node);
               }
           }
       }
    }
}

//Moves update the character's position
void move_left(creature* c)
{
	char needUpdate = 0;
	if((unsigned char)c->posx > 1)
	{
		if(!(c->characteristics & GHOST))
		{
			if(dungeon[(unsigned char)c->posy][(unsigned char)c->posx - 1].hardness > 0)
			{
				if(c->characteristics & TUNNELER)
				{
					dungeon[(unsigned char)c->posy][(unsigned char)c->posx - 1].hardness = max(dungeon[(unsigned char)c->posy][(unsigned char)c->posx - 1].hardness - 85, 0);
				}
				if(dungeon[(unsigned char)c->posy][(unsigned char)c->posx - 1].hardness > 0)
				{
					return;
				}
				dungeon[(unsigned char)c->posy][(unsigned char)c->posx - 1].tile = PATH3;
				needUpdate = 1;
			}
		}


		c->posx = c->posx - 1;

		if((c->characteristics & PICKITEMS) || (c->characteristics & DESTITEMS))
		{
			if(itemMap[c->posy][c->posx]->name.compare(""))
			{
				if(c->characteristics & PICKITEMS)
				{
					int pos = c->addItem(itemMap[c->posy][c->posx]);
					c->equipItem(pos);
				}
				nullItem(c->posx, c->posy);
			}
		}

		if(needUpdate)
		{
			//this has to come after the position update in case the creature moving is the player
			updatePath((unsigned char)player->posx, (unsigned char)player->posy, floorNum);
		}
		setMonster(c);
		nullMonster((unsigned char)c->posx + 1, (unsigned char)c->posy);
	}
}

void move_right(creature* c)
{
	char needUpdate = 0;
	if((unsigned char)c->posx < dungeon_x - 1)
	{
		if(!(c->characteristics & GHOST))
		{
			if(dungeon[(unsigned char)c->posy][(unsigned char)c->posx + 1].hardness > 0)
			{
				if(c->characteristics & TUNNELER)
				{
					dungeon[(unsigned char)c->posy][(unsigned char)c->posx + 1].hardness = max(dungeon[(unsigned char)c->posy][(unsigned char)c->posx + 1].hardness - 85, 0);
				}
				if(dungeon[(unsigned char)c->posy][(unsigned char)c->posx + 1].hardness > 0)
				{
					return;
				}
				dungeon[(unsigned char)c->posy][(unsigned char)c->posx + 1].tile = PATH3;
				needUpdate = 1;
			}
		}

		c->posx = c->posx + 1;
		if((c->characteristics & PICKITEMS) || (c->characteristics & DESTITEMS))
		{
			if(itemMap[c->posy][c->posx]->name.compare(""))
			{
				if(c->characteristics & PICKITEMS)
				{
					int pos = c->addItem(itemMap[c->posy][c->posx]);
					c->equipItem(pos);
				}
				nullItem(c->posx, c->posy);
			}
		}
		if(needUpdate)
		{
			//this has to come after the position update in case the creature moving is the player
			updatePath((unsigned char)player->posx, (unsigned char)player->posy, floorNum);
		}
		setMonster(c);
		nullMonster((unsigned char)c->posx - 1, (unsigned char)c->posy);
	}
}

void move_up(creature* c)
{
	char needUpdate = 0;
	if((unsigned char)c->posy > 1)
	{
		if(!(c->characteristics & GHOST))
		{
			if(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx].hardness > 0)
			{
				if(c->characteristics & TUNNELER)
				{
					dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx].hardness = max(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx].hardness - 85, 0);
				}
				if(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx].hardness > 0)
				{
					return;
				}
				dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx].tile = PATH3;
				needUpdate = 1;
			}
		}

		c->posy = c->posy - 1;
		if((c->characteristics & PICKITEMS) || (c->characteristics & DESTITEMS))
		{
			if(itemMap[c->posy][c->posx]->name.compare(""))
			{
				if(c->characteristics & PICKITEMS)
				{
					int pos = c->addItem(itemMap[c->posy][c->posx]);
					c->equipItem(pos);
				}
				nullItem(c->posx, c->posy);
			}
		}
		if(needUpdate)
		{
			//this has to come after the position update in case the creature moving is the player
			updatePath((unsigned char)player->posx, (unsigned char)player->posy, floorNum);
		}
		setMonster(c);
		nullMonster((unsigned char)c->posx, (unsigned char)c->posy + 1);
	}
}

void move_down(creature* c)
{
	char needUpdate = 0;
	if((unsigned char)c->posy < dungeon_y - 1)
	{
		if(!(c->characteristics & GHOST))
		{
			if(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx].hardness > 0)
			{
				if(c->characteristics & TUNNELER)
				{
					dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx].hardness = max(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx].hardness - 85, 0);
				}
				if(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx].hardness > 0)
				{
					return;
				}
				dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx].tile = PATH3;
				needUpdate = 1;
			}
		}

		c->posy = c->posy + 1;
		if((c->characteristics & PICKITEMS) || (c->characteristics & DESTITEMS))
		{
			if(itemMap[c->posy][c->posx]->name.compare(""))
			{
				if(c->characteristics & PICKITEMS)
				{
					int pos = c->addItem(itemMap[c->posy][c->posx]);
					c->equipItem(pos);
				}
				nullItem(c->posx, c->posy);
			}
		}
		if(needUpdate)
		{
			//this has to come after the position update in case the creature moving is the player
			updatePath((unsigned char)player->posx, (unsigned char)player->posy, floorNum);
		}
		setMonster(c);
		nullMonster((unsigned char)c->posx, (unsigned char)c->posy - 1);
	}
}

void move_upleft(creature* c)
{
	char needUpdate = 0;
	if((unsigned char)c->posy > 1 && (unsigned char)c->posx > 1)
	{
		if(!(c->characteristics & GHOST))
		{
			if(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx - 1].hardness > 0)
			{
				if(c->characteristics & TUNNELER)
				{
					dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx - 1].hardness = max(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx - 1].hardness - 85, 0);
				}
				if(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx - 1].hardness > 0)
				{
					return;
				}
				dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx - 1].tile = PATH3;
				needUpdate = 1;
			}
		}

		c->posy = c->posy - 1;
		c->posx = c->posx - 1;
		if((c->characteristics & PICKITEMS) || (c->characteristics & DESTITEMS))
		{
			if(itemMap[c->posy][c->posx]->name.compare(""))
			{
				if(c->characteristics & PICKITEMS)
				{
					int pos = c->addItem(itemMap[c->posy][c->posx]);
					c->equipItem(pos);
				}
				nullItem(c->posx, c->posy);
			}
		}
		if(needUpdate)
		{
			//this has to come after the position update in case the creature moving is the player
			updatePath((unsigned char)player->posx, (unsigned char)player->posy, floorNum);
		}
		setMonster(c);
		nullMonster((unsigned char)c->posx + 1, (unsigned char)c->posy + 1);
	}
}

void move_downright(creature* c)
{
	char needUpdate = 0;
	if((unsigned char)c->posy < dungeon_y - 1 && (unsigned char)c->posx < dungeon_x - 1)
	{
		if(!(c->characteristics & GHOST))
		{
			if(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx + 1].hardness > 0)
			{
				if(c->characteristics & TUNNELER)
				{
					dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx + 1].hardness = max(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx + 1].hardness - 85, 0);
				}
				if(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx + 1].hardness > 0)
				{
					return;
				}
				dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx + 1].tile = PATH3;
				needUpdate = 1;
			}
		}

		c->posy = c->posy + 1;
		c->posx = c->posx + 1;
		if((c->characteristics & PICKITEMS) || (c->characteristics & DESTITEMS))
		{
			if(itemMap[c->posy][c->posx]->name.compare(""))
			{
				if(c->characteristics & PICKITEMS)
				{
					int pos = c->addItem(itemMap[c->posy][c->posx]);
					c->equipItem(pos);
				}
				nullItem(c->posx, c->posy);
			}
		}
		if(needUpdate)
		{
			//this has to come after the position update in case the creature moving is the player
			updatePath((unsigned char)player->posx, (unsigned char)player->posy, floorNum);
		}
		setMonster(c);
		nullMonster((unsigned char)c->posx - 1, (unsigned char)c->posy - 1);
	}
}

void move_upright(creature* c)
{
	char needUpdate = 0;
	if((unsigned char)c->posy > 1 && (unsigned char)c->posx < dungeon_x - 1)
	{
		if(!(c->characteristics & GHOST))
		{
			if(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx + 1].hardness > 0)
			{
				if(c->characteristics & TUNNELER)
				{
					dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx + 1].hardness = max(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx + 1].hardness - 85, 0);
				}
				if(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx + 1].hardness > 0)
				{
					return;
				}
				dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx + 1].tile = PATH3;
				needUpdate = 1;
			}
		}

		c->posy = c->posy - 1;
		c->posx = c->posx + 1;
		if((c->characteristics & PICKITEMS) || (c->characteristics & DESTITEMS))
		{
			if(itemMap[c->posy][c->posx]->name.compare(""))
			{
				if(c->characteristics & PICKITEMS)
				{
					int pos = c->addItem(itemMap[c->posy][c->posx]);
					c->equipItem(pos);
				}
				nullItem(c->posx, c->posy);
			}
		}
		if(needUpdate)
		{
			//this has to come after the position update in case the creature moving is the player
			updatePath((unsigned char)player->posx, (unsigned char)player->posy, floorNum);
		}
		setMonster(c);
		nullMonster((unsigned char)c->posx - 1, (unsigned char)c->posy + 1);
	}
}

void move_downleft(creature* c)
{
	char needUpdate = 0;
	if((unsigned char)c->posy < dungeon_y - 1 && (unsigned char)c->posx > 1)
	{
		if(!(c->characteristics & GHOST))
		{
			if(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx - 1].hardness > 0)
			{
				if(c->characteristics & TUNNELER)
				{
					dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx - 1].hardness = max(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx - 1].hardness - 85, 0);
				}
				if(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx - 1].hardness > 0)
				{
					return;
				}
				dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx - 1].tile = PATH3;
				needUpdate = 1;
			}
		}

		c->posy = c->posy + 1;
		c->posx = c->posx - 1;
		if((c->characteristics & PICKITEMS) || (c->characteristics & DESTITEMS))
		{
			if(itemMap[c->posy][c->posx]->name.compare(""))
			{
				if(c->characteristics & PICKITEMS)
				{
					int pos = c->addItem(itemMap[c->posy][c->posx]);
					c->equipItem(pos);
				}
				nullItem(c->posx, c->posy);
			}
		}
		if(needUpdate)
		{
			//this has to come after the position update in case the creature moving is the player
			updatePath((unsigned char)player->posx, (unsigned char)player->posy, floorNum);
		}
		setMonster(c);
		nullMonster((unsigned char)c->posx + 1, (unsigned char)c->posy - 1);
	}
}

void callMove(int val, creature* mon)
{
	if(look(val, mon) == 0)
	{
		kill((MV_t)val, mon);
	}
	move_creature(val, mon);
}

//wtf, this should not work, there is no declaration of move anywhere... I guess all calls to it do happen after it's created
void move_creature(int val, creature* mon)
{
	move_table[val](mon);
}

//look to see what the tile nearby is like, a 0 means there is a creature there, while a 1 means an empty room/corridor, 256 means edge, and anything else is rock
int look_left(creature* c)
{
	//special case
	if(dungeon[(unsigned char)c->posy][(unsigned char)c->posx - 1].hardness > 0)
	{
		if(isMonsterDead((unsigned char)c->posx - 1, (unsigned char)c->posy))
		{
			return 2;
		}
		else
		{
			return 3;
		}
	}
	else
	{
		if(isMonsterDead((unsigned char)c->posx - 1, (unsigned char)c->posy))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

int look_right(creature* c)
{
	if(dungeon[(unsigned char)c->posy][(unsigned char)c->posx + 1].hardness > 0)
	{
		if(isMonsterDead((unsigned char)c->posx + 1, (unsigned char)c->posy))
		{
			return 2;
		}
		else
		{
			return 3;
		}
	}
	else
	{
		if(isMonsterDead((unsigned char)c->posx + 1, (unsigned char)c->posy))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

int look_up(creature* c)
{
	if(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx].hardness > 0)
	{
		if(isMonsterDead((unsigned char)c->posx, (unsigned char)c->posy - 1))
		{
			return 2;
		}
		else
		{
			return 3;
		}
	}
	else
	{
		if(isMonsterDead((unsigned char)c->posx, (unsigned char)c->posy - 1))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

int look_down(creature* c)
{
	if(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx].hardness > 0)
	{
		if(isMonsterDead((unsigned char)c->posx, (unsigned char)c->posy + 1))
		{
			return 2;
		}
		else
		{
			return 3;
		}
	}
	else
	{
		if(isMonsterDead((unsigned char)c->posx, (unsigned char)c->posy + 1))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}}

int look_upleft(creature* c)
{
	if(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx - 1].hardness > 0)
	{
		if(isMonsterDead((unsigned char)c->posx - 1, (unsigned char)c->posy - 1))
		{
			return 2;
		}
		else
		{
			return 3;
		}
	}
	else
	{
		if(isMonsterDead((unsigned char)c->posx - 1, (unsigned char)c->posy - 1))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

int look_downright(creature* c)
{
	if(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx + 1].hardness > 0)
	{
		if(isMonsterDead((unsigned char)c->posx + 1, (unsigned char)c->posy + 1))
		{
			return 2;
		}
		else
		{
			return 3;
		}
	}
	else
	{
		if(isMonsterDead((unsigned char)c->posx + 1, (unsigned char)c->posy + 1))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

int look_upright(creature* c)
{
	if(dungeon[(unsigned char)c->posy - 1][(unsigned char)c->posx + 1].hardness > 0)
	{
		if(isMonsterDead((unsigned char)c->posx + 1, (unsigned char)c->posy - 1))
		{
			return 2;
		}
		else
		{
			return 3;
		}
	}
	else
	{
		if(isMonsterDead((unsigned char)c->posx + 1, (unsigned char)c->posy - 1))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

int look_downleft(creature* c)
{
	if(dungeon[(unsigned char)c->posy + 1][(unsigned char)c->posx - 1].hardness > 0)
	{
		if(isMonsterDead((unsigned char)c->posx - 1, (unsigned char)c->posy + 1))
		{
			return 2;
		}
		else
		{
			return 3;
		}
	}
	else
	{
		if(isMonsterDead((unsigned char)c->posx - 1, (unsigned char)c->posy + 1))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

unsigned int attack_left(creature* c)
{
	creature* target = getMonster(c->posx - 1, c->posy);
	if(c->priority == 0 || target->priority == 0)
	{
		unsigned int r = rand();
		if((r % 100) > custMin(95, c->acc) || (r % 100) < custMin(85, target->dodge))
		{
			return 0;
		}

		unsigned int dmg = custMax(1, c->damage->roll() - target->armor);
		target->hitpoints = target->hitpoints - dmg;
		if(target->hitpoints < 0)
		{
			int i;
			for(i = 0; i < 20; i++)
			{
				if(target->inv[i] != 0)
				{
					target->dropItem(i);
				}
				if(i < 13)
				{
					if(target->equip[i] != 0)
					{
						target->unequipItem(i);
						target->dropItem(0);
					}
				}
			}

			target->isDead = 1;
		}
		return dmg;
	}
	return INT_MAX;
}

unsigned int attack_right(creature* c)
{
	creature* target = getMonster(c->posx + 1, c->posy);
	if(c->priority == 0 || target->priority == 0)
	{
		unsigned int r = rand();
		if((r % 100) > custMin(95, c->acc) || (r % 100) < custMin(85, target->dodge))
		{
			return 0;
		}

		unsigned int dmg = custMax(1, c->damage->roll() - target->armor);
		target->hitpoints = target->hitpoints - dmg;
		if(target->hitpoints < 0)
		{
			int i;
			for(i = 0; i < 20; i++)
			{
				if(target->inv[i] != 0)
				{
					target->dropItem(i);
				}
				if(i < 13)
				{
					if(target->equip[i] != 0)
					{
						target->unequipItem(i);
						target->dropItem(0);
					}
				}
			}

			target->isDead = 1;
		}
		return dmg;
	}
	return INT_MAX;
}

unsigned int attack_up(creature* c)
{
	creature* target = getMonster(c->posx, c->posy - 1);
	if(c->priority == 0 || target->priority == 0)
	{
		unsigned int r = rand();
		if((r % 100) > custMin(95, c->acc) || (r % 100) < custMin(85, target->dodge))
		{
			return 0;
		}

		unsigned int dmg = custMax(1, c->damage->roll() - target->armor);
		target->hitpoints = target->hitpoints - dmg;
		if(target->hitpoints < 0)
		{
			int i;
			for(i = 0; i < 20; i++)
			{
				if(target->inv[i] != 0)
				{
					target->dropItem(i);
				}
				if(i < 13)
				{
					if(target->equip[i] != 0)
					{
						target->unequipItem(i);
						target->dropItem(0);
					}
				}
			}

			target->isDead = 1;
		}
		return dmg;
	}
	return INT_MAX;
}

unsigned int attack_down(creature* c)
{
	creature* target = getMonster(c->posx, c->posy + 1);
	if(c->priority == 0 || target->priority == 0)
	{
		unsigned int r = rand();
		if((r % 100) > custMin(95, c->acc) || (r % 100) < custMin(85, target->dodge))
		{
			return 0;
		}

		unsigned int dmg = custMax(1, c->damage->roll() - target->armor);
		target->hitpoints = target->hitpoints - dmg;
		if(target->hitpoints < 0)
		{
			int i;
			for(i = 0; i < 20; i++)
			{
				if(target->inv[i] != 0)
				{
					target->dropItem(i);
				}
				if(i < 13)
				{
					if(target->equip[i] != 0)
					{
						target->unequipItem(i);
						target->dropItem(0);
					}
				}
			}

			target->isDead = 1;
		}
		return dmg;
	}
	return INT_MAX;
}

unsigned int attack_upleft(creature* c)
{
	creature* target = getMonster(c->posx - 1, c->posy - 1);

	if(c->priority == 0 || target->priority == 0)
	{
		unsigned int r = rand();
		if((r % 100) > custMin(95, c->acc) || (r % 100) < custMin(85, target->dodge))
		{
			return 0;
		}

		unsigned int dmg = custMax(1, c->damage->roll() - target->armor);
		target->hitpoints = target->hitpoints - dmg;
		if(target->hitpoints < 0)
		{
			int i;
			for(i = 0; i < 20; i++)
			{
				if(target->inv[i] != 0)
				{
					target->dropItem(i);
				}
				if(i < 13)
				{
					if(target->equip[i] != 0)
					{
						target->unequipItem(i);
						target->dropItem(0);
					}
				}
			}

			target->isDead = 1;
		}
		return dmg;
	}
	return INT_MAX;
}

unsigned int attack_downright(creature* c)
{
	creature* target = getMonster(c->posx + 1, c->posy + 1);

	if(c->priority == 0 || target->priority == 0)
	{
		unsigned int r = rand();
		if((r % 100) > custMin(95, c->acc) || (r % 100) < custMin(85, target->dodge))
		{
			return 0;
		}

		unsigned int dmg = custMax(1, c->damage->roll() - target->armor);
		target->hitpoints = target->hitpoints - dmg;
		if(target->hitpoints < 0)
		{
			int i;
			for(i = 0; i < 20; i++)
			{
				if(target->inv[i] != 0)
				{
					target->dropItem(i);
				}
				if(i < 13)
				{
					if(target->equip[i] != 0)
					{
						target->unequipItem(i);
						target->dropItem(0);
					}
				}
			}

			target->isDead = 1;
		}
		return dmg;
	}
	return INT_MAX;
}

unsigned int attack_upright(creature* c)
{
	creature* target = getMonster(c->posx + 1, c->posy - 1);

	if(c->priority == 0 || target->priority == 0)
	{
		unsigned int r = rand();
		if((r % 100) > custMin(95, c->acc) || (r % 100) < custMin(85, target->dodge))
		{
			return 0;
		}

		unsigned int dmg = custMax(1, c->damage->roll() - target->armor);
		target->hitpoints = target->hitpoints - dmg;
		if(target->hitpoints < 0)
		{
			int i;
			for(i = 0; i < 20; i++)
			{
				if(target->inv[i] != 0)
				{
					target->dropItem(i);
				}
				if(i < 13)
				{
					if(target->equip[i] != 0)
					{
						target->unequipItem(i);
						target->dropItem(0);
					}
				}
			}

			target->isDead = 1;
		}
		return dmg;
	}
	return INT_MAX;
}

unsigned int attack_downleft(creature* c)
{
	creature* target = getMonster(c->posx - 1, c->posy + 1);

	if(c->priority == 0 || target->priority == 0)
	{
		unsigned int r = rand();
		if((r % 100) > custMin(95, c->acc) || (r % 100) < custMin(85, target->dodge))
		{
			return 0;
		}

		unsigned int dmg = custMax(1, c->damage->roll() - target->armor);
		move(22,0);
		clrtoeol();
		target->hitpoints = target->hitpoints - dmg;
		if(target->hitpoints < 0)
		{
			int i;
			for(i = 0; i < 20; i++)
			{
				if(target->inv[i] != 0)
				{
					target->dropItem(i);
				}
				if(i < 13)
				{
					if(target->equip[i] != 0)
					{
						target->unequipItem(i);
						target->dropItem(0);
					}
				}
			}

			target->isDead = 1;
		}
		return dmg;
	}
	return INT_MAX;
}

int look(int val, creature* mon)
{
	return look_table[val](mon);
}

void attack(int val, creature* mon)
{
	unsigned int g = attack_table[val](mon);
	if(g == 0)
	{
		if(mon->priority == 0)
		{
			mvprintw(22, 0, "                              ");
			mvprintw(22, 0, "%s", missed[rand() % numPlayerMissedQuips].c_str());
		}
		else
		{
			move(22, 30);
			clrtoeol();
			mvprintw(22, 30, "%s", missed[(rand() % numCreatureMissedQuips) + numPlayerMissedQuips].c_str());
		}
		refresh();
	}
	else if(g < INT_MAX)
	{
		if(mon->priority == 0)
		{
			mvprintw(22, 0, "                              ");
			mvprintw(22, 0, "%s, did %u dmg!", hit[rand() % numHitQuips].c_str(), g);
		}
		else
		{
			move(22, 30);
			clrtoeol();
			mvprintw(22, 30, "%s, took %u dmg!", hit[rand() % numHitQuips].c_str(), g);
		}
		refresh();
	}
}

int update()
{
//	static unsigned int numCreatureMoves = 0;
//	static unsigned int numPlayerMoves = 0;

	creature* tempCreature;

	//this is a label for the goto statement in the event of a dead creature being found
	startupdate:

	if(!(tempCreature = (creature*)heap_peek_min(&eventQ)))
	{
		fprintf(stderr, "Error: no creature in queue.");
		return 1;
	}

	if(tempCreature->isDead)
	{
		heap_remove_min(&eventQ);
		//free up the monster and replace it with a null monster
		delete(tempCreature);
		numMonsters--;
		goto startupdate; //VERY BAD TO USE GOTO!
		//update();
	}
	if(tempCreature->nextMove < gameTime)
	{
		//this shouldn't be needed, but I don't know why it's happening
		heap_remove_min(&eventQ);
		tempCreature->nextMove = gameTime + (int)(1000 / tempCreature->speed);
		heap_insert(&eventQ, tempCreature);
		goto startupdate; //VERY BAD TO USE GOTO!
		//update();

	}

	while(tempCreature->nextMove == gameTime)
	{
//		numCreatureMoves++;

		if(tempCreature->isDead)
		{
			heap_remove_min(&eventQ);
			//free up the monster and replace it with a null monster
			//nullMonster(tempCreature->posx, tempCreature->posy); //this was done already
			delete(tempCreature);
			numMonsters--;
			goto startupdate; //VERY BAD TO USE GOTO!
			//update();
		}
		else if(!isNotPlayer(tempCreature))
		{
//			numPlayerMoves++;
			move(0,0);
			clrtoeol();
			mvprintw(0,0,"Players Turn %d, %d", player->posx, player->posy);
			move(23,0);
			clrtoeol();
			mvprintw(23,0,"HP: %d | Arm %d | Dam: %d+%dd%d | Dodge: %d | Accuracy: %d | LOS: %d",
					player->hitpoints, player->armor, player->damage->base, player->damage->numDice, player->damage->sidesPerDie, custMin(player->dodge, 85), custMin(player->acc, 85), player->los);
			refresh();
			heap_remove_min(&eventQ);
			displayDungeon();
			move_player(getch());
			tempCreature->nextMove = gameTime + (int)(1000 / tempCreature->speed);
			heap_insert(&eventQ, tempCreature);
		}
		else
		{
			heap_remove_min(&eventQ);

			if(!(tempCreature->characteristics & TUNNELER) && !(tempCreature->characteristics & GHOST))
			{
				nonTunnelerMovement(tempCreature);
			}
			else
			{
				tunnelerMovement(tempCreature);
			}

			tempCreature->nextMove = gameTime + (int)(1000 / tempCreature->speed);
			heap_insert(&eventQ, tempCreature);
		}

		if(!(tempCreature = (creature*)heap_peek_min(&eventQ)))
		{
			move(0,0);
			clrtoeol();
			mvprintw(0,0, "Error: null creature in queue %d.", gameTime);
			refresh();
			fprintf(stderr, "Error: null creature in queue.");
			return 1;
		}
	}

	if(player->isDead)
	{
		displayDungeon();
		move(0,0);
		clrtoeol();
		mvprintw(0,0,"Player dead at %lu\n", gameTime);

		refresh();
		usleep(10000);
		freePaths();
	}
	gameTime++;
	setFrame(gameTime);
	return player->isDead;
}

static int cmp_creature(const void *val1, const void *val2)
{
	//since nextMove is 1000/speed, the higher the speed, the smaller nextMove is.
	//This means we can use a min priority queue

	if(((creature*)val1)->nextMove == ((creature*)val2)->nextMove)
	{
		return ((creature*)val1)->priority - ((creature*)val2)->priority;
	}
	return ((creature*)val1)->nextMove - ((creature*)val2)->nextMove; //returns the priority for a creature
}

//Initialize the game with monsters and a player at the center of room 1
void initializeQ(int initialNumMons, int floor)
{
	int x;
	static bool gotCreatureTemplates = false;

	if(!gotCreatureTemplates)
	{
		if(getCreatureTemplates(&creatureTemplates))
		{
			cout << "Error getting creature templates!" << endl;
		}
		else
		{
			cout << "Got creature templates!" << endl;
			gotCreatureTemplates = true;
		}
	}

	gameTime = 0;

	unsigned char* rooms;
	rooms = (unsigned char*)malloc(200);
	if(getRooms(rooms))
	{
		//error
		fprintf(stderr, "Error getting rooms during initialize.\n");
		return;
	}

	itemMap = getItemMap();

	//add the player to the queue
	if(floor == 1)
	{
		heap_init(&eventQ, cmp_creature, NULL);
		player = initialize_player(rooms[0] + (rooms[2] / 2), rooms[1] + (rooms[3] / 2));
	}
	else
	{
		player->posx = rooms[0] + (rooms[2] / 2);
		player->posy = rooms[1] + (rooms[3] / 2);
	}
	numMonsters = 1;

	setMonster(player);
	updatePath(player->posx, player->posy, floorNum);
	player->nextMove = gameTime + (1000 / player->speed);
	heap_insert(&eventQ, player);

	int numRooms = getNumOfRooms();

	//make a number of monsters
	for(x = 0; x < initialNumMons; x++)
	{
		int randomCreature = rand() % creatureTemplates.size();

		creature* newMon = new creature(creatureTemplates[randomCreature], numMonsters); //monster from a template, randomly chosen

		numMonsters++;
		if(!(newMon->characteristics & TUNNELER))
		{
			int monset = 0;
			//creature is not a tunneler, and must go into an empty space
			while(!monset)
			{
				int randRoom = (rand() % (numRooms - 1)) + 1;
				int randX = rand() % rooms[randRoom * 4 + 2] + rooms[randRoom * 4];
				int randY = rand() % rooms[randRoom * 4 + 3] + rooms[randRoom * 4 + 1];

				//This has the potential to cause inifinite loops. if a room is full of monsters,
				//a new monster will never get placed. it will also take a long time if there
				//are many monsters in a room
				if(isMonsterDead(randX, randY))
				{
					newMon->posx = randX;
					newMon->posy = randY;
					newMon->nextMove = gameTime + (1000 / newMon->speed);
					//frees up the null monster
					releaseMonster(newMon->posx, newMon->posy);
					setMonster(newMon);
					heap_insert(&eventQ, newMon);
					monset = 1;
				}
			}
		}
		else if((newMon->characteristics & TUNNELER) || (newMon->characteristics & GHOST))
		{
			//tunneling monsters get spawned around the edge
			int monset = 0;
			while(!monset)
			{
				int randX = (rand() % (dungeon_x - 1)) + 1;
				int randY = (rand() % (dungeon_y - 1)) + 1;

				if((randX < (rooms[0] - 1) || (randX > ((rooms[0] + rooms[2]) + 1))) &&
						(randY < (rooms[1] - 1) || (randY > ((rooms[1] + rooms[3]) + 1)))
						&& isMonsterDead(randX, randY))
				{
					if(rand() % 2)
					{
						if(rand() % 2)
						{
							newMon->posx = 1;
						}
						else
						{
							newMon->posx = dungeon_x - 2;
						}
						newMon->posy = randY;
					}
					else
					{
						if(rand() % 2)
						{
							newMon->posy = 1;
						}
						else
						{
							newMon->posy = dungeon_y - 2;
						}
						newMon->posx = randX;
					}
					newMon->nextMove = gameTime + (1000 / newMon->speed);
					//frees up the null monster
					releaseMonster(newMon->posx, newMon->posy);
					setMonster(newMon);
					heap_insert(&eventQ, newMon);
					monset = 1;
				}
			}

		}
	}
	floorNum = floor;
	updateScreen();
	displayDungeon();
	free(rooms);
}

////creates a random item from the item templates
//void generateItem(item* i)
//{
//	i = new item(itemTemplates[rand() % itemTemplates.size()]);
//}

void removeMonster(creature* mon)
{

	delete(mon);
}

//don't think this would ever be used, but just in case
void freePaths()
{
	int i;
	for(i = 0; i < dungeon_y; i++)
	{
		free(tunnelerPath[i]);
		free(nonTunnelerPath[i]);
	}
	free(tunnelerPath);
	free(nonTunnelerPath);
}

//this does not move toward the last known player location,
//this moves toward the player if the creature knows where
//the player is, or randomly otherwise
void nonTunnelerMovement(creature* c)
{
	int erraticCheck = rand() % 1; //temporary

	//random movement, erratic check or creature doesn't know where player is
	if(((c->characteristics & ERATIC) && erraticCheck) ||
			(!playerInLOS(c) && !(c->characteristics & TELEPATH)))
	{
		//erratic movement
		enum MV_t randNum = (MV_t)0;
		int moved = 0;
		while(!moved)
		{
			if(!isNotPlayer(c))
			{
				randNum = (MV_t)(rand() % 4);
			}
			else
			{
				randNum = (MV_t)(rand() % 8);
			}
			/*
			 * possible results of look
			 * 0 - empty tile, no monster
			 * 1 - empty tile, monster
			 * 2 - rock tile, no monster
			 * 3 - rock tile, monster
			 */
			int l = look(randNum, c);
			if(l == 0)
			{
				move(24,0);
				clrtoeol();
				mvprintw(24, 0, "No creatures, moving");
				refresh();
				//empty cell with no creature, just go
				move_creature(randNum, c);
				moved = 1;
			}
			else if(l == 1 || l == 3)
			{
				move(24,0);
				clrtoeol();
				mvprintw(24, 0, "Creature detected, attacking");
				refresh();
				//theres a creature there
				attack(randNum, c);
				moved = 1;
			}
		}
		return;
	}

	if(playerInLOS(c) || (c->characteristics & TELEPATH))
	{
		int i, shortest;
		shortest = 500;
		enum MV_t bestMove = (MV_t)0;

		int pos[16] = {
				(unsigned char)c->posx - 1, (unsigned char)c->posy, //left
				(unsigned char)c->posx + 1, (unsigned char)c->posy, //right
				(unsigned char)c->posx, (unsigned char)c->posy - 1, //up
				(unsigned char)c->posx, (unsigned char)c->posy + 1, //down
				(unsigned char)c->posx - 1, (unsigned char)c->posy - 1, //upleft
				(unsigned char)c->posx + 1, (unsigned char)c->posy + 1, //downright
				(unsigned char)c->posx + 1, (unsigned char)c->posy - 1, //upright
				(unsigned char)c->posx - 1, (unsigned char)c->posy + 1 //downleft
		};


		for(i = 0; i < 8; i += 2)
		{
			if(nonTunnelerPath[pos[i + 1]][pos[i]].dist < shortest)
			{
				shortest = nonTunnelerPath[pos[i + 1]][pos[i]].dist;
				bestMove = (MV_t)(i / 2);
			}
		}
		int l = look(bestMove, c);
		if(l == 0)
		{
			move(24,0);
			clrtoeol();
			mvprintw(24, 0, "No creatures, moving");
			refresh();
			//empty cell with no creature, just go
			move_creature(bestMove, c);
		}
		else if(l == 1 || l == 3)
		{
			move(24,0);
			clrtoeol();
			mvprintw(24, 0, "Creature detected, attacking");
			refresh();
			//theres a creature there
			attack(bestMove, c);
		}
	}
}

//this does not move toward the last known player location,
//this moves toward the player if the creature knows where
//the player is, or randomly otherwise
void tunnelerMovement(creature* c)
{
	int erraticCheck = rand() % 1; //temporary

	//random movement, erratic check or creature doesn't know where player is
	if(((c->characteristics & ERATIC) && erraticCheck) ||
			(!playerInLOS(c) && !(c->characteristics & TELEPATH)))
	{
		//erratic movement
		enum MV_t randNum = (MV_t)0;
		int moved = 0;
		while(!moved)
		{
			if(!isNotPlayer(c))
			{
				randNum = (MV_t)(rand() % 4);
			}
			else
			{
				randNum = (MV_t)(rand() % 8);
			}
			int testCase = look(randNum, c);
			/*
			 * possible results of look
			 * 0 - empty tile, no monster
			 * 1 - empty tile, monster
			 * 2 - rock tile, no monster
			 * 3 - rock tile, monster
			 */
			if(testCase == 1 || testCase == 3)
			{
				move(24,0);
				clrtoeol();
				mvprintw(24, 0, "Creature detected, attacking (tunneler)");
				refresh();
				//theres a creature there
				attack(randNum, c);
				moved = 1;
			}
			else
			{
				move(24,0);
				clrtoeol();
				mvprintw(24, 0, "No creatures, moving (tunneler)");
				refresh();
				move_creature(randNum, c);
				moved = 1;
			}
		}
		return;
	}

	if(playerInLOS(c) || (c->characteristics & TELEPATH))
	{
		enum MV_t bestMove = (MV_t)0;
		if(c->characteristics & INTELLIGENT)
		{
			int i, shortest;
			shortest = 500;

			int pos[16] = {
					(unsigned char)c->posx - 1, (unsigned char)c->posy, //left
					(unsigned char)c->posx + 1, (unsigned char)c->posy, //right
					(unsigned char)c->posx, (unsigned char)c->posy - 1, //up
					(unsigned char)c->posx, (unsigned char)c->posy + 1, //down
					(unsigned char)c->posx - 1, (unsigned char)c->posy - 1, //upleft
					(unsigned char)c->posx + 1, (unsigned char)c->posy + 1, //downright
					(unsigned char)c->posx + 1, (unsigned char)c->posy - 1, //upright
					(unsigned char)c->posx - 1, (unsigned char)c->posy + 1 //downleft
			};


			for(i = 0; i < 8; i += 2)
			{
				if(tunnelerPath[pos[i + 1]][pos[i]].dist < shortest)
				{
					shortest = tunnelerPath[pos[i + 1]][pos[i]].dist;
					bestMove = (MV_t)(i / 2);
				}
			}
		}
		else
		{
			//unintelligent monsters do not move diagonally toward the player
			if(abs((unsigned char)c->posx - player->posx) > abs((unsigned char)c->posy - player->posy))
			{
				//if the difference is a positive value, the creature is right of the player
				if(((unsigned char)c->posx - player->posx) > 0)
				{
					bestMove = LEFT;
				}
				else
				{
					bestMove = RIGHT;
				}
			}
			else
			{
				//if the difference is a positive value, the creature is below the player
				if(((unsigned char)c->posy - player->posy) > 0)
				{
					bestMove = UP;
				}
				else
				{
					bestMove = RIGHT;
				}
			}
		}
		int l = look(bestMove, c);
		if(l == 1 || l == 3)
		{
			move(24,0);
			clrtoeol();
			mvprintw(24, 0, "Creature detected, attacking (tunneler)");
			refresh();
			//theres a creature there
			attack(bestMove, c);
		}
		else
		{
			move(24,0);
			clrtoeol();
			mvprintw(24, 0, "No creatures, moving (tunneler)");
			refresh();
			move_creature(bestMove, c);
		}
	}
}

//THIS HAS BECOME DEPRECATED
//kills the creature at the position that c is trying to move to
void kill(enum MV_t pos, creature* c)
{
	if(pos == LEFT)
	{
		getMonster((unsigned char)c->posx - 1, (unsigned char)c->posy)->isDead = 1;
	}
	else if(pos == RIGHT)
	{
		getMonster((unsigned char)c->posx + 1, (unsigned char)c->posy)->isDead = 1;
	}
	else if(pos == UP)
	{
		getMonster((unsigned char)c->posx, (unsigned char)c->posy - 1)->isDead = 1;
	}
	else if(pos == DOWN)
	{
		getMonster((unsigned char)c->posx, (unsigned char)c->posy + 1)->isDead = 1;
	}
	else if(pos == UPLEFT)
	{
		getMonster((unsigned char)c->posx - 1, (unsigned char)c->posy - 1)->isDead = 1;
	}
	else if(pos == DOWNRIGHT)
	{
		getMonster((unsigned char)c->posx + 1, (unsigned char)c->posy + 1)->isDead = 1;
	}
	else if(pos == UPRIGHT)
	{
		getMonster((unsigned char)c->posx + 1, (unsigned char)c->posy - 1)->isDead = 1;
	}
	else if(pos == DOWNLEFT)
	{
		getMonster((unsigned char)c->posx - 1, (unsigned char)c->posy + 1)->isDead = 1;
	}
}

//determines whether the player is in visible range of the creature
int playerInLOS(creature* c)
{
	//overly simple LOS. This makes a square area c->los distance from the creature visible, even through walls
	if(isNotPlayer(c) && ((unsigned int)abs(player->posx - c->posx) <= c->los) && ((unsigned int)abs(player->posy - c->posy) <= c->los))
	{
		return 1;
	}
	return 0;
}

int max(int val1, int val2)
{
	if(val1 > val2)
	{
		return val1;
	}
	return val2;
}

//empties the q and gets it ready for a new dungeon
void clearQ()
{
	creature* c;
	while((c = (creature*)heap_remove_min(&eventQ)))
	{
		if(isNotPlayer(c))
		{
			delete(c);
		}
	}
}
