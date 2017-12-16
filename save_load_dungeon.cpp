#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "dungeon.h"
#include "save_load_dungeon.h"

#define PATH getenv("HOME")
//#define PATH ".." //For working on windows
int makeDirectory();
//int betole(int);

//creates a subdirectory at the location of the executable
int makeDirectory()
{
  //printf("Making directory.\n");
  char* rlgDest = (char*)malloc(sizeof(PATH) + sizeof("/.rlg327"));
  strcpy(rlgDest, PATH);
  strcat(rlgDest, "/.rlg327/");

  if(mkdir(rlgDest, 0777))
  //if(mkdir(rlgDest)) //for windows
  {
      // fprintf(stderr, "Couldn't create directory: %s\n", rlgDest);
    free(rlgDest);
    return 1;
  }

  // printf("Successfully created new directory at %s.\n", rlgDest);
  free(rlgDest);
  return 0;
}

//stores the current dungeon
void save(char* sw)
{
    FILE* f;
    int ver = 0;
    int fileSize = 0;
    unsigned char* rooms;
    unsigned char* hardness;

    if(!makeDirectory()){
        fprintf(stderr, "Error making directory.\n");
    }

    char* fileLocation = (char *)malloc(sizeof(PATH) + sizeof("/.rlg327/dungeon"));
    strcpy(fileLocation, PATH);
    strcat(fileLocation, "/.rlg327/dungeon");

    //create the dungeon file if it doesn't exist or overwrite the current dungeon
    if(!(f = fopen(fileLocation, sw)))
    {
      fprintf(stderr, "Error opening file for save.");
    }

    hardness = (unsigned char*)malloc(16800);
    rooms = (unsigned char*)malloc(200);

    getHardness(hardness);

    getRooms(rooms);

    fileSize = 16820 + getNumOfRooms()*4;
    fwrite("RLG327-S2017", 1, 12, f); //Write the file type
    ver = htobe32(ver);
    //ver = betole(ver);
    fwrite(&ver, 4, 1, f); //Write the version (highly simplified right now)

    printf("File size: %d\n", fileSize);
    fileSize = htobe32(fileSize);
    //fileSize = betole(fileSize);
    fwrite(&fileSize, 4, 1, f);

    fwrite(hardness, 1, 16800, f);
    fwrite(rooms, 1, getNumOfRooms()*4, f);

    free(rooms);
    free(hardness);
    fclose(f);
    printf("Save complete!\n\n");
}

//reads a dungeon and displays it
void load(char* sw)
{
  FILE* f;
  char* fileLocation = (char*)malloc(sizeof(PATH) + sizeof("/.rlg327/dungeon"));
  strcpy(fileLocation, PATH);
  strcat(fileLocation, "/.rlg327/dungeon");
  if(!(f = fopen(fileLocation, sw)))
  {
      fprintf(stderr, "Error opening file.");
  }

  char* ftb = (char*)malloc(13);
  ftb[12] = '\0';
  fread(ftb, 1, 12, f); //WORKING
  //printf("\n\nftb: %s\n\n", ftb);

  int* ver = (int*)malloc(4);
  fread(ver, 4, 1, f); //WORKING
  *ver = be32toh(*ver);
  //*ver = betole(*ver);
  //printf("\n\nVer: %d\n\n", *ver);

  int* fileSize = (int*)malloc(4);
  fread(fileSize, 4, 1, f);
  *fileSize = be32toh(*fileSize);
  //*fileSize = betole(*fileSize);
  printf("\n\nFile size: %d\n\n", *fileSize);

  unsigned char* cellHardness = (unsigned char*)malloc(16800);
  fread(cellHardness, 1, 16800, f);


  unsigned char* rooms = (unsigned char*)malloc(*fileSize - 16820); //fileSize MUST be in proper host format, not big endian
  int remainder = *fileSize - 16820;
  //printf("\n\nRemaining bytes: %d\n\n", remainder);
  fread(rooms, 1, remainder, f);

  int num = (*fileSize - 16820) / 4;
  //printf("\nNumofRooms: %d\n", num);
  loadDungeon(cellHardness, rooms, num);

  free(fileLocation);
  free(ftb);
  free(fileSize);
  free(ver);
  free(cellHardness);
  fclose(f);
}

//Not entirely sure these are correct.
//int betole(int num)
//{
//    return ((num >> 24) & 0xff) | ((num << 8) & 0xff0000) | ((num >> 8) & 0xff00) | ((num << 24 & 0xff000000));
//}


