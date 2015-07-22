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
#include "mutex.h"

//===========================================================================
// Local Defines
//===========================================================================
#define MUTEX_STACK_SIZE        (256)
static K_WORD aucTestStack[MUTEX_STACK_SIZE];
static Thread_t stMutexThread;

static K_WORD aucTestStack2[MUTEX_STACK_SIZE];
static Thread_t stTestThread2;
static volatile K_UCHAR ucToken;

//===========================================================================
// Define Test Cases Here
//===========================================================================

void TypicalMutexTest(void *mutex_)
{
    Mutex_t *pstMutex = (Mutex_t*)mutex_;

    Mutex_Claim( pstMutex );
    ucToken = 0x69;
    Mutex_Release( pstMutex );

    // Exit the thread when we're done this operation.
    Thread_Exit( Scheduler_GetCurrentThread() );
}

TEST(ut_typical_mutex)
{
    // Test - Typical mutex usage, ensure that two threads can synchronize
    // access to a single resource
    Mutex_t stMutex;

    Mutex_Init( &stMutex );

    // Create a higher-priority thread that will immediately pre-empt us.
    // Verify that while we have the mutex held, that the high-priority thread
    // is blocked waiting for us to relinquish access.
    Thread_Init( &stMutexThread, aucTestStack, MUTEX_STACK_SIZE, 7, TypicalMutexTest, (void*)&stMutex);

    Mutex_Claim( &stMutex );

    ucToken = 0x96;
    Thread_Start( &stMutexThread );

    // Spend some time sleeping, just to drive the point home...
    Thread_Sleep(100);

    // Test Point - Verify that the token value hasn't changed (which would
    // indicate the high-priority thread held the mutex...)
    EXPECT_EQUALS( ucToken, 0x96 );

    // Relese the mutex, see what happens.
    Mutex_Release( &stMutex );

    // Test Point - Verify that after releasing the mutex, the higher-priority
    // thread immediately resumes, claiming the mutex, and adjusting the
    // token value to its value.  Check the new token value here.
    EXPECT_EQUALS( ucToken, 0x69 );

}
TEST_END

//===========================================================================
void TimedMutexTest(void *mutex_)
{
    Mutex_t *pstMutex = (Mutex_t*)mutex_;

    Mutex_Claim( pstMutex );
    Thread_Sleep(20);
    Mutex_Release( pstMutex );

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//===========================================================================
TEST(ut_timed_mutex)
{
    // Test - Enusre that when a thread fails to obtain a resource in a
    // timeout scenario, that the timeout is reported correctly

    Mutex_t stMutex;
    Mutex_Init( &stMutex );

    Thread_Init( &stMutexThread, aucTestStack, MUTEX_STACK_SIZE, 7, TimedMutexTest, (void*)&stMutex);
    Thread_Start( &stMutexThread );

    EXPECT_FALSE( Mutex_TimedClaim( &stMutex, 10 ) );

    Thread_Sleep(20);

    Thread_Init( &stMutexThread, aucTestStack, MUTEX_STACK_SIZE, 7, TimedMutexTest, (void*)&stMutex);
    Thread_Start( &stMutexThread );

    EXPECT_TRUE( Mutex_TimedClaim( &stMutex, 30 ) );
}
TEST_END

//===========================================================================
void LowPriThread(void *mutex_)
{
    Mutex_t *pstMutex = (Mutex_t*)mutex_;

    Mutex_Claim( pstMutex );

    Thread_Sleep( 100 );

    Mutex_Release( pstMutex );

    while(1)
    {        
        Thread_Sleep(1000);
    }
}

//===========================================================================
void HighPriThread(void *mutex_)
{
    Mutex_t *pstMutex = (Mutex_t*)mutex_;

    Mutex_Claim( pstMutex );

    Thread_Sleep(100);

    Mutex_Release( pstMutex );

    while(1)
    {
        Thread_Sleep(1000);
    }
}

//===========================================================================
TEST(ut_priority_mutex)
{
    // Test - Priority inheritence protocol.  Ensure that the priority
    // inversion problem is correctly avoided by our semaphore implementation
    // In the low/med/high scenario, we play the "med" priority thread
    Mutex_t stMutex;
    Mutex_Init( &stMutex );

    Thread_SetPriority( Scheduler_GetCurrentThread(), 3 );

    Thread_Init( &stMutexThread, aucTestStack, MUTEX_STACK_SIZE, 2, LowPriThread, (void*)&stMutex);
    Thread_Init( &stTestThread2, aucTestStack2, MUTEX_STACK_SIZE, 4, HighPriThread, (void*)&stMutex);

    // Start the low-priority thread and give it the mutex
    Thread_Start( &stMutexThread );
    Thread_Sleep(20);

    // Start the high-priority thread, which will block, waiting for the
    // low-priority action to complete...
    Thread_Start( &stTestThread2 );
    Thread_Sleep(20);

    // Test point - Low-priority thread boost:
    // Check the priorities of the threads.  The low-priority thread
    // should now have the same priority as the high-priority thread

    EXPECT_EQUALS(Thread_GetCurPriority( &stMutexThread ), 4 );
    EXPECT_EQUALS(Thread_GetCurPriority( &stTestThread2 ), 4 );

    Thread_Sleep(2000);

    // Test point - Low-priority thread drop:
    // After the threads have relinquished their mutexes, ensure that
    // they are placed back at their correct priorities

    EXPECT_EQUALS( Thread_GetCurPriority( &stMutexThread ), 2 );
    EXPECT_EQUALS( Thread_GetCurPriority( &stTestThread2 ), 4 );

    Thread_Exit( &stMutexThread );
    Thread_Exit( &stTestThread2 );
}
TEST_END


//===========================================================================
// Test Whitelist Goes Here
//===========================================================================
TEST_CASE_START
  TEST_CASE(ut_typical_mutex),
  TEST_CASE(ut_timed_mutex),
  TEST_CASE(ut_priority_mutex),
TEST_CASE_END

