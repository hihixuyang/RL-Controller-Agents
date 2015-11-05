#ifndef __BULLET_HEADER_
#define __BULLET_HEADER_


#include <stdlib.h>
#include "Boolean.h"

typedef struct{
    int newEntry;
    int processed;

    int age;

    int teamOwner;
    int agentOwner;

    int blockRow;
    int blockCol;
    int row;
    int col;
    int rowD;
    int colD;
} Bullet;

Boolean compareBullets(void *, void *);

void * destroyBullet(void *);

void setMinBulletValues(Bullet *, int, int, int, int);


#endif
