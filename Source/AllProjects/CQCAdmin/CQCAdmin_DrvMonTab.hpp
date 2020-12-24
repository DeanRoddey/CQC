//
// FILE NAME: CQCAdmin_DrvMonTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/07/2016
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
//  This is the header for the CQCClient_DrvMonTab.cpp file, which implements a
//  tab window that allows the user to monitor and do basic interactions with a
//  device driver. We do our interactions with the driver via the common polling
//  engine provided by our facility class.
//
//  Since we are using the common polling engine, we don't need any bgn thread to
//  do anything. We can get to all field info immediately without any blocking,
//  so we use a timer to keep ourself updated.
//
//  We have to have a per-field class of our own, just a simple one but we need it.
//  In theory we could use the poll engine poll info object, since it includes the
//  field def info that we need to drive most of the list window columns. But we
//  can only put readable fields into the poll engine. So we need a way to keep
//  a (redundant for readable fields) field def and the poll info objects for each
//  field.
//
//  We store the original index of the objects as the per-row id in the list window
//  so that we can get back to the original field (the list can be sorted by any of
//  the columns.)
//
//  We allow for setting the value of writeable fields. This is done in a couple ways:
//
//  1. If they click on the value cell, we let them edit in place
//  2. If they click a 'set value' type column to the left of the value, we either
//     pop up a dialog or drop down a menu, depending on the type of the field.
//
//  If it's a write only field, if the do #1, then we clear the value again once it's
//  been written, since it might otherwise imply that we are seeing the current value of
//  a write only field.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TDrvMonTabPFI
// PREFIX: pfi
// ---------------------------------------------------------------------------
class TDrvMonTabPFI
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TDrvMonTabPFI
        (
            const   TCQCFldDef&             flddThis
        );

        TDrvMonTabPFI(const TDrvMonTabPFI&) = delete;
        TDrvMonTabPFI(TDrvMonTabPFI&&) = delete;

        ~TDrvMonTabPFI();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDrvMonTabPFI& operator=(const TDrvMonTabPFI&) = delete;
        TDrvMonTabPFI& operator=(TDrvMonTabPFI&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bUpdate
        (
                    TCQCPollEngine&         polleToUse
        );

        tCIDLib::TVoid Init
        (
            const   TString&                strMoniker
            ,       TCQCPollEngine&         polleToUse
        );


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_cfpiThis
        //      A polling engine object we set up so as to track our field in the
        //      polling engine.
        //
        //  m_flddThis
        //      The field definition info. This will also be redundantly available
        //      in the m_cfpiThis object if this is a readable field.
        //
        //  m_strLastVal
        //      The last value we saw. We need a place to format it out in order to
        //      set it on the list window.
        //
        //  m_strTmpFmt
        //      Just used for temporary purposes when formatting out field values.
        //
        //  m_tmFmt
        //      Used for formatting out time values
        // -------------------------------------------------------------------
        TCQCFldPollInfo     m_cfpiThis;
        TCQCFldDef          m_flddThis;
        TString             m_strLastVal;
        TString             m_strTmpFmt;
        TTime               m_tmFmt;
};



// ---------------------------------------------------------------------------
//  CLASS: TDrvMonTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TDrvMonTab : public TContentTab, public MIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TDrvMonTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TDrvMonTab(const TDrvMonTab&) = delete;
        TDrvMonTab(TDrvMonTab&&) = delete;

        ~TDrvMonTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDrvMonTab& operator=(const TDrvMonTab&) = delete;
        TDrvMonTab& operator=(TDrvMonTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
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

        tCIDLib::TVoid Destroyed() final;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidSrc
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TFldList = TRefVector<TDrvMonTabPFI>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ConnToDriver();

        tCIDLib::TVoid DoIPEdit
        (
            const   tCIDLib::TCard4         c4ListIndex
            , const tCIDLib::TCard4         c4FldIndex
        );

        tCIDLib::TVoid DoVisEdit
        (
            const   tCIDLib::TCard4         c4ListIndex
            , const tCIDLib::TCard4         c4FldIndex
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditTrigger();

        tCIDLib::TVoid Poll
        (
            const   tCIDLib::TBoolean       bFirst
        );

        tCIDLib::TVoid RemoveAllTrigs();

        tCIDLib::TVoid WriteFld
        (
            const   TString&                strFldName
            , const TString&                strValue
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4XXXId
        //      Some ids we get back when we query fields of the currently
        //      selected driver in order to load up the field list. We can
        //      use these to quickly check if anything has changed that would
        //      force us to update the driver or field lists.
        //
        //  m_colFldList
        //      A list of the per-field objects we need to keep up with our fields
        //      and link them to the polling engine. We store the indices into this
        //      list as the list window row ids for quick association back to this
        //      list from the selected field.
        //
        //      There is an entry in this list even if the field is read only, but
        //      those non-readable fields are not set up for polling.
        //
        //  m_eDrvState
        //      The last driver state we saw, which controls what we do. Until we get it
        //      to connected state, we keep trying to connect. Once connected, we start
        //      polling.
        //
        //  m_eVerbosity
        //      The last driver verbosity we saw, which gets displayed in a combo box as
        //      the current verbosity.
        //
        //  m_pwndXXX
        //      Some pointers to widgets that we want to deal with via their
        //      real types. We don't own them, we just reference them.
        //
        //  m_strDropDownVal
        //      The text we put into the value selection column if the row will use a
        //      drop down menu to select the value.
        //
        //  m_strHost
        //      The host the driver is configured on. Gotten from the path, and used
        //      to create the CQCServer client proxy we need to monitor the driver.
        //
        //  m_strMoniker
        //      The moniker of the driver we are to monitor. Gotten from the path.
        //
        //  m_strPopupVal
        //      The text we put into the value selection column if the row will use a
        //      popup dialog to select the value.
        //
        //  m_strVerboseFldName
        //      We watch the verbosity field, so we store the name up front so we can
        //      efficiently check.
        //
        //  m_tmidMon
        //      The id for the timer we start up to monitor our driver and its
        //      fields.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldListId;
        tCIDLib::TCard4         m_c4DriverId;
        tCIDLib::TCard4         m_c4DrvListId;
        TFldList                m_colFldList;
        tCQCKit::EDrvStates     m_eDrvState;
        tCQCKit::EVerboseLvls   m_eVerbosity;
        TPushButton*            m_pwndClearTrigs;
        TPushButton*            m_pwndCopyVal;
        TPushButton*            m_pwndDriverInfo;
        TPushButton*            m_pwndEditTrig;
        TMultiColListBox*       m_pwndFldList;
        TStaticText*            m_pwndLimits;
        TStaticText*            m_pwndMoniker;
        TStaticText*            m_pwndStatus;
        TComboBox*              m_pwndVerbosity;
        const TString           m_strDropDownVal;
        TString                 m_strHost;
        TString                 m_strMoniker;
        const TString           m_strPopupVal;
        const TString           m_strVerboseFldName;
        tCIDCtrls::TTimerId     m_tmidMon;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDrvMonTab, TContentTab)
};


