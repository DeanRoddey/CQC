//
// FILE NAME: PCIIROS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/19/2006
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
//  This is a test driver which implements driver interface for the driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "PCIIROS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TPCIIROSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace PCIIROS_DriverImpl
{
    // -----------------------------------------------------------------------
    //  The registry keys that hold the items for the registered I/O boards.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszIOKey = L"Software\\PCIFIND\\NTioPCI\\Parameters";
}



// ---------------------------------------------------------------------------
//   CLASS: TPCIIROSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TPCIIROSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TPCIIROSDriver::TPCIIROSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c1HighContact(0)
    , m_c1HighDig(0)
    , m_c1LowContact(0)
    , m_c1LowDig(0)
    , m_c4Count(kCIDLib::False)
    , m_c4BaseAddr(0)
{
}

TPCIIROSDriver::~TPCIIROSDriver()
{
}


// ---------------------------------------------------------------------------
//  TPCIIROSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TPCIIROSDriver::bGetCommResource(TThread&)
{
    // Do one forced poll, to make sure our cached bits and fields are in sync
    PollBits(kCIDLib::True);
    return kCIDLib::True;
}


tCIDLib::TBoolean TPCIIROSDriver::bWaitConfig(TThread&)
{
    // We don't have any config
    return kCIDLib::True;
}


