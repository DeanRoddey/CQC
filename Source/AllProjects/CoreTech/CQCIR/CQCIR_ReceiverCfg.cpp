//
// FILE NAME: CQCIR_ReceiverCfg.cpp
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
//  This file implements the class that represents the on disk and in memory
//  format of IR receiver training data.
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
RTTIDecls(TIRReceiverCfg,TObject)


// ---------------------------------------------------------------------------
//  Local types anc constants
// ---------------------------------------------------------------------------
namespace CQCIR_Config
{
    namespace
    {
        constexpr tCIDLib::TCard2   c2FmtVersion = 1;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TIRReceiverCfg
//  PREFIX: irc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIRReceiverCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TIRReceiverCfg::TIRReceiverCfg() :

    m_colEvents(29, TStringKeyOps(), &TCQCStdKeyedCmdSrc::strKey)
{
}

TIRReceiverCfg::~TIRReceiverCfg()
{
}


// ---------------------------------------------------------------------------
//  TIRReceiverCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIRReceiverCfg::Add(const TCQCStdKeyedCmdSrc& smeToAdd)
{
    m_colEvents.objAdd(smeToAdd);
}


tCIDLib::TBoolean
TIRReceiverCfg::bAddOrUpdate(const TCQCStdKeyedCmdSrc& smeToAdd)
{
    tCIDLib::TBoolean bAdded;
    m_colEvents.objAddOrUpdate(smeToAdd, bAdded);
    return bAdded;
}


tCIDLib::TBoolean TIRReceiverCfg::bIsEmpty() const
{
    return m_colEvents.bIsEmpty();
}


tCIDLib::TCard4 TIRReceiverCfg::c4ElemCount() const
{
    return m_colEvents.c4ElemCount();
}


TIRReceiverCfg::TEventList::TCursor TIRReceiverCfg::cursEvents() const
{
    return TEventList::TCursor(&m_colEvents);
}


const TCQCStdKeyedCmdSrc*
TIRReceiverCfg::pcsrcFindByKey(const TString& strKeyToCheck)
{
    return m_colEvents.pobjFindByKey(strKeyToCheck);
}


tCIDLib::TVoid TIRReceiverCfg::RemoveKey(const TString& strKeyToRemove)
{
    m_colEvents.bRemoveKey(strKeyToRemove);
}


tCIDLib::TVoid TIRReceiverCfg::Reset()
{
    m_colEvents.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TIRReceiverCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIRReceiverCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCIR_Config::c2FmtVersion)
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

    // Get the number of elements stored, plus that value XOR'd
    tCIDLib::TCard4 c4Count;
    tCIDLib::TCard4 c4XORCount;
    strmToReadFrom >> c4Count >> c4XORCount;

    // If it doesn't agree, then we are in bad shape
    if (c4Count != (c4XORCount ^ kCIDLib::c4MaxCard))
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcCfg_BadElemCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Flush our current collection
    m_colEvents.RemoveAll();

    // Read in the number of elements indicated
    TCQCStdKeyedCmdSrc csrcTmp;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // And we should have a frame marker next
        strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

        // And now stream in the next event and add it to our list
        strmToReadFrom >> csrcTmp;
        m_colEvents.objAdd(csrcTmp);
    }

    // And it should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TIRReceiverCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    //
    //  Stream out a marker and a format version that will let us handle
    //  upgrading the file contents automatically later.
    //
    //  Write out the number of elements we are going to stream, and then
    //  write it out XOR'd for safety.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIR_Config::c2FmtVersion
                    << m_colEvents.c4ElemCount()
                    << (m_colEvents.c4ElemCount() ^ kCIDLib::c4MaxCard);

    // Iterate the items and store them
    TEventList::TCursor cursEvents(&m_colEvents);
    for (; cursEvents; ++cursEvents)
        strmToWriteTo << tCIDLib::EStreamMarkers::Frame << *cursEvents;

    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


