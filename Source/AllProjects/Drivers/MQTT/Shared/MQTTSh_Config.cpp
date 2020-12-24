//
// FILE NAME: MQTTSh_Config.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/31/2018
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
//  This is the implementation for the MQTT driver's config file related classes
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MQTTSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TMQTTFldCfg, TObject)
RTTIDecls(TMQTTCfg, TObject)


//
//  The DTD for our configuration data, so that we can get max benefit from
//  the overhead of using the XML parser.
//
static TString strEmbeddedCfgDTD =
L"<?xml encoding='$NativeWideChar$'?>\n"
L"<!ENTITY % Versions  '1'>\n"

// Stuff related to describing the playload data
L"<!ENTITY % FldTypes       'Boolean | Card | Float | Int | String'>\n"
L"<!ENTITY % PLDecDigits    '1 | 2 | 3 | 4 | 5'>\n"
L"<!ENTITY % PLPrecisTypes  '1 | 2 | 3 | 4'>\n"
L"<!ENTITY % PLSize         '1 | 2 | 4'>\n"
L"<!ENTITY % PLTypes        'BinText | Card | Int | Text'>\n"
L"<!ENTITY % ValDirs        'In | InOut | Out'>\n"
L"<!ENTITY % BaseMaps       'BoolGenMap | BoolNumMap | BoolTextMap | EnumMap\n"
L"                           | MRangeMap | NumMap | ScaleRangeMap | TextMap'>\n"
L"<!ENTITY % ValueMaps      'JSONMap | MLineMap | %BaseMaps;'>\n"


//
// We need various types of maps to map between field/MQTT values
//

//
//  Each value maps from one or more MQTT source values to a single target
//  field value. Or, from from one or more source field values to a single
//  MQTT target value. The former is an In type and the latter is an Out type.
//
//  When doing field writes, the first value that contains a source value
//  that matches the field value being written will be used.
//
//  For boolean fields, if it's numeric and the MQTT values are 0 and 1, or
//  it's text and the MQTT values are false/true or text 0/1 values, then
//  you don't need to provide a map. The driver will figure that out.
//  Otherwise, use a text or numeric map (based on the text/binary format
//  of the MQTT data) and use field values of False and True.
//
//  There are some other special case ones, such as RangeScale that just
//  scale between a field range and an MQTT range.
//


//
//  When it's a boolean field and there are numeric values that map to
//  the boolean values.
//
L"<!ELEMENT BoolNumVal      EMPTY>\n"
L"<!ATTLIST BoolNumVal      TarVal (False|True) #REQUIRED\n"
L"                          MinSrc CDATA #REQUIRED\n"
L"                          MaxSrc CDATA #REQUIRED>\n"
L"<!ELEMENT BoolNumMap      (BoolNumVal+)>\n"
L"<!ATTLIST BoolNumMap      FalseOut CDATA #IMPLIED\n"
L"                          TrueOut CDATA #IMPLIED>\n"


//
//  When it's a boolean field, if the MQTT payload is text and the values
//  will be 0/1 or false/true, or if binary and the values are 0 and non-zero,
//  this can be used and it will automatically adjust to that.
//
L"<!ELEMENT BoolGenMap      EMPTY>\n"
L"<!ATTLIST BoolGenMap      FalseOut CDATA #IMPLIED\n"
L"                          TrueOut CDATA #IMPLIED>\n"

//
//  For mapping arbitrary text values to boolean fields. Similar to
//  BoolNumMap above but for text.
//
L"<!ELEMENT BoolTextVal     EMPTY>\n"
L"<!ATTLIST BoolTextVal     TarVal (False|True) #REQUIRED\n"
L"                          MQTTVal CDATA #REQUIRED>\n"
L"<!ELEMENT BoolTextMap     (BoolTextVal+)>\n"
L"<!ATTLIST BoolTextMap     FalseOut CDATA #IMPLIED\n"
L"                          TrueOut CDATA #IMPLIED>\n"

//
//  For straight enumerated text mapping, where there are a small set of
//  values and each one maps back and forth in both directions.
//
L"<!ELEMENT EnumVal         EMPTY>\n"
L"<!ATTLIST EnumVal         FldVal CDATA #REQUIRED\n"
L"                          MQTTVal CDATA #REQUIRED>\n"
L"<!ELEMENT EnumMap         (EnumVal+)>\n"


//
//  Maps multiple ranges of values to single target values in either
//  direction. The source values are in the min/max range attributes
//  (inclusive)
//
L"<!ELEMENT MRangeVal       EMPTY>\n"
L"<!ATTLIST MRangeVal       TarVal CDATA #REQUIRED\n"
L"                          Dir (%ValDirs;) #REQUIRED\n"
L"                          MinVal CDATA #REQUIRED\n"
L"                          MaxVal CDATA #REQUIRED>\n"
L"<!ELEMENT MRangeMap       (MRangeVal+)>\n"

//
//  This one does a small set of one to one numeric mappings. SO there
//  is a single source value for each target value.
//
L"<!ELEMENT NumVal          EMPTY>\n"
L"<!ATTLIST NumVal          TarVal CDATA #REQUIRED\n"
L"                          Dir (%ValDirs;) #REQUIRED\n"
L"                          SrcVal CDATA #REQUIRED>\n"
L"<!ELEMENT NumMap          (NumVal+)>\n"

//
//  This one does a a simple scaling between two numeric ranges
//
L"<!ELEMENT ScaleRangeMap   EMPTY>\n"
L"<!ATTLIST ScaleRangeMap   MQMin CDATA #REQUIRED\n"
L"                          MQMax CDATA #REQUIRED\n"
L"                          FldMin CDATA #REQUIRED\n"
L"                          FldMax CDATA #REQUIRED>\n"

//
//  For text mappings. The source values are in the body text of each entry,
//  as a quoted comma list.
//
L"<!ELEMENT TextVal         (#PCDATA)>\n"
L"<!ATTLIST TextVal         TarVal CDATA #REQUIRED\n"
L"                          Dir (%ValDirs;) #REQUIRED>\n"
L"<!ELEMENT TextMap         (TextVal+)>\n"


//
//  For JSON mappings. For incoming, we get a JSON path that tells us
//  how to find the value in the incoming JSON. For outgoing, the
//  JSON to send (with a %(v) replacement value) is provided.
//
//  Since this just helps us handle the JSON stuff, there will be another
//  map defined as a child element, which will be used to do the actual
//  value mapping.
//
L"<!ELEMENT JSONOut         (#PCDATA)\n>"
L"<!ELEMENT JSONMap         ((%BaseMaps;)?, JSONOut?)>\n"
L"<!ATTLIST JSONMap         InPath CDATA #REQUIRED\n"
L"                          ExtractFrom CDATA #IMPLIED\n"
L"                          ExtractTo CDATA #IMPLIED\n"
L"                          IgnoreNotFnd (Yes | No) 'No'\n"
L"                          StripWS (Yes|No) 'Yes'>\n"

//
//  For multi-line mappings, where we have to extract one of the lines, and possibly
//  part of the line that has the value. Then we pass it on to a nested map to actually
//  do the mapping. They give us the number of the line to extract, and we offer a
//  set of options for getting the actual value text from the line.
//
//  If writeable, the outgoing text is in the body, and has a %(v) replacement
//  token.
//
//  ExtractFrom is in the form:
//      [Start|Offset=offset|Char=char]
//
//  ExtractTo is in the form:
//      [End|Count=count|Char=char|Offset=offset]
//
L"<!ELEMENT MLineOut        (#PCDATA)\n>"
L"<!ELEMENT MLineMap        ((%BaseMaps;)?, MLineOut?)\n>"
L"<!ATTLIST MLineMap        LineNum CDATA #REQUIRED\n"
L"                          ExtractFrom CDATA #IMPLIED\n"
L"                          ExtractTo CDATA #IMPLIED\n"
L"                          StripWS (Yes|No) 'Yes'>\n"


//
//  Defines the overall payload format
//
L"<!ELEMENT PLFmt           EMPTY>\n"
L"<!ATTLIST PLFmt           Type (%PLTypes;) #REQUIRED\n"
L"                          Bytes (%PLSize;) #IMPLIED\n"
L"                          Offset CDATA '0'\n"
L"                          Endian (Little | Big) 'Big'\n"
L"                          DecDigits (%PLDecDigits;) #IMPLIED\n"
L"                          Precis (%PLPrecisTypes;) #IMPLIED\n"
L"                          Encoding CDATA #IMPLIED>\n"



// The various types of fields they can define
L"<!ELEMENT BoolSwitch      (PLFmt, (%ValueMaps;)?)>\n"
L"<!ATTLIST BoolSwitch      Topic CDATA #REQUIRED\n"
L"                          BaseName CDATA #REQUIRED\n"
L"                          Access (W | RW) #REQUIRED\n"
L"                          AltInTopic CDATA #IMPLIED\n"
L"                          Retain (Yes|No) 'Yes'\n"
L"                          OnConnect CDATA #IMPLIED>\n"

L"<!ELEMENT ContactClosure  (PLFmt, (%ValueMaps;)?)>\n"
L"<!ATTLIST ContactClosure  Topic CDATA #REQUIRED\n"
L"                          BaseName CDATA #REQUIRED\n"
L"                          AltInTopic CDATA #IMPLIED\n"
L"                          OnConnect CDATA #IMPLIED>\n"

L"<!ELEMENT Generic         (PLFmt, (%ValueMaps;)?)>\n"
L"<!ATTLIST Generic         Topic CDATA #REQUIRED\n"
L"                          BaseName CDATA #REQUIRED\n"
L"                          Access (R | W | RW) #REQUIRED\n"
L"                          AltInTopic CDATA #IMPLIED\n"
L"                          FldType (%FldTypes;) #REQUIRED\n"
L"                          Retain (Yes|No) 'Yes'\n"
L"                          Limits CDATA #IMPLIED\n"
L"                          AlwaysWrite (Yes|No) 'No'\n"
L"                          OnConnect CDATA #IMPLIED>\n"

L"<!ELEMENT LightSwitch     (PLFmt, (%ValueMaps;)?)>\n"
L"<!ATTLIST LightSwitch     Topic CDATA #REQUIRED\n"
L"                          BaseName CDATA #REQUIRED\n"
L"                          Access (W | RW) #REQUIRED\n"
L"                          AltInTopic CDATA #IMPLIED\n"
L"                          Retain (Yes|No) 'Yes'\n"
L"                          OnConnect CDATA #IMPLIED>\n"

L"<!ELEMENT Dimmer          (PLFmt, (%ValueMaps;)?)>\n"
L"<!ATTLIST Dimmer          Topic CDATA #REQUIRED\n"
L"                          BaseName CDATA #REQUIRED\n"
L"                          Access (W | RW) #REQUIRED\n"
L"                          AltInTopic CDATA #IMPLIED\n"
L"                          Retain (Yes|No) 'Yes'\n"
L"                          MinVal CDATA #REQUIRED\n"
L"                          MaxVal CDATA #REQUIRED\n"
L"                          OnConnect CDATA #IMPLIED>\n"

L"<!ELEMENT MotionSensor    (PLFmt, (%ValueMaps;)?)>\n"
L"<!ATTLIST MotionSensor    Topic CDATA #REQUIRED\n"
L"                          BaseName CDATA #REQUIRED\n"
L"                          AltInTopic CDATA #IMPLIED\n"
L"                          OnConnect CDATA #IMPLIED>\n"

L"<!ELEMENT Relay           (PLFmt, (%ValueMaps;)?)>\n"
L"<!ATTLIST Relay           Topic CDATA #REQUIRED\n"
L"                          BaseName CDATA #REQUIRED\n"
L"                          Access (W | RW) #REQUIRED\n"
L"                          AltInTopic CDATA #IMPLIED\n"
L"                          Retain (Yes|No) 'Yes'\n"
L"                          OnConnect CDATA #IMPLIED>\n"

L"<!ELEMENT TempSensor      (PLFmt, (%ValueMaps;)?)>\n"
L"<!ATTLIST TempSensor      Topic CDATA #REQUIRED\n"
L"                          BaseName CDATA #REQUIRED\n"
L"                          AltInTopic CDATA #IMPLIED\n"
L"                          FldType (Int | Float) #REQUIRED\n"
L"                          MinVal CDATA #REQUIRED\n"
L"                          MaxVal CDATA #REQUIRED\n"
L"                          OnConnect CDATA #IMPLIED>\n"


// The device classes we currently support
L"<!ELEMENT PwrOut          (PLFmt)>\n"
L"<!ATTLIST PwrOut          Topic CDATA #REQUIRED\n"
L"                          TrueVal CDATA #REQUIRED\n"
L"                          FalseVal CDATA #REQUIRED\n"
L"                          OnConnect CDATA #IMPLIED>\n"
L"<!ELEMENT PwrIn           (PLFmt, EnumMap)>\n"
L"<!ATTLIST PwrIn           Topic CDATA #REQUIRED>\n"
L"<!ELEMENT Power           (PwrOut, PwrIn)>\n"
L"<!ATTLIST Power           BaseName CDATA #REQUIRED>\n"


// The overall configuration
L"<!ELEMENT Fields      (BoolSwitch | ContactClosure | Dimmer| Generic | LightSwitch "
L"                      | MotionSensor | Relay | TempSensor)+>\n"
L"<!ELEMENT DevClasses  (Power)+>\n"
L"<!ELEMENT MQTTCfg (Fields?, DevClasses?)>\n"
L"<!ATTLIST MQTTCfg Version (%Versions;) #REQUIRED\n"
L"                  UserName CDATA #IMPLIED\n"
L"                  Password CDATA #IMPLIED\n"
L"                  MQTTAddr CDATA #REQUIRED\n"
L"                  MQTTPort CDATA #REQUIRED\n"
L"                  MsgTrace (No|Yes) 'No'\n"
L"                  VerboseMode (No|Yes) 'No'\n"
L"                  Secure (No|Yes) 'No'\n"
L"                  SecPrincipal CDATA #IMPLIED\n"
L"                  CertInfo CDATA #IMPLIED\n"
L"                  PrefProtocol CDATA #IMPLIED>\n";




// ---------------------------------------------------------------------------
//   CLASS: TMQTTFldCfg
//  PREFIX: mqfldc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTFldCfg: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTFldCfg::TMQTTFldCfg() :

    m_bAltInTopic(kCIDLib::False)
    , m_bAlwaysWrite(kCIDLib::False)
    , m_bOnConnect(kCIDLib::False)
    , m_bRejected(kCIDLib::True)
    , m_bV2Compat(kCIDLib::False)
    , m_eAccess(tCQCKit::EFldAccess::None)
    , m_eActualQOS(tCQCMQTT::EQOSLevs::Q0)
    , m_eSemType(tCQCKit::EFldSTypes::Count)
    , m_eType(tCQCKit::EFldTypes::Count)
    , m_f8MaxValue(0)
    , m_f8MinValue(0)
    , m_mbufFmt(1024UL)
{
}

