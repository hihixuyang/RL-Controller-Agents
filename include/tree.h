#ifndef __TREE_HEADER_
#define __TREE_HEADER_

#include <stdlib.h>
#include "env.h"

struct Tree{
    int numBranches;
    float value;
    struct Tree ** branch;
};

typedef struct Tree Tree;

Tree * createTree();

void updateStateValue (Tree *, int *, float, int, int);
void updateStateValueC(Tree *, int *, float, int, int);

float getStateValue (Tree *, int *, int);
float getStateValueC(Tree *, int *, int);

Tree * destroyTree (Tree *, int);
Tree * destroyTreeC(Tree *, int);

void exportTree (Tree *, FILE *);
void exportTreeC(Tree *, FILE *);

void exportTreeR (Tree *, int *, int, FILE*);
void exportTreeRC(Tree *, int *, int, FILE*);

void importTree (Tree *, FILE *);
void importTreeC(Tree *, FILE *);

#endif
