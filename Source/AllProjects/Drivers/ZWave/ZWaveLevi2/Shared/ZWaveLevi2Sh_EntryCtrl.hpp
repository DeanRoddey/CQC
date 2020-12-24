//
// FILE NAME: ZWaveLevi2Sh_EntryCtrl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/01/2014
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
//  This is the header file for the ZWaveLevi2Sh_EntryCtrl.cpp file, which implements
//  the TZWCmdClass derivatives for locks.
//
//  See the parent class for details.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCEntryCtrl
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCEntryCtrl : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCEntryCtrl
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCEntryCtrl();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleGenReport
        (
            const   tCIDLib::TCardList&     fcolVals
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4FromInst
        );

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TBoolean bQueryFldVals
        (
                    TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCQCKit::ECommResults eWriteBoolFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
            ,       TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TVoid InitVals
        (
            const   TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
            , const TZWaveUnit&             unitMe
            ,       TString&                strTmp1
            ,       TString&                strTmp2
        )   const;

        tCIDLib::TVoid StoreFldIds
        (
            const   TZWaveUnit&             unitMe
            , const MZWLeviSrvFuncs&        mzwsfLevi
            ,       TString&                strTmp1
            ,       TString&                strTmp2
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TZWCCEntryCtrl();
        TZWCCEntryCtrl(const TZWCCEntryCtrl&);
        tCIDLib::TVoid operator=(const TZWCCEntryCtrl&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strType
        (
            const   tCIDLib::TCard4         c4AlarmType
        )   const;

        tCIDLib::TVoid StoreLockState
        (
            const   TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TBoolean       bState
            , const tCIDLib::TCard4         c4Level
            , const TString&                strType
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_State
        //      The locked/unlocked state of the device.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_State;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCEntryCtrl, TZWCmdClass)
};


#pragma CIDLIB_POPPACK


