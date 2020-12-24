//
// FILE NAME: BarcoCRTS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2002
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
//  This is a driver which implements the Barco CRT projector server side
//  object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "BarcoCRTS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TBarcoCRTSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//   CLASS: TBarcoCRTSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TBarcoCRTSDriver::TBarcoCRTSDriver(const  TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c1ProjAddr(1)
    , m_c4FldIdHorzPeriod(kCIDLib::c4MaxCard)
    , m_c4FldIdInput(kCIDLib::c4MaxCard)
    , m_c4FldIdPower(kCIDLib::c4MaxCard)
    , m_c4FldIdSigType(kCIDLib::c4MaxCard)
    , m_c4FldIdVertRate(kCIDLib::c4MaxCard)
    , m_c4ProjBaseType(800)
    , m_c4Timeout(0)
    , m_pcommBarco(0)
{
    //
    //  Set up the status poll message. We use a pre-fab message to avoid
    //  re-building it constantly. It will fault in some stuff up first xmit
    //  of it, but there's no threading issue because its only used from the
    //  poll thread.
    //
    //  NOTE:   These make use of the address gotten out of the prompts
    //          above, so keep it after that code!
    //
    m_msgQuerySrc.Set(m_c1ProjAddr, tBarcoCRTS::ECmds::QueryCurSource);
    m_msgQueryStatus.Set(m_c1ProjAddr, tBarcoCRTS::ECmds::QueryStatus);
}

