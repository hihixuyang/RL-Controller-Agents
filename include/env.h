#ifndef __ENV_HEADER_
#define __ENV_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LinkedList.h"
#include "bullet.h"
#include "tree.h"
#include "neuralnet.h"

#define SAVE_FREQ                   100
#define GRID_SIZE                   3
#define BLOCK_SIZE                  9
#define NUM_AGENTS                  2
#define BULLET_VELOCITY             2

#define AGENT_MEMORY                5
#define AGENT_STATE_MEMORY          15
#define AGENT_STATE_ACTION_SIZE     39
#define NUM_AGENT_ACTIONS           8
#define NUM_AGENT_GOALS             7
#define NUM_AGENT_BLOCK_TYPES       2
#define NUM_AGENT_POSITIONS         BLOCK_SIZE
#define NUM_AGENT_QUADRANT_STATES   4
#define NUM_AGENT_CELL_STATES       8

#define CTRLR_STATE_MEMORY          20
#define CTRLR_STATE_ACTION_SIZE     (GRID_SIZE*GRID_SIZE)+2
#define NUM_CTRLR_ACTIONS           NUM_AGENT_GOALS
#define NUM_CTRLR_BLOCK_STATUS      13

typedef enum {GRN, RED} Team;

typedef enum {AG1, AG2} Identity; 

typedef enum {ENGAGE, FINISH, GO_LEFT, GO_RIGHT, GO_UP, GO_DOWN, NO_ACTION} AgentGoal;

typedef enum {MOVE_LEFT,  MOVE_RIGHT, MOVE_UP,    MOVE_DOWN, 
              FIRE_LEFT,  FIRE_RIGHT, FIRE_UP,    FIRE_DOWN} AgentAction;


typedef enum {EMPTY,            SELF,           FRIENDLY,       ENEMY,
              BULLET_LEFT,      BULLET_RIGHT,   BULLET_UP,      BULLET_DOWN,
              HOME,             END,
              WALL,             GREEN_ZONE,       RED_ZONE,
              GREEN_AGENT_1,    GREEN_AGENT_2,  RED_AGENT_1,    RED_AGENT_2} CellStatus;
              /* Extra Agent State-Signal Constants: */

typedef enum {UNEXPLORED,               EXPLORED_NORMAL,        HOME_ZONE,  END_ZONE,
              NORMAL_AGENT1_NOTHING,    NORMAL_AGENT2_NOTHING,  NORMAL_AGENTS_NOTHING,
              NORMAL_AGENT1_ENEMY,      NORMAL_AGENT2_ENEMY,    NORMAL_AGENTS_ENEMY,
              NORMAL_AGENT1_ENEMIES,    NORMAL_AGENT2_ENEMIES,  NORMAL_AGENTS_ENEMIES,
              HOME_AGENT1_NOTHING,      HOME_AGENT2_NOTHING,    HOME_AGENTS_NOTHING,
              HOME_AGENT1_ENEMY,        HOME_AGENT2_ENEMY,      HOME_AGENTS_ENEMY,
              HOME_AGENT1_ENEMIES,      HOME_AGENT2_ENEMIES,    HOME_AGENTS_ENEMIES,
              END_AGENT1_NOTHING,       END_AGENT2_NOTHING,     END_AGENTS_NOTHING,
              END_AGENT1_ENEMY,         END_AGENT2_ENEMY,       END_AGENTS_ENEMY,
              END_AGENT1_ENEMIES,       END_AGENT2_ENEMIES,     END_AGENTS_ENEMIES} BlockStatus;
              

typedef enum {ACTION_REQUESTED, ACTION_UPDATED} ActionRequest;

/* Parameters of the learning algorithm: */
typedef enum {
                 /* Agent-Controller Orders Parameters */
                 TIME_PENALTY=-1,   GOAL_MET=80,    GOAL_NOT_MET=-80,

                 /* Agent-Instinct Parameters: */
                 KILL=50,           DEATH=-50,      AGENT_FINISH=100,

                 /* Controller-Reward Parameters: */
                 NONE=0,            WIN=100
             } Reward;

typedef struct{
    short row;
    short col;
} BlockLocation;

typedef struct{
    CellStatus cell [BLOCK_SIZE][BLOCK_SIZE];
    CellStatus permanentValue [BLOCK_SIZE][BLOCK_SIZE];
} Block;

typedef struct {
    int pause;

    int gameOver;

    int newEpisode;

    int graphics;

    int view;

    int teamShow;

    int showAgentStats;

    int showControllerStats;

    int showActionStats;

    int USE_NN;

    int boom    [5][5];

    int flash   [2];

    LList * bullets;

    BlockLocation zoneBlock         [2];

    int score                       [2];

    /* Agent-relevant variables */

    int agentMoved                  [2][NUM_AGENTS];

    Block block                     [GRID_SIZE][GRID_SIZE];

    BlockLocation agentBlock        [2][NUM_AGENTS];

    AgentAction agentAction         [2][NUM_AGENTS][AGENT_MEMORY+1];

    ActionRequest agentRequest      [2][NUM_AGENTS];

    Reward agentReward              [2][NUM_AGENTS][AGENT_MEMORY+1];

    int agentState                  [2][NUM_AGENTS][AGENT_STATE_MEMORY+1][AGENT_STATE_ACTION_SIZE];

    int agentEpisodeReset           [2][NUM_AGENTS];

    int agentActive                 [2][NUM_AGENTS];

    struct Tree * agentValueTree;

    /* neural network parameters */
    struct NNParams * nnParams;

    /* Controller-Agent communication variables */

    AgentGoal agentGoal             [2][NUM_AGENTS][AGENT_MEMORY+1];

    /* Controller-relevant variables */

    BlockStatus blockStatus         [2][GRID_SIZE][GRID_SIZE];

    ActionRequest controllerRequest [2];

    Reward controllerReward         [2];

    int controllerState             [2][CTRLR_STATE_MEMORY+1][CTRLR_STATE_ACTION_SIZE];

    int controllerUpdateNeeded      [2];

    int controllerEpisodeReset      [2];

    int controllerEpisodeStart;

    int controllerAgentToUpdate     [2][NUM_AGENTS];

    struct Tree * ctrlrValueTree;
} Env;

Env * initEnvironment();

int loadMapFromFile(Env *, char *);

int spawnEnvironment(Env *);

void addBullet(Env *, int, int, int, int, int, int, int, int);

void initControllerStateSignals(Env *);

void prepareControllerStateSignal(Env *, Team);

void prepareAgentStateSignals(Env *);

void setWinGame(Env *);

void clearMap(Env *);

Env * destroyEnvironment(Env *);

#endif
