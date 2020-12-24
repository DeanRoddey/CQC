//
// FILE NAME: iTunesPlayerTMS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/09/2012
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
//  This is the main implementation file of the driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "iTunesPlayerTMS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TiTunesPlayerTMSDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: iTunesPlayerTMSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  iTunesPlayerSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TiTunesPlayerTMSDriver::TiTunesPlayerTMSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldId_CurAlbum(kCIDLib::c4MaxCard)
    , m_c4FldId_CurArtist(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTrack(kCIDLib::c4MaxCard)
    , m_c4FldId_Mute(kCIDLib::c4MaxCard)
    , m_c4FldId_PlayerCmd(kCIDLib::c4MaxCard)
    , m_c4FldId_PBPercent(kCIDLib::c4MaxCard)
    , m_c4FldId_SelPLByCookie(kCIDLib::c4MaxCard)
    , m_c4FldId_SelTrackByCookie(kCIDLib::c4MaxCard)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_c4FldId_Volume(kCIDLib::c4MaxCard)
    , m_porbcTray(0)
{
}

TiTunesPlayerTMSDriver::~TiTunesPlayerTMSDriver()
{
}


// ---------------------------------------------------------------------------
//  TiTunesPlayerTMSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

// Try to create a client proxy for the tray monitor
tCIDLib::TBoolean TiTunesPlayerTMSDriver::bGetCommResource(TThread&)
{
    try
    {
        // See if we can find the object id, with a pretty short timeout
        const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusSecs(2);
        TOrbObjId ooidTray;
        if (facCIDOrbUC().bGetNSObject(m_strBinding, ooidTray, enctEnd))
            m_porbcTray = new TiTPlayerIntfClientProxy(ooidTray, m_strBinding);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // If we got the client proxy allocated, we succeeded
    return (m_porbcTray != 0);
}


// Nothing to do. The tray loads the database
tCIDLib::TBoolean TiTunesPlayerTMSDriver::bWaitConfig(TThread&)
{
    return kCIDLib::True;
}



tCQCKit::ECommResults
TiTunesPlayerTMSDriver::eBoolFldValChanged( const   TString&            strName
                                            , const tCIDLib::TCard4     c4FldId
                                            , const tCIDLib::TBoolean   bNewValue)
{
    try
    {
        if (c4FldId == m_c4FldId_Mute)
        {
            m_porbcTray->SetMute(bNewValue);
        }
         else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::LostCommRes;
    }

    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TiTunesPlayerTMSDriver::eCardFldValChanged( const   TString&        strName
                                            , const tCIDLib::TCard4 c4FldId
                                            , const tCIDLib::TCard4 c4NewValue)
{
    try
    {
        if (c4FldId == m_c4FldId_Volume)
        {
            m_porbcTray->SetVolume(c4NewValue);
        }
         else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::LostCommRes;
    }

    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TiTunesPlayerTMSDriver::eConnectToDevice(TThread& thrCaller)
{
    //
    //  We have already really connected, in the getting of the comm resource,
    //  so now we just do an initial state query to both wait for the
    //  tray monitor to get up and ready, and then to get the initial values
    //  of the readable fields.
    //
    //  Reset the serial number in case we get connected, so we'll be sure
    //  to get new data.
    //
    m_c4SerialNum = 0;
    return eGetPlayerStatus();
}


tCQCKit::EDrvInitRes TiTunesPlayerTMSDriver::eInitializeImpl()
{
    //
    //  Make sure that we were configured for a socket connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCOtherConnCfg::clsThis())
    {
        // Note that we are throwing a CQCKit error here!
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcOurs.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCOtherConnCfg::clsThis()
        );
    }

    SetPollTimes(1000, 5000);

    // Set up the binding we expect to find the tray monitor at
    m_strBinding = TiTPlayerIntfClientProxy::strImplBinding;
    m_strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_b);

    // Set up our fields
    TVector<TCQCFldDef> colFlds(16);
    TCQCFldDef flddNew;

    const tCIDLib::TCh* const pszFldCommand(L"Command");
    const tCIDLib::TCh* const pszFldCurAlbum(L"CurAlbum");
    const tCIDLib::TCh* const pszFldCurArtist(L"CurArtist");
    const tCIDLib::TCh* const pszFldCurTrack(L"CurTrack");
    const tCIDLib::TCh* const pszFldMute(L"Mute");
    const tCIDLib::TCh* const pszFldSelPLByCookie(L"SelPLByCookie");
    const tCIDLib::TCh* const pszFldSelTrackByCookie(L"SelTrackByCookie");
    const tCIDLib::TCh* const pszFldState(L"State");
    const tCIDLib::TCh* const pszFldVolume(L"Volume");

    flddNew.Set
    (
        pszFldCurAlbum
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldCurArtist
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldCurTrack
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldMute
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::Mute
    );
    colFlds.objAdd(flddNew);

    //
    //  Format out the value of the player command enum, into an enum
    //  limit for the command field.
    //
    m_strPollTmp1 = L"Enum: ";
    TiTPlayerIntfClientProxy::EPlCmds eCmd = TiTPlayerIntfClientProxy::EPlCmds::Min;
    while (eCmd < TiTPlayerIntfClientProxy::EPlCmds::Count)
    {
        if (eCmd > TiTPlayerIntfClientProxy::EPlCmds::Min)
            m_strPollTmp1.Append(L',');
        m_strPollTmp1.Append(TiTPlayerIntfClientProxy::strXlatEPlCmds(eCmd));
        eCmd++;
    }
    flddNew.Set
    (
        pszFldCommand
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , m_strPollTmp1
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PBPercent)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
        , L"Range: 0, 100"
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldSelPLByCookie
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldSelTrackByCookie
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldState
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldVolume
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::Volume
        , L"Range: 0, 100"
    );
    colFlds.objAdd(flddNew);

    //
    //  Now register our fields with the base driver class, and go back
    //  and look up the ids.
    //
    SetFields(colFlds);

    m_c4FldId_CurAlbum = pflddFind(pszFldCurAlbum, kCIDLib::True)->c4Id();
    m_c4FldId_CurArtist = pflddFind(pszFldCurArtist, kCIDLib::True)->c4Id();
    m_c4FldId_CurTrack = pflddFind(pszFldCurTrack, kCIDLib::True)->c4Id();
    m_c4FldId_Mute = pflddFind(pszFldMute, kCIDLib::True)->c4Id();
    m_c4FldId_PlayerCmd = pflddFind(pszFldCommand, kCIDLib::True)->c4Id();
    m_c4FldId_PBPercent = pflddFind(TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PBPercent), kCIDLib::True)->c4Id();
    m_c4FldId_SelPLByCookie = pflddFind(pszFldSelPLByCookie, kCIDLib::True)->c4Id();
    m_c4FldId_SelTrackByCookie = pflddFind(pszFldSelTrackByCookie, kCIDLib::True)->c4Id();
    m_c4FldId_State = pflddFind(pszFldState, kCIDLib::True)->c4Id();
    m_c4FldId_Volume = pflddFind(pszFldVolume, kCIDLib::True)->c4Id();

    // Set some initial values where appropriate
    bStoreStringFld
    (
        m_c4FldId_State
        , TiTPlayerIntfClientProxy::strXlatEPlStates(TiTPlayerIntfClientProxy::EPlStates::None)
        , kCIDLib::True
    );
    bStoreStringFld(m_c4FldId_CurAlbum, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurArtist, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTrack, TString::strEmpty(), kCIDLib::True);

    // We'll try to get a first load of the database before we come online
    return tCQCKit::EDrvInitRes::WaitCommRes;
}


//
//  We just call the status query method. It will return the appropriate
//  status value, so we just return its return.
//
tCQCKit::ECommResults TiTunesPlayerTMSDriver::ePollDevice(TThread& thrCaller)
{
    return eGetPlayerStatus();
}


tCQCKit::ECommResults
TiTunesPlayerTMSDriver::eStringFldValChanged(const  TString&        strName
                                            , const tCIDLib::TCard4 c4FldId
                                            , const TString&        strNewValue)
{
    if (c4FldId == m_c4FldId_PlayerCmd)
    {
        // Translate to a player cmd enum
        TiTPlayerIntfClientProxy::EPlCmds eCmd(TiTPlayerIntfClientProxy::EPlCmds::Min);

        while (eCmd < TiTPlayerIntfClientProxy::EPlCmds::Count)
        {
            if (strNewValue == TiTPlayerIntfClientProxy::strXlatEPlCmds(eCmd))
                break;
            eCmd++;
        }

        if (eCmd < TiTPlayerIntfClientProxy::EPlCmds::Count)
        {
            try
            {
                m_porbcTray->DoPlayerCmd(eCmd);
            }

            catch(TError& errToCatch)
            {
                if (eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
                return tCQCKit::ECommResults::LostCommRes;
            }
        }
         else
        {

            IncFailedWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
        }
    }
     else if (c4FldId == m_c4FldId_SelTrackByCookie)
    {
        //
        //  We should have gotten an item cookie. So parse it to make sure
        //  it's really one.
        //
        tCIDLib::TCard2 c2CatId;
        tCIDLib::TCard2 c2TitleId;
        tCIDLib::TCard2 c2ColId;
        tCIDLib::TCard2 c2ItemId;
        facCQCMedia().eParseItemCookie
        (
            strNewValue, c2CatId, c2TitleId, c2ColId, c2ItemId
        );

        // Looks reasonable so try it
        try
        {
            m_porbcTray->SelTrackByCookie(strNewValue);
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            return tCQCKit::ECommResults::LostCommRes;
        }
    }
     else if (c4FldId == m_c4FldId_SelPLByCookie)
    {
        //
        //  We should have gotten a title cookie. So parse it to make sure
        //  it's really one.
        //
        tCIDLib::TCard2 c2CatId;
        tCIDLib::TCard2 c2TitleId;
        facCQCMedia().eParseTitleCookie(strNewValue, c2CatId, c2TitleId);

        // Looks reasonable so try it
        try
        {
            m_porbcTray->SelPLByCookie(strNewValue);
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            return tCQCKit::ECommResults::LostCommRes;
        }
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TiTunesPlayerTMSDriver::ReleaseCommResource()
{
    // Free the client proxy if we have one
    if (m_porbcTray)
    {
        try
        {
            delete m_porbcTray;
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        m_porbcTray = 0;
    }
}


tCIDLib::TVoid TiTunesPlayerTMSDriver::TerminateImpl()
{
}


// ---------------------------------------------------------------------------
//  TiTunesPlayerTMSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method is called on connect and poll, to get the current player
//  stauts. We use a poll method, so we only get data back if the tray monitor
//  is returning new data. Else there's nothing to do, so we get maxium
//  efficiency.
//
tCQCKit::ECommResults TiTunesPlayerTMSDriver::eGetPlayerStatus()
{
    try
    {
        tCIDLib::TBoolean           bConn;
        tCIDLib::TBoolean           bMute;
        tCIDLib::TCard4             c4Volume;
        tCIDLib::TEncodedTime       enctCurPos;
        tCIDLib::TEncodedTime       enctCurTotal;
        TiTPlayerIntfClientProxy::EPlStates ePlState;

        const tCIDLib::TBoolean bNewData = m_porbcTray->bGetPlayerState
        (
            m_c4SerialNum
            , bConn
            , c4Volume
            , ePlState
            , bMute
            , enctCurTotal
            , enctCurPos
            , m_strPollTmp1
            , m_strPollTmp2
            , m_strPollTmp3
        );

        // If new data to store, store it
        if (bNewData)
        {
            // Calculate the playback percentage
            tCIDLib::TCard4 c4PBPercent = 0;
            if (enctCurTotal && enctCurPos)
            {
                c4PBPercent = tCIDLib::TCard4
                (
                    (tCIDLib::TFloat8(enctCurPos) / tCIDLib::TFloat8(enctCurTotal)) * 100.0
                );

                if (c4PBPercent > 100)
                    c4PBPercent = 100;
            }

            // If the connection status has gond false, recycle the connection
            if (!bConn)
                return tCQCKit::ECommResults::LostConnection;

            if (c4Volume > 100)
                c4Volume = 100;


            bStoreBoolFld(m_c4FldId_Mute, bMute, kCIDLib::True);
            bStoreCardFld(m_c4FldId_Volume, c4Volume, kCIDLib::True);
            bStoreCardFld(m_c4FldId_PBPercent, c4PBPercent, kCIDLib::True);
            bStoreStringFld
            (
                m_c4FldId_State
                , TiTPlayerIntfClientProxy::strXlatEPlStates(ePlState)
                , kCIDLib::True
            );
            bStoreStringFld(m_c4FldId_CurAlbum, m_strPollTmp1, kCIDLib::True);
            bStoreStringFld(m_c4FldId_CurArtist, m_strPollTmp2, kCIDLib::True);
            bStoreStringFld(m_c4FldId_CurTrack, m_strPollTmp3, kCIDLib::True);
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        //
        //  Assume the worst and recylce our client proxy. Note in this
        //  case we can't test if there is a loss of connection, since
        //  the server may still be there, but not the server side
        //  interface for this client proxy. The iTunes interface is
        //  configurable and it could get removed while the server side
        //  ORB is still there.
        //
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}

