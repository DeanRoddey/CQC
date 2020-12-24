//
// FILE NAME: IRClient_BlasterTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/02/2003
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
//  This file implements the tab that gets inserted into the main driver
//  window's tabbed control, if the device supports blaster functionality.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TIRBlasterTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIRBlasterTab : public TTabWindow
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TIRBlasterTab() = delete;

        TIRBlasterTab
        (
                    TIRClientWnd* const     pwndDriver
        );

        TIRBlasterTab(const TIRBlasterTab&) = delete;
        TIRBlasterTab(TIRBlasterTab&&) = delete;

        ~TIRBlasterTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIRBlasterTab& operator=(const TIRBlasterTab&) = delete;
        TIRBlasterTab& operator=(TIRBlasterTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetConnected
        (
            const   TString&                strFirmware
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

        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetInpExpFile
        (
                    TPathStr&               pathToFill
            , const TString&                strTitle
            , const tCIDLib::TBoolean       bImport
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eComboHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditOrCreate
        (
            const   tCIDLib::TBoolean       bEdit
        );

        tCIDLib::TVoid Export();

        tCIDLib::TVoid Import();

        tCIDLib::TVoid InvokeCmd();

        tCIDLib::TVoid LoadCmds
        (
            const   TIRBlasterDevModel&     irbdmSrc
        );

        tCIDLib::TVoid LoadConfigData();

        tCIDLib::TVoid LoadModel();

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid UnloadModel();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_irbcData
        //      The blaster config data that we download from the server.
        //      It provides us the info we need to drive our display.
        //
        //  m_pathLastExport
        //      The last path used for import/export, so that we can select
        //      it again next time. Initially we set it to the user's app
        //      data directory.
        //
        //  m_pwndXXX
        //      Some typed pointers to our widgets. We don't own them, our
        //      parent class does, but we want to have some typed pointers so
        //      that we avoid lots of downcasting. They are set up during
        //      startup.
        //
        //  m_pwndDriver
        //      A pointer to the driver window that we work in terms of.
        //      Primarily we need to get access it's driver client proxy.
        //
        //  m_strVersion
        //      We query the version info from the server side driver, for
        //      display purposes.
        // -------------------------------------------------------------------
        TIRBlasterCfg       m_irbcData;
        TPathStr            m_pathLastExport;
        TStaticMultiText*   m_pwndBlasterInfo;
        TMultiColListBox*   m_pwndCmds;
        TPushButton*        m_pwndCreateButton;
        TStaticMultiText*   m_pwndDevInfo;
        TIRClientWnd*       m_pwndDriver;
        TPushButton*        m_pwndEditButton;
        TPushButton*        m_pwndExportButton;
        TPushButton*        m_pwndImportButton;
        TPushButton*        m_pwndLoadButton;
        TComboBox*          m_pwndModels;
        TPushButton*        m_pwndUnloadButton;
        TComboBox*          m_pwndZone;
        TString             m_strVersion;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRBlasterTab, TTabWindow)
};

#pragma CIDLIB_POPPACK


