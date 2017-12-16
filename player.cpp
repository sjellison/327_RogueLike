/*
 * player->cpp
 *
 *  Created on: Feb 4, 2017
 *      Author: Sean Jellison
 */
#include <stdlib.h>
#include <ncurses.h>
#include <string>

#include "creature.h"
#include "ai.h"
#include "dungeon.h"
#include "player.h"

static creature* player;
static string slots[] = {
		"(a)Weapon:  ", "(s)Offhand: ", "(d)Ranged:  ",
		"(f)Armor:   ", "(g)Helmet:  ", "(h)Cloak:   ",
		"(j)Gloves:  ", "(z)Boots:   ", "(x)Ring1:   ",
		"(c)Ring2:   ", "(v)Amulet:  ", "(b)Light:   ",
		"(n)Bag:     "
};

void displayInventory(int);

using namespace std;

//creates the player character and sets its initial position
creature* initialize_player(int initx, int inity)
{
	player = new creature(1, 0);
	player->armor = 0;
	player->dodge = 0;
	player->acc = 65;
	player->priority = 0;
	player->posx = initx;
	player->posy = inity;
	player->hitpoints = 100;
	player->speed = 10;
	player->damage = new dice(5, 1, 4);
	player->type = '@';
	player->nextMove = 0;
	player->characteristics = 0;
	player->isDead = 0;
	player->los = 5;
	return player;
}

creature* getPlayer()
{
	return player;
}

int isNotPlayer(creature* c)
{
	return (c->priority - player->priority); //putting c first keeps the value positive
}

