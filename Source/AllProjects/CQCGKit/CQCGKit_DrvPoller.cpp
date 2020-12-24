//
// FILE NAME: CQCGKit_DrvPoller.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/07/2003
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
//  This file implements the driver poller class, typically used by client
//  side drivers.
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
#include    "CQCGKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCDrvPoller,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvPoller
//  PREFIX: dplr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDrvPoller: Destructor
// ---------------------------------------------------------------------------
TCQCDrvPoller::TCQCDrvPoller(const TString& strMoniker) :

    m_c4DriverId(kCIDLib::c4MaxCard)
    , m_c4DriverListId(kCIDLib::c4MaxCard)
    , m_c4FieldListId(kCIDLib::c4MaxCard)
    , m_c4ReadBytes(0)
    , m_colFldStores(tCIDLib::EAdoptOpts::Adopt, 8)
    , m_fcolChanges()
    , m_mbufPollData(4096)
    , m_strMoniker(strMoniker)
    , m_strmRead(&m_mbufPollData)
{
}

TCQCDrvPoller::~TCQCDrvPoller()
{
}


// ---------------------------------------------------------------------------
//  TCQCDrvPoller: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCDrvPoller::bFldChanged(const tCIDLib::TCard4 c4Index) const
{
    return m_fcolChanges[c4Index];
}


tCIDLib::TBoolean TCQCDrvPoller::bInError(const tCIDLib::TCard4 c4Index) const
{
    return m_colFldStores[c4Index]->bInError();
}


tCIDLib::TBoolean TCQCDrvPoller::bPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    //
    //  Do a field read using the field id and serial number info left for
    //  us by the bGetConnectData() method above.
    //
    orbcTarget->ReadDriverFields
    (
        m_c4DriverListId
        , m_c4DriverId
        , m_c4FieldListId
        , m_fcolFldIds
        , m_fcolSerialNums
        , m_c4ReadBytes
        , m_mbufPollData
    );

    //
    //  Call the helper method that extracts the flattened results from the
    //  m_mbufPollData buffer. It returns true if any changed data was
    //  extracted, which is what we want to return.
    //
    return bExtractResults();
}


tCIDLib::TCard4 TCQCDrvPoller::c4IndexFromFldId(const tCIDLib::TCard4 c4FldId) const
{
    const tCIDLib::TCard4 c4Count = m_fcolFldIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_fcolFldIds[c4Index] == c4FldId)
            return c4Index;
    }
    return c4Count;
}


const TCQCFldStoreBool& TCQCDrvPoller::cfsBool(const tCIDLib::TCard4 c4Index) const
{
    return static_cast<const TCQCFldStoreBool&>
    (
        cfsGetStore(c4Index, tCQCKit::EFldTypes::Boolean)
    );
}

const TCQCFldStoreCard& TCQCDrvPoller::cfsCard(const tCIDLib::TCard4 c4Index) const
{
    return static_cast<const TCQCFldStoreCard&>
    (
        cfsGetStore(c4Index, tCQCKit::EFldTypes::Card)
    );
}

const TCQCFldStoreFloat& TCQCDrvPoller::cfsFloat(const tCIDLib::TCard4 c4Index) const
{
    return static_cast<const TCQCFldStoreFloat&>
    (
        cfsGetStore(c4Index, tCQCKit::EFldTypes::Float)
    );
}

const TCQCFldStoreInt& TCQCDrvPoller::cfsInt(const tCIDLib::TCard4 c4Index) const
{
    return static_cast<const TCQCFldStoreInt&>
    (
        cfsGetStore(c4Index, tCQCKit::EFldTypes::Int)
    );
}

const TCQCFldStoreString& TCQCDrvPoller::cfsString(const tCIDLib::TCard4 c4Index) const
{
    return static_cast<const TCQCFldStoreString&>
    (
        cfsGetStore(c4Index, tCQCKit::EFldTypes::String)
    );
}

const TCQCFldStoreSList& TCQCDrvPoller::cfsSList(const tCIDLib::TCard4 c4Index) const
{
    return static_cast<const TCQCFldStoreSList&>
    (
        cfsGetStore(c4Index, tCQCKit::EFldTypes::StringList)
    );
}

