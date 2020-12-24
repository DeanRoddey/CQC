/******************************  ZW_adcdriv_api.h  *******************************
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
 *-----------------------------------------------------------------------------
 *
 * Description: Headerfile for interfacing to the Analog to digital converter.
 *
 * Author:Samer Seoud
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.21 $
 * Last Changed:     $Date: 2007/01/30 10:15:44 $
 *
 *****************************************************************************/
#ifndef _ZW_ADCDRIV_API_H_
#define _ZW_ADCDRIV_API_H_

/***************************************************************/
/* DEFINES                                                     */
/***************************************************************/
#define ADC_SINGLE_MODE       0x01
#define ADC_MULTI_CON_MODE    0x02

#ifdef ZW010x
#define ADC_MULTI_STP_MODE    0x03
#define ADC_MULTI_RST_MODE    0x04

#define ADC_REF_VDD           0x05
#define ADC_REF_V125          0x06
#endif


/***************************************************************/
/* ZW0201: P0.0/ZEROX                                          */
/***************************************************************/
#define ADC_PIN_1             0x01

/***************************************************************/
/* ZW0201: P0.1/TRIAC                                          */
/***************************************************************/
#define ADC_PIN_2             0x02
#if defined(ZW020x) || defined(ZW030x)

/***************************************************************/
/* ZW0201: P1.0/TXD                                            */
/***************************************************************/
#define ADC_PIN_3             0x04
/***************************************************************/
/* ZW0201: P1.1/RXD                                            */
/***************************************************************/
#define ADC_PIN_4             0x08

#define ADC_PIN_BATT           0x00

#define ADC_REF_U_EXT          0x01
#define ADC_REF_U_BGAB         0x02
#define ADC_REF_U_VDD          0x04

#define ADC_REF_L_EXT          0x10
#define ADC_REF_L_VSS          0x20

#define ADC_8_BIT             0x01
#define ADC_12_BIT            0x02

#define ADC_AZPL_128          0x00
#define ADC_AZPL_256          0x01
#define ADC_AZPL_512          0x02
#define ADC_AZPL_1024         0x03

#define ADC_THRES_LOWER       0x01
#define ADC_THRES_UPPER       0x02

#endif /*ZW020x,ZW030x*/



#define ADC_NOT_FINISHED  0xFFFF

/*===============================   ADC_Start   =============================
**  start the ADC unit.
**  Useful when running in continues mode.
**--------------------------------------------------------------------------*/
#define ZW_ADC_START                 ADC_Start()


/*===============================   ADC_Stop   =============================
**  Stop the  ADC unit.
**  Useful when running in continues mode.
**--------------------------------------------------------------------------*/
#define ZW_ADC_STOP                  ADC_Stop()

/*===============================   ADC_Off   ==============================
**  Power off the ADC unit.
**--------------------------------------------------------------------------*/
#define ZW_ADC_OFF                   ADC_Off()

#ifdef ZW010x
/*===============================   ADC_On ==================================
**  Power on the ADC unit in the ZW010x ASIC.
**  Conversion can be start 5 us after powering on when using the VDD volatage
** as a reference and 100 us when using the 1.25 v ref.
**--------------------------------------------------------------------------*/
#define ZW_ADC_ON                    ADC_On()

/*=======================   ZW_ADC_SET_THRESHOLD  ===========================
**  Set the threshold comparsion value of the ADC.
** Possible values are: 0 to 255
**--------------------------------------------------------------------------*/
#endif
#if defined(ZW020x) || defined(ZW030x)
/*=======================   ZW_ADC_SET_THRESHOLD  ===========================
**  Set the threshold comparsion value of the ADC.
** Possible values are: 0 to 0FFF
**--------------------------------------------------------------------------*/
#endif

#define ZW_ADC_SET_THRESHOLD(THRES)  (ADC_SetThres(THRES))

/*=======================   ZW_ADC_SELECT_ADx  ==============================
**  Selects the ADC input to use: x can be 1 or 2
**
**--------------------------------------------------------------------------*/
#define ZW_ADC_SELECT_AD1            (ADC_SelectPin(ADC_PIN_1))
#define ZW_ADC_SELECT_AD2            (ADC_SelectPin(ADC_PIN_2))

/*===============================   ZW_ADC_GET_READING ======================
**  get the value of an ADC conversion.
**--------------------------------------------------------------------------*/
#define ZW_ADC_GET_READING           (ADC_GetRes())

/*===============================   ZW_ADC_INT_ENABLE =======================
**  Enable ADC interrupt
**--------------------------------------------------------------------------*/
#define ZW_ADC_INT_ENABLE            (ADC_Int(TRUE))

/*===============================   ZW_ADC_INT_ENABLE =======================
**  Disable ADC interrupt
**--------------------------------------------------------------------------*/
#define ZW_ADC_INT_DISABLE           (ADC_Int(FALSE))

