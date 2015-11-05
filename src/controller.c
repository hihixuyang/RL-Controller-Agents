#include "controller.h"

void * controllerFunction(void * arg)
{
    Team team = ((ControllerStruct *)arg)->team;
    Env * env = ((ControllerStruct *)arg)->env;
    int episode [CTRLR_STATE_MEMORY+1];
    int startGame, update, i;

    episode[0] = 0;
    startGame = 1;

    while (!env->gameOver){
        env->controllerRequest[team] = ACTION_UPDATED;

        while (!(env->controllerRequest[team] == ACTION_REQUESTED || env->gameOver)){
            /* planning... */
        }

        if (env->gameOver)
            break;

        /* roll back episode vector */
        for (i=CTRLR_STATE_MEMORY; i>0; i--)
            episode[i] = episode[i-1];
        /* update current episode */
        if (env->controllerEpisodeReset[team])
            episode[0] = episode[0] >= CTRLR_STATE_MEMORY-1 ? 0 : episode[0]+1;
        env->controllerEpisodeReset[team] = 0;

        /* determine if actual state change for controller (or an agent possibly stuck defending) */
        if (env->controllerReward[team] != 0 || !statesEqualC(env->controllerState[team][0], env->controllerState[team][1]))
            update = 1;
        /* if enemy leaves block while agent is engaging, update agent's goal */
        else if ( (env->controllerState[team][1][AG1] == ENGAGE && 
              !hasEnemy(env->blockStatus[team][env->agentBlock[team][AG1].row][env->agentBlock[team][AG1].col])) || 
             (env->controllerState[team][1][AG2] == ENGAGE &&
              !hasEnemy(env->blockStatus[team][env->agentBlock[team][AG2].row][env->agentBlock[team][AG2].col])) ){
            update = 1;
        } 
        else
            update = 0;

        if (update)
            getAgentsToUpdate(env, team, episode);

        /* update state */
        if ((!startGame) && update){
            updateControllerStates(env, team, episode);
        }
        startGame = 0;

        if (update){
            /* determine next action if actual state change for controller */
            chooseControllerAction(env, team);
        }
        else{
            /* choose previous action if no actual state change for controller */
            env->controllerState[team][0][AG1]  = env->controllerState[team][1][AG1];
            env->controllerState[team][0][AG2]  = env->controllerState[team][1][AG2];
            env->agentGoal[team][AG1][0]        = env->controllerState[team][1][AG1];
            env->agentGoal[team][AG2][0]        = env->controllerState[team][1][AG2];
        }

        /* print state */
        if (team == GRN && env->showControllerStats && update){
            printControllerState(env);
            printControllerAction(env);
        }

        /* roll back state matrix */
        rollControllerStates(env, team);

        env->controllerReward[team] = 0;
    }

    pthread_exit(NULL);
}

int statesEqualC(int * state1, int * state2)
{
    int i;

    for (i=2; i<CTRLR_STATE_ACTION_SIZE; i++)
        if (state1[i] != state2[i])
            return 0;

    return 1;
}

