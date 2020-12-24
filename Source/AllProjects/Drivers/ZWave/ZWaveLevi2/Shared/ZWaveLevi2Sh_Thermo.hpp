//
// FILE NAME: ZWaveLevi2Sh_Thermo.hpp
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
//  This is the header file for the ZWaveLevi2Sh_Thermo.cpp file, which implements
//  the TZWCChermo derivative for thermostats.
//
//  See the parent class for details.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoCTemp
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCThermoCTemp : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCThermoCTemp
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCThermoCTemp();


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
        TZWCCThermoCTemp();
        TZWCCThermoCTemp(const TZWCCThermoCTemp&);
        tCIDLib::TVoid operator=(const TZWCCThermoCTemp&);


        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_XXX
        //      The various fields we provide.
        //
        //  m_strOptKey_xxx
        //      We prebuild our option keys to avoid errors and duplicate code
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_CurTemp;
        TString             m_strOptKey_Scale;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreTemp
        (
            const   tCIDLib::TCardList&     fcol4Values
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCThermoCTemp, TZWCmdClass)
};




// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoFM
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCThermoFM : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCThermoFM
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCThermoFM();


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

        tCQCKit::ECommResults eWriteStringFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strValue
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
        TZWCCThermoFM();
        TZWCCThermoFM(const TZWCCThermoFM&);
        tCIDLib::TVoid operator=(const TZWCCThermoFM&);


        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_FanMode
        //      The fan mode field.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_FanMode;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreFanMode
        (
            const   tCIDLib::TCard4         c4Val
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCThermoFM, TZWCmdClass)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoFOM
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCThermoFOM : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCThermoFOM
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCThermoFOM();


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
        TZWCCThermoFOM();
        TZWCCThermoFOM(const TZWCCThermoFOM&);
        tCIDLib::TVoid operator=(const TZWCCThermoFOM&);


        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_FanOpMode
        //      The fan operating mode field.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_FanOpMode;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreFanOpMode
        (
            const   tCIDLib::TCard4         c4Val
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCThermoFOM, TZWCmdClass)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoSP
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCThermoSP : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCThermoSP
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCThermoSP();


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

        tCQCKit::ECommResults eWriteIntFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4Value
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
        TZWCCThermoSP();
        TZWCCThermoSP(const TZWCCThermoSP&);
        tCIDLib::TVoid operator=(const TZWCCThermoSP&);


        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_HighSetPnt
        //  m_c4FldId_LowSetPnt
        //      The high and low set point fields
        //
        //  m_strOptKey_xxx
        //      We prebuild our option keys to avoid errors and duplicate code
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_HighSetPnt;
        tCIDLib::TCard4     m_c4FldId_LowSetPnt;
        TString             m_strOptKey_Scale;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreSP
        (
            const   tCIDLib::TCardList&     fcol4Values
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCThermoSP, TZWCmdClass)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoMode
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCThermoMode : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCThermoMode
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCThermoMode();


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

        tCQCKit::ECommResults eWriteStringFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strValue
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
        TZWCCThermoMode();
        TZWCCThermoMode(const TZWCCThermoMode&);
        tCIDLib::TVoid operator=(const TZWCCThermoMode&);


        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_Mode
        //      The thermo mode field.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Mode;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreMode
        (
            const   tCIDLib::TCard4         c4Val
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCThermoMode, TZWCmdClass)
};




// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoOMode
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCThermoOMode : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCThermoOMode
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCThermoOMode();


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
        TZWCCThermoOMode();
        TZWCCThermoOMode(const TZWCCThermoOMode&);
        tCIDLib::TVoid operator=(const TZWCCThermoOMode&);


        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_OpMode
        //      The thermo operating mode field.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_OpMode;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreOpMode
        (
            const   tCIDLib::TCard4         c4Val
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCThermoOMode, TZWCmdClass)
};

#pragma CIDLIB_POPPACK

