/*******************************  ZW_TRIAC_API.H  ****************************
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
 * Description: Interface driver for the ZW0101/2 built-in TRIAC controller.
 *
 * Author:   Johann Sigfredsson
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.14 $
 * Last Changed:     $Date: 2007/01/24 09:24:39 $
 *
 ****************************************************************************/
#ifndef _ZW_TRIAC_API_H_
#define _ZW_TRIAC_API_H_

#include <ZW_typedefs.h>

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

/* Z-Wave API macroes */
/* Defines used to select mains frequency when initializing TRIAC controller */
#define FREQUENCY_60HZ                        0
#define FREQUENCY_50HZ                        1
#ifdef ZW010x
#define TRIAC_3_WIRE                          0
#define TRIAC_2_WIRE                          1
#endif

#if defined(ZW020x) || defined(ZW030x)

#define TRIAC_HALFBRIDGE                      0
#define TRIAC_FULLBRIDGE                      1

/*Those constants specifiy the voltage drop across the ZEROX input*/
#ifdef US

#define VOLTAGE_DROP_0mV        0
#define VOLTAGE_DROP_8mV        1
#define VOLTAGE_DROP_16mV       2
#define VOLTAGE_DROP_24mV       3
#define VOLTAGE_DROP_32mV       4
#define VOLTAGE_DROP_40mV       5
#define VOLTAGE_DROP_48mV       6
#define VOLTAGE_DROP_56mV       7
#define VOLTAGE_DROP_64mV       8
#define VOLTAGE_DROP_72mV       9
#define VOLTAGE_DROP_80mV       10
#define VOLTAGE_DROP_88mV       11
#define VOLTAGE_DROP_96mV       12
#define VOLTAGE_DROP_104mV       13
#define VOLTAGE_DROP_112mV       14
#define VOLTAGE_DROP_120mV       15
#define VOLTAGE_DROP_128mV       16
#define VOLTAGE_DROP_136mV       17
#define VOLTAGE_DROP_144mV       18
#define VOLTAGE_DROP_152mV       19
#define VOLTAGE_DROP_160mV       20
#define VOLTAGE_DROP_168mV       21
#define VOLTAGE_DROP_176mV       22
#define VOLTAGE_DROP_184mV       23
#define VOLTAGE_DROP_192mV       24
#define VOLTAGE_DROP_200mV       25
#define VOLTAGE_DROP_208mV       26
#define VOLTAGE_DROP_216mV       27
#define VOLTAGE_DROP_224mV       28
#define VOLTAGE_DROP_232mV       29
#define VOLTAGE_DROP_240mV       30
#define VOLTAGE_DROP_248mV       31
#define VOLTAGE_DROP_256mV       32
#define VOLTAGE_DROP_264mV       33
#define VOLTAGE_DROP_272mV       34
#define VOLTAGE_DROP_280mV       35
#define VOLTAGE_DROP_288mV       36
#define VOLTAGE_DROP_296mV       37
#define VOLTAGE_DROP_304mV       38
#define VOLTAGE_DROP_312mV       39
#define VOLTAGE_DROP_320mV       40
#define VOLTAGE_DROP_328mV       41
#define VOLTAGE_DROP_336mV       42
#define VOLTAGE_DROP_344mV       43
#define VOLTAGE_DROP_352mV       44
#define VOLTAGE_DROP_360mV       45
#define VOLTAGE_DROP_368mV       46
#define VOLTAGE_DROP_376mV       47
#define VOLTAGE_DROP_384mV       48
#define VOLTAGE_DROP_392mV       49
#define VOLTAGE_DROP_400mV       50
#define VOLTAGE_DROP_408mV       51
#define VOLTAGE_DROP_416mV       52
#define VOLTAGE_DROP_424mV       53
#define VOLTAGE_DROP_432mV       54
#define VOLTAGE_DROP_440mV       55
#define VOLTAGE_DROP_448mV       56
#define VOLTAGE_DROP_456mV       57
#define VOLTAGE_DROP_464mV       58
#define VOLTAGE_DROP_472mV       59
#define VOLTAGE_DROP_480mV       60
#define VOLTAGE_DROP_488mV       61
#define VOLTAGE_DROP_496mV       62
#define VOLTAGE_DROP_504mV       63
#define VOLTAGE_DROP_512mV       64
#define VOLTAGE_DROP_520mV       65
#define VOLTAGE_DROP_528mV       66
#define VOLTAGE_DROP_536mV       67
#define VOLTAGE_DROP_544mV       68
#define VOLTAGE_DROP_552mV       69
#define VOLTAGE_DROP_560mV       70
#define VOLTAGE_DROP_568mV       71
#define VOLTAGE_DROP_576mV       72
#define VOLTAGE_DROP_584mV       73
#define VOLTAGE_DROP_592mV       74
#define VOLTAGE_DROP_600mV       75
#define VOLTAGE_DROP_608mV       76
#define VOLTAGE_DROP_616mV       77
#define VOLTAGE_DROP_624mV       78
#define VOLTAGE_DROP_632mV       79
#define VOLTAGE_DROP_640mV       80
#define VOLTAGE_DROP_648mV       81
#define VOLTAGE_DROP_656mV       82
#define VOLTAGE_DROP_664mV       83
#define VOLTAGE_DROP_672mV       84
#define VOLTAGE_DROP_680mV       85
#define VOLTAGE_DROP_688mV       86
#define VOLTAGE_DROP_696mV       87
#define VOLTAGE_DROP_704mV       88
#define VOLTAGE_DROP_712mV       89
#define VOLTAGE_DROP_720mV       90
#define VOLTAGE_DROP_728mV       91
#define VOLTAGE_DROP_736mV       92
#define VOLTAGE_DROP_744mV       93
#define VOLTAGE_DROP_752mV       94
#define VOLTAGE_DROP_760mV       95
#define VOLTAGE_DROP_768mV       96
#define VOLTAGE_DROP_776mV       97
#define VOLTAGE_DROP_784mV       98
#define VOLTAGE_DROP_792mV       99
#define VOLTAGE_DROP_800mV       100
#define VOLTAGE_DROP_808mV       101
#define VOLTAGE_DROP_816mV       102
#define VOLTAGE_DROP_824mV       103
#define VOLTAGE_DROP_832mV       104
#define VOLTAGE_DROP_840mV       105
#define VOLTAGE_DROP_848mV       106
#define VOLTAGE_DROP_856mV       107
#define VOLTAGE_DROP_864mV       108
#define VOLTAGE_DROP_872mV       109
#define VOLTAGE_DROP_880mV       110
#define VOLTAGE_DROP_888mV       111
#define VOLTAGE_DROP_896mV       112
#define VOLTAGE_DROP_904mV       113
#define VOLTAGE_DROP_912mV       114
#define VOLTAGE_DROP_920mV       115
#define VOLTAGE_DROP_928mV       116
#define VOLTAGE_DROP_936mV       117
#define VOLTAGE_DROP_944mV       118
#define VOLTAGE_DROP_952mV       119
#define VOLTAGE_DROP_960mV       120
#define VOLTAGE_DROP_968mV       121
#define VOLTAGE_DROP_976mV       122
#define VOLTAGE_DROP_984mV       123
#define VOLTAGE_DROP_992mV       124
#define VOLTAGE_DROP_1000mV       125
#define VOLTAGE_DROP_1008mV       126
#define VOLTAGE_DROP_1016mV       127
#define VOLTAGE_DROP_1024mV       128
#define VOLTAGE_DROP_1032mV       129
#define VOLTAGE_DROP_1040mV       130
#define VOLTAGE_DROP_1048mV       131
#define VOLTAGE_DROP_1056mV       132
#define VOLTAGE_DROP_1064mV       133
#define VOLTAGE_DROP_1072mV       134
#define VOLTAGE_DROP_1080mV       135
#define VOLTAGE_DROP_1088mV       136
#define VOLTAGE_DROP_1096mV       137
#define VOLTAGE_DROP_1104mV       138
#define VOLTAGE_DROP_1112mV       139
#define VOLTAGE_DROP_1120mV       140
#define VOLTAGE_DROP_1128mV       141
#define VOLTAGE_DROP_1136mV       142
#define VOLTAGE_DROP_1144mV       143
#define VOLTAGE_DROP_1152mV       144
#define VOLTAGE_DROP_1160mV       145
#define VOLTAGE_DROP_1168mV       146
#define VOLTAGE_DROP_1176mV       147
#define VOLTAGE_DROP_1184mV       148
#define VOLTAGE_DROP_1192mV       149
#define VOLTAGE_DROP_1200mV       150
#define VOLTAGE_DROP_1208mV       151
#define VOLTAGE_DROP_1216mV       152
#define VOLTAGE_DROP_1224mV       153
#define VOLTAGE_DROP_1232mV       154
#define VOLTAGE_DROP_1240mV       155
#define VOLTAGE_DROP_1248mV       156
#define VOLTAGE_DROP_1256mV       157
#define VOLTAGE_DROP_1264mV       158
#define VOLTAGE_DROP_1272mV       159
#define VOLTAGE_DROP_1280mV       160
#define VOLTAGE_DROP_1288mV       161
#define VOLTAGE_DROP_1296mV       162
#define VOLTAGE_DROP_1304mV       163
#define VOLTAGE_DROP_1312mV       164
#define VOLTAGE_DROP_1320mV       165
#define VOLTAGE_DROP_1328mV       166
#define VOLTAGE_DROP_1336mV       167
#define VOLTAGE_DROP_1344mV       168
#define VOLTAGE_DROP_1352mV       169
#define VOLTAGE_DROP_1360mV       170
#define VOLTAGE_DROP_1368mV       171
#define VOLTAGE_DROP_1376mV       172
#define VOLTAGE_DROP_1384mV       173
#define VOLTAGE_DROP_1392mV       174
#define VOLTAGE_DROP_1400mV       175
#define VOLTAGE_DROP_1408mV       176
#define VOLTAGE_DROP_1416mV       177
#define VOLTAGE_DROP_1424mV       178
#define VOLTAGE_DROP_1432mV       179
#define VOLTAGE_DROP_1440mV       180
#define VOLTAGE_DROP_1448mV       181
#define VOLTAGE_DROP_1456mV       182
#define VOLTAGE_DROP_1464mV       183
#define VOLTAGE_DROP_1472mV       184
#define VOLTAGE_DROP_1480mV       185
#define VOLTAGE_DROP_1488mV       186
#define VOLTAGE_DROP_1496mV       187
#define VOLTAGE_DROP_1504mV       188
#define VOLTAGE_DROP_1512mV       189
#define VOLTAGE_DROP_1520mV       190
#define VOLTAGE_DROP_1528mV       191
#define VOLTAGE_DROP_1536mV       192
#define VOLTAGE_DROP_1544mV       193
#define VOLTAGE_DROP_1552mV       194
#define VOLTAGE_DROP_1560mV       195
#define VOLTAGE_DROP_1568mV       196
#define VOLTAGE_DROP_1576mV       197
#define VOLTAGE_DROP_1584mV       198
#define VOLTAGE_DROP_1592mV       199
#define VOLTAGE_DROP_1600mV       200
#define VOLTAGE_DROP_1608mV       201
#define VOLTAGE_DROP_1616mV       202
#define VOLTAGE_DROP_1624mV       203
#define VOLTAGE_DROP_1632mV       204
#define VOLTAGE_DROP_1640mV       205
#define VOLTAGE_DROP_1648mV       206
#define VOLTAGE_DROP_1656mV       207
#define VOLTAGE_DROP_1664mV       208
#define VOLTAGE_DROP_1672mV       209
#define VOLTAGE_DROP_1680mV       210
#define VOLTAGE_DROP_1688mV       211
#define VOLTAGE_DROP_1696mV       212
#define VOLTAGE_DROP_1704mV       213
#define VOLTAGE_DROP_1712mV       214
#define VOLTAGE_DROP_1720mV       215
#define VOLTAGE_DROP_1728mV       216
#define VOLTAGE_DROP_1736mV       217
#define VOLTAGE_DROP_1744mV       218
#define VOLTAGE_DROP_1752mV       219
#define VOLTAGE_DROP_1760mV       220
#define VOLTAGE_DROP_1768mV       221
#define VOLTAGE_DROP_1776mV       222
#define VOLTAGE_DROP_1784mV       223
#define VOLTAGE_DROP_1792mV       224
#define VOLTAGE_DROP_1800mV       225
#define VOLTAGE_DROP_1808mV       226
#define VOLTAGE_DROP_1816mV       227
#define VOLTAGE_DROP_1824mV       228
#define VOLTAGE_DROP_1832mV       229
#define VOLTAGE_DROP_1840mV       230
#define VOLTAGE_DROP_1848mV       231
#define VOLTAGE_DROP_1856mV       232
#define VOLTAGE_DROP_1864mV       233
#define VOLTAGE_DROP_1872mV       234
#define VOLTAGE_DROP_1880mV       235
#define VOLTAGE_DROP_1888mV       236
#define VOLTAGE_DROP_1896mV       237
#define VOLTAGE_DROP_1904mV       238
#define VOLTAGE_DROP_1912mV       239
#define VOLTAGE_DROP_1920mV       240
#define VOLTAGE_DROP_1928mV       241
#define VOLTAGE_DROP_1936mV       242
#define VOLTAGE_DROP_1944mV       243
#define VOLTAGE_DROP_1952mV       244
#define VOLTAGE_DROP_1960mV       245
#define VOLTAGE_DROP_1968mV       246
#define VOLTAGE_DROP_1976mV       247
#define VOLTAGE_DROP_1984mV       248
#define VOLTAGE_DROP_1992mV       249
#define VOLTAGE_DROP_2000mV       250
#endif /*US*/

