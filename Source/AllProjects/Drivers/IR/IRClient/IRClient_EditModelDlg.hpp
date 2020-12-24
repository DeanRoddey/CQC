//
// FILE NAME: IRClient_EditModelDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/24/2002
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
//  This is the header for the device edit/create dialog.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TIRBlasterTab;

// ---------------------------------------------------------------------------
//   CLASS: TEditModelDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEditModelDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEditModelDlg();

        TEditModelDlg(const TEditModelDlg&) = delete;

        ~TEditModelDlg();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TEditModelDlg& operator=(const TEditModelDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TIRBlasterDevModel&     irbdmTarget
            ,       TIRClientWnd* const     pwndDriver
            , const tCIDLib::TCard4         c4ZoneCount
            , const tCIDLib::TBoolean       bEnableSave = kCIDLib::False
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
        tCIDLib::TBoolean bSaveData();

        tCIDLib::TVoid DeleteCmd();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid InvokeCmd();

        tCIDLib::TVoid LoadCmds();

        tCIDLib::TVoid NewEditCmd
        (
            const   tCIDLib::TBoolean       bNew
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bChanges
        //      This is set any time we modify anything, so that we can warn
        //      about lost changes on the way out if needed.
        //
        //  m_bEnableSave
        //      When we do a model import, we want the save button to be
        //      enabled from the start, because they have changes to store
        //      as soon as the dialog is shown.
        //
        //  m_c4ZoneCount
        //      The blaster tab knows how many zones, since he got it during
        //      connect, so he passes it to us so we don't have to figure it
        //      out ourself. We store it here so that we can load up the
        //      zones spin box duirng init.
        //
        //  m_pirdevTarget
        //      A pointer to our target device object that we are editing
        //      or filling in.
        //
        //  m_pwndDriver
        //      We need a pointer to the client so that we can have access
        //      to it's driver client proxy and lock it and so forth.
        //
        //  m_pwndXXX
        //      Since we have to interact with some controls a lot, we get
        //      typed pointers to them. We don't own these, we just are
        //      looking at them a convenient way.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bChanges;
        tCIDLib::TBoolean   m_bEnableSave;
        tCIDLib::TCard4     m_c4ZoneCount;
        TIRBlasterDevModel* m_pirbdmTarget;
        TComboBox*          m_pwndCategories;
        TPushButton*        m_pwndCancelButton;
        TPushButton*        m_pwndDelButton;
        TEntryField*        m_pwndDescr;
        TIRClientWnd*       m_pwndDriver;
        TStaticText*        m_pwndModel;
        TMultiColListBox*   m_pwndList;
        TEntryField*        m_pwndMake;
        TPushButton*        m_pwndNewButton;
        TComboBox*          m_pwndRepeatCount;
        TPushButton*        m_pwndSaveButton;
        TPushButton*        m_pwndTestButton;
        TPushButton*        m_pwndEditButton;
        TComboBox*          m_pwndZone;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEditModelDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