/*===============================   ZW_ADC_CLR_FLAG ==========================
**  Clear the interrupt flag
**--------------------------------------------------------------------------*/
#define ZW_ADC_CLR_FLAG              (ADC_IntFlagClr())
#ifdef ZW010x
/*============================  ZW_ADC_SAMPLE_RATE ==========================
** Set the sample rate of the ADC converter. The sample rate depeneds of the
** CPU freqeuncy The sample rate is calculated by CPU_FREQ/(176*rate)
** IMPORTANT: The maximum sample rate must not exeed 22.7kHz
** Valid values for input is 1 trough 64 (Verify that sample rate is below the
** above mentioned limit)
**--------------------------------------------------------------------------*/
#define ZW_ADC_SAMPLE_RATE(RATE)     (ADC_SetSamplingRate(RATE))
#endif
#ifdef ZW010x
/*===============================   ZW_ADC_INIT ==============================
** Initialize the ADC unit to work in specific configuration
         BYTE mode - IN  Mode of the ADC: ADC_SINGLE_MODE
                                          ADC_MULTI_CON_MODE
                                          ADC_MULTI_STP_MODE
                                          ADC_MULTI_RST_MODE
         BYTE ref - IN  Which type of voltage reference to use: ADC_REF_VDD
                                                                ADC_REF_V125
         BYTE pin - IN  Which pin to sample:  ADC_PIN_1
                                              ADC_PIN_2
**--------------------------------------------------------------------------*/
#define ZW_ADC_INIT(MODE,REFRENCE,INPUT)   (ADC_Init(MODE,REFRENCE,INPUT))
#endif

#if defined(ZW020x) || defined(ZW030x)
/*===============================   ZW_ADC_INIT ====================================================
** Initialize the ADC unit to work in specific configuration
         BYTE mode - IN  Mode of the ADC: ADC_SINGLE_MODE
                                          ADC_MULTI_CON_MODE

         BYTE upper_ref - IN  Which type of upper voltage reference to use: ADC_REF_U_EXT
                                                                            ADC_REF_U_BGAB
                                                                            ADC_REF_U_VDD

         BYTE lower_ref - IN  Which type of lower voltage reference to use: ADC_REF_L_EXT
                                                                            ADC_REF_L_VSS
         BYTE pin - IN  Which pin to sample:  ADC_PIN_1
                                              ADC_PIN_2
                                              ADC_PIN_2
                                              ADC_PIN_3
                                              ADC_PIN_BATT
**-------------------------------------------------------------------------------------------------------*/
#define ZW_ADC_INIT(MODE,UPPER_REF,LOWER_REF,INPUT)   (ADC_Init(MODE,UPPER_REF,LOWER_REF,INPUT))


/*===============================   ZW_ADC_IS_FIRED   ===========================
**  Check if the ADC conversion crossed over the threshold value.
**  RET TRUE if ADC conversion value crossed over thresold, else FALSE
**--------------------------------------------------------------------------*/
#define ZW_ADC_IS_FIRED()   (ADC_IsFired())

/*============================ ZW_ADC_RESOLUTION_8  ===========================
** Set the resolution of the ADC to 8 bit.
-------------------------------------------------------------------------*/
#define ZW_ADC_RESOLUTION_8    ADC_SetResolution(ADC_8_BIT)

/*============================ ZW_ADC_RESOLUTION_12  ===========================
** Set the resolution of the ADC to 12 bit.
-------------------------------------------------------------------------*/
#define ZW_ADC_RESOLUTION_12    ADC_SetResolution(ADC_12_BIT)

/*================================== ZW_ADC_THRESHOLD_UP  ======================================
** Set the threshold mode so the ADC interrrupt fire when input is above or equal threshold value.
--------------------------------------------------------------------------------------------------*/
#define ZW_ADC_THRESHOLD_UP    ADC_SetThresMode(ADC_THRES_UPPER)

/*================================== ZW_ADC_THRESHOLD_LO  ======================================
** Set the threshold mode so the ADC interrrupt fire when input is below or equal threshold value.
--------------------------------------------------------------------------------------------------*/
#define ZW_ADC_THRESHOLD_LO    ADC_SetThresMode(ADC_THRES_LOWER)

/*================================== ZW_ADC_BUFFER_ENABLE  ======================================
** Enable the buffer between the analog inputs and the ADC converter.
--------------------------------------------------------------------------------------------------*/
#define ZW_ADC_BUFFER_ENABLE    ADC_Buf(TRUE)

/*================================== ZW_ADC_BUFFER_DISABLE  ======================================
** Disable the buffer between the analog inputs and the ADC converter.
--------------------------------------------------------------------------------------------------*/

#define ZW_ADC_BUFFER_DISABLE    ADC_Buf(FALSE)

/*================================= ZW_ADC_SET_AZPL =========================
** The the auto zero period length
-------------------------------------------------------------------------*/
#define ZW_ADC_SET_AZPL(PERIOD)     ADC_SetAZPL(PERIOD)

#endif

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/

/*===============================   ADC_Start   =============================
**  start the ADC unit.
**  Useful when running in continues mode.
**--------------------------------------------------------------------------*/
extern void ADC_Start(void);

