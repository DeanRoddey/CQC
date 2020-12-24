//
// FILE NAME: VantageQS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2006
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
//  This is a driver which implements the Vantage Q series interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "VantageQS.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TVantageQSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//   CLASS: TVantageQSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TVantageQSDriver::TVantageQSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_mbufWrite(1024, 8192)
    , m_pcommVantage(0)
    , m_strSepComma(L",")
    , m_strSepSpace(L" ")
{
}

TVantageQSDriver::~TVantageQSDriver()
{
    // Clean up the comm port if not already
    if (m_pcommVantage)
    {
        try
        {
            if (m_pcommVantage->bIsOpen())
                m_pcommVantage->Close();
            delete m_pcommVantage;
            m_pcommVantage = 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TVantageQSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TVantageQSDriver::bGetCommResource(TThread& thrThis)
{
    try
    {
        // Create the port if needed, else close it just in case
        if (!m_pcommVantage)
            m_pcommVantage = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommVantage->bIsOpen())
            m_pcommVantage->Close();

        // Open the port now, and set the configuration
        m_pcommVantage->Open(tCIDComm::EOpenFlags::WriteThrough);
        m_pcommVantage->SetCfg(m_cpcfgSerial);
    }

    catch(const TError&)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TVantageQSDriver::bWaitConfig(TThread& thrThis)
{
    // Just say we are happy
    return kCIDLib::True;
}


tCQCKit::ECommResults
TVantageQSDriver::eBoolFldValChanged(const   TString&            strName
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TBoolean   bNewValue)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::FieldNotFound;
}


tCQCKit::ECommResults TVantageQSDriver::eConnectToDevice(TThread& thrThis)
{
    try
    {
        ProbeConfig();
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TVantageQSDriver::eInitializeImpl()
{
    //
    //  Make sure that we were configured for a serial connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCSerialConnCfg::clsThis())
    {
        facCQCKit().LogMsg
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
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    const TCQCSerialConnCfg& conncfgSer
        = static_cast<const TCQCSerialConnCfg&>(cqcdcOurs.conncfgReal());
    m_strPort = conncfgSer.strPortPath();
    m_cpcfgSerial = conncfgSer.cpcfgSerial();

    // Delete any existing port so it will get redone with new config
    delete m_pcommVantage;
    m_pcommVantage = 0;

    SetPollTimes(250, 5000);
    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults
TVantageQSDriver::eIntFldValChanged( const   TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TInt4  i4NewValue)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::FieldNotFound;
}


tCQCKit::ECommResults TVantageQSDriver::ePollDevice(TThread& thrThis)
{
    try
    {
    }

    catch(const TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            TModule::LogEventObj(errToCatch);
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TVantageQSDriver::eStringFldValChanged(  const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::FieldNotFound;
}


tCIDLib::TVoid TVantageQSDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommVantage->bIsOpen())
            m_pcommVantage->Close();
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


tCIDLib::TVoid TVantageQSDriver::TerminateImpl()
{
}


