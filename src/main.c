#include "main.h"


void runLoop(Env * env)
{
    pthread_t agents                    [2][NUM_AGENTS];
    AgentStruct agentStructs            [2][NUM_AGENTS];
    pthread_t controllers               [2];
    ControllerStruct controllerStructs  [2];
    ListNode * node;
    int trial   = 0;
    int trials  = TRIALS;
    int delayLen = DELAY_LEN;
    int controllerDelay = 0;
    int quit = 0;
    int i, j;
    int hold;

    if (sem_init(&ctrlrSem, 0, 1) != 0){
        fprintf(stderr, "Controller semaphore initialization failed\n");
        exit(1);
    }
    if (sem_init(&agentSem, 0, 1) != 0){
        fprintf(stderr, "Agent semaphore initialization failed\n");
        exit(1);
    }

    for (i=0; i<2; i++){
        controllerStructs[i].team   = i;
        controllerStructs[i].env    = env;
        env->controllerRequest[i] = ACTION_UPDATED;
        for (j=0; j<NUM_AGENTS; j++){
            agentStructs[i][j].team     = i;
            agentStructs[i][j].id       = j;
            agentStructs[i][j].env      = env;
            env->agentActive[i][j] = 1;
            env->agentRequest[i][j] = ACTION_UPDATED;
        }
    }

    pthread_create(&controllers[GRN], NULL, controllerFunction, &controllerStructs[GRN]);
    pthread_create(&controllers[RED], NULL, controllerFunction, &controllerStructs[RED]);

    for (i=0; i<NUM_AGENTS; i++){
        pthread_create(&agents[GRN][i], NULL, agentFunction, &agentStructs[GRN][i]);
        pthread_create(&agents[RED][i], NULL, agentFunction, &agentStructs[RED][i]);
    } 

    while(!env->gameOver){
        env->controllerReward[GRN] = NONE;
        env->controllerReward[RED] = NONE;
        for (i=0; i<NUM_AGENTS; i++){
            env->agentReward[GRN][i][0] = NONE;
            env->agentReward[RED][i][0] = NONE;
        }

        initControllerStateSignals(env);
        env->controllerEpisodeStart = 1;
        env->controllerUpdateNeeded[GRN] = env->controllerUpdateNeeded[RED] = 1;
        env->newEpisode = 0;

        while (!env->newEpisode){
            hold = 0;

            if (env->controllerUpdateNeeded[GRN]){
                prepareControllerStateSignal(env, GRN);
                env->controllerRequest[GRN] = ACTION_REQUESTED;
                env->agentEpisodeReset[GRN][AG1] = 1;
                env->agentEpisodeReset[GRN][AG2] = 1;
                hold = 1;
            }
            if (env->controllerUpdateNeeded[RED]){
                prepareControllerStateSignal(env, RED);
                env->controllerRequest[RED] = ACTION_REQUESTED;
                env->agentEpisodeReset[RED][AG1] = 1;
                env->agentEpisodeReset[RED][AG2] = 1;
            }

            while ((env->controllerUpdateNeeded[GRN] && env->controllerRequest[GRN] != ACTION_UPDATED) ||
                   (env->controllerUpdateNeeded[RED] && env->controllerRequest[RED] != ACTION_UPDATED));

            env->controllerEpisodeStart = 0;

            if (hold && controllerDelay)
                SDL_Delay(controllerDelay);

            env->controllerUpdateNeeded[GRN] = env->controllerUpdateNeeded[RED] = 0;

            env->controllerReward[GRN] = NONE;
            env->controllerReward[RED] = NONE;

            while (!(env->controllerUpdateNeeded[GRN] || env->controllerUpdateNeeded[RED])){
                prepareAgentStateSignals(env);

                // DEBUG
                /*if (env->pause){
                    SDL_Delay(2000);
                    env->pause = 0;
                }*/

                for (i=0; i<NUM_AGENTS; i++){
                    env->agentRequest[GRN][i] = ACTION_REQUESTED;
                    env->agentRequest[RED][i] = ACTION_REQUESTED;
                }

                while ( (env->agentRequest[GRN][AG1] != ACTION_UPDATED && env->agentActive[GRN][AG1] == 1) ||
                        (env->agentRequest[GRN][AG2] != ACTION_UPDATED && env->agentActive[GRN][AG2] == 1) ||
                        (env->agentRequest[RED][AG1] != ACTION_UPDATED && env->agentActive[RED][AG1] == 1) ||
                        (env->agentRequest[RED][AG2] != ACTION_UPDATED && env->agentActive[RED][AG2] == 1) );

                pollEvent(&delayLen, &controllerDelay, &quit, &env->showAgentStats, &env->showControllerStats, &env->showActionStats, &env->graphics, &env->view, &env->teamShow);

                if (delayLen)
                    SDL_Delay(delayLen);

                while ((node = getNext(env->bullets)) != NULL)
                    ((Bullet *)getNodeValue(node))->age++;
                resetCurrent(env->bullets);

                for (i=0; i<NUM_AGENTS; i++){
                    env->agentReward[GRN][i][0] = NONE;
                    env->agentReward[RED][i][0] = NONE;
                }

                for (i=0; i<BULLET_VELOCITY+1; i++){
                    if (!(updateWorld(env, i))){
                        env->newEpisode = 1;
                        env->controllerUpdateNeeded[GRN] = 1;
                        env->controllerUpdateNeeded[RED] = 1;
                        break;
                    }
                    if (env->graphics && !env->newEpisode)
                        drawMapSDL(env);
                }

                if (quit != 0 && env->newEpisode)
                    break;

                if (++trial > trials || quit == 1){
                    env->newEpisode = 1;
                    env->gameOver = 1;
                    break;
                }
            }
        }
    }

    for (i=0; i<NUM_AGENTS; i++){
        pthread_join(agents[GRN][i], NULL);
        pthread_join(agents[RED][i], NULL);
    }
    pthread_join(controllers[GRN], NULL);
    pthread_join(controllers[RED], NULL);

    sem_destroy(&agentSem);
    sem_destroy(&ctrlrSem);
}

