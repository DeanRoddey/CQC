//
// FILE NAME: ZWaveLevi2Sh_ColorSwitch.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2014
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
//  This file provides the implementation for the battery command class derivatives.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCColorSwitch, TZWCmdClass)




// ---------------------------------------------------------------------------
//   CLASS: TZWCCColorSwitch
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCColorSwitch: Cosntructors Destructor
// ---------------------------------------------------------------------------
TZWCCColorSwitch::TZWCCColorSwitch(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::ColorSwitch, zwdcInst, COMMAND_CLASS_SWITCH_COLOR)
    , m_c1Red(0)
    , m_c1Green(0)
    , m_c1Blue(0)
    , m_c4FldId_Color(kCIDLib::c4MaxCard)
{
}

TZWCCColorSwitch::~TZWCCColorSwitch()
{
}


// ---------------------------------------------------------------------------
//  TZWCCColorSwitch: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCColorSwitch::bHandleGenReport( const   tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    // If not enabled, do nothing
    if ((m_c4FldId_Color == kCIDLib::c4MaxCard) || (fcolVals.c4ElemCount() < 4))
        return kCIDLib::False;

    if ((c4FromInst == c4InstId())
    &&  (fcolVals[0] == COMMAND_CLASS_SWITCH_COLOR)
    &&  ((fcolVals[1] == SWITCH_COLOR_REPORT) || (fcolVals[1] == SWITCH_COLOR_SET)))
    {
        tCIDLib::TCard4 c4StartInd = 2;
        tCIDLib::TCard4 c4CompCnt = 1;
        if (fcolVals[1] == SWITCH_COLOR_SET)
            c4CompCnt = (fcolVals[c4StartInd++] & 0x7F);

        // Store the color components being reported
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CompCnt; c4Index++)
        {
            const tCIDLib::TCard4 c4CompId = fcolVals[c4StartInd++];
            const tCIDLib::TCard4 c4CompVal = fcolVals[c4StartInd++];

            if (c4CompId == 2)
                m_c1Red = tCIDLib::TCard1(c4CompVal);
            else if (c4CompId == 3)
                m_c1Green = tCIDLib::TCard1(c4CompVal);
            else if (c4CompId == 4)
                m_c1Blue = tCIDLib::TCard1(c4CompVal);
        }

        // And now update the computed HSV field value
        StoreValue(unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCColorSwitch::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Color);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCColorSwitch::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    const tCIDLib::TCard4 c4CmdClass = COMMAND_CLASS_SWITCH_COLOR;
    const tCIDLib::TCard4 c4Get      = SWITCH_COLOR_GET;
    const tCIDLib::TCard4 c4Report   = SWITCH_COLOR_REPORT;

    //
    //  We have to do three separate messages here, to get the three components because
    //  there is retardedly no query for all three at once.
    //
    tCIDLib::TCardList fcolVals(8);
    fcolVals.c4AddElement(COMMAND_CLASS_SWITCH_COLOR);
    fcolVals.c4AddElement(SWITCH_COLOR_GET);
    fcolVals.c4AddElement(2);
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);
    if (mzwsfLevi.bLeviWaitUnitMsg(unitMe, *this, c4CmdClass, c4Report, fcolVals))
    {
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());

        // That worked so do the green
        fcolVals.RemoveAll();
        fcolVals.c4AddElement(COMMAND_CLASS_SWITCH_COLOR);
        fcolVals.c4AddElement(SWITCH_COLOR_GET);
        fcolVals.c4AddElement(3);
        mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);
        if (mzwsfLevi.bLeviWaitUnitMsg(unitMe, *this, c4CmdClass, c4Report, fcolVals))
        {
            bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());

            // That worked so do the blue
            fcolVals.RemoveAll();
            fcolVals.c4AddElement(COMMAND_CLASS_SWITCH_COLOR);
            fcolVals.c4AddElement(SWITCH_COLOR_GET);
            fcolVals.c4AddElement(4);
            mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);
            if (mzwsfLevi.bLeviWaitUnitMsg(unitMe, *this, c4CmdClass, c4Report, fcolVals))
                bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
        }
    }
     else
    {
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Color);
    }
    return kCIDLib::False;
}


