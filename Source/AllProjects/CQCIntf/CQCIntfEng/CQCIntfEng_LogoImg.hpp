//
// FILE NAME: CQCIntfEng_LogoImg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/02/2004
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
//  This is the header for the CQCIntfEng_LogoImg.cpp file, which implements
//  a widget that is specialized for display of the images that map to channel
//  logos. It is set up to point to a paritcular scope within the image
//  repository, and when the associated field has a value of 10, it will look
//  in that scope for an image named 10. So it provides a very simple and easy
//  way to map a field that contains a channel number to a visual logo that
//  represents that channel.
//
//  Note that this used to be a specialized DirecTV logo widget. It was
//  expanded to be more general, but we can't change the name until we have
//  the installer go through the data and proactively change the names, since
//  the templates use polymorhic streaming based on class name. So all of
//  the templates have TCQCIntfDTVLogoImg embedded in them. For now, it can
//  just use the old class name. The user visible name is changed to represent
//  it's new, more generic, usage.
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
//   CLASS: TCQCIntfDTVLogoImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfDTVLogoImg

    : public TCQCIntfWidget, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static const TString strDTVScope;
        static const TString strXMScope;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfDTVLogoImg();

        TCQCIntfDTVLogoImg
        (
            const   TCQCIntfDTVLogoImg&     iwdgToCopy
        );

        ~TCQCIntfDTVLogoImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfDTVLogoImg& operator=
        (
            const   TCQCIntfDTVLogoImg&     iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bCanSizeTo() const override;

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

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
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


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        TBitmap bmpSample() const;

        tCIDLib::TCard1 c1Opacity() const;

        tCIDLib::TCard1 c1Opacity
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        const TString& strRepoPath() const;

        const TString& strRepoPath
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
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

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadLogoImg
        (
            const   TString&                strKey
        );

        tCIDLib::TVoid LoadLogoImg
        (
                    TDataSrvClient&         dsclInit
            , const TString&                strKey
        );

        tCIDLib::TVoid NewLogoImgVal
        (
            const   tCIDLib::TBoolean       bNoRedraw
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c1Opacity
        //      Constant opacity set for our widget.
        //
        //  m_cptrImg
        //      When the value changes, we load up the image we need.
        //
        //  m_strRepoPath
        //      This was added in 1.6, to make this widget more general. So
        //      instead of a hard coded path, it can be given any path.
        // -------------------------------------------------------------------
        tCIDLib::TCard1     m_c1Opacity;
        TIntfImgCachePtr    m_cptrImg;
        TString             m_strRepoPath;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfDTVLogoImg,TCQCIntfWidget)
        DefPolyDup(TCQCIntfDTVLogoImg)
};


#pragma CIDLIB_POPPACK


