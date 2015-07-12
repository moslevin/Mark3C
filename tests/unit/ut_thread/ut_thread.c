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
#include "thread.h"
#include "ksemaphore.h"
#include "kernelprofile.h"
#include "profile.h"
#include "kerneltimer.h"
#include "driver.h"
#include "memutil.h"
//===========================================================================
// Local Defines
//===========================================================================
#define TEST_STACK_SIZE     (224)
static K_WORD aucStack1[TEST_STACK_SIZE];
static K_WORD aucStack2[TEST_STACK_SIZE];
static K_WORD aucStack3[TEST_STACK_SIZE];

static Thread_t clThread1;
static Thread_t clThread2;
static Thread_t clThread3;

static Semaphore_t clSem1;
static Semaphore_t clSem2;

static volatile K_ULONG ulRR1;
static volatile K_ULONG ulRR2;
static volatile K_ULONG ulRR3;

//===========================================================================
static void Thread_tEntryPoint1(void *unused_)
{
    while(1)
    {
        Semaphore_Pend( &clSem2 );
        Semaphore_Post( &clSem1 );
    }

    unused_ = unused_;
}

//===========================================================================
// Define Test Cases Here
//===========================================================================
TEST(ut_thread_create)
{
    // Test point - Create a Thread_t, verify that the Thread_t actually starts.
    Semaphore_Init( &clSem1, 0, 1);
    Semaphore_Init( &clSem2, 0, 1);

    // Initialize our Thread_t
    Thread_Init( &clThread1, aucStack1, TEST_STACK_SIZE, 7, Thread_tEntryPoint1, NULL);

    // Start the Thread_t (Thread_ts are created in the stopped state)
    Thread_Start( &clThread1 );

    // Poke the Thread_t using a Semaphore_t, verify it's working
    Semaphore_Post( &clSem2 );
    Semaphore_TimedPend( &clSem1, 10 );

    // Ensure that the Semaphore_t was posted before we got to the 10ms timeout
    EXPECT_FALSE( Thread_GetExpired( Scheduler_GetCurrentThread() ) );
}
TEST_END

//===========================================================================
TEST(ut_thread_stop)
{
    // Test point - stop and restart a Thread_t
    Thread_Stop( &clThread1 );
    Thread_Sleep(10);
    Thread_Start( &clThread1 );

    // Poke the Thread_t using a Semaphore_t, verify it's still responding
    Semaphore_Post( &clSem2 );
    Semaphore_TimedPend( &clSem1, 10 );

    EXPECT_FALSE( Thread_GetExpired( Scheduler_GetCurrentThread() ) );
}
TEST_END

//===========================================================================
TEST(ut_thread_exit)
{
    // Test point - force a Thread_t exit; ensure it doesn't respond once
    // it's un-scheduled.
    Thread_Exit( &clThread1 );
    Semaphore_Post( &clSem2 );
    Semaphore_TimedPend( &clSem1, 100 );

    EXPECT_TRUE( Thread_GetExpired( Scheduler_GetCurrentThread() ) );
}
TEST_END

//===========================================================================
static ProfileTimer_t clProfiler1;
static void Thread_tSleepEntryPoint(void *unused_)
{
    unused_ = unused_;

    // Thread_t will sleep for various intervals, synchronized
    // to Semaphore_t-based IPC.
    Semaphore_Pend( &clSem1 );
    Thread_Sleep(5);
    Semaphore_Post( &clSem2 );

    Semaphore_Pend( &clSem1 );
    Thread_Sleep(50);
    Semaphore_Post( &clSem2 );

    Semaphore_Pend( &clSem1 );
    Thread_Sleep(500);
    Semaphore_Post( &clSem2 );

    // Exit this Thread_t.
    Thread_Exit( Scheduler_GetCurrentThread() );
}

