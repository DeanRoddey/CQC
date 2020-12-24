//
// FILE NAME: CQCInst_StopGUIAppsPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/14/2012
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
//  This is the header for the panel that is shown if any GUI apps are seen
//  to be running. We present that list to the user and give him a chance to
//  stop them. We provide a button to try to stop them ourself, or he can
//  stop them.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstStopGUIAppsPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstStopGUIAppsPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstStopGUIAppsPanel() = delete;

        TInstStopGUIAppsPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstStopGUIAppsPanel(const TInstStopGUIAppsPanel&) = delete;
        TInstStopGUIAppsPanel(TInstStopGUIAppsPanel&&) = delete;

        ~TInstStopGUIAppsPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstStopGUIAppsPanel& operator=(const TInstStopGUIAppsPanel&) = delete;
        TInstStopGUIAppsPanel& operator=(TInstStopGUIAppsPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPanelIsVisible() const final;

        tCIDLib::TVoid Entered() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        enum class EResults
        {
            Idle
            , Waiting
            , Done
            , DoneWithError
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFindActiveApps() const;

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eStopGUIAppsThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDCtrls::TWndHandle hwndFindWnd
        (
            const   TKeyValues&             kvalsApp
            ,       TExternalProcess&       extpTar
        );

        tCIDLib::TVoid LoadActiveAppList();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colAppList
        //      We can do this stuff pretty generically by just providing a
        //      couple pieces of info per app we want to deal with. So this
        //      is loaded up during init.
        //
        //  m_colActive
        //      We fill this list with the applications we find are active.
        //
        //  m_eStatus
        //      The timer watches this and the background thread updates it
        //      when it either succeeds or fails.
        //
        //  m_pwndXXX
        //      We get typed pointers to some of our child widgets. We don't
        //      own them, we just want typed pointers to interact with them.
        //
        //  m_thrStopGUIApps
        //      We have to do the work via a background thread. We start it
        //      up on the eStopGUIAppsThread method above.
        // -------------------------------------------------------------------
        TVector<TKeyValues>         m_colAppList;
        mutable tCIDLib::TStrList   m_colActive;
        EResults                    m_eStatus;
        TPushButton*                m_pwndDoIt;
        TListBox*                   m_pwndList;
        TThread                     m_thrStopGUIApps;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstStopGUIAppsPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK



