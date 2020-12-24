//
// FILE NAME: ZWaveUSB3C_ResetUnitDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/22/2018
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
//  This is the header for a small dialog that gives the user some warning info
//  before letting them reset a unit.
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
//   CLASS: TZWUSB3ResetUnitDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWUSB3ResetUnitDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWUSB3ResetUnitDlg();

        TZWUSB3ResetUnitDlg(const TZWUSB3ResetUnitDlg&) = delete;

        ~TZWUSB3ResetUnitDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUSB3ResetUnitDlg& operator=(const TZWUSB3ResetUnitDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
                    TZWaveUSB3CWnd&         wndDriver
            , const TZWUnitInfo&            unitiSrc
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
        //  m_punitiTar
        //      A pointer to the unit we are to reset.
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
        const TZWUnitInfo*  m_punitiTar;
        TPushButton*        m_pwndAccept;
        TPushButton*        m_pwndCancel;
        TZWaveUSB3CWnd*     m_pwndDriver;
        TCheckBox*          m_pwndIsAwake;
        TString             m_strTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3ResetUnitDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



