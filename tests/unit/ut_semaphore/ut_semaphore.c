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
#include "ksemaphore.h"
#include "thread.h"
#include "memutil.h"
#include "driver.h"
#include "kernelaware.h"
//===========================================================================
// Local Defines
//===========================================================================

//===========================================================================
// Define Test Cases Here
//===========================================================================
TEST(ut_semaphore_count)
{
    // Test - verify that we can only increment a counting Semaphore_t to the
    // maximum count.

    // Verify that a counting Semaphore_t with an initial count of zero and a
    // maximum count of 10 can only be posted 10 times before saturaiton and
    // failure.

    Semaphore_t stTestSem;
    Semaphore_Init( &stTestSem, 0, 10 );

    int i;
    for (i = 0; i < 10; i++)
    {
        EXPECT_TRUE(Semaphore_Post(&stTestSem));
    }
    EXPECT_FALSE(Semaphore_Post(&stTestSem));
}
TEST_END

//===========================================================================
static Thread_t stThread;
#define SEM_STACK_SIZE     (256)
static K_WORD aucStack[SEM_STACK_SIZE];
static Semaphore_t stSem1;
static Semaphore_t stSem2;
static volatile K_UCHAR ucCounter = 0;

//===========================================================================
void PostPendFunction(void *para)
{
    Semaphore_t *pstSem = (Semaphore_t*)para;
    while(1)
    {
        Semaphore_Pend( pstSem );
        ucCounter++;
    }
}

//===========================================================================
TEST(ut_semaphore_post_pend)
{
    // Test - Make sure that pending on a Semaphore_t causes a higher-priority
    // waiting thread to block, and that posting that Semaphore_t from a running
    // lower-priority thread awakens the higher-priority thread

    Semaphore_Init( &stSem1, 0, 1);

    Thread_Init( &stThread, aucStack, SEM_STACK_SIZE, 7, PostPendFunction, (void*)&stSem1);
    Thread_Start( &stThread );
    KernelAware_ProfileInit("seminit");
    int i;
    for (i = 0; i < 10; i++)
    {
        KernelAware_ProfileStart();
        Semaphore_Post( &stSem1 );
        KernelAware_ProfileStop();
    }
    KernelAware_ProfileReport();

    // Verify all 10 posts have been acknowledged by the high-priority thread
    EXPECT_EQUALS(ucCounter, 10);

    // After the test is over, kill the test thread.
    Thread_Exit( &stThread );

    // Test - same as above, but with a counting Semaphore_t instead of a
    // binary Semaphore_t.  Also using a default value.
    Semaphore_Init( &stSem2, 10, 10);

    // Restart the test thread.
    ucCounter = 0;
    Thread_Init( &stThread, aucStack, SEM_STACK_SIZE, 7, PostPendFunction, (void*)&stSem2);
    Thread_Start( &stThread );

    // We'll kill the thread as soon as it blocks.
    Thread_Exit( &stThread );

    // Semaphore_t should have pended 10 times before returning.
    EXPECT_EQUALS(ucCounter, 10);
}
TEST_END

//===========================================================================
void TimeSemFunction(void *para)
{
    Semaphore_t *pstSem = (Semaphore_t*)para;

    Thread_Sleep(20);

    Semaphore_Post( pstSem );

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//===========================================================================
TEST(ut_semaphore_timed)
{
    Semaphore_t stTestSem;
    Semaphore_t stTestSem2;

    Semaphore_Init( &stTestSem, 0,1);

    Thread_Init( &stThread, aucStack, SEM_STACK_SIZE, 7, TimeSemFunction, (void*)&stTestSem);
    Thread_Start( &stThread );

    EXPECT_FALSE( Semaphore_TimedPend( &stTestSem, 10 ) );
    Thread_Sleep(20);

    // Pretty nuanced - we can only re-init the Semaphore_t under the knowledge
    // that there's nothing blocking on it already...  don't do this in
    // production
    Semaphore_Init( &stTestSem2, 0,1);

    Thread_Init( &stThread, aucStack, SEM_STACK_SIZE, 7, TimeSemFunction, (void*)&stTestSem2);
    Thread_Start( &stThread );

    EXPECT_TRUE( Semaphore_TimedPend( &stTestSem2, 30) );
}

TEST_END

//===========================================================================
// Test Whitelist Goes Here
//===========================================================================
TEST_CASE_START
  TEST_CASE(ut_semaphore_count),
  TEST_CASE(ut_semaphore_post_pend),
  TEST_CASE(ut_semaphore_timed),
TEST_CASE_END
