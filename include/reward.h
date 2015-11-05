#ifndef __REWARD_HEAD_
#define __REWARD_HEAD_

#include "env.h"

void setMovementReward(Env *, Team, Identity, int, int);

void setDeathReward(Env *, Team, Identity);

void setKillReward(Env *, Team, Identity, int, int, int, int, int *);

void setKillReward2(Env *, Team, Identity, int, int);

void setFinishReward(Env *, Team, Identity);

#endif