/*===============================   ADC_Stop   =============================
**  Stop the  ADC unit.
**  Useful when running in continues mode.
**--------------------------------------------------------------------------*/
extern void ADC_Stop(void);
#ifdef ZW010x
/*===============================   ADC_On   =============================
**  Power on the ADC unit in the ZW010x ASIC.
**  Conversion can be start 5 us after powering on when using the VDD volatage
** as a reference and 100 us when using the 1.25 v ref.
**--------------------------------------------------------------------------*/
extern void ADC_On(void);
#endif
/*===============================   ADC_Off   =============================
**  Power off the ADC unit.
**--------------------------------------------------------------------------*/
extern void ADC_Off(void);

/*===============================   ADC_SelectPin   ======================
**  Select the input pin for the ADC.
**--------------------------------------------------------------------------*/
extern void ADC_SelectPin(BYTE adcPin);

/*===========================   ADC_SetThreshold  ===========================
**  Set the threshold comparsion value of the ADC.
**--------------------------------------------------------------------------*/
#ifdef ZW010x
extern void ADC_SetThres(BYTE Threshold);
#endif
#if defined(ZW020x) || defined(ZW030x)
extern void ADC_SetThres(WORD Threshold);
#endif

/*=============================   ADC_Int  ==================================
** enable or disable the ADC interrupt
**--------------------------------------------------------------------------*/
extern void ADC_Int(BYTE enable);  // IN: true if interrupt should be enable else false

/*=============================   ADC_IntFlagClr  ========================
** enable or disable the ADC interrupt
**--------------------------------------------------------------------------*/
extern void ADC_IntFlagClr(void);
#ifdef ZW010x
/*=============================   ADC_SampleRate  ========================
** Set the sample rate of the ADC converter. The sample rate depened of the
** CPU freqeuncy The MINIMUM sample rate is CPU_FREQ/1024*11
** The maximum sample rate is CPU_FREQ/16*11 <= 250Khz/11.
**--------------------------------------------------------------------------*/
void ADC_SetSamplingRate(BYTE rate);
#endif
/*===============================   ADC_GetRes   ===========================
**  get the value of an ADC conversion.
**--------------------------------------------------------------------------*/
extern WORD           // RET: return the conversion result
ADC_GetRes(void);

#ifdef ZW010x
/*=============================   ADC_Init  ========================
** Initialize the ADC unit to work in specific configuration
**--------------------------------------------------------------------------*/
extern void                       /*RET  Nothing       */
ADC_Init(
         BYTE mode,                 /*IN  Mode of the ADC, single, continious ect. */
         BYTE ref,                  /*IN  Which type of voltage reference to use*/
         BYTE pin);                /*IN  Which pin to sample */
#endif
#if defined(ZW020x) || defined(ZW030x)
/*===============================   ADC_IsFired   ===========================
**  Check if the ADC conversion crossed over the threshold value.
**--------------------------------------------------------------------------*/
BOOL                          /* RET TRUE if ADC conversion value crossed over thresold, else FALSE*/
ADC_IsFired();

/*=============================   ADC_Init  ========================
** Initialize the ADC unit to work in specific configuration
**--------------------------------------------------------------------------*/
extern void                       /*RET  Nothing       */
ADC_Init(
         BYTE mode,                /*IN  Mode of the ADC, single, continious ect. */
         BYTE upper_ref,           /*IN  Which type of upper voltage reference to use*/
         BYTE lower_ref,           /*IN  Which type of lower voltage reference to use*/
         BYTE pin_en);                /*IN  Which pin(s) to enable */

/*============================ ADC_SetResolution  ===========================
** Set the resolution of the ADC
** Parameters valid values:
**                           ADC_8_BIT for 8 bit ADC resolution
**                           ADC_12_BIT for 12 bit ADC resoltuion
-------------------------------------------------------------------------*/

void
ADC_SetResolution(
                  BYTE reso ); /*IN Valid values :ADC_8_BIT or ADC_12_BIT*/

/*======================================= ADC_SetThresMode  ==============================================
** Set the threshold trigger type.
** Parameter valid values:
**   ADC_THRES_UPPER  the interrupt is released when converted value is above/equal to the threshold
**   ADC_THRES_LOWER  the interrupt is released when converted value is below/equal to the threshold
-------------------------------------------------------------------------------------------------------*/

void
ADC_SetThresMode(
                 BYTE thresMode); /*IN valid values: ADC_THRES_UPPER or ADC_THRES_LOWER*/


/*=============================== ADC_SetAZPL ===========================
** The the auto zero period length
-------------------------------------------------------------------------*/
void
ADC_SetAZPL(
            BYTE Azpl);  /* IN the ADC auto zero period length
                             Valid values: ADC_AZPL_128
                                            ADC_AZPL_256
                                            ADC_AZPL_512
                                            ADC_AZPL_1024*/

/*============================  ADC_Buf =============================================
** Switches a buffer on/off in between the analog input and the AD converter
------------------------------------------------------------------------------*/
void
ADC_Buf(
        BYTE Enable); /* IN TRUE: Switcht the buffer on, FALSE: switch the buffer off*/


#endif

#endif /* _ZW_ADCDRIV_API__H_ */