TMQTTFldCfg::~TMQTTFldCfg()
{
    try
    {
        m_cptrMap.DropRef();
        m_cptrAltInTopic.DropRef();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TMQTTFldCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tMQTTSh::EInMapRes
TMQTTFldCfg::eMapInVal( const   TMemBuf&        mbufVal
                        , const tCIDLib::TCard4 c4Bytes
                        ,       TString&        strFldVal) const
{
    TString strInVal;
    try
    {
        // We have to extract the value based on payload format
        switch (m_mqplfData.m_eType)
        {
            case  tMQTTSh::EPLTypes::Card :
            {
                tCIDLib::TCard4 c4Val;
                if (!bExtractCard(mbufVal, c4Bytes, c4Val))
                    return tMQTTSh::EInMapRes::BadVal;
                strInVal.AppendFormatted(c4Val);
                break;
            }

            case  tMQTTSh::EPLTypes::Int :
            {
                tCIDLib::TInt4 i4Val;
                if (!bExtractInt(mbufVal, c4Bytes, i4Val))
                    return tMQTTSh::EInMapRes::BadVal;
                strInVal.AppendFormatted(i4Val);
                break;
            }

            case  tMQTTSh::EPLTypes::BinText :
            case  tMQTTSh::EPLTypes::Text :
            {
                if (m_mqplfData.m_eType == tMQTTSh::EPLTypes::Text)
                {
                    TMQTTInMsg::ParseMQString(strInVal, mbufVal, c4Bytes);
                }
                 else
                {
                    // Try to transcode the binary text
                    if (m_mqplfData.m_c4Offset + m_mqplfData.m_c4Bytes > c4Bytes)
                    {
                        // Can't be right
                        if (facMQTTSh().bTraceMode())
                        {
                            facMQTTSh().LogTraceErr
                            (
                                tMQTTSh::EMsgSrcs::IOThread
                                , L"The payload offset+bytes goes beyond the available incoming data"
                            );
                        }
                        return tMQTTSh::EInMapRes::BadVal;
                    }

                    //
                    //  If the configured byte count is non-zero, we take it as is. Else
                    //  we go to the end of the buffer from the configured offset.
                    //
                    tCIDLib::TCard4 c4TextBytes = c4Bytes;
                    if (m_mqplfData.m_c4Bytes)
                        c4TextBytes = m_mqplfData.m_c4Bytes;

                    // Get a pointer to the buffer at the configured offset
                    const tCIDLib::TCard1* pc1Text = mbufVal.pc1DataAt(m_mqplfData.m_c4Offset);

                    // And finally we can try to trancode it, which could throw
                    try
                    {
                        m_mqplfData.m_cptrEncode->c4ConvertFrom(pc1Text, c4TextBytes, strInVal);
                    }

                    catch(TError& errToCatch)
                    {
                        if (facMQTTSh().eVerbosity() > tCQCKit::EVerboseLvls::Low)
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }

                        if (facMQTTSh().bTraceMode())
                        {
                            facMQTTSh().LogTraceErr
                            (
                                tMQTTSh::EMsgSrcs::IOThread
                                , L"Could not transcode the incoming binary text. Encoding=%(1)"
                                , m_mqplfData.m_cptrEncode->strEncodingName()
                            );
                        }
                    }
                }
                break;
            }

            default :
                return tMQTTSh::EInMapRes::BadVal;
        };
    }

    catch(TError& errToCatch)
    {
        if (facMQTTSh().eVerbosity() > tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceMsg
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"Failed to map incoming value for topic '%(1)'"
                , strTopicPath()
            );
        }
        return tMQTTSh::EInMapRes::BadVal;
    }

    //
    //  Do any pre-mapping which handles getting the MQTT value out of any larger
    //  content it is embedded in.
    //
    TString strMQTTVal;
    const tMQTTSh::EInMapRes eRes = m_cptrMap->ePreMapInVal(strInVal, strMQTTVal);
    if (eRes != tMQTTSh::EInMapRes::GoodVal)
        return eRes;

    //
    //  If there are precision digits, we need to adjust it for that, and then format
    //  back out to text. This will only be set for floating point fields.
    //
    if (m_mqplfData.m_c4Precision)
    {
        tCIDLib::TFloat8 f8Tmp;
        if (!strMQTTVal.bToFloat8(f8Tmp))
            return tMQTTSh::EInMapRes::BadVal;

        f8Tmp /= TMathLib::f8Power(10, m_mqplfData.m_c4Precision);
        strMQTTVal.SetFormatted(f8Tmp, m_mqplfData.m_c4Precision);
    }

    // And now let the map do the actual mapping of the MQTT value to a field value
    return m_cptrMap->eMapInVal(m_eType, m_eSemType, strMQTTVal, strFldVal, m_mqplfData);
}


//
//  Maps the passed field value to an outgoing value, then packages that outgoing value
//  into a memory buffer based on the payload format for this field.
//
tCIDLib::TBoolean
TMQTTFldCfg::bMapOutVal( const  TString&            strFldVal
                        ,       TMemBuf&            mbufPLData
                        ,       tCIDLib::TCard4&    c4PLBytes) const
{
    // First do the mapping of the field data to the basic MQTT value. If bad we give up
    TString strMQTTVal;
    if (!m_cptrMap->bMapOutVal(m_eType, m_eSemType, strFldVal, strMQTTVal, m_mqplfData))
        return kCIDLib::False;

    //
    //  If there's a precision value, then the MQTT value must be a floating point value
    //  and we need to multiply it up and format it back out without any decimal digits.
    //
    if (m_mqplfData.m_c4Precision)
    {
        tCIDLib::TFloat8 f8Tmp;
        if (!strMQTTVal.bToFloat8(f8Tmp))
        {
            return kCIDLib::False;
        }

        f8Tmp *= TMathLib::f8Power(10, m_mqplfData.m_c4Precision);
        strMQTTVal.SetFormatted(f8Tmp, 0);
    }

    // And now do post mapping in case it needs to be embedded in a larger value
    TString strOutVal;
    if (!m_cptrMap->bPostMapOutVal(strMQTTVal, strOutVal))
        return kCIDLib::False;

    //
    //  Now call the packaging method to package it up according to the payload format
    //  into the caller's buffer.
    //
    return bPackageOutVal(strOutVal, mbufPLData, c4PLBytes);
}


//
//  Checks see if this field matches in the incoming topic. We compare against our main
//  topic path (non-case sensitive) and if that doesn't match we see if we have an alt
//  input path reg ex and test that.
//
tCIDLib::TBoolean TMQTTFldCfg::bMatchesInTopic(const TString& strToCheck) const
{
    if (strToCheck.bCompareI(m_strTopicPath))
        return kCIDLib::True;

    if (m_cptrAltInTopic.pobjData() && m_cptrAltInTopic->bFullyMatches(strToCheck))
        return kCIDLib::True;

    return kCIDLib::False;
}


