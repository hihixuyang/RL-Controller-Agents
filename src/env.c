#include "env.h"


Env * initEnvironment()
{
    FILE * fd, * fd2;
    char fn [128];
    int i;

    Env * env;
    if (!(env = malloc(sizeof(Env)))){
        perror("Error allocating memory!\n");
        exit(1);
    }

    env->gameOver = 0;
    env->showAgentStats = 0;
    env->showControllerStats = 0;
    env->showActionStats = 0;
    env->graphics = 1;
    env->view = 1;
    env->teamShow = GRN;

    env->bullets = createList(compareBullets, destroyBullet);

    env->agentValueTree = createTree();
    env->ctrlrValueTree = createTree();

    if (!(fd = fopen("config", "r"))){
        fprintf(stderr, "Missing config file.\n");
        exit(1);
    }

    fgets(fn, 100, fd);
    i = -1; while (fn[++i] != '\n'); fn[i] = '\0';
    printf("Loading map from \'%s\'...\n", fn); fflush(stdout);
    if (!strcmp(fn, "-")){
        fprintf(stderr, "No map in config.\n");
        exit(1);
    }
    if (!loadMapFromFile(env, fn)){
        fprintf(stderr, "Error loading map from file\n");
        exit(1);
    }

    fgets(fn, 100, fd);
    i = -1; while (fn[++i] != '\n'); fn[i] = '\0';
    if (strcmp(fn, "-")){
        printf("Importing agent state-action value tree from \'%s\'...", fn); fflush(stdout);
        if (!(fd2 = fopen(fn, "r"))){
            printf("error!");
            exit(1);
        }
        importTree(env->agentValueTree, fd2);
        fclose(fd2);
        printf("done!\n"); fflush(stdout);
    }

    fgets(fn, 100, fd);
    i = -1; while (fn[++i] != '\n'); fn[i] = '\0';
    if (strcmp(fn, "-")){
        printf("Importing controller state-action value tree from \'%s\'...", fn); fflush(stdout);
        if (!(fd2 = fopen(fn, "r"))){
            printf("error!");
            exit(1);
        }
        importTreeC(env->ctrlrValueTree, fd2);
        fclose(fd2);
        printf("done!\n"); fflush(stdout);
    }

    fgets(fn, 100, fd);
    i = -1; while (fn[++i] != '\n'); fn[i] = '\0';
    env->USE_NN = 0;
    if (strcmp(fn, "-")){
        env->USE_NN = 1;
        printf("Importing neural network parameters from \'%s\'...", fn); fflush(stdout);
        if (!(fd2 = fopen(fn, "r"))){
            printf("error!");
            exit(1);
        }

        /* import neural network */
        env->nnParams = malloc(sizeof(struct NNParams));
        importNNParams(env->nnParams, fd2);

        fclose(fd2);
        printf("done!\n"); fflush(stdout);
    }

    fclose(fd);

    return env;
}

int loadMapFromFile(Env * env, char * fn)
{
    FILE * fd;
    int i, j;

    if (!(fd = fopen(fn, "r"))){
        destroyEnvironment(env);
        return 0;
    }

    for (i=0; i<GRID_SIZE * BLOCK_SIZE; i++){
        if ((i != 0) && (i % BLOCK_SIZE == 0))
            fgetc(fd);
        for (j=0; j<GRID_SIZE * BLOCK_SIZE; j++){
            if ((j != 0) && (j % BLOCK_SIZE == 0))
                fgetc(fd);
            env->block[i/BLOCK_SIZE][j/BLOCK_SIZE].cell[i%BLOCK_SIZE][j%BLOCK_SIZE] = fgetc(fd) - '0';
            if (env->block[i/BLOCK_SIZE][j/BLOCK_SIZE].cell[i%BLOCK_SIZE][j%BLOCK_SIZE] == 1)
                env->block[i/BLOCK_SIZE][j/BLOCK_SIZE].cell[i%BLOCK_SIZE][j%BLOCK_SIZE] = 10;
            env->block[i/BLOCK_SIZE][j/BLOCK_SIZE].permanentValue[i%BLOCK_SIZE][j%BLOCK_SIZE] = 
                    env->block[i/BLOCK_SIZE][j/BLOCK_SIZE].cell[i%BLOCK_SIZE][j%BLOCK_SIZE];
        }
        fgetc(fd);
    }

    fclose(fd);

    return 1;
}

