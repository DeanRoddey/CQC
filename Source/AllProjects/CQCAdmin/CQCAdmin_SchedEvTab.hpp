//
// FILE NAME: CQCAdmin_SchedEvTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/22/2016
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
//  This is the header for the CQCClient_SchedEvTab.cpp file, which implements a
//  tab window that allows the user to configure a scheduled event.
//
//  This one has some special concerns since there are multiple schedule types and
//  each one has different controls required. So, we have a main dialog resource
//  and others for each type. The main one has the fixed stuff at the top and the
//  per-type ones have their stuff at the bottom, and all of their controls have
//  ids starting at 1010 or above. So, when the type changes, we remove all controls
//  with that id or higher and then create the controls for the new one, which will
//  slot themselves in under the fixed stuff.
//
//  Only one per-type set of controls are ever in existence at once, so no id clashes
//  will happen. We never keep pointers to them around. And we don't depend on any
//  event notifications from them so again ids are not an issue.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TSchedEvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TSchedEvTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TSchedEvTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TSchedEvTab(const TSchedEvTab&) = delete;
        TSchedEvTab(TSchedEvTab&&) = delete;

        ~TSchedEvTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSchedEvTab& operator=(const TSchedEvTab&) = delete;
        TSchedEvTab& operator=(TSchedEvTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
            , const TCQCSchEvent&           csrcEdit
            , const tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
        );

        tCIDLib::TVoid UpdatePauseState
        (
            const   tCIDLib::TBoolean       bToSet
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

        tCIDLib::TVoid LoadIncoming();

        tCIDLib::TVoid LoadNumCombo
        (
            const   tCIDLib::TInt4          i4Min
            , const tCIDLib::TInt4          i4Max
            ,       TComboBox&              wndTar
        );

        tCIDLib::TVoid UpdateNextTime
        (
            const   tCIDLib::TResId         ridTarget
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_f8Lat
        //  m_f8Long
        //      The calling app gives us the configured lat/long info. We need this
        //      in case we need to set a sun based event.
        //
        //  m_csrcEdit
        //  m_csrcOrg
        //      We make two copies of the incoming info, one that we can edit and
        //      another that holds the original content, so that we can check for
        //      changes.
        //
        //  m_pwndXXX
        //      Some pointers to widgets that we want to deal with via their
        //      real types. We don't own them, we just reference them. Some
        //      are in the m_apwndXXX members. These are only for the fixed ones.
        //      For the per-type ones, we just get pointers to them as required.
        //
        //  m_strPrefTxt_XXX
        //      Some strings that we pre-load and swap into the field value
        //      prefix control depending on what type of filter is selected.
        // -------------------------------------------------------------------
        tCIDLib::TFloat8    m_f8Lat;
        tCIDLib::TFloat8    m_f8Long;
        TCQCSchEvent        m_csrcEdit;
        TCQCSchEvent        m_csrcOrg;
        TPushButton*        m_pwndEditAct;
        TComboBox*          m_pwndEvType;
        TCheckBox*          m_pwndLogRuns;
        TPushButton*        m_pwndTest;
        TEntryField*        m_pwndTitle;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSchedEvTab, TContentTab)
};



