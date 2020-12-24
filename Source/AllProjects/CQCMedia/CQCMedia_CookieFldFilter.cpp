//
// FILE NAME: CQCMedia_CookieFldFilter.cpp
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
//  This is the implementation file for the cookie field filter class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCCookeFldFilter,TCQCFldFilter)



// ---------------------------------------------------------------------------
//   CLASS: TCQCCookeFldFilter
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFilter: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCCookeFldFilter::TCQCCookeFldFilter() :

    TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite)
{
}

TCQCCookeFldFilter::~TCQCCookeFldFilter()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCCookeFldFilter::bCanAcceptField(const   TCQCFldDef&             flddCheck
                                    , const TString&                strMon
                                    , const TString&                strMake
                                    , const TString&                strModel
                                    , const tCQCKit::EDevCats       eCat
                                    , const tCQCKit::TDevClassList& fcolCls) const
{
    // Call our parent first
    if (!TParent::bCanAcceptField(flddCheck, strMon, strMake, strModel, eCat, fcolCls))
        return kCIDLib::False;

    //
    //  It has to be a string type and have one of the expected field names.
    //  Check for V1 and V2 variations.
    //
    if ((flddCheck.eType() == tCQCKit::EFldTypes::String)
    &&  ((flddCheck.strName() == TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurColCookie))
    ||   (flddCheck.strName() == TCQCStdMediaRendDrv::strRendFld2(tCQCMedia::ERendFlds::CurColCookie))
    ||   (flddCheck.strName() == TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurTiCookie))
    ||   (flddCheck.strName() == TCQCStdMediaRendDrv::strRendFld2(tCQCMedia::ERendFlds::CurTiCookie))))
    {
        return kCIDLib::True;
    }
    return kCIDLib::False;
}

