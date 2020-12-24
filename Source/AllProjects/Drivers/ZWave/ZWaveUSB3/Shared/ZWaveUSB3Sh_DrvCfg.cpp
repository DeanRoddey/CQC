//
// FILE NAME: ZWaveUSB3Sh_DrvCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2017
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
//  This file implements the TZWaveDrvCfg class, which is just a container class to
//  hold all our configuration and handle streaming it in and out.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveDrvCfg,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_DrvCfg
{
    // -----------------------------------------------------------------------
    //  Our persistent format version
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtVersion = 1;
}




// ---------------------------------------------------------------------------
//   CLASS: TZWaveDrvCfg
//  PREFIX: dcfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveDrvCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveDrvCfg::TZWaveDrvCfg() :

    m_bsUnitMap(kZWaveUSB3Sh::c4MaxUnits)
{
    // Call the reset method to get everything set to defaults
    Reset();
}

TZWaveDrvCfg::TZWaveDrvCfg(const TZWaveDrvCfg& dcfgSrc) :

    m_bInNetwork(dcfgSrc.m_bInNetwork)
    , m_bIsSUC(dcfgSrc.m_bIsSUC)
    , m_bsUnitMap(dcfgSrc.m_bsUnitMap)
    , m_c1CtrlId(dcfgSrc.m_c1CtrlId)
    , m_c1MasterId(dcfgSrc.m_c1MasterId)
    , m_c4HomeId(dcfgSrc.m_c4HomeId)
    , m_c4SerAPIVersion(dcfgSrc.m_c4SerAPIVersion)
    , m_c4ZWLibMajVer(dcfgSrc.m_c4ZWLibMajVer)
    , m_c4ZWLibMinVer(dcfgSrc.m_c4ZWLibMinVer)
    , m_c8ManIds(dcfgSrc.m_c8ManIds)
    , m_colUnits(dcfgSrc.m_colUnits)
    , m_strCtrlModel(dcfgSrc.m_strCtrlModel)
    , m_strCtrlName(dcfgSrc.m_strCtrlName)
    , m_strFWVersion(dcfgSrc.m_strFWVersion)
{
}

TZWaveDrvCfg::~TZWaveDrvCfg()
{
}


// ---------------------------------------------------------------------------
//  TZWaveDrvCfg: Public operators
// ---------------------------------------------------------------------------
TZWaveDrvCfg& TZWaveDrvCfg::operator=(const TZWaveDrvCfg& dcfgSrc)
{
    if (this != &dcfgSrc)
    {
        m_bInNetwork    = dcfgSrc.m_bInNetwork;
        m_bIsSUC        = dcfgSrc.m_bIsSUC;
        m_bsUnitMap     = dcfgSrc.m_bsUnitMap;
        m_c1CtrlId      = dcfgSrc.m_c1CtrlId;
        m_c1MasterId    = dcfgSrc.m_c1MasterId;
        m_c4HomeId      = dcfgSrc.m_c4HomeId;
        m_c4ZWLibMajVer = dcfgSrc.m_c4ZWLibMajVer;
        m_c4ZWLibMinVer = dcfgSrc.m_c4ZWLibMinVer;
        m_c8ManIds      = dcfgSrc.m_c8ManIds;
        m_colUnits      = dcfgSrc.m_colUnits;
        m_strCtrlModel  = dcfgSrc.m_strCtrlModel;
        m_strCtrlName   = dcfgSrc.m_strCtrlName;
        m_strFWVersion  = dcfgSrc.m_strFWVersion;
    }
    return *this;
}

