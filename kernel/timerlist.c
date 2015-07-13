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

    \file   timerlist.cpp

    \brief  Implements timer list processing algorithms, responsible for all
            timer tick and expiry logic.

*/

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "ll.h"
#include "timerlist.h"
#include "kerneltimer.h"
#include "threadport.h"
#include "kerneldebug.h"
#include "quantum.h"

//---------------------------------------------------------------------------
#if defined __FILE_ID__
    #undef __FILE_ID__
#endif
#define __FILE_ID__     TIMERLIST_C       //!< File ID used in kernel trace calls

#if KERNEL_USE_TIMERS

//---------------------------------------------------------------------------
/*!
    TimerList object - a doubly-linked-list of timer objects.
*/

// Inherit from DoubleLinkList_t -- must be first!!
static DoubleLinkList_t m_clTimerList;

//! The time (in system clock ticks) of the next wakeup event
static K_ULONG m_ulNextWakeup;

//! Whether or not the timer is active
static K_UCHAR m_bTimerActive;


//---------------------------------------------------------------------------
void TimerList_Init(void)
{
    m_bTimerActive = 0;    
    m_ulNextWakeup = 0;    
	LinkList_Init( (LinkList_t*)&m_clTimerList );
}

//---------------------------------------------------------------------------
void TimerList_Add(Timer_t *pstListNode_)
{
#if KERNEL_TIMERS_TICKLESS
    K_BOOL bStart = 0;
    K_LONG lDelta;
#endif

    CS_ENTER();

#if KERNEL_TIMERS_TICKLESS
    if (LinkList_GetHead( (LinkList_t*)&m_clTimerList ) == NULL)
    {
        bStart = 1;
    }
#endif

    LinkListNode_Clear( (LinkListNode_t*)pstListNode_ );
    DoubleLinkList_Add( (DoubleLinkList_t*)&m_clTimerList, (LinkListNode_t*)pstListNode_);
    
    // Set the initial timer value
    pstListNode_->m_ulTimeLeft = pstListNode_->m_ulInterval;    

#if KERNEL_TIMERS_TICKLESS
    if (!bStart)
    {
        // If the new interval is less than the amount of time remaining...
        lDelta = KernelTimer_TimeToExpiry() - pstListNode_->m_ulInterval;
    
        if (lDelta > 0)
        {
            // Set the new expiry time on the timer.
            m_ulNextWakeup = KernelTimer_SubtractExpiry((K_ULONG)lDelta);
        }
    }
    else    
    {
        m_ulNextWakeup = pstListNode_->m_ulInterval;
        KernelTimer_SetExpiry(m_ulNextWakeup);
        KernelTimer_Start();
    }
#endif

    // Set the timer as active.
    pstListNode_->m_ucFlags |= TIMERLIST_FLAG_ACTIVE;    
    CS_EXIT();
}

//---------------------------------------------------------------------------
void TimerList_Remove(Timer_t *pstLinkListNode_)
{
    CS_ENTER();
    
    DoubleLinkList_Remove( (DoubleLinkList_t*)&m_clTimerList, (LinkListNode_t*)pstLinkListNode_ );

#if KERNEL_TIMERS_TICKLESS
    if ( LinkList_GetHead( (LinkList_t*)&m_clTimerList ) == NULL )
    {
        KernelTimer_Stop();
    }
#endif
    
    CS_EXIT();
}

