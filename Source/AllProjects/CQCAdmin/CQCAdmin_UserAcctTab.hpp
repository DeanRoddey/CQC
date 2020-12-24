//
// FILE NAME: CQCAdmin_UserAcctTab.hpp
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
//  This is the header for the CQCClient_UserAcctTab.cpp file, which implements a
//  tab window that allows the user to configure a user account. We also include the
//  key mapped actions for the user as part of this, though it's separate data. If the
//  user ever edits, we load the keymap info. If not, both the original and edited stay
//  empty and it shows up as no change anyway. So we don't pay the price of loading until
//  we have to.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TUserAcctTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TUserAcctTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TUserAcctTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TUserAcctTab(const TUserAcctTab&) = delete;
        TUserAcctTab(TUserAcctTab&&) = delete;

        ~TUserAcctTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TUserAcctTab& operator=(const TUserAcctTab&) = delete;
        TUserAcctTab& operator=(TUserAcctTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
            , const TCQCUserAccount&        uaccEdit
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


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bHaveKMaps
        //      Used to fault in the key maps upon first edit.
        //
        //  m_kmEdit
        //  m_kmOrg
        //      It the user edits the keymappings, we load the current values into
        //      Org, and copy to Edit and then let the user edit those. On subsequent
        //      edit requires we just pass back in Edit. m_bHaveKMaps let's us know if
        //      we have already loaded them.
        //
        //  m_pwndXXX
        //      Some pointers to widgets that we want to deal with via their
        //      real types. We don't own them, we just reference them.
        //
        //  m_uaccEdit
        //  m_uaccOrg
        //      We make two copies of the incoming user account data, one to edit
        //      and one to keep the original content so we can check for changes.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bHaveKMaps;
        TCQCKeyMap          m_kmEdit;
        TCQCKeyMap          m_kmOrg;
        TEntryField*        m_pwndDefIntfName;
        TMultiEdit*         m_pwndDescr;
        TPushButton*        m_pwndEdKeyMaps;
        TCheckBox*          m_pwndEnableTms;
        TComboBox*          m_pwndEndTm;
        TEntryField*        m_pwndFName;
        TEntryField*        m_pwndLogin;
        TEntryField*        m_pwndLName;
        TPushButton*        m_pwndResetPW;
        TComboBox*          m_pwndRole;
        TPushButton*        m_pwndSelTmpl;
        TComboBox*          m_pwndStartTm;
        TEntryField*        m_pwndWebPassword;
        TCQCUserAccount     m_uaccEdit;
        TCQCUserAccount     m_uaccOrg;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TUserAcctTab, TContentTab)
};