tCIDLib::TBoolean TZWaveDrvCfg::operator==(const TZWaveDrvCfg& dcfgSrc) const
{
    if (this != &dcfgSrc)
    {
        if ((m_bInNetwork != dcfgSrc.m_bInNetwork)
        ||  (m_bIsSUC != dcfgSrc.m_bIsSUC)
        ||  (m_bsUnitMap != dcfgSrc.m_bsUnitMap)
        ||  (m_c1CtrlId != dcfgSrc.m_c1CtrlId)
        ||  (m_c1MasterId != dcfgSrc.m_c1MasterId)
        ||  (m_c4HomeId != dcfgSrc.m_c4HomeId)
        ||  (m_c4ZWLibMajVer != dcfgSrc.m_c4ZWLibMajVer)
        ||  (m_c4ZWLibMinVer != dcfgSrc.m_c4ZWLibMinVer)
        ||  (m_c8ManIds != dcfgSrc.m_c8ManIds)
        ||  (m_strCtrlModel != dcfgSrc.m_strCtrlModel)
        ||  (m_strCtrlName != dcfgSrc.m_strCtrlName)
        ||  (m_strFWVersion != dcfgSrc.m_strFWVersion))
        {
            return kCIDLib::False;
        }

        // Check the units
        tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
        if (c4Count != dcfgSrc.m_colUnits.c4ElemCount())
            return kCIDLib::False;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colUnits[c4Index] != dcfgSrc.m_colUnits[c4Index])
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TZWaveDrvCfg::operator!=(const TZWaveDrvCfg& dcfgSrc) const
{
    return !operator==(dcfgSrc);
}


// ---------------------------------------------------------------------------
//  TZWaveDrvCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Add the passed unit to our list, sorted by id. We insure that neither its name or
//  unit id is already in the list, and that it is present in the unit id map.
//
tCIDLib::TBoolean TZWaveDrvCfg::bAddUnit(const TZWUnitInfo& unitiToAdd)
{
    const tCIDLib::TCard1 c1Id = unitiToAdd.c1Id();

    // Make sure it's correctly set up
    if ((c1Id == kCIDLib::c1MaxCard) || (c1Id == 0))
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_UnitNotCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Make sure it is in our unit map or the caller is really messed up
    if (!m_bsUnitMap[c1Id -1])
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_NotInUnitMap
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , TCardinal(c1Id)
        );
    }

    // Make sure that it doesn't already exist by either name or id
    if (punitiFindById(c1Id) || punitiFindByName(unitiToAdd.strName()))
        return kCIDLib::False;

    // Looks ok, so add it, in sorted unit id order
    tCIDLib::TCard4 c4At;
    m_colUnits.InsertSorted(unitiToAdd, &TZWUnitInfo::eCompById, c4At);

    //
    //  If it's our own controller or the master, disable it. This may not work for
    //  the master since we may not know it at this point. We'll do another check up
    //  at a higher level to try to catch that.
    //
    if ((c1Id == m_c1CtrlId) || (c1Id == m_c1MasterId))
        m_colUnits[c4At].eState(tZWaveUSB3Sh::EUnitStates::Disabled);

    return kCIDLib::True;
}


//
//  Get/set the in-network flag.
//
tCIDLib::TBoolean TZWaveDrvCfg::bInNetwork() const
{
    return m_bInNetwork;
}

tCIDLib::TBoolean TZWaveDrvCfg::bInNetwork(const tCIDLib::TBoolean bToSet)
{
    m_bInNetwork = bToSet;
    return m_bInNetwork;
}


// Returns true if we have no units
tCIDLib::TBoolean TZWaveDrvCfg::bIsEmpty() const
{
    return m_colUnits.bIsEmpty();
}


// Scan the units and see if any are in wait for approval state
tCIDLib::TBoolean TZWaveDrvCfg::bUnitsToApprove() const
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colUnits[c4Index].eState() == tZWaveUSB3Sh::EUnitStates::WaitApprove)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Returns true if the indicated unit name already exists
tCIDLib::TBoolean TZWaveDrvCfg::bUnitExists(const TString& strToCheck) const
{
    return (punitiFindByName(strToCheck) != nullptr);
}


