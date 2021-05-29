//
// FILE NAME: CQCDataSrv_SecurityChallenge.cpp
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
//  This file implements our simple security challenge item class, which we
//  use to remember challenge data we gave back to the client, so that we
//  can look it back up when they come back to us with the decoded challenge
//  data.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCDataSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic Securitys
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSrvSecChallenge,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCDataSrv_SecurityChallenge
    {
        // -----------------------------------------------------------------------
        //  The stale out time for challenges. We pre-calc it so that we can
        //  quickly calculate the stale time for new challenges.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TEncodedTime enctStalePeriod = kCIDLib::enctOneSecond * 5;
    }
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCSrvSecChallenge
//  PREFIX: ssecc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSrvSecChallenge: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The security server calls this when it gets a reply to a security challenge
//  that was outstanding. It verififes the pass challenge data.
//
//  If the client also asked that we provide him with some session data, we
//  do that as well. We will encrypt it with the challenge key for him.
//
tCIDLib::TBoolean
TCQCSrvSecChallenge::bValidateResponse(const TCQCSecChallenge& seccToCheck) const
{
    // Use the original key to decrypt the challenge buffer
    THeapBuf mbufDec(1024UL);
    TBlowfishEncrypter crypDec(m_ckeyChallenge);

    const tCIDLib::TCard4 c4Count = crypDec.c4Decrypt
    (
        seccToCheck.mbufChallenge(), mbufDec, seccToCheck.c4ChallengeBytes()
    );

    //
    //  Ok, this should be the raw bytes of the decrypted hash. So lets create
    //  a hash object from it and compare to our original.
    //
    TMD5Hash mhashTest(mbufDec.pc1Data(), c4Count);
    return (mhashTest == m_mhashChallenge);
}


// Return the stale time for this challenge
tCIDLib::TEncodedTime TCQCSrvSecChallenge::enctStale() const
{
    return m_enctStale;
}


tCIDLib::TVoid
TCQCSrvSecChallenge::GenerateChallenge( const   TCQCUserAccount&    uaccToUse
                                        ,       TCQCSecChallenge&   seccToFill)
{
    //
    //  Set the stale out time stamp for this one. Its now plus the stale
    //  period that we precalc.
    //
    m_enctStale = TTime::enctNow()
                  + CQCDataSrv_SecurityChallenge::enctStalePeriod;

    //
    //  Generate the challenge data. He stores the key and challenge data,
    //  and gives us back a copy also, filling in the passed parms.
    //
    seccToFill.GenData
    (
        uaccToUse.mhashPassword(), m_ckeyChallenge, m_mhashChallenge
    );

    //
    //  Store the user login name so we can look it back up later. And store
    //  the hash id that the challenge object set on itself.
    //
    m_strLoginName = uaccToUse.strLoginName();
    m_mhashId      = seccToFill.mhashId();
}


const TString& TCQCSrvSecChallenge::strLoginName() const
{
    return m_strLoginName;
}


