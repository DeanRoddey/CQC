/*******************************  ZW_EEP_ADDR.H  *******************************
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
 * Description: Application EEPROM start address
 *
 * Author:   Peter Shorty
 *
 * Last Changed By:  $Author: heh $
 * Revision:         $Revision: 1.13 $
 * Last Changed:     $Date: 2006/04/25 10:39:24 $
 *
 ****************************************************************************/
#ifndef _ZW_EEP_ADDR_H_
#define _ZW_EEP_ADDR_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/


/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/
#if defined(ZW_SLAVE) && (!defined(ZW_SLAVE_32))
/* Application data start address in internal EEPROM */
#ifdef ZW_SLAVE_ROUTING
#define EEPROM_APPL_OFFSET    0x80
#else
#define EEPROM_APPL_OFFSET    0x60
#endif
#endif

#ifdef ZW_SLAVE_32
#define EEPROM_APPL_OFFSET    0x120
#endif  /* ZW_SLAVE_32 */

#if defined(ZW_CONTROLLER)||defined(ZW_EEP_LOADER)
/* Application data start address in external EEPROM */
#define EEPROM_APPL_OFFSET    0x2700
#endif

#endif /* _ZW_EEP_ADDR_H_ */
