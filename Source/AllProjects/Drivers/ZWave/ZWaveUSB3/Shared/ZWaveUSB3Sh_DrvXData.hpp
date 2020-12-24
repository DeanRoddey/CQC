//
// FILE NAME: ZWaveUSB3Sh_DrvXData.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/24/2018
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
//  This is used by the client and server side drivers to exchange non-field oriented
//  data, mostly to support the bSendCmd() backdoor call that the cient users to send
//  most commands to the driver. But also in some queries potentially.
//
//  It's very generic with just some open ended fields. For anything more there is a
//  memory buffer to stream objects into.
//
// CAVEATS/GOTCHAS:
//
//  1)  THE CLIENT HAS to set the status flag to true to get all of the members to
//      be streamed. If it's false, then only the cmd, flag, and msg are streamed,
//      since that's all that's needed for a pure status reply from the server to the
//      client.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWDriverXData
//  PREFIX: zwdxd
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWDriverXData : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWDriverXData();

        TZWDriverXData
        (
            const   TZWDriverXData&         zwdxdSrc
        );

        ~TZWDriverXData();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWDriverXData& operator=
        (
            const   TZWDriverXData&         zwdxdSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset();

        tCIDLib::TVoid StatusReset
        (
            const   tCIDLib::TBoolean       bStatus
        );

        tCIDLib::TVoid StatusReset
        (
            const   tCIDLib::TBoolean       bStatus
            , const TString&                strMsg
        );

        tCIDLib::TVoid StatusReset
        (
            const   tCIDLib::TBoolean       bStatus
            , const TString&                strMsg
            , const MFormattable&           mfmtblToken1
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken3 = MFormattable::Nul_MFormattable()
        );


        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_bAwake
        //      Commands that target a unit can indicate that they know that the unit
        //      is currently awake, to avoid having msgs queued up on the unit's wake
        //      up queue.
        //
        //  m_bFlag1
        //      A puropes flag.
        //
        //  m_bStatus
        //      This indicates whether the reply is a success or error. If true it
        //      was successful, else false.
        //
        //  m_c1EPId
        //      If an end point is involved, this is the end point id. It should be
        //      0xFF if not end point.
        //
        //  m_c1UnitId
        //      If there is a target unit for a command, this is the id. If not used
        //      it will be zero.
        //
        //  m_c1Valx
        //      General purpose bytes.
        //
        //  m_c4BufSz
        //      If m_mbufData has data in it, this is how many bytes.
        //
        //  m_c4ValX
        //      General purpose unsigned values.
        //
        //  m_i4ValX
        //      General purpose signed values.
        //
        //  m_strCmd
        //      The name of the command being requested, or being replied to.
        //
        //  m_strMsg
        //      If m_bStatus is false, this is the error text. Else, if not empty it
        //      is some sort of confirmation or information msg to show to the user.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bAwake;
        tCIDLib::TBoolean       m_bFlag1;
        tCIDLib::TBoolean       m_bStatus;
        tCIDLib::TCard1         m_c1EPId;
        tCIDLib::TCard1         m_c1UnitId;
        tCIDLib::TCard1         m_c1Val1;
        tCIDLib::TCard1         m_c1Val2;
        tCIDLib::TCard4         m_c4Val1;
        tCIDLib::TCard4         m_c4BufSz;
        tCIDLib::TInt4          m_i4Val1;
        THeapBuf                m_mbufData;
        TString                 m_strCmd;
        TString                 m_strMsg;


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
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWDriverXData,TObject)
};

#pragma CIDLIB_POPPACK