//
//  Packages the passed outgoing MQTT value into the caller's buffer, based on the
//  payload format settings for this field.
//
//  We assume the caller is providing us with a valid value (typically by calling
//  bMapOutVal() above, but sometimes he may pass us setup data to send or some such.)
//  It still though has to be convertable according to the payload format, i.e. if
//  it's said to be a binary numeric value it has to be convertable to that type of
//  value.
//
tCIDLib::TBoolean
TMQTTFldCfg::bPackageOutVal(const   TString&            strMQTTVal
                            ,       TMemBuf&            mbufTar
                            ,       tCIDLib::TCard4&    c4Bytes) const
{
    if (m_mqplfData.m_eType == tMQTTSh::EPLTypes::BinText)
    {
        // If it's BinText, we can just transcode directly into the caller's buffer
        try
        {
            m_mqplfData.m_cptrEncode->c4ConvertTo(strMQTTVal, mbufTar, c4Bytes);
        }

        catch(TError& errToCatch)
        {
            if (facMQTTSh().eVerbosity() > tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceErr
                (
                    tMQTTSh::EMsgSrcs::IOThread
                    , L"Could not transcode the outgoing binary text. Encoding=%(1). Fld=%(2)"
                    , m_mqplfData.m_cptrEncode->strEncodingName()
                    , m_strFldName
                );
            }
            return kCIDLib::False;
        }
    }
     else
    {
        //
        //  Else create an output stream over the caller's bufffer, and set the
        //  endianness defined for this topic payload.
        //
        TBinMBufOutStream strmTar(&mbufTar);
        if (m_mqplfData.m_bBigEndian)
            strmTar.eEndianMode(tCIDLib::EEndianModes::Big);

        if (m_mqplfData.m_eType == tMQTTSh::EPLTypes::Card)
        {
            tCIDLib::TCard4 c4Val;
            if (!strMQTTVal.bToCard4(c4Val, tCIDLib::ERadices::Auto))
                return kCIDLib::False;

            if (m_mqplfData.m_c4Bytes == 1)
            {
                if (c4Val > kCIDLib::c1MaxCard)
                    return kCIDLib::False;
                strmTar << tCIDLib::TCard1(c4Val);
            }
            else if (m_mqplfData.m_c4Bytes == 2)
            {
                if (c4Val > kCIDLib::c2MaxCard)
                    return kCIDLib::False;
                strmTar << tCIDLib::TCard2(c4Val);
            }
            else if (m_mqplfData.m_c4Bytes == 4)
            {
                strmTar << c4Val;
            }
        }
         else if (m_mqplfData.m_eType == tMQTTSh::EPLTypes::Int)
        {
            tCIDLib::TInt4 i4Val;
            if (!strMQTTVal.bToInt4(i4Val, tCIDLib::ERadices::Auto))
                return kCIDLib::False;

            if (m_mqplfData.m_c4Bytes == 1)
            {
                if (i4Val > kCIDLib::i1MaxInt)
                    return kCIDLib::False;
                strmTar << tCIDLib::TInt1(i4Val);
            }
            else if (m_mqplfData.m_c4Bytes == 2)
            {
                if (i4Val > kCIDLib::i2MaxInt)
                    return kCIDLib::False;
                strmTar << tCIDLib::TInt2(i4Val);
            }
            else if (m_mqplfData.m_c4Bytes == 4)
            {
                strmTar << i4Val;
            }
        }
         else if (m_mqplfData.m_eType == tMQTTSh::EPLTypes::Text)
        {
            //
            //  We have to transcode into a temp, then format out to the stream in
            //  standard MQTT text format.
            //
            tCIDLib::TCard4 c4TextBytes = TMQTTOutMsg::c4EncodeMQTTString(strMQTTVal, m_mbufFmt);
            strmTar << tCIDLib::TCard2(c4TextBytes);
            strmTar.c4WriteBuffer(m_mbufFmt, c4TextBytes);
        }
         else
        {
            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceErr
                (
                    tMQTTSh::EMsgSrcs::IOThread
                    , L"Unknown outgoing payload format for field %(1)"
                    , m_strFldName
                );
            }
            return kCIDLib::False;
        }

        strmTar.Flush();
        c4Bytes = strmTar.c4CurSize();
    }

    return kCIDLib::True;
}


//
//  Called by the I/O thread after getting a subscription ack. This is all just runtime stuff
//  it doesn't affect the actual field config.
//
tCIDLib::TVoid TMQTTFldCfg::InitializeFld(const tCIDLib::TCard1 c1SubAckCode)
{
    if (c1SubAckCode == 0x80)
    {
        // It failed so mark it rejected
        m_bRejected = kCIDLib::True;
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread, L"Topic '%(1)' was rejected", strTopicPath()
            );
        }
    }
     else
    {
        // It should be the actual QOS allowed
        tCQCMQTT::EQOSLevs eQOS = tCQCMQTT::EQOSLevs(c1SubAckCode);
        if (tCQCMQTT::bIsValidEnum(eQOS))
        {
            // It's good so store QOS and clear rejected flag
            m_eActualQOS = eQOS;
            m_bRejected = kCIDLib::False;
        }
         else
        {
            // Mark it rejected since we don't know what to do with it
            m_bRejected = kCIDLib::True;
            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceErr
                (
                    tMQTTSh::EMsgSrcs::IOThread, L"Topic '%(1)' got bad code", strTopicPath()
                );
            }
        }
    }
}


