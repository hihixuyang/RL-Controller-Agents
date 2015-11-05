#include "tree.h"

Tree * createTree()
{
    Tree * tree;

    if (!(tree = malloc(sizeof(Tree)))){
        perror("Error allocating memory!\n");
        exit(1);
    }

    tree->value = 0;
    tree->numBranches = 0;
    tree->branch = NULL;

    return tree;
}

void updateStateValue(Tree * tree, int * state, float value, int level, int numUpdates)
{
    int i;

    if (level == AGENT_STATE_ACTION_SIZE){
        tree->value = value;
        if (numUpdates == 0)
            tree->numBranches++;
        else
            tree->numBranches = numUpdates;
        return;
    }

    if (level == 0)
        tree->value++;

    if (tree->numBranches == 0){
        if (level == 0)
            tree->numBranches = NUM_AGENT_ACTIONS;
        else if (level == 1)
            tree->numBranches = NUM_AGENT_GOALS;
        else if (level == 2)
            tree->numBranches = NUM_AGENT_BLOCK_TYPES;
        else if (level == 3 || level == 4)
            tree->numBranches = NUM_AGENT_POSITIONS;

        if (level > 4)
            tree->numBranches = NUM_AGENT_QUADRANT_STATES;
        if (level > 13)
            tree->numBranches = NUM_AGENT_CELL_STATES;

        if (!(tree->branch = malloc(sizeof(Tree *) * tree->numBranches))){
            fprintf(stderr, "Unable to allocate memory!\n"); fflush(stderr);
            exit(1);
        }

        for (i=0; i<tree->numBranches; i++)
            tree->branch[i] = NULL;
    }

    if (tree->branch[state[level]] == NULL)
        tree->branch[state[level]] = createTree();
    
    updateStateValue(tree->branch[state[level]], state, value, level+1, numUpdates);
}

void updateStateValueC(Tree * tree, int * state, float value, int level, int numUpdates)
{
    int i;

    if (level == CTRLR_STATE_ACTION_SIZE){
        tree->value = value;
        if (numUpdates == 0)
            tree->numBranches++;
        else
            tree->numBranches = numUpdates;
        return;
    }

    if (level == 0)
        tree->value++;

    if (tree->numBranches == 0){
        if (level == 0)
            tree->numBranches = NUM_CTRLR_ACTIONS;
        else
            tree->numBranches = NUM_CTRLR_BLOCK_STATUS;

        if (!(tree->branch = malloc(sizeof(Tree *) * tree->numBranches))){
            fprintf(stderr, "Unable to allocate memory!\n"); fflush(stderr);
            exit(1);
        }

        for (i=0; i<tree->numBranches; i++)
            tree->branch[i] = NULL;
    }

    if (tree->branch[state[level]] == NULL)
        tree->branch[state[level]] = createTree();
    
    updateStateValueC(tree->branch[state[level]], state, value, level+1, numUpdates);
}

float getStateValue(Tree * tree, int * state, int level)
{
    if (tree == NULL)
        return 0;

    if (level == AGENT_STATE_ACTION_SIZE)
        return tree->value;

    if (tree->numBranches == 0 || tree->branch[state[level]] == NULL)
        return 0;

    return getStateValue(tree->branch[state[level]], state, level+1);
}

float getStateValueC(Tree * tree, int * state, int level)
{
    if (tree == NULL)
        return 0;

    if (level == CTRLR_STATE_ACTION_SIZE)
        return tree->value;

    if (tree->numBranches == 0 || tree->branch[state[level]] == NULL)
        return 0;

    return getStateValueC(tree->branch[state[level]], state, level+1);
}

Tree * destroyTree(Tree * tree, int level)
{
    int i;

    if (level == AGENT_STATE_ACTION_SIZE){
        free(tree);
        return NULL;
    }

    if (tree->numBranches > 0)
        for (i=0; i<tree->numBranches; i++)
            if (tree->branch[i])
                destroyTree(tree->branch[i], level+1);

    free(tree);
    return NULL;
}

Tree * destroyTreeC(Tree * tree, int level)
{
    int i;

    if (level == CTRLR_STATE_ACTION_SIZE){
        free(tree);
        return NULL;
    }

    if (tree->numBranches > 0)
        for (i=0; i<tree->numBranches; i++)
            if (tree->branch[i])
                destroyTreeC(tree->branch[i], level+1);

    free(tree);
    return NULL;
}

void exportTree(Tree * tree, FILE * fd)
{
    int state [AGENT_STATE_ACTION_SIZE];

    exportTreeR(tree, state, 0, fd);
}

void exportTreeC(Tree * tree, FILE * fd)
{
    int state [CTRLR_STATE_ACTION_SIZE];

    exportTreeRC(tree, state, 0, fd);
}

void exportTreeR(Tree * tree, int * state, int level, FILE * fd)
{
    int i;

    if (level == AGENT_STATE_ACTION_SIZE){
        for (i=0; i<AGENT_STATE_ACTION_SIZE; i++)
            fprintf(fd, "%d,", state[i]);
        fprintf(fd, "%f,", tree->value);
        fprintf(fd, "%d\n", tree->numBranches);
        return;
    }

    for (i=0; i<tree->numBranches; i++){
        if (tree->branch[i] != NULL){
            state[level] = i;
            exportTreeR(tree->branch[i], state, level+1, fd);
        }
    }
}

void exportTreeRC(Tree * tree, int * state, int level, FILE * fd)
{
    int i;

    if (level == CTRLR_STATE_ACTION_SIZE){
        for (i=0; i<CTRLR_STATE_ACTION_SIZE; i++)
            fprintf(fd, "%d,", state[i]);
        fprintf(fd, "%f,", tree->value);
        fprintf(fd, "%d\n", tree->numBranches);
        return;
    }

    for (i=0; i<tree->numBranches; i++){
        if (tree->branch[i] != NULL){
            state[level] = i;
            exportTreeRC(tree->branch[i], state, level+1, fd);
        }
    }
}

void importTree(Tree * tree, FILE * fd)
{
    char line [512];
    int state [AGENT_STATE_ACTION_SIZE];
    float value;
    int numUpdates;
    int i, j;

    while (fgets(line, 512, fd)){
        for (i=0, j=0; i<AGENT_STATE_ACTION_SIZE; i++){
            state[i] = atoi(&line[j]);
            while (line[j] != ',') j++;
            j++;
        }
        value = atof(&line[j]);
        while (line[j] != ',') j++;
        j++;
        numUpdates = atoi(&line[j]);
        updateStateValue(tree, state, value, 0, numUpdates);
    }
}

void importTreeC(Tree * tree, FILE * fd)
{
    char line [512];
    int state [CTRLR_STATE_ACTION_SIZE];
    float value;
    int numUpdates;
    int i, j;

    while (fgets(line, 512, fd)){
        for (i=0, j=0; i<CTRLR_STATE_ACTION_SIZE; i++){
            state[i] = atoi(&line[j]);
            while (line[j] != ',') j++;
            j++;
        }
        value = atof(&line[j]);
        while (line[j] != ',') j++;
        j++;
        numUpdates = atoi(&line[j]);
        updateStateValueC(tree, state, value, 0, numUpdates);
    }
}
