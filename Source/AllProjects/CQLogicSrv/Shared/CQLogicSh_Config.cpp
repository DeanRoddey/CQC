//
// FILE NAME: CQLogicSh_Config.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2009
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
#include    "CQLogicSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQLogicSrvCfg,TObject)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQLogicSh_DrvConfig
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format
        //
        //  Version 1 -
        //      Initial version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard1   c1FmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQLogicSrvCfg
//  PREFIX: lscfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQLogicSrvCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQLogicSrvCfg::TCQLogicSrvCfg() :

    m_colFldTypes(tCIDLib::EAdoptOpts::Adopt, kCQLogicSh::c4MaxFields / 8)
{
}

TCQLogicSrvCfg::TCQLogicSrvCfg(const TCQLogicSrvCfg& lscfgSrc) :

    m_colFldTypes
    (
        tCIDLib::EAdoptOpts::Adopt, lscfgSrc.m_colFldTypes.c4ElemCount()
    )
{
    // Do a polymorphic dup of the field types of the source
    const tCIDLib::TCard4 c4Count = lscfgSrc.m_colFldTypes.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_colFldTypes.Add
        (
            ::pDupObject<TCQLSrvFldType>(*lscfgSrc.m_colFldTypes[c4Index])
        );
    }
}

TCQLogicSrvCfg::~TCQLogicSrvCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQLogicSrvCfg: Public oeprators
// ---------------------------------------------------------------------------
TCQLogicSrvCfg& TCQLogicSrvCfg::operator=(const TCQLogicSrvCfg& lscfgSrc)
{
    if (this != &lscfgSrc)
    {
        // Flush our current list and poly dup the source list
        m_colFldTypes.RemoveAll();

        const tCIDLib::TCard4 c4Count = lscfgSrc.m_colFldTypes.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colFldTypes.Add
            (
                ::pDupObject<TCQLSrvFldType>(*lscfgSrc.m_colFldTypes[c4Index])
            );
        }
    }
    return *this;
}


tCIDLib::TBoolean
TCQLogicSrvCfg::operator==(const TCQLogicSrvCfg& lscfgComp) const
{
    const tCIDLib::TCard4 c4Count = m_colFldTypes.c4ElemCount();

    // If they don't have the same count of elements, then it has changed
    if (lscfgComp.m_colFldTypes.c4ElemCount() != c4Count)
        return kCIDLib::False;

    // There are the same counts, so compare fields
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If not the same type, then obviously not
        if (m_colFldTypes[c4Index]->clsIsA() != lscfgComp.m_colFldTypes[c4Index]->clsIsA())
            return kCIDLib::False;

        if (!m_colFldTypes[c4Index]->bIsEqual(*lscfgComp.m_colFldTypes[c4Index]))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQLogicSrvCfg::operator!=(const TCQLogicSrvCfg& lscfgComp) const
{
    return !operator==(lscfgComp);
}



// ---------------------------------------------------------------------------
//  TCQLogicSrvCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Adds a new field to the existing list of fields
tCIDLib::TVoid TCQLogicSrvCfg::AddField(TCQLSrvFldType* const pclsftToAdd)
{
    TJanitor<TCQLSrvFldType> janAdd(pclsftToAdd);
    m_colFldTypes.CheckIsFull(kCQLogicSh::c4MaxFields, L"Logic Server fields");

    // Make sure it's unique
    #if CID_DEBUG_ON
    tCIDLib::TCard4 c4Index;
    CIDAssert
    (
        !bFindByName(pclsftToAdd->strFldName(), c4Index)
        , L"The field name is not unique"
    );
    #endif

    m_colFldTypes.Add(janAdd.pobjOrphan());
}


// Sees if the indicated name already exists in our field list
tCIDLib::TBoolean
TCQLogicSrvCfg::bFindByName(const   TString&            strName
                            ,       tCIDLib::TCard4&    c4Index) const
{
    const tCIDLib::TCard4 c4Count = m_colFldTypes.c4ElemCount();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colFldTypes[c4Index]->strFldName() == strName)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


// See if we a field with the passed id
tCIDLib::TBoolean
TCQLogicSrvCfg::bFindById(  const   tCIDLib::TCard4     c4ToFind
                            ,       tCIDLib::TCard4&    c4Index) const
{
    const tCIDLib::TCard4 c4Count = m_colFldTypes.c4ElemCount();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colFldTypes[c4Index]->c4FldId() == c4ToFind)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Just a convenience to return the count of fields
tCIDLib::TCard4 TCQLogicSrvCfg::c4FldCount() const
{
    return m_colFldTypes.c4ElemCount();
}


//
//  Read/write access to the list of configured fields. THe client side driver
//  needs writeable access to modify the list, and the server side will store
//  some info in them as well.
//
const tCQLogicSh::TFldList& TCQLogicSrvCfg::colFldTypes() const
{
    return m_colFldTypes;
}

tCQLogicSh::TFldList& TCQLogicSrvCfg::colFldTypes()
{
    return m_colFldTypes;
}


// Delete a field from our list
tCIDLib::TVoid TCQLogicSrvCfg::DeleteField(const tCIDLib::TCard4 c4At)
{
    m_colFldTypes.RemoveAt(c4At);
}


// A convenience to access specific fields by index
TCQLSrvFldType* TCQLogicSrvCfg::pclsftAt(const tCIDLib::TCard4 c4Index)
{
    return m_colFldTypes[c4Index];
}

const TCQLSrvFldType*
TCQLogicSrvCfg::pclsftAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colFldTypes[c4Index];
}


// Called to ask us to reset our config to default values
tCIDLib::TVoid TCQLogicSrvCfg::Reset()
{
    // Clear our list of fields
    m_colFldTypes.RemoveAll();
}



// ---------------------------------------------------------------------------
//  TCQLogicSrvCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQLogicSrvCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_DrvConfig::c1FmtVersion))
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

    // Clean up any current lists
    m_colFldTypes.RemoveAll();

    // Now get the field config count and its XORd version and check them
    tCIDLib::TCard4 c4Count;
    tCIDLib::TCard4 c4XORCount;
    strmToReadFrom >> c4Count >> c4XORCount;
    if (c4Count != (c4XORCount ^ kCIDLib::c4MaxCard))
    {
        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcCfg_BadFldCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // We need to polymorphically stream in all the field config objects
    tCIDLib::TCard4 c4TestInd;
    TCQLSrvFldType* pclsftCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Stream in the sanity check index and confirm it
        strmToReadFrom >> c4TestInd;
        if (c4TestInd != c4Index)
        {
            facCQLogicSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLShErrs::errcCfg_BadFldIndex
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        ::PolymorphicRead(pclsftCur, strmToReadFrom);
        m_colFldTypes.Add(pclsftCur);
    }

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQLogicSrvCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_DrvConfig::c1FmtVersion;

    // Stream out the field config count and a XOR'd version of it
    const tCIDLib::TCard4 c4Count = m_colFldTypes.c4ElemCount();
    strmToWriteTo << c4Count << tCIDLib::TCard4(c4Count ^ kCIDLib::c4MaxCard);

    //
    //  Now polymorphically stream them all out. For sanity we stream out the
    //  index before each one.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToWriteTo << c4Index;
        ::PolymorphicWrite(m_colFldTypes[c4Index], strmToWriteTo);
    }

    // And it shoudl end with an end object marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


