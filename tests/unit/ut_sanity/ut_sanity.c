#include "kerneltypes.h"
#include "mark3cfg.h"
#include "kernel.h"
#include "thread.h"
#include "driver.h"
#include "drvUART.h"
#include "profile.h"
#include "kernelprofile.h"
#include "ksemaphore.h"
#include "mutex.h"
#include "message.h"
#include "timerlist.h"
#include "../unit_test.h"
#include "../ut_platform.h"
#include "kernelaware.h"

//---------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

//---------------------------------------------------------------------------
static volatile K_UCHAR ucTestVal;

//---------------------------------------------------------------------------
static Mutex_t clMutex;

//---------------------------------------------------------------------------
#define TEST_STACK1_SIZE            (240)
#define TEST_STACK2_SIZE            (240)
#define TEST_STACK3_SIZE            (240)

static MessageQueue_t clMsgQ1;
static MessageQueue_t clMsgQ2;

static Thread_t clTestThread2;
static Thread_t clTestThread3;
static Thread_t clTestThread1;

static K_WORD aucTestStack1[TEST_STACK1_SIZE];
static K_WORD aucTestStack2[TEST_STACK2_SIZE];
static K_WORD aucTestStack3[TEST_STACK3_SIZE];

//---------------------------------------------------------------------------
void TestSemThread(Semaphore_t *pstSem_)
{
    Semaphore_Pend( pstSem_ );
    if (ucTestVal != 0x12)
    {
        ucTestVal = 0xFF;
    }
    else
    {
        ucTestVal = 0x21;
    }

    Semaphore_Pend( pstSem_ );
    if (ucTestVal != 0x32)
    {
        ucTestVal = 0xFF;
    }
    else
    {
        ucTestVal = 0x23;
    }

    Semaphore_Pend( pstSem_ );
    if (ucTestVal != 0x45)
    {
        ucTestVal = 0xFF;
    }
    else
    {
        ucTestVal = 0x54;
    }
    Semaphore_Pend( pstSem_ );
}
//---------------------------------------------------------------------------
// Binary semaphore test:
//  Create a worker thread at a higher priority, which pends on a semaphore
//  that we hold.  The main thread and the new thread alternate pending/posting
//  the semaphore, while modifying/verifying a global variable.
//---------------------------------------------------------------------------
//void UT_SemaphoreTest(void)
TEST(ut_sanity_sem)
{
    Semaphore_t clSemaphore;

    Semaphore_Init( &clSemaphore, 0, 1);

    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 1, (ThreadEntry_t)TestSemThread, (void*)&clSemaphore);
    Thread_Start( &clTestThread1 );

    ucTestVal = 0x12;
    Semaphore_Post( &clSemaphore );

    EXPECT_EQUALS(ucTestVal, 0x21);

    ucTestVal = 0x32;
    Semaphore_Post( &clSemaphore );

    EXPECT_EQUALS(ucTestVal, 0x23);

    ucTestVal = 0x45;    
    Semaphore_Post( &clSemaphore );

    EXPECT_EQUALS(ucTestVal, 0x54);

    Thread_Stop( &clTestThread1 );
    Thread_Exit( &clTestThread1 );

    Semaphore_Init( &clSemaphore, 0, 1 );

}
TEST_END