#ifdef EU
#define VOLTAGE_DROP_0mV       0
#define VOLTAGE_DROP_8mV       1
#define VOLTAGE_DROP_16mV       1
#define VOLTAGE_DROP_24mV       2
#define VOLTAGE_DROP_32mV       2
#define VOLTAGE_DROP_40mV       3
#define VOLTAGE_DROP_48mV       4
#define VOLTAGE_DROP_56mV       4
#define VOLTAGE_DROP_64mV       5
#define VOLTAGE_DROP_72mV       5
#define VOLTAGE_DROP_80mV       6
#define VOLTAGE_DROP_88mV       7
#define VOLTAGE_DROP_96mV       7
#define VOLTAGE_DROP_104mV       8
#define VOLTAGE_DROP_112mV       8
#define VOLTAGE_DROP_120mV       9
#define VOLTAGE_DROP_128mV       10
#define VOLTAGE_DROP_136mV       10
#define VOLTAGE_DROP_144mV       11
#define VOLTAGE_DROP_152mV       11
#define VOLTAGE_DROP_160mV       12
#define VOLTAGE_DROP_168mV       13
#define VOLTAGE_DROP_176mV       13
#define VOLTAGE_DROP_184mV       14
#define VOLTAGE_DROP_192mV       14
#define VOLTAGE_DROP_200mV       15
#define VOLTAGE_DROP_208mV       16
#define VOLTAGE_DROP_216mV       16
#define VOLTAGE_DROP_224mV       17
#define VOLTAGE_DROP_232mV       17
#define VOLTAGE_DROP_240mV       18
#define VOLTAGE_DROP_248mV       19
#define VOLTAGE_DROP_256mV       19
#define VOLTAGE_DROP_264mV       20
#define VOLTAGE_DROP_272mV       20
#define VOLTAGE_DROP_280mV       21
#define VOLTAGE_DROP_288mV       22
#define VOLTAGE_DROP_296mV       22
#define VOLTAGE_DROP_304mV       23
#define VOLTAGE_DROP_312mV       23
#define VOLTAGE_DROP_320mV       24
#define VOLTAGE_DROP_328mV       25
#define VOLTAGE_DROP_336mV       25
#define VOLTAGE_DROP_344mV       26
#define VOLTAGE_DROP_352mV       26
#define VOLTAGE_DROP_360mV       27
#define VOLTAGE_DROP_368mV       28
#define VOLTAGE_DROP_376mV       28
#define VOLTAGE_DROP_384mV       29
#define VOLTAGE_DROP_392mV       29
#define VOLTAGE_DROP_400mV       30
#define VOLTAGE_DROP_408mV       31
#define VOLTAGE_DROP_416mV       31
#define VOLTAGE_DROP_424mV       32
#define VOLTAGE_DROP_432mV       32
#define VOLTAGE_DROP_440mV       33
#define VOLTAGE_DROP_448mV       34
#define VOLTAGE_DROP_456mV       34
#define VOLTAGE_DROP_464mV       35
#define VOLTAGE_DROP_472mV       35
#define VOLTAGE_DROP_480mV       36
#define VOLTAGE_DROP_488mV       37
#define VOLTAGE_DROP_496mV       37
#define VOLTAGE_DROP_504mV       38
#define VOLTAGE_DROP_512mV       38
#define VOLTAGE_DROP_520mV       39
#define VOLTAGE_DROP_528mV       40
#define VOLTAGE_DROP_536mV       40
#define VOLTAGE_DROP_544mV       41
#define VOLTAGE_DROP_552mV       41
#define VOLTAGE_DROP_560mV       42
#define VOLTAGE_DROP_568mV       43
#define VOLTAGE_DROP_576mV       43
#define VOLTAGE_DROP_584mV       44
#define VOLTAGE_DROP_592mV       44
#define VOLTAGE_DROP_600mV       45
#define VOLTAGE_DROP_608mV       46
#define VOLTAGE_DROP_616mV       46
#define VOLTAGE_DROP_624mV       47
#define VOLTAGE_DROP_632mV       47
#define VOLTAGE_DROP_640mV       48
#define VOLTAGE_DROP_648mV       49
#define VOLTAGE_DROP_656mV       49
#define VOLTAGE_DROP_664mV       50
#define VOLTAGE_DROP_672mV       50
#define VOLTAGE_DROP_680mV       51
#define VOLTAGE_DROP_688mV       52
#define VOLTAGE_DROP_696mV       52
#define VOLTAGE_DROP_704mV       53
#define VOLTAGE_DROP_712mV       53
#define VOLTAGE_DROP_720mV       54
#define VOLTAGE_DROP_728mV       55
#define VOLTAGE_DROP_736mV       55
#define VOLTAGE_DROP_744mV       56
#define VOLTAGE_DROP_752mV       56
#define VOLTAGE_DROP_760mV       57
#define VOLTAGE_DROP_768mV       58
#define VOLTAGE_DROP_776mV       58
#define VOLTAGE_DROP_784mV       59
#define VOLTAGE_DROP_792mV       59
#define VOLTAGE_DROP_800mV       60
#define VOLTAGE_DROP_808mV       61
#define VOLTAGE_DROP_816mV       61
#define VOLTAGE_DROP_824mV       62
#define VOLTAGE_DROP_832mV       62
#define VOLTAGE_DROP_840mV       63
#define VOLTAGE_DROP_848mV       64
#define VOLTAGE_DROP_856mV       64
#define VOLTAGE_DROP_864mV       65
#define VOLTAGE_DROP_872mV       65
#define VOLTAGE_DROP_880mV       66
#define VOLTAGE_DROP_888mV       67
#define VOLTAGE_DROP_896mV       67
#define VOLTAGE_DROP_904mV       68
#define VOLTAGE_DROP_912mV       68
#define VOLTAGE_DROP_920mV       69
#define VOLTAGE_DROP_928mV       70
#define VOLTAGE_DROP_936mV       70
#define VOLTAGE_DROP_944mV       71
#define VOLTAGE_DROP_952mV       71
#define VOLTAGE_DROP_960mV       72
#define VOLTAGE_DROP_968mV       73
#define VOLTAGE_DROP_976mV       73
#define VOLTAGE_DROP_984mV       74
#define VOLTAGE_DROP_992mV       74
#define VOLTAGE_DROP_1000mV       75
#define VOLTAGE_DROP_1008mV       76
#define VOLTAGE_DROP_1016mV       76
#define VOLTAGE_DROP_1024mV       77
#define VOLTAGE_DROP_1032mV       77
#define VOLTAGE_DROP_1040mV       78
#define VOLTAGE_DROP_1048mV       79
#define VOLTAGE_DROP_1056mV       79
#define VOLTAGE_DROP_1064mV       80
#define VOLTAGE_DROP_1072mV       80
#define VOLTAGE_DROP_1080mV       81
#define VOLTAGE_DROP_1088mV       82
#define VOLTAGE_DROP_1096mV       82
#define VOLTAGE_DROP_1104mV       83
#define VOLTAGE_DROP_1112mV       83
#define VOLTAGE_DROP_1120mV       84
#define VOLTAGE_DROP_1128mV       85
#define VOLTAGE_DROP_1136mV       85
#define VOLTAGE_DROP_1144mV       86
#define VOLTAGE_DROP_1152mV       86
#define VOLTAGE_DROP_1160mV       87
#define VOLTAGE_DROP_1168mV       88
#define VOLTAGE_DROP_1176mV       88
#define VOLTAGE_DROP_1184mV       89
#define VOLTAGE_DROP_1192mV       89
#define VOLTAGE_DROP_1200mV       90
#define VOLTAGE_DROP_1208mV       91
#define VOLTAGE_DROP_1216mV       91
#define VOLTAGE_DROP_1224mV       92
#define VOLTAGE_DROP_1232mV       92
#define VOLTAGE_DROP_1240mV       93
#define VOLTAGE_DROP_1248mV       94
#define VOLTAGE_DROP_1256mV       94
#define VOLTAGE_DROP_1264mV       95
#define VOLTAGE_DROP_1272mV       95
#define VOLTAGE_DROP_1280mV       96
#define VOLTAGE_DROP_1288mV       97
#define VOLTAGE_DROP_1296mV       97
#define VOLTAGE_DROP_1304mV       98
#define VOLTAGE_DROP_1312mV       98
#define VOLTAGE_DROP_1320mV       99
#define VOLTAGE_DROP_1328mV       100
#define VOLTAGE_DROP_1336mV       100
#define VOLTAGE_DROP_1344mV       101
#define VOLTAGE_DROP_1352mV       101
#define VOLTAGE_DROP_1360mV       102
#define VOLTAGE_DROP_1368mV       103
#define VOLTAGE_DROP_1376mV       103
#define VOLTAGE_DROP_1384mV       104
#define VOLTAGE_DROP_1392mV       104
#define VOLTAGE_DROP_1400mV       105
#define VOLTAGE_DROP_1408mV       106
#define VOLTAGE_DROP_1416mV       106
#define VOLTAGE_DROP_1424mV       107
#define VOLTAGE_DROP_1432mV       107
#define VOLTAGE_DROP_1440mV       108
#define VOLTAGE_DROP_1448mV       109
#define VOLTAGE_DROP_1456mV       109
#define VOLTAGE_DROP_1464mV       110
#define VOLTAGE_DROP_1472mV       110
#define VOLTAGE_DROP_1480mV       111
#define VOLTAGE_DROP_1488mV       112
#define VOLTAGE_DROP_1496mV       112
#define VOLTAGE_DROP_1504mV       113
#define VOLTAGE_DROP_1512mV       113
#define VOLTAGE_DROP_1520mV       114
#define VOLTAGE_DROP_1528mV       115
#define VOLTAGE_DROP_1536mV       115
#define VOLTAGE_DROP_1544mV       116
#define VOLTAGE_DROP_1552mV       116
#define VOLTAGE_DROP_1560mV       117
#define VOLTAGE_DROP_1568mV       118
#define VOLTAGE_DROP_1576mV       118
#define VOLTAGE_DROP_1584mV       119
#define VOLTAGE_DROP_1592mV       119
#define VOLTAGE_DROP_1600mV       120
#define VOLTAGE_DROP_1608mV       121
#define VOLTAGE_DROP_1616mV       121
#define VOLTAGE_DROP_1624mV       122
#define VOLTAGE_DROP_1632mV       122
#define VOLTAGE_DROP_1640mV       123
#define VOLTAGE_DROP_1648mV       124
#define VOLTAGE_DROP_1656mV       124
#define VOLTAGE_DROP_1664mV       125
#define VOLTAGE_DROP_1672mV       125
#define VOLTAGE_DROP_1680mV       126
#define VOLTAGE_DROP_1688mV       127
#define VOLTAGE_DROP_1696mV       127
#define VOLTAGE_DROP_1704mV       128
#define VOLTAGE_DROP_1712mV       128
#define VOLTAGE_DROP_1720mV       129
#define VOLTAGE_DROP_1728mV       130
#define VOLTAGE_DROP_1736mV       130
#define VOLTAGE_DROP_1744mV       131
#define VOLTAGE_DROP_1752mV       131
#define VOLTAGE_DROP_1760mV       132
#define VOLTAGE_DROP_1768mV       133
#define VOLTAGE_DROP_1776mV       133
#define VOLTAGE_DROP_1784mV       134
#define VOLTAGE_DROP_1792mV       134
#define VOLTAGE_DROP_1800mV       135
#define VOLTAGE_DROP_1808mV       136
#define VOLTAGE_DROP_1816mV       136
#define VOLTAGE_DROP_1824mV       137
#define VOLTAGE_DROP_1832mV       137
#define VOLTAGE_DROP_1840mV       138
#define VOLTAGE_DROP_1848mV       139
#define VOLTAGE_DROP_1856mV       139
#define VOLTAGE_DROP_1864mV       140
#define VOLTAGE_DROP_1872mV       140
#define VOLTAGE_DROP_1880mV       141
#define VOLTAGE_DROP_1888mV       142
#define VOLTAGE_DROP_1896mV       142
#define VOLTAGE_DROP_1904mV       143
#define VOLTAGE_DROP_1912mV       143
#define VOLTAGE_DROP_1920mV       144
#define VOLTAGE_DROP_1928mV       145
#define VOLTAGE_DROP_1936mV       145
#define VOLTAGE_DROP_1944mV       146
#define VOLTAGE_DROP_1952mV       146
#define VOLTAGE_DROP_1960mV       147
#define VOLTAGE_DROP_1968mV       148
#define VOLTAGE_DROP_1976mV       148
#define VOLTAGE_DROP_1984mV       149
#define VOLTAGE_DROP_1992mV       149
#define VOLTAGE_DROP_2000mV       150
#endif /*EU*/