//===========================================================================
TEST(ut_thread_sleep)
{
    Profiler_Init();
    Profiler_Start();

    // Start another Thread_t, which sleeps for a various length of time
    Semaphore_Init( &clSem1, 0, 1);
    Semaphore_Init( &clSem2, 0, 1);

    // Initialize our Thread_t
    Thread_Init( &clThread1, aucStack1, TEST_STACK_SIZE, 7, Thread_tSleepEntryPoint, NULL);

    // Start the Thread_t (Thread_ts are created in the stopped state)
    Thread_Start( &clThread1 );

    ProfileTimer_Init( &clProfiler1 );
    ProfileTimer_Start( &clProfiler1 );
    Semaphore_Post( &clSem1 );
    Semaphore_Pend( &clSem2 );
    ProfileTimer_Stop( &clProfiler1 );


    EXPECT_GTE( (ProfileTimer_GetCurrent( &clProfiler1 ) * CLOCK_DIVIDE), (SYSTEM_FREQ / 200));
    EXPECT_LTE( (ProfileTimer_GetCurrent( &clProfiler1 ) * CLOCK_DIVIDE), (SYSTEM_FREQ / 200) + (SYSTEM_FREQ / 200) );

    ProfileTimer_Init( &clProfiler1 );
    ProfileTimer_Start( &clProfiler1 );
    Semaphore_Post( &clSem1 );
    Semaphore_Pend( &clSem2 );
    ProfileTimer_Stop( &clProfiler1 );

    EXPECT_GTE( (ProfileTimer_GetCurrent( &clProfiler1 ) * CLOCK_DIVIDE), SYSTEM_FREQ / 20 );
    EXPECT_LTE( (ProfileTimer_GetCurrent( &clProfiler1 ) * CLOCK_DIVIDE), (SYSTEM_FREQ / 20) + (SYSTEM_FREQ / 200));


    ProfileTimer_Init( &clProfiler1 );
    ProfileTimer_Start( &clProfiler1 );
    Semaphore_Post( &clSem1 );
    Semaphore_Pend( &clSem2 );
    ProfileTimer_Stop( &clProfiler1 );

    EXPECT_GTE( (ProfileTimer_GetCurrent( &clProfiler1 ) * CLOCK_DIVIDE), SYSTEM_FREQ / 2 );
    EXPECT_LTE( (ProfileTimer_GetCurrent( &clProfiler1 ) * CLOCK_DIVIDE), (SYSTEM_FREQ / 2) + (SYSTEM_FREQ / 200) );

    Profiler_Stop();
}
TEST_END

//===========================================================================
void RR_EntryPoint(void *value_)
{
    volatile K_ULONG *pulValue = (K_ULONG*)value_;
    while(1)
    {
        (*pulValue)++;
    }
}

//===========================================================================
TEST(ut_roundrobin)
{
    K_ULONG ulAvg;
    K_ULONG ulMax;
    K_ULONG ulMin;
    K_ULONG ulRange;

    // Create three Thread_ts that only increment counters, and keep them at
    // the same priority in order to test the roundrobin functionality of
    // the scheduler
    Thread_Init( &clThread1, aucStack1, TEST_STACK_SIZE, 1, RR_EntryPoint, (void*)&ulRR1);
    Thread_Init( &clThread2, aucStack2, TEST_STACK_SIZE, 1, RR_EntryPoint, (void*)&ulRR2);
    Thread_Init( &clThread3, aucStack3, TEST_STACK_SIZE, 1, RR_EntryPoint, (void*)&ulRR3);

    ulRR1 = 0;
    ulRR2 = 0;
    ulRR3 = 0;

    // Adjust Thread_t priority before starting test Thread_ts to ensure
    // they all start at the same time (when we hit the 1 second sleep)
    Thread_SetPriority( Scheduler_GetCurrentThread(), 2);
    Thread_Start( &clThread1 );
    Thread_Start( &clThread2 );
    Thread_Start( &clThread3 );

    Thread_Sleep(5000);

    // When the sleep ends, this will preempt the Thread_t in progress,
    // allowing us to stop them, and drop priority.
    Thread_Stop( &clThread1 );
    Thread_Stop( &clThread2 );
    Thread_Stop( &clThread3 );
    Thread_SetPriority( Scheduler_GetCurrentThread(), 1);

    // Compare the three counters - they should be nearly identical
    if (ulRR1 > ulRR2)
    {
        ulMax = ulRR1;
    }
    else
    {
        ulMax = ulRR2;
    }
    if (ulMax < ulRR3)
    {
        ulMax = ulRR3;
    }

    if (ulRR1 < ulRR2)
    {
        ulMin = ulRR1;
    }
    else
    {
        ulMin = ulRR2;
    }
    if (ulMin > ulRR3)
    {
        ulMin = ulRR3;
    }
    ulRange = ulMax - ulMin;
    ulAvg = (ulRR1 + ulRR2 + ulRR3) / 3;

    // Max-Min delta should not exceed 1% of average for this simple test
    EXPECT_LT( ulRange, ulAvg / 100);

    // Make sure none of the component values are 0
    EXPECT_FAIL_EQUALS( ulRR1, 0 );
    EXPECT_FAIL_EQUALS( ulRR2, 0 );
    EXPECT_FAIL_EQUALS( ulRR3, 0 );

}
TEST_END

