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

    \file   driver.cpp

    \brief  Device driver/hardware abstraction layer
*/

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "kerneldebug.h"
#include "driver.h"
#include "ll.h"

//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	DRIVER_C       //!< File ID used in kernel trace calls

//---------------------------------------------------------------------------
#if KERNEL_USE_DRIVER

static DoubleLinkList_t m_clDriverList;

/*!
	This class implements the "default" driver (/dev/null)
*/

//---------------------------------------------------------------------------
static DriverVTable_t stDevNullVT =
{
    0,
    0,
    0,
    0,
    0
};

//---------------------------------------------------------------------------
static Driver_t stDevNull = { .pstVTable = &stDevNullVT, .szName = "/dev/null" };

//---------------------------------------------------------------------------
/*!
 * \brief DrvCmp
 *
 * String comparison function used to compare input driver name against a
 * known driver name in the existing driver list.
 *
 * \param szStr1_   user-specified driver name
 * \param szStr2_   name of a driver, provided from the driver table
 * \return  1 on match, 0 on no-match
 */
static K_UCHAR DrvCmp( const K_CHAR *szStr1_, const K_CHAR *szStr2_ )
{
	K_CHAR *szTmp1 = (K_CHAR*) szStr1_;
	K_CHAR *szTmp2 = (K_CHAR*) szStr2_;

	while (*szTmp1 && *szTmp2)
	{
		if (*szTmp1++ != *szTmp2++)
		{
			return 0;
		}
	}

	// Both terminate at the same length
	if (!(*szTmp1) && !(*szTmp2))
	{
		return 1;
	}

	return 0;
}

//---------------------------------------------------------------------------
void DriverList_Init( void )
{
    // Ensure we always have at least one entry - a default in case no match
    // is found (/dev/null)
    DoubleLinkList_Init( (DoubleLinkList_t*)&m_clDriverList );
    DoubleLinkList_Add( (DoubleLinkList_t*)&m_clDriverList, (LinkListNode_t*)&stDevNull);
}

//---------------------------------------------------------------------------
void DriverList_Add( Driver_t *pstDriver_ )
{
    DoubleLinkList_Add( (DoubleLinkList_t*)&m_clDriverList, (LinkListNode_t*)pstDriver_ );
}

//---------------------------------------------------------------------------
Driver_t *DriverList_FindByPath( const K_CHAR *m_pcPath )
{
	KERNEL_ASSERT( m_pcPath );
    Driver_t *pstTemp;

    pstTemp = (Driver_t*)(LinkList_GetHead( (LinkList_t*)&m_clDriverList ) );
	
    // Iterate through the list of drivers until we find a match, or we
    // exhaust our list of installed drivers
	while (pstTemp)
	{

        if( DrvCmp( m_pcPath, Driver_GetPath( pstTemp ) ) )
		{
			return pstTemp;
		}
        pstTemp = (Driver_t*)(LinkListNode_GetNext( (LinkListNode_t*)pstTemp ) );
	}
    // No matching driver found - return a pointer to our /dev/null driver
    return &stDevNull;
}

//---------------------------------------------------------------------------
void Driver_SetName( Driver_t *pstDriver_, const K_CHAR *pcName_ )
{
    pstDriver_->szName = pcName_;
}

//---------------------------------------------------------------------------
const K_CHAR *Driver_GetPath( Driver_t *pstDriver_ )
{
    return pstDriver_->szName;
}

//---------------------------------------------------------------------------
K_UCHAR Driver_Open( Driver_t *pstDriver_ )
{
    if ( pstDriver_->pstVTable->pfOpen )
    {
        return pstDriver_->pstVTable->pfOpen( pstDriver_ );
    }
    return 0;
}
//---------------------------------------------------------------------------
K_UCHAR Driver_Close( Driver_t *pstDriver_ )
{
    if ( pstDriver_->pstVTable->pfClose )
    {
        return pstDriver_->pstVTable->pfClose( pstDriver_ );
    }
    return 0;
}

//---------------------------------------------------------------------------
K_USHORT Driver_Read( Driver_t *pstDriver_, K_USHORT usSize_, K_UCHAR *pucData_ )
{
    if ( pstDriver_->pstVTable->pfRead )
    {
        return pstDriver_->pstVTable->pfRead( pstDriver_, usSize_, pucData_ );
    }
    return usSize_;
}
//---------------------------------------------------------------------------
K_USHORT Driver_Write( Driver_t *pstDriver_, K_USHORT usSize_, K_UCHAR *pucData_ )
{
    if ( pstDriver_->pstVTable->pfWrite )
    {
        return pstDriver_->pstVTable->pfWrite( pstDriver_, usSize_, pucData_ );
    }
    return usSize_;
}
//---------------------------------------------------------------------------
K_USHORT Driver_Control( Driver_t *pstDriver_, K_USHORT usEvent_, K_USHORT usInSize_, K_UCHAR *pucIn_, K_USHORT usOutSize_, K_UCHAR *pucOut_)
{
    if ( pstDriver_->pstVTable->pfControl )
    {
        return pstDriver_->pstVTable->pfControl( pstDriver_, usEvent_, usInSize_, pucIn_, usOutSize_, pucOut_);
    }
    return 0;
}

#endif
