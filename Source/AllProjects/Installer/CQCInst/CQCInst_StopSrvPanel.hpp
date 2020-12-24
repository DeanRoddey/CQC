//
// FILE NAME: CQCInst_StopSrvPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/16/2005
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
//  This is the header for the info panel derived class that gives the user
//  a chance to back out. If they press next, our validation pass will try to
//  stop the service. If it works, we move on to the next steps. This guy
//  is only visible if this is an upgrade (i.e. an existing system is in
//  place, and the service appears to be running.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstStopSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstStopSrvPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstStopSrvPanel() = delete;

        TInstStopSrvPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstStopSrvPanel(const TInstStopSrvPanel&) = delete;
        TInstStopSrvPanel(TInstStopSrvPanel&&) = delete;

        ~TInstStopSrvPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstStopSrvPanel& operator=(const TInstStopSrvPanel&) = delete;
        TInstStopSrvPanel& operator=(TInstStopSrvPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPanelIsVisible() const final;

        tCIDLib::TBoolean bValidate
        (
                    tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid Entered() final;


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
        //  Private class types
        // -------------------------------------------------------------------
        enum class EResults
        {
            Idle
            , Waiting
            , Stopped
            , DoneWithError
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eRunThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eStatus
        //      The timer watches this and the background thread updates it
        //      when it either succeeds or fails.
        //
        //  m_enctStart
        //      We just move the progress bar across such that it would go all
        //      the way in 30 seconds, which is how long the background thread
        //      will wait for the service to stop. So we remember the start time
        //      and the timer handler will get teh current time and adjust
        //      the progress bar accordingly.
        //
        //  m_pwndXXX
        //      Some typed pointers to important widgets that we have to
        //      interact with enough to want to avoid type casting. These are
        //      the ones that are part of the base dialog. The ones that come
        //      and go as pages are swapped in and out are dealt with on a
        //      temporary basis.
        //
        //  m_thrRunner
        //      We do the actual shutdown request on a background thread, so
        //      that the foreground thread stays responsive. It will post a
        //      message when it's done.
        //
        //  m_tmidUpdate
        //      We start up a timer to keep our display info up to date, watching
        //      what the bgn thread is doing.
        // -------------------------------------------------------------------
        EResults                m_eStatus;
        tCIDLib::TEncodedTime   m_enctStart;
        TProgressBar*           m_pwndProgress;
        TThread                 m_thrRunner;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstStopSrvPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK

