//
// FILE NAME: CQCSrvDrvTI_TIImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2018
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
//  This is our own special implementation of the CQCServer driver admin interface.
//  This is the interface that exposes drivers to clients. It's also what client
//  side drivers use to talk to their associated drivers.
//
//  See the main header comments. This is an implementation of the CQCServer driver
//  admin interface that allows test harnesses and IDEs to wire client and server
//  side drivers directly together within themselves, not the normal scenario where
//  the server side runes in CQCServer and the client in CQCAdmin.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class CQCSRVDRVTIEXPORT TCQCSrvDrvTI : public TCQCSrvAdminServerBase
{
    public :
        // --------------------------------------------------------------------
        //  Constructors and destructor
        // --------------------------------------------------------------------
        TCQCSrvDrvTI();

        TCQCSrvDrvTI
        (
            const   TOrbObjId&              ooidThis
            ,       TCQCServerBase* const   psdrvTar

        );

        TCQCSrvDrvTI(const TCQCSrvDrvTI&) = delete;
        TCQCSrvDrvTI(TCQCSrvDrvTI&&) = delete;

        ~TCQCSrvDrvTI();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TCQCSrvDrvTI& operator=(const TCQCSrvDrvTI&) = delete;
        TCQCSrvDrvTI& operator=(TCQCSrvDrvTI&&) = delete;


        // --------------------------------------------------------------------
        //  Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bCheckFieldExists
        (
            const   TString&                strMoniker
            ,       tCIDLib::TBoolean&      bDevExists
            , const TString&                strField
            ,       tCIDLib::TBoolean&      bFldExists
        )   final;

        tCIDLib::TBoolean bCheckForAllNewFields
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const TFundVector<tCIDLib::TCard8>& fcolIds
        )   final;

        tCIDLib::TBoolean bCheckForNewFields
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
        )   final;

        tCIDLib::TBoolean bCheckForNewDrivers
        (
            const   tCIDLib::TCard4         c4DriverListId
        )   final;

        tCIDLib::TBoolean bHasQueuedTimedWrite
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       tCIDLib::TCard8&        c8EndTime
        )   final;

        tCIDLib::TBoolean bLoadDrivers
        (
            const   tCIDLib::TStrList&      colMonikers
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryCommPorts
        (
                    tCIDLib::TStrList&      colToFill
            , const tCIDLib::TBoolean       bLocalOnly
            , const tCIDLib::TBoolean       bAvailOnly
        )   final;

        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strMoniker
            , const TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufToFill
        )   final;

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strMoniker
            , const TString&                strQueryType
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        )   final;

        tCIDLib::TBoolean bQueryDriverInfo
        (
            const   TString&                strMoniker
            ,       TCQCDriverObjCfg&       cqcdcToFill
        )   final;

        tCIDLib::TBoolean bQueryDriverStates
        (
            const   tCIDLib::TCard4         c4DriverListId
            ,       tCIDLib::TCardList&     fcolIds
            ,       TFundVector<tCQCKit::EDrvStates>& fcolStates
        )   final;

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strMoniker
            , const TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        )   final;

        tCIDLib::TBoolean bQueryVal
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const tCIDLib::TBoolean       bNoQueue
        )   final;

        tCIDLib::TBoolean bQueryVal2
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const TString&                strValName
            , const tCIDLib::TBoolean       bNoQueue
        )   final;

        tCIDLib::TBoolean bReadBoolByName
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TBoolean&      bToFill
        )   final;

        tCIDLib::TBoolean bReadCardByName
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4ToFill
        )   final;

        tCIDLib::TBoolean bReadField
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FieldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TString&                strValue
            ,       tCQCKit::EFldTypes&     eType
        )   final;

        tCIDLib::TBoolean bReadField
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FieldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4RetBytes
            ,       THeapBuf&               mbufValue
            ,       tCQCKit::EFldTypes&     eType
        )   final;

        tCIDLib::TBoolean bReadFieldByName
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strMoniker
            , const TString&                strFldName
            ,       TString&                strToFill
            ,       tCQCKit::EFldTypes&     eType
        )   final;

        tCIDLib::TBoolean bReadFieldByName
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4RetBytes
            ,       THeapBuf&               mbufValue
            ,       tCQCKit::EFldTypes&     eType
        )   final;

        tCIDLib::TBoolean bReadFloatByName
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TFloat8&       f8ToFill
        )   final;

        tCIDLib::TBoolean bReadIntByName
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TInt4&         i4ToFill
        )   final;

        tCIDLib::TBoolean bReadSListByName
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TStrList&      colToFill
        )   final;

        tCIDLib::TBoolean bReadStringByName
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strMoniker
            , const TString&                strFldName
            ,       TString&                strToFill
        )   final;

        tCIDLib::TBoolean bReadTimeByName
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard8&        c8ToFill
        )   final;

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strMoniker
            , const TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TCard4 c4QueryAudioDevs
        (
                    tCIDLib::TStrList&      colToFill
        )   final;

        tCIDLib::TCard4 c4QueryDriverConfigObjs
        (
                    TVector<TCQCDriverObjCfg>& colToFill
            ,       tCIDLib::TCard4&        c4DriverListId
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TCard4 c4QueryDriverId
        (
            const   TString&                strMoniker
            ,       tCIDLib::TCard4&        c4DriverListId
        )   final;

        tCIDLib::TCard4 c4QueryDriverIds
        (
            const   tCIDLib::TStrList&      colMonikers
            ,       tCIDLib::TCardList&     fcolIds
            ,       tCIDLib::TKVPList&      colMakeModels
        )   final;

        tCIDLib::TCard4 c4QueryDriverIdList
        (
                    tCIDLib::TStrList&      colMonkers
            ,       tCIDLib::TCardList&     fcolIds
            ,       tCIDLib::TKVPList&      colMakeModels
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TCard4 c4QueryDriverIdList2
        (
                    tCIDLib::TStrList&      colMonikers
            ,       tCIDLib::TCardList&     fcolIds
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TCard4 c4QueryFields
        (
            const   TString&                strMoniker
            ,       tCQCKit::EDrvStates&    eState
            ,       tCQCKit::TFldDefList&   colToFill
            ,       tCIDLib::TCard4&        c4FldListId
            ,       tCIDLib::TCard4&        c4DriverId
            ,       tCIDLib::TCard4&        c4DriverListId
        )   final;

        tCIDLib::TCard4 c4QueryFieldNames
        (
            const   TString&                strMoniker
            ,       TVector<TString>&       colToFill
            , const tCQCKit::EReqAccess     eAccess
        )   final;

        tCIDLib::TCard4 c4QueryFieldNamesRX
        (
            const   TString&                strMoniker
            ,       tCQCKit::TFldDefList&   colToFill
            , const TString&                strRegEx
            , const tCQCKit::EReqAccess     eAccess
        )   final;

        tCIDLib::TCard4 c4QuerySemFields
        (
            const   tCQCKit::TFldSTypeList& fcolTypes
            , const tCQCKit::EReqAccess     eReqAccess
            ,       TVector<TCQCDrvFldDef>& colToFill
            , const TString&                strMoniker
        )   final;

        tCIDLib::TCard4 c4QueryTTSVoices
        (
                    tCIDLib::TStrList&      colToFill
        )   final;

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const tCIDLib::TBoolean       bNoQueue
        )   final;

        tCIDLib::TCard4 c4QueryVal2
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const TString&                strValName
            , const tCIDLib::TBoolean       bNoQueue
        )   final;

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strMoniker
            , const TString&                strCmdId
            , const TString&                strParms
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TInt4 i4QueryVal
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const tCIDLib::TBoolean       bNoQueue
        )   final;

        tCIDLib::TInt4 i4QueryVal2
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const TString&                strValName
            , const tCIDLib::TBoolean       bNoQueue
        )   final;

        tCIDLib::TVoid CancelTimedWrite
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid ClearDrvStats
        (
            const   TString&                strMoniker
        )   final;

        tCIDLib::TVoid DoTimedWrite
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const TString&                strNewValue
            , const TString&                strEndValue
            , const tCIDLib::TCard4         c4Seconds
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid QueryConfig
        (
            const   TString&                strMoniker
            ,       tCIDLib::TCard4&        c4BytesFilled
            ,       THeapBuf&               mbufToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid QueryDriverInfo
        (
            const   TString&                strMoniker
            ,       tCQCKit::EDrvStates&    eStatus
            ,       tCIDLib::TCard4&        c4ArchVer
            ,       tCQCKit::TDevClassList& fcolDevClasses
            ,       TString&                strMake
            ,       TString&                strModel
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid QueryDriverState
        (
            const   TString&                strMoniker
            ,       tCQCKit::EDrvStates&    eStatus
            ,       tCQCKit::EVerboseLvls&  eVerbose
            ,       tCIDLib::TCard4&        c4ThreadId
        )   final;

        tCIDLib::TVoid QueryDriverStats
        (
            const   TString&                strMoniker
            ,       TCQCDrvStats&           cdstatsToFill
        )   final;

        tCIDLib::TVoid QueryFieldDef
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       TCQCFldDef&             flddToFill
        )   final;

        tCIDLib::TVoid QueryFieldInfo
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       tCIDLib::TCard4&        c4DriverListId
            ,       tCIDLib::TCard4&        c4DriverId
            ,       tCIDLib::TCard4&        c4FieldListId
            ,       tCIDLib::TCard4&        c4FieldId
            ,       tCQCKit::EFldTypes&     eType
        )   final;

        tCIDLib::TVoid QueryPowerStates
        (
            const   tCIDLib::TKVPList&      colDrivers
            ,       TFundVector<tCQCKit::EPowerStatus>& fcolStates
        )   final;

        tCIDLib::TVoid QueryStatusReport
        (
                    TString&                strErrList
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid PauseResumeDrv
        (
            const   TString&                strMoniker
            , const tCIDLib::TBoolean       bPause
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid ReadDriverFields
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCardList&     fcolFldIds
            , const tCIDLib::TCardList&     fcolSerialNums
            ,       tCIDLib::TCard4&        c4BufSz_mbufData
            ,       THeapBuf&               mbufData
        )   final;

        tCIDLib::TVoid ReadFields
        (
            const   TFldIOPacket&           fiopToRead
            ,       tCIDLib::TCard4&        c4BytesRead
            ,       THeapBuf&               mbufData
        )   final;

        tCIDLib::TVoid ReadFieldsByName
        (
            const   TString&                strMoniker
            , const tCIDLib::TStrList&      colFieldNames
            ,       tCIDLib::TCard4&        c4BytesRead
            ,       THeapBuf&               mbufData
        )   final;

        tCIDLib::TVoid ReconfigDriver
        (
            const   TString&                strMoniker
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid RefreshCfg
        (
            const   TString&                strCfgType
        )   final;

        tCIDLib::TVoid SetConfig
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Bytes
            , const THeapBuf&               mbufNewCfg
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid SetVerboseLevel
        (
            const   TString&                strMoniker
            , const tCQCKit::EVerboseLvls   eToSet
        )   final;

        tCIDLib::TVoid UnloadAllDrivers
        (
            const   TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid UnloadDriver
        (
            const   TString&                strMonikerToUnload
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid UpdateTriggers
        (
            const   TString&                strMoniker
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid WriteBoolByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TBoolean       bValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteBoolFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteCardByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TCard4         c4Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteCardFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteFieldByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const TString&                strValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteFloatByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TFloat8        f8Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteFloatFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteIntByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TInt4          i4Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteIntFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteStringByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const TString&                strValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteStringFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteSListByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TStrList&      colValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteSListFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TStrList&      colValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteTimeByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TCard8         c8Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;

        tCIDLib::TVoid WriteTimeFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard8         c8Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid UpdateDrvPtr
        (
                    TCQCServerBase* const   psdrvTar
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_psdrvTar
        //      The target server side driver that we are providing the wiring to.
        //      We don't own it, we just get a pointer to it. And will get a new one
        //      generally each time the harness/IDE starts the driver up again.
        // --------------------------------------------------------------------
        TCQCServerBase*     m_psdrvTar;


        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TCQCSrvDrvTI, TCQCSrvAdminServerBase)
};

#pragma CIDLIB_POPPACK

