//
// FILE NAME: LexMC1CS_Constants.hpp
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
//  This is constants header for the Lexicon MC-1/DC2 driver. Note that some
//  of them are shared with the client and gotten via a header we share with
//  the client.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kLexMC1S
{
    // -----------------------------------------------------------------------
    //  Some general protocol constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1       c1SOP               = 0xF1;
    const tCIDLib::TCard1       c1EOP               = 0xF2;
    const tCIDLib::TCard4       c4FPDLineLen        = 20;
    const tCIDLib::TCard4       c4MaxParmNameLen    = 20;
    const tCIDLib::TCard4       c4MaxCustomNameLen  = 20;
    const tCIDLib::TCard4       c4MaxEffectNameLen  = 13;
    const tCIDLib::TCard4       c4MaxInputNameLen   = 8;



    // -----------------------------------------------------------------------
    //  A map of system param enums to names
    // -----------------------------------------------------------------------
    const tCIDLib::TCh apszSysParmNames[tLexMC1S::ESysParms_Count][18] =
    {
        L"PROGRAM"
        , L"BYPASS"
        , L"MUTE"
        , L"MUTE_MODE"
        , L"VOLUME"
        , L"LR_BALANCE"
        , L"INPUT"
        , L"AUDIO_BIT"
        , L"SRC_TAPE"
        , L"SRC_TUNER"
        , L"SRC_CD"
        , L"SRC_AUX"
        , L"SRC_TV"
        , L"SRC_VDISC"
        , L"SRC_VCR2"
        , L"SRC_VCR1"
        , L"RECORD_SOURCE"
        , L"RECORD_ENABLED"
        , L"RECORD_MODE_TAPE"
        , L"RECORD_MODE_TUNER"
        , L"RECORD_MODE_CD"
        , L"RECORD_MODE_AUX"
        , L"RECORD_MODE_TV"
        , L"RECORD_MODE_VDISC"
        , L"RECORD_MODE_VCR2"
        , L"RECORD_MODE_VCR1"
        , L"VIDEO_TAPE"
        , L"VIDEO_TUNER"
        , L"VIDEO_CD"
        , L"VIDEO_AUX"
        , L"VIDEO_TV"
        , L"VIDEO_VDISC"
        , L"VIDEO_VCR2"
        , L"VIDEO_VCR1"
        , L"VIDEO_RECORD"
        , L"ILVL_TAPE"
        , L"ILVL_TUNER"
        , L"ILVL_CD"
        , L"ILVL_AUX"
        , L"ILVL_TV"
        , L"ILVL_VDISC"
        , L"ILVL_VCR2"
        , L"ILVL_VCR1"
        , L"AUTOLVL_TAPE"
        , L"AUTOLVL_TUNER"
        , L"AUTOLVL_CD"
        , L"AUTOLVL_AUX"
        , L"AUTOLVL_TV"
        , L"AUTOLVL_VDISC"
        , L"AUTOLVL_VCR2"
        , L"AUTOLVL_VCR1"
        , L"FRONT_SPKR"
        , L"FRONT_HPASS_SMALL"
        , L"FRONT_HPASS"
        , L"CENTER_SPKR"
        , L"BSPLIT_SMALL"
        , L"BSPLIT"
        , L"SIDE_SPKR"
        , L"SIDE_STD_HPASS"
        , L"SIDE_DIPOLE_HPASS"
        , L"SIDE_HPASS"
        , L"REAR_SPKR"
        , L"REAR_SMALL_HPASS"
        , L"REAR_HPASS"
        , L"SUBWOOFER"
        , L"SUB_LOWPASS"
        , L"OUT_ADJ_CENTER"
        , L"OUT_ADJ_SUB"
        , L"OUT_ADJ_FRONT_L"
        , L"OUT_ADJ_FRONT_R"
        , L"OUT_ADJ_SIDE_L"
        , L"OUT_ADJ_SIDE_R"
        , L"OUT_ADJ_REAR_L"
        , L"OUT_ADJ_REAR_R"
        , L"ANGLE"
        , L"LISTENER_POS"
        , L"SURROUND_DELAY"
        , L"CENTER_DELAY"
        , L"EFFECT_TAPE"
        , L"EFFECT_TUNER"
        , L"EFFECT_CD"
        , L"EFFECT_AUX"
        , L"EFFECT_TV"
        , L"EFFECT_VDISC"
        , L"EFFECT_VCR2"
        , L"EFFECT_VCR1"
        , L"AC3_EFFECT_TAPE"
        , L"AC3_EFFECT_TUNER"
        , L"AC3_EFFECT_CD"
        , L"AC3_EFFECT_AUX"
        , L"AC3_EFFECT_TV"
        , L"AC3_EFFECT_VDISC"
        , L"AC3_EFFECT_VCR2"
        , L"AC3_EFFECT_VCR1"
        , L"DTS_EFFECT_TAPE"
        , L"DTS_EFFECT_TUNER"
        , L"DTS_EFFECT_CD"
        , L"DTS_EFFECT_AUX"
        , L"DTS_EFFECT_TV"
        , L"DTS_EFFECT_VDISC"
        , L"DTS_EFFECT_VCR2"
        , L"DTS_EFFECT_VCR1"
        , L"LAST_EFFECT_TAPE"
        , L"LAST_EFFECT_TUNER"
        , L"LAST_EFFECT_CD"
        , L"LAST_EFFECT_AUX"
        , L"LAST_EFFECT_TV"
        , L"LAST_EFFECT_VDISC"
        , L"LAST_EFFECT_VCR2"
        , L"LAST_EFFECT_VCR1"
        , L"LAST_AC3_TAPE"
        , L"LAST_AC3_TUNER"
        , L"LAST_AC3_CD"
        , L"LAST_AC3_AUX"
        , L"LAST_AC3_TV"
        , L"LAST_AC3_VDISC"
        , L"LAST_AC3_VCR2"
        , L"LAST_AC3_VCR1"
        , L"LAST_DTS_TAPE"
        , L"LAST_DTS_TUNER"
        , L"LAST_DTS_CD"
        , L"LAST_DTS_AUX"
        , L"LAST_DTS_TV"
        , L"LAST_DTS_VDISC"
        , L"LAST_DTS_VCR2"
        , L"LAST_DTS_VCR1"
        , L"LAST_DIG_TAPE"
        , L"LAST_DIG_TUNER"
        , L"LAST_DIG_CD"
        , L"LAST_DIG_AUX"
        , L"LAST_DIG_TV"
        , L"LAST_DIG_VDISC"
        , L"LAST_DIG_VCR2"
        , L"LAST_DIG_VCR1"
        , L"IN_FORMAT_TAPE"
        , L"IN_FORMAT_TUNER"
        , L"IN_FORMAT_CD"
        , L"IN_FORMAT_AUX"
        , L"IN_FORMAT_TV"
        , L"IN_FORMAT_VDISC"
        , L"IN_FORMAT_VCR2"
        , L"IN_FORMAT_VCR1"
        , L"TRIGGER_TAPE"
        , L"TRIGGER_TUNER"
        , L"TRIGGER_CD"
        , L"TRIGGER_AUX"
        , L"TRIGGER_TV"
        , L"TRIGGER_VDISC"
        , L"TRIGGER_VCR2"
        , L"TRIGGER_VCR1"
        , L"BYPASS_PGM"
        , L"LOCK"
        , L"LOCK_ACCESS"
        , L"Z2_VOL"
        , L"Z2_PWRON_VOL"
        , L"Z2_BAL"
        , L"Z2_MUTE"
        , L"VID_TYPE"
        , L"OSD_COLOR"
        , L"OSD_POS"
        , L"DISPLAY_TIMEOUT"
        , L"LCD_ENBL"
        , L"LCD_CNTRST"
        , L"FIP_BRIGHT"
        , L"CUSTOM_NAME"
        , L"PWRON_VOL"
        , L"BASS"
        , L"TREBLE"
        , L"LOUDNESS"
        , L"LIMITER"
        , L"LIMITER_VOLUME"
        , L"LIMITER_ON"
        , L"EFFECT_ONLY"
        , L"TILT"
        , L"RM_NOTCH"
        , L"RE_EQUAL"
        , L"CNTR_SPEECH"
        , L"DOLBY_SR"
        , L"TIMBRE"
        , L"SPKR_RANGE"
        , L"TALIGN_CENTER"
        , L"TALIGN_SUB"
        , L"TALIGN_FRONT_L"
        , L"TALIGN_FRONT_R"
        , L"TALIGN_SIDE_L"
        , L"TALIGN_SIDE_R"
        , L"TALIGN_REAR_L"
        , L"TALIGN_REAR_R"
        , L"AV_SYNC_DELAY"
        , L"MENU_BKGND"
        , L"CUSTOM_PARAM"
    };


        // -----------------------------------------------------------------------
    //  Field names for the pollable fields, the write only fields, and the
    //  fields that we only get upon connect.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszEffect       = L"Effect";
    const tCIDLib::TCh* const   pszInputSrc     = L"InputSrc";
    const tCIDLib::TCh* const   pszMute         = L"Mute";
    const tCIDLib::TCh* const   pszPower        = L"Power";
    const tCIDLib::TCh* const   pszRate         = L"SampleRate";
    const tCIDLib::TCh* const   pszStream       = L"StreamType";
    const tCIDLib::TCh* const   pszVolume       = L"Volume";

    const tCIDLib::TCh* const   pszAdjustVolume = L"AdjustVolume";
    const tCIDLib::TCh* const   pszBassLevel    = L"BassLevel";
    const tCIDLib::TCh* const   pszFPDisplay    = L"FPDisplay";
    const tCIDLib::TCh* const   pszNPInput      = L"NextPrevInput";

    const tCIDLib::TCh* const   pszSetCLev      = L"CLev";
    const tCIDLib::TCh* const   pszSetFLLev     = L"FLLev";
    const tCIDLib::TCh* const   pszSetFRLev     = L"FRLev";
    const tCIDLib::TCh* const   pszSetRLLev     = L"RLLev";
    const tCIDLib::TCh* const   pszSetRRLev     = L"RRLev";
    const tCIDLib::TCh* const   pszSetSLLev     = L"SLLev";
    const tCIDLib::TCh* const   pszSetSRLev     = L"SRLev";
    const tCIDLib::TCh* const   pszSetSubLev    = L"SubLevel";
    const tCIDLib::TCh* const   pszTogglePwr    = L"TogglePower";

    const tCIDLib::TCh* const   pszProduct      = L"Product";
    const tCIDLib::TCh* const   pszSWType       = L"SoftwareType";
    const tCIDLib::TCh* const   pszSWLevel      = L"SoftwareLevel";
    const tCIDLib::TCh* const   pszSWVersion    = L"SoftwareVersion";
    const tCIDLib::TCh* const   pszProtoVersion = L"ProtoVersion";
    const tCIDLib::TCh* const   pszTimeStamp    = L"TimeStamp";


    // -----------------------------------------------------------------------
    //  A map of effect enums to names
    // -----------------------------------------------------------------------
    const tCIDLib::TCh apszEffectNames[tLexMC1S::EEffects_Count][16] =
    {
        L"Unknown"
        , L"Church"
        , L"THX Cinema"
        , L"TV Matrix"
        , L"Mono Logic"
        , L"Panorama"
        , L"NightClub"
        , L"Pro Logic"
        , L"Music Logic"
        , L"Party"
        , L"Surround Noise"
        , L"Pan Noise"
        , L"Concert Hall"
        , L"Cathedral"
        , L"Music Surround"
        , L"Logic7"
        , L"2 Channel"
        , L"2.0"
        , L"2.0 THX"
        , L"2.0 Logic7"
        , L"2.0 Music"
        , L"2.0 2Ch"
        , L"5.1"
        , L"5.1 THX"
        , L"5.1 Logic7"
        , L"5.1 Music"
        , L"5.1 2Ch"
        , L"DTS Film"
        , L"DTS THX"
        , L"DTS Logic7"
        , L"DTS Music"
        , L"DTS 2Ch"
    };


    // -----------------------------------------------------------------------
    //  A map of input enums to names
    // -----------------------------------------------------------------------
    const tCIDLib::TCh apszInputNames[tLexMC1S::EInputs_Count][8] =
    {
        L"Tape"
        , L"Tuner"
        , L"CD"
        , L"Aux"
        , L"TV"
        , L"VDisc"
        , L"DVD"
        , L"VCR"
    };


    // -----------------------------------------------------------------------
    //  A map of product names
    // -----------------------------------------------------------------------
    const tCIDLib::TCh apszProductNames[tLexMC1S::EProducts_Count][8] =
    {
        L"Unknown"
        , L"DC2"
        , L"MC1"
        , L"SDP_3"
    };


    // -----------------------------------------------------------------------
    //  A map of sample rate enums to names
    // -----------------------------------------------------------------------
    const tCIDLib::TCh apszSampleRateNames[tLexMC1S::ESampleRates_Count][9] =
    {
        L"Unknown"
        , L"44Khz"
        , L"48Khz"
        , L"88Khz"
        , L"96Khz"
    };


    // -----------------------------------------------------------------------
    //  A map of software level enums to names
    // -----------------------------------------------------------------------
    const tCIDLib::TCh apszSoftwareLevelNames[tLexMC1S::ESWLevels_Count][13] =
    {
        L"Released"
        , L"Pre-Alpha"
        , L"Alpha"
        , L"Beta"
        , L"Gamma"
        , L"Unsupported"
    };


    // -----------------------------------------------------------------------
    //  A map of software type enums to names
    // -----------------------------------------------------------------------
    const tCIDLib::TCh apszSoftwareTypeNames[tLexMC1S::ESWTypes_Count][9] =
    {
        L"Unknown"
        , L"THX"
        , L"AC3"
        , L"DTS"
    };


    // -----------------------------------------------------------------------
    //  A map of stream type enums to names
    // -----------------------------------------------------------------------
    const tCIDLib::TCh apszStreamTypeNames[tLexMC1S::EStreamTypes_Count][12] =
    {
        L"Unknown"
        , L"AC3"
        , L"PCM"
        , L"Analog"
        , L"DTS"
        , L"AC3 2.0"
        , L"AC3 5.1"
    };
}


