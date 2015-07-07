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

    \file   threadlist.cpp

    \brief  Thread_t linked-list definitions

*/

#include "kerneltypes.h"
#include "ll.h"
#include "threadlist.h"
#include "thread.h"
#include "kerneldebug.h"
//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	THREADLIST_CPP       //!< File ID used in kernel trace calls

//---------------------------------------------------------------------------
void ThreadList_Init( ThreadList_t *pstList_ )
{ 
	CircularLinkList_Init( (CircularLinkList_t*)pstList_ );
	pstList_->m_ucPriority = 0; 
	pstList_->m_pucFlag = NULL; 
}

//---------------------------------------------------------------------------
void ThreadList_SetPriority( ThreadList_t *pstList_, K_UCHAR ucPriority_ )
{
    pstList_->m_ucPriority = ucPriority_;
}

//---------------------------------------------------------------------------
void ThreadList_SetFlagPointer( ThreadList_t *pstList_, K_UCHAR *pucFlag_)
{
    pstList_->m_pucFlag = pucFlag_;
}

//---------------------------------------------------------------------------
void ThreadList_Add( ThreadList_t *pstList_, Thread_t *node_ ) 
{
    CircularLinkList_t *pstCLL = (CircularLinkList_t*)pstList_;
	CircularLinkList_Add( pstCLL, (LinkListNode_t*)node_);
    CircularLinkList_PivotForward( pstCLL );
    
    // We've specified a bitmap for this threadlist
    if (pstList_->m_pucFlag)
    {
        // Set the flag for this priority level
        *pstList_->m_pucFlag |= (1 << pstList_->m_ucPriority);
    }
}

//---------------------------------------------------------------------------
void ThreadList_AddEx( ThreadList_t *pstList_, Thread_t *node_, K_UCHAR *pucFlag_, K_UCHAR ucPriority_) {
    // Set the threadlist's priority level, flag pointer, and then add the
    // thread to the threadlist
    ThreadList_SetPriority( pstList_, ucPriority_ );
    ThreadList_SetFlagPointer( pstList_, pucFlag_ );
    ThreadList_Add( pstList_, node_);
}

//---------------------------------------------------------------------------
void ThreadList_Remove( ThreadList_t *pstList_, Thread_t *node_) {
    // Remove the thread from the list
	CircularLinkList_t *pstCLL = (CircularLinkList_t*)pstList_;
    CircularLinkList_Remove( pstCLL, (LinkListNode_t*)node_);
    
    // If the list is empty...
    if (!pstCLL->m_pstHead)
    {
        // Clear the bit in the bitmap at this priority level
        if (pstList_->m_pucFlag)
        {
            *pstList_->m_pucFlag &= ~(1 << pstList_->m_ucPriority);
        }
    }
}

//---------------------------------------------------------------------------
Thread_t *ThreadList_HighestWaiter( ThreadList_t *pstList_ )
{
    Thread_t *pstTemp = (Thread_t*)LinkList_GetHead( (LinkList_t*)pstList_ );
    Thread_t *pstChosen = pstTemp;
	
	K_UCHAR ucMaxPri = 0;
    
    // Go through the list, return the highest-priority thread in this list.
	while(1)
	{
        // Compare against current max-priority thread
        if (Thread_GetPriority( pstTemp ) >= ucMaxPri)
		{
            ucMaxPri = Thread_GetPriority( pstTemp );
            pstChosen = pstTemp;
		}
        
        // Break out if this is the last thread in the list
        if (pstTemp == (Thread_t*)LinkList_GetTail( (LinkList_t*)pstList_ ) )
		{
			break;
		}
        
        pstTemp = (Thread_t*)LinkListNode_GetNext( (LinkListNode_t*)pstTemp );
	} 
    return pstChosen;
}
