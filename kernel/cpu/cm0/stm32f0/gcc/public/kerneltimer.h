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

    \file   kerneltimer.h    

    \brief  Kernel Timer Class declaration
*/

#include "kerneltypes.h"

#ifndef __KERNELTIMER_H_
#define __KERNELTIMER_H_

//---------------------------------------------------------------------------
#define SYSTEM_FREQ		48000000
#define TIMER_FREQ		1000	

//---------------------------------------------------------------------------
/*!
    \fn void Config(void)

    Initializes the kernel timer before use
*/
void KernelTimer_Config(void);

/*!
    \fn void Start(void)

    Starts the kernel time (must be configured first)
*/
void KernelTimer_Start(void);

/*!
    \fn void Stop(void)

    Shut down the kernel timer, used when no timers are scheduled
*/
void KernelTimer_Stop(void);

/*!
    \fn K_UCHAR DI(void)

    Disable the kernel timer's expiry interrupt
*/
K_UCHAR KernelTimer_DI(void);

/*!
    \fn void RI(K_BOOL bEnable_)

    Retstore the state of the kernel timer's expiry interrupt.

    \param bEnable_ 1 enable, 0 disable
*/
void KernelTimer_RI(K_BOOL bEnable_);

/*!
    \fn void EI(void)

    Enable the kernel timer's expiry interrupt
*/
void KernelTimer_EI(void);

/*!
    \fn K_ULONG SubtractExpiry(K_ULONG ulInterval_)

    Subtract the specified number of ticks from the timer's
    expiry count register.  Returns the new expiry value stored in
    the register.

    \param ulInterval_ Time (in HW-specific) ticks to subtract
    \return Value in ticks stored in the timer's expiry register
*/
K_ULONG KernelTimer_SubtractExpiry(K_ULONG ulInterval_);

/*!
    \fn K_ULONG TimeToExpiry(void)

    Returns the number of ticks remaining before the next timer
    expiry.

    \return Time before next expiry in platform-specific ticks
*/
K_ULONG KernelTimer_TimeToExpiry(void);

/*!
    \fn K_ULONG SetExpiry(K_ULONG ulInterval_)

    Resets the kernel timer's expiry interval to the specified value

    \param ulInterval_ Desired interval in ticks to set the timer for
    \return Actual number of ticks set (may be less than desired)
*/
K_ULONG KernelTimer_SetExpiry(K_ULONG ulInterval_);

/*!
    \fn K_ULONG GetOvertime(void)

    Return the number of ticks that have elapsed since the last
    expiry.

    \return Number of ticks that have elapsed after last timer expiration
*/
K_ULONG KernelTimer_GetOvertime(void);

/*!
    \fn void ClearExpiry(void)

    Clear the hardware timer expiry register
*/
void KernelTimer_ClearExpiry(void);

/*!
    \fn K_USHORT Read(void)

    Safely read the current value in the timer register

    \return Value held in the timer register
*/
K_USHORT KernelTimer_Read(void);

#endif //__KERNELTIMER_H_
