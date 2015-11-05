#include "agent.h"

void * agentFunction(void * arg)
{
    Team team = ((AgentStruct *)arg)->team;
    Identity id = ((AgentStruct *)arg)->id;
    Env * env = ((AgentStruct *)arg)->env;
    int episode [AGENT_STATE_MEMORY+1];
    int startGame;
    int i;

    startGame = 1;
    episode[0] = 0;

    while (!env->gameOver){
        /* set updated flag */
        env->agentRequest[team][id] = ACTION_UPDATED;

        if (!env->agentActive[team][id]){
            while (!env->agentActive[team][id] && !env->gameOver){
                /* waiting to become active */
            }
            startGame = 1;
        }

        while (env->agentRequest[team][id] != ACTION_REQUESTED && !env->gameOver){
            /* planning... */
        }

        if (env->gameOver)
            break;

        /* align reward and action vectors with goal vector */
        for (i=AGENT_MEMORY; i>0; i--)
            env->agentReward[team][id][i] = env->agentReward[team][id][i-1];
        for (i=AGENT_MEMORY; i>0; i--)
            env->agentAction[team][id][i] = env->agentAction[team][id][i-1];

        /* roll back episode vector */
        for (i=AGENT_STATE_MEMORY; i>0; i--)
            episode[i] = episode[i-1];
        /* update current episode */
        if (env->agentEpisodeReset[team][id])
            episode[0] = episode[0] >= AGENT_STATE_MEMORY-1 ? 0 : episode[0]+1;
        env->agentEpisodeReset[team][id] = 0;

        // update state values that received rewards
        if (!startGame)
            updateStates(env, team, id, episode);
        startGame = 0;

        if (env->agentActive[team][id]){
            /* determine next action ([0]) based on current state [0] and goal [0] */
            env->agentAction[team][id][0] = chooseAction(env, team, id);

            /* update state with action choice */
            env->agentState[team][id][0][0] = env->agentAction[team][id][0];
        }

        /* print agent stats */
        if (team == GRN && id == AG1 && env->showAgentStats){
            printAgentStates(env, team, id);
            printAgentStats(env, team, id, episode);
        }

        if (team == GRN && id == AG1){
            for (i=1; i<=AGENT_MEMORY; i++){
                if (env->agentReward[team][id][i] != 0){
                    env->pause = 1;
                    break;
                }
            }
        }

        /* roll back state matrix */
        rollAgentStates(env, team, id);

        /* roll back goal vector */
        for (i=AGENT_MEMORY; i>0; i--)
            env->agentGoal[team][id][i] = env->agentGoal[team][id][i-1];

        /* clear reward vector */
        for (i=0; i<=AGENT_MEMORY; i++)
            env->agentReward[team][id][i] = 0;
    }

    pthread_exit(NULL);
}

void copyState(int * state1, int * state2, int len)
{
    int i;

    for (i=0; i<len; i++)
        state1[i] = state2[i];
}

int statesEqual(int * state1, int * state2)
{
    int i;

    for (i=2; i<AGENT_STATE_ACTION_SIZE; i++)
        if (state1[i] != state2[i])
            return 0;

    return 1;
}

int positionChange(int * state1, int * state2)
{
    if ( (state1[3] == state2[3]) && (state1[4] == state2[4]) )
        return 0;
    else
        return 1;
}

void updateStates(Env * env, Team team, Identity id, int * episode)
{
    float value;
    float nextStateValue;
    float error;
    float eligibility;
    float Y             = 0.85;
    float learningRate  = 0.1;
    float updateValue;
    int state [AGENT_STATE_ACTION_SIZE];
    int i, j;

    sem_wait(&agentSem);

    for (i=AGENT_MEMORY; i>0; i--){
        if (env->agentReward[team][id][i] != 0 || (i == 1 && episode[i] == episode[i-1] && 
                                                   positionChange(env->agentState[team][id][i], env->agentState[team][id][i-1]))){
            copyState(state, env->agentState[team][id][i-1], AGENT_STATE_ACTION_SIZE);
            nextStateValue = -1000;
            for (j=0; j<NUM_AGENT_ACTIONS; j++){
                state[0] = j;
                value = getStateValue(env->agentValueTree, state, 0);
                if (value > nextStateValue)
                    nextStateValue = value;
            }

            error = (env->agentReward[team][id][i] + Y * nextStateValue) - getStateValue(env->agentValueTree, env->agentState[team][id][i], 0);

            if (error != 0){
                eligibility = 1;
                j = i;
                while ( (episode[j] == episode[i]) && (j < AGENT_STATE_MEMORY) ){
                    if (j == i || (env->agentState[team][id][j][0] <= 3 && 
                                   positionChange(env->agentState[team][id][j], env->agentState[team][id][j-1]))){
                        error = (env->agentReward[team][id][i] + Y * nextStateValue) - getStateValue(env->agentValueTree, env->agentState[team][id][j], 0);
                        updateValue = getStateValue(env->agentValueTree, env->agentState[team][id][j], 0) + learningRate * eligibility * error;
                        updateStateValue(env->agentValueTree, env->agentState[team][id][j], updateValue, 0, 0);
                    }
                    eligibility *= 0.85;
                    j++;
                }
            }
        }
    }

    sem_post(&agentSem);
}

