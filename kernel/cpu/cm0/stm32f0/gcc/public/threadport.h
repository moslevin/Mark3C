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

    \file   threadport.h    

    \brief  Cortex M-0 Multithreading support.
*/

#ifndef __THREADPORT_H_
#define __THREADPORT_H_

#include "kerneltypes.h"
#include "thread.h"

#include <stm32f0xx.h>

//---------------------------------------------------------------------------
//! ASM Macro - simplify the use of ASM directive in C
#define ASM      asm volatile

//---------------------------------------------------------------------------
//! Macro to find the top of a stack given its size and top address
#define TOP_OF_STACK(x, y)        (K_WORD*) ( ((K_ULONG)x) + (y - sizeof(K_WORD)) )
//! Push a value y to the stack pointer x and decrement the stack pointer
#define PUSH_TO_STACK(x, y)        *x = y; x--;

//------------------------------------------------------------------------
//! These macros *must* be used in matched-pairs !
//! Nesting *is* supported !
extern volatile K_ULONG g_ulCriticalCount;

//------------------------------------------------------------------------
#ifndef xDMB
    #define xDMB()					ASM(" dmb \n");
#endif
#ifndef xdisable_irq
    #define xdisable_irq()			ASM(" cpsid i \n");
#endif
#ifndef xenable_irq
    #define xenable_irq()			ASM(" cpsie i \n");
#endif

#define ENABLE_INTS()		{ xDMB(); xenable_irq(); }
#define DISABLE_INTS()		{ xdisable_irq(); xDMB(); }

//------------------------------------------------------------------------
//! Enter critical section (copy current PRIMASK register value, disable interrupts)
#define CS_ENTER()	\
{ \
    DISABLE_INTS(); \
    g_ulCriticalCount++;\
}
//------------------------------------------------------------------------
//! Exit critical section (restore previous PRIMASK status register value)
#define CS_EXIT() \
{ \
    g_ulCriticalCount--; \
    if( 0 == g_ulCriticalCount ) { \
        ENABLE_INTS(); \
    } \
}


/*!
    \fn void StartThreads()

    Function to start the scheduler, initial threads, etc.
*/
void ThreadPort_StartThreads();

/*!
    \fn void InitStack(Thread *pstThread_)

    Initialize the thread's stack.

    \param pstThread_ Pointer to the thread to initialize
*/
void ThreadPort_InitStack(Thread_t *pstThread_);

#endif //__ThreadPORT_H_
