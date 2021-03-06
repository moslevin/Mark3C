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

    \file   kernelswi.cpp

    \brief  Kernel Software interrupt implementation for ATMega328p

*/

#include "kerneltypes.h"
#include "kernelswi.h"

#include <avr/io.h>
#include <avr/interrupt.h>

//---------------------------------------------------------------------------
void KernelSWI_Config(void)
{
    PORTD &= ~0x04; // Clear INT0
    DDRD |= 0x04;    // Set PortD, bit 2 (INT0) As Output
    EICRA |= (1 << ISC00) | (1 << ISC01);    // Rising edge on INT0
}

//---------------------------------------------------------------------------
void KernelSWI_Start(void)
{        
    EIFR &= ~(1 << INTF0);    // Clear any pending interrupts on INT0
    EIMSK |= (1 << INT0);    // Enable INT0 interrupt (as K_LONG as I-bit is set)
}

//---------------------------------------------------------------------------
void KernelSWI_Stop(void)
{
    EIMSK &= ~(1 << INT0);    // Disable INT0 interrupts
}

//---------------------------------------------------------------------------
K_UCHAR KernelSWI_DI()
{
    K_BOOL bEnabled = ((EIMSK & (1 << INT0)) != 0);
    EIMSK &= ~(1 << INT0);
    return bEnabled;
}

//---------------------------------------------------------------------------
void KernelSWI_RI(K_BOOL bEnable_)
{
    if (bEnable_)
    {
        EIMSK |= (1 << INT0);    
    }
    else
    {
        EIMSK &= ~(1 << INT0);    
    }
}

//---------------------------------------------------------------------------
void KernelSWI_Clear(void)
{
    EIFR &= ~(1 << INTF0);     // Clear the interrupt flag for INT0
}

//---------------------------------------------------------------------------
void KernelSWI_Trigger(void)
{
    //if(Thread_IsSchedulerEnabled())
    {
        PORTD &= ~0x04;
        PORTD |= 0x04;    
    }    
}
