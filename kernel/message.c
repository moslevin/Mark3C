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

    \file   message.cpp    

    \brief  Inter-thread communications via message passing

*/

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "message.h"
#include "threadport.h"
#include "kerneldebug.h"

//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	MESSAGE_C       //!< File ID used in kernel trace calls


#if KERNEL_USE_MESSAGE

#if KERNEL_USE_TIMEOUTS
	#include "timerlist.h"
#endif

static Message_t m_aclMessagePool[GLOBAL_MESSAGE_POOL_SIZE];
static DoubleLinkList_t m_clList;

#if KERNEL_USE_TIMEOUTS
/*!
    * \brief Receive_i
    *
    * Internal function used to abstract timed and un-timed Receive calls.
    *
    * \param ulTimeWaitMS_ Time (in ms) to block, 0 for un-timed call.
    *
    * \return Pointer to a message, or 0 on timeout.
    */
static Message_t *MessageQueue_Receive_i( MessageQueue_t *pstMsgQ_, K_ULONG ulTimeWaitMS_ );
#else
/*!
    * \brief Receive_i
    *
    * Internal function used to abstract Receive calls.
    *
    * \return Pointer to a message.
    */
static Message_t *MessageQueue_Receive_i( MessageQueue_t *pstMsgQ_ );
#endif

//---------------------------------------------------------------------------
void Message_Init( Message_t* pstMsg_ ) 
{ 
	LinkListNode_Clear( (LinkListNode_t*)pstMsg_ );
	pstMsg_->m_pvData = NULL; 
	pstMsg_->m_usCode = 0; 
}
    
//---------------------------------------------------------------------------
void Message_SetData( Message_t* pstMsg_, void *pvData_ ) 
{ 
	pstMsg_->m_pvData = pvData_; 
}
    
//---------------------------------------------------------------------------
void *Message_GetData( Message_t* pstMsg_ ) 
{ 
	return pstMsg_->m_pvData; 
}
	
//---------------------------------------------------------------------------
void Message_SetCode(  Message_t* pstMsg_, K_USHORT usCode_ ) 
{ 
	pstMsg_->m_usCode = usCode_; 
}
	
//---------------------------------------------------------------------------
K_USHORT Message_GetCode( Message_t* pstMsg_ ) 
{ 
	return pstMsg_->m_usCode; 
}

//---------------------------------------------------------------------------
void GlobalMessagePool_Init( void )
{
	K_UCHAR i;

	DoubleLinkList_Init( &m_clList );
	
    for (i = 0; i < GLOBAL_MESSAGE_POOL_SIZE; i++)
	{
		Message_Init( &m_aclMessagePool[i] );
		DoubleLinkList_Add( &m_clList, (LinkListNode_t*)&m_aclMessagePool[i]);	
	}
}

//---------------------------------------------------------------------------
void GlobalMessagePool_Push( Message_t *pstMessage_ )
{
	KERNEL_ASSERT( pstMessage_ );
	
	CS_ENTER();
	
	DoubleLinkList_Add( &m_clList, (LinkListNode_t*)pstMessage_ );

	CS_EXIT();
}
	
//---------------------------------------------------------------------------
Message_t *GlobalMessagePool_Pop( void )
{
	Message_t *pstRet;
	CS_ENTER();
	
	pstRet = (Message_t*)( LinkList_GetHead( (LinkList_t*)&m_clList ) );
    if (0 != pstRet)
    {
		DoubleLinkList_Remove( &m_clList, (LinkListNode_t*)pstRet );
    }
	
	CS_EXIT();
	return pstRet;
}

//---------------------------------------------------------------------------
void MessageQueue_Init( MessageQueue_t *pstMsgQ_ ) 
{ 
	Semaphore_Init( &(pstMsgQ_->m_clSemaphore), 0, GLOBAL_MESSAGE_POOL_SIZE);  
	DoubleLinkList_Init( &(pstMsgQ_->m_clLinkList) );  
}

//---------------------------------------------------------------------------
Message_t *MessageQueue_Receive( MessageQueue_t *pstMsgQ_ )
{
#if KERNEL_USE_TIMEOUTS
    return MessageQueue_Receive_i( pstMsgQ_, 0 );
#else
    return MessageQueue_Receive_i( pstMsgQ_ );
#endif
}

//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
Message_t *MessageQueue_TimedReceive( MessageQueue_t *pstMsgQ_, K_ULONG ulTimeWaitMS_)
{
    return MessageQueue_Receive_i( pstMsgQ_, ulTimeWaitMS_ );
}
#endif

//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
Message_t *MessageQueue_Receive_i( MessageQueue_t *pstMsgQ_, K_ULONG ulTimeWaitMS_ )
#else
Message_t *MessageQueue_Receive_i(  MessageQueue_t *pstMsgQ_ )
#endif
{
	Message_t *pstRet;
	
	// Block the current thread on the counting Semaphore_t
#if KERNEL_USE_TIMEOUTS
	if ( 0 == Semaphore_TimedPend( &(pstMsgQ_->m_clSemaphore), ulTimeWaitMS_ ) )    
    {
        return NULL;
    }
#else
	Semaphore_Pend( &(pstMsgQ_->m_clSemaphore) );    
#endif
	
	CS_ENTER();
	
	// Pop the head of the message queue and return it
	pstRet = (Message_t*)LinkList_GetHead( (LinkList_t*)&(pstMsgQ_->m_clLinkList) );
	DoubleLinkList_Remove( (DoubleLinkList_t*)&(pstMsgQ_->m_clLinkList), (LinkListNode_t*)pstRet );
	
	CS_EXIT();
	
	return pstRet;	
}

//---------------------------------------------------------------------------
void MessageQueue_Send( MessageQueue_t *pstMsgQ_, Message_t *pstSrc_ )
{
	KERNEL_ASSERT( pstSrc_ );
	
	CS_ENTER();
	
	// Add the message to the head of the linked list
	DoubleLinkList_Add( (DoubleLinkList_t*)&(pstMsgQ_->m_clLinkList), (LinkListNode_t*)pstSrc_ );
		
	// Post the Semaphore_t, waking the blocking thread for the queue.
	Semaphore_Post( &(pstMsgQ_->m_clSemaphore) );
	
	CS_EXIT();
}

//---------------------------------------------------------------------------
K_USHORT MessageQueue_GetCount( MessageQueue_t *pstMsgQ_ )
{
	return Semaphore_GetCount( &(pstMsgQ_->m_clSemaphore) );
}
#endif //KERNEL_USE_MESSAGE
