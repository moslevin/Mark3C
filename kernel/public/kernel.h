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

    \file   kernel.h    

    \brief  Kernel initialization and startup object
    
    The Kernel namespace provides functions related to initializing and 
    starting up the kernel.
    
    The Kernel::Init() function must be called before any of the other
    functions in the kernel can be used.
    
    Once the initial kernel configuration has been completed (i.e. first 
    threads have been added to the scheduler), the Kernel::Start() 
    function can then be called, which will transition code execution from
    the "main()" context to the threads in the scheduler.    
*/

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "mark3cfg.h"
#include "kerneltypes.h"
#include "paniccodes.h"
#include "thread.h"

#ifdef __cplusplus
    extern "C" {
#endif

#if KERNEL_USE_IDLE_FUNC
typedef void (*idle_func_t)(void);
#endif

//---------------------------------------------------------------------------
/*!
	Class that encapsulates all of the kernel startup functions.
*/
  
/*!
	Kernel Initialization Function, call before any other OS function
        
	\fn Init()
        
    Initializes all global resources used by the operating system.  This 
    must be called before any other kernel function is invoked.
*/
void Kernel_Init( void );
        
/*!
	Start the kernel; function never returns
	
    \fn Start()
        
    Start the operating system kernel - the current execution context is
    cancelled, all kernel services are started, and the processor resumes
    execution at the entrypoint for the highest-priority thread.

    You must have at least one thread added to the kernel before calling
    this function, otherwise the behavior is undefined.
*/
void Kernel_Start( void );	

/*!
    \brief IsStarted
    \return Whether or not the kernel has started - true = running, false =
            not started
    */
K_BOOL Kernel_IsStarted( void );

/*!
    * \brief SetPanic Set a function to be called when a kernel panic occurs,
    *        giving the user to determine the behavior when a catastrophic
    *        failure is observed.
    *
    * \param pfPanic_ Panic function pointer
    */
void Kernel_SetPanic( panic_func_t pfPanic_ );

/*!
    * \brief IsPanic Returns whether or not the kernel is in a panic state
    * \return Whether or not the kernel is in a panic state
    */
K_BOOL Kernel_IsPanic( void );

/*!
    * \brief Panic Cause the kernel to enter its panic state
    * \param usCause_ Reason for the kernel panic
    */
void Kernel_Panic(K_USHORT usCause_);

#if KERNEL_USE_IDLE_FUNC
/*!
    * \brief SetIdleFunc Set the function to be called when no active threads
    *        are available to be scheduled by the scheduler.
    * \param pfIdle_ Pointer to the idle function
    */
void Kernel_SetIdleFunc( idle_func_t pfIdle_ );

/*!
    * \brief IdleFunc Call the low-priority idle function when no active
    *        threads are available to be scheduled.
    */
void Kernel_IdleFunc( void );

/*!
    * \brief GetIdleThread Return a pointer to the Kernel's idle thread
    *        object to the user.  Note that the Thread_t object involved is
    *        to be used for comparisons only -- the thread itself is "virtual",
    *        and doesn't represent a unique execution context with its own stack.
    * \return Pointer to the Kernel's idle thread object
    */
Thread_t *Kernel_GetIdleThread( void );

#ifdef __cplusplus
    }
#endif

#endif


#endif

