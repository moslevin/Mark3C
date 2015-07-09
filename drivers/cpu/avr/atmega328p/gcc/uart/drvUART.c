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

    \file   drvUART.cpp

    \brief  Atmega328p serial port driver
*/

#include "kerneltypes.h"
#include "drvUART.h"
#include "driver.h"
#include "thread.h"
#include "threadport.h"
#include "kerneltimer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

//---------------------------------------------------------------------------
static ATMegaUART_t *pstActive;    // Pointer to the active object

//---------------------------------------------------------------------------
static DriverVTable_t stUART_VT =
{
    (OpenFunc_t)ATMegaUART_Open,
    (CloseFunc_t)ATMegaUART_Close,
    (ReadFunc_t)ATMegaUART_Read,
    (WriteFunc_t)ATMegaUART_Write,
    (ControlFunc_t)ATMegaUART_Control
};

//---------------------------------------------------------------------------
ATMegaUART_t stUART =
{
    { .pstVTable = &stUART_VT, .szName = "/dev/tty" }
};

//---------------------------------------------------------------------------
static void ATMegaUART_StartTx( ATMegaUART_t *pstUART_ );
static void ATMegaUART_TxISR( ATMegaUART_t *pstUART_ );

//---------------------------------------------------------------------------
static void ATMegaUART_SetBaud( ATMegaUART_t *pstUART_ )
{    
    K_USHORT usBaudTemp;
    K_USHORT usPortTemp;
    
    // Calculate the baud rate from the value in the driver.    
    usBaudTemp = (K_USHORT)(((SYSTEM_FREQ/16)/pstUART_->m_ulBaudRate) - 1);

    // Save the current port config registers
    usPortTemp = UART_SRB;
    
    // Clear the registers (disable rx/tx/interrupts)
    UART_SRB = 0;
    UART_SRA = 0;
    
    // Set the baud rate high/low values.
    UART_BAUDH = (K_UCHAR)(usBaudTemp >> 8);
    UART_BAUDL = (K_UCHAR)(usBaudTemp & 0x00FF);
    
    // Restore the Rx/Tx flags to their previous state
    UART_SRB = usPortTemp;
}

//---------------------------------------------------------------------------
void ATMegaUART_Init( ATMegaUART_t *pstUART_ )
{    
    // Set up the FIFOs
    pstUART_->m_ucTxHead = 0;
    pstUART_->m_ucTxTail = 0;
    pstUART_->m_ucRxHead = 0;
    pstUART_->m_ucRxTail = 0;
    pstUART_->m_bEcho = 0;
    pstUART_->m_ucRxEscape = '\n';
    pstUART_->pfCallback = NULL;
    pstUART_->m_bRxOverflow = 0;
    pstUART_->m_ulBaudRate = UART_DEFAULT_BAUD;
    
    // Clear flags
    UART_SRA = 0;
    UART_SRB = 0;
    
    ATMegaUART_SetBaud( pstUART_ );
    
    // Set frame format: 8 N 1
    UART_SRC = 0x06;        
}

//---------------------------------------------------------------------------
K_UCHAR ATMegaUART_Open( ATMegaUART_t *pstUART_ )
{  
    // Enable Rx/Tx + Interrupts
    UART_SRB |= (1 << UART_RXEN) | ( 1 << UART_TXEN);
    UART_SRB |= (1 << UART_RXCIE) | (1 << UART_TXCIE);
    pstActive = pstUART_;
    return 0;
}

//---------------------------------------------------------------------------
K_UCHAR ATMegaUART_Close( ATMegaUART_t *pstUART_ )
{
    // Disable Rx/Tx + Interrupts 
    UART_SRB &= ~((1 << UART_RXEN) | ( 1 << UART_TXEN));
    UART_SRB &= ~((1 << UART_TXCIE) | (1 << UART_RXCIE));
    return 0;
}

