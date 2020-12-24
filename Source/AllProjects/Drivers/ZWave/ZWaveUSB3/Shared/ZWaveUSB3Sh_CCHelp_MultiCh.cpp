//
// FILE NAME: ZWaveUSB3Sh_CCHelp_MultiCh.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/24/2018
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
//  This is a CC helper namespace to provide some multi-channel oriented helper
//  functions.
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
RTTIDecls(TZWUSB3EndPntInfo, TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_ChHelp_MultiCh
{
    // Persistent format versions for our classes
    const tCIDLib::TCard2   c2EPInfoFmtVersion   = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3EndPntInfo
//  PREFIX: associ
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3EndPntInfo: Constructors and Destructor
// ---------------------------------------------------------------------------

// Set purposefully invalid values in case they get used by accident
TZWUSB3EndPntInfo::TZWUSB3EndPntInfo() :

    m_bDynamic(kCIDLib::False)
    , m_c1Generic(0)
    , m_c1Specific(0)
    , m_c4EndPntId(0xFF)
{
}

TZWUSB3EndPntInfo::TZWUSB3EndPntInfo(const TZWUSB3EndPntInfo& zwepiSrc) :

    m_bDynamic(zwepiSrc.m_bDynamic)
    , m_c1Generic(zwepiSrc.m_c1Generic)
    , m_c1Specific(zwepiSrc.m_c1Specific)
    , m_c4EndPntId(zwepiSrc.m_c4EndPntId)
    , m_fcolClasses(zwepiSrc.m_fcolClasses)
{
}

TZWUSB3EndPntInfo::~TZWUSB3EndPntInfo()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3EndPntInfo: Public operators
// ---------------------------------------------------------------------------

TZWUSB3EndPntInfo& TZWUSB3EndPntInfo::operator=(const TZWUSB3EndPntInfo& zwepiSrc)
{
    if (&zwepiSrc != this)
    {
        m_bDynamic     = zwepiSrc.m_bDynamic;
        m_c1Generic    = zwepiSrc.m_c1Generic;
        m_c1Specific   = zwepiSrc.m_c1Specific;
        m_c4EndPntId   = zwepiSrc.m_c4EndPntId;
        m_fcolClasses  = zwepiSrc.m_fcolClasses;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWUSB3EndPntInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWUSB3EndPntInfo::Reset()
{
    m_bDynamic      = kCIDLib::False;
    m_c1Generic     = 0;
    m_c1Specific    = 0;
    m_c4EndPntId    = 0xFF;
    m_fcolClasses.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TZWUSB3EndPntInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWUSB3EndPntInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
     // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSB3Sh_ChHelp_MultiCh::c2EPInfoFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_bDynamic
                    >> m_c1Generic
                    >> m_c1Specific
                    >> m_c4EndPntId
                    >> m_fcolClasses;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TZWUSB3EndPntInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSB3Sh_ChHelp_MultiCh::c2EPInfoFmtVersion
                    << m_bDynamic
                    << m_c1Generic
                    << m_c1Specific
                    << m_c4EndPntId
                    << m_fcolClasses
                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   NAMESPACE: TZWUSB3CCMultiCh
// ---------------------------------------------------------------------------

// Query the number of end points. They will be 1 to the count returned
tCIDLib::TBoolean
TZWUSB3CCMultiCh::bQueryEPCount(        TZWUnitInfo&        unitiSrc
                                ,       tCIDLib::TCard4&    c4ToFill
                                , const tCIDLib::TBoolean   bAsync
                                , const tCIDLib::TBoolean   bKnownAwake
                                ,       TString&            strErrMsg)
{
    // If multi-channel support is not there, then obviously dumb
    if (!unitiSrc.bSupportsClass(tZWaveUSB3Sh::ECClasses::MultiCh))
    {
        strErrMsg.LoadFromMsg
        (
            kZW3ShErrs::errcUnit_ClassNotSup
            , facZWaveUSB3Sh()
            , unitiSrc.strName()
            , tZWaveUSB3Sh::strXlatECClasses(tZWaveUSB3Sh::ECClasses::MultiCh)
        );
        return kCIDLib::False;
    }

    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiSrc
            , COMMAND_CLASS_MULTI_CHANNEL_V4
            , MULTI_CHANNEL_END_POINT_GET_V4
            , MULTI_CHANNEL_END_POINT_REPORT_V4
            , tZWaveUSB3Sh::EMsgPrios::Query
        );

        if (bAsync)
        {
            unitiSrc.SendMsgAsync(zwomOut, bKnownAwake);
        }
         else
        {
            TZWInMsg zwimReply;
            unitiSrc.SendWaitReply(zwomOut, zwimReply, bKnownAwake);
            c4ToFill = zwimReply.c1CCByteAt(3) & 0x7F;
        }
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean
TZWUSB3CCMultiCh::bQueryEndPoint(       TZWUnitInfo&        unitiSrc
                                , const tCIDLib::TCard4&    c4EPToQuery
                                , const tCIDLib::TBoolean   bAsync
                                , const tCIDLib::TBoolean   bKnownAwake
                                ,       TZWUSB3EndPntInfo&  zwepiToFill
                                ,       TString&            strErrMsg)
{
    zwepiToFill.Reset();

    // If multi-channel support is not there, then obviously dumb
    if (!unitiSrc.bSupportsClass(tZWaveUSB3Sh::ECClasses::MultiCh))
    {
        strErrMsg.LoadFromMsg
        (
            kZW3ShErrs::errcUnit_ClassNotSup
            , facZWaveUSB3Sh()
            , unitiSrc.strName()
            , tZWaveUSB3Sh::strXlatECClasses(tZWaveUSB3Sh::ECClasses::MultiCh)
        );
        return kCIDLib::False;
    }

    // It must be 1 to 127
    if (!c4EPToQuery || (c4EPToQuery > 127))
    {
        strErrMsg.LoadFromMsg(kZW3ShErrs::errcCfg_EPIdRange, facZWaveUSB3Sh());
        return kCIDLib::False;
    }

    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiSrc
            , COMMAND_CLASS_MULTI_CHANNEL_V4
            , MULTI_CHANNEL_CAPABILITY_GET_V4
            , MULTI_CHANNEL_CAPABILITY_REPORT_V4
            , tCIDLib::TCard1(c4EPToQuery & 0x7F)
            , tZWaveUSB3Sh::EMsgPrios::Query
        );

        if (bAsync)
        {
            unitiSrc.SendMsgAsync(zwomOut, bKnownAwake);
        }
         else
        {
            TZWInMsg zwimReply;
            unitiSrc.SendWaitReply(zwomOut, zwimReply, bKnownAwake);

            //
            //  This one is variable size so figure out how many command classes
            //  there are based on msg size.
            //
            const tCIDLib::TCard4 c4Bytes = zwimReply.c4CCBytes();
            zwepiToFill.m_c1Generic = zwimReply.c1CCByteAt(3);
            zwepiToFill.m_c1Specific = zwimReply.c1CCByteAt(4);
            for (tCIDLib::TCard4 c4Index = 5; c4Index < c4Bytes; c4Index++)
            {
                const tZWaveUSB3Sh::ECClasses eClass
                (
                    tZWaveUSB3Sh::eAltNumECClasses(zwimReply.c1CCByteAt(c4Index))
                );

                if (eClass != tZWaveUSB3Sh::ECClasses::None)
                    zwepiToFill.m_fcolClasses.c4AddElement(eClass);
            }

            zwepiToFill.m_bDynamic = (zwimReply.c1CCByteAt(2) & 0x80) != 0;
        }
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }

    return kCIDLib::True;
}
