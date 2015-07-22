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

static Thread_t stMsgThread;

#define MSG_STACK_SIZE  (192)
static K_WORD aucMsgStack[MSG_STACK_SIZE];

static MessageQueue_t stMsgQ;
static volatile K_UCHAR ucPassCount = 0;

//===========================================================================
// Local Defines
//===========================================================================

void MsgConsumer(void *unused_)
{
    Message_t *pstMsg;
    K_UCHAR i;

    for (i = 0; i < 20; i++)
    {
        pstMsg = MessageQueue_Receive( &stMsgQ );
        ucPassCount = 0;

        if (pstMsg)
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
                if (0 == Message_GetCode( pstMsg ))
                {
                    ucPassCount++;
                }
                if (0 == Message_GetData( pstMsg ))
                {
                    ucPassCount++;
                }
                break;
            case 1:
                if (1337 == (Message_GetCode( pstMsg )) )
                {
                    ucPassCount++;
                }
                if (7331 == (K_USHORT)(Message_GetData( pstMsg )))
                {
                    ucPassCount++;
                }

            case 2:
                if (0xA0A0== (Message_GetCode( pstMsg )) )
                {
                    ucPassCount++;
                }
                if (0xC0C0 == (K_USHORT)(Message_GetData( pstMsg )))
                {
                    ucPassCount++;
                }

                break;

            default:
                break;
        }
        GlobalMessagePool_Push(pstMsg);
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

    Message_t *pstMsg;

    Thread_Init( &stMsgThread, aucMsgStack, MSG_STACK_SIZE, 7, MsgConsumer, 0);

    MessageQueue_Init( &stMsgQ );

    Thread_Start( &stMsgThread );

    // Get a message from the pool
    pstMsg = GlobalMessagePool_Pop();
    EXPECT_FAIL_FALSE( pstMsg );

    // Send the message to the consumer thread
    Message_SetData( pstMsg, NULL );
    Message_SetCode( pstMsg, 0 );

    MessageQueue_Send( &stMsgQ, pstMsg );

    EXPECT_EQUALS(ucPassCount, 3);

    pstMsg = GlobalMessagePool_Pop();
    EXPECT_FAIL_FALSE( pstMsg );

    // Send the message to the consumer thread
    Message_SetCode( pstMsg, 1337);
    Message_SetData( pstMsg, (void*)7331);

    MessageQueue_Send( &stMsgQ, pstMsg);

    EXPECT_EQUALS(ucPassCount, 3);

    pstMsg = GlobalMessagePool_Pop();
    EXPECT_FAIL_FALSE( pstMsg );

    // Send the message to the consumer thread
    Message_SetCode( pstMsg, 0xA0A0 );
    Message_SetData( pstMsg, (void*)0xC0C0 );

    MessageQueue_Send( &stMsgQ, pstMsg );

    EXPECT_EQUALS(ucPassCount, 3);

    Thread_Exit( &stMsgThread );
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
    Message_t *pstRet;
    ucPassCount = 0;
    pstRet = MessageQueue_TimedReceive( &stMsgQ, 10);
    if (0 == pstRet)
    {
        ucPassCount++;
    }
    else
    {
        GlobalMessagePool_Push(pstRet);
    }

    pstRet = MessageQueue_TimedReceive( &stMsgQ, 1000);
    if (0 != pstRet)
    {
        ucPassCount++;
    }
    else
    {
        GlobalMessagePool_Push(pstRet);
    }

    while(1)
    {
        pstRet = MessageQueue_Receive( &stMsgQ );
        GlobalMessagePool_Push(pstRet);
    }
 }

//===========================================================================
TEST(ut_message_timed_rx)
{
    Message_t *pstMsg;

    pstMsg = GlobalMessagePool_Pop();
    EXPECT_FAIL_FALSE( pstMsg );

    // Send the message to the consumer thread
    Message_SetData( pstMsg, NULL );
    Message_SetCode( pstMsg, 0);

    // Test - Verify that the timed blocking in the message queues works
    Thread_Init( &stMsgThread, aucMsgStack, MSG_STACK_SIZE, 7, MsgTimed, 0);
    Thread_Start( &stMsgThread );

    // Just let the timeout expire
    Thread_Sleep(11);
    EXPECT_EQUALS( ucPassCount, 1 );

    // other thread has a timeout set... Don't leave them waiting!
    MessageQueue_Send( &stMsgQ, pstMsg );

    EXPECT_EQUALS( ucPassCount, 2 );

    MessageQueue_Send( &stMsgQ, pstMsg );

    Thread_Exit( &stMsgThread );
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
