//
// FILE NAME: ZWaveUSB3Sh_CCColorSwitch.cpp
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
//  This is the implementation for the color switch command class namespace.
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
RTTIDecls(TZWCCImplCLghtRGB, TZWCCImpl)
RTTIDecls(TZWCCImplCLghtRGBF, TZWCCImplCLghtRGB)



// -------------------------------------------------------------------------
//   CLASS: TZWCCImplCLghtRGB
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplCLghtRGB: Constructors and destructor
// ---------------------------------------------------------------------------

// We are never multi-end point, so we just pass 0xFF as the end point
TZWCCImplCLghtRGB::TZWCCImplCLghtRGB(TZWUnitInfo* const         punitiThis
                                    , const tCIDLib::TCard1     c1WhiteComp) :

    TZWCCImpl(punitiThis, tZWaveUSB3Sh::ECClasses::ClrSwitch)
    , m_c1WhiteComp(c1WhiteComp)
{
    //
    //  Set our default access to read/write. We won't undo any optional read
    //  flags if they are set, though that's not likely at this point.
    //
    SetAccess(tZWaveUSB3Sh::EUnitAcc::ReadWrite, tZWaveUSB3Sh::EUnitAcc::ReadWrite);
}

TZWCCImplCLghtRGB::~TZWCCImplCLghtRGB()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplCLghtRGB: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCImplCLghtRGB::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L" %(1)/%(2) - SendValueQuery() called for non-readable color light"
            , strUnitName()
            , strDesc()
        );
        return;
    }

    //
    //  Just queue up a query comp for each component, though not white, even if
    //  supported, since it is write only.
    //
    try
    {
        QueryComp(2);
    }

    catch(TError& errToCatch)
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2) - Failed to send red color comp query"
            , strUnitName()
            , strDesc()
        );

        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    try
    {
        QueryComp(3);
    }

    catch(TError& errToCatch)
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2) - Failed to send green color comp query"
            , strUnitName()
            , strDesc()
        );

        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    try
    {
        QueryComp(4);
    }

    catch(TError& errToCatch)
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2) - Failed to send blue color comp query"
            , strUnitName()
            , strDesc()
        );

        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}



// ---------------------------------------------------------------------------
//  TZWCCImplCLghtRGB: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Sets the overall color. We get a formatted out HSV color, space separated.
tCQCKit::ECommResults TZWCCImplCLghtRGB::eSetColor(const TString& strValue)
{
    // Sanity check
    if (!bIsWriteable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2) - SendValue() called for non-writeable color light"
            , strUnitName()
            , strDesc()
        );
        return tCQCKit::ECommResults::Access;
    }

    // Parse out the color
    THSVClr hsvSet;
    if (!hsvSet.bParseFromText(strValue, kCIDLib::True, tCIDLib::ERadices::Auto, L' '))
        return tCQCKit::ECommResults::BadValue;

    // Now let's convert that HSV color to RGB
    TRGBClr rgbSet;
    hsvSet.ToRGB(rgbSet);

    // And now we can set all of the color components. It works or throws
    SetComps(rgbSet.c1Red(), rgbSet.c1Green(), rgbSet.c1Blue());
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults  TZWCCImplCLghtRGB::eSetComp(const TString& strNewVal)
{
    // Sanity check
    if (!bIsWriteable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2) - SendValue() called for non-writeable color light component"
            , strUnitName()
            , strDesc()
        );
        return tCQCKit::ECommResults::Access;
    }

    // Parse the command, which has two space separated values
    TString strComp(strNewVal);
    TString strVal;
    if (!strComp.bSplit(strVal, kCIDLib::chSpace))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2) - Invalid SetComp() value '%(3)'"
            , strUnitName()
            , strDesc()
            , strNewVal
        );
        return tCQCKit::ECommResults::BadValue;
    }

    strComp.StripWhitespace();
    strVal.StripWhitespace();
    tCIDLib::TCard4 c4Percent;
    if (!strVal.bToCard4(c4Percent) || (c4Percent> 100))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2) - Invalid SetComp() percent '%(3)'"
            , strUnitName()
            , strDesc()
            , strVal
        );
        return tCQCKit::ECommResults::BadValue;
    }

    // Convert the component name to a color component
    tCIDLib::TCard1 c1CompId = 0xFF;
    if (strComp.bCompareI(L"Red"))
        c1CompId = 2;
    else if (strComp.bCompareI(L"Green"))
        c1CompId = 3;
    else if (strComp.bCompareI(L"Blue"))
        c1CompId = 4;
    else if (strComp.bCompareI(L"White") && (m_c1WhiteComp != 0xFF))
        c1CompId = m_c1WhiteComp;
    else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2) - Unsupported color comp name '%(3)'"
            , strUnitName()
            , strDesc()
            , strComp
        );
        return tCQCKit::ECommResults::BadValue;
    }

    // Scale the percent to the 0 to 255 range
    tCIDLib::TFloat8 f8Val = (tCIDLib::TFloat8(c4Percent) / 100) * 255;
    TMathLib::Round(f8Val);
    tCIDLib::TCard1 c1Val = tCIDLib::TCard1(f8Val);

    // Call a helper to set a single component. It works or throws
    SetComp(c1CompId, c1Val);
    return tCQCKit::ECommResults::Success;
}



