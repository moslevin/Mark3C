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

    \file   profile.cpp

    \brief  Code profiling utilities
*/

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "profile.h"
#include "kernelprofile.h"
#include "threadport.h"
#include "kerneldebug.h"
//---------------------------------------------------------------------------
#if defined __FILE_ID__
	#undef __FILE_ID__
#endif
#define __FILE_ID__ 	PROFILE_C       //!< File ID used in kernel trace calls


#if KERNEL_USE_PROFILER

//---------------------------------------------------------------------------
void ProfileTimer_Init( ProfileTimer_t *pstTimer_ )
{
    pstTimer_->ulCumulative = 0;
    pstTimer_->ulCurrentIteration = 0;
    pstTimer_->usIterations = 0;
    pstTimer_->bActive = 0;
}

//---------------------------------------------------------------------------    
void ProfileTimer_Start( ProfileTimer_t *pstTimer_ )
{
    if (!pstTimer_->bActive)
    {
        CS_ENTER();
        pstTimer_->ulCurrentIteration = 0;
        pstTimer_->ulInitialEpoch = Profiler_GetEpoch();
        pstTimer_->usInitial = Profiler_Read();
        CS_EXIT();
        pstTimer_->bActive = 1;
    }
}

//---------------------------------------------------------------------------    
void ProfileTimer_Stop( ProfileTimer_t *pstTimer_ )
{
    if (pstTimer_->bActive)
    {
        K_USHORT usFinal;
        K_ULONG ulEpoch;
        CS_ENTER();
        usFinal = Profiler_Read();
        ulEpoch = Profiler_GetEpoch();
        // Compute total for current iteration...
        pstTimer_->ulCurrentIteration = ProfileTimer_ComputeCurrentTicks( pstTimer_, usFinal, ulEpoch);
        pstTimer_->ulCumulative += pstTimer_->ulCurrentIteration;
        pstTimer_->usIterations++;
        CS_EXIT();
        pstTimer_->bActive = 0;
    }
}

//---------------------------------------------------------------------------    
K_ULONG ProfileTimer_GetAverage( ProfileTimer_t *pstTimer_ )
{
    if (pstTimer_->usIterations)
    {
        return pstTimer_->ulCumulative / (K_ULONG)pstTimer_->usIterations;
    }
    return 0;
}
 
//---------------------------------------------------------------------------     
K_ULONG ProfileTimer_GetCurrent( ProfileTimer_t *pstTimer_ )
{
    if (pstTimer_->bActive)
    {
		K_USHORT usCurrent;
		K_ULONG ulEpoch;
		CS_ENTER();
        usCurrent = Profiler_Read();
        ulEpoch = Profiler_GetEpoch();
		CS_EXIT();
        return ProfileTimer_ComputeCurrentTicks( pstTimer_, usCurrent, ulEpoch);
    }
    return pstTimer_->ulCurrentIteration;
}

//---------------------------------------------------------------------------
K_ULONG ProfileTimer_ComputeCurrentTicks( ProfileTimer_t *pstTimer_, K_USHORT usCurrent_, K_ULONG ulEpoch_)
{    
    K_ULONG ulTotal;
	K_ULONG ulOverflows;
	
    ulOverflows = ulEpoch_ - pstTimer_->ulInitialEpoch;
	
	// More than one overflow...
	if (ulOverflows > 1)
	{
        ulTotal = ((K_ULONG)(ulOverflows-1) * TICKS_PER_OVERFLOW)
                + (K_ULONG)(TICKS_PER_OVERFLOW - pstTimer_->usInitial) +
				(K_ULONG)usCurrent_;		
	}
	// Only one overflow, or one overflow that has yet to be processed
    else if (ulOverflows || (usCurrent_ < pstTimer_->usInitial))
	{
        ulTotal = (K_ULONG)(TICKS_PER_OVERFLOW - pstTimer_->usInitial) +
				(K_ULONG)usCurrent_;		
	}
	// No overflows, none pending.
	else
	{
        ulTotal = (K_ULONG)(usCurrent_ - pstTimer_->usInitial);
	}

    return ulTotal;
}

#endif
