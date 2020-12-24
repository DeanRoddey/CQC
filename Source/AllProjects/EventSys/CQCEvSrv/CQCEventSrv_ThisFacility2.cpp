//
// FILE NAME: CQCEventSrv_ThisFacility2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This file handles some of the grunt work facilty methods, to keep the
//  main file from getting overly large.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCEventSrv.hpp"


// ---------------------------------------------------------------------------
//  TFacCQCEventSrv: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Helpers to find our various event types by their path. The caller can tell us if
//  it must exist or cannot exist and we'll throw accordingly.
//
tCIDLib::TCard4
TFacCQCEventSrv::c4FindEvMon(const  TString&            strRelPath
                            , const tCIDLib::EFindFlags eFlag)
{
    const tCIDLib::TCard4 c4Count = m_colEvMonitors.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        if (m_colEvMonitors[c4Index]->strPath().bCompareI(strRelPath))
            break;
    }

    if (c4Index < c4Count)
    {
        // We found it. If that's bad, then throw
        if (eFlag == tCIDLib::EFindFlags::ThrowIfExists)
        {
            facCQCEventSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kEvSrvErrs::errcCfg_DupPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TString(L"event monitor")
                , strRelPath
            );
        }
    }
     else
    {
        // Not found. If that's bad, then throw
        if (eFlag == tCIDLib::EFindFlags::ThrowIfNotFnd)
        {
            facCQCEventSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kEvSrvErrs::errcCfg_PathNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strRelPath
            );
        }

        // Not found and that's ok, so just return maxcard
        return kCIDLib::c4MaxCard;
    }

    return c4Index;
}


tCIDLib::TCard4
TFacCQCEventSrv::c4FindSchEv(const  TString&            strRelPath
                            , const tCIDLib::EFindFlags eFlag)
{
    const tCIDLib::TCard4 c4Count = m_colSchEvents.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        if (m_colSchEvents[c4Index].strPath().bCompareI(strRelPath))
            break;
    }

    if (c4Index < c4Count)
    {
        // We found it. If that's bad, then throw
        if (eFlag == tCIDLib::EFindFlags::ThrowIfExists)
        {
            facCQCEventSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kEvSrvErrs::errcCfg_DupPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TString(L"scheduled event")
                , strRelPath
            );
        }
    }
     else
    {
        // Not found. If that's bad, then throw
        if (eFlag == tCIDLib::EFindFlags::ThrowIfNotFnd)
        {
            facCQCEventSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kEvSrvErrs::errcCfg_PathNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strRelPath
            );
        }

        // Not found and that's ok, so just return max card
        return kCIDLib::c4MaxCard;
    }

    return c4Index;
}


tCIDLib::TCard4
TFacCQCEventSrv::c4FindTrgEv(const  TString&            strRelPath
                            , const tCIDLib::EFindFlags eFlag)
{
    const tCIDLib::TCard4 c4Count = m_colTrgEvents.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        if (m_colTrgEvents[c4Index].strPath().bCompareI(strRelPath))
            break;
    }

    if (c4Index < c4Count)
    {
        // We found it. If that's bad, then throw
        if (eFlag == tCIDLib::EFindFlags::ThrowIfExists)
        {
            facCQCEventSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kEvSrvErrs::errcCfg_DupPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TString(L"triggered event")
                , strRelPath
            );
        }
    }
     else
    {
        // Not found. If that's bad, then throw
        if (eFlag == tCIDLib::EFindFlags::ThrowIfNotFnd)
        {
            facCQCEventSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kEvSrvErrs::errcCfg_PathNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strRelPath
            );
        }

        // Not found and that's ok, so just return maxcard
        return kCIDLib::c4MaxCard;
    }

    return c4Index;
}



//
//  Tries to load up the event monitor configuration. If it fails it throws. This is
//  called initially to load the events, and it's also called when a client makes any
//  substantial changes. For simple things like pause/resume or adding/deleting single
//  events, we provide methods for that. But if they copy/paste or delete a whole scope,
//  it'd better (more atomic) to just ask us to reload the list.
//
//  We assume the caller has locked if this is not being called on startup.
//