int updateWorld(Env * env, int iter)
{
    ListNode * node;
    Bullet * bullet;
    int i, j;

    /* process agent moves */

    if (iter == 0){
        for (i=0; i<NUM_AGENTS; i++){
            env->agentMoved[GRN][i] = 0;
            env->agentMoved[RED][i] = 0;
        }

        iter = 0;
        for (i=0; i<BLOCK_SIZE; i++){
            for (j=0; j<BLOCK_SIZE; j++){
                if (env->agentActive[GRN][AG1]){
                    if (!env->agentMoved[GRN][AG1] && env->block[env->agentBlock[GRN][AG1].row][env->agentBlock[GRN][AG1].col].cell[i][j] == GREEN_AGENT_1){
                        if (!(env->agentMoved[GRN][AG1] = moveAgent(env, GRN, AG1, i, j)))
                            return 0;
                    }
                }
                if (env->agentActive[GRN][AG2]){
                    if (!env->agentMoved[GRN][AG2] && env->block[env->agentBlock[GRN][AG2].row][env->agentBlock[GRN][AG2].col].cell[i][j] == GREEN_AGENT_2){
                        if (!(env->agentMoved[GRN][AG2] = moveAgent(env, GRN, AG2, i, j)))
                            return 0;
                    }
                }
                if (env->agentActive[RED][AG1]){
                    if (!env->agentMoved[RED][AG1] && env->block[env->agentBlock[RED][AG1].row][env->agentBlock[RED][AG1].col].cell[i][j] == RED_AGENT_1){
                        if (!(env->agentMoved[RED][AG1] = moveAgent(env, RED, AG1, i, j)))
                            return 0;
                    }
                }
                if (env->agentActive[RED][AG2]){
                    if (!env->agentMoved[RED][AG2] && env->block[env->agentBlock[RED][AG2].row][env->agentBlock[RED][AG2].col].cell[i][j] == RED_AGENT_2){
                        if (!(env->agentMoved[RED][AG2] = moveAgent(env, RED, AG2, i, j)))
                            return 0;
                    }
                }
            }
        }
    }

    /* process bullets */

    if (iter != 0){
        while ((node = getNext(env->bullets)) != NULL){
            bullet = getNodeValue(node);
            if (bullet->newEntry == 1)
                bullet->newEntry = 0;
            else
                bullet->processed = 0;
        }
        resetCurrent(env->bullets);

        while ((node = getNext(env->bullets)) != NULL){
            bullet = getNodeValue(node);
            if (bullet->processed == 0)
                moveBullet(env, bullet);
        }
        resetCurrent(env->bullets);
    }

    return 1;
}

