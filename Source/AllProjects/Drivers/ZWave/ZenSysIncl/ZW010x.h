/*******************************  ZW010X.H  *****************************
 *           #######
 *           ##  ##
 *           #  ##    ####   #####    #####  ##  ##   #####
 *             ##    ##  ##  ##  ##  ##      ##  ##  ##
 *            ##  #  ######  ##  ##   ####   ##  ##   ####
 *           ##  ##  ##      ##  ##      ##   #####      ##
 *          #######   ####   ##  ##  #####       ##  #####
 *                                           #####
 *          Z-Wave, the wireless lauguage.
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
 * Description: Synopsis DW8051 SFR defines for the Z-Wave ZW010x RF transceiver.
 *
 * Author:   Ivar Jeppesen
 *
 * Last Changed By:  $Author: jfr $
 * Revision:         $Revision: 1.7 $
 * Last Changed:     $Date: 2005/03/16 13:44:37 $
 *
 ****************************************************************************/
#ifndef _ZW010X_H_
#define _ZW010X_H_

/* Timer 2 / 3 */
sfr TCON2     = 0xA9;
sfr T2PRE     = 0xAA;
sfr T3PRE     = 0xAB;
sfr T2        = 0xAC;
sfr T3        = 0xAD;

sfr ACC    = 0xE0;

/*Ports*/
sfr P0     = 0x80; /*Only P0.0 to P0.3 exists*/
sfr P2     = 0xA0; /*P2.0 Triac output, P2.1 Zero cross input*/
sfr P3     = 0xB0; /*Only P3.0 to P3.4 exists*/

sfr P0DIR  = 0xA4;
sfr P1DIR  = 0xA5;
sfr P2DIR  = 0xA6;
sfr P3DIR  = 0xA7;

sfr SP     = 0x81;

sfr16 DP0  = 0x82;
sfr DPL0   = 0x82;
sfr DPH0   = 0x83;

sfr16 DP1  = 0x84;
sfr DPL1   = 0x84;
sfr DPH1   = 0x85;

sfr DPS    = 0x86;

sfr PCON   = 0x87;
sfr TCON   = 0x88;
sfr TMOD   = 0x89;
sfr TL0    = 0x8A;
sfr TL1    = 0x8B;
sfr TH0    = 0x8C;
sfr TH1    = 0x8D;
sfr CKCON  = 0x8E;

sfr EXIF   = 0x91;
sfr MPAGE  = 0x92;
sfr SCON0  = 0x98;
sfr SBUF0  = 0x99;
sfr IE     = 0xA8;
sfr IP     = 0xB8;

sfr SCON1  = 0xC0;
sfr SBUF1  = 0xC1;

sfr PSW    = 0xD0;

sfr EICON  = 0xD8;
sfr EIE    = 0xE8;
sfr B      = 0xF0;
sfr EIP    = 0xF8;


/******  BIT accessible Registers ******/
/*P0*/
sbit P0_0  = P0^0;
sbit P0_1  = P0^1;
sbit P0_2  = P0^2;
sbit P0_3  = P0^3;

sbit MI    = P0^2;
sbit MO    = P0^1;
sbit SCK   = P0^0;

/*P2*/
sbit P2_2  = P2^2;
sbit P2_3  = P2^3;
sbit P2_4  = P2^4;
sbit P2_5  = P2^5;
sbit P2_6  = P2^6;
sbit P2_7  = P2^7;

sbit ZEROX = P2^2;
sbit TRIAC = P2^3;

/*P3 */
sbit P3_0  = P3^0;
sbit P3_1  = P3^1;
sbit P3_2  = P3^2;
sbit P3_4  = P3^4;

sbit RXD0   = P3^0;
sbit TXD0   = P3^1;
sbit INT0_N = P3^2;
sbit T0     = P3^4;


/*  TCON  */
sbit TF1   = TCON^7;
sbit TR1   = TCON^6;
sbit TF0   = TCON^5;
sbit TR0   = TCON^4;
sbit IE1   = TCON^3;
sbit IT1   = TCON^2;
sbit IE0   = TCON^1;
sbit IT0   = TCON^0;

