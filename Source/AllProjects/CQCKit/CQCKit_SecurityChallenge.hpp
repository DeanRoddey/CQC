//
// FILE NAME: CQCKit_SecurityChallenge.hpp
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
//  This is the header for the CQCKit_SecurityChallenge.cpp file, which
//  implements a helper class used in login transactions. The client must
//  get a challenge from the security server, validate it with the hash of
//  the user's password, and then send it back in to get a security token.
//
//  This class makes no attempt to obfuscate the mechanism involved. The security
//  comes from the encryption mechanisms used.
//
//  In order to allow for some proxy servers to support encrypted sesssions,
//  this guy also creates a random session key, encrypted the same way as
//  the challenge data. This allows the proxy server to pass on a session
//  key to its clients, which they have to log on successfully before the
//  key will be useful (since it's encrypted with the passed random key
//  that the client can only decrypt if it has the right password hash.)
//
//  The security server doesn't use this session key at this time, it just
//  provides it as a convenience to such proxy servers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCSecChallenge
//  PREFIX: secc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCSecChallenge : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSecChallenge();

        TCQCSecChallenge
        (
            const   TCQCSecChallenge&       seccSrc
        );

        ~TCQCSecChallenge();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSecChallenge& operator=
        (
            const   TCQCSecChallenge&       seccSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ChallengeBytes() const;

        tCIDLib::TCard4 c4KeyBytes() const;

        tCIDLib::TCard4 c4SKeyBytes() const;

        tCIDLib::TVoid GenData
        (
            const   TMD5Hash&               mhashPassword
            ,       TCryptoKey&             ckeySent
            ,       TMD5Hash&               mhashSent
        );

        const THeapBuf& mbufChallenge() const;

        const THeapBuf& mbufKey() const;

        const THeapBuf& mbufSKey() const;

        const TMD5Hash& mhashId() const;

        tCIDLib::TVoid SetValidatedData
        (
            const   TMemBuf&                mbufToSet
            , const tCIDLib::TCard4         c4Bytes
        );

        tCIDLib::TVoid SetValidatedData
        (
            const   tCIDLib::TCard1* const  pc1ToSet
            , const tCIDLib::TCard4         c4Bytes
        );

        tCIDLib::TVoid Validate
        (
            const   TMD5Hash&               mhashPassword
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
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
        //  Private data members
        //
        //  m_c4ChallengeBytes
        //  m_c4KeyBytes
        //  m_c4SKeyBytes
        //      To keep up with how many bytes of the buffers are valid.
        //
        //  m_mbufChallenge
        //      The challenge data that is send to the client. It is encrypted
        //      with the user's password hash. It must be decrypted and then
        //      re-encrypted with the challenge key.
        //
        //  m_mbufKey
        //      The challenge key. This is encrypted with the user's password
        //      hash. It must decrypted by the client and used to encrypt the
        //      challenge hash.
        //
        //  m_mbufSKey
        //      The session key. This is encrypted with the challenge key
        //      (prior to it's being encrypted with the password hash of course.)
        //      This is provided as a convenience for CQC proxy type servers.
        //
        //  m_mhashId
        //      This is used to be able to identify a challenge after its
        //      returned. Having this in the open does not compromise security
        //      in any way. Its just random data used for house keeping.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4ChallengeBytes;
        tCIDLib::TCard4 m_c4KeyBytes;
        tCIDLib::TCard4 m_c4SKeyBytes;
        THeapBuf        m_mbufChallenge;
        THeapBuf        m_mbufKey;
        THeapBuf        m_mbufSKey;
        TMD5Hash        m_mhashId;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSecChallenge,TObject)
};

#pragma CIDLIB_POPPACK


