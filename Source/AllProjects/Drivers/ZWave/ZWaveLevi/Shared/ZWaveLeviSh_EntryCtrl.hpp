//
// FILE NAME: ZWaveLevih_EntryCtrl.hpp
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
//  entry control devices, e.g. locks and such.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWEntryCtrl
//  PREFIX: unit
//
//  This class represents a door lock control unit. We just need to have
//  storage for our one field id, which is the boolean field that we use to
//  control the lock/unlock state.
//
//  These require secure transmissions.
// ---------------------------------------------------------------------------
class ZWLEVISHEXPORT TZWEntryCtrl : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWEntryCtrl();

        TZWEntryCtrl
        (
            const   tCIDLib::TCard4         c4Id
        );

        TZWEntryCtrl
        (
            const   TZWEntryCtrl&           unitToCopy
        );

        ~TZWEntryCtrl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWEntryCtrl& operator=
        (
            const   TZWEntryCtrl&           unitSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWEntryCtrl&           unitSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWEntryCtrl&           unitSrc
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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strType
        (
            const   tCIDLib::TCard4         c4AlarmType
        )   const;

        tCIDLib::TVoid StoreLockState
        (
                    TCQCServerBase&         sdrvLevi
            , const tCIDLib::TBoolean       bState
            , const tCIDLib::TCard4         c4Level
            , const TString&                strType
        );


        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_State
        //      We just need one boolean field to represent the state of the
        //      lock
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4FldId_State;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWEntryCtrl,TZWaveUnit)
        DefPolyDup(TZWEntryCtrl)
};


#pragma CIDLIB_POPPACK

