//
// FILE NAME: ZWaveLevi2Sh_SwitchDim.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/31/2014
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
//  This is the header file for the ZWaveLevi2Sh_SwitchDim.cpp file, which implements
//  the TZWCmdClass derivatives for binary and multi-level switches. These are fairly
//  simple ones.
//
//  See the parent class for details.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCBinSwitch
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCBinSwitch : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCBinSwitch
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCBinSwitch();


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

        tCIDLib::TVoid LoadOptVals
        (
            const   TZWDevClass&            zwdcInst
            ,       tZWaveLevi2Sh::TOptValList& colToFill
            ,       TZWOptionVal&           zwoptvTmp
        )   const;

        tCIDLib::TVoid LoadOptList
        (
                    tZWaveLevi2Sh::TOptList& colToFill
        )   const;

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
        TZWCCBinSwitch();
        TZWCCBinSwitch(const TZWCCBinSwitch&);
        tCIDLib::TVoid operator=(const TZWCCBinSwitch&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreValue
        (
            const   tCIDLib::TCard4         c4Val
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_State
        //      The field id for the off/off state of the switch.
        //
        //  m_strOptKey_xxx
        //      We pre-build our option keys, to avoid a lot of redundant and
        //      possibly error prone busy work.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_State;
        TString             m_strOptKey_Trigs;
        TString             m_strOptKey_Type;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCBinSwitch, TZWCmdClass)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCMLSwitch
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCMLSwitch : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCMLSwitch
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCMLSwitch();


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

        tCQCKit::ECommResults eWriteCardFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            ,       TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TVoid InitVals
        (
            const   TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TVoid LoadOptVals
        (
            const   TZWDevClass&            zwdcInst
            ,       tZWaveLevi2Sh::TOptValList& colToFill
            ,       TZWOptionVal&           zwoptvTmp
        )   const;

        tCIDLib::TVoid LoadOptList
        (
                    tZWaveLevi2Sh::TOptList& colToFill
        )   const;

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
        TZWCCMLSwitch();
        TZWCCMLSwitch(const TZWCCMLSwitch&);
        tCIDLib::TVoid operator=(const TZWCCMLSwitch&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ScaleFromZW
        (
            const   tCIDLib::TCard4         c4ToScale
        );

        tCIDLib::TCard4 c4ScaleToZW
        (
            const   tCIDLib::TCard4         c4ToScale
        );

        tCIDLib::TVoid StoreValue
        (
            const   tCIDLib::TCard4         c4Value
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_Level
        //      The field id for the level of the switch.
        //
        //  m_c4FldId_State
        //      The field id for the off/on state of the switch.
        //
        //  m_strOptKey_xxx
        //      We pre-build our option keys, to avoid a lot of redundant and
        //      possibly error prone busy work.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Level;
        tCIDLib::TCard4     m_c4FldId_State;
        TString             m_strOptKey_Trigs;
        TString             m_strOptKey_Type;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCMLSwitch, TZWCmdClass)
};

#pragma CIDLIB_POPPACK

