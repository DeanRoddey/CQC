//
// FILE NAME: CQCInst_CheckMSPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/02/2004
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
//  This is the header for the info panel derived class that handles doing
//  a test for the availability of the master server if they aren't installing
//  it here.
//
//  If we contact it, we also get some info that the installer thread will need,
//  which the facility class will get from us and pass on to him. It's some stuff
//  that needs to go into the registry on the client machines.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstCheckMSPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstCheckMSPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstCheckMSPanel() = delete;

        TInstCheckMSPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstCheckMSPanel(const TInstCheckMSPanel&) = delete;
        TInstCheckMSPanel(TInstCheckMSPanel&&) = delete;

        ~TInstCheckMSPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInstCheckMSPanel& operator=(const TInstCheckMSPanel&) = delete;
        TInstCheckMSPanel& operator=(TInstCheckMSPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPanelIsVisible() const final;

        tCIDLib::TBoolean bValidate
        (
                    tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid ReactToChanges() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bTestDNSAddr
        (
                    TMainFrameWnd&          wndParent
            , const TString&                strAddr
            , const TCQCInstallInfo&        infoCurVals
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bPassed
        //      Allows us to remember that it's passed, and we remember the
        //      address that we tested in m_strLast as well, so that we
        //      can clear this flag if they change it, and make them do it
        //      again.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strLastTest
        //      Stores the last address that we successfully tested, so that
        //      we don't make them re-test it each time they come through
        //      this panel.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bPassed;
        TBitmap             m_bmpBad;
        TBitmap             m_bmpGood;
        TStaticImg*         m_pwndImage;
        TStaticText*        m_pwndStatus;
        TPushButton*        m_pwndTest;
        TString             m_strLastTest;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstCheckMSPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


