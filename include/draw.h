#ifndef __DRAW_HEADER_
#define __DRAW_HEADER_


#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "env.h"

#define CELL_SIZE       20

#define SCREEN_WIDTH    CELL_SIZE*BLOCK_SIZE*GRID_SIZE
#define SCREEN_HEIGHT   CELL_SIZE*BLOCK_SIZE*GRID_SIZE

#define MARGIN          1
#define BULLET_MARGIN   5

void drawMapASCII(Env *);

void startSDL();

void drawMapSDL(Env *);

void drawUnexplored(Block *, int, int, int);

void drawExplored(Env *, Block *, int, int, int);

void drawBlock(Env *, Block *, int, int, int);

void drawBackground(int, int, int, int);

void drawDark(int, int, int);

void drawEmpty(int, int, int, int);

void drawWall(int, int, int, int);

void drawGrnAgent(int, int, int);

void drawGrnAgent2(int, int, int);

void drawRedAgent(int, int, int);

void drawRedAgent2(int, int, int);

void drawBullet(int, int, int);

void drawGrnZone(Env *, int, int, int, int);

void drawRedZone(Env *, int, int, int, int);

void drawGrid();

void drawBooms(Env *);


#endif
