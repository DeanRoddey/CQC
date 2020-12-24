//
// FILE NAME: CQCTreeBrws_DrvWizDlg_Panels_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/11/2015
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
//  This is a helper file for the main driver wizard dialog file. The wizard defines
//  and works in terms of a set of tabbed window 'panels' that get the required info
//  from the user. To keep that file from getting too big, we define the tab panels
//  in a separate file.
//
//  We have a base panel class and then derivatives for each category of information
//  we need to query.
//
// CAVEATS/GOTCHAS:
//
//  1)  The tab window text is what gets drawn as the tab header text, so don't change
//      it. That's what is used to calculate the tab header area, so if you change it
//      the tab won't be the right size.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCDrvWizDlg;


// ---------------------------------------------------------------------------
//   CLASS: TDrvWizPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDrvWizPanel : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  The general categories of panels supported
        // -------------------------------------------------------------------
        enum class EPanTypes
        {
            Moniker
            , CommCfg
            , Prompt
            , Summary
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDrvWizPanel(const TDrvWizPanel&) = delete;
        TDrvWizPanel(TDrvWizPanel&&) = delete;

        ~TDrvWizPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDrvWizPanel& operator=(const TDrvWizPanel&) = delete;
        TDrvWizPanel& operator=(TDrvWizPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        ) = 0;

        virtual tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        ) = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreatePanel
        (
                    TCQCDrvWizDlg* const    pwndDlg
            ,       TTabbedWnd&             wndTabs
            , const TCQCSecToken&           sectUser
        );

        EPanTypes eType() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TDrvWizPanel
        (
            const   TString&                strName
            , const EPanTypes               eType
            , const tCIDLib::TResId         ridDlgDescr
            ,       TCQCDriverObjCfg* const pcqcdcEdit
        );


        // -------------------------------------------------------------------
        //  Protected, inherited method
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


        // -------------------------------------------------------------------
        //  Protected, non-virtual method
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseWC
        (
            const   TString&                strWC
            ,       tCIDLib::TKVPList&      colToFill
        );

        TCQCDriverObjCfg& cqcdcEdit();

        const TCQCDriverObjCfg& cqcdcEdit() const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eType
        //      Each derived type passes us a type indicator, which the
        //      main navigation logic uses to know how to deal with panels.
        //
        //  m_pcqcdcEdit
        //      Each panel gets a pointer to the driver config data being
        //      edited so that they can get data out or make changes as
        //      needed. We proivde protected methods above for them to
        //      access it.
        //
        //  m_ridDlgDescr
        //      The derived classes give us the dialog description that they
        //      want to use for their interface. We create the window and
        //      the controls for them.
        //
        //  m_pwndParent
        //      We get a point to our parent dialog so that we can communicate
        //      with him as required.
        //
        //  m_strName
        //      The name of the panel, for human consumption purposes.
        //
        //  m_widInitFocus
        //      The control that we want the focus to go to when this panel
        //      is selected. This comes from the dialog definition and we
        //      store the control marked as init focus here and use that to
        //      set the focus. If none are marked this is widInvalid and
        //      the default focus handle will be used.
        // -------------------------------------------------------------------
        EPanTypes           m_eType;
        TCQCDriverObjCfg*   m_pcqcdcEdit;
        TCQCDrvWizDlg*      m_pwndParent;
        tCIDLib::TResId     m_ridDlgDescr;
        TString             m_strName;
        tCIDCtrls::TWndId   m_widInitFocus;



        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDrvWizPanel,TTabWindow)
};


// ---------------------------------------------------------------------------
//   CLASS: TDWAudDevPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWAudDevPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWAudDevPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWAudDevPanel(const TDWAudDevPanel&) = delete;

        ~TDWAudDevPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWAudDevPanel& operator=(const TDWAudDevPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4PromptInd;
        TListBox*           m_pwndDevList;
        TStaticMultiText*   m_pwndDescr;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWAudDevPanel,TDrvWizPanel)
};


