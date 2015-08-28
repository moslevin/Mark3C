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

//---------------------------------------------------------------------------
#define TICKS_PER_OVERFLOW              (256)
#define CLOCK_DIVIDE                    (8)

/*!
    \fn void Init()

    Initialize the global system profiler.  Must be
    called prior to use.
*/
void Profiler_Init();

/*!
    \fn void Start()

    Start the global profiling timer service.
*/
void Profiler_Start();

/*!
    \fn void Stop()

    Stop the global profiling timer service
*/
void Profiler_Stop();

/*!
    \fn K_USHORT Read()

    Read the current tick count in the timer.
*/
K_USHORT Profiler_Read();

/*!
    Process the profiling counters from ISR.
*/
void Profiler_Process();

/*!
    Return the current timer epoch
*/
K_ULONG Profiler_GetEpoch();

#endif //KERNEL_USE_PROFILER

#endif

