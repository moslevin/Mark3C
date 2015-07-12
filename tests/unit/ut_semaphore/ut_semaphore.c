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

    Semaphore_t clTestSem;
    Semaphore_Init( &clTestSem, 0, 10 );

    int i;
    for (i = 0; i < 10; i++)
    {
        EXPECT_TRUE(Semaphore_Post(&clTestSem));
    }
    EXPECT_FALSE(Semaphore_Post(&clTestSem));
}
TEST_END

//===========================================================================
static Thread_t clThread;
#define SEM_STACK_SIZE     (256)
static K_WORD aucStack[SEM_STACK_SIZE];
static Semaphore_t clSem1;
static Semaphore_t clSem2;
static volatile K_UCHAR ucCounter = 0;

//===========================================================================
void PostPendFunction(void *param_)
{
    Semaphore_t *pclSem = (Semaphore_t*)param_;
    while(1)
    {
        Semaphore_Pend( pclSem );
        ucCounter++;
    }
}

//===========================================================================
TEST(ut_semaphore_post_pend)
{
    // Test - Make sure that pending on a Semaphore_t causes a higher-priority
    // waiting thread to block, and that posting that Semaphore_t from a running
    // lower-priority thread awakens the higher-priority thread

    Semaphore_Init( &clSem1, 0, 1);

    Thread_Init( &clThread, aucStack, SEM_STACK_SIZE, 7, PostPendFunction, (void*)&clSem1);
    Thread_Start( &clThread );
    KernelAware_ProfileInit("seminit");
    int i;
    for (i = 0; i < 10; i++)
    {
        KernelAware_ProfileStart();
        Semaphore_Post( &clSem1 );
        KernelAware_ProfileStop();
    }
    KernelAware_ProfileReport();

    // Verify all 10 posts have been acknowledged by the high-priority thread
    EXPECT_EQUALS(ucCounter, 10);

    // After the test is over, kill the test thread.
    Thread_Exit( &clThread );

    // Test - same as above, but with a counting Semaphore_t instead of a
    // binary Semaphore_t.  Also using a default value.
    Semaphore_Init( &clSem2, 10, 10);

    // Restart the test thread.
    ucCounter = 0;
    Thread_Init( &clThread, aucStack, SEM_STACK_SIZE, 7, PostPendFunction, (void*)&clSem2);
    Thread_Start( &clThread );

    // We'll kill the thread as soon as it blocks.
    Thread_Exit( &clThread );

    // Semaphore_t should have pended 10 times before returning.
    EXPECT_EQUALS(ucCounter, 10);
}
TEST_END

//===========================================================================
void TimeSemFunction(void *param_)
{
    Semaphore_t *pclSem = (Semaphore_t*)param_;

    Thread_Sleep(20);

    Semaphore_Post( pclSem );

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//===========================================================================
TEST(ut_semaphore_timed)
{
    Semaphore_t clTestSem;
    Semaphore_t clTestSem2;

    Semaphore_Init( &clTestSem, 0,1);

    Thread_Init( &clThread, aucStack, SEM_STACK_SIZE, 7, TimeSemFunction, (void*)&clTestSem);
    Thread_Start( &clThread );

    EXPECT_FALSE( Semaphore_TimedPend( &clTestSem, 10 ) );
    Thread_Sleep(20);

    // Pretty nuanced - we can only re-init the Semaphore_t under the knowledge
    // that there's nothing blocking on it already...  don't do this in
    // production
    Semaphore_Init( &clTestSem2, 0,1);

    Thread_Init( &clThread, aucStack, SEM_STACK_SIZE, 7, TimeSemFunction, (void*)&clTestSem2);
    Thread_Start( &clThread );

    EXPECT_TRUE( Semaphore_TimedPend( &clTestSem2, 30) );
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
