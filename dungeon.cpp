/*
 * DungeonGenerator.c
 *
 *  Created on: Jan 18, 2017
 *      Author: Sean Jellison
 */

/*
 * This is the api for generating random dungeon.
 */

#include "dungeon.h" //I don't understand why this is needed, but it doesn't seem to read the macros in the header without it

#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include <iostream>

#include "player.h"
#include "creature.h"
#include "item.h"
#include "itemTemplate.h"
#include "itemParser.h"

using namespace std;

//I'm leaving these as defined because I don't know how to make a dynamically sized array
#define rows 105 //Maximum number of vertical cells
#define cols 160 //Maximum number of horizontal cells
#define maxRooms 25 * 2 //The maximum number of rooms * 2. Due to how rooms are placed, lower values put few rooms to the right and toward the bottom of the board.

static int frame = 0;
static int roomWidth = 5; //Provides a range for the width of a room. Min 7, max roomWidth + 7
static int roomLength = 6; //Provides a range for the length of a room. Min 5, max roomLength + 5
static int roomChance = 10; //This is the change per cell for a room to be attempted to be made. The higher the value, the more rooms.
static int roomCenters[maxRooms];
static int numOfRooms = 0;
static int scr_w[] = {0, 80}; //screen 1: column 0-79, screen 2: column 40-119, screen 3: column 80-159
static int scr_l[] = {0, 21, 42, 63, 84}; //screen 1: row 0-20, screen 2: 21-41, screen 3: 42-62, screen 4: 63-83, screen 5: 84-104
static int screen[] = {0,0};
static int floor_Num = 1;
static unsigned char rooms_[maxRooms * 4];
static bool gotItems = false;
static cell_t dungeon[rows][cols];
static creature* monsters[rows][cols];
static item* items[rows][cols];
static vector<itemTemplate*> it;

//Functions
void makePath(int, int*);
void updateChanges(int, int, int*);
void findPath(int, int, int*, int*);
int* generateRoom();
int canPlace(int, int, int*);
int getColor_creature(creature*);
int getColor_item(item*);
void setRoomWidth(int);
void setRoomLength(int);
void setRoomChance(int);
char getItemShape(item*);

void setRoomWidth(int val)
{
  if(val) //prevents change if the val is 0
  {
    roomWidth = val;
  }
}

void setRoomLength(int val)
{
  if(val)
  {
    roomLength = val;
  }
}

void setRoomChance(int val)
{
 if(val)
  {
    roomChance = val;
  }
}

void getRoomOne(int* arr)
{
    arr[0] = roomCenters[0];
    arr[1] = roomCenters[1];
}

cell_t** getDungeon()
{
    cell_t** dptr = (cell_t**)malloc(rows * sizeof(cell_t*));

    int i;
    for(i = 0; i < rows; i++)
    {
        dptr[i] = dungeon[i];
    }

    return dptr;
}

//returns the rooms with x and y coordinate of the top left of the room and the width and length of the room
int getRooms(unsigned char* arr)
{
    if(!(arr = (unsigned char*)realloc(arr, numOfRooms * 4))) //re-adjust the rooms array size
    {
//        fprintf(stderr, "Error reallocating memory for rooms.\n");
        return -1;
    }
    int i;
    for(i = 0; i < (numOfRooms * 4); i += 4)
    {
	//printf("Getting rooms Pre:%d %d %d\n",i/4, rooms_[i+1], rooms_[i+3]);
        arr[i] = (unsigned char)rooms_[i];
        arr[i + 1] = (unsigned char)rooms_[i + 1];
        arr[i + 2] = (unsigned char)rooms_[i + 2];
        arr[i + 3] = (unsigned char)rooms_[i + 3];
	//printf("Getting rooms: %d %d\n", arr[i+1], arr[i+3]);
    }
    return 0;
}

int getHardness(unsigned char* arr)
{
    int y;
    int x;
    for(y = 0; y < rows; y++)
    {
        for(x = 0; x < cols; x++)
        {
            arr[x + y*160] = (unsigned char) dungeon[y][x].hardness;
        }
    }
    return 0;
}

int getNumOfRooms()
{
    return numOfRooms;
}

int getFloorNum()
{
	return floor_Num;
}

void setFrame(int num)
{
	frame = num;
}

//currently only capable of going down
int isOnStairs(creature* c)
{
	return dungeon[c->posy][c->posx].tile == STAIRSDOWN;
}

//Generates a new dungeon by emptying several relevant data structures
void newDungeon()
{
	int i, j;

	clearQ();
	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < cols; j++)
		{
			if(isNotPlayer(monsters[i][j]))
			{
				releaseMonster(j, i);
				monsters[i][j] = new creature(0, 0);
			}

			delete(items[i][j]);
			items[i][j] = new item();

			dungeon[i][j].tile = EMPTY;
			dungeon[i][j].hardness = 0;
			dungeon[i][j].visited = 0;
			dungeon[i][j].visible = 0;
		}
	}
	for(i = 0; i < numOfRooms; i++)
	{
		roomCenters[i] = 0;
		rooms_[i * 4] = 0;
		rooms_[i * 4 + 1] = 0;
		rooms_[i * 4 + 2] = 0;
		rooms_[i * 4 + 3] = 0;
	}

	numOfRooms = 0;

	floor_Num++;
	generateDungeon(0, 0, 0);
	int r = (rand() % 18) + 3; //3-20 monsters
	initializeQ(r, floor_Num);
}

void initialize()
{
	updateScreen();
	initscr();
	raw();
	noecho();
	start_color();
	init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
	curs_set(0);
}

void killScreen()
{
	//exit(EXIT_SUCCESS);
	endwin();
}

void updateScreen()
{
	// 21 42 63 84
	creature* player = getPlayer();
	if(player->posy < 21)
	{
		screen[1] = 0;
	}
	else if(player->posy < 42)
	{
		screen[1] = 1;
	}
	else if(player->posy < 63)
	{
		screen[1] = 2;
	}
	else if(player->posy < 84)
	{
		screen[1] = 3;
	}
	else
	{
		screen[1] = 4;
	}

	//40 80
	if(player->posx < 80)
	{
		screen[0] = 0;
	}
	else
	{
		screen[0] = 1;
	}
}

void moveScreen(int dir)
{
	if(dir == 0) //left
	{
		if(screen[0] != 0)
		{
			screen[0]--;
		}
	}
	else if(dir == 1) //right
	{
		if(screen[0] != 1)
		{
			screen[0]++;
		}
	}
	else if(dir == 2) //up
	{
		if(screen[1] != 0)
		{
			screen[1]--;
		}
	}
	else if(dir == 3) //down
	{
		if(screen[1] != 4)
		{
			screen[1]++;
		}
	}
}

//Displays the dungeon into the command prompt
void displayDungeon()
{
	int i,j;

	//this may cause issues later
	if(frame == 0)
	{
		initialize();
	}

	creature* player = getPlayer();

	for(i = 0; i < 21; i++)
	{
		for(j = 0; j < 80; j++)
		{
	    	int y = i + scr_l[screen[1]];
	    	int x = j + scr_w[screen[0]];
	    	bool los = player->isInLOS(x, y);
			//i + scr_l[screen[1]] gives the row to be printed at + screen offset
			//j + scr_w[screen[0]] gives the column to be printed at + screen offset
	    	if(los)
	    	{
	    		dungeon[y][x].visible = 1;
	    	}
		    if(los && (monsters[y][x]->isDead == 0) && (y != 0) && (y != rows)
		    		&& (x != 0) && (x != cols))
		    {
		    	if(player->posx == x && player->posy == y && items[y][x]->name.compare(""))
		    	{
		    		move(22, 0);
		    		clrtoeol();
		    		mvprintw(22, 0, "Item is: %s", items[y][x]->name.c_str());
					refresh();
		    	}

		    	int clr = getColor_creature(monsters[y][x]);
		    	attron(COLOR_PAIR(clr));
		    	mvaddch(i+1, j, monsters[y][x]->type);
		    	attroff(COLOR_PAIR(clr));
		    }
		    else if(items[y][x]->name.compare("") && los)
		    {
		    	int clr = getColor_item(items[y][x]);
		    	attron(COLOR_PAIR(clr));
		    	mvaddch(i+1, j, getItemShape(items[y][x]));
		    	attroff(COLOR_PAIR(clr));
		    }
		    else if(los || dungeon[y][x].visible)
		    {
		    	//This is where all the tile changes would be made if there are animated tiles
		    	mvaddch(i+1, j, dungeon[y][x].tile);
		    }
		    else
		    {
		    	mvaddch(i+1, j, ' ');
		    }
		}
	}
	refresh();
}//end of displaydungeon

