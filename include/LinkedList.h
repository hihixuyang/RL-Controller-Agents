/*
                Assignment 3 Part 2
                 Memory Management
                   LinkedList.h
 
    Created by  :   Patrick Hartman
    For         :   CIS 3110
    Taught By   :   Dr. Xining Li
    On          :   03.27.2015
*/


#ifndef __LINKEDLIST_H_
#define __LINKEDLIST_H_

#include "Boolean.h"

struct ListNode
{
    void * value;
    struct ListNode * next;
};
typedef struct ListNode ListNode;

typedef struct
{
    ListNode * head;
    ListNode * current;
    Boolean (* compare)(void *, void *);
    void * (* destroy)(void *);
} LList;

LList * createList(Boolean (*)(void *, void *), void * (*)(void *));

int getListSize(LList *);

ListNode * createNode(void *);

void * getNodeValue(ListNode *);

void unsetNodeValue(ListNode *);

void destroyNode(ListNode *, void * (*)(void *));

void addToList(LList *, void *);

void addToFront(LList *, void *);

void * removeFront(LList *);

void * removeValue(LList *, void *);

void printList(LList *, void (*)(void *));

ListNode * getNext(LList * );

void resetCurrent(LList *);

Boolean valueExists(LList *, void *);

void * getValue(LList *, void *);

LList * destroyList(LList *);

#endif