const TCQCFldStoreTime& TCQCDrvPoller::cfsTime(const tCIDLib::TCard4 c4Index) const
{
    return static_cast<const TCQCFldStoreTime&>
    (
        cfsGetStore(c4Index, tCQCKit::EFldTypes::Time)
    );
}


tCIDLib::TVoid TCQCDrvPoller::ClearFldChange(const tCIDLib::TCard4 c4Index)
{
    m_fcolChanges[c4Index] = kCIDLib::False;
}


const TCQCFldDef& TCQCDrvPoller::flddAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colFldStores[c4Index]->flddInfo();
}


tCIDLib::TVoid
TCQCDrvPoller::Initialize(          tCQCKit::TCQCSrvProxy&  orbcTarget
                            , const tCIDLib::TStrCollect&   colFldNames)
{
    m_colFldStores.CheckIsFull(kCQCKit::c4MaxDrvFields, L"driver poller field list");

    // Ask for all the info we need to know about our driver
    tCQCKit::EDrvStates eState;
    tCQCKit::TFldDefList colFlds;
    const tCIDLib::TCard4 c4Count = orbcTarget->c4QueryFields
    (
        m_strMoniker, eState, colFlds, m_c4FieldListId, m_c4DriverId, m_c4DriverListId
    );

    //
    //  Now load up the field list with the fields in the order that we
    //  want them to be returned in the results. For each one, put a zero
    //  value into the serial numbers list.
    //
    m_colFldStores.RemoveAll();
    m_fcolChanges.RemoveAll();
    m_fcolFldIds.RemoveAll();
    m_fcolSerialNums.RemoveAll();

    //
    //  Loop through the passed field names and find each one in the list
    //  of queried fields. For each one found, set them up in our data
    //  structures. If the cursor indicates no entries, then we are done.
    //
    TColCursor<TString>* pcursNames = colFldNames.pcursNew();
    TJanitor<TColCursor<TString>> janCurs(pcursNames);
    for (; pcursNames->bIsValid(); pcursNames->bNext())
    {
        // Get the current name
        const TString& strCurName = pcursNames->objRCur();

        tCIDLib::TCard4 c4FldInd = 0;
        TCQCFldDef* pflddCur = nullptr;
        for (; c4FldInd < c4Count; c4FldInd++)
        {
            pflddCur = &colFlds[c4FldInd];
            if (pflddCur->strName() == strCurName)
                break;
        }

        if (c4FldInd == c4Count)
        {
            // We never found it
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownFldName
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , m_strMoniker
                , strCurName
            );
        }

        //
        //  Add this guy's field id to the polling field id and back map
        //  vectors, and add an entry in the serial numbers and changes
        //  lists for it.
        //
        m_fcolFldIds.c4AddElement(pflddCur->c4Id());
        m_fcolSerialNums.c4AddElement(0);
        m_fcolChanges.c4AddElement(kCIDLib::False);

        //
        //  Gen up a storage object for this guy and put into the list.
        //  For our purposes here, we use field limit objects that have
        //  no limits.
        //
        TCQCFldStore* pcfsNew = nullptr;
        switch(pflddCur->eType())
        {
            case tCQCKit::EFldTypes::Boolean :
                pcfsNew = new TCQCFldStoreBool(m_strMoniker, *pflddCur, new TCQCFldBoolLimit);
                break;

            case tCQCKit::EFldTypes::Card :
                pcfsNew = new TCQCFldStoreCard(m_strMoniker, *pflddCur, new TCQCFldCardLimit);
                break;

            case tCQCKit::EFldTypes::Float :
                pcfsNew = new TCQCFldStoreFloat(m_strMoniker, *pflddCur, new TCQCFldFloatLimit);
                break;

            case tCQCKit::EFldTypes::Int :
                pcfsNew = new TCQCFldStoreInt(m_strMoniker, *pflddCur, new TCQCFldIntLimit);
                break;

            case tCQCKit::EFldTypes::String :
                pcfsNew = new TCQCFldStoreString(m_strMoniker, *pflddCur, new TCQCFldStrLimit);
                break;

            case tCQCKit::EFldTypes::StringList :
                pcfsNew = new TCQCFldStoreSList(m_strMoniker, *pflddCur, new TCQCFldStrListLimit);
                break;

            case tCQCKit::EFldTypes::Time :
                pcfsNew = new TCQCFldStoreTime(m_strMoniker, *pflddCur, new TCQCFldTimeLimit);
                break;

            default :
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcFld_UnknownType
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , TInteger(tCIDLib::i4EnumOrd(pflddCur->eType()))
                );
            }
        };
        m_colFldStores.Add(pcfsNew);

    }
}


