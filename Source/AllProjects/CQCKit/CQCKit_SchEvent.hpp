//
// FILE NAME: CQCKit_SchEvent.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/19/2004
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
//  This is the header for the CQCEventLib_SchEvent.cpp file, which implements
//  a simple derivative of the base command source class, i.e. it is an
//  'action'. The base class provides all the support for defining a user
//  action. All we have to do is add the stuff that is specific to our kinds
//  of actions, which in this case means the scheduling info.
//
//  There are a set of scheduling types, and the meaning of the members of
//  the scheduled action item depends on what type.
//
//      ESchType_Once
//          m_enctAt indicates the actual time at which it should be invoked, and it
//          is persisted since we have to keep up with the time to do this one.
//
//      ESchType_Daily
//          m_c4Hour and m_c4Min indicate the time of the day to do it. m_enctAt is
//          only for runtime and is set to the next scheduled time.
//
//      EShcType_Weekly
//          m_c4Hour and m_c4Min indicate the time of the day to do it. m_c4Mask has
//          the low 7 bits indicating which days of the week to do it. m_enctAt is
//          only for runtime and is set to the next scheduled time.
//
//      EShcType_Monthly
//          m_c4Hour and m_c4Min indicate the time of the day to do it. m_c4Day indicates
//          the day of the month to do it. m_c4Mask has the low 12 bits indicating which
//          months of the year to do it. m_enctAt is only for runtime and is set to the
//          next scheduled time.
//
//      ESchType_MinPeriod
//      ESchType_HrPeriod
//      ESchType_DayPeriod
//          m_c4Day, m_c4Hour, or m_c4Min will respectively hold the number of units in
//          the period.
//
//          For day periods hour holds the hour at which it should run. For hour periods
//          minute holds the minute to run it at.
//
//          m_enctAt is persisted with the next time that this should get invoked. If
//          it is beyond that time when we come up, we need this to calculate the next time.
//
//      ESchType_Sunrise
//      ESchType_Sunset
//          It happens once a day at sunrise or sunset. We do use the persisted next time
//          in that when we re-load, we don't re-calc the sunrise or set, we just assume
//          that the stored time is the time the last one was scheduled, so we know whether
//          it's time to re-schedule or not. m_i4Offset is used to store an optional offset
//          (in minutes) from sunrise or sunset.
//
//  For the periodic items (and sunrise/sunset), we have to be able to remember when
//  it was last invoked, so that we know when to do it next time. Actually, what we
//  do is to store the next scheduled time.
//
//  And, for consistency, we just use the same field to hold the next scheuled time
//  for all of them, though we only really need the peristence of the time for the
//  periodic (and one-shot) ones. This allows us to just sort the list by next
//  invocation time so we know that the item on the top of the list holds the next
//  target time.
//
//
//  Some folks need to keep lists of these keyed on the path, so we provide a simple
//  derivative that adds a path. It should be the type relative path, i.e. the one
//  used by the data server for reading/writing.
//
// CAVEATS/GOTCHAS:
//
//  1)  As of 5.0, we moved them to the standard hierarchical storage. They used to have
//      a unique id that was the actual id, plus a human readable name, and actions
//      referred to them via the id. Now the path is the unique id. During the upgrade
//      process the old id will be read in and the events will be moved to their new
//      location under the /User section of the triggered events scope, with the id
//      as the name.
//
//      Actions that reference the ids will be updated so put that path prefix before
//      the id, making them now correctly refer to them in the new scheme. The user can
//      rename them after the ugprade if they want to give them more meaningful names.
//
//  2)  We want to move the start time up to the most recent next time each time it's
//      edited, so that when they are next loaded, it reduces the time required to find
//      the next start time.
//
//      But it also will make the event seem like it has changed, just because the current
//      stuff is restored on the object. Since the start time is only exposed for one shot
//      events, we ony count it for those when doing equality testing.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCQCSchEvent
// PREFIX: csrc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCSchEvent : public TCQCStdCmdSrc
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bParseDHM
        (
            const   TString&                strDHM
            ,       tCIDLib::TCard4&        c4Day
            ,       tCIDLib::TCard4&        c4Hour
            ,       tCIDLib::TCard4&        c4Min
        );

        static tCIDLib::TBoolean bParseMask
        (
            const   TString&                strMask
            ,       tCIDLib::TCard4&        c4MaskBits
            ,       tCIDLib::TCard4&        c4BitCount
        );


        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSchEvent();

        TCQCSchEvent
        (
            const   TString&                strDescr
        );

        TCQCSchEvent(const TCQCSchEvent&) = default;
        TCQCSchEvent(TCQCSchEvent&&) = default;

        ~TCQCSchEvent();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSchEvent& operator=(const TCQCSchEvent&) = default;
        TCQCSchEvent& operator=(TCQCSchEvent&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCSchEvent&           csrcSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCSchEvent&           csrcSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsOneShot() const;

        tCIDLib::TBoolean bIsPeriodic() const;

        tCIDLib::TBoolean bIsScheduled() const;

        tCIDLib::TBoolean bIsSunBased() const;

        tCIDLib::TBoolean bLoggable() const;

        tCIDLib::TBoolean bLoggable
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bPaused() const;

        tCIDLib::TBoolean bPaused
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4Day() const;

        tCIDLib::TCard4 c4Hour() const;

        tCIDLib::TCard4 c4Mask() const;

        tCIDLib::TCard4 c4Minute() const;

        tCIDLib::TCard4 c4Period() const;

        tCIDLib::TCard4 c4SubPeriod() const;

        tCIDLib::TVoid CalcNextTime
        (
            const   tCIDLib::TFloat8&       f8Lat
            , const tCIDLib::TFloat8&       f8Long
        );

        tCIDLib::TEncodedTime enctAt() const;

        tCIDLib::TEncodedTime enctAt
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        tCQCKit::ESchTypes eType() const;

        tCIDLib::TVoid FormatToStr
        (
                    TString&                strToFill
        )   const;

        tCIDLib::TInt4 i4Offset() const;

        tCIDLib::TInt4 i4Offset
        (
            const   tCIDLib::TInt4          i4ToSet
        );

        tCIDLib::TVoid SetOneShot
        (
            const   tCIDLib::TEncodedTime   enctAt
        );

        tCIDLib::TVoid SetPeriodic
        (
            const   tCQCKit::ESchTypes      eType
            , const tCIDLib::TCard4         c4Period
            , const tCIDLib::TCard4         c4SubPeriod
            , const tCIDLib::TEncodedTime   enctAt
        );

        tCIDLib::TVoid SetScheduled
        (
            const   tCQCKit::ESchTypes      eType
            , const tCIDLib::TCard4         c4Day
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
            , const tCIDLib::TCard4         c4Mask
            , const tCIDLib::TEncodedTime   enctAt
        );

        tCIDLib::TVoid SetSunrise
        (
            const   tCIDLib::TFloat8&       f8Lat
            , const tCIDLib::TFloat8&       f8Long
            , const tCIDLib::TInt4          i4Offset
        );

        tCIDLib::TVoid SetSunset
        (
            const   tCIDLib::TFloat8&       f8Lat
            , const tCIDLib::TFloat8&       f8Long
            , const tCIDLib::TInt4          i4Offset
        );


        // -------------------------------------------------------------------
        //  For pre-5.0 compatibility
        // -------------------------------------------------------------------
        const TString& strId() const;

        const TString& strId
        (
            const   TString&                strToSet
        );


    protected           :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoSRCalc
        (
            const   TTime&                  tmCur
            ,       TTime&                  tmToAdjust
            , const tCIDLib::TFloat8&       f8Latitude
            , const tCIDLib::TFloat8&       f8Longitude
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLoggable
        //      Each event can be marked as to whether it should be logged
        //      when invoked or not. Most of the time you don't care and
        //      don't want the extra traffic. But for debugging purposes,
        //      sometimes it's a good thing to be able to do this.
        //
        //  m_bPaused
        //      Events can be paused so that they don't run when their
        //      scheduled times arrive.
        //
        //  m_c4Day
        //  m_c4Hour
        //  m_c4Min
        //      These may or may not be used, according to the type of
        //      schedule. The meanings are as defined by TTime. See the
        //      file comments above.
        //
        //  m_c4Mask
        //      For weekly and monthly, we need a mask of the days or months
        //      that they want to do it. Otherwise, not used.
        //
        //  m_eType
        //      The type of scheduling for this type of item.
        //
        //  m_enctAt
        //      At runtime it holds the next time for which this event is
        //      scheduled to run. For the periodic type events, it is also
        //      persisted so that we can track when the next period is up
        //      across starts/stops of the event server.
        //
        //  m_i4Offset
        //      Some event types support an offset from the calculated time,
        //      such as sunrise/sunset. This is the offset, and can be minutes,
        //      seconds, hours or whatever is appropriate for that type of
        //      event.
        //
        //  m_strId
        //      In the pre-5.0 world, each new event got a unique id to identify
        //      it in commands that operate on it. This is no longer used since
        //      they are now stored hierarchically. However, we still need to
        //      read it in since it has to be available for the conversion to the
        //      hierarchical storage (since it's the only unique id available to
        //      us for the events.) They can rename them after that.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLoggable;
        tCIDLib::TBoolean       m_bPaused;
        tCIDLib::TCard4         m_c4Day;
        tCIDLib::TCard4         m_c4Hour;
        tCIDLib::TCard4         m_c4Mask;
        tCIDLib::TCard4         m_c4Min;
        tCQCKit::ESchTypes      m_eType;
        tCIDLib::TEncodedTime   m_enctAt;
        tCIDLib::TInt4          m_i4Offset;
        TString                 m_strId;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSchEvent,TCQCStdCmdSrc)
};


