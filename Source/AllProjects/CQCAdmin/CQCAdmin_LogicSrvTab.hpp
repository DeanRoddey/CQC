//
// FILE NAME: CQCAdmin_LogicSrvTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/05/2016
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
//  This is the header for the CQCClient_LogicSrvTab.cpp file, which implements a tab
//  window that allows the user to configure the logic server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TLogicSrvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TLogicSrvTab : public TContentTab, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TLogicSrvTab
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TLogicSrvTab(const TLogicSrvTab&) = delete;
        TLogicSrvTab(TLogicSrvTab&&) = delete;

        ~TLogicSrvTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TLogicSrvTab&) = delete;
        tCIDLib::TVoid operator=(TLogicSrvTab&&) = delete;


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
            ,       TAttrData&              adatSrc
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const final;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
            , const TCQLogicSrvCfg&         lscfgEdit
        );


    protected  :
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

        tCIDLib::TBoolean bCreated() final;

        tCIDLib::ESaveRes eDoSave
        (
                    TString&                strErr
            , const tCQCAdmin::ESaveModes   eMode
            ,       tCIDLib::TBoolean&      bChangedSaved
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddSrcField();

        tCIDLib::TCard4 c4ListSelToFld
        (
            const   tCIDLib::TCard4         c4WndIndex
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;

        TCQLSrvFldType& clsftSelected();

        tCIDLib::TVoid DeleteFldCtrls();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );

        tCIDCtrls::EEvResponses eEFHandler
        (
                    TEFChangeInfo&          wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eMLEHandler
        (
                    TMLEChangeInfo&         wnotEvent
        );

        tCIDLib::TVoid LoadExprTypes
        (
                    TComboBox&              wndTar
        );

        tCIDLib::TVoid LoadSelField();

        tCIDLib::TVoid LoadSrcField
        (
            const   tCIDLib::TBoolean       bClear
        );

        tCIDLib::TVoid RenumberSrcFlds
        (
            const   tCIDLib::TCard4         c4Field
            , const tCIDLib::TCard4         c4StartSrc
        );

        tCIDLib::TVoid SelectExpr
        (
                    TComboBox&              wndTar
            , const TCQCExpression&         exprToSel
        );

        tCIDLib::TVoid StoreExprType
        (
                    TComboBox&              wndSrc
            ,       TEntryField&            wndCompVal
            ,       TCQCExpression&         exprTar
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4NextId
        //      We use the field id of the field config classes as a list window mapping
        //      id. We assign each one on load (and each newly created one) an id from this
        //      list and set it as the list window row id. The field ids are not persisted
        //      so it doesn't matter that we use it for this.
        //
        //  m_lscfgEdit
        //  m_lscfgOrg
        //      We make two copies of the incoming account data, one to edit and one to
        //      keep the original content so we can check for changes.
        //
        //  m_pwndXXX
        //      Some pointers to widgets that we want to deal with via their real types.
        //      We don't own them, we just reference them.
        //
        //  m_pwndClient
        //      We create a generic client window as the tab content and the dialog content
        //      as a child of it.
        //
        //  m_pwndPerType
        //      We create another generic client window to hold the per-field type info as
        //      fields are selected. We load a new dialog description into it based on the
        //      type of the selected field.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4NextId;
        TCQLogicSrvCfg      m_lscfgEdit;
        TCQLogicSrvCfg      m_lscfgOrg;
        TPushButton*        m_pwndAddFld;
        TPushButton*        m_pwndAddSrcFld;
        TPushButton*        m_pwndDelFld;
        TPushButton*        m_pwndDelSrcFld;
        TMultiColListBox*   m_pwndFldList;
        TGenericWnd*        m_pwndPerType;
        TPushButton*        m_pwndSrcFldDn;
        TPushButton*        m_pwndSrcFldUp;
        TMultiColListBox*   m_pwndSrcList;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TLogicSrvTab, TContentTab)
};