/*These constants define Triac signal minimum pulse width*/
#define PULSE_WIDTH_64_US       1
#define PULSE_WIDTH_128_US      2
#define PULSE_WIDTH_192_US      3
#define PULSE_WIDTH_256_US      4
#define PULSE_WIDTH_320_US      5
#define PULSE_WIDTH_384_US      6
#define PULSE_WIDTH_488_US      7
#define PULSE_WIDTH_512_US      8

#define ZW_TRIAC_INIT(mainsfreq,VOLTAGE_DROP,PULSE_WIDTH)              TRIAC_Init(TRIAC_HALFBRIDGE,mainsfreq,VOLTAGE_DROP,PULSE_WIDTH)
#define ZW_TRIAC_INIT_2_WIRE(mainsfreq,VOLTAGE_DROP,PULSE_WIDTH)       TRIAC_Init(TRIAC_FULLBRIDGE,mainsfreq,VOLTAGE_DROP,PULSE_WIDTH)
/*===============================   TRIAC_Interrupt_Enable   ==============================
**  Enable/Disable the the TRIAC interrupt
**  The triac interrupt fireds when the zero corss is detected.
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_TRIAC_INTERRUPT_ENABLE(ENABLE)   TRIAC_Interrupt_Enable(ENABLE)

/*=============================   TRIAC_IntFlagClr  ========================
** Clear the TRAIC interrupt flag
**--------------------------------------------------------------------------*/
#define ZW_TRIAC_INT_FLAG_CLR  TRIAC_IntFlagClr()

/*===============================   ADC_IsFired   ===========================
**  Check if ZERO cross is detected
**--------------------------------------------------------------------------*/
#define ZW_TRIAC_IS_FIRED  TRIAC_IsFired()

#endif

#ifdef ZW010x
#define ZW_TRIAC_INIT(mainsfreq)              TRIAC_Init(TRIAC_3_WIRE,mainsfreq)
#define ZW_TRIAC_INIT_2_WIRE(mainsfreq)       TRIAC_Init(TRIAC_2_WIRE,mainsfreq)
#endif

#define ZW_TRIAC_DIM_SET_LEVEL(dimLevel)      TRIAC_SetDimLevel(dimLevel)
#define ZW_TRIAC_LIGHT_SET_LEVEL(lightLevel)  TRIAC_SetDimLevel(100 - lightLevel)
#define ZW_TRIAC_OFF                          TRIAC_Off()



/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/


/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/


#ifdef ZW010x
/*=============================   TRIAC_Init   ==============================
**  Initializes the ZW0101/2 ASIC's integrated TRIAC.
**  Sets up the TRIAC control registers according to the mains frequency
**  which is 60 Hz in US
**
**--------------------------------------------------------------------------*/
void              /*RET Nothing */
TRIAC_Init(
  BYTE wiringType,
  BYTE mainsfreq);   /* IN mains frequency */
