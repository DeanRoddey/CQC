//
// FILE NAME: ZWaveLevi2Sh_Battery.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2014
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
//  This is the header file for the ZWaveLevi2Sh_Battery.cpp file, which implements
//  the TZWCmdClass derivatives for the battery command class.
//
//  See the parent class for details.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCBattery
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCBattery : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCBattery
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCBattery();


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
        TZWCCBattery();
        TZWCCBattery(const TZWCCBattery&);
        tCIDLib::TVoid operator=(const TZWCCBattery&);


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
        //  m_c4FldId_Level
        //      The current level value field.
        //
        //  m_strOptKey_xxx
        //      We pre-build our option keys, to avoid a lot of redundant and
        //      possibly error prone busy work. Scale is only added if a temp
        //      sensor.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Level;
        TString             m_strOptKey_Enable;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCBattery, TZWCmdClass)
};

#pragma CIDLIB_POPPACK



