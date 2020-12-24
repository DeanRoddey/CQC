//
// FILE NAME: ZWaveLevih_Sensor.hpp
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
//  This is the header file for the ZWaveLevi_Sensor.cpp file, which
//  implements derivatives of the base Z-Wave unit class which represent
//  binary and multi-level sensors.
//
//  We use the optional info value in the base class to let the user tell us
//  if we are a motion sensor or not. If so, then motion triggers need to be
//  sent on changes. Our two values are 0=no option, 1=motion sensor. So if
//  the optional value is 1, then we area motion sensor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)



// ---------------------------------------------------------------------------
//   CLASS: TZWBinSensor
//  PREFIX: unit
//
//  This class represents an entry control unit, basically locks. We just need
//  to have storage for our one field id, which is the boolean field that we
//  use to control the lock/unlock state.
// ---------------------------------------------------------------------------
class ZWLEVISHEXPORT TZWBinSensor : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWBinSensor();

        TZWBinSensor
        (
            const   tCIDLib::TCard4         c4Id
        );

        TZWBinSensor
        (
            const   TZWBinSensor&           unitToCopy
        );

        ~TZWBinSensor();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWBinSensor& operator=
        (
            const   TZWBinSensor&           unitSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWBinSensor&           unitSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWBinSensor&           unitSrc
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
        //  m_c4FldId_State
        //      We just need one boolean field to represent the state of the
        //      sensor.
        // -------------------------------------------------------------------
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
        RTTIDefs(TZWBinSensor,TZWaveUnit)
        DefPolyDup(TZWBinSensor)
};


#pragma CIDLIB_POPPACK