int spawnEnvironment(Env * env)
{
    int i, j;
    int agent1X, agent1Y, agent2X, agent2Y;

    for (i=0; i<5; i++)
        for (j=0; j<5; j++)
            env->boom[i][j] = 0;

    env->zoneBlock[GRN].row = rand() % GRID_SIZE;
    env->zoneBlock[GRN].col = rand() % GRID_SIZE;
    do{
        env->zoneBlock[RED].row = rand() % GRID_SIZE;
        env->zoneBlock[RED].col = rand() % GRID_SIZE;
    } while (env->zoneBlock[RED].row == env->zoneBlock[GRN].row && 
             env->zoneBlock[RED].col == env->zoneBlock[GRN].col);

    for (i=0; i<3; i++)
        for (j=0; j<3; j++){
            env->block[env->zoneBlock[GRN].row][env->zoneBlock[GRN].col].cell[i+3][j+3] = GREEN_ZONE;
            env->block[env->zoneBlock[GRN].row][env->zoneBlock[GRN].col].permanentValue[i+3][j+3] = GREEN_ZONE;
            env->block[env->zoneBlock[RED].row][env->zoneBlock[RED].col].cell[i+3][j+3] = RED_ZONE;
            env->block[env->zoneBlock[RED].row][env->zoneBlock[RED].col].permanentValue[i+3][j+3] = RED_ZONE;
        }

    for (i=0; i<NUM_AGENTS; i++){
        env->agentBlock[GRN][i].row = env->zoneBlock[GRN].row;
        env->agentBlock[GRN][i].col = env->zoneBlock[GRN].col;
        env->agentBlock[RED][i].row = env->zoneBlock[RED].row;
        env->agentBlock[RED][i].col = env->zoneBlock[RED].col;
    }

    agent1X = rand() % 3;
    agent1Y = rand() % 3;

    do{
        agent2X = rand() % 3;
        agent2Y = rand() % 3;
    } while (agent2X == agent1X && agent2Y == agent1Y);

    env->block[env->zoneBlock[GRN].row][env->zoneBlock[GRN].col].cell[agent1X+3][agent1Y+3] = GREEN_AGENT_1;
    env->block[env->zoneBlock[GRN].row][env->zoneBlock[GRN].col].cell[agent2X+3][agent2Y+3] = GREEN_AGENT_2;
    env->block[env->zoneBlock[RED].row][env->zoneBlock[RED].col].cell[agent1X+3][agent1Y+3] = RED_AGENT_1;
    env->block[env->zoneBlock[RED].row][env->zoneBlock[RED].col].cell[agent2X+3][agent2Y+3] = RED_AGENT_2;

    env->controllerEpisodeStart = 1;

    return 1;
}

void addBullet(Env * env, int teamVal, int idVal, int blockRowVal, int blockColVal, int rowVal, int colVal, int rowDVal, int colDVal)
{
    Bullet * bullet;
    
    if (!(bullet = malloc(sizeof(Bullet)))){
        perror("Error allocating memory!\n");
        exit(1);
    }

    bullet->newEntry    = 1;
    bullet->processed   = 1;
    bullet->age         = 0;
    bullet->teamOwner   = teamVal;
    bullet->agentOwner  = idVal;
    bullet->blockRow    = blockRowVal;
    bullet->blockCol    = blockColVal;
    bullet->row         = rowVal;
    bullet->col         = colVal;
    bullet->rowD        = rowDVal;
    bullet->colD        = colDVal;

    addToList(env->bullets, bullet);
}

void initControllerStateSignals(Env * env)
{
    int i, j;

    for (i=0; i<GRID_SIZE; i++){
        for (j=0; j<GRID_SIZE; j++){
            env->blockStatus[GRN][i][j] = UNEXPLORED;
            env->blockStatus[RED][i][j] = UNEXPLORED;
        }
    }

    env->blockStatus[GRN][env->zoneBlock[GRN].row][env->zoneBlock[GRN].col] = HOME_AGENTS_NOTHING;
    env->blockStatus[RED][env->zoneBlock[RED].row][env->zoneBlock[RED].col] = HOME_AGENTS_NOTHING;
}

