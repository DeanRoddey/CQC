//
// FILE NAME: ZWaveUSBC_LearnDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/27/2005
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
//  This is the header for the ZWaveUSBC_LearnDlg.cpp file, which implements
//  the TZWaveLearnDlg class. This class handles the replication process,
//  whereby CQC learns the units and groups defined by the primary Z-Wave
//  controller.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveLearnDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWaveLearnDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveLearnDlg();

        ~TZWaveLearnDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strMoniker
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TZWaveLearnDlg(const TZWaveLearnDlg&);
        tCIDLib::TVoid operator=(const TZWaveLearnDlg&);


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
        //  m_c4BlinkCounter
        //      We use this counter to blink the blinker LED every second,
        //      by counting timer events.
        //
        //  m_ePrevRes
        //      Used to know when we need to change the display status
        //      string.
        //
        //  m_orbcTarget
        //      We get a client proxy for the CQCServer that is hosting our
        //      target driver.
        //
        //  m_pwndXXXX
        //      Since we have to interact with these controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them a convenient way.
        //
        //  m_strMoniker
        //      The moniker of our driver, which the caller provides for us,
        //      so that we can create a client proxy.
        //
        //  m_tmidBlinker
        //      A timer we use during the replication process.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bBlinkState;
        TBitmap                 m_bmpBlinkOff;
        TBitmap                 m_bmpBlinkOn;
        tCIDLib::TCard4         m_c4BlinkCounter;
        tZWaveUSBSh::ERepRes    m_ePrevRes;
        tCQCKit::TCQCSrvProxy   m_orbcTarget;
        TStaticImg*             m_pwndBlinker;
        TPushButton*            m_pwndCancelButton;
        TEntryField*            m_pwndStatus;
        TString                 m_strMoniker;
        tCIDCtrls::TTimerId     m_tmidBlinker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveLearnDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


