/*
 * player.h
 *
 *  Created on: Feb 4, 2017
 *      Author: Sean Jellison
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include "creature.h"

#ifdef __cplusplus
extern "C" {
#endif

creature* initialize_player(int, int);
creature* getPlayer();
int isNotPlayer(creature*);
//int isInLOS(int, int);

int move_player(int);
void destroyPlayer();

#ifdef __cplusplus
}
#endif

#endif /* PLAYER_H_ */