int moveAgent(Env * env, Team team, Identity id, int i, int j)
{
    CellStatus self;
    CellStatus friendlyAgent;
    CellStatus enemy1           = team == GRN ? RED_AGENT_1 : GREEN_AGENT_1;
    CellStatus enemy2           = team == GRN ? RED_AGENT_2 : GREEN_AGENT_2;
    CellStatus homeZone         = team == GRN ? GREEN_ZONE  : RED_ZONE;
    CellStatus endZone          = team == GRN ? RED_ZONE    : GREEN_ZONE;
    CellStatus nextCellState;
    int friendlyFire = 0;
    int curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol;
    int rowD = 0, colD = 0;

    if (team == GRN && id == AG1){
        self            = GREEN_AGENT_1;
        friendlyAgent   = GREEN_AGENT_2;
    }
    else if (team == GRN && id == AG2){
        self            = GREEN_AGENT_2;
        friendlyAgent   = GREEN_AGENT_1;
    }
    else if (team == RED && id == AG1){
        self            = RED_AGENT_1;
        friendlyAgent   = RED_AGENT_2;
    }
    else if (team == RED && id == AG2){
        self            = RED_AGENT_2;
        friendlyAgent   = RED_AGENT_1;
    }

    switch(env->agentAction[team][id][0]){
        case MOVE_LEFT  :
        case FIRE_LEFT  :
            colD = -1;
            break;
        case MOVE_RIGHT :
        case FIRE_RIGHT :
            colD = 1;
            break;
        case MOVE_UP    :
        case FIRE_UP    :
            rowD = -1;
            break;
        case MOVE_DOWN  :
        case FIRE_DOWN  :
            rowD = 1;
            break;
        default         :
            break;
    }

    nextCellState = env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD];

    switch(env->agentAction[team][id][0]){
        case MOVE_LEFT  :
        case MOVE_RIGHT :
        case MOVE_UP    :
        case MOVE_DOWN  :
            if (i + rowD < 0 || i + rowD >= BLOCK_SIZE || j + colD < 0 || j + colD >= BLOCK_SIZE){
                if (canMoveToNextBlock(env, team, id, self, i, j, rowD, colD)){
                    setMovementReward(env, team, id, rowD, colD);
                    env->controllerUpdateNeeded[team] = 1;

                    curBlockRow = env->agentBlock[team][id].row;
                    curBlockCol = env->agentBlock[team][id].col;
                    moveToNextBlock(env, team, id, self, i, j, rowD, colD);
                    nxtBlockRow = env->agentBlock[team][id].row;
                    nxtBlockCol = env->agentBlock[team][id].col;

                    updateBlockStatus(env, team, curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol, 0, 0);
                }
            }
            else if (nextCellState == EMPTY || nextCellState == homeZone){
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i][j] = 
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].permanentValue[i][j];
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = self;
            }
            else if (nextCellState == BULLET_LEFT || nextCellState == BULLET_RIGHT ||
                     nextCellState == BULLET_UP   || nextCellState == BULLET_DOWN  ){
                setBoom(env, env->agentBlock[team][id].row, env->agentBlock[team][id].col, i+rowD, j+colD);
                setDeathReward(env, team, id);
                env->controllerUpdateNeeded[team] = 1;
                setKillReward(env, team, id, env->agentBlock[team][id].row, env->agentBlock[team][id].col, i+rowD, j+colD, &friendlyFire);
                if (friendlyFire == 0)
                    env->controllerUpdateNeeded[team == GRN ? RED : GRN] = 1;

                /* reset bullet's cell */
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = 
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].permanentValue[i+rowD][j+colD];
                removeBullet(env, env->agentBlock[team][id].row, env->agentBlock[team][id].col, i+rowD, j+colD);
                /* reset agent's cell */
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i][j] = 
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].permanentValue[i][j];

                curBlockRow = env->agentBlock[team][id].row;
                curBlockCol = env->agentBlock[team][id].col;
                respawnAgent(env, team, id);
                nxtBlockRow = env->agentBlock[team][id].row;
                nxtBlockCol = env->agentBlock[team][id].col;

                updateBlockStatus(env, team, curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol, 1, 0);
            }
            else if (nextCellState == endZone){
                env->flash[team == GRN ? RED : GRN] = FLASH_LEN;
                env->agentActive[team][id] = 0;
                setFinishReward(env, team, id);
                env->controllerReward[team] += WIN;

                curBlockRow = env->agentBlock[team][id].row;
                curBlockCol = env->agentBlock[team][id].col;
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i][j] = 
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].permanentValue[i][j];

                if (env->graphics)
                    drawMapSDL(env);

                updateBlockStatus(env, team, curBlockRow, curBlockCol, 0, 0, 0, 1);

                env->score[team]++;
                if (env->score[team] >= NUM_AGENTS){
                    setWinGame(env);
                    return 0;
                }
                else
                    env->controllerUpdateNeeded[team] = 1;
            }
            break;
        case FIRE_LEFT  :
        case FIRE_RIGHT :
        case FIRE_UP    :
        case FIRE_DOWN  :
            if (i + rowD < 0 || i + rowD >= BLOCK_SIZE || j + colD < 0 || j + colD >= BLOCK_SIZE){
                break;
            }
            else if (nextCellState == EMPTY || nextCellState == homeZone || nextCellState == endZone){
                addBullet(env, team, id, env->agentBlock[team][id].row, env->agentBlock[team][id].col, i+rowD, j+colD, rowD, colD);

                if (rowD == -1)
                    env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = BULLET_UP;
                if (rowD == 1)
                    env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = BULLET_DOWN;
                if (colD == -1)
                    env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = BULLET_LEFT;
                if (colD == 1)
                    env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = BULLET_RIGHT;
            }
            else if (nextCellState == BULLET_LEFT || nextCellState == BULLET_RIGHT ||
                     nextCellState == BULLET_UP   || nextCellState == BULLET_DOWN  ){
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = 
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].permanentValue[i+rowD][j+colD];
                removeBullet(env, env->agentBlock[team][id].row, env->agentBlock[team][id].col, i+rowD, j+colD);
            }
            else if (nextCellState == friendlyAgent){
                setBoom(env, env->agentBlock[team][id].row, env->agentBlock[team][id].col, i+rowD, j+colD);
                env->controllerUpdateNeeded[team] = 1;
                setDeathReward(env, team, id == AG1 ? AG2 : AG1);
                setKillReward2(env, team, id, 1, 0);
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = 
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].permanentValue[i+rowD][j+colD];

                curBlockRow = env->agentBlock[team][id].row;
                curBlockCol = env->agentBlock[team][id].col;
                respawnAgent(env, team, id == AG1 ? AG2 : AG1);
                nxtBlockRow = env->agentBlock[team][id == AG1 ? AG2 : AG1].row;
                nxtBlockCol = env->agentBlock[team][id == AG1 ? AG2 : AG1].col;

                updateBlockStatus(env, team, curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol, 1, 0);
            }
            else if (nextCellState == enemy1){
                setBoom(env, env->agentBlock[team][id].row, env->agentBlock[team][id].col, i+rowD, j+colD);
                env->controllerUpdateNeeded[team] = 1;
                env->controllerUpdateNeeded[team == GRN ? RED : GRN] = 1;
                setDeathReward(env, team == GRN ? RED : GRN, AG1);
                setKillReward2(env, team, id, 0, 0);
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = 
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].permanentValue[i+rowD][j+colD];

                
                curBlockRow = env->agentBlock[team][id].row;
                curBlockCol = env->agentBlock[team][id].col;
                respawnAgent(env, team == GRN ? RED : GRN, AG1);
                nxtBlockRow = env->agentBlock[team == GRN ? RED : GRN][AG1].row;
                nxtBlockCol = env->agentBlock[team == GRN ? RED : GRN][AG1].col;

                updateBlockStatus(env, (team == GRN ? RED : GRN), curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol, 1, 0);
            }
            else if (nextCellState == enemy2){
                setBoom(env, env->agentBlock[team][id].row, env->agentBlock[team][id].col, i+rowD, j+colD);
                env->controllerUpdateNeeded[team] = 1;
                env->controllerUpdateNeeded[team == GRN ? RED : GRN] = 1;
                setDeathReward(env, team == GRN ? RED : GRN, AG2);
                setKillReward2(env, team, id, 0, 0);
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i+rowD][j+colD] = 
                env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].permanentValue[i+rowD][j+colD];

                curBlockRow = env->agentBlock[team][id].row;
                curBlockCol = env->agentBlock[team][id].col;
                respawnAgent(env, team == GRN ? RED : GRN, AG2);
                nxtBlockRow = env->agentBlock[team == GRN ? RED : GRN][AG2].row;
                nxtBlockCol = env->agentBlock[team == GRN ? RED : GRN][AG2].col;

                updateBlockStatus(env, (team == GRN ? RED : GRN), curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol, 1, 0);
            }
            break;
        default         :
            break;
    }

    return 1;
}

