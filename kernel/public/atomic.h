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

    \file   atomic.h

    \brief  Basic Atomic Operations
*/

#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "threadport.h"

#if KERNEL_USE_ATOMIC

#ifdef __cplusplus
    extern "C" {
#endif

/*!
 * \brief The Atomic class
 *
 * This utility class provides primatives for atomic operations - that is,
 * operations that are guaranteed to execute uninterrupted.  Basic atomic
 * primatives provided here include Set/Add/Delete for 8, 16, and 32-bit
 * integer types, as well as an atomic test-and-set.
 *
 */

//---------------------------------------------------------------------------
/*!
 * \brief Set Set a variable to a given value in an uninterruptable operation
 * \param pucSource_ Pointer to a variable to set the value of
 * \param ucVal_ New value to set in the variable
 * \return Previously-set value
 */
K_UCHAR Atomic_Set8( K_UCHAR *pucSource_, K_UCHAR ucVal_ );
K_USHORT Atomic_Set16( K_USHORT *pusSource_, K_USHORT usVal_ );
K_ULONG Atomic_Set32( K_ULONG *pulSource_, K_ULONG ulVal_ );

//---------------------------------------------------------------------------
/*!
 * \brief Add Add a value to a variable in an uninterruptable operation
 * \param pucSource_ Pointer to a variable
 * \param ucVal_ Value to add to the variable
 * \return Previously-held value in pucSource_
 */
K_UCHAR Atomic_Add8( K_UCHAR *pucSource_, K_UCHAR ucVal_ );
K_USHORT Atomic_Add16( K_USHORT *pusSource_, K_USHORT usVal_ );
K_ULONG Atomic_Add32( K_ULONG *pulSource_, K_ULONG ulVal_ );

//---------------------------------------------------------------------------
/*!
 * \brief Sub Subtract a value from a variable in an uninterruptable operation
 * \param pucSource_ Pointer to a variable
 * \param ucVal_ Value to subtract from the variable
 * \return Previously-held value in pucSource_
 */
K_UCHAR Atomic_Sub8( K_UCHAR *pucSource_, K_UCHAR ucVal_ );
K_USHORT Atomic_Sub16( K_USHORT *pusSource_, K_USHORT usVal_ );
K_ULONG Atomic_Sub32( K_ULONG *pulSource_, K_ULONG ulVal_ );

//---------------------------------------------------------------------------
/*!
 * \brief TestAndSet Test to see if a variable is set, and set it if
 *        is not already set.  This is an uninterruptable operation.
 *
 *        If the value is false, set the variable to true, and return
 *        the previously-held value.
 *
 *        If the value is already true, return true.
 *
 * \param pbLock Pointer to a value to test against.  This will always
 *        be set to "true" at the end of a call to TestAndSet.
 *
 * \return true - Lock value was "true" on entry, false - Lock was set
 */
K_BOOL Atomic_TestAndSet( K_BOOL *pbLock );

#ifdef __cplusplus
    }
#endif

#endif // KERNEL_USE_ATOMIC

#endif //__ATOMIC_H__
