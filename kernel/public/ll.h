/*===========================================================================
     _____        _____        _____        _____
 ___|    _|__  __|_    |__  __|__   |__  __| __  |__  ______
|    \  /  | ||    \      ||     |     ||  |/ /     ||___   |
|     \/   | ||     \     ||     \     ||     \     ||___   |
|__/\__/|__|_||__|\__\  __||__|\__\  __||__|\__\  __||______|
    |_____|      |_____|      |_____|      |_____|

--[Mark3 Realtime Platform]--------------------------------------------------

Copyright (c) 2012-2015 Funkenstein Software Consulting, all rights reserved.
See license.txt for more information
===========================================================================*/
/*!

    \file   ll.h    

    \brief  Core linked-list declarations, used by all kernel list types
    
    At the heart of RTOS data structures are linked lists.  Having a robust
    and efficient set of linked-list types that we can use as a foundation for 
    building the rest of our kernel types allows us to keep our RTOS code 
    efficient and logically-separated.

    So what data types rely on these linked-list classes?
    
    -Threads
    -ThreadLists
    -The Scheduler
    -Timers,
    -The Timer_t Scheduler
    -Blocking objects (Semaphores, Mutexes, etc...)
    
    Pretty much everything in the kernel uses these linked lists.  By 
    having objects inherit from the base linked-list node type, we're able
    to leverage the double and circular linked-list classes to manager
    virtually every object type in the system without duplicating code.
    These functions are very efficient as well, allowing for very deterministic
    behavior in our code.
    
*/

#ifndef __LL_H__
#define __LL_H__

#include "kerneltypes.h"

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
#ifndef NULL
#define NULL        (0)
#endif

//---------------------------------------------------------------------------
/*!
    Basic linked-list node data structure.  This data is managed by the 
    linked-list class types, and can be used transparently between them.
*/
typedef struct _LinkListNode
{
    struct _LinkListNode *next;     //!< Pointer to the next node in the list
    struct _LinkListNode *prev;     //!< Pointer to the previous node in the list
} LinkListNode_t;

//---------------------------------------------------------------------------
/*!
    \fn void ClearNode()

    Initialize the linked list node, clearing its next and previous node.
*/
void LinkListNode_Clear( LinkListNode_t *pstNode_ );

//---------------------------------------------------------------------------
/*!
    \fn LinkListNode_t *GetNext();

    Returns a pointer to the next node in the list.

    \return a pointer to the next node in the list.
*/
LinkListNode_t *LinkListNode_GetNext( LinkListNode_t *pstNode_ );

//---------------------------------------------------------------------------
/*!
    \fn LinkListNode_t *GetPrev();

    Returns a pointer to the previous node in the list.

    \return a pointer to the previous node in the list.
*/
LinkListNode_t *LinkListNode_GetPrev( LinkListNode_t *pstNode_ );


//---------------------------------------------------------------------------
/*!
    Abstract-data-type from which all other linked-lists are derived
*/
typedef struct _LinkList
{
    LinkListNode_t *m_pstHead;    //!< Pointer to the head node in the list
    LinkListNode_t *m_pstTail;    //!< Pointer to the tail node in the list
} LinkList_t;

//---------------------------------------------------------------------------
/*!
    Clear the linked list.
*/
void LinkList_Init( LinkList_t *pstList_ );

//---------------------------------------------------------------------------
/*!
    \fn LinkListNode_t *GetHead()

    Get the head node in the linked list

    \return Pointer to the head node in the list
*/
LinkListNode_t *LinkList_GetHead( LinkList_t *pstList_ );

//---------------------------------------------------------------------------
/*!
    \fn LinkListNode_t *GetTail()

    Get the tail node of the linked list

    \return Pointer to the tail node in the list
*/
LinkListNode_t *LinkList_GetTail( LinkList_t *pstList_ );

//---------------------------------------------------------------------------
/*!
    Doubly-linked-list data type, inherited from the base LinkList_t type.
*/
typedef struct _DoubleLinkList
{
    LinkListNode_t *m_pstHead;    //!< Pointer to the head node in the list
    LinkListNode_t *m_pstTail;    //!< Pointer to the tail node in the list
} DoubleLinkList_t;

//---------------------------------------------------------------------------
/*!
    Default constructor - initializes the head/tail nodes to NULL
*/
void DoubleLinkList_Init( DoubleLinkList_t *pstList_ );

//---------------------------------------------------------------------------
/*!
    \fn void Add(LinkListNode_t *node_)

    Add the linked list node to this linked list

    \param node_ Pointer to the node to add
*/
void DoubleLinkList_Add( DoubleLinkList_t *pstList_, LinkListNode_t *node_ );

//---------------------------------------------------------------------------
/*!
    \fn void Remove(LinkListNode_t *node_)

    Add the linked list node to this linked list

    \param node_ Pointer to the node to remove
*/
void DoubleLinkList_Remove( DoubleLinkList_t *pstList_, LinkListNode_t *node_ );

//---------------------------------------------------------------------------
/*!
    Circular-linked-list data type, inherited from the base LinkList_t type.
*/
typedef struct _CircularLinkList
{
    LinkListNode_t *m_pstHead;    //!< Pointer to the head node in the list
    LinkListNode_t *m_pstTail;    //!< Pointer to the tail node in the list
} CircularLinkList_t;

//---------------------------------------------------------------------------
void CircularLinkList_Init( CircularLinkList_t *pstList_ );

//---------------------------------------------------------------------------
/*!
    \fn void Add(LinkListNode_t *node_)

    Add the linked list node to this linked list

    \param node_ Pointer to the node to add
*/
void CircularLinkList_Add( CircularLinkList_t *pstList_, LinkListNode_t *node_);

//---------------------------------------------------------------------------
/*!
    \fn void Remove(LinkListNode_t *node_)

    Add the linked list node to this linked list

    \param node_ Pointer to the node to remove
*/
void CircularLinkList_Remove( CircularLinkList_t *pstList_, LinkListNode_t *node_);

//---------------------------------------------------------------------------
/*!
    \fn void PivotForward()

    Pivot the head of the circularly linked list forward
    ( Head = Head->next, Tail = Tail->next )
*/
void CircularLinkList_PivotForward( CircularLinkList_t *pstList_ );

//---------------------------------------------------------------------------
/*!
    \fn void PivotBackward()

    Pivot the head of the circularly linked list backward
    ( Head = Head->prev, Tail = Tail->prev )
*/
void CircularLinkList_PivotBackward( CircularLinkList_t *pstList_ );

#ifdef __cplusplus
    }
#endif

#endif
