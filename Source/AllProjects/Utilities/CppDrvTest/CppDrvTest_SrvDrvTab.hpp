//
// FILE NAME: CppDrvTest_SrvDrvTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/26/2018
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
//  This is a tab window that goes inserted into the main tabbed window of our
//  interface. This one is the visual interface for the server side driver. It
//  shows the fields, allows them to be modified, allows for various backdoor
//  calls to be tested, etc...
//
//  We just set up a timer and periodically ask the driver directly for his field
//  info. And we queue up commands on him for outgoing operations. The CQC driver
//  architecture is inherently based on a queue and wait (or not) scheme. So the
//  sync issues are pretty minimal.
//
//  The facility object has a pointer to the driver, and we just get it from him
//  when we need it. so that we don't have multiple pointers floating around to
//  have to worry about getting out of sync.
//
//  We implement the in place editor interface so that we can allow writable fields
//  to be edited in place. For non-enumerated string fields we have an ellipsis column
//  to pop up a text editor, since they may need to write multi-line text in some
//  cases. Else, all values are either edited by just typing in the value.
//
//  Later we can add drop down menus for the fields with simple values.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TSrvDrvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TSrvDrvTab : public TTabWindow, public MIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TSrvDrvTab();

        TSrvDrvTab(const TTabWindow&) = delete;

        ~TSrvDrvTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TSrvDrvTab&) = delete;


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
        )   const override;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DriverStartStop
        (
            const   tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid SessionStartStop
        (
            const   tCIDLib::TBoolean       bNewState
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

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidSrc
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoIPEdit
        (
            const   tCIDLib::TCard4         c4ListIndex
            , const tCIDLib::TCard4         c4FldIndex
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadFields();


        // -------------------------------------------------------------------
        //  Private constants
        // -------------------------------------------------------------------
        const tCIDLib::TCard4   c4Col_Name      = 0;
        const tCIDLib::TCard4   c4Col_Type      = 1;
        const tCIDLib::TCard4   c4Col_SemType   = 2;
        const tCIDLib::TCard4   c4Col_Access    = 3;
        const tCIDLib::TCard4   c4Col_SetValue  = 4;
        const tCIDLib::TCard4   c4Col_Value     = 5;
        const tCIDLib::TCard4   c4Col_Count     = 6;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLastRunState
        //      We have to keep the text of the start/stop button correct. So we need
        //      to keep up with the last running/not running state we saw the driver
        //      in, in order to know when that state has changed. It starts initially
        //      at false, and the button starts with Start text, so it's ready for an
        //      initial start.
        //
        //  m_bLastSessState
        //      Similar to last run state above, we keep up with the last session
        //      started/stopped state we got from the main window.
        //
        //  m_c4FldListId
        //      We store this when we query the fields. If the driver's version of it
        //      is different, we know new fields have been registered and we have to
        //      reload our field list.
        //
        //  m_colFields
        //      The list of field definitions we get from the driver when it starts
        //      up.
        //
        //  m_eLastState
        //      The last driver state we saw, so that we will know if it changed and
        //      can update our status display.
        //
        //  m_colValues
        //  m_fcolSerialNums
        //  m_fcolChanged
        //  m_fcolErrors
        //      These are lists that we maintain for our polling of field changes from
        //      the driver. Upon starting up the driver, when we get the fields, we
        //      allocate these to the size of the field list, and initialize them. Then
        //      we just pass them back in on each poll.
        //
        //  m_pwndClient
        //      We use a generic client window to hold our controls. We keep this
        //      sized to fit our tab area, and he keeps the controls sized to fit
        //      him.
        //
        //  m_pwndXXX
        //      Typed pointers to child widgets (of the client window of course) that
        //      we need to interact with directly.
        //
        //  m_strEllipsis
        //  m_strError
        //      Some constant strings we display
        //
        //  m_tmidUpdate
        //      We use a timer to keep our driver info up to date. This is the id of
        //      the timer we create for that.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLastRunState;
        tCIDLib::TBoolean       m_bLastSessState;
        tCIDLib::TCard4         m_c4FieldCount;
        tCIDLib::TCard4         m_c4FieldListId;
        tCQCKit::TFldDefList    m_colFields;
        tCIDLib::TStrList       m_colValues;
        tCQCKit::EDrvStates     m_eLastState;
        tCIDLib::TCardArray     m_fcolSerialNums;
        tCIDLib::TBoolArray     m_fcolChanged;
        tCIDLib::TBoolArray     m_fcolErrors;
        TGenericWnd*            m_pwndClient;
        TStaticText*            m_pwndLimits;
        TStaticText*            m_pwndState;
        TMultiColListBox*       m_pwndFields;
        const TString           m_strEllipsis;
        const TString           m_strError;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSrvDrvTab, TTabWindow)
};

#pragma CIDLIB_POPPACK
