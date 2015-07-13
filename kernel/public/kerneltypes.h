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
    \file kerneltypes.h
    \brief Basic data type primatives used throughout the OS
*/

#ifndef __KERNELTYPES_H__
#define __KERNELTYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include "ll.h"
#include "mark3cfg.h"

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
#if defined(bool)
    #define K_BOOL            bool          //!< Basic boolean data type (true = 1, false = 0)
#else
    #define K_BOOL            uint8_t       //!< Basic boolean data type (true = 1, false = 0)
#endif
    
#define K_CHAR          char                //!< The 8-bit signed integer type used by Mark3
#define K_UCHAR         uint8_t             //!< The 8-bit unsigned integer type used by Mark3
#define K_USHORT        uint16_t            //!< The 16-bit unsigned integer type used by Mark3
#define K_SHORT         int16_t             //!< The 16-bit signed integer type used by Mark3
#define K_ULONG         uint32_t            //!< The 32-bit unsigned integer type used by Mark3
#define K_LONG          int32_t             //!< The 32-bit signed integer type used by Mark3

#if !defined(K_ADDR)
    #define K_ADDR      uint16_t            //!< Primative datatype representing address-size
#endif
#if !defined(K_WORD)
    #define K_WORD      uint8_t            //!< Primative datatype representing a data word
#endif

//---------------------------------------------------------------------------
// Forward declarations
struct _Thread;
struct _ThreadList;
struct _Timer;

//---------------------------------------------------------------------------
/*!
 * Function pointer type used to implement kernel-panic handlers.
 */
typedef void (*panic_func_t)( K_USHORT usPanicCode_ );

//---------------------------------------------------------------------------
/*!
 * This enumeration describes the different operations supported by the
 * event flag blocking object.
 */
typedef enum
{
    EVENT_FLAG_ALL,             //!< Block until all bits in the specified bitmask are set
    EVENT_FLAG_ANY,             //!< Block until any bits in the specified bitmask are set
    EVENT_FLAG_ALL_CLEAR,       //!< Block until all bits in the specified bitmask are cleared
    EVENT_FLAG_ANY_CLEAR,       //!< Block until any bits in the specified bitmask are cleared
//---
    EVENT_FLAG_MODES,           //!< Count of event-flag modes.  Not used by user
    EVENT_FLAG_PENDING_UNBLOCK  //!< Special code.  Not used by user
} EventFlagOperation_t;

//---------------------------------------------------------------------------
/*!
    This object is used for building thread-management facilities, such as 
    schedulers, and blocking objects.
*/
struct _ThreadList
{
    // Inherit from CircularLinkList_t -- must be first!!
    CircularLinkList_t clList;

    //! Priority of the threadlist
    K_UCHAR m_ucPriority;

    //! Pointer to the bitmap/flag to set when used for scheduling.
    K_UCHAR *m_pucFlag;
};

//---------------------------------------------------------------------------
/*!
 * This type defines the callback function type for timer events.  Since these
 * are called from an interrupt context, they do not operate from within a
 * thread or object context directly -- as a result, the context must be
 * manually passed into the calls.
 *
 * pstOwner_ is a pointer to the thread that owns the timer
 * pvData_ is a pointer to some data or object that needs to know about the
 *         timer's expiry from within the timer interrupt context.
 */
typedef void (*TimerCallback_t)( struct _Thread *pstOwner_, void *pvData_ );

//---------------------------------------------------------------------------
struct _Timer
{
	// Inherit from LinkListNode_t -- must be first!
	LinkListNode_t clNode;

	//! Flags for the timer, defining if the timer is one-shot or repeated
	K_UCHAR m_ucFlags;

	//! Pointer to the callback function
	TimerCallback_t m_pfCallback;

	//! Interval of the timer in timer ticks
	K_ULONG m_ulInterval;

	//! Time remaining on the timer
	K_ULONG m_ulTimeLeft;

	//! Maximum tolerance (used for timer harmonization)
	K_ULONG m_ulTimerTolerance;

	//! Pointer to the owner thread
	struct _Thread  *m_pstOwner;

	//! Pointer to the callback data
	void    *m_pvData;

};

//---------------------------------------------------------------------------
/*!
    Function pointer type used for thread entrypoint functions
*/
typedef void (*ThreadEntry_t)(void *pvArg_);

//---------------------------------------------------------------------------
/*!
    Enumeration representing the different states a thread can exist in
*/
typedef enum
{
    THREAD_STATE_EXIT = 0,
    THREAD_STATE_READY,
    THREAD_STATE_BLOCKED,
    THREAD_STATE_STOP,
//--
    THREAD_STATES
} ThreadState_t;

//---------------------------------------------------------------------------
/*!
    Object providing fundamental multitasking support in the kernel.
*/
struct _Thread
{
    //! Linked-list metadata.  Must be first!
    LinkListNode_t    stLinkList;

    //! Pointer to the top of the thread's stack
    K_WORD *m_pwStackTop;

    //! Pointer to the thread's stack
    K_WORD *m_pwStack;

    //! Thread_t ID
    K_UCHAR m_ucThreadID;

    //! Default priority of the thread
    K_UCHAR m_ucPriority;

    //! Current priority of the thread (priority inheritence)
    K_UCHAR m_ucCurPriority;

    //! Enum indicating the thread's current state
    ThreadState_t m_eState;

#if KERNEL_USE_THREADNAME
    //! Thread_t name
    const K_CHAR *m_szName;
#endif

    //! Size of the stack (in bytes)
    K_USHORT m_usStackSize;

    //! Pointer to the thread-list where the thread currently resides
    struct _ThreadList *m_pstCurrent;

    //! Pointer to the thread-list where the thread resides when active
    struct _ThreadList *m_pstOwner;

    //! The entry-point function called when the thread starts
    ThreadEntry_t m_pfEntryPoint;

    //! Pointer to the argument passed into the thread's entrypoint
    void *m_pvArg;

#if KERNEL_USE_QUANTUM
    //! Thread_t quantum (in milliseconds)
    K_USHORT m_usQuantum;
#endif

#if KERNEL_USE_EVENTFLAG
    //! Event-flag mask
    K_USHORT m_usFlagMask;

    //! Event-flag mode
    EventFlagOperation_t m_eFlagMode;
#endif

#if KERNEL_USE_TIMEOUTS || KERNEL_USE_SLEEP
    //! Timer_t used for blocking-object timeouts
    struct _Timer	m_clTimer;
#endif
#if KERNEL_USE_TIMEOUTS
    //! Indicate whether or not a blocking-object timeout has occurred
    K_BOOL	m_bExpired;
#endif
};

typedef struct _Thread Thread_t;
typedef struct _Timer Timer_t;
typedef struct _ThreadList ThreadList_t;

#ifdef __cplusplus
    }
#endif

#endif