// ---------------------------------------------------------------------------
//   CLASS: TDWACPromptPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWACPromptPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWACPromptPanel() = delete;

        TDWACPromptPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWACPromptPanel(const TDWACPromptPanel&) = delete;

        ~TDWACPromptPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWACPromptPanel& operator=(const TDWACPromptPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bOptional
        //      We get the optional path value out of the prompt during the
        //      ctor since it is a fixed value.
        //
        //  m_bPathOnly
        //      Indicates whether we should just store a path or require the
        //      full path, name, and extention. Usually we just want the path.
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strFileTypes
        //      We get the file types list out during ctor and store it for
        //      later use.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bOptional;
        tCIDLib::TBoolean   m_bPathOnly;
        tCIDLib::TCard4     m_c4PromptInd;
        TComboBox*          m_pwndBinding;
        TEntryField*        m_pwndPath;
        TString             m_strFileTypes;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWACPromptPanel,TDrvWizPanel)
};


// ---------------------------------------------------------------------------
//   CLASS: TDWBoolPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWBoolPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWBoolPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWBoolPanel(const TDWBoolPanel&) = delete;

        ~TDWBoolPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWBoolPanel& operator=(const TDWBoolPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strSubKey
        //      We have a single value, with this sub-key
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4PromptInd;
        TCheckBox*          m_pwndValue;
        TStaticMultiText*   m_pwndDescr;
        const TString       m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWBoolPanel,TDrvWizPanel)
};



// ---------------------------------------------------------------------------
//   CLASS: TDWFileSelPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWFileSelPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWFileSelPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
            , const tCIDLib::TBoolean       bFileMode
        );

        TDWFileSelPanel(const TDWFileSelPanel&) = delete;

        ~TDWFileSelPanel();


        // -------------------------------------------------------------------
        //  Pubilc operators
        // -------------------------------------------------------------------
        TDWFileSelPanel& operator=(const TDWFileSelPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
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
        //  m_bCMLDriver
        //      We set this during the ctor if the driver is a CML driver.
        //      If so, we limit them to selecting a file under the macro
        //      file root, and we convert the final resulting path to a
        //      path relative to that.
        //
        //  m_bFileMode
        //      This same one is used for both file and path selection. We get
        //      a flag in the ctor indicating which one.
        //
        //  m_bOptional
        //      We get the optional file value out of the prompt during the
        //      ctor since it is a fixed value.
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strFileTypes
        //      We get the file types list out during ctor and store it for
        //      later use.
        //
        //  m_strSubKey
        //      We have a single value, with this sub-key
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bCMLDriver;
        tCIDLib::TBoolean   m_bFileMode;
        tCIDLib::TBoolean   m_bOptional;
        tCIDLib::TCard4     m_c4PromptInd;
        TPushButton*        m_pwndBrowse;
        TStaticMultiText*   m_pwndDescr;
        TEntryField*        m_pwndFile;
        TString             m_strFileTypes;
        TString             m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWFileSelPanel,TDrvWizPanel)
};



// ---------------------------------------------------------------------------
//   CLASS: TDWListPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWListPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWListPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWListPanel(const TDWListPanel&) = delete;

        ~TDWListPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWListPanel& operator=(const TDWListPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadListItems
        (
            const   TString&                strCurVal
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strListValues
        //      The list values, which we store away in the ctor.
        //
        //  m_strSubKey
        //      We have a single value, with this sub-key
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4PromptInd;
        TStaticMultiText*   m_pwndDescr;
        TComboBox*          m_pwndList;
        TString             m_strListValues;
        const TString       m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWListPanel,TDrvWizPanel)
};



// ---------------------------------------------------------------------------
//   CLASS: TDWMListPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWMListPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWMListPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWMListPanel(const TDWMListPanel&) = delete;

        ~TDWMListPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWMListPanel& operator=(const TDWMListPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadListItems
        (
            const   TString&                strCurVals
            , const TString&                strListVals
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strSubKey
        //      We have a single value, with this sub-key
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4PromptInd;
        TStaticMultiText*   m_pwndDescr;
        TCheckedMCListWnd*  m_pwndList;
        const TString       m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWMListPanel,TDrvWizPanel)
};



