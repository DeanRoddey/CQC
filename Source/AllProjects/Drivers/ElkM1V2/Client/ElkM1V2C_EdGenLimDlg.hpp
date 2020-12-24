//
// FILE NAME: ElkM1V2C_GenLimDlg.hpp
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
//  This is the header for the ElkM1V2C_GenLinDlt.cpp file, which implements
//  the TEdM1GenLim class. This dialog is used for all the types that only need
//  a name, id, and low/high limit values.
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
//   CLASS: TEdM1GenLimDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdM1GenLimDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdM1GenLimDlg();

        TEdM1GenLimDlg(const TEdM1GenLimDlg&) = delete;

        ~TEdM1GenLimDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TEdM1GenLimDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const tCIDLib::TCard4         c4Index
            ,       tElkM1V2C::TItemList&   colList
            ,       TLimInfo&               iiEdit
            , const TString&                strInstruct
            , const tElkM1V2C::EItemTypes   eType
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
        //  m_eType
        //      The type we are editing, for setting up the title text.
        //
        //  m_pcolList
        //      A pointer to the list of items. We need this to check for name
        //      or id dups.
        //
        //  m_piiOrg
        //      A pointer to the original object
        //
        //  m_pwndXXXX
        //      Since we have to interact with these controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them in a convenient way.
        //
        //  m_strInstruct
        //      This is a generic one, so we have to get the instruction text.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4Index;
        tElkM1V2C::EItemTypes   m_eType;
        tElkM1V2C::TItemList*   m_pcolList;
        TLimInfo*               m_piiOrg;
        TPushButton*            m_pwndCancel;
        TEntryField*            m_pwndElkId;
        TEntryField*            m_pwndLowLim;
        TEntryField*            m_pwndHighLim;
        TEntryField*            m_pwndName;
        TPushButton*            m_pwndSave;
        TString                 m_strInstruct;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdM1GenLimDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK




