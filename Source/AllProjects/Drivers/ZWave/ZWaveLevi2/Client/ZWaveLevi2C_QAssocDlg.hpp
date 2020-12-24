//
// FILE NAME: ZWaveLevi2C_QAssocDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/12/2014
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
//  This is the header for the ZWaveLevi2C_QAssocDlg.cpp file, which implements the
//  TZWaveQAssocDlg class. This class allows the user to query the list of units that
//  associated with a particular group in a source unit.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveQAssocDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWaveQAssocDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveQAssocDlg();

        TZWaveQAssocDlg(const TZWaveQAssocDlg&) = delete;

        ~TZWaveQAssocDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TZWaveQAssocDlg& operator=(const TZWaveQAssocDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strMoniker
            , const TZWaveUnit&             unitSrc
            , const TZWaveDrvConfig&        dcfgCur
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
        //  m_pdcfgCur
        //      We get the configuration just so that we can display names for the
        //      associated units, where we have them, instead of just raw ids.
        //
        //  m_punitSrc
        //      We get a pointer to the unit to which we will add the association,
        //      which is the 'source' in the association, i.e. the one that wil send
        //      out notifications.
        //
        //  m_pdcfgUnits
        //      We get a pointer to the current configuration so that we can load up
        //      up the list of units and get their ids and whatnot.
        //
        //  m_strMoniker
        //      The driver passes us the moniker so that we can create a client proxy
        //      and send the backdoor commands to the driver.
        // -------------------------------------------------------------------
        const TZWaveDrvConfig*  m_pdcfgCur;
        const TZWaveUnit*       m_punitSrc;
        TPushButton*            m_pwndClose;
        TNumSpinBox*            m_pwndGrpNum;
        TMultiEdit*             m_pwndOutput;
        TPushButton*            m_pwndQuery;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveQAssocDlg, TDlgBox)
};

#pragma CIDLIB_POPPACK


