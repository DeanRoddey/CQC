//
// FILE NAME: CQCDocComp_HelpNode.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/03/2015
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
//  The implementation the basic recursive markup containing node.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCDocComp.hpp"



// ---------------------------------------------------------------------------
//   CLASS: THelpNode
//  PREFIX: hn
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THelpNode: Constructors and Destructor
// ---------------------------------------------------------------------------

THelpNode::THelpNode() :

    m_c4Extra(0)
    , m_eType(tCQCDocComp::EType_None)
{
}

THelpNode::~THelpNode()
{
}


// ---------------------------------------------------------------------------
//  THelpNode: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THelpNode::bIsEmpty() const
{
    //
    //  If this guy isn't actual content, and it holds no content. Or, it's a text node
    //  and there's no text, consider it empty.
    //
    return
    (
        m_colNodes.bIsEmpty() || ((m_eType == tCQCDocComp::EType_Text) && m_strText.bIsEmpty())
    );
}