//---------------------------------------------------------------------------
void TimerList_Process(void)
{
#if KERNEL_TIMERS_TICKLESS
    K_ULONG ulNewExpiry;
    K_ULONG ulOvertime;
    K_BOOL bContinue;
#endif
    
    Timer_t *pstNode;
    Timer_t *pstPrev;

#if KERNEL_USE_QUANTUM
    Quantum_SetInTimer();
#endif
#if KERNEL_TIMERS_TICKLESS
    // Clear the timer and its expiry time - keep it running though
    KernelTimer_ClearExpiry();
    do 
    {        
#endif
        pstNode = (Timer_t*)LinkList_GetHead( (LinkList_t*)&m_clTimerList );
        pstPrev = NULL;

#if KERNEL_TIMERS_TICKLESS
        bContinue = 0;
        ulNewExpiry = MAX_TIMER_TICKS;
#endif

        // Subtract the elapsed time interval from each active timer.
        while (pstNode)
        {        
            // Active timers only...
            if (pstNode->m_ucFlags & TIMERLIST_FLAG_ACTIVE)
            {
                // Did the timer expire?
#if KERNEL_TIMERS_TICKLESS
                if (pstNode->m_ulTimeLeft <= m_ulNextWakeup)
#else
                pstNode->m_ulTimeLeft--;
                if (0 == pstNode->m_ulTimeLeft)
#endif
                {
                    // Yes - set the "callback" flag - we'll execute the callbacks later
                    pstNode->m_ucFlags |= TIMERLIST_FLAG_CALLBACK;
                                
                    if (pstNode->m_ucFlags & TIMERLIST_FLAG_ONE_SHOT)
                    {
                        // If this was a one-shot timer, deactivate the timer.
                        pstNode->m_ucFlags |= TIMERLIST_FLAG_EXPIRED;
                        pstNode->m_ucFlags &= ~TIMERLIST_FLAG_ACTIVE;                    
                    }
                    else
                    {
                        // Reset the interval timer.
                        //!ToDo - figure out if we need to deal with any overtime here.
                        // I think we're good though...                        
                        pstNode->m_ulTimeLeft = pstNode->m_ulInterval;
                        
#if KERNEL_TIMERS_TICKLESS
                        // If the time remaining (plus the length of the tolerance interval)
                        // is less than the next expiry interval, set the next expiry interval.
                        K_ULONG ulTmp = pstNode->m_ulTimeLeft + pstNode->m_ulTimerTolerance;

                        if (ulTmp < ulNewExpiry)
                        {
                            ulNewExpiry = ulTmp;
                        }
#endif
                    }
                }
#if KERNEL_TIMERS_TICKLESS
                else
                {
                    // Not expiring, but determine how K_LONG to run the next timer interval for.
                    pstNode->m_ulTimeLeft -= m_ulNextWakeup;
                    if (pstNode->m_ulTimeLeft < ulNewExpiry)
                    {
                        ulNewExpiry = pstNode->m_ulTimeLeft;
                    }
                }
#endif
            }
            pstNode = (Timer_t*)LinkListNode_GetNext( (LinkListNode_t*)pstNode );
        }
    
        // Process the expired timers callbacks.
        pstNode = (Timer_t*)LinkList_GetHead( (LinkList_t*)&m_clTimerList );
        while (pstNode)
        {
            pstPrev = NULL;
        
            // If the timer expired, run the callbacks now.
            if (pstNode->m_ucFlags & TIMERLIST_FLAG_CALLBACK)
            {
                // Run the callback. these callbacks must be very fast...
                pstNode->m_pfCallback( pstNode->m_pstOwner, pstNode->m_pvData );
                pstNode->m_ucFlags &= ~TIMERLIST_FLAG_CALLBACK;
            
                // If this was a one-shot timer, let's remove it.
                if (pstNode->m_ucFlags & TIMERLIST_FLAG_ONE_SHOT)
                {
                    pstPrev = pstNode;
                }
            }
            pstNode = (Timer_t*)LinkListNode_GetNext( (LinkListNode_t*)pstNode );
        
            // Remove one-shot-timers
            if (pstPrev)
            {
                TimerList_Remove( pstPrev );
            }        
        }    

#if KERNEL_TIMERS_TICKLESS
        // Check to see how much time has elapsed since the time we 
        // acknowledged the interrupt... 
        ulOvertime = KernelTimer_GetOvertime();
        
        if( ulOvertime >= ulNewExpiry ) {
            m_ulNextWakeup = ulOvertime;
            bContinue = 1;
        }
        
    // If it's taken longer to go through this loop than would take us to
    // the next expiry, re-run the timing loop

    } while (bContinue);

    // This timer elapsed, but there's nothing more to do...
    // Turn the timer off.
    if (ulNewExpiry >= MAX_TIMER_TICKS)
    {
        KernelTimer_Stop();
    }
    else 
    {
        // Update the timer with the new "Next Wakeup" value, plus whatever
        // overtime has accumulated since the last time we called this handler

        m_ulNextWakeup = KernelTimer_SetExpiry(ulNewExpiry + ulOvertime);
    }
#endif

#if KERNEL_USE_QUANTUM
    Quantum_ClearInTimer();
#endif
}


#endif //KERNEL_USE_TIMERS
