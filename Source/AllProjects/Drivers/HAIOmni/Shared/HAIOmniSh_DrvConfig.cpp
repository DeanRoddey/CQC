//
// FILE NAME: HAIOmniSh_DrvConfig.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2006
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
//  This file implements the driver configuration class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniSh.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TOmniDrvCfg,TObject)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace HAIOmniSh_DrvConfig
{
    // -----------------------------------------------------------------------
    //  Our persistent format
    //
    //  Version 2 -
    //      Changed the zone array from a boolean flag array to an enum based
    //      array so that we can indicate unused or, if used, what type of
    //      zone.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1FmtVersion = 2;
}



// ---------------------------------------------------------------------------
//   CLASS: TOmniDrvCfg
//  PREFIX: dcfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TOmniDrvCfg: Public, static methods
// ---------------------------------------------------------------------------

//
//  Builds the field name for a given type of item. This functoinality is
//  needed by both client and server sides.
//
tCIDLib::TCard4
TOmniDrvCfg::c4MakeItemName(const   tHAIOmniSh::EItemTypes  eType
                            , const tCIDLib::TCard4         c4ItemNum
                            ,       TString&                strToFill)
{
    tCIDLib::TCard4 c4NumWidth;
    switch(eType)
    {
        case tHAIOmniSh::EItem_Area :
            c4NumWidth = 1;
            strToFill = L"Area";
            break;

        case tHAIOmniSh::EItem_Button :
            c4NumWidth = 3;
            strToFill = L"Butt";
            break;

        case tHAIOmniSh::EItem_Code :
            c4NumWidth = 2;
            strToFill = L"Code";
            break;

        case tHAIOmniSh::EItem_Message :
            c4NumWidth = 3;
            strToFill = L"Msg";
            break;

        case tHAIOmniSh::EItem_Thermo :
            c4NumWidth = 2;
            strToFill = L"Thermo";
            break;

        case tHAIOmniSh::EItem_Unit :
            c4NumWidth = 3;
            strToFill = L"Unit";
            break;

        case tHAIOmniSh::EItem_Zone :
            c4NumWidth = 3;
            strToFill = L"Zone";
            break;
    };

    // Figure out how many incoming digits we have
    tCIDLib::TCard4 c4InDigs;
    if (c4ItemNum < 10)
        c4InDigs = 1;
    else if (c4ItemNum < 100)
        c4InDigs = 2;
    else
        c4InDigs = 3;

    //
    //  Based on the required width of the number field in the field name,
    //  add some leading zeros if the incoming digits is less than what is
    //  required to fill up the width.
    //
    if (c4NumWidth == 2)
    {
        if (c4InDigs == 1)
            strToFill.Append(L"0");
    }
     else if (c4NumWidth == 3)
    {
        if (c4InDigs == 1)
            strToFill.Append(L"00");
        else if (c4InDigs == 2)
            strToFill.Append(L"0");
    }

    // And append the item number now
    strToFill.AppendFormatted(c4ItemNum);
    return strToFill.c4Length();
}


// ---------------------------------------------------------------------------
//  TOmniDrvCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TOmniDrvCfg::TOmniDrvCfg() :

    m_fcolAreas(kHAIOmniSh::c4MaxAreas)
    , m_fcolPLCUnits(kHAIOmniSh::c4MaxUnits)
    , m_fcolThermos(kHAIOmniSh::c4MaxThermos)
    , m_fcolZones(kHAIOmniSh::c4MaxZones)
{
}

TOmniDrvCfg::TOmniDrvCfg(const  tHAIOmniSh::TFlagArray  fcolAreas
                        , const tHAIOmniSh::TPLCArray   fcolPLCUnits
                        , const tHAIOmniSh::TFlagArray  fcolThermos
                        , const tHAIOmniSh::TZoneArray  fcolZones) :

    m_fcolAreas(fcolAreas)
    , m_fcolPLCUnits(fcolPLCUnits)
    , m_fcolThermos(fcolThermos)
    , m_fcolZones(fcolZones)
{
}

TOmniDrvCfg::TOmniDrvCfg(const TOmniDrvCfg& dcfgToCopy) :

    m_fcolAreas(dcfgToCopy.m_fcolAreas)
    , m_fcolPLCUnits(dcfgToCopy.m_fcolPLCUnits)
    , m_fcolThermos(dcfgToCopy.m_fcolThermos)
    , m_fcolZones(dcfgToCopy.m_fcolZones)
{
}