int canMoveToNextBlock(Env * env, Team team, Identity id, CellStatus self, int i, int j, int rowD, int colD)
{
    int nextI, nextJ;

    if (rowD == -1){
        nextI = BLOCK_SIZE-1;
        nextJ = j;
    }
    if (rowD == 1){
        nextI = 0;
        nextJ = j;
    }
    if (colD == -1){
        nextI = i;
        nextJ = BLOCK_SIZE-1;
    }
    if (colD == 1){
        nextI = i;
        nextJ = 0;
    }

    if (env->block[env->agentBlock[team][id].row + rowD][env->agentBlock[team][id].col + colD].cell[nextI][nextJ] == EMPTY)
        return 1;

    return 0;
}

void moveToNextBlock(Env * env, Team team, Identity id, CellStatus self, int i, int j, int rowD, int colD)
{
    int nextI, nextJ;

    if (rowD == -1){
        nextI = BLOCK_SIZE-1;
        nextJ = j;
    }
    if (rowD == 1){
        nextI = 0;
        nextJ = j;
    }
    if (colD == -1){
        nextI = i;
        nextJ = BLOCK_SIZE-1;
    }
    if (colD == 1){
        nextI = i;
        nextJ = 0;
    }

    if (env->block[env->agentBlock[team][id].row + rowD][env->agentBlock[team][id].col + colD].cell[nextI][nextJ] == EMPTY){
        env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i][j] = 
        env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].permanentValue[i][j];
        env->block[env->agentBlock[team][id].row + rowD][env->agentBlock[team][id].col + colD].cell[nextI][nextJ] = self;

        env->agentBlock[team][id].row += rowD;
        env->agentBlock[team][id].col += colD;
    }
}

void respawnAgent(Env * env, Team team, Identity id)
{
    int agentX, agentY;
    CellStatus homeZone = team == GRN ? GREEN_ZONE : RED_ZONE;
    CellStatus self;

    if (team == GRN && id == AG1)
        self = GREEN_AGENT_1;
    else if (team == GRN && id == AG2)
        self = GREEN_AGENT_2;
    else if (team == RED && id == AG1)
        self = RED_AGENT_1;
    else if (team == RED && id == AG2)
        self = RED_AGENT_2;

    env->agentBlock[team][id].row = env->zoneBlock[team].row;
    env->agentBlock[team][id].col = env->zoneBlock[team].col;

    do{
        agentX = rand() % 3;
        agentY = rand() % 3;
    } while (env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[agentY+3][agentX+3] != homeZone);
    
    env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[agentY+3][agentX+3] = self;

    env->agentMoved[team][id] = 1;
}

