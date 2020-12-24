/********************************  ZW_RF010X.H  *****************************
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
 * Description: Application flash ROM RF table offset
 *
 * Author:   Johann Sigfredsson
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.7 $
 * Last Changed:     $Date: 2007/03/09 13:46:30 $
 *
 ****************************************************************************/
#ifndef _ZW_RF010X_H_
#define _ZW_RF010X_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/


/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/


#define RF_MAGIC_VALUE          0x42

/* Valid values for FLASH_FREQ */
 /* EU frequency (868.42MHz) */
#define RF_EU                   0
 /* US frequency (908.42MHz) */
#define RF_US                   1

/* Use default defined in Z-WAVE lib */
#define APP_DEFAULT_FREQ        0xFF  /* Valid values is RF_EU and RF_US */
/* Use default defined in Z-WAVE lib */
#define APP_DEFAULT_RX_MATCH    0xFF
/* Use default defined in Z-WAVE lib */
#define APP_DEFAULT_TX_MATCH    0xFF
/* Use default defined in Z-WAVE lib */
#define APP_DEFAULT_NORM_POWER  0xFF
/* Use default defined in Z-WAVE lib */
#define APP_DEFAULT_LOW_POWER   0xFF

/* Application RF const table offsets */

/* Application RF const table fixed adress in flash ROM - DON'T CHANGE THIS!!! */
#ifdef ZW_DUT_ZEN
#define FLASH_APPL_TABLE_OFFSET 0x0100
#else
#define FLASH_APPL_TABLE_OFFSET 0x7FB0
#endif

/* Must contain RF_MAGIC_VALUE for table to be valid */
#define FLASH_APPL_MAGIC_VALUE_OFFS  0

/* RF frequency number offset */
#define FLASH_APPL_FREQ_OFFS         1
/* MATCHing capacity array value offset for RX */
#define FLASH_APPL_RX_MATCH_OFFS     2
/* MATCHing capacity array value offset for TX */
#define FLASH_APPL_TX_MATCH_OFFS     3
/* Normal power setting offset */
#define FLASH_APPL_NORM_POWER_OFFS   4
/* Low power setting offset */
#define FLASH_APPL_LOW_POWER_OFFS    5



#endif
