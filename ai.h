/*
 * ai.h
 *
 *  Created on: Feb 4, 2017
 *      Author: Sean Jellison
 */

#ifndef AI_H_
#define AI_H_

#include "creature.h"

#ifdef __cplusplus
extern "C" {
#endif

enum MV_t
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
    UPLEFT,
    DOWNRIGHT,
    UPRIGHT,
    DOWNLEFT
};
void initializeQ(int, int);
void displayPath(char);
void callMove(int, creature*);
void attack(int, creature*);
void updatePath(int, int, int);
void clearQ();
int update();
int look(int, creature*);
creature* makeMonster(int, int);

#ifdef __cplusplus
}
#endif

#endif /* AI_H_ */