void getAgentsToUpdate(Env * env, Team team, int * episode)
{
    int agentState1, agentState2, i;

    env->controllerAgentToUpdate[team][AG1] = 0;
    env->controllerAgentToUpdate[team][AG2] = 0;

    if (episode[0] != episode[1] || env->controllerEpisodeStart){
        env->controllerAgentToUpdate[team][AG1] = 1;
        env->controllerAgentToUpdate[team][AG2] = 1;
        return;
    }

    agentState1 = agentState2 = -1;
    for (i=2; i<CTRLR_STATE_ACTION_SIZE; i++){
        if (env->controllerState[team][0][i] == 4  || env->controllerState[team][0][i] == 6  ||
            env->controllerState[team][0][i] == 7  || env->controllerState[team][0][i] == 9  ||
            env->controllerState[team][0][i] == 10 || env->controllerState[team][0][i] == 12 ){
            agentState1 = i;
            break;
        }
    }
    for (i=2; i<CTRLR_STATE_ACTION_SIZE; i++){
        if (env->controllerState[team][1][i] == 4  || env->controllerState[team][1][i] == 6  ||
            env->controllerState[team][1][i] == 7  || env->controllerState[team][1][i] == 9  ||
            env->controllerState[team][1][i] == 10 || env->controllerState[team][1][i] == 12 ){
            agentState2 = i;
            break;
        }
    }

    if (agentState1 != agentState2)
        env->controllerAgentToUpdate[team][AG1] = 1;
    else if (env->controllerState[team][1][AG1] == ENGAGE){
        switch (env->blockStatus[team][env->agentBlock[team][AG1].row][env->agentBlock[team][AG1].col]){
            case NORMAL_AGENT1_NOTHING :    case NORMAL_AGENTS_NOTHING :
            case HOME_AGENT1_NOTHING   :    case HOME_AGENTS_NOTHING   :
            case END_AGENT1_NOTHING    :    case END_AGENTS_NOTHING    :
                env->controllerAgentToUpdate[team][AG1] = 1;
                break;
            default :
                break;
        } 
    }

    agentState1 = agentState2 = -1;
    for (i=2; i<CTRLR_STATE_ACTION_SIZE; i++){
        if (env->controllerState[team][0][i] == 5  || env->controllerState[team][0][i] == 6  ||
            env->controllerState[team][0][i] == 8  || env->controllerState[team][0][i] == 9  ||
            env->controllerState[team][0][i] == 11 || env->controllerState[team][0][i] == 12 ){
            agentState1 = i;
            break;
        }
    }
    for (i=2; i<CTRLR_STATE_ACTION_SIZE; i++){
        if (env->controllerState[team][1][i] == 5  || env->controllerState[team][1][i] == 6  ||
            env->controllerState[team][1][i] == 8  || env->controllerState[team][1][i] == 9  ||
            env->controllerState[team][1][i] == 11 || env->controllerState[team][1][i] == 12 ){
            agentState2 = i;
            break;
        }
    }
    if (agentState1 != agentState2)
        env->controllerAgentToUpdate[team][AG2] = 1;
    else if (env->controllerState[team][1][AG2] == ENGAGE){
        switch (env->blockStatus[team][env->agentBlock[team][AG2].row][env->agentBlock[team][AG2].col]){
            case NORMAL_AGENT2_NOTHING :    case NORMAL_AGENTS_NOTHING :
            case HOME_AGENT2_NOTHING   :    case HOME_AGENTS_NOTHING   :
            case END_AGENT2_NOTHING    :    case END_AGENTS_NOTHING    :
                env->controllerAgentToUpdate[team][AG2] = 1;
                break;
            default :
                break;
        } 
    }
}

void updateControllerStates(Env * env, Team team, int * episode)
{
    float value;
    float nextStateValue;
    float error;
    float eligibility;
    float Y             = 0.85;
    float learningRate  = 0.1;
    float updateValue;
    int state [CTRLR_STATE_ACTION_SIZE];
    int i, j;

    if (!env->controllerReward[team])
        return;

    sem_wait(&ctrlrSem);

    copyState(state, env->controllerState[team][0], CTRLR_STATE_ACTION_SIZE);
    nextStateValue = 0;
    for (i=0; i<NUM_CTRLR_ACTIONS; i++){
        if (isEligibleAction(env, team, AG1, i)){
            for (j=0; j<NUM_CTRLR_ACTIONS; j++){
                if (isEligibleAction(env, team, AG2, j)){
                    state[AG1] = i;
                    state[AG2] = j;
                    value = getStateValueC(env->ctrlrValueTree, state, 0);
                    if (value > nextStateValue)
                        nextStateValue = value;
                }
            }
        }
    }

    error = (env->controllerReward[team] + Y * nextStateValue) - getStateValueC(env->ctrlrValueTree, env->controllerState[team][1], 0);

    if (error != 0){
        eligibility = 1;
        i = 1;
        while ( (episode[i] == episode[1]) && (i < CTRLR_STATE_MEMORY) ){
            error = (env->controllerReward[team] + Y * nextStateValue) - getStateValueC(env->ctrlrValueTree, env->controllerState[team][i], 0);
            updateValue = getStateValueC(env->ctrlrValueTree, env->controllerState[team][i], 0) + learningRate * eligibility * error;
            /*if      (updateValue > 370) updateValue = 370;
            else if (updateValue < 0  ) updateValue = 0;*/
            updateStateValueC(env->ctrlrValueTree, env->controllerState[team][i], updateValue, 0, 0);
            eligibility *= 0.85;
            i++;
        }
    }

    sem_post(&ctrlrSem);
}

void rollControllerStates(Env * env, Team team)
{
    int i, j;

    for (i=CTRLR_STATE_MEMORY; i>0; i--)
        for (j=0; j<CTRLR_STATE_ACTION_SIZE; j++)
            env->controllerState[team][i][j] = env->controllerState[team][i-1][j];
}