TBarcoCRTSDriver::~TBarcoCRTSDriver()
{
    // Clean up the comm port if not already
    if (m_pcommBarco)
    {
        try
        {
            if (m_pcommBarco->bIsOpen())
                m_pcommBarco->Close();
            delete m_pcommBarco;
            m_pcommBarco = 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TBarcoCRTSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TBarcoCRTSDriver::bGetCommResource(TThread& thrThis)
{
    try
    {
        // Create the port if needed, else close if open just in case
        if (!m_pcommBarco)
            m_pcommBarco = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommBarco->bIsOpen())
            m_pcommBarco->Close();

        // Open the port now, and set the configuration
        m_pcommBarco->Open(tCIDComm::EOpenFlags::WriteThrough);
        m_pcommBarco->SetCfg(m_cpcfgSerial);
    }

    catch(const TError&)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TBarcoCRTSDriver::bWaitConfig(TThread& thrThis)
{
    // Just say we are happy
    return kCIDLib::True;
}


tCQCKit::ECommResults
TBarcoCRTSDriver::eBoolFldValChanged(const  TString&            strName
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TBoolean   bNewValue)
{
    if (c4FldId == m_c4FldIdPower)
    {
        TBarcoMsg msgTmp;
        msgTmp.Set(m_c1ProjAddr, tBarcoCRTS::ECmds::Standby);
        msgTmp.bSendOn(m_pcommBarco);
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TBarcoCRTSDriver::eCardFldValChanged(const  TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TCard4 c4NewValue)
{
    if (c4FldId == m_c4FldIdInput)
    {
        //
        //  We have to send it as IR commands, which is kludgy, but there
        //  doesn't seem to be any other way. We have to send two digits,
        //  so be sure to send a leading zero if < 10.
        //
        const tBarcoCRTS::ECmds eOne = tBarcoCRTS::ECmds
        (
            (c4NewValue / 10) + tBarcoCRTS::ECmds::Num0
        );
        const tBarcoCRTS::ECmds eTwo = tBarcoCRTS::ECmds
        (
            (c4NewValue % 10) + tBarcoCRTS::ECmds::Num0
        );

        TBarcoMsg msgTmp;
        if (c4NewValue > 9)
        {
            msgTmp.Set(m_c1ProjAddr, eOne);
            msgTmp.bSendOn(m_pcommBarco);
        }

        msgTmp.Set(m_c1ProjAddr, eTwo);
        msgTmp.bSendOn(m_pcommBarco);
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TBarcoCRTSDriver::eConnectToDevice(TThread& thrThis)
{
    //
    //  We don't have any static data fields, so just do a status poll
    //  call and return the result of that. If it works, our fields are
    //  updated and we are ready to go.
    //
    const tBarcoCRTS::EMsgResults eRes = eDoStatusPoll(thrThis);

    tCQCKit::ECommResults eRet;
    switch(eRes)
    {
        case tBarcoCRTS::EMsgResults::Ok :
            eRet = tCQCKit::ECommResults::Success;
            break;

        case tBarcoCRTS::EMsgResults::NoReply :
        case tBarcoCRTS::EMsgResults::PartialReply :
            eRet = tCQCKit::ECommResults::LostConnection;
            break;

        case tBarcoCRTS::EMsgResults::BadCheckSum :
        case tBarcoCRTS::EMsgResults::BadCmd :
        case tBarcoCRTS::EMsgResults::SendFailed :
            eRet = tCQCKit::ECommResults::LostCommRes;
            break;
    };
    return eRet;
}


tCQCKit::EDrvInitRes TBarcoCRTSDriver::eInitializeImpl()
{
    //
    //  Make sure that we were configured for a serial connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCSerialConnCfg::clsThis())
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
            , TCQCSerialConnCfg::clsThis()
        );
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    const TCQCSerialConnCfg& conncfgSer
            = static_cast<const TCQCSerialConnCfg&>(cqcdcOurs.conncfgReal());
    m_cpcfgSerial = conncfgSer.cpcfgSerial();
    m_strPort = conncfgSer.strPortPath();

    // Clean up any existing port so it'll get redone with new config
    delete m_pcommBarco;
    m_pcommBarco = 0;

    // We should have gotten two prompts
    TString strAddr;
    if (!cqcdcOurs.bFindPromptValue(L"Address", L"Value", strAddr))
    {
        facBarcoCRTS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The project address prompt was not set"
            , tCIDLib::ESeverities::Status
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    TString strType;
    if (!cqcdcOurs.bFindPromptValue(L"ProjType", L"Selected", strType))
    {
        facBarcoCRTS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The projector type prompt was not set"
            , tCIDLib::ESeverities::Status
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    m_c1ProjAddr = tCIDLib::TCard1(strAddr.c4Val());
    if (strType == L"700")
        m_c4ProjBaseType = 700;
    else if (strType == L"800")
        m_c4ProjBaseType = 800;
    else if (strType == L"1100")
        m_c4ProjBaseType = 1100;
    else if (strType == L"1200")
        m_c4ProjBaseType = 1200;
    else if (strType == L"1600")
        m_c4ProjBaseType = 1600;
    else
    {
        facBarcoCRTS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcCfg_UnknownProjType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , strType
        );
    }

    // Set up our fields and register them
    TVector<TCQCFldDef> colFlds(6);
    TCQCFldDef flddNew;

    flddNew.Set
    (
        kBarcoCRTS::pszFld_HorzPeriod
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kBarcoCRTS::pszFld_Input
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::ReadWrite
        , L"Range:1,99"
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kBarcoCRTS::pszFld_Power
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kBarcoCRTS::pszFld_SigType
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kBarcoCRTS::pszFld_VertRate
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    SetFields(colFlds);

    //
    //  Look up the ids of the fields, so that we can 'by id' access to
    //  them instead name based.
    //
    m_c4FldIdHorzPeriod = pflddFind(kBarcoCRTS::pszFld_HorzPeriod, kCIDLib::True)->c4Id();
    m_c4FldIdInput = pflddFind(kBarcoCRTS::pszFld_Input, kCIDLib::True)->c4Id();
    m_c4FldIdPower = pflddFind(kBarcoCRTS::pszFld_Power, kCIDLib::True)->c4Id();
    m_c4FldIdSigType = pflddFind(kBarcoCRTS::pszFld_SigType, kCIDLib::True)->c4Id();
    m_c4FldIdVertRate = pflddFind(kBarcoCRTS::pszFld_VertRate, kCIDLib::True)->c4Id();

    // Set a longer reconn and poll time than the default
    SetPollTimes(500, 3000);

    return tCQCKit::EDrvInitRes::WaitCommRes;
}



tCQCKit::ECommResults TBarcoCRTSDriver::ePollDevice(TThread& thrThis)
{
    // Just call the status poll helper methods
    const tBarcoCRTS::EMsgResults eRes = eDoStatusPoll(thrThis);

    // Translate our message result to a comm result
    tCQCKit::ECommResults eRet;
    switch(eRes)
    {
        case tBarcoCRTS::EMsgResults::Ok :
            eRet = tCQCKit::ECommResults::Success;
            break;

        case tBarcoCRTS::EMsgResults::NoReply :
            eRet = tCQCKit::ECommResults::LostConnection;
            break;

        case tBarcoCRTS::EMsgResults::PartialReply :
            IncBadMsgCounter();
            eRet = tCQCKit::ECommResults::LostConnection;
            break;

        case tBarcoCRTS::EMsgResults::BadCheckSum :
        case tBarcoCRTS::EMsgResults::BadCmd :
        case tBarcoCRTS::EMsgResults::SendFailed :
            IncBadMsgCounter();
            eRet = tCQCKit::ECommResults::LostCommRes;
            break;
    };

    if (eRet == tCQCKit::ECommResults::Success)
    {
        // Reset the timeout count
        m_c4Timeout = 0;
    }
     else
    {
        //
        //  Bump the timeout count. If it hits our tolerance level, then
        //  let the false value propogate back. Else just flip it back to
        //  indicate success.
        //
        m_c4Timeout++;
        if (m_c4Timeout < 15)
        {
            eRet = tCQCKit::ECommResults::Success;
        }
         else
        {
            IncTimeoutCounter();
            m_c4Timeout = 0;
        }
    }
    return eRet;
}


tCIDLib::TVoid TBarcoCRTSDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommBarco->bIsOpen())
            m_pcommBarco->Close();
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


tCIDLib::TVoid TBarcoCRTSDriver::TerminateImpl()
{
}


