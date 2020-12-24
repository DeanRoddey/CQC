//
// FILE NAME: ElkM1V2C_EdLoadDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/12/2014
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
//  This is the header for the ElkM1V2C_EdLoadDlg.cpp file, which implements
//  the TEdM1Load class. This dialog is used to edit lighting loads.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEdM1LoadDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdM1LoadDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdM1LoadDlg();

        TEdM1LoadDlg(const TEdM1LoadDlg&) = delete;

        ~TEdM1LoadDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEdM1LoadDlg& operator=(const TEdM1LoadDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const tCIDLib::TCard4         c4Index
            ,       tElkM1V2C::TItemList&   colList
            ,       TLoadInfo&              iiEdit
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

        tCIDLib::TVoid SaveChanges();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Index
        //      The caller tells us the index of the item so that we can
        //      can ignore it when checking for dups.
        //
        //  m_pcolList
        //      A pointer to the list of loads. We need this to check for name
        //      or id dups.
        //
        //  m_iiOrg
        //      A copy of the original incoming load object.
        //
        //  m_pwndXXXX
        //      Since we have to interact with these controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them in a convenient way.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4Index;
        tElkM1V2C::TItemList*   m_pcolList;
        TLoadInfo               m_iiOrg;
        TPushButton*            m_pwndCancel;
        TEntryField*            m_pwndElkId;
        TEnumSpinBox*           m_pwndType;
        TEntryField*            m_pwndName;
        TPushButton*            m_pwndSave;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdM1LoadDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


