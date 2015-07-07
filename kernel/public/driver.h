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

    \file   driver.h    

    \brief  Driver abstraction framework

    \section DrvIntro Intro

    This is the basis of the driver framework.  In the context of Mark3, drivers
    don't necessarily have to be based on physical hardware peripherals.  They
    can be used to represent algorithms (such as random number generators), files,
    or protocol stacks.  Unlike FunkOS, where driver IO is protected automatically
    by a Mutex_t, we do not use this kind of protection - we leave it up to the
    driver implementor to do what's right in its own context.  This also frees up
    the driver to implement all sorts of other neat stuff, like sending messages
    to threads associated with the driver.  Drivers are implemented as
    character devices, with the standard array of posix-style accessor methods
    for reading, writing, and general driver control.
    
    A global driver list is provided as a convenient and minimal
    "filesystem" structure, in which devices can be accessed by name.

    \section DrvDesign Driver Design

    A device driver needs to be able to perform the following operations:
        -Initialize a peripheral
        -Start/stop a peripheral
        -Handle I/O control operations
        -Perform various read/write operations

    At the end of the day, that's pretty much all a device driver has to do,
    and all of the functionality that needs to be presented to the developer.
    
    We abstract all device drivers using a base-class which implements the 
    following methods:
        -Start/Open
        -Stop/Close
        -Control
        -Read
        -Write

    A basic driver framework and API can thus be implemented in five function
    calls - that's it! You could even reduce that further by handling the initialize,
    start, and stop operations inside the "control" operation.

    \section DrvAPI Driver API

    In C++, we can implement this as a class to abstract these event handlers, with 
    virtual void functions in the base class overridden by the inherited
    objects.

    To add and remove device drivers from the global table, we use the 
    following methods:

    \code
    void DriverList::Add( Driver *pstDriver_ );
    void DriverList::Remove( Driver *pstDriver_ );
    \endcode

    DriverList::Add()/Remove() takes a single arguments � the pointer to he 
    object to operate on.

    Once a driver has been added to the table, drivers are opened by NAME using
    DriverList::FindByName("/dev/name"). This function returns a pointer to 
    the specified driver if successful, or to a built in /dev/null device 
    if the path name is invalid.  After a driver is open, that pointer is used 
    for all other driver access functions.
    
    This abstraction is incredibly useful � any peripheral or service can be 
    accessed through a consistent set of APIs, that make it easy to substitute 
    implementations from one platform to another.  Portability is ensured, the 
    overhead is negligible, and it emphasizes the reuse of both driver and 
    application code as separate entities.

    Consider a system with drivers for I2C, SPI, and UART peripherals - under 
    our driver framework, an application can initialize these peripherals
    and write a greeting to each using the same simple API functions for all 
    drivers:

    \code
    pstI2C  = DriverList::FindByName("/dev/i2c");
    pstUART = DriverList::FindByName("/dev/tty0");
    pstSPI  = DriverList::FindByName("/dev/spi");

    pstI2C->Write(12,"Hello World!");
    pstUART->Write(12, "Hello World!");
    pstSPI->Write(12, "Hello World!");
    \endcode
    
*/

#include "kerneltypes.h"
#include "mark3cfg.h"

#include "ll.h"

#ifndef __DRIVER_H__
#define __DRIVER_H__

#if KERNEL_USE_DRIVER

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
typedef void* (*OpenFunc_t)(void);
typedef K_UCHAR (*CloseFunc_t)(void);
typedef K_USHORT (*ReadFunc_t)(K_USHORT usSize_, K_UCHAR *pucData_ );
typedef K_USHORT (*WriteFunc_t)(K_USHORT usSize_, K_UCHAR *pucData_ );
typedef K_USHORT (*ControlFunc_t)(K_USHORT usInSize_, K_UCHAR *pucIn_, K_USHORT usOutSize_, K_UCHAR *pucOut_);

//---------------------------------------------------------------------------
typedef struct
{
    OpenFunc_t        pfOpen;
    CloseFunc_t       pfClose;
    ReadFunc_t        pfRead;
    WriteFunc_t       pfWrite
    ControlFunc_t     pfControl;
} DriverVTable_t;

