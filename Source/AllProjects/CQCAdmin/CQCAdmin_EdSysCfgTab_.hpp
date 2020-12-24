//
// FILE NAME: CQCAdmin_EdSysCfgTab_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/05/2016
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
//  This is an overflow header to hold the header stuff only needed internally within
//  the system configuration editor tab. That's the bulk of it since we have a lot of
//  dialogs and a number of tab windows that we don't need to expose.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TEdSCHVACDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdSCHVACDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdSCHVACDlg();

        TEdSCHVACDlg(const TEdSCHVACDlg&) = delete;
        TEdSCHVACDlg(TEdSCHVACDlg&&) = delete;

        ~TEdSCHVACDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEdSCHVACDlg& operator=(const TEdSCHVACDlg&) = delete;
        TEdSCHVACDlg& operator=(TEdSCHVACDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TSCHVACInfo&            scliEdit
            , const TCQCFldCache&           cfcFields
            ,       TEdSysCfgTab::EAddOpts& eAddOpt
            , const TCQCSysCfg&             scfgEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


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
        //  m_eAddOpt
        //      To store the selected add option until we can get it back to the caller's
        //      parameter.
        //
        //  m_pcfcFields
        //      The dialog box has a field cache that it keeps around, and passes
        //      to us for us to make things more efficient.
        //
        //  m_pscfgCur
        //      We need the current config data in order to be able to insure that
        //      they don't enter a duplicate name. If we are editing. We remember the
        //      incoming name, if any, so that we don't trigger on itself.
        //
        //  m_pseciWorking
        //      A pointer to the caller's info object to edit. He should provide
        //      us with a copy, so we can modify it all we want, even if we end up
        //      not committing to changes.
        //
        //  m_pwndXXX
        //      We get typed pointers to the widgets we wnat to interact with.
        //
        //  m_strOrgTitle
        //      The incoming title, so that when we check for dups we don't reject it
        //      because it is a match of the original title.
        // -------------------------------------------------------------------
        TEdSysCfgTab::EAddOpts  m_eAddOpt;
        const TCQCFldCache*     m_pcfcFields;
        const TCQCSysCfg*       m_pscfgCur;
        TRadioButton*           m_pwndAddToAll;
        TRadioButton*           m_pwndAddIfThermos;
        TRadioButton*           m_pwndAddToNone;
        TSCHVACInfo*            m_pscliWorking;
        TPushButton*            m_pwndCancel;
        TEntryField*            m_pwndThermo;
        TPushButton*            m_pwndSelThermo;
        TPushButton*            m_pwndSave;
        TEntryField*            m_pwndTitle;
        TString                 m_strOrgTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdSCHVACDlg,TDlgBox)
};



