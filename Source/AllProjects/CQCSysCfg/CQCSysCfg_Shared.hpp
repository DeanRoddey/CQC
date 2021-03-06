// ----------------------------------------------------------------------------
//  FILE: CQCSysCfg_Shared.hpp
//  DATE: Fri, Feb 12 21:14:15 2021 -0500
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ----------------------------------------------------------------------------
// Types namespace
// ----------------------------------------------------------------------------
namespace tCQCSysCfg
{
    
    // ------------------------------------------------------------------------
    //  This is used in the layout class to reference various lists that it
    //  maintains. Not persisted directly, but add new ones at the end.
    //                  
    // ------------------------------------------------------------------------
    enum class ECfgLists
    {
        Lighting
        , Media
        , Security
        , Weather
        , XOvers
        , HVAC
        , Count
        , Min = Lighting
        , Max = HVAC
    };
    [[nodiscard]] CQCSYSCFGEXP ECfgLists eXlatECfgLists(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]] CQCSYSCFGEXP const TString& strXlatECfgLists(const ECfgLists eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]] CQCSYSCFGEXP tCIDLib::TBoolean bIsValidEnum(const ECfgLists eVal);

    
    // ------------------------------------------------------------------------
    //  Each room can have a list of global actions that can be invoked
    //  at various places. This is used as an index into that list.
    //  
    //  This is persisted in room config so add new ones at the end!
    //                  
    // ------------------------------------------------------------------------
    enum class EGlobActs
    {
        OnRoomLoad
        , OnIVExit
        , PreMovie
        , PreMusic
        , MusicPowerOff
        , MusicPowerOn
        , MoviePowerOff
        , MoviePowerOn
        , Count
        , Min = OnRoomLoad
        , Max = MoviePowerOn
    };
    [[nodiscard]] CQCSYSCFGEXP EGlobActs eXlatEGlobActs(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]] CQCSYSCFGEXP const TString& strXlatEGlobActs(const EGlobActs eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]] CQCSYSCFGEXP tCIDLib::TBoolean bIsValidEnum(const EGlobActs eVal);

    inline tCQCSysCfg::EGlobActs operator++(tCQCSysCfg::EGlobActs& eVal, int)
    {
        tCQCSysCfg::EGlobActs eTmp = eVal;
        eVal = tCQCSysCfg::EGlobActs(int(eVal)+1);
        return eTmp;
    }

    inline TBinOutStream& operator<<(TBinOutStream& strmTar, const tCQCSysCfg::EGlobActs eVal)
    {
        strmTar.WriteEnum(tCIDLib::TCard4(eVal));
        return strmTar;
    }
    inline TBinInStream& operator>>(TBinInStream& strmSrc, COP tCQCSysCfg::EGlobActs& eToFill)
    {
        eToFill = tCQCSysCfg::EGlobActs(strmSrc.c4ReadEnum());
        return strmSrc;
    }
    
    // ------------------------------------------------------------------------
    //  This is used in the room class to store some info based on the various
    //  pre-defined functional areas we support per room. Not directly persisted
    //  but indirectly so, since the object arrays that hold the stuff indexed
    //  by these are persisted.
    //                  
    // ------------------------------------------------------------------------
    enum class ERmFuncs
    {
        Lighting
        , Movies
        , Music
        , Security
        , Weather
        , HVAC
        , Count
        , Min = Lighting
        , Max = HVAC
    };
    [[nodiscard]] CQCSYSCFGEXP ERmFuncs eXlatERmFuncs(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]] CQCSYSCFGEXP const TString& strXlatERmFuncs(const ERmFuncs eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]] CQCSYSCFGEXP tCIDLib::TBoolean bIsValidEnum(const ERmFuncs eVal);

    inline tCQCSysCfg::ERmFuncs operator++(tCQCSysCfg::ERmFuncs& eVal, int)
    {
        tCQCSysCfg::ERmFuncs eTmp = eVal;
        eVal = tCQCSysCfg::ERmFuncs(int(eVal)+1);
        return eTmp;
    }

}

#pragma CIDLIB_POPPACK


CQCSYSCFGEXP tCIDLib::TVoid TBinInStream_ReadArray(TBinInStream& strmSrc, tCQCSysCfg::EGlobActs* const aeList, const tCIDLib::TCard4 c4Count);
CQCSYSCFGEXP tCIDLib::TVoid TBinOutStream_WriteArray(TBinOutStream& strmTar, const tCQCSysCfg::EGlobActs* const aeList, const tCIDLib::TCard4 c4Count);


