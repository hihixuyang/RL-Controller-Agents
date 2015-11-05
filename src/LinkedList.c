/*
                Assignment 3 Part 2
                 Memory Management
                   LinkedList.c
 
    Created by  :   Patrick Hartman
    For         :   CIS 3110
    Taught By   :   Dr. Xining Li
    On          :   03.27.2015
*/


#include <stdio.h>
#include <stdlib.h>
#include "Boolean.h"
#include "LinkedList.h"


/*  Description         : Initializes a new list object.
 *  Preconditions       : compareF and destroyF point to valid compare()
 *                        and destroy() functions respectively
 *  Postconditions      : Returns an empty LList structure */
LList * createList(Boolean(* compareF)(void *, void *),
                   void * (* destroyF)(void *))
{
    LList * theList;

    if (!(theList = malloc(sizeof(LList))))
        return NULL;

    if (!(theList->head = malloc(sizeof(ListNode)))){
        free(theList);
        return NULL;
    }
    theList->head->value = NULL;
    theList->head->next = NULL;

    theList->current = theList->head;

    theList->compare = compareF;
    theList->destroy = destroyF;

    return theList;
}

int getListSize(LList * theList)
{
    ListNode * node;
    int count = 0;

    if (theList == NULL || theList->head == NULL)
        return 0;

    node = theList->head;

    while (node->next != NULL)
        count++;

    return count;
}

/*  Description         : Initializes a new ListNode object for use in a list .
 *  Preconditions       : newValue is of the type being maintained by the list
 *                            and able to be used with the create and destroy
 *                            functions 
 *  Postconditions           : Returns a new ListNode structure */
ListNode * createNode(void * newValue)
{
    ListNode * newNode;

    if (!(newNode = malloc(sizeof(ListNode)))){
        perror("Error allocating memory!\n");
        exit(1);
    }
    newNode->value = newValue;
    newNode->next = NULL;

    return newNode;
}

void * getNodeValue(ListNode * node)
{
    return node->value;
}

void unsetNodeValue(ListNode * node)
{
    node->value = NULL;
}

/*  Description                : Destroys a ListNode object's value and frees the
 *                            node itself.
 *  Preconditions        : node is a valid and initialized ListNode structure.
 *                            destroy is a valid destroy function for use with the
 *                            currently-managed ListNode's value type.
 *  Postconditions        : All memory related to the node has been released. */
void destroyNode(ListNode * node, void * (* destroy)(void *))
{
    if (node != NULL){
        if (node->value != NULL)
            destroy(node->value);
        free(node);
    }
}

/*  Description                : Creates a new node initialied to newValue and adds it
 *                            to the tail of the list.
 *  Preconditions        : theList and newValue are valid. newValue is of the
 *                            type currently handled by the list's compare and
 *                            destroy functions.
 *  Postconditions        : A new node with value newValue is added to the list.
 */
void addToList(LList * theList, void * newValue)
{
    ListNode * position;

    if (theList == NULL || theList->head == NULL || newValue == NULL)
        return;

    position = theList->head;
    while (position->next != NULL)
        position = position->next;

    position->next = createNode(newValue);
}

void addToFront(LList * theList, void * newValue)
{
    ListNode * temp;

    temp = theList->head->next;

    theList->head->next = createNode(newValue);

    theList->head->next->next = temp;
}

void * removeFront(LList * theList)
{
    ListNode * node;
    void * value;

    if (theList == NULL || theList->head == NULL || theList->head->next == NULL)
        return NULL;

    node = theList->head->next;

    theList->head->next = theList->head->next->next;

    value = getNodeValue(node);

    unsetNodeValue(node);

    destroyNode(node, theList->destroy);

    return value;
}

void * removeValue(LList * theList, void * value)
{
    ListNode * node;
    ListNode * prev;

    if (theList == NULL || theList->head == NULL || value == NULL)
        return NULL;

    node = theList->head;

    while (node->next != NULL){
        prev = node;
        node = node->next;

        if (theList->compare(node->value, value)){
            prev->next = node->next;
            destroyNode(node, theList->destroy);
        }
    }

    return NULL;
}

void printList(LList * theList, void (* printFunction)(void *))
{
    ListNode * node;
    void * value;

    if (theList == NULL || theList-> head == NULL)
        return;

    while ((node = getNext(theList)) != NULL){
        value = node->value;
        printFunction(value);
    }

    resetCurrent(theList);
}

/*  Description                : Returns the item in the list after the item returned
 *                            by the previous call to getNext() (assuming 
 *                            resetCurrent() hasn't been called in between).
 *  Preconditions        : theList is valid, initialized.
 *  Postconditions        : Returns the next item in the list. The 'current'
 *                            iterator points to the next item in the list. Returns
 *                            NULL if that is the end of the list. */
ListNode * getNext(LList * theList)
{
    if (theList->current->next != NULL){
        return (theList->current = theList->current->next);
/*        theList->current = theList->current->next;
        return theList->current;*/
    }
    else
        return NULL;
}

/*  Description                : Resets the 'current' iterator to the head of the 
 *                            list.
 *  Preconditions        : theList is valid, initialized.
 *  Postconditions        : current = head. */
void resetCurrent(LList * theList)
{
    theList->current = theList->head;
}

/*  Description                : Calls the list's compare function to see whether a
 *                            certain value already exists in the list.
 *  Preconditions        : theList is a valid, initialized list. value is of the
 *                            type currently being managed by the list.
 *  Postconditions        : Returns true if the value is found, false otherwise. 
 */
Boolean valueExists(LList * theList, void * value)
{
    ListNode * node;

    if (theList == NULL)
        return false; 
    if (theList->head == NULL)
        return false;

    node = theList->head;

    while (node->next != NULL){
        node = node->next;
        if (theList->compare(node->value, value))
            return true;
    }

    return false;
}

/*  Description                : Uses the list's compare function to return a value
 *                            matching the value sent by comparison.
 *  Preconditions        : comparison is of the type able to be used by the
 *                            list's compare function and theList is valid and
 *                            initialized.
 *  Postconditions        : The value of the list item is returned if the
 *                            comparison is true. Otherwise, NULL is returned. */
void * getValue(LList * theList, void * comparison)
{
    ListNode * node = theList->head;

    while (node->next != NULL){
        node = node->next;
        if (theList->compare(node->value, comparison))
            return node->value;
    }

    return NULL;
}

/*  Description                : Frees up memory maintained by the list object.
 *  Preconditions        : theList is NULL or a valid, initialized list object.
 *  Postconditions        : The list's destroy function is called on the value
 *                            of each node in the list and memory is freed from
 *                            each node, then the list itself. */
LList * destroyList(LList * theList)
{
    ListNode * node;
    ListNode * prev;

    if (theList == NULL || theList->head == NULL)
        return NULL;

    node = theList->head;

    while (node->next != NULL){
        prev = node;
        node = node->next;
        destroyNode(prev, theList->destroy);
    }

    destroyNode(node, theList->destroy);

    free(theList);

    return NULL;
}
