//
// FILE NAME: CQLogicSrv_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2009
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
//  This is the header for the facility class for the server. Our IDL
//  interface class just delegates to us here to serve up data to clients.
//  Our clients are the driver that works through this server and exposes
//  our fields, and there's a configuration dialog in the admin interface
//  that needs to get our configuration and send us back new config after
//  the user changes it.
//
//  Basically we load the current configuration from the master server
//  config server (we only run on the master server), register then with
//  an instance of the polling engine that we create, and regularly poll
//  the polling engine for changes.
//
//  We mark any changed source fields that we've seen since the last
//  query from the driver. When the driver asks for changes, we give him
//  all those fields that have changed, and clear the marks for next time.
//  We only support one instance of our driver, so we don't need to do any
//  fancy work to keep separate change lists or anything like that.
//
//  We also provide some methods for our implementation of the standard
//  server admin protocol can call.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQLogicSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQLogicSrv : public TCQCSrvCore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQLogicSrv();

        TFacCQLogicSrv(const TFacCQLogicSrv&) = delete;
        TFacCQLogicSrv(TFacCQLogicSrv&&) = delete;

        ~TFacCQLogicSrv() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQLogicSrv& operator=(const TFacCQLogicSrv&) = delete;
        TFacCQLogicSrv& operator=(TFacCQLogicSrv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryFields
        (
            const   tCIDLib::TCard4         c4CfgSerialNum
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufData
            , const tCIDLib::TBoolean       bReturnAll
        );

        tCIDLib::TBoolean bQueryGraphSamples
        (
            const   TString&                strFldName
            ,       tCIDLib::TCard4&        c4CfgSerialNum
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4NewSamples
            ,       TFundVector<tCIDLib::TFloat4>& fcolNewSamples
        );

        tCIDLib::TCard4 c4QueryGraphFlds
        (
                    TVector<TString>&       colToFill
        );

        tCIDLib::TVoid QueryConfig
        (
                    TCQLogicSrvCfg&         cfgToFill
            ,       tCIDLib::TCard4&        c4SerialNum
        );

        tCIDLib::TVoid ResetElTimeFld
        (
            const   TString&                strFldName
        );

        tCIDLib::TVoid SetConfig
        (
            const   TCQLogicSrvCfg&         cfgToSet
            ,       tCIDLib::TCard4&        c4SerialNum
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DeregisterSrvObjs() final;

        tCQCSrvFW::EStateRes eLoadConfig
        (
                    tCIDLib::TCard4&        c4WaitNext
            , const tCIDLib::TCard4         c4Count
        )   final;

        tCIDLib::TVoid PostDeregTerm() final;

        tCIDLib::TVoid PreRegInit() final;

        tCIDLib::TVoid QueryCoreAdminInfo
        (
                    TString&                strCoreAdminBinding
            ,       TString&                strCoreAdminDesc
            ,       TString&                strExtra1
            ,       TString&                strExtra2
            ,       TString&                strExtra3
            ,       TString&                strExtra4
        )   final;

        tCIDLib::TVoid RegisterSrvObjs() final;

        tCIDLib::TVoid StoreConfig() final;

        tCIDLib::TVoid WaitForTerm
        (
                    TEvent&                 evWait
        )   final;

        tCIDLib::TVoid UnbindSrvObjs
        (
                    tCIDOrbUC::TNSrvProxy&  orbcNS
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid InitFields();

        tCIDLib::TVoid ProcessRequests();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4CfgSerialNum
        //      Any time the configuration changes we bump this. The driver
        //      has to pass in the last serial number it got, so that we can
        //      tell it if the config has changed.
        //
        //  m_lscfgServer
        //      This is our configuration data, which defines the virtual
        //      fields that we provide. And it also provides the means to
        //      generate the virtual field values at runtime and some support
        //      for the client side configuration editor. We store this in
        //      a file in CQCData\Server\Data\LogicSrv.
        //
        //  m_colValues
        //      We have to create a value object for each field, which we
        //      pass to the field configuration objects to let them generate
        //      the value into. It has to be polymorphic since each of our
        //      fields can have a different type. If we get an error while
        //      evaluating a field we set it into error state.
        //
        //      We use the serial number as a "what's changed since the driver
        //      last asked" flag. We bump the serial number any time the
        //      value or state changes, and zero it back again when the
        //      driver asks for the latest values.
        //
        //  m_mtxLock
        //      We need to provide some synchronization to our configuration
        //      data between incoming client calls and our main thread that
        //      keeps the field values up to date.
        //
        //  m_oseData
        //      Though we currently only have the field config, we have our own
        //      repo in case we need to store other stuff in the future. This just
        //      lets us avoid a lot of grunt work.
        //
        //  m_polleSrv
        //      Our polling engine that provides the core functionality of
        //      the server. It handles polling all the source fields for us,
        //      and we just pull field state/value from it.
        //
        //  m_porbsImpl
        //      We keep around a pointer to our main interface impl object so that
        //      we can de-register it during shutdown.
        //
        //  m_sciCalcErrors
        //  m_sciFields
        //      Some stats cache values we maintain at our level
        //
        //  m_strFldCfgPath
        //      The path in our private repo of the field config data
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4CfgSerialNum;
        TCQLogicSrvCfg          m_lscfgServer;
        tCQCKit::TFldValList    m_colValues;
        TCIDObjStore            m_oseData;
        TMutex                  m_mtxLock;
        TOrbObjId               m_ooidAdmin;
        TCQCPollEngine          m_polleSrv;
        TLogicSrvImpl*          m_porbsImpl;
        TStatsCacheItem         m_sciCalcErrors;
        TStatsCacheItem         m_sciFieldCnt;
        const TString           m_strFldCfgPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQLogicSrv,TFacility)
};

#pragma CIDLIB_POPPACK

