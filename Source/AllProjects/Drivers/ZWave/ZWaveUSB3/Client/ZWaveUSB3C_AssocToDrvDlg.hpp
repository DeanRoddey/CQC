//
// FILE NAME: ZWaveUSB3C_AssocToDrvDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/20/2018
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
//  This is the header for a small dialog that lets the user associate a unit with
//  the driver's Z-Stick controller in the standard (group 1) way.
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
//   CLASS: TZWUSB3AssocToDrvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWUSB3AssocToDrvDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWUSB3AssocToDrvDlg();

        TZWUSB3AssocToDrvDlg(const TZWUSB3AssocToDrvDlg&) = delete;

        ~TZWUSB3AssocToDrvDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUSB3AssocToDrvDlg& operator=(const TZWUSB3AssocToDrvDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
                    TZWaveUSB3CWnd&         wndDriver
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
        tCIDLib::TBoolean bCheckAwake() const;

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_punitiTar
        //      A pointer to the unit we are auto-configuring. We don't need to
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
        //
        //  m_punitiTar
        //      The target unit that we are setting the association on.
        // -------------------------------------------------------------------
        const TZWUnitInfo*  m_punitiTar;
        TPushButton*        m_pwndCancel;
        TZWaveUSB3CWnd*     m_pwndDriver;
        TCheckBox*          m_pwndIsAwake;
        TPushButton*        m_pwndSet;
        TString             m_strTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3AssocToDrvDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



