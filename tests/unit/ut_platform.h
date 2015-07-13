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

#ifndef __UT_PLATFORM_H__
#define __UT_PLATFORM_H__

#include "mark3.h"
#include "kerneltypes.h"
#include "kernel.h"
#include "unit_test.h"

//---------------------------------------------------------------------------
#if KERNEL_USE_IDLE_FUNC
#define STACK_SIZE_APP      (300)	//!< Size of the main app's stack
#else
#define STACK_SIZE_APP      (192)
#endif

#define STACK_SIZE_IDLE		(192)	//!< Size of the idle thread stack

//---------------------------------------------------------------------------
#define UART_SIZE_RX		(12)	//!< UART RX Buffer size
#define UART_SIZE_TX		(12)	//!< UART TX Buffer size

//---------------------------------------------------------------------------
typedef void (*TestFunc)(void);

//---------------------------------------------------------------------------
typedef struct
{
    const K_CHAR *szName;
    UnitTest_t *pstTestCase;
    TestFunc pfTestFunc;
} MyTestCase;

//---------------------------------------------------------------------------
#define TEST(x)        \
static UnitTest_t TestObj_##x;   \
static void TestFunc_##x(void) \
{ \
    UnitTest_t *pstCurrent = &TestObj_##x;

#define TEST_END \
    UnitTest_Complete( pstCurrent ); \
    MyUnitTest_PrintTestResult( pstCurrent ); \
}

//---------------------------------------------------------------------------
#define EXPECT_TRUE(x)      UnitTest_Start( pstCurrent); UnitTest_ExpectTrue(pstCurrent, x)
#define EXPECT_FALSE(x)     UnitTest_Start( pstCurrent); UnitTest_ExpectFalse(pstCurrent, x)
#define EXPECT_EQUALS(x,y)  UnitTest_Start( pstCurrent); UnitTest_ExpectEquals(pstCurrent, (K_LONG)(x), (K_LONG)(y))
#define EXPECT_GT(x,y)      UnitTest_Start( pstCurrent); UnitTest_ExpectGreaterThan(pstCurrent, (K_LONG)(x), (K_LONG)(y))
#define EXPECT_LT(x,y)      UnitTest_Start( pstCurrent); UnitTest_ExpectLessThan(pstCurrent, (K_LONG)(x), (K_LONG)(y))
#define EXPECT_GTE(x,y)     UnitTest_Start( pstCurrent); UnitTest_ExpectGreaterThanEquals(pstCurrent, (K_LONG)(x), (K_LONG)(y))
#define EXPECT_LTE(x,y)     UnitTest_Start( pstCurrent); UnitTest_ExpectLessThanEquals(pstCurrent, (K_LONG)(x), (K_LONG)(y))

//---------------------------------------------------------------------------
#define EXPECT_FAIL_TRUE(x)      UnitTest_Start( pstCurrent); UnitTest_ExpectFailTrue(pstCurrent, x)
#define EXPECT_FAIL_FALSE(x)     UnitTest_Start( pstCurrent); UnitTest_ExpectFailFalse(pstCurrent, x)
#define EXPECT_FAIL_EQUALS(x,y)  UnitTest_Start( pstCurrent); UnitTest_ExpectFailEquals(pstCurrent, (K_LONG)(x), (K_LONG)(y))
#define EXPECT_FAIL_GT(x,y)      UnitTest_Start( pstCurrent); UnitTest_ExpectFailGreaterThan(pstCurrent, (K_LONG)(x), (K_LONG)(y))
#define EXPECT_FAIL_LT(x,y)      UnitTest_Start( pstCurrent); UnitTest_ExpectFailLessThan(pstCurrent, (K_LONG)(x), (K_LONG)(y))
#define EXPECT_FAIL_GTE(x,y)     UnitTest_Start( pstCurrent); UnitTest_ExpectFailGreaterThanEquals(pstCurrent, (K_LONG)(x), (K_LONG)(y))
#define EXPECT_FAIL_LTE(x,y)     UnitTest_Start( pstCurrent); UnitTest_ExpectFailLessThanEquals(pstCurrent, (K_LONG)(x), (K_LONG)(y))

//---------------------------------------------------------------------------
#define TEST_NAME_EVALUATE(name)       #name
#define TEST_NAME_STRINGIZE(name)      TEST_NAME_EVALUATE(name)

//---------------------------------------------------------------------------
#define TEST_CASE_START MyTestCase astTestCases[] = {
#define TEST_CASE(x)    { TEST_NAME_STRINGIZE(x), &TestObj_##x, TestFunc_##x }
#define TEST_CASE_END   { 0, 0, 0 } };

//---------------------------------------------------------------------------
extern MyTestCase astTestCases[];
extern void run_tests();

//---------------------------------------------------------------------------
void PrintString(const K_CHAR *szStr_);
//---------------------------------------------------------------------------
void MyUnitTest_PrintTestResult( UnitTest_t *pstTest_ );


#endif //__UT_PLATFORM_H__

