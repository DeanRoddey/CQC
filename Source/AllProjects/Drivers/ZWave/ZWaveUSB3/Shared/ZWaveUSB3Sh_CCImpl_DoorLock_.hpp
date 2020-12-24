//
// FILE NAME: ZWaveUSB3Sh_CCImpl_DoorLock_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/03/2018
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
//  Some CC impl classes that implement useful aspects of the Door Lock Z-Wave
//  command class. We assume a standard V2 compliant lock scenario.
//
//  We set default access of read/write (with read after write) in our ctor, since
//  that is the most common scenario. The extra info or owning unit can override if
//  needed.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplDLockState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplDLockState : public TZWCCImplBin
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplDLockState
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplDLockState(const TZWCCImplDLockState&) = delete;

        ~TZWCCImplDLockState();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::ECommResults eSendValue
        (
            const   tCIDLib::TBoolean       bValue
        );


    protected :
        // -------------------------------------------------------------------
        //  protected, inherited methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   override;

        tCIDLib::TVoid SendValueQuery() override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplDLockState, TZWCCImplBin)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplDLockStateF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplDLockStateF : public TZWCCImplDLockState
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplDLockStateF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strSubUnit
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplDLockStateF(const TZWCCImplDLockStateF&) = delete;

        ~TZWCCImplDLockStateF();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBoolFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
            ,       tCQCKit::ECommResults&  eRes
        )   override;

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const override;

        tCIDLib::TVoid StoreFldIds() override;


        // -------------------------------------------------------------------
        //  Public, non-static methods
        // -------------------------------------------------------------------
        const TString& strFldName() const
        {
            return m_strFldName;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid EnteredErrState() override;

        tCIDLib::TVoid ValueChanged
        (
            const   tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_State
        //      A boolean field that holds the state of our lock
        //
        //  m_strFldName
        //      For convenience we store our built up field name for ongoing use.
        //
        //  m_strSubUnit
        //      We assume a V2 compliant lock so we can create the name ourself.
        //      But we need to know any sub-unit name if appropriate.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_State;
        TString             m_strFldName;
        TString             m_strSubUnit;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplDLockStateF, TZWCCImplDLockState)
};

#pragma CIDLIB_POPPACK
