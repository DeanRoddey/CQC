//
// FILE NAME: CQCIntfEng_AnimatedImg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/16/2005
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
//  This is the header for the CQCIntfEng_AnimatedImg.cpp file, which
//  implements a widget that displays one of a set of images in a timed
//  sequence, to create a simple animation scheme. It is used for things
//  like indicating that some action is occuring or not, such a DVD playing
//  or a fan spinning and so forth.
//
//  We have a base class and three derivatives. The field and variable ones
//  use a field or variable to drive whether the animation is active or not.
//  They evaluate their value via the boolean interface and enable or disable
//  the animatino by setting an active/inactive status on the base class.
//  The static one just sets it active on init and leaves it that way so it
//  always animates.
//
// CAVEATS/GOTCHAS:
//
//  1)  The static animated image doesn't use either the static tab or
//      the boolean tab in the configuration dialog. It doesn't need any
//      of that configured. It just directly takes a user set true/false
//      value via action command. So it's just hard coded to the static
//      interface as boolean, but the boolean interface is never actually
//      used to evaluate the value.
//
//      But we still mix in the static interface and stream it, just in case
//      we should want to use it later.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfAnimatedImgBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfAnimatedImgBase

    : public TCQCIntfWidget
    , public MCQCIntfBooleanIntf
    , public MCQCIntfImgListIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TCQCIntfAnimatedImgBase() = delete;

        ~TCQCIntfAnimatedImgBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

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

        TSize szDefaultSize() const override;

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


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSkipFirst() const;

        tCIDLib::TBoolean bSkipFirst
        (
            const   tCIDLib::TBoolean       bToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfAnimatedImgBase
        (
            const   tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
        );

        TCQCIntfAnimatedImgBase
        (
            const   TCQCIntfAnimatedImgBase& iwdgToCopy
        );

        TCQCIntfAnimatedImgBase& operator=
        (
            const   TCQCIntfAnimatedImgBase& iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const tCIDLib::TBoolean         bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
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
        tCQCIntfEng::EBoolStates eCurState() const;

        tCIDLib::TVoid SetState
        (
            const   tCQCIntfEng::EBoolStates eToSet
            , const tCIDLib::TBoolean       bNoRedraw
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSkipFirst
        //      An option to skip the first image when animating. This allows
        //      the first image to be the 'normal' image and the others to
        //      animate when in a non-normal situation.
        //
        //  m_c4CurIndex
        //      The current index into the list of images, so that we can
        //      round robin through them. This is not persisted. We run up
        //      till we find an undefined image, then we go back to zero and
        //      start again.
        //
        //  m_enctLast
        //      We only want to update so fast, so we remember the last time
        //      we changed the image.
        //
        //  m_eCurState
        //      The derived classes tell us what state we are in, which are
        //      True (do the animation), False (don't animiate) or in error
        //      state.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bSkipFirst;
        tCIDLib::TCard4             m_c4CurIndex;
        tCIDLib::TEncodedTime       m_enctLast;
        tCQCIntfEng::EBoolStates    m_eCurState;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfAnimatedImgBase,TCQCIntfWidget)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldAnimatedImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFldAnimatedImg

    : public TCQCIntfAnimatedImgBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFldAnimatedImg();

        TCQCIntfFldAnimatedImg
        (
            const   TCQCIntfFldAnimatedImg& iwdgToCopy
        );

        ~TCQCIntfFldAnimatedImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFldAnimatedImg& operator=
        (
            const   TCQCIntfFldAnimatedImg& iwdgToAssign
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

        tCIDLib::TVoid QueryBoolAttrLims
        (
                    tCQCKit::EFldTypes&     eType
            ,       TString&                strLimits
        )   const override;

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
        RTTIDefs(TCQCIntfFldAnimatedImg,TCQCIntfAnimatedImgBase)
        DefPolyDup(TCQCIntfFldAnimatedImg)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticAnimatedImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStaticAnimatedImg

    : public TCQCIntfAnimatedImgBase, public MCQCIntfStaticIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticAnimatedImg();

        TCQCIntfStaticAnimatedImg
        (
            const   TCQCIntfStaticAnimatedImg& iwdgToCopy
        );

        ~TCQCIntfStaticAnimatedImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticAnimatedImg& operator=
        (
            const   TCQCIntfStaticAnimatedImg& iwdgToAssign
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
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStaticAnimatedImg,TCQCIntfAnimatedImgBase)
        DefPolyDup(TCQCIntfStaticAnimatedImg)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarAnimatedImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVarAnimatedImg

    : public TCQCIntfAnimatedImgBase, public MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarAnimatedImg();

        TCQCIntfVarAnimatedImg
        (
            const   TCQCIntfVarAnimatedImg& iwdgToCopy
        );

        ~TCQCIntfVarAnimatedImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarAnimatedImg& operator=
        (
            const   TCQCIntfVarAnimatedImg& iwdgToAssign
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

        tCIDLib::TVoid QueryBoolAttrLims
        (
                    tCQCKit::EFldTypes&     eType
            ,       TString&                strLimits
        )   const override;

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
        RTTIDefs(TCQCIntfVarAnimatedImg,TCQCIntfAnimatedImgBase)
        DefPolyDup(TCQCIntfVarAnimatedImg)
};


#pragma CIDLIB_POPPACK


