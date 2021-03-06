// ----------------------------------------------------------------------------
//  FILE: CQCInst_Types.hpp
//  DATE: Fri, Feb 12 21:14:17 2021 -0500
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ----------------------------------------------------------------------------
// Types namespace
// ----------------------------------------------------------------------------
namespace tCQCInst
{
    
    // ------------------------------------------------------------------------
    //  The individual components that we can install. Which ones depends on the
    //  install mode above. We load a list of these into a checked list box and
    //  check/uncheck them depending on the mode selected.
    //                  
    // ------------------------------------------------------------------------
    enum class EInstComps
    {
        CTools
        , CQCVoice
        , DevCtrl
        , EventSrv
        , IntfViewer
        , LogicSrv
        , MasterSrv
        , TrayMon
        , WebSrv
        , XMLGW
        , Count
        , Min = CTools
        , Max = XMLGW
    };
    [[nodiscard]]  EInstComps eXlatEInstComps(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  const TString& strXlatEInstComps(const EInstComps eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  tCIDLib::TBoolean bIsValidEnum(const EInstComps eVal);

    inline tCQCInst::EInstComps operator++(tCQCInst::EInstComps& eVal, int)
    {
        tCQCInst::EInstComps eTmp = eVal;
        eVal = tCQCInst::EInstComps(int(eVal)+1);
        return eTmp;
    }

    
    // ------------------------------------------------------------------------
    //  We have three buttons (back, next, and close.) Each panel can control
    //  the button states by calling a method on the main frame window (their
    //  parent.)
    //                  
    // ------------------------------------------------------------------------
    enum class ENavFlags : tCIDLib::TCard4
    {
        AllowNext = 0x0001
        , AllowPrev = 0x0002
        , AllowClose = 0x0004
        , AllowAll = 0x0007
        , NoBack = 0x0005
        , Count = 5
        , None = 0
        , AllBits = 0x7
    };
    [[nodiscard]]  tCIDLib::TBoolean bIsValidEnum(const ENavFlags eVal);

    [[nodiscard]] constexpr tCQCInst::ENavFlags operator|(const tCQCInst::ENavFlags eLHS, const tCQCInst::ENavFlags eRHS)
    {    
        return tCQCInst::ENavFlags(tCIDLib::TEnumMaskType(eLHS) | tCIDLib::TEnumMaskType(eRHS));
    }
    constexpr tCQCInst::ENavFlags& operator|=(tCQCInst::ENavFlags& eLHS, const tCQCInst::ENavFlags eRHS)
    {    
        eLHS = tCQCInst::ENavFlags(tCIDLib::TEnumMaskType(eLHS) | tCIDLib::TEnumMaskType(eRHS));
        return eLHS;
    }
    [[nodiscard]] constexpr tCQCInst::ENavFlags operator&(const tCQCInst::ENavFlags eLHS, const tCQCInst::ENavFlags eRHS)
    {    
        return tCQCInst::ENavFlags(tCIDLib::TEnumMaskType(eLHS) & tCIDLib::TEnumMaskType(eRHS));
    }
    constexpr tCQCInst::ENavFlags& operator&=(tCQCInst::ENavFlags& eLHS, const tCQCInst::ENavFlags eRHS)
    {    
        eLHS = tCQCInst::ENavFlags(tCIDLib::TEnumMaskType(eLHS) & tCIDLib::TEnumMaskType(eRHS));
        return eLHS;
    }
}

#pragma CIDLIB_POPPACK



