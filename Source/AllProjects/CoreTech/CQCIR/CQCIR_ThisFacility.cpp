//
// FILE NAME: CQCIR_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2003
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIR_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCIR,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIR
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCIR: Public, static data
// ---------------------------------------------------------------------------
const TString TFacCQCIR::strFldName_FirmwareVer(L"FirmwareVer");
const TString TFacCQCIR::strFldName_Invoke(L"Invoke");
const TString TFacCQCIR::strFldName_TrainingState(L"TrainingState");


// ---------------------------------------------------------------------------
//  TFacCQCIR: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCIR::TFacCQCIR() :

    TFacility
    (
        L"CQCIR"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_ctarGVars(tCIDLib::EMTStates::Safe, kCIDLib::False)
{
}

TFacCQCIR::~TFacCQCIR()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCIR: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  These guys check a passed string to see if it appears to hold, as best
//  we can figure out, a valid formatted IR blaster code for various formats
//  that we support.
//
//  We use a regular expression to do this checking, which makes for some
//  kind of tortured expressions, which still don't absolutely guarantee
//  the validity of the code, but close enough.
//
//  We take the overhead of recreating the regular expression on every call so
//  that we can remain reentrant. Multiple drivers could be calling these
//  guys. And it's not something ever called rapidly. It's mostly for validating
//  manually input IR data.
//
tCIDLib::TBoolean
TFacCQCIR::bIsValidGC100BlastData(const TString& strTest) const
{
    TRegEx regxTest
    (
        L"sendir,%\\(m\\):%\\(c\\),%\\(i\\),[0-9]+,%\\(C\\)(,([0-9])+)+"
    );
    return regxTest.bFullyMatches(strTest);
}

tCIDLib::TBoolean TFacCQCIR::bIsValidR2DIBlastData(const TString& strTest) const
{
    //
    //  We accept the overhead of setting it up every time because we have
    //  to be reentrant here.
    //
    TRegEx regxTest(L"AIRX([A-Za-z0-9])+:AIRD([A-Za-z0-9]+:)+AIRE");
    return regxTest.bFullyMatches(strTest);
}

tCIDLib::TBoolean TFacCQCIR::bIsValidUIRTBlastData(const TString& strTest) const
{
    //
    //  We accept the overhead of setting it up every time because we have
    //  to be reentrant here.
    //
    TRegEx regxTest
    (
        L"([A-Za-z0-9][A-Za-z0-9][A-Za-z0-9][A-Za-z0-9] *)+"
        L"[A-Za-z0-9][A-Za-z0-9][A-Za-z0-9][A-Za-z0-9]"
    );
    return regxTest.bFullyMatches(strTest);
}


//
//  This is used to limit the chars that can be used in an IR device model
//  name or IR command name, to avoid ambiguity issues when they are used
//  in text commands.
//
tCIDLib::TBoolean
TFacCQCIR::bIsValidNameChar(const   tCIDLib::TCh    chToCheck
                            , const tCIDLib::TCard4 c4Index) const
{
    // We don't allow colons, quotes, or periods in the names
    static const tCIDLib::TCh achList[] =
    {
        kCIDLib::chApostrophe
        , kCIDLib::chColon
        , kCIDLib::chPeriod
        , kCIDLib::chQuotation
        , kCIDLib::chComma
        , kCIDLib::chNull
    };

    // If it's not in the list, then it's ok
    return !TRawStr::pszFindChar(achList, chToCheck, 0, kCIDLib::False);
}


//
//  All of the IR blasters take the same format string to their invoke field,
//  which is how user actions make the blaster drivers send commands. It's
//  always in one of two formats:
//
//  devname.cmdname
//  devname.cmdName:z
//
//  So they send a device name, which indicates the device model, and a
//  command name which indicates which of that device's commands should be
//  sent. They can optionally send a zone number, which is 1 through whatever
//  is the number of zones available on that device. If not device is sent,
//  1 (zero internally) is assumed.
//
tCIDLib::TCard4
TFacCQCIR::c4ParseInvokeCmd(const   TString&        strToParse
                            ,       TString&        strDevName
                            ,       TString&        strCmdName
                            , const tCIDLib::TCard4 c4ZoneCount
                            , const TString&        strMoniker) const
{
    const tCIDLib::TCard4 c4Len = strToParse.c4Length();

    //
    //  We use a small state machine here to parse through the format, just
    //  to keep from doing a lot of special case checking and whatnot.
    //
    enum EStates
    {
        EState_Start
        , EState_DevName
        , EState_Period
        , EState_CmdName
        , EState_Colon
        , EState_Zone
        , EState_End
        , EState_Done
        , EState_Error
    };

    // Make sure the out names are cleared since we just append
    strDevName.Clear();
    strCmdName.Clear();

    tCIDLib::TCard4     c4Index = 0;
    tCIDLib::TCard4     c4ZoneRet = 1;
    EStates             eCurState = EState_Start;
    TString             strZone;
    while ((c4Index < c4Len) && (eCurState != EState_Error))
    {
        const tCIDLib::TCh chCur = strToParse[c4Index];

        switch(eCurState)
        {
            case EState_Start :
            {
                // Eat any leading whitespace, else move forward
                if (!TRawStr::bIsSpace(chCur))
                {
                    // Make sure it's a valid name char
                    if (bIsValidNameChar(chCur, c4Index))
                    {
                        eCurState = EState_DevName;
                        strDevName.Append(chCur);
                    }
                     else
                    {
                        eCurState = EState_Error;
                    }
                }
                break;
            }

            case EState_DevName :
            {
                if (chCur == kCIDLib::chPeriod)
                {
                    if (strDevName.bIsEmpty())
                        eCurState = EState_Error;
                    else
                        eCurState = EState_Period;
                }
                 else
                {
                    // Has to be valid name char
                    if (bIsValidNameChar(chCur, c4Index))
                        strDevName.Append(chCur);
                    else
                        eCurState = EState_Error;
                }
                break;
            }

            case EState_Period :
            {
                // Next must be a valid name char starting the command name
                if (bIsValidNameChar(chCur, c4Index))
                {
                    strCmdName.Append(chCur);
                    eCurState = EState_CmdName;
                }
                 else
                {
                    eCurState = EState_Error;
                }
                break;
            }

            case EState_CmdName :
            {
                if (chCur == kCIDLib::chColon)
                {
                    if (strCmdName.bIsEmpty())
                        eCurState = EState_Error;
                    else
                        eCurState = EState_Colon;
                }
                 else
                {
                    // Has to be valid name char
                    if (bIsValidNameChar(chCur, c4Index))
                        strCmdName.Append(chCur);
                    else
                        eCurState = EState_Error;
                }
                break;
            }

            case EState_Colon :
            {
                // Has to be a digit
                if (TRawStr::bIsDigit(chCur))
                {
                    strZone.Append(chCur);
                    eCurState = EState_Zone;
                }
                 else
                {
                    eCurState = EState_Error;
                }
                break;
            }

            case EState_Zone :
            {
                //
                //  We have to get only decimal digits here. If we hit a
                //  space char and have at least one digit, we'll take that
                //  and move on.
                //
                if (TRawStr::bIsSpace(chCur))
                {
                    if (!strZone.bIsEmpty())
                        eCurState = EState_End;
                }
                 else if (TRawStr::bIsDigit(chCur))
                {
                    strZone.Append(chCur);
                }
                 else
                {
                    eCurState = EState_Error;
                }
                break;
            }

            case EState_End :
            {
                // Eat any trailing whitespace, but anything else is bad
                if (!TRawStr::bIsSpace(chCur))
                    eCurState = EState_Error;
                break;
            }
        };

        // Move up to handle the next char if not already in error
        if (eCurState != EState_Error)
        {
            c4Index++;

            //
            //  If we hit the end, then see if we are in a place where that
            //  is valid. If not, put us into error state.
            //
            if (c4Index == c4Len)
            {
                if ((eCurState == EState_CmdName)
                ||  (eCurState == EState_Zone))
                {
                    eCurState = EState_Done;
                }
                 else
                {
                    eCurState = EState_Error;
                }
            }
        }
    }

    // Try to convert the zone string, if any
    tCIDLib::TBoolean bZoneOk = kCIDLib::True;
    if ((eCurState != EState_Error) && !strZone.bIsEmpty())
        c4ZoneRet = TRawStr::c4AsBinary(strZone.pszBuffer(), bZoneOk, tCIDLib::ERadices::Dec);

    //
    //  If we didn't reach the done state or there is anything else left
    //  in the string or a bad zone, then not happy
    //
    if ((eCurState != EState_Done)
    ||  (c4Index != c4Len)
    ||  !bZoneOk
    ||  (c4ZoneRet == 0))
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcProto_BadInvokeFmt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(c4Index)
        );
    }

    //
    //  Make sure the zone is legal for the passed zone count. The number
    //  is 1 based here so check for > not >=.
    //
    if (c4ZoneRet > c4ZoneCount)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcBlast_BadZone
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4ZoneRet)
            , strMoniker
        );
    }

    //
    //  Return the zone we got, or the default if we never saw one. Adjust
    //  by one since the user uses 1 based numbers but we use zero based
    //  numbers internally.
    //
    return c4ZoneRet - 1;
}


