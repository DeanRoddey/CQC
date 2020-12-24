//
// FILE NAME: ZWaveLevi2Sh_ColorSwitch.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/31/2017
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
//  This is the header file for the ZWaveLevi2Sh_ColorSwitch.cpp file, which implements
//  the TZWCmdClass derivatives for color based lights. This effectively is mapped to
//  our 'Colored Light' device class, and we expose that interface via this class. We'll
//  also have a binary switch and multi-level switch field, as per the colored light
//  spec, with those being handled by their respective command classes (which this device
//  implements.)
//
//  Since our field has to be in HSV, but we get RGB, we store each of the color components
//  as RGB values. When one of them changes, we have to re-recalculate the HSV value and
//  format it back out to text and store it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCColorSwitch
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCColorSwitch : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCColorSwitch() = delete;

        TZWCCColorSwitch
        (
            const   TZWDevClass&            zwdcInst
        );

        TZWCCColorSwitch(const TZWCCColorSwitch&) = delete;

        ~TZWCCColorSwitch();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWCCColorSwitch& operator=(const TZWCCColorSwitch&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleGenReport
        (
            const   tCIDLib::TCardList&     fcolVals
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4FromInst
        )   override;

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TBoolean bQueryFldVals
        (
                    TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        )   override;

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
        )   override;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
            , const TZWaveUnit&             unitMe
            ,       TString&                strTmp1
            ,       TString&                strTmp2
        )   const override;

        tCIDLib::TVoid StoreFldIds
        (
            const   TZWaveUnit&             unitMe
            , const MZWLeviSrvFuncs&        mzwsfLevi
            ,       TString&                strTmp1
            ,       TString&                strTmp2
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreValue
        (
            const   TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c1Blue
        //  m_c1Green
        //  m_c1Red
        //      We get RGB values but have to store HSV. So we have to keep track of
        //      all of the color components. When we get a report that one changed, we
        //      have to store it here and then call StoreValue() which will conver the
        //      new RGB combination to HSV, format it out, and store the result.
        //
        //  m_c4FldId_Color
        //      The current color field. It's a string field in HSV format, as per
        //      the CQC Colored Light device class.
        // -------------------------------------------------------------------
        tCIDLib::TCard1     m_c1Blue;
        tCIDLib::TCard1     m_c1Green;
        tCIDLib::TCard1     m_c1Red;
        tCIDLib::TCard4     m_c4FldId_Color;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCColorSwitch, TZWCmdClass)
};

#pragma CIDLIB_POPPACK