//
//  This is called by the client when it sees non-replication style changes to a
//  unit. We have to be careful when this happens because it's not legal for the
//  server side to change things except in very restricted ways. See the unit info
//  class' header comments.
//
tCIDLib::TBoolean
TZWaveDrvCfg::bUpdateUnit(const TZWUnitInfo& unitiNew, tCIDLib::TCard4& c4At)
{
    // Find the unit with the passed id. The name may have changed
    const tCIDLib::TCard1 c1UnitId = unitiNew.c1Id();
    TZWUnitInfo* punitiOld = nullptr;
    c4At = 0;
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (; c4At < c4Count; c4At++)
    {
        TZWUnitInfo& unitiCur = m_colUnits[c4At];
        if (unitiCur.c1Id() == c1UnitId)
        {
            // This is our guy, so save it and break out
            punitiOld = &unitiCur;
            break;
        }
    }

    //
    //  If we didn't find it, then that's bad since he's only supposed to be
    //  updating existing units here.
    //
    if (!punitiOld)
    {
        facZWaveUSB3Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c1UnitId)
        );
        c4At = kCIDLib::c4MaxCard;
        return kCIDLib::False;
    }

    // If no changes, then return false now
    if (unitiNew == *punitiOld)
        return kCIDLib::False;

    *punitiOld = unitiNew;
    return kCIDLib::True;
}


//
//  Validate our configuration data. Mostly it's not allowed to be inconsistent but
//  some things we need to check.
//
tCIDLib::TBoolean TZWaveDrvCfg::bValidate(TString& strErr)
{
    //
    //  Because we can take names from units, and because the user can also edit them,
    //  the server side driver can only verify that a name it gets from the unit is
    //  unique within the configuration he has. The client driver have changes locally.
    //  So even if it's not a dup on the server side it could be ultimately. So
    //  we need to make sure there are no dups.
    //
    //  We use a hash set to check for dups. We set the key ops to case insensitive so
    //  we can't have the same name with different case.
    //
    tCIDLib::TStrHashSet colUnique(109, TStringKeyOps(kCIDLib::False));

    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWUnitInfo& unitiCur = m_colUnits[c4Index];
        if (colUnique.bHasElement(unitiCur.strName()))
        {
            strErr = L"'";
            strErr.Append(unitiCur.strName());
            strErr.Append(L"' is a duplicate unit name. All names must be unique");
            return kCIDLib::False;
        }
        colUnique.objAdd(unitiCur.strName());
    }


    return kCIDLib::True;
}


//
//  Get or set the master controller id. The driver will set this as soon as he sees a
//  msg from the master during initial inclusion.
//
tCIDLib::TCard1 TZWaveDrvCfg::c1MasterId() const
{
    return m_c1MasterId;
}

tCIDLib::TCard1 TZWaveDrvCfg::c1MasterId(const tCIDLib::TCard1 c1ToSet)
{
    m_c1MasterId = c1ToSet;

    //
    //  See if we have this guy. If so, and it's not been explicitly named, then
    //  set a useful default name.
    //
    TZWUnitInfo* punitiMaster = punitiFindByName(L"MasterCtrl");
    if (!punitiMaster)
    {
        // Not a dup name or anything, so see if it's already been explicitly named
        punitiMaster = punitiFindById(c1ToSet);
        CIDAssert(punitiMaster != nullptr, L"The id set as master is not in our unit list");

        if (punitiMaster && (punitiMaster->eNameSrc() != tZWaveUSB3Sh::EUnitNameSrcs::User))
            punitiMaster->strName(L"MasterCtrl");
    }
    return m_c1MasterId;
}


// We generate a simple hash of the serial numbers of the units.
tCIDLib::TCard4 TZWaveDrvCfg::c4CalcSNHash() const
{
    tCIDLib::TCard4 c4Ret = 0;
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWUnitInfo& unitiCur = m_colUnits[c4Index];
        c4Ret ^= unitiCur.c4SerialNum();
        c4Ret <<= 1;
    }
    return c4Ret;
}