// ---------------------------------------------------------------------------
//  TZWCCImplCLghtRGB: Protected, inherited methods
// ---------------------------------------------------------------------------

// We just need to process color switch reports
tZWaveUSB3Sh::ECCMsgRes
TZWCCImplCLghtRGB::eHandleCCMsg(const   tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimNew)
{
    if ((c1Class == COMMAND_CLASS_SWITCH_COLOR)
    &&  (c1Cmd == SWITCH_COLOR_REPORT))
    {
        const tCIDLib::TCard1 c1CompNum = zwimNew.c1CCByteAt(2);
        const tCIDLib::TCard1 c1CompVal = zwimNew.c1CCByteAt(3);

        if ((c1CompNum >= 2) && (c1CompNum <= 4))
        {
            // We have to combine the new value with the existing ones
            TRGBClr rgbNew(m_rgbColor);

            //
            //  If it's one of the components we have to deal with. We only do
            //  RGB here. There's no field for white, it's outgoing only.
            //
            if (c1CompNum == 2)
                rgbNew.c1Red(c1CompVal);
            else if (c1CompNum == 3)
                rgbNew.c1Green(c1CompVal);
            else if (c1CompNum == 4)
                rgbNew.c1Blue(c1CompVal);

            // If we are currently in error or the value changed, then handle it
            if (bError() || (rgbNew != m_rgbColor))
            {
                m_rgbColor = rgbNew;
                ReportValChange(tZWaveUSB3Sh::EValSrcs::Unit, tCIDLib::TInt4(c1CompNum));
            }
            return tZWaveUSB3Sh::ECCMsgRes_Value;
        }
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}

// ---------------------------------------------------------------------------
//  TZWCCImplCLghtRGB: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Queue up an async query for a specific component
tCIDLib::TVoid TZWCCImplCLghtRGB::QueryComp(const tCIDLib::TCard1 c1CompId)
{
    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiOwner()
            , COMMAND_CLASS_SWITCH_COLOR
            , SWITCH_COLOR_GET
            , SWITCH_COLOR_REPORT
            , c1CompId
            , tZWaveUSB3Sh::EMsgPrios::Async
        );
        SendMsgAsync(zwomOut);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



//
//  Set a single color component. This is done at command priority, and we wait
//  for the trans ack.
//
tCIDLib::TVoid
TZWCCImplCLghtRGB::SetComp(const tCIDLib::TCard1 c1CompId, const tCIDLib::TCard1 c1Val)
{
    try
    {
        // This is a set, so ask for a trans ack
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiOwner()
            , COMMAND_CLASS_SWITCH_COLOR
            , SWITCH_COLOR_SET
            , 0
            , 1
            , c1CompId
            , c1Val
            , tZWaveUSB3Sh::EMsgPrios::Command
            , kCIDLib::True
        );
        SendWaitAck(zwomOut);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Set all three color components. This is done at command priority and we wait
//  for the trans ack.
//
tCIDLib::TVoid
TZWCCImplCLghtRGB::SetComps(const   tCIDLib::TCard1     c1Red
                            , const tCIDLib::TCard1     c1Green
                            , const tCIDLib::TCard1     c1Blue)
{
    try
    {
        TZWOutMsg zwomOut;
        zwomOut.Reset
        (
            c1UnitId()
            , COMMAND_CLASS_SWITCH_COLOR
            , SWITCH_COLOR_SET
            , 0
            , 0
            , tZWaveUSB3Sh::EMsgPrios::Command
        );
        zwomOut += 4;
        zwomOut += 2;
        zwomOut += c1Red;
        zwomOut += 3;
        zwomOut += c1Green;
        zwomOut += 4;
        zwomOut += c1Blue;
        zwomOut += 0;
        zwomOut += 0;
        zwomOut.EndCounter();

        // This is a set command, so let's ask for a receipt ack
        zwomOut.AppendTransOpts(kCIDLib::True);
        zwomOut.AppendCallback();
        zwomOut.Finalize(unitiOwner());

        SendWaitAck(zwomOut);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



// -------------------------------------------------------------------------
//   CLASS: TZWCCImplCLghtRGBF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplCLghtRGBF: Constructors and destructor
// ---------------------------------------------------------------------------

// We are never multi-end point, so we just pass 0xFF as the end point
TZWCCImplCLghtRGBF::TZWCCImplCLghtRGBF(         TZWUnitInfo* const  punitiThis
                                        , const TString&            strSubUnit
                                        , const tCIDLib::TCard1     c1WhiteComp) :

    TZWCCImplCLghtRGB(punitiThis, c1WhiteComp)
    , m_c4FldId_Color(kCIDLib::c4MaxCard)
    , m_c4FldId_SetComp(kCIDLib::c4MaxCard)
    , m_strSubUnit(strSubUnit)
{
}

TZWCCImplCLghtRGBF::~TZWCCImplCLghtRGBF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplCLghtRGBF: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWCCImplCLghtRGBF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return
    (
        (c4FldId == m_c4FldId_Color)
        || (c4FldId == m_c4FldId_SetComp)
    );
}


tCIDLib::TBoolean
TZWCCImplCLghtRGBF::bStringFldChanged(  const   TString&                strFldName
                                        , const tCIDLib::TCard4         c4FldId
                                        , const TString&                strValue
                                        ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    try
    {
        if (c4FldId == m_c4FldId_Color)
            eRes = eSetColor(strValue);
         else if (c4FldId == m_c4FldId_SetComp)
            eRes = eSetComp(strValue);
        else
            bRes = kCIDLib::False;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


tCIDLib::TVoid
TZWCCImplCLghtRGBF::QueryFldDefs(tCQCKit::TFldDefList& colAddTo, TCQCFldDef& flddTmp) const
{
    // The V2 color light
    TString strFldName = L"CLGHT#";
    if (!m_strSubUnit.bIsEmpty())
    {
        strFldName.Append(m_strSubUnit);
        strFldName.Append(kCIDLib::chTilde);
    }
    strFldName.Append(strUnitName());
    flddTmp.Set
    (
        strFldName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::ClrLight
    );
    colAddTo.objAdd(flddTmp);

    // And one of our own set set the color components
    strFldName = strUnitName();
    if (!m_strSubUnit.bIsEmpty())
    {
        strFldName.Append(kCIDLib::chUnderscore);
        strFldName.Append(m_strSubUnit);
    }
    strFldName.Append(L"_SetComp");
    flddTmp.Set(strFldName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Write);
    colAddTo.objAdd(flddTmp);
}



tCIDLib::TVoid TZWCCImplCLghtRGBF::StoreFldIds()
{
    TString strFldName = strUnitName();
    if (!m_strSubUnit.bIsEmpty())
    {
        strFldName.Append(kCIDLib::chUnderscore);
        strFldName.Append(m_strSubUnit);
    }
    strFldName.Append(L"_SetComp");
    m_c4FldId_SetComp = c4LookupFldId(strFldName);

    // The V2 color light field
    strFldName = L"CLGHT#";
    if (!m_strSubUnit.bIsEmpty())
    {
        strFldName.Append(m_strSubUnit);
        strFldName.Append(kCIDLib::chTilde);
    }
    strFldName.Append(strUnitName());
    m_c4FldId_Color = c4LookupFldId(strFldName);

    // If readable, default to error state until we get a value
    if (bIsReadable())
        SetFieldErr(m_c4FldId_Color);
}


// ---------------------------------------------------------------------------
//  TZWCCImplCLghtRGBF: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWCCImplCLghtRGBF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_Color);
}


tCIDLib::TVoid
TZWCCImplCLghtRGBF::ValueChanged(const  tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          i4ExtraInfo)
{
    //
    //  We have to get the current RGB color from our parent, change it to an HSV,
    //  then format it out and write it.
    //
    THSVClr hsvNew(rgbColor());
    TString strValue;
    hsvNew.FormatToText(strValue, kCIDLib::chSpace, kCIDLib::True);
    bWriteFld(m_c4FldId_Color, strValue);

    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}
