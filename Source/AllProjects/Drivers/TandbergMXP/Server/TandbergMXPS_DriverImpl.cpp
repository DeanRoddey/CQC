//
// FILE NAME: TandbergMXPS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/31/2008
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
//  This file impelements the main logic of the driver interface. Some of
//  the grunt work is split off into a second file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "TandbergMXPS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTandbergMXPSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local data and types
// ---------------------------------------------------------------------------
namespace TandbergMXPS_DriverImpl
{
}



// ---------------------------------------------------------------------------
//   CLASS: TTandbergMXPSDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWeatherSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TTandbergMXPSDriver::TTandbergMXPSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldId_RemoteKey(kCIDLib::c4MaxCard)
    , m_mbufOut(8192, 0x10000)
    , m_pcommMXP(0)
{
}

TTandbergMXPSDriver::~TTandbergMXPSDriver()
{
    // Clean up the comm port if not already
    if (m_pcommMXP)
    {
        try
        {
            if (m_pcommMXP->bIsOpen())
                m_pcommMXP->Close();

            delete m_pcommMXP;
            m_pcommMXP = 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TTandbergMXPSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TTandbergMXPSDriver::bGetCommResource(TThread&)
{
    try
    {
        // Create the port object if not already, else close just in case
        if (!m_pcommMXP)
            m_pcommMXP = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommMXP->bIsOpen())
            m_pcommMXP->Close();

        // Open the port now, and set the configuration
        m_pcommMXP->Open(tCIDComm::EOpenFlags::WriteThrough);
        m_pcommMXP->SetCfg(m_cpcfgSerial);
    }

    catch(const TError& errToCatch)
    {
        if ((eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium) && bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TTandbergMXPSDriver::bWaitConfig(TThread& thrThis)
{
    return kCIDLib::True;
}


// Handles getting connected to our device
tCQCKit::ECommResults TTandbergMXPSDriver::eConnectToDevice(TThread&)
{
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TTandbergMXPSDriver::eInitializeImpl()
{
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

    const TCQCSerialConnCfg& conncfgSer
        = static_cast<const TCQCSerialConnCfg&>(cqcdcOurs.conncfgReal());
    m_strPort = conncfgSer.strPortPath();
    m_cpcfgSerial = conncfgSer.cpcfgSerial();

    // Clean up any existing port so it gets any new config
    delete m_pcommMXP;
    m_pcommMXP = 0;

    TString strName;
    TCQCFldDef flddTmp;

    // Set up a collection of field defs to register
    TVector<TCQCFldDef> colFlds(16);

    // Send a remote control simulation key. Has to be always write!
    const TString strRemKeyLims
    (
        L"Enum: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Asterisk, Pound, Connect"
        L", Disconnect, Up, Down, Right, Left, Selfview, Layout, Phonebook"
        L", Cancel, MicOff, Presentation, VolumeUp, VolumeDown, OK, ZoomIn"
        L", ZoomOut, Grab, Cabinet"
    );

    flddTmp.Set
    (
        L"RemoteKey"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , strRemKeyLims
        , kCIDLib::True
    );
    colFlds.objAdd(flddTmp);

    // Tell our base class about our fields
    SetFields(colFlds);

    // Look up the field ids for later use

    // Do the call status items
    m_c4FldId_RemoteKey = pflddFind(L"RemoteKey", kCIDLib::True)->c4Id();

    SetPollTimes(5000, 10000);
    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TTandbergMXPSDriver::ePollDevice(TThread&)
{
    try
    {

    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && (eVerboseLevel() >= tCQCKit::EVerboseLvls::High))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TTandbergMXPSDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommMXP->bIsOpen())
            m_pcommMXP->Close();
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
    }
}


// Called when a string field is written to
tCQCKit::ECommResults
TTandbergMXPSDriver::eStringFldValChanged(  const   TString&
                                            , const tCIDLib::TCard4 c4FldId
                                            , const TString&        strNewValue)
{
    if (c4FldId == m_c4FldId_RemoteKey)
    {
        TString strCmd(L"xcommand keypress ");

        if (strNewValue == L"Asterisk")
            strCmd.Append(L"*");
        else if (strNewValue == L"Pound")
            strCmd.Append(L"#");
        else
            strCmd.Append(strNewValue);
        strCmd.Append(L"\r");

        // Transcode it to a memory buffer
        tCIDLib::TCard4 c4ToSend;
        m_tcvtIO.c4ConvertTo(strCmd, m_mbufOut, c4ToSend);

        m_pcommMXP->c4WriteMBufMS(m_mbufOut, c4ToSend, 2000);
    }
     else
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facTandbergMXPS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kTandMXPErrs::errcFld_UnknownFldId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


// Called on driver cleanup
tCIDLib::TVoid TTandbergMXPSDriver::TerminateImpl()
{
    // Nothing to do for this one
}



