/*
 * dice.h
 *
 *  Created on: Mar 29, 2017
 *      Author: sjelli
 */

#ifndef DICE_H_
#define DICE_H_

class dice{
	public:
		unsigned int base;
		unsigned int numDice;
		unsigned int sidesPerDie;

		unsigned int roll();

		dice();
		dice(unsigned int, unsigned int, unsigned int);
		~dice();
};



#endif /* DICE_H_ */
