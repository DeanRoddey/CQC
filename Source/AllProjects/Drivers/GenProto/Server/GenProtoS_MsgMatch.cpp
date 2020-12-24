//
// FILE NAME: GenProtoS_MsgMatch.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This file implements the class that represents a single msg match block
//  in the MsgMatches= block.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//



// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenProtoS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenProtoMsgMatch,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMsgMatch
//  PREFIX: msgm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoMsgMatch: Public, static methods
// ---------------------------------------------------------------------------
const TString& TGenProtoMsgMatch::strKey(const TGenProtoMsgMatch& msgmSrc)
{
    return msgmSrc.m_prepToUse->strKey();
}


// ---------------------------------------------------------------------------
//  TGenProtoMsgMatch: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoMsgMatch::TGenProtoMsgMatch(TGenProtoReply* const      prepToUse
                                    , TGenProtoExprNode* const  pnodeToAdopt) :
    m_pnodeMatch(pnodeToAdopt)
    , m_prepToUse(prepToUse)
{
}

TGenProtoMsgMatch::~TGenProtoMsgMatch()
{
    // We own the expression, but not the reply
    delete m_pnodeMatch;
}


// ---------------------------------------------------------------------------
//  TGenProtoMsgMatch: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoMsgMatch::bMatches(TGenProtoCtx& ctxToUse)
{
    // Evaluate our match expressions
    m_pnodeMatch->Evaluate(ctxToUse);

    // And return the boolean result
    return m_pnodeMatch->evalCur().bValue();
}


TGenProtoReply* TGenProtoMsgMatch::prepMatch()
{
    return m_prepToUse;
}


