//
// FILE NAME: JAPwrSer_ThisFacility.hpp
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
//   CLASS: TFacJAPwrSer
//  PREFIX: fac
// ---------------------------------------------------------------------------
class JAPWRSEREXPORT TFacJAPwrSer : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacJAPwrSer();

        TFacJAPwrSer(const TFacJAPwrSer&) = delete;

        ~TFacJAPwrSer();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacJAPwrSer& operator=(const TFacJAPwrSer&) = delete;


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
                    TJAPwrCfgList&          japlToFill
            ,       tCIDLib::TCard4&        c4SerialNum
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bRegisterFactory
        (
            const   TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bRegisterFactory
        (
            const   TJAPwrCfgList&          japlToSet
        );

        tCIDLib::TBoolean bSaveConfig
        (
            const   TJAPwrCfgList&          japlNew
            ,       tCIDLib::TCard4&        c4SerialNum
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bUpdateFactory
        (
            const   TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bUpdateFactory
        (
            const   TJAPwrCfgList&          japlNew
        );

        tCIDLib::ELoadRes eQueryConfig
        (
                    TJAPwrCfgList&          japlToFill
            ,       tCIDLib::TCard4&        c4SerialNum
            , const TCQCSecToken&           sectUser
        );


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacJAPwrSer,TFacility)
};

#pragma CIDLIB_POPPACK