//
//  Parse our content out of the passed XML node, which is one of the semantic
//  type nodes under the <Fields> tag.
//
tCIDLib::TVoid
TMQTTFldCfg::ParseFrom( const   TXMLTreeElement&        xtnodeSrc
                        ,       tCIDLib::TStrCollect&   colErrs)
{
    // Topic and base name are required for all of them
    m_strBaseName = xtnodeSrc.xtattrNamed(L"BaseName").strValue();
    m_strTopicPath = xtnodeSrc.xtattrNamed(L"Topic").strValue();

    // Get the element name, which should be mappable to our semantic field type
    m_eSemType = tCQCKit::eAltXlatEFldSTypes(xtnodeSrc.strQName());

    // Some need access and some are prefab
    switch(m_eSemType)
    {
        case tCQCKit::EFldSTypes::BoolSwitch :
        case tCQCKit::EFldSTypes::Dimmer :
        case tCQCKit::EFldSTypes::Generic :
        case tCQCKit::EFldSTypes::LightSwitch :
        case tCQCKit::EFldSTypes::Relay :
            m_eAccess = tCQCKit::eAltXlatEFldAccess
            (
                xtnodeSrc.xtattrNamed(L"Access").strValue()
            );
            break;

        case tCQCKit::EFldSTypes::ContactClosure :
        case tCQCKit::EFldSTypes::MotionSensor :
        case tCQCKit::EFldSTypes::TempSensor :
            m_eAccess = tCQCKit::EFldAccess::Read;
            break;

        default :
            TMQTTCfg::AddBadValErr(colErrs, m_strBaseName, xtnodeSrc.strQName(), L"field access");
            break;
    };

    // And do the field type
    switch(m_eSemType)
    {
        case tCQCKit::EFldSTypes::BoolSwitch :
        case tCQCKit::EFldSTypes::ContactClosure :
        case tCQCKit::EFldSTypes::LightSwitch :
        case tCQCKit::EFldSTypes::MotionSensor :
        case tCQCKit::EFldSTypes::Relay :
            m_eType = tCQCKit::EFldTypes::Boolean;
            break;

        case tCQCKit::EFldSTypes::Dimmer :
            m_eType = tCQCKit::EFldTypes::Card;
            break;

        case tCQCKit::EFldSTypes::Generic :
        case tCQCKit::EFldSTypes::TempSensor :
            // These are set explicitly
            m_eType = tCQCKit::eAltXlatEFldTypes(xtnodeSrc.xtattrNamed(L"FldType").strValue());
            if (m_eType == tCQCKit::EFldTypes::Count)
                TMQTTCfg::AddBadValErr(colErrs, m_strBaseName, xtnodeSrc.strQName(), L"field type");
            break;

        default :
            TMQTTCfg::AddBadValErr(colErrs, m_strBaseName, xtnodeSrc.strQName(), L"field type");
            break;
    };

    //
    //  And the limits. If it's one of the types that has a limit, then get the
    //  values out first.
    //
    if ((m_eSemType == tCQCKit::EFldSTypes::Dimmer)
    ||  (m_eSemType == tCQCKit::EFldSTypes::TempSensor))
    {
        if (!xtnodeSrc.xtattrNamed(L"MinVal").strValue().bToFloat8(m_f8MinValue)
        ||  !xtnodeSrc.xtattrNamed(L"MaxVal").strValue().bToFloat8(m_f8MaxValue)
        ||  (m_f8MaxValue <= m_f8MinValue))
        {
            TString strRange = xtnodeSrc.xtattrNamed(L"MinVal").strValue()
                               + TString(L"/")
                               + xtnodeSrc.xtattrNamed(L"MaxVal").strValue();
            TMQTTCfg::AddBadValErr(colErrs, m_strBaseName, strRange, L"range limit");
        }
    }

    switch(m_eSemType)
    {
        case tCQCKit::EFldSTypes::BoolSwitch :
        case tCQCKit::EFldSTypes::ContactClosure :
        case tCQCKit::EFldSTypes::LightSwitch :
        case tCQCKit::EFldSTypes::MotionSensor :
        case tCQCKit::EFldSTypes::Relay :
            m_strLimits.Clear();
            break;

        case tCQCKit::EFldSTypes::Generic :
            // This one they provide the limits for directly
            if (!xtnodeSrc.bAttrExists(L"Limits", m_strLimits))
                m_strLimits.Clear();
            break;

        case tCQCKit::EFldSTypes::Dimmer :
            m_strLimits = L"Range: ";
            m_strLimits.AppendFormatted(tCIDLib::TCard4(m_f8MinValue));
            m_strLimits.Append(kCIDLib::chComma);
            m_strLimits.AppendFormatted(tCIDLib::TCard4(m_f8MaxValue));
            break;

        case tCQCKit::EFldSTypes::TempSensor :
            m_strLimits = L"Range: ";
            if (m_eType == tCQCKit::EFldTypes::Int)
            {
                m_strLimits.AppendFormatted(tCIDLib::TInt4(m_f8MinValue));
                m_strLimits.Append(kCIDLib::chComma);
                m_strLimits.AppendFormatted(tCIDLib::TInt4(m_f8MaxValue));
            }
             else
            {
                m_strLimits.AppendFormatted(m_f8MinValue);
                m_strLimits.Append(kCIDLib::chComma);
                m_strLimits.AppendFormatted(m_f8MaxValue);
            }
            break;

        default :
            TMQTTCfg::AddBadValErr(colErrs, m_strBaseName, xtnodeSrc.strQName(), L"limits");
            break;
    };

    // And set up the field name
    switch(m_eSemType)
    {
        case tCQCKit::EFldSTypes::BoolSwitch :
        {
            m_strFldName = L"Sw_";
            m_strFldName += m_strBaseName;
            break;
        }

        case tCQCKit::EFldSTypes::ContactClosure :
        {
            m_strFldName = L"Contact_";
            m_strFldName += m_strBaseName;
            break;
        }

        case tCQCKit::EFldSTypes::Dimmer :
        {
            if ((m_eAccess == tCQCKit::EFldAccess::ReadWrite)
            &&  (m_f8MinValue == 0) && (m_f8MaxValue == 100))
            {
                // It's V2 compatible
                m_strFldName = L"LGHT#Dim_";
                m_strFldName += m_strBaseName;
                m_bV2Compat = kCIDLib::True;
            }
             else
            {
                m_strFldName = L"Dim_";
                m_strFldName += m_strBaseName;
            }
            break;
        }

        case tCQCKit::EFldSTypes::Generic :
        {
            // They provide the literal name in this case
            m_strFldName = m_strBaseName;
            break;
        }

        case tCQCKit::EFldSTypes::LightSwitch :
        {
            if (m_eAccess == tCQCKit::EFldAccess::ReadWrite)
            {
                m_strFldName = L"LGHT#Sw_";
                m_strFldName += m_strBaseName;
                m_bV2Compat = kCIDLib::True;
            }
             else
            {
                m_strFldName = L"Sw_";
                m_strFldName += m_strBaseName;
            }
            break;
        }

        case tCQCKit::EFldSTypes::MotionSensor :
        {
            m_strFldName = L"MOT#";
            m_strFldName += m_strBaseName;
            m_bV2Compat = kCIDLib::True;
            break;
        }

        case tCQCKit::EFldSTypes::Relay :
        {
            m_strFldName = L"Relay_";
            m_strFldName += m_strBaseName;
            break;
        }

        case tCQCKit::EFldSTypes::TempSensor :
        {
            m_strFldName = L"Temp_";
            m_strFldName += m_strBaseName;
            break;
        }

        default :
            TMQTTCfg::AddBadValErr(colErrs, m_strBaseName, xtnodeSrc.strQName(), L"semantic type");
            break;
    };

    // And we have to have a payload format child first
    const TXMLTreeElement& xtnodePLFmt = xtnodeSrc.xtnodeChildAtAsElement(0);
    m_mqplfData.ParseFrom
    (
        m_strFldName, m_eSemType, m_eType, m_eAccess, xtnodePLFmt, colErrs
    );

    //
    //  And we may have a map. If so we create one based on the type and let it parse
    //  itself. Else we look at the info and decide on a default one to create. Else
    //  we throw.
    //
    m_cptrMap.DropRef();
    if (xtnodeSrc.c4ChildCount() > 1)
    {
        const TXMLTreeElement& xtnodeMap  = xtnodeSrc.xtnodeChildAtAsElement(1);

        // the facility class has a helper to gen up a map by name
        TMQTTValMap* pmqvmpNew = facMQTTSh().pmqvmpMakeNew(xtnodeMap.strQName());
        if (pmqvmpNew)
        {
            m_cptrMap.SetPointer(pmqvmpNew);
            if (m_cptrMap->bParseFrom(m_strFldName, m_eAccess, xtnodeMap, colErrs))
            {
                // Ask the map if it is valid for the semantic and field types
                if (!m_cptrMap->bIsValidFor(m_eType, m_eSemType, m_mqplfData))
                {
                    colErrs.objAdd
                    (
                        TString(kMQTTShErrs::errcCfg_MapMismatch, facMQTTSh(), m_strFldName)
                    );
                }
             }
        }
         else
        {
            TMQTTCfg::AddBadValErr
            (
                colErrs, m_strBaseName, xtnodeMap.strQName(), L"value map type"
            );
        }
    }
     else
    {
        // Just add a default passthrough map
        m_cptrMap.SetPointer(new TMQTTPassthroughMap());
    }

    //
    //  If it's writeable they can have a retain flag. It is defaulted to Yes, so we can
    //  just store it. If it's not writeable, it'll never be used.
    //
    if (tCIDLib::bAllBitsOn(m_eAccess , tCQCKit::EFldAccess::Write))
        m_bRetainWrite = xtnodeSrc.xtattrNamed(L"Retain").bValueAs();
    else
        m_bRetainWrite = kCIDLib::False;

    // Check the always write flag if it's a generic
    if (m_eSemType == tCQCKit::EFldSTypes::Generic)
    {
        m_bAlwaysWrite = xtnodeSrc.xtattrNamed(L"AlwaysWrite").bValueAs();
        if (m_bAlwaysWrite && !tCIDLib::bAllBitsOn(m_eAccess, tCQCKit::EFldAccess::Write))
        {
            TMQTTCfg::AddError
            (
                colErrs, m_strBaseName, L"AlwaysWrite is only used for writable fields"
            );
        }
    }

    // If there's an OnConnect value, get that
    m_bOnConnect = xtnodeSrc.bAttrExists(L"OnConnect", m_strOnConnect);
    if (!m_bOnConnect)
        m_strOnConnect.Clear();

    // If there's an alt input topic process that
    m_cptrAltInTopic.DropRef();
    TString strAltTopic;
    if (xtnodeSrc.bAttrExists(L"AltInTopic", strAltTopic))
    {
        // If this field is not readable, this is meaningless
        if (!tCIDLib::bAllBitsOn(m_eAccess , tCQCKit::EFldAccess::Read))
        {
            TMQTTCfg::AddError
            (
                colErrs, m_strBaseName, L"AltTopicPath is only used for readable fields"
            );
        }
         else
        {
            try
            {
                m_cptrAltInTopic.SetPointer(new TRegEx(strAltTopic));
            }

            catch(TError& errToCatch)
            {
                TMQTTCfg::AddError
                (
                    colErrs, m_strBaseName, L"AltTopicPath must be a valid regular expression"
                );
                TMQTTCfg::AddError(colErrs, m_strBaseName, errToCatch.strErrText());
            }
        }
    }

    // Reset any runtime only stuff
    m_bRejected     = kCIDLib::True;
    m_eActualQOS    = tCQCMQTT::EQOSLevs::Q0;
}


