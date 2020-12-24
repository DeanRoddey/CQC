//
// FILE NAME: IRClient_SelModelDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/22/2002
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
//  This is the header for the device model selection dialog.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TSelModelDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TSelModelDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TVector<TIRBlasterDevModelInfo> TInfoList;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TSelModelDlg();

        TSelModelDlg(const TSelModelDlg&) = delete;

        ~TSelModelDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSelModelDlg& operator=(const TSelModelDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TString&                strToFill
            , const TString&                strDevType
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

        tCIDCtrls::EEvResponses eComboHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadModelList();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colList
        //      We load up a list of the available devices from the IR repository and
        //      store them here for access.
        //
        //  m_eDevice
        //      The IR device type that the caller want's us to operate on. This tells us
        //      what IR repository to act on.
        //
        //  m_pwndXXXX
        //      Since we have to interact with these controls a lot, we get convenient
        //      typed, pointers to them. We don't own these.
        //
        //  m_strDevType
        //      The device type, which is our key for accessing the models of the server
        //      side device.
        //
        //  m_strSelected
        //      We need a place to store the selected model until we can get it back to
        //      the caller's parameter.
        // -------------------------------------------------------------------
        TInfoList           m_colList;
        TCheckBox*          m_pwndByCat;
        TCheckBox*          m_pwndByMake;
        TPushButton*        m_pwndCancelButton;
        TStaticText*        m_pwndCategory;
        TComboBox*          m_pwndCatFilter;
        TPushButton*        m_pwndDelButton;
        TStaticText*        m_pwndDescr;
        TMultiColListBox*   m_pwndList;
        TStaticText*        m_pwndMake;
        TComboBox*          m_pwndMakeFilter;
        TPushButton*        m_pwndSelectButton;
        TString             m_strDevType;
        TString             m_strSelected;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TSelModelDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


