//
// FILE NAME: CppDrvTest_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2018
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
//  This is the header for the CppDrvTest_ThisFacility.cpp file, which implements the
//  facility class for this program. Since this is a GUI based program, it derives from
//  the TGUIFacility class, instead of the base TFacility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCppDrvTest
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCppDrvTest : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCppDrvTest();

        TFacCppDrvTest(const TFacCppDrvTest&) = delete;

        ~TFacCppDrvTest();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCppDrvTest& operator=(const TFacCppDrvTest&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHaveSession() const
        {
            return !m_strManifestFile.bIsEmpty();
        }

        tCIDLib::TBoolean bStartDriver();

        tCIDLib::TBoolean bStopDriver();

        tCIDLib::TBoolean bWriteField
        (
            const   tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FieldId
            , const TString&                strValue
        );

        const TCQCDriverCfg& cqcdcOrg() const
        {
            return m_cqcdcOrg;
        }

        const TCQCDriverObjCfg& cqcdcSession() const
        {
            return m_cqcdcSession;
        }

        const TCQCUserCtx& cuctxToUse() const
        {
            return m_cuctxToUse;
        }

        tCIDLib::TVoid CloseSession();

        tCIDLib::TVoid DoNewSession();

        tCIDLib::TVoid DoV2Validation();

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        TCQCServerBase* psdrvTest();

        const TString& strManifestFile() const
        {
            return m_strManifestFile;
        }

        const TString& strMoniker() const
        {
            return m_cqcdcSession.strMoniker();
        }

        const TString& strResDir() const
        {
            return m_strResDir;
        }

        const TString& strTitleText() const
        {
            return m_strTitleText;
        }

        tCIDLib::TVoid SetNewVerbosity
        (
            const   tCQCKit::EVerboseLvls   eNew
        );


    private :
        // -------------------------------------------------------------------
        //  Private non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCloseCurrent();

        tCIDLib::TBoolean bDoLogon();

        tCIDLib::TBoolean bParseParms
        (
                    tCIDLib::TBoolean&      bIgnoreState
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cqcdcOrg
        //      This is the original manifest file content. This is only valid if
        //      m_strManifestFile is not empty. If it's not, then this is the info we
        //      parsed out of that file.
        //
        //  m_cqcdcSession
        //      This is the original manifest info plus the options set by the user
        //      by the driver install wizard that was run in the session open process.
        //
        //  m_cuctxToUse
        //      The user context for the logged in user, which we have to
        //      pass to various things.
        //
        //  m_eLastVerbosity
        //      The last verbosity the user selected. We set it as the initial level
        //      when starting the driver up next time.
        //
        //  m_ippnListen
        //      The IP port on which we tell the Orb server to listen. If its not given
        //      on the command line, we use a default one not likely to be an issue
        //
        //  m_pfacSrvDrv
        //      When we load the server driver we have to dynamically load the facility
        //      that contains it. We then use it to load up m_psdrvTest. These are kept
        //      around as long as we need the driver.
        //
        //  m_porbsDrvAdmin
        //      We create a 'test interface' version of the CQCServer driver admin
        //      ORB interface. Each time we start the driver, we pass it a pointer to
        //      m_psdrvTest. We start any client driver in test mode and it will connect
        //      to this handler, which will pass calls on to our server side driver.
        //
        //  m_psdrvTest
        //      A pointer to the server side driver. Drivers are of course their own
        //      threads, so we start it and stop it as required. This is dynamically
        //      loaded from a DLL.
        //
        //  m_strManifestFile
        //      The manifest file that was opened to start the current session. If no
        //      session, this is empty. So we don't store this until we have the
        //      session successfully opened up.
        //
        //  m_strResDir
        //      When we load the driver DLLs, we don't load them from their shipped
        //      positions, we load them from the output results directory, so that we
        //      pick up any new changes each time changes are made.
        //
        //  m_strTitleText
        //      For use in popups since the actual title text will include the
        //      manifest file path, which would be messy in a popup. So we pass
        //      this in.
        //
        //  m_wndMainFrame
        //      Our main frame window, which we create upon startup.
        // -------------------------------------------------------------------
        TCQCDriverCfg           m_cqcdcOrg;
        TCQCDriverObjCfg        m_cqcdcSession;
        TCQCUserCtx             m_cuctxToUse;
        tCQCKit::EVerboseLvls   m_eLastVerbosity;
        tCIDLib::TIPPortNum     m_ippnListen;
        TFacility*              m_pfacSrvDrv;
        TCQCSrvDrvTI*           m_porbsDrvAdmin;
        TCQCServerBase*         m_psdrvTest;
        TString                 m_strManifestFile;
        TString                 m_strResDir;
        TString                 m_strTitleText;
        TCppDrvTestMainFrame    m_wndMainFrame;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCppDrvTest,TGUIFacility)
};


#pragma CIDLIB_POPPACK

