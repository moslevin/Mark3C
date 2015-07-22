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
static WriteBuffer16_t stBuffer;		//!< Object used to implement the tracebuffer
static volatile K_USHORT usIndex;		//!< Current print index
static K_USHORT ausBuffer[ (TRACE_BUFFER_SIZE / sizeof( K_USHORT )) ];	//!< Data buffer

//---------------------------------------------------------------------------
void TraceBuffer_Init()
{
    WriteBuffer16_SetBuffers( &stBuffer, ausBuffer, TRACE_BUFFER_SIZE/sizeof(K_USHORT));
	usIndex = 0;
}

//---------------------------------------------------------------------------
K_USHORT TraceBuffer_Increment()
{
	return usIndex++;	
}

//---------------------------------------------------------------------------
void TraceBuffer_Write( K_USHORT *pusData_, K_USHORT usSize_ )
{
	// Pipe the data directly to the circular buffer
    WriteBuffer16_WriteData( &stBuffer, pusData_, usSize_ );
}

//---------------------------------------------------------------------------
void TraceBuffer_SetCallback( WriteBufferCallback pfCallback_ )
{
    WriteBuffer16_SetCallback( &stBuffer, pfCallback_ );
}

#endif