// We return the field definition for our data
tCIDLib::TVoid TMQTTFldCfg::QueryFldDef(TCQCFldDef& flddToFill) const
{
    // All the info is there, so just load it up
    flddToFill.Set(m_strFldName, m_eType, m_eAccess, m_eSemType, m_strLimits);
    if (m_bAlwaysWrite)
        flddToFill.bAlwaysWrite(m_bAlwaysWrite);
}


tCIDLib::TVoid
TMQTTFldCfg::Set(const  tCQCKit::EFldAccess     eAccess
                , const tCQCKit::EFldSTypes     eSemType
                , const tCQCKit::EFldTypes      eType
                , const TString&                strBaseName
                , const TString&                strFldName
                , const TString&                strLimits
                , const TString&                strTopicPath
                , const TMapPtr&                cptrMap
                , const TMQTTPLFmt&             mqplfData
                , const tCIDLib::TBoolean       bV2Compat)
{
    m_bAlwaysWrite = kCIDLib::False;
    m_bRejected = kCIDLib::False;
    m_bV2Compat = bV2Compat;
    m_eAccess = eAccess;
    m_eActualQOS = tCQCMQTT::EQOSLevs::Q0;
    m_eSemType= eSemType;
    m_eType = eType;
    m_f8MaxValue = 0;
    m_f8MinValue = 0;

    m_strBaseName = strBaseName;
    m_strFldName = strFldName;
    m_strLimits = strLimits;
    m_strTopicPath = strTopicPath;

    m_cptrMap = cptrMap;
    m_mqplfData = mqplfData;
}


// ---------------------------------------------------------------------------
//  TMQTTFldCfg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Given a received payload, we will try to extract a valid card or int value
//  based on our payload info. The caller should have already checked that this
//  is a binary payload type field (based on user config.)
//
tCIDLib::TBoolean
TMQTTFldCfg::bExtractCard(  const   TMemBuf&            mbufPL
                            , const tCIDLib::TCard4     c4PLBytes
                            ,       tCIDLib::TCard4&    c4NewVal) const
{
    //
    //  Make sure we have enough bytes. The offset plus the configured size of
    //  the payload data cannot be more than the payload bytes we got.
    //
    if (m_mqplfData.m_c4Offset + m_mqplfData.m_c4Bytes > c4PLBytes)
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"The offset+bytes (%(1)+%(2)) is beyond the payload size %(3)"
                , TCardinal(m_mqplfData.m_c4Offset)
                , TCardinal(m_mqplfData.m_c4Bytes)
                , TCardinal(c4PLBytes)
            );
        }
        return kCIDLib::False;
    }

    // Pull the value out and byte swap if needed
    const tCIDLib::TCard4 c4At = m_mqplfData.m_c4Offset;
    if (m_mqplfData.m_c4Bytes == 1)
    {
        c4NewVal = mbufPL.c1At(c4At);
    }
     else if (m_mqplfData.m_c4Bytes == 2)
    {
        tCIDLib::TCard2 c2Val = mbufPL.c2At(c4At);
        if (m_mqplfData.m_bBigEndian != TSysInfo::bBigEndian())
            c2Val = TRawBits::c2SwapBytes(c2Val);
        c4NewVal = c2Val;
    }
     else if (m_mqplfData.m_c4Bytes == 4)
    {
        c4NewVal = mbufPL.c4At(c4At);
        if (m_mqplfData.m_bBigEndian != TSysInfo::bBigEndian())
            c4NewVal = TRawBits::c4SwapBytes(c4NewVal);
    }
     else
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"%(1) is not a valid payload size for field %(2)"
                , TCardinal(c4PLBytes)
                , m_strFldName
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TMQTTFldCfg::bExtractInt(const  TMemBuf&        mbufPL
                        , const tCIDLib::TCard4 c4PLBytes
                        ,       tCIDLib::TInt4& i4NewVal) const
{
    //
    //  Make sure we have enough bytes. The offset plus the configured size of
    //  the payload data cannot be more than the payload bytes we got.
    //
    if (m_mqplfData.m_c4Offset + m_mqplfData.m_c4Bytes > c4PLBytes)
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"The offset+bytes (%(1)+%(2)) is beyond the payload size %(3)"
                , TCardinal(m_mqplfData.m_c4Offset)
                , TCardinal(m_mqplfData.m_c4Bytes)
                , TCardinal(c4PLBytes)
            );
        }
        return kCIDLib::False;
    }

    // Pull the value out and byte swap if needed
    if (m_mqplfData.m_c4Bytes == 1)
    {
        i4NewVal = mbufPL.i1At(m_mqplfData.m_c4Offset);
    }
     else if (m_mqplfData.m_c4Bytes == 2)
    {
        tCIDLib::TInt2 i2Val = mbufPL.i2At(m_mqplfData.m_c4Offset);
        if (m_mqplfData.m_bBigEndian != TSysInfo::bBigEndian())
            i2Val = TRawBits::i2SwapBytes(i2Val);
        i4NewVal = i2Val;
    }
     else if (m_mqplfData.m_c4Bytes == 4)
    {
        i4NewVal = mbufPL.i4At(m_mqplfData.m_c4Offset);
        if (m_mqplfData.m_bBigEndian != TSysInfo::bBigEndian())
            i4NewVal = TRawBits::i4SwapBytes(i4NewVal);
    }
     else
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"%(1) is not a valid payload size for field %(2)"
                , TCardinal(c4PLBytes)
                , m_strFldName
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TMQTTCfg
//  PREFIX: mqcfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTCfg: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TMQTTCfg::AddBadValErr(         tCIDLib::TStrCollect&   colErrs
                        , const TString&                strFldName
                        , const TString&                strValue
                        , const TString&                strValDescr)
{
    colErrs.objAdd
    (
        TString
        (
            kMQTTShErrs::errcCfg_BadValue
            , facMQTTSh()
            , strValue
            , strValDescr
            , strFldName
        )
    );
}

