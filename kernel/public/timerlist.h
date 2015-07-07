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

    \file   timerlist.h    

    \brief  Timer_t list declarations
    
    These classes implements a linked list of timer objects attached to the 
    global kernel timer scheduler.
*/

#ifndef __TIMERLIST_H__
#define __TIMERLIST_H__

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "timer.h"
#if KERNEL_USE_TIMERS

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
/*!
    \fn void TimerList_Init()

    Initialize the TimerList object.  Must be called before
    using the object.
*/
void TimerList_Init( void );

//---------------------------------------------------------------------------
/*!
    \fn void TimerList_Add(Timer_t *pstListNode_)

    Add a timer to the TimerList.

    \param pstListNode_ Pointer to the Timer_t to Add
*/
void TimerList_Add( Timer_t *pstListNode_ );

//---------------------------------------------------------------------------
/*!
    \fn void TimerList_Remove(Timer_t *pstListNode_)

    Remove a timer from the TimerList, cancelling its expiry.

    \param pstListNode_ Pointer to the Timer_t to remove
*/
void TimerList_Remove( Timer_t *pstListNode_ );

//---------------------------------------------------------------------------
/*!
    \fn void TimerList_Process()

    Process all timers in the timerlist as a result of the timer expiring.
    This will select a new timer epoch based on the next timer to expire.
*/
void TimerList_Process( void );


#ifdef __cplusplus
    }
#endif

#endif // KERNEL_USE_TIMERS

#endif
