//
// FILE NAME: HAIOmniTCP2C_Window_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/09/2014
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
//   CLASS: THAIOmniTCP2CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class THAIOmniTCP2CWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniTCP2CWnd() = delete;

        THAIOmniTCP2CWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        THAIOmniTCP2CWnd(const THAIOmniTCP2CWnd&) = delete;

        ~THAIOmniTCP2CWnd();


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
        enum class EChangeTypes
        {
            None
            , Modified
            , Added
            , Removed
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCompareItems
        (
            const   THAIOmniItem* const     pitem1
            , const THAIOmniItem* const     pitem2
            ,       tCIDLib::TStrList&      colVals
            ,       tCIDLib::TBoolean&      bUsed
            ,       EChangeTypes&           eChType
            , const tHAIOmniTCP2Sh::EItemTypes eType
            , const tCIDLib::TCard2         c2Id
        )   const;

        tCIDLib::TBoolean bUpdateACfgList
        (
                    TMultiColListBox&       wndList
            ,       tCIDLib::TStrList&      colVals
            , const tHAIOmniTCP2Sh::EItemTypes eType
            , const tCIDLib::TCard4         c4Max
        )   const;

        tCIDLib::TBoolean bUpdateCfgItem
        (
                    TMultiColListBox&       wndList
            ,       tCIDLib::TStrList&      colVals
            , const THAIOmniItem* const     pitemOrg
            , const THAIOmniItem* const     pitemNew
            , const tCIDLib::TCard4         c4Index
            , const tCIDLib::TCard2         c2Id
            , const tHAIOmniTCP2Sh::EItemTypes eType
        )   const;

        tCIDLib::TBoolean bUpdateCfgLists
        (
            const   tCIDLib::TBoolean       bForceRedraw
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
        //  n_pwndXXX
        //      We have to interact with some child widgets often enough that
        //      we want to get typed pointers to them.
        //
        //  m_strCfg_XXX
        //      The values we show in the 0th column of the lists, to indicate changes.
        //      If no changes, it's empty.
        // -------------------------------------------------------------------
        TOmniTCPDrvCfg      m_dcfgCurrent;
        TOmniTCPDrvCfg      m_dcfgOrg;
        TPushButton*        m_pwndAllTrigs;
        TMultiColListBox*   m_pwndAreas;
        TPushButton*        m_pwndClear;
        TMultiColListBox*   m_pwndThermos;
        TMultiColListBox*   m_pwndUnits;
        TMultiColListBox*   m_pwndZones;
        const TString       m_strCfg_Added;
        const TString       m_strCfg_Modified;
        const TString       m_strCfg_Removed;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniTCP2CWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