void loadDungeon(unsigned char* hardness, unsigned char* rms, int numrms)
{
	if(!gotItems)
	{
		if(getItemTemplates(&it))
		{
			cout << "Error getting items." << endl;
		}
		else
		{
			cout << "Got item templates! Got " << it.size() << " templates." << endl;
			gotItems = true;
		}
	}

    int y;
    int x;

    numOfRooms = numrms;
    //printf("Beginning of load\n");
    for(y = 0; y < 105; y++)
    {
        for(x = 0; x < 160; x++)
        {
            dungeon[y][x].hardness = hardness[x + y*160];
            if(dungeon[y][x].hardness == 255)
            {
                dungeon[y][x].tile = EDGE;
            }
            else if(dungeon[y][x].hardness > 0)
            {
                dungeon[y][x].tile = ROCK;
            }
            else
            {
                dungeon[y][x].tile = PATH3;
            }
            monsters[y][x] = new creature(0,0);
            items[y][x] = new item();
        }
    }

    if(numrms > maxRooms)
    {
       void* ret = realloc(roomCenters, numrms * sizeof(int) * 2);
       if(ret == NULL)
       {
    	   printf("Error when reallocating rooms size.\n");
       }
    }

    for(y = 0; y < numrms*4; y += 4)
    {
        rooms_[y] = rms[y];
        rooms_[y + 1] = rms[y + 1];
        rooms_[y + 2] = rms[y + 2];
        rooms_[y + 3] = rms[y + 3];


        roomCenters[(y/2)] = rooms_[y] + (rooms_[y + 2] / 2);
        roomCenters[(y/2) + 1] = rooms_[y + 1] + (rooms_[y + 3] / 2);

        int i = 0;
        int j = 0;
        for(j = rooms_[y + 1]; j < rooms_[y + 1] + rooms_[y + 3]; j++)
        {
            for(i = rooms_[y]; i < rooms_[y] + rooms_[y + 2]; i++)
            {
                dungeon[j][i].tile = ROOM;
            }
        }

        int randNum = rand();
		delete(items[(randNum % rooms_[y + 3]) + rooms_[y + 1]][(randNum % rooms_[y + 2]) + rooms_[y]]);
		items[(randNum % rooms_[y + 3]) + rooms_[y + 1]][(randNum % rooms_[y + 2]) + rooms_[y]] = new item(it[randNum % it.size()]);
//		cout << "Item placed at x: " << (randNum % rooms_[y + 2]) + rooms_[y] << " y: " << (randNum % rooms_[y + 3]) + rooms_[y + 1] << randNum % 10000 << endl;
    }

	//add stairs
	int r = rand();
	int randRoom= (r % (numOfRooms-2)) + 2; //gets a room number that is not the first 2 rooms
	int xp = (r % (rooms_[(randRoom * 4) + 2])) + rooms_[randRoom * 4]; //gets a random x value in the random room
	int yp = (r % (rooms_[(randRoom * 4) + 3])) + rooms_[(randRoom * 4) + 1]; //gets a random y value in the random room
	dungeon[yp][xp].tile = STAIRSDOWN;
}

