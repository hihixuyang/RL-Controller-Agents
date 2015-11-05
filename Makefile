FLAGS= -g -std=c99 -Wall -pedantic
LINKS= -lSDL -lGL -lpthread -lgsl -lm

all: obj/main.o obj/env.o obj/agent.o obj/controller.o obj/draw.o obj/LinkedList.o obj/bullet.o obj/reward.o obj/event.o obj/tree.o obj/neuralnet.o
	gcc $(FLAGS) -D_REENTRANT -Iinclude obj/main.o obj/env.o obj/agent.o obj/controller.o obj/draw.o obj/LinkedList.o obj/bullet.o obj/reward.o obj/event.o obj/tree.o obj/neuralnet.o $(LINKS) -o bin/run

obj/main.o: include/main.h src/main.c
	gcc -c $(FLAGS) -Iinclude src/main.c -o obj/main.o

obj/env.o: include/env.h src/env.c
	gcc -c $(FLAGS) -Iinclude src/env.c -o obj/env.o

obj/agent.o: include/agent.h src/agent.c
	gcc -c $(FLAGS) -Iinclude src/agent.c -o obj/agent.o

obj/controller.o: include/controller.h src/controller.c
	gcc -c $(FLAGS) -Iinclude src/controller.c -o obj/controller.o

obj/draw.o: include/draw.h src/draw.c
	gcc -c $(FLAGS) -Iinclude src/draw.c -lSDL -lGL -o obj/draw.o

obj/LinkedList.o: include/LinkedList.h src/LinkedList.c
	gcc -c $(FLAGS) -Iinclude src/LinkedList.c -o obj/LinkedList.o

obj/bullet.o: include/bullet.h src/bullet.c
	gcc -c $(FLAGS) -Iinclude src/bullet.c -o obj/bullet.o

obj/reward.o: include/reward.h src/reward.c
	gcc -c $(FLAGS) -Iinclude src/reward.c -o obj/reward.o

obj/event.o: include/event.h src/event.c
	gcc -c $(FLAGS) -Iinclude src/event.c -o obj/event.o

obj/tree.o: include/tree.h src/tree.c
	gcc -c $(FLAGS) -Iinclude src/tree.c -o obj/tree.o

obj/neuralnet.o: include/neuralnet.h src/neuralnet.c
	gcc -c $(FLAGS) -Iinclude src/neuralnet.c -o obj/neuralnet.o

run:
	bin/run

rund:
	gdb bin/run

clean:
	rm obj/*
	rm bin/*
