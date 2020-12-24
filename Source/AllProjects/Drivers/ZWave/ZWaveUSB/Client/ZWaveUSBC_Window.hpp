//
// FILE NAME: ZWaveUSBC_Window_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/26/2005
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
//  This file implements the main window for the ZWave client driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWaveUSBCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TZWaveUSBCWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveUSBCWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TZWaveUSBCWnd();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChanges() const override;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        //
        //  These methods are called by our parent class and implementing a
        //  driver is basically responding to these calls.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bGetConnectData
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TBoolean bDoPoll
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TVoid Connected() override;

        tCIDLib::TVoid DoCleanup() override;

        tCIDLib::TVoid LostConnection() override;

        tCIDLib::TVoid UpdateDisplayData() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDrvMode
        (
                    tZWaveUSBSh::ERepRes&   eToFill
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditUnit
        (
            const   tCIDLib::TCard4         c4ListInd
        );

        tCIDLib::TVoid LoadConfig();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_dcfgEdit
        //      The copy of the configuration that we make changes to.
        //
        //  m_dcfgLast
        //      The last config data we got from the server side driver, which we can
        //      compare to m_dcfgEdit to see if changes have been made.
        //
        //  m_dplrClient
        //      A driver poller object, so that we can poll the unit values
        //      for display. We also get information via a back-door ORB
        //      interface to the server driver, to get group/unit infromation
        //      and overall status info.
        //
        //  n_pwndXXX
        //      We have to interact with some child widgets often enough that
        //      we want to get typed pointers to them.
        // -------------------------------------------------------------------
        TZWaveDrvConfig         m_dcfgEdit;
        TZWaveDrvConfig         m_dcfgLast;
        TCQCDrvPoller           m_dplrClient;
        TPushButton*            m_pwndLearn;
        TPushButton*            m_pwndReset;
        TPushButton*            m_pwndUnitEdit;
        TMultiColListBox*       m_pwndUnits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUSBCWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

