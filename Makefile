default: game

game: main.cpp main.h player.o ai.o dg.o sl.o heap.o creature.o item.o cp.o ip.o dice.o ct.o it.o
	g++ -Wall -Werror -ggdb creature.o player.o dg.o sl.o heap.o item.o cp.o ip.o ai.o dice.o it.o ct.o main.cpp -o game -lncurses
	make clean

ai.o: ai.h ai.cpp
	g++ -c -Wall -Werror -ggdb ai.cpp -o ai.o
	
player.o: player.h player.cpp
	g++ -c -Wall -Werror -ggdb player.cpp -o player.o

dg.o: dungeon.h dungeon.cpp creature.h
	g++ -c -Wall -Werror -ggdb dungeon.cpp -o dg.o

sl.o: save_load_dungeon.cpp save_load_dungeon.h
	g++ -c -Wall -Werror -ggdb save_load_dungeon.cpp -o sl.o
	
heap.o: heap.cpp heap.h
	g++ -c -Wall -Werror -ggdb heap.cpp -o heap.o
	
creature.o: creature.cpp creature.h
	g++ -c -Wall -Werror -ggdb creature.cpp -o creature.o

cp.o: charParser.cpp charParser.h
	g++ -c -Wall -Werror -ggdb charParser.cpp -o cp.o

dice.o: dice.cpp dice.h
	g++ -c -Wall -Werror -ggdb dice.cpp -o dice.o
	
ct.o: creatureTemplate.cpp creatureTemplate.h
		g++ -c -Wall -Werror -ggdb creatureTemplate.cpp -o ct.o
		
it.o: itemTemplate.cpp itemTemplate.h
	g++ -c -Wall -Werror -ggdb itemTemplate.cpp -o it.o
	
ip.o: itemParser.cpp itemParser.h
	g++ -c -Wall -Werror -ggdb itemParser.cpp -o ip.o
	
item.o: item.cpp item.h
	g++ -c -Wall -Werror -ggdb item.cpp -o item.o

clean:
	rm -f *.o *~

cleanwin:
	del *.o

fullclean:
	rm -f *.o *~ game
