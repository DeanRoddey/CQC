//
// FILE NAME: IRClient_IRTrainingDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/02/2003
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
//  This is the header for the IRClient_RecTrainingDlg.cpp file, which
//  implements the TRecTrainingDlg class. This class does the work of
//  prompting the user to press the remote a few times, while it watches.
//  Once its seen the same IR event key a few times, its sure that its got
//  the right info and returns with a success status. It will wait until
//  the user either gets us a good event or presses the cancel button.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TRecTrainingDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TRecTrainingDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRecTrainingDlg();

        TRecTrainingDlg(const TRecTrainingDlg&) = delete;

        ~TRecTrainingDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRecTrainingDlg& operator=(const TRecTrainingDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TString&                strToFill
            , const TString&                strRetrainKey
            ,       TIRClientWnd* const     pwndDriver
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        );


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
        //  m_c4Counter
        //      We keep up with how many identical keys we've seen in a row.
        //      We have to see a few in a row before we assume we've got the
        //      right value.
        //
        //  m_pwndXXXX
        //      Since we have to interact with these controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them a convenient way.
        //
        //  m_pwndDriver
        //      We need a pointer to the driver wnd so that we can lock it
        //      while using the client proxy, and it tells us what device
        //      type we are dealing with.
        //
        //  m_strEvent
        //  m_strEvent2
        //      We use these to store the results we get back from the server.
        //      The number 2 event is used to store the previous ones. We
        //      watch for a few button presses to be sure that we've got the
        //      right event.
        //
        //  m_strRetrainKey
        //      If we are being called to retrain, they can pass us the
        //      previous key that the target item had, so that we can supress
        //      the warning about the key already being in use.
        //
        //  m_tmidUpdate
        //      The id of the time we create to watch for data.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bBlinkState;
        TBitmap                 m_bmpOff;
        TBitmap                 m_bmpOn;
        tCIDLib::TCard4         m_c4BlinkCounter;
        tCIDLib::TCard4         m_c4Counter;
        TStaticImg*             m_pwndBlinker;
        TPushButton*            m_pwndCancel;
        TIRClientWnd*           m_pwndDriver;
        TString                 m_strEvent;
        TString                 m_strEvent2;
        TString                 m_strRetrainKey;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TRecTrainingDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



