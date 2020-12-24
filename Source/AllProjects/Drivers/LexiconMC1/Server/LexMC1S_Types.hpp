//
// FILE NAME: LexMC1CS_Types.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2000
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2020
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and 
//  the author (Dean Roddey.) It is licensed under the MIT Open Source 
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//  This is the types file for the Lexicon MC-1/DC-2 driver. Some are shared
//  with the client and come in from a client side header that we include.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tLexMC1S
{
    // -----------------------------------------------------------------------
    //  This enum represents the 'commands' that are passed via messages
    //  between us and the Lexicon. This one conglomerates the notifications
    //  it sends to us, the commands we send to it, and the replies it sends
    //  back.
    // -----------------------------------------------------------------------
    enum ECmds
    {
        // These are notifications from the Lexicon
        ECmd_None               = 0x00
        , ECmd_Wakeup           = 0x01
        , ECmd_Sleep            = 0x02
        , ECmd_FrontPan         = 0x03
        , ECmd_ParmChange       = 0x04

        // These are commands from us
        , ECmd_SoftReset        = 0x10
        , ECmd_HostWakeup       = 0x11
        , ECmd_HostSleep        = 0x12
        , ECmd_RestoreDefs      = 0x13
        , ECmd_SendIR           = 0x14
        , ECmd_GetSysConfig     = 0x15
        , ECmd_GetSysStatus     = 0x16
        , ECmd_GetZ2Status      = 0x17
        , ECmd_GetSysParmById   = 0x18
        , ECmd_GetSysParmByName = 0x19
        , ECmd_GetSysParmValues = 0x1A
        , ECmd_GetEffect        = 0x1B
        , ECmd_GetFxParmDef     = 0x1C
        , ECmd_GetFxParmVals    = 0x1D
        , ECmd_SetSysParmValues = 0x1E
        , ECmd_SetFxParmVals    = 0x1F
        , ECmd_SetFxNameById    = 0x20
        , ECmd_SetSysVolume     = 0x21
        , ECmd_SetSysBalance    = 0x22
        , ECmd_SetActiveEffect  = 0x24
        , ECmd_SetZ2Input       = 0x25
        , ECmd_ClearZ2Input     = 0x26
        , ECmd_SetZ2Volume      = 0x27
        , ECmd_SetZ2Balance     = 0x28
        , ECmd_GetFPDRegs       = 0x29
        , ECmd_SetFPDRegs       = 0x2A
        , ECmd_GetCustomName    = 0x2B
        , ECmd_SetCustomName    = 0x2C
        , ECmd_GetInputName     = 0x2D
        , ECmd_SetInputNameById = 0x2E
        , ECmd_GetComConfig     = 0x2F
        , ECmd_SetComConfig     = 0x30
        , ECmd_SetMute          = 0x31
        , ECmd_SetOutLevels     = 0x32
        , ECmd_SetDisplayStr    = 0x33

        // These are replies from the Lexicon to our commands
        , ECmds_RepFirst        = 0x80
        , ECmd_SysConfig        = 0x80
        , ECmd_SysStatus        = 0x81
        , ECmd_Z2Status         = 0x82
        , ECmd_SysParamDef      = 0x83
        , ECmd_SysParamVals     = 0x84
        , ECmd_EffectDef        = 0x85
        , ECmd_EffectParamDef   = 0x86
        , ECmd_EffectParamVals  = 0x87
        , ECmd_FPDStatus        = 0x88
        , ECmd_CustName         = 0x89
        , ECmd_InputName        = 0x8A
        , ECmd_PeekValue        = 0x8B
        , ECmd_ComConfig        = 0x8C

        // And the special ack/nack status replies
        , ECmd_Ack              = 0xE0
        , ECmd_Nack             = 0xE1
        , ECmds_RepLast         = 0xE1
    };


    // -----------------------------------------------------------------------
    //  The support effect types
    // -----------------------------------------------------------------------
    enum EEffects
    {
        EEffect_Unknown         = 0
        , EEffect_Church        = 1
        , EEffect_THXCinema     = 2
        , EEffect_TVMatrix      = 3
        , EEffect_MonoLogic     = 4
        , EEffect_Panorama      = 5
        , EEffect_NightClub     = 6
        , EEffect_ProLogic      = 7
        , EEffect_MusicLogic    = 8
        , EEffect_Party         = 9
        , EEffect_SurroundNoise = 10
        , EEffect_PanNoise      = 11
        , EEffect_ConcertHall   = 12
        , EEffect_Cathedral     = 13
        , EEffect_MusicSurround = 14
        , EEffect_Logic7        = 15
        , EEffect_2Channel      = 16
        , EEffect_2_0           = 17
        , EFffect_2_0_THX       = 18
        , EEffect_2_0_Logic7    = 19
        , EEffect_2_0_Music     = 20
        , EEffect_2_0_2Ch       = 21
        , EEffect_Dolby         = 22
        , EEffect_DolbyTHX      = 23
        , EEffect_DolbyLogic7   = 24
        , EEffect_DolbyMusic    = 25
        , EEffect_Dolby2Ch      = 26
        , EEffect_DTSFilm       = 27
        , EEffect_DTSTHX        = 28
        , EEffect_DTSLogic7     = 29
        , EEffect_DTSMusic      = 30
        , EEffect_DTS2Ch        = 31

        , EEffects_Count
        , EEffects_Min          = 0
        , EEffects_Max          = EEffects_Count - 1
        , EEffects_First        = 1
    };


    // -----------------------------------------------------------------------
    //  Error code constants. These are adjusted up by one from the real
    //  codes, in order to have a 'no error' value.
    // -----------------------------------------------------------------------
    enum EErrors
    {
        EErr_NoAck              = 0x00
        , EErr_NoError          = 0x01
        , EErr_Parity           = 0x02
        , EErr_Framing          = 0x03
        , EErr_Overrun          = 0x04
        , EErr_BadPacket        = 0x05
        , EErr_TimeOut          = 0x06
        , EErr_BufFull          = 0x07
        , EErr_BadCount         = 0x10
        , EErr_BadCmd           = 0x11
        , EErr_BadData          = 0x12
        , EErr_BadAddr          = 0x13
        , EErr_BadFxId          = 0x14
        , EErr_BadParmId        = 0x15
        , EErr_BadName          = 0x16
        , EErr_BadInput         = 0x17
    };


    // -----------------------------------------------------------------------
    //  The are the inputs supported by the device
    //
    //  NOTE:   The order is IMPORTANT, because they are set up to have the
    //          same values as the MC-1 input values.
    // -----------------------------------------------------------------------
    enum EInputs
    {
        EInput_Tape             = 0
        , EInput_Tuner          = 1
        , EInput_CD             = 2
        , EInput_Aux            = 3
        , EInput_TV             = 4
        , EInput_VDisc          = 5
        , EInput_DVD            = 6
        , EInput_VCR            = 7

        , EInputs_Count
        , EInputs_Min           = 0
        , EInputs_Max           = EInputs_Count - 1
    };


    // -----------------------------------------------------------------------
    //  The are the input formats supported by the device
    //
    //  NOTE:   The order is IMPORTANT, because they are set up to have the
    //          same values as the MC-1 input format values.
    // -----------------------------------------------------------------------
    enum EInputFmts
    {
        EInputFmt_Unknown
        , EInputFmt_AC3
        , EInputFmt_PCM
        , EInputFmt_Analog
        , EInputFmt_DTS
        , EInputFmt_AC3_20
        , EInputFmt_AC3_51

        , EInputFmts_Count
        , EInputFmts_Min        = EInputFmt_Unknown
        , EInputFmts_Max        = EInputFmt_AC3_51
    };


    // -----------------------------------------------------------------------
    //  The IR codes supported by the Lexicon interface
    // -----------------------------------------------------------------------
    enum EIRCodes
    {
        EIRCode_None            = 0x00
        , EIRCode_MenuUp        = 0x01
        , EIRCode_OSDOff        = 0x02
        , EIRCode_FPDOff        = 0x03
        , EIRCode_FPDOn         = 0x04
        , EIRCode_OSDOn         = 0x05
        , EIRCode_Select        = 0x08
        , EIRCode_Done          = 0x0A
        , EIRCode_Tape          = 0x0C
        , EIRCode_Tuner         = 0x0D
        , EIRCode_CD            = 0x0E
        , EIRCode_Aux           = 0x0F
        , EIRCode_TV            = 0x10
        , EIRCode_VDisc         = 0x11
        , EIRCode_DVD           = 0x12
        , EIRCode_VCR           = 0x13
        , EIRCode_Mute          = 0x15
        , EIRCode_VolumeDec     = 0x16
        , EIRCode_VolumeInc     = 0x17
        , EIRCode_PowerOn       = 0x18
        , EIRCode_PowerOff      = 0x19
        , EIRCode_EffectUp      = 0x1A
        , EIRCode_EffectDown    = 0x1B
        , EIRCode_MenuDown      = 0x1D
        , EIRCode_Dolby         = 0x20
        , EIRCode_THX           = 0x21
        , EIRCode_Logic7        = 0x22
        , EIRCode_DTS           = 0x23
        , EIRCode_2ChanToggle   = 0x24
        , EIRCode_Party         = 0x25
        , EIRCode_TVMatrix      = 0x26
        , EIRCode_Music         = 0x27

        , EIRCode_FullMute      = 0x95
        , EIRCode_TriggerOn     = 0x98
        , EIRCode_TriggerOff    = 0x99

        // <TBD> Add the rest later

        , EIRCodes_Count
        , EIRCodes_Min          = 0
        , EIRCodes_Max          = EIRCodes_Count - 1
    };


    // -----------------------------------------------------------------------
    //  This enum indicates the types of products supported by the serial
    //  protocl. We only support the MC-1/DC-2 products, but define the
    //  others so that we can report what we found on the other side in an
    //  error message.
    // -----------------------------------------------------------------------
    enum EProducts
    {
        EProduct_Unknown
        , EProduct_DC2
        , EProduct_MC1
        , EProduct_SDP_3

        , EProducts_Count
        , EProducts_Min         = 0
        , EProducts_Max         = EProducts_Count - 1
    };


    // -----------------------------------------------------------------------
    //  The sample rates supported
    // -----------------------------------------------------------------------
    enum ESampleRates
    {
        ESampleRate_Unknown     = 0
        , ESampleRate_44        = 1
        , ESampleRate_48        = 2
        , ESampleRate_88        = 3
        , ESampleRate_96        = 4

        , ESampleRates_Count
        , ESampleRates_Min      = 0
        , ESampleRates_Max      = ESampleRates_Count - 1
    };


    // -----------------------------------------------------------------------
    //  The 7 speakers possible in an MC-1 system.
    //
    //  NOTE:   The order is IMPORTANT, because they are set up to have the
    //          same values as the MC-1 speaker values.
    // -----------------------------------------------------------------------
    enum ESpeakers
    {
        ESpeaker_Center         = 0
        , ESpeaker_Sub          = 1
        , ESpeaker_LeftFront    = 2
        , ESpeaker_RightFront   = 3
        , ESpeaker_LeftSide     = 4
        , ESpeaker_RightSide    = 5
        , ESpeaker_LeftRear     = 6
        , ESpeaker_RightRear    = 7

        , ESpeakers_Count
        , ESpeakers_Min         = 0
        , ESpeakers_Max         = ESpeakers_Count - 1
    };


    // -----------------------------------------------------------------------
    //  Data stream types
    // -----------------------------------------------------------------------
    enum EStreamTypes
    {
        EStream_Unknown         = 0
        , EStream_AC3           = 1
        , EStream_PCM           = 2
        , EStream_Analog        = 3
        , EStream_DTS           = 4
        , EStream_AC3_20        = 5
        , EStream_AC3_51        = 6

        , EStreamTypes_Count
        , EStreamTypes_Min      = 0
        , EStreamTypes_Max      = EStreamTypes_Count - 1
    };


    // -----------------------------------------------------------------------
    //  Indicates the software level currently loaded by the device
    // -----------------------------------------------------------------------
    enum ESWLevels
    {
        ESWLevel_Released
        , ESWLevel_PreAlpha
        , ESWLevel_Alpha
        , ESWLevel_Beta
        , ESWLevel_Gamma
        , ESWLevel_Unsupported

        , ESWLevels_Count
        , ESWLevels_Min         = 0
        , ESWLevels_Max         = ESWLevels_Count - 1
    };


    // -----------------------------------------------------------------------
    //  Indicates the software type supported by the device
    // -----------------------------------------------------------------------
    enum ESWTypes
    {
        ESWType_Unknown
        , ESWType_THX
        , ESWType_AC3
        , ESWType_DTS

        , ESWTypes_Count
        , ESWTypes_Min          = 0
        , ESWTypes_Max          = ESWTypes_Count - 1
    };


    // -----------------------------------------------------------------------
    //  The system parameters that we support.
    // -----------------------------------------------------------------------
    enum ESysParms
    {
        ESysParm_Unknown            = 0
        , ESysParm_Program          = 1
        , ESysParm_Bypass           = 2
        , ESysParm_Mute             = 3
        , ESysParm_MuteMode         = 4
        , ESysParm_Volume           = 5
        , ESysParm_LRBalance        = 6
        , ESysParm_Input            = 7
        , ESysParm_AudioBit         = 8
        , ESysParm_SrcTape          = 9
        , ESysParm_SrcTuner         = 10
        , ESysParm_SrcCD            = 11
        , ESysParm_SrcAux           = 12
        , ESysParm_SrcTV            = 13
        , ESysParm_SrcVDisc         = 14
        , ESysParm_SrcVCR2          = 15
        , ESysParm_SrcVCR1          = 16
        , ESysParm_RecordSource     = 17
        , ESysParm_RecordEnabled    = 18
        , ESysParm_RecordModeTape   = 19
        , ESysParm_RecordModeTuner  = 20
        , ESysParm_RecordModeCD     = 21
        , ESysParm_RecordModeAux    = 22
        , ESysParm_RecordModeTV     = 23
        , ESysParm_RecordModeVDisc  = 24
        , ESysParm_RecordModeVCR2   = 25
        , ESysParm_RecordModeVCR1   = 26
        , ESysParm_VideoTape        = 27
        , ESysParm_VideoTuner       = 28
        , ESysParm_VideoCD          = 29
        , ESysParm_VideoAux         = 30
        , ESysParm_VideoTV          = 31
        , ESysParm_VideoVDisc       = 32
        , ESysParm_VideoVCR2        = 33
        , ESysParm_VideoVCR1        = 34
        , ESysParm_VideoRecord      = 35
        , ESysParm_ILvlTape         = 36
        , ESysParm_ILvlTuner        = 37
        , ESysParm_ILvlCD           = 38
        , ESysParm_ILvlAux          = 39
        , ESysParm_ILvlTV           = 40
        , ESysParm_ILvlVDisc        = 41
        , ESysParm_ILvlVCR2         = 42
        , ESysParm_ILvlVCR1         = 43
        , ESysParm_AutoLvlTape      = 44
        , ESysParm_AutoLvlTuner     = 45
        , ESysParm_AutoLvlCD        = 46
        , ESysParm_AutoLvlAux       = 47
        , ESysParm_AutoLvlTV        = 48
        , ESysParm_AutoLvlVDisc     = 49
        , ESysParm_AutoLvlVCR2      = 50
        , ESysParm_AutoLvlVCR1      = 51
        , ESysParm_FrontSpkr        = 52
        , ESysParm_FrontHPassSmall  = 53
        , ESysParm_FrontHPass       = 54
        , ESysParm_CenterSpkr       = 55
        , ESysParm_BSplitSmall      = 56
        , ESysParm_BSplit           = 57
        , ESysParm_SideSpkr         = 58
        , ESysParm_SideSTDHPass     = 59
        , ESysParm_SideDipoleHPass  = 60
        , ESysParm_SideHPass        = 61
        , ESysParm_RearSpkr         = 62
        , ESysParm_RearSmallHPass   = 63
        , ESysParm_RearHPass        = 64
        , ESysParm_SubWoofer        = 65
        , ESysParm_SubLowPass       = 66
        , ESysParm_OutAdjustCenter  = 67
        , ESysParm_OutAdjustSub     = 68
        , ESysParm_OutAdjustFrontL  = 69
        , ESysParm_OutAdjustFrontR  = 70
        , ESysParm_OutAdjustSideL   = 71
        , ESysParm_OutAdjustSideR   = 72
        , ESysParm_OutAdjustRearL   = 73
        , ESysParm_OutAdjustRearR   = 74
        , ESysParm_Angle            = 75
        , ESysParm_ListenerPos      = 76
        , ESysParm_SurroundDelay    = 77
        , ESysParm_CenterDelay      = 78
        , ESysParm_EffectTape       = 79
        , ESysParm_EffectTuner      = 80
        , ESysParm_EffectCD         = 81
        , ESysParm_EffectAux        = 82
        , ESysParm_EffectTV         = 83
        , ESysParm_EffectVDisc      = 84
        , ESysParm_EffectVCR2       = 85
        , ESysParm_EffectVCR1       = 86
        , ESysParm_AC3EffectTape    = 87
        , ESysParm_AC3EffectTuner   = 88
        , ESysParm_AC3EffectCD      = 89
        , ESysParm_AC3EffectAux     = 90
        , ESysParm_AC3EffectTV      = 91
        , ESysParm_AC3EffectVDisc   = 92
        , ESysParm_AC3EffectVCR2    = 93
        , ESysParm_AC3EffectVCR1    = 94
        , ESysParm_DTSEffectTape    = 95
        , ESysParm_DTSEffectTuner   = 96
        , ESysParm_DTSEffectCD      = 97
        , ESysParm_DTSEffectAux     = 98
        , ESysParm_DTSEffectTV      = 99
        , ESysParm_DTSEffectVDisc   = 100
        , ESysParm_DTSEffectVCR2    = 101
        , ESysParm_DTSEffectVCR1    = 102
        , ESysParm_LastEffectTape   = 103
        , ESysParm_LastEffectTuner  = 104
        , ESysParm_LastEffectCD     = 105
        , ESysParm_LastEffectAux    = 106
        , ESysParm_LastEffectTV     = 107
        , ESysParm_LastEffectVDisc  = 108
        , ESysParm_LastEffectVCR2   = 109
        , ESysParm_LastEffectVCR1   = 110
        , ESysParm_LastAC3Tape      = 111
        , ESysParm_LastAC3Tuner     = 112
        , ESysParm_LastAC3CD        = 113
        , ESysParm_LastAC3Aux       = 114
        , ESysParm_LastAC3TV        = 115
        , ESysParm_LastAC3VDisc     = 116
        , ESysParm_LastAC3VCR2      = 117
        , ESysParm_LastAC3VCR1      = 118
        , ESysParm_LastDTSTape      = 119
        , ESysParm_LastDTSTuner     = 120
        , ESysParm_LastDTSCD        = 121
        , ESysParm_LastDTSAux       = 122
        , ESysParm_LastDTSTV        = 123
        , ESysParm_LastDTSVDisc     = 124
        , ESysParm_LastDTSVCR2      = 125
        , ESysParm_LastDTSVCR1      = 126
        , ESysParm_LastDigTape      = 127
        , ESysParm_LastDigTuner     = 128
        , ESysParm_LastDigCD        = 129
        , ESysParm_LastDigAux       = 130
        , ESysParm_LastDigTV        = 131
        , ESysParm_LastDigVDisc     = 132
        , ESysParm_LastDigVCR2      = 133
        , ESysParm_LastDigVCR1      = 134
        , ESysParm_InFormatTape     = 135
        , ESysParm_InFormatTuner    = 136
        , ESysParm_InFormatCD       = 137
        , ESysParm_InFormatAux      = 138
        , ESysParm_InFormatTV       = 139
        , ESysParm_InFormatVDisc    = 140
        , ESysParm_InFormatVCR2     = 141
        , ESysParm_InFormatVCR1     = 142
        , ESysParm_TriggerTape      = 143
        , ESysParm_TriggerTuner     = 144
        , ESysParm_TriggerCD        = 145
        , ESysParm_TriggerAux       = 146
        , ESysParm_TriggerTV        = 147
        , ESysParm_TriggerVDisc     = 148
        , ESysParm_TriggerVCR2      = 149
        , ESysParm_TriggerVCR1      = 150
        , ESysParm_BypassPgm        = 151
        , ESysParm_Lock             = 152
        , ESysParm_LockAccess       = 153
        , ESysParm_Z2Vol            = 154
        , ESysParm_Z2PwrOnVolume    = 155
        , ESysParm_Z2Balance        = 156
        , ESysParm_Z2Mute           = 157
        , ESysParm_VidioType        = 158
        , ESysParm_OSDColor         = 159
        , ESysParm_OSDPos           = 160
        , ESysParm_DisplayTimeout   = 161
        , ESysParm_LCDEnable        = 162
        , ESysParm_LCDContrast      = 163
        , ESysParm_FIPBright        = 164
        , ESysParm_CustomName       = 165
        , ESysParm_PwrOnVolume      = 166
        , ESysParm_Bass             = 167
        , ESysParm_Treble           = 168
        , ESysParm_Loundness        = 169
        , ESysParm_Limiter          = 170
        , ESysParm_LimiterVolume    = 171
        , ESysParm_LimiterOn        = 172
        , ESysParm_EffectOnly       = 173
        , ESysParm_Tilt             = 174
        , ESysParm_RMNotch          = 175
        , ESysParm_ReEqualization   = 176
        , ESysParm_CenterSpeech     = 177
        , ESysParm_DolbySR          = 178
        , ESysParm_Timbre           = 179
        , ESysParm_SpkrRange        = 180
        , ESysParm_TAlignCenter     = 181
        , ESysParm_TAlignSub        = 182
        , ESysParm_TAlignFrontL     = 183
        , ESysParm_TAlignFrontR     = 184
        , ESysParm_TAlignSideL      = 185
        , ESysParm_TAlignSideR      = 186
        , ESysParm_TAlignRearL      = 187
        , ESysParm_TAlignRearR      = 188
        , ESysParm_AVSyncDelay      = 189
        , ESysParm_MenuBkGround     = 190
        , ESysParm_CustomParam      = 191

        , ESysParms_Count
        , ESysParms_Unknown
        , ESysParms_Min         = ESysParm_Program
        , ESysParms_Max         = ESysParm_CustomParam
    };




}


// ---------------------------------------------------------------------------
//  Most of these are done in the common header, but these we have to do for
//  ourself.
// ---------------------------------------------------------------------------
StdEnumTricks(tLexMC1S::EEffects)
StdEnumTricks(tLexMC1S::EInputs)
StdEnumTricks(tLexMC1S::EProducts)
StdEnumTricks(tLexMC1S::ESampleRates)
StdEnumTricks(tLexMC1S::ESWLevels)
StdEnumTricks(tLexMC1S::ESWTypes)
StdEnumTricks(tLexMC1S::EStreamTypes)
StdEnumTricks(tLexMC1S::ESysParms)