int move_player(int val)
{
	move(22, 0);
	clrtoeol();

	if(val < 0)
	{
		movestart:
		val = getch();
	}
	int direction = -1;

	if(val == 'q')
	{
		//exit(EXIT_SUCCESS);
		endwin();
	}
	if(val == '4' || val == 'h')
	{
		direction = 0;
	}
	else if(val == '6' || val == 'l')
	{
		direction = 1;
	}
	else if(val == '8' || val == 'k')
	{
		direction = 2;
	}
	else if(val == '2' || val == 'j')
	{
		direction = 3;
	}
	else if(val == '7' || val == 'y')
	{
		direction = 4;
	}
	else if(val == '3' || val == 'n')
	{
		direction = 5;
	}
	else if(val == '9' || val == 'u')
	{
		direction = 6;
	}
	else if(val == '1' || val == 'b')
	{
		direction = 7;
	}
	else if(val == '5' || val == ' ')
	{
		if(getItem(player->posx, player->posy)->name.compare(""))
		{
			if(player->addItem(getItem(player->posx, player->posy)) != -1)
			{
				nullItem(player->posx, player->posy);
			}
		}
		direction = 8;
	}
	else if(val == '>' && isOnStairs((creature*)player))
	{
		newDungeon();
		return 0;
	}
	else if(val == 'Z')
	{
		endwin();
		displayPath(1);
	}
	else if(val == 'X')
	{
		endwin();
		displayPath(0);
	}
	else if(val == 'L')
	{
		char ch;
		move(0,0);
		clrtoeol();
		mvprintw(0,0,"Look Mode");
		refresh();
		do
		{
			ch = getch();

			if(ch == '4' || ch == 'h')
			{
				moveScreen(0);
			}
			else if(ch == '6' || ch == 'l')
			{
				moveScreen(1);
			}
			else if(ch == '8' || ch == 'k')
			{
				moveScreen(2);
			}
			else if(ch == '2' || ch == 'j')
			{
				moveScreen(3);
			}
			displayDungeon();
		}while(ch != 'Q' && ch != 27);

		updateScreen();
		displayDungeon();
		goto movestart;
		return 0;
	}
	else if(val == 'i')
	{
		do
		{
			displayInventory(0);
			val = getch();

			if(val == 'e')
			{
				displayInventory(1);
				val = getch();
				if(val >= '0' && val <= '9')
				{
					player->equipItem(((int)val) - '0');
				}
				else if(val == ')')
				{
					player->equipItem(10);
				}
				else if(val == '!')
				{
					player->equipItem(11);
				}
				else if(val == '@')
				{
					player->equipItem(12);
				}
				else if(val == '#')
				{
					player->equipItem(13);
				}
				else if(val == '$')
				{
					player->equipItem(14);
				}
				else if(val == '%')
				{
					player->equipItem(15);
				}
				else if(val == '^')
				{
					player->equipItem(16);
				}
				else if(val == '&')
				{
					player->equipItem(17);
				}
				else if(val == '*')
				{
					player->equipItem(18);
				}
				else if(val == '(')
				{
					player->equipItem(19);
				}
			}
			else if(val == 'a')
			{
				if(player->equip[0] != 0)
				{
					player->unequipItem(0);
				}
			}
			else if(val == 's')
			{
				if(player->equip[1] != 0)
				{
					player->unequipItem(1);
				}
			}
			else if(val == 'd')
			{
				if(player->equip[2] != 0)
				{
					player->unequipItem(2);
				}
			}
			else if(val == 'f')
			{
				if(player->equip[3] != 0)
				{
					player->unequipItem(3);
				}
			}
			else if(val == 'g')
			{
				if(player->equip[4] != 0)
				{
					player->unequipItem(4);
				}
			}
			else if(val == 'h')
			{
				if(player->equip[5] != 0)
				{
					player->unequipItem(5);
				}
			}
			else if(val == 'j')
			{
				if(player->equip[6] != 0)
				{
					player->unequipItem(6);
				}
			}
			else if(val == 'z')
			{
				if(player->equip[7] != 0)
				{
					player->unequipItem(7);
				}
			}
			else if(val == 'x')
			{
				if(player->equip[8] != 0)
				{
					player->unequipItem(8);
				}
			}
			else if(val == 'c')
			{
				if(player->equip[9] != 0)
				{
					player->unequipItem(9);
				}
			}
			else if(val == 'v')
			{
				if(player->equip[10] != 0)
				{
					player->unequipItem(10);
				}
			}
			else if(val == 'b')
			{
				if(player->equip[11] != 0)
				{
					player->unequipItem(11);
				}
			}
			else if(val == 'n')
			{
				if(player->equip[12] != 0)
				{
					player->unequipItem(12);
				}
			}
			else if(val == '0')
			{
				if(player->inv[0] != 0)
				{
					player->dropItem(0);
				}
			}
			else if(val == '1')
			{
				if(player->inv[1] != 0)
				{
					player->dropItem(1);
				}
			}
			else if(val == '2')
			{
				if(player->inv[2] != 0)
				{
					player->dropItem(2);
				}
			}
			else if(val == '3')
			{
				if(player->inv[3] != 0)
				{
					player->dropItem(3);
				}
			}
			else if(val == '4')
			{
				if(player->inv[4] != 0)
				{
					player->dropItem(4);
				}
			}
			else if(val == '5')
			{
				if(player->inv[5] != 0)
				{
					player->dropItem(5);
				}
			}
			else if(val == '6')
			{
				if(player->inv[6] != 0)
				{
					player->dropItem(6);
				}
			}
			else if(val == '7')
			{
				if(player->inv[7] != 0)
				{
					player->dropItem(7);
				}
			}
			else if(val == '8')
			{
				if(player->inv[8] != 0)
				{
					player->dropItem(8);
				}
			}
			else if(val == '9')
			{
				if(player->inv[9] != 0)
				{
					player->dropItem(9);
				}
			}
			else if(player->inv[12] != 0)
			{
				if(val == ')')
				{
					if(player->inv[10] != 0)
					{
						player->dropItem(10);
					}
				}
				else if(val == '!')
				{
					if(player->inv[11] != 0)
					{
						player->dropItem(11);
					}
				}
				else if(val == '@')
				{
					if(player->inv[12] != 0)
					{
						player->dropItem(12);
					}
				}
				else if(val == '#')
				{
					if(player->inv[13] != 0)
					{
						player->dropItem(13);
					}
				}
				else if(val == '$')
				{
					if(player->inv[14] != 0)
					{
						player->dropItem(14);
					}
				}
				else if(val == '%')
				{
					if(player->inv[15] != 0)
					{
						player->dropItem(15);
					}
				}
				else if(val == '^')
				{
					if(player->inv[16] != 0)
					{
						player->dropItem(16);
					}
				}
				else if(val == '&')
				{
					if(player->inv[17] != 0)
					{
						player->dropItem(17);
					}
				}
				else if(val == '*')
				{
					if(player->inv[18] != 0)
					{
						player->dropItem(18);
					}
				}
				else if(val == '(')
				{
					if(player->inv[19] != 0)
					{
						player->dropItem(19);
					}
				}
			}

		}while(val != 'Q' && val != 27);
		move(22, 0);
		clrtoeol();
		updateScreen();
		displayDungeon();
		goto movestart;
		return 0;
	}

	if(direction >=0 && direction <= 8)
	{
		if(direction != 8)
		{
			int prevX = player->posx;
			int prevY = player->posy;
			int l = look(direction, player);
			if(l == 1 || l == 3)
			{
				attack(direction, player);
			}
			else
			{
				callMove(direction, player);
			}

			//have to update the path every time the player moves
			if(prevX != player->posx || prevY != player->posy)
			{
				updatePath(player->posx, player->posy, getFloorNum());
			}
			updateScreen();
		}

		displayDungeon();
		return 0;
	}
	else
	{
		//this will make it wait on the player's turn until a movement key is pressed
		goto movestart;
		//return move_player(getch());
	}
}

