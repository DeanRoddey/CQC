//
// FILE NAME: CQCDrvDev_MainFrame.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/14/2003
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
//  This is the header for the CQCDrvDev_MainFrame.cpp file, which implements
//  the TCQCDrvDevFrameWnd. This is the main frame window of the driver
//  development test harness. It basically just wraps the CIDLib macro debugger
//  framework.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TCQCDrvDevFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCDrvDevFrameWnd : public TFrameWnd, public MRemBrwsBrwsCB
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCDrvDevFrameWnd();

        ~TCQCDrvDevFrameWnd();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProcessAccel
        (
            const   tCIDLib::TVoid* const   pMsgData
        )   const override;

        tCIDLib::TVoid TreeBrowserEvent
        (
                    TTreeBrowseInfo&        wnotEvent
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CloseAll();

        tCIDLib::TVoid Create
        (
            const   TCQCFrameState&         fstMain
        );

        tCIDLib::TVoid SaveAll();

        TMacroDbgMainWnd& wndDbgClient();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActivationChange
        (
            const   tCIDLib::TBoolean       bState
            , const tCIDCtrls::TWndHandle   hwndOther
        )   override;

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

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid MenuCommand
        (
            const   tCIDLib::TResId         ridItem
            , const tCIDLib::TBoolean       bChecked
            , const tCIDLib::TBoolean       bEnabled
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckShutdown();

        tCIDLib::TBoolean bCloseCurrent();

        tCIDLib::TVoid DoNewSession();

        tCIDCtrls::EEvResponses eBrowseHandler
        (
                    TTreeBrowseInfo&        wnotBrowser
        );

        tCIDCtrls::EEvResponses eDbgHandler
        (
                    TCMLDbgChangeInfo&      wnotEvent
        );

        tCIDLib::TVoid GetLastPath
        (
                    TString&                strToFill
            , const TString&                strToGet
        );

        tCIDLib::TVoid LoadFile
        (
            const   TTreeBrowseInfo&        wnotBrowser
            , const tCIDLib::TBoolean       bEditMode
        );

        tCIDLib::TVoid PackageDriver
        (
            const   tCIDLib::TBoolean       bEncrypt
        );

        tCIDLib::TVoid SaveState();

        tCIDLib::TVoid SetLastPath
        (
            const   TString&                strToStore
            , const TString&                strToGet
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_accelMain
        //      We need an accellerator table to handle a couple things at our level. All
        //      others are passed down to the debugger window.
        //
        //  m_colSkipWnds
        //      When we query the available client area, we have to skip our own
        //      client window, so we pre-set up the skip list that we pass into the
        //      query.
        //
        //  m_cv2vValidator
        //      A V2 validator object to support validation of driver fields.
        //
        //  m_mecmToUse
        //  m_mefrToUse
        //      The file resolver and class manager that we give to the debugger to use.
        //      We use a simple local disk based file resolver for this work, and use the
        //      CQC provided class manager that reads/writes the data server macro repository.
        //
        //  m_pathCurClass
        //      The class path of the current session class, if any.
        //
        //  m_pfactRemote
        //      If the user has set us up for a remote port, then we register a remote port
        //      factory with CIDComm. We keep a pointer around for our own use and store it
        //      here, but we don't own it. If we aren't configured for a remote port, then
        //      this is null.
        //
        //  m_pwndDbgClient
        //      A pointer to the client window. We keep it sized to our client area.
        //
        //  m_strManifestFile
        //      The name of the manifest file that was opened to starte the current session.
        //      If no session, this is empty.
        //
        //  m_strTitleText
        //      To avoid reloading the text every time we open a new macro for debugging,
        //      we preload it.
        //
        //  m_strTmp
        //      For temporary formatting. Don't assume it keeps its value across any call to
        //      a method of this class.
        //
        //  m_wndChat
        //      The chat window. It's only accessible if the remote port server interface is
        //      enabled. It's a secondary frame so that it can be separately managed. We call
        //      it to let it know about changes in the remote port enable status.
        //
        //  m_wndFldMon
        //      Our field monitor window. We create it when we start up and just leave it
        //      around until we are closed.
        // -------------------------------------------------------------------
        TAccelTable                 m_accelMain;
        tCIDCtrls::TWndList         m_colSkipWnds;
        TCQCV2Validator             m_cv2vValidator;
        TCQCInterClassMgr           m_mecmToUse;
        TMEngFixedBaseFileResolver  m_mefrToUse;
        TPathStr                    m_pathCurClass;
        TCQCRemSrvPortFactory*      m_pfactRemote;
        TMacroDbgMainWnd*           m_pwndDbgClient;
        TString                     m_strManifestFile;
        TString                     m_strTitleText;
        TString                     m_strTmp;
        TCQCChatWnd                 m_wndChat;
        TCQCFldMonWnd               m_wndFldMon;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDrvDevFrameWnd,TFrameWnd)
};