AgentAction chooseAction(Env * env, Team team, Identity id)
{
    float maxValue=-1000, value;
    float values[NUM_AGENT_ACTIONS];
    int badAction[4];
    int maxAction=0, equalMax, i;

    for (i=0; i<NUM_AGENT_ACTIONS; i++)
        values[i] = -1000.0;
    for (i=0; i<4; i++)
        badAction[i] = -1;

    sem_wait(&agentSem);

    if (statesEqual(env->agentState[team][id][0], env->agentState[team][id][1]) && env->agentState[team][id][1][0] <= 3)
        badAction[0] = env->agentState[team][id][1][0];
    if (statesEqual(env->agentState[team][id][0], env->agentState[team][id][2]) && env->agentState[team][id][2][0] <= 3)
        badAction[1] = env->agentState[team][id][2][0];
    if (statesEqual(env->agentState[team][id][0], env->agentState[team][id][3]) && env->agentState[team][id][3][0] <= 3)
        badAction[2] = env->agentState[team][id][3][0];
    if (statesEqual(env->agentState[team][id][0], env->agentState[team][id][4]) && env->agentState[team][id][4][0] <= 3)
        badAction[3] = env->agentState[team][id][4][0];

    if (rand() % EPSILON_A == 0){
        if (team == GRN && id == AG1 && env->showActionStats)
            printf("Random Action selected\n\n");
        sem_post(&agentSem);
        return rand() % NUM_AGENT_ACTIONS;
    }

    if (team == GRN && id == AG1 && env->showActionStats)
        printf("Options:\n");

    for (i=0; i<NUM_AGENT_ACTIONS; i++){
        if (i == badAction[0] || i == badAction[1] || 
            i == badAction[2] || i == badAction[3]  )
            continue;

        env->agentState[team][id][0][0] = i;
        value = getStateValue(env->agentValueTree, env->agentState[team][id][0], 0);

        if (env->USE_NN && value == 0)
            value = predictStateValue(env->nnParams, env->agentState[team][id][0], team == GRN && id == AG1);

        values[i] = value;

        if (team == GRN && id == AG1 && env->showActionStats)
            printActionValue(i, value);

        if (value > maxValue){
            maxValue = value;
            maxAction = i;
        }
    }

    equalMax = 0;
    for (i=0; i<NUM_AGENT_ACTIONS; i++)
        if (values[i] == maxValue)
            equalMax++;

    /* if more than one action at maxValue, select action with equal likelihood ... */
    if (equalMax > 1){
        do{
            maxAction = rand() % NUM_AGENT_ACTIONS;
        } while (values[maxAction] < maxValue);
    }

    sem_post(&agentSem);

    if (team == GRN && id == AG1 && env->showActionStats){
        printf("\nChoice:  ");
        printActionValue(maxAction, maxValue);
        printf("\n\n");
    }

    return maxAction;
}

void rollAgentStates(Env * env, Team team, Identity id)
{
    int i, j;

    for (i=AGENT_STATE_MEMORY; i>0; i--)
        for (j=0; j<AGENT_STATE_ACTION_SIZE; j++)
            env->agentState[team][id][i][j] = env->agentState[team][id][i-1][j];
}

void printActionValue(AgentAction action, float value)
{
    switch(action){
        case MOVE_LEFT  :
            printf("ML:");
            break;
        case MOVE_RIGHT :
            printf("MR:");
            break;
        case MOVE_UP    :
            printf("MU:");
            break;
        case MOVE_DOWN  :
            printf("MD:");
            break;
        case FIRE_LEFT  :
            printf("FL:");
            break;
        case FIRE_RIGHT :
            printf("FR:");
            break;
        case FIRE_UP    :
            printf("FU:");
            break;
        case FIRE_DOWN  :
            printf("FD:");
            break;
    }

    printf("%-7.2f ", value);
}

