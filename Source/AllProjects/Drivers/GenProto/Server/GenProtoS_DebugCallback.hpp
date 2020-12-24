//
// FILE NAME: GenProtoS_DebugCallback.hpp
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
//  This header defines a simple mixin interface that allows the generic
//  protocol driver test harness to install a callback via which is can show
//  the message traffic and other important events.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: MGenProtoDebug
//  PREFIX: mdbg
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT MGenProtoDebug
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MGenProtoDebug() {}

        virtual ~MGenProtoDebug() {}


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid Event
        (
            const   TString&                strTitle
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
        ) = 0;

        virtual tCIDLib::TVoid Msg
        (
            const   TString&                strMsg
        ) = 0;

        virtual tCIDLib::TVoid Msg
        (
            const   TString&                strMsg
            , const TString&                strMsg2
        ) = 0;

        virtual tCIDLib::TVoid Msg
        (
            const   TString&                strMsg
            , const TString&                strMsg2
            , const TString&                strMsg3
        ) = 0;

        virtual tCIDLib::TVoid StateTransition
        (
            const   tCIDLib::TCard1         c1ByteReceived
            , const TString&                strAction
            , const TString&                strNewState
        ) = 0;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors
        // -------------------------------------------------------------------
        MGenProtoDebug(const MGenProtoDebug&);
        MGenProtoDebug& operator=(const MGenProtoDebug&);
};

#pragma CIDLIB_POPPACK


