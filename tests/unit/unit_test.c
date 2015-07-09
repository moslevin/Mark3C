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
    \file unit_test.cpp
    \brief Unit test class definition
*/

#include "kerneltypes.h"
#include "unit_test.h"

//---------------------------------------------------------------------------
void UnitTest_Init( UnitTest_t *pstTest_ )
{
    pstTest_->m_bIsActive = false;
    pstTest_->m_usIterations = 0;
    pstTest_->m_usPassed = 0;
    pstTest_->m_bComplete = false;
}

//---------------------------------------------------------------------------
void UnitTest_Pass( UnitTest_t *pstTest_ )
{ 
    if (pstTest_->m_bComplete)
    {
        return;
    }
    
    if (pstTest_->m_bIsActive)
    { 
        pstTest_->m_bIsActive = false;
        pstTest_->m_usIterations++;
        pstTest_->m_usPassed++;
        pstTest_->m_bStatus = true;
    }        
}    

//---------------------------------------------------------------------------
void UnitTest_Fail( UnitTest_t *pstTest_ )
{
    if (pstTest_->m_bComplete)
    {
        return;
    }
    
    if (pstTest_->m_bIsActive)
    {
        pstTest_->m_bIsActive = false;
        pstTest_->m_usIterations++;
        pstTest_->m_bStatus = false;
    }
}

//---------------------------------------------------------------------------
void UnitTest_ExpectTrue( UnitTest_t *pstTest_, bool bExpression_ )
    { bExpression_ ? UnitTest_Pass(pstTest_) : UnitTest_Fail(pstTest_); }
//---------------------------------------------------------------------------
void UnitTest_ExpectFalse( UnitTest_t *pstTest_, bool bExpression_ )
    { !bExpression_ ? UnitTest_Pass(pstTest_) : UnitTest_Fail(pstTest_); }
//---------------------------------------------------------------------------
void UnitTest_ExpectEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ == lExpression_) ? UnitTest_Pass(pstTest_) : UnitTest_Fail(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectFailTrue( UnitTest_t *pstTest_, bool bExpression_ )
    { bExpression_ ? UnitTest_Fail(pstTest_) : UnitTest_Pass(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectFailFalse( UnitTest_t *pstTest_, bool bExpression_ )
    { !bExpression_ ? UnitTest_Fail(pstTest_) : UnitTest_Pass(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectFailEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ == lExpression_) ? UnitTest_Fail(pstTest_) : UnitTest_Pass(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectGreaterThan( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ > lExpression_) ? UnitTest_Pass(pstTest_) : UnitTest_Fail(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectLessThan( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ < lExpression_) ? UnitTest_Pass(pstTest_) : UnitTest_Fail(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectGreaterThanEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ >= lExpression_) ? UnitTest_Pass(pstTest_) : UnitTest_Fail(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectLessThanEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ <= lExpression_) ? UnitTest_Pass(pstTest_) : UnitTest_Fail(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectFailGreaterThan( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ > lExpression_) ? UnitTest_Fail(pstTest_) : UnitTest_Pass(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectFailLessThan( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ < lExpression_) ? UnitTest_Fail(pstTest_) : UnitTest_Pass(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectFailGreaterThanEquals(UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ >= lExpression_) ? UnitTest_Fail(pstTest_) : UnitTest_Pass(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_ExpectFailLessThanEquals( UnitTest_t *pstTest_, K_LONG lVal_, K_LONG lExpression_ )
    { (lVal_ <= lExpression_) ? UnitTest_Fail(pstTest_) : UnitTest_Pass(pstTest_); }

//---------------------------------------------------------------------------
void UnitTest_SetName( UnitTest_t *pstTest_, const K_CHAR *szName_ )
{
    pstTest_->m_szName = szName_;
}

//---------------------------------------------------------------------------
void UnitTest_Start( UnitTest_t *pstTest_ )
{
    pstTest_->m_bIsActive = 1;
}

//---------------------------------------------------------------------------
void UnitTest_Complete( UnitTest_t *pstTest_ )
{
    pstTest_->m_bComplete = 1;
}

//---------------------------------------------------------------------------
const K_CHAR *UnitTest_GetName( UnitTest_t *pstTest_ )
{
    return pstTest_->m_szName;
}

//---------------------------------------------------------------------------
bool UnitTest_GetResult(UnitTest_t *pstTest_)
{
    return pstTest_->m_bStatus;
}

//---------------------------------------------------------------------------
K_USHORT UnitTest_GetPassed(UnitTest_t *pstTest_)
{
    return pstTest_->m_usPassed;
}

//---------------------------------------------------------------------------
K_USHORT UnitTest_GetFailed(UnitTest_t *pstTest_)
{
    return pstTest_->m_usIterations - pstTest_->m_usPassed;
}

//---------------------------------------------------------------------------
K_USHORT UnitTest_GetTotal(UnitTest_t *pstTest_)
{
    return pstTest_->m_usIterations;
}
