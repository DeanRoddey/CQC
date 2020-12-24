//
// FILE NAME: CQCAdmin_EMailAcctTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/11/2015
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
//  This is the header for the CQCClient_EMailAcctTab.cpp file, which implements a
//  tab window that allows the user to configure a user account.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TEMailAcctTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TEMailAcctTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TEMailAcctTab
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TEMailAcctTab(const TEMailAcctTab&) = delete;
        TEMailAcctTab(TEMailAcctTab&&) = delete;

        ~TEMailAcctTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TEMailAcctTab&) = delete;
        tCIDLib::TVoid operator=(TEMailAcctTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TCQCEMailAccount&       emacctEdit
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
        //  Private data members
        //
        //  m_pwndXXX
        //      Some pointers to widgets that we want to deal with via their
        //      real types. We don't own them, we just reference them.
        //
        //  m_emacctEdit
        //  m_emacctOrg
        //      We make two copies of the incoming account data, one to edit and
        //      one to keep the original content so we can check for changes.
        // -------------------------------------------------------------------
        TCQCEMailAccount    m_emacctEdit;
        TCQCEMailAccount    m_emacctOrg;
        TEntryField*        m_pwndFromAddr;
        TEntryField*        m_pwndName;
        TEntryField*        m_pwndPassword;
        TEntryField*        m_pwndPort;
        TEntryField*        m_pwndServer;
        TComboBox*          m_pwndType;
        TEntryField*        m_pwndUser;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEMailAcctTab, TContentTab)
};




