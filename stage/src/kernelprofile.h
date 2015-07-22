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
    \file kernelprofile.h
    
    \brief Profiling timer hardware interface
*/

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "ll.h"

#ifndef __KPROFILE_H__
#define __KPROFILE_H__

#if KERNEL_USE_PROFILER

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
#define TICKS_PER_OVERFLOW              (256)
#define CLOCK_DIVIDE                    (8)

//---------------------------------------------------------------------------
/*!
    System profiling timer interface
*/

/*!
    \fn void Init()
        
    Initialize the global system profiler.  Must be 
    called prior to use.
*/
void Profiler_Init( void );
    
/*!
    \fn void Start()
        
    Start the global profiling timer service.
*/
void Profiler_Start( void );
    
/*!
    \fn void Stop()
        
    Stop the global profiling timer service
*/
void Profiler_Stop( void );
    
/*!
    \fn K_USHORT Read()
        
    Read the current tick count in the timer.  
*/
K_USHORT Profiler_Read( void );
    
/*!
    Process the profiling counters from ISR.
*/
void Profiler_Process( void );
    
/*!
    Return the current timer epoch    
*/
K_ULONG Profiler_GetEpoch( void );

#ifdef __cplusplus
    }
#endif

#endif //KERNEL_USE_PROFILER

#endif

