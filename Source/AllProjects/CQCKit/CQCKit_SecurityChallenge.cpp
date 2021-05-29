//
// FILE NAME: CQCKit_SecurityChallenge.cpp
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
//  This is the implementation file for the security challenge class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSecChallenge,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_SecurityChallenge
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        //
        //  Version 1 - Initial version
        //  Version 2 - Added the session key members and their length
        //  Version 3 - Got rid of shared session key
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 3;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCSecChallenge
//  PREFIX: secc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSecChallenge: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSecChallenge::TCQCSecChallenge() :

    m_c4ChallengeBytes(0)
    , m_c4KeyBytes(0)
    , m_c4SKeyBytes(0)
    , m_mbufChallenge(16, 32)
    , m_mbufKey(16, 32)
    , m_mbufSKey(16, 32)
{
}

TCQCSecChallenge::TCQCSecChallenge(const TCQCSecChallenge& seccSrc) :

    m_c4ChallengeBytes(seccSrc.m_c4ChallengeBytes)
    , m_c4KeyBytes(seccSrc.m_c4KeyBytes)
    , m_c4SKeyBytes(seccSrc.m_c4SKeyBytes)
    , m_mbufChallenge(seccSrc.m_mbufChallenge)
    , m_mbufKey(seccSrc.m_mbufKey)
    , m_mbufSKey(seccSrc.m_mbufSKey)
    , m_mhashId(seccSrc.m_mhashId)
{
}

TCQCSecChallenge::~TCQCSecChallenge()
{
}


