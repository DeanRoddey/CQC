//
// FILE NAME: ZWaveUSB3C_Window_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/05/2018
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
//  This file implements the main window for the ZWave USB3 client driver. Our basic
//  job is to get the current configuration from the server side driver and allow the
//  user to set configurable options, then save it back to the server.
//
//  We cannot add/delete units here. That is done via the Z-Wave master controller.
//  We just get the available units from the master. The server side driver tries to
//  auto-identify units as much as he can. Otherwise, the user has to tell us what they
//  are. Once we know or are told, then we know any configurable options. The name of
//  the unit is always configurable.
//
//  Since our server side driver is C++, we have a shared facility so we don't have to
//  define any classes here to represent units and such.
//
//  This is a considerably more complex client driver than most. In most cases we assume
//  that the server side driver config will not change unless we change it. Of course
//  multiple users could edit at the same time, but that's not our fault. In this case
//  the config could change at any time as battery powered devices wake up and we gather
//  more information about them.
//
//  So our polling thread watches for changes in driver status and configuration. That
//  means we need to sync access to some stuff that both of them access, since the polling
//  thread leaves info for the GUI thread to make use of.
//
//  Editable Units
//
//  A unit is only editable if it is at GetInitVals state or beyond. This is VERY
//  important. See the class header comments of TZWUnitInfo, in the gotchas section.
//  Once it gets to that point, the server side driver cannot change anything
//  except to go up to Ready. Or if it is also changing the state back down below
//  GetInitVals again (because it failed or because a replication or startup just
//  occurred and it is making the units re-scan their unit hardware again.) During
//  that time we won't be able to edit them, so that is fine.
//
//  If we see the state change back below GetInitVals for the selected unit, we will
//  clear the attribute editor until such time as it recovers back to an editable
//  state. If the user is currently editing it, the eventual value will be rejected
//  when we see that the unit serial number has changed since when the edit started.
//
//  The only legal change (by the server side driver) for a unit that is at GetInitVals
//  is to move up to Ready state. The only legal change for a unit that is at Ready
//  state is to fall in some way back below GetInitVals. So we will do some debug
//  checking to make sure the unit has not changed in an invalid way when we see
//  changes.
//
//  This is the only sane way to deal with both sides updating the config.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TZW3UnitAttrWnd;