//---------------------------------------------------------------------------
typedef struct
{
    // Inherit from LinkListNode -- must go first
    LinkListNode_t  stNode;

    DriverVTable_t  *pstVTable;
    const K_CHAR    *szName;
} Driver_t;

//---------------------------------------------------------------------------
/*!
    Base device-driver class used in hardware abstraction.  All other device
    drivers inherit from this class
*/

/*!
    \fn void Init()

    Initialize a driver, must be called prior to use
*/
void Driver_Init( Driver_t *pstDriver_ );

/*!
    \fn K_UCHAR Open()

    Open a device driver prior to use.

    \return Driver-specific return code, 0 = OK, non-0 = error
*/
K_UCHAR Driver_Open( Driver_t *pstDriver_ );

/*!
    \fn K_UCHAR Close()

    Close a previously-opened device driver.

    \return Driver-specific return code, 0 = OK, non-0 = error
*/
K_UCHAR Driver_Close( Driver_t *pstDriver_ );

/*!
    \fn K_USHORT Read( K_USHORT usBytes_,
                             K_UCHAR *pucData_)

    Read a specified number of bytes from the device into a specific buffer.
    Depending on the driver-specific implementation, this may be a number
    less than the requested number of bytes read, indicating that there
    there was less input than desired, or that as a result of buffering,
    the data may not be available.

    \param usBytes_ Number of bytes to read (<= size of the buffer)
    \param pucData_ Pointer to a data buffer receiving the read data

    \return Number of bytes actually read
*/
K_USHORT Driver_Read( Driver_t *pstDriver_, K_USHORT usSize_, K_UCHAR *pucData_ );


/*!
    \fn K_USHORT Write( K_USHORT usBytes_,
                              K_UCHAR *pucData_)

    Write a payload of data of a given length to the device.
    Depending on the implementation of the driver, the amount of data
    written to the device may be less than the requested number of
    bytes.  A result less than the requested size may indicate that
    the device buffer is full, indicating that the user must retry
    the write at a later point with the remaining data.

    \param usBytes_ Number of bytes to write (<= size of the buffer)
    \param pucData_ Pointer to a data buffer containing the data to write

    \return Number of bytes actually written
*/
K_USHORT Driver_Write( Driver_t *pstDriver_, K_USHORT usSize_, K_UCHAR *pucData_ );


/*!
    \fn K_USHORT Control( K_USHORT usEvent_,
                                void *pvDataIn_,
                                K_USHORT usSizeIn_,
                                void *pvDataOut_,
                                K_USHORT usSizeOut_ )

    This is the main entry-point for device-specific io and control
    operations.  This is used for implementing all "side-channel"
    communications with a device, and any device-specific IO
    operations that do not conform to the typical POSIX
    read/write paradigm.  Use of this funciton is analagous to
    the non-POSIX (yet still common) devctl() or ioctl().

    \param usEvent_ Code defining the io event (driver-specific)
    \param pvDataIn_ Pointer to the intput data
    \param usSizeIn_ Size of the input data (in bytes)
    \param pvDataOut_ Pointer to the output data
    \param usSizeOut_ Size of the output data (in bytes)

    \return Driver-specific return code, 0 = OK, non-0 = error
*/
K_USHORT Driver_Control( Driver_t *pstDriver_, K_USHORT usInSize_, K_UCHAR *pucIn_, K_USHORT usOutSize_, K_UCHAR *pucOut_);


/*!
    \fn void SetName( const K_CHAR *pcName_ )

    Set the path for the driver.  Name must be set prior to
    access (since driver access is name-based).

    \param pcName_ String constant containing the device path
*/
void Driver_SetName( Driver_t *pstDriver_, const K_CHAR *pcName_ ) { m_pcPath = pcName_; }

/*!
    \fn const K_CHAR *GetPath()

    Returns a string containing the device path.

    \return pcName_ Return the string constant representing the device path
*/
const K_CHAR *Driver_GetPath( Driver_t *pstDriver_ ) { return m_pcPath; }

//---------------------------------------------------------------------------
/*!
    List of Driver objects used to keep track of all device drivers in the 
    system.  By default, the list contains a single entity, "/dev/null".
*/
#ifdef __cplusplus
    }
#endif

#endif //KERNEL_USE_DRIVER

#endif

