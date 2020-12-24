//
// FILE NAME: ZWaveUSB3Sh_Outlet.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/02/2018
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
//  This is the implementation for a flexible one or two-way switchable outlet handler
//  which various units can use via the device info file.
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
//  Magic macros.
// ---------------------------------------------------------------------------
RTTIDecls(TOutletUnit, TZWaveUnit)


// ---------------------------------------------------------------------------
//   CLASS: TOutletUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TOutletUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TOutletUnit::TOutletUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_bAsLight(kCIDLib::False)
    , m_eAccess(tZWaveUSB3Sh::EUnitAcc::Write)
    , m_eNotifyClass(tZWaveUSB3Sh::ECClasses::Count)
    , m_eOutletClass(tZWaveUSB3Sh::ECClasses::Basic)
    , m_pzwcciNotify(nullptr)
    , m_pzwcciOutlet(nullptr)
{
}

TOutletUnit::~TOutletUnit()
{
}


// ---------------------------------------------------------------------------
//  TOutletUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We parse out our one/two way indicator and save that flag away for later use when
//  the fields are created.
//
tCIDLib::TBoolean TOutletUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();
    TString strVal;

    // Set the one/two way flag
    if (!bFindExtraVal(L"IOType", strVal))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) has no read/write access extra info", unitiOwner().strName()
        );
        return kCIDLib::False;
    }

    if (strVal.bCompareI(L"ReadWrite"))
        m_eAccess = tZWaveUSB3Sh::EUnitAcc::ReadWrite;
    else
        m_eAccess = tZWaveUSB3Sh::EUnitAcc::Write;


    // See if we have a separate async notification class
    m_eNotifyClass = tZWaveUSB3Sh::ECClasses::Count;
    if (bFindExtraVal(L"AsyncNot", strVal))
    {
        m_eNotifyClass = tZWaveUSB3Sh::eXlatECClasses(strVal);
        if (m_eNotifyClass == tZWaveUSB3Sh::ECClasses::Count)
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Unit %(1) invalid notification class extra info info (%(2))"
                , unitiUs.strName()
                , strVal
            );
            return kCIDLib::False;
        }
    }


    // Get the outlet CC impl
    if (!bFindExtraVal(L"OutletCC", strVal))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) has no outlet command class extra info", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    m_eOutletClass = tZWaveUSB3Sh::eXlatECClasses(strVal);
    if (m_eOutletClass == tZWaveUSB3Sh::ECClasses::Count)
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) invalid outlet class extra info info (%(2))"
            , unitiUs.strName()
            , strVal
        );
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//  We need to parse our extra data and create our impl objects.
tCIDLib::TBoolean TOutletUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // See if we are supposed to be acting like a light
    m_bAsLight = unitiUs.bQueryOptVal(kZWaveUSB3Sh::strUOpt_AsLight, kCIDLib::False);

    //
    //  And now create our CC impls based on the classes the user selected. If we are
    //  two way and set up to be a light switch, we create a V2 compliant field.
    //
    tCIDLib::TCard4 c4ImplId;
    if (m_bAsLight && tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read))
    {
        m_strFldName = L"LGHT#Sw_";
        m_strFldName.Append(unitiUs.strName());
    }
     else if (m_bAsLight)
    {
        m_strFldName = unitiUs.strName();
        m_strFldName.Append(L"_Switch");
    }
     else
    {
        m_strFldName = unitiUs.strName();
        m_strFldName.Append(L"_Outlet");
    }

    const tCQCKit::EFldSTypes eSemType
    (
        m_bAsLight ? tCQCKit::EFldSTypes::LightSwitch : tCQCKit::EFldSTypes::Generic
    );
    if (m_eOutletClass == tZWaveUSB3Sh::ECClasses::Basic)
    {
        m_pzwcciOutlet = new TZWCCImplBasicSwitchF
        (
            &unitiUs, m_strFldName, eSemType
        );
    }
     else if (m_eOutletClass == tZWaveUSB3Sh::ECClasses::BinSwitch)
    {
        m_pzwcciOutlet = new TZWCCImplBinSwitchF
        (
            &unitiUs, m_strFldName, eSemType
        );
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) unsupported Z-Wave outlet command class", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    m_pzwcciOutlet->SetAccess(m_eAccess, tZWaveUSB3Sh::EUnitAcc::AllBits);
    m_pzwcciOutlet->strDesc(L"Outlet");
    if (!bAddImplObject(m_pzwcciOutlet, c4ImplId))
        return kCIDLib::False;

    //
    //  If we have a separate notification class, do that guy. Access is none for
    //  these since they are just to receive notification.
    //
    m_pzwcciNotify = nullptr;
    if (m_eNotifyClass == tZWaveUSB3Sh::ECClasses::Basic)
        m_pzwcciNotify = new TZWCCImplBasicSwitch(&unitiUs);
    else if (m_eNotifyClass == tZWaveUSB3Sh::ECClasses::BinSwitch)
        m_pzwcciNotify = new TZWCCImplBinSwitch(&unitiUs);
    else if (m_eNotifyClass == tZWaveUSB3Sh::ECClasses::Notification)
        m_pzwcciNotify = new TZWCCImplNotifyState(&unitiUs);
    if (m_pzwcciNotify)
    {
        m_pzwcciNotify->SetAccess(tZWaveUSB3Sh::EUnitAcc::None, tZWaveUSB3Sh::EUnitAcc::AllBits);
        m_pzwcciNotify->strDesc(L"Notifier");
        if (!bAddImplObject(m_pzwcciNotify, c4ImplId))
            return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  We have to watch for the switch impl to change and send a trigger if this is a
//  V2 type switch.
//
tCIDLib::TVoid
TOutletUnit::ImplValueChanged(  const   tCIDLib::TCard4         c4ImplId
                                , const tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          )
{
    // If it's a programmatic change, we don't care about it
    if (eSource == tZWaveUSB3Sh::EValSrcs::Program)
        return;

    //
    //  If a change from notification, then push it into the switch. Then see if
    //  the switch state changed one way or another. If so, we may need to send a
    //  trigger.
    //
    tCIDLib::TBoolean bReport = kCIDLib::False;;
    if (m_pzwcciNotify && (c4ImplId == m_pzwcciNotify->c4ImplId()))
    {
        // If this changes the switch's state, then it's a reportable change
        bReport = m_pzwcciOutlet->bSetBinState
        (
            m_pzwcciNotify->bState(), tZWaveUSB3Sh::EValSrcs::Program
        );
    }
     else if (c4ImplId == m_pzwcciOutlet->c4ImplId())
    {
        // The switch itself changed, so always a change we want to report
        bReport = kCIDLib::True;

        //
        //  Push the switch state into the notification to keep them in sync, if
        //  we have one. Else we may get bogus change callbacks. This is a
        //  programmatic change.
        //
        if (m_pzwcciNotify)
        {
            m_pzwcciNotify->bSetBinState
            (
                m_pzwcciOutlet->bState(), tZWaveUSB3Sh::EValSrcs::Program
            );
        }
    }

    // If a reportable and we are two way and acting like a light, then send a trigger
    if (bReport)
    {
        if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read) && m_bAsLight)
        {
            TString strId;
            strId.AppendFormatted(c1Id());

            unitiOwner().QueueEventTrig
            (
                tCQCKit::EStdDrvEvs::LoadChange
                , m_strFldName
                , facCQCKit().strBoolOffOn(m_pzwcciOutlet->bState())
                , strId
                , strName()
                , TString::strEmpty()
            );
        }
    }
}


// We override this so we can our special unit level option
tCIDLib::TVoid
TOutletUnit::QueryUnitAttrs(tCIDMData::TAttrList& colToFill, TAttrData& adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();

    adatTmp.Set(L"Outlet Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colToFill.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Act as Light", kZWaveUSB3Sh::strUOpt_AsLight, tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(unitiUs.bQueryOptVal(kZWaveUSB3Sh::strUOpt_AsLight, kCIDLib::False));
    colToFill.objAdd(adatTmp);
}
