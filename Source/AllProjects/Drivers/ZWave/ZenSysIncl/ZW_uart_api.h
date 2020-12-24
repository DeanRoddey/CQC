/*******************************  ZW_UART_API.H  ****************************
 *           #######
 *           ##  ##
 *           #  ##    ####   #####    #####  ##  ##   #####
 *             ##    ##  ##  ##  ##  ##      ##  ##  ##
 *            ##  #  ######  ##  ##   ####   ##  ##   ####
 *           ##  ##  ##      ##  ##      ##   #####      ##
 *          #######   ####   ##  ##  #####       ##  #####
 *                                           #####
 *          Z-Wave, the wireless language.
 *
 *              Copyright (c) 2001
 *              Zensys A/S
 *              Denmark
 *
 *              All Rights Reserved
 *
 *    This source file is subject to the terms and conditions of the
 *    Zensys Software License Agreement which restricts the manner
 *    in which it may be used.
 *
 *---------------------------------------------------------------------------
 *
 * Description: Interface driver for the built-in UART.
 *
 * Author:   Ivar Jeppesen
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.20 $
 * Last Changed:     $Date: 2007/01/24 09:17:08 $
 *
 ****************************************************************************/
#ifndef _ZW_UART_API_H_
#define _ZW_UART_API_H_
/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/
/* Z-Wave API macroes */
/*===============================   UART_Init   =============================
**  Initializes the MCU's integrated UART.
**  Enables transmit and receive, selects 8 databits and
**  sets the specified baudrate.
**
**  IN  Baud Rate / 100 (e.g. 96, 1152 (9600, 115200))
**--------------------------------------------------------------------------*/
#define ZW_UART_INIT(baud)      UART_Init(baud)

#define ZW_UART_SEND_BYTE(bData) UART_SendByte(bData)
/*===============================   ZW_UART_REC_BYTE =========================
**  Receive a byte over the UART
**
**    Side effects: Wait until data received
**
**--------------------------------------------------------------------------*/
#define ZW_UART_REC_BYTE        (UART_RecByte())

/*===============================   UART_RecStatus   ========================
**  Read the UART receive data status
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_UART_REC_STATUS      (UART_RecStatus())

/*===============================   UART_SendNum   ==========================
**  Converts a byte to a two-byte hexadecimal ASCII representation,
**  and transmits it over the UART.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_UART_SEND_NUM(bData)  UART_SendNum(bData)
/*===============================   UART_SendStr   ==========================
**  Transmit a null terminated string over the UART.
**  The null data is not transmitted.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_UART_SEND_STRING(str)  UART_SendStr(str)

#define ZW_UART_SEND_NL         (UART_SendNL())
/*===============================   UART_SendStatus   ========================
**  Read the UART send data status
**
**  RET TRUE if transmitter busy
**
**--------------------------------------------------------------------------*/
#define ZW_UART_SEND_STATUS     (UART_SendStatus())

#if defined(ZW020x) || defined(ZW030x)
#define ZW_UART_CLEAR_RX    UART_ClearRx()
#define ZW_UART_CLEAR_TX    UART_ClearTx()
#define ZW_UART_READ        UART_Read()
#define ZW_UART_WRITE(TXBYTE)  UART_Write(TXBYTE)
#define ZW_UART_ENABLE           (UART_Enable())
#define ZW_UART_DISABLE           (UART_Disable())
#define ZW_UART_TX_FIRED       (UART_TxFired())
#endif

/* Macroes for debug output */
#ifdef ZW010x
#define ZW_DEBUG_BAUD_RATE    384
#endif
#if defined(ZW020x) || defined(ZW030x)
#define ZW_DEBUG_BAUD_RATE    1152
#endif
#ifdef ZW_DEBUG
#define ZW_DEBUG_INIT(baud)      UART_Init(baud)
#define ZW_DEBUG_SEND_BYTE(bData) UART_SendByte(bData)
#define ZW_DEBUG_SEND_NUM(bData)  UART_SendNum(bData)
#define ZW_DEBUG_SEND_NL()        UART_SendNL()
#else
#define ZW_DEBUG_INIT(baud)
#define ZW_DEBUG_SEND_BYTE(bData)
#define ZW_DEBUG_SEND_NUM(bData)
#define ZW_DEBUG_SEND_NL()
#endif /* ZW_DEBUG */

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/


