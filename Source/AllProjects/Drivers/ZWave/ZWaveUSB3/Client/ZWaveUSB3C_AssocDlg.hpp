//
// FILE NAME: ZWaveUSB3C_AssocDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2018
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
//  This is the header for a small dialog that lets the user manually manage the
//  associations of a unit.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3AssocDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWUSB3AssocDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWUSB3AssocDlg();

        TZWUSB3AssocDlg(const TZWUSB3AssocDlg&) = delete;

        ~TZWUSB3AssocDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUSB3AssocDlg& operator=(const TZWUSB3AssocDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
                    TZWaveUSB3CWnd&         wndDriver
            , const TZWaveDrvCfg&           dcfgEdit
            , const TZWUnitInfo&            unitiTar
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
        tCIDLib::TVoid AddAssoc();

        tCIDLib::TBoolean bCheckAwake() const;

        tCIDLib::TBoolean bGetEndPoint
        (
                    tCIDLib::TCard1&        c1ToFill
        )   const;

        tCIDLib::TBoolean bGetTarGroup
        (
                    tCIDLib::TCard1&        c1ToFill
        )   const;

        tCIDLib::TBoolean bGetToAdd
        (
                    tCIDLib::TCard1&        c1ToFill
        )   const;

        tCIDLib::TVoid DeleteAssoc();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid QueryAssocs();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pdcfgEdit
        //      We need this to load up the available units to to the target unit's
        //      groups.
        //
        //  m_punitiTar
        //      A pointer to the unit we are managing associations for. We don't need to
        //      modify it.
        //
        //  m_pwndXXX
        //      Some controls we need to interact with, so we get typed pointers
        //
        //  m_pwndDriver
        //      We get a pointer to the driver so that we can send msgs. It's also
        //      our owner.
        //
        //  m_strTitle
        //      The original title text. We put the unit name into the title text, so
        //      we don't want to let popups pick that up automatically.
        // -------------------------------------------------------------------
        const TZWaveDrvCfg* m_pdcfgEdit;
        const TZWUnitInfo*  m_punitiTar;
        TPushButton*        m_pwndAddAssoc;
        TMultiColListBox*   m_pwndAssocList;
        TPushButton*        m_pwndClose;
        TPushButton*        m_pwndDelAssoc;
        TZWaveUSB3CWnd*     m_pwndDriver;
        TComboBox*          m_pwndEndPnt;
        TComboBox*          m_pwndGroupId;
        TCheckBox*          m_pwndIsAwake;
        TPushButton*        m_pwndQueryGrp;
        TComboBox*          m_pwndToAdd;
        TString             m_strTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3AssocDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



