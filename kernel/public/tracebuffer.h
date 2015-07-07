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
	\file tracebuffer.h
	\brief Kernel trace buffer class declaration
	
	Global kernel trace-buffer.  Used to instrument the kernel with lightweight
	encoded print statements.  If something goes wrong, the tracebuffer can be
	examined for debugging purposes.  Also, subsets of kernel trace information
	can be extracted and analyzed to provide information about runtime 
	performance, thread-scheduling, and other nifty things in real-time.  
*/
#ifndef __TRACEBUFFER_H__
#define __TRACEBUFFER_H__

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "writebuf16.h"

#if KERNEL_USE_DEBUG && !KERNEL_AWARE_SIMULATION

#ifdef __cplusplus
    extern "C" {
#endif

#define TRACE_BUFFER_SIZE			(16)

//---------------------------------------------------------------------------
/*!
    \fn void TraceBuffer_Init();

    Initialize the tracebuffer before use.
*/
void TraceBuffer_Init();

//---------------------------------------------------------------------------
/*!
    \fn K_USHORT TraceBuffer_Increment();

    Increment the tracebuffer's atomically-incrementing index.

    \return a 16-bit index
*/
K_USHORT TraceBuffer_Increment();

//---------------------------------------------------------------------------
/*!
    \fn void TraceBuffer_Write( K_USHORT *pusData_, K_USHORT usSize_ )

    Write a packet of data to the global tracebuffer.

    \param pusData_ Pointer to the source data buffer to copy to the trace buffer
    \param usSize_ Size of the source data buffer in 16-bit words.
*/
void TraceBuffer_Write( K_USHORT *pusData_, K_USHORT usSize_ );

//---------------------------------------------------------------------------
/*!
    \fn void TraceBuffer_SetCallback( WriteBufferCallback pfCallback_ )

    Set a callback function to be triggered whenever the tracebuffer
    hits the 50% point, or rolls over.

    \param pfCallback_ Callback to assign
*/
void TraceBuffer_SetCallback( WriteBufferCallback pfCallback_ );


#ifdef __cplusplus
    }
#endif

#endif //KERNEL_USE_DEBUG

#endif 
