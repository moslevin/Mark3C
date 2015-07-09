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

//---------------------------------------------------------------------------

#include "mark3.h"
#include "drvUART.h"
#include "unit_test.h"
#include "ut_platform.h"
#include "memutil.h"

#if defined(AVR)
#include <avr/io.h>
#include <avr/sleep.h>
#endif

//---------------------------------------------------------------------------
// Global objects
static Thread_t AppThread;			//!< Main "application" thread
static K_WORD aucAppStack[STACK_SIZE_APP];

//---------------------------------------------------------------------------
#if !KERNEL_USE_IDLE_FUNC
static Thread_t IdleThread;			//!< Idle thread - runs when app can't
static K_UCHAR aucIdleStack[STACK_SIZE_IDLE];
#endif

//---------------------------------------------------------------------------
static K_UCHAR aucTxBuffer[UART_SIZE_TX];
static K_UCHAR aucRxBuffer[UART_SIZE_RX];

//---------------------------------------------------------------------------
static void AppEntry(void);
static void IdleEntry(void);

//---------------------------------------------------------------------------
void MyUnitTest_PrintTestResult( UnitTest_t *pstTest_ )
{
    K_CHAR acTemp[6];
    int iLen;

    PrintString("Test ");
    PrintString(UnitTest_GetName( pstTest_ ));
    PrintString(": ");
    iLen = MemUtil_StringLength(UnitTest_GetName( pstTest_ ));
    if (iLen >= 32)
    {
        iLen = 32;
    }
    uint8_t i;
    for (i = 0; i < 32 - iLen; i++)
    {
        PrintString(".");
    }
    if (UnitTest_GetPassed(pstTest_) == UnitTest_GetTotal(pstTest_))
    {
        PrintString("(PASS)[");
    }
    else
    {
        PrintString("(FAIL)[");
    }
    MemUtil_DecimalToString16(UnitTest_GetPassed(pstTest_), (K_CHAR*)acTemp);
    PrintString((const K_CHAR*)acTemp);
    PrintString("/");
    MemUtil_DecimalToString16(UnitTest_GetTotal(pstTest_), (K_CHAR*)acTemp);
    PrintString((const K_CHAR*)acTemp);
    PrintString("]\n");
}

typedef void (*FuncPtr)(void);
//---------------------------------------------------------------------------
void run_tests()
{
    MyTestCase *pstTestCase;
    pstTestCase = astTestCases;

    while (pstTestCase->pstTestCase)
    {
        pstTestCase->pfTestFunc();
        pstTestCase++;
    }
    PrintString("--DONE--\n");
    Thread_Sleep(100);

    FuncPtr pfReset = 0;
    pfReset();
}

//---------------------------------------------------------------------------
void init_tests()
{
    MyTestCase *pstTestCase;
    pstTestCase = astTestCases;

    while (pstTestCase->pstTestCase)
    {
        UnitTest_SetName( pstTestCase->pstTestCase, pstTestCase->szName );
        pstTestCase++;
    }
}

//---------------------------------------------------------------------------
void PrintString(const K_CHAR *szStr_)
{
    K_CHAR *szTemp = (K_CHAR*)szStr_;
    while (*szTemp)
    {
        while( 1 != Driver_Write( (Driver_t*)&stUART, 1, (K_UCHAR*)szTemp ) ) { /* Do nothing */ }
        szTemp++;
    }
}

//---------------------------------------------------------------------------
void AppEntry(void)
{
    {

        ATMegaUART_Init( &stUART );
        Driver_Control( (Driver_t*)&stUART, CMD_SET_BUFFERS, UART_SIZE_RX, aucRxBuffer, UART_SIZE_TX, aucTxBuffer );
        Driver_Open( (Driver_t*)&stUART );

        init_tests();
    }

    while(1)
    {
        run_tests();
    }
}

//---------------------------------------------------------------------------
void IdleEntry(void)
{
#if !KERNEL_USE_IDLE_FUNC
    while(1)
    {
#endif

#if defined(AVR)
        // LPM code;
        set_sleep_mode(SLEEP_MODE_IDLE);
        cli();
        sleep_enable();
        sei();
        sleep_cpu();
        sleep_disable();
        sei();
#endif

#if !KERNEL_USE_IDLE_FUNC
    }
#endif

}

//---------------------------------------------------------------------------
int main(void)
{
    Kernel_Init();						//!< MUST be before other kernel ops

    Thread_Init(	&AppThread,
                    aucAppStack,		//!< Pointer to the stack
                    STACK_SIZE_APP,		//!< Size of the stack
                    1,					//!< Thread priority
                    (ThreadEntry_t)AppEntry,	//!< Entry function
                    (void*)&AppThread );//!< Entry function argument

    Thread_Start( &AppThread );					//!< Schedule the threads

#if KERNEL_USE_IDLE_FUNC
    Kernel_SetIdleFunc(IdleEntry);
#else
    Thread_Init(    &IdleThread,
                    aucIdleStack,		//!< Pointer to the stack
                    STACK_SIZE_IDLE,	//!< Size of the stack
                    0,					//!< Thread priority
                    (ThreadEntry_t)IdleEntry,	//!< Entry function
                    NULL );			//!< Entry function argument

    Thread_Start( &IdleThread );
#endif

    Kernel_Start();					//!< Start the kernel!
    return 0;
}
