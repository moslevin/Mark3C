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

#include "kerneltypes.h"
#include "kernel.h"
#include "../ut_platform.h"
#include "eventflag.h"
#include "thread.h"
#include "memutil.h"
#include "driver.h"

//===========================================================================
// Local Defines
//===========================================================================
Thread_t clThread1;
Thread_t clThread2;

#define THREAD1_STACK_SIZE      (256)
K_WORD aucThreadStack1[THREAD1_STACK_SIZE];
#define THREAD2_STACK_SIZE      (160)
K_WORD aucThreadStack2[THREAD2_STACK_SIZE];

EventFlag_t clFlagGroup;
volatile K_UCHAR ucFlagCount = 0;
volatile K_UCHAR ucTimeoutCount = 0;

//---------------------------------------------------------------------------
void WaitOnFlag1Any(void *unused_)
{
    EventFlag_Wait( &clFlagGroup, 0x0001, EVENT_FLAG_ANY);
    ucFlagCount++;

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
void WaitOnMultiAny(void *unused_)
{
    EventFlag_Wait( &clFlagGroup, 0x5555, EVENT_FLAG_ANY);
    ucFlagCount++;

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
void WaitOnMultiAll(void *unused_)
{
    EventFlag_Wait( &clFlagGroup, 0x5555, EVENT_FLAG_ALL);
    ucFlagCount++;

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
void WaitOnAny(void *mask_)
{
    K_USHORT usMask = *((K_USHORT*)mask_);
    while(1)
    {
        EventFlag_Wait( &clFlagGroup, usMask, EVENT_FLAG_ANY);
        ucFlagCount++;
        EventFlag_Clear( &usMask, usMask);
    }
}

//---------------------------------------------------------------------------
void WaitOnAll(void *mask_)
{
    K_USHORT usMask = *((K_USHORT*)mask_);
    while(1)
    {
        EventFlag_Wait( &clFlagGroup, usMask, EVENT_FLAG_ALL);
        ucFlagCount++;
        EventFlag_Clear( &usMask, usMask);
    }
}

//---------------------------------------------------------------------------
void TimedWait(void *time_)
{
    K_USHORT usRet;
    K_USHORT usTime = *((K_USHORT*)time_);
    usRet = EventFlag_TimedWait( &clFlagGroup, 0x0001, EVENT_FLAG_ALL, usTime);
    if (usRet == 0x0001)
    {
        ucFlagCount++;
    }
    else if (usRet == 0x0000)
    {
        ucTimeoutCount++;
    }
    EventFlag_Clear( &clFlagGroup, 0x0001);
    Thread_Exit( Scheduler_GetCurrentThread() );

}


//---------------------------------------------------------------------------
void TimedWaitAll(void *time_)
{
    K_USHORT usRet;
    K_USHORT usTime = *((K_USHORT*)time_);
    while(1)
    {
        usRet = EventFlag_TimedWait( &clFlagGroup, 0x0001, EVENT_FLAG_ALL, 200);
        if (usRet == 0x0001)
        {
            ucFlagCount++;
        }
        else if (usRet == 0x0000)
        {            
            Thread_SetExpired( Scheduler_GetCurrentThread(), false );
            ucTimeoutCount++;
        }
        EventFlag_Clear( &clFlagGroup, 0x0001);
    }

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//===========================================================================
// Define Test Cases Here
//===========================================================================
TEST(ut_waitany)
{
    // Test - ensure that threads can block using the "waitany" mechanism, and
    // only wake up when bits from its pattern are encountered.
    K_USHORT i;
    K_USHORT usMask = 0x8000;

    EventFlag_Init( &clFlagGroup );
    ucFlagCount = 0;

    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, WaitOnAny, (void*)(&usMask));
    Thread_Start( &clThread1 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    usMask = 0x0001;
    while(usMask)
    {
        EventFlag_Set( &clFlagGroup, usMask);
        Thread_Sleep(100);

        if (usMask != 0x8000)
        {
            EXPECT_EQUALS(ucFlagCount, 0);
        }
        else
        {
            EXPECT_EQUALS(ucFlagCount, 1);
        }

        usMask <<= 1;
    }
    Thread_Exit( &clThread1 );

    // Okay, that was a single bit-flag test.  Now let's try using a multi-bit flag
    // and verify that any matching pattern will cause a wakeup

    EventFlag_Init( &clFlagGroup );
    ucFlagCount = 0;
    usMask = 0xAAAA;

    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, WaitOnAny, (void*)(&usMask));
    Thread_Start( &clThread1 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    // Test point - the flag set should kick the test thread on even-indexed
    // counters indexes.
    for (i = 0; i < 16; i++)
    {
        K_UCHAR ucLastFlagCount = ucFlagCount;

        EventFlag_Set( &clFlagGroup, (K_USHORT)(1 << i));

        Thread_Sleep(100);
        if ((i & 1) == 0)
        {
            EXPECT_EQUALS(ucFlagCount, ucLastFlagCount);
        }
        else
        {
            EXPECT_EQUALS(ucFlagCount, ucLastFlagCount+1);
        }
    }

    Thread_Exit( &clThread1 );
}
TEST_END


//===========================================================================
TEST(ut_waitall)
{
    // Test - ensure that threads can block using the "waitany" mechanism, and
    // only wake up when bits from its pattern are encountered.
    K_USHORT i;
    K_USHORT usMask = 0x8000;

    EventFlag_Init( &clFlagGroup );
    ucFlagCount = 0;

    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, WaitOnAll, (void*)(&usMask));
    Thread_Start( &clThread1 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    usMask = 0x0001;
    while(usMask)
    {
        EventFlag_Set( &clFlagGroup, usMask);
        Thread_Sleep(100);

        if (usMask != 0x8000)
        {
            EXPECT_EQUALS(ucFlagCount, 0);
        }
        else
        {
            EXPECT_EQUALS(ucFlagCount, 1);
        }

        usMask <<= 1;
    }
    Thread_Exit( &clThread1 );

    // Okay, that was a single bit-flag test.  Now let's try using a multi-bit flag
    // and verify that any matching pattern will cause a wakeup

    EventFlag_Init( &clFlagGroup );
    ucFlagCount = 0;
    usMask = 0xAAAA;

    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, WaitOnAll, (void*)(&usMask));
    Thread_Start( &clThread1 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    // Test point - the flag set should kick the test thread on even-indexed
    // counters indexes.
    for (i = 0; i < 16; i++)
    {
        K_UCHAR ucLastFlagCount = ucFlagCount;

        EventFlag_Set( &clFlagGroup, (K_USHORT)(1 << i));

        Thread_Sleep(100);
        if (i != 15)
        {
            EXPECT_EQUALS(ucFlagCount, ucLastFlagCount);
        }
        else
        {
            EXPECT_EQUALS(ucFlagCount, ucLastFlagCount+1);
        }
    }

    Thread_Exit( &clThread1 );
}
TEST_END

//---------------------------------------------------------------------------
TEST(ut_flag_multiwait)
{

    // Test - ensure that all forms of event-flag unblocking work when there
    // are multiple threads blocked on the same flag.

    EventFlag_Init( &clFlagGroup );

    // Test point - 2 threads blocking on an event flag, bit 1.  Wait on these
    // threads until this thread sets bit 0x0001.  When that bit is set, the
    // threads should wake up, incrementing the "ucFlagCount" variable.
    ucFlagCount = 0;
    EventFlag_Clear( &clFlagGroup, 0xFFFF);

    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, WaitOnFlag1Any, 0);
    Thread_Init( &clThread2, aucThreadStack2, THREAD2_STACK_SIZE, 7, WaitOnFlag1Any, 0);

    Thread_Start( &clThread1 );
    Thread_Start( &clThread2 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0x0001);

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 2);

    ucFlagCount = 0;
    EventFlag_Clear( &clFlagGroup, 0xFFFF);

    // Test point - 2 threads blocking on an event flag, bits 0x5555.  Block
    // on these threads, and verify that only bits in the pattern will cause
    // the threads to awaken
    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, WaitOnMultiAny, 0);
    Thread_Init( &clThread2, aucThreadStack2, THREAD2_STACK_SIZE, 7, WaitOnMultiAny, 0);

    Thread_Start( &clThread1 );
    Thread_Start( &clThread2 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0xAAAA);
    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0x5555);
    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 2);

    ucFlagCount = 0;
    EventFlag_Clear( &clFlagGroup, 0xFFFF);


    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, WaitOnMultiAny, 0);
    Thread_Init( &clThread2, aucThreadStack2, THREAD2_STACK_SIZE, 7, WaitOnMultiAny, 0);

    Thread_Start( &clThread1 );
    Thread_Start( &clThread2 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0xA000);
    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0x0005);
    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 2);
    // Test point - same thing as above, but with the "ALL" flags set.

    ucFlagCount = 0;
    EventFlag_Clear( &clFlagGroup, 0xFFFF);

    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, WaitOnMultiAll, 0);
    Thread_Init( &clThread2, aucThreadStack2, THREAD2_STACK_SIZE, 7, WaitOnMultiAll, 0);

    Thread_Start( &clThread1 );
    Thread_Start( &clThread2 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0xAAAA);
    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0x5555);
    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 2);


    ucFlagCount = 0;
    EventFlag_Clear( &clFlagGroup, 0xFFFF);

    // "All" mode - each flag must be set in order to ensure that the threads
    // unblock.
    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, WaitOnMultiAll, 0);
    Thread_Init( &clThread2, aucThreadStack2, THREAD2_STACK_SIZE, 7, WaitOnMultiAll, 0);

    Thread_Start( &clThread1 );
    Thread_Start( &clThread2 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0xAAAA);
    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0x5500);
    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0x0055);
    Thread_Sleep(100);

    EXPECT_EQUALS(ucFlagCount, 2);
}
TEST_END

