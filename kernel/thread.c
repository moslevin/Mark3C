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

    \file   thread.cpp

    \brief  Platform-Independent thread class Definition

*/

#define INLINE

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "thread.h" 
#include "scheduler.h"
#include "kernelswi.h"
#include "timerlist.h"
#include "ksemaphore.h"
#include "quantum.h"
#include "kernel.h"
#include "kerneldebug.h"

//---------------------------------------------------------------------------
#if defined __FILE_ID__
    #undef __FILE_ID__
#endif
#define __FILE_ID__     THREAD_C       //!< File ID used in kernel trace calls

//---------------------------------------------------------------------------
void Thread_Init(   Thread_t *pstThread_,
                    K_WORD *pwStack_,
                    K_USHORT usStackSize_,
                    K_UCHAR ucPriority_,
                    ThreadEntry_t pfEntryPoint_,
                    void *pvArg_ )
{
    static K_UCHAR ucThreadID = 0;

    KERNEL_ASSERT( pwStack_ );
    KERNEL_ASSERT( pfEntryPoint_ );
    
    LinkListNode_Clear( (LinkListNode_t*)pstThread_ );

    pstThread_->m_ucThreadID = ucThreadID++;
    
    KERNEL_TRACE_1( STR_STACK_SIZE_1, usStackSize_ );
    KERNEL_TRACE_1( STR_PRIORITY_1, (K_UCHAR)ucPriority_ );
    KERNEL_TRACE_1( STR_THREAD_ID_1, (K_USHORT)pstThread_->m_ucThreadID );
    KERNEL_TRACE_1( STR_ENTRYPOINT_1, (K_USHORT)pfEntryPoint_ );
    
    // Initialize the thread parameters to their initial values.
    pstThread_->m_pwStack = pwStack_;
    pstThread_->m_pwStackTop = TOP_OF_STACK(pwStack_, usStackSize_);
    
    pstThread_->m_usStackSize = usStackSize_;
    
#if KERNEL_USE_QUANTUM    
    pstThread_->m_usQuantum = THREAD_QUANTUM_DEFAULT;
#endif

    pstThread_->m_ucPriority = ucPriority_ ;
    pstThread_->m_ucCurPriority = pstThread_->m_ucPriority;
    pstThread_->m_pfEntryPoint = pfEntryPoint_;
    pstThread_->m_pvArg = pvArg_;
    pstThread_->m_eState = THREAD_STATE_STOP;
    
#if KERNEL_USE_THREADNAME    
    pstThread_->m_szName = NULL;
#endif
#if KERNEL_USE_TIMERS
	Timer_Init( &(pstThread_->m_clTimer) );
#endif

    // Call CPU-specific stack initialization
    ThreadPort_InitStack( pstThread_ );
    
    // Add to the global "stop" list.
    CS_ENTER();
    pstThread_->m_pstOwner = Scheduler_GetThreadList(pstThread_->m_ucPriority);
    pstThread_->m_pstCurrent = Scheduler_GetStopList();
    ThreadList_Add( pstThread_->m_pstCurrent, pstThread_ );
	CS_EXIT();
}

//---------------------------------------------------------------------------
void Thread_Start( Thread_t *pstThread_ )
{
    // Remove the thread from the scheduler's "stopped" list, and add it 
    // to the scheduler's ready list at the proper priority.
    KERNEL_TRACE_1( STR_THREAD_START_1, (K_USHORT)pstThread_->m_ucThreadID );
    
    CS_ENTER();
    ThreadList_Remove( Scheduler_GetStopList(), pstThread_ );
    Scheduler_Add(pstThread_);
    pstThread_->m_pstOwner = Scheduler_GetThreadList(pstThread_->m_ucPriority);
    pstThread_->m_pstCurrent = pstThread_->m_pstOwner;
    pstThread_->m_eState = THREAD_STATE_READY;

#if KERNEL_USE_QUANTUM
    if ( Thread_GetCurPriority( pstThread_ ) >= 
		 Thread_GetCurPriority( Scheduler_GetCurrentThread() ) )
    {
        // Deal with the thread Quantum
        Quantum_RemoveThread();
        Quantum_AddThread(pstThread_);
    }
#endif

    if (Kernel_IsStarted())
    {
        if ( Thread_GetCurPriority( pstThread_ ) >= 
			 Thread_GetCurPriority( Scheduler_GetCurrentThread() ) )
        {
            Thread_Yield();
        }
    }
    CS_EXIT();
}