void printAgentStates(Env * env, Team team, Identity id)
{
    int i, j, k;

    printf("goal: ");
    switch (env->agentState[team][id][0][1]){
        case ENGAGE     :
            printf("ENGAGE   ");
            break;
        case FINISH     :
            printf("FINISH   ");
            break;
        case GO_LEFT    :
            printf("GO_LEFT  ");
            break;
        case GO_RIGHT   :
            printf("GO_RIGHT ");
            break;
        case GO_UP      :
            printf("GO_UP    ");
            break;
        case GO_DOWN    :
            printf("GO_DOWN  ");
            break;
        case NO_ACTION  :
            printf("NO_ACTION");
            break;
    }

    printf("  action: ");
    switch(env->agentState[team][id][0][0]){
        case MOVE_LEFT  :
            printf("MOVE_LEFT ");
            break;
        case MOVE_RIGHT :
            printf("MOVE_RIGHT");
            break;
        case MOVE_UP    :
            printf("MOVE_UP   ");
            break;
        case MOVE_DOWN  :
            printf("MOVE_DOWN ");
            break;
        case FIRE_LEFT  :
            printf("FIRE_LEFT ");
            break;
        case FIRE_RIGHT :
            printf("FIRE_RIGHT");
            break;
        case FIRE_UP    :
            printf("FIRE_UP   ");
            break;
        case FIRE_DOWN  :
            printf("FIRE_DOWN ");
            break;
    }

    printf("\nblock type: ");
    if      (env->agentState[team][id][0][2] == 0) printf("NORMAL\n");
    else if (env->agentState[team][id][0][2] == 1) printf("END_ZONE\n");

    printf("row: %d, col: %d\n", env->agentState[team][id][0][3], env->agentState[team][id][0][4]);

    k = 5;
    for (i=0; i<3; i++){
        for (j=0; j<3; j++){
            printf("q%d: ", k-4);
            switch (env->agentState[team][id][0][k]){
                case    0   :
                    printf(" . ");
                    break;
                case    1   :
                    printf("X  ");
                    break;
                case    2   :
                    printf("F  ");
                    break;
                case    3   :
                    printf("FX ");
                    break;
            }
            printf("\t");
            k++;
        }
        printf("\n");
    }

    for (i=0; i<5; i++){
        for (j=0; j<5; j++){
            switch(env->agentState[team][id][0][k]){
                case EMPTY          :
                    printf(". ");
                    break;
                case SELF           :
                    printf("S ");
                    break;
                case FRIENDLY       :
                    printf("F ");
                    break;
                case ENEMY          :
                    printf("X ");
                    break;
                case BULLET_LEFT    :
                    printf("< ");
                    break;
                case BULLET_RIGHT   :
                    printf("> ");
                    break;
                case BULLET_UP      :
                    printf("^ ");
                    break;
                case BULLET_DOWN    :
                    printf("v ");
                    break;
                default             :
                    printf("%d ", env->agentState[team][id][0][k]);
                    exit(1);
                    break;
            }
            k++;
        }
        printf("\n");
    }
    printf("\n");
}

void printAgentStats(Env * env, Team team, Identity id, int * episode)
{
    int i;

    for (i=AGENT_STATE_MEMORY; i>=0; i--){
        printf("E: %-2d ", episode[i]);
        printf("Goal at t-%-2d:  ", i);
        switch (env->agentState[team][id][i][1]){
            case ENGAGE     :
                printf("ENGAGE   ");
                break;
            case FINISH     :
                printf("FINISH   ");
                break;
            case GO_LEFT    :
                printf("GO_LEFT  ");
                break;
            case GO_RIGHT   :
                printf("GO_RIGHT ");
                break;
            case GO_UP      :
                printf("GO_UP    ");
                break;
            case GO_DOWN    :
                printf("GO_DOWN  ");
                break;
            case NO_ACTION  :
                printf("NO_ACTION");
                break;
        }

        printf("  Action: ");
        switch(env->agentState[team][id][i][0]){
            case MOVE_LEFT  :
                printf("MOVE_LEFT ");
                break;
            case MOVE_RIGHT :
                printf("MOVE_RIGHT");
                break;
            case MOVE_UP    :
                printf("MOVE_UP   ");
                break;
            case MOVE_DOWN  :
                printf("MOVE_DOWN ");
                break;
            case FIRE_LEFT  :
                printf("FIRE_LEFT ");
                break;
            case FIRE_RIGHT :
                printf("FIRE_RIGHT");
                break;
            case FIRE_UP    :
                printf("FIRE_UP   ");
                break;
            case FIRE_DOWN  :
                printf("FIRE_DOWN ");
                break;
        }

        if (i > 0 && i <= AGENT_MEMORY)
            printf("  r: %d", env->agentReward[GRN][AG1][i]);
        
        printf("\n");
    }

    printf("\n\n");
    fflush(stdout);
}
