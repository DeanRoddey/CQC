/*******************************  ZW_SYSDEFS.H  *****************************
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
 * Description: Z-Wave system defines
 *
 * Author:   Ivar Jeppesen
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.21 $
 * Last Changed:     $Date: 2007/01/26 15:26:59 $
 *
 ****************************************************************************/
#ifndef _ZW_SYSDEFS_H_
#define _ZW_SYSDEFS_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#ifdef ZW030x
#include <ZW030x.h>
#elif defined(ZW020x)
#include <ZW020x.h>
#elif defined(ZW010x)
#include <ZW010x.h>
#endif

#define __flash	code

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

/* Reference frequency */

#ifdef ZW010x
#define CPU_FREQ 7376974
#endif /*ZW010x*/
#if defined(ZW020x) || defined(ZW030x)
#define CPU_FREQ  16000000
#endif

#endif /* _ZW_SYSDEFS_H_ */