// Const and non-const access to our unit info list
const tZWaveUSB3Sh::TUnitInfoList& TZWaveDrvCfg::colUnits() const
{
    return m_colUnits;
}

tZWaveUSB3Sh::TUnitInfoList& TZWaveDrvCfg::colUnits()
{
    return m_colUnits;
}


//
//  The starting point of a hierarchical report generation mechanism. We format our
//  top level stuff, then we iterate the units and let them do theirs, and they do
//  their CC impls, and so on.
//
tCIDLib::TVoid
TZWaveDrvCfg::FormatReport(TTextOutStream& strmTar, const TString& strMoniker) const
{
    strmTar << L"Z-Wave Driver Report\n"
            << L"-------------------------------------\n";

    // Format our stuff
    strmTar << L"Moniker:" << strMoniker << kCIDLib::NewLn
            << L"Ctrl Id:" << m_c1CtrlId << kCIDLib::NewLn

            << kCIDLib::NewLn;

    // Set the indent for the unit stuff
    TStreamIndentJan janIndent(&strmTar, 4);

    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Skip disabled ones
        if (m_colUnits[c4Index].eState() == tZWaveUSB3Sh::EUnitStates::Disabled)
            continue;

        m_colUnits[c4Index].FormatReport(strmTar);
        strmTar << L"\n\n";
    }

    strmTar.Flush();
}


