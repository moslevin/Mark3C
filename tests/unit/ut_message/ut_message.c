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
#include "message.h"
#include "thread.h"

static Thread_t clMsgThread;

#define MSG_STACK_SIZE  (192)
static K_WORD aucMsgStack[MSG_STACK_SIZE];

static MessageQueue_t clMsgQ;
static volatile K_UCHAR ucPassCount = 0;

//===========================================================================
// Local Defines
//===========================================================================

void MsgConsumer(void *unused_)
{
    Message_t *pclMsg;
    K_UCHAR i;

    for (i = 0; i < 20; i++)
    {
        pclMsg = MessageQueue_Receive( &clMsgQ );
        ucPassCount = 0;

        if (pclMsg)
        {
            ucPassCount++;
        }
        else
        {
            ucPassCount = 0;
            continue;
        }

        switch(i)
        {
            case 0:
                if (0 == Message_GetCode( pclMsg ))
                {
                    ucPassCount++;
                }
                if (0 == Message_GetData( pclMsg ))
                {
                    ucPassCount++;
                }
                break;
            case 1:
                if (1337 == (Message_GetCode( pclMsg )) )
                {
                    ucPassCount++;
                }
                if (7331 == (K_USHORT)(Message_GetData( pclMsg )))
                {
                    ucPassCount++;
                }

            case 2:
                if (0xA0A0== (Message_GetCode( pclMsg )) )
                {
                    ucPassCount++;
                }
                if (0xC0C0 == (K_USHORT)(Message_GetData( pclMsg )))
                {
                    ucPassCount++;
                }

                break;

            default:
                break;
        }
        GlobalMessagePool_Push(pclMsg);
    }
}

//===========================================================================
// Define Test Cases Here
//===========================================================================
TEST(ut_message_tx_rx)
{
    // Test - verify that we can use a message queue object to send data
    // from one thread to another, and that the receiver can block on the
    // message queue.  This test also relies on priority scheduling working
    // as expected.

    Message_t *pclMsg;

    Thread_Init( &clMsgThread, aucMsgStack, MSG_STACK_SIZE, 7, MsgConsumer, 0);

    MessageQueue_Init( &clMsgQ );

    Thread_Start( &clMsgThread );

    // Get a message from the pool
    pclMsg = GlobalMessagePool_Pop();
    EXPECT_FAIL_FALSE( pclMsg );

    // Send the message to the consumer thread
    Message_SetData( pclMsg, NULL );
    Message_SetCode( pclMsg, 0 );

    MessageQueue_Send( &clMsgQ, pclMsg );

    EXPECT_EQUALS(ucPassCount, 3);

    pclMsg = GlobalMessagePool_Pop();
    EXPECT_FAIL_FALSE( pclMsg );

    // Send the message to the consumer thread
    Message_SetCode( pclMsg, 1337);
    Message_SetData( pclMsg, (void*)7331);

    MessageQueue_Send( &clMsgQ, pclMsg);

    EXPECT_EQUALS(ucPassCount, 3);

    pclMsg = GlobalMessagePool_Pop();
    EXPECT_FAIL_FALSE( pclMsg );

    // Send the message to the consumer thread
    Message_SetCode( pclMsg, 0xA0A0 );
    Message_SetData( pclMsg, (void*)0xC0C0 );

    MessageQueue_Send( &clMsgQ, pclMsg );

    EXPECT_EQUALS(ucPassCount, 3);

    Thread_Exit( &clMsgThread );
}
TEST_END

//===========================================================================
TEST(ut_message_exhaust)
{
    // Test - exhaust the global message pool and ensure that we eventually
    // get "NULL" returned when the pool is depleted, and not some other
    // unexpected condition/system failure.
    int i;
    for ( i = 0; i < GLOBAL_MESSAGE_POOL_SIZE; i++)
    {
        EXPECT_FAIL_FALSE( GlobalMessagePool_Pop() );
    }
    EXPECT_FALSE( GlobalMessagePool_Pop());

    // Test is over - re-init the pool..
    GlobalMessagePool_Init();
}
TEST_END

static volatile K_BOOL bTimedOut = false;
//===========================================================================
void MsgTimed(void *unused)
{
    Message_t *pclRet;
    ucPassCount = 0;
    pclRet = MessageQueue_TimedReceive( &clMsgQ, 10);
    if (0 == pclRet)
    {
        ucPassCount++;
    }
    else
    {
        GlobalMessagePool_Push(pclRet);
    }

    pclRet = MessageQueue_TimedReceive( &clMsgQ, 1000);
    if (0 != pclRet)
    {
        ucPassCount++;
    }
    else
    {
        GlobalMessagePool_Push(pclRet);
    }

    while(1)
    {
        pclRet = MessageQueue_Receive( &clMsgQ );
        GlobalMessagePool_Push(pclRet);
    }
 }

//===========================================================================
TEST(ut_message_timed_rx)
{
    Message_t *pclMsg;

    pclMsg = GlobalMessagePool_Pop();
    EXPECT_FAIL_FALSE( pclMsg );

    // Send the message to the consumer thread
    Message_SetData( pclMsg, NULL );
    Message_SetCode( pclMsg, 0);

    // Test - Verify that the timed blocking in the message queues works
    Thread_Init( &clMsgThread, aucMsgStack, MSG_STACK_SIZE, 7, MsgTimed, 0);
    Thread_Start( &clMsgThread );

    // Just let the timeout expire
    Thread_Sleep(11);
    EXPECT_EQUALS( ucPassCount, 1 );

    // other thread has a timeout set... Don't leave them waiting!
    MessageQueue_Send( &clMsgQ, pclMsg );

    EXPECT_EQUALS( ucPassCount, 2 );

    MessageQueue_Send( &clMsgQ, pclMsg );

    Thread_Exit( &clMsgThread );
}
TEST_END

//===========================================================================
// Test Whitelist Goes Here
//===========================================================================
TEST_CASE_START
  TEST_CASE(ut_message_tx_rx),
  TEST_CASE(ut_message_exhaust),
  TEST_CASE(ut_message_timed_rx),
TEST_CASE_END
