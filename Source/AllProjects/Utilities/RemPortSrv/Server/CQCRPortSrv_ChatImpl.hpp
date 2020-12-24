//
// FILE NAME: CQCRPortSrv_ChatImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/22/2014
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
//  This is the header for the CQCRPortSrv_ChatImpl.cpp file, which implements
//  our derivative of the server side chat interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  CLASS: TRemChatImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------
class TRemChatImpl : public TRemChatServerBase
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TRemChatImpl
        (
            const   TOrbObjId&              ooidToUse
        );

        ~TRemChatImpl();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bExchange
        (
            const   TString&                strSendMsg
            ,       TString&                strRecMsg
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bActiveClient() const;

        tCIDLib::TBoolean bQueueMsg
        (
            const   TString&                strMsg
        );

        tCIDLib::TBoolean bGetMsg
        (
                    TString&                strMsg
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize();

        tCIDLib::TVoid Terminate();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colInMsgs
        //      The bExchange method (called by the other side) drops incoming
        //      messages into here, on the bottom. The bGetMsg() method will
        //      pull msgs out on the top for the window to display as msgs from
        //      the other side.
        //
        //  m_colOutMsgs
        //      The bQueueMsg method lets the window queue outgoing msg (on the
        //      bottom.) The bExchange method will pull msgs (from the top) to
        //      return to the other side.
        //
        //  m_enctLastMsg
        //      The last time we got a call from the client. Our bActiveClient
        //      method uses this to tell the caller if we have an active client.
        //      It polls at a certain rate. If we don't see any for a conservative
        //      amount over that, he can't be there.
        //
        //  m_enctTimeout
        //      The timeout period when we will consider having lost a client
        //      if we don't see a message in this time frame.
        // -------------------------------------------------------------------
        TDeque<TString>         m_colInMsgs;
        TDeque<TString>         m_colOutMsgs;
        tCIDLib::TEncodedTime   m_enctLastMsg;
        tCIDLib::TEncodedTime   m_enctTimeout;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TRemChatImpl,TRemChatServerBase)
};

