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

    \file   mailbox.cpp

    \brief  MailBox + Envelope IPC mechanism
*/

#include "mark3cfg.h"
#include "kerneltypes.h"
#include "ksemaphore.h"
#include "kerneldebug.h"
#include "mailbox.h"

#if KERNEL_USE_MAILBOX

//---------------------------------------------------------------------------
/*!
 * \brief GetHeadPointer
 *
 * Return a pointer to the current head of the mailbox's internal
 * circular buffer.
 *
 * \return pointer to the head element in the mailbox
 */
static void *MailBox_GetHeadPointer( MailBox_t *pstMailBox_ )
{
    K_ADDR uAddr = (K_ADDR)pstMailBox_->pvBuffer;
    uAddr += pstMailBox_->usElementSize * pstMailBox_->usHead;
    return (void*)uAddr;
}

//---------------------------------------------------------------------------
/*!
 * \brief GetTailPointer
 *
 * Return a pointer to the current tail of the mailbox's internal
 * circular buffer.
 *
 * \return pointer to the tail element in the mailbox
 */
static void *MailBox_GetTailPointer( MailBox_t *pstMailBox_ )
{
    K_ADDR uAddr = (K_ADDR)pstMailBox_->pvBuffer;
    uAddr += (K_ADDR)(pstMailBox_->usElementSize * pstMailBox_->usTail);
    return (void*)uAddr;
}

//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
/*!
 * \brief Send_i
 *
 * Internal method which implements all Send() methods in the class.
 *
 * \param pvData_   Pointer to the envelope data
 * \param bTail_    true - write to tail, false - write to head
 * \param ulWaitTimeMS_ Time to wait before timeout (in ms).
 * \return          true - data successfully written, false - buffer full
 */
static bool MailBox_Send_i( MailBox_t *pstMailBox_, const void *pvData_, bool bTail_, K_ULONG ulWaitTimeMS_ );
#else
/*!
 * \brief Send_i
 *
 * Internal method which implements all Send() methods in the class.
 *
 * \param pvData_   Pointer to the envelope data
 * \param bTail_    true - write to tail, false - write to head
 * \return          true - data successfully written, false - buffer full
 */
static bool MailBox_Send_i( MailBox_t *pstMailBox_, const void *pvData_, bool bTail_ );
#endif
//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
/*!
 * \brief Receive_i
 *
 * Internal method which implements all Read() methods in the class.
 *
 * \param pvData_       Pointer to the envelope data
 * \param bTail_        true - read from tail, false - read from head
 * \param ulWaitTimeMS_ Time to wait before timeout (in ms).
 * \return              true - read successfully, false - timeout.
 */
static bool MailBox_Receive_i( MailBox_t *pstMailBox_, const void *pvData_, bool bTail_, K_ULONG ulWaitTimeMS_ );
#else
/*!
 * \brief Receive_i
 *
 * Internal method which implements all Read() methods in the class.
 *
 * \param pvData_       Pointer to the envelope data
 * \param bTail_        true - read from tail, false - read from head
 */
static void MailBox_Receive_i( MailBox_t *pstMailBox_, const void *pvData_, bool bTail_ );
#endif

//---------------------------------------------------------------------------
/*!
 * \brief CopyData
 *
 * Perform a direct byte-copy from a source to a destination object.
 *
 * \param src_  Pointer to an object to read from
 * \param dst_  Pointer to an object to write to
 * \param len_  Length to copy (in bytes)
 */
static void MailBox_CopyData( const void *src_, const void *dst_, K_USHORT len_ )
{
    uint8_t *u8Src = (uint8_t*)src_;
    uint8_t *u8Dst = (uint8_t*)dst_;
    while (len_--)
    {
        *u8Dst++ = *u8Src++;
    }
}
//---------------------------------------------------------------------------
/*!
 * \brief MoveTailForward
 *
 * Move the tail index forward one element
 */
static void MailBox_MoveTailForward( MailBox_t *pstMailBox_ )
{
    pstMailBox_->usTail++;
    if (pstMailBox_->usTail == pstMailBox_->usCount)
    {
        pstMailBox_->usTail = 0;
    }
}
//---------------------------------------------------------------------------
/*!
 * \brief MoveHeadForward
 *
 * Move the head index forward one element
 */
static void MailBox_MoveHeadForward( MailBox_t *pstMailBox_ )
{
    pstMailBox_->usHead++;
    if (pstMailBox_->usHead == pstMailBox_->usCount)
    {
        pstMailBox_->usHead = 0;
    }
}
//---------------------------------------------------------------------------
/*!
 * \brief MoveTailBackward
 *
 * Move the tail index backward one element
 */