// ---------------------------------------------------------------------------
//   CLASS: TEdSCLoadDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdSCLoadDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdSCLoadDlg();

        TEdSCLoadDlg(const TEdSCLoadDlg&) = delete;
        TEdSCLoadDlg(TEdSCLoadDlg&&) = delete;

        ~TEdSCLoadDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEdSCLoadDlg& operator=(const TEdSCLoadDlg&) = delete;
        TEdSCLoadDlg& operator=(TEdSCLoadDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TSCLoadInfo&            scliEdit
            , const TCQCFldCache&           cfcFields
            ,       TEdSysCfgTab::EAddOpts& eAddOpt
            , const TCQCSysCfg&             scfgEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


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
        //  m_eAddOpt
        //      To store the add option until we can get it back to the caller's parm
        //
        //  m_pcfcFields
        //      The dialog box has a field cache that it keeps around, and passes
        //      to us for us to make things more efficient.
        //
        //  m_pscfgCur
        //      We need the current config data in order to be able to insure that
        //      they don't enter a duplicate name. If we are editing. We remember the
        //      incoming name, if any, so that we don't trigger on itself.
        //
        //  m_pliWorking
        //      A pointer to the caller's load info object to edit. He should provide
        //      us with a copy, so we can modify it all we want, even if we end up
        //      not committing to changes.
        //
        //  m_pwndXXX
        //      We get typed pointers to the widgets we wnat to interact with.
        //      We don't own them.
        //
        //  m_strOrgAltName
        //  m_strOrgTitle
        //      The incoming title and alt name, so that when we check for dups we don't
        //      reject it because it is a match of the original values.
        // -------------------------------------------------------------------
        TEdSysCfgTab::EAddOpts  m_eAddOpt;
        const TCQCFldCache*     m_pcfcFields;
        const TCQCSysCfg*       m_pscfgCur;
        TSCLoadInfo*            m_pscliWorking;
        TRadioButton*           m_pwndAddToAll;
        TRadioButton*           m_pwndAddIfLoads;
        TRadioButton*           m_pwndAddToNone;
        TPushButton*            m_pwndCancel;
        TStaticText*            m_pwndCField;
        TEntryField*            m_pwndField;
        TStaticText*            m_pwndField2;
        TPushButton*            m_pwndSave;
        TPushButton*            m_pwndSelFld;
        TEntryField*            m_pwndSpokenName;
        TEntryField*            m_pwndTitle;
        TString                 m_strOrgAltName;
        TString                 m_strOrgTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdSCLoadDlg,TDlgBox)
};



// ---------------------------------------------------------------------------
//   CLASS: TEdSCMediaDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdSCMediaDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdSCMediaDlg();

        TEdSCMediaDlg(const TEdSCMediaDlg&) = delete;
        TEdSCMediaDlg(TEdSCMediaDlg&&) = delete;

        ~TEdSCMediaDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEdSCMediaDlg& operator=(const TEdSCMediaDlg&) = delete;
        TEdSCMediaDlg& operator=(TEdSCMediaDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TSCMediaInfo&           scliEdit
            , const TCQCFldCache&           cfcFields
            , const TCQCSysCfg&             scfgEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSelectMon
        (
            const   tCQCKit::EDevClasses    eClass
            ,       TString&                strToFill
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pcfcFields
        //      The dialog box has a field cache that it keeps around, and passes
        //      to us for us to make things more efficient.
        //
        //  m_pscfgCur
        //      We need the current config data in order to be able to insure that
        //      they don't enter a duplicate name. If we are editing. We remember the
        //      incoming name, if any, so that we don't trigger on itself.
        //
        //  m_pseciWorking
        //      A pointer to the caller's info object to edit. He should provide
        //      us with a copy, so we can modify it all we want, even if we end up
        //      not committing to changes.
        //
        //  m_pwndXXX
        //      We get typed pointers to the widgets we wnat to interact with.
        //      We don't own them.
        //
        //  m_strOrgTitle
        //      The incoming title, so that when we check for dups we don't reject it
        //      because it is a match of the original title.
        // -------------------------------------------------------------------
        const TCQCFldCache* m_pcfcFields;
        const TCQCSysCfg*   m_pscfgCur;
        TSCMediaInfo*       m_pscliWorking;
        TEntryField*        m_pwndAudioMon;
        TPushButton*        m_pwndCancel;
        TEntryField*        m_pwndRepoMon;
        TEntryField*        m_pwndRendMon;
        TPushButton*        m_pwndSave;
        TPushButton*        m_pwndSelAudio;
        TPushButton*        m_pwndSelRend;
        TPushButton*        m_pwndSelRepo;
        TEntryField*        m_pwndTitle;
        TString             m_strOrgTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdSCMediaDlg,TDlgBox)
};


