//
// FILE NAME: ZWaveLevih_Thermo.hpp
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
//  thermostats.
//
// CAVEATS/GOTCHAS:
//
//  1)  We require readability, so it's marked in our fixed capabilities
//      flags. It makes no sense to have a thermo that's not readable.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWThermo
//  PREFIX: unit
//
//  This class represents a thermostat. This is one of the more complex unit
//  types since it has a current temp plus setpoints, operating mode, fan
//  mode, etc...
// ---------------------------------------------------------------------------
class ZWLEVISHEXPORT TZWThermo : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWThermo();

        TZWThermo
        (
            const   tCIDLib::TCard4         c4Id
        );

        TZWThermo
        (
            const   TZWThermo&              unitToCopy
        );

        ~TZWThermo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWThermo& operator=
        (
            const   TZWThermo&              unitSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWThermo&              unitSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWThermo&              unitSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBuildGetMsg
        (
                    TString&                strToFill
            , const tCIDLib::TCard4         c4Index
        );

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


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid BuildSPSetMsg
        (
            const   tCIDLib::TCard4         c4SPId
            , const tCIDLib::TBoolean       bCelsius
            , const TString&                strValue
            ,       TString&                strToFill
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_CurTemp
        //  m_c4FldId_FanMode
        //  m_c4FldId_FanState
        //  m_c4FldId_Mode
        //  m_c4FldId_State
        //      We keep track of the current temp, the operational mode set
        //      by the user, the current operating state, and the fan mode
        //      set by the user and the current fan state.
        //
        //      These are all runtime only of course, not persisted.
        //
        //  m_c4FldId_LowSetPnt
        //  m_c4FldId_HighSetPnt
        //      If we are marked as supporting async notifcations, we'll also
        //      create these. Else, we don't support them due to the amount of
        //      polling.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_CurTemp;
        tCIDLib::TCard4     m_c4FldId_FanMode;
        tCIDLib::TCard4     m_c4FldId_FanState;
        tCIDLib::TCard4     m_c4FldId_Mode;
        tCIDLib::TCard4     m_c4FldId_State;

        tCIDLib::TCard4     m_c4FldId_HighSetPnt;
        tCIDLib::TCard4     m_c4FldId_LowSetPnt;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreFanState
        (
            const   tCIDLib::TCard4         c4Val
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TVoid StoreMode
        (
            const   tCIDLib::TCard4         c4Val
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TVoid StoreState
        (
            const   tCIDLib::TCard4         c4Val
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TVoid StoreTemp
        (
            const   tCIDLib::TCard4         ac4Values[]
            , const tCIDLib::TCard4         c4ValCnt
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TVoid StoreSP
        (
            const   tCIDLib::TCard4         ac4Values[]
            , const tCIDLib::TCard4         c4ValCnt
            ,       TCQCServerBase&         sdrvLevi
        );


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWThermo,TZWaveUnit)
        DefPolyDup(TZWThermo)
};


#pragma CIDLIB_POPPACK