static void MailBox_MoveTailBackward(MailBox_t *pstMailBox_ )
{
    if (pstMailBox_->usTail == 0)
    {
        pstMailBox_->usTail = pstMailBox_->usCount;
    }
    pstMailBox_->usTail--;
}
//---------------------------------------------------------------------------
/*!
 * \brief MoveHeadBackward
 *
 * Move the head index backward one element
 */
static void MailBox_MoveHeadBackward(MailBox_t *pstMailBox_ )
{
    if (pstMailBox_->usHead == 0)
    {
        pstMailBox_->usHead = pstMailBox_->usCount;
    }
    pstMailBox_->usHead--;
}

//---------------------------------------------------------------------------
void MailBox_Init( MailBox_t *pstMailBox_, void *pvBuffer_, K_USHORT usBufferSize_, K_USHORT usElementSize_ )
{
    KERNEL_ASSERT(usBufferSize_);
    KERNEL_ASSERT(usElementSize_);
    KERNEL_ASSERT(pvBuffer_);

    pstMailBox_->pvBuffer = pvBuffer_;
    pstMailBox_->usElementSize = usElementSize_;

    pstMailBox_->usCount = (usBufferSize_ / usElementSize_);
    pstMailBox_->usFree = pstMailBox_->usCount;

    pstMailBox_->usHead = 0;
    pstMailBox_->usTail = 0;

    // We use the counting semaphore to implement blocking - with one element
    // in the mailbox corresponding to a post/pend operation in the semaphore.
    Semaphore_Init( &pstMailBox_->stRecvSem, 0, pstMailBox_->usFree );

#if KERNEL_USE_TIMEOUTS
    // Binary semaphore is used to track any threads that are blocked on a
    // "send" due to lack of free slots.
    Semaphore_Init( &pstMailBox_->stSendSem, 0, 1 );
#endif
}

//---------------------------------------------------------------------------
void MailBox_Receive( MailBox_t *pstMailBox_, void *pvData_ )
{
    KERNEL_ASSERT( pvData_ );

#if KERNEL_USE_TIMEOUTS
    MailBox_Receive_i( pstMailBox_, pvData_, false, 0 );
#else
    MailBox_Receive_i( pstMailBox_, pvData_, false );
#endif
}

#if KERNEL_USE_TIMEOUTS
//---------------------------------------------------------------------------
bool MailBox_TimedReceive( MailBox_t *pstMailBox_, void *pvData_, K_ULONG ulTimeoutMS_ )
{
    KERNEL_ASSERT( pvData_ );
    return MailBox_Receive_i( pstMailBox_, pvData_, false, ulTimeoutMS_ );
}
#endif

//---------------------------------------------------------------------------
void MailBox_ReceiveTail( MailBox_t *pstMailBox_, void *pvData_ )
{
    KERNEL_ASSERT( pvData_ );

#if KERNEL_USE_TIMEOUTS
    MailBox_Receive_i( pstMailBox_, pvData_, true, 0 );
#else
    MailBox_Receive_i( pstMailBox_, pvData_, true );
#endif
}

#if KERNEL_USE_TIMEOUTS
//---------------------------------------------------------------------------
bool MailBox_TimedReceiveTail( MailBox_t *pstMailBox_, void *pvData_, K_ULONG ulTimeoutMS_ )
{
    KERNEL_ASSERT( pvData_ );
    return MailBox_Receive_i( pstMailBox_, pvData_, true, ulTimeoutMS_ );
}
#endif

//---------------------------------------------------------------------------
bool MailBox_Send( MailBox_t *pstMailBox_, void *pvData_ )
{
    KERNEL_ASSERT( pvData_ );

#if KERNEL_USE_TIMEOUTS
    return MailBox_Send_i( pstMailBox_, pvData_, false, 0 );
#else
    return MailBox_Send_i( pstMailBox_, pvData_, false );
#endif
}

//---------------------------------------------------------------------------
bool MailBox_SendTail( MailBox_t *pstMailBox_, void *pvData_ )
{
    KERNEL_ASSERT( pvData_ );

#if KERNEL_USE_TIMEOUTS
    return MailBox_Send_i( pstMailBox_, pvData_, true, 0 );
#else
    return MailBox_Send_i( pstMailBox_, pvData_, true );
#endif
}

#if KERNEL_USE_TIMEOUTS
//---------------------------------------------------------------------------
bool MailBox_TimedSend( MailBox_t *pstMailBox_, void *pvData_, K_ULONG ulTimeoutMS_ )
{
    KERNEL_ASSERT( pvData_ );

    return MailBox_Send_i( pstMailBox_, pvData_, false, ulTimeoutMS_ );
}

