//
// FILE NAME: CQCIntfEng_StaticMarquee.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/21/2004
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
//  This file implements a derivative of the static text widget that just
//  tells the parent class that it should operate in Marquee style.
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
#include    "CQCIntfEng_StaticText.hpp"
#include    "CQCIntfEng_StaticMarquee.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfStaticMarquee,TCQCIntfStaticText)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCIntfEng_StaticMarquee
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The caps flags for static marquees
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eMarqCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
            , tCQCIntfEng::ECapFlags::OptionalAct
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticMarquee
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticMarquee: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticMarquee::TCQCIntfStaticMarquee() :

    TCQCIntfStaticText
    (
        kCIDLib::True
        , CQCIntfEng_StaticMarquee::eMarqCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          +  facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Marquee)
    )
{
    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Marquee");
}

TCQCIntfStaticMarquee::
TCQCIntfStaticMarquee(const TCQCIntfStaticMarquee& iwdgToCopy) :

    TCQCIntfStaticText(iwdgToCopy)
{
}

TCQCIntfStaticMarquee::~TCQCIntfStaticMarquee()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticMarquee: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticMarquee&
TCQCIntfStaticMarquee::operator=(const TCQCIntfStaticMarquee& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticMarquee: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfStaticMarquee::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticMarquee& iwdgOther(static_cast<const TCQCIntfStaticMarquee&>(iwdgSrc));

    // Nothing else here for now
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfStaticMarquee::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Nothing else for us for now
}


tCIDLib::TVoid
TCQCIntfStaticMarquee::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Nothing else here for now
}