void generateDungeon(int rw, int rl, int rc)
{
	if(!gotItems)
	{
		if(getItemTemplates(&it))
		{
			cout << "Error getting items." << endl;
		}
		else
		{
			cout << "Got item templates! Got " << it.size() << " templates." << endl;
			gotItems = true;
		}
	}


	setRoomWidth(rw);
	setRoomLength(rl);
	setRoomChance(rc);
	int x;
	int y;
	//fill the dungeon with empty blocks
	for(y = 0; y < rows; y++)
	{
		for(x = 0; x < cols; x++)
		{
			if(x == 0 || y == 0 || x == cols-1 || y == rows-1)
			{
				dungeon[y][x].tile = EDGE;
				dungeon[y][x].hardness = 255;
			}
			else
			{
				dungeon[y][x].tile = EMPTY;
			}
			dungeon[y][x].visible = 0;
			if(floor_Num == 1)
			{
				monsters[y][x] = new creature(0,0); //essentially a null monster
			}
			items[y][x] = new item();
		}
	}

	//generate and count the rooms
	for(y = 1; y < rows-1; y++)
	{
		for(x = 1; x < cols-1; x++)
		{
			if(numOfRooms >= 25)
			{
				break;
			}

			int randNum = rand() % 10000;
			int* room = generateRoom();
			if(randNum < roomChance)
			{
				int placeable = canPlace(x, y, room);
				//Make the room and store its center.
				if(placeable)
				{
					int rx = 0;
					int ry = 0;
					for(ry = 0; ry < room[1]; ry++)
					{
						for(rx = 0; rx < room[0]; rx++)
						{
							dungeon[y+ry][x+rx].tile = ROOM;
							dungeon[y+ry][x+rx].hardness = 0;
						}
					}
					roomCenters[2 * numOfRooms] = x + (room[0] / 2);
					roomCenters[(2 * numOfRooms) + 1] = y + (room[1] / 2);

					rooms_[numOfRooms*4] = (unsigned char) x;
					rooms_[numOfRooms*4 + 1] = (unsigned char) y;
					rooms_[numOfRooms*4 + 2] = (unsigned char) room[0];
					rooms_[numOfRooms*4 + 3] = (unsigned char) room[1];
					//printf("Room values: y=%d length=%d\n", rooms_[numOfRooms*4 +1], rooms_[numOfRooms*4 + 3]);
					numOfRooms++;

					//place an item
					if(randNum < 100)
					{
						delete(items[(randNum % room[1]) + y][(randNum % room[0]) + x]);
						items[(randNum % room[1]) + y][(randNum % room[0]) + x] = new item(it[randNum % it.size()]);
					}
				}
			}
		}
	}
	//fill in the empty space
	for(y = 1; y < rows-1; y++)
	{
		for(x = 1; x < cols-1; x++)
		{
			if(dungeon[y][x].tile != ROOM)
			{
				int randNum = (rand() % 250) + 1;
				dungeon[y][x].tile = ROCK;
				dungeon[y][x].hardness = randNum;
			}
		}
	}
	makePath(numOfRooms, roomCenters);

	//add down stairs
	int r = rand();
	int randRoom= (r % (numOfRooms - 2)) + 2; //gets a room number that is not the first 2 rooms
	int xp = (r % (rooms_[(randRoom * 4) + 2])) + rooms_[randRoom * 4]; //gets a random x value in the random room
	int yp = (r % (rooms_[(randRoom * 4) + 3])) + rooms_[(randRoom * 4) + 1]; //gets a random y value in the random room
	dungeon[yp][xp].tile = STAIRSDOWN;

}

//returns an array of random width and length for a room
int* generateRoom()
{
	static int dimensions[2];
	int rw = rand() % abs(roomWidth) + 7; //Provides a random room width with a minimum of 7 cells wide
	int rl = rand() % abs(roomLength) + 5; //Provides a random room length with a minimum of 5 cells long
	dimensions[0] = rw;
	dimensions[1] = rl;
	return dimensions;
}

int canPlace(int x, int y, int* r)
{
	int z;
	int w;
	int canPlace = 1;
	//since x and y start at 1, using -1 for z and w is fine
	//the point is to guarantee at least one space between rooms
	for(z = -1; z <= r[1]; z++)
	{
		for(w = -1; w <= r[0]; w++)
		{
			//look to see if the block is an edge of the board or already a room tile
		    if(dungeon[y+z][x+w].tile == ROOM || x == 0 || y == 0 || (x+r[0]) >= (cols-1) || (y+r[1]) >= (rows-1))
			{
				canPlace = 0;
				return canPlace;
			}
		}
	}
	return canPlace;
}

