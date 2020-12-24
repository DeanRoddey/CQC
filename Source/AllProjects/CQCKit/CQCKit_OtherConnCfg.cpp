//
// FILE NAME: CQCKit_OtherConnCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2001
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
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCOtherConnCfg,TCQCConnCfg)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCKit_OtherConnCfg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCOtherConnCfg
//  PREFIX: ccfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCOtherConnCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCOtherConnCfg::TCQCOtherConnCfg()
{
}


TCQCOtherConnCfg::~TCQCOtherConnCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQCOtherConnCfg: Public operators
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCOtherConnCfg::operator==(const TCQCOtherConnCfg&) const
{
    //
    //  We have no data members at this time, and our parent has no externally
    //  important state, so just return true.
    //
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TCQCOtherConnCfg: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCOtherConnCfg::bIsSame(const TCQCConnCfg& conncfgToCompare) const
{
    // See if they are of the same polymorphic type. If not, can't be equal
    if (clsThis() != conncfgToCompare.clsIsA())
        return kCIDLib::False;

    //
    //  They are, so compare them. We cast the other one to our type and
    //  then we can just do an equality test.
    //
    const TCQCOtherConnCfg& conncfgOther = static_cast<const TCQCOtherConnCfg&>
    (
        conncfgToCompare
    );
    return (*this == conncfgOther);
}


tCIDLib::TBoolean TCQCOtherConnCfg::bParseConnInfo(const TString& strInfo)
{
    // No-op for us here. Just say we accept
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCOtherConnCfg::LoadFromXML(const TXMLTreeElement& xtnodeSrc)
{
    // Nothing to do for now, but validate the node type
    AssertElemName(xtnodeSrc, L"CQCCfg:OtherConn")
}


TString TCQCOtherConnCfg::strConnInfo() const
{
    // We don't really have any
    return TString(L"N/A");
}


// ---------------------------------------------------------------------------
//  TCQCOtherConnCfg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  NOTE that the streaming order is wrong here for historic reasons. We should
//  check our frame marker and version first, then call the parent. This should
//  probably be fixed at some point but it's difficult to do (which is why it
//  should be done the other way.)
//
tCIDLib::TVoid TCQCOtherConnCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Do our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_OtherConnCfg::c2FmtVersion))
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

    // We've got not data at this time

    // Check the start object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCOtherConnCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    TParent::StreamTo(strmToWriteTo);
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCKit_OtherConnCfg::c2FmtVersion

                    // We have no data at this time

                    << tCIDLib::EStreamMarkers::EndObject;
}