void moveBullet(Env * env, Bullet * bullet)
{
    CellStatus nextStatus;
    int curBlockRow, curBlockCol;
    int nxtBlockRow, nxtBlockCol;

    if (bullet->row + bullet->rowD < 0 || bullet->row + bullet->rowD >= BLOCK_SIZE || 
        bullet->col + bullet->colD < 0 || bullet->col + bullet->colD >= BLOCK_SIZE ||
        env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row + bullet->rowD][bullet->col + bullet->colD] == WALL){
        env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row][bullet->col] = 
        env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row][bullet->col];
        bullet = removeValue(env->bullets, bullet);
    }
    else{
        switch(env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row + bullet->rowD][bullet->col + bullet->colD]){
            case EMPTY : case GREEN_ZONE : case RED_ZONE :
                nextStatus = env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row][bullet->col];
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row][bullet->col] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row][bullet->col];
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row + bullet->rowD][bullet->col + bullet->colD] = nextStatus;
                bullet->row += bullet->rowD;
                bullet->col += bullet->colD;
                break;
            case BULLET_LEFT : case BULLET_RIGHT : case BULLET_UP : case BULLET_DOWN :
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row][bullet->col] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row][bullet->col];
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row + bullet->rowD][bullet->col + bullet->colD] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row + bullet->rowD][bullet->col + bullet->colD];
                removeBullet(env, bullet->blockRow, bullet->blockCol, bullet->row + bullet->rowD, bullet->col + bullet->colD);
                bullet = removeValue(env->bullets, bullet);
                break;
            case GREEN_AGENT_1  :
                setBoom(env, bullet->blockRow, bullet->blockCol, bullet->row + bullet->rowD, bullet->col + bullet->colD);
                env->controllerUpdateNeeded[GRN] = 1;
                setDeathReward(env, GRN, AG1);
                setKillReward2(env, bullet->teamOwner, bullet->agentOwner, bullet->teamOwner == GRN, bullet->age);
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row][bullet->col] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row][bullet->col];
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row + bullet->rowD][bullet->col + bullet->colD] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row + bullet->rowD][bullet->col + bullet->colD];
                removeBullet(env, bullet->blockRow, bullet->blockCol, bullet->row, bullet->col);

                curBlockRow = env->agentBlock[GRN][AG1].row;
                curBlockCol = env->agentBlock[GRN][AG1].col;
                respawnAgent(env, GRN, AG1);
                nxtBlockRow = env->agentBlock[GRN][AG1].row;
                nxtBlockCol = env->agentBlock[GRN][AG1].col;

                updateBlockStatus(env, GRN, curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol, 1, 0);
                break;
            case GREEN_AGENT_2  :
                setBoom(env, bullet->blockRow, bullet->blockCol, bullet->row + bullet->rowD, bullet->col + bullet->colD);
                env->controllerUpdateNeeded[GRN] = 1;
                setDeathReward(env, GRN, AG2);
                setKillReward2(env, bullet->teamOwner, bullet->agentOwner, bullet->teamOwner == GRN, bullet->age);
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row][bullet->col] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row][bullet->col];
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row + bullet->rowD][bullet->col + bullet->colD] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row + bullet->rowD][bullet->col + bullet->colD];
                removeBullet(env, bullet->blockRow, bullet->blockCol, bullet->row, bullet->col);

                curBlockRow = env->agentBlock[GRN][AG2].row;
                curBlockCol = env->agentBlock[GRN][AG2].col;
                respawnAgent(env, GRN, AG2);
                nxtBlockRow = env->agentBlock[GRN][AG2].row;
                nxtBlockCol = env->agentBlock[GRN][AG2].col;

                updateBlockStatus(env, GRN, curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol, 1, 0);
                break;
            case RED_AGENT_1    :
                setBoom(env, bullet->blockRow, bullet->blockCol, bullet->row + bullet->rowD, bullet->col + bullet->colD);
                env->controllerUpdateNeeded[RED] = 1;
                setDeathReward(env, RED, AG1);
                setKillReward2(env, bullet->teamOwner, bullet->agentOwner, bullet->teamOwner == RED, bullet->age);
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row][bullet->col] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row][bullet->col];
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row + bullet->rowD][bullet->col + bullet->colD] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row + bullet->rowD][bullet->col + bullet->colD];
                removeBullet(env, bullet->blockRow, bullet->blockCol, bullet->row, bullet->col);

                curBlockRow = env->agentBlock[RED][AG1].row;
                curBlockCol = env->agentBlock[RED][AG1].col;
                respawnAgent(env, RED, AG1);
                nxtBlockRow = env->agentBlock[RED][AG1].row;
                nxtBlockCol = env->agentBlock[RED][AG1].col;

                updateBlockStatus(env, RED, curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol, 1, 0);
                break;
            case RED_AGENT_2    :
                setBoom(env, bullet->blockRow, bullet->blockCol, bullet->row + bullet->rowD, bullet->col + bullet->colD);
                env->controllerUpdateNeeded[RED] = 1;
                setDeathReward(env, RED, AG2);
                setKillReward2(env, bullet->teamOwner, bullet->agentOwner, bullet->teamOwner == RED, bullet->age);
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row][bullet->col] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row][bullet->col];
                env->block[bullet->blockRow][bullet->blockCol].cell[bullet->row + bullet->rowD][bullet->col + bullet->colD] =
                env->block[bullet->blockRow][bullet->blockCol].permanentValue[bullet->row + bullet->rowD][bullet->col + bullet->colD];
                removeBullet(env, bullet->blockRow, bullet->blockCol, bullet->row, bullet->col);

                curBlockRow = env->agentBlock[RED][AG2].row;
                curBlockCol = env->agentBlock[RED][AG2].col;
                respawnAgent(env, RED, AG2);
                nxtBlockRow = env->agentBlock[RED][AG2].row;
                nxtBlockCol = env->agentBlock[RED][AG2].col;

                updateBlockStatus(env, RED, curBlockRow, curBlockCol, nxtBlockRow, nxtBlockCol, 1, 0);
                break;
            default     :
                break;
        }
    }

    if (bullet != NULL)
        bullet->processed = 1;
}

void removeBullet(Env * env, int blockRowVal, int blockColVal, int rowVal, int colVal)
{
    Bullet bullet;

    bullet.blockRow = blockRowVal;
    bullet.blockCol = blockColVal;
    bullet.row = rowVal;
    bullet.col = colVal;

    removeValue(env->bullets, &bullet);
}

