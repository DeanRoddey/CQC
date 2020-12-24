//
// FILE NAME: ElkM1C_Window_.hpp
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
//  This file implements the main window for the Elk M1 client driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TElkM1CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TElkM1CWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TElkM1CWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TElkM1CWnd();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChanges() const override;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        ) override;


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
        tCIDLib::TVoid ClearAllFlags();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid FormatPLC
        (
            const   tCIDLib::TCard4         c4Index
            ,       TString&                strToFill
        );

        tCIDLib::TVoid LoadConfig
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
        );

        tCIDLib::TVoid ParseFlagList
        (
                    tElkM1C::TFlagArray&    fcolToFill
            ,       TString&                strValues
            , const TString&                strExpectedPrefix
        );

        tCIDLib::TVoid ParsePLCList
        (
                    tElkM1C::TPLCArray&     fcolToFill
            ,       TString&                strValues
            , const TString&                strExpectedPrefix
        );

        tCIDLib::TVoid ParseVoltages
        (
                    tElkM1C::TVoltArray&    fcolToFill
            ,       TTextInStream&          strmIn
        );

        tCIDLib::TVoid SendChanges();

        tCIDLib::TVoid SendChangesFmtFlags
        (
            const   TString&                strPrefix
            , const tElkM1C::TFlagArray&    fcolCfg
            ,       TTextOutStream&         strmTar
        );

        tCIDLib::TVoid UpdateCfgLists();

        tCIDLib::TVoid UpdateFlagCfgList
        (
                    TMultiColListBox&       wndTar
            , const tElkM1C::TFlagArray&    fcolCfg
            , const tElkM1C::TFlagArray&    fcolMod
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bmpXXX
        //      The bitmaps we used to mark our list items.
        //
        //  m_colXXX
        //  m_fcolXXX
        //      We have a set of flag arrays for each of the lists of optional
        //      things. These get filled in during the bGetConnectData()
        //      phase. Then they are used to fill in the list windows during
        //      the Connected() phase.
        //
        //      The Mod versions are changed as the user double clicks on items
        //      to modify them. The differences between these and the first
        //      set indicates configuration changes that need to be saved.
        //
        //  n_pwndXXX
        //      We have to interact with some child widgets often enough that
        //      we want to get typed pointers to them.
        // -------------------------------------------------------------------
        tElkM1C::TVoltArray m_colModVoltages;
        tElkM1C::TVoltArray m_colVoltages;
        tElkM1C::TFlagArray m_fcolOutputs;
        tElkM1C::TPLCArray  m_fcolPLCUnits;
        tElkM1C::TFlagArray m_fcolThermostats;
        tElkM1C::TFlagArray m_fcolThermoCpls;
        tElkM1C::TFlagArray m_fcolZones;
        tElkM1C::TFlagArray m_fcolModOutputs;
        tElkM1C::TPLCArray  m_fcolModPLCUnits;
        tElkM1C::TFlagArray m_fcolModThermostats;
        tElkM1C::TFlagArray m_fcolModThermoCpls;
        tElkM1C::TFlagArray m_fcolModZones;
        TPushButton*        m_pwndClearButton;
        TMultiColListBox*   m_pwndOutputs;
        TMultiColListBox*   m_pwndPLCUnits;
        TPushButton*        m_pwndSendButton;
        TMultiColListBox*   m_pwndThermostats;
        TMultiColListBox*   m_pwndThermoCpls;
        TStaticText*        m_pwndVoltEQ;
        TStaticText*        m_pwndVoltLims;
        TMultiColListBox*   m_pwndVoltages;
        TMultiColListBox*   m_pwndZones;
        const TString       m_strState_Added;
        const TString       m_strState_Cfg;
        const TString       m_strState_Removed;
        const TString       m_strState_Unused;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TElkM1CWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