//===========================================================================
TEST(ut_quanta)
{
    K_ULONG ulAvg;
    K_ULONG ulMax;
    K_ULONG ulMin;
    K_ULONG ulRange;

    // Create three Thread_ts that only increment counters - similar to the
    // previous test.  However, modify the Thread_t quanta such that each Thread_t
    // will get a different proportion of the CPU cycles.
    Thread_Init( &clThread1, aucStack1, TEST_STACK_SIZE, 1, RR_EntryPoint, (void*)&ulRR1);
    Thread_Init( &clThread2, aucStack2, TEST_STACK_SIZE, 1, RR_EntryPoint, (void*)&ulRR2);
    Thread_Init( &clThread3, aucStack3, TEST_STACK_SIZE, 1, RR_EntryPoint, (void*)&ulRR3);

    ulRR1 = 0;
    ulRR2 = 0;
    ulRR3 = 0;

    // Adjust Thread_t priority before starting test Thread_ts to ensure
    // they all start at the same time (when we hit the 1 second sleep)
    Thread_SetPriority( Scheduler_GetCurrentThread(), 2);

    // Set a different execution quanta for each Thread_t
    Thread_SetQuantum( &clThread1, 3);
    Thread_SetQuantum( &clThread2, 6);
    Thread_SetQuantum( &clThread3, 9);

    Thread_Start( &clThread1 );
    Thread_Start( &clThread2 );
    Thread_Start( &clThread3 );

    Thread_Sleep(1800);

    // When the sleep ends, this will preempt the Thread_t in progress,
    // allowing us to stop them, and drop priority.
    Thread_Stop( &clThread1 );
    Thread_Stop( &clThread2 );
    Thread_Stop( &clThread3 );
    Thread_SetPriority( Scheduler_GetCurrentThread(), 1);

    // Test point - make sure that Q3 > Q2 > Q1
    EXPECT_GT( ulRR2, ulRR1 );
    EXPECT_GT( ulRR3, ulRR2 );

    // scale the counters relative to the largest value, and compare.
    ulRR1 *= 3;
    ulRR2 *= 3;
    ulRR2 = (ulRR2 + 1) / 2;

    // After scaling, they should be nearly identical (well, close at least)
    if (ulRR1 > ulRR2)
    {
        ulMax = ulRR1;
    }
    else
    {
        ulMax = ulRR2;
    }
    if (ulMax < ulRR3)
    {
        ulMax = ulRR3;
    }

    if (ulRR1 < ulRR2)
    {
        ulMin = ulRR1;
    }
    else
    {
        ulMin = ulRR2;
    }
    if (ulMin > ulRR3)
    {
        ulMin = ulRR3;
    }
    ulRange = ulMax - ulMin;
    ulAvg = (ulRR1 + ulRR2 + ulRR3) / 3;

#if KERNEL_TIMERS_TICKLESS
    // Max-Min delta should not exceed 5% of average for this test
    EXPECT_LT( ulRange, ulAvg / 20);
#else
    // Max-Min delta should not exceed 20% of average for this test -- tick-based timers
    // are coarse, and prone to Thread_t preference due to phase.
    EXPECT_LT( ulRange, ulAvg / 5);
#endif


    // Make sure none of the component values are 0
    EXPECT_FAIL_EQUALS( ulRR1, 0 );
    EXPECT_FAIL_EQUALS( ulRR2, 0 );
    EXPECT_FAIL_EQUALS( ulRR3, 0 );
}
TEST_END

//===========================================================================
// Test Whitelist Goes Here
//===========================================================================
TEST_CASE_START
  TEST_CASE(ut_thread_create),
  TEST_CASE(ut_thread_stop),
  TEST_CASE(ut_thread_exit),
  TEST_CASE(ut_thread_sleep),
  TEST_CASE(ut_roundrobin),
  TEST_CASE(ut_quanta),
TEST_CASE_END