/*  SCON0  */
sbit SM0_0 = SCON0^7; /* alternative SM0_FE_0 */
sbit SM1_0 = SCON0^6; /* alternative SM1_0 */
sbit SM2_0 = SCON0^5; /* alternative SM2_0 */
sbit REN_0 = SCON0^4; /* alternative REN_0 */
sbit TB8_0 = SCON0^3; /* alternative TB8_0 */
sbit RB8_0 = SCON0^2; /* alternative RB8_0 */
sbit TI_0  = SCON0^1; /* alternative TI_0  */
sbit RI_0  = SCON0^0; /* alternative RI_0  */

/*  SCON1  */
sbit SM0_1 = SCON1^7; /* alternative SM0_FE_1 */
sbit SM1_1 = SCON1^6; /* alternative SM1_1 */
sbit SM2_1 = SCON1^5; /* alternative SM2_1 */
sbit REN_1 = SCON1^4; /* alternative REN_1 */
sbit TB8_1 = SCON1^3; /* alternative TB8_1 */
sbit RB8_1 = SCON1^2; /* alternative RB8_1 */
sbit TI_1  = SCON1^1; /* alternative TI_1  */
sbit RI_1  = SCON1^0; /* alternative RI_1  */

/*IE*/
sbit EA    = IE^7;
sbit ES1   = IE^6;
sbit ES0   = IE^4;
sbit ET1   = IE^3;
sbit EX1   = IE^2;
sbit ET0   = IE^1;
sbit EX0   = IE^0;

/*IP*/
sbit PS1   = IP^6;
sbit PS0   = IP^4;
sbit PT1   = IP^3;
sbit PX1   = IP^2;
sbit PT0   = IP^1;
sbit PX0   = IP^0;

/*PSW*/
sbit CY    = PSW^7;
sbit AC    = PSW^6;
sbit F0    = PSW^5;
sbit RS1   = PSW^4;
sbit RS0   = PSW^3;
sbit OV    = PSW^2;
sbit FL    = PSW^1;
sbit P     = PSW^0;

/*EICON*/
sbit SMOD1 = EICON^7;
sbit FWIE  = EICON^5;
sbit FWIF  = EICON^4;
sbit RTCIF = EICON^3;

/*EIE*/
sbit RTCIE   = EIE^4;
sbit ET3     = EIE^3;
sbit ADIE    = EIE^2;
sbit ET2     = EIE^1;
sbit RFIE    = EIE^0;

/*EIP*/
sbit PRTC    = EIP^4;
sbit PT3     = EIP^3;
sbit PAD     = EIP^2;
sbit PT2     = EIP^1;
sbit PRF     = EIP^0;

/****Common 8051 SFR registers not previously defined****/

/*  BYTE Register  */

sfr DPL  = 0x82;
sfr DPH  = 0x83;
sfr SCON = 0x98;
sfr SBUF = 0x99;

/*  BIT Register  */

/*  IE   */
sbit ES   = 0xAC;

/*  IP   */
sbit PS   = 0xBC;

/*  P3  */
sbit INT1 = 0xB3;
sbit INT0 = 0xB2;
sbit TXD  = 0xB1;
sbit RXD  = 0xB0;

/*  SCON  */
sbit SM0  = 0x9F;
sbit SM1  = 0x9E;
sbit SM2  = 0x9D;
sbit REN  = 0x9C;
sbit TB8  = 0x9B;
sbit RB8  = 0x9A;
sbit TI   = 0x99;
sbit RI   = 0x98;

/* ISR vector definitions */

/* ISR servicing external 0 - Pin P32/INT_N */
#define INUM_INT0    0

/* ISR servicing timer 0 */
#define INUM_TIMER0  1

/* ISR servicing timer 1 */
#define INUM_TIMER1  3

/* ISR servicing timer 2 */
#define INUM_TIMER2  9

/* ISR servicing serial port 0 */
#define INUM_SERIAL0 4

/* ISR servicing  ADC  */
#define INUM_ADC     10

#endif /* _ZW010X_H_ */