//---------------------------------------------------------------------------
void TimedSemaphoreThread_Short( Semaphore_t *pstSem_ )
{
    Thread_Sleep(10);
    Semaphore_Post( pstSem_ );

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
void TimedSemaphoreThread_Long( Semaphore_t *pstSem_ )
{
    Thread_Sleep(20);
    Semaphore_Post( pstSem_ );
    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
//void UT_TimedSemaphoreTest(void)
TEST(ut_sanity_timed_sem)
{
    Semaphore_t clSem;

    Thread_SetPriority( Scheduler_GetCurrentThread(), 3);

    Semaphore_Init( &clSem, 0,1);
    
    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TimedSemaphoreThread_Short, (void*)&clSem );
    Thread_Start( &clTestThread1 );
    
// Test 1 - block on a semaphore, wait on thread that will post before expiry

    EXPECT_TRUE( Semaphore_TimedPend( &clSem, 15 ) );
    
// Test 2 - block on a semaphore, wait on thread that will post after expiry
    Semaphore_Init( &clSem, 0, 1 );
    
    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TimedSemaphoreThread_Long, (void*)&clSem );
    Thread_Start( &clTestThread1 );
        
    EXPECT_FALSE( Semaphore_TimedPend( &clSem, 15 ) );

    Thread_SetPriority( Scheduler_GetCurrentThread(), 1);
}
TEST_END

//---------------------------------------------------------------------------

void TestSleepThread(void *pvArg_)
{
    while(1)
    {
        ucTestVal = 0xAA;
    }
}

//---------------------------------------------------------------------------
// Sleep Test
//  Verify that thread sleeping works as expected.  Check that the lower
//  priority thread is able to execute, setting the global variable to a
//  target value.
//---------------------------------------------------------------------------
//void UT_SleepTest(void)
TEST(ut_sanity_sleep)
{
    Thread_SetPriority( Scheduler_GetCurrentThread(), 3 );

    ucTestVal = 0x00;
    
    // Create a lower-priority thread that sets the test value to a known
    // cookie.
    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TestSleepThread, NULL);
    Thread_Start( &clTestThread1 );
    
    // Sleep, when we wake up check the test value
    Thread_Sleep(5);

    EXPECT_EQUALS(ucTestVal, 0xAA);

    Thread_Exit( &clTestThread1 );

    Thread_SetPriority( Scheduler_GetCurrentThread(), 1 );
    
}
TEST_END

//---------------------------------------------------------------------------
void TestMutexThread(Mutex_t *pclMutex_)
{
    Mutex_Claim( pclMutex_ );
    if (ucTestVal != 0xDC)
    {
        ucTestVal = 0xAA;
    }
    else
    {
        ucTestVal = 0xAC;
    }
    Mutex_Release( pclMutex_ );

    Thread_Exit( Scheduler_GetCurrentThread() );
}


//---------------------------------------------------------------------------
void TestTimedMutexThreadShort(Mutex_t *pclMutex_)
{
    Mutex_Claim( pclMutex_ );
    Thread_Sleep( 10 );
    Mutex_Release( pclMutex_ );

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
void TestTimedMutexThreadLong(Mutex_t *pclMutex_)
{
    Mutex_Claim( pclMutex_ );
    Thread_Sleep( 20 );
    Mutex_Release( pclMutex_ );

    Thread_Exit( Scheduler_GetCurrentThread() );
}


//---------------------------------------------------------------------------
// Mutex test
//  Create a mutex and claim it.  While the mutex is owned, create a new
//  thread at a higher priority, which tries to claim the mutex itself.
//  Use a global variable to verify that the threads do not proceed outside
//  of the control.
//---------------------------------------------------------------------------
//void UT_MutexTest(void)
TEST(ut_sanity_mutex)
{
    Mutex_Init( &clMutex );

    ucTestVal = 0x10;
    Mutex_Claim( &clMutex );

    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TestMutexThread, (void*)&clMutex );
    Thread_Start( &clTestThread1 );
    
    ucTestVal = 0xDC;
    
    Mutex_Release( &clMutex );

    EXPECT_EQUALS(ucTestVal, 0xAC);

    Mutex_Init( &clMutex );
    
    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TestTimedMutexThreadShort, (void*)&clMutex );
    Thread_Start( &clTestThread1 );
    
    EXPECT_TRUE( Mutex_TimedClaim( &clMutex, 15 ) );

    Mutex_Init( &clMutex );

    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TestTimedMutexThreadLong, (void*)&clMutex );
    Thread_Start( &clTestThread1 );
    
    EXPECT_FALSE( Mutex_TimedClaim( &clMutex, 15 ) );
}
TEST_END

