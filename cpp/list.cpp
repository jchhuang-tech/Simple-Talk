#include "list.h"
#include <stdio.h>
#include <assert.h>

static Node nodePool[LIST_MAX_NUM_NODES];
static List listPool[LIST_MAX_NUM_HEADS];

static bool listCreateCalledOnce = 0;
static int freeNodes[LIST_MAX_NUM_NODES];
static int freeLists[LIST_MAX_NUM_HEADS];
static int numberOfFreeNodes = LIST_MAX_NUM_NODES;
static int numberOfFreeLists = LIST_MAX_NUM_HEADS;



// General Error Handling:
// Client code is assumed never to call these functions with a NULL List pointer, or 
// bad List pointer. If it does, any behaviour is permitted (such as crashing).
// HINT: Use assert(pList != NULL); just to add a nice check, but not required.

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.
List* List_create()
{
    if (listCreateCalledOnce == 0) {//List_create gets called the first time
        for (int i = 0; i < LIST_MAX_NUM_NODES; i++) {
            nodePool[i].item = NULL;
            nodePool[i].next = NULL;
            nodePool[i].prev = NULL;
            nodePool[i].nodeID = i;

            freeNodes[i] = i;//record the nodeID with the array freeNodes[]
        }
        for (int j = 0; j < LIST_MAX_NUM_HEADS; j++) {
            listPool[j].size = 0;
            listPool[j].cur = NULL;
            listPool[j].first = NULL;
            listPool[j].last = NULL;
            listPool[j].curPos = 0;
            listPool[j].listID = j;

            freeLists[j] = j;//record the listID with the array freeLists[]
        }
        //for (int k = 0; k < LIST_MAX_NUM_NODES; k++) {
        //}
        //for (int l = 0; l < LIST_MAX_NUM_HEADS; l++) {
        //}


        List* newList = &listPool[freeLists[numberOfFreeLists - 1]];
        freeLists[numberOfFreeLists - 1] = '\0';//remove the listID from freeLists
        numberOfFreeLists--;
        listCreateCalledOnce = 1;
        return newList;
    }
    else if (numberOfFreeLists > 0) {//not the first time called, and there are free nodes left
        List* newList = &listPool[freeLists[numberOfFreeLists - 1]];
        freeLists[numberOfFreeLists - 1] = '\0';
        numberOfFreeLists--;
        return newList;
    }
    else {//no free nodes left, failure
        return NULL;
    }
}

// Returns the number of items in pList.
int List_count(List* pList)
{
    assert(pList != NULL);
    return pList->size;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList)
{
    assert(pList != NULL);
    if (pList->size == 0) {//plist is empty
        pList->cur = NULL;
        pList->curPos = 0;
        return NULL;
    }
    else {
        pList->cur = pList->first;
        pList->curPos = 0;
        return pList->first->item;
    }
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList)
{
    assert(pList != NULL);
    if (pList->size == 0) {//plist is empty
        pList->cur = NULL;
        pList->curPos = 0;
        return NULL;
    }
    else {
        pList->cur = pList->last;
        pList->curPos = 0;
        return pList->last->item;
    }
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList)
{
    assert(pList != NULL);
    if (pList->cur == NULL) {
        if (pList->size == 0) {//plist is empty
            return NULL;
        }
        else if (pList->curPos == 1) {//cur is before the beginning
            pList->cur = pList->first;
            pList->curPos = 0;
            return pList->cur->item;
        }
        else if (pList->curPos == 2) {//cur is beyond the end
            return NULL;
        }
    }
    else {
        pList->cur = pList->cur->next;
        if (pList->cur == NULL) {//cur is beyond the end
            pList->curPos = 2;
            return NULL;
        }
        else {
            return pList->cur->item;
        }
    }
    return NULL;
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList)
{
    assert(pList != NULL);
    if (pList->cur == NULL) {
        if (pList->size == 0) {//plist is empty
            return NULL;
        }
        else if (pList->curPos == 1) {//cur is before the beginning
            return NULL;
        }
        else if (pList->curPos == 2) {//cur is beyond the end
            pList->cur = pList->last;
            pList->curPos = 0;
            return pList->cur->item;
        }
    }
    else {
        pList->cur = pList->cur->prev;
        if (pList->cur == NULL) {//cur is before the start
            pList->curPos = 1;
            return NULL;
        }
        else {
            return pList->cur->item;
        }
    }
    return NULL;
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList)
{
    assert(pList != NULL);
    if (pList->cur == NULL) {//plist is empty
        return NULL;
    }
    else {
        return pList->cur->item;
    }
}

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_add(List* pList, void* pItem) 
{
    assert(pList != NULL);
    if (numberOfFreeNodes <= 0) {//no free nodes left, failure
        return -1;
    }
    else {//there are free nodes left
        Node* newNode = &nodePool[freeNodes[numberOfFreeNodes - 1]];//find a free node from nodePool
        freeNodes[numberOfFreeNodes - 1] = '\0';//remove the nodeID from freeNodes
        numberOfFreeNodes--;
        newNode->item = pItem;
        newNode->next = NULL;
        newNode->prev = NULL;

        if (pList->size == 0) {
            pList->first = newNode;
            pList->last = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        else if (pList->curPos == 1) {//current item is before the beginning
            pList->first->prev = newNode;
            newNode->next = pList->first;
            pList->first = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        else if (pList->curPos == 2) {
            pList->last->next = newNode;
            newNode->prev = pList->last;
            pList->last = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        else if (pList->curPos == 0) {
            if (pList->cur == pList->last) {
                pList->cur->next = newNode;
                newNode->prev = pList->cur;
                pList->last = newNode;
                pList->cur = newNode;
                pList->curPos = 0;
                pList->size++;
            }
            else {
                Node* curNext = pList->cur->next;
                pList->cur->next = newNode;
                curNext->prev = newNode;
                newNode->prev = pList->cur;
                newNode->next = curNext;
                pList->cur = newNode;
                pList->curPos = 0;
                pList->size++;
            }
        }
        return 0;
    }
}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert(List* pList, void* pItem) 
{
    assert(pList != NULL);
    if (numberOfFreeNodes <= 0) {//no free nodes left, failure
        return -1;
    }
    else {
        Node* newNode = &nodePool[freeNodes[numberOfFreeNodes - 1]];
        freeNodes[numberOfFreeNodes - 1] = '\0';
        numberOfFreeNodes--;
        newNode->item = pItem;
        newNode->next = NULL;
        newNode->prev = NULL;

        if (pList->size == 0) {
            pList->first = newNode;
            pList->last = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        else if (pList->curPos == 1) {//current item is before the beginning
            pList->first->prev = newNode;
            newNode->next = pList->first;
            pList->first = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        else if (pList->curPos == 2) {
            pList->last->next = newNode;
            newNode->prev = pList->last;
            pList->last = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        else if (pList->curPos == 0) {
            if (pList->cur == pList->first) {
                pList->cur->prev = newNode;
                newNode->next = pList->cur;
                pList->first = newNode;
                pList->cur = newNode;
                pList->curPos = 0;
                pList->size++;
            }
            else {
                Node* curPrev = pList->cur->prev;
                pList->cur->prev = newNode;
                curPrev->next = newNode;
                newNode->next = pList->cur;
                newNode->prev = curPrev;
                pList->cur = newNode;
                pList->curPos = 0;
                pList->size++;
            }
        }
        return 0;
    }
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) 
{
    assert(pList != NULL);
    if (numberOfFreeNodes <= 0) {//no free nodes left, failure
        return -1;
    }
    else {
        Node* newNode = &nodePool[freeNodes[numberOfFreeNodes - 1]];
        freeNodes[numberOfFreeNodes - 1] = '\0';
        numberOfFreeNodes--;
        newNode->item = pItem;
        newNode->next = NULL;
        newNode->prev = NULL;

        if (pList->size <= 0) {
            pList->first = newNode;
            pList->last = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        else {
            pList->last->next = newNode;
            newNode->prev = pList->last;
            pList->last = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        return 0;
    }
}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem)
{
    assert(pList != NULL);
    if (numberOfFreeNodes <= 0) {//no free nodes left, failure
        return -1;
    }
    else {
        Node* newNode = &nodePool[freeNodes[numberOfFreeNodes - 1]];
        freeNodes[numberOfFreeNodes - 1] = '\0';
        numberOfFreeNodes--;
        newNode->item = pItem;
        newNode->next = NULL;
        newNode->prev = NULL;

        if (pList->size <= 0) {
            pList->first = newNode;
            pList->last = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        else {
            pList->first->prev = newNode;
            newNode->next = pList->first;
            pList->first = newNode;
            pList->cur = newNode;
            pList->curPos = 0;
            pList->size++;
        }
        return 0;
    }
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList)
{
    assert(pList != NULL);
    if (pList->cur == NULL) {
        return NULL;
    }
    else if (pList->first == pList->last) {//plist ahs one item
        Node* current = pList->cur;

        pList->first = NULL;
        pList->last = NULL;
        pList->cur = NULL;
        pList->size--;
        pList->curPos = 0;

        void* ret = current->item;
        current->item = NULL;
        current->next = NULL;
        current->prev = NULL;
        freeNodes[numberOfFreeNodes] = current->nodeID;
        numberOfFreeNodes++;
        return ret;
    }
    else if (pList->cur == pList->first) {
        Node* current = pList->cur;

        pList->first->next->prev = NULL;
        pList->first = pList->first->next;
        pList->cur = pList->first;
        pList->size--;
        pList->curPos = 0;

        void* ret = current->item;
        current->item = NULL;
        current->next = NULL;
        current->prev = NULL;
        freeNodes[numberOfFreeNodes] = current->nodeID;
        numberOfFreeNodes++;
        return ret;
    }
    else if (pList->cur == pList->last) {
        Node* current = pList->cur;

        pList->last->prev->next = NULL;
        pList->last = pList->last->prev;
        pList->cur = NULL;
        pList->size--;
        pList->curPos = 2;

        void* ret = current->item;
        current->item = NULL;
        current->next = NULL;
        current->prev = NULL;
        freeNodes[numberOfFreeNodes] = current->nodeID;
        numberOfFreeNodes++;
        return ret;
    }
    else {
        Node* current = pList->cur;

        pList->cur->prev->next = pList->cur->next;
        pList->cur->next->prev = pList->cur->prev;
        pList->cur = pList->cur->next;
        pList->size--;
        pList->curPos = 0;

        void* ret = current->item;
        current->item = NULL;
        current->next = NULL;
        current->prev = NULL;
        freeNodes[numberOfFreeNodes] = current->nodeID;
        numberOfFreeNodes++;
        return ret;
    }
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2)
{
    assert(pList1 != NULL);
    assert(pList2 != NULL);
    if (pList2->size == 0) {//plist2 is empty
        freeLists[numberOfFreeLists] = pList2->listID;
        numberOfFreeLists++;
    }
    else if (pList1->size == 0) {//pList1 is empty and pList2 is not empty
        pList1->size = pList2->size;
        pList1->cur = NULL;
        pList1->curPos = 1;
        pList1->first = pList2->first;
        pList1->last = pList2->last;

        pList2->first = NULL;
        pList2->last = NULL;
        pList2->size = 0;
        pList2->cur = NULL;
        pList2->curPos = 0;

        freeLists[numberOfFreeLists] = pList2->listID;
        numberOfFreeLists++;
    }
    else{//plist1 and plist2 are both not empty
        pList1->last->next = pList2->first;
        pList2->first->prev = pList1->last;
        pList1->size += pList2->size;
        pList1->last = pList2->last;

        pList2->first = NULL;
        pList2->last = NULL;
        pList2->size = 0;
        pList2->cur = NULL;
        pList2->curPos = 0;
        
        freeLists[numberOfFreeLists] = pList2->listID;
        numberOfFreeLists++;
    }
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
// UPDATED: Changed function pointer type, May 19
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn)
{
    assert(pList != NULL);
    if (pList->size == 0) {//plist is empty
        freeLists[numberOfFreeLists] = pList->listID;
        numberOfFreeLists++;
    }
    else {
        Node* nodeToBeFreed = pList->first;

        while (nodeToBeFreed != NULL) {//has not gone through the whole list
            pList->first = nodeToBeFreed->next;
            (*pItemFreeFn)(nodeToBeFreed->item);
            nodeToBeFreed->next = NULL;
            nodeToBeFreed->prev = NULL;
            freeNodes[numberOfFreeNodes] = nodeToBeFreed->nodeID;
            numberOfFreeNodes++;
            nodeToBeFreed = pList->first;
        }

        pList->cur = 0;
        pList->size = 0;
        pList->first = NULL;
        pList->last = NULL;
        pList->curPos = 0;
        freeLists[numberOfFreeLists] = pList->listID;
        numberOfFreeLists++;
    }
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList)
{
    assert(pList != NULL);
    if (pList->size <= 0) {//plist is empty
        return NULL;
    }
    else if (pList->size == 1) {//plist has one item
        Node* lastItem = pList->last;

        pList->first = NULL;
        pList->last = NULL;
        pList->size--;
        pList->cur = NULL;
        pList->curPos = 0;

        void* ret = lastItem->item;
        lastItem->item = NULL;
        lastItem->next = NULL;
        lastItem->prev = NULL;
        freeNodes[numberOfFreeNodes] = lastItem->nodeID;
        numberOfFreeNodes++;
        return ret;
    }
    else {//plist has more than one item
        Node* lastItem = pList->last;

        pList->last->prev->next = NULL;
        pList->last = pList->last->prev;
        pList->cur = pList->last;
        pList->curPos = 0;
        pList->size--;
        
        void* ret = lastItem->item;
        lastItem->item = NULL;
        lastItem->next = NULL;
        lastItem->prev = NULL;
        freeNodes[numberOfFreeNodes] = lastItem->nodeID;
        numberOfFreeNodes++;
        return ret;
    }
}

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// UPDATED: Added clarification of behaviour May 19
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg)
{
    assert(pList != NULL);
    if (pList->size == 0 || pList->curPos == 2) {//if plist is empty or cur is beyond end
        return NULL;
    }
    else {
        if (pList->curPos == 1) {//if cur is before start
            pList->cur = pList->first;
            pList->curPos = 0;
        }

        while (pList->cur != NULL) {
            
            if ((*pComparator)(pList->cur->item, pComparisonArg)) {//if a match is found
                return pList->cur->item;
            }
            else {
                List_next(pList);
            }
        }

        return NULL;
    }
}