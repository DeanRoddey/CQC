//
// FILE NAME: CQCIntfEng_MediaCatBrowser.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/28/2005
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
//  This is the header for the CQCIntfEng_MediaCatBrowser.cpp file, which
//  implements a horizontal browser for the category lists of media drivres.
//  It is associated with a media drivers, not a driver field as most device
//  related widgets are.
//
//  This guy allows the user to page through the category names in a set of
//  horizontal slots. It has no look of its own, it just displays text in
//  regularly spaced slots, which can be placed over any background. In
//  designer mode, we just display some sample text, and an outline of the
//  area, so that they can see the text appearance and know where the area
//  limits are.
//
//  This guy gets key/value pairs for the values it displays. The key is
//  the human readable text of the category and the value is the driver
//  specific category cookie. This later is what is used in any commands to
//  the driver that tell it to do something category related. We make both
//  these values available as runtime values so that they can be passed in
//  commands.
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
//   CLASS: TCQCIntfMCatRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMCatBRTV : public TCQCIntfHVBrwRTV
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMCatBRTV() = delete;
        TCQCIntfMCatBRTV
        (
            const   TString&                strCookie
            , const TString&                strText
            , const tCIDLib::TCard4         c4SelectIndex
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfMCatBRTV(const TCQCIntfMCatBRTV&) = delete;

        ~TCQCIntfMCatBRTV();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMCatBRTV& operator=(const TCQCIntfMCatBRTV&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue
        (
            const   TString&                strId
            ,       TString&                strToFill
        )  const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strCookie
        //      The category cookie values we got in the ctor. The text and
        //      index are stored in the base class.
        // -------------------------------------------------------------------
        TString m_strCookie;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMCatBRTV, TCQCIntfHVBrwRTV)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaCatBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMediaCatBrowser : public TCQCIntfHVBrowserBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMediaCatBrowser();

        TCQCIntfMediaCatBrowser
        (
            const   TCQCIntfMediaCatBrowser& iwdgToCopy
        );

        ~TCQCIntfMediaCatBrowser();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMediaCatBrowser& operator=
        (
            const   TCQCIntfMediaCatBrowser& iwdgToAssign
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
                    TCollection<TCQCCmd>&   colToFill
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

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const override;

        tCIDLib::TVoid PostInit() override;

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
        tCQCMedia::EMediaTypes eInitMediaType() const;

        tCQCMedia::EMediaTypes eInitMediaType
        (
            const   tCQCMedia::EMediaTypes  eToSet
        );

        tCQCMedia::EMTFlags eMediaFlags() const;

        tCQCMedia::EMTFlags eMediaFlags
        (
            const   tCQCMedia::EMTFlags     eToSet
        );

        tCQCMedia::EMediaTypes eMediaType() const;

        tCQCMedia::EMediaTypes eMediaType
        (
            const   tCQCMedia::EMediaTypes  eToSet
        );

        const TString& strRepoMoniker() const;

        const TString& strRepoMoniker
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4NameCount() const override;

        const TString& strDisplayTextAt
        (
            const   tCIDLib::TCard4         c4Index
        )   const override;

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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadCategories();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eInitMediaType
        //      This is persisted and is used to set the initial media type
        //      to be displayed. If it is found not to be supported by the
        //      target repository, it is ignored. This is persisted of course.
        //
        //  m_eMediaType
        //      The current media type we are browsing. If the repository only
        //      supports one type, then this won't change. If it supports
        //      multiple types, then we accept a command that will let the user
        //      change the media type dynamically.
        //
        //      It is initially set to EMediaType_Count, and that is unised
        //      to know when we do the initial load. At that point, we'll ask
        //      the repository what it supports and se if the initial is
        //      in that set. If so, we take the initial, else we select one
        //      that is supported.
        //
        //  m_eMTFlags
        //      The types of media supported by our driver, which we query
        //      from it during initialization. This is used to select an
        //      initial type that is supported and to limit incoming commands
        //      to set the type to those that are supported. This is a runtime
        //      value only.
        //
        //  m_colCats
        //      These are the categories. For each category we have a human
        //      readable category name (the key) and a category cookie. These
        //      are the categories for a particular media type, indicated in
        //      m_eMediaType.
        //
        //  m_strRepoMoniker
        //      The moniker of the media repository driver we get category
        //      info from. This is persisted and used at runtime to send
        //      backdoor type commands to the media driver we are associated
        //      with.
        // -------------------------------------------------------------------
        tCQCMedia::EMediaTypes  m_eInitMediaType;
        tCQCMedia::EMTFlags     m_eMediaFlags;
        tCQCMedia::EMediaTypes  m_eMediaType;
        TVector<TKeyValuePair>  m_colCats;
        TString                 m_strRepoMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMediaCatBrowser,TCQCIntfHVBrowserBase)
        DefPolyDup(TCQCIntfMediaCatBrowser)
};


#pragma CIDLIB_POPPACK


