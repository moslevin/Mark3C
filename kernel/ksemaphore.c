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

    \file   ksemaphore.cpp

    \brief  Semaphore_t Blocking-Object Implemenation 

*/

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "ksemaphore.h"
#include "blocking.h"   
#include "kerneldebug.h"
//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	SEMAPHORE_C       //!< File ID used in kernel trace calls

#if KERNEL_USE_SEMAPHORE

#if KERNEL_USE_TIMEOUTS
#include "timerlist.h"

/*!
    \fn K_UCHAR WakeNext();
        
    Wake the next thread waiting on the Semaphore_t.
*/
static K_UCHAR Semaphore_WakeNext( Semaphore_t *pstSem_ );

#if KERNEL_USE_TIMEOUTS
/*!
	\fn void WakeMe(Thread_t *pstChosenOne_)
		
	Wake a thread blocked on the Semaphore_t.  This is an
	internal function used for implementing timed semaphores
	relying on timer callbacks.  Since these do not have
	access to the private data of the Semaphore_t and its base
	classes, we have to wrap this as a public method - do not
	use this for any other purposes.
*/
static void Semaphore_WakeMe( Semaphore_t *pstSem_, Thread_t *pstChosenOne_);
/*!
    * \brief Pend_i
    *
    * Internal function used to abstract timed and untimed Semaphore_t pend operations.
    *
    * \param ulWaitTimeMS_ Time in MS to wait
    * \return true on success, false on failure.
    */
static K_BOOL Semaphore_Pend_i( Semaphore_t *pstSem_, K_ULONG ulWaitTimeMS_ );
#else
/*!
    * \brief Pend_i
    *
    * Internal function used to abstract timed and untimed Semaphore_t pend operations.
    *
    */
static void Semaphore_Pend_i( Semaphore_t *pstSem_ );
#endif
    
//---------------------------------------------------------------------------
/*!
 * \brief TimedSemaphore_Callback
 *
 * This function is called from the timer-expired context to trigger a timeout
 * on this semphore.  This results in the waking of the thread that generated
 * the Semaphore_t pend call that was not completed in time.
 *
 * \param pstOwner_ Pointer to the thread to wake
 * \param pvData_   Pointer to the Semaphore_t object that the thread is blocked on
 */
void TimedSemaphore_Callback( Thread_t *pstOwner_, void *pvData_)
{
	Semaphore_t *pstSemaphore = (Semaphore_t*)(pvData_);
	
	// Indicate that the Semaphore_t has expired on the thread	
	Thread_SetExpired( pstOwner_, true );
	// Wake up the thread that was blocked on this Semaphore_t.
	Semaphore_WakeMe( pstSemaphore, pstOwner_ );
	
    if ( Thread_GetCurPriority( pstOwner_ ) >= 
		 Thread_GetCurPriority( Scheduler_GetCurrentThread() ) )
	{
        Thread_Yield();
	}	
}

//---------------------------------------------------------------------------
void Semaphore_WakeMe( Semaphore_t *pstSem_, Thread_t *pstChosenOne_ )
{ 
    // Remove from the Semaphore_t waitlist and back to its ready list.
    BlockingObject_UnBlock( pstChosenOne_ );
}

#endif // KERNEL_USE_TIMEOUTS

//---------------------------------------------------------------------------
K_UCHAR Semaphore_WakeNext( Semaphore_t *pstSem_ )
{
    Thread_t *pstChosenOne;
    
    pstChosenOne = ThreadList_HighestWaiter( (ThreadList_t*)pstSem_ );
    
    // Remove from the Semaphore_t waitlist and back to its ready list.
    BlockingObject_UnBlock( pstChosenOne );

    // Call a task switch if higher or equal priority thread
    if ( Thread_GetCurPriority( pstChosenOne ) >= 
		 Thread_GetCurPriority( Scheduler_GetCurrentThread() ) )
    {
        return 1;
    }
    return 0;
}

//---------------------------------------------------------------------------
void Semaphore_Init( Semaphore_t *pstSem_, K_USHORT usInitVal_, K_USHORT usMaxVal_)
{
    // Copy the paramters into the object - set the maximum value for this
    // Semaphore_t to implement either binary or counting semaphores, and set
    // the initial count.  Clear the wait list for this object.
    pstSem_->m_usValue = usInitVal_;
    pstSem_->m_usMaxValue = usMaxVal_;    

	ThreadList_Init( (ThreadList_t*)pstSem_ );
}

