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

    \file   notify.h

    \brief  Lightweight thread notification - blocking object

*/

#ifndef __NOTIFY_H__
#define __NOTIFY_H__

#include "mark3cfg.h"
#include "blocking.h"

typedef struct
{
    ThreadList_t stBlockList;
} Notify_t;

/*!
 * \brief Init
 *
 * Initialze the Notification object prior to use.
 */
void Notify_Init( Notify_t *pstNotify_ );

/*!
 * \brief Signal
 *
 * Signal the notification object.  This will cause the
 * highest priority thread currently blocking on the object
 * to wake.  If no threads are currently blocked on the
 * object, the call has no effect.
 *
 */
void Notify_Signal( Notify_t *pstNotify_ );

/*!
 * \brief Wait
 *
 * Block the current thread, waiting for a signal on the
 * object.
 *
 * \param pbFlag_ Flag set to false on block, and true
 *                upon wakeup.
 */
void Notify_Wait( Notify_t *pstNotify_, bool *pbFlag_ );

#if KERNEL_USE_TIMEOUTS
/*!
 * \brief Wait
 * Block the current thread, waiting for a signal on the
 * object.
 *
 * \param ulWaitTimeMS_ Time to wait for the notification
 *                      event.
 * \param pbFlag_       Flag set to false on block, and
 *                      true upon wakeup.
 * \return true on notification, false on timeout
 */
bool Notify_TimedWait( Notify_t *pstNotify_, K_ULONG ulWaitTimeMS_, bool *pbFlag_ );
#endif

/*!
 * \brief WakeMe
 *
 * Wake the specified thread from its current blocking queue.
 * Note that this is only public in order to be accessible
 * from a timer callack.
 *
 * \param pclChosenOne_ Thread to wake up
 */
void Notify_WakeMe( Notify_t *pstNotify_, Thread_t *pclChosenOne_ );

#endif
