//
// FILE NAME: ZWaveUSB3C_InclusionDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/10/2018
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
//  This is the header for a small dialog that handles doing the inclusion process.
//  It's not normal that a client side driver would do a modal dialog across something
//  like this, but it's important in this case. This keeps the user from doing things
//  that would cause problems during the inclusion process. And it allows us to get
//  the final configuration info and pass it back to the driver.
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
//   CLASS: TZWUSB3IncludeDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWUSB3IncludeDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWUSB3IncludeDlg();

        TZWUSB3IncludeDlg(const TZWUSB3IncludeDlg&) = delete;

        ~TZWUSB3IncludeDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUSB3IncludeDlg& operator=(const TZWUSB3IncludeDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
                    TZWaveUSB3CWnd&         wndDriver
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidSrc
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tZWaveUSB3Sh::EDrvStates eGetRepState();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eLastRepState
        //      The last replication state we saw, so that we know if we need to update
        //      our display or react.
        //
        //  m_mbufComm
        //      A memory buffer to use when flattening msgs to send to the server.
        //
        //  m_pwndXXX
        //      Some controls we need to interact with, so we get typed pointers
        //
        //  m_pwndDriver
        //      We get a pointer to the driver so that we can send msgs. It's also
        //      our owner.
        //
        //  m_tmidUpdate
        //      We use a timer to keep our driver info up to date. This is the id of
        //      the timer we create for that.
        //
        //  m_zwdxdComm
        //      A server driver comm object to use.
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::EDrvStates    m_eLastRepState;
        THeapBuf                    m_mbufComm;
        TActivityBar*               m_pwndWait;
        TPushButton*                m_pwndCancel;
        TZWaveUSB3CWnd*             m_pwndDriver;
        TStaticText*                m_pwndInstruct;
        TStaticText*                m_pwndState;
        TPushButton*                m_pwndStart;
        tCIDCtrls::TTimerId         m_tmidUpdate;
        TZWDriverXData              m_zwdxdComm;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3IncludeDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