TOmniDrvCfg::~TOmniDrvCfg()
{
}


// ---------------------------------------------------------------------------
//  TOmniDrvCfg: Public oeprators
// ---------------------------------------------------------------------------
TOmniDrvCfg& TOmniDrvCfg::operator=(const TOmniDrvCfg& dcfgToAssign)
{
    if (this != &dcfgToAssign)
    {
        m_fcolAreas     = dcfgToAssign.m_fcolAreas;
        m_fcolPLCUnits  = dcfgToAssign.m_fcolPLCUnits;
        m_fcolThermos   = dcfgToAssign.m_fcolThermos;
        m_fcolZones     = dcfgToAssign.m_fcolZones;
    }
    return *this;
}



// ---------------------------------------------------------------------------
//  TOmniDrvCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This driver (as of CQC 5.x) uses a simple file for configuration. The driver opens
//  the file and calls us here to parse it out. He also uses this to convert from the
//  old binary format (stored in the CQC repository) by formatting it out to a string
//  stream and call us here to parse from that.
//
//  The format is very simple. It's something like:
//
//  Areas=1, 2, 3
//  Thermos=1, 14, 16
//  ; This is a comment
//  Units=1:Binary, 2:Flag
//  Zones=3:Alarm, 15:Humidity
//
tCIDLib::TBoolean TOmniDrvCfg::bParseFrom(TTextInStream& strmSrc)
{
    const TString& strAreas(L"Areas");
    const TString& strThermos(L"Thermos");
    const TString& strUnits(L"Units");
    const TString& strZones(L"Zones");

    // We parse into temp arrays and only save if successful
    tHAIOmniSh::TFlagArray  fcolAreas(kHAIOmniSh::c4MaxAreas);
    tHAIOmniSh::TPLCArray   fcolPLCUnits(kHAIOmniSh::c4MaxUnits);
    tHAIOmniSh::TFlagArray  fcolThermos(kHAIOmniSh::c4MaxThermos);
    tHAIOmniSh::TZoneArray  fcolZones(kHAIOmniSh::c4MaxZones);

    strmSrc.Reset();
    tCIDLib::TCard4 c4Num;
    tHAIOmniSh::EUnitTypes eUnitType;
    tHAIOmniSh::EZoneTypes eZoneType;
    TString strCurLn;
    TString strNum;
    TString strType;
    TStringTokenizer stokParse;
    try
    {
        tCIDLib::TCard4 c4LineNum = 0;
        while (!strmSrc.bEndOfStream())
        {
            c4LineNum++;
            strmSrc >> strCurLn;
            strCurLn.StripWhitespace();

            // Skip empty and comment lines
            if (strCurLn.bIsEmpty() || (strCurLn[0] == kCIDLib::chSemiColon))
                continue;

            if (strCurLn.bStartsWithI(strAreas) || strCurLn.bStartsWithI(strThermos))
            {
                // We just break out numbers
                tHAIOmniSh::TFlagArray* pfcolTar;
                if (strCurLn.bStartsWithI(strAreas))
                {
                    pfcolTar = &fcolAreas;
                    strCurLn.CutUpTo(kCIDLib::chEquals);
                }
                else
                {
                    pfcolTar = &fcolThermos;
                    strCurLn.CutUpTo(kCIDLib::chEquals);
                }

                stokParse.Reset(&strCurLn, L",");
                while (stokParse.bGetNextToken(strNum))
                {
                    strNum.StripWhitespace();
                    if (!strNum.bToCard4(c4Num, tCIDLib::ERadices::Dec)
                    ||  (c4Num == 0)
                    ||  (c4Num > pfcolTar->c4ElemCount()))
                    {
                        facHAIOmniSh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Line #%(1) - Invalid area/thermo number"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , TCardinal(c4LineNum)
                        );
                        return kCIDLib::False;
                    }

                    // Mark this one in the list, adjusting downward for an index
                    (*pfcolTar)[c4Num - 1] = kCIDLib::True;
                }
            }
             else if (strCurLn.bStartsWithI(strUnits))
            {
                // We are parsing out number:type pairs
                tCIDLib::TCard4 c4USlot = 0;
                strCurLn.CutUpTo(kCIDLib::chEquals);
                stokParse.Reset(&strCurLn, L",");
                while (stokParse.bGetNextToken(strNum))
                {
                    // Split this guy on a colon
                    c4USlot++;
                    if (!strNum.bSplit(strType, kCIDLib::chColon))
                    {
                        facHAIOmniSh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Line #%(1) - Unit entries must be in the form num:type. Slot=%(2)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , TCardinal(c4LineNum)
                            , TCardinal(c4USlot)
                        );
                        return kCIDLib::False;
                    }

                    strNum.StripWhitespace();
                    strType.StripWhitespace();

                    if (!strNum.bToCard4(c4Num, tCIDLib::ERadices::Dec)
                    ||  (c4Num == 0)
                    ||  (c4Num > kHAIOmniSh::c4MaxUnits))
                    {
                        facHAIOmniSh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Line #%(1) - Invalid unit number. Slot=%(2)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , TCardinal(c4LineNum)
                            , TCardinal(c4USlot)
                        );
                        return kCIDLib::False;
                    }

                    eUnitType = tHAIOmniSh::eXlatEUnitTypes(strType);
                    if (eUnitType == tHAIOmniSh::EUnitTypes_Count)
                    {
                        facHAIOmniSh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Line #%(1) - Invalid unit type. Slot=%(2)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , TCardinal(c4LineNum)
                            , TCardinal(c4USlot)
                        );
                        return kCIDLib::False;
                    }

                    fcolPLCUnits[c4Num - 1] = eUnitType;
                }
            }
             else if (strCurLn.bStartsWithI(strZones))
            {
                // We are parsing out number:type pairs
                tCIDLib::TCard4 c4USlot = 0;
                strCurLn.CutUpTo(kCIDLib::chEquals);
                stokParse.Reset(&strCurLn, L",");
                while (stokParse.bGetNextToken(strNum))
                {
                    // Split this guy on a colon
                    c4USlot++;
                    if (!strNum.bSplit(strType, kCIDLib::chColon))
                    {
                        facHAIOmniSh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Line #%(1) - Zone entries must be in the form num:type. Slot=%(2)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , TCardinal(c4LineNum)
                            , TCardinal(c4USlot)
                        );
                        return kCIDLib::False;
                    }

                    strNum.StripWhitespace();
                    strType.StripWhitespace();

                    if (!strNum.bToCard4(c4Num, tCIDLib::ERadices::Dec)
                    ||  (c4Num == 0)
                    ||  (c4Num > kHAIOmniSh::c4MaxZones))
                    {
                        facHAIOmniSh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Line #%(1) - Invalid zone number. Slot=%(2)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , TCardinal(c4LineNum)
                            , TCardinal(c4USlot)
                        );
                        return kCIDLib::False;
                    }

                    eZoneType = tHAIOmniSh::eXlatEZoneTypes(strType);
                    if (eZoneType == tHAIOmniSh::EZoneTypes_Count)
                    {
                        facHAIOmniSh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Line #%(1) - Invalid zone type.Slot=%(2)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , TCardinal(c4LineNum)
                            , TCardinal(c4USlot)
                        );
                        return kCIDLib::False;
                    }

                    fcolZones[c4Num - 1] = eZoneType;
                }
            }
        }

        // It worked so store the info
        m_fcolAreas = fcolAreas;
        m_fcolPLCUnits = fcolPLCUnits;
        m_fcolThermos = fcolThermos;
        m_fcolZones = fcolZones;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


