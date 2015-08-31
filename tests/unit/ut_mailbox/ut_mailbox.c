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
#include "mark3.h"
#include "memutil.h"
#include "mailbox.h"

//===========================================================================
// Local Defines
//===========================================================================

static Thread_t stMBoxThread;
static K_WORD akMBoxStack[160];

static MailBox_t stMBox;
static K_UCHAR aucMBoxBuffer[128];

static volatile K_UCHAR aucTxBuf[17] = "abcdefghijklmnop"; //allocate a byte of slack for null-termination
static volatile K_UCHAR aucRxBuf[16];
static volatile bool exit_flag;
//===========================================================================
// Define Test Cases Here
//===========================================================================

void mbox_test(void *unused_)
{    
    while(1)
    {
        MailBox_Receive( &stMBox, (void*)aucRxBuf);
    }
}

TEST(mailbox_blocking_receive)
{
    MailBox_Init( &stMBox, (void*)aucMBoxBuffer, 128, 16);
    Thread_Init( &stMBoxThread, akMBoxStack, 160, 7, mbox_test, 0);
    Thread_Start( &stMBoxThread ) ;

    int i, j;
    for (i = 0; i < 100; i++)
    {
        EXPECT_TRUE(MailBox_Send( &stMBox, (void*)aucTxBuf));
        EXPECT_TRUE( MemUtil_CompareMemory((void*)aucRxBuf, (void*)aucTxBuf, 16) );
        for (j = 0; j < 16; j++)
        {
            aucTxBuf[j]++;
        }
    }
    Thread_Exit( &stMBoxThread );
}
TEST_END

volatile K_USHORT usTimeouts = 0;
void mbox_timed_test(void *param)
{
    usTimeouts = 0;
    exit_flag = false;
    while(!exit_flag)
    {
        if (!MailBox_TimedReceive( &stMBox, (void*)aucRxBuf, 10))
        {
            // KernelAware_Trace(0,0, usTimeouts);
            usTimeouts++;
        } else {
            // KernelAware_Trace(0,0, 1337);
            usTimeouts = usTimeouts;

        }
    }
    Thread_Exit( &stMBoxThread );
}

TEST(mailbox_blocking_timed)
{
    usTimeouts = 0;
    MailBox_Init( &stMBox, (void*)aucMBoxBuffer, 128, 16);
    Thread_Init( &stMBoxThread, akMBoxStack, 160, 7, mbox_timed_test, (void*)&usTimeouts);
    Thread_Start( &stMBoxThread );

    int i,j;
    for (j = 0; j < 16; j++)
    {
        aucTxBuf[j] = 'x';
    }

    Thread_Sleep(109);
    EXPECT_EQUALS(usTimeouts, 10);
    // KernelAware_Trace(0,0, usTimeouts);
    for (i = 0; i < 10; i++)
    {
        EXPECT_TRUE(MailBox_Send( &stMBox, (void*)aucTxBuf));
        EXPECT_TRUE( MemUtil_CompareMemory((void*)aucRxBuf, (void*)aucTxBuf, 16) );
        for (j = 0; j < 16; j++)
        {
            aucTxBuf[j]++;
        }
        Thread_Sleep(5);
    }
    exit_flag = true;
    Thread_Sleep(100);
}
TEST_END

TEST(mailbox_send_recv)
{
    MailBox_Init( &stMBox, (void*)aucMBoxBuffer, 128, 16);

    int i,j;
    for (i = 0; i < 8; i++)
    {
        EXPECT_TRUE(MailBox_Send( &stMBox, (void*)aucTxBuf));
        for (j = 0; j < 16; j++)
        {
            aucTxBuf[j]++;
        }
    }
    EXPECT_FALSE(MailBox_Send( &stMBox, (void*)aucTxBuf));

    for (i = 0; i < 8; i++)
    {
        MailBox_Receive( &stMBox, (void*)aucRxBuf);
        for (j = 0; j < 16; j++)
        {
            aucTxBuf[j]--;
        }
        EXPECT_TRUE( MemUtil_CompareMemory((void*)aucRxBuf, (void*)aucTxBuf, 16) );
    }
    EXPECT_FALSE(MailBox_TimedReceive( &stMBox, (void*)aucRxBuf, 10));
}
TEST_END

void mbox_recv_test(void *unused)
{
    exit_flag = false;
    while(!exit_flag)
    {
        MailBox_TimedReceive( &stMBox, (void*)aucRxBuf, 10);
    }
    Thread_Exit( &stMBoxThread );
}

TEST(mailbox_send_blocking)
{
    usTimeouts = 0;
    MailBox_Init( &stMBox, (void*)aucMBoxBuffer, 128, 16);
    Thread_Init( &stMBoxThread, akMBoxStack, 160, 7, mbox_recv_test, (void*)&usTimeouts);

    int i, j;
    for (j = 0; j < 16; j++)
    {
        aucTxBuf[j] = 'x';
    }

    for (i = 0; i < 8; i++)
    {
        MailBox_Send( &stMBox, (void*)aucTxBuf);
    }

    for (i = 0; i < 10; i++)
    {
        EXPECT_FALSE(MailBox_TimedSend( &stMBox, (void*)aucTxBuf, 20));
    }

    Thread_Start( &stMBoxThread ) ;
    for (i = 0; i < 10; i++)
    {
        EXPECT_TRUE(MailBox_TimedSend( &stMBox, (void*)aucTxBuf, 20));
    }

    exit_flag = true;
    Thread_Sleep(100);
}
TEST_END

//===========================================================================
// Test Whitelist Goes Here
//===========================================================================
TEST_CASE_START
  TEST_CASE(mailbox_send_recv),
  TEST_CASE(mailbox_blocking_receive),
  TEST_CASE(mailbox_blocking_timed),
  TEST_CASE(mailbox_send_blocking),
TEST_CASE_END
