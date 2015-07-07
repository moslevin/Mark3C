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

    \file   timer.h

    \brief  Timer_t object declarations
*/

#ifndef __TIMER_H__
#define __TIMER_H__

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "ll.h"

#if KERNEL_USE_TIMERS

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
#define TIMERLIST_FLAG_ONE_SHOT         (0x01)    //!< Timer_t is one-shot
#define TIMERLIST_FLAG_ACTIVE           (0x02)    //!< Timer_t is currently active
#define TIMERLIST_FLAG_CALLBACK         (0x04)    //!< Timer_t is pending a callback
#define TIMERLIST_FLAG_EXPIRED          (0x08)    //!< Timer_t is actually expired.

//---------------------------------------------------------------------------
#if KERNEL_TIMERS_TICKLESS

//---------------------------------------------------------------------------
#define MAX_TIMER_TICKS                 (0x7FFFFFFF)    //!< Maximum value to set

//---------------------------------------------------------------------------
/*
    Ugly macros to support a wide resolution of delays.
    Given a 16-bit timer @ 16MHz & 256 cycle prescaler, this gives us...
    Max time, SECONDS_TO_TICKS:  68719s
    Max time, MSECONDS_TO_TICKS: 6871.9s
    Max time, USECONDS_TO_TICKS: 6.8719s

    ...With a 16us tick resolution.

    Depending on the system frequency and timer resolution, you may want to
    customize these values to suit your system more appropriately.
*/
//---------------------------------------------------------------------------
#define SECONDS_TO_TICKS(x)             ((((K_ULONG)x) * TIMER_FREQ))
#define MSECONDS_TO_TICKS(x)            ((((((K_ULONG)x) * (TIMER_FREQ/100)) + 5) / 10))
#define USECONDS_TO_TICKS(x)            ((((((K_ULONG)x) * TIMER_FREQ) + 50000) / 1000000))

//---------------------------------------------------------------------------
#define MIN_TICKS                        (3)    //!< The minimum tick value to set
//---------------------------------------------------------------------------

#else
//---------------------------------------------------------------------------
// Tick-based timers, assuming 1khz tick rate
#define MAX_TIMER_TICKS                 (0x7FFFFFFF)    //!< Maximum value to set

//---------------------------------------------------------------------------
// add time because we don't know how far in an epoch we are when a call is made.
#define SECONDS_TO_TICKS(x)             (((K_ULONG)(x) * 1000) + 1)
#define MSECONDS_TO_TICKS(x)            ((K_ULONG)(x + 1))
#define USECONDS_TO_TICKS(x)            (((K_ULONG)(x + 999)) / 1000)

//---------------------------------------------------------------------------
#define MIN_TICKS                       (1)    //!< The minimum tick value to set
//---------------------------------------------------------------------------

#endif // KERNEL_TIMERS_TICKLESS


//---------------------------------------------------------------------------
/*!
    Timer_t - an event-driven execution context based on a specified time
    interval.  This inherits from a LinkListNode_t for ease of management by
    a global TimerList object.
*/


//---------------------------------------------------------------------------
/*!
 * \brief Timer_Init Re-initialize the Timer_t to default values.
 * \param pstTimer_  Pointer to the timer object to manipulate
 */
void Timer_Init( Timer_t *pstTimer_ );

//---------------------------------------------------------------------------
/*!
    \brief Timer_Start

    Start a timer using default ownership, using repeats as an option, and
    millisecond resolution.

    \param pstTimer_  Pointer to the timer object to manipulate
    \param bRepeat_ 0 - timer is one-shot.  1 - timer is repeating.
    \param ulIntervalMs_ - Interval of the timer in miliseconds
    \param pfCallback_ - Function to call on timer expiry
    \param pvData_ - Data to pass into the callback function
*/
void Timer_Start( Timer_t *pstTimer_, K_BOOL bRepeat_, K_ULONG ulIntervalMs_, TimerCallback_t pfCallback_, void *pvData_ );

//---------------------------------------------------------------------------
/*!
    \brief Timer_StartEx

    Start a timer using default ownership, using repeats as an option, and
    millisecond resolution.

    \param pstTimer_  Pointer to the timer object to manipulate
    \param bRepeat_ 0 - timer is one-shot.  1 - timer is repeating.
    \param ulIntervalMs_ - Interval of the timer in miliseconds
    \param ulToleranceMs - Allow the timer expiry to be delayed by an additional maximum time, in
                           order to have as many timers expire at the same time as possible.
    \param pfCallback_ - Function to call on timer expiry
    \param pvData_ - Data to pass into the callback function
*/
void Timer_StartEx( Timer_t *pstTimer_, K_BOOL bRepeat_, K_ULONG ulIntervalMs_, K_ULONG ulToleranceMs_, TimerCallback_t pfCallback_, void *pvData_ );

