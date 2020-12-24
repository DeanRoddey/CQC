//
// FILE NAME: ZWaveUSB3Sh_LightSwitch.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/24/2018
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
//  This is the implementation for a flexible one or two-way ligth switch handler
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
RTTIDecls(TLightSwitchUnit, TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_Unit_GenLightSwitch
{
}


// ---------------------------------------------------------------------------
//   CLASS: TLightSwitchUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLightSwitchUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TLightSwitchUnit::TLightSwitchUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_eAccess(tZWaveUSB3Sh::EUnitAcc::Write)
    , m_eNotifyClass(tZWaveUSB3Sh::ECClasses::Count)
    , m_eSwitchClass(tZWaveUSB3Sh::ECClasses::Basic)
    , m_pzwcciNotify(nullptr)
    , m_pzwcciSwitch(nullptr)
{
}

TLightSwitchUnit::~TLightSwitchUnit()
{
}


// ---------------------------------------------------------------------------
//  TLightSwitchUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We parse out our one/two way indicator and save that flag away for later use when
//  the fields are created.
//
tCIDLib::TBoolean TLightSwitchUnit::bParseExtraInfo()
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


    // Get the switch CC impl
    if (!bFindExtraVal(L"SwitchCC", strVal))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) has no switch command class extra info", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    m_eSwitchClass = tZWaveUSB3Sh::eXlatECClasses(strVal);
    if (m_eSwitchClass == tZWaveUSB3Sh::ECClasses::Count)
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) invalid switch class extra info info (%(2))"
            , unitiUs.strName()
            , strVal
        );
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//  We need to parse our extra data and create our impl objects.
tCIDLib::TBoolean TLightSwitchUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    //
    //  And now create our CC impls based on the classes the user selected. If we are
    //  two way, we create a V2 compliant field.
    //
    tCIDLib::TCard4 c4ImplId;
    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read))
    {
        m_strSwFldName = L"LGHT#Sw_";
        m_strSwFldName.Append(unitiUs.strName());
    }
     else
    {
        m_strSwFldName = unitiUs.strName();
        m_strSwFldName.Append(L"_Switch");
    }

    if (m_eSwitchClass == tZWaveUSB3Sh::ECClasses::Basic)
    {
        m_pzwcciSwitch = new TZWCCImplBasicSwitchF
        (
            &unitiUs, m_strSwFldName, tCQCKit::EFldSTypes::LightSwitch
        );
    }
     else if (m_eSwitchClass == tZWaveUSB3Sh::ECClasses::BinSwitch)
    {
        m_pzwcciSwitch = new TZWCCImplBinSwitchF
        (
            &unitiUs, m_strSwFldName, tCQCKit::EFldSTypes::LightSwitch
        );
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) unsupported Z-Wave switch command class", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    m_pzwcciSwitch->SetAccess(m_eAccess);
    m_pzwcciSwitch->strDesc(L"Switch");
    if (!bAddImplObject(m_pzwcciSwitch, c4ImplId))
        return kCIDLib::False;

    //
    //  If we have a separate notification class, do that guy. Force access to
    //  none on these, since the defaults would tend to enable reading of some
    //  sort on some of them.
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
TLightSwitchUnit::ImplValueChanged( const   tCIDLib::TCard4         c4ImplId
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
        bReport = m_pzwcciSwitch->bSetBinState
        (
            m_pzwcciNotify->bState(), tZWaveUSB3Sh::EValSrcs::Program
        );
    }
     else if (c4ImplId == m_pzwcciSwitch->c4ImplId())
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
                m_pzwcciSwitch->bState(), tZWaveUSB3Sh::EValSrcs::Program
            );
        }
    }

    // If a reportable change and we are V2 type, then send a trigger
    if (bReport)
    {
        if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read))
        {
            TString strId;
            strId.AppendFormatted(c1Id());

            unitiOwner().QueueEventTrig
            (
                tCQCKit::EStdDrvEvs::LoadChange
                , m_strSwFldName
                , facCQCKit().strBoolOffOn(m_pzwcciSwitch->bState())
                , strId
                , strName()
                , TString::strEmpty()
            );
        }
    }
}