//---------------------------------------------------------------------------
void TimedMessageThread(MessageQueue_t *pclMsgQ_)
{
    Message_t *pclMsg = GlobalMessagePool_Pop();
    
    Message_SetData( pclMsg, NULL );
    Message_SetCode( pclMsg, 0 );
    
    Thread_Sleep(10);
    
    MessageQueue_Send( pclMsgQ_, pclMsg );
    
    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
//void UT_TimedMessageTest(void)
TEST(ut_sanity_timed_msg)
{
    Message_t *pclMsg;

    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TimedMessageThread, (void*)&clMsgQ1);
    Thread_Start( &clTestThread1 );
    
    pclMsg = MessageQueue_TimedReceive( &clMsgQ1, 15 );
    
    if (!pclMsg)
    {
        EXPECT_TRUE(0);
    }
    else
    {
        EXPECT_TRUE(1);
        GlobalMessagePool_Push(pclMsg);
    }

    pclMsg = MessageQueue_TimedReceive( &clMsgQ1, 10 );
    
    if (pclMsg)
    {
        EXPECT_TRUE(0);
        GlobalMessagePool_Push(pclMsg);
    }
    else
    {
        EXPECT_TRUE(1);
    }
}
TEST_END

//---------------------------------------------------------------------------
void TestMessageTest(void *pvArg)
{
    Message_t *pclMesg;
    bool bPass = true;

    pclMesg = MessageQueue_Receive( &clMsgQ2 );

    if (Message_GetCode( pclMesg ) != 0x11)
    {
        bPass = false;
    }

    GlobalMessagePool_Push(pclMesg);
    
    pclMesg = GlobalMessagePool_Pop();
    
    Message_SetCode( pclMesg, 0x22 );

    MessageQueue_Send( &clMsgQ1, pclMesg );
    
    pclMesg = MessageQueue_Receive( &clMsgQ2 );
    if(Message_GetCode( pclMesg ) != 0xAA)
    {
        bPass = false;
    }

    GlobalMessagePool_Push(pclMesg);
    
    pclMesg = MessageQueue_Receive( &clMsgQ2 );
    if(Message_GetCode( pclMesg ) != 0xBB)
    {
        bPass = false;
    }

    GlobalMessagePool_Push(pclMesg);

    pclMesg = MessageQueue_Receive( &clMsgQ2 );
    if(Message_GetCode( pclMesg ) != 0xCC)
    {
        bPass = false;
    }

    GlobalMessagePool_Push(pclMesg);

    pclMesg = GlobalMessagePool_Pop();
    if (bPass)
    {
        Message_SetCode( pclMesg, 0xDD );
    }
    else
    {
        Message_SetCode( pclMesg, 0xFF );
    }
    MessageQueue_Send( &clMsgQ1, pclMesg );

    pclMesg = GlobalMessagePool_Pop();
    if (bPass)
    {
        Message_SetCode( pclMesg, 0xEE );
    }
    else
    {
        Message_SetCode( pclMesg, 0x00 );
    }
    MessageQueue_Send( &clMsgQ1, pclMesg );

    pclMesg = GlobalMessagePool_Pop();
    if (bPass)
    {
        Message_SetCode( pclMesg, 0xFF);
    }
    else
    {
        Message_SetCode( pclMesg, 0x11);
    }
    MessageQueue_Send( &clMsgQ1, pclMesg );

    Thread_Exit( Scheduler_GetCurrentThread() );
}