//
//  Given the moniker of a server side IR device driver, this method will call
//  a backdoor method on the driver to get a list of device models that are
//  load on that driver. The caller could have just as easily done this
//  himself, but this just keeps it hidden away, since the format is based on
//  flatted data that must be ressurrected and not a typed call. This is
//  due to the fact that we don't want to have the driver maintain a separate
//  specialized ORB interface.
//
//  We also have a version that takes a pre-existing proxy.
//
//  They both return the count of zones supported by the target IR device.
//
tCIDLib::TCard4
TFacCQCIR::c4QueryDevIRModels(          tCQCKit::TCQCSrvProxy&      orbcDev
                                , const TString&                    strMoniker
                                ,       TFacCQCIR::TModelInfoList&  colToFill)
{
    tCIDLib::TCard4 c4ZoneCount;
    {
        tCIDLib::TCard4 c4Bytes;
        THeapBuf mbufData(8192UL);
        orbcDev->bQueryData
        (
            strMoniker
            , kCQCIR::strQueryBlasterCfg
            , TString::strEmpty()
            , c4Bytes
            , mbufData
        );
        TBinMBufInStream strmData(&mbufData, c4Bytes);
        strmData >> colToFill >> c4ZoneCount;
    }
    return c4ZoneCount;
}

tCIDLib::TCard4
TFacCQCIR::c4QueryDevIRModels(  const   TString&                    strMoniker
                                ,       TFacCQCIR::TModelInfoList&  colToFill)
{
    // Get a proxy for the target driver and do the call
    tCQCKit::TCQCSrvProxy orbcDev = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);
    tCIDLib::TCard4 c4ZoneCount;
    {
        tCIDLib::TCard4 c4Bytes;
        THeapBuf mbufData(8192UL);
        orbcDev->bQueryData
        (
            strMoniker
            , kCQCIR::strQueryBlasterCfg
            , TString::strEmpty()
            , c4Bytes
            , mbufData
        );
        TBinMBufInStream strmData(&mbufData, c4Bytes);
        strmData >> colToFill >> c4ZoneCount;
    }
    return c4ZoneCount;
}