// ---------------------------------------------------------------------------
//   CLASS: TZWaveUSB3CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TZWaveUSB3CWnd : public TCQCDriverClient, public MIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveUSB3CWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TZWaveUSB3CWnd();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const override;

        tCIDLib::TBoolean bChanges() const override;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        )   override;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanEdit() const;

        tCIDLib::TBoolean bSendSrvMsg
        (
            const   TWindow&                wndOwner
            ,       TZWDriverXData&         zwdxdComm
            ,       THeapBuf&               mbufTmp
        );


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
        tCIDLib::TVoid ApproveNewUnits();

        tCIDLib::TBoolean bSendGUICmd
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        );

        tCIDLib::TBoolean bSendPollCmd
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        );

        tCIDLib::TCard4 c4LoadUnitToList
        (
            const   TZWUnitInfo&            unitiCur
            ,       tCIDLib::TStrList&      colCols
        );

        tCIDLib::TVoid DisableUnit
        (
                    TZWUnitInfo&            unitiTar
            , const tCIDLib::TCard4         c4ListInd
        );

        tCIDLib::TVoid DoIncEx();

        tCIDLib::TVoid DoUnitOpt
        (
            const   tCIDLib::TCard1         c1Id
            , const tCIDLib::TCard4         c4ListInd
        );

        tCIDCtrls::EEvResponses eAttrEditHandler
        (
                    TAttrEditInfo&          wnotInfo
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid InitStateData();

        tCIDLib::TVoid LoadUnitAttrs
        (
            const   tCIDLib::TCard1         c1UnitId
        );

        tCIDLib::TVoid LoadUnits();

        tCIDLib::TVoid SaveCfgToFile();

        tCIDLib::TVoid SetUnitType
        (
                    TZWUnitInfo&            unitiTar
            , const tCIDLib::TCard4         c4ListInd
        );

        tCIDLib::TVoid SetTraceOption
        (
            const   tCIDLib::TCard4         c4SelId
        );

        tCIDLib::TVoid UpdateData
        (
            const   tCIDLib::TBoolean       bInitial
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDelayNewCfg
        //      Any time the replication dialog is up, or any of the dialogs that get
        //      a reference to the edit config, we need to prevent newly gotten config
        //      changes from being written to it. So this flag is set. It'll just be
        //      left in m_dcfgNew until it can be stored.
        //
        //  m_bGotConfig
        //      We save our config upon being asked to close. But, if we never managed
        //      to get config, we'd just end up saving out bogus, empty configuration.
        //      So we set this the first time we manage to download config, and won't
        //      save anything until we get some. We also don't report any changes
        //      available until after that as well.
        //
        //  m_bInNetworkCur
        //  m_bInNetworkNew
        //      The current 'in Z-Wave network' status we think we are in, and the newest
        //      one that the polling thread downloaded. If they are different, then our
        //      in network status has changed.
        //
        //  m_c4OurSerNumCur
        //  m_c4OurSerNumNew
        //      The configuration serial number that we stored when we downloaded
        //      m_dcfgOrg, and the newest one that the polling thread downloaded.
        //      If they are different, then the server has new configuration changes.
        //
        //  m_dcfgEdit
        //      When we download m_dcfgOrg, we make a copy here and we use this for
        //      editing. If this is different from org, then we have local changes.
        //
        //  m_dcfgNew
        //      If the poll thread sees that the configuration has changed, then it
        //      will pull down new data and store it here. The GUI thread can take
        //      this up when it is ready and update the current serial number to
        //      indicate it has taken the info.
        //
        //  m_dcfgOrg
        //      The last configuration we got. When we get it, we get it to generate a
        //      config serial number, which we store in m_c4LastCfgSerNum.
        //
        //  m_dcfgTmp
        //      For the poll thread to use for downloading new config.
        //
        //  m_eDrvStateCur
        //  m_eDrvStateNew
        //      The current driver state that we are working with, and the latest one
        //      stored by the poll thread. If they are different, then our driver state
        //      changed.
        //
        //  m_eTraceLevelCur
        //  m_eTraceLevelNew
        //      The current trace level we have marked in the menu and the latest one
        //      stored by the poll thread. If they are different we have to update our
        //      popup menu.
        //
        //  m_mbufGUI
        //  m_mbufPoll
        //      Temp buffer to use when sending commands to the driver. We need one
        //      for the GUI thread nad one for the poll thread.
        //
        //  m_menuOpts
        //      We pre-create the options popup menu and keep it around.
        //
        //  m_pwndXXX
        //      We have to interact with some child widgets often enough that we want
        //      to get typed pointers to them.
        //
        //      The attribute editor is a derivative of ours so that we can intercept
        //      some stuff. So we destroy the original and replace it with our own
        //      in the same place. The implementation is simple and internal.
        //
        //  m_tmidAttrDelay
        //      To load the attribute editor, we have to query attribute info from the
        //      server driver. So we can't afford to just do it every time the list
        //      selection changes. If they held down the arrow key and had a lot of
        //      units, we could beat the server side driver to death. So we use this
        //      to delay the process. When the selection changes we start this. If it
        //      goes off and there's a selected item, we load that one. If we get another
        //      selection before it does, we just restart it.
        //
        //  m_zwdxdGUI
        //  m_zwdxdPoll
        //      Data exchange objects we use to talk to the server. We need separate
        //      ones for the poll thread and GUI thread.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bDelayNewCfg;
        tCIDLib::TBoolean           m_bGotConfig;
        tCIDLib::TBoolean           m_bInNetworkCur;
        tCIDLib::TBoolean           m_bInNetworkNew;
        tCIDLib::TCard4             m_c4CfgSerNumCur;
        tCIDLib::TCard4             m_c4CfgSerNumNew;
        TZWaveDrvCfg                m_dcfgEdit;
        TZWaveDrvCfg                m_dcfgNew;
        TZWaveDrvCfg                m_dcfgOrg;
        TZWaveDrvCfg                m_dcfgTmp;
        tZWaveUSB3Sh::EDrvStates    m_eDrvStateCur;
        tZWaveUSB3Sh::EDrvStates    m_eDrvStateNew;
        tCQCKit::EVerboseLvls       m_eTraceLevelCur;
        tCQCKit::EVerboseLvls       m_eTraceLevelNew;
        THeapBuf                    m_mbufGUI;
        THeapBuf                    m_mbufPoll;
        TPopupMenu                  m_menuOpts;
        TZW3UnitAttrWnd*            m_pwndAttrs;
        TStaticText*                m_pwndDrvState;
        TStaticText*                m_pwndInNetwork;
        TMultiColListBox*           m_pwndList;
        TPushButton*                m_pwndOptions;
        TStaticMultiText*           m_pwndUnitInst;
        TZWDriverXData              m_zwdxdGUI;
        TZWDriverXData              m_zwdxdPoll;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUSB3CWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