// ---------------------------------------------------------------------------
//   CLASS: TEdSCSecDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdSCSecDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdSCSecDlg();

        TEdSCSecDlg(const TEdSCSecDlg&) = delete;
        TEdSCSecDlg(TEdSCSecDlg&&) = delete;

        ~TEdSCSecDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEdSCSecDlg& operator=(const TEdSCSecDlg&) = delete;
        TEdSCSecDlg& operator=(TEdSCSecDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TSCSecInfo&             scliEdit
            , const TCQCFldCache&           cfcFields
            , const TCQCSysCfg&             scfgEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLoadAreaList
        (
            const   TString&                strMoniker
            , const TString&                strAreaName
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pcfcFields
        //      The dialog box has a field cache that it keeps around, and passes
        //      to us for us to make things more efficient.
        //
        //  m_pscfgCur
        //      We need the current config data in order to be able to insure that
        //      they don't enter a duplicate name. If we are editing. We remember the
        //      incoming name, if any, so that we don't trigger on itself.
        //
        //  m_pseciWorking
        //      A pointer to the caller's info object to edit. He should provide
        //      us with a copy, so we can modify it all we want, even if we end up
        //      not committing to changes.
        //
        //  m_pwndXXX
        //      We get typed pointers to the widgets we wnat to interact with.
        //      We don't own them.
        //
        //  m_strOrgTitle
        //      The incoming title, so that when we check for dups we don't reject it
        //      because it is a match of the original title.
        // -------------------------------------------------------------------
        const TCQCFldCache* m_pcfcFields;
        const TCQCSysCfg*   m_pscfgCur;
        TSCSecInfo*         m_pscliWorking;
        TMultiColListBox*   m_pwndAreaList;
        TEntryField*        m_pwndArmingCode;
        TPushButton*        m_pwndCancel;
        TEntryField*        m_pwndMoniker;
        TPushButton*        m_pwndSelMon;
        TPushButton*        m_pwndSave;
        TEntryField*        m_pwndTitle;
        TString             m_strOrgTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdSCSecDlg,TDlgBox)
};



// ---------------------------------------------------------------------------
//   CLASS: TEdSCWeatherDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdSCWeatherDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdSCWeatherDlg();

        TEdSCWeatherDlg(const TEdSCWeatherDlg&) = delete;
        TEdSCWeatherDlg(TEdSCWeatherDlg&&) = delete;

        ~TEdSCWeatherDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TEdSCWeatherDlg& operator=(const TEdSCWeatherDlg&) = delete;
        TEdSCWeatherDlg& operator=(TEdSCWeatherDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TSCWeatherInfo&         scliEdit
            , const TCQCFldCache&           cfcFields
            , const TCQCSysCfg&             scfgEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


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
        //  m_pcfcFields
        //      The dialog box has a field cache that it keeps around, and passes
        //      to us for us to make things more efficient.
        //
        //  m_pscfgCur
        //      We need the current config data in order to be able to insure that
        //      they don't enter a duplicate name. If we are editing. We remember the
        //      incoming name, if any, so that we don't trigger on itself.
        //
        //  m_pseciWorking
        //      A pointer to the caller's info object to edit. He should provide
        //      us with a copy, so we can modify it all we want, even if we end up
        //      not committing to changes.
        //
        //  m_pwndXXX
        //      We get typed pointers to the widgets we wnat to interact with.
        //      We don't own them.
        //
        //  m_strOrgTitle
        //      The incoming tiltle, so that when we check for dups we don't reject it
        //      because it is a match of the original title.
        // -------------------------------------------------------------------
        const TCQCFldCache* m_pcfcFields;
        const TCQCSysCfg*   m_pscfgCur;
        TSCWeatherInfo*     m_pscliWorking;
        TPushButton*        m_pwndCancel;
        TEntryField*        m_pwndMoniker;
        TComboBox*          m_pwndScale;
        TPushButton*        m_pwndSelMon;
        TPushButton*        m_pwndSave;
        TEntryField*        m_pwndTitle;
        TString             m_strOrgTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdSCWeatherDlg,TDlgBox)
};



