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
	\file tracebuffer.cpp
	\brief Kernel trace buffer object definition	
*/

#include "kerneltypes.h"
#include "tracebuffer.h"
#include "mark3cfg.h"
#include "writebuf16.h"
#include "kerneldebug.h"

#if KERNEL_USE_DEBUG && !KERNEL_AWARE_SIMULATION

//---------------------------------------------------------------------------
static WriteBuffer16_t m_clBuffer;		//!< Object used to implement the tracebuffer
static volatile K_USHORT m_usIndex;		//!< Current print index
static K_USHORT m_ausBuffer[ (TRACE_BUFFER_SIZE / sizeof( K_USHORT )) ];	//!< Data buffer

//---------------------------------------------------------------------------
void TraceBuffer_Init()
{
    WriteBuffer16_SetBuffers( &m_clBuffer, m_ausBuffer, TRACE_BUFFER_SIZE/sizeof(K_USHORT));
	m_usIndex = 0;
}

//---------------------------------------------------------------------------
K_USHORT TraceBuffer_Increment()
{
	return m_usIndex++;	
}

//---------------------------------------------------------------------------
void TraceBuffer_Write( K_USHORT *pusData_, K_USHORT usSize_ )
{
	// Pipe the data directly to the circular buffer
    WriteBuffer16_WriteData( &m_clBuffer, pusData_, usSize_ );
}

//---------------------------------------------------------------------------
void TraceBuffer_SetCallback( WriteBufferCallback pfCallback_ )
{
    WriteBuffer16_SetCallback( &m_clBuffer, pfCallback_ );
}

#endif

