//
// FILE NAME: MediaRepoMgr_EdMCatsDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/04/2006
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
//  This is the header for MediaRepoMgr_EdMCatsDlg.cpp, which implements a small
//  dialog for editing the media categories. Categories are added on the fly as new
//  collections are uploaded. But sometimes they want to consolidate categories, or
//  rename them, or add new ones manually and so forth.
//
//  On entry, we get copies of all of the names and unique ids of the categories for
//  each media type. We allow the user to edit these lists. If any removed, we move
//  those over to a deleted list.
//
//  At the end, if the user commits, we go through and for any that are not in
//  the DB currently we add them, and any in the deleted list are removed from
//  the DB.
//
// CAVEATS/GOTCHAS:
//
//  1)  It's always possible that they might add a category and it's already in the
//      deleted list, i.e. they put back one with the same name as something they
//      deleted. We just move it back out of the deleted list to the regular list
//      again in that case.
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEdMCatsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdMCatsDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdMCatsDlg();

        TEdMCatsDlg(const TEdMCatsDlg&) = delete;

        ~TEdMCatsDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TEdMCatsDlg& operator=(const TEdMCatsDlg&);


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
            const   TWindow&                wndOwner
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
        tCIDLib::TVoid AddCategory();

        tCIDLib::TBoolean bCheckDup
        (
            const   TString&                strToCheck
            , const tCQCMedia::EMediaTypes  eType
        )   const;

        tCIDLib::TBoolean bCheckIsSpecial
        (
            const   TString&                strName
            , const TString&                strUniqueId
            , const tCQCMedia::EMediaTypes  eType
        )   const;

        tCIDLib::TBoolean bCommitChanges();

        tCIDLib::TVoid DeleteCurrent();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eComboHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditCategory
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid LoadCats();


        typedef TObjArray<tCIDLib::TKVPList, tCQCMedia::EMediaTypes>  TMTKVArray;

        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_acolDel
        //      A list of names and unique ids for each media type of cats that we
        //      deleted. These need to be removed at the end.
        //
        //  m_acolNames
        //      A list of names and unique ids for each media type that is loaded at
        //      the start and this is what we edit.
        //
        //  m_eType
        //      The currently selected media type. The categories are per-
        //      media type.
        //
        //  m_pmdbEdit
        //      A pointer to the media database we are editing. We store the pointer
        //      so that we can do the commit processing before we exit and can tell
        //      the user of issues while still in the dialog, so we can remain here
        //      for the user to fix the problem.
        //
        //  m_pwndXXX
        //      Pointers to the child controls we want to interact with in a
        //      type specific way. We don't own them.
        // -------------------------------------------------------------------
        TMTKVArray              m_objaDel;
        TMTKVArray              m_objaNames;
        tCQCMedia::EMediaTypes  m_eType;
        TMediaDB*               m_pmdbEdit;
        TPushButton*            m_pwndAdd;
        TPushButton*            m_pwndCancel;
        TPushButton*            m_pwndDelete;
        TMultiColListBox*       m_pwndList;
        TComboBox*              m_pwndMType;
        TPushButton*            m_pwndSave;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdMCatsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


