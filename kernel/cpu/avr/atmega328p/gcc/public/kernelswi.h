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

    \file   kernelswi.h    

    \brief  Kernel Software interrupt declarations

*/


#include "kerneltypes.h"
#ifndef __KERNELSWI_H_
#define __KERNELSWI_H_

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
/*!
    Class providing the software-interrupt required for context-switching in 
    the kernel.
*/

/*!
    \fn void Config(void)
        
    Configure the software interrupt - must be called before any other 
    software interrupt functions are called.
*/
void KernelSWI_Config(void);

/*!
    \fn void Start(void)
        
    Enable ("Start") the software interrupt functionality
*/
void KernelSWI_Start(void);
    
/*!
    \fn void Stop(void)
        
    Disable the software interrupt functionality
*/
void KernelSWI_Stop(void);
    
/*!
    \fn void Clear(void)
        
    Clear the software interrupt
*/
void KernelSWI_Clear(void);
    
/*!
    Call the software interrupt
        
    \fn void Trigger(void)
*/
void KernelSWI_Trigger(void);
    
/*!
    \fn K_UCHAR DI();
        
    Disable the SWI flag itself
        
    \return previous status of the SWI, prior to the DI call
*/
K_UCHAR KernelSWI_DI();
    
/*!
    \fn void RI(K_BOOL bEnable_)
        
    Restore the state of the SWI to the value specified
        
    \param bEnable_ true - enable the SWI, false - disable SWI
*/        
void KernelSWI_RI(K_BOOL bEnable_);    

#ifdef __cplusplus
    }
#endif

#endif // __KERNELSIW_H_