// One of our boolean fields (a contact closure) was written to
tCQCKit::ECommResults
TPCIIROSDriver::eBoolFldValChanged( const   TString&            strName
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TBoolean   bNewValue)
{
    //
    //  Figure out which contact closure. We insured below that the contacts
    //  have their own, contiguous range of field ids. So we can quickly
    //  figure out if it's a valid field id and which index it is.
    //
    if ((c4FldId < m_ac4Fld_Contacts[0])
    ||  (c4FldId > m_ac4Fld_Contacts[m_c4Count - 1]))
    {
        // Can't be valid since it's not in our range
        return tCQCKit::ECommResults::Internal;
    }

    // Figure out the index
    tCIDLib::TCard4 c4Index = c4FldId - m_ac4Fld_Contacts[0];

    //
    //  Figure out what address we need to read/write, and adjust the
    //  index down if we are doing the high 8, so that the subsequent code
    //  will work the same for high or low bytes.
    //
    tCIDLib::TCard4 c4Addr;
    if (c4Index < 8)
    {
        c4Addr = m_c4BaseAddr;
    }
     else
    {
        c4Addr = m_c4BaseAddr + 4;
        c4Index -= 8;
    }

    // Query the current value of the register
    tCIDLib::TCard1 c1CurBits = tCIDLib::TCard1(InPortB(c4Addr));
    c1CurBits &= ~(0x1 << c4Index);
    if (bNewValue)
        c1CurBits |= (0x1 << c4Index);
    OutPortB(c4Addr, c1CurBits);

    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TPCIIROSDriver::eConnectToDevice(TThread&)
{
    // We are always 'connected' once we get our comm resource.
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TPCIIROSDriver::eInitializeImpl()
{
    //
    //  We need to check the registry and see if there are any keys under
    //  the scope that holds the AccesIO board entries.
    //
    tCIDKernel::TWRegHandle hkeyIO = TKrnlWin32Registry::hkeyGetRootSubKey
    (
        tCIDKernel::ERootKeys::LocalMachine
        , PCIIROS_DriverImpl::pszIOKey
        , tCIDKernel::ERegAccFlags::ReadOnly
    );

    if (!hkeyIO)
    {
        // Key doesn't exist, so we cannot load
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facPCIIROS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kPCIIROErrs::errcCard_NoRegistry
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    // Read the number of entries
    tCIDLib::TCard4 c4Count;
    if (!TKrnlWin32Registry::bQueryCardValue(hkeyIO, L"NumDevices", c4Count)
    || (c4Count < 1))
    {
        TKrnlWin32Registry::bCloseKey(hkeyIO);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facPCIIROS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kPCIIROErrs::errcCard_EmptyRegistry
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    // There is at least one, so read in the list of configurations
    tCIDLib::TCard4         c4ActualSz;
    tCIDKernel::ERegDTypes  eType;
    const tCIDLib::TCard4   c4MaxConfig = 64;
    TPCI_COMMON_CONFIG      aConfig[c4MaxConfig];
    const tCIDLib::TBoolean bRes = TKrnlWin32Registry::bQueryValue
    (
        hkeyIO
        , L"PCICommonConfig"
        , eType
        , sizeof(TPCI_COMMON_CONFIG) * c4Count
        , reinterpret_cast<tCIDLib::TCard1*>(aConfig)
        , c4ActualSz
    );

    // If none, or we got a bad read, then we are done
    if (!bRes || (c4ActualSz != (sizeof(TPCI_COMMON_CONFIG) * c4Count)))
    {
        TKrnlWin32Registry::bCloseKey(hkeyIO);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facPCIIROS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kPCIIROErrs::errcCard_InvalidRegistry
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , bRes ? TString(L"Size was wrong") : TString(L"Registry read failed")
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    // Ok, we are done with the registry key so close it
    TKrnlWin32Registry::bCloseKey(hkeyIO);

    // Now search the list for a board of a type we support
    m_c4BaseAddr = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (aConfig[c4Index].VendorID == 0x494F)
        {
            if ((aConfig[c4Index].DeviceID == 0x0F00)
            ||  (aConfig[c4Index].DeviceID == 0x0F08))
            {
                // Take the base address and remember the type
                m_c4BaseAddr = aConfig[c4Index].BaseAddresses[2] & 0xFFFE;
                m_c4Count = (aConfig[c4Index].DeviceID == 0x0F00) ? 8 : 16;

                // And we are done
                break;
            }
        }
    }

    // If we never found one, then give up
    if (!m_c4BaseAddr)
    {
        // It's not a supported board type
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facPCIIROS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kPCIIROErrs::errcCard_NoCardFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    // Ok, based on the type, register our fields
    TRawMem::SetMemBuf(m_ac4Fld_DigIns, tCIDLib::TCard4(0), kPCIIROS::c4MaxPoints);
    TRawMem::SetMemBuf(m_ac4Fld_Contacts, tCIDLib::TCard4(0), kPCIIROS::c4MaxPoints);

    TSetFieldList   colFlds((m_c4Count * 2) + 16);
    TCQCFldDef      flddNew;
    TString         strName;

    //
    //  We have to do two separate loops here, so that the digitals and contacts
    //  will have two separate, contiguous ranges of fields ids. This lets us
    //  search the contact closure list for a given incoming field id on a
    //  field write and figure out the index.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4Count; c4Index++)
    {
        // Do a digital
        strName = L"DigIn";
        if (c4Index + 1 < 10)
            strName.Append(L"0");
        strName.AppendFormatted(c4Index + 1);
        flddNew.Set
        (
            strName
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::Read
        );
        colFlds.objAdd(flddNew);
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4Count; c4Index++)
    {
        // Do a contact closure
        strName = L"Contact";
        if (c4Index + 1 < 10)
            strName.Append(L"0");
        strName.AppendFormatted(c4Index + 1);
        flddNew.Set
        (
            strName
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::ReadWrite
        );
        colFlds.objAdd(flddNew);
    }

    // Register the board type field
    flddNew.Set
    (
        L"BoardType"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    //
    //  Register the fields with our parent class and look up the ids for
    //  the I/O fields.
    //
    SetFields(colFlds);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4Count; c4Index++)
    {
        // Add a digital input for this index
        strName = L"DigIn";
        if (c4Index + 1 < 10)
            strName.Append(L"0");
        strName.AppendFormatted(c4Index + 1);
        m_ac4Fld_DigIns[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = L"Contact";
        if (c4Index + 1 < 10)
            strName.Append(L"0");
        strName.AppendFormatted(c4Index + 1);
        m_ac4Fld_Contacts[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();
    }

    // We can set the board type field here and never worry about it again.
    strName = L"PCI-IIRO-";
    strName.AppendFormatted(m_c4Count);
    bStoreStringFld(L"BoardType", strName, kCIDLib::True);

    //
    //  We can set the poll period very quick here because our polling
    //  is extremely low overhead. The reconnect doesn't matter since we
    //  either connect or the driver doesn't load.
    //
    SetPollTimes(50, 1000);
    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TPCIIROSDriver::ePollDevice(TThread&)
{
    // Do a non-forced poll
    PollBits(kCIDLib::False);
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TPCIIROSDriver::ReleaseCommResource()
{
    // Nothing to do
}


tCIDLib::TVoid TPCIIROSDriver::TerminateImpl()
{
    // Nothing to do
}


// ---------------------------------------------------------------------------
//  TPCIIROSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to poll all the stuff for the board type we have. The caller
//  call force an update, since normally we won't write to the fields unless
//  the values we read have changed from the last time (and the vast majority
//  of the time they will not have.
//
tCIDLib::TVoid TPCIIROSDriver::PollBits(const tCIDLib::TBoolean bForce)
{
    tCIDLib::TCard1 c1Bits;

    // Do the low 8 digital inputs
    c1Bits = tCIDLib::TCard1(InPortB(m_c4BaseAddr + 1));
    if ((c1Bits != m_c1LowDig) || bForce)
    {
        StoreBits(c1Bits, m_ac4Fld_DigIns, 0);
        m_c1LowDig = c1Bits;
    }

    // If we have the high 8 digitals, do those
    if (m_c4Count == 16)
    {
        c1Bits = tCIDLib::TCard1(InPortB(m_c4BaseAddr + 5));
        StoreBits(c1Bits, m_ac4Fld_DigIns, 8);
        m_c1HighDig = c1Bits;
    }

    // Do the low 8 contacts
    c1Bits = tCIDLib::TCard1(InPortB(m_c4BaseAddr));
    if ((c1Bits != m_c1LowDig) || bForce)
    {
        StoreBits(c1Bits, m_ac4Fld_Contacts, 0);
        m_c1LowContact = c1Bits;
    }

    // If we have the high 8 contacts, do those
    if (m_c4Count == 16)
    {
        c1Bits = tCIDLib::TCard1(InPortB(m_c4BaseAddr + 4));
        StoreBits(c1Bits, m_ac4Fld_Contacts, 8);
        m_c1HighContact = c1Bits;
    }
}


//
//  The caller provides us the bits read, a pointer to the list of field ids
//  for the particular set of fields being polled, and the start index into
//  that list. It'll be either 0 or 8, i.e. either the low 8 values or the
//  high 8 values if this is a 16 way board.
//
tCIDLib::TVoid
TPCIIROSDriver::StoreBits(  const   tCIDLib::TCard1     c1Bits
                            , const tCIDLib::TCard4*    pac4FldIds
                            , const tCIDLib::TCard4     c4StartInd)
{
    // And update the fields
    for (tCIDLib::TCard4 c4Index = 0; c4Index < 8; c4Index++)
    {
        bStoreBoolFld
        (
            pac4FldIds[c4Index + c4StartInd]
            , (c1Bits & (0x1 << c4Index)) ? kCIDLib::True : kCIDLib::False
            , kCIDLib::True
        );
    }
}

