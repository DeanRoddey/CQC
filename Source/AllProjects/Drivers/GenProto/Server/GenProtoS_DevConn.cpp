//
// FILE NAME: GenProtoS_DevConn.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This file implements the device connection abstraction used by this
//  generic driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenProtoS_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TDevConn,TObject)
RTTIDecls(TSerialConn,TDevConn)
RTTIDecls(TSockConn,TDevConn)



// ---------------------------------------------------------------------------
//   CLASS: TDevConn
//  PREFIX: devc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDevConn: Public Destructor
// ---------------------------------------------------------------------------
TDevConn::~TDevConn()
{
}


// ---------------------------------------------------------------------------
//  TDevConn: Hidden constructors
// ---------------------------------------------------------------------------
TDevConn::TDevConn()
{
}




// ---------------------------------------------------------------------------
//   CLASS: TSerialConn
//  PREFIX: devc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSerialConn: Public Destructor
// ---------------------------------------------------------------------------

//
//  We just store away the config info. We don't bother trying to create the
//  serial port object here. We'll do it below in the get com res method,
//  since the com port might not physically be available and we need to
//  keep trying until we get it.
//
TSerialConn::TSerialConn(const TCQCSerialConnCfg& conncfgToUse) :

    m_conncfgToUse(conncfgToUse)
    , m_pcommDev(0)
{
}

