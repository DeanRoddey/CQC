//
// FILE NAME: CQSLLogicS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2009
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
//  This is the header file for the driver implementation class. We derive
//  from the base server side class and override callbacks.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQSLLogicS
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TCQSLLogicSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLLogicSDriver() = delete;

        TCQSLLogicSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TCQSLLogicSDriver(const TCQSLLogicSDriver&) = delete;
        TCQSLLogicSDriver(TCQSLLogicSDriver&&) = delete;

        ~TCQSLLogicSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQSLLogicSDriver& operator=(const TCQSLLogicSDriver&) = delete;
        TCQSLLogicSDriver& operator=(TCQSLLogicSDriver&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmd
            , const TString&                strParms
        )   final;

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::EDrvInitRes eInitializeImpl() final;

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TVoid ReleaseCommResource() final;

        tCIDLib::TVoid TerminateImpl() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCQSLLogicS::EUpdateRes eUpdateFields
        (
            const   tCIDLib::TBoolean       bInitialPass
        );

        tCIDLib::TVoid RegisterFields();

        tCIDLib::TVoid StoreFields
        (
            const   THeapBuf&               mbufUpdate
            , const tCIDLib::TCard4         c4Size
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4CfgSerialNum
        //      Our configuration serial number, which we have to pass back
        //      to the server, so that he knows if we get out of sync.
        //
        //  m_cfgCurrent
        //      Our current driver configuration, downloaded from the data
        //      server, or accepted from the client driver.
        //
        //  m_mbufUpdate
        //      A memory buffer we use for querying changes from the server.
        //
        //  m_orbcLS
        //      Our client proxy for the logic server.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4CfgSerialNum;
        TCQLogicSrvCfg              m_cfgCurrent;
        THeapBuf                    m_mbufUpdate;
        tCQLogicSh::TLogicSrvProxy  m_orbcLS;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLLogicSDriver,TCQCServerBase)
};


#pragma CIDLIB_POPPACK

