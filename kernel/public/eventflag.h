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
    \file eventflag.h
    \brief Event Flag Blocking Object/IPC-Object definition.
*/

#ifndef __EVENTFLAG_H__
#define __EVENTFLAG_H__

#include "mark3cfg.h"
#include "kernel.h"
#include "kerneltypes.h"
#include "blocking.h"
#include "thread.h"

#if KERNEL_USE_EVENTFLAG

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
/*!
 * \brief The EventFlag_t object is a blocking object, similar to a Semaphore_t or
 * Mutex_t, commonly used for synchronizing thread execution based on events
 * occurring within the system.
 *
 * Each EventFlag_t object contains a 16-bit bitmask, which is used to trigger
 * events on associated threads.  Threads wishing to block, waiting for
 * a specific event to occur can wait on any pattern within this 16-bit bitmask
 * to be set.  Here, we provide the ability for a thread to block, waiting
 * for ANY bits in a specified mask to be set, or for ALL bits within a
 * specific mask to be set.  Depending on how the object is configured, the
 * bits that triggered the wakeup can be automatically cleared once a match
 * has occurred.
 *
 */
typedef struct  
{
	// Inherit from BlockingObject -- must go first
	ThreadList_t clList;
	
	K_USHORT m_usSetMask;       //!< Event flags currently set in this object
} EventFlag_t;

/*!
  \brief Init Initializes the EventFlag_t object prior to use.
 */
void EventFlag_Init( EventFlag_t *pstFlag_ );

/*!
    * \brief Wait - Block a thread on the specific flags in this event flag group
    * \param usMask_ - 16-bit bitmask to block on
    * \param eMode_ - EVENT_FLAG_ANY:  Thread_t will block on any of the bits in the mask
    *               - EVENT_FLAG_ALL:  Thread_t will block on all of the bits in the mask
    * \return Bitmask condition that caused the thread to unblock, or 0 on error or timeout
    */
K_USHORT EventFlag_Wait( EventFlag_t *pstFlag_, K_USHORT usMask_, EventFlagOperation_t eMode_);

#if KERNEL_USE_TIMEOUTS
/*!
    * \brief Wait - Block a thread on the specific flags in this event flag group
    * \param usMask_ - 16-bit bitmask to block on
    * \param eMode_ - EVENT_FLAG_ANY:  Thread_t will block on any of the bits in the mask
    *               - EVENT_FLAG_ALL:  Thread_t will block on all of the bits in the mask
    * \param ulTimeMS_ - Time to block (in ms)
    * \return Bitmask condition that caused the thread to unblock, or 0 on error or timeout
    */
K_USHORT EventFlag_TimedWait( EventFlag_t *pstFlag_, K_USHORT usMask_, EventFlagOperation_t eMode_, K_ULONG ulTimeMS_);


#endif

/*!
    * \brief Set - Set additional flags in this object (logical OR).  This API can potentially
    *              result in threads blocked on Wait() to be unblocked.
    * \param usMask_ - Bitmask of flags to set.
    */
void EventFlag_Set( EventFlag_t *pstFlag_, K_USHORT usMask_);

/*!
    * \brief ClearFlags - Clear a specific set of flags within this object, specific by bitmask
    * \param usMask_ - Bitmask of flags to clear
    */
void EventFlag_Clear( EventFlag_t *pstFlag_, K_USHORT usMask_);

/*!
    * \brief GetMask Returns the state of the 16-bit bitmask within this object
    * \return The state of the 16-bit bitmask
    */
K_USHORT EventFlag_GetMask( EventFlag_t *pstFlag_ );

#ifdef __cplusplus
    }
#endif

#endif //KERNEL_USE_EVENTFLAG
#endif //__EVENTFLAG_H__