const tHAIOmniSh::TFlagArray& TOmniDrvCfg::fcolAreas() const
{
    return m_fcolAreas;
}

tHAIOmniSh::TFlagArray& TOmniDrvCfg::fcolAreas()
{
    return m_fcolAreas;
}

const tHAIOmniSh::TPLCArray& TOmniDrvCfg::fcolPLCUnits() const
{
    return m_fcolPLCUnits;
}

tHAIOmniSh::TPLCArray& TOmniDrvCfg::fcolPLCUnits()
{
    return m_fcolPLCUnits;
}

const tHAIOmniSh::TFlagArray& TOmniDrvCfg::fcolThermos() const
{
    return m_fcolThermos;
}

tHAIOmniSh::TFlagArray& TOmniDrvCfg::fcolThermos()
{
    return m_fcolThermos;
}

const tHAIOmniSh::TZoneArray& TOmniDrvCfg::fcolZones() const
{
    return m_fcolZones;
}

tHAIOmniSh::TZoneArray& TOmniDrvCfg::fcolZones()
{
    return m_fcolZones;
}


tCIDLib::TVoid TOmniDrvCfg::Reset()
{
    // Reset all of the flag arrays
    m_fcolAreas.SetAll(kCIDLib::False);
    m_fcolPLCUnits.SetAll(tHAIOmniSh::EUnit_Unused);
    m_fcolThermos.SetAll(kCIDLib::False);
    m_fcolZones.SetAll(tHAIOmniSh::EZone_Unused);
}