void displayInventory(int i)
{
	int j;

	for(j = 0; j < 24; j++)
	{
		move(j, 0);
		clrtoeol();
		if(j == 1)
		{
			mvprintw(j, 20, "INVENTORY");
		}
		if(j == 3)
		{
			mvprintw(j, 5, "Equipped Items");
			mvprintw(j, 35, "Bag");
			if(player->equip[12] != 0)
			{
				mvprintw(j, 50, "Extended Bag");
			}
		}
		if(j == 19)
		{
			mvprintw(j, 4, "Press the letter next to an item slot to un-equip the item");
		}
		if(j == 20 && i != 1)
		{
			mvprintw(j, 4, "Press the number next to a bag slot to drop the item");
		}
		if(j == 21)
		{
			mvprintw(j, 4, "Press 'Q' or esc to close inventory screen");
		}
		if(j == 22)
		{
			if(i == 1)
			{
				mvprintw(j, 4, "Press a number, 0-9, to equip that item.");
			}
			else
			{
				mvprintw(j, 4, "Press e to enter equip mode. Press a corresponding number to equip the item.");
			}
		}
		if(j == 23)
		{
			if(player->equip[12] != 0)
			{
				mvprintw(j, 4, "To equip items from the extended bag, hold shift and press a number.");
			}
		}
		if(j > 4 && j < 18)
		{
			refresh();
			if(player->equip[j - 5] != 0)
			{
				mvprintw(j, 2, "%s%s", slots[j - 5].c_str(), player->equip[j - 5]->name.c_str());
			}
			else
			{
				mvprintw(j, 2, "%sNONE", slots[j - 5].c_str());
			}
			if(j < 15)
			{
				if(player->inv[j - 5] != 0)
				{
					mvprintw(j, 35, "%d: %s", j - 5, player->inv[j - 5]->name.c_str());
				}
				else
				{
					mvprintw(j, 35, "%d: NONE", j - 5);
				}
				if(player->equip[12] != 0)
				{
					if(player->inv[j + 5] != 0)
					{
						mvprintw(j, 60, "%d: %s", j - 5, player->inv[j + 5]->name.c_str());
					}
					else
					{
						mvprintw(j, 60, "%d: NONE", j - 5);
					}
				}
			}
		}
	}
}