tCIDLib::TVoid
TCQCDrvPoller::WriteBool(       tCQCKit::TCQCSrvProxy&  orbcTarget
                        , const tCIDLib::TCard4         c4Index
                        , const tCIDLib::TBoolean       bToWrite
                        , const TCQCSecToken&           sectUser)
{
    orbcTarget->WriteBoolFld
    (
        m_c4DriverListId
        , m_c4DriverId
        , m_c4FieldListId
        , m_fcolFldIds[c4Index]
        , bToWrite
        , sectUser
        , tCQCKit::EDrvCmdWaits::DontCare
    );
}


tCIDLib::TVoid
TCQCDrvPoller::WriteCard(       tCQCKit::TCQCSrvProxy&  orbcTarget
                        , const tCIDLib::TCard4         c4Index
                        , const tCIDLib::TCard4         c4ToWrite
                        , const TCQCSecToken&           sectUser)
{
    orbcTarget->WriteCardFld
    (
        m_c4DriverListId
        , m_c4DriverId
        , m_c4FieldListId
        , m_fcolFldIds[c4Index]
        , c4ToWrite
        , sectUser
        , tCQCKit::EDrvCmdWaits::DontCare
    );
}


tCIDLib::TVoid
TCQCDrvPoller::WriteFloat(          tCQCKit::TCQCSrvProxy&  orbcTarget
                            , const tCIDLib::TCard4         c4Index
                            , const tCIDLib::TFloat8        f8ToWrite
                            , const TCQCSecToken&           sectUser)
{
    orbcTarget->WriteFloatFld
    (
        m_c4DriverListId
        , m_c4DriverId
        , m_c4FieldListId
        , m_fcolFldIds[c4Index]
        , f8ToWrite
        , sectUser
        , tCQCKit::EDrvCmdWaits::DontCare
    );
}


tCIDLib::TVoid
TCQCDrvPoller::WriteInt(        tCQCKit::TCQCSrvProxy&  orbcTarget
                        , const tCIDLib::TCard4         c4Index
                        , const tCIDLib::TInt4          i4ToWrite
                        , const TCQCSecToken&           sectUser)
{
    orbcTarget->WriteIntFld
    (
        m_c4DriverListId
        , m_c4DriverId
        , m_c4FieldListId
        , m_fcolFldIds[c4Index]
        , i4ToWrite
        , sectUser
        , tCQCKit::EDrvCmdWaits::DontCare
    );
}


tCIDLib::TVoid
TCQCDrvPoller::WriteString(         tCQCKit::TCQCSrvProxy&  orbcTarget
                            , const tCIDLib::TCard4         c4Index
                            , const TString&                strToWrite
                            , const TCQCSecToken&           sectUser)
{
    orbcTarget->WriteStringFld
    (
        m_c4DriverListId
        , m_c4DriverId
        , m_c4FieldListId
        , m_fcolFldIds[c4Index]
        , strToWrite
        , sectUser
        , tCQCKit::EDrvCmdWaits::DontCare
    );
}


tCIDLib::TVoid
TCQCDrvPoller::WriteSList(          tCQCKit::TCQCSrvProxy&  orbcTarget
                            , const tCIDLib::TCard4         c4Index
                            , const TVector<TString>&       colToWrite
                            , const TCQCSecToken&           sectUser)
{
    orbcTarget->WriteSListFld
    (
        m_c4DriverListId
        , m_c4DriverId
        , m_c4FieldListId
        , m_fcolFldIds[c4Index]
        , colToWrite
        , sectUser
        , tCQCKit::EDrvCmdWaits::DontCare
    );
}


