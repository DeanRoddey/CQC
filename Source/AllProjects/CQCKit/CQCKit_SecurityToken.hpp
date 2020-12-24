//
// FILE NAME: CQCKit_SecurityToken.hpp
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
//  This is the header for the CQCKit_SecurityToken.cpp file, which
//  implements a helper class used in security transactions. Client apps
//  get a security token during the login process. This object must then be
//  presented back in certain remote operations, to gain access to sensitive
//  operations.
//
//  The data is maintained in encrypted format. This prevents faking of
//  security tokens. The key is only known to the security server, is random,
//  and changes periodically. They are also time stamped to avoid capture and
//  replay. The client app will be given the invalidation time at login, and
//  must re-login again after that time has expired, getting a new token.
//  This is done without bothering the user, just using already known
//  information.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCSecToken
//  PREFIX: secc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCSecToken : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSecToken();

        TCQCSecToken
        (
            const   TCQCSecToken&           seccSrc
        );

        ~TCQCSecToken();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSecToken& operator=
        (
            const   TCQCSecToken&           seccSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4Bytes() const
        {
            return m_c4Count;
        }

        const THeapBuf& mbufData() const;

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid Set
        (
            const   TMemBuf&                mbufToSet
            , const tCIDLib::TCard4         c4Bytes
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Count
        //      The count of bytes in the data buffer.
        //
        //  m_mbufData
        //      The Token data. Its encrypted so we can't store individual
        //      fields. We just store the encrypted buffer.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4Count;
        THeapBuf                m_mbufData;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSecToken,TObject)
};

#pragma CIDLIB_POPPACK


