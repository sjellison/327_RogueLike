/*
 * main.c
 *
 *  Created on: Feb 4, 2017
 *      Author: Sean Jellison
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#include "player.h"
#include "save_load_dungeon.h"
#include "ai.h"
#include "dungeon.h"

char saveSwitch = 0;
char loadSwitch = 0;
//bool debug = 0;
int initialMonsters = 0;

int main(int argc, char* argv[])
{

  int seed = time(NULL);

  //get input from the command line
  //WARNING! Assumes that any input is a seed if it is not a specified switch
  int i;
  //printf("Checking for save/load switch\n");
  for(i = 1; i < argc; i++)//need to look at this, this requires correct order
  {
    //printf("i: %d\n", i);
    if(!strcmp(argv[i], "--load"))
    {
      //printf("Found load\n");
      loadSwitch = 1;
    }

    else if(!strcmp(argv[i], "--save"))
    {
      //printf("Found save\n");
      saveSwitch = 1;
    }

    else if(!strcmp(argv[i], "--nummon"))
    {
    	i++;
    	if(!(initialMonsters = atoi(argv[i])))
    	{
    		fprintf(stderr, "Error! Expected integer after --nummon, got %s", argv[i]);
    	}
    }

//    else if(!strcmp(argv[i], "--debug"))
//    {
//    	debug = 1;
//    }

    else
    {
      printf("Found seed\n");
      seed = atoi(argv[i]);
    }
  }

  printf("Current seed: %d\n\n", seed);
  srand(seed);

  //loadSwitch = 1;
  if(loadSwitch)
  {
	char* in = (char*) malloc(sizeof(char));
	*in = 'r';
	load(in);
	free(in);
    //load("r"); //get the warning that this is deprecated
    //load("rb");
  }
  else{
    //generate a new dungeon
    generateDungeon(0, 0, 0);
  }
  //saveSwitch = 1;
  if(saveSwitch){
	char* in = (char*) malloc(sizeof(char));
	*in = 'w';
    save(in);
    free(in);
    //save("w"); //this is deprecated
    //save("wb");
  }

  //initialMonsters = 10;
  //starts the event queue and populates initial monsters
  mvprintw(0, 0, "Current seed: %d", seed);
  initializeQ(initialMonsters, 1);

  //go until the player is dead
  while(!update()){}


  usleep(10000000);
  killScreen();
  printf("Ending game\n");
  return 0;

}