// ---------------------------------------------------------------------------
//  TOmniDrvCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TOmniDrvCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniSh_DrvConfig::c1FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom >> m_fcolAreas
                   >> m_fcolPLCUnits
                   >> m_fcolThermos;

    //
    //  If pre-V2, then we read in a flag array, and for each one that is
    //  on, we set the zone type to alarm. Else, just read in the new V2
    //  data.
    //
    if (c1FmtVersion < 2)
    {
        tHAIOmniSh::TFlagArray fcolOld;
        strmToReadFrom >> fcolOld;
        const tCIDLib::TCard4 c4Count = fcolOld.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (fcolOld[c4Index])
                m_fcolZones[c4Index] = tHAIOmniSh::EZone_Alarm;
            else
                m_fcolZones[c4Index] = tHAIOmniSh::EZone_Unused;
        }
    }
     else
    {
        strmToReadFrom >> m_fcolZones;
    }

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


//
//  Format out our content to the new text format. This is just used to do the
//  conversion for existing users to the new text format.
//
tCIDLib::TVoid TOmniDrvCfg::FormatTo(TTextOutStream& strmTar)
{
    // Do the areas
    strmTar << L"Areas=";
    tCIDLib::TCard4 c4SoFar = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxAreas; c4Index++)
    {
        if (m_fcolAreas[c4Index])
        {
            if (c4SoFar)
                strmTar << L", ";
            strmTar << (c4Index + 1);
            c4SoFar++;
        }
    }
    strmTar << kCIDLib::DNewLn;

    // Do the thermos
    strmTar << L"Thermos=";
    c4SoFar = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxThermos; c4Index++)
    {
        if (m_fcolThermos[c4Index])
        {
            if (c4SoFar)
                strmTar << L", ";
            strmTar << (c4Index + 1);
            c4SoFar++;
        }
    }
    strmTar << kCIDLib::DNewLn;


    // Do the units
    strmTar << L"Units=";
    c4SoFar = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxUnits; c4Index++)
    {
        const tHAIOmniSh::EUnitTypes eType = m_fcolPLCUnits[c4Index];
        if (eType != tHAIOmniSh::EUnit_Unused)
        {
            if (c4SoFar)
                strmTar << L", ";

            strmTar << (c4Index + 1) << L":";

            if (eType == tHAIOmniSh::EUnit_Binary)
                strmTar << L"Binary";
            else if (eType == tHAIOmniSh::EUnit_Dimmer)
                strmTar << L"Dimmer";
            else if (eType == tHAIOmniSh::EUnit_Flag)
                strmTar << L"Flag";

            c4SoFar++;
        }
    }
    strmTar << kCIDLib::DNewLn;


    // Do the zones
    strmTar << L"Zones=";
    c4SoFar = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxZones; c4Index++)
    {
        const tHAIOmniSh::EZoneTypes eType = m_fcolZones[c4Index];
        if (eType != tHAIOmniSh::EZone_Unused)
        {
            if (c4SoFar)
                strmTar << L", ";

            strmTar << (c4Index + 1) << L":";

            if (eType == tHAIOmniSh::EZone_Alarm)
                strmTar << L"Alarm";
            else if (eType == tHAIOmniSh::EZone_Humidity)
                strmTar << L"Humidity";
            else if (eType == tHAIOmniSh::EZone_Temp)
                strmTar << L"Temp";

            c4SoFar++;
        }
    }
    strmTar << kCIDLib::DNewLn;

}


// Not used anymore, for the old client side driver based configuration
tCIDLib::TVoid TOmniDrvCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniSh_DrvConfig::c1FmtVersion
                    << m_fcolAreas
                    << m_fcolPLCUnits
                    << m_fcolThermos
                    << m_fcolZones
                    << tCIDLib::EStreamMarkers::EndObject;
}


