//
// FILE NAME: CQCKit_FldIOPacket.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2001
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
//  This file implements the field I/O packet class.
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
RTTIDecls(TFldIOData,TObject)
RTTIDecls(TFldIOPacket,TObject)
RTTIDecls(TFldIOPacket::TDriverInfo,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_FldIOPacket
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TFldIOData
//  PREFIX: fiod
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFldIOData: Constructors and destructor
// ---------------------------------------------------------------------------
TFldIOData::TFldIOData() :

    m_c4FieldId(0)
    , m_c4SerialNum(0)
    , m_eFldType(tCQCKit::EFldTypes::Count)
{
}

TFldIOData::TFldIOData( const   tCIDLib::TCard4     c4FieldId
                        , const tCQCKit::EFldTypes  eFldType) :

    m_c4FieldId(c4FieldId)
    , m_c4SerialNum(0)
    , m_eFldType(eFldType)
{
}

TFldIOData::~TFldIOData()
{
}


// ---------------------------------------------------------------------------
//  TFldIOData: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TFldIOData::c4FieldId() const
{
    return m_c4FieldId;
}

tCIDLib::TCard4 TFldIOData::c4FieldId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FieldId = c4ToSet;
    return m_c4FieldId;
}


tCIDLib::TCard4 TFldIOData::c4SerialNum() const
{
    return m_c4SerialNum;
}

tCIDLib::TCard4 TFldIOData::c4SerialNum(const tCIDLib::TCard4 c4ToSet)
{
    m_c4SerialNum = c4ToSet;
    return m_c4SerialNum;
}


tCQCKit::EFldTypes TFldIOData::eFldType() const
{
    return m_eFldType;
}

tCQCKit::EFldTypes TFldIOData::eFldType(const tCQCKit::EFldTypes eToSet)
{
    m_eFldType = eToSet;
    return m_eFldType;
}


// ---------------------------------------------------------------------------
//  TFldIOData: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TFldIOData::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCKit_FldIOPacket::c2FmtVersion)
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

    strmToReadFrom  >> m_c4FieldId
                    >> m_c4SerialNum
                    >> m_eFldType;


    // Check the start object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TFldIOData::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_FldIOPacket::c2FmtVersion
                    << m_c4FieldId
                    << m_c4SerialNum
                    << m_eFldType
                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TFldIOPacket::TDriverInfo
//  PREFIX: drvi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFldIOPacket::TDriverInfo: Constructors and Destructors
// ---------------------------------------------------------------------------
TFldIOPacket::TDriverInfo::TDriverInfo() :

    m_c4DriverId(kCIDLib::c4MaxCard)
    , m_c4FldListId(0)
{
}

TFldIOPacket::TDriverInfo::TDriverInfo( const   tCIDLib::TCard4 c4DriverId
                                        , const tCIDLib::TCard4 c4FldListId) :
    m_c4DriverId(c4DriverId)
    , m_c4FldListId(c4FldListId)
{
}

TFldIOPacket::TDriverInfo::~TDriverInfo()
{
}


// ---------------------------------------------------------------------------
//  TFldIOPacket::TDriverInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4
TFldIOPacket::TDriverInfo::c4AddField(  const   tCIDLib::TCard4     c4FieldId
                                        , const tCQCKit::EFldTypes  eType)
{
    // Make sure its not already preset
    const tCIDLib::TCard4 c4Count = m_colFldIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colFldIds[c4Index].c4FieldId() == c4FieldId)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFIOP_DupFieldId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(c4FieldId)
                , TCardinal(m_c4DriverId)
            );
        }
    }

    // Its not there already, so add it and return the new index
    m_colFldIds.objPlace(c4FieldId, eType);
    return c4Count;
}


tCIDLib::TCard4
TFldIOPacket::TDriverInfo::c4AddOrFindField(const   tCIDLib::TCard4     c4FieldId
                                            , const tCQCKit::EFldTypes  eType)

