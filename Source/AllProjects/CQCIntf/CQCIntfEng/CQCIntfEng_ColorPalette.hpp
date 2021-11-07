//
// FILE NAME: CQCIntfEng_ClrPalette.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/29/2014
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
//  This is the header for the CQCIntfEng_ClrPalette.cpp file, which implements a
//  widget that displays an interactive color palette. it is primarily intended to
//  support something like the Hue LED lighting system, but it would serve as a
//  color selection mechanism for other things.
//
//  We dipslay an HSV type palette with Hue horizontal and saturation increasing
//  downwards.
//
//  We maintain a current color driven by the user touching/dragging around on the
//  palette. We default to a 100% value, but we accept commands to allow the
//  outside world to adjust the value component as well, which we use to update
//  the current color.
//
//  Any time the current color changes, we send out an event so that they can
//  update some current color display, or use the color value. But typically they
//  will only really use it after it's set and will just query the color value
//  from us and write it to a driver field or some such thing. The changes during
//  a drag would be too fast for that kind of thing.
//
//  Because of the overhead of setting up the palette, we use a fixed size. It
//  is 360 pixels wide (one pixel per degree), and 256 pixels high (one pixel per
//  256 levels of saturation we support.) We create the bitmap once and keep it
//  around so that we can reuse it any time a palette is created after that.
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
//   CLASS: TCQCIntfClrPalette
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfClrPalette :

    public TCQCIntfWidget, public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfClrPalette();

        TCQCIntfClrPalette
        (
            const   TCQCIntfClrPalette&     iwdgToCopy
        );

        ~TCQCIntfClrPalette();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfClrPalette& operator=
        (
            const   TCQCIntfClrPalette&     iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const final;

        tCIDLib::TBoolean bCanSizeTo() const final;

        tCIDLib::TBoolean bPerGestOpts
        (
            const   TPoint&                 pntAt
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TBoolean       bTwoFingers
            ,       tCIDCtrls::EGestOpts&   eToSet
            ,       tCIDLib::TFloat4&       f4VScale
        )   final;

        tCIDLib::TBoolean bProcessGestEv
        (
            const   tCIDCtrls::EGestEvs     eEv
            , const TPoint&                 pntStart
            , const TPoint&                 pntAt
            , const TPoint&                 pntDelta
            , const tCIDLib::TBoolean       bTwoFinger
        )   final;

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
        )   final;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   final;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   final;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   final;

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const final;

        tCIDLib::TVoid PostInit() final;

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   final;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const final;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const final;

        TSize szDefaultSize() const final;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const final;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   final;

        tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
        )   final;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   final;

    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPointToColor
        (
            const   TPoint&                 pntSrc
        );

        tCIDLib::TVoid DoSizeCalcs
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bmpPal
        //      The palette bitmap we create upon initialization and draw.
        //
        //  m_hsvCur
        //      Our current color, i.e. the last one the user set on us, or
        //      the initial default black. This is what we return if our current
        //      color is queried. It's also updated during gesture processing as
        //      we track the user's movement, or when we are sent a command to
        //      set our value level.
        //
        //  m_szPal
        //      The size of the palette. It's our size minus any border. We will
        //      only allow it to get so small then we'll stop and let it clip.
        // -------------------------------------------------------------------
        TBitmap             m_bmpPal;
        THSVClr             m_hsvCur;
        TSize               m_szPal;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfClrPalette,TCQCIntfWidget)
        DefPolyDup(TCQCIntfClrPalette)
};

#pragma CIDLIB_POPPACK