//finds a path and changes the tiles of the path
void makePath(int numOfRooms, int* room)
{
	int path[rows*cols];

	int i;
	for(i = 0; i < (2 * numOfRooms); i += 2)
	{
		int z;
		for(z = 0; z < rows*cols; z++)
		{
			path[z] = 9;
		}
		findPath(i, numOfRooms, room, path);
		int currX = room[i];
		int currY = room[i + 1];
		int changes[8] = {currX - 1, currY, currX, currY + 1, currX + 1, currY, currX, currY - 1};

		int j;
		for(j = 1; j < path[0]-1; j++)
		{
			currX = changes[path[j]];
			currY = changes[path[j]+1];
			cell_t* nextCell = &dungeon[changes[path[j] + 1]][changes[path[j]]];

			if(nextCell->tile != ROOM && nextCell->tile != PATH3)
			{
				nextCell->tile = PATH3;
				nextCell->hardness = 0;
			}

			//This was a neat idea, but ultimately unneccessary
//			if(nextCell->tile != ROOM && nextCell->tile != PATH1 && nextCell->tile != PATH2)
//			{
//				if(path[j] == 0 || path[j] == 4)
//				{
//					nextCell->tile = PATH1;
//				}
//				else
//				{
//					nextCell->tile = PATH2;
//				}
//				nextCell->hardness = 0;
//			}

			updateChanges(currX, currY, changes);
		}
	}
}

void updateChanges(int x, int y, int* arr)
{
	arr[0] = x - 1;
	arr[1] = y;
	arr[2] = x;
	arr[3] = y + 1;
	arr[4] = x + 1;
	arr[5] = y;
	arr[6] = x;
	arr[7] = y - 1;
}

//finds the path from the given room, to the next room, or to the
//first room if it is the last room
void findPath(int roomNum, int numOfRooms, int* room, int* path)
{
	int tentWeight = 0;
	int currX;
	int destX;
	int destY;
	int currY;
	int moveCounter = 1;
	//set all cells visited to 0
	int x;
	int y;
	for(y = 0; y < rows; y++)
	{
		for(x = 0; x < cols; x++)
		{
			dungeon[y][x].visited = 0; //This is changing the value of room[32] for some reason
		}
	}
	//set destination to the next room
	//if you are in the last room, destination is the original room
	cell_t* destination;
	if(roomNum/2 == numOfRooms-1)
	{
		destination = &dungeon[room[1]][room[0]];
		destX = room[0];
		destY = room[1];
	}
	else
	{
		destination = &dungeon[room[roomNum + 3]][room[roomNum + 2]];
		destX = room[roomNum + 2];
		destY = room[roomNum + 3];
	}

	cell_t* currCell = &dungeon[room[roomNum + 1]][room[roomNum]];
	currCell->visited = 1; //starting node
	currX = room[roomNum];
	currY = room[roomNum + 1];
	int currMovements[1 + cols + rows];
	moveCounter = 1;
	currMovements[0] = 1;
	//continue until the destination is reached
	while(destination->visited == 0)
	{
		int offsets[8] = {currX - 1, currY, currX, currY + 1, currX + 1, currY, currX, currY - 1}; //left, below, right, above, the current cell
		int j = 0;
		int tempWeight = 1000;
		for(j = 0; j < 8; j += 2) //for every possible path
		{
			if((dungeon[offsets[j + 1]][offsets[j]].visited == 0) && dungeon[offsets[j + 1]][offsets[j]].tile != EDGE)
			{
			    if((j==0 && offsets[j] >= destX) || (j==2 && offsets[j+1] <= destY) || (j==4 && offsets[j] <= destX) || (j==6 && offsets[j+1] >= destY))
				{
					tentWeight = dungeon[offsets[j + 1]][offsets[j]].hardness;
					if(tentWeight < tempWeight)
					{
						tempWeight = tentWeight;
                        if(dungeon[currY][currX].hardness == 0)
                        {
                            currMovements[moveCounter] = j;
                            currX = offsets[j];
                            currY = offsets[j+1];
                            break;
                        }
                        currMovements[moveCounter] = j;
                        currX = offsets[j];
                        currY = offsets[j+1];
					}
				}
			}
		}
		//movement decision made, update relavent
        currMovements[0]++;
        currCell = &dungeon[currY][currX];
        moveCounter++;
		currCell->visited = 1; //done at the end so that the destination cell will be updated if reached before the next iteration
	}
	int l;
	int m = currMovements[0];
	for(l = 0; l <= m; l++)
	{
		path[l] = currMovements[l];
		currMovements[l] = 0;
	}
}


void setMonster(creature* mon)
{
	monsters[mon->posy][mon->posx] = mon;
}