//
//  This is a convenience that extracts some IR device info from the driver
//  config data, put there by fixed prompt values in the manifest.
//
tCQCIR::EIRDevCaps
TFacCQCIR::eExtractDevInfo( const   TCQCDriverObjCfg&   cqcdcToLoad
                            ,       TString&            strType
                            ,       TString&            strDescr)
{
    TString strCaps;
    if ((cqcdcToLoad.c4FixedCount() != 1)
    ||  !cqcdcToLoad.bFindFixedVal(L"DevInfo", L"DevDescr", strDescr)
    ||  !cqcdcToLoad.bFindFixedVal(L"DevInfo", L"DevType", strType)
    ||  !cqcdcToLoad.bFindFixedVal(L"DevInfo", L"DevCaps", strCaps))
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcCfg_NoManifestInfo
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Parse out the capabilities flags
    tCQCIR::EIRDevCaps eCaps = tCQCIR::EIRDevCaps::None;
    TStringTokenizer stokCaps(&strCaps, L", ");
    TString strToken;
    while(stokCaps.bGetNextToken(strToken))
    {
        // Account for some old style enums still around in existing config
        strToken.StripWhitespace();
        if (strToken.bStartsWith(L"EIRDevCap_"))
            strToken.Cut(0, 10);

        eCaps = tCIDLib::eOREnumBits(eCaps, tCQCIR::eXlatEIRDevCaps(strToken));
    }
    return eCaps;
}


