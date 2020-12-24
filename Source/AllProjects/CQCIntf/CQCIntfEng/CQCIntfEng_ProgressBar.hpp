//
// FILE NAME: CQCIntfEng_ProgressBar.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/04/2008
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
//  This is the header for the CQCIntfEng_ProgressBar.cpp file, which
//  implements a widget that is associated with a numeric range (either via a
//  field or an internal value.) It will show the current value graphically as
//  a percentage of the possible range, using a partial color fill from left
//  to right, bottom to top, right to left, or top to bottom.
//
//  The user provides us with two images, one of which is the background and
//  the other tells us where the bar fill will go. It must be an alpha based
//  image so we make a copy of it, which gets us the correct alpha mask, then we
//  can draw into it with color fills and blit that to the screen so that the
//  bar shows up whereever they want. This lets them use hilights and so
//  forth in the underlying image and they'll be kept since our bar will be
//  opaque/translucent wherever the fill image is that they provide us.
//
//  We use F1/F2 for the fill colors, so we do a gradient fill. We have to
//  interpolate the position color based on the percentage of the fill. We
//  use F3 color to fill any portion of the fill image that isn't filled in
//  by the percentage display fill.
//
//  There is a base class that provides the bulk of the functionality, plus
//  two derivatives for the field and static variations. There's an attributes
//  tab class for the static one, to let the user define the range.
//
//  The static version only has to tell the parent class what the desired
//  min/max values are (as configured by the user.) After that, the internal
//  value of the slider is the actual value. The field based one updates the
//  value of the slider if the field value changes, and the min/max values
//  come from the limits definition of the field it is associated with.
//
//  We have to support cardinal, integral and floating point fields. So we
//  use floating point values to store the actual value and min/max. We have
//  to just deal with conversion as necessary.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// Forward references
class TCQCIntfStaticPBarCfg;


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfProgBarBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfProgBarBase

    : public TCQCIntfWidget, public MCQCIntfImgListIntf, public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static const TString  strBgnKey;
        static const TString  strFillKey;
        static const TString  strOverlayKey;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfProgBarBase();


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

        tCIDLib::TBoolean bProcessGestEv
        (
            const   tCIDCtrls::EGestEvs     eEv
            , const TPoint&                 pntStart
            , const TPoint&                 pntAt
            , const TPoint&                 pntDelta
            , const tCIDLib::TBoolean       bTwoFinger
        )   override;

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
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

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   override;

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
        tCIDLib::TBoolean bFillUnused() const;

        tCIDLib::TBoolean bFillUnused
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsWriteable() const;

        tCIDLib::TBoolean bIsWriteable
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::EDirs eDir() const;

        tCIDLib::EDirs eDir
        (
            const   tCIDLib::EDirs          eToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfProgBarBase
        (
            const   tCQCIntfEng::ECapFlags  eCapFlags
            , const TString&                strTypeName
        );

        TCQCIntfProgBarBase
        (
            const   TCQCIntfProgBarBase&    iwdgToCopy
        );

        TCQCIntfProgBarBase& operator=
        (
            const   TCQCIntfProgBarBase&    iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid FormatValue
        (
            const   tCIDLib::TFloat8        f8ToFmt
            ,       TString&                strToFill
        )   const = 0;

        virtual tCIDLib::TVoid UserSetValue
        (
            const   tCIDLib::TFloat8        f8NewValue
        ) = 0;


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

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValueOutOfRange
        (
            const   tCIDLib::TFloat8        f8Check
        )   const;

        tCIDLib::TFloat8 f8PBValue() const;

        tCIDLib::TVoid ForceNextRedraw();

        tCIDLib::TVoid SetFillMask
        (
            const   tCIDLib::TFloat8        f8TarValue
        );

        tCIDLib::TVoid SetRangeLimits
        (
            const   tCIDLib::TFloat8        f8Min
            , const tCIDLib::TFloat8        f8Max
        );

        tCIDLib::TVoid SetValue
        (
            const   tCIDLib::TFloat8        f8New
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bValError
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TFloat8 f8ValFromPos
        (
            const   TPoint&                 pntAt
        );

        tCIDLib::TCard4 c4CalcPercent
        (
            const   tCIDLib::TFloat8        f8TarValue
        )   const;

        tCIDLib::TVoid CreateFillMask();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bFillUnused
        //      Normally we fill the unused part with a fill color. But we can also
        //      leave that part transparent if desired.
        //
        //  m_bValError
        //      The derived class tells us what our value is and also tells us if we
        //      have a good value or not. If not, then we don't show the bar. We won't
        //      allow tracking if this is set either.
        //
        //  m_bWriteable
        //      This is a configurable option, but overwrideable by the derived class.
        //      So the user configures whether he will allow the user to update the
        //      value interactively. But, the derived class may know the value cannot
        //      be updated and will override this at runtime if so to disable it.
        //
        //  m_c4LastPercent
        //      Just because the source value changes, doesn't mean that the bar
        //      percentage will change. So we store the last integral percentage we
        //      saw and only update if we get a different percentage.
        //
        //  m_eDir
        //      The direction that the bar extends from, which controls how we do
        //      the fill. This is the only persistent attribute.
        //
        //  m_f8Max
        //  m_f8Min
        //      The min/max values of the field's range, which we use to calculate
        //      the position of the bar. The derived class provides these values.
        //
        //  m_f8SaveValue
        //      When we start an event, we save the original value, if we can't
        //      complete the operation, we go back to this value. Otherwise this is
        //      not used.
        //
        //  m_f8Value
        //      The current value. During design time we set this to a value mid-
        //      range, so that some of the bar will show up. At viewing time it's set
        //      from the derived class (field, static value, variable. etc...) It's
        //      also updated when the user is changing the value.
        //
        //  m_bmpFill
        //      Any time the percentage changes, we set m_bmpFill to hold the alpha
        //      bar image that we are going to blit. This way, we don't have to do it
        //      every time we redraw.
        //
        //      But, if we are in remote viewer mode, we just call a graphics device
        //      call to do it all every time, since we have to forward that to the
        //      RIVA client for the actual drawing.
        //
        //  m_pixaMask
        //      Unfortunately, Windows isn't very bright about alpha channels, so when
        //      we blit into the image it'll waste the alpha channel, even if we
        //      initially set our fill image from the original. So, we also have to
        //      grab the alpha mask out and store that so that we can re-apply it
        //      again after we do the fills.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bFillUnused;
        tCIDLib::TBoolean   m_bValError;
        tCIDLib::TBoolean   m_bWriteable;
        tCIDLib::TCard4     m_c4LastPercent;
        tCIDLib::EDirs      m_eDir;
        tCIDLib::TFloat8    m_f8Max;
        tCIDLib::TFloat8    m_f8Min;
        tCIDLib::TFloat8    m_f8SaveValue;
        tCIDLib::TFloat8    m_f8Value;
        TBitmap             m_bmpFill;
        TPixelArray         m_pixaMask;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfProgBarBase,TCQCIntfWidget)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFieldProgBar
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFieldProgBar

    : public TCQCIntfProgBarBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFieldProgBar();

        TCQCIntfFieldProgBar
        (
            const   TCQCIntfFieldProgBar&   iwdgToCopy
        );

        ~TCQCIntfFieldProgBar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFieldProgBar& operator=
        (
            const   TCQCIntfFieldProgBar&   iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             fldTest
            , const TString&                strMake
            , const TString&                strModel
            , const tCIDLib::TBoolean       bStore
        )   override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

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

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
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
        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;

        tCIDLib::TVoid FormatValue
        (
            const   tCIDLib::TFloat8        f8ToFmt
            ,       TString&                strToFill
        )   const override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid UserSetValue
        (
            const   tCIDLib::TFloat8        f8NewValue
        )   override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfFieldProgBar,TCQCIntfProgBarBase)
        DefPolyDup(TCQCIntfFieldProgBar)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticProgBar
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStaticProgBar

    : public TCQCIntfProgBarBase, public MCQCIntfStaticIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticProgBar();

        TCQCIntfStaticProgBar
        (
            const   TCQCIntfStaticProgBar&  iwdgToCopy
        );

        ~TCQCIntfStaticProgBar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticProgBar& operator=
        (
            const   TCQCIntfStaticProgBar&  iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

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

        tCIDLib::TVoid PostInit() override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

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


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatValue
        (
            const   tCIDLib::TFloat8        f8ToFmt
            ,       TString&                strToFill
        )   const override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid UserSetValue
        (
            const   tCIDLib::TFloat8        f8NewValue
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStaticProgBar,TCQCIntfProgBarBase)
        DefPolyDup(TCQCIntfStaticProgBar)
};


#pragma CIDLIB_POPPACK


