//
// FILE NAME: CQCServer_DriverInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/30/2001
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
//  This file implements the simple class we use to store info about each
//  driver that we currently have loaded.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCServer.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TServerDriverInfo,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TServerDriverInfo
//  PREFIX: sdi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TServerDriverInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TServerDriverInfo::TServerDriverInfo(TFacility* const       pfacDriver
                                    , TCQCServerBase* const psdrvDriver) :
    m_pfacDriver(pfacDriver)
    , m_psdrvDriver(psdrvDriver)
{
}

TServerDriverInfo::~TServerDriverInfo()
{
    // We don't own the stuff we point to
}


// ---------------------------------------------------------------------------
//  TServerDriverInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Check to see if the driver is dead yet, meaning the driver is in terminated
//  state and the driver thread is stopped. We don't block here at all.
//
tCIDLib::TBoolean
TServerDriverInfo::bIsDead(const tCIDLib::TCard4 c4WaitMS)
{
    // Shouldn't happen, but just in case
    if (!m_psdrvDriver)
        return kCIDLib::True;

    if (m_psdrvDriver->eState() != tCQCKit::EDrvStates::Terminated)
        return kCIDLib::False;

    return m_psdrvDriver->bWaitTillDead(c4WaitMS);
}


//
//  If the scavenger thread sees our thread with a terminated state, it will
//  call this so we can clean it up. The driver will be all closed down now
//  so we can just delete it.
//
//  Upon return from here the facility class will remove this driver from the
//  driver config and will delete this server info object.
//
tCIDLib::TVoid TServerDriverInfo::DropDriver()
{
    // If no driver set, then we are done. SHouldn't happen but be careful
    if (!m_psdrvDriver)
        return;

    // Get a copy of the driver name to use in any error messages
    const TString strMoniker = m_psdrvDriver->strMoniker();

    // now we can delete the actual driver
    try
    {
        delete m_psdrvDriver;
        m_psdrvDriver = 0;
    }

    catch(const TError& errToCatch)
    {
        if (facCQCServer.bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        if (facCQCServer.bLogFailures())
        {
            facCQCServer.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_ExceptOnDrvCleanup
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker
            );
        }
    }

    catch(...)
    {
        if (facCQCServer.bLogFailures())
        {
            facCQCServer.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_ExceptOnDrvCleanup
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker
            );
        }
    }

    //
    //  Now we can delete the facility. If this was the last reference to
    //  this particular facility, it will be dropped. This is safe now since
    //  no one is going to access it now.
    //
    try
    {
        TJanitor<TFacility> janFac(m_pfacDriver);
        m_pfacDriver = nullptr;
    }

    catch( TError& errToCatch)
    {
        if (facCQCServer.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facCQCServer.bLogFailures())
        {
            facCQCServer.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_ExceptOnFacCleanup
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker
            );
        }
    }

    catch(...)
    {
        if (facCQCServer.bLogFailures())
        {
            facCQCServer.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_ExceptOnFacCleanup
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker
            );
        }
    }
}



// ----------------------------------------------------------------------------
//  TServerDriverInfo: Public, non-virtual methods
// ----------------------------------------------------------------------------
const TFacility& TServerDriverInfo::facDriver() const
{
    CIDAssert(m_pfacDriver != 0, L"Driver facility pointer isn't set in drv info");
    return *m_pfacDriver;
}

TFacility& TServerDriverInfo::facDriver()
{
    CIDAssert(m_pfacDriver != 0, L"Driver facility pointer isn't set in drv info");
    return *m_pfacDriver;
}

const TCQCServerBase& TServerDriverInfo::sdrvDriver() const
{
    CIDAssert(m_psdrvDriver != 0, L"Driver pointer isn't set in drv info");
    return *m_psdrvDriver;
}

TCQCServerBase& TServerDriverInfo::sdrvDriver()
{
    CIDAssert(m_psdrvDriver != 0, L"Driver pointer isn't set in drv info");
    return *m_psdrvDriver;
}



// Tell the driver to start shutting down
tCIDLib::TVoid TServerDriverInfo::StartShutdown()
{
    // Should not happen but check
    if (!m_psdrvDriver)
        return;

    m_psdrvDriver->StartShutdown();
}