{
    // See if its already present
    const tCIDLib::TCard4 c4Count = m_colFldIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        //
        //  If so, then, return this index. But make sure this one has the
        //  same type as what they were going to give if new.
        //
        TFldIOData& fiodCur = m_colFldIds[c4Index];
        if (fiodCur.c4FieldId() == c4FieldId)
        {
            if (fiodCur.eFldType() != eType)
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcFIOP_NewTypeWrong
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::TypeMatch
                    , tCQCKit::strXlatEFldTypes(eType)
                    , tCQCKit::strXlatEFldTypes(fiodCur.eFldType())
                );
            }
            return c4Index;
        }
    }

    // Its not there already, so add it and return the new index
    m_colFldIds.objPlace(c4FieldId, eType);
    return c4Count;
}


tCIDLib::TCard4 TFldIOPacket::TDriverInfo::c4DriverId() const
{
    return m_c4DriverId;
}


tCIDLib::TCard4 TFldIOPacket::TDriverInfo::c4FieldCount() const
{
    return m_colFldIds.c4ElemCount();
}


tCIDLib::TCard4 TFldIOPacket::TDriverInfo::c4FieldListId() const
{
    return m_c4FldListId;
}


tCIDLib::TCard4
TFldIOPacket::TDriverInfo::c4FieldIndexForId(const tCIDLib::TCard4 c4FieldId) const
{
    const tCIDLib::TCard4 c4Count = m_colFldIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colFldIds[c4Index].c4FieldId() == c4FieldId)
            return c4Index;
    }
    return kCIDLib::c4MaxCard;
}


const TFldIOData&
TFldIOPacket::TDriverInfo::fiodAt(const tCIDLib::TCard4 c4Index) const
{
    if (c4Index > m_colFldIds.c4ElemCount())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_FldIndexNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4Index)
        );
    }
    return m_colFldIds[c4Index];
}

TFldIOData& TFldIOPacket::TDriverInfo::fiodAt(const tCIDLib::TCard4 c4Index)
{
    if (c4Index > m_colFldIds.c4ElemCount())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_FldIndexNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4Index)
        );
    }
    return m_colFldIds[c4Index];
}



const TFldIOData&
TFldIOPacket::TDriverInfo::fiodById(const tCIDLib::TCard4 c4FieldId) const
{
    const tCIDLib::TCard4 c4Index = c4FieldIndexForId(c4FieldId);
    if (c4Index == kCIDLib::c4MaxCard)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_FldIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4FieldId)
        );
    }
    return m_colFldIds[c4Index];
}

TFldIOData&
TFldIOPacket::TDriverInfo::fiodById(const tCIDLib::TCard4 c4FieldId)
{
    const tCIDLib::TCard4 c4Index = c4FieldIndexForId(c4FieldId);
    if (c4Index == kCIDLib::c4MaxCard)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_FldIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4FieldId)
        );
    }
    return m_colFldIds[c4Index];
}


tCIDLib::TVoid
TFldIOPacket::TDriverInfo::RemoveFieldAt(const tCIDLib::TCard4 c4At)
{
    m_colFldIds.RemoveAt(c4At);
}


tCIDLib::TVoid
TFldIOPacket::TDriverInfo::Reset(const  tCIDLib::TCard4 c4DriverId
                                , const tCIDLib::TCard4 c4FldListId)
{
    m_c4DriverId = c4DriverId;
    m_c4FldListId = c4FldListId;
    m_colFldIds.RemoveAll();
}



// ---------------------------------------------------------------------------
//  TFldIOPacket::TDriverInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TFldIOPacket::TDriverInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCKit_FldIOPacket::c2FmtVersion)
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

    strmToReadFrom  >> m_c4DriverId
                    >> m_c4FldListId
                    >> m_colFldIds;

    // Check the start object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
TFldIOPacket::TDriverInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_FldIOPacket::c2FmtVersion
                    << m_c4DriverId
                    << m_c4FldListId
                    << m_colFldIds
                    << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//   CLASS: TFldIOPacket
//  PREFIX: fiop
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFldIOPacket: Constructors and destructor
// ---------------------------------------------------------------------------
TFldIOPacket::TFldIOPacket() :

    m_c4DriverListId(0)
    , m_c4PacketId(0)
{
}

TFldIOPacket::TFldIOPacket(const tCIDLib::TCard4 c4DriverListId) :

    m_c4DriverListId(c4DriverListId)
    , m_c4PacketId(0)
{
}

TFldIOPacket::~TFldIOPacket()
{
}


// ---------------------------------------------------------------------------
//  TFldIOPacket: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TFldIOPacket::bIsEmpty() const
{
    // If no driver entries, then we are empty
    return m_colDriverList.bIsEmpty();
}


