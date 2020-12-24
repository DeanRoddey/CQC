//
// FILE NAME: MediaRepoMgr_SelMetaMatchDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2006
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
//  This is the header for MediaRepoMgr_SelMetaMatchDlg.cpp, which implements
//  a small dialog for displaying to the user all of the matches for a title
//  or disc id based metadata lookup. The user can select the one that they
//  want to use.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TSelMetaMatchDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TSelMetaMatchDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TSelMetaMatchDlg();

        TSelMetaMatchDlg(const TSelMetaMatchDlg&) = delete;

        ~TSelMetaMatchDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TSelMetaMatchDlg& operator=(const TSelMetaMatchDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TMyMoviesMetaSrc&       rmsToUse
            , const TString&                strVolume
            , const TKrnlRemMedia::EMediaTypes eMType
            ,       TRepoMetaMatch&         rmmSelected
        );

        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TMyMoviesMetaSrc* const prmsToUse
            ,       TRepoMetaMatch&         rmmSelected
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoSearch
        (
            const   tCIDLib::TBoolean       bTitleType
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadList();



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4SelIndex
        //      To leave the index of the selection which will be used to get back
        //      to the original list entry after we end.
        //
        //  m_colList
        //      A list of matches we are displaying. Initially we can do a
        //      disk ID based search if we are given one. After that this is
        //      updated with the results of any searches they choose to do.
        //
        //  m_eMType
        //      If we get an initial volume to use, the caller has to provide
        //      the low level media type info. This would have been gotten
        //      anyway during the initial probe of the drives to find available
        //      media. This tells us the type of id we have to generate for
        //      the lookup.
        //
        //  m_prmsToUse
        //      A pointer to the metadata source to use. The caller provides
        //      us with this, so we can work with any available source.
        //
        //  m_pwndXXX
        //      Typed points to controls we need to interact with.
        //
        //  m_strVolume
        //      We can be given a disc volume to attempt to automatically do
        //      a look for initially. If not, we come up empty and they can
        //      do a manual search.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4SelIndex;
        tMediaRepoMgr::TMetaList    m_colList;
        TKrnlRemMedia::EMediaTypes  m_eMType;
        TMyMoviesMetaSrc*           m_prmsToUse;
        TPushButton*                m_pwndCancel;
        TMultiColListBox*           m_pwndList;
        TEntryField*                m_pwndSearchVal;
        TPushButton*                m_pwndSelect;
        TPushButton*                m_pwndTitleSearch;
        TPushButton*                m_pwndUPCSearch;
        TString                     m_strVolume;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TSelMetaMatchDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


