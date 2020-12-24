//
// FILE NAME: CQCInst_StatusPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/02/2004
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
//  This is the header for the info panel derived class that handles showing
//  the installation status as it is progressing.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstStatusPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstStatusPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstStatusPanel() = delete;

        TInstStatusPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstStatusPanel(const TInstStatusPanel&) = delete;
        TInstStatusPanel(TInstStatusPanel&&) = delete;

        ~TInstStatusPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstStatusPanel& operator=(const TInstStatusPanel&) = delete;
        TInstStatusPanel& operator=(TInstStatusPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCQCInst::ENavFlags eNavFlags() const final;

        tCQCInst::EProcRes eProcess() final;

        tCIDLib::TVoid Entered() final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid EnterFailureMode();

        tCIDLib::TVoid PostEvent
        (
            const   tCQCInst::EInstSteps    eStep
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TVoid Destroyed() final;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid WaitInstallThread();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colStatusQ
        //      This is a queue that the install thread will post status msgs to. The
        //      install progress dialog will periodically grab messages from this dialog
        //      and display them. We create it thread safe, so the thread and dialog
        //      don't have to worry about synchronization.
        //
        //  m_eStep
        //      The last step we saw from the background thread. The timer callback
        //      also looks at this to see if it needs to do anything. We initialize it
        //      to Count, which tells the timer callback that nothing is happening.
        //      Once we start the proces it will start looking for items in the status
        //      queue because we set the step to Start and the background thread moves
        //      it up.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with enough not
        //      to want to cast them all the time. We don't own them.
        //
        //  m_strMsg
        //      A string to load messages into
        //
        //  m_thrInstaller
        //      The installer thread that does the actual work in the background,
        //      while posting messages to use for status updates.
        //
        //  m_tmidUpdate
        //      We start up a timer to keep our display info up to date, watching
        //      what the bgn thread is doing.
        // -------------------------------------------------------------------
        tCQCInst::TStatusQ      m_colStatusQ;
        tCQCInst::EInstSteps    m_eStep;
        TProgressBar*           m_pwndMeter;
        TMultiEdit*             m_pwndText;
        TString                 m_strMsg;
        TInstallThread          m_thrInstaller;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstStatusPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


