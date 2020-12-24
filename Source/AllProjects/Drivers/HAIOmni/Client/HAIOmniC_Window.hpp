//
// FILE NAME: HAIOmniC_Window_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/12/2006
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
//  This file implements the main window for the HAI Omni client driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: THAIOmniCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class THAIOmniCWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniCWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~THAIOmniCWnd();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        //
        //  These methods are called by our parent class and implementing a
        //  driver is basically responding to these calls.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetConnectData
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        );

        tCIDLib::TBoolean bDoPoll
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        );

        tCIDLib::TVoid Connected();

        tCIDLib::TVoid Created
        (
            const   TArea&                  areaInit
        );

        tCIDLib::TVoid DoCleanup();

        tCIDLib::TVoid UpdateDisplayData();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearAllFlags();

        tCIDWnd::EEventResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDWnd::EEventResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadConfig
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
        );

        tCIDLib::TVoid MakeUnitText
        (
            const   tCIDLib::TCard4         c4Index
            , const tHAIOmniSh::EUnitTypes  eCfg
            ,       TString&                strToFill
        );

        tCIDLib::TVoid MakeZoneText
        (
            const   tCIDLib::TCard4         c4Index
            , const tHAIOmniSh::EZoneTypes  eCfg
            ,       TString&                strToFill
        );

        tCIDLib::TVoid UpdateCfgLists();

        tCIDLib::TVoid UpdateFlagCfgList
        (
                    TListBox&               wndTar
            , const tHAIOmniSh::TFlagArray& fcolCfg
            , const tHAIOmniSh::TFlagArray& fcolMod
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bmpXXX
        //      The bitmaps we used to mark our list items.
        //
        //  m_fcolModXXX
        //      We have a set of flag arrays for each of the lists of
        //      configurable items. These are for remembering modifications.
        //      The extent to which these differ from the stuff in the driver
        //      config data, m_dcfgCurrent, is the stuff that the user has
        //      changed. This is used to keep our interface up to date in
        //      showing the changed items that need to be sent.
        //
        //  n_pwndXXX
        //      We have to interact with some child widgets often enough that
        //      we want to get typed pointers to them.
        // -------------------------------------------------------------------
        TBitmap                 m_bmpAdded;
        TBitmap                 m_bmpBlank;
        TBitmap                 m_bmpCfg;
        TBitmap                 m_bmpRemoved;
        TOmniDrvCfg             m_dcfgCurrent;
        tHAIOmniSh::TFlagArray  m_fcolModAreas;
        tHAIOmniSh::TFlagArray  m_fcolModThermos;
        tHAIOmniSh::TPLCArray   m_fcolModUnits;
        tHAIOmniSh::TZoneArray  m_fcolModZones;
        TListBox*               m_pwndAreas;
        TPushButton*            m_pwndClearButton;
        TPushButton*            m_pwndSendButton;
        TListBox*               m_pwndThermos;
        TListBox*               m_pwndUnits;
        TListBox*               m_pwndZones;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniCWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

