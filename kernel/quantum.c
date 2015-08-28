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

    \file   quantum.cpp

    \brief  Thread_t Quantum Implementation for Round-Robin Scheduling

*/

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "thread.h"
#include "timerlist.h"
#include "quantum.h"
#include "kerneldebug.h"
#include "kernelaware.h"
//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	QUANTUM_C       //!< File ID used in kernel trace calls

#if KERNEL_USE_QUANTUM

//---------------------------------------------------------------------------
static volatile K_BOOL bAddQuantumTimer;	// Indicates that a timer add is pending

//---------------------------------------------------------------------------
static Timer_t   stQuantumTimer;	// The global timernodelist_t object
static K_UCHAR bActive;
static K_UCHAR bInTimer;
//---------------------------------------------------------------------------
/*!
 * \brief QuantumCallback
 *
 * This is the timer callback that is invoked whenever a thread has exhausted
 * its current execution quantum and a new thread must be chosen from within
 * the same priority level.
 *
 * \param pstThread_ Pointer to the thread currently executing
 * \param pvData_ Unused in this context.
 */
static void QuantumCallback(Thread_t *pstThread_, void *pvData_)
{
    // Validate thread pointer, check that source/destination match (it's
    // in its real priority list).  Also check that this thread was part of
    // the highest-running priority level.
    if ( Thread_GetPriority( pstThread_ ) >=
         Thread_GetPriority( Scheduler_GetCurrentThread() ) )
    {
        ThreadList_t *pstList = Thread_GetCurrent( pstThread_ );
        if ( LinkList_GetHead( (LinkList_t*)pstList )
             != LinkList_GetTail( (LinkList_t*)pstList ) )
        {
            bAddQuantumTimer = true;
            CircularLinkList_PivotForward( (CircularLinkList_t*)pstList );
        }
    }
}

//---------------------------------------------------------------------------
void Quantum_SetTimer(Thread_t *pstThread_)
{
    Timer_SetIntervalMSeconds( &stQuantumTimer, Thread_GetQuantum( pstThread_ ) );
    Timer_SetFlags( &stQuantumTimer, TIMERLIST_FLAG_ONE_SHOT );
    Timer_SetData( &stQuantumTimer, NULL );
    Timer_SetCallback( &stQuantumTimer, (TimerCallback_t)QuantumCallback );
    Timer_SetOwner( &stQuantumTimer, pstThread_ );
}

//---------------------------------------------------------------------------
void Quantum_AddThread( Thread_t *pstThread_ )
{
    if (bActive
#if KERNEL_USE_IDLE_FUNC
            || (pstThread_ == Kernel_GetIdleThread())
#endif
       )
	{
		return;	
	}		
	
	// If this is called from the timer callback, queue a timer add...
	if (bInTimer)
	{
		bAddQuantumTimer = true;
		return;
	}
	
    // If this isn't the only thread in the list.
    ThreadList_t *pstOwner = Thread_GetCurrent( pstThread_ );
    if ( LinkList_GetHead( (LinkList_t*)pstOwner ) !=
           LinkList_GetTail( (LinkList_t*)pstOwner ) )
    {
        Quantum_SetTimer( pstThread_ );
        TimerScheduler_Add( &stQuantumTimer );
		bActive = 1;
    }    
}

//---------------------------------------------------------------------------
void Quantum_RemoveThread( void )
{
	if (!bActive)
	{
		return;
	}		

    // Cancel the current timer
    TimerScheduler_Remove( &stQuantumTimer );
	bActive = 0;
}

//---------------------------------------------------------------------------
void Quantum_UpdateTimer( void )
{
    // If we have to re-add the quantum timer (more than 2 threads at the 
    // high-priority level...)
    if (bAddQuantumTimer)
    {
        // Trigger a thread yield - this will also re-schedule the 
		// thread *and* reset the round-robin scheduler. 
        Thread_Yield();
		bAddQuantumTimer = false;		
    }    
}
//---------------------------------------------------------------------------
void Quantum_SetInTimer( void )
{
    bInTimer = true;
}

//---------------------------------------------------------------------------
void Quantum_ClearInTimer(void)
{
    bInTimer = false;
}


#endif //KERNEL_USE_QUANTUM
