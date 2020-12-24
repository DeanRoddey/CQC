//
// FILE NAME: ZWaveLevi2Sh_Sensors.hpp
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
//  This is the header file for the ZWaveLevi2Sh_Sensors.cpp file, which implements
//  the TZWCmdClass derivatives for binary and multi-level sensors.
//
//  See the parent class for details.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCBinSensor
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCBinSensor : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCBinSensor
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCBinSensor();


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
        TZWCCBinSensor();
        TZWCCBinSensor(const TZWCCBinSensor&);
        tCIDLib::TVoid operator=(const TZWCCBinSensor&);


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
        //      The on/off (or active/inactive or whatever it means) field.
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
        RTTIDefs(TZWCCBinSensor, TZWCmdClass)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCMLSensor
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCMLSensor : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCMLSensor
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCMLSensor();


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
        TZWCCMLSensor();
        TZWCCMLSensor(const TZWCCMLSensor&);
        tCIDLib::TVoid operator=(const TZWCCMLSensor&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseReport
        (
            const   tCIDLib::TCardList&     fcolVals
            ,       tCIDLib::TCard1&        c1Type
            ,       tCIDLib::TCard1&        c1Precision
            ,       tCIDLib::TCard1&        c1Scale
            ,       tCIDLib::TCard1&        c1Size
        );

        tCIDLib::TVoid StoreValue
        (
            const   tCIDLib::TCardList&     fcolVals
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_Level
        //      The current level value field.
        //
        //  m_strOptKey_xxx
        //      We pre-build our option keys, to avoid a lot of redundant and
        //      possibly error prone busy work. Scale is only added if a temp
        //      sensor.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Level;
        TString             m_strOptKey_Data;
        TString             m_strOptKey_Scale;
        TString             m_strOptKey_Type;
        TString             m_strOptKey_Units;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCMLSensor, TZWCmdClass)
};

#pragma CIDLIB_POPPACK