// ---------------------------------------------------------------------------
//   CLASS: TEdSCXOverDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdSCXOverDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdSCXOverDlg();

        TEdSCXOverDlg(const TEdSCXOverDlg&) = delete;
        TEdSCXOverDlg(TEdSCXOverDlg&&) = delete;

        ~TEdSCXOverDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TEdSCXOverDlg& operator=(const TEdSCXOverDlg&) = delete;
        TEdSCXOverDlg& operator=(TEdSCXOverDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TSCXOverInfo&           scliEdit
            , const TCQCSysCfg&             scfgEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSelectTmpl
        (
                    TString&                strToFill
            , const tCIDLib::TBoolean       bTile
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pseciWorking
        //      A pointer to the caller's info object to edit. He should provide
        //      us with a copy, so we can modify it all we want, even if we end up
        //      not committing to changes.
        //
        //  m_pwndXXX
        //      We get typed pointers to the widgets we wnat to interact with.
        //      We don't own them.
        // -------------------------------------------------------------------
        TSCXOverInfo*       m_pscliWorking;
        const TCQCSysCfg*   m_pscfgCur;
        TPushButton*        m_pwndCancel;
        TEntryField*        m_pwndContTmpl;
        TPushButton*        m_pwndDelCont;
        TPushButton*        m_pwndSave;
        TPushButton*        m_pwndSelCont;
        TPushButton*        m_pwndSelTile;
        TEntryField*        m_pwndTileTmpl;
        TEntryField*        m_pwndTitle;
        TString             m_strOrgTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdSCXOverDlg,TDlgBox)
};



