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

    \file   timerscheduler.h

    \brief  Timer_t scheduler declarations
*/

#ifndef __TIMERSCHEDULER_H__
#define __TIMERSCHEDULER_H__

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "ll.h"
#include "timer.h"
#include "timerlist.h"

#if KERNEL_USE_TIMERS

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
/*!
    \fn void TimerScheduler_Init()

    Initialize the timer scheduler.  Must be called before any timer, or
    timer-derived functions are used.
*/
void TimerScheduler_Init( void );

//---------------------------------------------------------------------------
/*!
    \fn void TimerScheduler_Add(Timer_t *pstListNode_)

    Add a timer to the timer scheduler.  Adding a timer implicitly starts
    the timer as well.

    \param pstListNode_ Pointer to the timer list node to add
*/
void TimerScheduler_Add( Timer_t *pstListNode_ );

//---------------------------------------------------------------------------
/*!
    \fn void TimerScheduler_Remove(Timer_t *pstListNode_)

    Remove a timer from the timer scheduler.  May implicitly stop the
    timer if this is the only active timer scheduled.

    \param pstListNode_ Pointer to the timer list node to remove
*/
void TimerScheduler_Remove( Timer_t *pstListNode_ );

//---------------------------------------------------------------------------
/*!
    \fn void TimerScheduler_Process()

    This function must be called on timer expiry (from the timer's ISR
    context).  This will result in all timers being updated based on
    the epoch that just elapsed.  The next timer epoch is set based on the
    next Timer_t object to expire.
*/
void TimerScheduler_Process( void );


#ifdef __cplusplus
    }
#endif

#endif //KERNEL_USE_TIMERS

#endif //__TIMERSCHEDULER_H__

