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

    \file   profile.h

    \brief  High-precision profiling timers
	
	Enables the profiling and instrumentation of performance-critical code.
	Multiple timers can be used simultaneously to enable system-wide
	performance metrics to be computed in a lightweight manner.
	
	Usage:
	
	\code
	
	ProfileTimer_t clMyTimer;
	int i;
	
	clMyTimer.Init();
	
	// Profile the same block of code ten times
	for (i = 0; i < 10; i++)
	{
		clMyTimer.Start();
		...
		//Block of code to profile
		...
		clMyTimer.Stop();
	}
	
	// Get the average execution time of all iterations
	ulAverageTimer = clMyTimer.GetAverage();
	
	// Get the execution time from the last iteration
	ulLastTimer = clMyTimer.GetCurrent();
	
	\endcode
*/


#ifndef __PROFILE_H__
#define __PROFILE_H__

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "ll.h"

#if KERNEL_USE_PROFILER

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
typedef struct
{
    K_ULONG m_ulCumulative;  //!< Cumulative tick-count for this timer
    K_ULONG m_ulCurrentIteration; //!< Tick-count for the current iteration
    K_USHORT m_usInitial;	   //!< Initial count
    K_ULONG m_ulInitialEpoch; //!< Initial Epoch
    K_USHORT m_usIterations; //!< Number of iterations executed for this profiling timer
    K_UCHAR m_bActive;	   //!< Wheter or not the timer is active or stopped
} ProfileTimer_t;

//---------------------------------------------------------------------------
/*!
    \fn void Init( void )

    Initialize the profiling timer prior to use.  Can also
    be used to reset a timer that's been used previously.
*/
void ProfileTimer_Init( ProfileTimer_t *pstTimer_ );

//---------------------------------------------------------------------------
/*!
    \fn void Start( void )

    Start a profiling session, if the timer is not already active.
    Has no effect if the timer is already active.
*/
void ProfileTimer_Start( ProfileTimer_t *pstTimer_ );

//---------------------------------------------------------------------------
/*!
    \fn void Stop( void )

    Stop the current profiling session, adding to the
    cumulative time for this timer, and the total iteration count.
*/
void ProfileTimer_Stop( ProfileTimer_t *pstTimer_ );

//---------------------------------------------------------------------------
/*!
    \fn K_ULONG GetAverage( void )

    Get the average time associated with this operation.

    \return Average tick count normalized over all iterations
*/
K_ULONG ProfileTimer_GetAverage( ProfileTimer_t *pstTimer_ );

//---------------------------------------------------------------------------
/*!
    \fn K_ULONG GetCurrent( void )

    Return the current tick count held by the profiler.  Valid
    for both active and stopped timers.

    \return The currently held tick count.
*/
K_ULONG ProfileTimer_GetCurrent( ProfileTimer_t *pstTimer_ );


//---------------------------------------------------------------------------
/*!
    Figure out how many ticks have elapsed in this iteration

    \param usCount_ Current timer count
    \param ulEpoch_ Current timer epoch

    \return Current tick count
*/
K_ULONG ProfileTimer_ComputeCurrentTicks( ProfileTimer_t *pstTimer_, K_USHORT usCount_, K_ULONG ulEpoch_ );

#ifdef __cplusplus
    }
#endif

#endif // KERNEL_USE_PROFILE

#endif 
