//
// FILE NAME: GenProtoS_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//   CLASS: TFacGenProtoS
//  PREFIX: fac
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TFacGenProtoS : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tGenProtoS::ETypes eFldTypeToExprType
        (
            const   tCQCKit::EFldTypes      eFldType
        );

        static tGenProtoS::ETypes eTokenToType
        (
            const   tGenProtoS::ETokens     eToken
        );

        static tGenProtoS::ETokens eTypeToToken
        (
            const   tGenProtoS::ETypes      eType
        );

        static tGenProtoS::ETypes eXlatTypeName
        (
            const   TString&                strToXlat
        );

        static const TString& strXlatType
        (
            const   tGenProtoS::ETypes      eType
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacGenProtoS();

        ~TFacGenProtoS();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TFacGenProtoS(const TFacGenProtoS&);
        tCIDLib::TVoid operator=(const TFacGenProtoS&);


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacGenProtoS,TFacility)
};

#pragma CIDLIB_POPPACK


