//
// FILE NAME: JAPwrSer_PortFactory.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2013
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
//  This file implements the port factory for our Just Add Power based ports.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "JAPwrSer_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TJAPwrPortFactory,TComPortFactory)



// ---------------------------------------------------------------------------
//  CLASS: TJAPwrPortFactory
// PREFIX: pfact
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TJAPwrPortFactory: Constructors and Destructor
// ---------------------------------------------------------------------------

// Tell our parent these ports are not configurable by us
TJAPwrPortFactory::TJAPwrPortFactory(const TJAPwrCfgList& japlSrc) :

    TComPortFactory(kJAPwrSer::pszFactoryId, kJAPwrSer::pszPrefix, kCIDLib::False)
    , m_japlCfg(japlSrc)
{
}

TJAPwrPortFactory::~TJAPwrPortFactory()
{
}


// ---------------------------------------------------------------------------
//  TJAPwrPortFactory: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We put all the ports we support into the passed collection. We don't
//  clear it first since the facility class is building up a list from all
//  the installed factories.
//
tCIDLib::TBoolean
TJAPwrPortFactory::bQueryPorts(         tCIDLib::TStrCollect& colToFill
                                , const tCIDLib::TBoolean     bAvailOnly) const
{
    try
    {
        //
        //  Iterate all the JAPs and ask each one for the ports it supports.
        //  Build up the names of them in each into the list.
        //
        TString strPath;
        const tCIDLib::TCard4 c4Count = m_japlCfg.c4Count();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TJAPwrCfg& japcfgCur = m_japlCfg.japcfgAt(c4Index);

            //
            //  Build up the part of the path specific to this device, and
            //  with the COM part. After that we just need to add the
            //  port number for each supported port.
            //
            strPath = kJAPwrSer::pszPrefix;
            strPath.Append(japcfgCur.strName());
            strPath.Append(L"/COM");


            // Remember the length so we can truncate back again
            const tCIDLib::TCard4 c4BaseLen = strPath.c4Length();

            // Get the port bit mask and do any that are set
            const tCIDLib::TCard4 c4Mask = japcfgCur.c4Ports();
            for (tCIDLib::TCard4 c4PInd = 0; c4PInd < kJAPwrSer::c4MaxPortsPer; c4PInd++)
            {
                if ((0x1UL << c4PInd) & c4Mask)
                {
                    strPath.CapAt(c4BaseLen);
                    strPath.AppendFormatted(c4PInd + 1);

                    // And return this one
                    colToFill.objAdd(strPath);
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (facJAPwrSer().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return !colToFill.bIsEmpty();
}


//
//  Our limitations are pretty high. We ignore any control line stuff since
//  they will often be set in the manifest, just to make sure the other side
//  in a full serial connection is happy.
//
tCIDLib::TBoolean
TJAPwrPortFactory::bTestCfg(const   TCommPortCfg&   cpcfgTest
                            ,       TString&        strReason) const
{
    if (cpcfgTest.eDataBits() != tCIDComm::EDataBits::Eight)
    {
        strReason = facJAPwrSer().strMsg
        (
            kJAPwrErrs::errcTest_BadDataBits
            , TCardinal(tCIDLib::c4EnumOrd(cpcfgTest.eDataBits()))
        );
        return kCIDLib::False;
    }

    if (!TJAPwrCommPort::bCheckBaud(cpcfgTest.c4Baud()))
    {
        strReason = facJAPwrSer().strMsg
        (
            kJAPwrErrs::errcTest_BadBaud, TCardinal(cpcfgTest.c4Baud())
        );
        return kCIDLib::False;
    }

    if ((cpcfgTest.eParity() != tCIDComm::EParities::None)
    &&  (cpcfgTest.eParity() != tCIDComm::EParities::Even)
    &&  (cpcfgTest.eParity() != tCIDComm::EParities::Odd))
    {
        strReason = facJAPwrSer().strMsg
        (
            kJAPwrErrs::errcTest_BadParity
            , tCIDComm::strXlatEParities(cpcfgTest.eParity())
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TJAPwrPortFactory::bValidatePath(const TString& strPath) const
{
    tCIDLib::TCard4 c4Dummy;
    TString         strName;
    return facJAPwrSer().bParsePath(strPath, strName, c4Dummy);
}


TCommPortBase* TJAPwrPortFactory::pcommMakeNew(const TString& strPath)
{
    // See if this is a valid path. If not return null
    tCIDLib::TCard4 c4PortNum;
    TString strName;
    if (!facJAPwrSer().bParsePath(strPath, strName, c4PortNum))
        return nullptr;

    // See if we have this named device in our list
    TCommPortBase* pcommRet = 0;

    const tCIDLib::TCard4 c4Count = m_japlCfg.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TJAPwrCfg& japcfgCur = m_japlCfg.japcfgAt(c4Index);

        if (japcfgCur.strName().bCompareI(strName))
        {
            //
            //  See if this port number is valid. If not, then break out
            //  with the null pointer.
            //
            if (((0x1UL << (c4PortNum - 1)) & japcfgCur.c4Ports()) == 0)
                break;

            // This is our guy, so create a port
            pcommRet = new TJAPwrCommPort
            (
                strPath, strName, japcfgCur.strAddr(), c4PortNum
            );

            // And break out now
            break;
        }
    }

    // Return either null or the port we created
    return pcommRet;
}


//
//  If the user changes configuration, the new config has to be downloaed
//  and updated on this factory, so that the new ports are available.
//
tCIDLib::TVoid TJAPwrPortFactory::UpdateConfig(const TJAPwrCfgList& japlToSet)
{
    m_japlCfg = japlToSet;
}


