#ifndef __CONTROLLER_HEADER_
#define __CONTROLLER_HEADER_


#include "env.h"
#include "tree.h"
#include "agent.h"
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define EPSILON_B   8

sem_t ctrlrSem;

typedef struct {
    Team team;
    Env * env;
} ControllerStruct;

void * controllerFunction(void *);

int statesEqualC(int *, int *);

void getAgentsToUpdate(Env *, Team team, int *);

void updateControllerStates(Env *, Team, int *);

void rollControllerStates(Env *, Team);

void chooseControllerAction(Env *, Team);

int isEligibleAction(Env *, Team, Identity, AgentGoal);

void printControllerAction(Env *);

void printControllerState(Env *);

int hasEnemy(BlockStatus);

#endif