//---------------------------------------------------------------------------
K_USHORT ATMegaUART_Control( ATMegaUART_t *pstUART_, K_USHORT usCmdId_, K_USHORT usSizeIn_, void *pvIn_, K_USHORT usSizeOut_, void *pvOut_ )
{
    switch ((CMD_UART)usCmdId_)
    {
        case CMD_SET_BAUDRATE:
        {
            K_ULONG ulBaudRate = *((K_ULONG*)pvIn_);
            pstUART_->m_ulBaudRate = ulBaudRate;
            ATMegaUART_SetBaud( pstUART_ );
        }
            break;
        case CMD_SET_BUFFERS:
        {
            pstUART_->m_pucRxBuffer = (K_UCHAR*)pvIn_;
            pstUART_->m_pucTxBuffer = (K_UCHAR*)pvOut_;
            pstUART_->m_ucRxSize = usSizeIn_;
            pstUART_->m_ucTxSize = usSizeOut_;
        }            
            break;        
        case CMD_SET_RX_ESCAPE:
        {
            pstUART_->m_ucRxEscape = *((K_UCHAR*)pvIn_);
        }
            break;
        case CMD_SET_RX_CALLBACK:
        {
            pstUART_->pfCallback = (UART_Rx_Callback_t)pvIn_;
        }
            break;
        case CMD_SET_RX_ECHO:
        {
            pstUART_->m_bEcho = *((K_UCHAR*)pvIn_);
        }
            break;
        case CMD_SET_RX_ENABLE:
        {
            UART_SRB |= (1 << UART_RXEN);
        }
            break;
        case CMD_SET_RX_DISABLE:
        {
            UART_SRB &= ~(1 << UART_RXEN);
        }
            break;                        
        default:
            break;
    }
    return 0;
}

//---------------------------------------------------------------------------
K_USHORT ATMegaUART_Read( ATMegaUART_t *pstUART_, K_USHORT usSizeIn_, K_UCHAR *pvData_ )
{
    // Read a string of characters of length N.  Return the number of bytes
    // actually read.  If less than the 1 length, this indicates that
    // the buffer is full and that the app needs to wait.
    
    K_USHORT i = 0;
    K_USHORT usRead = 0;
    K_BOOL bExit = 0;
    K_UCHAR *pucData = (K_UCHAR*)pvData_;
    
    for (i = 0; i < usSizeIn_; i++)
    {        
        // If Tail != Head, there's data in the buffer.
        CS_ENTER();
        if (pstUART_->m_ucRxTail != pstUART_->m_ucRxHead)
        {
            // We have room to add the byte, so add it.
            pucData[i] = pstUART_->m_pucRxBuffer[pstUART_->m_ucRxTail];
            
            // Update the buffer head pointer.
            pstUART_->m_ucRxTail++;
            if (pstUART_->m_ucRxTail >= pstUART_->m_ucRxSize)
            {
                pstUART_->m_ucRxTail = 0;
            }
            usRead++;
        }
        else
        {
            // Can't do anything else - the buffer is empty
            bExit = 1; 
        } 
        CS_EXIT();
        
        // If we have to bail because the buffer is empty, do it now.
        if (bExit == 1)
        {
            break;
        }        
    }
    return usRead;
}

