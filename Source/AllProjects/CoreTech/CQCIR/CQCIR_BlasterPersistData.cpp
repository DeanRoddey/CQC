//
// FILE NAME: CQCIR_BlasterPersistData.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/12/2003
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
//  This is the header file for the CQCIR_BlasterPersistData.cpp file. This
//  file implements the TIRBlasterPerData class.
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
RTTIDecls(TIRBlasterPerData,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIR_BlasterPersistData
{
    namespace
    {
        // Our file format version marker
        constexpr   tCIDLib::TCard2 c2FmtVersion    = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TIRBlasterPerData
//  PREFIX: perd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TIRBlasterPerData::TIRBlasterPerData()
{
}

TIRBlasterPerData::TIRBlasterPerData(const TString& strMoniker) :

    m_strMoniker(strMoniker)
{
}

TIRBlasterPerData::~TIRBlasterPerData()
{
}


// ---------------------------------------------------------------------------
//  TIRBlasterPerData: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIRBlasterPerData::AddModel(const TString& strToAdd)
{
    m_colModels.objAdd(strToAdd);
}


TIRBlasterPerData::TModelCursor TIRBlasterPerData::cursModels() const
{
    return TModelCursor(&m_colModels);
}


tCIDLib::TVoid TIRBlasterPerData::RemoveModel(const TString& strToRemove)
{
    // See if this guy is in the list and remove it if so
    const tCIDLib::TCard4 c4Count = m_colModels.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (strToRemove == m_colModels[c4Index])
        {
            m_colModels.RemoveAt(c4Index);
            break;
        }
    }
}


const TString& TIRBlasterPerData::strMoniker() const
{
    return m_strMoniker;
}


tCIDLib::TVoid
TIRBlasterPerData::SetModels(TBlasterCursor cursModels)
{
    // Clear out our current list
    m_colModels.RemoveAll();

    // Now iterate the cursor and add all the device model names to our list
    for (; cursModels; ++cursModels)
        m_colModels.objAdd(cursModels->strModel());
}


// ---------------------------------------------------------------------------
//  TIRBlasterPerData: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIRBlasterPerData::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (c2FmtVersion != CQCIR_BlasterPersistData::c2FmtVersion)
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

    // Flush our device list before we reload
    m_colModels.RemoveAll();

    strmToReadFrom  >> m_strMoniker
                    >> m_colModels;

    // Make sure we get the end obj marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TIRBlasterPerData::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIR_BlasterPersistData::c2FmtVersion
                    << m_strMoniker
                    << m_colModels
                    << tCIDLib::EStreamMarkers::EndObject;
}


