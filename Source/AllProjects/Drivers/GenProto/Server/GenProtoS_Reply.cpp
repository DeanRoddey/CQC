//
// FILE NAME: GenProtoS_Reply.cpp
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
//  This file implements the reply class.
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
RTTIDecls(TGenProtoReply,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoReply
//  PREFIX: rep
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoReply: Public, static methods
// ---------------------------------------------------------------------------
const TString& TGenProtoReply::strKey(const TGenProtoReply& repSrc)
{
    return repSrc.m_strKey;
}


// ---------------------------------------------------------------------------
//  TGenProtoReply: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoReply::TGenProtoReply(const TString& strKey) :

    m_colStoreOps(tCIDLib::EAdoptOpts::Adopt)
    , m_colValidate(tCIDLib::EAdoptOpts::Adopt)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
}


TGenProtoReply::TGenProtoReply(const TGenProtoReply& repToCopy) :

    m_colStoreOps(tCIDLib::EAdoptOpts::Adopt, repToCopy.m_colStoreOps.c4ElemCount())
    , m_colValidate(tCIDLib::EAdoptOpts::Adopt, repToCopy.m_colValidate.c4ElemCount())
    , m_hshKey(repToCopy.m_hshKey)
    , m_strKey(repToCopy.m_strKey)
{
    // And dup the expression lists
    tCIDLib::TCard4 c4Count = repToCopy.m_colStoreOps.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_colStoreOps.Add
        (
            ::pDupObject<TGenProtoStoreOp>(repToCopy.m_colStoreOps[c4Index])
        );
    }

    c4Count = repToCopy.m_colValidate.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_colValidate.Add
        (
            ::pDupObject<TGenProtoExprNode>(repToCopy.m_colValidate[c4Index])
        );
    }
}

TGenProtoReply::~TGenProtoReply()
{
    //
    //  Flush the expression collections. Its not required, since the
    //  collections will clean up anyway, but just to make it obvious to
    //  readers of the code that we are destroying the expression lists.
    //
    m_colStoreOps.RemoveAll();
    m_colValidate.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TGenProtoReply: Public operators
// ---------------------------------------------------------------------------
TGenProtoReply& TGenProtoReply::operator=(const TGenProtoReply& repToAssign)
{
    if (this != &repToAssign)
    {
        // Copy over the field info member
        m_hshKey        = repToAssign.m_hshKey;
        m_strKey        = repToAssign.m_strKey;

        // Clear our expression lists and dup the source's
        m_colStoreOps.RemoveAll();
        m_colValidate.RemoveAll();

        tCIDLib::TCard4 c4Count = repToAssign.m_colStoreOps.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colStoreOps.Add
            (
                ::pDupObject<TGenProtoStoreOp>(repToAssign.m_colStoreOps[c4Index])
            );
        }

        c4Count = repToAssign.m_colValidate.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colValidate.Add
            (
                ::pDupObject<TGenProtoExprNode>(repToAssign.m_colValidate[c4Index])
            );
        }
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoReply: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoReply::AddStoreOp( TGenProtoFldInfo* const     pfldiTarget
                            , TGenProtoExprNode* const  pnodeValue)
{
    m_colStoreOps.Add(new TGenProtoStoreOp(pfldiTarget, pnodeValue));
}

tCIDLib::TVoid
TGenProtoReply::AddStoreOp(         TGenProtoVarInfo* const     pvariTarget
                            ,       TGenProtoExprNode* const    pnodeValue
                            , const tCIDLib::TBoolean           bIsDeref)
{
    m_colStoreOps.Add(new TGenProtoStoreOp(pvariTarget, pnodeValue, bIsDeref));
}


tCIDLib::TVoid
TGenProtoReply::AddValidationExpr(TGenProtoExprNode* const pnodeToAdopt)
{
    m_colValidate.Add(pnodeToAdopt);
}


tCIDLib::TBoolean TGenProtoReply::bValidateReply(TGenProtoCtx& ctxThis)
{
    //
    //  That's ok, so lets run all of the message validation expressions.
    //  As soon as one comes back false, we give up.
    //
    const tCIDLib::TCard4 c4Count = m_colValidate.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        try
        {
            // Get the current node and evaluate it
            TGenProtoExprNode* pnodeCur = m_colValidate[c4Index];
            pnodeCur->Evaluate(ctxThis);

            // If it comes back false, give up
            if (!pnodeCur->evalCur().bValue())
            {
                facGenProtoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcRunT_BadMsgValidation
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , m_strKey
                    , ctxThis.strProtoFile()
                );
                return kCIDLib::False;
            }
        }

        catch(const TError& errToCatch)
        {
            if (facGenProtoS().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcRunT_ErrInReplyVal
                , tCIDLib::ESeverities::Failed
                , errToCatch.eClass()
                , m_strKey
                , ctxThis.strProtoFile()
            );
        }
    }
    return kCIDLib::True;
}


tCIDLib::THashVal TGenProtoReply::hshKey() const
{
    return m_hshKey;
}


const TString& TGenProtoReply::strKey() const
{
    return m_strKey;
}


tCIDLib::TVoid
TGenProtoReply::StoreValues(TGenProtoCtx&               ctxThis
                            , TGenProtoReply::TFldList& colFields)
{
    //
    //  That's ok, so lets run all of the store operations in our list and
    //  ask each one to store its value.
    //
    const tCIDLib::TCard4   c4Count = m_colStoreOps.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the current store op
        TGenProtoStoreOp* psopCur = m_colStoreOps[c4Index];

        //
        //  If it's target is a dereferenced const or variable, then we
        //  need to provide it with the actual field to write to, since it
        //  is dynamic. So ask it for the value of the variable it is attached
        //  to, and use that to look up the field.
        //
        if (psopCur->bIsDeref())
        {
            const TString& strName = psopCur->strVarValue(ctxThis);

            const tCIDLib::TCard4   c4Count = colFields.c4ElemCount();
            tCIDLib::TCard4         c4Index2;
            for (c4Index2 = 0; c4Index2 < c4Count; c4Index2++)
            {
                if (colFields[c4Index2].flddInfo().strName() == strName)
                {
                    psopCur->SetField(&colFields[c4Index2]);
                    break;
                }
            }

            // If we didn't find it, that's an error
            if (c4Index2 == c4Count)
            {
                facGenProtoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcRunT_DynFieldNotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , strName
                );

                // We can't store
                continue;
            }
        }

        psopCur->bDoStore(ctxThis);
    }
}


