//
// FILE NAME: ElkM1V2C_VoltDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/13/2007
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
//  This is the header for the ElkM1V2C_VoltDlt.cpp file, which implements
//  the TEdM1Volt class. This class handles the editing of the info we store
//  for a voltage in the M1. The raw voltage value isn't of much use, so
//  we allow them to provide the A, B, and C values of a quadratic equation
//  that is used to map the voltage value to something they can use. And
//  they also edit the used/unused setting here.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEdM1VoltDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdM1VoltDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdM1VoltDlg();

        TEdM1VoltDlg(const TEdM1VoltDlg&) = delete;

        ~TEdM1VoltDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TEdM1VoltDlg& operator=(const TEdM1VoltDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const tCIDLib::TCard4         c4Index
            ,       tElkM1V2C::TItemList&   colList
            ,       TVoltInfo&              iiEdit
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
        //      The caller tells us the index of the voltage so that we can
        //      can get the right one out of the list.
        //
        //  m_iiEdit
        //      A copy of the incoming object to edit. We put the changes back
        //      into this and it will be copied to the caller's object if the
        //      user commits.
        //
        //  m_iiOrg
        //      We have to save the original so we know if there are changes
        //      at the end.
        //
        //  m_pcolList
        //      A pointer to the list of voltages. We need this to check for name
        //      or id dups.
        //
        //  m_pwndXXXX
        //      Since we have to interact with these controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them in a convenient way.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4Index;
        TVoltInfo               m_iiEdit;
        TVoltInfo               m_iiOrg;
        tElkM1V2C::TItemList*   m_pcolList;
        TEntryField*            m_pwndAVal;
        TEntryField*            m_pwndBVal;
        TPushButton*            m_pwndCancel;
        TEntryField*            m_pwndCVal;
        TEntryField*            m_pwndElkId;
        TEntryField*            m_pwndHighLim;
        TEntryField*            m_pwndLowLim;
        TEntryField*            m_pwndName;
        TPushButton*            m_pwndSave;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdM1VoltDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


