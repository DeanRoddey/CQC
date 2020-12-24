//
// FILE NAME: ZWaveUSB3Sh_CCImpl_BinSwitch_.hpp
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
//  A CC implementation class for the binary switch Z-Wave command class. This is
//  pretty commonly used. We just supoprt off/on functionality.
//
//  We set default access of read/write in our ctor since that's the most common
//  scenario for this CC, but the extra info/creating unit, can override this.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBinSwitch
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplBinSwitch : public TZWCCImplBin
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplBinSwitch
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplBinSwitch(const TZWCCImplBinSwitch&) = delete;

        ~TZWCCImplBinSwitch();


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

        tCIDLib::TVoid SendValueQuery() override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplBinSwitch, TZWCCImplBin)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBinSwitchF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplBinSwitchF : public TZWCCImplBinSwitch
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplBinSwitchF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
            , const tCQCKit::EFldSTypes     eSemType = tCQCKit::EFldSTypes::Generic
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplBinSwitchF(const TZWCCImplBinSwitchF&) = delete;

        ~TZWCCImplBinSwitchF();


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
        RTTIDefs(TZWCCImplBinSwitchF, TZWCCImplBinSwitch)
};

#pragma CIDLIB_POPPACK