// ---------------------------------------------------------------------------
//  TCQCSecChallenge: Public operators
// ---------------------------------------------------------------------------
TCQCSecChallenge&
TCQCSecChallenge::operator=(const TCQCSecChallenge& seccSrc)
{
    if (this != &seccSrc)
    {
        m_c4ChallengeBytes  = seccSrc.m_c4ChallengeBytes;
        m_c4KeyBytes        = seccSrc.m_c4KeyBytes;
        m_c4SKeyBytes       = seccSrc.m_c4SKeyBytes;
        m_mbufChallenge     = seccSrc.m_mbufChallenge;
        m_mbufKey           = seccSrc.m_mbufKey;
        m_mbufSKey          = seccSrc.m_mbufSKey;
        m_mhashId           = seccSrc.m_mhashId;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCSecChallenge: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCSecChallenge::c4ChallengeBytes() const
{
    return m_c4ChallengeBytes;
}


tCIDLib::TCard4 TCQCSecChallenge::c4KeyBytes() const
{
    return m_c4KeyBytes;
}


// This is for the session key and second session key buffers
tCIDLib::TCard4 TCQCSecChallenge::c4SKeyBytes() const
{
    return m_c4SKeyBytes;
}


//
//  This generates the challenge data that needs to be sent back to the
//  client. It also generates a session key that can be used in some servers
//  that act as proxies for non-ORB based clients.
//
tCIDLib::TVoid TCQCSecChallenge::GenData(const  TMD5Hash&           mhashPassword
                                        ,       TCryptoKey&         ckeySent
                                        ,       TMD5Hash&           mhashSent)
{
    //
    //  Create a random key and challenge. We put these into the caller's
    //  parms, since he needs to save them, too.
    //
    ckeySent.Set(TUniqueId::mhashMakeId());
    mhashSent = TUniqueId::mhashMakeId();

    //
    //  Create a crypto key from the user's password hash, and a Blowfish
    //  encrypter. Use it to encrypt our two values into their respective
    //  member buffers.
    //
    TCryptoKey ckeyPassword(mhashPassword);
    TBlowfishEncrypter crypPW(ckeyPassword);
    m_c4KeyBytes = crypPW.c4Encrypt
    (
        ckeySent.pc1Buffer(), m_mbufKey, ckeySent.c4Bytes()
    );

    m_c4ChallengeBytes = crypPW.c4Encrypt
    (
        mhashSent.pc1Buffer(), m_mbufChallenge, mhashSent.c4Bytes()
    );

    //
    //  This one the caller doesn't care about, we just store it here.
    //  It's a session key that can be used by some proxy type servers.
    //  It's just another generated key encrypted with the password
    //  hash.
    //
    TCryptoKey ckeySess(TUniqueId::mhashMakeId());
    m_c4SKeyBytes = crypPW.c4Encrypt
    (
        ckeySess.pc1Buffer(), m_mbufSKey, ckeySess.c4Bytes()
    );


    // Store a unique id hash to indentify this challenge data
    m_mhashId = TUniqueId::mhashMakeId();
}


const THeapBuf& TCQCSecChallenge::mbufChallenge() const
{
    return m_mbufChallenge;
}


const THeapBuf& TCQCSecChallenge::mbufKey() const
{
    return m_mbufKey;
}


const THeapBuf& TCQCSecChallenge::mbufSKey() const
{
    return m_mbufSKey;
}


const TMD5Hash& TCQCSecChallenge::mhashId() const
{
    return m_mhashId;
}


tCIDLib::TVoid
TCQCSecChallenge::SetValidatedData( const   TMemBuf&        mbufToSet
                                    , const tCIDLib::TCard4 c4Bytes)
{
    m_c4ChallengeBytes = c4Bytes;
    m_mbufChallenge.CopyIn(mbufToSet, c4Bytes);
}

tCIDLib::TVoid
TCQCSecChallenge::SetValidatedData( const   tCIDLib::TCard1* const  pc1ToSet
                                    , const tCIDLib::TCard4         c4Bytes)
{
    m_c4ChallengeBytes = c4Bytes;
    m_mbufChallenge.CopyIn(pc1ToSet, c4Bytes);
}


//
//  The client side calls this to update the content of the object so that
//  its ready to send back. They must provide the hash of the user's password.
//
tCIDLib::TVoid TCQCSecChallenge::Validate(const TMD5Hash& mhashPassword)
{
    //
    //  Create a crypto key from the user's password hash, and a Blowfish
    //  encrypter. Use it to decrypt our two buffers.
    //
    TCryptoKey ckeyTmp(mhashPassword);
    TBlowfishEncrypter crypPW(ckeyTmp);

    // Decrypt the key bytes and make a crypto key from it
    THeapBuf mbufTmp(128);
    const tCIDLib::TCard4 c4KeyBytes = crypPW.c4Decrypt(m_mbufKey, mbufTmp, m_c4KeyBytes);
    ckeyTmp.Set(mbufTmp, c4KeyBytes);

    // Decrypt the challenge bytes now
    const tCIDLib::TCard4 c4CBytes = crypPW.c4Decrypt
    (
        m_mbufChallenge, mbufTmp, m_c4ChallengeBytes
    );

    //
    //  And now re-encrypt the challenge data into our challenge buffer,
    //  using the passed key.
    //
    TBlowfishEncrypter crypChallenge(ckeyTmp);
    m_c4ChallengeBytes = crypChallenge.c4Encrypt(mbufTmp, m_mbufChallenge, c4CBytes);
}


// ---------------------------------------------------------------------------
//  TCQCSecChallenge: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSecChallenge::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_SecurityChallenge::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_c4ChallengeBytes
                    >> m_mbufChallenge
                    >> m_c4KeyBytes
                    >> m_mbufKey
                    >> m_mhashId;

    //
    //  If pre-Version 2, or indicate no session key. If post version 2 we
    //  don't have the second key, else we read it in and discard it.
    //
    if (c2FmtVersion < 2)
    {
        m_c4SKeyBytes = 0;
    }
     else if (c2FmtVersion > 2)
    {
        strmToReadFrom  >> m_c4SKeyBytes
                        >> m_mbufSKey;
    }
     else
    {
        THeapBuf mbufDiscard;
        strmToReadFrom  >> m_c4SKeyBytes
                        >> m_mbufSKey
                        >> mbufDiscard;
    }

    // And the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCSecChallenge::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_SecurityChallenge::c2FmtVersion
                    << m_c4ChallengeBytes
                    << m_mbufChallenge
                    << m_c4KeyBytes
                    << m_mbufKey
                    << m_mhashId

                    // Version 2 stuff
                    << m_c4SKeyBytes
                    << m_mbufSKey

                    // Version 3 discarded one of the buffers from Version 2

                    << tCIDLib::EStreamMarkers::EndObject;
}


