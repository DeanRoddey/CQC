//
// FILE NAME: GenProtoS_StoreOp.cpp
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
//  This file implements the store op class.
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
RTTIDecls(TGenProtoStoreOp,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoStoreOp
//  PREFIX: sop
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoStoreOp: Constructors and Destructors
// ---------------------------------------------------------------------------
TGenProtoStoreOp::TGenProtoStoreOp( TGenProtoFldInfo* const     pfldiTarget
                                    , TGenProtoExprNode* const  pnodeValue) :
    m_bDeref(kCIDLib::False)
    , m_eType(EStoreTypes::Field)
    , m_pfldiTarget(pfldiTarget)
    , m_pnodeValue(pnodeValue)
    , m_pvariTarget(0)
{
}

TGenProtoStoreOp::
TGenProtoStoreOp(       TGenProtoVarInfo* const     pvariTarget
                ,       TGenProtoExprNode* const    pnodeValue
                , const tCIDLib::TBoolean           bDeref) :

    m_bDeref(bDeref)
    , m_eType(EStoreTypes::Variable)
    , m_pfldiTarget()
    , m_pnodeValue(pnodeValue)
    , m_pvariTarget(pvariTarget)
{
    //
    //  If it's a deref, then set the type to field. The target field will
    //  be set dynamically at runtime.
    //
    if (bDeref)
        m_eType = EStoreTypes::Field;
}

TGenProtoStoreOp::TGenProtoStoreOp(const TGenProtoStoreOp& sopToCopy) :

    m_bDeref(sopToCopy.m_bDeref)
    , m_eType(sopToCopy.m_eType)
    , m_pfldiTarget(sopToCopy.m_pfldiTarget)
    , m_pnodeValue(::pDupObject<TGenProtoExprNode>(sopToCopy.m_pnodeValue))
    , m_pvariTarget(sopToCopy.m_pvariTarget)
{
}

TGenProtoStoreOp::~TGenProtoStoreOp()
{
    // We only own the expression node, not the info objects
    delete m_pnodeValue;
}


// ---------------------------------------------------------------------------
//  TGenProtoStoreOp: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoStoreOp::operator=(const TGenProtoStoreOp& sopToAssign)
{
    if (this != &sopToAssign)
    {
        // We don't own the info pointers, so we can just shallow copy
        m_bDeref        = sopToAssign.m_bDeref;
        m_eType         = sopToAssign.m_eType;
        m_pfldiTarget   = sopToAssign.m_pfldiTarget;
        m_pvariTarget   = sopToAssign.m_pvariTarget;

        delete m_pnodeValue;
        m_pnodeValue = ::pDupObject<TGenProtoExprNode>(sopToAssign.m_pnodeValue);
    }
}



// ---------------------------------------------------------------------------
//  TGenProtoStoreOp: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoStoreOp::bIsDeref() const
{
    return m_bDeref;
}


tCIDLib::TBoolean TGenProtoStoreOp::bDoStore(TGenProtoCtx& ctxThis)
{
    // Evaluate the expression. Acording to the field type, store the result
    m_pnodeValue->Evaluate(ctxThis);

    //
    //  If the target is a field, then call the appropriate set method
    //  based on the type of the field.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (m_eType == EStoreTypes::Field)
    {
        TGenProtoSDriver* psdrvTar = ctxThis.psdrvThis();

        const tCIDLib::TCard4 c4Id = m_pfldiTarget->flddInfo().c4Id();
        switch(m_pfldiTarget->flddInfo().eType())
        {
            case tCQCKit::EFldTypes::Boolean :
                bRet = psdrvTar->bStoreBoolFld
                (
                    c4Id
                    , m_pnodeValue->evalCur().bValue()
                    , kCIDLib::True
                );
                break;

            case tCQCKit::EFldTypes::Card :
                bRet = psdrvTar->bStoreCardFld
                (
                    c4Id
                    , m_pnodeValue->evalCur().c4Value()
                    , kCIDLib::True
                );
                break;

            case tCQCKit::EFldTypes::Float :
                bRet = psdrvTar->bStoreFloatFld
                (
                    c4Id
                    , m_pnodeValue->evalCur().f8Value()
                    , kCIDLib::True
                );
                break;

            case tCQCKit::EFldTypes::Int :
                bRet = psdrvTar->bStoreIntFld
                (
                    c4Id
                    , m_pnodeValue->evalCur().i4Value()
                    , kCIDLib::True
                );
                break;

            case tCQCKit::EFldTypes::String :
                bRet = psdrvTar->bStoreStringFld
                (
                    c4Id
                    , m_pnodeValue->evalCur().strValue()
                    , kCIDLib::True
                );
                break;

            default :
                // This is bad
                break;
        };
    }
     else
    {
        //
        //  Its a variable, so store it and return false, since we don't count
        //  these as a 'change', because the outside world cannot see it.
        //
        m_pvariTarget->Set(m_pnodeValue->evalCur());
    }
    return bRet;
}


const TString& TGenProtoStoreOp::strTarget() const
{
    if (m_eType == EStoreTypes::Field)
        return m_pfldiTarget->flddInfo().strName();

    return m_pvariTarget->strName();
}


const TString& TGenProtoStoreOp::strVarValue(TGenProtoCtx& ctxThis)
{
    #if CID_DEBUG_ON
    if (!m_pvariTarget)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcDebug_MustBeDeref
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
    #endif

    return m_pvariTarget->evalThis().strValue();
}


tCIDLib::TVoid TGenProtoStoreOp::SetField(TGenProtoFldInfo* const pfldiTarget)
{
    // This can only be called for deref stores
    #if CID_DEBUG_ON
    if (!m_bDeref)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcDebug_MustBeDeref
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
    #endif

    m_pfldiTarget = pfldiTarget;
}


