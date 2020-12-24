//
// FILE NAME: CQCVoice_MainFrame.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/28/2017
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
//  This is the header for the CQCVoice_MainFrame.cpp file, which implements
//  the TCQCVoiceFrameWnd class. This window provides the actual interface. It's
//  only shown when the user clicks on our tray icon.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TCQCVoiceFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCVoiceFrameWnd : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCVoiceFrameWnd();

        ~TCQCVoiceFrameWnd();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Create();

        tCIDLib::TVoid QueueMsg
        (
            const   TString&                strMsg
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid CodeReceived
        (
            const   tCIDLib::TInt4          i4Code
            , const tCIDLib::TCard4         c4Data
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colMsgQ
        //      A thread safe queue used to queue up msgs from the background threads
        //      for display. We post to ourself when a message is added and the queue
        //      is empty. Our handler displays the messages until the queue is empty.
        //
        //  m_colTmpMsgs
        //      A temp string list that we use to parse out lines of msgs that are
        //      queued, which we then pass to the text output window.
        //
        //  m_pwndXXX
        //      We get typed pointers to some of our child controls
        // -------------------------------------------------------------------
        TQueue<TString>     m_colMsgQ;
        tCIDLib::TStrList   m_colTmpMsgs;
        TCheckBox*          m_pwndDebugOutput;
        TCheckBox*          m_pwndDebugVerbose;
        TTextOutWnd*        m_pwndMsgs;
        TPushButton*        m_pwndReload;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCVoiceFrameWnd,TFrameWnd)
};
