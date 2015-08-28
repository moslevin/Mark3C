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

    \file   kerneltimer.cpp

    \brief  Kernel Timer Implementation for ARM Cortex-M0
*/

#include "kerneltypes.h"
#include "kerneltimer.h"
#include "threadport.h"

//---------------------------------------------------------------------------
void KernelTimer_Config(void)
{
	   
}

//---------------------------------------------------------------------------
void KernelTimer_Start(void)
{	
	SysTick_Config(SYSTEM_FREQ / 1000); // 1KHz fixed clock...
	NVIC_EnableIRQ(SysTick_IRQn);
}

//---------------------------------------------------------------------------
void KernelTimer_Stop(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

//---------------------------------------------------------------------------
K_USHORT KernelTimer_Read(void)
{
	// Not implemented in this port
	return 0;
}

//---------------------------------------------------------------------------
K_ULONG KernelTimer_SubtractExpiry(K_ULONG ulInterval_)
{
	return 0;
}

//---------------------------------------------------------------------------
K_ULONG KernelTimer_TimeToExpiry(void)
{
	return 0;
}

//---------------------------------------------------------------------------
K_ULONG KernelTimer_GetOvertime(void)
{
    return 0;
}

//---------------------------------------------------------------------------
K_ULONG KernelTimer_SetExpiry(K_ULONG ulInterval_)
{	
	return 0;
}

//---------------------------------------------------------------------------
void KernelTimer_ClearExpiry(void)
{
}

//-------------------------------------------------------------------------
K_UCHAR KernelTimer_DI(void)
{
	return 0;
}

//---------------------------------------------------------------------------
void KernelTimer_EI(void)
{	
    KernelTimer_RI(0);
}

//---------------------------------------------------------------------------
void KernelTimer_RI(K_BOOL bEnable_)
{
}

//---------------------------------------------------------------------------
