//
// FILE NAME: CQCIntfEng_SpecialImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/18/2005
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
//  This file provides the little bit of out of line implementation that
//  this abstract class requires.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfEng_.hpp"
#include    "CQCIntfEng_SpecialImg.hpp"



// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfSpecialImg,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfSpecialImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfSpecialImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfSpecialImg::
TCQCIntfSpecialImg( const   tCQCIntfEng::ECapFlags  eCaps
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCaps, strTypeName)
{
}

TCQCIntfSpecialImg::TCQCIntfSpecialImg(const TCQCIntfSpecialImg& iwdgToCopy) :

    TCQCIntfWidget(iwdgToCopy)
{
}

TCQCIntfSpecialImg::~TCQCIntfSpecialImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfSpecialImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfSpecialImg&
TCQCIntfSpecialImg::operator=(const TCQCIntfSpecialImg& iwdgToAssign)
{
    if (this == &iwdgToAssign)
        return *this;

    TParent::operator=(iwdgToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfSpecialImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfSpecialImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfSpecialImg& iwdgOther(static_cast<const TCQCIntfSpecialImg&>(iwdgSrc));

    // Nothing else to do for now
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCIntfSpecialImg::
QueryReferencedImgs(        tCIDLib::TStrHashSet&
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean
                    , const tCIDLib::TBoolean) const
{
    //
    //  We can just provide a no-op implementation for our derivatives. THey
    //  use images, but only system images and at runtime. They have no
    //  configurable (i.e. persisted) mappings.
    //
}


tCIDLib::TVoid
TCQCIntfSpecialImg::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    // For now, nothing to do but pass it on
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfSpecialImg::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // For now, nothing to do but pass it on
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
}

