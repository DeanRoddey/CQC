//
// FILE NAME: CQCEvMonEng_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//   CLASS: TFacCQCEvMonEng
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCEVMONENGEXP TFacCQCEvMonEng : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCEvMonEng();

        TFacCQCEvMonEng(const TFacCQCEvMonEng&) = delete;
        TFacCQCEvMonEng(TFacCQCEvMonEng&&) = delete;

        ~TFacCQCEvMonEng();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCEvMonEng& operator=(const TFacCQCEvMonEng&) = delete;
        TFacCQCEvMonEng& operator=(TFacCQCEvMonEng&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddFields
        (
            const   tCIDLib::TStrList&      colToAdd
            ,       tCQCEvMonEng::TPollList& colToFill
        );

        tCIDLib::TVoid Initialize
        (
                    TCQCPollEngine* const   ppolleToUse
            , const TCQCSecToken&           sectUser
        );

        TCQCPollEngine& polleThis();

        tCIDLib::TVoid Terminate();

        tCIDLib::TVoid WriteField
        (
            const   TString&                strFld
            , const TString&                strValue
        );

        tCIDLib::TVoid WriteField
        (
            const   TString&                strMoniker
            , const TString&                strFld
            , const TString&                strValue
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCMLInit
        //      We could possibly get initailized and terminated multile times, so
        //      we remember if we've already set up our CML classes with the
        //      class loader. This doesn't have to be done again.
        //
        //  m_ppolleMon
        //      Each monitor can register a list of fields it wants to monitor. The
        //      client application has to provide us with a polling engine. THere
        //      are typically various users of such engines, and we don't want to
        //      have redundant ones adding overhead.
        //
        //  m_sectUser
        //      We need credentials to do some things we need to do, so the main server
        //      facility will provide us with one.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bCMLInit;
        TCQCPollEngine*     m_ppolleMon;
        TCQCSecToken        m_sectUser;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCEvMonEng,TFacility)

};

#pragma CIDLIB_POPPACK