//0 means the monster is alive
int isMonsterDead(int x, int y)
{
	return monsters[y][x]->isDead;
}

//nullifies a monster
void nullMonster(int x, int y)
{
	if(monsters[y][x]->isDead)
	{
		delete(monsters[y][x]);
		//free(monsters[y][x]);
	}

	monsters[y][x] = new creature(0,0);
}

void releaseMonster(int x, int y)
{
	delete(monsters[y][x]);
}

creature* getMonster(int x, int y)
{
	return monsters[y][x];
}

//sets the item at position x, y
void setItem(item* item, int x, int y)
{
	delete(items[y][x]); //hard replace of the item
	items[y][x] = item;
}

//nullifies a monster
void nullItem(int x, int y)
{
	delete(items[y][x]);

	items[y][x] = new item();
}

item* getItem(int x, int y)
{
	return items[y][x];
}

item*** getItemMap()
{
	item*** iptr = (item***)malloc(rows * sizeof(item**));

	int i;
	for(i = 0; i < rows; i++)
	{
		iptr[i] = items[i];
	}

	return iptr;
}

//thiiiiiiiis needs a better solution
char getItemShape(item* i)
{
	string s = i->type[0];

	if(!s.compare("WEAPON"))
	{
		return WEAPON;
	}
	if(!s.compare("OFFHAND"))
	{
		return OFFHAND;
	}
	if(!s.compare("RANGED"))
	{
		return RANGED;
	}
	if(!s.compare("ARMOR"))
	{
		return ARMOR;
	}
	if(!s.compare("HELMET"))
	{
		return HELMET;
	}
	if(!s.compare("CLOAK"))
	{
		return CLOAK;
	}
	if(!s.compare("GLOVES"))
	{
		return GLOVES;
	}
	if(!s.compare("BOOTS"))
	{
		return BOOTS;
	}
	if(!s.compare("RING"))
	{
		return RING;
	}
	if(!s.compare("AMULET"))
	{
		return AMULET;
	}
	if(!s.compare("LIGHT"))
	{
		return LIGHT;
	}
	if(!s.compare("SCROLL"))
	{
		return SCROLL;
	}
	if(!s.compare("BOOK"))
	{
		return BOOK;
	}
	if(!s.compare("FLASK"))
	{
		return FLASK;
	}
	if(!s.compare("GOLD"))
	{
		return GOLD;
	}
	if(!s.compare("AMMUNITION"))
	{
		return AMMUNITION;
	}
	if(!s.compare("FOOD"))
	{
		return FOOD;
	}
	if(!s.compare("WAND"))
	{
		return WAND;
	}
	if(!s.compare("CONTAINER"))
	{
		return CONTAINER;
	}

	return '\0'; //should be unreachable, but just in case
}

int getColor_creature(creature* c)
{
	int ret = 0;
	string col = c->color[0];
	if(!col.compare("RED"))
	{
		ret = COLOR_RED;
	}
	else if(!col.compare("YELLOW"))
	{
		ret = COLOR_YELLOW;
	}
	else if(!col.compare("GREEN"))
	{
		ret = COLOR_GREEN;
	}
	else if(!col.compare("MAGENTA"))
	{
		ret = COLOR_MAGENTA;
	}
	else if(!col.compare("CYAN"))
	{
		ret = COLOR_CYAN;
	}
	else if(!col.compare("WHITE"))
	{
		ret = COLOR_WHITE;
	}
	else if(!col.compare("BLUE"))
	{
		ret = COLOR_BLUE;
	}
	return ret;
}

int getColor_item(item* i)
{
	int ret = 0;
	string col = i->color;
	if(!col.compare("RED"))
	{
		ret = COLOR_RED;
	}
	else if(!col.compare("YELLOW"))
	{
		ret = COLOR_YELLOW;
	}
	else if(!col.compare("GREEN"))
	{
		ret = COLOR_GREEN;
	}
	else if(!col.compare("MAGENTA"))
	{
		ret = COLOR_MAGENTA;
	}
	else if(!col.compare("CYAN"))
	{
		ret = COLOR_CYAN;
	}
	else if(!col.compare("WHITE"))
	{
		ret = COLOR_WHITE;
	}
	else if(!col.compare("BLUE"))
	{
		ret = COLOR_BLUE;
	}
	return ret;
}