void chooseControllerAction(Env * env, Team team)
{
    float maxValue=-1, value;
    float values [NUM_CTRLR_ACTIONS][NUM_CTRLR_ACTIONS];
    AgentGoal maxAction [2] = {0,0};
    int equalMax, i, j;

    if (rand() % EPSILON_B == 0 || statesEqualC(env->controllerState[team][0], env->controllerState[team][1]) ||
                                   statesEqualC(env->controllerState[team][0], env->controllerState[team][2]) ){
        do{
            maxAction[AG1] = rand() % NUM_CTRLR_ACTIONS;
            maxAction[AG2] = rand() % NUM_CTRLR_ACTIONS;
        } while (!(isEligibleAction(env, team, AG1, maxAction[AG1]) &&
                   isEligibleAction(env, team, AG2, maxAction[AG2])));
        env->controllerState[team][0][AG1]  = maxAction[AG1];
        env->controllerState[team][0][AG2]  = maxAction[AG2];
        env->agentGoal[team][AG1][0]        = maxAction[AG1];
        env->agentGoal[team][AG2][0]        = maxAction[AG2];
        return;
    }

    for (i=0; i<NUM_CTRLR_ACTIONS; i++)
        for (j=0; j<NUM_CTRLR_ACTIONS; j++)
            values[i][j] = -1000.0;

    sem_wait(&ctrlrSem);

    for (i=0; i<NUM_CTRLR_ACTIONS; i++){
        if (isEligibleAction(env, team, AG1, i)){
            for (j=0; j<NUM_CTRLR_ACTIONS; j++){
                if (isEligibleAction(env, team, AG2, j)){
                    env->controllerState[team][0][AG1] = i;
                    env->controllerState[team][0][AG2] = j;
                    value = getStateValueC(env->ctrlrValueTree, env->controllerState[team][0], 0);
                    values[i][j] = value;
                    if (value > maxValue){
                        maxValue = value;
                        maxAction[AG1] = i;
                        maxAction[AG2] = j;
                    }
                }
            }
        }
    }

    equalMax = 0;
    for (i=0; i<NUM_CTRLR_ACTIONS; i++)
        for (j=0; j<NUM_CTRLR_ACTIONS; j++)
            if (values[i][j] == maxValue)
                equalMax++;

    /* if more than one action pair at maxValue, select action pair with equal likelihood ... */
    if (equalMax > 0){
        do{
            maxAction[AG1] = rand() % NUM_CTRLR_ACTIONS;
            maxAction[AG2] = rand() % NUM_CTRLR_ACTIONS;
        } while (values[maxAction[AG1]][maxAction[AG2]] < maxValue);
    }

    env->controllerState[team][0][AG1]  = maxAction[AG1];
    env->controllerState[team][0][AG2]  = maxAction[AG2];
    env->agentGoal[team][AG1][0]        = maxAction[AG1];
    env->agentGoal[team][AG2][0]        = maxAction[AG2];

    sem_post(&ctrlrSem);
}

int isEligibleAction(Env * env, Team team, Identity id, AgentGoal action)
{
    BlockStatus status;
    status = env->blockStatus[team][env->agentBlock[team][id].row][env->agentBlock[team][id].col];

    /*printf("...\n");
    printf("team: %d, agent: %d, alive: %d\n", team, id, env->agentActive[team][id]);
    printf("block: %d,%d, block status: %d\n", env->agentBlock[team][id].row, env->agentBlock[team][id].col, status);
    printf("action: %d, previous action: %d\n", action, env->controllerState[team][1][id]);
    printf("agentToUpdate: %d\n", env->controllerAgentToUpdate[team][id]);
    fflush(stdout);*/

    /* if agent active, never choose NO_ACTION */
    if (env->agentActive[team][id] && action == NO_ACTION)
        return 0;
    /* if agent not active, always choose NO_ACTION */
    if (env->agentActive[team][id] == 0){
        if (action == NO_ACTION)
            return 1;
        else
            return 0;
    }

    if (!env->controllerAgentToUpdate[team][id]){
        if (action == env->controllerState[team][1][id])
            return 1;
        else
            return 0;
    }

    switch (action){
        case ENGAGE     :
            if (status == NORMAL_AGENT1_NOTHING || status == NORMAL_AGENT2_NOTHING  ||
                status == NORMAL_AGENTS_NOTHING || status == HOME_AGENT1_NOTHING    ||
                status == HOME_AGENT2_NOTHING   || status == HOME_AGENTS_NOTHING    ||
                status == END_AGENT1_NOTHING    || status == END_AGENT2_NOTHING     ||
                status == END_AGENTS_NOTHING)
                return 0;
            break;
        case FINISH     :
            if (! (status == END_AGENT1_NOTHING  || status == END_AGENT1_ENEMY       ||
                   status == END_AGENT1_ENEMIES  || status == END_AGENT2_NOTHING     || 
                   status == END_AGENT2_ENEMY    || status == END_AGENT2_ENEMIES     || 
                   status == END_AGENTS_NOTHING  || status == END_AGENTS_ENEMY       || 
                   status == END_AGENTS_ENEMIES) )
                return 0;
            break;
        case GO_LEFT    :
            if (env->agentBlock[team][id].col == 0)
                return 0;
            break;
        case GO_RIGHT   :
            if (env->agentBlock[team][id].col == GRID_SIZE-1)
                return 0;
            break;
        case GO_UP      :
            if (env->agentBlock[team][id].row == 0)
                return 0;
            break;
        case GO_DOWN    :
            if (env->agentBlock[team][id].row == GRID_SIZE-1)
                return 0;
            break;
        default         :
            break;
    }

    return 1;
}

