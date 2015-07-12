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
#include "timerlist.h"
#include "thread.h"
#include "kernelprofile.h"
#include "profile.h"
#include "kernel.h"
#include "ksemaphore.h"
#include "kerneltimer.h"
#include "driver.h"
#include "memutil.h"

//===========================================================================
// Local Defines
//===========================================================================
static Timer_t clTimer1;
static Timer_t clTimer2;
static Timer_t clTimer3;
static Semaphore_t clTimerSem;
static ProfileTimer_t clProfileTimer;
static ProfileTimer_t clProfileTimer2;
static ProfileTimer_t clProfileTimer3;
static K_ULONG ulTimeVal;
static K_ULONG ulTempTime;
static volatile K_ULONG ulCallbackCount = 0;

static void TimerCallback( Thread_t *pclOwner_, void *pvVal_ )
{
    Semaphore_Post( &clTimerSem );
    ulCallbackCount++;
}

//===========================================================================
// Define Test Cases Here
//===========================================================================
TEST(ut_timer_tolerance)
{
    Profiler_Start();
    Semaphore_Init( &clTimerSem, 0, 1);

    // Test point - 1ms Timer_t should take at least 1ms
    ProfileTimer_Init( &clProfileTimer );
    ProfileTimer_Start( &clProfileTimer );

    Timer_Start( &clTimer1, false, 1, TimerCallback, 0 );
    Semaphore_Pend( &clTimerSem );
    ProfileTimer_Stop( &clProfileTimer );

    ulTimeVal = ProfileTimer_GetCurrent( &clProfileTimer ) * CLOCK_DIVIDE;
    ulTempTime = SYSTEM_FREQ / 1000;
    EXPECT_GT(ulTimeVal, ulTempTime);

    // Test point - 1ms Timer_t should be no more than 3ms
    ulTempTime *= 3;
    EXPECT_LT(ulTimeVal, ulTempTime);

    // Test point - 10ms Timer_t should take at least 10ms
    ProfileTimer_Init( &clProfileTimer );
    ProfileTimer_Start( &clProfileTimer );

    Timer_Start( &clTimer1, false, 10, TimerCallback, 0 );
    Semaphore_Pend( &clTimerSem );
    ProfileTimer_Stop( &clProfileTimer );

    ulTimeVal = ProfileTimer_GetCurrent( &clProfileTimer ) * CLOCK_DIVIDE;
    ulTempTime = SYSTEM_FREQ / 100;

    EXPECT_GT(ulTimeVal, ulTempTime);

    // Test point - 10ms Timer_t should be no more than 12ms
    ulTempTime += 2* (SYSTEM_FREQ / 1000);
    EXPECT_LT(ulTimeVal, ulTempTime);

    // Test point - 100ms Timer_t should take at least 100ms
    ProfileTimer_Init( &clProfileTimer );
    ProfileTimer_Start( &clProfileTimer );

    Timer_Start( &clTimer1, false, 100, TimerCallback, 0 );
    Semaphore_Pend( &clTimerSem );
    ProfileTimer_Stop( &clProfileTimer );

    ulTimeVal = ProfileTimer_GetCurrent( &clProfileTimer ) * CLOCK_DIVIDE;
    ulTempTime = SYSTEM_FREQ / 10;

    EXPECT_GT(ulTimeVal, ulTempTime);

    // Test point - 100ms Timer_t should be no more than 102ms
    ulTempTime += 2 * (SYSTEM_FREQ / 1000);
    EXPECT_LT(ulTimeVal, ulTempTime);

    // Test point - 1000ms Timer_t should take at least 100ms
    ProfileTimer_Init( &clProfileTimer );
    ProfileTimer_Start( &clProfileTimer );

    Timer_Start( &clTimer1, false, 1000, TimerCallback, 0 );
    Semaphore_Pend( &clTimerSem );
    ProfileTimer_Stop( &clProfileTimer );

    ulTimeVal = ProfileTimer_GetCurrent( &clProfileTimer ) * CLOCK_DIVIDE;
    ulTempTime = SYSTEM_FREQ;

    EXPECT_GT(ulTimeVal, ulTempTime);

    // Test point - 1000ms Timer_t should be no more than 1002ms
    ulTempTime += 2* (SYSTEM_FREQ / 1000);
    EXPECT_LT(ulTimeVal, ulTempTime);

    Profiler_Stop();
}
TEST_END


TEST(ut_timer_longrun)
{
    // Profiling Timer_t is not really designed for long profiling
    // operations (1.2 seconds is about as high as we get, since it's
    // so high resolution).  So, use sleeps and multiple iterations
    // w/averaging in order to verify.

    K_ULONG ulSleepCount = 0;
    Profiler_Start();
    Semaphore_Init( &clTimerSem, 0, 1);

    // Test point - long running Timer_t accuracy; 10-second Timer_t
    // expires after 10 seconds.
    ProfileTimer_Init( &clProfileTimer );
    Timer_Start( &clTimer1, false, 10000, TimerCallback, 0 );
    ulCallbackCount = 0;

    while (!ulCallbackCount)
    {
        ProfileTimer_Start( &clProfileTimer );
        Thread_Sleep(100);
        ProfileTimer_Stop( &clProfileTimer );
        ulSleepCount++;
    }

    ProfileTimer_Stop( &clProfileTimer );

    ulTimeVal = ProfileTimer_GetAverage( &clProfileTimer ) * CLOCK_DIVIDE * ulSleepCount;
    ulTempTime = SYSTEM_FREQ * 10;

    EXPECT_GT(ulTimeVal, ulTempTime);

    // Test point - 100ms accuracy over 10 seconds
    ulTempTime += SYSTEM_FREQ / 10;
    EXPECT_LT(ulTimeVal, ulTempTime);

    Profiler_Stop();
}
TEST_END