// ---------------------------------------------------------------------------
//  CLASS: TESCGlobalTab
// PREFIX: wnd
//
//  Lets the user edit the gear available. This one doesn't derive from the base sys cfg
//  tab defined in the header. Those are for the room oriented stuff. This one is a
//  special case.
// ---------------------------------------------------------------------------
class TESCGlobalTab : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TESCGlobalTab
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TESCGlobalTab(const TESCGlobalTab&) = delete;
        TESCGlobalTab(TESCGlobalTab&&) = delete;

        ~TESCGlobalTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TESCGlobalTab& operator=(const TESCGlobalTab&) = delete;
        TESCGlobalTab& operator=(TESCGlobalTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reload();

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        );

        tCIDLib::TVoid Validate
        (
                    TValErrInfo::TErrList&  colList
        );


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  pcfcDevs
        //      The main editor tab gives us a pointer to a field cache that all of
        //      the tabs use, for efficiency.
        //
        //  m_pscfgEdit
        //      A pointer to the system configuration we are editing
        //
        //  m_pwndCont
        //      We create a generic window to hold the content and put it inside the scroll
        //      area.
        //
        //  m_pwndScroll
        //      We create a scroll area window and keep it sized to fit our client area.
        //      We put the controls inside of it.
        // -------------------------------------------------------------------
        TCQCFldCache*       m_pcfcDevs;
        TCQCSysCfg*         m_pscfgEdit;
        TPushButton*        m_pwndAddHVAC;
        TPushButton*        m_pwndAddLoad;
        TPushButton*        m_pwndAddMedia;
        TPushButton*        m_pwndAddSec;
        TPushButton*        m_pwndAddTarRes;
        TPushButton*        m_pwndAddWeath;
        TPushButton*        m_pwndAddXOver;
        TGenericWnd*        m_pwndCont;
        TEntryField*        m_pwndCustLayout;
        TPushButton*        m_pwndDelCustLayout;
        TPushButton*        m_pwndDelHVAC;
        TPushButton*        m_pwndDelLoad;
        TPushButton*        m_pwndDelMedia;
        TPushButton*        m_pwndDelSec;
        TPushButton*        m_pwndDelTarRes;
        TPushButton*        m_pwndDelWeath;
        TPushButton*        m_pwndDelXOver;
        TMultiColListBox*   m_pwndHVACList;
        TMultiColListBox*   m_pwndLoadList;
        TMultiColListBox*   m_pwndMediaList;
        TScrollArea*        m_pwndScroll;
        TMultiColListBox*   m_pwndSecList;
        TPushButton*        m_pwndSelCustLayout;
        TPushButton*        m_pwndSelLoad;
        TPushButton*        m_pwndSelTarScope;
        TMultiColListBox*   m_pwndTarResList;
        TEntryField*        m_pwndTarScope;
        TCheckBox*          m_pwndUseCustLights;
        TMultiColListBox*   m_pwndWeatherList;
        TMultiColListBox*   m_pwndXOverList;


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
        )  override;

        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DeleteListItem
        (
            const   tCIDLib::TCh* const     pszType
            ,       TMultiColListBox* const pwndList
            , const tCQCSysCfg::ECfgLists   eList
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadData();
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTGen
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTGen : public TCQCESCTabBase, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTGen() = delete;

        TCQCESCTGen
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTGen(const TCQCESCTGen&) = delete;
        TCQCESCTGen(TCQCESCTGen&&) = delete;

        ~TCQCESCTGen();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTGen& operator=(const TCQCESCTGen&) = delete;
        TCQCESCTGen& operator=(TCQCESCTGen&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPETestCanEdit
        (
            const   tCIDCtrls::TWndId       widSrc
            , const tCIDLib::TCard4         c4ColInd
            ,       TAttrData&              adatFill
            ,       tCIDLib::TBoolean&      bValueSet
        )   final;

        tCIDLib::TVoid DisableCtrls() final;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   final;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        )   final;

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   final;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


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
        // -------------------------------------------------------------------
        TCheckBox*          m_pwndAllowRmSel;
        TCheckBox*          m_pwndEnabled;
        TMultiColListBox*   m_pwndVarList;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTGen,TCQCESCTabBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTGActs
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTGActs : public TCQCESCTabBase, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTGActs() = delete;

        TCQCESCTGActs
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTGActs(const TCQCESCTGActs&) = delete;
        TCQCESCTGActs(TCQCESCTGActs&&) = delete;

        ~TCQCESCTGActs();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTGActs& operator=(const TCQCESCTGActs&);
        TCQCESCTGActs& operator=(TCQCESCTGActs&&);


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPETestCanEdit
        (
            const   tCIDCtrls::TWndId       widSrc
            , const tCIDLib::TCard4         c4ColInd
            ,       TAttrData&              adatFill
            ,       tCIDLib::TBoolean&      bValueSet
        )   final;

        tCIDLib::TVoid DisableCtrls() final;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   final;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        )   final;

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   final;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditCur();

        tCIDLib::TVoid LoadParms
        (
            const   TSCGlobActInfo&         scliCur
            , const tCIDLib::TCard4         c4SelInd
        );

        tCIDLib::TVoid ShowActInfo
        (
            const   tCQCSysCfg::EGlobActs   eAct
        );


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        TCheckedMCListWnd*  m_pwndActList;
        TPushButton*        m_pwndDelParm;
        TPushButton*        m_pwndDownParm;
        TMultiColListBox*   m_pwndParmList;
        TEntryField*        m_pwndPath;
        TPushButton*        m_pwndSelPath;
        TPushButton*        m_pwndUpParm;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTGActs,TCQCESCTabBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTHVAC
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTHVAC : public TCQCESCTabBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTHVAC() = delete;

        TCQCESCTHVAC
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTHVAC(const TCQCESCTHVAC&) = delete;
        TCQCESCTHVAC(TCQCESCTHVAC&&) = delete;

        ~TCQCESCTHVAC();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTHVAC& operator=(const TCQCESCTHVAC&) = delete;
        TCQCESCTHVAC& operator=(TCQCESCTHVAC&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DisableCtrls() final;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        )   final;

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   final;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs

        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadList
        (
                    TCQCSysCfgRmInfo&       scriRm
        );


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        TPushButton*        m_pwndClearAll;
        TEntryField*        m_pwndDefHVAC;
        TCheckedMCListWnd*  m_pwndList;
        TPushButton*        m_pwndSelAll;
        TPushButton*        m_pwndSetDef;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTHVAC,TCQCESCTabBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTLoads
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTLoads : public TCQCESCTabBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTLoads() = delete;

        TCQCESCTLoads
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTLoads(const TCQCESCTLoads&) = delete;
        TCQCESCTLoads(TCQCESCTLoads&&) = delete;

        ~TCQCESCTLoads();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTLoads& operator=(const TCQCESCTLoads&) = delete;
        TCQCESCTLoads& operator=(TCQCESCTLoads&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DisableCtrls() override;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        )   final;

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   final;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadList
        (
                    TCQCSysCfgRmInfo&       scriRm
        );


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        TPushButton*        m_pwndClearAll;
        TEntryField*        m_pwndDefLoad;
        TCheckedMCListWnd*  m_pwndList;
        TPushButton*        m_pwndSelAll;
        TPushButton*        m_pwndSetDef;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTLoads,TCQCESCTabBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTMedia
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTMedia : public TCQCESCTabBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTMedia() = delete;

        TCQCESCTMedia
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTMedia(const TCQCESCTMedia&) = delete;
        TCQCESCTMedia(TCQCESCTMedia&&) = delete;

        ~TCQCESCTMedia();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTMedia& operator=(const TCQCESCTMedia&) = delete;
        TCQCESCTMedia& operator=(TCQCESCTMedia&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DisableCtrls() final;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        )   final;

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   final;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


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
        // -------------------------------------------------------------------
        TEntryField*    m_pwndMovie;
        TPushButton*    m_pwndMovieDisable;
        TPushButton*    m_pwndMovieSel;
        TEntryField*    m_pwndMusic;
        TPushButton*    m_pwndMusicDisable;
        TPushButton*    m_pwndMusicSel;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTMedia,TCQCESCTabBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTRmModes
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTRmModes : public TCQCESCTabBase, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTRmModes() = delete;

        TCQCESCTRmModes
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTRmModes(const TCQCESCTRmModes&) = delete;
        TCQCESCTRmModes(TCQCESCTRmModes&&) = delete;

        ~TCQCESCTRmModes();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTRmModes& operator=(const TCQCESCTRmModes&);
        TCQCESCTRmModes& operator=(TCQCESCTRmModes&&);


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPETestCanEdit
        (
            const   tCIDCtrls::TWndId       widSrc
            , const tCIDLib::TCard4         c4ColInd
            ,       TAttrData&              adatFill
            ,       tCIDLib::TBoolean&      bValueSet
        )   override;

        tCIDLib::TVoid DisableCtrls() override;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   override;

        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const override;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        );

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   override;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearModeInfo();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditCur();

        tCIDLib::TVoid LoadParms
        (
            const   TSCGlobActInfo&         scliCur
            , const tCIDLib::TCard4         c4SelInd
        );

        tCIDLib::TVoid ShowActInfo
        (
            const   tCIDLib::TCard4         c4Index
        );


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        TPushButton*        m_pwndAddMode;
        TMultiColListBox*   m_pwndModeList;
        TPushButton*        m_pwndDelMode;
        TPushButton*        m_pwndDelParm;
        TPushButton*        m_pwndDownParm;
        TMultiColListBox*   m_pwndParmList;
        TEntryField*        m_pwndPath;
        TPushButton*        m_pwndSelPath;
        TPushButton*        m_pwndUpParm;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTRmModes,TCQCESCTabBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTSecure
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTSecure : public TCQCESCTabBase, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTSecure() = delete;

        TCQCESCTSecure
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTSecure(const TCQCESCTSecure&) = delete;
        TCQCESCTSecure(TCQCESCTSecure&&) = delete;

        ~TCQCESCTSecure();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTSecure& operator=(const TCQCESCTSecure&) = delete;
        TCQCESCTSecure& operator=(TCQCESCTSecure&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPETestCanEdit
        (
            const   tCIDCtrls::TWndId       widSrc
            , const tCIDLib::TCard4         c4ColInd
            ,       TAttrData&              adatFill
            ,       tCIDLib::TBoolean&      bValueSet
        )   final;

        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const final;

        tCIDLib::TVoid DisableCtrls() final;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   final;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        )   final;

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   final;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid SelectSecZones();


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        TPushButton*        m_pwndDisable;
        TPushButton*        m_pwndSelArea;
        TPushButton*        m_pwndSelZones;
        TEntryField*        m_pwndSecArea;
        TMultiColListBox*   m_pwndZones;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTSecure,TCQCESCTabBase)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTVoice
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTVoice : public TCQCESCTabBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTVoice() = delete;

        TCQCESCTVoice
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTVoice(const TCQCESCTVoice&) = delete;
        TCQCESCTVoice(TCQCESCTVoice&&) = delete;

        ~TCQCESCTVoice();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTVoice& operator=(const TCQCESCTVoice&);
        TCQCESCTVoice& operator=(TCQCESCTVoice&&);


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DisableCtrls() final;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        )   final;

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   final;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid LoadCQCVoiceInstances();


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eEFHandler
        (
                    TEFChangeInfo&          wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndXXX
        //      Typed pointers to controls that we need to interact with. We don't
        //      own them.
        // -------------------------------------------------------------------
        TCheckBox*          m_pwndEnable;
        TEntryField*        m_pwndKeyWord;
        TMultiColListBox*   m_pwndList;


        // -------------------------------------------------------------------
        //  Private, static data members
        //
        //  s_enctNextCheck
        //      Each time we update the list, we set this to the next time that we
        //      will get new data.
        //
        //  s_colVoiceHosts
        //      A list of hosts that are currently running the CQC Voice program. Each
        //      instance of this tab has to show this list, and it has to be gotten
        //      from the name server, so we don't want to do it over and over again
        //      for every room. So we provide a static method above that will only
        //      update it every so often.
        // -------------------------------------------------------------------
        static tCIDLib::TEncodedTime    s_enctNextCheck;
        static tCIDLib::TStrList        s_colVoiceHosts;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTVoice,TCQCESCTabBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTWeather
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTWeather : public TCQCESCTabBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTWeather() = delete;

        TCQCESCTWeather
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTWeather(const TCQCESCTWeather&) = delete;
        TCQCESCTWeather(TCQCESCTWeather&&) = delete;

        ~TCQCESCTWeather();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTWeather& operator=(const TCQCESCTWeather&) = delete;
        TCQCESCTWeather& operator=(TCQCESCTWeather&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DisableCtrls() final;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        )   final;

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   final;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


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
        // -------------------------------------------------------------------
        TEntryField*    m_pwndMoniker;
        TPushButton*    m_pwndDisable;
        TPushButton*    m_pwndSel;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTWeather,TCQCESCTabBase)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTXOvers
//  PREFIX: esct
// ---------------------------------------------------------------------------
class TCQCESCTXOvers : public TCQCESCTabBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCESCTXOvers() = delete;

        TCQCESCTXOvers
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TCQCESCTXOvers(const TCQCESCTXOvers&) = delete;
        TCQCESCTXOvers(TCQCESCTXOvers&&) = delete;

        ~TCQCESCTXOvers();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCESCTXOvers& operator=(const TCQCESCTXOvers&) = delete;
        TCQCESCTXOvers& operator=(TCQCESCTXOvers&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DisableCtrls() final;

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        )   final;

        tCIDLib::TVoid ShowInfo
        (
            const   TString&                strUID
        )   final;

        tCIDLib::TVoid Validate
        (
                    TCQCSysCfgRmInfo&       scriRoom
            ,       TValErrInfo::TErrList&  colErrs
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        TPushButton*        m_pwndDeselAll;
        TCheckedMCListWnd*  m_pwndList;
        TPushButton*        m_pwndSelAll;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCESCTXOvers,TCQCESCTabBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TESCRoomsTab
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TESCRoomsTab : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static const tCIDLib::TCard4 c4CmdId_RefreshTabs;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TESCRoomsTab() = delete;

        TESCRoomsTab
        (
                    TCQCSysCfg* const       pscfgEdit
            ,       TCQCFldCache* const     pcfcDevs
        );

        TESCRoomsTab(const TESCRoomsTab&) = delete;
        TESCRoomsTab(TESCRoomsTab&&) = delete;

        ~TESCRoomsTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TESCRoomsTab& operator=(const TESCRoomsTab&) = delete;
        TESCRoomsTab& operator=(TESCRoomsTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reload();

        tCIDLib::TVoid SelectErrTarget
        (
            const   TValErrInfo&            verrToSel
        );

        tCIDLib::TVoid Validate
        (
                    TValErrInfo::TErrList&  colList
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
        )   final;

        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TVoid TabActivationChange
        (
            const   tCIDLib::TBoolean       bGetting
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddNew
        (
            const   tCIDLib::TBoolean       bCopyFrom
        );

        tCIDLib::TVoid DeleteCur();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadRooms
        (
            const   TString&                strUIDInit
        );

        tCIDLib::TVoid SelectListItem
        (
            const   tCIDLib::TCard4         c4Index
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colTabs
        //      A list of the tabs that we create and put into the tabbed
        //      window, so that we can translate from a tab index to a tab.
        //
        //  m_pcfcDevs
        //      A pointer to a field cache that the main dialog provides us, to
        //      avoid a lot of redundant overhead looking up devices and fields.
        //
        //  m_pesctXXX
        //      We keep typed pointers to any tabs we need to directly
        //      interact with.
        //
        //  m_pscfgEdit
        //      On creation we get a pointer to the system configuration that
        //      is being edited. It will remain valid while the config dailog
        //      is open.
        //
        //  m_scrmSaveRoom
        //      When the user selects a new room in the list, we copy it here so
        //      that we can undo changes until they move to another one and choose
        //      to either save or discard the changes.
        //
        //  m_pwndXXX
        //      Since we have to interact with these controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them a convenient way.
        // -------------------------------------------------------------------
        TCQCFldCache*       m_pcfcDevs;
        TCQCSysCfg*         m_pscfgEdit;
        TPushButton*        m_pwndAddRoom;
        TPushButton*        m_pwndCopyFrom;
        TPushButton*        m_pwndDelRoom;
        TMultiColListBox*   m_pwndList;
        TTabbedWnd*         m_pwndRoomTabs;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TESCRoomsTab,TTabWindow)
};


// ---------------------------------------------------------------------------
//   CLASS: TEdSCValErrDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdSCValErrDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdSCValErrDlg();

        TEdSCValErrDlg(const TEdSCValErrDlg&) = delete;
        TEdSCValErrDlg(TEdSCValErrDlg&&) = delete;

        ~TEdSCValErrDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEdSCValErrDlg& operator=(const TEdSCValErrDlg&) = delete;
        TEdSCValErrDlg& operator=(TEdSCValErrDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TValErrInfo::TErrList&  colErrs
            ,       tCIDLib::TCard4&        c4SelErr
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4SelErr
        //      If they double click, we store the one they double clicked on
        //      and return as though Save was clicked, to indicate an error
        //      was selected to be brought up for editing.
        //
        //  m_pcolErrList
        //      We get the list of errors that were gotten during a validation
        //      pass, for display to the user.
        //
        //  m_pwndXXX
        //      The usual typed pointers we get to the controls we need to
        //      interact with directly.
        // -------------------------------------------------------------------
        tCIDLib::TCard4                 m_c4SelErr;
        const TValErrInfo::TErrList*    m_pcolErrList;
        TPushButton*                    m_pwndCancel;
        TPushButton*                    m_pwndGoTo;
        TSectionalList*                 m_pwndList;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdSCValErrDlg,TDlgBox)
};

