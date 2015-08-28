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
    \file kernelprofile.cpp
    
    \brief Profiling timer implementation
*/

#include "kerneltypes.h"
#include "mark3cfg.h"
#include "profile.h"
#include "kernelprofile.h"
#include "threadport.h"


#if KERNEL_USE_PROFILER
K_ULONG m_ulEpoch;

//---------------------------------------------------------------------------
void Profiler_Init()
{

}

//---------------------------------------------------------------------------
void Profiler_Start()
{

}    

//---------------------------------------------------------------------------
void Profiler_Stop()
{

}    
//---------------------------------------------------------------------------
K_USHORT Profiler_Read()
{
    return 0;
}

//---------------------------------------------------------------------------
void Profiler_Process()
{

}

//---------------------------------------------------------------------------
K_ULONG Profiler_GetEpoch()
{
	return m_ulEpoch;
}
#endif
