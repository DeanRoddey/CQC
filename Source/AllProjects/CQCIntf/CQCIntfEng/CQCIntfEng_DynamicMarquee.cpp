//
// FILE NAME: CQCIntfEng_DynamicMarquee.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/22/2004
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
//  This file implements a derivative of the dynamic text widget that just
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
#include    "CQCIntfEng_DynamicText.hpp"
#include    "CQCIntfEng_DynamicMarquee.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfFldMarquee,TCQCIntfFldText)
AdvRTTIDecls(TCQCIntfVarMarquee,TCQCIntfVarText)



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldMarquee
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldMarquee: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldMarquee::TCQCIntfFldMarquee() :

    TCQCIntfFldText(kCIDLib::True)
{
    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Marquee");
}

TCQCIntfFldMarquee::
TCQCIntfFldMarquee(const TCQCIntfFldMarquee& iwdgSrc) :

    TCQCIntfFldText(iwdgSrc)
{
}

TCQCIntfFldMarquee::~TCQCIntfFldMarquee()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldMarquee: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldMarquee&
TCQCIntfFldMarquee::operator=(const TCQCIntfFldMarquee& iwdgSrc)
{
    if (this != &iwdgSrc)
        TParent::operator=(iwdgSrc);
    return *this;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarMarquee
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarMarquee: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarMarquee::TCQCIntfVarMarquee() :

    TCQCIntfVarText(kCIDLib::True)
{
}

TCQCIntfVarMarquee::
TCQCIntfVarMarquee(const TCQCIntfVarMarquee& iwdgToCopy) :

    TCQCIntfVarText(iwdgToCopy)
{
}

TCQCIntfVarMarquee::~TCQCIntfVarMarquee()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarMarquee: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarMarquee&
TCQCIntfVarMarquee::operator=(const TCQCIntfVarMarquee& iwdgToAssign)
{
    if (this != &iwdgToAssign)
        TParent::operator=(iwdgToAssign);
    return *this;
}