TSerialConn::~TSerialConn()
{
    try
    {
        // If we created a serial port, then clean it up
        if (m_pcommDev)
        {
            try
            {
                if (m_pcommDev->bIsOpen())
                    m_pcommDev->Close();
            }

            catch(...)
            {
            }
        }

        delete m_pcommDev;
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TSerialConn: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TSerialConn::bConnect(TThread&, const tCQCKit::EVerboseLvls)
{
    // Nothing to do on this one for us
    return kCIDLib::True;
}


tCIDLib::TBoolean
TSerialConn::bGetCommResource(TThread&, const tCQCKit::EVerboseLvls eVerbose)
{
    try
    {
        // If we've not created the object so far, then try it
        if (!m_pcommDev)
        {
            m_pcommDev = facCIDComm().pcommMakeNew(m_conncfgToUse.strPortPath());

            // If still not, then log if verbose
            if (!m_pcommDev)
            {
                if (eVerbose > tCQCKit::EVerboseLvls::Medium)
                {
                    facCIDComm().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCommErrs::errcPort_Open
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , m_conncfgToUse.strPortPath()
                    );
                }
                return kCIDLib::False;
            }
        }

        m_pcommDev->Open(tCIDComm::EOpenFlags::WriteThrough, 8192);
        m_pcommDev->SetCfg(m_conncfgToUse.cpcfgSerial());

        // Purge any partial gorp we might have waiting
        m_pcommDev->PurgeReadData();
    }

    catch(TError& errToCatch)
    {
        if ((eVerbose > tCQCKit::EVerboseLvls::Medium) && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (m_pcommDev)
        {
            try
            {
                m_pcommDev->Close();
            }

            catch(...)
            {
            }
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TSerialConn::bIsConnected() const
{
    // Just say yes, we can't lose the connection once made
    return kCIDLib::True;
}


tCIDLib::TBoolean
TSerialConn::bReadByte( const   tCIDLib::TCard4     c4WaitFor
                        ,       tCIDLib::TCard1&    c1ToFill)
{
    // If not created yet, then obvious we failed
    if (!m_pcommDev)
        return kCIDLib::False;

    if (!m_pcommDev->c4ReadRawBufMS(&c1ToFill, 1, c4WaitFor))
        return kCIDLib::False;
    return kCIDLib::True;
}


tCIDLib::TBoolean
TSerialConn::bWrite(TMemBuf& mbufToFill, const tCIDLib::TCard4 c4ToWrite)
{
    // If not created yet, then obvious we failed
    if (!m_pcommDev)
        return kCIDLib::False;

    const tCIDLib::TCard4 c4Written = m_pcommDev->c4WriteRawBufMS
    (
        mbufToFill.pc1Data(), c4ToWrite, 20 * c4ToWrite
    );
    return (c4Written == c4ToWrite);
}


tCIDLib::TCard4
TSerialConn::c4Read(        TMemBuf&        mbufToFill
                    , const tCIDLib::TCard4 c4MaxToRead
                    , const tCIDLib::TCard4 c4Wait)
{
    // If not created yet, then obvious we failed
    if (!m_pcommDev)
        return 0;
    return m_pcommDev->c4ReadMBufMS(mbufToFill, c4MaxToRead, c4Wait);
}


tCIDLib::TVoid TSerialConn::PurgeReadBuf()
{
    if (m_pcommDev)
        m_pcommDev->PurgeReadData();
}


tCIDLib::TVoid
TSerialConn::ReleaseCommResource(const tCQCKit::EVerboseLvls eVerbose)
{
    try
    {
        // If we've created the object and it's open, then close
        if (m_pcommDev && m_pcommDev->bIsOpen())
            m_pcommDev->Close();
    }

    catch(TError& errToCatch)
    {
        if ((eVerbose > tCQCKit::EVerboseLvls::Medium) && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    catch(...)
    {
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TSockConn
//  PREFIX: devc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSockConn: Public Destructor
// ---------------------------------------------------------------------------
TSockConn::TSockConn(const TCQCIPConnCfg& conncfgToUse) :

    m_conncfgSock(conncfgToUse)
    , m_sockDev(conncfgToUse.eProto(), m_conncfgSock.ipepConn().eAddrType())
{
}

TSockConn::~TSockConn()
{
    // Cleanly shutdown the socket if needed
    try
    {
        if (m_sockDev.bIsConnected())
            m_sockDev.c4Shutdown();
    }

    catch(...) {}

    // And close it
    try
    {
        m_sockDev.Close();
    }

    catch(...) {}
}


// ---------------------------------------------------------------------------
//  TSockConn: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TSockConn::bConnect(TThread& thrThis, const tCQCKit::EVerboseLvls eVerbose)
{
    try
    {
        // Try to connect to the remote end point
        m_sockDev.Connect(m_conncfgSock.ipepConn());
    }

    catch(TError& errToCatch)
    {
        if ((eVerbose > tCQCKit::EVerboseLvls::Medium) && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TSockConn::bGetCommResource(TThread&, const tCQCKit::EVerboseLvls eVerbose)
{
    try
    {
        // If not open, then open it
        if (!m_sockDev.bIsOpen())
        {
            m_sockDev.Open
            (
                m_conncfgSock.eProto(), m_conncfgSock.ipepConn().eAddrType()
            );
        }
    }

    catch(TError& errToCatch)
    {
        if ((eVerbose > tCQCKit::EVerboseLvls::Medium) && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    catch(...)
    {
        // Not much we can do, so eat it
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean TSockConn::bIsConnected() const
{
    return m_sockDev.bIsConnected();
}


tCIDLib::TBoolean
TSockConn::bReadByte(const  tCIDLib::TCard4     c4WaitFor
                    ,       tCIDLib::TCard1&    c1ToFill)
{
    const tCIDLib::TCard4 c4Cnt = m_sockDev.c4ReceiveRawTOMS
    (
        &c1ToFill, c4WaitFor, 1, tCIDLib::EAllData::OkIfNotAll
    );
    return (c4Cnt == 1);
}


tCIDLib::TBoolean
TSockConn::bWrite(TMemBuf& mbufToFill, const tCIDLib::TCard4 c4ToWrite)
{
    m_sockDev.Send(mbufToFill.pc1Data(), c4ToWrite);
    return kCIDLib::True;
}


tCIDLib::TCard4
TSockConn::c4Read(          TMemBuf&        mbufToFill
                    , const tCIDLib::TCard4 c4MaxToRead
                    , const tCIDLib::TCard4 c4Wait)
{
    return m_sockDev.c4ReceiveMBufTOMS(mbufToFill, c4Wait, c4MaxToRead);
}


tCIDLib::TVoid TSockConn::PurgeReadBuf()
{
    // <TBD>
}


tCIDLib::TVoid
TSockConn::ReleaseCommResource(const tCQCKit::EVerboseLvls eVerbose)
{
    // First do a clean shutdown
    try
    {
        m_sockDev.c4Shutdown();
    }

    catch(TError& errToCatch)
    {
        if ((eVerbose > tCQCKit::EVerboseLvls::Medium) && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    catch(...)
    {
    }

    // And close it
    try
    {
        m_sockDev.Close();
    }

    catch(TError& errToCatch)
    {
        if ((eVerbose > tCQCKit::EVerboseLvls::Medium) && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    catch(...)
    {
    }
}


