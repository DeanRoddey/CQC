//
// FILE NAME: MediaRepoMgr_SelTitleSetDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/23/2006
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
//  This is the header for MediaRepoMgr_SelTitleSetDlg.cpp, which allows the user
//  to select a title set into which a collection will be placed. We get the collect
//  and potentially incoming existing title set info. We don't actually add a new
//  title set to the database, we just return the basic info which the caller can
//  use or not.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TSelTitleSetDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TSelTitleSetDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TSelTitleSetDlg();

        TSelTitleSetDlg(const TSelTitleSetDlg&) = delete;

        ~TSelTitleSetDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSelTitleSetDlg& operator=(const TSelTitleSetDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TMediaTitleSet&         mtsEdit
            , const TMediaCollect&          mcolSrc
            ,       TMediaDB&               mdbEdit
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
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TVoid LoadTitleList
        (
            const   tCIDLib::TCard2         c2InitId
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLoadedList
        //      We use this to fault in the current title list, since during the
        //      ripping process they will often almost always create a new title
        //      set anyway so we will avoid even having to load the list.
        //
        //  m_pmdbEdit
        //      We need the database in order to load the title set list in case they
        //      want to add the collection to an existing title set.
        //
        //  m_pmcolSrc
        //      A pointer to the caller's collection that we are adding to a title
        //      set. We don't modify this.
        //
        //  m_pmtsEdit
        //      A pointer to the caller's title set that we will edit/fill in with
        //      new info.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our child controls we want to
        //      interact with in a type specific way. We don't own them.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLoadedList;
        TMediaTitleSet          m_mtsEdit;
        TMediaDB*               m_pmdbEdit;
        const TMediaCollect*    m_pmcolSrc;
        TPushButton*            m_pwndCancel;
        TRadioButton*           m_pwndExisting;
        TListBox*               m_pwndList;
        TRadioButton*           m_pwndNew;
        TEntryField*            m_pwndNewName;
        TPushButton*            m_pwndSave;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TSelTitleSetDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK




