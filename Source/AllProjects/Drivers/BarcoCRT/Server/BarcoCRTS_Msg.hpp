//
// FILE NAME: BarcoCRTS_Msg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2002
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
//  Since theres a bit of futzing about required with Barco messages, we have
//  a separate class to encapsulate that work, and to insure that it gets done
//  right all the time.
//
// CAVEATS/GOTCHAS:
//
//  1)  This is just a small helper method, so it doesn't implement any of
//      the standard CIDLib RTTI macros.
//
// LOG:
//
//  $Log$
//

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBarcoMsg
//  PREFIX: msg
// ---------------------------------------------------------------------------
class TBarcoMsg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBarcoMsg();

        TBarcoMsg
        (
            const   tCIDLib::TCard1         c1Address
            , const tBarcoCRTS::ECmds       eCmd
        );

        TBarcoMsg
        (
            const   tCIDLib::TCard1         c1Address
            , const tBarcoCRTS::ECmds       eCmd
            , const tCIDLib::TCard1         c1Data1
            , const tCIDLib::TCard1         c1Data2 = 0
            , const tCIDLib::TCard1         c1Data3 = 0
            , const tCIDLib::TCard1         c1Data4 = 0
        );

        TBarcoMsg
        (
            const   TBarcoMsg&              msgToCopy
        );

        ~TBarcoMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBarcoMsg& operator=
        (
            const   TBarcoMsg&              msgToAssign
        );

        tCIDLib::TCard1 operator[]
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TCard1& operator[]
        (
            const   tCIDLib::TCard4         c4Index
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsReplyTo
        (
            const   TBarcoMsg&              msgToCheck
        )   const;

        tCIDLib::TBoolean bSendOn
        (
                    TCommPortBase* const    pcommTarget
        );

        tCIDLib::TVoid Clear();

        tBarcoCRTS::ECmds eCmd() const;

        tBarcoCRTS::EMsgResults eReadShortFrom
        (
                    TCommPortBase* const    pcommTarget
            , const tCIDLib::TCard4         c4WaitMillis
        );

        tCIDLib::TVoid SendOn
        (
                    TCommPortBase* const    pcommTarget
        );

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TCard1         c1Address
            , const tBarcoCRTS::ECmds       eCmd
        );

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TCard1         c1Address
            , const tBarcoCRTS::ECmds       eCmd
            , const tCIDLib::TCard1         c1Data1
            , const tCIDLib::TCard1         c1Data2 = 0
            , const tCIDLib::TCard1         c1Data3 = 0
            , const tCIDLib::TCard1         c1Data4 = 0
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard1 c1XlatCmd
        (
            const   tBarcoCRTS::ECmds       eToXlat
        )   const;

        tBarcoCRTS::ECmds eXlatCmd
        (
            const   tCIDLib::TCard1         c1ToFind
        )   const;

        tCIDLib::TVoid MakeXlated();

        const tCIDLib::TCh* pszXlatCmd
        (
            const   tBarcoCRTS::ECmds       eCmd
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_ac1Msg
        //      This is the message buffer. Its allocated to 255 bytes, which
        //      is guaranteed to be large enough for any message.
        //
        //  m_ac1Xlated
        //      This is a separate buffer that we use to create the form of
        //      the message that must be sent to the projector, which involves
        //      adding checksum info and add offset info. It is built on the
        //      fly when needed.
        //
        //  m_bChanges
        //      In order to avoid rebuilding the translated version if there
        //      haven't been any changes to the raw data, we set this when we
        //      make changes, and clear it when we build the xlated version.
        //
        //  m_c4Len
        //      The current length of the message.
        //
        //  m_c4XlatedLen
        //      The length of the translated version of the message.
        // -------------------------------------------------------------------
        tCIDLib::TCard1     m_ac1Msg[255];
        tCIDLib::TCard1     m_ac1Xlated[255];
        tCIDLib::TBoolean   m_bChanges;
        tCIDLib::TCard4     m_c4Len;
        tCIDLib::TCard4     m_c4XlatedLen;
};

#pragma CIDLIB_POPPACK

