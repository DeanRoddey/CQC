//
// FILE NAME: CQCGKit_DrvInfoDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/07/2016
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
//  This is the header for a simple, generic driver info dialog. It shows the config
//  info for a driver, given the moniker, and it also polls and displays stats from
//  the driver. So this only works on a live, loaded driver of course.
//
//  The configu info tab is a public, reuable tab defined in this facility which we
//  use as well. The stats tab, for now at least, is just an internal detail of
//  ours.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TDrvInfoDlg
// PREFIX: dlg
// ---------------------------------------------------------------------------
class TDrvInfoDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TDrvInfoDlg() = delete;

        TDrvInfoDlg
        (
            const   TCQCSecToken&           sectUser
        );

        TDrvInfoDlg(const TDrvInfoDlg&) = delete;
        TDrvInfoDlg(TDrvInfoDlg&&) = delete;

        ~TDrvInfoDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDrvInfoDlg& operator=(const TDrvInfoDlg&) = delete;
        TDrvInfoDlg& operator=(TDrvInfoDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strMoniker
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_orbcDrv
        //      We have to set up a client proxy for the driver's host. We do this up front
        //      so we can report an error and let them know that stats won't be available.
        //
        //  m_cqcdcInfo
        //      We query the driver's config info upon entry and store it here for later
        //      loading up once we get the info tab created. We don't ever need to refresh
        //      it again.
        //
        //  m_pwndClose
        //      This one we get from the dialog resource we load.
        //
        //  m_pwndTabs
        //      This comes from the dialog resource we load, and holds our two tab windows.
        //
        //  m_sectUser
        //      We need a security token to get the driver config info
        //
        //  m_strMoniker
        //      The moniker of the driver we are targeting.
        // -------------------------------------------------------------------
        tCQCKit::TCQCSrvProxy   m_orbcDrv;
        TCQCDriverObjCfg        m_cqcdcInfo;
        TPushButton*            m_pwndClose;
        TTabbedWnd*             m_pwndTabs;
        TCQCSecToken            m_sectUser;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDrvInfoDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



