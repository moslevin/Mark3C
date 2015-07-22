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

    \file   ll.cpp   

    \brief  Core Linked-List implementation, from which all kernel objects
			are derived
*/

#include "kerneltypes.h"
#include "ll.h"
#include "kerneldebug.h"

//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	LL_C        //!< File ID used in kernel trace calls

//---------------------------------------------------------------------------
void DoubleLinkList_Add( DoubleLinkList_t *pstList_, LinkListNode_t *node_)
{
	KERNEL_ASSERT( node_ );
	
    // Add a node to the end of the linked list.
    if (!pstList_->pstHead)
    {
        // If the list is empty, initilize the nodes
        pstList_->pstHead = node_;
        pstList_->pstTail = node_;

        pstList_->pstHead->prev = NULL;
        pstList_->pstTail->next = NULL;
        return;
    }
    
    // Move the tail node, and assign it to the new node just passed in
    pstList_->pstTail->next = node_;
    node_->prev = pstList_->pstTail;
    node_->next = NULL;    
    pstList_->pstTail = node_;
}

//---------------------------------------------------------------------------
void DoubleLinkList_Remove( DoubleLinkList_t *pstList_, LinkListNode_t *node_)
{
	KERNEL_ASSERT( node_ );

    if (node_->prev)
    {
#if SAFE_UNLINK
        if (node_->prev->next != node_)
        {
            Kernel_Panic(PANIC_LIST_UNLINK_FAILED);
        }
#endif
        node_->prev->next = node_->next;
    }
    if (node_->next)
    {
#if SAFE_UNLINK
        if (node_->next->prev != node_)
        {
            Kernel_Panic(PANIC_LIST_UNLINK_FAILED);
        }
#endif
        node_->next->prev = node_->prev;
    }
    if (node_ == pstList_->pstHead)
    {        
        pstList_->pstHead = node_->next;
    }
    if (node_ == pstList_->pstTail)
    {
        pstList_->pstTail = node_->prev;
    }
    
    LinkListNode_Clear( node_ );
}



//---------------------------------------------------------------------------
void CircularLinkList_Add( CircularLinkList_t *pstList_, LinkListNode_t *node_)
{
	KERNEL_ASSERT( node_ );

    // Add a node to the end of the linked list.
    if (!pstList_->pstHead)
    {
        // If the list is empty, initilize the nodes
        pstList_->pstHead = node_;
        pstList_->pstTail = node_;

        pstList_->pstHead->prev = pstList_->pstHead;
        pstList_->pstHead->next = pstList_->pstHead;
        return;
    }
    
    // Move the tail node, and assign it to the new node just passed in
    pstList_->pstTail->next = node_;
    node_->prev = pstList_->pstTail;
    node_->next = pstList_->pstHead;
    pstList_->pstTail = node_;
    pstList_->pstHead->prev = node_;
}

//---------------------------------------------------------------------------
void CircularLinkList_Remove( CircularLinkList_t *pstList_, LinkListNode_t *node_)
{
	KERNEL_ASSERT( node_ );
	
    // Check to see if this is the head of the list...
    if ((node_ == pstList_->pstHead) && (pstList_->pstHead == pstList_->pstTail))
    {
        // Clear the head and tail pointers - nothing else left.
        pstList_->pstHead = NULL;
        pstList_->pstTail = NULL;
        return;
    }
    
#if SAFE_UNLINK
    // Verify that all nodes are properly connected
    if ((node_->prev->next != node_) || (node_->next->prev != node_))
    {
        Kernel_Panic(PANIC_LIST_UNLINK_FAILED);
    }
#endif

    // This is a circularly linked list - no need to check for connection,
    // just remove the node.
    node_->next->prev = node_->prev;
    node_->prev->next = node_->next;
    
    if (node_ == pstList_->pstHead)
    {
        pstList_->pstHead = pstList_->pstHead->next;
    }
    if (node_ == pstList_->pstTail)
    {
        pstList_->pstTail = pstList_->pstTail->prev;
    }
    LinkListNode_Clear( node_ );
}

//---------------------------------------------------------------------------
void CircularLinkList_PivotForward( CircularLinkList_t *pstList_ )
{
    if (pstList_->pstHead)
    {
        pstList_->pstHead = pstList_->pstHead->next;
        pstList_->pstTail = pstList_->pstTail->next;
    }
}

//---------------------------------------------------------------------------
void CircularLinkList_PivotBackward(  CircularLinkList_t *pstList_ )
{
    if (pstList_->pstHead)
    {
        pstList_->pstHead = pstList_->pstHead->prev;
        pstList_->pstTail = pstList_->pstTail->prev;
    }
}