//---------------------------------------------------------------------------
bool MailBox_TimedSendTail( MailBox_t *pstMailBox_, void *pvData_, K_ULONG ulTimeoutMS_ )
{
    KERNEL_ASSERT( pvData_ );

    return MailBox_Send_i( pstMailBox_, pvData_, true, ulTimeoutMS_ );
}
#endif

//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
bool MailBox_Send_i( MailBox_t *pstMailBox_, const void *pvData_, bool bTail_, K_ULONG ulTimeoutMS_)
#else
bool MailBox_Send_i( MailBox_t *pstMailBox_, const void *pvData_, bool bTail_)
#endif
{
    const void *pvDst;

    bool bRet = false;
    bool bSchedState = Scheduler_SetScheduler( false );

#if KERNEL_USE_TIMEOUTS
    bool bBlock = false;
    bool bDone = false;
    while (!bDone)
    {
        // Try to claim a slot first before resorting to blocking.
        if (bBlock)
        {
            bDone = true;
            Scheduler_SetScheduler( bSchedState );
            Semaphore_TimedPend( &pstMailBox_->stSendSem, ulTimeoutMS_ );
            Scheduler_SetScheduler( false );
        }
#endif

        CS_ENTER();
        // Ensure we have a free slot before we attempt to write data
        if (pstMailBox_->usFree)
        {
            pstMailBox_->usFree--;

            if (bTail_)
            {
                pvDst = MailBox_GetTailPointer( pstMailBox_ );
                MailBox_MoveTailBackward( pstMailBox_ );
            }
            else
            {
                MailBox_MoveHeadForward( pstMailBox_ );
                pvDst = MailBox_GetHeadPointer( pstMailBox_ );
            }
            bRet = true;
#if KERNEL_USE_TIMEOUTS
            bDone = true;
#endif
        }

#if KERNEL_USE_TIMEOUTS
        else if (ulTimeoutMS_)
        {
            bBlock = true;
        }
        else
        {
            bDone = true;
        }
#endif

        CS_EXIT();

#if KERNEL_USE_TIMEOUTS
    }
#endif

    // Copy data to the claimed slot, and post the counting semaphore
    if (bRet)
    {
        MailBox_CopyData( pvData_, pvDst, pstMailBox_->usElementSize );
    }

    Scheduler_SetScheduler( bSchedState );

    if (bRet)
    {
        Semaphore_Post( &pstMailBox_->stRecvSem );
    }

    return bRet;
}

//---------------------------------------------------------------------------
#if KERNEL_USE_TIMEOUTS
bool MailBox_Receive_i( MailBox_t *pstMailBox_, const void *pvData_, bool bTail_, K_ULONG ulWaitTimeMS_ )
#else
void MailBox_Receive_i( MailBox_t *pstMailBox_, const void *pvData_, bool bTail_ )
#endif
{
    const void *pvSrc;

#if KERNEL_USE_TIMEOUTS
    if (!Semaphore_TimedPend( &pstMailBox_->stRecvSem, ulWaitTimeMS_ ))
    {
        // Failed to get the notification from the counting semaphore in the
        // time allotted.  Bail.
        return false;
    }    
#else
    Semaphore_Pend( &pstMailBox_->stRecvSem );
#endif

    // Disable the scheduler while we do this -- this ensures we don't have
    // multiple concurrent readers off the same queue, which could be problematic
    // if multiple writes occur during reads, etc.
    bool bSchedState = Scheduler_SetScheduler( false );

    // Update the head/tail indexes, and get the associated data pointer for
    // the read operation.
    CS_ENTER();

    pstMailBox_->usFree++;
    if (bTail_)
    {
        MailBox_MoveTailForward( pstMailBox_ );
        pvSrc = MailBox_GetTailPointer( pstMailBox_ );
    }
    else
    {
        pvSrc = MailBox_GetHeadPointer( pstMailBox_ );
        MailBox_MoveHeadBackward( pstMailBox_ );
    }

    CS_EXIT();

    MailBox_CopyData( pvSrc, pvData_, pstMailBox_->usElementSize );

    Scheduler_SetScheduler( bSchedState );

    // Unblock a thread waiting for a free slot to send to
    Semaphore_Post( &pstMailBox_->stSendSem );

#if KERNEL_USE_TIMEOUTS
    return true;
#endif
}

K_USHORT MailBox_GetFreeSlots( MailBox_t *pstMailBox_ )
{
    K_USHORT rc;
    CS_ENTER();
    rc = pstMailBox_->usFree;
    CS_EXIT();
    return rc;
}

bool MailBox_IsFull( MailBox_t *pstMailBox_ )
{
    return (MailBox_GetFreeSlots(pstMailBox_) == 0);
}

bool MailBox_IsEmpty( MailBox_t *pstMailBox_ )
{
    return (MailBox_GetFreeSlots(pstMailBox_) == pstMailBox_->usCount);
}




#endif