//===========================================================================
TEST(ut_timedwait)
{
    K_USHORT usInterval;

    // Test point - verify positive test case (no timeout, no premature
    // unblocking)
    ucTimeoutCount = 0;
    ucFlagCount = 0;
    usInterval = 200;

    EventFlag_Init( &clFlagGroup );

    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, TimedWait, (void*)&usInterval);
    Thread_Start( &clThread1 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucTimeoutCount, 0);
    EXPECT_EQUALS(ucFlagCount, 0);

    EventFlag_Set( &clFlagGroup, 0x0001);

    EXPECT_EQUALS(ucTimeoutCount, 0);
    EXPECT_EQUALS(ucFlagCount, 1);

    // Test point - verify negative test case (timeouts), followed by a
    // positive test result.
    ucTimeoutCount = 0;
    ucFlagCount = 0;
    usInterval = 200;

    EventFlag_Init( &clFlagGroup );
    EventFlag_Clear( &clFlagGroup, 0xFFFF);

    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, TimedWait, (void*)&usInterval);
    Thread_Start( &clThread1 );

    Thread_Sleep(100);

    EXPECT_EQUALS(ucTimeoutCount, 0);
    EXPECT_EQUALS(ucFlagCount, 0);

    Thread_Sleep(200);

    EXPECT_EQUALS(ucTimeoutCount, 1);
    EXPECT_EQUALS(ucFlagCount, 0);

    // Test point - verify negative test case (timeouts), followed by a
    // positive test result.
    ucTimeoutCount = 0;
    ucFlagCount = 0;
    usInterval = 200;

    EventFlag_Init( &clFlagGroup );
    EventFlag_Clear( &clFlagGroup, 0xFFFF);

    Thread_Init( &clThread1, aucThreadStack1, THREAD1_STACK_SIZE, 7, TimedWaitAll, (void*)&usInterval);
    Thread_Start( &clThread1 );

    Thread_Sleep(210);
    EXPECT_EQUALS(ucTimeoutCount, 1);
    EXPECT_EQUALS(ucFlagCount, 0);

    Thread_Sleep(210);
    EXPECT_EQUALS(ucTimeoutCount, 2);
    EXPECT_EQUALS(ucFlagCount, 0);

    Thread_Sleep(210);
    EXPECT_EQUALS(ucTimeoutCount, 3);
    EXPECT_EQUALS(ucFlagCount, 0);

    Thread_Sleep(210);
    EXPECT_EQUALS(ucTimeoutCount, 4);
    EXPECT_EQUALS(ucFlagCount, 0);

    Thread_Sleep(210);
    EXPECT_EQUALS(ucTimeoutCount, 5);
    EXPECT_EQUALS(ucFlagCount, 0);

    Thread_Sleep(80);
    EventFlag_Set( &clFlagGroup, 0x0001);

    EXPECT_EQUALS(ucTimeoutCount, 5);
    EXPECT_EQUALS(ucFlagCount, 1);

    Thread_Sleep(80);
    EventFlag_Set( &clFlagGroup, 0x0001);

    EXPECT_EQUALS(ucTimeoutCount, 5);
    EXPECT_EQUALS(ucFlagCount, 2);

    Thread_Sleep(80);
    EventFlag_Set( &clFlagGroup, 0x0001);

    EXPECT_EQUALS(ucTimeoutCount, 5);
    EXPECT_EQUALS(ucFlagCount, 3);

    Thread_Sleep(80);
    EventFlag_Set( &clFlagGroup, 0x0001);

    EXPECT_EQUALS(ucTimeoutCount, 5);
    EXPECT_EQUALS(ucFlagCount, 4);

    Thread_Sleep(80);
    EventFlag_Set( &clFlagGroup, 0x0001);

    EXPECT_EQUALS(ucTimeoutCount, 5);
    EXPECT_EQUALS(ucFlagCount, 5);
}
TEST_END

//===========================================================================
// Test Whitelist Goes Here
//===========================================================================
TEST_CASE_START
  TEST_CASE(ut_waitany),
  TEST_CASE(ut_waitall),
  TEST_CASE(ut_flag_multiwait),
  TEST_CASE(ut_timedwait),
TEST_CASE_END
