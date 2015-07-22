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
    \file EventFlag_t.cpp
    \brief Event Flag Blocking Object/IPC-Object implementation.
*/

#include "mark3cfg.h"
#include "blocking.h"
#include "kernel.h"
#include "thread.h"
#include "eventflag.h"
#include "kernelaware.h"

#if KERNEL_USE_EVENTFLAG

#if KERNEL_USE_TIMEOUTS
#include "timerlist.h"

#if KERNEL_USE_TIMEOUTS
/*!
    * \brief WakeMe
    *
    * Wake the given thread, currently blocking on this object
    *
    * \param pstOwner_ Pointer to the owner thread to unblock.
    */
static void EventFlag_WakeMe( EventFlag_t *pstFlag_, Thread_t *pstOwner_);

/*!
    * \brief Wait_i
    *
    * Interal abstraction used to manage both timed and untimed wait operations
    *
    * \param usMask_ - 16-bit bitmask to block on
    * \param eMode_ - EVENT_FLAG_ANY:  Thread_t will block on any of the bits in the mask
    *               - EVENT_FLAG_ALL:  Thread_t will block on all of the bits in the mask
    * \param ulTimeMS_ - Time to block (in ms)
    *
    * \return Bitmask condition that caused the thread to unblock, or 0 on error or timeout
    */
static K_USHORT EventFlag_Wait_i( EventFlag_t *pstFlag_, K_USHORT usMask_, EventFlagOperation_t eMode_, K_ULONG ulTimeMS_);
#else
/*!
    * \brief Wait_i
    * Interal abstraction used to manage wait operations
    *
    * \param usMask_ - 16-bit bitmask to block on
    * \param eMode_ - EVENT_FLAG_ANY:  Thread_t will block on any of the bits in the mask
    *               - EVENT_FLAG_ALL:  Thread_t will block on all of the bits in the mask
    *
    * \return Bitmask condition that caused the thread to unblock.
    */
static K_USHORT EventFlag_Wait_i( EventFlag_t *pstFlag_, K_USHORT usMask_, EventFlagOperation_t eMode_);
#endif

/*!
  \brief Init Initializes the EventFlag_t object prior to use.
 */
void EventFlag_Init( EventFlag_t *pstFlag_ ) 
{ 
    pstFlag_->usSetMask = 0;
	ThreadList_Init( (ThreadList_t*)pstFlag_ );
}

//---------------------------------------------------------------------------
/*!
 * \brief TimedEventFlag_Callback
 *
 * This funciton is called whenever a timed event flag wait operation fails
 * in the time provided.  This function wakes the thread for which the timeout
 * was requested on the blocking call, sets the thread's expiry flags, and
 * reschedules if necessary.
 *
 * \param pstOwner_ Thread_t to wake
 * \param pvData_ Pointer to the event-flag object
 */
void TimedEventFlag_Callback(Thread_t *pstOwner_, void *pvData_)
{
    EventFlag_t *pstEventFlag = (EventFlag_t*)(pvData_);

	EventFlag_WakeMe( pstEventFlag, pstOwner_ );
	Thread_SetExpired( pstOwner_, true );
	Thread_SetEventFlagMask( pstOwner_, 0 );
	
	if (Thread_GetCurPriority( pstOwner_ ) >=
	    Thread_GetCurPriority( Scheduler_GetCurrentThread() )) 
	{
        Thread_Yield();
    }
}

//---------------------------------------------------------------------------
void EventFlag_WakeMe( EventFlag_t *pstFlag_, Thread_t *pstChosenOne_)
{
    BlockingObject_UnBlock( pstChosenOne_ );
}
#endif

//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
    K_USHORT EventFlag_Wait_i( EventFlag_t *pstFlag_, K_USHORT usMask_, EventFlagOperation_t eMode_, K_ULONG ulTimeMS_)
#else
    K_USHORT EventFlag_Wait_i( EventFlag_t *pstFlag_, K_USHORT usMask_, EventFlagOperation_t eMode_)
