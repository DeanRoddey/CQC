//
// FILE NAME: CQCDataSrv_SecurityChallenge.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/01/2002
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
//  This class is related to the TCQCSecChallenge class in CQCKit. The clients
//  only see the challenge object, but we need to track some other info above
//  and beyond what we show to them
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCSrvSecChallenge
//  PREFIX: ssecc
// ---------------------------------------------------------------------------
class TCQCSrvSecChallenge : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TMD5Hash& mhashGetKey(const TCQCSrvSecChallenge& sseccSrc)
        {
            return sseccSrc.m_mhashId;
        }


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSrvSecChallenge() = default;

        TCQCSrvSecChallenge(const TCQCSrvSecChallenge&) = default;
        TCQCSrvSecChallenge(TCQCSrvSecChallenge&&) = default;

        ~TCQCSrvSecChallenge() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSrvSecChallenge& operator=(const TCQCSrvSecChallenge&) = default;
        TCQCSrvSecChallenge& operator=(TCQCSrvSecChallenge&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidateResponse
        (
            const   TCQCSecChallenge&       seccToCheck
        )   const;

        tCIDLib::TEncodedTime enctStale() const;

        tCIDLib::TVoid GenerateChallenge
        (
            const   TCQCUserAccount&        uaccToUse
            ,       TCQCSecChallenge&       seccToFill
        );

        const TString& strLoginName() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_ckeyChallenge
        //      This is the key we encrypted the challenge data with. We have
        //      to keep this around to decrypt the returned challenge data.
        //
        //  m_enctStale
        //      This lets us know when this challenge goes stale, to prevent
        //      capture and playback.
        //
        //  m_mhashChallenge
        //      This is the original challenge data before encryption. This
        //      is not streamed, so the client will never see it. We just
        //      need to have it around to compare with the results that come
        //      back.
        //
        //  m_mhashId
        //      We put this hash id into our object, and into the challenge
        //      object we send back to the client. We use this as a key to
        //      store this object in a list of pending challenges, and to
        //      match it back up with the returned challenge in the second
        //      stage of the login.
        //
        //  m_strLoginName
        //      So we can get back to the user account when the challenge
        //      comes back.
        // -------------------------------------------------------------------
        TCryptoKey              m_ckeyChallenge;
        tCIDLib::TEncodedTime   m_enctStale = 0;
        TMD5Hash                m_mhashChallenge;
        TMD5Hash                m_mhashId;
        TString                 m_strLoginName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSrvSecChallenge,TObject)
};

#pragma CIDLIB_POPPACK


