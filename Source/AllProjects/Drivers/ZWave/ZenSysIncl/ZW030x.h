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
 * Description: Inventra 8051 SFR defines for the Z-Wave ZW030x RF transceiver.
 *
 * Author:   Samer Seoud
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.1 $
 * Last Changed:     $Date: 2007/01/24 09:37:16 $
 *
 ****************************************************************************/
#ifndef _ZW030X_H_
#define _ZW030X_H_

    // Ports : P0 P1
sfr P0       = 0x80;
sfr P1       = 0x90;
    // Ports : P0 P1 direction registers
sfr P0DIR    = 0xAE;
sfr P1DIR    = 0xAF;
    // Ports : P0 P1 pullup registers
sfr P0PULLUP = 0xB1;
sfr P1PULLUP = 0xB2;
    // Address map end
// Auto-generated ^^^^^^^^^^^^^^^^^^

/* The rest! */
sfr ACC    = 0xE0;

/*Ports*/
sfr SP     = 0x81;

sfr PCON   = 0x87;
sfr TCON   = 0x88;
sfr TMOD   = 0x89;
sfr TL0    = 0x8A;
sfr TL1    = 0x8B;
sfr TH0    = 0x8C;
sfr TH1    = 0x8D;


sfr IE     = 0xA8;
sfr IP     = 0xB8;

sfr PSW    = 0xD0;

sfr IE_1    = 0xE8;
sfr B      = 0xF0;
sfr IP1    = 0xF8;

/******  BIT accessible Registers ******/
/*P0*/
sbit P0_0  = P0^0;
sbit P0_1  = P0^1;


/*P1*/
sbit P1_0  = P1^0;
sbit P1_1  = P1^1;
sbit P1_2  = P1^2;
sbit P1_3  = P1^3;
sbit P1_4  = P1^4;
sbit P1_5  = P1^5;
sbit P1_6  = P1^6;
sbit P1_7  = P1^7;


/*  TCON  */
sbit TF1   = TCON^7;
sbit TR1   = TCON^6;
sbit TF0   = TCON^5;
sbit TR0   = TCON^4;
sbit IE1   = TCON^3;
sbit IT1   = TCON^2;
sbit IE0   = TCON^1;
sbit IT0   = TCON^0;


/*IE*/
sbit EA     = IE^7;
sbit ESPI   = IE^5;
sbit ES     = IE^4;
sbit ET1    = IE^3;
sbit EX1    = IE^2;
sbit ET0    = IE^1;
sbit EX0    = IE^0;



/*IP*/
sbit PSPI   = IP^5;
sbit PS     = IP^4;
sbit PT1    = IP^3;
sbit PX1    = IP^2;
sbit PT0    = IP^1;
sbit PX0    = IP^0;

/* IE1 */
sbit ERF     = IE_1^3;
sbit EADC    = IE_1^2;
sbit EGPT    = IE_1^1;
sbit ETRI    = IE_1^0;

/*IP1*/
sbit PRF      = IP1^3;
sbit PADC     = IP1^2;
sbit PGPT     = IP1^1;
sbit PTRI     = IP1^0;



/*PSW*/
/*  PSW */
sbit CY    = PSW^7;
sbit AC    = PSW^6;
sbit F0    = PSW^5;
sbit RS1   = PSW^4;
sbit RS0   = PSW^3;
sbit OV    = PSW^2;
sbit FL    = PSW^1;
sbit P     = PSW^0;

/*ACC*/


/****Common 8051 SFR registers not previously defined****/
/*  BYTE Register  */
sfr DPL  = 0x82;
sfr DPH  = 0x83;



//ISR servicing external 0
#define INUM_INT0    0


//ISR servicing external 1
#define INUM_INT1    2

//ISR servicing timer 1
#define INUM_TIMER1  3

//ISR servicing serial port 0
#define INUM_SERIAL 4

//ISR servicing serial port 1
#define INUM_SPI  5

//INUM servicing TRIAC
#define INUM_TRIAC   6

//ISR servicing general purpose timer
#define INUM_GP_TIMER  7

//ISR servicing ADC
#define INUM_ADC 8


#endif /* _ZW030X_H_ */