void printControllerAction(Env * env)
{
    printf("agent 1: ");
    switch (env->controllerState[GRN][0][AG1]){
        case ENGAGE     :
            printf("ENGAGE   ");
            break;
        case FINISH     :
            printf("FINISH   ");
            break;
        case GO_LEFT    :
            printf("GO LEFT  ");
            break;
        case GO_RIGHT   :
            printf("GO RIGHT ");
            break;
        case GO_UP      :
            printf("GO UP    ");
            break;
        case GO_DOWN    :
            printf("GO DOWN  ");
            break;
        case NO_ACTION  :
            printf("NONE     ");
            break;
    }

    printf("\tagent 2: ");
    switch (env->controllerState[GRN][0][AG2]){
        case ENGAGE     :
            printf("ENGAGE   ");
            break;
        case FINISH     :
            printf("FINISH   ");
            break;
        case GO_LEFT    :
            printf("GO LEFT  ");
            break;
        case GO_RIGHT   :
            printf("GO RIGHT ");
            break;
        case GO_UP      :
            printf("GO UP    ");
            break;
        case GO_DOWN    :
            printf("GO DOWN  ");
            break;
        case NO_ACTION  :
            printf("NONE     ");
            break;
    }

    printf("\tvalue: ");
    sem_wait(&ctrlrSem);
    printf("%-7.3f\n", getStateValueC(env->ctrlrValueTree, env->controllerState[GRN][0], 0));
    sem_post(&ctrlrSem);
}

void printControllerState(Env * env)
{
    int status;
    int i, j;

    printf("GREEN:       |  RED\n");
    
    for (i=0; i<GRID_SIZE; i++){
        for (j=0; j<GRID_SIZE*2; j++){
            if (j == GRID_SIZE)
                printf(" |  ");
            if (j < GRID_SIZE)
                status = env->controllerState[GRN][0][2+(i*GRID_SIZE)+j];
            else
                status = env->controllerState[RED][0][2+(i*GRID_SIZE)+(j-GRID_SIZE)];
            switch (status){
                case UNEXPLORED         :
                    printf("?   ");
                    break;
                case EXPLORED_NORMAL    :
                    printf(".   ");
                    break;
                case END_ZONE           :
                    printf("E   ");
                    break;
                case 4                  :
                    printf(".1  ");
                    break;
                case 5                  :
                    printf(".2  ");
                    break;
                case 6                  :
                    printf(".12 ");
                    break;
                case 7                  :
                    printf("H1  ");
                    break;
                case 8                  :
                    printf("H2  ");
                    break;
                case 9                  :
                    printf("H12 ");
                    break;
                case 10                 :
                    printf("E1  ");
                    break;
                case 11                 :
                    printf("E2  ");
                    break;
                case 12                 :
                    printf("E12 ");
                    break;
            }
        }

        printf("\n");
    }

    printf("\n");
}

int hasEnemy(BlockStatus status)
{
    switch (status){
        case NORMAL_AGENT1_NOTHING  : case NORMAL_AGENT2_NOTHING    : case NORMAL_AGENTS_NOTHING :
        case HOME_AGENT1_NOTHING    : case HOME_AGENT2_NOTHING      : case HOME_AGENTS_NOTHING   :
        case END_AGENT1_NOTHING     : case END_AGENT2_NOTHING       : case END_AGENTS_NOTHING    :
            return 0;
            break;
        default :
            return 1;
            break;
    }

    return 0;
}
