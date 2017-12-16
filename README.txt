
	Items fully implemented!
	
	Items are now fully useable. They appear randomly with generally one per room, and
	are created randomly from among the item templates created from the item_desc.txt
	file. Items can be picked up and equipped by the player and creatures with the
	proper characteristics. Some creatures will also destroy items they pass over, so
	that's not good. Items will only appear in rooms and will be dropped when a creature
	with an item dies, but the item must be within line of sight for it to appear.
	
		To pick up an item, walk over it and press 5 or 'space'. You will know you are
		over an item because an item description will be displayed near the bottom of
		the screen.
		
		A creature or player can hold up to 10 items in their bag space, plus the 12
		equipment slots. A creature or player can also equip a bag to extend the
		amount of bag space up to 20 slots.
		
		To equip an item, open the inventory screen by pressing 'i'. A new screen will
		pop up that shows the player's inventory.
		While in the inventory screen, press 'e' to activate equip mode. Press a number
		that is next to an inventory slot to equip the item. If it replaces an item
		already equipped, the items will be switched.
		
		NOTE: All items have random stats (according to their template) but
			  there is currently no way of knowing the stats until the item
			  is equipped and stats are compared. This is a work in progress.
			  
		To un-equip an item, press the letter next to the equipment slot you want to
		remove and the item will be placed into the inventory. If there is no space in
		the inventory, then the item will not be removed. Un-equipping a bag will
		attempt to place the items in the extra inventory slots into the player's
		normal inventory before un-equipping the bag.
		
		Items can be dropped by having the inventory screen open and pressing a number
		corresponding to an inventory slot.
		
	The player's stats can now be viewed at the bottom of the screen. Nice!
	
	Armor, accuracy, and dodge are all implemented. Armor reduces damage a creature
	receives from an attack by a flat amount to a minimum of 1 damage. Accuracy is the
	chance the creature will score a hit up to a maximum of 95%. Dodge is the chance for
	a creature to avoid an attack, up to 85% chance to dodge.
	
		SO! For a hit to occur, a random value is determined between 0 - 99. If the value is
		is greater than the attacker's accuracy, or below the defender's dodge chance, the
		attack misses. Most creatures have an accuracy of 75% and no dodge chance.
	
		Although a message will pop up near the bottom of the screen that announces if a
		player hits/misses or is hit/missed, creatures will sometimes get multiple turns
		and it will look like the player loses more health than it said. Unfortunately
		only the last attack attempt made is displayed. So this is really only good to show
		something is happening... it's a work in progress.
	
	Creatures can now move in a ghostly manor! Creatures with the GHOST characteristic
	can pass through rocks and will thus ignore them entirely. If a creature has GHOST
	and TUNNELER characteristics, the rock will be ignored and no mining attempt will
	be made. Spooky!
	
	To accomodate these updates, some changes to the ai have also been made. Creatures will
	not fight with one another and cannot end up on the same space, so creatures may get
	bunched up if there are a lot in a small area. Creatures cannot mine a rock if a creature
	is over it. For player vs creature, this means attacks will be made if a creature is over
	a rock and no attempt to mine the rock will be made.
	
	FOR THE TA
	Written in Eclipse on Ubuntu, tested in Xterm on Slackware
	
	I've added a good amount of stuff in this one, and it has made the game unstable. I need to do some extensive debugging
	because the game will crash seemingly randomly. As far as I can tell, this has to do with creature interactions.
	
	Some notes from previous updates: The first floor will not spawn monsters unless --nummon is used. If --nummon is used, it will
	ONLY affect the first floor. Every other dungeon floor will spawn between 3 and 20 creatures. Tunneling monsters spawn along
	the edge of the dungeon while non-tunneling monsters spawn in rooms.
	
	dungeon.cpp handles all the item spawning and several new functions were made to be able to manipulate the itemMap. These are
	all mostly trivial helper methods for adding/removing items. Every time a room is created, an item is randomly generated, which
	makes loading a dungeon spawn random items in random locations, but still within the rooms.
	
	creature.cpp was modified pretty heavily to handle items. A few new properties were added as well, including acc (accuracy),
	armor, dodge, and two item arrays, inv and equip.
	
		inv has a size of 20, but only the first 10 spaces are used unless equip[12] (the bag slot) is not empty. These are held
		items giving no benefit to the creature.
		
		equip has a size of 13, representing the 12 equipment slots plus an additional bag slot. Because of how this is implemented,
		all bags will give only 10 slots, but a bag can give additional stats like any other item.
		
		creature::addItem(item*) - adds an item to the first open inventory slot, if no open inv slots, does nothing. Specifically,
					   this copies the item and replaces the first inv array position == 0. This doesn't know anything
					   about the dungeon or itemMap
								  
		creature::isInLOS(int, int) - this was moved from player.cpp to creature.cpp. This now determines if the space is within view
					      of the creature, rather than if the space is within view of the player
									  
		creature::equipItem(int) - equips the item at inv space i. This means an item can only be equipped if it first is held by
					   the creature. This automatically checks the equipment slot the item can go into and will either
					   equip the item and remove it from the inv, replace the currently equipped item, or in the case
					   of two-handed weapons, replaces the weapon and offhand slot.
					   NOTE: I currently do not have an implementation for replacing a 2h with a previously equipped
			     		   	 weapon and offhand. One of the items would have to be manually removed first before
			            		 equipping the 2h. Rings will only equip to the Ring2 slot if the slot was empty. This
				   		 means the Ring2 slot must be manually removed to replace the item.
								   		 
		creature::unequipItem(int) - un-equips the item at the equip space i and places it into an open inv slot.
			   		     If no inv slot is available, it will not un-equip the item. In the case of a bag, it will attempt
			   		     to move items in the extended inventory over to the normal inventory, or drop the items if it cannot.
					     If after shuffling items, no open inv slots can be found, the bag will not be un-equipped, but items
		 			     may have been shuffled/dropped
									 
		creature::dropItem(int) - removes the item at inv slot i and places it into the dungeon up to 1 space away from the creature. Items
				          can be dropped into any dungeon space with hardness == 0, so mined rocks and paths, not just rooms. Items
					  are not stacked either
								  
		creature::destroyItem(item*) - simply destroys the provided item. This can be used on items on the creature or elsewhere. Pretty sure
				   	       this isn't needed, I haven't actually used it anywhere, but it's available.
									   
	player.cpp was updated as well. The move_player() function was modified to handle item pick up and inventory controls. I feel like all the
	if/else statements for handling key presses is the wrong way to do it, but I'm not sure of a better way.
	
	Finally, ai.cpp was updated to handle the new combat mechanics and creature characteristics.
	Several attack functions were made similar to the look and move functions that handles creature attacking.
	
		each attack determines if the attacker or defender is the player. If not, the function does essentially nothing. If either are the player,
		it first determines if the attack hits by checking the attacker's accuracy and the defender's dodge, then handles the damage. Armor
		reduces damage received by a flat amount to a minimum of 1.
		
		An attack attempt is made if a creature tries to move into a space occupied by any other creature. No move attempt is made if an attack is,
		and since the attack functions handle whether the fight is between the player and a creature, this essentially does nothing if a creature
		is already there. I do not handle pushing creatures around as the requirements ask.
		
		The look functions have been updated to check for creatures over rocks and for readability. The move functions have all been updated as well
		to handle the GHOST characteristic and creature item pickup and destruction. 
									 
		
	