//---------------------------------------------------------------------------
// Message test
//  Create a second thread that we communicate with by passing messages.
//  Ensure that messages being passed between threads are received as expected
//  in the correct FIFO order, and have the correct IDs.
//---------------------------------------------------------------------------
//void UT_MessageTest(void)
TEST(ut_sanity_msg)
{
    MessageQueue_Init( &clMsgQ1 );
    MessageQueue_Init( &clMsgQ2 );

    Message_t *pclMsg;

    pclMsg = GlobalMessagePool_Pop();

    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TestMessageTest, NULL);
    Thread_Start( &clTestThread1 );
    Thread_Yield();
    
    Message_SetCode( pclMsg, 0x11 );
    Message_SetData( pclMsg, NULL );

    MessageQueue_Send( &clMsgQ2, pclMsg );

    pclMsg = MessageQueue_Receive( &clMsgQ1 );
    
    EXPECT_EQUALS(Message_GetCode( pclMsg ), 0x22);

    GlobalMessagePool_Push(pclMsg);

    pclMsg = GlobalMessagePool_Pop();
    Message_SetCode( pclMsg, 0xAA);
    MessageQueue_Send( &clMsgQ2, pclMsg );

    pclMsg = GlobalMessagePool_Pop();
    Message_SetCode( pclMsg, 0xBB);
    MessageQueue_Send( &clMsgQ2, pclMsg );

    pclMsg = GlobalMessagePool_Pop();
    Message_SetCode( pclMsg, 0xCC);
    MessageQueue_Send( &clMsgQ2, pclMsg );

    pclMsg = MessageQueue_Receive( &clMsgQ1 );
    EXPECT_EQUALS(Message_GetCode( pclMsg ), 0xDD);

    GlobalMessagePool_Push(pclMsg);

    pclMsg = MessageQueue_Receive( &clMsgQ1 );
    EXPECT_EQUALS(Message_GetCode( pclMsg ), 0xEE);

    GlobalMessagePool_Push(pclMsg);
    
    pclMsg = MessageQueue_Receive( &clMsgQ1 );
    EXPECT_EQUALS(Message_GetCode( pclMsg ), 0xFF);

    GlobalMessagePool_Push(pclMsg);
}
TEST_END

//---------------------------------------------------------------------------
void TestRRThread(volatile K_ULONG *pulCounter_)
{
    while (1)
    {
        (*pulCounter_)++;
    }
}

//---------------------------------------------------------------------------
// Round-Robin Thread
//  Create 3 threads in the same priority group (lower than our thread), and
//  set their quantums to different values.  Verify that the ratios of their
//  "work cycles" are close to equivalent.
//---------------------------------------------------------------------------

//void UT_RoundRobinTest(void)
TEST(ut_sanity_rr)
{
    volatile K_ULONG ulCounter1 = 0;
    volatile K_ULONG ulCounter2 = 0;
    volatile K_ULONG ulCounter3 = 0;
    K_ULONG ulDelta;

    Thread_SetPriority( Scheduler_GetCurrentThread(), 3);

    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TestRRThread, (void*)&ulCounter1 );
    Thread_Init( &clTestThread2, aucTestStack2, TEST_STACK2_SIZE, 2, (ThreadEntry_t)TestRRThread, (void*)&ulCounter2 );
    Thread_Init( &clTestThread3, aucTestStack3, TEST_STACK3_SIZE, 2, (ThreadEntry_t)TestRRThread, (void*)&ulCounter3 );
    
    Thread_Start( &clTestThread1 );
    Thread_Start( &clTestThread2 );
    Thread_Start( &clTestThread3 );

    // Sleep for a while to let the other threads execute
    Thread_Sleep(120);  // Must be modal to the worker thread quantums

    if (ulCounter1 > ulCounter2)
    {
        ulDelta = ulCounter1 - ulCounter2;
    }
    else
    {
        ulDelta = ulCounter2 - ulCounter1;
    }

    // Give or take...
    EXPECT_FALSE(ulDelta > ulCounter1/2);

    if (ulCounter1 > ulCounter3)
    {
        ulDelta = ulCounter1 - ulCounter3;
    }
    else
    {
        ulDelta = ulCounter3 - ulCounter1;
    }

    // Give or take...
    EXPECT_FALSE(ulDelta > ulCounter1/2);

    Thread_Exit( &clTestThread1 );
    Thread_Exit( &clTestThread2 );
    Thread_Exit( &clTestThread3 );

    Thread_SetPriority( Scheduler_GetCurrentThread(), 1 );
}
TEST_END