/*===============================   UART_Init   =============================
**  Initializes the MCU's integrated UART.
**  Enables transmit and receive, selects 8 databits and
**  sets the specified baudrate.
**
**
**--------------------------------------------------------------------------*/
extern	void      /*RET Nothing */
UART_Init(
WORD baudRate);   /*IN  Baud Rate / 100 (e.g. 96, 1152 (9600, 115200)) */

/*===============================   UART_RecStatus   ========================
**  Read the UART receive data status
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern	BYTE            /*RET TRUE if data received */
UART_RecStatus(void);   /*IN  Nothing */

/*===============================   UART_RecByte   ==========================
**  Receive a byte over the UART
**
**    Side effects: Wait until data received
**
**--------------------------------------------------------------------------*/
extern	BYTE          /*RET received data */
UART_RecByte(void);   /*IN  Nothing */

/*===============================   UART_SendStatus   ========================
**  Read the UART send data status
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern	BYTE            /*RET TRUE if transmitter busy */
UART_SendStatus(void);  /*IN  Nothing */

/*===============================   UART_SendByte   =========================
**  Transmit a byte over the UART.
**
**    Side effects: Destroy scratch registers
**                  Wait until data register is free.
**
**--------------------------------------------------------------------------*/
extern	void          /*RET Nothing */
UART_SendByte(
BYTE bData);           /*IN bData to send */

/*===============================   UART_SendNum   ==========================
**  Converts a byte to a two-byte hexadecimal ASCII representation,
**  and transmits it over the UART.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern  void          /*RET Nothing */
UART_SendNum(
BYTE bData);           /*IN bData to send */

/*===============================   UART_SendStr   ==========================
**  Transmit a null terminated string over the UART.
**  The null data is not transmitted.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern  void          /*RET Nothing */
UART_SendStr(
BYTE_P str);          /*IN String pointer */

/*===============================   UART_SendNL   ===========================
**  Transmit CR + LF over the UART.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern  void          /*RET Nothing */
UART_SendNL(void);    /*IN Nothing */

#if defined(ZW020x) || defined(ZW030x)
/*===============================   UART_ClearRx   ========================
**  Clear the UART Rx indication flag
**
**    Side effects:
**--------------------------------------------------------------------------*/
void
UART_ClearRx();
/*===============================   UART_ClearTx   ========================
**  Clear the UART Tx Done indication flag
**
**    Side effects:
**--------------------------------------------------------------------------*/
void
UART_ClearTx();
/*===============================   UART_ReadRx   ========================
**  Read the content of the UART receive register
**
**    Side effects:
**--------------------------------------------------------------------------*/

BYTE    /*RET: the content of the receive register*/
UART_Read();

/*===============================   UART_WriteTx   ========================
**  Write a byte to UART Transmit register
**    Side effects:
**--------------------------------------------------------------------------*/
void
UART_Write(
  BYTE txByte);  /*IN: a byte to write to the UART transmit register.*/


/*===============================   UART_Enable   ========================
**  Enabel the UART.
**    Side effects:
**--------------------------------------------------------------------------*/
void
UART_Enable(void);

/*===============================   UART_Disable   ========================
**  Enabel the UART.
**    Side effects:
**--------------------------------------------------------------------------*/
void
UART_Disable(void);

/*===============================   UART_TxFired   ========================
**  This funtion check if the UART has sent a byte.
**    Side effects:
**--------------------------------------------------------------------------*/
BOOL              /*Return TRUE if the UART is done with sending a byte else retrun FALSE.*/
UART_TxFired(void);

#endif /*ZW020x*/
#endif /* _ZW_UART_API_H_ */
