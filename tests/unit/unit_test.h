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
/*!
    \file unit_test.h
    \brief Unit test object declarations
*/
#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__


#include "kerneltypes.h"

typedef struct
{
    const K_CHAR *szName;    //!< Name of the tests performed
    bool bIsActive;          //!< Whether or not the test is active
    K_UCHAR bComplete;       //!< Whether or not the test is complete
    bool bStatus;            //!< Status of the last-run test
    K_USHORT usIterations;   //!< Number of iterations executed
    K_USHORT usPassed;       //!< Number of iterations that have passed
} UnitTest_t;

//---------------------------------------------------------------------------
void UnitTest_Init( UnitTest_t *pstTest_ );
//---------------------------------------------------------------------------
/*!
    \fn void SetName( const K_CHAR *szName_ )

    Set the name of the test object

    \param szName_ Name of the tests associated with this object

*/
void UnitTest_SetName( UnitTest_t *pstTest_, const K_CHAR *szName_ );
//---------------------------------------------------------------------------
/*!
    \fn void Start()

    Start a new test iteration.
*/
void UnitTest_Start( UnitTest_t *pstTest_ );
//---------------------------------------------------------------------------
/*!
    \fn void Pass()

    Stop the current iteration (if started), and register that the test
    was successful.
*/
void UnitTest_Pass( UnitTest_t *pstTest_ );

//---------------------------------------------------------------------------
/*!
    \fn void Fail();

    Stop the current iterations (if started), and register that the
    current test failed.
*/
void UnitTest_Fail( UnitTest_t *pstTest_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectTrue( UnitTest_t *pstTest_, bool bExpression_ );
//---------------------------------------------------------------------------
void UnitTest_ExpectFalse( UnitTest_t *pstTest_, bool bExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectFailTrue( UnitTest_t *pstTest_, bool bExpression_ );
//---------------------------------------------------------------------------
void UnitTest_ExpectFailFalse( UnitTest_t *pstTest_, bool bExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectFailEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectGreaterThan( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectLessThan( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectGreaterThanEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectLessThanEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectFailGreaterThan( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectFailLessThan( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectFailGreaterThanEquals(UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
void UnitTest_ExpectFailLessThanEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ );

//---------------------------------------------------------------------------
/*!
    \fn void Complete()

    Complete the test.  Once a test has been completed, no new iterations
    can be started (i.e Start()/Pass()/Fail() will have no effect).
*/
void UnitTest_Complete( UnitTest_t *pstTest_ );

//---------------------------------------------------------------------------
/*!
    \fn const K_CHAR *GetName()

    Get the name of the tests associated with this object

    \return Name of the test
*/
const K_CHAR *UnitTest_GetName(UnitTest_t *pstTest_);

//---------------------------------------------------------------------------
/*!
    \fn bool GetResult()

    Return the result of the last test

    \return Status of the last run test (false = fail, true = pass)
*/
bool UnitTest_GetResult(UnitTest_t *pstTest_);

//---------------------------------------------------------------------------
/*!
    \fn K_USHORT GetPassed()

    Return the total number of test points/iterations passed

    \return Count of all successful test points/iterations
*/
K_USHORT UnitTest_GetPassed(UnitTest_t *pstTest_);

//---------------------------------------------------------------------------
/*!
    \fn K_USHORT GetFailed()

    Return the number of failed test points/iterations

    \return Failed test point/iteration count
*/
K_USHORT UnitTest_GetFailed(UnitTest_t *pstTest_);

//---------------------------------------------------------------------------
/*!
    \fn K_USHORT GetTotal()

    Return the total number of iterations/test-points executed

    \return Total number of ierations/test-points executed
*/
K_USHORT UnitTest_GetTotal(UnitTest_t *pstTest_);


#endif