tCIDLib::TVoid
TMQTTCfg::AddError(         tCIDLib::TStrCollect&   colErrs
                    , const TString&                strFldName
                    , const TString&                strErrText)
{
    colErrs.objAdd
    (
        TString
        (
            kMQTTShErrs::errcCfg_FldCfgError
            , facMQTTSh()
            , strFldName
            , strErrText
        )
    );
}


tCIDLib::TVoid
TMQTTCfg::AddRequiredValErr(         tCIDLib::TStrCollect&   colErrs
                            , const TString&                strFldName
                            , const TString&                strValDescr)
{
    colErrs.objAdd
    (
        TString(kMQTTShErrs::errcCfg_RequiredVal, facMQTTSh(), strFldName, strValDescr)
    );
}

tCIDLib::TVoid
TMQTTCfg::AddNotUsedValErr(         tCIDLib::TStrCollect&   colErrs
                            , const TString&                strFldName
                            , const TString&                strValDescr)
{
    colErrs.objAdd
    (
        TString(kMQTTShErrs::errcCfg_NotUsedVal, facMQTTSh(), strFldName, strValDescr)
    );
}


// ---------------------------------------------------------------------------
//  TMQTTCfg: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTCfg::TMQTTCfg() :

    m_bMsgTrace(kCIDLib::False)
    , m_bSecure(kCIDLib::False)
    , m_bVerboseMode(kCIDLib::False)
    , m_c4Version(0)
    , m_colFldList
      (
          73
          , TStringKeyOps(kCIDLib::False)
          , [](const TMQTTFldCfg& mqfldcSrc) -> const TString& { return mqfldcSrc.strFldName(); }
      )
    , m_ippnMQTT(1883)
{
}

TMQTTCfg::~TMQTTCfg()
{
}


// ---------------------------------------------------------------------------
//  TMQTTCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------
//
//  These guys are stored via XML, so we need to parse it out. We get the main
//  entity source reference, so we need to create an XML parser and parse the
//  contents. If that doesn't throw, then we get the root element and start
//  pulling out data out.
//
//  We have another convenience method that takes a path, sets up a file entity
//  source for it, and passes it to the other one.
//
tCIDLib::TBoolean
TMQTTCfg::bParseFrom(const  TString&        strPath
                    , const TString&        strMoniker
                    , tCIDLib::TStrCollect& colErrs)
{
    tCIDXML::TEntitySrcRef esrSrc(new TFileEntitySrc(strPath));
    return bParseFrom(esrSrc, strMoniker, colErrs);
}

tCIDLib::TBoolean
TMQTTCfg::bParseFrom(       tCIDXML::TEntitySrcRef&     esrSrc
                    , const TString&                    strMoniker
                    ,       tCIDLib::TStrCollect&       colErrs)
{
    colErrs.RemoveAll();
    try
    {
        TXMLTreeParser xtprsConfig;

        //
        //  Add an entity mapping so that the public URN that is used in the
        //  manifest files get mapped to our hard coded internal DTD text.
        //
        xtprsConfig.AddMapping
        (
            new TMemBufEntitySrc
            (
                L"CQCMQTTCfg.DTD"
                , L"urn:charmedquark.com:CQC-MQTTCfg.DTD"
                , strEmbeddedCfgDTD
            )
        );

        //
        //  Ok, we have a file, so lets try to use the XML tree parser to get
        //  a representation of the file into memory. Tell it that we only
        //  want to see non-ignorable chars and tag information. That will
        //  save us a lot of filtering out comments and whitspace.
        //
        const tCIDLib::TBoolean bOk = xtprsConfig.bParseRootEntity
        (
            esrSrc, tCIDXML::EParseOpts::Validate, tCIDXML::EParseFlags::TagsNText
        );

        if (!bOk)
        {
            //
            //  Format the first error in the error list, into a string that
            //  we can pass as a replacement parameter to the real error we
            //  log.
            //
            const TXMLTreeParser::TErrInfo& erriFirst = xtprsConfig.colErrors()[0];

            TPathStr pathCfg(erriFirst.strSystemId());
            pathCfg.bRemovePath();
            TTextStringOutStream strmOut(512);
            strmOut << L"[" << pathCfg << L"/"
                    << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                    << L"] " << erriFirst.strText() << kCIDLib::FlushIt;
            colErrs.objAdd(strmOut.strData());
            return kCIDLib::False;
        }

        // Get the root out and let's start pulling out data out
        const TXMLTreeElement& xtnodeRoot = xtprsConfig.xtdocThis().xtnodeRoot();
        ParseFrom(xtnodeRoot, strMoniker, colErrs);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        colErrs.objAdd(TStrCat(L"Exception occurred: ", errToCatch.strErrText()));
        return kCIDLib::False;
    }

    // If no errors reported, then we are good
    return colErrs.bIsEmpty();
}


// Return a const cursor to our main field list
TMQTTCfg::TFldCursor TMQTTCfg::cursFldList() const
{
    return TFldCursor(&m_colFldList);
}


//
//  Find all of the fields that reference the passed topic and initialize them with
//  the passed code that was returned from the subscription request for this topic.
//
tCIDLib::TVoid
TMQTTCfg::InitFldsForTopic(const TString& strTopic, const tCIDLib::TCard1 c1Code)
{
    m_colFldList.bForEachNC
    (
        [&strTopic, c1Code](TMQTTFldCfg& mqfldcCur)
        {
            if (strTopic.bCompareI(mqfldcCur.strTopicPath()))
                mqfldcCur.InitializeFld(c1Code);
            return kCIDLib::True;
        }
    );
}


//
//  Find the field config for the passed field name. We use the main list which is
//  keyed on the field name.
//
const TMQTTFldCfg* TMQTTCfg::pmqfldcFindFld(const TString& strToFind) const
{
    return m_colFldList.pobjFindByKey(strToFind);
}


