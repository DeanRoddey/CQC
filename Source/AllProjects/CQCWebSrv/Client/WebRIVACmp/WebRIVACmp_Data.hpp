// ----------------------------------------------------------------------------
//  FILE: WebRIVACmp_Data.hpp
//  DATE: Fri, Feb 12 21:14:15 2021 -0500
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ----------------------------------------------------------------------------
// Constants namespace
// ----------------------------------------------------------------------------
namespace kWebRIVACmp
{
    
    // ------------------------------------------------------------------------
    //  Some values we look for in the XML file we parse.
    //                  
    // ------------------------------------------------------------------------
     const extern TString strAttrName;
     const extern TString strAttrDir;
     const extern TString strAttrDirBoth;
     const extern TString strAttrDirCS;
     const extern TString strAttrDirSC;
     const extern TString strAttrAName;
     const extern TString strAttrType;
     const extern TString strAttrVal;
     const extern TString strAttrEnumType;
    
    // ------------------------------------------------------------------------
    //  Some general values used in output generation
    //                  
    // ------------------------------------------------------------------------
     const extern TString strVal_False;
     const extern TString strVal_True;
     const extern TString strVal_LineSep;
     const extern TString strVal_CommaFalse;
     const extern TString strVal_CommaTrue;
}

// ----------------------------------------------------------------------------
// Types namespace
// ----------------------------------------------------------------------------
namespace tWebRIVACmp
{
    
    // ------------------------------------------------------------------------
    //  A mapping enum for the types we support for constants.
    //                  
    // ------------------------------------------------------------------------
    enum class EConstTypes
    {
        Boolean
        , Int1
        , Int2
        , Int4
        , Int8
        , Card1
        , Card2
        , Card4
        , Card8
        , Float8
        , String
        , Count
        , Min = Boolean
        , Max = String
    };
    [[nodiscard]]  EConstTypes eXlatEConstTypes(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  const TString& strXlatEConstTypes(const EConstTypes eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  const TString& strAltXlatEConstTypes(const EConstTypes eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  EConstTypes eAltXlatEConstTypes(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  tCIDLib::TBoolean bIsValidEnum(const EConstTypes eVal);

    
    // ------------------------------------------------------------------------
    //  A mapping enum for the types we support for structure members. Some
    //  are fundamental types and some are a set of common compound types
    //  that are used in the protocol.
    //  
    //  We use the AltText2 for the actual C++ type, so that we don't have to
    //  special case constantly which are tCIDLib:: prefixed fundamental types
    //  and which are represented as objects on the C++ side.
    //  
    //  And we put all of the fundamental types first, and the object types
    //  at the end. Int8 is the last regular fundamental type. After that are
    //  some special cases, of which enum is the last.
    //  
    //  And then the object types come last.
    //                  
    // ------------------------------------------------------------------------
    enum class EMemTypes
    {
        Boolean
        , Card1
        , Card2
        , Card4
        , Card8
        , Float8
        , Int1
        , Int2
        , Int4
        , Int8
        , Opacity
        , Enum
        , AlphaColor
        , Area
        , Color
        , Point
        , Size
        , String
        , Passthrough
        , Count
        , Min = Boolean
        , Max = Passthrough
        , LastFundType = Int8
    };
    [[nodiscard]]  EMemTypes eXlatEMemTypes(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  const TString& strXlatEMemTypes(const EMemTypes eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  const TString& strAltXlatEMemTypes(const EMemTypes eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  EMemTypes eAltXlatEMemTypes(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  const TString& strAltXlat2EMemTypes(const EMemTypes eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  EMemTypes eAltXlat2EMemTypes(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  tCIDLib::TBoolean bIsValidEnum(const EMemTypes eVal);

}

#pragma CIDLIB_POPPACK



