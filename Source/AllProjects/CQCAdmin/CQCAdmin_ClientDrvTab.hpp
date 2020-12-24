//
// FILE NAME: CQCAdmin_ClientDrvTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/18/2016
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
//  This is the generic client driver interface tab. It is fairly minimal and really
//  just hosts a TCQCDriverClient derived window which we keep sized appropriately. We
//  provide just a bit of display, basically the current driver connection status. The
//  rest is the driver content within its own window. It mostly manages its own business.
//
//  Client side drivers are dynamically loadable facilities, which provide us with a
//  factory function to gen up the appropriate type of window. We just look at it via the
//  TCQCDriverClient base class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TClientDrvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TClientDrvTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TClientDrvTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
            , const TString&                strHost
            , const TString&                strMoniker
        );

        TClientDrvTab(const TClientDrvTab&) = delete;
        TClientDrvTab(TClientDrvTab&&) = delete;

        ~TClientDrvTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TClientDrvTab&) = delete;
        tCIDLib::TVoid operator=(TClientDrvTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
            , const TCQCDriverObjCfg&       cqcdcInfo
        );


    protected  :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::ESaveRes eDoSave
        (
                    TString&                strErr
            , const tCQCAdmin::ESaveModes   eMode
            ,       tCIDLib::TBoolean&      bChangedSaved
        )   final;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TVoid Destroyed() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndDriver
        //      The driver window we are managing.
        //
        //  m_pwndStatus
        //      our own window that we use to display the connection status.
        //
        //  m_strHost
        //  m_strMoniker
        //      We pull the host/moniker out of the path that we get in the ctor. We
        //      need these to communicate with the driver.
        // -------------------------------------------------------------------
        TCQCDriverClient*   m_pwndDriver;
        TString             m_strHost;
        TString             m_strMoniker;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TClientDrvTab, TContentTab)
};

