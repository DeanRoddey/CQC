//
// FILE NAME: iTunesRepoTMS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2012
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
//  This is the header file for the actual driver class. This one is pretty
//  simple since it just acts as a pass through to the tray monitor that it
//  talks to. The tray monitor manages the actual database. Since we don't
//  manage the database we don't derive from the standard media repo driver
//  class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: iTunesRepoS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TiTunesRepoTMSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TiTunesRepoTMSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TiTunesRepoTMSDriver();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
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

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufToSend
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmd
            , const TString&                strParms
        );

        tCIDLib::TVoid ReloadDB();


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

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TiTunesRepoTMSDriver();
        TiTunesRepoTMSDriver(const TiTunesRepoTMSDriver&);
        tCIDLib::TVoid operator=(const TiTunesRepoTMSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckConnected();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so
        //      that we can do efficient 'by id' reads/writes.
        //
        //  m_porbcTray
        //      A pointer to a client proxy for the tray app. We create it
        //      during get comm resource and delete it if we lose connection.
        //
        //  m_strBinding
        //      We pre-build the name server binding that we expect to find
        //      the tray monitor at.
        //
        //  m_strPollTmp
        //      A temp string for the poll method to use.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldId_DBSerialNum;
        tCIDLib::TCard4         m_c4FldId_Loaded;
        tCIDLib::TCard4         m_c4FldId_LoadStatus;
        tCIDLib::TCard4         m_c4FldId_ReloadDB;
        tCIDLib::TCard4         m_c4FldId_Status;
        tCIDLib::TCard4         m_c4FldId_TitleCnt;
        TiTRepoIntfClientProxy* m_porbcTray;
        TString                 m_strBinding;
        TString                 m_strPollTmp;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TiTunesRepoTMSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


