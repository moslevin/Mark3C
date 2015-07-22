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

    \file   timer.cpp

    \brief  Timer_t implementations

*/

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "timer.h"
#include "timerlist.h"
#include "timerscheduler.h"
#include "kerneltimer.h"
#include "threadport.h"
#include "kerneldebug.h"
#include "quantum.h"

//---------------------------------------------------------------------------
#if defined __FILE_ID__
    #undef __FILE_ID__
#endif
#define __FILE_ID__     TIMER_C       //!< File ID used in kernel trace calls

#if KERNEL_USE_TIMERS

//---------------------------------------------------------------------------
void Timer_Init( Timer_t *pstTimer_ )
{
    LinkListNode_Clear( (LinkListNode_t*)pstTimer_ );
    pstTimer_->ulInterval = 0;
    pstTimer_->ulTimerTolerance = 0;
    pstTimer_->ulTimeLeft = 0;
    pstTimer_->ucFlags = 0;
}

//---------------------------------------------------------------------------
void Timer_Start( Timer_t *pstTimer_, K_BOOL bRepeat_, K_ULONG ulIntervalMs_, TimerCallback_t pfCallback_, void *pvData_ )
{
    Timer_SetIntervalMSeconds( pstTimer_, ulIntervalMs_ );
    pstTimer_->ulTimerTolerance = 0;
    pstTimer_->pfCallback = pfCallback_;
    pstTimer_->pvData = pvData_;
    if (!bRepeat_)
    {
        pstTimer_->ucFlags = TIMERLIST_FLAG_ONE_SHOT;
    }
    else
    {
        pstTimer_->ucFlags = 0;
    }
    pstTimer_->pstOwner = Scheduler_GetCurrentThread();
    TimerScheduler_Add( pstTimer_ );
}

//---------------------------------------------------------------------------
void Timer_StartEx( Timer_t *pstTimer_, K_BOOL bRepeat_, K_ULONG ulIntervalMs_, K_ULONG ulToleranceMs_, TimerCallback_t pfCallback_, void *pvData_ )
{
    pstTimer_->ulTimerTolerance = MSECONDS_TO_TICKS( ulToleranceMs_ );
    Timer_Start( pstTimer_, bRepeat_, ulIntervalMs_, pfCallback_, pvData_ );
}

//---------------------------------------------------------------------------
void Timer_Stop( Timer_t *pstTimer_ )
{
    TimerScheduler_Remove( pstTimer_ );
}

//---------------------------------------------------------------------------
void Timer_SetFlags ( Timer_t *pstTimer_, K_UCHAR ucFlags_)
{
    pstTimer_->ucFlags = ucFlags_;
}

//---------------------------------------------------------------------------
void Timer_SetCallback( Timer_t *pstTimer_, TimerCallback_t pfCallback_)
{
    pstTimer_->pfCallback = pfCallback_;
}

//---------------------------------------------------------------------------
void Timer_SetData( Timer_t *pstTimer_, void *pvData_ )
{
    pstTimer_->pvData = pvData_;
}

//---------------------------------------------------------------------------
void Timer_SetOwner( Timer_t *pstTimer_, Thread_t *pstOwner_)
{
    pstTimer_->pstOwner = pstOwner_;
}

//---------------------------------------------------------------------------
void Timer_SetIntervalTicks( Timer_t *pstTimer_, K_ULONG ulTicks_ )
{
    pstTimer_->ulInterval = ulTicks_;
}

//---------------------------------------------------------------------------
//!! The next three cost us 330 bytes of flash on AVR...
//---------------------------------------------------------------------------
void Timer_SetIntervalSeconds( Timer_t *pstTimer_, K_ULONG ulSeconds_)
{
    pstTimer_->ulInterval = SECONDS_TO_TICKS(ulSeconds_);
}

//---------------------------------------------------------------------------
void Timer_SetIntervalMSeconds( Timer_t *pstTimer_, K_ULONG ulMSeconds_)
{
    pstTimer_->ulInterval = MSECONDS_TO_TICKS(ulMSeconds_);
}

//---------------------------------------------------------------------------
void Timer_SetIntervalUSeconds( Timer_t *pstTimer_, K_ULONG ulUSeconds_)
{
    pstTimer_->ulInterval = USECONDS_TO_TICKS(ulUSeconds_);
}

//---------------------------------------------------------------------------
void Timer_SetTolerance( Timer_t *pstTimer_, K_ULONG ulTicks_)
{
    pstTimer_->ulTimerTolerance = ulTicks_;
}
//---------------------------------------------------------------------------
K_ULONG Timer_GetInterval( Timer_t *pstTimer_ )
{
    return pstTimer_->ulInterval;
}


#endif

