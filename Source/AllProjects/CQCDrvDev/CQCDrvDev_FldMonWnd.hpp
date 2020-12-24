//
// FILE NAME: CQCDrvDev_FldMonWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/24/2003
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
//  This is the header for the file file CQCDrvDev_FldMonWnd.cpp file, which
//  implements a window that lets the user monitor the fields of a driver,
//  look at their values, and emulate the writing of new values as a client
//  would.
//
//  The driver glue class, the generic driver that provides the glue code
//  between the C++ driver world and the macro based driver world, provides
//  some statics that allow us to interact with him. He create a mutex to
//  sync access to this data, and he provides a pointer to the instance that
//  was created by the macro class being debugged in the macro language IDE.
//
//  So we use a timer and on each round, we sync, then see if the driver
//  pointer is set. If so, then we lock the driver itself, update our data
//  if needed, then release the driver lock and then the static member lock.
//
//  We watch for the driver to show up and go away and update automatically.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once

class TCQCDrvDevFrameWnd;

// ---------------------------------------------------------------------------
//  CLASS: TCQCFldMonWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCFldMonWnd : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCFldMonWnd();

        ~TCQCFldMonWnd();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Create
        (
                    TCQCDrvDevFrameWnd* const pwndOwner
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActivationChange
        (
            const   tCIDLib::TBoolean       bState
            , const tCIDCtrls::TWndHandle   hwndOther
        )   override;

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
            const   tCIDCtrls::TTimerId     tmidToDo
        );


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

        tCIDLib::TVoid LoadFields();

        tCIDLib::TVoid SetButtonStates();

        tCIDLib::TVoid ShowCurFldVal();

        tCIDLib::TVoid WriteToField();

        tCIDLib::TVoid WriteValue
        (
            const   TCQCFldDef&             flddTarget
            , const TString&                strToWrite
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLastTime
        //      We need to know if the data showed up or disapeared on each
        //      round of the timer, so we need to know what the state was
        //      last time. If the same as last time we either do nothing (if
        //      it's not present) or either do an init if it just showed up
        //      or a refresh if it was there before. True means it was there
        //      last time.
        //
        //  m_c4FldListId
        //      If the driver changes the list of fields, it will bump up
        //      it's field list id, so we have to keep up with the id of
        //      the list we are monitoring.
        //
        //  m_c4FldSerialNum
        //      Each field has a serial number that let's us know if it has
        //      been changed because it gives us back the number each time
        //      we read it. So each time we update the currently selected
        //      field, we update this with the new serial number.
        //
        //  m_c4DrvSerialNum
        //      The driver maintains a serial number that get's bumped when
        //      any field is changed. So we keep up with the last serial
        //      number we saw, which let's us quickly check for changes.
        //
        //  m_colFields
        //      The list of fields that we are currently monitoring. The
        //      items in the field list are sorted by name, but their id
        //      field is an index into this list.
        //
        //  m_pdcmdWait
        //      The command we queued up asynchronously on the driver, which
        //      our timer callback will check until it's completed.
        //
        //  m_pwndOwner
        //      A pointer to the main frame window that owns us. We will post
        //      a message to him when we need to be closed and he'll clean
        //      us up.
        //
        //  m_pwndXXX
        //      For convenience, we get some typed pointers to some of our
        //      widgets. we don't own them, we just look at them.
        //
        //  m_tmFmt
        //      A temp time object for formatting out the value of time fields.
        //      We pre-set it up with the desired format in the ctor.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean               m_bLastTime;
        tCIDLib::TCard4                 m_c4DrvSerialNum;
        tCIDLib::TCard4                 m_c4FldListId;
        tCIDLib::TCard4                 m_c4FldSerialNum;
        TCQCServerBase::TQueryFieldList m_colFields;
        tCQCKit::EDrvStates             m_eCurState;
        TCQCServerBase::TDrvCmd*        m_pdcmdWait;
        TCQCDrvDevFrameWnd*             m_pwndOwner;
        TEntryField*                    m_pwndAccess;
        TPushButton*                    m_pwndChange;
        TStaticMultiText*               m_pwndLimits;
        TListBox*                       m_pwndList;
        TEntryField*                    m_pwndName;
        TPushButton*                    m_pwndQText;
        TPushButton*                    m_pwndSendCmd;
        TStaticText*                    m_pwndStatus;
        TEntryField*                    m_pwndType;
        TStaticMultiText*               m_pwndValue;
        tCIDCtrls::TTimerId             m_tmidUpdate;
        TTime                           m_tmFmt;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldMonWnd,TFrameWnd)
};