//---------------------------------------------------------------------------
void Thread_Stop( Thread_t *pstThread_ )
{
    K_BOOL bReschedule = 0;

    CS_ENTER();

    // If a thread is attempting to stop itself, ensure we call the scheduler
    if (pstThread_ == Scheduler_GetCurrentThread())
    {
        bReschedule = true;
    }

    // Add this thread to the stop-list (removing it from active scheduling)
    // Remove the thread from scheduling
    if (pstThread_->m_eState == THREAD_STATE_READY)
    {
        Scheduler_Remove(pstThread_);
    }
    else if (pstThread_->m_eState == THREAD_STATE_BLOCKED)
    {
		ThreadList_Remove( pstThread_->m_pstCurrent, pstThread_ );        
    }

    pstThread_->m_pstOwner = Scheduler_GetStopList();
    pstThread_->m_pstCurrent = pstThread_->m_pstOwner;
	ThreadList_Add( pstThread_->m_pstOwner, pstThread_ );
    
    pstThread_->m_eState = THREAD_STATE_STOP;

#if KERNEL_USE_TIMERS
    // Just to be safe - attempt to remove the thread's timer
    // from the timer-scheduler (does no harm if it isn't
    // in the timer-list)
    TimerScheduler_Remove(&pstThread_->m_clTimer);
#endif

    CS_EXIT();

    if (bReschedule)
    {
        Thread_Yield();
    }
}

#if KERNEL_USE_DYNAMIC_THREADS
//---------------------------------------------------------------------------
void Thread_Exit( Thread_t *pstThread_ )
{
    K_BOOL bReschedule = 0;
    
    KERNEL_TRACE_1( STR_THREAD_EXIT_1, pstThread_->m_ucThreadID );
    
    CS_ENTER();
    
    // If this thread is the actively-running thread, make sure we run the
    // scheduler again.
    if (pstThread_ == Scheduler_GetCurrentThread())
    {
        bReschedule = 1;            
    }
    
    // Remove the thread from scheduling
    if (pstThread_->m_eState == THREAD_STATE_READY)
    {
        Scheduler_Remove(pstThread_);
    }
    else if (pstThread_->m_eState == THREAD_STATE_BLOCKED)
    {
		ThreadList_Remove( pstThread_->m_pstCurrent, pstThread_ );
    }

    pstThread_->m_pstCurrent = 0;
    pstThread_->m_pstOwner = 0;
    pstThread_->m_eState = THREAD_STATE_EXIT;

    // We've removed the thread from scheduling, but interrupts might
    // trigger checks against this thread's currently priority before
    // we get around to scheduling new threads.  As a result, set the
    // priority to idle to ensure that we always wind up scheduling
    // new threads.
    pstThread_->m_ucCurPriority = 0;
    pstThread_->m_ucPriority = 0;

#if KERNEL_USE_TIMERS
    // Just to be safe - attempt to remove the thread's timer
    // from the timer-scheduler (does no harm if it isn't
    // in the timer-list)
    TimerScheduler_Remove(&pstThread_->m_clTimer);
#endif

    CS_EXIT();
    
    if (bReschedule) 
    {
        // Choose a new "next" thread if we must
        Thread_Yield();
    }
}
#endif

#if KERNEL_USE_SLEEP
//---------------------------------------------------------------------------
//! This callback is used to wake up a thread once the interval has expired
static void ThreadSleepCallback( Thread_t *pstOwner_, void *pvData_ )
{
    Semaphore_t *pstSemaphore = (Semaphore_t*)(pvData_);
    // Post the Semaphore_t, which will wake the sleeping thread.
    Semaphore_Post( pstSemaphore );
}

//---------------------------------------------------------------------------
void Thread_Sleep( K_ULONG ulTimeMs_)
{    
    Semaphore_t clSemaphore;
    Timer_t *pstTimer = Thread_GetTimer( g_pstCurrent );

    // Create a Semaphore_t that this thread will block on
	Semaphore_Init( &clSemaphore, 0, 1 );
    
    // Create a one-shot timer that will call a callback that posts the 
    // Semaphore_t, waking our thread.
	Timer_Init( pstTimer );
	Timer_SetIntervalMSeconds( pstTimer, ulTimeMs_ );
	Timer_SetCallback( pstTimer, ThreadSleepCallback );
	Timer_SetData( pstTimer, (void*)&clSemaphore );
	Timer_SetFlags( pstTimer, TIMERLIST_FLAG_ONE_SHOT );
	
    // Add the new timer to the timer scheduler, and block the thread
    TimerScheduler_Add(pstTimer);
	Semaphore_Pend( &clSemaphore );    
}

//---------------------------------------------------------------------------
void Thread_USleep( K_ULONG ulTimeUs_)
{    
    Semaphore_t clSemaphore;
    Timer_t *pstTimer = Thread_GetTimer( g_pstCurrent );

    // Create a Semaphore_t that this thread will block on
    Semaphore_Init( &clSemaphore, 0, 1 );
        
    // Create a one-shot timer that will call a callback that posts the
    // Semaphore_t, waking our thread.
    Timer_Init( pstTimer );
    Timer_SetIntervalUSeconds( pstTimer, ulTimeUs_ );
    Timer_SetCallback( pstTimer, ThreadSleepCallback );
    Timer_SetData( pstTimer, (void*)&clSemaphore );
    Timer_SetFlags( pstTimer, TIMERLIST_FLAG_ONE_SHOT );
        
    // Add the new timer to the timer scheduler, and block the thread
    TimerScheduler_Add(pstTimer);
    Semaphore_Pend( &clSemaphore );
}
#endif // KERNEL_USE_SLEEP

