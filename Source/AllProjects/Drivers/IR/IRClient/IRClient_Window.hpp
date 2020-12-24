//
// FILE NAME: IRClient_Window.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This file implements the main window for the IRClient client driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TIRClientWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TIRClientWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIRClientWnd() = delete;

        TIRClientWnd
        (
            const   TCQCDriverObjCfg&       cqcdcInit
            , const TCQCUserCtx&            cuctxToUse
        );

        TIRClientWnd(const TIRClientWnd&) = delete;
        TIRClientWnd(TIRClientWnd&&) = delete;

        ~TIRClientWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIRClientWnd& operator=(const TIRClientWnd&) = delete;
        TIRClientWnd& operator=(TIRClientWnd&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChanges() const override;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bTrainingMode() const;

        tCQCIR::EIRDevCaps eDevCaps() const;

        const TString& strDevType() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bDoPoll
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TBoolean bGetConnectData
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TVoid Connected() override;

        tCIDLib::TVoid DoCleanup() override;

        tCIDLib::TVoid LostConnection() override;

        tCIDLib::TVoid UpdateDisplayData() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bTrainingState
        //      This is where we store the current state of the server
        //      driver's training mode field value.
        //
        //  m_c4BlasterTabId
        //  m_c4RecTabId
        //      The ids of the two tabs we add to the tabbed window. We
        //      set these as we install the tabs, and need them so that
        //      we can tell which one is active later.
        //
        //  m_c4TrainingStateSer
        //      The serial number for the training state field, which is
        //      the only field we poll. So we don't bother using the more
        //      complex bulk polling methods, and just do a single field
        //      poll, by name.
        //
        //  m_eDevCaps
        //      We get this from the manifest which tells us the capabilities
        //      of the server side driver.
        //
        //  m_pwndXXX
        //      Some typed pointers to our widgets. We don't own them, our
        //      parent class does, but we want to have some typed pointers
        //      so that we avoid lots of downcasting. They are set up during
        //      startup.
        //
        //  m_strDevDescr
        //  m_strDevType
        //      We get this info from the manifest. It's the device type,
        //      which is a key for loading/storing models on the server, and/
        //      a human readable name for display.
        //
        //  m_strFirmware
        //      We get this during the 'get connect data' callback, and need
        //      to save it until we can get it displayed, which happens
        //      later during the Connected() callback.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bTrainingState;
        tCIDLib::TCard4     m_c4BlasterTabId;
        tCIDLib::TCard4     m_c4RecTabId;
        tCIDLib::TCard4     m_c4TrainingStateSer;
        tCQCIR::EIRDevCaps  m_eDevCaps;
        TIRBlasterTab*      m_pwndBlasterTab;
        TIRReceiverTab*     m_pwndRecTab;
        TTabbedWnd*         m_pwndTabbed;
        TString             m_strDevDescr;
        TString             m_strDevType;
        TString             m_strFirmware;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRClientWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK


