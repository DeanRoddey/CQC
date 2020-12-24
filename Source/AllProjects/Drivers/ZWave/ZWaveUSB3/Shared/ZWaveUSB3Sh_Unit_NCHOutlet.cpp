//
// FILE NAME: ZWaveUSB3Sh_NChOutlet.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/02/2017
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
//  This is the implementation for a generic N-channel outlet device.
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
RTTIDecls(TNChOutletUnit, TZWaveUnit)




// ---------------------------------------------------------------------------
//   CLASS: TNChOutletUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TNChOutletUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TNChOutletUnit::TNChOutletUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_c4EPCount(0)
    , m_c4FirstEPNum(1)
    , m_eClass(tZWaveUSB3Sh::ECClasses::Count)
{
}

TNChOutletUnit::~TNChOutletUnit()
{
}


// ---------------------------------------------------------------------------
//  TNChOutletUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Parse our extra data out and store it away. You can wait till bPrepare() but
//  we need some of this info in QueryOptions, which is called before bPrepare().
//
tCIDLib::TBoolean TNChOutletUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    TString strClass;
    TString strCnt;
    TString strStart;
    if (!bFindExtraVal(L"Class", strClass)
    ||  !bFindExtraVal(L"Cnt", strCnt)
    ||  !bFindExtraVal(L"StartNum", strStart))
    {
        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceErr(L"Unit %(1) missing extra info", strName());
        return kCIDLib::False;
    }

    // The end point count must be from 1 to 127
    if (!strCnt.bToCard4(m_c4EPCount, tCIDLib::ERadices::Auto)
    ||  (m_c4EPCount == 0)
    ||  (m_c4EPCount > 127))
    {
        if(facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceErr(L"Unit %(1) invalid end point count", strName());
        return kCIDLib::False;
    }

    // Get the handler class
    m_eClass = tZWaveUSB3Sh::eXlatECClasses(strClass);
    if ((m_eClass != tZWaveUSB3Sh::ECClasses::Basic)
    &&  (m_eClass != tZWaveUSB3Sh::ECClasses::BinSwitch))
    {
        if(facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Unit %(1) invalid extra info impl class (%(2))", strName(), strClass
            );
        }
        return kCIDLib::False;
    }

    //
    //  And the first end point also must be from 1 to 127, and the combination of the
    //  start and count must not go beyond 127.
    //
    if (!strStart.bToCard4(m_c4FirstEPNum, tCIDLib::ERadices::Auto)
    ||  (m_c4FirstEPNum == 0)
    ||  (m_c4FirstEPNum > 127)
    ||  (m_c4FirstEPNum + (m_c4EPCount - 1) > 127))
    {
        if(facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceErr(L"Unit %(1) invalid end point start", strName());
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//  We need to parse our extra data and create our impl objects.
tCIDLib::TBoolean TNChOutletUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    //
    //  For each of our end points, create and store a CC Impl of the correct type
    //  for the command class requested.
    //
    tCIDLib::TBoolean bRes = kCIDLib::True;
    tCIDLib::TCard4 c4ImplId;
    TString strFldName;
    TString strOutletName;
    TString strOptKey;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4EPCount; c4Index++)
    {
        // Create the option key for the unit name for this one
        strOptKey = L"/UnitOpt/Outlet/=";
        strOptKey.AppendFormatted(c4Index + 1);
        strOptKey.Append(L"/Name");

        // Use that to build the field name
        strFldName = unitiUs.strName();
        strFldName.Append(kCIDLib::chUnderscore);
        if (unitiUs.bFindOptVal(strOptKey, strOutletName))
        {
            strFldName.Append(strOutletName);
        }
        else
        {
            strFldName.Append(L"Outlet");
            strFldName.AppendFormatted(c4Index + 1);

            // Set the outlet name to the same, since we use it below
            strOutletName = strFldName;
        }

        const tCIDLib::TCard1 c1CurEP = tCIDLib::TCard1(c4Index + m_c4FirstEPNum);
        TZWCCImplBin* pzwcciOutlet = nullptr;
        if (m_eClass == tZWaveUSB3Sh::ECClasses::Basic)
        {
            pzwcciOutlet = new TZWCCImplBasicSwitchF
            (
                &unitiUs
                , strFldName
                , tCQCKit::EFldSTypes::Generic
                , c1CurEP
            );
        }
         else if (m_eClass == tZWaveUSB3Sh::ECClasses::BinSwitch)
        {
            pzwcciOutlet = new TZWCCImplBinSwitchF
            (
                &unitiUs
                , strFldName
                , tCQCKit::EFldSTypes::Generic
                , c1CurEP
            );
        }
         else
        {
            if(facZWaveUSB3Sh().bLowTrace())
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"Unit %(1) unhandled impl class", strName()
                    , tZWaveUSB3Sh::strXlatECClasses(m_eClass)
                );
            }
        }

        //
        //  Set the configured name (or default) as the description on this CC. We
        //  assume for now always read/write so force read/write.
        //
        pzwcciOutlet->SetAccess
        (
            tZWaveUSB3Sh::EUnitAcc::ReadWrite, tZWaveUSB3Sh::EUnitAcc::AllBits
        );
        pzwcciOutlet->strDesc(strOutletName);

        // Break out on any failure
        bRes = bAddImplObject(pzwcciOutlet, c4ImplId);
        if (!bRes)
            break;
    }

    return bRes;
}


//
//  We expose an option for each end point, to let the user set the per-end
//  point name. We default them to OutletX, where is the 1 based outlet index
//  which is what would make sense to the user.
//
tCIDLib::TVoid
TNChOutletUnit::QueryUnitAttrs( tCIDMData::TAttrList&   colToFill
                                , TAttrData&            adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();

    adatTmp.Set(L"Outlet Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colToFill.objAdd(adatTmp);

    TString strDesc;
    TString strDefName;
    TString strOptKey;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4EPCount; c4Index++)
    {
        // Create the key for this one
        strOptKey = L"/UnitOpt/Outlet/=";
        strOptKey.AppendFormatted(c4Index + 1);
        strOptKey.Append(L"/Name");

        // Create the description for the user for editing
        strDesc = L"Outlet ";
        strDesc.AppendFormatted(c4Index + 1);
        strDesc.Append(L" Name");

        strDefName = L"Outlet";
        strDefName.AppendFormatted(c4Index+ 1);

        adatTmp.Set
        (
            strDesc
            , strOptKey
            , tCIDMData::EAttrTypes::String
            , tCIDMData::EAttrEdTypes::InPlace
        );
        adatTmp.SetString(unitiUs.strQueryOptVal(strOptKey, strDefName));
        colToFill.objAdd(adatTmp);
    }
}