tCIDLib::TBoolean TFldIOPacket::bNoFields() const
{
    tCIDLib::TBoolean bRet = kCIDLib::True;

    // If no drivers, then clearly no fields
    const tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If any driver isn't empty, then we have fields
        if (m_colDriverList[c4Index].c4FieldCount())
        {
            bRet = kCIDLib::False;
            break;
        }
    }
    return bRet;
}


tCIDLib::TCard4 TFldIOPacket::c4AddDriver(  const   tCIDLib::TCard4 c4DriverId
                                            , const tCIDLib::TCard4 c4FldListId)
{
    // See if we have this driver already
    const tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colDriverList[c4Index].c4DriverId() == c4DriverId)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFIOP_DupDriverId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TCardinal(c4DriverId)
            );
        }
    }

    // Its not there already, so add it
    m_colDriverList.objPlace(c4DriverId, c4FldListId);

    // Bump our packet id to indicate we've been changed
    m_c4PacketId++;

    // Its index will be the previous count
    return c4Count;
}


tCIDLib::TCard4 TFldIOPacket::c4AddField(const  tCIDLib::TCard4     c4DriverId
                                        , const tCIDLib::TCard4     c4FieldId
                                        , const tCQCKit::EFldTypes  eType)

{
    // Find this driver
    const tCIDLib::TCard4 c4DrvCount = m_colDriverList.c4ElemCount();
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4DrvCount; c4Index++)
    {
        if (m_colDriverList[c4Index].c4DriverId() == c4DriverId)
            break;
    }

    // If we didn't find the driver, then throw
    if (c4Index == c4DrvCount)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_DriverIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4DriverId)
        );
    }

    // Bump our packet id to indicate we've been changed
    m_c4PacketId++;

    // Try to add it to this driver. It will reject if a duplicate
    return m_colDriverList[c4Index].c4AddField(c4FieldId, eType);
}


tCIDLib::TCard4
TFldIOPacket::c4AddOrFindField( const   tCIDLib::TCard4     c4DriverId
                                , const tCIDLib::TCard4     c4FldListId
                                , const tCIDLib::TCard4     c4FieldId
                                , const tCQCKit::EFldTypes  eType)
{
    // Find this driver
    const tCIDLib::TCard4 c4DrvCount = m_colDriverList.c4ElemCount();
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4DrvCount; c4Index++)
    {
        if (m_colDriverList[c4Index].c4DriverId() == c4DriverId)
            break;
    }

    // If we didn't find the driver, then add it
    if (c4Index == c4DrvCount)
    {
        c4Index = c4AddDriver(c4DriverId, c4FldListId);
        m_c4PacketId++;
    }

    // And ask the existing or new driver to add this field
    return m_colDriverList[c4Index].c4AddOrFindField(c4FieldId, eType);
}


tCIDLib::TCard4 TFldIOPacket::c4DriverCount() const
{
    return m_colDriverList.c4ElemCount();
}


tCIDLib::TCard4
TFldIOPacket::c4DriverIdAt(const tCIDLib::TCard4 c4DrvIndex) const
{
    return m_colDriverList[c4DrvIndex].c4DriverId();
}


tCIDLib::TCard4
TFldIOPacket::c4DriverIndexForId(const tCIDLib::TCard4 c4DriverId) const
{
    const tCIDLib::TCard4 c4DrvCount = m_colDriverList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DrvCount; c4Index++)
    {
        if (m_colDriverList[c4Index].c4DriverId() == c4DriverId)
            return c4Index;
    }
    return kCIDLib::c4MaxCard;
}


tCIDLib::TCard4 TFldIOPacket::c4DriverListId() const
{
    return m_c4DriverListId;
}


tCIDLib::TCard4
TFldIOPacket::c4FieldCount(const tCIDLib::TCard4 c4DriverId) const
{
    // Find this driver
    const tCIDLib::TCard4 c4DrvCount = m_colDriverList.c4ElemCount();
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4DrvCount; c4Index++)
    {
        if (m_colDriverList[c4Index].c4DriverId() == c4DriverId)
            break;
    }

    // If we didn't find the driver, then throw
    if (c4Index == c4DrvCount)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_DriverIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4DriverId)
        );
    }
    return m_colDriverList[c4Index].c4FieldCount();
}


