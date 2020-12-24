//
// FILE NAME: ZWaveLevi2C_SelTypeDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2014
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
//  This is the header for the ZWaveLevi2C_SelTypeDlg.cpp file, which implements the
//  TZWaveSelTypeDlg class. This class allows the user to select the device info
//  info for a unit.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveSelTypeDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWaveSelTypeDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveSelTypeDlg();

        TZWaveSelTypeDlg(const TZWaveSelTypeDlg&) = delete;

        ~TZWaveSelTypeDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveSelTypeDlg& operator=(const TZWaveSelTypeDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TZWaveUnit&             unitEdit
            , const TZDIIndex&              zwdinDevs
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

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadMakes();

        tCIDLib::TVoid LoadModels
        (
            const   TString&                strForMake
            , const TString&                strInitModel
        );


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        TPushButton*        m_pwndCancel;
        TMultiColListBox*   m_pwndMakes;
        TMultiColListBox*   m_pwndModels;
        TPushButton*        m_pwndSave;
        TZWaveUnit*         m_punitEdit;
        const TZDIIndex*    m_pzwdinDevs;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveSelTypeDlg, TDlgBox)
};

#pragma CIDLIB_POPPACK
