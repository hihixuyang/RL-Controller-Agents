#ifndef __AGENT_HEADER_
#define __AGENT_HEADER_


#include "env.h"
#include "draw.h"
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define EPSILON_A   8

sem_t agentSem;

typedef struct {
    Team team;
    Identity id;
    Env * env;
} AgentStruct;

void * agentFunction(void *);

void copyState(int *, int *, int);

int statesEqual(int *, int *);

int positionChange(int *, int *);

void rollAgentStates(Env *, Team, Identity);

void printActionValue(AgentAction, float);

void printAgentStates(Env *, Team, Identity);

void printAgentStats(Env *, Team, Identity, int *);

void updateStates(Env *, Team, Identity, int *);

AgentAction chooseAction(Env *, Team, Identity);

#endif
