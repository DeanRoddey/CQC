//
// FILE NAME: GC100Ser_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2013
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
//  This file defines facility class for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacGC100Ser
//  PREFIX: fac
// ---------------------------------------------------------------------------
class GC100SEREXPORT TFacGC100Ser : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacGC100Ser();

        TFacGC100Ser(const TFacGC100Ser&) = delete;
        TFacGC100Ser(TFacGC100Ser&&) = delete;

        ~TFacGC100Ser();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TFacGC100Ser&) = delete;
        tCIDLib::TVoid operator=(TFacGC100Ser&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParsePath
        (
            const   TString&                strPath
            ,       TString&                strName
            ,       tCIDLib::TCard4&        c4PortNum
        );

        tCIDLib::TBoolean bRefreshFactory
        (
                    TGC100CfgList&          gcclToFill
            ,       tCIDLib::TCard4&        c4SerialNum
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bRegisterFactory
        (
            const   TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bRegisterFactory
        (
            const   TGC100CfgList&          gcclToSet
        );

        tCIDLib::TBoolean bSaveConfig
        (
            const   TGC100CfgList&          gcclNew
            ,       tCIDLib::TCard4&        c4SerialNum
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bUpdateFactory
        (
            const   TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bUpdateFactory
        (
            const   TGC100CfgList&          gcclNew
        );

        tCIDLib::ELoadRes eQueryConfig
        (
                    TGC100CfgList&          gcclToFill
            ,       tCIDLib::TCard4&        c4SerialNum
            , const TCQCSecToken&           sectUser
        );


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacGC100Ser,TFacility)
};

#pragma CIDLIB_POPPACK


