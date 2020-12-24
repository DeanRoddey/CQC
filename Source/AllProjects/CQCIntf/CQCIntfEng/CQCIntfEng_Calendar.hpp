//
// FILE NAME: CQCIntfEng_Calendar.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/16/2011
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
//  This is the header for the CQCIntfEng_Calendar.cpp file, which implements
//  some calendar based widgets. As with most there is a base class and
//  derivatives that are used in different ways. This one displays a standard
//  sort of calendar grid.
//
//  It accepts a command to scroll forward/backwards by a month at a time. It
//  sends out an OnScroll event when this happens, with the selected month
//  and year, so that it can be reacted to.
//
//  When clicked, if they hit a day, an OnSelect is sent out with the year, month,
//  and day info. So it can be used to allow the user to select some particular
//  day.
//
//  By default it will send out OnSelect for any day clicked. But it can be
//  sent a command to tell it only to do so for today or beyond, or for today
//  or before, if that is desirable.
//
//  Currently we just have a static calendar derivative, which really doesn't
//  add anything to the base. It's just for allowing the user to browse the
//  celendar or to select days in order to drive some user created activity.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCalendarBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfCalendarBase :

    public TCQCIntfWidget
    , public MCQCCmdSrcIntf
    , public MCQCIntfFontIntf
    , public MCQCIntfImgIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TCQCIntfCalendarBase() = delete;

        ~TCQCIntfCalendarBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bPerGestOpts
        (
            const   TPoint&                 pntAt
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TBoolean       bTwoFingers
            ,       tCIDCtrls::EGestOpts&   eToSet
            ,       tCIDLib::TFloat4&       f4VScale
        )   override;

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
        )   override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const override;

        tCIDLib::TVoid PostInit() override;

        tCIDLib::TVoid ProcessFlick
        (
            const   tCIDLib::EDirs          eDirection
            , const TPoint&                 pntStartPos
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid RefreshImages
        (
                    TDataSrvClient&         dsclToUse
        )   override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCIntfEng::ECalSelTypes eSelType() const;

        tCQCIntfEng::ECalSelTypes eSelType
        (
            const   tCQCIntfEng::ECalSelTypes eToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfCalendarBase
        (
            const   TString&                strTypeName
            , const tCQCIntfEng::ECalSelTypes eInitSelType
        );

        TCQCIntfCalendarBase
        (
            const   TCQCIntfCalendarBase&   iwdgToCopy
        );

        TCQCIntfCalendarBase& operator=
        (
            const   TCQCIntfCalendarBase&   iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   override;

        tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AdjustMonth
        (
            const   TString&                strType
            , const tCIDLib::TBoolean       bCanRedraw
        );

        TBitmap bmpGatherBgn();

        tCIDLib::TCard4 c4PointToDay
        (
            const   TPoint&                 pntAt
        )   const;

        tCIDLib::TVoid CalcAreas
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        );

        tCIDLib::TVoid CalcDayArea
        (
            const   tCIDLib::TCard4         c4Day
            , const TArea&                  areaTar
            , const tCIDLib::EWeekDays      eFirstDay
            ,       TArea&                  areaToFill
        )   const;

        tCIDLib::TVoid CalcRowArea
        (
            const   tCIDLib::TCard4         c4Index
            , const TArea&                  areaTar
            ,       TArea&                  areaToFill
        )   const;

        tCIDLib::TVoid DrawCalendar
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaTar
            , const TArea&                  areaInvalid
            , const TTime&                  tmTarget
            , const tCIDLib::TBoolean       bForceAlpha
        );

        tCIDLib::TVoid SetDate
        (
            const   TTime&                  tmToSet
        );

        tCIDLib::TVoid SlideScroll
        (
            const   TTime&                  tmOld
            , const TTime&                  tmNew
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaDisplay
        //      The area where we display our contents and do scrolling. This
        //      is updated any time our area changes.
        //
        //  m_eSelType
        //      Indicates whether we send OnSelect for any day clicked, only
        //      for today or forward, or only today or before.
        //
        //  m_szBlock
        //      The size of the individual day blocks. This is updated any
        //      time our size changes.
        //
        //  m_tmCurrent
        //      The current time stamp that we are displaying for.
        // -------------------------------------------------------------------
        TArea                       m_areaDisplay;
        tCQCIntfEng::ECalSelTypes   m_eSelType;
        TSize                       m_szBlock;
        TTime                       m_tmCurrent;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfCalendarBase,TCQCIntfWidget)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticCalendar
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStaticCalendar : public TCQCIntfCalendarBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticCalendar();

        TCQCIntfStaticCalendar
        (
            const   TCQCIntfStaticCalendar& iwdgToCopy
        );

        ~TCQCIntfStaticCalendar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticCalendar& operator=
        (
            const   TCQCIntfStaticCalendar& iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStaticCalendar,TCQCIntfCalendarBase)
        DefPolyDup(TCQCIntfStaticCalendar)
};

#pragma CIDLIB_POPPACK


