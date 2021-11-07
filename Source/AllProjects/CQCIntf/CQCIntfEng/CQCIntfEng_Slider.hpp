//
// FILE NAME: CQCIntfEng_Slider.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/07/2006
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
//  This is the header for the CQCIntfEng_Slider.cpp file. This file
//  implements an interface widget that places a 'thumb' image somewhere along
//  its horizontal or vertical extent based on a current value. It lets
//  the user drag the thumb up and down (sending out OnDrag events as it goes)
//  and lets them drop it when they are done, when it sends out an OnSet
//  event (if they drop within the edges of the slider in the direction of
//  travel.)
//
//  There is a base class that provides all the basic smarts, and two derived
//  classes. One of them associates the slider with a numeric, range based
//  field. The other just uses the current value inside the base class as the
//  value.
//
//  For the field based one, if the field is writeable, then the user can slide
//  the thumb up or down and when it is dropped it will set the field value.
//
//  We support two images for the thumb slider, one to normally display and
//  one to show then the thumb is pressed during sliding.
//
//  We support int, card, and float fields, so we just use floating point
//  to store our values and convert where necessary (just a couple places.)
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
//   CLASS: TCQCIntfSliderBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfSliderBase

    : public TCQCIntfWidget, public MCQCIntfImgListIntf, public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfSliderBase();


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

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
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
        tCIDLib::TBoolean bUseLimits() const;

        tCIDLib::TBoolean bUseLimits
        (
            const   tCIDLib::TBoolean       bToSet
        );

        const tCQCIntfEng::TUserLims& fcolLimits() const;

        tCIDLib::TFloat8 f8MaxVal() const;

        tCIDLib::TFloat8 f8MinVal() const;

        tCIDLib::TVoid QueryLimitsFor
        (
            const   tCQCKit::EUserRoles     eRole
            ,       tCIDLib::TInt4&         i4Min
            ,       tCIDLib::TInt4&         i4Max
        );

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TBoolean       bUseLimits
            , const tCQCIntfEng::TUserLims& fcolLimits
        );

        tCIDLib::TVoid SetLimitsFor
        (
            const   tCQCKit::EUserRoles     eRole
            , const tCIDLib::TInt4          i4Min
            , const tCIDLib::TInt4          i4Max
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfSliderBase
        (
            const   tCQCIntfEng::ECapFlags  eCapFlags
            , const TString&                strTypeName
        );

        TCQCIntfSliderBase
        (
            const   TCQCIntfSliderBase&     iwdgToCopy
        );


        TCQCIntfSliderBase& operator=
        (
            const   TCQCIntfSliderBase&     iwdgToAssign
        );


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
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TFloat8        f8ToSet
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bValError
        );

        tCIDLib::TBoolean bValueOutOfRange
        (
            const   tCIDLib::TFloat8        f8Check
        )   const;

        tCIDLib::TVoid SetValueAttrs
        (
            const   tCIDLib::TBoolean       bWriteable
            , const tCIDLib::TFloat8        f8Min
            , const tCIDLib::TFloat8        f8Max
            , const tCIDLib::TBoolean       bInitToMin
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TFloat8 f8ValFromPos
        (
            const   TPoint&                 pntAt
            , const tCIDLib::TBoolean       bDrag
        );

        MCQCIntfImgListIntf::TImgInfo* pimgiCalcThumbArea
        (
            const   tCIDLib::TFloat8        f8Val
            ,       TArea&                  areaImgSrc
            ,       TArea&                  areaToFill
            , const tCIDLib::TBoolean       bDragging
        );

        MCQCIntfImgListIntf::TImgInfo* pimgiForState
        (
            const   tCIDLib::TBoolean       bDragging
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bUseLimits
        //      Indicates whether we use per-user limits or not.
        //
        //  m_bValError
        //      If the value we are given is outside of the legal range, we have to go
        //      into error state, so we don't display the LED.
        //
        //  m_bWriteable
        //      When we get a new field set on us, we remember whether it's writeable. If
        //      not, we don't allow the user to click and drag to set a new value.
        //
        //  m_eCurRole
        //      The user role of the user whose account we are running under. We set it
        //      during our init and leave it alone, since it cannot change while we exist.
        //      So it's not persisted.
        //
        //  m_f8Max
        //  m_f8Min
        //      The min/max values of the slider range, which we use to place the thumb
        //      at the right offset. The derived class provides this info based on config
        //      or selected field.
        //
        //  m_f8Value
        //      The current value. During design time we set this to the min value of the
        //      range. At viewing time it's set from the values provided by the derived
        //      class.
        //
        //  m_fcolLimits
        //      If per-user limits are engaged, this contains the list of min/max values
        //      available to each user. We only allow fields with ranges less than 1024 to
        //      be used, so we store Card4 values and put the min value in the low half and
        //      the max value in the high half. The values stored are actually signed.
        //
        //  m_i4Offset
        //      Only used at runtime. Any time the value changes, we precalc the offset
        //      from the left/bottom for the thumb center. This is used during drawing
        //      to position the thumb.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bUseLimits;
        tCIDLib::TBoolean       m_bValError;
        tCIDLib::TBoolean       m_bWriteable;
        tCQCKit::EUserRoles     m_eCurRole;
        tCIDLib::TFloat8        m_f8Max;
        tCIDLib::TFloat8        m_f8Min;
        tCIDLib::TFloat8        m_f8Value;
        tCQCIntfEng::TUserLims  m_fcolLimits;
        tCIDLib::TInt4          m_i4Offset;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfSliderBase,TCQCIntfWidget)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFieldSlider
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFieldSlider

    : public TCQCIntfSliderBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFieldSlider();

        TCQCIntfFieldSlider
        (
            const   TCQCIntfFieldSlider&    iwdgToCopy
        );

        ~TCQCIntfFieldSlider();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFieldSlider& operator=
        (
            const   TCQCIntfFieldSlider&    iwdgToAssign
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
        RTTIDefs(TCQCIntfFieldSlider,TCQCIntfSliderBase)
        DefPolyDup(TCQCIntfFieldSlider)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticSlider
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStaticSlider

    : public TCQCIntfSliderBase, public MCQCIntfStaticIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticSlider();

        TCQCIntfStaticSlider
        (
            const   TCQCIntfStaticSlider&        iwdgToCopy
        );

        ~TCQCIntfStaticSlider();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticSlider& operator=
        (
            const   TCQCIntfStaticSlider&        iwdgToAssign
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

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid SetStaticInfo
        (
            const   tCIDLib::TFloat8        f8Min
            , const tCIDLib::TFloat8        f8Max
            , const tCQCKit::EFldTypes      eType
        )   override;

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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckRangeVals
        (
            const   TString&                strMin
            , const TString&                strMax
        );


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStaticSlider,TCQCIntfSliderBase)
        DefPolyDup(TCQCIntfStaticSlider)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarSlider
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVarSlider

    : public TCQCIntfSliderBase, public MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarSlider();

        TCQCIntfVarSlider
        (
            const   TCQCIntfVarSlider&      iwdgToCopy
        );

        ~TCQCIntfVarSlider();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarSlider& operator=
        (
            const   TCQCIntfVarSlider&      iwdgToAssign
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

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

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
        tCIDLib::TBoolean bCanAcceptVar
        (
            const   TCQCActVar&             varToTest
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

        tCIDLib::TVoid VarInError
        (
            const   tCIDLib::TBoolean       bNoRedraw
        )   override;

        tCIDLib::TVoid VarValChanged
        (
            const   TCQCActVar&             varNew
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfVarSlider,TCQCIntfSliderBase)
        DefPolyDup(TCQCIntfVarSlider)
};


#pragma CIDLIB_POPPACK


