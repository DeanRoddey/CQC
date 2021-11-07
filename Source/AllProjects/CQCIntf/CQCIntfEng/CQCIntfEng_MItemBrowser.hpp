//
// FILE NAME: CQCIntfEng_MItemBrowser.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2005
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
//  This is the header for the CQCIntfEng_MItemBrowser.cpp file, which
//  implements a browser for the items within a collection of a media
//  renderer driver. It is associated with the 'playlist serial number' field
//  of a media renderer device. That value changes any time the playlist
//  changes. This widget will watch for such changes and reload the playlist
//  info.
//
//  It maintains the artist name, item name, item cookie, and location info for
//  each of the items in its list, and provides them as RTVs when one of the
//  list items is clicked on.
//
// CAVEATS/GOTCHAS:
//
//  1)  There is an advanced media item browser that allows for fancier display
//      of the content. It shares our runtime value class in order to avoid
//      just redundantly creating effectively the same thing. So be sure to
//      update it if you change the RTV class below.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMIBrwRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMIBrwRTV : public TCQCIntfHVBrwRTV
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMIBrwRTV() = delete;

        TCQCIntfMIBrwRTV
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfMIBrwRTV
        (
            const   TCQCMediaPLItem&        mpliSelected
            , const tCIDLib::TCard4         c4SelectIndex
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfMIBrwRTV(const TCQCIntfMIBrwRTV&) = delete;

        ~TCQCIntfMIBrwRTV();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMIBrwRTV& operator=(const TCQCIntfMIBrwRTV&) = delete;


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
        //  m_c4ListItemId
        //      The unique playlist item id for this item.
        //
        //  m_strArtist
        //  m_strCookie
        //  m_strLocInfo
        //      The artist, cookie, and location info of the item selected
        //      in the item browser. The name is stored in the text member
        //      of the parent class.
        //
        //  m_strRepoMoniker
        //      The moniker of the repository that the selected item was
        //      associated with.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4ListItemId;
        TCQCMediaPLItem m_mpliSelected;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMIBrwRTV, TCQCIntfHVBrwRTV)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMItemBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMItemBrowser

    : public TCQCIntfHVBrowserBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMItemBrowser();

        TCQCIntfMItemBrowser
        (
            const   TCQCIntfMItemBrowser&   iwdgToCopy
        );

        ~TCQCIntfMItemBrowser();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMItemBrowser& operator=
        (
            const   TCQCIntfMItemBrowser&   iwdgToAssign
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
        tCIDLib::TCard4 c4NameCount() const override;

        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;

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
        tCIDLib::TBoolean bLoadItems();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colItems
        //      These hold the playlist items we get from the renderer driver.
        // -------------------------------------------------------------------
        TVector<TCQCMediaPLItem>    m_colItems;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMItemBrowser,TCQCIntfHVBrowserBase)
        DefPolyDup(TCQCIntfMItemBrowser)
};

#pragma CIDLIB_POPPACK


