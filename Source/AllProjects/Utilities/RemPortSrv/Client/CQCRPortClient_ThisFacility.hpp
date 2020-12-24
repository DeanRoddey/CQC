//
// FILE NAME: CQCRPortClient_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/13/2005
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


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIGKit
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCRPORTCEXPORT TFacCQCRPortClient : public TFacility
{
    public :

        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCRPortClient();

        ~TFacCQCRPortClient();


        // -------------------------------------------------------------------
        //  Public, non-virtual method
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParsePath
        (
            const   TString&                strPath
            ,       tCIDLib::TCard4&        c4PortNum
        )   const;

        tRPortClient::TChatClient orbcMakeChatProxy
        (
            const   TIPEndPoint&            ipepSrv
        );

        tRPortClient::TSerialPortClient orbcMakePortProxy
        (
            const   TIPEndPoint&            ipepSrv
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TFacCQCRPortClient(const TFacCQCRPortClient&);
        tCIDLib::TVoid operator=(const TFacCQCRPortClient&);


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCRPortClient,TFacility)
};

#pragma CIDLIB_POPPACK


