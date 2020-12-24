//
// FILE NAME: ZWaveUSB3C_UnitInfoDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/19/2018
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
//  This is the header for a small dialog that displays the information from a
//  unit info object. It also allows the user to save the info to a text file that
//  they can send to use to help use add support.
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
//   CLASS: TZWUSB3UnitInfoDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWUSB3UnitInfoDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWUSB3UnitInfoDlg();

        TZWUSB3UnitInfoDlg(const TZWUSB3UnitInfoDlg&) = delete;

        ~TZWUSB3UnitInfoDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUSB3UnitInfoDlg& operator=(const TZWUSB3UnitInfoDlg&) = delete;


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

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eQueryThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid FormatText
        (
                    TTextOutStream&         strmFmt
        );

        tCIDLib::TVoid QueryInfo();

        tCIDLib::TVoid SaveInfo();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bError
        //      Our timer watches this. It indicates an error occurred and m_strErrMsg
        //      is set. This means the thread has stopped
        //
        //  m_bGotInfo
        //      It's a two stage process, so we need to know which to do when the
        //      user hits either of the buttons. Starts off false. The timer watches
        //      this to be set to indicate success.
        //
        //  m_c4FanModes
        //  m_c4ThermoModes
        //      If the thermostat/fan mode CCs are supported then the supported modes
        //      are gotten, returned as a bitmask, so bit 0 (low bit) is the first mode,
        //      bit 1 is the next mode, and so forth.
        //
        //  m_c4SetPoints
        //      If thermo set point CC is supported, then this is a mask of the supported
        //      setpoints, similar to the thermo modes above.
        //
        //  m_c8ManIds
        //      If the bgn thread is successful it will leave the man ids here.
        //
        //  m_colCCs
        //      If the bgn thread is successful it will leave the support classes
        //      list here.
        //
        //  m_colEPs
        //      If the unit supports multi-channel, this is the information about the
        //      end points.
        //
        //  m_deviUnit
        //      We get a device info object back from the server if the query
        //      process goes well.
        //
        //  m_punitiSrc
        //      A pointer to the unit we were told to get info for.
        //
        //  m_pwndDriver
        //      We get a pointer to the driver window so that we can send msgs.
        //
        //  m_pwndXXX
        //      Some controls we need to interact with, so we get typed pointers
        //
        //  m_strErrMsg
        //      If the thread gets back an error from the server, it will set
        //      m_bError and place the error msg here for us to display.
        //
        //  m_strTitle
        //      The original title text. We put the unit name into the title text, so
        //      we don't want to let popups pick that up automatically.
        //
        //  m_tmidMonitor
        //      A timer to monitor the thread and know when he completes, so that
        //      we can re-enable the buttons and display the gotten info (or an error
        //      if we failed.)
        //
        //  m_thrQuery
        //      There's a lot going on to get all the info, so we use a background
        //      thread.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bError;
        tCIDLib::TBoolean           m_bGotInfo;
        tCIDLib::TCard4             m_c4FanModes;
        tCIDLib::TCard4             m_c4SetPoints;
        tCIDLib::TCard4             m_c4ThermoModes;
        tCIDLib::TCard8             m_c8ManIds;
        tZWaveUSB3Sh::TCInfoList    m_colCCs;
        tZWaveUSB3Sh::TEPInfoList   m_colEPs;
        const TZWUnitInfo*          m_punitiSrc;
        TPushButton*                m_pwndClose;
        TZWaveUSB3CWnd*             m_pwndDriver;
        TPushButton*                m_pwndDoIt;
        TStaticText*                m_pwndProcessing;
        TMultiEdit*                 m_pwndText;
        TString                     m_strErrMsg;
        TString                     m_strTitle;
        tCIDCtrls::TTimerId         m_tmidMonitor;
        TThread                     m_thrQuery;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3UnitInfoDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



