//
// FILE NAME: ZWaveLevi2C_SetCfgParamDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/11/2014
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
//  This is the header for the ZWaveLevi2C_SetCfgParamDlg.cpp file, which
//  implements the TZWaveSetCfgParamDlg class. This class allows the user
//  to set a configuration parameter value on a selected unit.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveSetCfgParamDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWaveSetCfgParamDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveSetCfgParamDlg();

        TZWaveSetCfgParamDlg(const TZWaveSetCfgParamDlg&) = delete;
        TZWaveSetCfgParamDlg(TZWaveSetCfgParamDlg&&) = delete;

        ~TZWaveSetCfgParamDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveSetCfgParamDlg& operator=(const TZWaveSetCfgParamDlg&) = delete;
        TZWaveSetCfgParamDlg& operator=(TZWaveSetCfgParamDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
                    TZWaveLevi2CWnd&        wndOwner
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
        //  m_pwndDriver
        //      We need a pointer to the driver window (our owner) to do some stuff.
        //
        //  m_pwndXXX
        //      Some typed pointers to child controls, which we don't own.
        //
        //  m_strMoniker
        //      The driver passes us the moniker so that we can create a
        //      client proxy and send the set config command to the driver.
        // -------------------------------------------------------------------
        const TZWaveDrvConfig*  m_pdcfgUnits;
        TPushButton*            m_pwndClose;
        TZWaveLevi2CWnd*        m_pwndDriver;
        TNumSpinBox*            m_pwndPBytes;
        TNumSpinBox*            m_pwndPNum;
        TEntryField*            m_pwndPVal;
        TPushButton*            m_pwndQuery;
        TPushButton*            m_pwndSet;
        TComboBox*              m_pwndTarUnit;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveSetCfgParamDlg, TDlgBox)
};

#pragma CIDLIB_POPPACK


