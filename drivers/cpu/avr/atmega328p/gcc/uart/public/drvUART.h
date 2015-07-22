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

    \file   ATMegaUART.h

    \brief  Atmega328p serial port driver

*/
#ifndef __ATMEGAUART_H_
#define __ATMEGAUART_H_

#include "kerneltypes.h"
#include "driver.h"

//---------------------------------------------------------------------------
// UART defines - user-configurable for different targets
//---------------------------------------------------------------------------
#define UART_SRA                (UCSR0A)
#define UART_SRB                (UCSR0B)
#define UART_SRC                (UCSR0C)
#define UART_BAUDH              (UBRR0H)
#define UART_BAUDL              (UBRR0L)
#define UART_RXEN               (RXEN0)
#define UART_TXEN               (TXEN0)
#define UART_TXCIE              (TXCIE0)
#define UART_RXCIE              (RXCIE0)
#define UART_UDR                (UDR0)
#define UART_UDRE               (UDRE0)
#define UART_RXC                (RXC0)

#define UART_DEFAULT_BAUD       ((K_ULONG)57600)

#define UART_RX_ISR             (USART_RX_vect)
#define UART_TX_ISR             (USART_TX_vect)

//---------------------------------------------------------------------------
typedef enum
{
    CMD_SET_BAUDRATE = 0x80,
    CMD_SET_BUFFERS,    
    CMD_SET_RX_ESCAPE,
    CMD_SET_RX_CALLBACK,
    CMD_SET_RX_ECHO,
    CMD_SET_RX_ENABLE,
    CMD_SET_RX_DISABLE
} CMD_UART;

//---------------------------------------------------------------------------
struct ATMegaUART_;

//---------------------------------------------------------------------------
typedef void (*UART_Rx_Callback_t)( struct ATMegaUART_ *pstUART );

//---------------------------------------------------------------------------
/*!
    Implements a UART driver on the ATMega328p
*/
struct ATMegaUART_
{
    //Inherit from Driver_t -- must be first.
    Driver_t stDriver;

    K_UCHAR ucTxSize;                //!< Size of the TX Buffer
    K_UCHAR ucTxHead;                //!< Head index
    K_UCHAR ucTxTail;                //!< Tail index

    K_UCHAR ucRxSize;                //!< Size of the RX Buffer
    K_UCHAR ucRxHead;                //!< Head index
    K_UCHAR ucRxTail;                //!< Tail index

    K_UCHAR bRxOverflow;              //!< Receive buffer overflow
    K_UCHAR bEcho;                    //!< Whether or not to echo RX characters to TX

    K_UCHAR *pucRxBuffer;            //!< Receive buffer pointer
    K_UCHAR *pucTxBuffer;            //!< Transmit buffer pointer

    K_ULONG ulBaudRate;              //!< Baud rate

    K_UCHAR ucRxEscape;              //!< Escape character

    UART_Rx_Callback_t    pfCallback;    //!< Callback function on matched escape character
};

typedef struct ATMegaUART_ ATMegaUART_t;

//---------------------------------------------------------------------------
extern ATMegaUART_t stUART;

//---------------------------------------------------------------------------
void ATMegaUART_Init( ATMegaUART_t *pstUART_ );
//---------------------------------------------------------------------------
K_UCHAR ATMegaUART_Open( ATMegaUART_t *pstUART_ );
//---------------------------------------------------------------------------
K_UCHAR ATMegaUART_Close( ATMegaUART_t *pstUART_ );
//---------------------------------------------------------------------------
K_USHORT ATMegaUART_Read( ATMegaUART_t *pstUART_,
                          K_USHORT usBytes_,
                          K_UCHAR *pucData_ );

//---------------------------------------------------------------------------
K_USHORT ATMegaUART_Write( ATMegaUART_t *pstUART_,
                           K_USHORT usBytes_,
                              K_UCHAR *pucData_ );

//---------------------------------------------------------------------------
K_USHORT ATMegaUART_Control( ATMegaUART_t *pstUART_,
                             K_USHORT usEvent_,
                             K_USHORT usSizeIn_,
                             void *pvIn_,
                             K_USHORT usSizeOut_,
                             void *pvOut_
                             );

//---------------------------------------------------------------------------
/*!
    \fn K_UCHAR *GetRxBuffer(void)

    Return a pointer to the receive buffer for this UART.

    \return pointer to the driver's RX buffer
*/
K_UCHAR *ATMegaUART_GetRxBuffer( ATMegaUART_t *pstUART_ );

//---------------------------------------------------------------------------
/*!
    \fn K_UCHAR *GetTxBuffer(void)

    Return a pointer to the transmit buffer for this UART.

    \return pointer to the driver's TX buffer
*/
K_UCHAR *ATMegaUART_GetTxBuffer( ATMegaUART_t *pstUART_ );

//---------------------------------------------------------------------------

#endif 
