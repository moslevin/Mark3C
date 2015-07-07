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
    \file Mutex_t.cpp
    
    \brief Mutual-exclusion object
*/

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "blocking.h"
#include "mutex.h"
#include "kerneldebug.h"
//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	MUTEX_C       //!< File ID used in kernel trace calls

#if KERNEL_USE_MUTEX

/*!
    \fn K_UCHAR WakeNext();
        
    Wake the next thread waiting on the Mutex_t.
*/
static K_UCHAR Mutex_WakeNext( Mutex_t *pstMutex_ );

#if KERNEL_USE_TIMEOUTS
/*!
    * \brief Claim_i
    *
    * Abstracts out timed/non-timed Mutex_t claim operations.
    *
    * \param ulWaitTimeMS_ Time in MS to wait, 0 for infinite
    * \return true on successful claim, false otherwise
    */
static K_BOOL Mutex_Claim_i( Mutex_t *pstMutex_, K_ULONG ulWaitTimeMS_ );

/*!
    \fn void WakeMe( Thread_t *pstOwner_ )

    Wake a thread blocked on the Mutex_t.  This is an
    internal function used for implementing timed mutexes
    relying on timer callbacks.  Since these do not have
    access to the private data of the Mutex_t and its base
    classes, we have to wrap this as a public method - do not
    use this for any other purposes.

    \param pstOwner_ Thread_t to unblock from this object.
*/
static void Mutex_WakeMe( Mutex_t *pstMutex_, Thread_t *pstOwner_ );

#else
/*!
    * \brief Claim_i
    *
    * Abstraction for Mutex_t claim operations.
    *
    */
static void Mutex_Claim_i( Mutex_t *pstMutex_ );
#endif

#if KERNEL_USE_TIMEOUTS

//---------------------------------------------------------------------------
/*!
 * \brief TimedMutex_Calback
 *
 * This function is called from the timer-expired context to trigger a timeout
 * on this Mutex_t.  This results in the waking of the thread that generated
 * the Mutex_t claim call that was not completed in time.
 *
 * \param pstOwner_ Pointer to the thread to wake
 * \param pvData_   Pointer to the Mutex_t object that the thread is blocked on
 */
void TimedMutex_Calback(Thread_t *pstOwner_, void *pvData_)
{
	Mutex_t *pstMutex = (Mutex_t*)(pvData_);
		
	// Indicate that the Semaphore_t has expired on the thread
	Thread_SetExpired( pstOwner_, true );
		
	// Wake up the thread that was blocked on this Semaphore_t.
	Mutex_WakeMe( pstMutex, pstOwner_ );
		
	if ( Thread_GetCurPriority( pstOwner_ ) >=
 		 Thread_GetCurPriority( Scheduler_GetCurrentThread() ) )
	{
 		Thread_Yield();
	}
}

//---------------------------------------------------------------------------
void Mutex_WakeMe( Mutex_t *pstMutex_, Thread_t *pstOwner_ )
{
	// Remove from the Semaphore_t waitlist and back to its ready list.
	BlockingObject_UnBlock(pstOwner_);
}

#endif

//---------------------------------------------------------------------------
K_UCHAR Mutex_WakeNext( Mutex_t *pstMutex_ )
{
    Thread_t *pstChosenOne = NULL;

    // Get the highest priority waiter thread
    pstChosenOne = ThreadList_HighestWaiter( (ThreadList_t*)pstMutex_ );
    
    // Unblock the thread
    BlockingObject_UnBlock(pstChosenOne);
    
    // The chosen one now owns the Mutex_t
    pstMutex_->m_pstOwner = pstChosenOne;

    // Signal a context switch if it's a greater than or equal to the current priority
    if ( Thread_GetCurPriority(pstChosenOne) >= 
		 Thread_GetCurPriority( Scheduler_GetCurrentThread() ) )		
    {
        return 1;
    }
    return 0;
}

//---------------------------------------------------------------------------
void Mutex_Init( Mutex_t *pstMutex_ )
{
	ThreadList_Init( (ThreadList_t*)pstMutex_ );
	
    // Reset the data in the Mutex_t
    pstMutex_->m_bReady = 1;             // The Mutex_t is free.
    pstMutex_->m_ucMaxPri = 0;           // Set the maximum priority inheritence state
    pstMutex_->m_pstOwner = NULL;        // Clear the Mutex_t owner
    pstMutex_->m_ucRecurse = 0;          // Reset recurse count
}

