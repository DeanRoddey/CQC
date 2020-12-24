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
 * Author:   Samer Seoud
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.3 $
 * Last Changed:     $Date: 2007/02/14 12:01:57 $
 *
 ****************************************************************************/
#ifndef _ZW_RF_TF_H_
#define _ZW_RF_TF_H_
/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/
#ifdef TESTFREQ
/*Use these for test freq builds*/
#ifdef ZW010x
#define RF_866                  2
#define RF_870                  3
#define RF_906                  4
#define RF_910                  5
#else
#define RF_866                  4
#define RF_870                  5
#define RF_906                  6
#define RF_910                  7
#endif
#endif /*TESTFREQ*/
#endif