// ---------------------------------------------------------------------------
//   CLASS: TDWMonikerPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWMonikerPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWMonikerPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TBoolean       bReconfigMode
            , const tCIDLib::TStrList&      colCurMons
        );

        TDWMonikerPanel(const TDWMonikerPanel&) = delete;

        ~TDWMonikerPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWMonikerPanel& operator=(const TDWMonikerPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bReconfigMode
        //      We are created in reconfigure or config modes. In reconfigure
        //      mode we don't allow the moniker to be changed.
        //
        //  m_pcolCurMons
        //      We get a pointer to a list of existing driver monikers, so that
        //      we can check for dup.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strOrgMon
        //      Our original moniker, which we save at the start. We have to check
        //      for dups, but we need to know what our original one was, so that
        //      we can know if its changed.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bReconfigMode;
        const tCIDLib::TStrList*    m_pcolCurMons;
        TEntryField*                m_pwndMoniker;
        TString                     m_strOrgMon;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWMonikerPanel,TDrvWizPanel)
};


// ---------------------------------------------------------------------------
//   CLASS: TDWMediaRepoPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWMediaRepoPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWMediaRepoPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWMediaRepoPanel(const TDWMediaRepoPanel&) = delete;

        ~TDWMediaRepoPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWMediaRepoPanel& operator=(const TDWMediaRepoPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strSubKey
        //      We have a single value, with this sub-key
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4PromptInd;
        TStaticMultiText*   m_pwndDescr;
        TComboBox*          m_pwndList;
        const TString       m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWMediaRepoPanel,TDrvWizPanel)
};


// ---------------------------------------------------------------------------
//   CLASS: TDWNamedListPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWNamedListPanel : public TDrvWizPanel, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWNamedListPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWNamedListPanel(const TDWNamedListPanel&) = delete;

        ~TDWNamedListPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWNamedListPanel& operator=(const TDWNamedListPanel&) = delete;


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

        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const override;

        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadListItems
        (
            const   TString&                strCurVals
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSpacesOK
        //      Indicates if spaces in the name values are OK or not. We get this
        //      out of the prompt during the ctor and store it.
        //
        //  m_c4Count
        //      The count of items in our list, which is stored in the limits
        //      string of the prompt.
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strSubKey
        //      We have a single value, with this sub-key
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bSpacesOK;
        tCIDLib::TCard4     m_c4Count;
        tCIDLib::TCard4     m_c4PromptInd;
        TStaticMultiText*   m_pwndDescr;
        TMultiColListBox*   m_pwndList;
        const TString       m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWNamedListPanel,TDrvWizPanel)
};


// ---------------------------------------------------------------------------
//   CLASS: TDWODBCPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWODBCPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWODBCPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWODBCPanel(const TDWODBCPanel&) = delete;

        ~TDWODBCPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWODBCPanel& operator=(const TDWODBCPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadListItems
        (
            const   TString&                strCurVal
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_colSrcs
        //      The sources we found. We store the indicates in the list box so
        //      we can get back to this list.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strSubKey
        //      We have a single value, with this sub-key
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4PromptInd;
        tCIDLib::TKVPList   m_colSrcs;
        TStaticMultiText*   m_pwndDescr;
        TListBox*           m_pwndList;
        const TString       m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWODBCPanel,TDrvWizPanel)
};



// ---------------------------------------------------------------------------
//   CLASS: TDWRangePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWRangePanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWRangePanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWRangePanel(const TDWRangePanel&) = delete;

        ~TDWRangePanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWRangePanel& operator=(const TDWRangePanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_i4Max
        //  m_i4Min
        //      We get the min/max values out during the ctor and keep them
        //      around.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strSubKey
        //      We have a single value, with this sub-key
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4PromptInd;
        tCIDLib::TInt4      m_i4Max;
        tCIDLib::TInt4      m_i4Min;
        TStaticMultiText*   m_pwndDescr;
        TEntryField*        m_pwndValue;
        const TString       m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWRangePanel,TDrvWizPanel)
};