#endif
{
    K_BOOL bThreadYield = false;
    K_BOOL bMatch = false;

#if KERNEL_USE_TIMEOUTS
    Timer_t stEventTimer;
    K_BOOL bUseTimer = false;
#endif

    // Ensure we're operating in a critical section while we determine
    // whether or not we need to block the current thread on this object.
    CS_ENTER();

    // Check to see whether or not the current mask matches any of the
    // desired bits.
	Thread_SetEventFlagMask( g_pstCurrent, usMask_ );

    if ((eMode_ == EVENT_FLAG_ALL) || (eMode_ == EVENT_FLAG_ALL_CLEAR))
    {
        // Check to see if the flags in their current state match all of
        // the set flags in the event flag group, with this mask.
        if ((pstFlag_->usSetMask & usMask_) == usMask_)
        {
            bMatch = true;
			Thread_SetEventFlagMask( g_pstCurrent, usMask_ );
        }
    }
    else if ((eMode_ == EVENT_FLAG_ANY) || (eMode_ == EVENT_FLAG_ANY_CLEAR))
    {
        // Check to see if the existing flags match any of the set flags in
        // the event flag group  with this mask
        if (pstFlag_->usSetMask & usMask_)
        {
            bMatch = true;
            Thread_SetEventFlagMask( g_pstCurrent, pstFlag_->usSetMask & usMask_);
        }
    }

    // We're unable to match this pattern as-is, so we must block.
    if (!bMatch)
    {
        // Reset the current thread's event flag mask & mode
		Thread_SetEventFlagMask( g_pstCurrent, usMask_ );
		Thread_SetEventFlagMode( g_pstCurrent, eMode_ );

#if KERNEL_USE_TIMEOUTS
        if (ulTimeMS_)
        {
			Thread_SetExpired( g_pstCurrent, false );
            
            Timer_Init( &stEventTimer );
            Timer_Start( &stEventTimer, false, ulTimeMS_, TimedEventFlag_Callback, (void*)pstFlag_);
            bUseTimer = true;
        }
#endif

        // Add the thread to the object's block-list.
        BlockingObject_Block( (ThreadList_t*)pstFlag_, g_pstCurrent);

        // Trigger that
        bThreadYield = true;
    }

    // If bThreadYield is set, it means that we've blocked the current thread,
    // and must therefore rerun the scheduler to determine what thread to
    // switch to.
    if (bThreadYield)
    {
        // Switch threads immediately
        Thread_Yield();
    }

    // Exit the critical section and return back to normal execution
    CS_EXIT();

    //!! If the Yield operation causes a new thread to be chosen, there will
    //!! Be a context switch at the above CS_EXIT().  The original calling
    //!! thread will not return back until a matching SetFlags call is made
    //!! or a timeout occurs.
#if KERNEL_USE_TIMEOUTS
    if (bUseTimer && bThreadYield)
    {
        Timer_Stop( &stEventTimer );
    }
#endif

    return Thread_GetEventFlagMask( g_pstCurrent );
}

//---------------------------------------------------------------------------
K_USHORT EventFlag_Wait( EventFlag_t *pstFlag_, K_USHORT usMask_, EventFlagOperation_t eMode_)
{
#if KERNEL_USE_TIMEOUTS
    return EventFlag_Wait_i( pstFlag_, usMask_, eMode_, 0);
#else
    return EventFlag_Wait_i( pstFlag_, usMask_, eMode_);
#endif
}

#if KERNEL_USE_TIMEOUTS
//---------------------------------------------------------------------------
K_USHORT EventFlag_TimedWait( EventFlag_t *pstFlag_, K_USHORT usMask_, EventFlagOperation_t eMode_, K_ULONG ulTimeMS_)
{
    return EventFlag_Wait_i( pstFlag_, usMask_, eMode_, ulTimeMS_);
}
#endif

