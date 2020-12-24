//
// FILE NAME: ZWaveLeviC_Window_.hpp
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
//  This file implements the main window for the Leviton Z-Wave client driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWaveLeviCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TZWaveLeviCWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveLeviCWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TZWaveLeviCWnd();


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
        tCIDLib::TVoid CheckForChanges();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eSpinHandler
        (
                    TSpinChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditUnit
        (
                    TZWaveUnit&             unitTar
        );

        tCIDLib::TVoid LoadConfig
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TZWaveDrvConfig&        dcfgNew
            , const tCIDLib::TCard4         c4SerialNum
        );

        tCIDLib::TVoid QueryConfig
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            ,       TZWaveDrvConfig&        dcfgToFill
            ,       tCIDLib::TCard4&        c4NewSerialNum
        );

        tCIDLib::TVoid SetPollInt
        (
            const   TZWaveUnit&             unitSrc
        );

        tCIDLib::TVoid SetUnitSelState
        (
            const   tCIDLib::TBoolean       bState
        );

        tCIDLib::TVoid ShowUnit
        (
            const   TString&                strName
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNewConfig
        //      If the poll thread sees new data, it will store that info in
        //      m_dcfgNew and set this flag. The UpdateDisplayData() method,
        //      which runs in the GUI thread, will update our display data
        //      as soon as it can.
        //
        //  m_c4LastSerialNum
        //      The last configuration serial number we got from the server.
        //
        //  m_c4LastSerialNum
        //      When new config is downloaded by the poll thread, it is stored
        //      in m_dcfgNew, and the new serial number is stored here so that
        //      we can store it when we load the new config.
        //
        //  m_dcfgCur
        //      The current working configuration, which may include local
        //      edits not saved yet (in which case it will be different from
        //      m_dcfgPrev.)
        //
        //  m_dcfgNew
        //      When the poll thread sees that new data is available, it will
        //      store that data here and set the m_bNewData flag.
        //
        //  m_dcfgPrev
        //      In order to know when there are changes, we keep a previously
        //      saved (or initially loaded) copy of the configuration. This
        //      way we can always compare the current one to the last one
        //      saved.
        //
        //  n_pwndXXX
        //      We have to interact with some child widgets often enough that
        //      we want to get typed pointers to them.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bNewConfig;
        tCIDLib::TCard4     m_c4LastSerialNum;
        tCIDLib::TCard4     m_c4NewSerialNum;
        TZWaveDrvConfig     m_dcfgCur;
        TZWaveDrvConfig     m_dcfgNew;
        TZWaveDrvConfig     m_dcfgPrev;
        TPushButton*        m_pwndAddAssoc;
        TCheckBox*          m_pwndAsyncNot;
        TCheckBox*          m_pwndBattPwr;
        TPushButton*        m_pwndDelete;
        TCheckBox*          m_pwndDisabled;
        TPushButton*        m_pwndEdGroups;
        TSeparatorWnd*      m_pwndInfoGrp;
        TComboBox*          m_pwndOptInfo;
        TEnumSpinBox*       m_pwndPollInt;
        TCheckBox*          m_pwndReadable;
        TPushButton*        m_pwndRename;
        TPushButton*        m_pwndRescan;
        TCheckBox*          m_pwndSecure;
        TPushButton*        m_pwndSetCfgParm;
        TColoredList*       m_pwndUnits;
        TStaticText*        m_pwndUnitId;
        TCheckBox*          m_pwndWriteable;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveLeviCWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

