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

    \brief  Kernel Timer_t Implementation for ATMega328p
*/

#include "kerneltypes.h"
#include "kerneltimer.h"
#include "mark3cfg.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define TCCR1B_INIT        ((1 << WGM12) | (1 << CS12))
#define TIMER_IMSK        (1 << OCIE1A)
#define TIMER_IFR        (1 << OCF1A)

//---------------------------------------------------------------------------
void KernelTimer_Config(void)
{        
    TCCR1B = TCCR1B_INIT;
}

//---------------------------------------------------------------------------
void KernelTimer_Start(void)
{
#if !KERNEL_TIMERS_TICKLESS
    TCCR1B = ((1 << WGM12) | (1 << CS11) | (1 << CS10));
    OCR1A = ((SYSTEM_FREQ / 1000) / 64);
#else
    TCCR1B |= (1 << CS12);
#endif

    TCNT1 = 0;
    TIFR1 &= ~TIMER_IFR;
    TIMSK1 |= TIMER_IMSK;
}

//---------------------------------------------------------------------------
void KernelTimer_Stop(void)
{
#if KERNEL_TIMERS_TICKLESS
    TIFR1 &= ~TIMER_IFR;
    TIMSK1 &= ~TIMER_IMSK;
    TCCR1B &= ~(1 << CS12);      // Disable count...    
    TCNT1 = 0;
    OCR1A = 0;
#endif
}

//---------------------------------------------------------------------------
K_USHORT KernelTimer_Read(void)
{
#if KERNEL_TIMERS_TICKLESS
    volatile K_USHORT usRead1;
    volatile K_USHORT usRead2;
    
    do {
        usRead1 = TCNT1;
        usRead2 = TCNT1;            
    } while (usRead1 != usRead2);
    
    return usRead1;    
#else
    return 0;
#endif
}

//---------------------------------------------------------------------------
K_ULONG KernelTimer_SubtractExpiry(K_ULONG ulInterval_)
{
#if KERNEL_TIMERS_TICKLESS
    OCR1A -= (K_USHORT)ulInterval_;        
    return (K_ULONG)OCR1A;
#else
    return 0;
#endif
}

//---------------------------------------------------------------------------
K_ULONG KernelTimer_TimeToExpiry(void)
{
#if KERNEL_TIMERS_TICKLESS
    K_USHORT usRead = KernelTimer_Read();
    K_USHORT usOCR1A = OCR1A;

    if (usRead >= usOCR1A)
    {
        return 0;
    }
    else
    {
        return (K_ULONG)(usOCR1A - usRead);    
    }
#else
    return 0;
#endif
}

//---------------------------------------------------------------------------
K_ULONG KernelTimer_GetOvertime(void)
{
    return KernelTimer_Read();
}

//---------------------------------------------------------------------------
K_ULONG KernelTimer_SetExpiry(K_ULONG ulInterval_)
{
#if KERNEL_TIMERS_TICKLESS
    K_USHORT usSetInterval;
    if (ulInterval_ > 65535)
    {
        usSetInterval = 65535;
    } 
    else 
    {
        usSetInterval = (K_USHORT)ulInterval_ ;
    }

    OCR1A = usSetInterval;
    return (K_ULONG)usSetInterval;
#else
    return 0;
#endif
}

//---------------------------------------------------------------------------
void KernelTimer_ClearExpiry(void)
{
#if KERNEL_TIMERS_TICKLESS
    OCR1A = 65535;                    // Clear the compare value
#endif
}

//---------------------------------------------------------------------------
K_UCHAR KernelTimer_DI(void)
{
#if KERNEL_TIMERS_TICKLESS
    K_BOOL bEnabled = ((TIMSK1 & (TIMER_IMSK)) != 0);
    TIFR1 &= ~TIMER_IFR;      // Clear interrupt flags
    TIMSK1 &= ~TIMER_IMSK;    // Disable interrupt
    return bEnabled;
#else
    return 0;
#endif
}

//---------------------------------------------------------------------------
void KernelTimer_EI(void)
{
    KernelTimer_RI(0);
}

//---------------------------------------------------------------------------
void KernelTimer_RI(K_BOOL bEnable_)
{
#if KERNEL_TIMERS_TICKLESS
    if (bEnable_)    
    {
        TIMSK1 |= (1 << OCIE1A);    // Enable interrupt
    }
    else
    {
        TIMSK1 &= ~(1 << OCIE1A);
    }    
#endif
}