tCQCKit::ECommResults
TZWCCColorSwitch::eWriteStringFld(  const   tCIDLib::TCard4     c4FldId
                                    , const TString&            strNewValue
                                    ,       TZWaveUnit&         unitMe
                                    ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (c4FldId != m_c4FldId_Color)
        return tCQCKit::ECommResults::FieldNotFound;

    // Parse out the HSV values
    TString strHue = strNewValue;
    TString strSat;
    strHue.bSplit(strSat, kCIDLib::chSpace);
    TString strVal;
    strSat.bSplit(strVal, kCIDLib::chSpace);

    const tCIDLib::TFloat4 f4Hue = tCIDLib::TFloat4(strHue.f8Val());
    const tCIDLib::TFloat4 f4Sat = tCIDLib::TFloat4(strSat.c4Val() / 255.0);
    const tCIDLib::TFloat4 f4Val = tCIDLib::TFloat4(strVal.c4Val() / 255.0);

    // And now convert that to RGB
    TRGBClr clrNew;
    clrNew.FromHSV(f4Hue, f4Sat, f4Val);

    tCIDLib::TCardList fcolVals(16);
    fcolVals.c4AddElement(COMMAND_CLASS_SWITCH_COLOR);
    fcolVals.c4AddElement(SWITCH_COLOR_SET);
    fcolVals.c4AddElement(3);
    fcolVals.c4AddElement(2);
    fcolVals.c4AddElement(clrNew.c1Red());
    fcolVals.c4AddElement(3);
    fcolVals.c4AddElement(clrNew.c1Green());
    fcolVals.c4AddElement(4);
    fcolVals.c4AddElement(clrNew.c1Blue());

    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);

    return tCQCKit::ECommResults::Success;
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCColorSwitch::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    // We may not be enabled, do nothing if not, since the field id isn't set
    if (m_c4FldId_Color != kCIDLib::c4MaxCard)
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Color);
}



//
//  Add our fields to the list.
//
tCIDLib::TVoid
TZWCCColorSwitch::QueryFldDefs(         tCQCKit::TFldDefList& colAddTo
                                ,       TCQCFldDef&           flddTmp
                                , const TZWaveUnit&           unitMe
                                ,       TString&              strTmp1
                                ,       TString&              strTmp2) const
{
    strTmp1 = L"CLGHT#";
    strTmp1.Append(unitMe.strName());
    flddTmp.Set(strTmp1, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::ReadWrite);
    colAddTo.objAdd(flddTmp);
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCColorSwitch::StoreFldIds(  const   TZWaveUnit&         unitMe
                                , const MZWLeviSrvFuncs&    mzwsfLevi
                                ,       TString&            strTmp1
                                ,       TString&            strTmp2)
{
    strTmp1 = L"CLGHT#";
    strTmp1.Append(unitMe.strName());
    m_c4FldId_Color = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCColorSwitch: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We keep track of each of the color components. When one changes, we re-format them
//  into the string field, since all three components must be set. And we have to change
//  them from RGB to HSV.
//
tCIDLib::TVoid
TZWCCColorSwitch::StoreValue(const  TZWaveUnit&         unitMe
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    // Combine the RGB values into a color object
    TRGBClr rgbOrg(m_c1Red, m_c1Green, m_c1Blue);

    //
    //  Ask it to convert that to HSV for us. The saturation and value are 0 to 1. The
    //  hue is 0 to 360, or 359 anyway.
    //
    tCIDLib::TFloat4    f4Hue;
    tCIDLib::TFloat4    f4Saturation;
    tCIDLib::TFloat4    f4Value;
    rgbOrg.ToHSV(f4Hue, f4Saturation, f4Value);

    // We have to scale the saturation and value back to 0 to 0xFF
    const tCIDLib::TCard4 c4Sat = tCIDLib::TCard1(255 * f4Saturation);
    const tCIDLib::TCard4 c4Value = tCIDLib::TCard1(255 * f4Value);

    TString strVal(16UL);
    strVal.AppendFormatted(tCIDLib::TCard4(f4Hue));
    strVal.Append(kCIDLib::chSpace);
    strVal.AppendFormatted(c4Sat);
    strVal.Append(kCIDLib::chSpace);
    strVal.AppendFormatted(c4Value);

    mzwsfLevi.bLeviStoreStrFld(m_c4FldId_Color, strVal);
}
