//
// FILE NAME: MediaRepoMgr_EdTitleSetDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/13/2006
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
//  This is the header for MediaRepoMgr_EditTitleSetDlg.cpp, which implements a
//  dialog for editing the contents of a title set. This is only valid for existing
//  title sets that are in the database. Title sets are only created in the process
//  of editing a collection, when the user selects which title set the collection
//  should be in. A new one is created at that point if needed.
//
//  So here we always have existing sets to edit. This means we can just copy an
//  incoming title set, modify it in place, and then commit it back to the DB at
//  the end.
//
//  Since we can delete collections here, the caller should redisplay the list
//  of collections for this set if applicable.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEditTitleSetDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEditTitleSetDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        enum class EEditRes
        {
            NoChange    = 9997
            , Changed   = 9998
            , Deleted   = 9999
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEditTitleSetDlg();

        TEditTitleSetDlg(const TEditTitleSetDlg&) = delete;

        ~TEditTitleSetDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TEditTitleSetDlg& operator=(const TEditTitleSetDlg&);


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        EEditRes eRun
        (
            const   TWindow&                wndOwner
            , const TMediaTitleSet&         mtsEdit
            ,       TMediaDB&               mdbEdit
            , const TString&                strRepoMoniker
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
        tCIDLib::TBoolean bValidate();

        tCIDLib::TVoid DeleteCollect();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TVoid LoadArt
        (
            const   tCIDLib::TCard2         c2ArtId
        );

        tCIDLib::TVoid SelectArt();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNewImage
        //      In order to avoid uploading images that never get used, we just
        //      store any newly selected image and set this flag. At the end,
        //      if they commit, we store the new image in m_imgNew;
        //
        //  m_c2ArtId
        //      The incoming art id, so we know if we had any to start with or
        //      not.
        //
        //  m_eMType
        //      We pull the media type out of the incoming set, since it is used a
        //      lot.
        //
        //  m_eRes
        //      The result to return is left in this member, since we have more states
        //      than buttons.
        //
        //  m_imgNew
        //      If a new image is loaded, we store it here until the user commits
        //      and we can upload it.
        //
        //  m_mtsEdit
        //      A pointer to the caller's title set to edit.
        //
        //  m_pmdbEdit
        //      The database we put the changes into if the user commits.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our child controls we want to
        //      interact with in a type specific way. We don't own them.
        //
        //  m_strRepoMoniker
        //      The caller gives us the repo moniker we should talk to if we need
        //      to query anything, image data in our case.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bNewImage;
        tCIDLib::TCard2         m_c2ArtId;
        tCQCMedia::EMediaTypes  m_eMType;
        EEditRes                m_eRes;
        TJPEGImage              m_imgiNew;
        TMediaTitleSet          m_mtsEdit;
        TMediaDB*               m_pmdbEdit;
        TStaticImg*             m_pwndArt;
        TEntryField*            m_pwndArtist;
        TPushButton*            m_pwndCancel;
        TPushButton*            m_pwndColDel;
        TPushButton*            m_pwndColDn;
        TListBox*               m_pwndColList;
        TPushButton*            m_pwndColUp;
        TPushButton*            m_pwndDelArt;
        TPushButton*            m_pwndSave;
        TPushButton*            m_pwndSetArt;
        TEntryField*            m_pwndSortTitle;
        TStaticText*            m_pwndTitle;
        TComboBox*              m_pwndUserRating;
        TString                 m_strRepoMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEditTitleSetDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