//
//  This is called to have us load up our basic info about our controller. This is often
//  done against previously stored and reloaded config objects, so we have to first
//  remember anything that we need to be aware of a change in. This is mostly whether
//  our in or out of network status changed.
//
tCIDLib::TVoid TZWaveDrvCfg::LoadControllerInfo(MZWaveCtrlIntf& mzwciIO)
{
    //
    //  Do a controller version query. This is often called right after the replication
    //  completes, so provide an explicit timeout and make it longer than usual to give
    //  the controller time ot get his ducks in a row.
    //
    try
    {
        // Default some stuff until we get it set explicitly
        m_c4HomeId = 0;

        TZWInMsg zwimReply;
        TZWOutMsg zwomSend;

        // Get basic version info on our controller
        zwomSend.Reset(FUNC_ID_ZW_GET_VERSION, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
        zwomSend.Finalize(kCIDLib::False, kCIDLib::False);
        mzwciIO.USB3SendCtrlRes(zwomSend, zwimReply, 4000);
        CIDAssert(zwimReply.m_c1PLLength >= 13, L"ZW version reply is too short");

        //
        //  We got the expected reply. So pull out the Z-Wave firmware version
        //  that it reported.
        //
        const tCIDLib::TCard1* pc1Buf = zwimReply.pc1Payload();
        const tCIDLib::TCard1* const pc1EndPtr = zwimReply.pc1PayloadEnd();

        //
        //  Pull out the version info and set up a version stamp. And
        //  move up to the period.
        //
        pc1Buf = &pc1Buf[7];
        const tCIDLib::TCard1* pc1Ptr = pc1Buf;
        while((*pc1Ptr != 0x2E) && (pc1Ptr < pc1EndPtr))
            pc1Ptr++;

        // Convert the part before it to the major version
        m_c4ZWLibMajVer = 0;
        while (pc1Buf < pc1Ptr)
        {
            m_c4ZWLibMajVer *= 10;
            m_c4ZWLibMajVer += (*pc1Buf - 0x30);
            pc1Buf++;
        }

        //
        //  Move up past the period and find the nul terminator at the end of
        //  the version info.
        //
        pc1Buf++;
        while(*pc1Ptr && (pc1Ptr < pc1EndPtr))
            pc1Ptr++;

        // And convert this part to the minor version, skipping leading zeros
        m_c4ZWLibMinVer = 0;
        while (pc1Buf < pc1Ptr)
        {
            if (*pc1Buf != 0x30)
            {
                m_c4ZWLibMinVer *= 10;
                m_c4ZWLibMinVer += (*pc1Buf - 0x30);
            }
            pc1Buf++;
        }

        //
        //  Let's find out if we are in or out of the network. If we are on another
        //  network (not our home id), then we are on someone else's network. We also
        //  need to be a secondary controller.
        //
        zwomSend.Reset(FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
        zwomSend.Finalize(kCIDLib::False, kCIDLib::False);
        mzwciIO.USB3SendCtrlRes(zwomSend, zwimReply);
        m_bIsSUC = (zwimReply[0] & CONTROLLER_IS_SUC) != 0;
        m_bInNetwork = (zwimReply[0] & CONTROLLER_ON_OTHER_NETWORK) != 0;

        if ((zwimReply[0] & CONTROLLER_IS_SECONDARY) == 0)
            facZWaveUSB3Sh().LogTraceErr(L"We are not marked as a secondary controller");

        //
        //  OK, that worked, so let's try to get the initial data info. This
        //  can be different sizes depending on the firmware version.
        //
        zwomSend.Reset(FUNC_ID_SERIAL_API_GET_INIT_DATA, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
        zwomSend.Finalize(kCIDLib::False, kCIDLib::False);
        mzwciIO.USB3SendCtrlRes(zwomSend, zwimReply);
        m_c4SerAPIVersion = zwimReply[0];

        //
        //  Set up the node map with all the known nodes. The map is the 29 bytes starting
        //  at offset 6. Unit id 1 is at bit index 0 and so forth.
        //
        m_bsUnitMap.FromRawArray(zwimReply.pc1PayloadAt(3), 29);

        // And get our id info
        zwomSend.Reset(FUNC_ID_MEMORY_GET_ID, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
        zwomSend.Finalize(kCIDLib::False, kCIDLib::False);
        mzwciIO.USB3SendCtrlRes(zwomSend, zwimReply);

        // Don't set the home id unless we are in the network
        if (m_bInNetwork)
        {
            m_c4HomeId = zwimReply[3];
            m_c4HomeId <<= 8;
            m_c4HomeId |= zwimReply[2];
            m_c4HomeId <<= 8;
            m_c4HomeId |= zwimReply[1];
            m_c4HomeId <<= 8;
            m_c4HomeId |= zwimReply[0];
        }
        m_c1CtrlId = zwimReply[4];

        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Serial ver=%(1), CtrlId=%(2), HomeId=%(3)"
                , TCardinal(m_c4SerAPIVersion)
                , TCardinal(m_c1CtrlId)
                , TCardinal(m_c4HomeId)
            );
        }

        //
        //  Set a default, generic controller model. Then, if the serial API version is 4
        //  or beyond, let's get serial API info and see if we have info on this model.
        //
        m_strCtrlModel = L"Z-Wave Ctrl";
        if (m_c4SerAPIVersion >= 4)
        {
            zwomSend.Reset(FUNC_ID_SERIAL_API_GET_CAPABILITIES, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
            zwomSend.Finalize(kCIDLib::False, kCIDLib::False);
            mzwciIO.USB3SendCtrlRes(zwomSend, zwimReply);

            // Load up the encoded manufacturer specific ids
            m_c8ManIds = zwimReply[2];
            m_c8ManIds <<= 8;
            m_c8ManIds |= zwimReply[3];
            m_c8ManIds <<= 8;
            m_c8ManIds |= zwimReply[4];
            m_c8ManIds <<= 8;
            m_c8ManIds |= zwimReply[5];
            m_c8ManIds <<= 8;
            m_c8ManIds |= zwimReply[6];
            m_c8ManIds <<= 8;
            m_c8ManIds |= zwimReply[7];

            // Get the product info, if we can
            TString strCppClass;
            TString strMake;
            TString strModel;
            const tCIDLib::TBoolean bRes = facZWaveUSB3Sh().bQueryDevInfo
            (
                m_c8ManIds, strMake, strModel, strCppClass
            );

            if (bRes)
            {
                m_strCtrlModel = strMake;
                m_strCtrlModel.Append(kCIDLib::chSpace);
                m_strCtrlModel.Append(strModel);
            }
        }

        // Format a text version of the firmware stuff
        m_strFWVersion.Clear();
        m_strFWVersion.AppendFormatted(m_c4ZWLibMajVer);
        m_strFWVersion.Append(kCIDLib::chPeriod);
        m_strFWVersion.AppendFormatted(m_c4ZWLibMinVer);

        // If in network, do some work to reconcile the unit map with what we have
        if (m_bInNetwork)
        {
            //
            //  Go through our units and remove any that no longer are in the map.
            //
            //  IT IS IMPORTANT we do this first, since it's possible another unit
            //  might show up with the same id as an old we we had before. So we
            //  need to get rid of missing ones first.
            //
            tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
            tCIDLib::TCard4 c4Index = 0;
            while (c4Index < c4Count)
            {
                TZWUnitInfo& unitiCur = m_colUnits[c4Index];
                const tCIDLib::TCard1 c1Id = unitiCur.c1Id();

                if (m_bsUnitMap[c1Id - 1])
                {
                    // Still got it, so move forward
                    c4Index++;
                }
                 else
                {
                    facZWaveUSB3Sh().LogTraceMsg
                    (
                        L"Node %(1) not in node map, removing it", TCardinal(c1Id)
                    );

                    // Dont' access unitiCur after this!
                    m_colUnits.RemoveAt(c4Index);
                    c4Count--;
                }
            }

            //
            //  Then go through the unit map. For each unit that is present, see if
            //  we have that one. If not, add a unit for this guy with default info.
            //
            //  If one we add is our own controller, it will be marked as disabled
            //  when added. We don't care about controllers.
            //
            for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveUSB3Sh::c4MaxUnits; c4Index++)
            {
                if (m_bsUnitMap[c4Index])
                {
                    const tCIDLib::TCard1 c1Id = tCIDLib::TCard1(c4Index + 1);
                    if (!punitiFindById(c1Id))
                    {
                        TZWUnitInfo unitiNew(c1Id);

                        //
                        //  If it's us to to give it a useful name automatically. We
                        //  won't know the master controller's id yet, if this is the
                        //  first inclusion (and that's the only time here we'd have
                        //  chance to set the name. We'll try to deal with that
                        //  elsewhere.
                        //
                        if (c1Id == m_c1CtrlId)
                        {
                            if (!punitiFindByName(L"ThisDriver"))
                                unitiNew.strName(L"ThisDriver");
                        }
                        bAddUnit(unitiNew);
                    }
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        // Reset us to defaults
        Reset();

        if (facZWaveUSB3Sh().bMediumTrace())
            facZWaveUSB3Sh().LogTraceMsg(L"Got no controller version reply");

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Find an existing unit in our list with the passed unit id. If not found, we
//  return a null pointer. They are sorted by name, so for this we just have to
//  search the whole list.
//
const TZWUnitInfo* TZWaveDrvCfg::punitiFindById(const tCIDLib::TCard1 c1ToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWUnitInfo& unitiCur = m_colUnits[c4Index];
        if (unitiCur.c1Id() == c1ToFind)
            return &unitiCur;
    }
    return nullptr;
}

TZWUnitInfo* TZWaveDrvCfg::punitiFindById(const tCIDLib::TCard1 c1ToFind)
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWUnitInfo& unitiCur = m_colUnits[c4Index];
        if (unitiCur.c1Id() == c1ToFind)
            return &unitiCur;
    }
    return nullptr;
}


//
//  Find the unit with the passed name. If found, we return a pointer, else
//  we return null.
//
const TZWUnitInfo* TZWaveDrvCfg::punitiFindByName(const TString& strToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWUnitInfo& unitiCur = m_colUnits[c4Index];
        if (unitiCur.strName().bCompareI(strToFind))
            return &unitiCur;
    }
    return nullptr;
}

TZWUnitInfo* TZWaveDrvCfg::punitiFindByName(const TString& strToFind)
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWUnitInfo& unitiCur = m_colUnits[c4Index];
        if (unitiCur.strName().bCompareI(strToFind))
            return &unitiCur;
    }
    return nullptr;
}


// Get the Z-Wave library versions we got during initial discovery
tCIDLib::TVoid
TZWaveDrvCfg::QueryZWLibVer(tCIDLib::TCard4& c4ZWLibMajVer, tCIDLib::TCard4& c4ZWLibMinVer)
{
    c4ZWLibMajVer = m_c4ZWLibMajVer;
    c4ZWLibMinVer = m_c4ZWLibMinVer;
}


//
//  This is called by the driver when it is about to start using a new configuration.
//  We give the units a change to do any init they only need when being put into use.
//  We don't want to do these things when they are created or copied or streamed
//  because that happens a lot.
//
tCIDLib::TVoid TZWaveDrvCfg::PrepForUse(MZWaveCtrlIntf* const pmzwciOwner)
{
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWUnitInfo& unitiCur = m_colUnits[c4Index];
        unitiCur.PrepForUse(enctNow, pmzwciOwner);
    }
}


