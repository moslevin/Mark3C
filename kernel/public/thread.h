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

    \file   thread.h    

    \brief  Platform independent thread object declarations
    
    Threads are an atomic unit of execution, and each instance of the thread
    object represents an instance of a program running of the processor.
    The Thread_t is the fundmanetal user-facing object in the kernel - it
    is what makes multiprocessing possible from application code.
    
    In Mark3, threads each have their own context - consisting of a stack, and
    all of the registers required to multiplex a processor between multiple 
    threads.
    
    The Thread_t object inherits directly from the LinkListNode_t object to 
    facilitate efficient thread management using Double, or Double-Circular
    linked lists.
    
*/

#ifndef __THREAD_H__
#define __THREAD_H__

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "ll.h"
#include "threadlist.h"
#include "scheduler.h"
#include "threadport.h"
#include "quantum.h"

#ifdef __cplusplus
    extern "C" {
#endif

#ifndef INLINE
# define INLINE extern inline
#endif


//---------------------------------------------------------------------------
/*!
 * \brief Thread_Init
 *
 * Initialize a thread prior to its use.  Initialized threads are
 * placed in the stopped state, and are not scheduled until the
 * thread's start method has been invoked first.
 *
 * \param pstThread_    Pointer to the thread to initialize
 * \param paucStack_    Pointer to the stack to use for the thread
 * \param usStackSize_  Size of the stack (in bytes)
 * \param ucPriority_   Priority of the thread (0 = idle, 7 = max)
 * \param pfEntryPoint_ This is the function that gets called when the
 *                      thread is started
 * \param pvArg_        Pointer to the argument passed into the thread's
 *                        entrypoint function.
 */
void Thread_Init(Thread_t *pstThread_,
                 K_WORD *paucStack_,
                 K_USHORT usStackSize_,
                 K_UCHAR ucPriority_,
                 ThreadEntry_t pfEntryPoint_,
                 void *pvArg_ );


//---------------------------------------------------------------------------
/*!
 * \brief Thread_Start
 *
 *  Start the thread - remove it from the stopped list, add it to the
 *  scheduler's list of threads (at the thread's set priority), and
 *  continue along.
 *
 * \param pstThread_ Pointer to the thread to start
 */
void Thread_Start( Thread_t *pstThread_ );

//---------------------------------------------------------------------------
/*!
 * \brief Thread_Stop
 *
 * Stop a thread that's actively scheduled without destroying its
 * stacks.  Stopped threads can be restarted using the Start() API.
 *
 * \param pstThread_ Pointer to the thread to start
 *
 */
void Thread_Stop( Thread_t *pstThread_ );

#if KERNEL_USE_THREADNAME    

//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetName
 *
 * Set the name of the thread - this is purely optional, but can be
 * useful when identifying issues that come along when multiple threads
 * are at play in a system.
 *
 * \param pstThread_ Pointer to the thread to modify
 * \param szName_ Char string containing the thread name
 */
#define Thread_SetName( pstThread_, szName_) ( ((Thread_t*)pstThread_)->szName = szName_ )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetName
 *
 * Return back a pointer to the thread's name to the user.
 *
 * \param pstThread_ Pointer to the thread to inspect
 * \return Pointer to the name of the thread.  If this is not set, will be NULL.
 */
#define Thread_GetName( pstThread_ ) ( ((Thread_t*)pstThread_)->szName )
#endif

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetOwner
 *
 * Return the ThreadList_t where the thread belongs when it's in the
 * active/ready state in the scheduler.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \return Pointer to the Thread_t's owner list
 */
#define Thread_GetOwner( pstThread_ ) ( ((Thread_t*)pstThread_)->pstOwner )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetCurrent
 *
 * Return the ThreadList_t where the thread is currently located
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \return Pointer to the thread's current list
 */
#define Thread_GetCurrent( pstThread_ ) ( ((Thread_t*)pstThread_)->pstCurrent )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetPriority
 *
 * Return the priority of the current thread
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \return Priority of the current thread
 */
#define Thread_GetPriority( pstThread_ )  (((Thread_t*)pstThread_)->ucPriority)

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetCurPriority
 *
 * Return the priority of the current thread
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \return Priority of the current thread
 */
#define Thread_GetCurPriority( pstThread_ ) (((Thread_t*)pstThread_)->ucCurPriority)

#if KERNEL_USE_QUANTUM
//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetQuantum
 *
 *     Set the thread's round-robin execution quantum.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \param usQuantu Thread's execution quantum (in milliseconds)
 */
#define Thread_SetQuantum( pstThread_, usQuantu ) ( ((Thread_t*)pstThread_)->usQuantum = usQuantu )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetQuantum
 *
 *  Get the thread's round-robin execution quantum.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \return  The thread's round-robin timeslice quantum
 */
#define Thread_GetQuantum( pstThread_ ) ( ((Thread_t*)pstThread_)->usQuantum )
#endif

//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetCurrent
 * \param pstThread_ Pointer to the thread to access/modify
 * \param pstNewList_ Pointer to the threadlist to apply thread ownership
 */
#define Thread_SetCurrent( pstThread_, pstNewList_ ) ( ((Thread_t*)pstThread_)->pstCurrent = pstNewList_)

//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetOwner
 *
 * Set the thread's owner to the specified thread list
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \param pstNewList_ Pointer to the threadlist to apply thread ownership
 */
#define Thread_SetOwner( pstThread_, pstNewList_ ) ( ((Thread_t*)pstThread_)->pstOwner = pstNewList_ )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetPriority
 *
 *  Set the priority of the Thread_t (running or otherwise) to a different
 *  level.  This activity involves re-scheduling, and must be done so
 *  with due caution, as it may effect the determinism of the system.
 *
 *  This should *always* be called from within a critical section to
 *  prevent system issues.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \param ucPriority_
 */
void Thread_SetPriority( Thread_t *pstThread_, K_UCHAR ucPriority_);

//---------------------------------------------------------------------------
/*!
 * \brief Thread_InheritPriority
 *
 *   Allow the thread to run at a different priority level (temporarily)
 *   for the purpose of avoiding priority inversions.  This should
 *   only be called from within the implementation of blocking-objects.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \param ucPriority_  New Priority to boost the thread to temporarily
 */
void Thread_InheritPriority( Thread_t *pstThread_, K_UCHAR ucPriority_);

#if KERNEL_USE_DYNAMIC_THREADS
//---------------------------------------------------------------------------
/*!
 * \brief Thread_Exit
 *
 * Remove the thread from being scheduled again.  The thread is
 * effectively destroyed when this occurs.  This is extremely
 * useful for cases where a thread encounters an unrecoverable
 * error and needs to be restarted, or in the context of systems
 * where threads need to be created and destroyed dynamically.
 *
 * This must not be called on the idle thread.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 */
void Thread_Exit( Thread_t *pstThread_ );
#endif    

#if KERNEL_USE_SLEEP    
//---------------------------------------------------------------------------
/*!
 * \brief Thread_Sleep
 *
 *   Put the thread to sleep for the specified time (in milliseconds).
 *   Actual time slept may be longer (but not less than) the interval specified.
 *
 * \param ulTimeMs_ Time to sleep (in ms)
 */
void Thread_Sleep( K_ULONG ulTimeMs_);

//---------------------------------------------------------------------------
/*!
 * \brief Thread_USleep
 *
 * Put the thread to sleep for the specified time (in microseconds).
 * Actual time slept may be longer (but not less than) the interval specified.
 *
 * \param ulTimeUs_ Time to sleep (in microseconds)
 */
void Thread_USleep( K_ULONG ulTimeUs_);
#endif

//---------------------------------------------------------------------------
/*!
 * \brief Thread_Yield
 *
 *  Yield the thread - this forces the system to call the scheduler and
 *  determine what thread should run next.  This is typically used when
 *  threads are moved in and out of the scheduler.
 */
void Thread_Yield( void );

//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetID
 *
 *   Set an 8-bit ID to uniquely identify this thread.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \param ucID_ 8-bit Thread_t ID, set by the user
 */
#define Thread_SetID( pstThread_, ucID_ ) (	((Thread_t*)pstThread_)->ucThreadID = ucID_ )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetID
 *
 *     Return the 8-bit ID corresponding to this thread.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \return Thread_t's 8-bit ID, set by the user
 */
#define Thread_GetID( pstThread_ ) ( ((Thread_t*)pstThread_)->ucThreadID )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetStackSlack
 *
 * Performs a (somewhat lengthy) check on the thread stack to check the
 * amount of stack margin (or "slack") remaining on the stack. If you're
 * having problems with blowing your stack, you can run this function
 * at points in your code during development to see what operations
 * cause problems.  Also useful during development as a tool to optimally
 * size thread stacks.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \return The amount of slack (unused bytes) on the stack
 */
K_USHORT Thread_GetStackSlack( Thread_t *pstThread_ );

#if KERNEL_USE_EVENTFLAG
//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetEventFlagMask returns the thread's current event-flag mask,
 *        which is used in conjunction with the EventFlag_t blocking object
 *        type.
 * \param pstThread_ Pointer to the thread to access/modify
 * \return A copy of the thread's event flag mask
 */
#define Thread_GetEventFlagMask( pstThread_ ) (((Thread_t*)pstThread_)->usFlagMask)

//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetEventFlagMask  Sets the active event flag bitfield mask
 * \param pstThread_ Pointer to the thread to access/modify
 * \param usMask_ Binary mask value to set for this thread
 */
#define Thread_SetEventFlagMask( pstThread_, usMask_ ) (((Thread_t*)pstThread_)->usFlagMask = usMask_)

//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetEventFlagMode Sets the active event flag operation mode
 * \param eMode_ Event flag operation mode, defines the logical operator
 *               to apply to the event flag.
 */
#define Thread_SetEventFlagMode( pstThread_, eMode_ ) (((Thread_t*)pstThread_)->eFlagMode = eMode_ )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetEventFlagMode Returns the thread's event flag's operating mode
 * \return The thread's event flag mode.
 */
#define Thread_GetEventFlagMode( pstThread_ ) ( ((Thread_t*)pstThread_)->eFlagMode )
#endif

#if KERNEL_USE_TIMEOUTS || KERNEL_USE_SLEEP
//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetTimer
 * \param pstThread_ Pointer to the thread to access/modify
 * \return Return a pointer to the thread's timer object
 */
#define Thread_GetTimer( pstThread_ ) ( &((Thread_t*)pstThread_)->stTimer )
#endif
#if KERNEL_USE_TIMEOUTS

//---------------------------------------------------------------------------
/*!
 * \brief SetExpired
 *
 * Set the status of the current blocking call on the thread.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \param bExpired_ true - call expired, false - call did not expire
 */
#define Thread_SetExpired( pstThread_, bExpired_ ) ( ((Thread_t*)pstThread_)->bExpired = bExpired_ )

//---------------------------------------------------------------------------
/*!
 * \brief GetExpired
 *
 * Return the status of the most-recent blocking call on the thread.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 * \return true - call expired, false - call did not expire
 */
#define Thread_GetExpired( pstThread_ )	( ((Thread_t*)pstThread_)->bExpired )
#endif

#if KERNEL_USE_IDLE_FUNC

//---------------------------------------------------------------------------
/*!
 * \brief InitIdle Initialize this Thread_t object as the Kernel's idle
 *        thread.  There should only be one of these, maximum, in a
 *        given system.
 *
 * \param pstThread_ Pointer to the thread to access/modify
 */
void Thread_InitIdle( Thread_t *pstThread_ );
#endif

//---------------------------------------------------------------------------
/*!
 * \brief Thread_GetState Returns the current state of the thread to the
 *        caller.  Can be used to determine whether or not a thread
 *        is ready (or running), stopped, or terminated/exit'd.
 * \param pstThread_ Pointer to the thread to access/modify
 * \return ThreadState_t representing the thread's current state
 */
#define Thread_GetState( pstThread_ ) ( ((Thread_t*)pstThread_)->eState )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetState Set the thread's state to a new value.  This
 *        is only to be used by code within the kernel, and is not
 *        indended for use by an end-user.
 * \param pstThread_ Pointer to the thread to access/modify
 * \param eState_ New thread state to set.
 */
#define Thread_SetState( pstThread_, eState_ ) ( ((Thread_t*)pstThread_)->eState = eState_ )

//---------------------------------------------------------------------------
/*!
 * \brief Thread_ContextSwitchSWI
 *
 * This code is used to trigger the context switch interrupt.  Called
 * whenever the kernel decides that it is necessary to swap out the
 * current thread for the "next" thread.
 */
void Thread_ContextSwitchSWI( void );

//---------------------------------------------------------------------------
/*!
 * \brief Thread_SetPriorityBase
 * \param pstThread_ Pointer to the thread to access/modify
 * \param ucPriority_
 */
void Thread_SetPriorityBase( Thread_t *pstThread_, K_UCHAR ucPriority_);

#if KERNEL_USE_IDLE_FUNC
//---------------------------------------------------------------------------
/*!
    If the kernel is set up to use an idle function instead of an idle thread,
    we use a placeholder data structure to "simulate" the effect of having an
    idle thread in the system.  When cast to a Thread_t, this data structure will
    still result in GetPriority() calls being valid, which is all that is
    needed to support the tick-based/tickless times -- while saving a fairly
    decent chunk of RAM on a small micro.

    Note that this struct must have the same memory layout as the Thread_t object
    up to the last item.
*/
typedef struct
{
    LinkListNode_t *next;
    LinkListNode_t *prev;

    //! Pointer to the top of the thread's stack
    K_WORD *pwStackTop;

    //! Pointer to the thread's stack
    K_WORD *pwStack;

    //! Thread_t ID
    K_UCHAR ucThreadID;

    //! Default priority of the thread
    K_UCHAR ucPriority;

    //! Current priority of the thread (priority inheritence)
    K_UCHAR ucCurPriority;

    //! Enum indicating the thread's current state
    ThreadState_t eState;

#if KERNEL_USE_THREADNAME
    //! Thread_t name
    const K_CHAR *szName;
#endif

} FakeThread_t;
#endif


#ifdef __cplusplus
    }
#endif

#endif