//---------------------------------------------------------------------------
void EventFlag_Set( EventFlag_t *pstFlag_, K_USHORT usMask_)
{
    Thread_t *pstPrev;
    Thread_t *pstCurrent;
    K_BOOL bReschedule = false;
    K_USHORT usNewMask;

    CS_ENTER();

    // Walk through the whole block list, checking to see whether or not
    // the current flag set now matches any/all of the masks and modes of
    // the threads involved.

    pstFlag_->usSetMask |= usMask_;
    usNewMask = pstFlag_->usSetMask;

    // Start at the head of the list, and iterate through until we hit the
    // "head" element in the list again.  Ensure that we handle the case where
    // we remove the first or last elements in the list, or if there's only
    // one element in the list.
    pstCurrent = (Thread_t*)(LinkList_GetHead( (LinkList_t*)pstFlag_ ));

    // Do nothing when there are no objects blocking.
    if (pstCurrent)
    {
        // First loop - process every thread in the block-list and check to
        // see whether or not the current flags match the event-flag conditions
        // on the thread.
        do
        {
            pstPrev = pstCurrent;
            pstCurrent = (Thread_t*)(LinkListNode_GetNext( (LinkListNode_t*)pstCurrent ) );

            // Read the thread's event mask/mode
            K_USHORT usThreadMask = Thread_GetEventFlagMask( pstPrev );
            EventFlagOperation_t eThreadMode = Thread_GetEventFlagMode( pstPrev );

            // For the "any" mode - unblock the blocked threads if one or more bits
            // in the thread's bitmask match the object's bitmask
            if ((EVENT_FLAG_ANY == eThreadMode) || (EVENT_FLAG_ANY_CLEAR == eThreadMode))
            {
                if (usThreadMask & pstFlag_->usSetMask)
                {
                    Thread_SetEventFlagMode( pstPrev, EVENT_FLAG_PENDING_UNBLOCK );
                    Thread_SetEventFlagMask( pstPrev, pstFlag_->usSetMask & usThreadMask );
                    bReschedule = true;

                    // If the "clear" variant is set, then clear the bits in the mask
                    // that caused the thread to unblock.
                    if (EVENT_FLAG_ANY_CLEAR == eThreadMode)
                    {
                        usNewMask &=~ (usThreadMask & usMask_);
                    }
                }
            }
            // For the "all" mode, every set bit in the thread's requested bitmask must
            // match the object's flag mask.
            else if ((EVENT_FLAG_ALL == eThreadMode) || (EVENT_FLAG_ALL_CLEAR == eThreadMode))
            {
                if ((usThreadMask & pstFlag_->usSetMask) == usThreadMask)
                {
                    Thread_SetEventFlagMode( pstPrev, EVENT_FLAG_PENDING_UNBLOCK );
                    Thread_SetEventFlagMask( pstPrev, usThreadMask);
                    bReschedule = true;

                    // If the "clear" variant is set, then clear the bits in the mask
                    // that caused the thread to unblock.
                    if (EVENT_FLAG_ALL_CLEAR == eThreadMode)
                    {
                        usNewMask &=~ (usThreadMask & usMask_);
                    }
                }
            }
        }
        // To keep looping, ensure that there's something in the list, and
        // that the next item isn't the head of the list.
        while (pstPrev != (Thread_t*)LinkList_GetTail( (LinkList_t*)pstFlag_ ) );

        // Second loop - go through and unblock all of the threads that
        // were tagged for unblocking.
        pstCurrent = (Thread_t*)(LinkList_GetHead( (LinkList_t*)pstFlag_ ));
        K_BOOL bIsTail = false;
        do
        {
            pstPrev = pstCurrent;
            pstCurrent = (Thread_t*)(LinkListNode_GetNext( (LinkListNode_t*)pstCurrent ));

            // Check to see if this is the condition to terminate the loop
            if (pstPrev == (Thread_t*)(LinkList_GetTail( (LinkList_t*)pstFlag_ )))
            {
                bIsTail = true;
            }

            // If the first pass indicated that this thread should be
            // unblocked, then unblock the thread
            if (Thread_GetEventFlagMode( pstPrev ) == EVENT_FLAG_PENDING_UNBLOCK)
            {
                BlockingObject_UnBlock( pstPrev );
            }
        }
        while (!bIsTail);
    }

    // If we awoke any threads, re-run the scheduler
    if (bReschedule)
    {
        Thread_Yield();
    }

    // Update the bitmask based on any "clear" operations performed along
    // the way
    pstFlag_->usSetMask = usNewMask;

    // Restore interrupts - will potentially cause a context switch if a
    // thread is unblocked.
    CS_EXIT();
}

//---------------------------------------------------------------------------
void EventFlag_Clear( EventFlag_t *pstFlag_, K_USHORT usMask_)
{
    // Just clear the bitfields in the local object.
    CS_ENTER();
    pstFlag_->usSetMask &= ~usMask_;
    CS_EXIT();
}

//---------------------------------------------------------------------------
K_USHORT EventFlag_GetMask( EventFlag_t *pstFlag_ )
{
    // Return the presently held event flag values in this object.  Ensure
    // we get this within a critical section to guarantee atomicity.
    K_USHORT usReturn;
    CS_ENTER();
    usReturn = pstFlag_->usSetMask;
    CS_EXIT();
    return usReturn;
}

#endif // KERNEL_USE_EVENTFLAG
