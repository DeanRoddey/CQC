/*******************************  ZW_non_zero.H  *******************************
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
 * Description: This module define an address range in XRAM reserved for uninitialized variables.
 *
 * Author:   Samer Seoud
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.8 $
 * Last Changed:     $Date: 2007/01/25 10:07:53 $
 *
 ****************************************************************************/
#ifndef _NON_ZERO_H_
#define _NON_ZERO_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

#define XDATA_LENGTH	0x800
#if defined(ZW020x) || defined(ZW030x)
#ifdef ZW_SLAVE
#define NON_ZERO_SIZE    256
#else
#define NON_ZERO_SIZE    16
#endif
#else /* ZW010x */
#define NON_ZERO_SIZE    0
#endif /* ZW020x , ZW030x*/

#define NON_ZERO_START_ADDR   (XDATA_LENGTH-NON_ZERO_SIZE)
#endif /* _NON_ZERO_H_ */