//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
K_BOOL Mutex_Claim_i( Mutex_t *pstMutex_, K_ULONG ulWaitTimeMS_)
#else
void Mutex_Claim_i( Mutex_t *pstMutex_ )
#endif
{
    KERNEL_TRACE_1( STR_MUTEX_CLAIM_1, (K_USHORT)Thread_GetID( g_pstCurrent ) );

#if KERNEL_USE_TIMEOUTS
    Timer_t clTimer;
    K_BOOL bUseTimer = false;
#endif

    // Disable the scheduler while claiming the Mutex_t - we're dealing with all
    // sorts of private thread data, can't have a thread switch while messing
    // with internal data structures.
    Scheduler_SetScheduler( false );

    // Check to see if the Mutex_t is claimed or not
    if (pstMutex_->m_bReady != 0)
    {
        // Mutex_t isn't claimed, claim it.
        pstMutex_->m_bReady = 0;
        pstMutex_->m_ucRecurse = 0;
        pstMutex_->m_ucMaxPri = Thread_GetPriority( g_pstCurrent );
        pstMutex_->m_pstOwner = g_pstCurrent;

        Scheduler_SetScheduler( true );

#if KERNEL_USE_TIMEOUTS
        return true;
#else
        return;
#endif
    }

    // If the Mutex_t is already claimed, check to see if this is the owner thread,
    // since we allow the Mutex_t to be claimed recursively.
    if (g_pstCurrent == pstMutex_->m_pstOwner)
    {
        // Ensure that we haven't exceeded the maximum recursive-lock count
        KERNEL_ASSERT( (pstMutex_->m_ucRecurse < 255) );
        pstMutex_->m_ucRecurse++;

        // Increment the lock count and bail
        Scheduler_SetScheduler( true );
#if KERNEL_USE_TIMEOUTS
        return true;
#else
        return;
#endif
    }

    // The Mutex_t is claimed already - we have to block now.  Move the
    // current thread to the list of threads waiting on the Mutex_t.
#if KERNEL_USE_TIMEOUTS
    if (ulWaitTimeMS_)
    {
		Thread_SetExpired( g_pstCurrent, false );
        
		Timer_Init( &clTimer );
		Timer_Start( &clTimer, false, ulWaitTimeMS_, (TimerCallback_t)TimedMutex_Calback, (void*)pstMutex_);
        bUseTimer = true;
    }
#endif
    BlockingObject_Block( (ThreadList_t*)pstMutex_, g_pstCurrent );

    // Check if priority inheritence is necessary.  We do this in order
    // to ensure that we don't end up with priority inversions in case
    // multiple threads are waiting on the same resource.
    if(pstMutex_->m_ucMaxPri <= Thread_GetPriority( g_pstCurrent ) )
    {
        pstMutex_->m_ucMaxPri = Thread_GetPriority( g_pstCurrent );

        Thread_t *pstTemp = (Thread_t*)(LinkList_GetHead( (LinkList_t*)pstMutex_ ));
        while(pstTemp)
        {
			Thread_InheritPriority( pstTemp, pstMutex_->m_ucMaxPri );            
			if(pstTemp == (Thread_t*)(LinkList_GetTail( (LinkList_t*)pstMutex_ )) )
            {
                break;
            }
            pstTemp = (Thread_t*)LinkListNode_GetNext( (LinkListNode_t*)pstTemp );
        }
		Thread_InheritPriority( pstMutex_->m_pstOwner, pstMutex_->m_ucMaxPri );        
    }

    // Done with thread data -reenable the scheduler
    Scheduler_SetScheduler( true );

    // Switch threads if this thread acquired the Mutex_t
    Thread_Yield();

#if KERNEL_USE_TIMEOUTS
    if (bUseTimer)
    {
		Timer_Stop( &clTimer );        
        return ( Thread_GetExpired( g_pstCurrent ) == 0);
    }
    return true;
#endif
}

//---------------------------------------------------------------------------
void Mutex_Claim( Mutex_t *pstMutex_ )
{
#if KERNEL_USE_TIMEOUTS
    Mutex_Claim_i( pstMutex_ , 0 );
#else
    Mutex_Claim_i( pstMutex_ );
#endif
}

//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
K_BOOL Mutex_TimedClaim( Mutex_t *pstMutex_, K_ULONG ulWaitTimeMS_ )
{
    return Mutex_Claim_i( pstMutex_ , ulWaitTimeMS_ );        
}
#endif

//---------------------------------------------------------------------------
void Mutex_Release( Mutex_t *pstMutex_ )
{
	KERNEL_TRACE_1( STR_MUTEX_RELEASE_1, (K_USHORT)Thread_GetID( g_pstCurrent ) );

    K_BOOL bSchedule = 0;

    // Disable the scheduler while we deal with internal data structures.
    Scheduler_SetScheduler( false );

    // This thread had better be the one that owns the Mutex_t currently...
    KERNEL_ASSERT( (g_pstCurrent == pstMutex_->m_pstOwner) );

    // If the owner had claimed the lock multiple times, decrease the lock
    // count and return immediately.
    if (pstMutex_->m_ucRecurse)
    {
        pstMutex_->m_ucRecurse--;
        Scheduler_SetScheduler( true );
        return;
    }

    // Restore the thread's original priority
    if (Thread_GetCurPriority( g_pstCurrent ) != Thread_GetPriority( g_pstCurrent ))
    {
        Thread_SetPriority( g_pstCurrent, Thread_GetPriority(g_pstCurrent) );
        // In this case, we want to reschedule
        bSchedule = 1;
    }

    // No threads are waiting on this Mutex_t?
    if ( LinkList_GetHead( (LinkList_t*)pstMutex_ ) == NULL)
    {
        // Re-initialize the Mutex_t to its default values
        pstMutex_->m_bReady = 1;
        pstMutex_->m_ucMaxPri = 0;
        pstMutex_->m_pstOwner = NULL;
    }
    else
    {
        // Wake the highest priority Thread_t pending on the Mutex_t
        if( Mutex_WakeNext( pstMutex_ ) )
        {
            // Switch threads if it's higher or equal priority than the current thread
            bSchedule = 1;
        }
    }

    // Must enable the scheduler again in order to switch threads.
    Scheduler_SetScheduler( true );
    if(bSchedule)
    {
        // Switch threads if a higher-priority thread was woken
        Thread_Yield();
    }
}

#endif //KERNEL_USE_MUTEX
