//
// FILE NAME: CQCEvCl_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCEvCl_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCEvCl,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCEvCl
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCEvCl: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCEvCl::TFacCQCEvCl() :

    TFacility
    (
        L"CQCEvCl"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}

TFacCQCEvCl::~TFacCQCEvCl()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCEvCl: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Conveniences to map between the event server's event types enum and the remote
//  browser's more general data type enum.
//
tCQCKit::EEvSrvTypes
TFacCQCEvCl::eBrwsDTypeToEvType(const   tCQCRemBrws::EDTypes    eToXlat
                                , const tCIDLib::TBoolean       bThrowIfNot) const
{
    tCQCKit::EEvSrvTypes eRet = tCQCKit::EEvSrvTypes::None;
    if (eToXlat == tCQCRemBrws::EDTypes::EvMonitor)
        eRet = tCQCKit::EEvSrvTypes::EvMonitor;
    else if (eToXlat == tCQCRemBrws::EDTypes::SchEvent)
        eRet = tCQCKit::EEvSrvTypes::SchEvent;
    else if (eToXlat == tCQCRemBrws::EDTypes::TrgEvent)
        eRet = tCQCKit::EEvSrvTypes::TrgEvent;

    if ((eRet == tCQCKit::EEvSrvTypes::None) && bThrowIfNot)
    {
        facCQCEvCl().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kEvClErrs::errcGen_NotAnEventDType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , tCQCRemBrws::strXlatEDTypes(eToXlat)
        );
    }
    return eRet;
}

tCQCRemBrws::EDTypes
TFacCQCEvCl::eEvTypeToBrwsDType(const   tCQCKit::EEvSrvTypes    eToXlat
                                , const tCIDLib::TBoolean       bThrowIfNot) const
{
    tCQCRemBrws::EDTypes eRet = tCQCRemBrws::EDTypes::Count;

    if (eToXlat == tCQCKit::EEvSrvTypes::EvMonitor)
        eRet = tCQCRemBrws::EDTypes::EvMonitor;
    else if (eToXlat == tCQCKit::EEvSrvTypes::SchEvent)
        eRet = tCQCRemBrws::EDTypes::SchEvent;
    else if (eToXlat == tCQCKit::EEvSrvTypes::TrgEvent)
        eRet = tCQCRemBrws::EDTypes::TrgEvent;

    if (eRet == tCQCRemBrws::EDTypes::Count)
    {
        facCQCEvCl().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kEvClErrs::errcGen_BadEvType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , TInteger(tCIDLib::i4EnumOrd(eToXlat))
        );
    }
    return eRet;
}


//
//  A convenience method to pause/resume an event. Since there are a few places from
//  which this can be done (XML GW, user action, admin client, possibly others) this
//  keeps it in one place in case the requirements change.
//
//  The path is the type-relative path used by the data server and the event server.
//  The caller also has to tell us the data type, just as a sanity check.
//
tCIDLib::TVoid
TFacCQCEvCl::SetEvPauseState(const  tCQCKit::EEvSrvTypes    eEvType
                            , const TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4ChangeSerNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            , const tCIDLib::TBoolean       bPause
                            , const TCQCSecToken&           sectUser)
{
    try
    {
        CIDAssert
        (
            (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
            || (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
            || (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
            , L"The passed type is not a known event server event type"
        );

        // Get a data server client and ask it to do the pause for us.
        TDataSrvClient dsclSet;
        dsclSet.SetEventPauseState
        (
            eEvType, strRelPath, c4ChangeSerNum, enctLastChange, bPause, sectUser
        );

        //
        //  That works, set get an event server proxy and tell it that we changed the
        //  pause state of the event, so that it will make the actual live change.
        //  It will go to the data server and read the new info.
        //
        tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy();
        orbcES->PauseEvent(eEvType, strRelPath, c4ChangeSerNum, bPause, sectUser);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  A convenience method to set the period and start time of a periodic scheduled event.
//
tCIDLib::TVoid
TFacCQCEvCl::SetPeriodicEvTime( const   TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                , const tCIDLib::TEncodedTime   enctStart
                                , const tCIDLib::TCard4         c4Period
                                , const TCQCSecToken&           sectUser)
{
    try
    {
        // Get a data server and ask it to set the time
        TDataSrvClient dsclSet;
        dsclSet.SetPeriodicEvTime
        (
            strRelPath, c4ChangeSerNum, enctLastChange, enctStart, c4Period, sectUser
        );

        // Now let's tell the event server to update this event
        tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy();
        orbcES->UpdateEvent
        (
            tCQCKit::EEvSrvTypes::SchEvent, strRelPath, c4ChangeSerNum, sectUser
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  A convenience to update the hour/minute and day of a regular scheduled event (i.e.
//  the do it at this time on these days type.) This can be done from a couple different
//  places so we want to consolidate the logic in one place.
//
//  The mask can be zero in which case it will not be updated. It will only be used
//  if the event type is one that uses the mask (weekly or monthly.)
//
tCIDLib::TVoid
TFacCQCEvCl::SetScheduledEvTime(const   TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                , const tCIDLib::TCard4         c4Day
                                , const tCIDLib::TCard4         c4Hour
                                , const tCIDLib::TCard4         c4Min
                                , const tCIDLib::TCard4         c4Mask
                                , const TCQCSecToken&           sectUser)
{
    try
    {
        // Get a data server and ask it to set the scheduled info
        TDataSrvClient dsclSet;
        dsclSet.SetScheduledEvTime
        (
            strRelPath, c4ChangeSerNum, enctLastChange, c4Day, c4Hour, c4Min, c4Mask, sectUser
        );

        // Now let's tell the event server to update this event
        tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy();
        orbcES->UpdateEvent
        (
            tCQCKit::EEvSrvTypes::SchEvent, strRelPath, c4ChangeSerNum, sectUser
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  A convenience to update the offset on a sunrise/sunset based scheduled event. We
//  read the event from the data server, update it, write it back, and then inform
//  the event server of the change.
//
tCIDLib::TVoid
TFacCQCEvCl::SetSunBasedEvOffset(   const   TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4ChangeSerNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const tCIDLib::TInt4          i4Offset
                                    , const TCQCSecToken&           sectUser)
{
    try
    {
        // Get a data server client and ask it to set the offset
        TDataSrvClient dsclSet;
        dsclSet.SetSunBasedEvOffset
        (
            strRelPath, c4ChangeSerNum, enctLastChange, i4Offset, sectUser
        );

        // Now let's tell the event server to update this event
        tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy();
        orbcES->UpdateEvent
        (
            tCQCKit::EEvSrvTypes::SchEvent, strRelPath, c4ChangeSerNum, sectUser
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Gets a client proxy for the event server.
//
//  Note that it's returning a reference counted admin proxy wrapper here
//  not the raw proxy object pointer. This lets us use by-value semantics
//  on these.
//
//  If we fail, we throw an exception
//
tCQCEvCl::TEventSrvProxy
TFacCQCEvCl::orbcEventSrvProxy(const tCIDLib::TCard4 c4WaitUpTo) const
{
    TEventSrvClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TEventSrvClientProxy>
    (
        TEventSrvClientProxy::strBinding, c4WaitUpTo
    );

    if (!porbcProxy)
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kEvClErrs::errcRem_EvServerNotAvail
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }
    return tCQCEvCl::TEventSrvProxy(porbcProxy);
}


