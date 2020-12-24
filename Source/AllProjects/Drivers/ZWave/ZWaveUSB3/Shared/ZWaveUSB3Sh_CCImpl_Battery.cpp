//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Battery.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/21/2017
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
//  This is the implementation classs for the Z-Wave Battery command class
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
//  Magic maros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCImplBattery, TZWCCImplLevel)
RTTIDecls(TZWCCImplBatteryF, TZWCCImplBattery)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBattery
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBattery: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  The parent will default us to read/readonwakeup access, which is correct for
//  our needs in most cases. The extra info or creating unit can override this
//  if needed.
//
TZWCCImplBattery::TZWCCImplBattery(TZWUnitInfo* const punitiThis) :

    TZWCCImplLevel(punitiThis, tZWaveUSB3Sh::ECClasses::Battery)
{
    strDesc(L"Battery");
}

TZWCCImplBattery::~TZWCCImplBattery()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBattery: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  For backwards compatibility we have to override this. We use to have an ad hoc
//  'read access' extra info. This has been replaced by some underlying generic
//  flags. But we need to translate this for existing stuff.
//
tCIDLib::TBoolean TZWCCImplBattery::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    // See if this one is readable, or if we can only get notifications
    TString strAcc;
    if (bFindExtraVal(L"ReadAcc", strAcc))
    {
        tZWaveUSB3Sh::EUnitAcc eAcc = tZWaveUSB3Sh::EUnitAcc::None;

        if (strAcc.bCompareI(L"Read"))
            eAcc = tZWaveUSB3Sh::EUnitAcc::Read;
        else if (strAcc.bCompareI(L"Wakeup"))
            eAcc = tZWaveUSB3Sh::EUnitAcc::BatteryRead;
        else if (strAcc.bCompareI(L"Notify"))
            eAcc = tZWaveUSB3Sh::EUnitAcc::None;
        else
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Battery %(1), unknown ReadAcc value: %(2)", strUnitName(), strAcc
            );
            return kCIDLib::False;
        }
        SetAccess(eAcc);
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TZWCCImplBattery: Protected, inherited methods
// ---------------------------------------------------------------------------

//  We just watch for a battery report and store the percentage.
tZWaveUSB3Sh::ECCMsgRes
TZWCCImplBattery::eHandleCCMsg( const   tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_BATTERY) && (c1Cmd == BATTERY_REPORT))
    {
        tCIDLib::TCard4 c4Per = zwimIn.c1CCByteAt(2);

        // Treat a low battery warning as a really low percent
        if (c4Per == 0xFF)
            c4Per = 5;
        else if (c4Per > 100)
            c4Per = kCIDLib::c4MaxCard;

        if (c4Per == kCIDLib::c1MaxCard)
            SetErrState();
        else
            bSetLevel(c4Per, tZWaveUSB3Sh::EValSrcs::Unit);

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}



tCIDLib::TVoid TZWCCImplBattery::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValueQuery() called for non-readable battery level %(1)/%(2)"
            , strUnitName()
            , strDesc()
        );
        return;
    }

    try
    {
        SendGetMsg();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// ---------------------------------------------------------------------------
//  TZWCCImplBattery: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We need to do this from more than one place. We always send async. If it's done
//  on the regular ping, then we would do it that way. If done on wakeup, then this
//  will get added to the wakeup queue and will subsequently be sent.
//
tCIDLib::TVoid TZWCCImplBattery::SendGetMsg()
{
    TZWOutMsg zwomOut;
    facZWaveUSB3Sh().BuildCCMsg
    (
        zwomOut
        , unitiOwner()
        , COMMAND_CLASS_BATTERY
        , BATTERY_GET
        , 0
        , tZWaveUSB3Sh::EMsgPrios::Async
        , kCIDLib::False
    );

    SendMsgAsync(zwomOut);
}



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBatteryF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBatteryF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplBatteryF::
TZWCCImplBatteryF(TZWUnitInfo* const  punitiThis, const TString& strFldName) :

    TZWCCImplBattery(punitiThis)
    , m_c4FldId_Level(kCIDLib::c4MaxCard)
    , m_strFldName(strFldName)
{
}

TZWCCImplBatteryF::~TZWCCImplBatteryF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBattery: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWCCImplBatteryF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Level);
}


tCIDLib::TVoid
TZWCCImplBatteryF::QueryFldDefs(tCQCKit::TFldDefList& colAddTo, TCQCFldDef& flddTmp) const
{
    // We just have our one boolean state field
    flddTmp.Set
    (
        m_strFldName
        , tCQCKit::EFldTypes::Card
        , eToFldAccess()
        , tCQCKit::EFldSTypes::Generic
        , L"Range: 0, 100"
    );
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplBatteryF::StoreFldIds()
{
    // Just look up our one field using the name we were given
    m_c4FldId_Level = c4LookupFldId(m_strFldName);
    SetFieldErr(m_c4FldId_Level);
}


// ---------------------------------------------------------------------------
//  TZWCCImplBatteryF: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWCCImplBatteryF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_Level);
    TParent::EnteredErrState();
}


tCIDLib::TVoid
TZWCCImplBatteryF::ValueChanged(const   tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          i4ExtraInfo)
{
    bWriteFld(m_c4FldId_Level, c4Level());
    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}
