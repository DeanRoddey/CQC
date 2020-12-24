//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Basic_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/21/2017
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
//  A CC implementation class for the basic command class. We have one for use with
//  off/on devices and one for use with level based devices. If a particular unit
//  supports both, just use one of each. We have a basic version of each one that
//  just stores its internal state which is often what is needed. And we have a
//  simple derivative of each that can create and maintain its own field. Where that's
//  doable, it saves a lot of busy work.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBasicLevel
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplBasicLevel : public TZWCCImplLevel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplBasicLevel
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplBasicLevel(const TZWCCImplBasicLevel&) = delete;

        ~TZWCCImplBasicLevel();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::ECommResults eSendValue
        (
            const   tCIDLib::TCard4         c4ToSend
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
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
        RTTIDefs(TZWCCImplBasicLevel, TZWCCImplLevel)
};


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBasicLevelF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplBasicLevelF : public TZWCCImplBasicLevel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplBasicLevelF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
            , const tCQCKit::EFldSTypes     eSemType = tCQCKit::EFldSTypes::Generic
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplBasicLevelF(const TZWCCImplBasicLevelF&) = delete;

        ~TZWCCImplBasicLevelF();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TBoolean bCardFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            ,       tCQCKit::ECommResults&  eRes
        )   override;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const override;

        tCIDLib::TVoid StoreFldIds() override;


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
        //  m_c4FldId_Level
        //      If this guy supports a dimming level, this is the field for that.
        //      It may not.
        //
        //  m_eSemType
        //      The semantic field type for our field. Not much interest to us,
        //      but we need it for creating the field definition.
        //
        //  m_strFldName
        //      The name of the field. Has to include the whole field, including
        //      V2 stuff since we don't get involved in such decisions here and we
        //      will be used for both V2 and non-V2 stuff.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Level;
        tCQCKit::EFldSTypes m_eSemType;
        TString             m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplBasicLevelF, TZWCCImplBasicLevel)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBasicSwitch
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplBasicSwitch : public TZWCCImplBin
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplBasicSwitch
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplBasicSwitch(const TZWCCImplBasicSwitch&) = delete;

        ~TZWCCImplBasicSwitch();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SendValueQuery() override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::ECommResults eSendValue
        (
            const   tCIDLib::TBoolean       bToSend
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplBasicSwitch, TZWCCImplBin)
};


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBasicSwitchF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplBasicSwitchF : public TZWCCImplBasicSwitch
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplBasicSwitchF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
            , const tCQCKit::EFldSTypes     eSemType = tCQCKit::EFldSTypes::Generic
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplBasicSwitchF(const TZWCCImplBasicSwitchF&) = delete;

        ~TZWCCImplBasicSwitchF();


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

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const override;

        tCIDLib::TVoid StoreFldIds() override;


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
        //      A boolean field that holds the state of our unit (or end point.)
        //
        //  m_eSemType
        //      The semantic field type for our field. Not much interest to us,
        //      but we need it for creating the field definition.
        //
        //  m_strFldName
        //      The name of the field. Has to include the whole field, including
        //      V2 stuff since we don't get involved in such decisions here and we
        //      will be used for both V2 and non-V2 stuff.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_State;
        tCQCKit::EFldSTypes m_eSemType;
        TString             m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplBasicSwitchF, TZWCCImplBasicSwitch)
};

#pragma CIDLIB_POPPACK