//---------------------------------------------------------------------------
K_BOOL Semaphore_Post( Semaphore_t *pstSem_ )
{
	KERNEL_TRACE_1( STR_SEMAPHORE_POST_1, (K_USHORT)Thread_GetID( g_pstCurrent ));
	
    K_BOOL bThreadWake = 0;
    K_BOOL bBail = false;
    // Increment the Semaphore_t count - we can mess with threads so ensure this
    // is in a critical section.  We don't just disable the scheudler since
    // we want to be able to do this from within an interrupt context as well.
    CS_ENTER();

    // If nothing is waiting for the Semaphore_t
	
    if ( LinkList_GetHead( (LinkList_t*)pstSem_ ) == NULL)
    {
        // Check so see if we've reached the maximum value in the Semaphore_t
        if (pstSem_->m_usValue < pstSem_->m_usMaxValue)
        {
            // Increment the count value
            pstSem_->m_usValue++;
        }
        else
        {
            // Maximum value has been reached, bail out.
            bBail = true;
        }
    }
    else
    {
        // Otherwise, there are threads waiting for the Semaphore_t to be
        // posted, so wake the next one (highest priority goes first).
        bThreadWake = Semaphore_WakeNext( pstSem_ );
    }

    CS_EXIT();

    // If we weren't able to increment the Semaphore_t count, fail out.
    if (bBail)
    {
        return false;
    }

    // if bThreadWake was set, it means that a higher-priority thread was
    // woken.  Trigger a context switch to ensure that this thread gets
    // to execute next.
    if (bThreadWake)
    {
        Thread_Yield();
    }
    return true;
}

//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
K_BOOL Semaphore_Pend_i( Semaphore_t *pstSem_, K_ULONG ulWaitTimeMS_ )
#else
void Semaphore_Pend_i( void )
#endif
{
    KERNEL_TRACE_1( STR_SEMAPHORE_PEND_1, (K_USHORT)Thread_GetID( g_pstCurrent ) );

#if KERNEL_USE_TIMEOUTS
    Timer_t stSemTimer;
    K_BOOL bUseTimer = false;
#endif

    // Once again, messing with thread data - ensure
    // we're doing all of these operations from within a thread-safe context.
    CS_ENTER();

    // Check to see if we need to take any action based on the Semaphore_t count
    if (pstSem_->m_usValue != 0)
    {
        // The Semaphore_t count is non-zero, we can just decrement the count
        // and go along our merry way.
        pstSem_->m_usValue--;
    }
    else
    {
        // The Semaphore_t count is zero - we need to block the current thread
        // and wait until the Semaphore_t is posted from elsewhere.
#if KERNEL_USE_TIMEOUTS        
        if (ulWaitTimeMS_)
        {
            Thread_SetExpired( g_pstCurrent, false );
			Timer_Init( &stSemTimer );
            Timer_Start( &stSemTimer, false, ulWaitTimeMS_, TimedSemaphore_Callback, (void*)pstSem_ );
            bUseTimer = true;
        }
#endif
        BlockingObject_Block( (ThreadList_t*)pstSem_, g_pstCurrent );

        // Switch Threads immediately
        Thread_Yield();
    }

    CS_EXIT();

#if KERNEL_USE_TIMEOUTS
    if (bUseTimer)
    {
		Timer_Stop( &stSemTimer );
        return ( Thread_GetExpired( g_pstCurrent ) == 0);
    }
    return true;
#endif
}

//---------------------------------------------------------------------------
// Redirect the untimed pend API to the timed pend, with a null timeout.
void Semaphore_Pend( Semaphore_t *pstSem_ )
{
#if KERNEL_USE_TIMEOUTS
    Semaphore_Pend_i( pstSem_, 0);
#else
    Semaphore_Pend_i( pstSem_ );
#endif
}

#if KERNEL_USE_TIMEOUTS
//---------------------------------------------------------------------------	
K_BOOL Semaphore_TimedPend( Semaphore_t *pstSem_, K_ULONG ulWaitTimeMS_ )
{
    return Semaphore_Pend_i( pstSem_, ulWaitTimeMS_ );
}
#endif

//---------------------------------------------------------------------------
K_USHORT Semaphore_GetCount( Semaphore_t *pstSem_ )
{
	K_USHORT usRet;
	CS_ENTER();
	usRet = pstSem_->m_usValue;
	CS_EXIT();
	return usRet;
}

#endif
