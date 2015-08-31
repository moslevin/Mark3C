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

    \file   mailbox.h

    \brief  MailBox + Envelope IPC Mechanism
*/

#ifndef __MAILBOX_H__
#define __MAILBOX_H__

#include "mark3cfg.h"
#include "kerneltypes.h"
#include "ksemaphore.h"

#if KERNEL_USE_MAILBOX

typedef struct
{
    ThreadList_t clBlockList;

    K_USHORT usHead;          //!< Current head index
    K_USHORT usTail;          //!< Current tail index

    K_USHORT usCount;         //!< Count of items in the mailbox
    volatile K_USHORT usFree; //!< Current number of free slots in the mailbox

    K_USHORT usElementSize;   //!< Size of the objects tracked in this mailbox
    const void *pvBuffer;     //!< Pointer to the data-buffer managed by this mailbox

    Semaphore_t stRecvSem;      //!< Counting semaphore used to synchronize threads on the object

#if KERNEL_USE_TIMEOUTS
    Semaphore_t stSendSem;      //!< Binary semaphore for send-blocked threads.
#endif
} MailBox_t;

/*!
 * \brief Init
 *
 * Initialize the mailbox object prior to its use.  This must be called before
 * any calls can be made to the object.
 *
 * \param pvBuffer_         Pointer to the static buffer to use for the mailbox
 * \param usBufferSize_    Size of the mailbox buffer, in bytes
 * \param usElementSize_   Size of each envelope, in bytes
 */
void MailBox_Init( MailBox_t *pstMailBox_, void *pvBuffer_, K_USHORT usBufferSize_, K_USHORT usElementSize_ );

/*!
 * \brief Send
 *
 * Send an envelope to the mailbox.  This safely copies the data contents of the
 * datastructure to the previously-initialized mailbox buffer.  If there is a
 * thread already blocking, awaiting delivery to the mailbox, it will be unblocked
 * at this time.
 *
 * This method delivers the envelope at the head of the mailbox.
 *
 * \param pvData_           Pointer to the data object to send to the mailbox.
 * \return                  true - envelope was delivered, false - mailbox is full.
 */
bool MailBox_Send( MailBox_t *pstMailBox_, void *pvData_ );

/*!
 * \brief SendTail
 *
 * Send an envelope to the mailbox.  This safely copies the data contents of the
 * datastructure to the previously-initialized mailbox buffer.  If there is a
 * thread already blocking, awaiting delivery to the mailbox, it will be unblocked
 * at this time.
 *
 * This method delivers the envelope at the tail of the mailbox.
 *
 * \param pvData_           Pointer to the data object to send to the mailbox.
 * \return                  true - envelope was delivered, false - mailbox is full.
 */
bool MailBox_SendTail( MailBox_t *pstMailBox_, void *pvData_ );

#if KERNEL_USE_TIMEOUTS
/*!
 * \brief Send
 *
 * Send an envelope to the mailbox.  This safely copies the data contents of the
 * datastructure to the previously-initialized mailbox buffer.  If there is a
 * thread already blocking, awaiting delivery to the mailbox, it will be unblocked
 * at this time.
 *
 * This method delivers the envelope at the head of the mailbox.
 *
 * \param pvData_           Pointer to the data object to send to the mailbox.
 * \param ulTimeoutMS_      Maximum time to wait for a free transmit slot
 * \return                  true - envelope was delivered, false - mailbox is full.
 */
bool MailBox_TimedSend( MailBox_t *pstMailBox_, void *pvData_, K_ULONG ulTimeoutMS_ );

/*!
 * \brief SendTail
 *
 * Send an envelope to the mailbox.  This safely copies the data contents of the
 * datastructure to the previously-initialized mailbox buffer.  If there is a
 * thread already blocking, awaiting delivery to the mailbox, it will be unblocked
 * at this time.
 *
 * This method delivers the envelope at the tail of the mailbox.
 *
 * \param pvData_           Pointer to the data object to send to the mailbox.
 * \param ulTimeoutMS_      Maximum time to wait for a free transmit slot
 * \return                  true - envelope was delivered, false - mailbox is full.
 */
bool MailBox_TimedSendTail( MailBox_t *pstMailBox_, void *pvData_, K_ULONG ulTimeoutMS_ );
#endif

/*!
 * \brief Receive
 *
 * Read one envelope from the head of the mailbox.  If the mailbox is currently
 * empty, the calling thread will block until an envelope is delivered.
 *
 * \param pvData_ Pointer to a buffer that will have the envelope's contents
 *                copied into upon delivery.
 */
void MailBox_Receive( MailBox_t *pstMailBox_, void *pvData_ );

/*!
 * \brief ReceiveTail
 *
 * Read one envelope from the tail of the mailbox.  If the mailbox is currently
 * empty, the calling thread will block until an envelope is delivered.
 *
 * \param pvData_ Pointer to a buffer that will have the envelope's contents
 *                copied into upon delivery.
 */
void MailBox_ReceiveTail( MailBox_t *pstMailBox_, void *pvData_ );

#if KERNEL_USE_TIMEOUTS
/*!
 * \brief Receive
 *
 * Read one envelope from the head of the mailbox.  If the mailbox is currently
 * empty, the calling thread will block until an envelope is delivered, or the
 * specified time has elapsed without delivery.
 *
 * \param pvData_ Pointer to a buffer that will have the envelope's contents
 *                copied into upon delivery.
 * \param ulTimeoutMS_ Maximum time to wait for delivery.
 * \return true - envelope was delivered, false - delivery timed out.
 */
bool MailBox_TimedReceive( MailBox_t *pstMailBox_, void *pvData_, K_ULONG ulTimeoutMS_ );

/*!
 * \brief ReceiveTail
 *
 * Read one envelope from the tail of the mailbox.  If the mailbox is currently
 * empty, the calling thread will block until an envelope is delivered, or the
 * specified time has elapsed without delivery.
 *
 * \param pvData_ Pointer to a buffer that will have the envelope's contents
 *                copied into upon delivery.
 * \param ulTimeoutMS_ Maximum time to wait for delivery.
 * \return true - envelope was delivered, false - delivery timed out.
 */
bool MailBox_TimedReceiveTail( MailBox_t *pstMailBox_, void *pvData_, K_ULONG ulTimeoutMS_ );
#endif

K_USHORT MailBox_GetFreeSlots( MailBox_t *pstMailBox_ );

bool MailBox_IsFull( MailBox_t *pstMailBox_ );

bool MailBox_IsEmpty( MailBox_t *pstMailBox_ );


#endif

#endif

