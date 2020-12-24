//
// FILE NAME: ZWaveLevi2C_AddAssocDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2014
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
//  This is the header for the ZWaveLevi2C_AddAssocDlg.cpp file, which
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
        TZWaveAddAssocDlg(TZWaveAddAssocDlg&&) = delete;

        ~TZWaveAddAssocDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveAddAssocDlg& operator=(const TZWaveAddAssocDlg&) = delete;
        TZWaveAddAssocDlg& operator=(TZWaveAddAssocDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
                    TZWaveLevi2CWnd&        wndOwner
            , const TString&                strMoniker
            , const TZWaveUnit&             unitSrc
            , const tCIDLib::TCard4         c4VRCOPId
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


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
        //  m_c4VRCOPId
        //      The caller tells us the id of the VRCOP whose id to add to the group.
        //
        //  m_punitSrc
        //      We get a pointer to the unit to which we will add the association, which is
        //      the 'source' in the association, i.e. the one that wil send out
        //      notifications to the VRCOP (target.)
        //
        //  m_pwndDriver
        //      We need a pointer to the driver window (our owner) to do some stuff.
        //
        //  m_pwndXXX
        //      Some typed pointers to child controls, which we don't own.
        //
        //  m_strMoniker
        //      The driver passes us the moniker so that we can create a client proxy and
        //      send the set config command to the driver.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4VRCOPId;
        const TZWaveUnit*   m_punitSrc;
        TPushButton*        m_pwndClose;
        TZWaveLevi2CWnd*    m_pwndDriver;
        TNumSpinBox*        m_pwndGrpNum;
        TPushButton*        m_pwndSet;
        TString             m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveAddAssocDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