tCIDLib::TVoid TFacCQCEventSrv::LoadEvMonitors(TDataSrvClient& dsclSrc)
{
    try
    {
        // Make sure we start with an empty list
        m_colEvMonitors.RemoveAll();

        //
        //  Start us at the root for event monitors. What we first get is the full
        //  hierarchical path, but we need to convert that to a type relative path since
        //  that's what the data server client accepts, and that's what we use to key
        //  them in our lists as well.
        //
        TString strScope;
        facCQCRemBrws().CreateRelPath
        (
            tCQCRemBrws::strAltXlatEDTypes(tCQCRemBrws::EDTypes::EvMonitor)
            , tCQCRemBrws::EDTypes::EvMonitor
            , strScope
        );

        // Ask the server for a list of all non-scope items under the start scope
        const TCQCSecToken& sectUser = cuctxToUse().sectUser();
        tCIDLib::TStrList colPaths;
        dsclSrc.QueryTree2
        (
            strScope
            , tCQCRemBrws::EDTypes::EvMonitor
            , colPaths
            , kCIDLib::False
            , sectUser
        );

        TCQCEvMonCfg emoncCur;
        const tCIDLib::TCard4 c4Count = colPaths.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TString& strPath = colPaths[c4Index];
            tCIDLib::TCard4 c4SerNum = 0;
            tCIDLib::TEncodedTime   enctLast;
            tCIDLib::TKVPFList colMeta;
            if (!dsclSrc.bReadEventMon(strPath, c4SerNum, enctLast, emoncCur, colMeta, sectUser))
            {
                if (facCQCKit().bLogWarnings())
                {
                    facCQCEventSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kEvSrvMsgs::midStatus_CantLoadEv
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::AppStatus
                        , TString(L"event monitor")
                        , strPath
                    );
                }
                continue;
            }

            //
            //  Add a monitor thread obejct to the list for this this guy. We don't
            //  start them yet.
            //
            m_colEvMonitors.Add(new TCQCEvMonitor(strPath, emoncCur));
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Tries to load up the scheduled event configuration. If it fails it throws. This is
//  called initially to load the events, and it's also called when a client makes any
//  substantial changes. For simple things like pause/resume or adding/deleting single
//  events, we provide methods for that. But if they copy/paste or delete a whole scope,
//  it'd better (more atomic) to just ask us to reload the list.
//
//  We assume the caller has locked if this is not being called on startup.
//

tCIDLib::TVoid TFacCQCEventSrv::LoadSchedEvs(TDataSrvClient& dsclSrc)
{
    try
    {
        m_colSchEvents.RemoveAll();

        //
        //  Start us at the root for event monitors. What we first get is the full
        //  hierarchical path, but we need to convert that to a type relative path since
        //  that's what the data server client accepts.
        //
        TString strScope;
        facCQCRemBrws().CreateRelPath
        (
            tCQCRemBrws::strAltXlatEDTypes(tCQCRemBrws::EDTypes::SchEvent)
            , tCQCRemBrws::EDTypes::SchEvent
            , strScope
        );

        // Ask the server for a list of all non-scope items under the start scope
        const TCQCSecToken& sectUser = cuctxToUse().sectUser();
        tCIDLib::TStrList colPaths;
        dsclSrc.QueryTree2
        (
            strScope, tCQCRemBrws::EDTypes::SchEvent, colPaths, kCIDLib::False, sectUser
        );

        TCQCSchEvent            csrcTmp;
        tCIDLib::TEncodedTime   enctNow = TTime::enctNow();
        const tCIDLib::TCard4   c4Count = colPaths.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TString& strPath = colPaths[c4Index];
            tCIDLib::TCard4 c4SerNum = 0;
            tCIDLib::TEncodedTime enctLast;
            tCIDLib::TKVPFList colMeta;
            if (!dsclSrc.bReadSchedEvent(strPath, c4SerNum, enctLast, csrcTmp, colMeta, sectUser))
            {
                if (facCQCKit().bLogWarnings())
                {
                    facCQCEventSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kEvSrvMsgs::midStatus_CantLoadEv
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::AppStatus
                        , TString(L"scheduled event")
                        , strPath
                    );
                }
                continue;
            }

            //
            //  Note that on the ones that are we have to update for the next
            //  run time, we try to write them back out. This is so that we
            //  don't have to calculate the next time from ever older times
            //  and that many more loops to get up to the right time.
            //
            //  If it's a one-short that has already popped, or has passed,
            //  then delete this object from the repository. Else, add it
            //  since it's still active. No need to update the time since it's
            //  a one shot with a fixed time.
            //
            if (csrcTmp.bIsOneShot())
            {
                if (enctNow >= csrcTmp.enctAt())
                    dsclSrc.DeleteFile(strPath, tCQCRemBrws::EDTypes::SchEvent, sectUser);
                else
                    m_colSchEvents.objAdd(TKeyedCQCSchEvent(strPath, csrcTmp));
            }
             else
            {
                //
                //  It's still good, so update it's time for it's next
                //  pop and put it in our list.
                //
                csrcTmp.CalcNextTime(m_f8Lat, m_f8Long);
                m_colSchEvents.objAdd(TKeyedCQCSchEvent(strPath, csrcTmp));
                tCIDLib::TKVPFList colExtraMeta;
                dsclSrc.WriteScheduledEvent
                (
                    strPath
                    , c4SerNum
                    , enctLast
                    , csrcTmp
                    , kCQCRemBrws::c4Flag_OverwriteOK
                    , colExtraMeta
                    , sectUser
                );
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Tries to load up the triggered event configuration. If it fails it throws. This is
//  called initially to load the events, and it's also called when a client makes any
//  substantial changes. For simple things like pause/resume or adding/deleting single
//  events, we provide methods for that. But if they copy/paste or delete a whole scope,
//  it'd better (more atomic) to just ask us to reload the list.
//
//  We assume the caller has locked if this is not being called on startup.
//
tCIDLib::TVoid TFacCQCEventSrv::LoadTrigEvs(TDataSrvClient& dsclSrc)
{
    try
    {
        // Make sure we start with an empty list
        m_colTrgEvents.RemoveAll();

        //
        //  Start us at the root for event monitors. What we first get is the full
        //  hierarchical path, but we need to convert that to a type relative path since
        //  that's what the data server client accepts.
        //
        TString strScope;
        facCQCRemBrws().CreateRelPath
        (
            tCQCRemBrws::strAltXlatEDTypes(tCQCRemBrws::EDTypes::TrgEvent)
            , tCQCRemBrws::EDTypes::TrgEvent
            , strScope
        );

        // Ask the server for a list of all non-scope items under the start scope
        const TCQCSecToken& sectUser = cuctxToUse().sectUser();
        tCIDLib::TStrList colPaths;
        dsclSrc.QueryTree2
        (
            strScope, tCQCRemBrws::EDTypes::TrgEvent, colPaths, kCIDLib::False, sectUser
        );

        const tCIDLib::TCard4 c4Count = colPaths.c4ElemCount();
        TCQCTrgEvent csrcTmp;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TString& strPath = colPaths[c4Index];
            tCIDLib::TCard4 c4SerialNum = 0;
            tCIDLib::TEncodedTime   enctLast;
            tCIDLib::TKVPFList colMeta;
            if (!dsclSrc.bReadTrigEvent(strPath, c4SerialNum, enctLast, csrcTmp, colMeta, sectUser))
            {
                if (facCQCKit().bLogWarnings())
                {
                    facCQCEventSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kEvSrvMsgs::midStatus_CantLoadEv
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::AppStatus
                        , TString(L"triggered event")
                        , strPath
                    );
                }
                continue;
            }

            m_colTrgEvents.objAdd(TKeyedCQCTrgEvent(strPath, csrcTmp));
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Sets up the sorted 'by reference' collection that provides a time sorted view of
//  the scheduled events list. This way, we don't have to sort the actual list, but
//  can just sort this view into the list, which is far more efficient. We have to
//  re-sort after modifications by client calls or after a scheduled event is queued
//  up and the next time is calculated.
//
tCIDLib::TVoid TFacCQCEventSrv::MakeSortedSchedule()
{
    // Get a copy of the source list count for efficiency
    const tCIDLib::TCard4 c4Count = m_colSchEvents.c4ElemCount();;

    // Flush it. If no originals anymore, we are done, else load up again
    m_colTimeView.RemoveAll();
    if (!c4Count)
        return;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colTimeView.Add(&m_colSchEvents[c4Index]);

    // And now sort it by the time
    m_colTimeView.Sort(&TKeyedCQCSchEvent::eCompByTime);
}
