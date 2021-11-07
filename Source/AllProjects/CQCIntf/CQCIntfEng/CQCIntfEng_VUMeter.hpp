//
// FILE NAME: CQCIntfEng_VUMeter.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/14/2002
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
//  This is the header for the CQCIntfEng_VUMeter.cpp file, which implements
//  a digital VU meter. It displays a current value as a percentage within
//  a range, using a segmented LED type display.
//
//  The range and value can come from a field, a variable, or a user set
//  value, so we have a base class that does the bulk of the work, plus
//  derivatives that provide the actual value.
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
//   CLASS: TCQCIntfVUMeterBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVUMeterBase

    : public TCQCIntfWidget, public MCQCIntfImgListIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfVUMeterBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
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
        tCIDLib::TBoolean bHorizontal() const;

        tCIDLib::TBoolean bLightOnly() const;

        tCIDLib::TBoolean bLightOnly
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4HighPer() const;

        tCIDLib::TCard4 c4HighPer
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4MidPer() const;

        tCIDLib::TCard4 c4MidPer
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4Spacing() const;

        tCIDLib::TCard4 c4Spacing
        (
            const   tCIDLib::TCard4         c4ToSet
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
        TCQCIntfVUMeterBase
        (
            const   tCQCIntfEng::ECapFlags  eCapFlags
            , const TString&                strTypeName
        );

        TCQCIntfVUMeterBase
        (
            const   TCQCIntfVUMeterBase&    iwdgToCopy
        );

        TCQCIntfVUMeterBase& operator=
        (
            const   TCQCIntfVUMeterBase&    iwdgToAssign
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


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TFloat8        f8Value
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bValError
        );

        tCIDLib::TVoid SetValueAttrs
        (
            const   tCIDLib::TFloat8        f8Min
            , const tCIDLib::TFloat8        f8Max
            , const tCIDLib::TBoolean       bInitToMin
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddImageKeys();

        tCIDLib::TVoid CalcAreas
        (
            const   TArea&                  areaNew
        );

        tCIDLib::TVoid CalcIndices
        (
            const   tCIDLib::TFloat8        f8Val
            ,       tCIDLib::TCard4&        c4HighInd
            ,       tCIDLib::TCard4&        c4MidInd
            ,       tCIDLib::TCard4&        c4TopInd
        );

        tCIDLib::TVoid DrawLEDs
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaLEDs
        //      This is the actual area where the LEDs will be displayed. We
        //      round this area to exactly fit the number of LEDs we can
        //      display at our current size, and center it in our full area.
        //      This makes it easy to place them. This is calculated at run
        //      time, not persisted.
        //
        //  m_bLightOnly
        //      We have the option of only displaying the lit up images, and not
        //      showing the dark ones. It defaults to showing both.
        //
        //  m_bValError
        //      The derived class tells us the current value, but he might not
        //      be able to get it, so he tells us if the value is in error or
        //      not. If so, we display differently.
        //
        //  m_c4LEDCount
        //      The count of LEDs that fit into our area. This is the number
        //      of chunks that we have to divide the range into. This is
        //      calculated at runtime, not persisted, but it is implied by
        //      the area of the widget itself, so we know that when we stream
        //      it back it, this will end up the same as last time we streamed
        //      it out.
        //
        //  m_c4HighPer
        //  m_c4MidPer
        //      The percents of the range that the mid and high LEDs start at.
        //      So it's low up to the mid percent, then mid up to the high percent,
        //      and then high from there up. We have to round of course since the
        //      LED size limits the chunkiness of the range.
        //
        //  m_c4Spacing
        //      The spacing between the LEDs.
        //
        //  m_eDir
        //      Indicates the direction of the VU meter (i.e. left to right,
        //      bottom up, top down, or right to left.) It used to be only
        //      horizontal or vertical, but we updated it to support all four
        //      directions.
        //
        //  m_f8Max
        //  m_f8Min
        //      The min/max values of the range. We get this from the derived
        //      class, so it's not persisted.
        //
        //  m_f8Value
        //      The last value we got updated with. We need to store this so
        //      that we can redraw when required. This drives number of LEDs
        //      to display. It's not streamed, it is just live data.
        // -------------------------------------------------------------------
        TArea               m_areaLEDs;
        tCIDLib::TBoolean   m_bLightOnly;
        tCIDLib::TBoolean   m_bValError;
        tCIDLib::TCard4     m_c4LEDCount;
        tCIDLib::TCard4     m_c4HighPer;
        tCIDLib::TCard4     m_c4MidPer;
        tCIDLib::TCard4     m_c4Spacing;
        tCIDLib::EDirs      m_eDir;
        tCIDLib::TFloat8    m_f8Max;
        tCIDLib::TFloat8    m_f8Min;
        tCIDLib::TFloat8    m_f8Value;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfVUMeterBase,TCQCIntfWidget)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldVUMeter
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFldVUMeter

    : public TCQCIntfVUMeterBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFldVUMeter();

        TCQCIntfFldVUMeter
        (
            const   TCQCIntfFldVUMeter&     iwdgToCopy
        );

        ~TCQCIntfFldVUMeter();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFldVUMeter& operator=
        (
            const   TCQCIntfFldVUMeter&     iwdgToAssign
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

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

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
        RTTIDefs(TCQCIntfFldVUMeter,TCQCIntfVUMeterBase)
        DefPolyDup(TCQCIntfFldVUMeter)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarVUMeter
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVarVUMeter

    : public TCQCIntfVUMeterBase, public MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarVUMeter();

        TCQCIntfVarVUMeter
        (
            const   TCQCIntfVarVUMeter&     iwdgToCopy
        );

        ~TCQCIntfVarVUMeter();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarVUMeter& operator=
        (
            const   TCQCIntfVarVUMeter&     iwdgToAssign
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

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

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
        RTTIDefs(TCQCIntfVarVUMeter,TCQCIntfVUMeterBase)
        DefPolyDup(TCQCIntfVarVUMeter)
};


#pragma CIDLIB_POPPACK


