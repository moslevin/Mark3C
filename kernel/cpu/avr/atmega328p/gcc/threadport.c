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

    \file   threadport.cpp   

    \brief  ATMega328p Multithreading

*/

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "thread.h"
#include "threadport.h"
#include "kernelswi.h"
#include "kerneltimer.h"
#include "timerlist.h"
#include "quantum.h"
#include "kernel.h"
#include "kernelaware.h"
#include <avr/io.h>
#include <avr/interrupt.h>

//---------------------------------------------------------------------------
void ThreadPort_InitStack(Thread_t *pstThread_)
{
    // Initialize the stack for a Thread_t
    K_USHORT usAddr;
    K_UCHAR *pucStack;
    K_USHORT i;

    // Get the address of the thread's entry function
    usAddr = (K_USHORT)(pstThread_->pfEntryPoint);

    // Start by finding the bottom of the stack
    pucStack = (K_UCHAR*)pstThread_->pwStackTop;

    // clear the stack, and initialize it to a known-default value (easier
    // to debug when things go sour with stack corruption or overflow)
    for (i = 0; i < pstThread_->usStackSize; i++)
    {
        pstThread_->pwStack[i] = 0xFF;
    }

    // Our context starts with the entry function
    PUSH_TO_STACK(pucStack, (K_UCHAR)(usAddr & 0x00FF));
    PUSH_TO_STACK(pucStack, (K_UCHAR)((usAddr >> 8) & 0x00FF));

    // R0
    PUSH_TO_STACK(pucStack, 0x00);    // R0

    // Push status register and R1 (which is used as a constant zero)
    PUSH_TO_STACK(pucStack, 0x80);  // SR
    PUSH_TO_STACK(pucStack, 0x00);  // R1

    // Push other registers
    for (i = 2; i <= 23; i++) //R2-R23
    {
        PUSH_TO_STACK(pucStack, i);
    }

    // Assume that the argument is the only stack variable
    PUSH_TO_STACK(pucStack, (K_UCHAR)(((K_USHORT)(pstThread_->pvArg)) & 0x00FF));    //R24
    PUSH_TO_STACK(pucStack, (K_UCHAR)((((K_USHORT)(pstThread_->pvArg))>>8) & 0x00FF)); //R25

    // Push the rest of the registers in the context
    for (i = 26; i <=31; i++)
    {
        PUSH_TO_STACK(pucStack, i);
    }
    
    // Set the top o' the stack.
    pstThread_->pwStackTop = (K_UCHAR*)pucStack;

    // That's it!  the thread is ready to run now.
}

//---------------------------------------------------------------------------
static void Thread_Switch(void)
{
#if KERNEL_USE_IDLE_FUNC
    // If there's no next-thread-to-run...
    if (g_pstNext == Kernel_GetIdleThread())
    {
        g_pstCurrent = Kernel_GetIdleThread();

        // Disable the SWI, and re-enable interrupts -- enter nested interrupt
        // mode.
        KernelSWI_DI();

        K_UCHAR ucSR = _SFR_IO8(SR_);

        // So long as there's no "next-to-run" thread, keep executing the Idle
        // function to conclusion...

        while (g_pstNext == Kernel_GetIdleThread())
        {
           // Ensure that we run this block in an interrupt enabled context (but
           // with the rest of the checks being performed in an interrupt disabled
           // context).
           ASM( "sei" );
           Kernel_IdleFunc();
           ASM( "cli" );
        }

        // Progress has been achieved -- an interrupt-triggered event has caused
        // the scheduler to run, and choose a new thread.  Since we've already
        // saved the context of the thread we've hijacked to run idle, we can
        // proceed to disable the nested interrupt context and switch to the
        // new thread.

        _SFR_IO8(SR_) = ucSR;
        KernelSWI_RI( true );        
    }
#endif
    g_pstCurrent = (Thread_t*)g_pstNext;
}


//---------------------------------------------------------------------------
void ThreadPort_StartThreads()
{
    KernelSWI_Config();                 // configure the task switch SWI
    KernelTimer_Config();               // configure the kernel timer
    
    Scheduler_SetScheduler(1);          // enable the scheduler
    Scheduler_Schedule();               // run the scheduler - determine the first thread to run

    Thread_Switch();                     // Set the next scheduled thread to the current thread

    KernelTimer_Start();                // enable the kernel timer
    KernelSWI_Start();                  // enable the task switch SWI

    // Restore the context...
    Thread_RestoreContext();        // restore the context of the first running thread
    ASM("reti");                    // return from interrupt - will return to the first scheduled thread
}

//---------------------------------------------------------------------------
/*!
    SWI using INT0 - used to trigger a context switch
    \fn ISR(INT0_vect) __attribute__ ( ( signal, naked ) );
*/
//---------------------------------------------------------------------------
ISR(INT0_vect) __attribute__ ( ( signal, naked ) );
ISR(INT0_vect)
{
    Thread_SaveContext();       // Push the context (registers) of the current task
    Thread_Switch();            // Switch to the next task
    Thread_RestoreContext();    // Pop the context (registers) of the next task
    ASM("reti");                // Return to the next task
}

//---------------------------------------------------------------------------
/*!
    Timer_t interrupt ISR - causes a tick, which may cause a context switch
    \fn ISR(TIMER1_COMPA_vect) ;
*/
//---------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
#if KERNEL_USE_TIMERS    
    TimerScheduler_Process();
#endif    
#if KERNEL_USE_QUANTUM    
    QuantuUpdateTimer();
#endif
}