//---------------------------------------------------------------------------
//void UT_QuantumTest(void)
TEST(ut_sanity_quantum)
{
    volatile K_ULONG ulCounter1 = 0;
    volatile K_ULONG ulCounter2 = 0;
    volatile K_ULONG ulCounter3 = 0;
    K_ULONG ulDelta;

    Thread_SetPriority( Scheduler_GetCurrentThread(), 3);

    Thread_Init( &clTestThread1, aucTestStack1, TEST_STACK1_SIZE, 2, (ThreadEntry_t)TestRRThread, (void*)&ulCounter1 );
    Thread_Init( &clTestThread2, aucTestStack2, TEST_STACK2_SIZE, 2, (ThreadEntry_t)TestRRThread, (void*)&ulCounter2 );
    Thread_Init( &clTestThread3, aucTestStack3, TEST_STACK3_SIZE, 2, (ThreadEntry_t)TestRRThread, (void*)&ulCounter3 );
    
    Thread_SetQuantum( &clTestThread1, 10 );
    Thread_SetQuantum( &clTestThread2, 20);
    Thread_SetQuantum( &clTestThread3, 30);
    
    Thread_Start( &clTestThread1 );
    Thread_Start( &clTestThread2 );
    Thread_Start( &clTestThread3 );
    
    // Sleep for a while to let the other threads execute
    Thread_Sleep(180);  // Must be modal to the worker thread quantums

    // Kill the worker threads
    ulCounter2 /= 2;
    ulCounter3 /= 3;
    
    if (ulCounter1 > ulCounter2)
    {
        ulDelta = ulCounter1 - ulCounter2;
    }
    else
    {
        ulDelta = ulCounter2 - ulCounter1;
    }

    // Give or take...
    EXPECT_FALSE(ulDelta > ulCounter1/2);

    if (ulCounter1 > ulCounter3)
    {
        ulDelta = ulCounter1 - ulCounter3;
    }
    else
    {
        ulDelta = ulCounter3 - ulCounter1;
    }

    // Give or take...
    EXPECT_FALSE(ulDelta > ulCounter1/2);

    Thread_Exit( &clTestThread1 );
    Thread_Exit( &clTestThread2 );
    Thread_Exit( &clTestThread3 );

    Thread_SetPriority( Scheduler_GetCurrentThread(), 1 );
}
TEST_END

void TimerTestCallback(Thread_t *pclOwner_, void *pvData_)
{
    ucTestVal++;
}

//void UT_TimerTest(void)
TEST(ut_sanity_timer)
{
    Timer_t clTimer;

    ucTestVal = 0;

    Timer_Init( &clTimer );
    Timer_Start( &clTimer, 1, 2, TimerTestCallback, NULL );

    Thread_Sleep(3);
    EXPECT_EQUALS(ucTestVal, 1);

    ucTestVal = 0;
    Timer_Stop( &clTimer );

    Timer_Start( &clTimer, 1, 1, TimerTestCallback, NULL);
    
    Thread_Sleep(10);
    
    EXPECT_GTE(ucTestVal, 9);

    Timer_Stop( &clTimer );
}
TEST_END

//===========================================================================
// Test Whitelist Goes Here
//===========================================================================
TEST_CASE_START
  TEST_CASE(ut_sanity_sem),
  TEST_CASE(ut_sanity_timed_sem),
  TEST_CASE(ut_sanity_sleep),
  TEST_CASE(ut_sanity_mutex),
  TEST_CASE(ut_sanity_msg),
  TEST_CASE(ut_sanity_timed_msg),
  TEST_CASE(ut_sanity_rr),
  TEST_CASE(ut_sanity_quantum),
  TEST_CASE(ut_sanity_timer),
TEST_CASE_END