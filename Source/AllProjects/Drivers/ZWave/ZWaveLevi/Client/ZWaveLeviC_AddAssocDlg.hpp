//
// FILE NAME: ZWaveLeviC_AddAssocDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/16/2012
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
//  This is the header for the ZWaveLeviC_AddAssocDlg.cpp file, which
//  implements the TZWaveAddAssocDlg class. This class allows the user to set
//  a configuration parameter value on a selected unit.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveAddAssocDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWaveAddAssocDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveAddAssocDlg();

        TZWaveAddAssocDlg(const TZWaveAddAssocDlg&) = delete;

        ~TZWaveAddAssocDlg();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TZWaveAddAssocDlg& operator=(const TZWaveAddAssocDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Run
        (
            const   TWindow&                wndOwner
            , const TString&                strMoniker
            , const TZWaveDrvConfig&        dcfgUnits
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


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pdcfgUnits
        //      We get a pointer to the current configuration so that we
        //      can load up the list of units and get their ids and whatnot.
        //
        //  m_strMoniker
        //      The driver passes us the moniker so that we can create a
        //      client proxy and send the set config command to the driver.
        // -------------------------------------------------------------------
        const TZWaveDrvConfig*  m_pdcfgUnits;
        TPushButton*            m_pwndClose;
        TNumSpinBox*            m_pwndGrpNum;
        TPushButton*            m_pwndSet;
        TEnumSpinBox*           m_pwndSrcUnit;
        TEnumSpinBox*           m_pwndTarUnit;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveAddAssocDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



