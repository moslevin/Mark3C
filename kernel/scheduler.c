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

    \file   scheduler.cpp

    \brief  Strict-Priority + Round-Robin thread scheduler implementation

*/

#include "kerneltypes.h"
#include "ll.h"
#include "scheduler.h"
#include "thread.h"
#include "threadport.h"
#include "kernel.h"
#include "kerneldebug.h"
//---------------------------------------------------------------------------
#if defined __FILE_ID__
    #undef __FILE_ID__
#endif
#define __FILE_ID__     SCHEDULER_C //!< File ID used in kernel trace calls

//---------------------------------------------------------------------------
volatile Thread_t *g_pstNext;         //!< Pointer to the currently-chosen next-running thread
Thread_t *g_pstCurrent;               //!< Pointer to the currently-running thread

//---------------------------------------------------------------------------
static ThreadList_t m_clStopList;     //! ThreadList_t for all stopped threads
static ThreadList_t m_aclPriorities[NUM_PRIORITIES];    //! ThreadLists for all threads at all priorities
static K_BOOL m_bEnabled;           //! Scheduler's state - enabled or disabled
static K_BOOL m_bQueuedSchedule;    //! Variable representing whether or not there's a queued scheduler operation
static K_UCHAR m_ucPriFlag;         //! Bitmap flag for each

//---------------------------------------------------------------------------
/*!
 * This implements a 4-bit "Count-leading-zeros" operation using a RAM-based
 * lookup table.  It is used to efficiently perform a CLZ operation under the
 * assumption that a native CLZ instruction is unavailable.  This table is
 * further optimized to provide a 0xFF result in the event that the index value
 * is itself zero, allowing us to quickly identify whether or not subsequent
 * 4-bit LUT operations are required to complete the scheduling process.
 */
static const K_UCHAR aucCLZ[16] ={255,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3};

//---------------------------------------------------------------------------
void Scheduler_Init()
{
    m_ucPriFlag = 0;
    uint8_t i;
    for (i = 0; i < NUM_PRIORITIES; i++)
    {
        ThreadList_Init( &m_aclPriorities[i] );
        ThreadList_SetPriority( &m_aclPriorities[i], i );
        ThreadList_SetFlagPointer( &m_aclPriorities[i], &m_ucPriFlag );
    }
    m_bQueuedSchedule = false;
}

//---------------------------------------------------------------------------
void Scheduler_Schedule()
{
    K_UCHAR ucPri = 0;
    
    // Figure out what priority level has ready tasks (8 priorities max)
    // To do this, we apply our current active-thread bitmap (m_ucPriFlag)
    // and perform a CLZ on the upper four bits.  If no tasks are found
    // in the higher priority bits, search the lower priority bits.  This
    // also assumes that we always have the idle thread ready-to-run in
    // priority level zero.
    ucPri = aucCLZ[m_ucPriFlag >> 4 ];
    if (ucPri == 0xFF)
    {
        ucPri = aucCLZ[m_ucPriFlag & 0x0F];
    }
    else
    {
        ucPri += 4;
    }

#if KERNEL_USE_IDLE_FUNC
    if (ucPri == 0xFF)
    {
        // There aren't any active threads at all - set g_pstNext to IDLE
        g_pstNext = Kernel_GetIdleThread();
    }
    else
#endif
    {
        // Get the thread node at this priority.
        g_pstNext = (Thread_t*)( LinkList_GetHead( (LinkList_t*)&m_aclPriorities[ucPri] ) );
    }
    KERNEL_TRACE_1( STR_SCHEDULE_1, (K_USHORT)Thread_GetID( (Thread_t*)g_pstNext) );
}

//---------------------------------------------------------------------------
void Scheduler_Add(Thread_t *pstThread_)
{
    ThreadList_Add( &m_aclPriorities[ Thread_GetPriority(pstThread_) ],
                    pstThread_ );
}

//---------------------------------------------------------------------------
void Scheduler_Remove(Thread_t *pstThread_)
{
    ThreadList_Remove( &m_aclPriorities[ Thread_GetPriority(pstThread_) ],
                       pstThread_ );
}

//---------------------------------------------------------------------------
K_BOOL Scheduler_SetScheduler(K_BOOL bEnable_)
{
    K_BOOL bRet ;
    CS_ENTER();
    bRet = m_bEnabled;
    m_bEnabled = bEnable_;
    // If there was a queued scheduler evevent, dequeue and trigger an
    // immediate Yield
    if (m_bEnabled && m_bQueuedSchedule)
    {
        m_bQueuedSchedule = false;
        Thread_Yield();
    }
    CS_EXIT();
    return bRet;
}

//---------------------------------------------------------------------------
Thread_t *Scheduler_GetCurrentThread()
{
    return g_pstCurrent;
}

//---------------------------------------------------------------------------
volatile Thread_t *Scheduler_GetNextThread()
{
    return g_pstNext;
}

//---------------------------------------------------------------------------
ThreadList_t *Scheduler_GetThreadList( K_UCHAR ucPriority_ )
{
    return &m_aclPriorities[ucPriority_];
}

//---------------------------------------------------------------------------
ThreadList_t *Scheduler_GetStopList()
{
    return &m_clStopList;
}

//---------------------------------------------------------------------------
K_UCHAR Scheduler_IsEnabled( void )
{
    return m_bEnabled;
}

//---------------------------------------------------------------------------
void Scheduler_QueueScheduler( void )
{
    m_bQueuedSchedule = true;
}
