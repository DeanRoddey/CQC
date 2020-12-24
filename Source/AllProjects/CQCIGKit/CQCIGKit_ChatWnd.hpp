//
// FILE NAME: CQCIGKit_ChatWnd.hpp
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
//  This is the header for the file file CQCIGkit_ChatWnd.cpp file, which implements
//  a window that knows how to talk to the chat portion of our remote port server. This
//  is used in the CML and C++ IDEs.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


class TCQCDrvDevFrameWnd;

// ---------------------------------------------------------------------------
//  CLASS: TCQCChatWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class CQCIGKITEXPORT TCQCChatWnd : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCChatWnd();

        ~TCQCChatWnd();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Create
        (
            const   TWindow&                wndOwner
            , const TString&                strStateKey
        );

        tCIDLib::TVoid DisableChat();

        tCIDLib::TVoid EnableChat
        (
            const   TIPEndPoint&            ipepTar
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActivationChange
        (
            const   tCIDLib::TBoolean       bState
            , const tCIDCtrls::TWndHandle   hwndOther
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eSpoolThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid ResetConnStatus();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colInMsgs
        //  m_colOutMsgs
        //      We have to have a place to store outgoing messages until they can be
        //      sent and incoming msgs until they can be displayed. The chat proxy
        //      only allows us to deal with a msg at a time, and the GUI thread can't
        //      make the remote calls itself. So we have a thread that pulls msgs
        //      out of the out deque and sends them and puts incoming msgs into the
        //      in deque until the GUI thread's time can display them.
        //
        //      These are thread safe so there's no problems with the spool thread
        //      and the GUI thread accessing them.
        //
        //  m_orbcChat
        //      A chat proxy that we use to talk to the server. WE create or
        //      destroy it based on calls from the main window indicating whether
        //      the remote interface is enabled or disabled.
        //
        //  m_pwndXXX
        //      For convenience, we get some typed pointers to some of our
        //      widgets. we don't own them, we just look at them.
        //
        //  m_strStateKey
        //      We are given a local config repo state key to load our initial state from
        //      and to store state changes.
        //
        //  m_thrSpool
        //      This thread is started on eSpoolThread and handles getting and
        //      sending msgs in the background. See m_colInMsgs/m_colOutMsgs.
        //
        //  m_tmidUpdate
        //      We start up a timer to grab incoming msgs and display them.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bCommStatus;
        tCIDLib::TBoolean           m_bLastCommStatus;
        tCIDLib::TBoolean           m_bShowErrs;
        TDeque<TString>             m_colInMsgs;
        TDeque<TString>             m_colOutMsgs;
        tRPortClient::TChatClient   m_orbcChat;
        TPushButton*                m_pwndClearMsgs;
        TPushButton*                m_pwndClose;
        TTextOutWnd2*               m_pwndMsgList;
        TMultiEdit*                 m_pwndNewMsg;
        TStaticText*                m_pwndStatus;
        TPushButton*                m_pwndTransmit;
        TRGBClr                     m_rgbHis;
        TRGBClr                     m_rgbMine;
        const TString               m_strConnText;
        const TString               m_strDisconnText;
        TString                     m_strStateKey;
        TString                     m_strTimerTmp;
        TThread                     m_thrSpool;
        tCIDCtrls::TTimerId         m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCChatWnd,TFrameWnd)
};