#endif

#if defined(ZW020x) || defined(ZW030x)
/*===============================   TRIAC_Interrupt_Enable   ==============================
**  Enable/Disable the the TRIAC interrupt
**  The triac interrupt fireds when the zero corss is detected.
**    Side effects:
**
**--------------------------------------------------------------------------*/
void
TRIAC_Interrupt_Enable(BOOL enable);/*IN TRUE if to enable the triac interrupt , FALSE to disable the interrupt*/

/*=============================   TRIAC_IntFlagClr  ========================
** Clear the TRAIC interrupt flag
**--------------------------------------------------------------------------*/
void TRIAC_IntFlagClr();

/*===============================   ADC_IsFired   ===========================
**  Check if ZERO cross is detected
**--------------------------------------------------------------------------*/
BOOL TRIAC_IsFired();

/*=============================   TRIAC_Init   ==============================
**  Initializes the ZW0201 ASIC's integrated TRIAC.
**  Sets up the TRIAC control registers according to the mains frequency
**  which is 60 Hz in US
**
**--------------------------------------------------------------------------*/
void              /*RET Nothing */
TRIAC_Init(
  BYTE bridgeType,  /*IN Triac diode bridge type*/
  BYTE mainsfreq,   /*IN mains frequency */
  BYTE VoltageDrop, /*IN The voltage drop across the ZEROX input*/
  BYTE miniumPulse   /*IN the minumum pulse width of the triac signal*/
  );
#endif


/*==========================   TRIAC_SetDimLevel  ============================
**  Sets the TRIAC up for dimLevel dim
**  according to mains frequency selected by calling TRIAC_Init
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
void                  /*RET Nothing */
TRIAC_SetDimLevel(
BYTE dimLevel);	      /* IN dim level 0-100 */


/*===============================   TRIAC_Off   ==============================
**  Turns the TRIAC controller OFF
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern void      		/*RET Nothing */
TRIAC_Off( void );  /* IN Nothing */

#endif /* _ZW_TRIAC_API_H_ */