// ---------------------------------------------------------------------------
//  CLASS: TKeyedCQCSchEvent
// PREFIX: csrc
//
//  A convenience for folks who need to keep a list of these events, keyed by
//  the path.
// ---------------------------------------------------------------------------
class CQCKITEXPORT TKeyedCQCSchEvent : public TCQCSchEvent
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompByTime
        (
            const   TKeyedCQCSchEvent&      csrc1
            , const TKeyedCQCSchEvent&      csrc2
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TKeyedCQCSchEvent();

        TKeyedCQCSchEvent
        (
            const   TString&                strPath
            , const TCQCSchEvent&           csrcBase
        );

        TKeyedCQCSchEvent(const TKeyedCQCSchEvent&) = default;
        TKeyedCQCSchEvent(TKeyedCQCSchEvent&&) = default;

        ~TKeyedCQCSchEvent();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TKeyedCQCSchEvent& operator=(const TKeyedCQCSchEvent&) = default;
        TKeyedCQCSchEvent& operator=(TKeyedCQCSchEvent&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4SerialNum() const;

        tCIDLib::TCard4 c4SerialNum
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        const TString& strPath() const;

        const TString& strPath
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetConfig
        (
            const   TCQCSchEvent&           csrcBase
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4SerialNum
        //      A serial number for the client code's use, typically used to make
        //      syncing with clients more efficient.
        //
        //  m_strPath
        //      The path from which this event was loaded. It could change if the
        //      user does a rename.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4SerialNum;
        TString         m_strPath;
};

#pragma CIDLIB_POPPACK

