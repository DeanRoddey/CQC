//
// FILE NAME: GenProtoS_StateMachine.cpp
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
//  This file implements the class that represents a state in the message
//  matching state machine.
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
RTTIDecls(TGenProtoStateInfo,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoStateInfo::StateTrans
//  PREFIX: strans
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and destructor
// ---------------------------------------------------------------------------
TGenProtoStateInfo::
TStateTrans::TStateTrans(const  TString&                strNextName
                        ,       TGenProtoExprNode* const  pnodeToAdopt) :

    m_c4NextStateInd(kCIDLib::c4MaxCard)
    , m_pnodeTrans(pnodeToAdopt)
    , m_strNextName(strNextName)
{
}

TGenProtoStateInfo::TStateTrans::~TStateTrans()
{
    // Clean up our expression tree
    delete m_pnodeTrans;
}


// ---------------------------------------------------------------------------
//  Public, non-virutal methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4
TGenProtoStateInfo::TStateTrans::c4NextStateInd(const tCIDLib::TCard4 c4ToSet)
{
    m_c4NextStateInd = c4ToSet;
    return m_c4NextStateInd;
}

tCIDLib::TCard4 TGenProtoStateInfo::TStateTrans::c4NextStateInd() const
{
    return m_c4NextStateInd;
}


TGenProtoExprNode* TGenProtoStateInfo::TStateTrans::pnodeTrans()
{
    return m_pnodeTrans;
}


const TString& TGenProtoStateInfo::TStateTrans::strNextName() const
{
    return m_strNextName;
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoStateInfo
//  PREFIX: stati
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoStateInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoStateInfo::TGenProtoStateInfo( const   TString&        strName
                                    , const tCIDLib::TCard4 c4Index) :

    m_c4Index(c4Index)
    , m_colTransitions(tCIDLib::EAdoptOpts::Adopt)
    , m_pcolStoreOps(0)
    , m_strName(strName)
{
}

TGenProtoStateInfo::~TGenProtoStateInfo()
{
    delete m_pcolStoreOps;
}


// ---------------------------------------------------------------------------
//  TGenProtoStateInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoStateInfo::AddStoreOp( TGenProtoVarInfo* const     pvariTarget
                                , TGenProtoExprNode* const  pnodeValue)
{
    // Fault in the store op list if necessary
    if (!m_pcolStoreOps)
        m_pcolStoreOps = new TStoreOpList(tCIDLib::EAdoptOpts::Adopt);
    m_pcolStoreOps->Add(new TGenProtoStoreOp(pvariTarget, pnodeValue, kCIDLib::False));
}


tCIDLib::TVoid TGenProtoStateInfo::
AddTransition(  const   TString&                    strNextStateName
                ,       TGenProtoExprNode* const    pnodeToAdopt)
{
    // Just stick another static transition on the end of the list
    m_colTransitions.Add(new TStateTrans(strNextStateName, pnodeToAdopt));
}


tCIDLib::TCard4 TGenProtoStateInfo::c4NextState(TGenProtoCtx& ctxToUse)
{
    //
    //  Run our list of expressions on the passed byte. The first one that
    //  reports success, return the index of it's target state.
    //
    const tCIDLib::TCard4 c4Count = m_colTransitions.c4ElemCount();

    // Default the return value to the 'no transition' code
    tCIDLib::TCard4 c4Next = kCIDLib::c4MaxCard;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the current state transition
        TStateTrans& stransCur = *m_colTransitions[c4Index];

        // See if has a transition for the current byte
        try
        {
            stransCur.pnodeTrans()->Evaluate(ctxToUse);

            // If the resulting value is true, then this is our guy
            if (stransCur.pnodeTrans()->evalCur().bValue())
            {
                c4Next = stransCur.c4NextStateInd();
                break;
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
                , kGPDErrs::errcRunT_ErrInStateMachine
                , tCIDLib::ESeverities::Failed
                , errToCatch.eClass()
                , m_strName
                , ctxToUse.strProtoFile()
                , stransCur.strNextName()
            );
        }
    }

    // Return either failure or the next index
    return c4Next;
}


tCIDLib::TCard4 TGenProtoStateInfo::c4TransCount() const
{
    return m_colTransitions.c4ElemCount();
}


const TString& TGenProtoStateInfo::strName() const
{
    return m_strName;
}


const TString&
TGenProtoStateInfo::strTransAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colTransitions[c4Index]->strNextName();
}


tCIDLib::TVoid
TGenProtoStateInfo::SetIndices( const   TString&        strStateName
                                , const tCIDLib::TCard4 c4StateIndex)
{
    const tCIDLib::TCard4 c4Count = m_colTransitions.c4ElemCount();

    //
    //  Find all of our transitions that reference this state name and set
    //  their index to the passed index.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the current state transition
        TStateTrans& stransCur = *m_colTransitions[c4Index];

        if (stransCur.strNextName() == strStateName)
            stransCur.c4NextStateInd(c4StateIndex);
    }
}


tCIDLib::TVoid TGenProtoStateInfo::StoreValues(TGenProtoCtx& ctxThis)
{
    // If we never had any, then return now
    if (!m_pcolStoreOps)
        return;

    //
    //  That's ok, so lets run all of the store operations in our list and
    //  ask each one to store its value.
    //
    const tCIDLib::TCard4 c4Count = m_pcolStoreOps->c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_pcolStoreOps->pobjAt(c4Index)->bDoStore(ctxThis);
}


tCIDLib::TVoid TGenProtoStateInfo::Validate()
{
    // If its the magic accept state, then just take it
    if (m_strName == kGenProtoS_::pszAcceptState)
        return;

    // Else validate it
    const tCIDLib::TCard4 c4Count = m_colTransitions.c4ElemCount();

    // There must be at least one transition
    if (!c4Count)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcState_NoTransitions
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , m_strName
        );
    }

    //
    //  Make sure that we have at least one transition that move's beyond
    //  our own state.
    //
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the current state transition
        TStateTrans& stransCur = *m_colTransitions[c4Index];

        if (stransCur.c4NextStateInd() > m_c4Index)
            break;
    }

    // If we didn't find one, then that's an error
    if (c4Index == c4Count)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcState_NoFowardTrans
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , m_strName
        );
    }
}



