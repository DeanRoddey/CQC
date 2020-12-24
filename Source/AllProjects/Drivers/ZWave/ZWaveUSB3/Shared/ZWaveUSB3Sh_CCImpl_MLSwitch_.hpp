//
// FILE NAME: ZWaveUSB3Sh_CCImpl_MLSwitch_.hpp
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
//  This file implements two CC impl classes, which provide support for the off/on
//  and dimming level functions of the multi-level switch command class.
//
//  The field based Level class supports the extra info values:
//
//      Range=min max
//
//  This allows you to force the range used by the field to something other than the
//  default 0 to 100.
//
//
//  For read access we default to the base CC impl's settings, plus we add write
//  if we are in an always on unit.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLLevel
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplMLLevel : public TZWCCImplLevel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplMLLevel
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplMLLevel(const TZWCCImplMLLevel&) = delete;

        ~TZWCCImplMLLevel();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::ECommResults eSendValue
        (
            const   tCIDLib::TCard4         c4ToSend
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
        RTTIDefs(TZWCCImplMLLevel, TZWCCImplLevel)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLLevelF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplMLLevelF : public TZWCCImplMLLevel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplMLLevelF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
            , const tCQCKit::EFldSTypes     eSemType = tCQCKit::EFldSTypes::Generic
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplMLLevelF(const TZWCCImplMLLevelF&) = delete;

        ~TZWCCImplMLLevelF();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCardFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            ,       tCQCKit::ECommResults&  eRes
        )   override;

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TBoolean bParseExtraInfo() override;

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


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetRange
        (
            const   tCIDLib::TCard4         c4Min
            , const tCIDLib::TCard4         c4Max
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bRangeForced
        //      If SetRange() is called to manually set the range, this is set to true
        //      which will prevent us from looking for any extra info, which wouldn't
        //      be there in that case most likely.
        //
        //  m_c4FldId_Level
        //      If this guy supports a dimming level, this is the field for that.
        //      It may not.
        //
        //  m_c4RangeMin
        //  m_c4RangeMax
        //      Defaulted to 0 and 100, but can be set via extra info or explicit
        //      call. Used in the creation of our level field.
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
        tCIDLib::TBoolean   m_bRangeForced;
        tCIDLib::TCard4     m_c4FldId_Level;
        tCIDLib::TCard4     m_c4RangeMin;
        tCIDLib::TCard4     m_c4RangeMax;
        tCQCKit::EFldSTypes m_eSemType;
        TString             m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplMLLevelF, TZWCCImplMLLevel)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLSwitch
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplMLSwitch : public TZWCCImplBin
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplMLSwitch
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplMLSwitch(const TZWCCImplMLSwitch&) = delete;

        ~TZWCCImplMLSwitch();


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
        //  protected, inherited methods
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
        RTTIDefs(TZWCCImplMLSwitch, TZWCCImplBin)
};


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLSwitchF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplMLSwitchF : public TZWCCImplMLSwitch
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplMLSwitchF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
            , const tCQCKit::EFldSTypes     eSemType = tCQCKit::EFldSTypes::Generic
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplMLSwitchF(const TZWCCImplMLSwitchF&) = delete;

        ~TZWCCImplMLSwitchF();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TBoolean bBoolFldChanged
        (
             const  TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
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
        //  m_c4FldId_State
        //      The state field for this ML switch.
        //
        //  m_eSemType
        //      The semantic field type for our field. Not much interest to us, but
        //      we need it for creating the field definition.
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
        RTTIDefs(TZWCCImplMLSwitchF, TZWCCImplMLSwitch)
};

#pragma CIDLIB_POPPACK
