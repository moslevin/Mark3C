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

    \file   kernelaware.h

    \brief  Kernel aware simulation support
*/

#ifndef __KERNEL_AWARE_H__
#define __KERNEL_AWARE_H__

#include "mark3cfg.h"
#include "kerneltypes.h"

#if KERNEL_AWARE_SIMULATION

#ifdef __cplusplus
    extern "C" {
#endif

extern volatile K_BOOL         g_bIsKernelAware;        //!< Will be set to true by a kernel-aware host.
extern volatile K_UCHAR        g_ucKACommand;           //!< Kernel-aware simulator command to execute

//---------------------------------------------------------------------------
/*!
    This enumeration contains a list of supported commands that can be
    executed to invoke a response from a kernel aware host.
*/
typedef enum
{
    KA_COMMAND_IDLE = 0,        //!< Null command, does nothing.
    KA_COMMAND_PROFILE_INIT,    //!< Initialize a new profiling session
    KA_COMMAND_PROFILE_START,   //!< Begin a profiling sample
    KA_COMMAND_PROFILE_STOP,    //!< End a profiling sample
    KA_COMMAND_PROFILE_REPORT,  //!< Report current profiling session
    KA_COMMAND_EXIT_SIMULATOR,  //!< Terminate the host simulator
    KA_COMMAND_TRACE_0,         //!< 0-argument kernel trace
    KA_COMMAND_TRACE_1,         //!< 1-argument kernel trace
    KA_COMMAND_TRACE_2,         //!< 2-argument kernel trace
    KA_COMMAND_PRINT            //!< Print an arbitrary string of data
} KernelAwareCommand_t;

//---------------------------------------------------------------------------
/*!
 * \brief The KernelAware object
 *
 * This object contains functions that are used to trigger kernel-aware
 * functionality within a supported simulation environment (i.e. flAVR).
 *
 * These static methods operate on a singleton set of global variables,
 * which are monitored for changes from within the simulator.  The simulator
 * hooks into these variables by looking for the correctly-named symbols in
 * an elf-formatted binary being run and registering callbacks that are called
 * whenever the variables are changed.  On each change of the command variable,
 * the kernel-aware data is analyzed and interpreted appropriately.
 *
 * If these methods are run in an unsupported simulator or on actual hardware
 * the commands generally have no effect (except for the exit-on-reset command,
 * which will result in a jump-to-0 reset).
 */

//---------------------------------------------------------------------------
/*!
 * \brief ProfileInit
 *
 * Initializes the kernel-aware profiler.  This function instructs the
 * kernel-aware simulator to reset its accounting variables, and prepare to
 * start counting profiling data tagged to the given string.  How this is
 * handled is the responsibility of the simulator.
 *
 * \param szStr_ String to use as a tag for the profilng session.
 */
void KernelAware_ProfileInit( const K_CHAR *szStr_ );

//---------------------------------------------------------------------------
/*!
 * \brief ProfileStart
 *
 * Instruct the kernel-aware simulator to begin counting cycles towards the
 * current profiling counter.
 *
 */
void KernelAware_ProfileStart( void );

//---------------------------------------------------------------------------
/*!
 * \brief ProfileStop
 *
 * Instruct the kernel-aware simulator to end counting cycles relative to the
 * current profiling counter's iteration.
 */
void KernelAware_ProfileStop( void );

//---------------------------------------------------------------------------
/*!
 * \brief ProfileReport
 *
 * Instruct the kernel-aware simulator to print a report for its current
 * profiling data.
 *
 */
void KernelAware_ProfileReport( void );

//---------------------------------------------------------------------------
/*!
 * \brief ExitSimulator
 *
 * Instruct the kernel-aware simulator to terminate (destroying the virtual
 * CPU).
 *
 */
void KernelAware_ExitSimulator( void );

//---------------------------------------------------------------------------
/*!
 * \brief Print
 *
 * Instruct the kernel-aware simulator to print a char string
 *
 * \param szStr_
 */
void KernelAware_Print( const K_CHAR *szStr_ );

//---------------------------------------------------------------------------
/*!
 * \brief Trace
 *
 * Insert a kernel trace statement into the kernel-aware simulator's debug
 * data stream.
 *
 * \param usFile_   16-bit code representing the file
 * \param usLine_   16-bit code representing the line in the file
 * \param usCode_   16-bit data code, which indicates the line's format.
 */
void KernelAware_Trace( K_USHORT usFile_,
              K_USHORT usLine_,
              K_USHORT usCode_ );

//---------------------------------------------------------------------------
/*!
 * \brief Trace
 *
 * Insert a kernel trace statement into the kernel-aware simulator's debug
 * data stream.
 *
 * \param usFile_   16-bit code representing the file
 * \param usLine_   16-bit code representing the line in the file
 * \param usCode_   16-bit data code, which indicates the line's format
 * \param usArg1_   16-bit argument to the format string.
 */
void KernelAware_Trace1( K_USHORT usFile_,
              K_USHORT usLine_,
              K_USHORT usCode_,
              K_USHORT usArg1_);

//---------------------------------------------------------------------------
/*!
 * \brief Trace
 *
 * Insert a kernel trace statement into the kernel-aware simulator's debug
 * data stream.
 *
 * \param usFile_   16-bit code representing the file
 * \param usLine_   16-bit code representing the line in the file
 * \param usCode_   16-bit data code, which indicates the line's format
 * \param usArg1_   16-bit argument to the format string.
 * \param usArg2_   16-bit argument to the format string.
 */
void KernelAware_Trace2( K_USHORT usFile_,
              K_USHORT usLine_,
              K_USHORT usCode_,
              K_USHORT usArg1_,
              K_USHORT usArg2_);

//---------------------------------------------------------------------------
/*!
 * \brief IsSimulatorAware
 *
 * Use this function to determine whether or not the code is running on a
 * simulator that is aware of the kernel.
 *
 * \return true - the application is being run in a kernel-aware simulator.
 *         false - otherwise.
 */
K_BOOL KernelAware_IsSimulatorAware(void);

#ifdef __cplusplus
    }
#endif

#endif

#endif
