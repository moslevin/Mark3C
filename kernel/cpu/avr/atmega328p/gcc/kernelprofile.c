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
    \file kernelprofile.cpp
    
    \brief ATMega328p Profiling timer implementation
*/

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "profile.h"
#include "kernelprofile.h"
#include "threadport.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#if KERNEL_USE_PROFILER
K_ULONG m_ulEpoch;

//---------------------------------------------------------------------------
void Profiler_Init( void )
{
    TCCR0A = 0;
    TCCR0B = 0;
    TIFR0 = 0;
    TIMSK0 = 0;
    m_ulEpoch = 0;
}

//---------------------------------------------------------------------------
void Profiler_Start( void )
{
    TIFR0 = 0;
    TCNT0 = 0;
    TCCR0B |= (1 << CS01);
    TIMSK0 |= (1 << TOIE0);
}    

//---------------------------------------------------------------------------
void Profiler_Stop( void )
{
    TIFR0 = 0;
    TCCR0B &= ~(1 << CS01);
    TIMSK0 &= ~(1 << TOIE0);
}    
//---------------------------------------------------------------------------
K_USHORT Profiler_Read( void )
{
    K_USHORT usRet;
    CS_ENTER();
    TCCR0B &= ~(1 << CS01);
    usRet = TCNT0;
    TCCR0B |= (1 << CS01);
    CS_EXIT();
    return usRet;
}

//---------------------------------------------------------------------------
void Profiler_Process( void )
{
    CS_ENTER();
    m_ulEpoch++;
    CS_EXIT();
}

//---------------------------------------------------------------------------
K_ULONG Profiler_GetEpoch( void )
{ 
	return m_ulEpoch; 
}

//---------------------------------------------------------------------------
ISR(TIMER0_OVF_vect)
{
    Profiler_Process();
}

#endif