void setBoom(Env * env, int blockRow, int blockCol, int row, int col)
{
    int i;

    for (i=0; i<5; i++){
        if (env->boom[i][0] == 0){
            env->boom[i][0] = BOOM_LEN;
            env->boom[i][1] = blockRow;
            env->boom[i][2] = blockCol;
            env->boom[i][3] = row;
            env->boom[i][4] = col;
            return;
        }
    }
}

void updateBlockStatus(Env * env, Team team, int curBlockRow, int curBlockCol, int nextBlockRow, int nextBlockCol, int death, int finished)
{
    Team enem = (team == GRN ? RED : GRN);
    BlockStatus * curBlock, * nextBlock, * enemyBlock, * enemyNextBlock;

    /* in case of death in home zone, no change */
    if (death && curBlockRow == nextBlockRow && curBlockCol == nextBlockCol)
        return;
    
    curBlock        = &env->blockStatus[team][curBlockRow][curBlockCol];
    enemyBlock      = &env->blockStatus[enem][curBlockRow][curBlockCol];

    if (finished){
        nextBlock      = NULL;
        enemyNextBlock = NULL;
    }
    else{
        nextBlock       = &env->blockStatus[team][nextBlockRow][nextBlockCol];
        enemyNextBlock  = &env->blockStatus[enem][nextBlockRow][nextBlockCol];
    }

    updateBlock(env, team, curBlock, curBlockRow, curBlockCol);

    if (nextBlock)
        updateBlock(env, team, nextBlock, nextBlockRow, nextBlockCol);

    if (*enemyBlock > 3){
        env->controllerUpdateNeeded[enem] = 1;
        updateBlock(env, enem, enemyBlock, curBlockRow, curBlockCol);
    }
    if (enemyNextBlock && *enemyNextBlock > 3){
        env->controllerUpdateNeeded[enem] = 1;
        updateBlock(env, enem, enemyNextBlock, nextBlockRow, nextBlockCol);
    }
}

void updateBlock(Env * env, Team team, BlockStatus * block, int blockRow, int blockCol)
{
    BlockStatus prev;
    int agents [NUM_AGENTS+1] = {0};
    int zone = 0;
    int i, j;

    prev = *block;

    if (blockRow == env->zoneBlock[team].row && blockCol == env->zoneBlock[team].col)
        zone = 1;
    else if (blockRow == env->zoneBlock[team == GRN ? RED : GRN].row && blockCol == env->zoneBlock[team == GRN ? RED : GRN].col)
        zone = 2;

    for (i=0; i<BLOCK_SIZE; i++){
        for (j=0; j<BLOCK_SIZE; j++){
            switch(env->block[blockRow][blockCol].cell[i][j]){
                case GREEN_AGENT_1  :
                    if (team == GRN)
                        agents[0]++;
                    else
                        agents[2]++;
                    break;
                case GREEN_AGENT_2  :
                    if (team == GRN)
                        agents[1]++;
                    else
                        agents[2]++;
                    break;
                case RED_AGENT_1    :
                    if (team == RED)
                        agents[0]++;
                    else
                        agents[2]++;
                    break;
                case RED_AGENT_2    :
                    if (team == RED)
                        agents[1]++;
                    else
                        agents[2]++;
                    break;
                default             :
                    break;
            }
        }
    }

    // if zone was and is to remain unoccupied 
    if ((*block == UNEXPLORED || *block == EXPLORED_NORMAL || *block == HOME_ZONE || *block == END_ZONE) && 
        (agents[0] == 0 && agents[1] == 0))
        return;

    // if zone is to become unoccupied
    if (agents[0] == 0 && agents[1] == 0){
        *block = EXPLORED_NORMAL;
        if (prev > 12)
            *block = HOME_ZONE;
        if (prev > 21)
            *block = END_ZONE;
        return;
    }

    // if zone is to become unoccupied
    if (zone == 0)
        *block = 4;
    else if (zone == 1)
        *block = 13;
    else if (zone == 2)
        *block = 22;

    if (agents[0] == 0 && agents[1] == 1)
        (*block) += 1;
    else if (agents[0] == 1 && agents[1] == 1)
        (*block) += 2;

    if (agents[2])
        (*block) += (agents[2] * 3);
}

