//
// FILE NAME: CQCRemBrws_TemplateInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/22/2003
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
//  Just for conversion purposes now, see the header comments
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRemBrws_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfTmplInfo,TObject)
RTTIDecls(TCQCIntfTmplData,TCQCIntfTmplInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_TemplateInfo
    {
        // -----------------------------------------------------------------------
        //  Our info persistent format version
        //
        //  Version 1 -
        //      We reset the format version for 1.4 since this data gets re-done
        //      during the installer upgrade process.
        //
        //  Version 2 -
        //      Removed the XML representation of the template now that the .Net
        //      Viewer has been replaced by the RIVA system.
        //
        //  Version 3 -
        //      The template notes value is stored here redundantly so that it
        //      can be accessed via the basic info class.
        // -----------------------------------------------------------------------
        constexpr   tCIDLib::TCard2 c2InfoFmtVersion    = 3;


        // -----------------------------------------------------------------------
        //  Our data persistent format version
        //
        //  Version 1 -
        //      The original version
        //
        //  Version 2 -
        //      Bogus version change but can't undo that now.
        //
        //  Version 3 -
        //      Made the serial number persistent for easier debugging of data server
        //      issues.
        // -----------------------------------------------------------------------
        constexpr   tCIDLib::TCard2 c2DataFmtVersion    = 3;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTmplInfo
//  PREFIX: itfi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfTmplInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfTmplInfo::TCQCIntfTmplInfo() :

    m_c4Bytes(0)
    , m_eMinRole(tCQCKit::EUserRoles::LimitedUser)
{
}

TCQCIntfTmplInfo::~TCQCIntfTmplInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfTmplInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfTmplInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // It must start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_TemplateInfo::c2InfoFmtVersion))
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

    // Looks ok, so pull out the template name
    strmToReadFrom  >> m_strName
                    >> m_eMinRole;

    //
    //  That worked, so get the bytes that were stored so that we know how
    //  many bytes to read in. For safety we stored a XOR'd version of the
    //  byte count, so get that too and verify it.
    //
    tCIDLib::TCard4 c4XORBytes;
    strmToReadFrom >> m_c4Bytes >> c4XORBytes;
    if (m_c4Bytes != (c4XORBytes ^ kCIDLib::c4MaxCard))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcIntf_BadPersistBytes
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // If version 1, then read in and discard the formatted XML data
    if (c2FmtVersion == 1)
    {
        TString strTmp;
        strmToReadFrom >> strTmp;
    }

    // If version 3 or beyond, read in the notes, else clear them
    if (c2FmtVersion >= 3)
        strmToReadFrom >> m_strNotes;
    else
        m_strNotes.Clear();

    // It must end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfTmplInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    //
    //  Stream out a start object marker and all of the easy stuff. Note
    //  that we stream out the bytes of the buffer that are used, so that we
    //  will know how much to stream back in later. We also write out XOR'd
    //  version of the byte count, for safety.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_TemplateInfo::c2InfoFmtVersion
                    << m_strName
                    << m_eMinRole
                    << m_c4Bytes
                    << tCIDLib::TCard4(m_c4Bytes ^ kCIDLib::c4MaxCard)
                    << m_strNotes
                    << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTmplData
//  PREFIX: itfd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfTmplData: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfTmplData::TCQCIntfTmplData() :

    m_c4SerialNum(facCIDCrypto().c4GetRandom(kCIDLib::True))
{
}

TCQCIntfTmplData::~TCQCIntfTmplData()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfTmplData: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfTmplData::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Do our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Our stuff must start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_TemplateInfo::c2DataFmtVersion))
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

    //
    //  Looks ok, so read it in. Then read in the check sum that should
    //  follow and make sure it matches.
    //
    strmToReadFrom.c4ReadBuffer(m_mbufData, m_c4Bytes);
    tCIDLib::TCard4 c4Sum;
    strmToReadFrom >> c4Sum;
    if (c4Sum != m_mbufData.c4CheckSum(0, m_c4Bytes))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcIntf_BadPersistSum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // If V3 or beyond read in the persistent serial number, else gen one up
    if (c2FmtVersion > 2)
        strmToReadFrom >> m_c4SerialNum;
    else
        m_c4SerialNum = facCIDCrypto().c4GetRandom(kCIDLib::True);

    // It must end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfTmplData::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our parent first
    TParent::StreamTo(strmToWriteTo);

    //  Stream out a start object marker to start our stuff, and our version marker.
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCKit_TemplateInfo::c2DataFmtVersion;

    // Only stream out as many bytes as are used in the buffer
    strmToWriteTo.c4WriteBuffer(m_mbufData, m_c4Bytes);

    // Ask the buffer to checksum itself and write that out
    tCIDLib::TCard4 c4Sum = m_mbufData.c4CheckSum(0, m_c4Bytes);
    strmToWriteTo << c4Sum;

    // And the rest of our stuff and end up with an end object marker
    strmToWriteTo   << m_c4SerialNum
                    << tCIDLib::EStreamMarkers::EndObject;
}