// ---------------------------------------------------------------------------
//  TMQTTCfg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TMQTTCfg::ParseFrom(const   TXMLTreeElement&        xtnodeSrc
                    , const TString&                strMoniker
                    ,       tCIDLib::TStrCollect&   colErrs)
{
    //
    //  They can force the initial trace and verbosity states in the config. They are
    //  both defaulted in the DTD so we an always get these. Do them first and update
    //  the shared facility so that we can get as much as possible traced and/or logged
    //  if they are enabled.
    //
    m_bMsgTrace = xtnodeSrc.xtattrNamed(L"MsgTrace").bValueAs();
    m_bVerboseMode = xtnodeSrc.xtattrNamed(L"VerboseMode").bValueAs();

    facMQTTSh().bTraceMode(m_bMsgTrace);
    if (m_bVerboseMode)
        facMQTTSh().eVerbosity(tCQCKit::EVerboseLvls::High);

    //
    //  Get out the general info, which is done as attributes on our node. First
    //  get the version and validate it.
    //
    const TString& strVer = xtnodeSrc.xtattrNamed(L"Version").strValue();
    tCIDLib::TCard4 c4Version;
    if (!strVer.bToCard4(c4Version) || !c4Version || (c4Version > kMQTTSh::c4CurCfgVer))
        colErrs.objAdd(TString(kMQTTShErrs::errcCfg_BadVersion, facMQTTSh(), strVer));

    // These are optional
    if (!xtnodeSrc.bAttrExists(L"UserName", m_strUserName))
        m_strUserName.Clear();
    if (!xtnodeSrc.bAttrExists(L"Password", m_strPassword))
        m_strPassword.Clear();

    // These are required
    m_strMQTTAddr = xtnodeSrc.xtattrNamed(L"MQTTAddr").strValue();
    m_ippnMQTT = xtnodeSrc.xtattrNamed(L"MQTTPort").c4ValueAs();

    // We need to clean the list of any previous parse attempt
    m_colFldList.RemoveAll();

    // If there is a fields block, then let's parse that
    tCIDLib::TCard4 c4At;
    TMQTTFldCfg mqfldcNew;
    {
        const TXMLTreeElement* pxtnodeFields = xtnodeSrc.pxtnodeFindElement(L"Fields", 0, c4At);
        if (pxtnodeFields)
        {
            const tCIDLib::TCard4 c4FldCnt = pxtnodeFields->c4ChildCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FldCnt; c4Index++)
            {
                const TXMLTreeElement& xtnodeFld(pxtnodeFields->xtnodeChildAtAsElement(c4Index));
                mqfldcNew.ParseFrom(xtnodeFld, colErrs);
                m_colFldList.objAdd(mqfldcNew);
            }
        }
    }

    // And if any device classes, deal with this
    {
        const TXMLTreeElement* pxtnodeCls = xtnodeSrc.pxtnodeFindElement(L"DevClasses", 0, c4At);
        if (pxtnodeCls)
        {
            const tCIDLib::TCard4 c4ClsCnt = pxtnodeCls->c4ChildCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ClsCnt; c4Index++)
            {
                const TXMLTreeElement& xtnodeCls(pxtnodeCls->xtnodeChildAtAsElement(c4Index));
                if (xtnodeCls.strQName() == L"Power")
                    ParseDevClsPower(strMoniker, xtnodeCls, mqfldcNew, colErrs);
            }
        }
    }

    //
    //  Check the secure connection stuff. The secure flag defaults to No, so we
    //  can always check it. If secure, check for a principal. If not set then we
    //  use the address.
    //
    m_bSecure = xtnodeSrc.xtattrNamed(L"Secure").bValueAs();
    if (m_bSecure)
    {
        if (!xtnodeSrc.bAttrExists(L"SecPrincipal", m_strSecPrincipal))
            m_strSecPrincipal = m_strMQTTAddr;
    }

    // The preferrred protocol, which is optional
    if (!xtnodeSrc.bAttrExists(L"PrefProcol", m_strPrefProtocol))
        m_strPrefProtocol.Clear();

    // And the client side cert info, which is optional
    if (!xtnodeSrc.bAttrExists(L"CertInfo", m_strCertInfo))
        m_strCertInfo.Clear();
}


//
//  The device classes are pretty messy, so we break them out.
//
tCIDLib::TVoid
TMQTTCfg::ParseDevClsPower( const   TString&                strMoniker
                            , const TXMLTreeElement&        xtnodeCls
                            ,       TMQTTFldCfg&            mqfldcTmp
                            ,       tCIDLib::TStrCollect&   colErrs)
{
    const TString& strBaseName = xtnodeCls.xtattrNamed(L"BaseName").strValue();
    TString strFldName;
    TMQTTPLFmt mqplfData;
    TMQTTFldCfg::TMapPtr cptrMap;

    // First is the output field info
    {
        const TXMLTreeElement& xtnodeOut = xtnodeCls.xtnodeChildAtAsElement(0);
        const TString& strTopic = xtnodeOut.xtattrNamed(L"Topic").strValue();
        const TString& strFalse  = xtnodeOut.xtattrNamed(L"FalseVal").strValue();
        const TString& strTrue = xtnodeOut.xtattrNamed(L"TrueVal").strValue();

        strFldName = L"PWR#";
        strFldName.Append(strBaseName);
        strFldName.Append(kCIDLib::chTilde);
        strFldName.Append(L"Power");

        // Parse out a payload format
        const TXMLTreeElement& xtnodePLFmt = xtnodeOut.xtnodeChildAtAsElement(0);
        mqplfData.ParseFrom
        (
            strFldName
            , tCQCKit::EFldSTypes::Power
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::Write
            , xtnodePLFmt
            , colErrs
        );

        // Create a map on the fly based on the true/false values and payload type
        if (mqplfData.bIsNumericPL())
        {
            tCIDLib::TInt8 i8False;
            tCIDLib::TInt8 i8True;

            if (!strFalse.bToInt8(i8False))
                AddBadValErr(colErrs, strFldName, strFalse, L"false out value");
            if (!strTrue.bToInt8(i8True))
                AddBadValErr(colErrs, strFldName, strTopic, L"true out value");

            TMQTTBoolNumMap* pmqvmapOut = new TMQTTBoolNumMap();
            cptrMap.SetPointer(pmqvmapOut);
            pmqvmapOut->Reset(i8False, i8True);
        }
         else
        {
            TMQTTBoolTextMap* pmqvmapOut = new TMQTTBoolTextMap();
            cptrMap.SetPointer(pmqvmapOut);
            pmqvmapOut->Reset(strFalse, strTrue);
        }

        mqfldcTmp.Set
        (
            tCQCKit::EFldAccess::Write
            , tCQCKit::EFldSTypes::Power
            , tCQCKit::EFldTypes::Boolean
            , strBaseName
            , strFldName
            , TString::strEmpty()
            , strTopic
            , cptrMap
            , mqplfData
            , kCIDLib::True
        );
        m_colFldList.objAdd(mqfldcTmp);
    }

    // And the input field
    {
        const TXMLTreeElement& xtnodeIn = xtnodeCls.xtnodeChildAtAsElement(1);
        const TString& strTopic = xtnodeIn.xtattrNamed(L"Topic").strValue();

        strFldName = L"PWR#";
        strFldName.Append(strBaseName);
        strFldName.Append(kCIDLib::chTilde);
        strFldName.Append(L"Status");

        // Parse out a payload format
        const TXMLTreeElement& xtnodePLFmt = xtnodeIn.xtnodeChildAtAsElement(0);
        mqplfData.ParseFrom
        (
            strFldName
            , tCQCKit::EFldSTypes::PowerState
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
            , xtnodePLFmt
            , colErrs
        );

        // And next we have an enum map which can parse itself out
        const TXMLTreeElement& xtnodeMap = xtnodeIn.xtnodeChildAtAsElement(1);
        TMQTTEnumMap* pmqvmapIn = new TMQTTEnumMap();
        cptrMap.SetPointer(pmqvmapIn);
        pmqvmapIn->bParseFrom(strFldName, tCQCKit::EFldAccess::Read, xtnodeMap, colErrs);

        mqfldcTmp.Set
        (
            tCQCKit::EFldAccess::Read
            , tCQCKit::EFldSTypes::PowerState
            , tCQCKit::EFldTypes::String
            , strBaseName
            , strFldName
            , TString::strEmpty()
            , strTopic
            , cptrMap
            , mqplfData
            , kCIDLib::True
        );
        m_colFldList.objAdd(mqfldcTmp);
    }
}