void updateBlockStatusOld(Env * env, Team team, Identity id, int rowD, int colD, int death, int finish)
{
    Team enem = (team == GRN ? RED : GRN);
    BlockStatus * curBlock, * nextBlock, * enemyBlock, * enemyNextBlock;

    curBlock       = &env->blockStatus[team][env->agentBlock[team][id].row][env->agentBlock[team][id].col];
    enemyBlock     = &env->blockStatus[enem][env->agentBlock[team][id].row][env->agentBlock[team][id].col];

    if (death){
        nextBlock      = &env->blockStatus[team][env->zoneBlock[team].row][env->zoneBlock[team].col];
        enemyNextBlock = &env->blockStatus[enem][env->zoneBlock[team].row][env->zoneBlock[team].col];
    }
    else{
        nextBlock      = &env->blockStatus[team][env->agentBlock[team][id].row+rowD][env->agentBlock[team][id].col+colD];
        enemyNextBlock = &env->blockStatus[enem][env->agentBlock[team][id].row+rowD][env->agentBlock[team][id].col+colD];
    }

    /* in case of death in home zone, no change */
    if (death && (curBlock == nextBlock))
        return;

    /* UPDATE CURRENT BLOCK */

    switch(*curBlock){
        /* No-enemy cases: */
        case NORMAL_AGENT1_NOTHING  : 
        case NORMAL_AGENT2_NOTHING  :
            *curBlock = EXPLORED_NORMAL;
            break;
        case NORMAL_AGENTS_NOTHING  :
            *curBlock = NORMAL_AGENT1_NOTHING + (id == AG1 ? AG2 : AG1);
            break;

        case HOME_AGENT1_NOTHING    :
        case HOME_AGENT2_NOTHING    :
            *curBlock = HOME_ZONE;
            break;
        case HOME_AGENTS_NOTHING    :
            *curBlock = HOME_AGENT1_NOTHING + (id == AG1 ? AG2 : AG1);
            break;

        case END_AGENT1_NOTHING     :
        case END_AGENT2_NOTHING     :
            *curBlock = END_ZONE;
            break;
        case END_AGENTS_NOTHING     :
            *curBlock = END_AGENT1_NOTHING + (id == AG1 ? AG2 : AG1);
            break;

        /* Single-enemy cases: */
        case NORMAL_AGENT1_ENEMY    :
        case NORMAL_AGENT2_ENEMY    :
            *curBlock   = EXPLORED_NORMAL;
            *enemyBlock = ( (*enemyBlock == NORMAL_AGENT1_ENEMY) ? NORMAL_AGENT1_NOTHING : NORMAL_AGENT2_NOTHING);
            env->controllerUpdateNeeded[enem] = 1;
            break;
        case NORMAL_AGENTS_ENEMY    :
            *curBlock   = NORMAL_AGENT1_ENEMY + (id == AG1 ? AG2 : AG1);
            *enemyBlock = ( (*enemyBlock == NORMAL_AGENT1_ENEMIES) ? NORMAL_AGENT1_ENEMY : NORMAL_AGENT2_ENEMY);
            env->controllerUpdateNeeded[enem] = 1;
            break;

        case HOME_AGENT1_ENEMY      :
        case HOME_AGENT2_ENEMY      :
            *curBlock   = HOME_ZONE;
            *enemyBlock = ( (*enemyBlock == END_AGENT1_ENEMY) ? END_AGENT1_NOTHING : END_AGENT2_NOTHING);
            env->controllerUpdateNeeded[enem] = 1;
            break;
        case HOME_AGENTS_ENEMY      :
            *curBlock   = HOME_AGENT1_ENEMY + (id == AG1 ? AG2 : AG1);
            *enemyBlock = ( (*enemyBlock == END_AGENT1_ENEMIES) ? END_AGENT1_ENEMY : END_AGENT2_ENEMY);
            env->controllerUpdateNeeded[enem] = 1;
            break;

        case END_AGENT1_ENEMY       :
        case END_AGENT2_ENEMY       :
            *curBlock   = END_ZONE;
            *enemyBlock = ( (*enemyBlock == HOME_AGENT1_ENEMY) ? HOME_AGENT1_NOTHING : HOME_AGENT2_NOTHING);
            env->controllerUpdateNeeded[enem] = 1;
            break;
        case END_AGENTS_ENEMY       :
            *curBlock   = END_AGENT1_ENEMY + (id == AG1 ? AG2 : AG1);
            *enemyBlock = ( (*enemyBlock == HOME_AGENT1_ENEMIES) ? HOME_AGENT1_ENEMY : HOME_AGENT2_ENEMY);
            env->controllerUpdateNeeded[enem] = 1;
            break;

        /* Multiple-enemy cases: */
        case NORMAL_AGENT1_ENEMIES  :
        case NORMAL_AGENT2_ENEMIES  :
            *curBlock   = EXPLORED_NORMAL;
            *enemyBlock = NORMAL_AGENTS_NOTHING;
            env->controllerUpdateNeeded[enem] = 1;
            break;
        case NORMAL_AGENTS_ENEMIES  :
            *curBlock   = NORMAL_AGENT1_ENEMIES + (id == AG1 ? AG2 : AG1);
            *enemyBlock = NORMAL_AGENTS_ENEMY;
            env->controllerUpdateNeeded[enem] = 1;
            break;

        case HOME_AGENT1_ENEMIES    :
        case HOME_AGENT2_ENEMIES    :
            *curBlock   = HOME_ZONE;
            *enemyBlock = END_AGENTS_NOTHING;
            env->controllerUpdateNeeded[enem] = 1;
            break;
        case HOME_AGENTS_ENEMIES    :
            *curBlock   = HOME_AGENT1_ENEMIES + (id == AG1 ? AG2 : AG1);
            *enemyBlock = END_AGENTS_ENEMY;
            env->controllerUpdateNeeded[enem] = 1;
            break;

        case END_AGENT1_ENEMIES     :
        case END_AGENT2_ENEMIES     :
            *curBlock   = END_ZONE;
            *enemyBlock = HOME_AGENTS_NOTHING;
            env->controllerUpdateNeeded[enem] = 1;
            break;
        case END_AGENTS_ENEMIES     :
            *curBlock   = END_AGENT1_ENEMIES + (id == AG1 ? AG2 : AG1);
            *enemyBlock = HOME_AGENTS_ENEMY;
            env->controllerUpdateNeeded[enem] = 1;
            break;

        default                     :
            break;
    }

    if (finish)
        return;

    /* UPDATE NEXT BLOCK */

    /* if it's inferred that next block must be normal and empty: */
    if ((*nextBlock == UNEXPLORED      || *nextBlock == EXPLORED_NORMAL) && 
        (*enemyNextBlock == UNEXPLORED || *enemyNextBlock == EXPLORED_NORMAL)){
        *nextBlock = NORMAL_AGENT1_NOTHING + id;
    }

    /* if next block is occupied by friendly agent: */
    else if (env->score[team] == 0 && 
            ( (death == 0 && (env->agentBlock[team][id].row+rowD == env->agentBlock[team][id == AG1 ? AG2 : AG1].row   &&
                              env->agentBlock[team][id].col+colD == env->agentBlock[team][id == AG1 ? AG2 : AG1].col)) ||
              (death == 1 && (env->zoneBlock[team].row           == env->agentBlock[team][id == AG1 ? AG2 : AG1].row   &&
                              env->zoneBlock[team].col           == env->agentBlock[team][id == AG1 ? AG2 : AG1].col)) ) ){

        /* changes AGENT1 or AGENT2 to AGENTS (see BlockStatus enum in header for why this works) */
        *nextBlock += id + 1;

        /* changes ENEMY to ENEMIES (see BlockStatus enum in header for why this works) */
        switch (*enemyNextBlock){
            case NORMAL_AGENT1_ENEMY    : case NORMAL_AGENT2_ENEMY  : case NORMAL_AGENTS_ENEMY  :
            case HOME_AGENT1_ENEMY      : case HOME_AGENT2_ENEMY    : case HOME_AGENTS_ENEMY    :
            case END_AGENT1_ENEMY       : case END_AGENT2_ENEMY     : case END_AGENTS_ENEMY     :
                *enemyNextBlock += 3;
                env->controllerUpdateNeeded[enem] = 1;
                break;
            default :
                break;
        }
    }

    /* otherwise, get information about next block from enemy team's blockStatus: */
    else{
        switch (*enemyNextBlock){
            case UNEXPLORED             :
            case EXPLORED_NORMAL        :
                if (*nextBlock == HOME_ZONE)
                    *nextBlock = HOME_AGENT1_NOTHING + id;
                else
                    *nextBlock = NORMAL_AGENT1_NOTHING + id;
                break;

            case NORMAL_AGENT1_NOTHING  :
            case NORMAL_AGENT2_NOTHING  :
                *nextBlock      = NORMAL_AGENT1_ENEMY + id;
                *enemyNextBlock = *enemyNextBlock == NORMAL_AGENT1_NOTHING ? NORMAL_AGENT1_ENEMY : NORMAL_AGENT2_ENEMY;
                env->controllerUpdateNeeded[enem] = 1;
                break;
            case NORMAL_AGENTS_NOTHING  :
                *nextBlock      = NORMAL_AGENT1_ENEMIES + id;
                *enemyNextBlock = NORMAL_AGENTS_ENEMY;
                env->controllerUpdateNeeded[enem] = 1;
                break;

            case HOME_ZONE              :
                *nextBlock      = END_AGENT1_NOTHING + id;
                break;

            case HOME_AGENT1_NOTHING    :
            case HOME_AGENT2_NOTHING    :
                *nextBlock      = END_AGENT1_ENEMY + id;
                *enemyNextBlock = *enemyNextBlock == HOME_AGENT1_NOTHING ? HOME_AGENT1_ENEMY : HOME_AGENT2_ENEMY;
                env->controllerUpdateNeeded[enem] = 1;
                break;
            case HOME_AGENTS_NOTHING    :
                *nextBlock      = END_AGENT1_ENEMIES + id;
                *enemyNextBlock = HOME_AGENTS_ENEMY;
                env->controllerUpdateNeeded[enem] = 1;
                break;

            case END_ZONE               :
                *nextBlock      = HOME_AGENT1_NOTHING + id;
                break;

            case END_AGENT1_NOTHING     :
            case END_AGENT2_NOTHING     :
                *nextBlock      = HOME_AGENT1_ENEMY + id;
                *enemyNextBlock = *enemyNextBlock == END_AGENT1_NOTHING ? END_AGENT1_ENEMY : END_AGENT2_ENEMY;
                env->controllerUpdateNeeded[enem] = 1;
                break;
            case END_AGENTS_NOTHING     :
                *nextBlock      = HOME_AGENT1_ENEMIES + id;
                *enemyNextBlock = END_AGENTS_ENEMY;
                env->controllerUpdateNeeded[enem] = 1;
                break;

            default         :
                break;
        }
    }
}

int main(int argc, char ** argv)
{
    Env * env;
    FILE * fd;

    srand(time(NULL));

    if (!(env = initEnvironment())){
        fprintf(stderr, "Error: memory allocation\n");
        exit(1);
    }

    if (!spawnEnvironment(env)){
        fprintf(stderr, "Error: spawning environment\n");
        exit(1);
    }

    startSDL(env);

    runLoop(env);

    SDL_Quit();

    printf("Exporting agent state-action value tree to \'agentValues.csv'..."); fflush(stdout);
    fd = fopen("agentValues.csv", "w");
    exportTree(env->agentValueTree, fd);
    fclose(fd);
    printf("done!\n"); fflush(stdout);

    printf("Exporting controller state-action value tree to \'ctrlrValues.csv'..."); fflush(stdout);
    fd = fopen("ctrlrValues.csv", "w");
    exportTreeC(env->ctrlrValueTree, fd);
    fclose(fd);
    printf("done!\n"); fflush(stdout);

    destroyEnvironment(env);

    return 0;
}