tCIDLib::TVoid
TCQCDrvPoller::WriteTime(       tCQCKit::TCQCSrvProxy&  orbcTarget
                        , const tCIDLib::TCard4         c4Index
                        , const tCIDLib::TCard8&        c8ToWrite
                        , const TCQCSecToken&           sectUser)
{
    orbcTarget->WriteTimeFld
    (
        m_c4DriverListId
        , m_c4DriverId
        , m_c4FieldListId
        , m_fcolFldIds[c4Index]
        , c8ToWrite
        , sectUser
        , tCQCKit::EDrvCmdWaits::DontCare
    );
}



// ---------------------------------------------------------------------------
//  TCQCDrvPoller: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCDrvPoller::bExtractResults()
{
    //
    //  Pull out the data from the formatted buffer. The caller has done
    //  a poll call to fill the buffer. First of all we should get a start
    //  object marker, then a version marker.
    //
    m_strmRead.Reset();
    m_strmRead.SetEndIndex(m_c4ReadBytes);
    m_strmRead.CheckForStartMarker(CID_FILE, CID_LINE);
    tCIDLib::TCard1 c1FmtVersion;
    m_strmRead >> c1FmtVersion;
    if (c1FmtVersion != kCQCKit::c1FldFmtVersion)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFIOP_BadFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsIsA()
        );
    }

    // Remember if we pulled any changed ata
    tCIDLib::TBoolean bChanges = kCIDLib::False;

    // Update our poll data fields with the most recent data
    tCIDLib::TCard1 c1Marker;
    tCIDLib::TCard2 c2Id;
    tCIDLib::TCard4 c4Index = 0;
    tCIDLib::TCard4 c4SerialNum;
    TString strTmp;
    while (!m_strmRead.bEndOfStream())
    {
        //
        //  We should see a frame marker before each one, then the field
        //  id, squeeezd into a TCard2 value, and the marker that indicates
        //  whether new data is available of not.
        //
        m_strmRead.CheckForFrameMarker(CID_FILE, CID_LINE);
        m_strmRead >> c2Id >> c1Marker;

        //
        //  If the field is in error, and wasn't before, then indicate a
        //  change, reset the serial number so that we are sure to get a
        //  new value when it comes back online, and set the error flag on
        //  this one in the store object.
        //
        //  If it's not in error and has changed, then update our serial number
        //  and value and set the changes flag. And make sure that the error
        //  flag is changed.
        //
        if (c1Marker == kCQCKit::c1FldData_InError)
        {
            if (!m_colFldStores[c4Index]->bInError())
            {
                m_fcolChanges[c4Index] = kCIDLib::True;
                m_fcolSerialNums[c4Index] = 0;
                m_colFldStores[c4Index]->bInError(kCIDLib::True);

                // We got a change, so remember that
                bChanges = kCIDLib::True;
            }
        }
         else if (c1Marker == kCQCKit::c1FldData_Changed)
        {
            // Get out the new serial number and update our list
            m_strmRead >> c4SerialNum;
            m_fcolSerialNums[c4Index] = c4SerialNum;

            // Mark this one changed and update the serial number
            m_fcolChanges[c4Index] = kCIDLib::True;

            // Now get out the value
            m_colFldStores[c4Index]->StreamIn(m_strmRead);
            m_colFldStores[c4Index]->bInError(kCIDLib::False);

            // We got a change, so remember that
            bChanges = kCIDLib::True;
        }
        c4Index++;
    }

    return bChanges;
}


const TCQCFldStore&
TCQCDrvPoller::cfsGetStore( const   tCIDLib::TCard4     c4Index
                            , const tCQCKit::EFldTypes  eType) const
{
    const TCQCFldStore* pcfsRet = m_colFldStores[c4Index];
    if (pcfsRet->flddInfo().eType() != eType)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_TypeMismatch
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , m_strMoniker
            , pcfsRet->flddInfo().strName()
            , tCQCKit::strXlatEFldTypes(eType)
            , tCQCKit::strXlatEFldTypes(pcfsRet->flddInfo().eType())
        );
    }
    return *pcfsRet;
}



