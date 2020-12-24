//
// FILE NAME: ZWaveUSB3C_AutoCfgDlg.hpp
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
//  This is the header for a small dialog that lets the user auto-configure a unit.
//  We get the unit, which has the unit name, which is all we need to do the
//  command. We also show the commands that are going to be done. They also can tell
//  us, if it's a non-listener, if the unit is known to be awake.
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
//   CLASS: TZWUSB3AutoCfgDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWUSB3AutoCfgDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWUSB3AutoCfgDlg();

        TZWUSB3AutoCfgDlg(const TZWUSB3AutoCfgDlg&) = delete;

        ~TZWUSB3AutoCfgDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUSB3AutoCfgDlg& operator=(const TZWUSB3AutoCfgDlg&) = delete;


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
        // -------------------------------------------------------------------
        const TZWUnitInfo*  m_punitiTar;
        TPushButton*        m_pwndClose;
        TMultiEdit*         m_pwndCmds;
        TZWaveUSB3CWnd*     m_pwndDriver;
        TCheckBox*          m_pwndIsAwake;
        TPushButton*        m_pwndStart;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3AutoCfgDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