tCIDLib::TCard4
TFldIOPacket::c4FieldCountAt(const tCIDLib::TCard4 c4DrvIndex) const
{
    return m_colDriverList[c4DrvIndex].c4FieldCount();
}


tCIDLib::TCard4
TFldIOPacket::c4FieldIndexForId(const   tCIDLib::TCard4 c4DrvIndex
                                , const tCIDLib::TCard4 c4FldId) const
{
    return m_colDriverList[c4DrvIndex].c4FieldIndexForId(c4FldId);
}


tCIDLib::TCard4
TFldIOPacket::c4FieldListIdAt(const tCIDLib::TCard4 c4DrvIndex) const
{
    return m_colDriverList[c4DrvIndex].c4FieldListId();
}


tCIDLib::TCard4 TFldIOPacket::c4PacketId() const
{
    return m_c4PacketId;
}


const TFldIOData&
TFldIOPacket::fiodAt(const  tCIDLib::TCard4 c4DrvIndex
                    , const tCIDLib::TCard4 c4FldIndex) const
{
    return m_colDriverList[c4DrvIndex].fiodAt(c4FldIndex);
}

TFldIOData&
TFldIOPacket::fiodAt(const  tCIDLib::TCard4 c4DrvIndex
                    , const tCIDLib::TCard4 c4FldIndex)
{
    return m_colDriverList[c4DrvIndex].fiodAt(c4FldIndex);
}


const TFldIOData&
TFldIOPacket::fiodById( const   tCIDLib::TCard4 c4DriverId
                        , const tCIDLib::TCard4 c4FieldId) const
{
    const tCIDLib::TCard4 c4DrvIndex = c4DriverIndexForId(c4DriverId);
    if (c4DrvIndex == kCIDLib::c4MaxCard)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_DriverIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4DriverId)
        );
    }
    return m_colDriverList[c4DrvIndex].fiodById(c4FieldId);
}

TFldIOData&
TFldIOPacket::fiodById( const   tCIDLib::TCard4 c4DriverId
                        , const tCIDLib::TCard4 c4FieldId)
{
    const tCIDLib::TCard4 c4DrvIndex = c4DriverIndexForId(c4DriverId);
    if (c4DrvIndex == kCIDLib::c4MaxCard)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_DriverIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4DriverId)
        );
    }
    return m_colDriverList[c4DrvIndex].fiodById(c4FieldId);
}


tCIDLib::TVoid TFldIOPacket::RemoveDriverAt(const tCIDLib::TCard4 c4At)
{
    m_colDriverList.RemoveAt(c4At);
}


tCIDLib::TVoid
TFldIOPacket::RemoveFieldAt(const   tCIDLib::TCard4 c4DrvAt
                            , const tCIDLib::TCard4 c4FldAt)
{
    m_colDriverList[c4DrvAt].RemoveFieldAt(c4FldAt);
}


tCIDLib::TVoid TFldIOPacket::Reset(const tCIDLib::TCard4 c4DriverListId)
{
    m_c4DriverListId = c4DriverListId;
    m_colDriverList.RemoveAll();

    // Bump our packet id to indicate we've been changed
    m_c4PacketId++;
}


tCIDLib::TVoid
TFldIOPacket::SetSerialNum( const   tCIDLib::TCard4 c4DriverId
                            , const tCIDLib::TCard4 c4FieldId
                            , const tCIDLib::TCard4 c4SerialNum)
{
    const tCIDLib::TCard4 c4DrvIndex = c4DriverIndexForId(c4DriverId);
    if (c4DrvIndex == kCIDLib::c4MaxCard)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_DriverIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4DriverId)
        );
    }
    m_colDriverList[c4DrvIndex].fiodById(c4FieldId).c4SerialNum(c4SerialNum);
}


// ---------------------------------------------------------------------------
//  TFldIOPacket: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TFldIOPacket::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCKit_FldIOPacket::c2FmtVersion)
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

    strmToReadFrom  >> m_c4DriverListId
                    >> m_c4PacketId
                    >> m_colDriverList;

    // Check the start object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TFldIOPacket::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_FldIOPacket::c2FmtVersion
                    << m_c4DriverListId
                    << m_c4PacketId
                    << m_colDriverList
                    << tCIDLib::EStreamMarkers::EndObject;
}

