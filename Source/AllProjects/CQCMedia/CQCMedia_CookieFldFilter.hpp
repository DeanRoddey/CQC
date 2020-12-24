//
// FILE NAME: CQCMedia_CookieFldFilter.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/18/2008
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
//  This is the header for the CQCMedia_CookieFldFilter.cpp file, which
//  implements a derivative of the standard CQC field filter class. This
//  one accepts only those fields that represent the collection or title
//  cookies of media renderer drivers, which is something commonly filtered
//  for when configuring things that deal with media players.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCCookeFldFilter
// PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCCookeFldFilter : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCCookeFldFilter();

        TCQCCookeFldFilter(const TCQCCookeFldFilter&) = default;
        TCQCCookeFldFilter(TCQCCookeFldFilter&&) = default;

        ~TCQCCookeFldFilter();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCCookeFldFilter& operator=(const TCQCCookeFldFilter&) = default;
        TCQCCookeFldFilter& operator=(TCQCCookeFldFilter&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eDevCat
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCCookeFldFilter, TCQCFldFilter)
};

#pragma CIDLIB_POPPACK