// ---------------------------------------------------------------------------
//   CLASS: TDWSerialPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWSerialPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWSerialPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
        );

        TDWSerialPanel(const TDWSerialPanel&) = delete;

        ~TDWSerialPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWSerialPanel& operator=(const TDWSerialPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


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
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        TPushButton*        m_pwndEdit;
        TListBox*           m_pwndPortList;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWSerialPanel,TDrvWizPanel)
};



// ---------------------------------------------------------------------------
//   CLASS: TDWSockPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWSockPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWSockPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
        );

        TDWSockPanel(const TDWSockPanel&) = delete;

        ~TDWSockPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWSockPanel& operator=(const TDWSockPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        TEntryField*    m_pwndAddr;
        TEntryField*    m_pwndPort;
        TComboBox*      m_pwndProto;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWSockPanel,TDrvWizPanel)
};


// ---------------------------------------------------------------------------
//   CLASS: TDWSumPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWSumPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWSumPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
        );

        TDWSumPanel(const TDWSumPanel&) = delete;

        ~TDWSumPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWSumPanel& operator=(const TDWSumPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        T2ColSectList*  m_pwndSummary;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWSumPanel,TDrvWizPanel)
};



// ---------------------------------------------------------------------------
//   CLASS: TDWTextPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWTextPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWTextPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWTextPanel(const TDWTextPanel&) = delete;

        ~TDWTextPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWTextPanel& operator=(const TDWTextPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bGotRegEx
        //      Remember if we got a regular expression. If so, m_regxLimit is
        //      set up.
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_regxLimit
        //      We can get a regular expression for validation of the value.
        //
        //  m_strSubKey
        //      We have a single value, with this sub-key
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bGotRegEx;
        tCIDLib::TCard4     m_c4PromptInd;
        TStaticMultiText*   m_pwndDescr;
        TMultiEdit*         m_pwndValue;
        TRegEx              m_regxLimit;
        const TString       m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWTextPanel,TDrvWizPanel)
};



// ---------------------------------------------------------------------------
//   CLASS: TDWTTSVoicePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWTTSVoicePanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWTTSVoicePanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
            , const tCIDLib::TCard4         c4PromptInd
        );

        TDWTTSVoicePanel(const TDWTTSVoicePanel&) = delete;

        ~TDWTTSVoicePanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWTTSVoicePanel& operator=(const TDWTTSVoicePanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PromptInd
        //      The index of our prompt in the driver config.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strKey
        //      We have one value, with this sub-key.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4PromptInd;
        TListBox*           m_pwndList;
        TStaticMultiText*   m_pwndDescr;
        const TString       m_strSubKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWTTSVoicePanel,TDrvWizPanel)
};


// ---------------------------------------------------------------------------
//   CLASS: TDWUPnPCommPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TDWUPnPCommPanel : public TDrvWizPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDWUPnPCommPanel
        (
                    TCQCDriverObjCfg* const pcqcdcEdit
        );

        TDWUPnPCommPanel(const TDWUPnPCommPanel&) = delete;

        ~TDWUPnPCommPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDWUPnPCommPanel& operator=(const TDWUPnPCommPanel&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    TCQCDrvWizDlg&          wndParent
            ,       tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCDrvWizDlg&          wndParent
            , const tCIDLib::TStrList&      colPorts
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoSearch();

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
        //  m_colLastSearch
        //      The device info we found in the last search. We have to keep these
        //      around so that we can get back to them upon selection.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        tCIDLib::TKValsList m_colLastSearch;
        TStaticText*        m_pwndCurDev;
        TStaticMultiText*   m_pwndDescr;
        TPushButton*        m_pwndSearch;
        TListBox*           m_pwndList;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDWUPnPCommPanel,TDrvWizPanel)
};


#pragma CIDLIB_POPPACK