//
//  Upon startup or after a replication, we need to do basic probes of any newly
//  discovered units to get the most basic info about them, mostly are they listeners
//  or frequent listeners.
//
tCIDLib::TVoid TZWaveDrvCfg::ProbeNewUnits(MZWaveCtrlIntf* const pmzwciOwner)
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWUnitInfo& unitiCur = m_colUnits[c4Index];
        if (unitiCur.m_eState != tZWaveUSB3Sh::EUnitStates::InitUnit)
            continue;

        try
        {
            if (unitiCur.bProbeUnit(pmzwciOwner))
                unitiCur.eState(tZWaveUSB3Sh::EUnitStates::GetUnitInfo);
            else
                unitiCur.eState(tZWaveUSB3Sh::EUnitStates::Failed);

            // We should be in init unit state anymore, so check that
            CIDAssertX
            (
                (unitiCur.eState() != tZWaveUSB3Sh::EUnitStates::InitUnit)
                , L"Probe of unit %(1) did not change its state"
                , unitiCur.strName()
            );
        }

        catch(TError& errToCatch)
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Exception during basic probe of unit %(1)", unitiCur.strName()
            );

            // Shouldn't happen but just in case
            unitiCur.eState(tZWaveUSB3Sh::EUnitStates::Failed);
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}



// Sets us to default values
tCIDLib::TVoid TZWaveDrvCfg::Reset()
{
    m_bInNetwork = kCIDLib::False;
    m_bIsSUC = kCIDLib::False;
    m_bsUnitMap.Clear();
    m_c1CtrlId = 0;
    m_c1MasterId = 0;
    m_c4HomeId = 0;
    m_c4SerAPIVersion = 0;
    m_c4ZWLibMajVer = 0;
    m_c4ZWLibMinVer = 0;
    m_c8ManIds = 0;
    m_strCtrlModel.Clear();
    m_strCtrlName = L"CQCZWCtrl";
    m_strFWVersion.Clear();

    ResetUnitList();
}


tCIDLib::TVoid TZWaveDrvCfg::ResetUnitList()
{
    m_colUnits.RemoveAll();
}



// Get or set our controller's assigned module name
const TString& TZWaveDrvCfg::strCtrlName() const
{
    return m_strCtrlName;
}

const TString& TZWaveDrvCfg::strCtrlName(const TString& strToSet)
{
    m_strCtrlName = strToSet;
    return m_strCtrlName;
}


// Return the unit info at the indicated index
const TZWUnitInfo& TZWaveDrvCfg::unitiAt(const tCIDLib::TCard4 c4Index) const
{
    if (c4Index >= m_colUnits.c4ElemCount())
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_BadUnitIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4Index)
            , TCardinal(m_colUnits.c4ElemCount())
        );
    }
    return m_colUnits[c4Index];
}