//---------------------------------------------------------------------------
K_USHORT ATMegaUART_Write( ATMegaUART_t *pstUART_, K_USHORT usSizeOut_, K_UCHAR *pvData_)
{
    // Write a string of characters of length N.  Return the number of bytes
    // actually written.  If less than the 1 length, this indicates that
    // the buffer is full and that the app needs to wait.    
    K_USHORT i = 0;
    K_USHORT usWritten = 0;
    K_UCHAR ucNext;
    K_BOOL bActivate = 0;
    K_BOOL bExit = 0;
    K_UCHAR *pucData = (K_UCHAR*)pvData_;
    
    // If the head = tail, we need to start sending data out the data ourselves.
    if (pstUART_->m_ucTxHead == pstUART_->m_ucTxTail)
    {
        bActivate = 1;
    }
    
    for (i = 0; i < usSizeOut_; i++)
    {
        CS_ENTER();
        // Check that head != tail (we have room)
        ucNext = (pstUART_->m_ucTxHead + 1);
        if (ucNext >= pstUART_->m_ucTxSize)
        {
            ucNext = 0;
        }
        
        if (ucNext != pstUART_->m_ucTxTail)
        {
            // We have room to add the byte, so add it.
            pstUART_->m_pucTxBuffer[pstUART_->m_ucTxHead] = pucData[i];
            
            // Update the buffer head pointer.
            pstUART_->m_ucTxHead = ucNext;
            usWritten++;
        }
        else
        {
            // Can't do anything - buffer is full
            bExit = 1;
        } 
        CS_EXIT();
        
        // bail if the buffer is full
        if (bExit == 1)
        {
            break;
        }        
    }
    
    // Activate the transmission if we're currently idle
    if (bActivate == 1)
    {
        // We know we're idle if we get here.
        CS_ENTER();
        if (UART_SRA & (1 << UDRE0))
        {
            ATMegaUART_StartTx(pstUART_);
        }        
        CS_EXIT();
    }
    
     return usWritten;
}

//---------------------------------------------------------------------------
void ATMegaUART_StartTx( ATMegaUART_t *pstUART_ )
{
    // Send the tail byte out.
    K_UCHAR ucNext;

    CS_ENTER();
    
    // Send the byte at the tail index
    UART_UDR = pstUART_->m_pucTxBuffer[pstUART_->m_ucTxTail];
    
    // Update the tail index
    ucNext = (pstUART_->m_ucTxTail + 1);
    if (ucNext >= pstUART_->m_ucTxSize)
    {
        ucNext = 0;
    }
    pstUART_->m_ucTxTail = ucNext;
    
    CS_EXIT();
}

//---------------------------------------------------------------------------
void ATMegaUART_RxISR( ATMegaUART_t *pstUART_ )
{
    K_UCHAR ucTemp;
    K_UCHAR ucNext;
    
    // Read the byte from the data buffer register
    ucTemp = UART_UDR;
    
    // Check that head != tail (we have room)
    ucNext = (pstUART_->m_ucRxHead + 1);
    if (ucNext >= pstUART_->m_ucRxSize)
    {
        ucNext = 0;
    }
    
    // Always add the byte to the buffer (but flag an error if it's full...)
    pstUART_->m_pucRxBuffer[pstUART_->m_ucRxHead] = ucTemp;
    
    // Update the buffer head pointer.
    pstUART_->m_ucRxHead = ucNext;
    
    // If the buffer's full, discard the oldest byte in the buffer and flag an error
    if (ucNext == pstUART_->m_ucRxTail)
    {
        // Update the buffer tail pointer
        pstUART_->m_ucRxTail = (pstUART_->m_ucRxTail + 1);
        if (pstUART_->m_ucRxTail >= pstUART_->m_ucRxSize)
        {
            pstUART_->m_ucRxTail = 0;
        }

        // Flag an error - the buffer is full
        pstUART_->m_bRxOverflow = 1;
    }
    
    // If local-echo is enabled, TX the K_CHAR
    if (pstUART_->m_bEcho)
    {
        ATMegaUART_Write(pstUART_, 1, &ucTemp);
    }
    
    // If we've hit the RX callback character, run the callback
    // This is used for calling line-end functions, etc..
    if (ucTemp == pstUART_->m_ucRxEscape)
    {
        if (pstUART_->pfCallback)
        {
            pstUART_->pfCallback( pstUART_ );
        }
    }
}

//---------------------------------------------------------------------------
ISR(UART_RX_ISR)
{
    ATMegaUART_RxISR( pstActive );
}

//---------------------------------------------------------------------------
void ATMegaUART_TxISR( ATMegaUART_t *pstUART_ )
{
    // If the head != tail, there's something to send.
    if (pstUART_->m_ucTxHead != pstUART_->m_ucTxTail)
    {
        ATMegaUART_StartTx( pstUART_ );
    }
}

//---------------------------------------------------------------------------
ISR(UART_TX_ISR)
{
    ATMegaUART_TxISR( pstActive );
}