//---------------------------------------------------------------------------
/*!
    Stop a timer already in progress.   Has no effect on timers that have
    already been stopped.
*/
/*!
 * \brief Timer_Stop
 *
 *  Stop a timer already in progress.   Has no effect on timers that have
 *  already been stopped.
 *
 * \param pstTimer_  Pointer to the timer object to manipulate
 */
void Timer_Stop( Timer_t *pstTimer_ );

//---------------------------------------------------------------------------
/*!
    \fn void Timer_SetFlags (K_UCHAR ucFlags_)

    Set the timer's flags based on the bits in the ucFlags_ argument

    \param pstTimer_  Pointer to the timer object to manipulate
    \param ucFlags_ Flags to assign to the timer object.
                TIMERLIST_FLAG_ONE_SHOT for a one-shot timer,
                0 for a continuous timer.
*/
void Timer_SetFlags ( Timer_t *pstTimer_, K_UCHAR ucFlags_);

//---------------------------------------------------------------------------
/*!
    \fn void Timer_SetCallback( TimerCallback_t pfCallback_)

    Define the callback function to be executed on expiry of the timer

    \param pstTimer_  Pointer to the timer object to manipulate
    \param pfCallback_ Pointer to the callback function to call
*/
void Timer_SetCallback( Timer_t *pstTimer_, TimerCallback_t pfCallback_);

//---------------------------------------------------------------------------
/*!
    \fn void Timer_SetData( void *pvData_ )

    Define a pointer to be sent to the timer callbcak on timer expiry

    \param pvData_ Pointer to data to pass as argument into the callback
*/
void Timer_SetData( Timer_t *pstTimer_, void *pvData_ );

//---------------------------------------------------------------------------
/*!
    \fn void Timer_SetOwner( Thread_t *pstOwner_)

    Set the owner-thread of this timer object (all timers must be owned by
    a thread).

    \param pstOwner_ Owner thread of this timer object
*/
void Timer_SetOwner( Timer_t *pstTimer_, Thread_t *pstOwner_);

//---------------------------------------------------------------------------
/*!
    \fn void Timer_SetIntervalTicks(K_ULONG ulTicks_)

    Set the timer expiry in system-ticks (platform specific!)

    \param pstTimer_  Pointer to the timer object to manipulate
    \param ulTicks_ Time in ticks
*/
void Timer_SetIntervalTicks( Timer_t *pstTimer_, K_ULONG ulTicks_);

//---------------------------------------------------------------------------
/*!
    \fn void Timer_SetIntervalSeconds(K_ULONG ulSeconds_);

    Set the timer expiry interval in seconds (platform agnostic)

    \param ulSeconds_ Time in seconds
*/
void Timer_SetIntervalSeconds( Timer_t *pstTimer_, K_ULONG ulSeconds_);

//---------------------------------------------------------------------------
/*!
 * \brief Timer_GetInterval Return the timer object's programmed interval
 * \param pstTimer_  Pointer to the timer object to manipulate
 * \return The timer object's programmed interval, in ticks.
 */
K_ULONG Timer_GetInterval( Timer_t *pstTimer_ );

//---------------------------------------------------------------------------
/*!
    \fn void Timer_SetIntervalMSeconds(K_ULONG ulMSeconds_)

    Set the timer expiry interval in milliseconds (platform agnostic)

    \param pstTimer_  Pointer to the timer object to manipulate
    \param ulMSeconds_ Time in milliseconds
*/
void Timer_SetIntervalMSeconds( Timer_t *pstTimer_, K_ULONG ulMSeconds_);

//---------------------------------------------------------------------------
/*!
    \fn void Timer_SetIntervalUSeconds(K_ULONG ulUSeconds_)

    Set the timer expiry interval in microseconds (platform agnostic)

    \param pstTimer_  Pointer to the timer object to manipulate
    \param ulUSeconds_ Time in microseconds
*/
void Timer_SetIntervalUSeconds( Timer_t *pstTimer_, K_ULONG ulUSeconds_);

//---------------------------------------------------------------------------
/*!
    \fn void Timer_SetTolerance(K_ULONG ulTicks_)

    Set the timer's maximum tolerance in order to synchronize timer
    processing with other timers in the system.

    \param pstTimer_  Pointer to the timer object to manipulate
    \param ulTicks_ Maximum tolerance in ticks

*/
void Timer_SetTolerance( Timer_t *pstTimer_, K_ULONG ulTicks_);

#ifdef __cplusplus
    }
#endif

#endif // KERNEL_USE_TIMERS

#endif
