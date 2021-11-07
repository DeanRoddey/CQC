//
// FILE NAME: CQCIntfEng_MRItemBrowser.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/21/2006
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
//  This is the header for the CQCIntfEng_MRItemBrowser.cpp file, which
//  implements a vertical browerserver variant that allows the user to
//  view item (track) information from an associated media repository,
//  mostly for preview purposes.
//
//  Our runtime values are the same as the other media item browser class,
//  so we just use his RTV class.
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
//   CLASS: TCQCIntfMRIBrwRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMRIBrwRTV : public TCQCIntfHVBrwRTV
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMRIBrwRTV() = delete;

        TCQCIntfMRIBrwRTV
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfMRIBrwRTV
        (
            const   TMediaItem&             mitemSelected
            , const TString&                strColCookie
            , const TString&                strRepoMoniker
            , const tCIDLib::TCard4         c4SelectIndex
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfMRIBrwRTV(const TCQCIntfMRIBrwRTV&) = delete;

        ~TCQCIntfMRIBrwRTV();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMRIBrwRTV& operator=(const TCQCIntfMRIBrwRTV&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue
        (
            const   TString&                strId
            ,       TString&                strToFill
        )  const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_mitemSelected
        //      The repo database item for the selected item.
        //
        //  m_strColCookie
        //      The collection cookie for the col this item is from.
        //
        //  m_strRepoMoniker
        //      The moniker of the repository that the selected item was
        //      associated with.
        // -------------------------------------------------------------------
        TMediaItem      m_mitemSelected;
        TString         m_strColCookie;
        TString         m_strRepoMoniker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMRIBrwRTV, TCQCIntfHVBrwRTV)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMRItemBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMRItemBrowser : public TCQCIntfHVBrowserBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMRItemBrowser();

        TCQCIntfMRItemBrowser
        (
            const   TCQCIntfMRItemBrowser&  iwdgToCopy
        );

        ~TCQCIntfMRItemBrowser();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMRItemBrowser& operator=
        (
            const   TCQCIntfMRItemBrowser&  iwdgToAssign
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
        tCIDLib::TBoolean bLoadItems
        (
            const   TString&                strTitleCookie
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_coltems
        //      These hold the info we get from the repository driver about
        //      the items we are currently displaying.
        //
        //  m_strColCookie
        //      The collection cookie we are currently holding the items of.
        //      This is set at runtime only, not persisted.
        //
        //  m_strRepoMoniker
        //      The moniker of the media repository driver we get category
        //      info from. This is persisted and used at runtime to send
        //      backdoor type commands to the media driver we are associated
        //      with.
        // -------------------------------------------------------------------
        TVector<TMediaItem> m_colItems;
        TString             m_strColCookie;
        TString             m_strRepoMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMRItemBrowser,TCQCIntfHVBrowserBase)
        DefPolyDup(TCQCIntfMRItemBrowser)
};


#pragma CIDLIB_POPPACK


