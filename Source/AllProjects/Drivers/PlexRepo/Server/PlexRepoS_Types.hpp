// ----------------------------------------------------------------------------
//  FILE: PlexRepoS_Types.hpp
//  DATE: Fri, Feb 12 21:14:16 2021 -0500
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ----------------------------------------------------------------------------
// Types namespace
// ----------------------------------------------------------------------------
namespace tPlexRepoS
{
    
    // ------------------------------------------------------------------------
    //  Used to translate between the element names and enums we use to
    //  represent them.
    //                  
    // ------------------------------------------------------------------------
    enum class EElems
    {
        None
        , Director
        , Genre
        , Media
        , Part
        , Role
        , Root
        , Video
        , Count
        , Min = None
        , Max = Video
    };
    [[nodiscard]]  EElems eXlatEElems(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  const TString& strXlatEElems(const EElems eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  const TString& strAltXlatEElems(const EElems eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  EElems eAltXlatEElems(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  tCIDLib::TBoolean bIsValidEnum(const EElems eVal);

    
    // ------------------------------------------------------------------------
    //  Used to translate between the attribute names and enums we use to
    //  represent them. Some are not really stored by their attribute names
    //  because they are generic attribute names and it's the element that
    //  matter. Their text will be empty.
    //                  
    // ------------------------------------------------------------------------
    enum class EAttrs
    {
        Added
        , AspectRatio
        , Art
        , ArtThumb
        , Channels
        , Director
        , Descr
        , Duration
        , FilePath
        , Genre
        , Id
        , Key
        , Rating
        , Studio
        , Title
        , UserRating
        , Year
        , Count
        , Min = Added
        , Max = Year
    };
    [[nodiscard]]  const TString& strAltXlatEAttrs(const EAttrs eToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::True);
    [[nodiscard]]  EAttrs eAltXlatEAttrs(const TString& strToXlat, const tCIDLib::TBoolean bThrowIfNot = kCIDLib::False);
    [[nodiscard]]  tCIDLib::TBoolean bIsValidEnum(const EAttrs eVal);

}

#pragma CIDLIB_POPPACK