//
//  Find an existing unit in our list with the passed unit id. If not found,
//  we throw.
//
TZWUnitInfo& TZWaveDrvCfg::unitiFindById(const tCIDLib::TCard1 c1ToFind)
{
    TZWUnitInfo* punitiRet = punitiFindById(c1ToFind);
    if (!punitiRet)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c1ToFind)
        );
    }
    return *punitiRet;
}

const TZWUnitInfo& TZWaveDrvCfg::unitiFindById(const tCIDLib::TCard1 c1ToFind) const
{
    const TZWUnitInfo* punitiRet = punitiFindById(c1ToFind);
    if (!punitiRet)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c1ToFind)
        );
    }
    return *punitiRet;
}


//
//  Find a unit with the passed name. If not found, we throw. We have const and
//  non-const versions.
//
const TZWUnitInfo& TZWaveDrvCfg::unitiFindByName(const TString& strToFind) const
{
    const TZWUnitInfo* punitiCheck = punitiFindByName(strToFind);

    if (!punitiCheck)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_UnitNameNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
        );
    }
    return *punitiCheck;
}

TZWUnitInfo& TZWaveDrvCfg::unitiFindByName(const TString& strToFind)
{
    TZWUnitInfo* punitiCheck = punitiFindByName(strToFind);
    if (!punitiCheck)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_UnitNameNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
        );
    }
    return *punitiCheck;
}


// ---------------------------------------------------------------------------
//  TZWaveDrvCfg: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWaveDrvCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSB3Sh_DrvCfg::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_bInNetwork
                    >> m_bIsSUC
                    >> m_bsUnitMap
                    >> m_c1CtrlId
                    >> m_c1MasterId
                    >> m_c4HomeId
                    >> m_c4SerAPIVersion
                    >> m_c4ZWLibMajVer
                    >> m_c4ZWLibMinVer
                    >> m_c8ManIds
                    >> m_strCtrlModel
                    >> m_strCtrlName;

    // Get the count of stored units
    tCIDLib::TCard4 c4UnitCnt;
    strmToReadFrom  >> c4UnitCnt;

    //
    //  We have to polymorphically stream the units in. They were sorted when they
    //  were stored so they should still be.
    //
    m_colUnits.RemoveAll();
    TZWUnitInfo unitiCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        strmToReadFrom >> unitiCur;
        m_colUnits.objAdd(unitiCur);
    }

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Format a text version of the firmware stuff
    m_strFWVersion.Clear();
    m_strFWVersion.AppendFormatted(m_c4ZWLibMajVer);
    m_strFWVersion.Append(kCIDLib::chPeriod);
    m_strFWVersion.AppendFormatted(m_c4ZWLibMinVer);
}


tCIDLib::TVoid TZWaveDrvCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    const tCIDLib::TCard4 c4UnitCnt  = m_colUnits.c4ElemCount();

    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSB3Sh_DrvCfg::c2FmtVersion

                    << m_bInNetwork
                    << m_bIsSUC
                    << m_bsUnitMap
                    << m_c1CtrlId
                    << m_c1MasterId
                    << m_c4HomeId
                    << m_c4SerAPIVersion
                    << m_c4ZWLibMajVer
                    << m_c4ZWLibMinVer
                    << m_c8ManIds
                    << m_strCtrlModel
                    << m_strCtrlName;


    // Do the unit count and then the unit info objects
    strmToWriteTo   << c4UnitCnt;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
        strmToWriteTo << m_colUnits[c4Index];

    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TZWaveDrvCfg: Private, non-virtual methods
// ---------------------------------------------------------------------------

