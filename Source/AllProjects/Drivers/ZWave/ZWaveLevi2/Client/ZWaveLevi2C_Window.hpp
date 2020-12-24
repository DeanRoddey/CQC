//
// FILE NAME: ZWaveLevi2C_Window_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2014
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
//  This file implements the main window for the V2 Leviton Z-Wave client driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TUnitAttrWnd;

// ---------------------------------------------------------------------------
//   CLASS: TZWaveLevi2CWnd
//  PREFIX: wnd
//// ---------------------------------------------------------------------------
class TZWaveLevi2CWnd : public TCQCDriverClient, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveLevi2CWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TZWaveLevi2CWnd();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPETestCanEdit
        (
            const   tCIDCtrls::TWndId       widSrc
            , const tCIDLib::TCard4         c4ColInd
            ,       TAttrData&              adatFill
            ,       tCIDLib::TBoolean&      bValueSet
        )   override;

        tCIDLib::TBoolean bChanges() const override;

        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const override;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInRescan() const;

        const TZDIIndex& swdinToUse();

        TZWaveUnit* punitById
        (
            const   tCIDLib::TCard4         c4IdToFind
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
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid DoAutoCfg
        (
            const   tCIDLib::TCard4         c4Id
        );

        tCIDCtrls::EEvResponses eAttrEditHandler
        (
                    TAttrEditInfo&          wnotInfo
        );

        tCIDLib::TVoid LoadConfig
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TZWaveDrvConfig&        dcfgNew
            , const tCIDLib::TCard4         c4SerialNum
        );

        tCIDLib::TVoid LoadIndex();

        tCIDLib::TVoid LoadUnits();

        tCIDLib::TVoid LoadUnitAttributes
        (
            const   TZWaveUnit&             unitShow
        );

        const TZWDIInfo* pzwdiiFindDevInfo
        (
            const   TString&                strMake
            , const TString&                strModel
        )   const;

        tCIDLib::TVoid PrepUnitOpts
        (
            const   TZWaveUnit&             unitCur
            , const tZWaveLevi2Sh::TOptList& colUnitOpts
            ,       tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        );

        tCIDLib::TVoid QueryConfig
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            ,       TZWaveDrvConfig&        dcfgToFill
            ,       tCIDLib::TCard4&        c4NewSerialNum
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
        //  m_c4ColInd_XXX
        //      The ids of our columns.
        //
        //  m_c4LastSerialNum
        //      The last configuration serial number we got from the server.
        //
        //  m_c4LastSerialNum
        //      When new config is downloaded by the poll thread, it is stored
        //      in m_dcfgNew, and the new serial number is stored here so that
        //      we can store it when we load the new config.
        //
        //  m_colUnitOpts
        //      When a new unit is selected, we will query the options for that unit and
        //      keep them around, to avoid requerying them a lot.
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
        //      we want to have typed pointers to them.
        //
        //  m_swdinList
        //      The device info index. It is faulted in the first time the user
        //      asks to select a device type.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bInRescan;
        tCIDLib::TBoolean       m_bNewConfig;
        const tCIDLib::TCard4   m_c4ColInd_BasicType;
        const tCIDLib::TCard4   m_c4ColInd_Enabled;
        const tCIDLib::TCard4   m_c4ColInd_Id;
        const tCIDLib::TCard4   m_c4ColInd_MakeModel;
        const tCIDLib::TCard4   m_c4ColInd_Name;
        const tCIDLib::TCard4   m_c4ColInd_Status;
        tCIDLib::TCard4         m_c4LastSerialNum;
        tCIDLib::TCard4         m_c4NewSerialNum;
        tZWaveLevi2Sh::TOptList m_colUnitOpts;
        TZWaveDrvConfig         m_dcfgCur;
        TZWaveDrvConfig         m_dcfgNew;
        TZWaveDrvConfig         m_dcfgPrev;
        TDummyLevi              m_mzwsfDummy;
        TPushButton*            m_pwndAddAssoc;
        TUnitAttrWnd*           m_pwndAttrEd;
        TPushButton*            m_pwndAutoCfg;
        TPushButton*            m_pwndDelete;
        TPaneWndCont*           m_pwndPanes;
        TPushButton*            m_pwndQAssoc;
        TPushButton*            m_pwndRename;
        TPushButton*            m_pwndRescan;
        TStaticImg*             m_pwndScanLED;
        TStaticText*            m_pwndScanText;
        TPushButton*            m_pwndSetCfgParm;
        TMultiColListBox*       m_pwndUnits;
        TZDIIndex               m_swdinList;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveLevi2CWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK

