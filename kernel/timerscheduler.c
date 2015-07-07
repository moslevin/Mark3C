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

    \file   timerscheduler.cpp

    \brief  Timer_t scheduler definition
*/

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "ll.h"
#include "timer.h"
#include "timerlist.h"
#include "timerscheduler.h"


//---------------------------------------------------------------------------
void TimerScheduler_Init( void )
{
    TimerList_Init( );
}

//---------------------------------------------------------------------------
void TimerScheduler_Add( Timer_t *pstListNode_ )
{
    TimerList_Add( pstListNode_ );
}

//---------------------------------------------------------------------------
void TimerScheduler_Remove( Timer_t *pstListNode_ )
{
    TimerList_Remove( pstListNode_ );
}

//---------------------------------------------------------------------------
void TimerScheduler_Process( void )
{
    TimerList_Process( );
}
