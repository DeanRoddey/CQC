//
// FILE NAME: VarDriverS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2003
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
//  This is the header file for the actual driver class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TVarDriverSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TVarDriverSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TVarDriverSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TVarDriverSDriver();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------

        // Purely for driver testing, returning canned data
        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strQueryType
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bQueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TBoolean bQueryVal2
        (
            const   TString&                strValId
            , const TString&                strValName
        );

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufToSend
        );

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TCard4 c4QueryVal2
        (
            const   TString&                strValId
            , const TString&                strValName
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmdId
            , const TString&                strParms
        );

        tCIDLib::TInt4 i4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TInt4 i4QueryVal2
        (
            const   TString&                strValId
            , const TString&                strValName
        );


        // Really used
        tCIDLib::TCard4 c4QueryCfg
        (
                    THeapBuf&               mbufToFill
        );

        tCIDLib::TVoid SetConfig
        (
            const   tCIDLib::TCard4         c4Bytes
            , const THeapBuf&               mbufNewCfg
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eFloatFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8NewValue
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults eIntFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewValue
        );

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        );

        tCQCKit::ECommResults eSListFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TStrList&      colNewValue
        );

        tCQCKit::ECommResults eTimeFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard8&        c8NewValue
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TVarDriverSDriver();
        TVarDriverSDriver(const TVarDriverSDriver&);
        tCIDLib::TVoid operator=(const TVarDriverSDriver&);


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDLib::TVoid PersistValue
        (
            const   TString&                strField
            , const TString&                strValue
        );

        tCIDLib::TVoid RegisterFields();

        tCIDLib::TVoid SetFieldDefault
        (
            const   TString&                strField
        );

        tCIDLib::TVoid StoreConfig();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4ChangeCnt
        //      We don't store persistent value changes every time it happens,
        //      since that could be very inefficient. We just track how many
        //      changes have happened and set a long poll time. On each poll,
        //      we flush any changes.
        //
        //  m_cfgData
        //      The configuration loaded from the config respository. It tells
        //       us what fields to create and any other config we need. We
        //      don't actually use the configuration data live, since we just
        //      create the fields and that's that. But, in order to support
        //      queries of our current config, we keep a copy around.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4ChangeCnt;
        TVarDrvCfg              m_cfgData;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TVarDriverSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK

