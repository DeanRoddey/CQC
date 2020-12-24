//
// FILE NAME: CQCRPortSrv_MainFrame.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
//  This is the header for the CQCRPortSrv_MainFrame.cpp file, which provides
//  the main frame for this program.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TMainFrameWnd : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TMainFrameWnd();

        ~TMainFrameWnd();


        // -------------------------------------------------------------------
        // Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Create();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidSrc
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckShutdown();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid SendCurText();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLastConn
        //      The last client connection status our timer stored, so that we
        //      can know when it changes.
        //
        //  m_porbsChat
        //  m_porbsSerial
        //      We register and deregister our server side objects based on
        //      the enable/disable check box. We keep these pointers around
        //      so that we can call the degister method on the ORB and if we
        //      need to call methods on them. We don't own them.
        //
        //  m_pwndXXX
        //      We create some controls to display some of the most important
        //      pieces of status info in our client area. We don't own them,
        //      they will be cleaned up for us during our destruction. We only
        //      keep typed pointers here to the ones we have to interact with.
        //
        //  m_strChatTmp
        //      A temp string that the timer handler uses to read in queued up
        //      chat msgs.
        //
        //  m_strSecKey
        //      The security key the user entered. We use this to generate the
        //      encryption key.
        //
        //  m_tmidUpdate
        //      We need a timer to update the GUI periodically.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLastConn;
        TRemChatImpl*           m_porbsChat;
        TRemSerialImpl*         m_porbsSerial;
        TCheckedMCListWnd*      m_pwndAllowedPorts;
        TGenericWnd*            m_pwndClient;
        TStaticText*            m_pwndConnStatus;
        TPushButton*            m_pwndEnable;
        TTextOutWnd2*           m_pwndMsgList;
        TMultiEdit*             m_pwndNewMsg;
        TEntryField*            m_pwndPort;
        TEntryField*            m_pwndSecKey;
        TPushButton*            m_pwndTransmit;
        TRGBClr                 m_rgbHis;
        TRGBClr                 m_rgbMine;
        TString                 m_strChatTmp;
        TString                 m_strClientConn;
        TString                 m_strClientNotConn;
        TString                 m_strSecKey;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMainFrameWnd,TFrameWnd)
};

#pragma CIDLIB_POPPACK


