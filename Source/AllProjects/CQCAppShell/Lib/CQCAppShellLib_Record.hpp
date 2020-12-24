//
// FILE NAME: CQCAppShell_Record.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2001
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
//  This is the header for the CQCAppShell_Record.cpp file, which implements
//  a simple derivative of the core app shell record class. We need to also
//  store some extra info so that when we get callbacks to do shutdowns
//  on the processes, we can get access to its admin object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCAppShRecord
//  PREFIX: asr
// ---------------------------------------------------------------------------
class TCQCAppShRecord : public TAppShRecord
{
    public :
        // -------------------------------------------------------------------
        //  A status we use during shutdown, to know what we've managed to
        //  get done so far.
        // -------------------------------------------------------------------
        enum class EStopStatus
        {
            None
            , PostedEvent
            , NoEvent
            , Killed
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCAppShRecord
        (
            const   TString&                strTitle
            , const TString&                strAppName
            , const TString&                strAppPath
            , const TString&                strParms
            , const TString&                strAdminPath
            , const tCIDAppSh::EAppLevels   eLevel
        );

        TCQCAppShRecord(const TCQCAppShRecord&) = delete;
        TCQCAppShRecord(TCQCAppShRecord&&) = delete;

        ~TCQCAppShRecord();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCAppShRecord& operator=(const TCQCAppShRecord&) = delete;
        TCQCAppShRecord& operator=(TCQCAppShRecord&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4LastCycleTime() const;

        tCIDLib::TCard4 c4LastCycleTime
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4LastLogTime() const;

        tCIDLib::TCard4 c4LastLogTime
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4LastULogTime() const;

        tCIDLib::TCard4 c4LastULogTime
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        EStopStatus eStopStatus() const;

        EStopStatus eStopStatus
        (
            const   EStopStatus             eToSet
        );

        const TError& errLast() const;

        const TError& errLast
        (
            const   TError&                 errToSet

        );

        const TResourceName& rsnWait() const;

        const TString& strAdminPath() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4LastCycleTime
        //  m_c4LastLogTime
        //  m_c4LastULogTime
        //      The last time we logged an error or cycle for this app.
        //
        //  m_errLast
        //      The last error reported on this app. The shell throttles
        //      repeated errors of the same type to one per a given period
        //      of time, to avoid writing huge numbers of redundant messages.
        //
        //  m_eStopStatus
        //      Used during shutdown to know what steps have been taken so
        //      far.
        //
        //  m_rsnWait
        //      The resource name for the event that the main thread of the
        //      process will be blocked on. We use this to shut it down by
        //      triggering the event.
        //
        //  m_strAdminPath
        //      This is the name server path to the core admin binding for
        //      this process.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4LastCycleTime;
        tCIDLib::TCard4 m_c4LastLogTime;
        tCIDLib::TCard4 m_c4LastULogTime;
        EStopStatus     m_eStopStatus;
        TError          m_errLast;
        TResourceName   m_rsnWait;
        TString         m_strAdminPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCAppShRecord,TAppShRecord)
};

#pragma CIDLIB_POPPACK