//---------------------------------------------------------------------------
K_USHORT Thread_GetStackSlack( Thread_t *pstThread_ )
{
    K_USHORT usCount = 0;
    
    CS_ENTER();
    
    //!! ToDo: Take into account stacks that grow up
    for (usCount = 0; usCount < pstThread_->m_usStackSize; usCount++)
    {
        if (pstThread_->m_pwStack[usCount] != 0xFF)
        {
            break;
        }
    }
    
    CS_EXIT();
    
    return usCount;
}

//---------------------------------------------------------------------------
void Thread_Yield( void )
{
    CS_ENTER();
    // Run the scheduler
    if (Scheduler_IsEnabled())
    {
        Scheduler_Schedule();

        // Only switch contexts if the new task is different than the old task
        if (Scheduler_GetCurrentThread() != Scheduler_GetNextThread())
        {
#if KERNEL_USE_QUANTUM
            // new thread scheduled.  Stop current quantum timer (if it exists),
            // and restart it for the new thread (if required).
            Quantum_RemoveThread();
            Quantum_AddThread((Thread_t*)g_pstNext);
#endif
            Thread_ContextSwitchSWI();
        }
    }
    else
    {
        Scheduler_QueueScheduler();
    }

    CS_EXIT();
}

//---------------------------------------------------------------------------
void Thread_SetPriorityBase( Thread_t *pstThread_, K_UCHAR ucPriority_ )
{
	 ThreadList_Remove( Thread_GetCurrent( pstThread_ ), pstThread_ );
	    
	 Thread_SetCurrent( pstThread_, Scheduler_GetThreadList(pstThread_->m_ucPriority) );
    
     ThreadList_Add( Thread_GetCurrent( pstThread_ ), pstThread_ );
}

//---------------------------------------------------------------------------
void Thread_SetPriority( Thread_t *pstThread_, K_UCHAR ucPriority_ )
{
    K_BOOL bSchedule = 0;

    CS_ENTER();
    // If this is the currently running thread, it's a good idea to reschedule
    // Or, if the new priority is a higher priority than the current thread's.
    if ((g_pstCurrent == pstThread_) || (ucPriority_ > Thread_GetPriority( g_pstCurrent )) )
    {
        bSchedule = 1;
    }
    Scheduler_Remove(pstThread_);
    CS_EXIT();

    pstThread_->m_ucCurPriority = ucPriority_;
    pstThread_->m_ucPriority = ucPriority_;
    
    CS_ENTER();    
    Scheduler_Add(pstThread_);
    CS_EXIT();
    
    if (bSchedule)
    {
        if (Scheduler_IsEnabled())
        {
            CS_ENTER();
            Scheduler_Schedule();
    #if KERNEL_USE_QUANTUM
            // new thread scheduled.  Stop current quantum timer (if it exists),
            // and restart it for the new thread (if required).
            Quantum_RemoveThread();
            Quantum_AddThread((Thread_t*)g_pstNext);
    #endif
            CS_EXIT();
            Thread_ContextSwitchSWI();
        }
        else
        {
            Scheduler_QueueScheduler();
        }
    }
}

//---------------------------------------------------------------------------
void Thread_InheritPriority( Thread_t *pstThread_, K_UCHAR ucPriority_ )
{    
    Thread_SetOwner(pstThread_, Scheduler_GetThreadList(ucPriority_));
    pstThread_->m_ucCurPriority = ucPriority_;
}

//---------------------------------------------------------------------------
void Thread_ContextSwitchSWI( void )
{
    // Call the context switch interrupt if the scheduler is enabled.
    if (Scheduler_IsEnabled() == 1)
    {        
        KERNEL_TRACE_1( STR_CONTEXT_SWITCH_1, (K_USHORT)Thread_GetID( (Thread_t*)g_pstNext ) );
        KernelSWI_Trigger();
    }
}

#if KERNEL_USE_IDLE_FUNC
//---------------------------------------------------------------------------
void Thread_InitIdle( Thread_t *pstThread_ )
{
    LinkListNode_Clear( (LinkListNode_t*)pstThread_ );

    pstThread_->m_ucPriority = 0;
    pstThread_->m_ucCurPriority = 0;
    pstThread_->m_pfEntryPoint = 0;
    pstThread_->m_pvArg = 0;
    pstThread_->m_ucThreadID = 255;
    pstThread_->m_eState = THREAD_STATE_READY;
#if KERNEL_USE_THREADNAME
    pstThread_->m_szName = "IDLE";
#endif
}
#endif