//
//  Just provides access to our global variables object. This is used as
//  a global variable scope for all actions invoked via the IR/generic
//  trigger drivers.
//
TStdVarsTar& TFacCQCIR::ctarGVars()
{
    return m_ctarGVars;
}


tCQCIR::TRepoSrvProxy
TFacCQCIR::orbcRepoSrvProxy(const tCIDLib::TCard4 c4WaitUpTo) const
{
    tCQCIR::TRepoSrvProxy orbcRet
    (
        facCIDOrbUC().porbcMakeClient<TIRRepoClientProxy>
        (
            TIRRepoClientProxy::strBinding
        )
    );

    if (!orbcRet.pobjData())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcRem_IRSrvNotAvail
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }
    return orbcRet;
}


//
//  Gets a Pronto IR code, and writes it out to the output text stream in
//  the GC-100 format.
//
tCIDLib::TVoid TFacCQCIR::ProntoToGC100(const   TString&        strSrc
                                        ,       TTextOutStream& strmTarget)
{
    //
    //  We format some tokens that will be replaced at runtime by the
    //  target device address info.
    //
    strmTarget << L"sendir,%(m):%(c),%(i),";

    //
    //  Now we have to work our way through, handling one of the
    //  hex values at a time.
    //
    TStringTokenizer stokValue(&strSrc, L" ");

    // Skip the header entry, which is always zero
    stokValue.bEatNextToken();

    // The next one is the frequency value, which we have to massage
    TString strToken;
    stokValue.bGetNextToken(strToken);
    tCIDLib::TCard4 c4Tmp = strToken.c4Val(tCIDLib::ERadices::Hex);
    if (!c4Tmp)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcFmt_ZeroFreqBase
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    c4Tmp = (((41450 / c4Tmp) + 5) / 10) * 1000;
    strmTarget << c4Tmp;

    // Put out the repeat count token
    strmTarget << L",%(C)";

    // Next is the offset, which requires some massaging
    stokValue.bGetNextToken(strToken);
    c4Tmp = strToken.c4Val(tCIDLib::ERadices::Hex);
    strmTarget << L"," << ((2 * c4Tmp) + 1);

    // Skip one
    stokValue.bEatNextToken();

    // And the rest just get converted to decimal
    while (stokValue.bGetNextToken(strToken))
        strmTarget << L"," << strToken.c4Val(tCIDLib::ERadices::Hex);

    strmTarget << kCIDLib::FlushIt;
}


