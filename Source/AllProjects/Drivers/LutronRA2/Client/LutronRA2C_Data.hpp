// ----------------------------------------------------------------------------
//  FILE: LutronRA2C_Data.hpp
//  DATE: Sat, Sep 25 12:27:39 2021 -0400
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ----------------------------------------------------------------------------
// Constants namespace
// ----------------------------------------------------------------------------
namespace kLutronRA2C
{
    
    // ------------------------------------------------------------------------
    //  Identifiers for the values we allow the user to edit via the in-place
    //  editing mechanism.
    //                  
    // ------------------------------------------------------------------------
     const extern TString strAttr_Addr;
     const extern TString strAttr_Extra;
     const extern TString strAttr_Name;
     const extern TString strAttr_Trigger;
}

// ----------------------------------------------------------------------------
// Types namespace
// ----------------------------------------------------------------------------
namespace tLutronRA2C
{
    
    // ------------------------------------------------------------------------
    //  The column indices in the multi-column list box. This lets everyone
    //  easily agree on what data is where. We also use this as the column
    //  titles.
    //                  
    // ------------------------------------------------------------------------
    enum class EListCols
    {
        Type
        , Name
        , Addr
        , Trigger
        , Extra
        , Count
        , Min = Type
        , Max = Extra
    };
    [[nodiscard]]  EListCols eXlatEListCols(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  const TString& strXlatEListCols(const EListCols eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  tCIDLib::TBoolean bIsValidEnum(const EListCols eVal);

    inline tLutronRA2C::EListCols operator++(CIOP tLutronRA2C::EListCols& eVal, int)
    {
        const tLutronRA2C::EListCols eTmp = eVal;
        eVal = tLutronRA2C::EListCols(int(eVal)+1);
        return eTmp;
    }

    
    // ------------------------------------------------------------------------
    //  This list of item types we currently support. They are stored as text, so
    //  the order can be changed.
    //                  
    // ------------------------------------------------------------------------
    enum class EItemTypes
    {
        Button
        , ContactCl
        , Dimmer
        , Fan
        , LED
        , OccSensor
        , Shade
        , ShadeGroup
        , Switch
        , Thermo
        , Count
        , Min = Button
        , Max = Thermo
    };
    [[nodiscard]]  EItemTypes eXlatEItemTypes(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  const TString& strXlatEItemTypes(const EItemTypes eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  tCIDLib::TBoolean bIsValidEnum(const EItemTypes eVal);

    inline tLutronRA2C::EItemTypes operator++(CIOP tLutronRA2C::EItemTypes& eVal, int)
    {
        const tLutronRA2C::EItemTypes eTmp = eVal;
        eVal = tLutronRA2C::EItemTypes(int(eVal)+1);
        return eTmp;
    }

     TTextOutStream& operator<<(TTextOutStream& strmTar, const tLutronRA2C::EItemTypes eToStream);
}

#pragma CIDLIB_POPPACK



