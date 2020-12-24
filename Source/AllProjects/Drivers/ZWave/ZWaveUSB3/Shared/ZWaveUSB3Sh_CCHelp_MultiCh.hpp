//
// FILE NAME: ZWaveUSB3Sh_CCHelp_MultiCh.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/24/2018
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
//  This is a CC helper namespace to provide some grunt work helpers for the
//  multi-channel command class. We also provide a little class used to return
//  end point information.
//
//  These helper namespaces are for utility type CCs, not for those that implement
//  actual automation functionality, or to implement the utility aspects of CCs that
//  may also be implemented as a CCImpl type class.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3EndPntInfo
//  PREFIX: zwepi
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWUSB3EndPntInfo : public TObject, public MStreamable
{
    public :
        // ---------------------------------------------------------------------
        //  Constructors and Destructor
        // ---------------------------------------------------------------------
        TZWUSB3EndPntInfo();

        TZWUSB3EndPntInfo
        (
            const   TZWUSB3EndPntInfo&  zwepiSrc
        );

        ~TZWUSB3EndPntInfo();


        // ---------------------------------------------------------------------
        //  Public operators
        // ---------------------------------------------------------------------
        TZWUSB3EndPntInfo& operator=
        (
            const   TZWUSB3EndPntInfo&  zwepiSrc
        );


        // ---------------------------------------------------------------------
        //  Public, non-virtual methods
        // ---------------------------------------------------------------------
        tCIDLib::TVoid Reset();


        // ---------------------------------------------------------------------
        //  Public data members
        // ---------------------------------------------------------------------
        tCIDLib::TBoolean           m_bDynamic;
        tCIDLib::TCard1             m_c1Generic;
        tCIDLib::TCard1             m_c1Specific;
        tCIDLib::TCard4             m_c4EndPntId;
        tZWaveUSB3Sh::TClassList    m_fcolClasses;


    protected :
        // ---------------------------------------------------------------------
        //  Protected, inherited methods
        // ---------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const   override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3EndPntInfo, TObject)
};


// ---------------------------------------------------------------------------
//   NAMESPACE: TZWUSB3CCMultiCh
// ---------------------------------------------------------------------------
namespace TZWUSB3CCMultiCh
{
    // Get the number of end points implemented by this device
    ZWUSB3SHEXPORT tCIDLib::TBoolean bQueryEPCount
    (
                TZWUnitInfo&                unitiTar
        ,       tCIDLib::TCard4&            c4ToFill
        , const tCIDLib::TBoolean           bAsync
        , const tCIDLib::TBoolean           bKnownAwake
        ,       TString&                    strErrMsg
    );

    ZWUSB3SHEXPORT tCIDLib::TBoolean bQueryEndPoint
    (
                TZWUnitInfo&                unitiTar
        , const tCIDLib::TCard4&            c4EPToQuery
        , const tCIDLib::TBoolean           bAsync
        , const tCIDLib::TBoolean           bKnownAwake
        ,       TZWUSB3EndPntInfo&          zwepiToFill
        ,       TString&                    strErrMsg
    );
}

#pragma CIDLIB_POPPACK

