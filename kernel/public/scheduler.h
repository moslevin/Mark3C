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

    \file   scheduler.h    

    \brief  Thread_t scheduler function declarations
    
    This scheduler implements a very flexible type of scheduling, which has
    become the defacto industry standard when it comes to real-time operating
    systems.  This scheduling mechanism is referred to as priority round-
    robin.
    
    From the name, there are two concepts involved here:
        
    1)  Priority scheduling:
    
    Threads are each assigned a priority, and the thread with the highest 
    priority which is ready to run gets to execute.  

    2)  Round-robin scheduling:
    
    Where there are multiple ready threads at the highest-priority level,
    each thread in that group gets to share time, ensuring that progress is
    made.
    
    The scheduler uses an array of ThreadList_t objects to provide the necessary
    housekeeping required to keep track of threads at the various priorities.
    As s result, the scheduler contains one ThreadList_t per priority, with an
    additional list to manage the storage of threads which are in the 
    "stopped" state (either have been stopped, or have not been started yet).
   
*/

#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "kerneltypes.h"
#include "thread.h"
#include "threadport.h"


#ifdef __cplusplus
    extern "C" {
#endif

#define NUM_PRIORITIES              (8)     //!< Defines the maximum number of thread priorities supported in the scheduler
//---------------------------------------------------------------------------

extern volatile Thread_t *g_pstNext;
extern Thread_t *g_pstCurrent;
//---------------------------------------------------------------------------
extern K_BOOL m_bEnabled;           //! Scheduler's state - enabled or disabled

//---------------------------------------------------------------------------
/*!
    \brief Scheduler_Init

    Intiailize the scheduler, must be called before use.
*/
void Scheduler_Init( void );

//---------------------------------------------------------------------------
/*!
    \brief Scheduler_Schedule

    Run the scheduler, determines the next thread to run based on the
    current state of the threads.  Note that the next-thread chosen
    from this function is only valid while in a critical section.
*/
void Scheduler_Schedule( void );

//---------------------------------------------------------------------------
/*!
    \brief Scheduler_Add

    Add a thread to the scheduler at its current priority level.

    \param pstThread_ Pointer to the thread to add to the scheduler
*/
void Scheduler_Add(Thread_t *pstThread_);

//---------------------------------------------------------------------------
/*!
    \brief Scheduler_Remove

    Remove a thread from the scheduler at its current priority level.

    \param pstThread_ Pointer to the thread to be removed from the
           scheduler
*/
void Scheduler_Remove(Thread_t *pstThread_);

//---------------------------------------------------------------------------
/*!
    \brief Scheduler_SetScheduler

    Set the active state of the scheduler.  When the scheduler is
    disabled, the *next thread* is never set; the currently
    running thread will run forever until the scheduler is enabled
    again.  Care must be taken to ensure that we don't end up
    trying to block while the scheduler is disabled, otherwise the
    system ends up in an unusable state.

    \param bEnable_ true to enable, false to disable the scheduler
*/
K_BOOL Scheduler_SetScheduler(K_BOOL bEnable_);

//---------------------------------------------------------------------------
/*!
    \brief Scheduler_GetThreadList

    Return the pointer to the active list of threads that are at the
    given priority level in the scheduler.

    \param ucPriority_ Priority level of

    \return Pointer to the ThreadList_t for the given priority level
*/
ThreadList_t *Scheduler_GetThreadList( K_UCHAR ucPriority_ );

//---------------------------------------------------------------------------
/*!
    \brief Scheduler_GetCurrentThread

    Return the pointer to the currently-running thread.

    \return Pointer to the currently-running thread
*/
#define Scheduler_GetCurrentThread() ( g_pstCurrent )

//---------------------------------------------------------------------------
/*!
    \brief Scheduler_GetNextThread

    Return the pointer to the thread that should run next, according
    to the last run of the scheduler.

    \return Pointer to the next-running thread
*/
#define Scheduler_GetNextThread() ( g_pstNext )

//---------------------------------------------------------------------------
/*!
    \brief Scheduler_IsEnabled

    Return the current state of the scheduler - whether or not scheudling
    is enabled or disabled.

    \return true - scheduler enabled, false - disabled
*/
#define Scheduler_IsEnabled() ( m_bEnabled )

#ifdef __cplusplus
    }
#endif

#endif

