C/C++ Rogue Like Dungeon Crawler
made by Sean Jellison

This is a terminal based game made for my Computer Science 327: Advanced Programming Techniques course.
Beaware that this creates a hidden folder to save files to, including dungeon information.

---***Installation***---
The game was made for linux based environments and was developed in Ubuntu 16.04.
A C/C++ compiler is needed. GCC is highly recomended as it was built and tested
with it, and it comes with Ubuntu Linux.

This project uses ncurses for some of its graphical elements. Not all linux distributions
come with ncurses. Ubuntu users can use sudo apt-get ncurses. For other operating systems,
you will have to download the ncurses library and put them into the default C++ headers 
location.
Ncurses is a very commonly used library so a quick google search should give you the
instructions you need.

1. Download/Clone the git repository to the desired installation destination.

2. Open a terminal and navigate to folder the files were downloaded to.

3. Type make. This will compile all the necessary files and create the game's
   executable, called game.

4. Type ./game into the terminal. You should see a mostly blank terminal with an @ symbol
   representing the player's character.

---***Controls***---
WARNING! Some of these controls are wierd. These are because of the requirements given to us.

UP - 8 or k
UP/Right - 9 or u
UP/Left - 7 or y
DOWN - 2 or j
DOWN/Right - 3 or n
DOWN/Left - 1 oor b
Use Stairs - > (can only go down)
LEFT - 4 or h
RIGHT - 6 or l
Enter LOOK mode - L
Exit LOOK mode - Q or esc
Pickup Item/Skip Turn - 5 or SPACE
Open Inventory - i
Close Inventory - q

