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

    \file   mutex.h    

    \brief  Mutual exclusion class declaration
    
    Resource locks are implemented using mutual exclusion semaphores (Mutex_t).
    Protected blocks can be placed around any resource that may only be accessed
    by one thread at a time.  If additional threads attempt to access the
    protected resource, they will be placed in a wait queue until the resource
    becomes available.  When the resource becomes available, the thread with
    the highest original priority claims the resource and is activated.
    Priority inheritance is included in the implementation to prevent priority
    inversion.  Always ensure that you claim and release your Mutex_t objects
    consistently, otherwise you may end up with a deadlock scenario that's
    hard to debug.

    \section MInit Initializing

    Initializing a Mutex_t object by calling:

    \code
    clMutex.Init();
    \endcode

    \section MUsage Resource protection example

    \code
    clMutex.Claim();
    ...
    <resource protected block>
    ...
    clMutex.Release();
    \endcode

*/
#ifndef __MUTEX_H_
#define __MUTEX_H_

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "blocking.h"

#if KERNEL_USE_MUTEX

#if KERNEL_USE_TIMEOUTS
#include "timerlist.h"
#endif

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
/*!
    Mutual-exclusion locks, based on BlockingObject.
*/
typedef struct
{
	// Inherit from BlockingObject -- must go first.
	ThreadList_t m_stList;
	
	K_UCHAR m_ucRecurse;    //!< The recursive lock-count when a Mutex_t is claimed multiple times by the same owner
	K_UCHAR m_bReady;       //!< State of the Mutex_t - true = ready, false = claimed
	K_UCHAR m_ucMaxPri;     //!< Maximum priority of thread in queue, used for priority inheritence
	Thread_t *m_pstOwner;     //!< Pointer to the thread that owns the Mutex_t (when claimed)
} Mutex_t;

//---------------------------------------------------------------------------
/*!
    \brief Mutex_Init
        
    Initialize a Mutex_t object for use - must call this function before using
    the object.
*/
void Mutex_Init( Mutex_t *pstMutex_ );

//---------------------------------------------------------------------------
/*!
    \brief Mutex_Claim

    Claim the Mutex_t.  When the Mutex_t is claimed, no other thread can claim a
    region protected by the object.
*/
void Mutex_Claim( Mutex_t *pstMutex_ );

#if KERNEL_USE_TIMEOUTS

//---------------------------------------------------------------------------
/*!
    \brief Mutex_TimedClaim
		
    Claim the Mutex_t.  When the Mutex_t is claimed, no other thread can claim a
    region protected by the object.  This function enforces a maximum time (ms-
    resolution) for which the thread will wait before giving up and returning
    a timeout.

	\param ulWaitTimeMS_
		
	\return true - Mutex_t was claimed within the time period specified
			false - Mutex_t operation timed-out before the claim operation.
*/
K_BOOL Mutex_TimedClaim( Mutex_t *pstMutex_, K_ULONG ulWaitTimeMS_ );
	
#endif

//---------------------------------------------------------------------------
/*!
    \brief Mutex_Release

    Release the Mutex_t.  When the Mutex_t is released, another object can enter
    the Mutex_t-protected region.
*/
void Mutex_Release( Mutex_t *pstMutex_ );
    

#ifdef __cplusplus
    }
#endif

#endif
#endif //__MUTEX_H_