void prepareAgentStateSignals(Env * env)
{
    Team team;
    Identity id;
    int agents [2];
    int agentRow, agentCol;
    int outerBound, innerBound;
    int i, j, k, l, m;

    for (team=0; team < 2; team++){
        for (id=0; id < NUM_AGENTS; id++){

            env->agentState[team][id][0][1] = env->agentGoal[team][id][0];  // agent goal

            env->agentState[team][id][0][2] = 0;       // block type: normal/home
            if (env->blockStatus[team][env->agentBlock[team][id].row][env->agentBlock[team][id].col] > 21)
                env->agentState[team][id][0][2] = 1;   // block type: end

            agentRow = agentCol = 0;

            for (i=0; i < BLOCK_SIZE; i++){
                for (j=0; j < BLOCK_SIZE; j++){
                    switch(env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i][j]){
                        case    GREEN_AGENT_1   :
                            if (team == GRN && id == AG1){
                                agentRow = i;
                                agentCol = j;
                            }
                            break;
                        case    GREEN_AGENT_2   :
                            if (team == GRN && id == AG2){
                                agentRow = i;
                                agentCol = j;
                            }
                            break;
                        case    RED_AGENT_1     :
                            if (team == RED && id == AG1){
                                agentRow = i;
                                agentCol = j;
                            }
                            break;
                        case    RED_AGENT_2     :
                            if (team == RED && id == AG2){
                                agentRow = i;
                                agentCol = j;
                            }
                            break;
                        default                 :
                            break;
                    }
                }
            }

            env->agentState[team][id][0][3] = agentRow;    // agent row
            env->agentState[team][id][0][4] = agentCol;    // agent col

            m = 5;
            outerBound = innerBound = 0;
            for (k=0; k<3; k++){
                outerBound = (BLOCK_SIZE/3)*(k+1);
                for (l=0; l<3; l++){
                    innerBound = (BLOCK_SIZE/3)*(l+1);

                    agents[0] = agents[1] = 0;

                    for (i=outerBound-BLOCK_SIZE/3; i<outerBound; i++){
                        for (j=innerBound-BLOCK_SIZE/3; j<innerBound; j++){
                            switch (env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i][j]){
                                case GREEN_AGENT_1  :
                                    if (team == GRN && id == AG2)
                                        agents[0]++;
                                    else if (team == RED)
                                        agents[1]++;
                                    break;
                                case GREEN_AGENT_2  :
                                    if (team == GRN && id == AG1)
                                        agents[0]++;
                                    else if (team == RED)
                                        agents[1]++;
                                    break;
                                case RED_AGENT_1    :
                                    if (team == RED && id == AG2)
                                        agents[0]++;
                                    else if (team == GRN)
                                        agents[1]++;
                                    break;
                                case RED_AGENT_2    :
                                    if (team == RED && id == AG1)
                                        agents[0]++;
                                    else if (team == GRN)
                                        agents[1]++;
                                    break;
                                default             :
                                    break;
                            }
                        }
                    }

                    if      (agents[0] == 0 && agents[1] == 0)
                        env->agentState[team][id][0][m] = 0;       // quadrant m-4: empty
                    else if (agents[0] == 0 && agents[1] > 0 )
                        env->agentState[team][id][0][m] = 1;       // quadrant m-4: enemy(s)
                    else if (agents[0] == 1 && agents[1] == 0)
                        env->agentState[team][id][0][m] = 2;       // quadrant m-4: friendly
                    else if (agents[0] == 1 && agents[1] > 0 )
                        env->agentState[team][id][0][m] = 3;       // quadrant m-4: friendly-enemy(s)
                    else{
                        printf("agents[0]=%d, agents[1]=%d\n", agents[0], agents[1]);
                    }
             
                    m++;           
                }
            }

            // agent critical surround field
            for (i=agentRow-2; i<=agentRow+2; i++){
                for (j=agentCol-2; j<=agentCol+2; j++){
                    if (i < 0 || i >= BLOCK_SIZE || j < 0 || j >= BLOCK_SIZE){
                        env->agentState[team][id][0][m] = EMPTY;
                        m++;
                        continue;
                    }

                    switch(env->block[env->agentBlock[team][id].row][env->agentBlock[team][id].col].cell[i][j]){
                        case GREEN_AGENT_1  :
                            if      (team == GRN && id == AG1)
                                env->agentState[team][id][0][m] = SELF;
                            else if (team == GRN && id == AG2)
                                env->agentState[team][id][0][m] = FRIENDLY;
                            else if (team == RED)
                                env->agentState[team][id][0][m] = ENEMY;
                            break;
                        case GREEN_AGENT_2  :
                            if      (team == GRN && id == AG2)
                                env->agentState[team][id][0][m] = SELF;
                            else if (team == GRN && id == AG1)
                                env->agentState[team][id][0][m] = FRIENDLY;
                            else if (team == RED)
                                env->agentState[team][id][0][m] = ENEMY;
                            break;
                        case RED_AGENT_1    :
                            if      (team == RED && id == AG1)
                                env->agentState[team][id][0][m] = SELF;
                            else if (team == RED && id == AG2)
                                env->agentState[team][id][0][m] = FRIENDLY;
                            else if (team == GRN)
                                env->agentState[team][id][0][m] = ENEMY;
                            break;
                        case RED_AGENT_2    :
                            if      (team == RED && id == AG2)
                                env->agentState[team][id][0][m] = SELF;
                            else if (team == RED && id == AG1)
                                env->agentState[team][id][0][m] = FRIENDLY;
                            else if (team == GRN)
                                env->agentState[team][id][0][m] = ENEMY;
                            break;
                        case BULLET_LEFT    :
                            if (i >= agentRow-1 && i <= agentRow+1 && j >= agentCol)
                                env->agentState[team][id][0][m] = BULLET_LEFT;
                            else
                                env->agentState[team][id][0][m] = EMPTY;
                            break;
                        case BULLET_RIGHT   :
                            if (i >= agentRow-1 && i <= agentRow+1 && j <= agentCol)
                                env->agentState[team][id][0][m] = BULLET_RIGHT;
                            else
                                env->agentState[team][id][0][m] = EMPTY;
                            break;
                        case BULLET_UP      :
                            if (j >= agentCol-1 && j <= agentCol+1 && i >= agentRow)
                                env->agentState[team][id][0][m] = BULLET_UP;
                            else
                                env->agentState[team][id][0][m] = EMPTY;
                            break;
                        case BULLET_DOWN    :
                            if (j >= agentCol-1 && j <= agentCol+1 && i <= agentRow)
                                env->agentState[team][id][0][m] = BULLET_DOWN;
                            else
                                env->agentState[team][id][0][m] = EMPTY;
                            break;
                        default             :
                            env->agentState[team][id][0][m] = EMPTY;
                            break;
                    }
                m++;

                }
            }
        }
    }
}

