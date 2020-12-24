//
// FILE NAME: MediaRepoMgr_EdMItemDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/31/2006
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
//  This is the header for MediaRepoMgr_EdItemDlg.cpp, which implements a dialog
//  that lets the user edit the attributes of a media item object. For items, we
//  sometimes are editing new ones that haven't been committed and sometimes ones
//  that are in the DB already.
//
//  So the caller provides us with an item object which we copy internally and use
//  for edits. We don't commit to the database, we just edit the object and copy
//  it back to the caller's object if the user commits. He will commit it if he
//  wants to. This means we don't need a database pointer as well.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEdMItemDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdMItemDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdMItemDlg();

        TEdMItemDlg(const TEdMItemDlg&) = delete;

        ~TEdMItemDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TEdMItemDlg& operator=(const TEdMItemDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const TMediaCollect&          mcolParent
            ,       TMediaItem&             mitemEdit
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

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eMType
        //      We pull the collection's media type out for quick access.
        //
        //  m_pitemEdit
        //      A copy of the caller's item object which we edit and copy back at
        //      the end if the user commits.
        //
        //  m_pmcolParent
        //      We get a const ref to the parent collection in case we want to get
        //      any info from it.
        //
        //  m_pwndXXX
        //      Typed pointers to children that we want to interact with directly.
        // -------------------------------------------------------------------
        tCQCMedia::EMediaTypes  m_eMType;
        TMediaItem              m_mitemEdit;
        const TMediaCollect*    m_pmcolParent;
        TEntryField*            m_pwndArtist;
        TPushButton*            m_pwndCancel;
        TEntryField*            m_pwndDuration;
        TPushButton*            m_pwndSave;
        TEntryField*            m_pwndTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdMItemDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


