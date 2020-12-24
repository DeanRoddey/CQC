//
// FILE NAME: ZWaveLevih_SwitchDim.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/12/2012
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
//  This is the header file for the ZWaveLevi_SwitchDim.cpp file, which
//  implements derivatives of the base Z-Wave unit class which represent
//  switches and dimmers. These are the core unit types that will be in almost
//  any Z-Wave network.
//
//  We use the optional info value in the base class to let the user tell us
//  if we are a light switch or dimmer. If so we send out the appropriate
//  event triggers. Our values are 0=no option, 1=switch or dimmer. So if the
//  value is 1, we are of the special semantic type.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWBinSwitch
//  PREFIX: unit
//
//  This class represents a generic binary switch unit. We just need to have
//  storage for our one field id, which is the boolean field that we use to
//  control the switch.
// ---------------------------------------------------------------------------
class ZWLEVISHEXPORT TZWBinSwitch : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWBinSwitch();

        TZWBinSwitch
        (
            const   tCIDLib::TCard4         c4Id
        );

        TZWBinSwitch
        (
            const   TZWBinSwitch&           unitToCopy
        );

        ~TZWBinSwitch();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWBinSwitch& operator=
        (
            const   TZWBinSwitch&           unitSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWBinSwitch&           unitSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWBinSwitch&           unitSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBuildSetMsg
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            ,       TString&                strToFill
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TBoolean bStoreBasicReport
        (
            const   tCIDLib::TCard4         c4Value
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TBoolean bStoreFldIds
        (
            const   TVector<TCQCFldDef>&    colFlds
            , const tZWaveLeviSh::TValList& fcolIds
        );

        tCIDLib::TBoolean bStoreGenReport
        (
            const   tCIDLib::TCard4         ac4Values[]
            , const tCIDLib::TCard4         c4ValCnt
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TBoolean bStoreSceneReport
        (
            const   tCIDLib::TCard4         c4Scene
            , const tCIDLib::TCard4         c4Levle
            , const tCIDLib::TCard4         c4FadeTime
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TBoolean bWantsOptInfo
        (
                    tCIDLib::TStrCollect&   colToFill
        )   const;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    TVector<TCQCFldDef>&    colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        tCIDLib::TVoid PrepForUse();

        tCIDLib::TVoid SetFieldErrs
        (
                    TCQCServerBase&         sdrvLevi
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_Switch
        //      We just need one boolean field to represent the switch.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Switch;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreValue
        (
            const   tCIDLib::TCard4         c4Val
            ,       TCQCServerBase&         sdrvLevi
        );


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWBinSwitch,TZWaveUnit)
        DefPolyDup(TZWBinSwitch)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWMLSwitch
//  PREFIX: unit
//
//  This class represents a generic multi-level switch unit. We just need to
//  have storage for our one field id, which is the card field that we use
//  to control the switch.
// ---------------------------------------------------------------------------
class ZWLEVISHEXPORT TZWMLSwitch : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWMLSwitch();

        TZWMLSwitch
        (
            const   tCIDLib::TCard4         c4Id
        );

        TZWMLSwitch
        (
            const   TZWMLSwitch&            unitToCopy
        );

        ~TZWMLSwitch();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWMLSwitch& operator=
        (
            const   TZWMLSwitch&            unitToAssign
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWMLSwitch&            unitSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWMLSwitch&            unitSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBuildSetMsg
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            ,       TString&                strToFill
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TBoolean bStoreBasicReport
        (
            const   tCIDLib::TCard4         c4Value
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TBoolean bStoreFldIds
        (
            const   TVector<TCQCFldDef>&    colFlds
            , const tZWaveLeviSh::TValList& fcolIds
        );

        tCIDLib::TBoolean bStoreGenReport
        (
            const   tCIDLib::TCard4         ac4Values[]
            , const tCIDLib::TCard4         c4ValCnt
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TBoolean bStoreSceneReport
        (
            const   tCIDLib::TCard4         c4Scene
            , const tCIDLib::TCard4         c4Levle
            , const tCIDLib::TCard4         c4FadeTime
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TBoolean bWantsOptInfo
        (
                    tCIDLib::TStrCollect&   colToFill
        )   const;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    TVector<TCQCFldDef>&    colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        tCIDLib::TVoid PrepForUse();

        tCIDLib::TVoid SetFieldErrs
        (
                    TCQCServerBase&         sdrvLevi
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_Switch
        //      The main field contains the dimmer level. We have a secondary
        //      field that can be used just for on/off state, so that they
        //      do have to use the more general text command field to do that.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Level;
        tCIDLib::TCard4     m_c4FldId_State;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreValue
        (
            const   tCIDLib::TCard4         c4Val
            ,       TCQCServerBase&         sdrvLevi
        );


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWMLSwitch,TZWaveUnit)
        DefPolyDup(TZWMLSwitch)
};


#pragma CIDLIB_POPPACK