void prepareControllerStateSignal(Env * env, Team team)
{
    int i, j, k;

    env->controllerState[team][0][0] = 0;   // agent 1 action
    env->controllerState[team][0][1] = 0;   // agent 2 action
    k = 2;
    for (i=0; i<GRID_SIZE; i++){
        for (j=0; j<GRID_SIZE; j++){
            switch (env->blockStatus[team][i][j]){
                case HOME_ZONE             :
                    env->controllerState[team][0][k] = EXPLORED_NORMAL;
                    break;
                case NORMAL_AGENT1_NOTHING :    case NORMAL_AGENT1_ENEMY :  case NORMAL_AGENT1_ENEMIES  :
                    env->controllerState[team][0][k] = 4;
                    break;
                case NORMAL_AGENT2_NOTHING :    case NORMAL_AGENT2_ENEMY :  case NORMAL_AGENT2_ENEMIES  :
                    env->controllerState[team][0][k] = 5;
                    break;
                case NORMAL_AGENTS_NOTHING :    case NORMAL_AGENTS_ENEMY :  case NORMAL_AGENTS_ENEMIES  :
                    env->controllerState[team][0][k] = 6;
                    break;
                case HOME_AGENT1_NOTHING   :    case HOME_AGENT1_ENEMY   :  case HOME_AGENT1_ENEMIES    :
                    env->controllerState[team][0][k] = 7;
                    break;
                case HOME_AGENT2_NOTHING   :    case HOME_AGENT2_ENEMY   :  case HOME_AGENT2_ENEMIES    :
                    env->controllerState[team][0][k] = 8;
                    break;
                case HOME_AGENTS_NOTHING   :    case HOME_AGENTS_ENEMY   :  case HOME_AGENTS_ENEMIES    :
                    env->controllerState[team][0][k] = 9;
                    break;
                case END_AGENT1_NOTHING    :    case END_AGENT1_ENEMY    :  case END_AGENT1_ENEMIES     :
                    env->controllerState[team][0][k] = 10;
                    break;
                case END_AGENT2_NOTHING    :    case END_AGENT2_ENEMY    :  case END_AGENT2_ENEMIES     :
                    env->controllerState[team][0][k] = 11;
                    break;
                case END_AGENTS_NOTHING    :    case END_AGENTS_ENEMY    :  case END_AGENTS_ENEMIES     :
                    env->controllerState[team][0][k] = 12;
                    break;
                default :
                    env->controllerState[team][0][k] = env->blockStatus[team][i][j];
                    break;
            }
            k++;
        }
    }
}

void setWinGame(Env * env)
{
    FILE * fd;
    static int iter=0;
    int i;

    env->flash[GRN] = 0;
    env->flash[RED] = 0;

    if (env->score[GRN] >= NUM_AGENTS)
        env->controllerReward[GRN] += WIN;
    if (env->score[RED] >= NUM_AGENTS)
        env->controllerReward[RED] += WIN;

    env->controllerEpisodeReset[GRN] = 1;
    env->controllerEpisodeReset[RED] = 1;
    for (i=0; i<NUM_AGENTS; i++){
        env->agentEpisodeReset[GRN][i] = 1;
        env->agentEpisodeReset[RED][i] = 1;
    }

    env->controllerRequest[GRN]      = ACTION_REQUESTED;
    env->controllerRequest[RED]      = ACTION_REQUESTED;
    for (i=0; i<NUM_AGENTS; i++){
        env->agentRequest[GRN][i] = ACTION_REQUESTED;
        env->agentRequest[RED][i] = ACTION_REQUESTED;
    }

    while (  env->controllerRequest[GRN] != ACTION_UPDATED ||
             env->controllerRequest[RED] != ACTION_UPDATED ||
            (env->agentRequest[GRN][AG1] != ACTION_UPDATED && env->agentActive[GRN][AG1]) ||
            (env->agentRequest[GRN][AG2] != ACTION_UPDATED && env->agentActive[GRN][AG2]) ||
            (env->agentRequest[RED][AG1] != ACTION_UPDATED && env->agentActive[RED][AG1]) ||
            (env->agentRequest[RED][AG2] != ACTION_UPDATED && env->agentActive[RED][AG2]) );

    for (i=0; i<NUM_AGENTS; i++){
        env->agentRequest[GRN][i] = ACTION_UPDATED;
        env->agentRequest[RED][i] = ACTION_UPDATED;
        env->agentActive[GRN][i]  = 1;
        env->agentActive[RED][i]  = 1;
    }
    
    if (iter >= SAVE_FREQ){
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

        iter = -1;
    }
    iter++;

    env->bullets = destroyList(env->bullets);
    env->bullets = createList(compareBullets, destroyBullet);

    clearMap(env);
    spawnEnvironment(env);

    env->score[GRN] = 0;
    env->score[RED] = 0;
}

void clearMap(Env * env)
{
    int blockRow, blockCol;
    int i, j;

    for (blockRow=0; blockRow<GRID_SIZE; blockRow++){
        for (blockCol=0; blockCol<GRID_SIZE; blockCol++){
            for (i=0; i<BLOCK_SIZE; i++){
                for (j=0; j<BLOCK_SIZE; j++){
                    if (env->block[blockRow][blockCol].permanentValue[i][j] == GREEN_ZONE ||
                        env->block[blockRow][blockCol].permanentValue[i][j] == RED_ZONE)
                        env->block[blockRow][blockCol].permanentValue[i][j] = EMPTY;
                    env->block[blockRow][blockCol].cell[i][j] = env->block[blockRow][blockCol].permanentValue[i][j];
                }
            }
        }
    }
}

Env * destroyEnvironment(Env * env)
{
    env->bullets = destroyList(env->bullets);

    env->agentValueTree = destroyTree (env->agentValueTree, 0);
    env->ctrlrValueTree = destroyTreeC(env->ctrlrValueTree, 0);

    if (env->USE_NN)
        destroyNNParams(env->nnParams);

    free(env);

    return NULL;
}
