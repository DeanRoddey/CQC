//
// FILE NAME: LexMC1CS_Msg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2000
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
//  This is is the header for the LexMC1S_Msg.cpp file, which implements
//  a small class which represents Lexicon messages. It simplifies dealing
//  with messages, and also insures that things are done correctly.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLexMsg
//  PREFIX: lm
// ---------------------------------------------------------------------------
class TLexMsg
{
    public :
        // -------------------------------------------------------------------
        //  Local types
        //
        //  EMsgResults
        //      This is returned from the read method, to indicate the basic
        //      status of the read.
        // -------------------------------------------------------------------
        enum EMsgResults
        {
            EMsgResult_Ok
            , EMsgResult_NoReply
            , EMsgResult_MalformedReply
            , EMsgResult_Nack
        };


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const tCIDLib::TCh* pszXlatResult();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLexMsg();

        TLexMsg
        (
            const   TLexMsg&                lmToCopy
        );

        TLexMsg
        (
            const   tLexMC1S::ECmds         eCmd
            , const tCIDLib::TCard1* const  pc1Data = 0
            , const tCIDLib::TCard1         c1Bytes = 0
        );

        ~TLexMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TLexMsg& operator=
        (
            const   TLexMsg&                lmToAssign
        );

        tCIDLib::TCard1 operator[]
        (
            const   tCIDLib::TCard4         c4Index
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddDataByte
        (
            const   tCIDLib::TCard1         c1ToAdd
        );

        tCIDLib::TVoid AckOn
        (
                    TCommPortBase&          commTarget
        )   const;

        tCIDLib::TBoolean bIsReply() const;

        tCIDLib::TBoolean bIsReplyTo
        (
            const   TLexMsg&                lmToTest
        )   const;

        tCIDLib::TCard4 c4DataBytes() const;

        tCIDLib::TVoid Clear();

        tLexMC1S::ECmds eCmd() const;

        tLexMC1S::ECmds eCmd
        (
            const   tLexMC1S::ECmds         eCmd
        );

        EMsgResults eReadFrom
        (
                    TCommPortBase&          commSrc
            ,       tLexMC1S::EErrors&      eToFill
            , const tCIDLib::TCard4         c4ReqEndTime
        );

        const tCIDLib::TCard1* pc1Data() const;

        const tCIDLib::TCard1* pc1DataAt
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TVoid SendOn
        (
                    TCommPortBase&          commTarget
            , const tCIDLib::TCard4         c4Timeout = 250
        );

        tCIDLib::TVoid SetAll
        (
            const   tLexMC1S::ECmds         eCmd
            , const tCIDLib::TCard1* const  pc1Data
            , const tCIDLib::TCard1         c1Bytes
        );

        tCIDLib::TVoid SetData
        (
            const   tCIDLib::TCard1* const  pc1Data
            , const tCIDLib::TCard1         c1Bytes
        );


    private :
        // -------------------------------------------------------------------
        //  A simple structure that makes it easier to deal with Lexicon
        //  messages. It must be byte packed!
        // -------------------------------------------------------------------
        #pragma CIDLIB_PACK(1)
        struct TMsgPacket
        {
            tCIDLib::TCard1     c1SOP;
            tCIDLib::TCard1     c1DLLCount;
            tCIDLib::TCard1     c1Cmd;
            tCIDLib::TCard1     c1DataCount;
            tCIDLib::TCard1     ac1Data[256];
        };
        #pragma CIDLIB_POPPACK


        // -------------------------------------------------------------------
        //  Private, data members
        //
        //  m_mpData
        //      The actual buffer that we manage the message data in.
        // -------------------------------------------------------------------
        TMsgPacket      m_mpData;
};

#pragma CIDLIB_POPPACK


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
inline tCIDLib::TBoolean TLexMsg::bIsReply() const
{
    return ((m_mpData.c1Cmd >= tLexMC1S::ECmds_RepFirst)
        &&  (m_mpData.c1Cmd <= tLexMC1S::ECmds_RepLast));
}

inline tCIDLib::TCard4 TLexMsg::c4DataBytes() const
{
    return m_mpData.c1DataCount;
}

inline tLexMC1S::ECmds TLexMsg::eCmd() const
{
    return tLexMC1S::ECmds(m_mpData.c1Cmd);
}

inline tLexMC1S::ECmds TLexMsg::eCmd(const tLexMC1S::ECmds eCmd)
{
    m_mpData.c1Cmd = tCIDLib::TCard1(eCmd);
    return tLexMC1S::ECmds(m_mpData.c1Cmd);
}

inline const tCIDLib::TCard1* TLexMsg::pc1Data() const
{
    return m_mpData.ac1Data;
}

