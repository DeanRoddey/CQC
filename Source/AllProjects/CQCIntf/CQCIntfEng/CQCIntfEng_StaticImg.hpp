//
// FILE NAME: CQCIntfEng_StaticImg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2003
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
//  This is the header for the CQCIntfEng_StaticImg.cpp file, which implements a
//  widget that just displays an image statically, mostly for ornamentation of the
//  interface.
//
//  The bulk of the functionality is provided by the image interface mixin which
//  manages and displays the image, so we have no members at this level.
//
//  This one has a somewhat special feature in that it can optionally be transparent
//  to hit testing. The reason for this is that these are often used to create glare
//  effects which overlay some input oriented widget. If this guy was opaque to hits,
//  then only the part of the underlying widget that wasn't covered would react to
//  hits.
//
//  The default is to be opaque, since that's the most common scenario.
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
//   CLASS: TCQCIntfStaticImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStaticImg

    : public TCQCIntfWidget, public MCQCIntfImgIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticImg();

        TCQCIntfStaticImg
        (
            const   TCQCIntfStaticImg&      iwdgToCopy
        );

        ~TCQCIntfStaticImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticImg& operator=
        (
            const   TCQCIntfStaticImg&      iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

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

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
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

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
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

    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStaticImg,TCQCIntfWidget)
        DefPolyDup(TCQCIntfStaticImg)
};

#pragma CIDLIB_POPPACK


