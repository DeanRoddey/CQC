//
// FILE NAME: GenProtoS_Query.cpp
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
//  This file implements the query class.
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
RTTIDecls(TGenProtoQuery,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoQuery
//  PREFIX: qry
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoQuery: Public, static methods
// ---------------------------------------------------------------------------
const TString& TGenProtoQuery::strKey(const TGenProtoQuery& qrySrc)
{
    return qrySrc.m_strKey;
}


// ---------------------------------------------------------------------------
//  TGenProtoQuery: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoQuery::TGenProtoQuery(const TString& strKey) :

    m_colExprs(tCIDLib::EAdoptOpts::Adopt)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
}

TGenProtoQuery::TGenProtoQuery(const TGenProtoQuery& qryToCopy) :

    m_colExprs(tCIDLib::EAdoptOpts::Adopt, qryToCopy.m_colExprs.c4ElemCount())
    , m_hshKey(qryToCopy.m_hshKey)
    , m_strKey(qryToCopy.m_strKey)
{
    // And dup the expressions that build the write command
    const tCIDLib::TCard4 c4Count = qryToCopy.m_colExprs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_colExprs.Add
        (
            ::pDupObject<TGenProtoExprNode>(qryToCopy.m_colExprs[c4Index])
        );
    }
}


TGenProtoQuery::~TGenProtoQuery()
{
    //
    //  Flush the query expression collection. Its not required, since the
    //  collection will clean up anyway, but just to make it obvious to
    //  readers of the code that we are destroying the expression list.
    //
    m_colExprs.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TGenProtoQuery: Public operators
// ---------------------------------------------------------------------------
TGenProtoQuery& TGenProtoQuery::operator=(const TGenProtoQuery& qryToAssign)
{
    if (this != &qryToAssign)
    {
        // Copy over the field info member
        m_hshKey = qryToAssign.m_hshKey;
        m_strKey = qryToAssign.m_strKey;

        // Clear our list of query expressions and dup the source's
        m_colExprs.RemoveAll();
        const tCIDLib::TCard4 c4Count = qryToAssign.m_colExprs.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colExprs.Add
            (
                ::pDupObject<TGenProtoExprNode>(qryToAssign.m_colExprs[c4Index])
            );
        }
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoQuery: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoQuery::AddQueryExpr(TGenProtoExprNode* const pnodeToAdopt)
{
    //
    //  Just add it to our collection. We don't have to check it since it
    //  can be of any type.
    //
    m_colExprs.Add(pnodeToAdopt);
}


tCIDLib::TVoid TGenProtoQuery::BuildQueryCmd(TGenProtoCtx& ctxToUse)
{
    //
    //  Allrighty. Lets loop through the expressions that make up the query
    //  command for this field. For each one, according to the return type,
    //  we copy bytes into the target buffer.
    //
    tCIDLib::TCard4 c4Bytes = 0;

    const tCIDLib::TCard4 c4Count = m_colExprs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        //
        //  Update the context with the bytes in the send buffer. We have to do
        //  this at each step because any expression could refer to the current
        //  length.
        //
        ctxToUse.c4SendBytes(c4Bytes);

        // Get the current expression and evaluate it
        try
        {
            TGenProtoExprNode* pnodeCur = m_colExprs[c4Index];
            pnodeCur->Evaluate(ctxToUse);

            //
            //  And ask it to write its value to our buffer. It will update
            //  the c4Bytes parameter by the bytes written.
            //
            pnodeCur->evalCur().ValueToMemBuf
            (
                ctxToUse
                , ctxToUse.mbufSend()
                , c4Bytes
            );
        }

        catch(const TError& errToCatch)
        {
            if (facGenProtoS().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcRunT_ErrInQuery
                , tCIDLib::ESeverities::Failed
                , errToCatch.eClass()
                , m_strKey
                , ctxToUse.strProtoFile()
            );
        }
    }

    // And do the final update of the length
    ctxToUse.c4SendBytes(c4Bytes);
}


tCIDLib::THashVal TGenProtoQuery::hshKey() const
{
    return m_hshKey;
}


const TString& TGenProtoQuery::strKey() const
{
    return m_strKey;
}



