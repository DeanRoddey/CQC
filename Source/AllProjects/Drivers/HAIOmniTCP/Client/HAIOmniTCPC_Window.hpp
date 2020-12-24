//
// FILE NAME: HAIOmniTCPC_Window_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/24/2008
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
//  This file implements the main window for the HAI TCP/IP Omni client driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: THAIOmniTCPCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class THAIOmniTCPCWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniTCPCWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~THAIOmniTCPCWnd();


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
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetConnectData
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TBoolean bCreated() override;

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
        //  Private class types
        // -------------------------------------------------------------------
        enum EChangeTypes
        {
            EChType_None
            , EChType_Modified
            , EChType_Added
            , EChType_Removed
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCompareItems
        (
            const   THAIOmniItem* const     pitem1
            , const THAIOmniItem* const     pitem2
            ,       TString&                strText
            ,       tCIDLib::TBoolean&      bUsed
            ,       EChangeTypes&           eChType
            , const tHAIOmniTCPSh::EItemTypes eType
            , const tCIDLib::TCard4         c4ItemNum
        )   const;

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadConfig
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
        );

        tCIDLib::TVoid UpdateCfgItem
        (
                    TMultiColListBox&       wndList
            , const tCIDLib::TCard4         c4ListIndex
            , const tCIDLib::TCard4         c4UnitIndex
            ,       THAIOmniItem* const     pitemOrg
            ,       THAIOmniItem* const     pitemNew
            , const tHAIOmniTCPSh::EItemTypes eType
        );

        tCIDLib::TVoid UpdateCfgLists();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_dcfgCurrent
        //  m_dcfgOrg
        //      The current and original configuration data. We make changes
        //      to the current. If we need to revert, we set it back to the
        //      original. Once we successfully save, we copy it over to the
        //      original to make it the new original.
        //
        //  m_fcolModXXX
        //      We have a set of flag arrays for each of the lists of
        //      configurable items. These are for remembering modifications.
        //      The extent to which these differ from the stuff in the driver
        //      config data, m_dcfgCurrent, is the stuff that the user has
        //      changed. This is used to keep our interface up to date in
        //      showing the changed items that need to be sent.
        //
        //  m_pwndXXX
        //      We have to interact with some child widgets often enough that
        //      we want to get typed pointers to them.
        //
        //  m_strCfg_xxx
        //      The values we put into the first column
        // -------------------------------------------------------------------
        TOmniTCPDrvCfg      m_dcfgCurrent;
        TOmniTCPDrvCfg      m_dcfgOrg;
        TPushButton*        m_pwndAllTrigs;
        TMultiColListBox*   m_pwndAreas;
        TPushButton*        m_pwndClear;
        TPushButton*        m_pwndSend;
        TMultiColListBox*   m_pwndThermos;
        TMultiColListBox*   m_pwndUnits;
        TMultiColListBox*   m_pwndZones;
        const TString       m_strCfg_Added;
        const TString       m_strCfg_Cfg;
        const TString       m_strCfg_Modified;
        const TString       m_strCfg_Removed;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniTCPCWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

