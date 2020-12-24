//
// FILE NAME: GC100Port_PortFactory.cpp
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
//  This file implements the port factory for our GC based ports.
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
#include    "GC100Ser_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TGC100PortFactory,TComPortFactory)



// ---------------------------------------------------------------------------
//  CLASS: TGC100PortFactory
// PREFIX: pfact
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGC100PortFactory: Constructors and Destructor
// ---------------------------------------------------------------------------

// Tell our parent these ports are not configurable by us
TGC100PortFactory::TGC100PortFactory(const TGC100CfgList& gcclSrc) :

    TComPortFactory(kGC100Ser::pszFactoryId, kGC100Ser::pszPrefix, kCIDLib::False)
    , m_gcclCfg(gcclSrc)
{
}

TGC100PortFactory::~TGC100PortFactory()
{
}


// ---------------------------------------------------------------------------
//  TGC100PortFactory: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We put all the ports we support into the passed collection. We don't
//  clear it first since the facility class is building up a list from all
//  the installed factories.
//
tCIDLib::TBoolean
TGC100PortFactory::bQueryPorts(         tCIDLib::TStrCollect& colToFill
                                , const tCIDLib::TBoolean     bAvailOnly) const
{
    try
    {
        //
        //  Iterate all the GCs and ask each one for the ports it supports.
        //  Build up the names of them in each into the list.
        //
        TString strPath;
        const tCIDLib::TCard4 c4Count = m_gcclCfg.c4Count();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TGC100Cfg& gccfgCur = m_gcclCfg.gccfgAt(c4Index);

            //
            //  Build up the part of the path specific to this GC, and
            //  with the COM part. After that we just need to add the
            //  port number for each supported port.
            //
            strPath = kGC100Ser::pszPrefix;
            strPath.Append(gccfgCur.strName());
            strPath.Append(L"/COM");


            // Remember the length so we can truncate back again
            const tCIDLib::TCard4 c4BaseLen = strPath.c4Length();

            // Get the port bit mask and do any that are set
            const tCIDLib::TCard4 c4Mask = gccfgCur.c4Ports();
            for (tCIDLib::TCard4 c4PInd = 0; c4PInd < kGC100Ser::c4MaxPortsPer; c4PInd++)
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

    catch(const TError& errToCatch)
    {
        if (facGC100Ser().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
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
TGC100PortFactory::bTestCfg(const   TCommPortCfg&   cpcfgTest
                            ,       TString&        strReason) const
{
    if (cpcfgTest.eDataBits() != tCIDComm::EDataBits::Eight)
    {
        strReason = facGC100Ser().strMsg
        (
            kGC100Errs::errcTest_BadDataBits
            , TCardinal(tCIDLib::c4EnumOrd(cpcfgTest.eDataBits()))
        );
        return kCIDLib::False;
    }

    if (!TGC100CommPort::bCheckBaud(cpcfgTest.c4Baud()))
    {
        strReason = facGC100Ser().strMsg
        (
            kGC100Errs::errcTest_BadBaud, TCardinal(cpcfgTest.c4Baud())
        );
        return kCIDLib::False;
    }

    if ((cpcfgTest.eParity() != tCIDComm::EParities::None)
    &&  (cpcfgTest.eParity() != tCIDComm::EParities::Even)
    &&  (cpcfgTest.eParity() != tCIDComm::EParities::Odd))
    {
        strReason = facGC100Ser().strMsg
        (
            kGC100Errs::errcTest_BadParity
            , tCIDComm::strXlatEParities(cpcfgTest.eParity())
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TGC100PortFactory::bValidatePath(const TString& strPath) const
{
    tCIDLib::TCard4 c4Dummy;
    TString         strName;
    return facGC100Ser().bParsePath(strPath, strName, c4Dummy);
}


TCommPortBase* TGC100PortFactory::pcommMakeNew(const TString& strPath)
{
    // See if this is a valid path. If not return null
    tCIDLib::TCard4 c4PortNum;
    TString strName;
    if (!facGC100Ser().bParsePath(strPath, strName, c4PortNum))
        return nullptr;

    // See if we have this named GC in our list
    TCommPortBase* pcommRet = 0;

    const tCIDLib::TCard4 c4Count = m_gcclCfg.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TGC100Cfg& gccfgCur = m_gcclCfg.gccfgAt(c4Index);

        if (gccfgCur.strName().bCompareI(strName))
        {
            //
            //  See if this port number is valid. If not, then break out
            //  with the null pointer.
            //
            if (((0x1UL << (c4PortNum - 1)) & gccfgCur.c4Ports()) == 0)
                break;

            // This is our guy, so create a port
            pcommRet = new TGC100CommPort
            (
                strPath, strName, gccfgCur.strAddr(), c4PortNum
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
tCIDLib::TVoid TGC100PortFactory::UpdateConfig(const TGC100CfgList& gcclToSet)
{
    m_gcclCfg = gcclToSet;
}


