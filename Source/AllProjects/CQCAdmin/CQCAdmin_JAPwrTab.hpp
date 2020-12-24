//
// FILE NAME: CQCAdmin_JAPwrTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/04/2016
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
//  This is the header for the CQCClient_JAPwrTab.cpp file, which implements a tab
//  window that allows the user to configure a Just Add Power serial ports which makes
//  them available within CQC.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TJAPwrTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TJAPwrTab : public TContentTab, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TJAPwrTab
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TJAPwrTab(const TJAPwrTab&) = delete;
        TJAPwrTab(TJAPwrTab&&) = delete;

        ~TJAPwrTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TJAPwrTab& operator=(const TJAPwrTab&) = delete;
        TJAPwrTab& operator=(TJAPwrTab&&) = delete;


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
            , const TJAPwrCfgList&          gcclEdit
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
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_emacctEdit
        //  m_emacctOrg
        //      We make two copies of the incoming account data, one to edit and
        //      one to keep the original content so we can check for changes.
        //
        //  m_pwndXXX
        //      Some pointers to widgets that we want to deal with via their
        //      real types. We don't own them, we just reference them.
        // -------------------------------------------------------------------
        TJAPwrCfgList       m_japlEdit;
        TJAPwrCfgList       m_japlOrg;
        TMultiColListBox*   m_pwndList;
        TPushButton*        m_pwndAddNew;
        TPushButton*        m_pwndDel;
        TPushButton*        m_pwndDelAll;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TJAPwrTab, TContentTab)
};

