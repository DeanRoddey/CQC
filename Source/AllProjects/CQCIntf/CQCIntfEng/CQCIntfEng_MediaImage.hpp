//
// FILE NAME: CQCIntfEng_MediaImage.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/25/2005
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
//  This is the header for the CQCIntfEng_MediaImg.cpp file, which
//  implements widgets that display media images.
//
//  There are two scenarios. One is where we weant to see the art associated
//  with the item currently playing in a media renderer driver. The other is
//  where we want to see the art for a title or collection from a repository,
//  for preview purposes.
//
//  The former is associated with a playlist item key field in a media
//  renderer. Any time it sees the field change, it will use the info that
//  field to go to the indicated repo and grab the cover art.
//
//  The later is not connection to a field. It just allows a cookie to be
//  set on it via an action, and goes to the repository it is configured to
//  interact with and gets the art for that cookie and displays it. That repo
//  can be changed on the fly at runtime.
//
//  We derive from the special image base class, which just allows us to
//  use a standard attribute tab where the user can adjust some display
//  attributes of our image.
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
//   CLASS: TCQCIntfMediaImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMediaImg

    : public TCQCIntfSpecialImg, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMediaImg();

        TCQCIntfMediaImg
        (
            const   TCQCIntfMediaImg&      iwdgToCopy
        );

        ~TCQCIntfMediaImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMediaImg& operator=
        (
            const   TCQCIntfMediaImg&      iwdgToAssign
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

        tCIDLib::TBoolean bCanSizeTo() const override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        TBitmap bmpSample() const override;

        tCIDLib::TCard1 c1Opacity() const override;

        tCIDLib::TCard1 c1Opacity
        (
            const   tCIDLib::TCard1         c1ToSet
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

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   override;

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

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DownloadArt
        (
            const   TString&                strKey
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bmpBgn
        //      When the value changes, we load up the image we need. We don't
        //      support color based transparency for these images, so we don't
        //      need image masks.
        //
        //  m_c1Opacity
        //      Constant opacity set for our widget. This is set via a
        //      configuration tab.
        // -------------------------------------------------------------------
        TBitmap         m_bmpBgn;
        tCIDLib::TCard1 m_c1Opacity;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMediaImg,TCQCIntfSpecialImg)
        DefPolyDup(TCQCIntfMediaImg)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaRepoImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMediaRepoImg : public TCQCIntfSpecialImg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMediaRepoImg();

        TCQCIntfMediaRepoImg
        (
            const   TCQCIntfMediaRepoImg&   iwdgToCopy
        );

        ~TCQCIntfMediaRepoImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMediaRepoImg& operator=
        (
            const   TCQCIntfMediaRepoImg&   iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        TBitmap bmpSample() const override;

        tCIDLib::TCard1 c1Opacity() const override;

        tCIDLib::TCard1 c1Opacity
        (
            const   tCIDLib::TCard1         c1ToSet
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

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   override;

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
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

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCMedia::EMImgAttrs eAttr() const;

        tCQCMedia::EMImgAttrs eAttr
        (
            const   tCQCMedia::EMImgAttrs   eToSet
        );

        const TString& strRepoMoniker() const;

        const TString& strRepoMoniker
        (
             const  TString&                strToSet
        );


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
        tCIDLib::TVoid LoadPosterArt
        (
            const   TString&                strColCookie
        );

        tCIDLib::TVoid LoadRatingImg
        (
            const   TString&                strColCookie
            , const TCQCSecToken&           sectUser
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bmpBgn
        //      When we get a Set Title command we go get the image and store
        //      it here.
        //
        //  m_c1Opacity
        //      Constant opacity set for our widget. This is set via a
        //      configuration tab.
        //
        //  m_eAttr
        //      The particular attribute we are to display. This is a config
        //      time setting.
        //
        //  m_strRepoMoniker
        //      The moniker of the repository driver that we are associated
        //      with. This is a design time thing.
        // -------------------------------------------------------------------
        TBitmap                 m_bmpBgn;
        tCIDLib::TCard1         m_c1Opacity;
        tCQCMedia::EMImgAttrs   m_eAttr;
        TString                 m_strRepoMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMediaRepoImg,TCQCIntfSpecialImg)
        DefPolyDup(TCQCIntfMediaRepoImg)
};


#pragma CIDLIB_POPPACK


