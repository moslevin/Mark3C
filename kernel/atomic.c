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

    \file   atomic.cpp

    \brief  Basic Atomic Operations
*/

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "atomic.h"
#include "threadport.h"

#if KERNEL_USE_ATOMIC

//---------------------------------------------------------------------------
K_UCHAR Atomic_Set8( K_UCHAR *pucSource_, K_UCHAR ucVal_ )
{
    K_UCHAR ucRet;
    CS_ENTER();
    ucRet = *pucSource_;
    *pucSource_ = ucVal_;
    CS_EXIT();
    return ucRet;
}
//---------------------------------------------------------------------------
K_USHORT Atomic_Set16( K_USHORT *pusSource_, K_USHORT usVal_ )
{
    K_USHORT usRet;
    CS_ENTER();
    usRet = *pusSource_;
    *pusSource_ = usVal_;
    CS_EXIT();
    return usRet;
}
//---------------------------------------------------------------------------
K_ULONG Atomic_Set32( K_ULONG *pulSource_, K_ULONG ulVal_ )
{
    K_ULONG ulRet;
    CS_ENTER();
    ulRet = *pulSource_;
    *pulSource_ = ulVal_;
    CS_EXIT();
    return ulRet;
}

//---------------------------------------------------------------------------
K_UCHAR Atomic_Add8( K_UCHAR *pucSource_, K_UCHAR ucVal_ )
{
    K_UCHAR ucRet;
    CS_ENTER();
    ucRet = *pucSource_;
    *pucSource_ += ucVal_;
    CS_EXIT();
    return ucRet;
}

//---------------------------------------------------------------------------
K_USHORT Atomic_Add16( K_USHORT *pusSource_, K_USHORT usVal_ )
{
    K_USHORT usRet;
    CS_ENTER();
    usRet = *pusSource_;
    *pusSource_ += usVal_;
    CS_EXIT();
    return usRet;
}

//---------------------------------------------------------------------------
K_ULONG Atomic_Add32( K_ULONG *pulSource_, K_ULONG ulVal_ )
{
    K_ULONG ulRet;
    CS_ENTER();
    ulRet = *pulSource_;
    *pulSource_ += ulVal_;
    CS_EXIT();
    return ulRet;
}

//---------------------------------------------------------------------------
K_UCHAR Atomic_Sub8( K_UCHAR *pucSource_, K_UCHAR ucVal_ )
{
    K_UCHAR ucRet;
    CS_ENTER();
    ucRet = *pucSource_;
    *pucSource_ -= ucVal_;
    CS_EXIT();
    return ucRet;
}

//---------------------------------------------------------------------------
K_USHORT Atomic_Sub16( K_USHORT *pusSource_, K_USHORT usVal_ )
{
    K_USHORT usRet;
    CS_ENTER();
    usRet = *pusSource_;
    *pusSource_ -= usVal_;
    CS_EXIT();
    return usRet;
}

//---------------------------------------------------------------------------
K_ULONG Atomic_Sub32( K_ULONG *pulSource_, K_ULONG ulVal_ )
{
    K_ULONG ulRet;
    CS_ENTER();
    ulRet = *pulSource_;
    *pulSource_ -= ulVal_;
    CS_EXIT();
    return ulRet;
}

//---------------------------------------------------------------------------
K_BOOL Atomic_TestAndSet( K_BOOL *pbLock_ )
{
    K_UCHAR ucRet;
    CS_ENTER();
    ucRet = *pbLock_;
    if (!ucRet)
    {
        *pbLock_ = 1;
    }
    CS_EXIT();
    return ucRet;
}

#endif // KERNEL_USE_ATOMIC
