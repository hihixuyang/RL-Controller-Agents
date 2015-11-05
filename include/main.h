#ifndef __MAIN_HEADER_
#define __MAIN_HEADER_


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "env.h"
#include "draw.h"
#include "agent.h"
#include "controller.h"
#include "reward.h"
#include "event.h"

#define DELAY_LEN   10
#define DELAY_STEP  10
#define TRIALS      9999999
#define BOOM_LEN    10
#define FLASH_LEN   100

void runLoop(Env *);

int updateWorld(Env *, int);

int moveAgent(Env *, Team, Identity, int, int);

int canMoveToNextBlock(Env *, Team, Identity, CellStatus, int, int, int, int);

void moveToNextBlock(Env *, Team, Identity, CellStatus, int, int, int, int);

void respawnAgent(Env *, Team, Identity);

void moveBullet(Env *, Bullet *);

void removeBullet(Env *, int, int, int, int);

void setBoom(Env *, int, int, int, int);

void updateBlockStatus(Env *, Team, int, int, int, int, int, int);

void updateBlock(Env *, Team, BlockStatus *, int, int);

void updateBlockStatusOld(Env *, Team, Identity, int, int, int, int);


#endif
