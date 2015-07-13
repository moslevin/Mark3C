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

    \file   ksemaphore.h    

    \brief  Semaphore_t Blocking Object object declarations

*/

#ifndef __KSEMAPHORE_H__
#define __KSEMAPHORE_H__

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "blocking.h"
#include "threadlist.h"

#if KERNEL_USE_SEMAPHORE

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
/*!
    Counting Semaphore_t, based on BlockingObject base object.
*/
typedef struct 
{	
	// Inherit from BlockingObject -- must go first!!
	ThreadList_t	clBlockList;
	
	K_USHORT m_usValue;         //!< Current count held by the Semaphore_t
	K_USHORT m_usMaxValue;      //!< Maximum count that can be held by this Semaphore_t
} Semaphore_t;

//---------------------------------------------------------------------------
/*!
    \fn void Init(K_USHORT usInitVal_, K_USHORT usMaxVal_)

    Initialize a Semaphore_t before use.  Must be called
    before post/pend operations.
        
    \param usInitVal_ Initial value held by the Semaphore_t
    \param usMaxVal_ Maximum value for the Semaphore_t
*/
void Semaphore_Init( Semaphore_t *pstSem_, K_USHORT usInitVal_, K_USHORT usMaxVal_);
    
//---------------------------------------------------------------------------
/*!
    \fn void Post();
        
    Increment the Semaphore_t count.

    \return true if the Semaphore_t was posted, false if the count
            is already maxed out.
*/
K_BOOL Semaphore_Post( Semaphore_t *pstSem_ );

//---------------------------------------------------------------------------
/*!
    \fn void Pend();
        
    Decrement the Semaphore_t count.  If the count is zero,
    the thread will block until the Semaphore_t is pended.        
*/
void Semaphore_Pend( Semaphore_t *pstSem_ );
    
//---------------------------------------------------------------------------
/*!
	\fn K_USHORT GetCount()
		
	Return the current Semaphore_t counter. This can be 
	used by a thread to bypass blocking on a Semaphore_t -
	allowing it to do other things until a non-zero count
	is returned, instead of blocking until the Semaphore_t
	is posted.
		
	\return The current Semaphore_t counter value.
*/
K_USHORT Semaphore_GetCount( Semaphore_t *pstSem_ );
	
#if KERNEL_USE_TIMEOUTS
//---------------------------------------------------------------------------
/*!
    \fn K_BOOL Pend( K_ULONG ulWaitTimeMS_ );
        
    Decrement the Semaphore_t count.  If the count is zero,
    the thread will block until the Semaphore_t is pended.
	If the specified interval expires before the thread is
	unblocked, then the status is returned back to the user.
		
	\return true - Semaphore_t was acquired before the timeout
			false - timeout occurred before the Semaphore_t was claimed.
*/
K_BOOL Semaphore_TimedPend( Semaphore_t *pstSem_, K_ULONG ulWaitTimeMS_);
	
#endif	
	
#ifdef __cplusplus
    }
#endif

#endif //KERNEL_USE_SEMAPHORE

#endif