TEST(ut_timer_repeat)
{
    // Profiling Timer_t is not really designed for long profiling
    // operations (1.2 seconds is about as high as we get, since it's
    // so high resolution).  So, use sleeps and multiple iterations
    // w/averaging in order to verify.
    K_ULONG ulSleepCount = 0;
    Profiler_Start();
    Semaphore_Init( &clTimerSem, 0, 1);

    // Repeated Timer_t case - run a 10ms Timer_t 100 times and measure
    // accuracy.  Average iteration must be > 10ms
    ulCallbackCount = 0;

    ProfileTimer_Init( &clProfileTimer );
    ProfileTimer_Start( &clProfileTimer );

    Timer_Start( &clTimer1,  true, 10, TimerCallback, 0 );

    while (ulCallbackCount < 100)
    {
        Semaphore_Pend( &clTimerSem );
    }

    ProfileTimer_Stop( &clProfileTimer );
    Timer_Stop( &clTimer1 );

    ulTimeVal = ProfileTimer_GetCurrent( &clProfileTimer ) * CLOCK_DIVIDE;
    ulTempTime = SYSTEM_FREQ;

    EXPECT_GT(ulTimeVal, ulTempTime);

#if KERNEL_TIMERS_TICKLESS
    // Test point - 50ms (5%) maximum tolerance for callback overhead, etc.
    ulTempTime += SYSTEM_FREQ / 20;
    EXPECT_LT(ulTimeVal, ulTempTime);
#else
    // Test point - 100ms (10%) maximum tolerance for callback overhead, etc.
    ulTempTime += SYSTEM_FREQ / 10;
    EXPECT_LT(ulTimeVal, ulTempTime);
#endif

#if 0
    // Debug code to print out the profiling times
    Driver *pclDriver = DriverList_FindByPath("/dev/tty");
    K_CHAR acData[13];
    MemUtil_DecimalToString(ulTimeVal, acData);
    pclDriver->Write( MemUtil_StringLength(acData), (K_UCHAR*)acData);
    pclDriver->Write(1, (K_UCHAR*)(" "));
    MemUtil_DecimalToString(ulTempTime, acData);
    pclDriver->Write( MemUtil_StringLength(acData), (K_UCHAR*)acData);
#endif

    Profiler_Stop();

}
TEST_END

TEST(ut_timer_multi)
{
    Profiler_Start();
    Semaphore_Init( &clTimerSem, 0, 3);

    // Test using multiple timers simultaneously, verify that
    // each of them expire at the expected times within a specific
    // tolerance

    ProfileTimer_Init( &clProfileTimer );
    ProfileTimer_Init( &clProfileTimer2 );
    ProfileTimer_Init( &clProfileTimer3 );

    ProfileTimer_Start( &clProfileTimer );
    Timer_Start( &clTimer1, false, 100, TimerCallback, 0 );
    ProfileTimer_Start( &clProfileTimer2 );
    Timer_Start( &clTimer2, false, 200, TimerCallback, 0 );
    ProfileTimer_Start( &clProfileTimer3 );
    Timer_Start( &clTimer3, false, 50, TimerCallback, 0 );

    // Each Timer_t expiry will post the Semaphore_t.
    Semaphore_Pend( &clTimerSem );
    ProfileTimer_Stop( &clProfileTimer3 );

    Semaphore_Pend( &clTimerSem );
    ProfileTimer_Stop( &clProfileTimer );

    Semaphore_Pend( &clTimerSem );
    ProfileTimer_Stop( &clProfileTimer2 );

    // Test Point - Timer_t 1 expired @ 100ms, with a 1 ms tolerance
    ulTimeVal = ProfileTimer_GetCurrent( &clProfileTimer ) * CLOCK_DIVIDE;
    ulTempTime = SYSTEM_FREQ / 10;
    EXPECT_GT(ulTimeVal, ulTempTime);

    ulTempTime += SYSTEM_FREQ / 1000;
    EXPECT_LT(ulTimeVal, ulTempTime);

    // Test Point - Timer_t 2 expired @ 200ms, with a 1 ms tolerance
    ulTimeVal = ProfileTimer_GetCurrent( &clProfileTimer2 ) * CLOCK_DIVIDE;
    ulTempTime = SYSTEM_FREQ / 5;
    EXPECT_GT(ulTimeVal, ulTempTime);

    ulTempTime += SYSTEM_FREQ / 1000;
    EXPECT_LT(ulTimeVal, ulTempTime);

    // Test Point - Timer_t 3 expired @ 50ms, with a 1 ms tolerance
    ulTimeVal = ProfileTimer_GetCurrent( &clProfileTimer3 ) * CLOCK_DIVIDE;
    ulTempTime = SYSTEM_FREQ / 20;
    EXPECT_GT(ulTimeVal, ulTempTime);

    ulTempTime += SYSTEM_FREQ / 1000;
    EXPECT_LT(ulTimeVal, ulTempTime);

    Profiler_Stop();
}
TEST_END


//===========================================================================
// Test Whitelist Goes Here
//===========================================================================
TEST_CASE_START
  TEST_CASE(ut_timer_tolerance),
  TEST_CASE(ut_timer_longrun),
  TEST_CASE(ut_timer_repeat),
  TEST_CASE(ut_timer_multi),
TEST_CASE_END
