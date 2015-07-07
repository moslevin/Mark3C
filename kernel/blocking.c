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

    \file   blocking.cpp

    \brief  Implementation of base class for blocking objects
*/

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "kerneldebug.h"

#include "blocking.h"
#include "thread.h"

//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	BLOCKING_C       //!< File ID used in kernel trace calls

#if KERNEL_USE_SEMAPHORE || KERNEL_USE_MUTEX
//---------------------------------------------------------------------------
void BlockingObject_Block( ThreadList_t *pstList_, Thread_t *pstThread_ )
{
    KERNEL_ASSERT( pstThread_ );
    KERNEL_TRACE_1( STR_THREAD_BLOCK_1, (K_USHORT)Thread_GetID( pstThread_ ) );
	
    // Remove the thread from its current thread list (the "owner" list)
    // ... And add the thread to this object's block list    
    Scheduler_Remove( pstThread_ );
    ThreadList_Add( pstList_, pstThread_ );
    
    // Set the "current" list location to the blocklist for this thread
    Thread_SetCurrent( pstThread_, pstList_ );
    Thread_SetState( pstThread_, THREAD_STATE_BLOCKED );
}

//---------------------------------------------------------------------------
void BlockingObject_UnBlock( Thread_t *pstThread_ )
{
    KERNEL_ASSERT( pstThread_ );
    KERNEL_TRACE_1( STR_THREAD_UNBLOCK_1, (K_USHORT)Thread_GetID( pstThread_ ) );
    
	// Remove the thread from its current thread list (the "owner" list)
    ThreadList_Remove( Thread_GetCurrent( pstThread_ ), pstThread_ );
    
    // Put the thread back in its active owner's list.  This is usually
    // the ready-queue at the thread's original priority.    
    Scheduler_Add( pstThread_ );
    
    // Tag the thread's current list location to its owner
    Thread_SetCurrent( pstThread_, Thread_GetOwner( pstThread_ ) );
    Thread_SetState( pstThread_, THREAD_STATE_READY );
}

#endif
