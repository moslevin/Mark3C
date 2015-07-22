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
	\file writebuf16.cpp
	
	\brief 16 bit circular buffer implementation with callbacks.
*/

#include "kerneltypes.h"
#include "writebuf16.h"
#include "kerneldebug.h"
#include "threadport.h"

#if KERNEL_USE_DEBUG && !KERNEL_AWARE_SIMULATION
//---------------------------------------------------------------------------
void WriteBuffer16_SetBuffers( WriteBuffer16_t *pstBuffer_, K_USHORT *pusData_, K_USHORT usSize_ )
{
	pstBuffer_->pusData = pusData_;
	pstBuffer_->usSize = usSize_;
	pstBuffer_->usHead = 0;
	pstBuffer_->usTail = 0;
}
//---------------------------------------------------------------------------
void WriteBuffer16_SetCallback( WriteBuffer16_t *pstBuffer_, WriteBufferCallback pfCallback_ )
{ 
	pstBuffer_->pfCallback = pfCallback_; 
}
//---------------------------------------------------------------------------
void WriteBuffer16_WriteData( WriteBuffer16_t *pstBuffer_, K_USHORT *pusBuf_, K_USHORT usLen_ )
{
	K_USHORT *apusBuf[1];
	K_USHORT ausLen[1];
	
	apusBuf[0] = pusBuf_;
	ausLen[0] = usLen_;
	
	WriteBuffer16_WriteVector( pstBuffer_, apusBuf, ausLen, 1 );
}

//---------------------------------------------------------------------------
void WriteBuffer16_WriteVector( WriteBuffer16_t *pstBuffer_, K_USHORT **ppusBuf_, K_USHORT *pusLen_, K_UCHAR ucCount_ )
{
	K_USHORT usTempHead;	
	K_UCHAR i;
	K_UCHAR j;
	K_USHORT usTotalLen = 0;
    K_BOOL bCallback = false;
    K_BOOL bRollover = false;
	// Update the head pointer synchronously, using a small 
	// critical section in order to provide thread safety without
	// compromising on responsiveness by adding lots of extra
	// interrupt latency.
	
	CS_ENTER();
	
    usTempHead = pstBuffer_->usHead;
	{		
		for (i = 0; i < ucCount_; i++)
		{
			usTotalLen += pusLen_[i];
		}
        pstBuffer_->usHead = (usTempHead + usTotalLen) % pstBuffer_->usSize;
	}	
	CS_EXIT();
	
	// Call the callback if we cross the 50% mark or rollover 
    if (pstBuffer_->usHead < usTempHead)
	{
        if (pstBuffer_->pfCallback)
		{
			bCallback = true;
			bRollover = true;
		}
	}
    else if ((usTempHead < (pstBuffer_->usSize >> 1)) && (pstBuffer_->usHead >= (pstBuffer_->usSize >> 1)))
	{
		// Only trigger the callback if it's non-null
        if (pstBuffer_->pfCallback)
		{
			bCallback = true;
		}
	}	
	
	// Are we going to roll-over?
	for (j = 0; j < ucCount_; j++)
	{
		K_USHORT usSegmentLength = pusLen_[j];
        if (usSegmentLength + usTempHead >= pstBuffer_->usSize)
		{	
			// We need to two-part this... First part: before the rollover
			K_USHORT usTempLen;
            K_USHORT *pusTmp = &pstBuffer_->pusData[ usTempHead ];
			K_USHORT *pusSrc = ppusBuf_[j];
            usTempLen = pstBuffer_->usSize - usTempHead;
			for (i = 0; i < usTempLen; i++)
			{
				*pusTmp++ = *pusSrc++;
			}

			// Second part: after the rollover
			usTempLen = usSegmentLength - usTempLen;
            pusTmp = pstBuffer_->pusData;
			for (i = 0; i < usTempLen; i++)
			{		
				*pusTmp++ = *pusSrc++;
			}
		}
		else
		{	
			// No rollover - do the copy all at once.
			K_USHORT *pusSrc = ppusBuf_[j];
            K_USHORT *pusTmp = &pstBuffer_->pusData[ usTempHead ];
			for (K_USHORT i = 0; i < usSegmentLength; i++)
			{		
				*pusTmp++ = *pusSrc++;
			}
		}
	}


	// Call the callback if necessary
	if (bCallback)
	{
		if (bRollover)
		{
			// Rollover - process the back-half of the buffer
            pstBuffer_->pfCallback( &pstBuffer_->pusData[ pstBuffer_->usSize >> 1], pstBuffer_->usSize >> 1 );
		} 
		else 
		{
			// 50% point - process the front-half of the buffer
            pstBuffer_->pfCallback( pstBuffer_->pusData, pstBuffer_->usSize >> 1);
		}
	}
}

#endif
