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
void LinkListNode_Clear( LinkListNode_t *pstNode_  )
{
    pstNode_->next = NULL;
    pstNode_->prev = NULL;
}

//---------------------------------------------------------------------------
LinkListNode_t *LinkListNode_GetNext( LinkListNode_t *pstNode_ )
{
	return pstNode_->next;
}

//---------------------------------------------------------------------------
LinkListNode_t *LinkListNode_GetPrev( LinkListNode_t *pstNode_ )
{
	return pstNode_->prev;
}

//---------------------------------------------------------------------------
void LinkList_Init( LinkList_t *pstList_ )
{
	pstList_->m_pstHead = NULL; 
	pstList_->m_pstTail = NULL; 
}

//---------------------------------------------------------------------------
LinkListNode_t *LinkList_GetHead( LinkList_t *pstList_ ) 
{ 
	return pstList_->m_pstHead; 
}

//---------------------------------------------------------------------------
LinkListNode_t *LinkList_GetTail( LinkList_t *pstList_ ) 
{ 
	return pstList_->m_pstTail; 
}

//---------------------------------------------------------------------------
void DoubleLinkList_Init( DoubleLinkList_t *pstList_ ) 
{ 
	pstList_->m_pstHead = NULL; 
	pstList_->m_pstTail = NULL; 
}

//---------------------------------------------------------------------------
void DoubleLinkList_Add( DoubleLinkList_t *pstList_, LinkListNode_t *node_)
{
	KERNEL_ASSERT( node_ );
	
    // Add a node to the end of the linked list.
    if (!pstList_->m_pstHead)
    {
        // If the list is empty, initilize the nodes
        pstList_->m_pstHead = node_;
        pstList_->m_pstTail = node_;

        pstList_->m_pstHead->prev = NULL;
        pstList_->m_pstTail->next = NULL;
        return;
    }
    
    // Move the tail node, and assign it to the new node just passed in
    pstList_->m_pstTail->next = node_;
    node_->prev = pstList_->m_pstTail;
    node_->next = NULL;    
    pstList_->m_pstTail = node_;
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
    if (node_ == pstList_->m_pstHead)
    {        
        pstList_->m_pstHead = node_->next;
    }
    if (node_ == pstList_->m_pstTail)
    {
        pstList_->m_pstTail = node_->prev;
    }
    
    LinkListNode_Clear( node_ );
}

//---------------------------------------------------------------------------
void CircularLinkList_Init( CircularLinkList_t *pstList_ ) 
{ 
	pstList_->m_pstHead = NULL; 
	pstList_->m_pstTail = NULL; 
}

//---------------------------------------------------------------------------
void CircularLinkList_Add( CircularLinkList_t *pstList_, LinkListNode_t *node_)
{
	KERNEL_ASSERT( node_ );

    // Add a node to the end of the linked list.
    if (!pstList_->m_pstHead)
    {
        // If the list is empty, initilize the nodes
        pstList_->m_pstHead = node_;
        pstList_->m_pstTail = node_;

        pstList_->m_pstHead->prev = pstList_->m_pstHead;
        pstList_->m_pstHead->next = pstList_->m_pstHead;
        return;
    }
    
    // Move the tail node, and assign it to the new node just passed in
    pstList_->m_pstTail->next = node_;
    node_->prev = pstList_->m_pstTail;
    node_->next = pstList_->m_pstHead;
    pstList_->m_pstTail = node_;
    pstList_->m_pstHead->prev = node_;
}

//---------------------------------------------------------------------------
void CircularLinkList_Remove( CircularLinkList_t *pstList_, LinkListNode_t *node_)
{
	KERNEL_ASSERT( node_ );
	
    // Check to see if this is the head of the list...
    if ((node_ == pstList_->m_pstHead) && (pstList_->m_pstHead == pstList_->m_pstTail))
    {
        // Clear the head and tail pointers - nothing else left.
        pstList_->m_pstHead = NULL;
        pstList_->m_pstTail = NULL;
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
    
    if (node_ == pstList_->m_pstHead)
    {
        pstList_->m_pstHead = pstList_->m_pstHead->next;
    }
    if (node_ == pstList_->m_pstTail)
    {
        pstList_->m_pstTail = pstList_->m_pstTail->prev;
    }
    LinkListNode_Clear( node_ );
}

//---------------------------------------------------------------------------
void CircularLinkList_PivotForward( CircularLinkList_t *pstList_ )
{
    if (pstList_->m_pstHead)
    {
        pstList_->m_pstHead = pstList_->m_pstHead->next;
        pstList_->m_pstTail = pstList_->m_pstTail->next;
    }
}

//---------------------------------------------------------------------------
void CircularLinkList_PivotBackward(  CircularLinkList_t *pstList_ )
{
    if (pstList_->m_pstHead)
    {
        pstList_->m_pstHead = pstList_->m_pstHead->prev;
        pstList_->m_pstTail = pstList_->m_pstTail->prev;
    }
}
