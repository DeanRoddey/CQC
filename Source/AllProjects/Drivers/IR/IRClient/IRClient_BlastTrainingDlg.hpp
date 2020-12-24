//
// FILE NAME: IRClient_BlastTrainingDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2002
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
//  This is the header for the IRClient_BlastTrainDlg.cpp file, which
//  implements the TBlastTrainingDlg class. This class does the work of
//  prompting the user to press the remote key, while it watches. Once its
//  seen the IR key, it returns with a success status. It will wait until
//  the user either gets us a good event or presses the cancel button.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TIRBlasterTab;

// ---------------------------------------------------------------------------
//   CLASS: TBlastTrainingDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TBlastTrainingDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBlastTrainingDlg();

        TBlastTrainingDlg(const TBlastTrainingDlg&) = delete;

        ~TBlastTrainingDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBlastTrainingDlg& operator=(const TBlastTrainingDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       THeapBuf&               mbufToFill
            ,       tCIDLib::TCard4&        c4Bytes
            ,       TIRClientWnd* const     pwndDriver
            , const TString&                strPrevData
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
        //  Private data types
        // -------------------------------------------------------------------
        enum class EModes
        {
            Train
            , Manual
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TVoid SetMode
        (
            const   EModes                  eToSet
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bBlinkState
        //      Keeps up with the current blink state, with false being black
        //      and true being red.
        //
        //  m_bmpOff
        //  m_bmpOn
        //      The bitmaps we use to create a blinking light. We preload them upon
        //      our startup.
        //
        //  m_c4BlinkCounter
        //      We use this counter to blink the blinker LED every second,
        //      by counting timer events.
        //
        //  m_c4Bytes
        //      This is filled in with the IR data bytes we got, if any.
        //
        //  m_mbufData
        //      This is where we store the IR data until we get into the caller's parms.
        //
        //  m_pwndDriver
        //      We need a pointer to the driver wnd so that we can lock it while using
        //      the client proxy, and it tells us what device type we are dealing with.
        //
        //  m_pwndXXX
        //      Since we have to interact with these controls a lot, we get convenience,
        //      typed, pointers to them. We don't own these.
        //
        //  m_pwndDriver
        //      We get a pointer to the client driver window, so that we can have access
        //      to the driver client proxy.
        //
        //  m_strPrevData
        //      If there was any previous blaster data, the caller will format it to text
        //      and pass it to us to initialize the manual paste window.
        //
        //  m_szManual
        //  m_szTrain
        //      We have to change the size of the windows as we change mode, so we
        //      pre-calculate those sizes.
        //
        //  m_tmidUpdate
        //      We create a timer to poll the server for data.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bBlinkState;
        TBitmap             m_bmpOff;
        TBitmap             m_bmpOn;
        tCIDLib::TCard4     m_c4BlinkCounter;
        tCIDLib::TCard4     m_c4Bytes;
        EModes              m_eMode;
        TPushButton*        m_pwndCancel;
        THeapBuf            m_mbufData;
        TStaticImg*         m_pwndBlinker;
        TPushButton*        m_pwndDoIt;
        TIRClientWnd*       m_pwndDriver;
        TStaticMultiText*   m_pwndInstruct;
        TMultiEdit*         m_pwndIRData;
        TRadioButton*       m_pwndManualMode;
        TPushButton*        m_pwndTest;
        TRadioButton*       m_pwndTrainMode;
        TString             m_strPrevData;
        TSize               m_szManual;
        TSize               m_szTrain;
        tCIDCtrls::TTimerId m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TBlastTrainingDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


