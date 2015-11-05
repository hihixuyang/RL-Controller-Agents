#include "reward.h"

void setMovementReward(Env * env, Team team, Identity id, int rowD, int colD)
{
    switch (env->agentGoal[team][id][1]){
        case ENGAGE     :
        case FINISH     :
            env->agentReward[team][id][0] += GOAL_NOT_MET;
            break;
        case GO_LEFT    :
            if (colD == -1)
                env->agentReward[team][id][0] += GOAL_MET;
            else
                env->agentReward[team][id][0] += GOAL_NOT_MET;
            break;
        case GO_RIGHT   :
            if (colD == 1)
                env->agentReward[team][id][0] += GOAL_MET;
            else
                env->agentReward[team][id][0] += GOAL_NOT_MET;
            break;
        case GO_UP      :
            if (rowD == -1)
                env->agentReward[team][id][0] += GOAL_MET;
            else
                env->agentReward[team][id][0] += GOAL_NOT_MET;
            break;
        case GO_DOWN    :
            if (rowD == 1)
                env->agentReward[team][id][0] += GOAL_MET;
            else
                env->agentReward[team][id][0] += GOAL_NOT_MET;
            break;
        case NO_ACTION  :
            break;
    }
}

void setDeathReward(Env * env, Team team, Identity id)
{
    env->agentReward[team][id][0] += GOAL_NOT_MET;
}

/* use when relative standing of killer and killed are not known */
void setKillReward(Env * env, Team team, Identity id, int blockRow, int blockCol, int row, int col, int * friendlyFire)
{
    Bullet hypotheticalBullet;
    Bullet * shotBullet;
    int fairShot;
    int i;

    setMinBulletValues(&hypotheticalBullet, blockRow, blockCol, row, col);
    shotBullet = getValue(env->bullets, &hypotheticalBullet);

    if (shotBullet->teamOwner == team){
        *friendlyFire = 1;

        /* punish shot only if friendly is present in block at time of firing */
        fairShot = 0;
        for (i=5; i<14; i++){
            if (env->agentState[shotBullet->teamOwner][shotBullet->agentOwner][shotBullet->age+1][i] == 2 ||
                env->agentState[shotBullet->teamOwner][shotBullet->agentOwner][shotBullet->age+1][i] == 3 ){
                fairShot = 1;
                break;
            }
        }

        if (fairShot)
            env->agentReward[shotBullet->teamOwner][shotBullet->agentOwner][shotBullet->age] += DEATH;
    }
    else{
        *friendlyFire = 0;

        /* reward shot only if enemy is present in block at time of firing */
        fairShot = 0;
        for (i=5; i<14; i++){
            if (env->agentState[shotBullet->teamOwner][shotBullet->agentOwner][shotBullet->age+1][i] == 1 ||
                env->agentState[shotBullet->teamOwner][shotBullet->agentOwner][shotBullet->age+1][i] == 3 ){
                fairShot = 1;
                break;
            }
        }

        if (fairShot){
            if (env->agentGoal[shotBullet->teamOwner][shotBullet->agentOwner][shotBullet->age+1] == ENGAGE)
                env->agentReward[shotBullet->teamOwner][shotBullet->agentOwner][shotBullet->age] += KILL + GOAL_MET;
            else
                env->agentReward[shotBullet->teamOwner][shotBullet->agentOwner][shotBullet->age] += KILL;
        }
    }
}

/* use when relative standing of killer and killed are known */
void setKillReward2(Env * env, Team team, Identity id, int friendlyFire, int bulletAge)
{
    int fairShot, i;

    if (friendlyFire){
        /* punish shot only if friendly is present in block at time of firing */
        fairShot = 0;
        for (i=5; i<14; i++){
            if (env->agentState[team][id][bulletAge+1][i] == 2 ||
                env->agentState[team][id][bulletAge+1][i] == 3 ){
                fairShot = 1;
                break;
            }
        }

        if (fairShot)
            env->agentReward[team][id][bulletAge] += DEATH;
    }
    else{
        /* reward shot only if enemy is present in block at time of firing */
        fairShot = 0;
        for (i=5; i<14; i++){
            if (env->agentState[team][id][bulletAge+1][i] == 1 ||
                env->agentState[team][id][bulletAge+1][i] == 3 ){
                fairShot = 1;
                break;
            }
        }

        if (fairShot){
            if (env->agentGoal[team][id][bulletAge+1] == ENGAGE)
                env->agentReward[team][id][bulletAge] += KILL + GOAL_MET;
            else
                env->agentReward[team][id][bulletAge] += KILL;
        }
    }
}

void setFinishReward(Env * env, Team team, Identity id)
{
    if (env->agentGoal[team][id][1] == FINISH)
        env->agentReward[team][id][0] += AGENT_FINISH + GOAL_MET;
    else
        env->agentReward[team][id][0] += AGENT_FINISH + GOAL_NOT_MET;
}