//
//  Gets a Pronto IR code, and writes it out to the output stream in the
//  R2DI format.
//
tCIDLib::TVoid TFacCQCIR::ProntoToR2DI( const   TString&        strSrc
                                        ,       TTextOutStream& strmTarget)
{
    // Break the string into separate pairs
    TVector<TString> colPairs;

    {
        TStringTokenizer stokValue(&strSrc, L" ");
        TString strCur;
        while (stokValue.bGetNextToken(strCur))
            colPairs.objAdd(strCur);
    }

    // It has to be at least 8 pairs
    const tCIDLib::TCard4 c4ValCount = colPairs.c4ElemCount();
    if (c4ValCount < 8)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcFmt_TooFewPairs
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
        return;
    }

    //
    //  Take the Pronto frequency base and convert to a frequency. THen
    //  calculate the multiplier for each on/off period. Since the Pronto
    //  on/off values are in terms of cycles of the frequency per second,
    //  we have to convert that to a value that will convert to micro
    //  seconds, which is what the values are in terms of for R2DI.
    //
    //  So first get the hertz and then convert to hundreds of cycles
    //  per second and round.
    //
    const tCIDLib::TCard4 c4Freq = colPairs[1].c4Val(tCIDLib::ERadices::Hex);
    const tCIDLib::TFloat8 f8R = 4194304.0 / c4Freq;
    tCIDLib::TFloat8 f8Tmp = f8R / 100;
    TMathLib::Round(f8Tmp);

    // Get the 100 hertz value as an unsigned that we can format out later
    const tCIDLib::TCard4 c4K = tCIDLib::TCard4(f8Tmp);

    //
    //  And calculate the number of microseconds per cycle of this
    //  frequency.
    //
    const tCIDLib::TFloat8 f8Interval = 1 / f8R * 1000000;

    //
    //  We can't write it out in order, since we have to write the interval
    //  count first and we don't know that till we've processed it. So we
    //  get the transition data into a temp string.
    //
    TString strTmp;
    TString strFld;
    TString strTransData;
    tCIDLib::TCard4 c4NumInts = 0;
    for (tCIDLib::TCard4 c4Index = 4; c4Index < c4ValCount; c4Index += 2)
    {
        // Calculate the microseconds for this on period
        tCIDLib::TCard4 c4Time = tCIDLib::TCard4
        (
            colPairs[c4Index].c4Val(tCIDLib::ERadices::Hex) * f8Interval
        );

        // Handle times larger than 0x7FFF by doing extra values
        while (c4Time > 0x7FFF)
        {
            strTransData.Append(L"FFFF:");
            c4NumInts++;
            c4Time -= 0x7FFF;
        }

        // Turn on the high bit of any remainder and format it out
        c4Time |= 0x8000;
        strTmp.SetFormatted(c4Time, tCIDLib::ERadices::Hex);
        strFld.FormatToFld(strTmp, 4, tCIDLib::EHJustify::Right, L'0');
        strTransData.Append(strFld);
        strTransData.Append(L':');
        c4NumInts += 1;

        // Calculate the microseconds for this off period
        c4Time = tCIDLib::TCard4
        (
            colPairs[c4Index + 1].c4Val(tCIDLib::ERadices::Hex) * f8Interval
        );

        // Handle times larger than 0x7FFF by doing extra values
        while (c4Time > 0x7FFF)
        {
            strTransData.Append(L"7FFF:");
            c4NumInts++;
            c4Time -= 0x7FFF;
        }

        // And format any remainder out without a high bit
        strTmp.SetFormatted(c4Time, tCIDLib::ERadices::Hex);
        strFld.FormatToFld(strTmp, 4, tCIDLib::EHJustify::Right, L'0');
        strTransData.Append(strFld);
        strTransData.Append(L':');
        c4NumInts += 1;
    }

    TStreamFmt strmfDef(0, 0, tCIDLib::EHJustify::Left, kCIDLib::chSpace);
    TStreamFmt strmfHexPair(4, 0, tCIDLib::EHJustify::Right, kCIDLib::chDigit0);

    //
    //  Ok, write it all out. Note that we don't deal with repeats here. That
    //  will be put on the command on the fly by the driver as indicated by
    //  the repeat setting the user has set on the device model.
    //
    strmTarget  << L"AIRX" << strmfHexPair
                << TCardinal(c4K, tCIDLib::ERadices::Hex) << strmfDef
                << L":AIRD" << strmfHexPair
                << TCardinal(c4NumInts, tCIDLib::ERadices::Hex) << strmfDef << L':'
                << strTransData
                << L"AIRE";

    strmTarget << kCIDLib::FlushIt;
}


//
//  This is a fair amount of standard goop that goes at the top of our IRExport
//  files, so we provide this standard method to stream one out, based on
//  proivded device, make, model, etc... info.
//
//  For completeness and since we open up elements in the header, we also
//  provide a footer writer, so all the callers have to do is write out
//  the simple command format.
//
//  The footer writer gets some stuff that we don't use now, but we make them
//  pass it anyway, in case the format should change in the future.
//
tCIDLib::TVoid
TFacCQCIR::WriteExpFooter(          TTextOutStream&     strmExp
                            , const TString&
                            , const TString&
                            , const TString&            )
{
    strmExp << L"  </CQCIR:Commands>\n</CQCIR:ExportFile>\n"
            << kCIDLib::FlushIt;
}

tCIDLib::TVoid
TFacCQCIR::WriteExpHeader(          TTextOutStream&     strmExp
                            , const TString&            strDevType
                            , const TString&            strMake
                            , const TString&            strModel
                            , const TString&            strDescription
                            , const TString&            strCategory)
{
    strmExp << L"<?xml version='1.0' encoding='UTF-8'?>\n"
            << L"<CQCIR:ExportFile CQCIR:Blaster='" << strDevType << L"'\n                  CQCIR:Make='" << strMake
            << L"'\n                  CQCIR:Model='" << strModel
            << L"'\n                  CQCIR:Description='" << strDescription
            << L"'\n                  CQCIR:Category='" << strCategory
            << L"'\n                  xmlns:CQCIR='http://www.charmedquark.com/IRExport'"
            << L">\n"
            << L"  <CQCIR:Commands>\n";
}



