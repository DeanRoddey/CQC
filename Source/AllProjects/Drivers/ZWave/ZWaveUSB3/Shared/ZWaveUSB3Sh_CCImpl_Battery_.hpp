//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Battery_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/20/2018
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
//  A CC implementation class for the Battery command class. It's very simple and
//  the base unit class will automatically add one of these if the device supports
//  the battery class, so that any units that can report battery level will get
//  that implemented.
//
//  We have to know how to get the value of this CC, so it uses the standard read
//  access extra info.
//
//      ReadAcc=[Read, Wakeup, Notify]
//
//  We use this to both set our access and to set a flag to let us know what we need
//  to do.
//
// CAVEATS/GOTCHAS:
//
//  1)  These are never implemented as an end point, so it doesn't accept an end
//      point id in the ctor like most CC impls do.
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBattery
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplBattery : public TZWCCImplLevel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplBattery
        (
                    TZWUnitInfo* const      punitiThis
        );

        TZWCCImplBattery(const TZWCCImplBattery&) = delete;

        ~TZWCCImplBattery();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExtraInfo() override;


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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SendGetMsg();


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplBattery, TZWCCImplLevel)
};


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBatteryF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplBatteryF : public TZWCCImplBattery
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplBatteryF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
        );

        TZWCCImplBatteryF(const TZWCCImplBatteryF&) = delete;

        ~TZWCCImplBatteryF();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
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
        //  m_c4FldId_Level
        //      The percent level of the battery.
        //
        //  m_strFldName
        //      The name of the field. Has to include the whole field, including
        //      V2 stuff since we don't get involved in such decisions here and we
        //      will be used for both V2 and non-V2 stuff.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Level;
        TString             m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplBatteryF, TZWCCImplBattery)
};

#pragma CIDLIB_POPPACK
