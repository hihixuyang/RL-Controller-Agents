#include "bullet.h"

Boolean compareBullets(void * b1, void * b2)
{
    if ( ((Bullet *)b1)->blockRow == ((Bullet *)b2)->blockRow &&
         ((Bullet *)b1)->blockCol == ((Bullet *)b2)->blockCol &&
         ((Bullet *)b1)->row      == ((Bullet *)b2)->row      &&
         ((Bullet *)b1)->col      == ((Bullet *)b2)->col       )
        return true;
    else
        return false;
}

void * destroyBullet(void * data)
{
    Bullet * bullet = (Bullet *)data;

    free(bullet);

    return NULL;
}

void setMinBulletValues(Bullet * bullet, int blockRow, int blockCol, int row, int col)
{
    bullet->blockRow = blockRow;
    bullet->blockCol = blockCol;
    bullet->row      = row;
    bullet->col      = col;
}
