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

    \file   notify.c

    \brief  Lightweight thread notification - blocking object

*/


#include "mark3cfg.h"
#include "notify.h"

#if KERNEL_USE_NOTIFY
//---------------------------------------------------------------------------
void TimedNotify_Callback( Thread_t *pstOwner_, void *pvData_ )
{
    Notify_t *pstNotify = (Notify_t*)(pvData_);

    // Indicate that the semaphore has expired on the thread
    Thread_SetExpired( pstOwner_, true );

    // Wake up the thread that was blocked on this semaphore.
    Notify_WakeMe( pstNotify, pstOwner_ );

    if ( Thread_GetCurPriority( pstOwner_ ) >=
         Thread_GetCurPriority( Scheduler_GetCurrentThread() ) )
    {
        Thread_Yield();
    }
}

//---------------------------------------------------------------------------
void Notify_Init( Notify_t *pstNotify_ )
{
    ThreadList_Init( (ThreadList_t*)pstNotify_ );
}

//---------------------------------------------------------------------------
void Notify_Signal( Notify_t *pstNotify_ )
{
    bool bReschedule = false;

    CS_ENTER();
    Thread_t *pstCurrent = (Thread_t*)LinkList_GetHead((LinkListNode_t*)pstNotify_);
    while (pstCurrent != NULL)
    {
        BlockingObject_UnBlock( pstCurrent );
        if ( !bReschedule &&
           ( Thread_GetCurPriority( pstCurrent ) >=
             Thread_GetCurPriority( Scheduler_GetCurrentThread() ) ) )
        {
            bReschedule = true;
        }
        pstCurrent = (Thread_t*)LinkList_GetHead(pstNotify_);
    }
    CS_EXIT();

    if (bReschedule)
    {
        Thread_Yield();
    }
}

//---------------------------------------------------------------------------
void Notify_Wait( Notify_t *pstNotify_, bool *pbFlag_ )
{
    CS_ENTER();
    BlockingObject_Block( (ThreadList_t*)pstNotify_, Scheduler_GetCurrentThread() );
    if (pbFlag_)
    {
        *pbFlag_ = false;
    }
    CS_EXIT();

    Thread_Yield();
    if (pbFlag_)
    {
        *pbFlag_ = true;
    }
}

//---------------------------------------------------------------------------
#if KERNEL_USER_TIMEOUTS
bool Notify_Wait( Notify_t *pstNotify_, K_ULONG ulWaitTimeMS_, bool *pbFlag_ )
{
    bool bUseTimer = false;
    Timer_t stNotifyTimer;

    CS_ENTER();
    if (ulWaitTimeMS_)
    {
        bUseTimer = true;
        Thread_SetExpired( Scheduler_GetCurrentThread(), false );

        Timer_Init( &stNotifyTimer );
        Timer_Start( &stNotifyTimer, 0, ulWaitTimeMS_, TimedNotify_Callback, (void*)pstNotify_);
    }

    Block(g_pstCurrent);

    if (pbFlag_)
    {
        *pbFlag_ = false;
    }
    CS_EXIT();

    Thread_Yield();

    if (pbFlag_)
    {
        *pbFlag_ = true;
    }

    if (bUseTimer)
    {
        Timer_Stop( &stNotifyTimer );
        return ( Thread_GetExpired( Scheduler_GetCurrentThread() ) == false );
    }

    return true;
}
#endif
//---------------------------------------------------------------------------

void Notify_WakeMe( Notify_t *pstNotify_, Thread_t *pclChosenOne_ )
{
    BlockingObject_UnBlock( pclChosenOne_ );
}

#endif
