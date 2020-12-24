//
// FILE NAME: ZWaveUSB3Sh_GenRGBW.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/16/2017
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
//  This is the implementation for a fairly generic RGBW controller
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
RTTIDecls(TGenRGBWUnit, TZWaveUnit)




// ---------------------------------------------------------------------------
//   CLASS: TGenRGBWUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenRGBWUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenRGBWUnit::TGenRGBWUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_pzwcciColor(nullptr)
    , m_pzwcciLevel(nullptr)
    , m_pzwcciSwitch(nullptr)
{
}

TGenRGBWUnit::~TGenRGBWUnit()
{
}


// ---------------------------------------------------------------------------
//  TGenRGBWUnit: Public, inherited methods
// ---------------------------------------------------------------------------

// We create and store our CC impl objects and remember their ids
tCIDLib::TBoolean TGenRGBWUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();
    tCIDLib::TCard4 c4ImplId;
    tZWaveUSB3Sh::ECClasses eClass;

    // See if the white component is supported
    tCIDLib::TCard1 c1WhiteComp = 0xFF;
    TString strWhite;
    if (bFindExtraVal(L"WhiteComp", strWhite))
    {
        tCIDLib::TCard1 c1CompId;
        if (!strWhite.bToCard1(c1CompId, tCIDLib::ERadices::Auto)
        || (c1CompId > 1))
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Invalid white component id for unit %(1)", strName()
            );
            return kCIDLib::False;
        }
        c1WhiteComp = c1CompId;
    }


    //
    //  Add a color switch one for the basic V2 color switch support. The R, G, and
    //  B color components are fixed by Z-Wave so it knows what to look for. For
    //  white we pass what we got above.
    //
    m_pzwcciColor = new TZWCCImplCLghtRGBF(&unitiUs, TString::strEmpty(), c1WhiteComp);
    if (bAlwaysOn())
        m_pzwcciColor->SetWriteAccess(kCIDLib::True);
    m_pzwcciColor->strDesc(L"RBG");
    if (!bAddImplObject(m_pzwcciColor, c4ImplId))
        return kCIDLib::False;


    // See if Off/On support is supported
    TString strOffOn;
    if (bFindExtraVal(L"OffOn", strOffOn))
    {
        m_strSwFldName = L"LGHT#Sw_";
        m_strSwFldName.Append(unitiUs.strName());

        eClass = tZWaveUSB3Sh::eXlatECClasses(strOffOn);
        if (eClass == tZWaveUSB3Sh::ECClasses::Basic)
        {
            m_pzwcciSwitch = new TZWCCImplBasicSwitchF
            (
                &unitiUs, m_strSwFldName, tCQCKit::EFldSTypes::LightSwitch
            );
        }
         else if (eClass == tZWaveUSB3Sh::ECClasses::BinSwitch)
        {
            m_pzwcciSwitch = new TZWCCImplBinSwitchF
            (
                &unitiUs, m_strSwFldName, tCQCKit::EFldSTypes::LightSwitch
            );
        }
         else if (eClass == tZWaveUSB3Sh::ECClasses::MLSwitch)
        {
            m_pzwcciSwitch = new TZWCCImplMLSwitchF
            (
                &unitiUs, m_strSwFldName, tCQCKit::EFldSTypes::LightSwitch
            );
        }
         else
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Unknown RGB off/on class %(1), unit=%(2)", strOffOn, strName()
            );
            return kCIDLib::False;
        }

        if (bAlwaysOn())
            m_pzwcciSwitch->SetWriteAccess(kCIDLib::True);
        m_pzwcciSwitch->strDesc(L"Switch");
        if (!bAddImplObject(m_pzwcciSwitch, c4ImplId))
            return kCIDLib::False;
    }

    // See if separate dimming is supported
    TString strDim;
    if (bFindExtraVal(L"Dim", strDim))
    {
        TString strFldName = L"LGHT#Dim_";
        strFldName.Append(unitiUs.strName());

        eClass = tZWaveUSB3Sh::eXlatECClasses(strOffOn);
        if (eClass == tZWaveUSB3Sh::ECClasses::Basic)
        {
            m_pzwcciLevel = new TZWCCImplBasicLevelF
            (
                &unitiUs, strFldName, tCQCKit::EFldSTypes::Dimmer
            );
        }
         else if (eClass == tZWaveUSB3Sh::ECClasses::MLSwitch)
        {
            m_pzwcciLevel = new TZWCCImplMLLevelF
            (
                &unitiUs, strFldName, tCQCKit::EFldSTypes::Dimmer
            );
        }
         else
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Unknown RGB dimmer class %(1), unit=%(2)", strDim, strName()
            );
            return kCIDLib::False;
        }

        if (bAlwaysOn())
            m_pzwcciLevel->SetWriteAccess(kCIDLib::True);
        m_pzwcciLevel->strDesc(L"Dimmer");
        if (!bAddImplObject(m_pzwcciLevel, c4ImplId))
            return kCIDLib::False;
    }

    return kCIDLib::True;
}


// We have to watch for change sin the switch impl to send out load change triggers
tCIDLib::TVoid
TGenRGBWUnit::ImplValueChanged( const   tCIDLib::TCard4         c4ImplId
                                , const tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          )
{
    if (eSource != tZWaveUSB3Sh::EValSrcs::Program)
    {
        // If it's our switch impl, then send a load change trigger
        if (m_pzwcciSwitch->c4ImplId() == c4ImplId)
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

        //
        //  If it's either our switch or level, then send a value query for the
        //  color components, because it doesn't send anything for those typically
        //  when the off/on or level state is changed. This will get the color
        //  CC impl to do a query as soon as is reasonable.
        //
        if ((m_pzwcciSwitch->c4ImplId() == c4ImplId)
        ||  (m_pzwcciLevel->c4ImplId() == c4ImplId))
        {
            m_pzwcciColor->RequestValueQuery();
        }
    }
}
