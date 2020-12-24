//
// FILE NAME: CQCIntfEng_VolKnob.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/15/2003
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
//  This is the header for the CQCIntfEng_VolKnob.cpp file, which implements
//  a rotary volume knob. It supports a range of values, driven by the range
//  of the int/card field it is associated with and a current value within
//  that range, and a percentage of the shortest axis that should be used as
//  a radius of rotation for the LED.
//
//  All the volume knob provides is a small LED that moves around a center
//  point of the area of the volume knob. It is assumed that the background
//  will provide the knob image.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfVolKnobTab;


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVolKnobBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVolKnobBase

    : public TCQCIntfWidget, public MCQCIntfImgIntf, public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfVolKnobBase();


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
        tCIDLib::TBoolean bUseLimits() const;

        tCIDLib::TBoolean bUseLimits
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4Radius() const;

        tCIDLib::TCard4 c4Radius
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4Range() const;

        tCIDLib::TCard4 c4Range
        (
            const   tCIDLib::TCard4         c4ToSet
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
            , const tCIDLib::TCard4         c4Radius
            , const tCIDLib::TCard4         c4Range
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
        //  Hidden constructorse and operators
        // -------------------------------------------------------------------
        TCQCIntfVolKnobBase
        (
            const   tCQCIntfEng::ECapFlags  eCapFlags
            , const TString&                strTypeName
        );

        TCQCIntfVolKnobBase
        (
            const   TCQCIntfVolKnobBase&    iwdgToCopy
        );

        TCQCIntfVolKnobBase& operator=
        (
            const   TCQCIntfVolKnobBase&    iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDCtrls::EWndAreas eHitTest
        (
            const   TPoint&                 pntTest
        )   const override;

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
        TArea areaCalcLEDPos
        (
            const   tCIDLib::TFloat8        f8Value
            , const tCIDLib::TFloat8        f8Min
            , const tCIDLib::TFloat8        f8Max
        );

        tCIDLib::TVoid DrawLED
        (
            const   tCIDLib::TFloat8        f8Value
        );

        tCIDLib::TFloat8 f8TrackDrag
        (
            const   TPoint&                 pntStart
            , const TPoint&                 pntAt
        );

        tCIDLib::TVoid MoveLED
        (
            const   tCIDLib::TFloat8        f8NewVal
            , const tCIDLib::TFloat8        f8OldVal
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bHorizontal
        //      This is used during gesture handling. We remember if it's
        //      a horizontal or vertical gesture, for later user.
        //
        //  m_bUseLimits
        //      Indicates whether we use per-user limits or not.
        //
        //  m_bValError
        //      If the value we are given is outside of the legal range,
        //      we have to go into error state, so we don't display the
        //      LED.
        //
        //  m_c4Radius
        //      The radius we use for rotating the LED around.
        //
        //  m_c4Range
        //      Controls how many degrees of travel around the knob are
        //      supported. It is streamed, and can be from 280 down to
        //      90.
        //
        //  m_cptrPosImg
        //      The image we use for our position indicator.
        //
        //  m_eCurRole
        //      The user role of the user whose account we are running under.
        //      We set it during our init and leave it alone, since it cannot
        //      change while we exist. So it's not persisted.
        //
        //  m_f8PerUnit
        //      At runtime, we calculate now many degrees of rotation are
        //      represented by a unit of range adjustment. It's not streamed
        //      but calculated as required when the min/max values change.
        //
        //  m_fcolLimits
        //      If per-user limits are engaged, this contains the list of
        //      maximum values available to each user. We only allow fields
        //      with ranges less than 512 to be used, so we store Card4
        //      values and put the min value in the low half and the max
        //      value in the high half.
        //
        //  m_f8Max
        //  m_f8Min
        //      The min/max values that this field can have. We get this from
        //      the field definition limits value. We will reject any field
        //      with a range greater than a certain size.
        //
        //      These are not streamed, they are gotten from the field limits
        //      info upon load, to insure that we stay in sync with the field.
        //
        //  m_f8SaveValue
        //      We save the original value when a drag gesture starts, so that
        //      we know where we are moving from and can go back there if
        //      an error occurs.
        //
        //  m_f8Value
        //      The last value we got updated with. We need to store this so
        //      that we can redraw when required. It's not streamed, it is just
        //      live data.
        //
        //  m_strPosImage
        //      The image repo path to the position indicator image that we use to
        //      show the current position.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bHorizontal;
        tCIDLib::TBoolean       m_bUseLimits;
        tCIDLib::TBoolean       m_bValError;
        tCIDLib::TCard4         m_c4Radius;
        tCIDLib::TCard4         m_c4Range;
        TIntfImgCachePtr        m_cptrPosImg;
        tCQCKit::EUserRoles     m_eCurRole;
        tCIDLib::TFloat8        m_f8PerUnit;
        tCQCIntfEng::TUserLims  m_fcolLimits;
        tCIDLib::TFloat8        m_f8Max;
        tCIDLib::TFloat8        m_f8Min;
        tCIDLib::TFloat8        m_f8SaveValue;
        tCIDLib::TFloat8        m_f8Value;
        TString                 m_strPosImage;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfVolKnobBase,TCQCIntfWidget)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldVolKnob
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFldVolKnob

    : public TCQCIntfVolKnobBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFldVolKnob();

        TCQCIntfFldVolKnob
        (
            const   TCQCIntfFldVolKnob&     iwdgToCopy
        );

        ~TCQCIntfFldVolKnob();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFldVolKnob& operator=
        (
            const   TCQCIntfFldVolKnob&     iwdgToAssign
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
        RTTIDefs(TCQCIntfFldVolKnob,TCQCIntfVolKnobBase)
        DefPolyDup(TCQCIntfFldVolKnob)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticVolKnob
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStaticVolKnob

    : public TCQCIntfVolKnobBase, public MCQCIntfStaticIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticVolKnob();

        TCQCIntfStaticVolKnob
        (
            const   TCQCIntfStaticVolKnob&  iwdgToCopy
        );

        ~TCQCIntfStaticVolKnob();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticVolKnob& operator=
        (
            const   TCQCIntfStaticVolKnob&  iwdgToAssign
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
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStaticVolKnob,TCQCIntfVolKnobBase)
        DefPolyDup(TCQCIntfStaticVolKnob)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarVolKnob
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVarVolKnob

    : public TCQCIntfVolKnobBase, public MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarVolKnob();

        TCQCIntfVarVolKnob
        (
            const   TCQCIntfVarVolKnob&     iwdgToCopy
        );

        ~TCQCIntfVarVolKnob();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarVolKnob& operator=
        (
            const   TCQCIntfVarVolKnob&     iwdgToAssign
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
        RTTIDefs(TCQCIntfVarVolKnob,TCQCIntfVolKnobBase)
        DefPolyDup(TCQCIntfVarVolKnob)
};

#pragma CIDLIB_POPPACK

