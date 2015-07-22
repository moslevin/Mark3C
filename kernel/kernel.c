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

    \file   kernel.cpp

    \brief  Kernel initialization and startup code
*/

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "kernel.h"
#include "scheduler.h"
#include "thread.h"
#include "threadport.h"
#include "timerlist.h"
#include "message.h"
#include "driver.h"
#include "profile.h"
#include "kernelprofile.h"
#include "tracebuffer.h"
#include "kerneldebug.h"
#include "kernelaware.h"

K_BOOL bIsStarted;
K_BOOL bIsPanic;
panic_func_t pfPanic;

#if KERNEL_USE_IDLE_FUNC
idle_func_t pfIdle;
FakeThread_t stIdle;
#endif

//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	KERNEL_C       //!< File ID used in kernel trace calls

//---------------------------------------------------------------------------
void Kernel_Init(void)
{
    bIsStarted = false;
    bIsPanic = false;
    pfPanic = 0;

#if KERNEL_AWARE_SIMULATION
    g_ucKACommand = KA_COMMAND_IDLE;
    g_bIsKernelAware = g_bIsKernelAware;
#endif

#if KERNEL_USE_DEBUG & !KERNEL_AWARE_SIMULATION
	TraceBuffer_Init();
#endif

#if KERNEL_USE_IDLE_FUNC
	Thread_InitIdle( (Thread_t*)&stIdle );
	pfIdle = 0;
#endif
	
	KERNEL_TRACE( STR_MARK3_INIT );

    // Initialize the global kernel data - scheduler, timer-scheduler, and
    // the global message pool.	
    Scheduler_Init();
#if KERNEL_USE_DRIVER
	DriverList_Init();
#endif	
#if KERNEL_USE_TIMERS    
    TimerScheduler_Init();
#endif
#if KERNEL_USE_MESSAGE    
    GlobalMessagePool_Init();
#endif
#if KERNEL_USE_PROFILER
	Profiler_Init();
#endif
}
    
//---------------------------------------------------------------------------
void Kernel_Start(void)
{
	KERNEL_TRACE( STR_THREAD_START );    
    bIsStarted = true;
    ThreadPort_StartThreads();
	KERNEL_TRACE( STR_START_ERROR );
}

//---------------------------------------------------------------------------
void Kernel_Panic(K_USHORT usCause_)
{
    bIsPanic = true;
    if (pfPanic)
    {
        pfPanic(usCause_);
    }
    else
    {
#if KERNEL_AWARE_SIMULATION
        KernelAware_ExitSimulator();
#endif
        while(1);
    }
}

//---------------------------------------------------------------------------
K_BOOL Kernel_IsStarted( void )    
{   
	return bIsStarted;    
}

//---------------------------------------------------------------------------
void Kernel_SetPanic( panic_func_t pfPanic_ ) 
{ 
	pfPanic = pfPanic_; 
}
	
//---------------------------------------------------------------------------
K_BOOL Kernel_IsPanic( void )      
{   
	return bIsPanic;   
}
	
//---------------------------------------------------------------------------
void Kernel_SetIdleFunc( idle_func_t pfIdle_ )  
{   
	pfIdle = pfIdle_; 
}
	
//---------------------------------------------------------------------------
void Kernel_IdleFunc( void ) 
{ 
	if (pfIdle != 0 ) 
	{ 
		pfIdle(); 
	} 
}

//---------------------------------------------------------------------------
Thread_t *Kernel_GetIdleThread( void ) 
{ 
	return (Thread_t*)&stIdle; 
}

