//
// FILE NAME: CQCPollEng_Engine2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/16/2002
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
//  This file just handles some overflow from the main engine implementation
//  file, which is kind of large. This one provides the implementation of
//  the nested classes that represent servers, drivers, and fields.
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
#include    "CQCPollEng_.hpp"


// Local types and constants
namespace
{
    namespace CQCPollEng_Engine2
    {
        // -----------------------------------------------------------------------
        //  Some minimum times or time thresholds we use.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TEncodedTime enctReconnThreshold = (kCIDLib::enctOneSecond * 3);
        constexpr tCIDLib::TEncodedTime enctPruneCheckInterval = (kCIDLib::enctOneSecond * 15);
        constexpr tCIDLib::TEncodedTime enctStateChangeThreshold = kCIDLib::enctOneMinute;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCPollEngine::TFldItem
//  PREFIX: fldi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPollEngine::TFldItem: Public, static methods
// ---------------------------------------------------------------------------
const TString&
TCQCPollEngine::TFldItem::strKey(const TCQCPollEngine::TFldItem& fldiSrc)
{
    return fldiSrc.m_flddInfo.strName();
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine::TFldItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCPollEngine::TFldItem::TFldItem( const   tCIDLib::TCard4     c4DriverId
                                    , const TCQCFldDef&         flddData
                                    , const TCQCPollEngFldLink& flnkThis) :
    m_bOnPollList(kCIDLib::False)
    , m_c4FmtSerialNum(0)
    , m_enctLastAccess(TTime::enctNow())
    , m_flddInfo(flddData)
    , m_flnkThis(flnkThis)
    , m_pfvCurrent(nullptr)
{
    // Ask the kit facility to make us an appropriate value object for this
    m_pfvCurrent = facCQCKit().pfvMakeNewFor
    (
        c4DriverId, flddData.c4Id(), flddData.eType()
    );
}

TCQCPollEngine::TFldItem::~TFldItem()
{
    delete m_pfvCurrent;
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine::TFldItem: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCPollEngine::TFldItem::bInError() const
{
    // Delegate to the value object
    return m_pfvCurrent->bInError();
}


// Get or set the on poll list flag
tCIDLib::TBoolean TCQCPollEngine::TFldItem::bOnPollList() const
{
    return m_bOnPollList;
}

tCIDLib::TBoolean
TCQCPollEngine::TFldItem::bOnPollList(const tCIDLib::TBoolean bToSet)
{
    m_bOnPollList = bToSet;
    return m_bOnPollList;
}


tCIDLib::TBoolean
TCQCPollEngine::TFldItem::bPolledSince(const tCIDLib::TEncodedTime enctSince) const
{
    return (m_enctLastAccess >= enctSince);
}


tCIDLib::TCard4 TCQCPollEngine::TFldItem::c4FieldId() const
{
    return m_flddInfo.c4Id();
}


tCIDLib::TCard4 TCQCPollEngine::TFldItem::c4SerialNum() const
{
    return m_pfvCurrent->c4SerialNum();
}


tCIDLib::TEncodedTime TCQCPollEngine::TFldItem::enctLastAccess() const
{
    return m_enctLastAccess;
}


const TCQCFldDef& TCQCPollEngine::TFldItem::flddInfo() const
{
    return m_flddInfo;
}


const TCQCFldValue& TCQCPollEngine::TFldItem::fvCurrent() const
{
    return *m_pfvCurrent;
}

// If you change the value via this, be sure to bump the serial number!
TCQCFldValue& TCQCPollEngine::TFldItem::fvCurrent()
{
    return *m_pfvCurrent;
}


const TCQCPollEngFldLink& TCQCPollEngine::TFldItem::flnkThis() const
{
    return m_flnkThis;
}


tCIDLib::TVoid
TCQCPollEngine::TFldItem::QueryValue(TCQCFldValue& fvToFill)
{
    // Make sure the value to fill is the same type as ours
    TCQCFldValue& fvSrc = fvCurrent();
    #if CID_DEBUG_ON
    if (fvToFill.clsIsA() != fvSrc.clsIsA())
    {
        facCQCPollEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kPEngErrs::errcEng_NotSameType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , fvSrc.clsIsA()
            , fvToFill.clsIsA()
        );
    }
    #endif

    switch(m_flddInfo.eType())
    {
        case tCQCKit::EFldTypes::Boolean :
            static_cast<TCQCBoolFldValue&>(fvToFill)
                        = static_cast<TCQCBoolFldValue&>(fvSrc);
            break;

        case tCQCKit::EFldTypes::Card :
            static_cast<TCQCCardFldValue&>(fvToFill)
                        = static_cast<TCQCCardFldValue&>(fvSrc);
            break;

        case tCQCKit::EFldTypes::Float :
            static_cast<TCQCFloatFldValue&>(fvToFill)
                        = static_cast<TCQCFloatFldValue&>(fvSrc);
            break;

        case tCQCKit::EFldTypes::Int :
            static_cast<TCQCIntFldValue&>(fvToFill)
                        = static_cast<TCQCIntFldValue&>(fvSrc);
            break;

        case tCQCKit::EFldTypes::String :
            static_cast<TCQCStringFldValue&>(fvToFill)
                            = static_cast<TCQCStringFldValue&>(fvSrc);
            break;

        case tCQCKit::EFldTypes::StringList :
            static_cast<TCQCStrListFldValue&>(fvToFill)
                            = static_cast<TCQCStrListFldValue&>(fvSrc);
            break;

        case tCQCKit::EFldTypes::Time :
            static_cast<TCQCTimeFldValue&>(fvToFill)
                            = static_cast<TCQCTimeFldValue&>(fvSrc);
            break;

        default :
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , TInteger(tCIDLib::i4EnumOrd(m_flddInfo.eType()))
                , TString(L"tCQCit::EFldTypes")
            );
            break;
    };
}


const TString& TCQCPollEngine::TFldItem::strName() const
{
    return m_flddInfo.strName();
}


const TString& TCQCPollEngine::TFldItem::strValue() const
{
    // See if we need to update the text value
    if (m_c4FmtSerialNum != m_pfvCurrent->c4SerialNum())
    {
        m_pfvCurrent->Format(m_strValue);
        m_c4FmtSerialNum = m_pfvCurrent->c4SerialNum();
    }
    return m_strValue;
}


//
//  If we get an indication from the driver that the field is in error we need
//  reflex that in our value. If this is an actual change it will bump the value
//  serial number.
//
tCIDLib::TVoid TCQCPollEngine::TFldItem::SetErrorState()
{
    m_pfvCurrent->bInError(kCIDLib::True);
}


//
//  This has to be const because it has to be callable from methods that
//  only look at the field item value, and that has to update the access
//  stamp, which is mutable for this purpose.
//
tCIDLib::TVoid TCQCPollEngine::TFldItem::SetLastAccessStamp() const
{
    m_enctLastAccess = TTime::enctNow();
}


tCIDLib::TVoid
TCQCPollEngine::TFldItem::UpdateServerId(const tCIDLib::TCard4 c4NewSrvId)
{
    // Update our field link with the new server ids
    m_flnkThis.SetServerId(c4NewSrvId);
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCPollEngine::TDrvItem
//  PREFIX: drvi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPollEngine::TDrvItem: Public, static methods
// ---------------------------------------------------------------------------
const TString&
TCQCPollEngine::TDrvItem::strKey(const TCQCPollEngine::TDrvItem& drviSrc)
{
    return drviSrc.m_strMoniker;
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine::TDrvItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCPollEngine::TDrvItem::TDrvItem() :

    m_c4DriverId(kCIDLib::c4MaxCard)
    , m_c4FieldListId(kCIDLib::c4MaxCard)
    , m_colById(tCIDLib::EAdoptOpts::NoAdopt)
    , m_colFields(tCIDLib::EAdoptOpts::Adopt, 29, TStringKeyOps(), &TFldItem::strKey)
    , m_enctLastCheck(0)
    , m_eState(tCQCKit::EDrvStates::NotLoaded)
{
}

TCQCPollEngine::TDrvItem::TDrvItem( const   TString&        strMoniker
                                    , const tCIDLib::TCard4 c4DriverId
                                    , const TString&        strMake
                                    , const TString&        strModel) :
    m_c4DriverId(c4DriverId)
    , m_c4FieldListId(kCIDLib::c4MaxCard)
    , m_colById(tCIDLib::EAdoptOpts::NoAdopt)
    , m_colFields(tCIDLib::EAdoptOpts::Adopt, 29, TStringKeyOps(), &TFldItem::strKey)
    , m_enctLastCheck(0)
    , m_eState(tCQCKit::EDrvStates::NotLoaded)
    , m_strMake(strMake)
    , m_strModel(strModel)
    , m_strMoniker(strMoniker)
{
}

TCQCPollEngine::TDrvItem::~TDrvItem()
{
    try
    {
        m_colFields.RemoveAll();
        m_colById.RemoveAll();
    }

    catch(TError& errToCatch)
    {
        if (facCQCPollEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine::TDrvItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Get a boolean online/offline state for this driver, which just checks the last
//  stored driver state.
//
tCIDLib::TBoolean TCQCPollEngine::TDrvItem::bOnline() const
{
    return (m_eState == tCQCKit::EDrvStates::Connected);
}


// Provide access to our driver and our field list id
tCIDLib::TCard4 TCQCPollEngine::TDrvItem::c4DriverId() const
{
    return m_c4DriverId;
}


tCIDLib::TCard4 TCQCPollEngine::TDrvItem::c4FieldListId() const
{
    return m_c4FieldListId;
}


tCQCKit::EDrvStates TCQCPollEngine::TDrvItem::eState() const
{
    return m_eState;
}


// Allow fields to be looked up by id or name
TCQCPollEngine::TFldItem*
TCQCPollEngine::TDrvItem::pfldiFind(const tCIDLib::TCard4 c4FldId)
{
    if (c4FldId >= m_colById.c4ElemCount())
        return 0;
    return m_colById[c4FldId];
}


const TCQCPollEngine::TFldItem*
TCQCPollEngine::TDrvItem::pfldiFind(const tCIDLib::TCard4 c4FldId) const
{
    if (c4FldId >= m_colById.c4ElemCount())
        return 0;
    return m_colById[c4FldId];
}


TCQCPollEngine::TFldItem*
TCQCPollEngine::TDrvItem::pfldiFind(const TString& strFldName)
{
    return m_colFields.pobjFindByKey(strFldName, kCIDLib::False);
}


//
//  This method re-queries all of our field definitions and gets us back into sync with
//  our driver. The server will call this for all its drivers when it initial is created
//  or any time it losses contact and has to reconnect. The server id and driver list id
//  will change in those cases so he passes those to us so that we can set them in our
//  field link for each field.
//
tCIDLib::TVoid
TCQCPollEngine::TDrvItem::Reload(       TCQCSrvAdminClientProxy& orbcAdmin
                                , const tCIDLib::TCard4          c4ServerId
                                , const tCIDLib::TCard4          c4DriverListId)
{
    try
    {
        // Flush the field list and reset our list id
        m_c4FieldListId = kCIDLib::c4MaxCard;
        m_colFields.RemoveAll();
        m_colById.RemoveAll();

        //
        //  Ask for the field list for this device. If this device happens to
        //  have no fields, which is possible, we will just end up with
        //  none in our list.
        //
        TVector<TCQCFldDef> colFldDefs;
        tCIDLib::TCard4 c4DriverListId;
        const tCIDLib::TCard4 c4Count = orbcAdmin.c4QueryFields
        (
            m_strMoniker
            , m_eState
            , colFldDefs
            , m_c4FieldListId
            , m_c4DriverId
            , c4DriverListId
        );

        //
        //  Set up a link object that we'll use to set the link info on
        //  each new field. Pass a zero for the field id and we'll just
        //  update that id on each round.
        //
        TCQCPollEngFldLink flnkTmp
        (
            c4ServerId, c4DriverListId, m_c4DriverId, m_c4FieldListId, 0
        );

        //
        //  If we got any, then add field items for each one. We add it to
        //  both the by name and by id collections.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCFldDef& flddCur = colFldDefs[c4Index];

            // Set up the link for this one
            flnkTmp.SetFieldId(flddCur.c4Id());

            //
            //  Set up the new item and add it to our 'by name' and 'by id'
            //  views.
            //
            TFldItem* pfldiNew = new TFldItem(m_c4DriverId, flddCur, flnkTmp);
            m_colFields.Add(pfldiNew);
            m_colById.Add(pfldiNew);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        // Clean it back out and rethrow
        m_colFields.RemoveAll();
        m_c4FieldListId = kCIDLib::c4MaxCard;
        throw;
    }
}


// Proivdde access to the make, model, and moniker of this driver
const TString& TCQCPollEngine::TDrvItem::strMake() const
{
    return m_strMake;
}

const TString& TCQCPollEngine::TDrvItem::strModel() const
{
    return m_strModel;
}

const TString& TCQCPollEngine::TDrvItem::strMoniker() const
{
    return m_strMoniker;
}


tCIDLib::TVoid
TCQCPollEngine::TDrvItem::SetState(const tCQCKit::EDrvStates eToSet)
{
    m_eState = eToSet;
}


//
//  Called by the engine to give us the new server id for our server item
//  any time it changes. We just pass it on to all of our fields.
//
tCIDLib::TVoid
TCQCPollEngine::TDrvItem::UpdateServerId(const tCIDLib::TCard4 c4NewSrvId)
{
    const tCIDLib::TCard4 c4FldCount = m_colById.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FldCount; c4Index++)
        m_colById[c4Index]->UpdateServerId(c4NewSrvId);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCPollEngine::TSrvItem
//  PREFIX: srvi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPollEngine::TSrvItem: Public, static methods
// ---------------------------------------------------------------------------

// A key extractor to support creating keyed collections of server items
const TString&
TCQCPollEngine::TSrvItem::strKey(const TCQCPollEngine::TSrvItem& srviSrc)
{
    return srviSrc.m_strNodeName;
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine::TSrvItem: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We start off in offline state and with no client proxy. We start up the
//  poll thread which will start trying to get connected and start polling.
//
//  The thread name we give to our poll thread is a combination of the unique
//  name (within the process) of the owning polling engine, plus the name
//  of the host we are serving. This should always be unique within the
//  process even if multiple polling engines are used. There's never more
//  than one server item for a given server in a given polling engine.
//
//  Set the last prune time to now so that we don't get accidentally pruned
//  by the engine before we can get going.
//
TCQCPollEngine::TSrvItem::TSrvItem(         TCQCPollEngine* const   polleOwner
                                    , const TString&                strEngInstName
                                    , const TString&                strNodeName
                                    , const tCIDLib::TEncodedTime   enctDropInterval) :

    m_c4DriverListId(kCIDLib::c4MaxCard)
    , m_colById(tCIDLib::EAdoptOpts::NoAdopt)
    , m_colDrivers(tCIDLib::EAdoptOpts::Adopt, 29, TStringKeyOps(), &TDrvItem::strKey)
    , m_enctDropInterval(enctDropInterval)
    , m_enctLastStateChange(TTime::enctNow())
    , m_enctLastConn(0)
    , m_enctLastPrune(TTime::enctNow())
    , m_eState(ESrvStates::Offline)
    , m_polleOwner(polleOwner)
    , m_porbcAdmin(0)
    , m_strNodeName(strNodeName)
    , m_thrPoll
      (
        strEngInstName + strNodeName
        , TMemberFunc<TCQCPollEngine::TSrvItem>(this, &TCQCPollEngine::TSrvItem::ePollThread)
      )
{
    // Get an initial server id
    m_c4ServerId = polleOwner->c4NextServerId();

    //
    //  Keep around the name server binding of our remote CQCServer interface,
    //  for use in refreshing the object cache
    //
    m_strBinding = TCQCSrvAdminClientProxy::strAdminScope;
    m_strBinding.Append(kCIDLib::pszTreeSepChar);
    m_strBinding.Append(strNodeName);

    // Start the thread up immediately
    m_thrPoll.Start();
}

TCQCPollEngine::TSrvItem::~TSrvItem()
{
    //
    //  They should have already stopped the thread, but just in case
    //  don't take any chances.
    //
    try
    {
        m_thrPoll.ReqShutdownSync(10000);
        m_thrPoll.eWaitForDeath(4000);
    }

    catch(TError& errToCatch)
    {
        if (facCQCPollEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    try
    {
        m_colDrivers.RemoveAll();
        m_colById.RemoveAll();
    }

    catch(TError& errToCatch)
    {
        if (facCQCPollEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    try
    {
        delete m_porbcAdmin;
    }

    catch(TError& errToCatch)
    {
        if (facCQCPollEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine::TSrvItem: Public, non-virual methods
// ---------------------------------------------------------------------------

//
//  When clients want to access a field, and it's not being polled already,
//  they have to add it to the list of fields to poll. But they can't directly
//  add it to the I/O poll list object because that object is used by the
//  poll thread and we cannot be locked across the remote calls it is used
//  in. So instead we drop them into a list, for the poll thread to grab
//  on its next poll round.
//
//  The caller has to have locked us to sync access to the new fields list.
//  We make sure not to add redundant fields to this list, since it's cheaper
//  to check it here than to make the poll thread have to deal with the
//  same field over and over, which can happen if multiple accesses to the
//  same thread are occuring (common in user interfaces.)
//
//  Once the field is on the poll list it is marked as such and this won't
//  be called for those fields anymore.
//
tCIDLib::TVoid
TCQCPollEngine::TSrvItem::AddNewField(  const   TString& strMoniker
                                        , const TString& strField)
{
    //
    //  Make sure it's not already there first. It's cheaper to do it here
    //  that to put redundant entries into the list for the server item
    //  to have to deal with.
    //
    TKeyValuePair kvalNew(strMoniker, strField);
    const tCIDLib::TCard4 c4Count = m_colNewFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (kvalNew == m_colNewFlds[c4Index])
            return;
    }

    // Not in there yet, so add it
    m_colNewFlds.objAdd(kvalNew);
}


//
//  Sees if we have the indicated driver. The caller must lock us before
//  calling this method.
//
tCIDLib::TBoolean
TCQCPollEngine::TSrvItem::bHasDriver(const TString& strMoniker) const
{
    return m_colDrivers.bKeyExists(strMoniker);
}


//
//  See if it's been at least the state change threshold period of time
//  since our state last changed. This is used  (when the state is less than
//  Ready) to know if we should just drop a server since it's either never
//  connecting or hasn't been accessed in a while. The polling engine's
//  pruning thread calls this.
//
tCIDLib::TBoolean
TCQCPollEngine::TSrvItem::bStateChangeThreshold(const tCIDLib::TEncodedTime enctNow) const
{
    //
    //  If the last state change plus the threshold is less than now, then
    //  the period has expired.
    //
    return (m_enctLastStateChange + CQCPollEng_Engine2::enctStateChangeThreshold)
           < enctNow;
}


//
//  Make our driver list and server ids available. Since these are fundamental
//  values, it's not really necessary to lock before accesing them.
//
tCIDLib::TCard4 TCQCPollEngine::TSrvItem::c4DriverListId() const
{
    return m_c4DriverListId;
}


tCIDLib::TCard4 TCQCPollEngine::TSrvItem::c4ServerId() const
{
    return m_c4ServerId;
}


// The caller should lock this server item before calling this
tCIDLib::TCard4
TCQCPollEngine::TSrvItem::c4ServerId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4ServerId = c4ToSet;
    return m_c4ServerId;
}


//
//  Our polling thread is started up on this method. We just contually
//  try to get connected to our target server and, once connected, we
//  poll it regularly for data.
//
tCIDLib::EExitCodes
TCQCPollEngine::TSrvItem::ePollThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    while (kCIDLib::True)
    {
        // Watch for a shutdown request
        if (thrThis.bCheckShutdownRequest())
            break;

        // Remember the state now before we do this round
        const TCQCPollEngine::ESrvStates eOldState = m_eState;
        try
        {
            switch(m_eState)
            {
                case ESrvStates::Offline :
                {
                    //
                    //  Try to create a client proxy. This guy will
                    //  actually throttle, so we aren't actually trying
                    //  to connect as often as this is called.
                    //
                    MakeNewProxy();
                    break;
                }

                case ESrvStates::NotLoaded :
                {
                    //
                    //  Try to re-load this driver. We are in this
                    //  state if we have the proxy but haven't
                    //  gotten the server/driver/field info yet.
                    //
                    Reload();
                    break;
                }

                case ESrvStates::Idle :
                case ESrvStates::Ready :
                {
                    //
                    //  If in idle state, then we are just polling
                    //  for driver/field changes. If in ready state,
                    //  we are polling live fields.
                    //
                    Poll();
                    break;
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (facCQCPollEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
        }

        //
        //  If our state has moved forward this time, then don't sleep
        //  any. We want to move forward as fast as we can to ready state.
        //  If it has fallen back or stayed the same, then sleep a bit.
        //
        if (m_eState <= eOldState)
            TThread::Sleep(250);
    }
    return tCIDLib::EExitCodes::Normal;
}


// Get or set our state
TCQCPollEngine::ESrvStates TCQCPollEngine::TSrvItem::eState() const
{
    return m_eState;
}

// The caller should lock before calling this
TCQCPollEngine::ESrvStates
TCQCPollEngine::TSrvItem::eState(const TCQCPollEngine::ESrvStates eToSet)
{
    //
    //  If it's different, then remember the last change time and store it.
    //  If it was idle or greater and now is less than idle, get us a new
    //  server id in order to invalidate any links we've given out.
    //
    if (m_eState != eToSet)
    {
        if ((m_eState >= TCQCPollEngine::ESrvStates::Idle)
        &&  (eToSet < TCQCPollEngine::ESrvStates::Idle))
        {
            UpdateServerId(m_polleOwner->c4NextServerId());
        }

        m_enctLastStateChange = TTime::enctNow();
        m_eState = eToSet;
    }
    return m_eState;
}


//
//  Before this is called, the node name must be set, either via the ctor,
//  or after the fact if the default ctor was used.
//
//  Note that we don't have to do any locking here since at this point we
//  are marked as not having any field info and so no client code will be
//  event attemping to access anything.
//
tCIDLib::TVoid TCQCPollEngine::TSrvItem::MakeNewProxy()
{
    //
    //  See if at least the reconnect threshold time interval has elapsed
    //  since the last time we tried. If not, do nothing.
    //
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    if (enctNow < m_enctLastConn + CQCPollEng_Engine2::enctReconnThreshold)
        return;

    // Ok we are going to try again, so update the last time stamp
    m_enctLastConn = enctNow;
    try
    {
        // Clean up all our data
        Reset();

        // Ok, lets try to get an admin client proxy for this server
        m_porbcAdmin = facCQCKit().porbcCQCSrvAdminProxy(m_strNodeName);

        //
        //  If we got it, then set our state to indicate we've got the
        //  proxy, but we aren't loaded yet.
        //
        if (m_porbcAdmin)
            eState(ESrvStates::NotLoaded);
    }

    catch(TError& errToCatch)
    {
        if (facCQCPollEng().bLogWarnings() && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


// Just a convenience around the mutex
tCIDLib::TVoid TCQCPollEngine::TSrvItem::Lock()
{
    m_mtxSync.Lock();
}


// Provide access to our mutex so we can be locked
TMutex& TCQCPollEngine::TSrvItem::mtxSync()
{
    return m_mtxSync;
}


//
//  These next methods are for looking up fields, drivers, or servers from
//  our list. For the field lookups, they can tell us to to force the field
//  into the I/O list (make it start being polled) if it's not already.
//
//  These all require that caller lock this server item so that our poll
//  thread cannot be updating the data at the same time.
//
TCQCPollEngine::TFldItem*
TCQCPollEngine::TSrvItem::pfldiFind(        TDrvItem&           drviOwner
                                    , const TString&            strField
                                    , const tCIDLib::TBoolean   bForceOnPoll)
{
    TFldItem* pfldiRet = drviOwner.pfldiFind(strField);
    if (!pfldiRet)
        return 0;

    //
    //  If they want us to force onto the poll list, add it to the new
    //  fields list if not already being polled.
    //
    if (bForceOnPoll && !pfldiRet->bOnPollList())
        m_colNewFlds.objAdd(TKeyValuePair(drviOwner.strMoniker(), strField));
    return pfldiRet;
}

TCQCPollEngine::TFldItem*
TCQCPollEngine::TSrvItem::pfldiFind(        TDrvItem&           drviOwner
                                    , const tCIDLib::TCard4     c4FieldId
                                    , const tCIDLib::TBoolean   bForceOnPoll)
{
    TFldItem* pfldiRet = drviOwner.pfldiFind(c4FieldId);
    if (!pfldiRet)
        return 0;

    //
    //  If they want us to force onto the poll list, add it to the new
    //  fields list if not already being polled.
    //
    if (bForceOnPoll && !pfldiRet->bOnPollList())
        m_colNewFlds.objAdd(TKeyValuePair(drviOwner.strMoniker(), pfldiRet->strName()));
    return pfldiRet;
}


// A const field item lookup, which can't do any forcing
const TCQCPollEngine::TFldItem*
TCQCPollEngine::TSrvItem::pfldiFind(const   TDrvItem&           drviOwner
                                    , const tCIDLib::TCard4     c4FieldId) const
{
    return drviOwner.pfldiFind(c4FieldId);
}


// Look up a given driver in our list by its id
TCQCPollEngine::TDrvItem*
TCQCPollEngine::TSrvItem::pdrviFind(const tCIDLib::TCard4 c4DriverId)
{
    return m_colById[c4DriverId];
}

const TCQCPollEngine::TDrvItem*
TCQCPollEngine::TSrvItem::pdrviFind(const tCIDLib::TCard4 c4DriverId) const
{
    return m_colById[c4DriverId];
}


// Look up a driver in our list by its moniker
TCQCPollEngine::TDrvItem*
TCQCPollEngine::TSrvItem::pdrviFind(const TString& strMoniker)
{
    return m_colDrivers.pobjFindByKey(strMoniker, kCIDLib::False);
}


//
//  This is called periodically by our polling thread, if we are in the
//  state that indicates we have active fields to poll or if we are in
//  idle state and just want to watch for driver/field changes.
//
tCIDLib::TVoid TCQCPollEngine::TSrvItem::Poll()
{
    //
    //  If not in the ready or idle state, he shouldn't have called us, but
    //  just in case, watch for it.
    //
    #if CID_DEBUG_ON
    if ((eState() != ESrvStates::Idle) && (eState() != ESrvStates::Ready))
    {
        TPopUp::PopUpMsg
        (
            CID_FILE
            , CID_LINE
            , L"CQC Intf Engine"
            , L"Charmed Quark Systems, Ltd"
            , L"bPoll() called when not in ready state"
            , 0
        );
    }

    // Make sure the client proxy has been set
    if (!m_porbcAdmin)
    {
        TPopUp::PopUpMsg
        (
            CID_FILE
            , CID_LINE
            , L"CQC Intf Engine"
            , L"Charmed Quark Systems, Ltd"
            , L"bPoll() called when admin proxy not created"
            , 0
        );
    }
    #endif

    //
    //  Add any fields to the poll list that have been queued up since we last polled. He
    //  will lock as required.
    //
    LoadNewFields();

    //
    //  If the pruning period has passed, then see if any fields have not
    //  been accessed in the drop interval. If so, remove them from the
    //  list. If we end up empty, then mark us idle.
    //
    //  Note that any new fields loaded above will have had their access
    //  time stamp set to now so they won't get pruned right back out.
    //
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    if (enctNow > (m_enctLastPrune + CQCPollEng_Engine2::enctPruneCheckInterval))
    {
        //
        //  Lock while we do this, since we'll be updating the I/O list
        //  so we can't have clients trying to access the field info.
        //
        TLocker lockrSync(&m_mtxSync);

        tCIDLib::TBoolean bRemovedSome = kCIDLib::False;

        // Set the time we test against for dropping fallow fields
        const tCIDLib::TEncodedTime enctDropTest(enctNow - m_enctDropInterval);

        // Update our last prune time stamp
        m_enctLastPrune = enctNow;

        //
        //  Run through the I/O poll list and look up each field. If it its last access
        //  time is before the last prune, then toss it.
        //
        //  NOTE:   We DO NOT remove drivers from the list, even if we remove
        //          all the fields, so that we'll see field list changes
        //          during polling and get an 'out of sync' exception that
        //          will make us update.
        //
        tCIDLib::TCard4 c4DCount = m_fiopPoll.c4DriverCount();
        for (tCIDLib::TCard4 c4DInd = 0; c4DInd < c4DCount; c4DInd++)
        {
            // Find our driver corresponding to this driver index in the I/o
            TDrvItem* pdrviCur = pdrviFind(m_fiopPoll.c4DriverIdAt(c4DInd));

            // And get the number of fields in the I/O for this index
            tCIDLib::TCard4 c4FCount = m_fiopPoll.c4FieldCountAt(c4DInd);
            tCIDLib::TCard4 c4FInd = 0;
            while (c4FInd < c4FCount)
            {
                // Find our druver for the current driver/field index
                const TFldIOData& fiodCur = m_fiopPoll.fiodAt(c4DInd, c4FInd);
                TFldItem* pfldiCur = pdrviCur->pfldiFind(fiodCur.c4FieldId());

                //  If it's out of date, drop it.
                if (pfldiCur->enctLastAccess() < enctDropTest)
                {
                    // Remove this one and bump down the count
                    m_fiopPoll.RemoveFieldAt(c4DInd, c4FInd);
                    c4FCount--;

                    // Mark it as no longer on the list
                    pfldiCur->bOnPollList(kCIDLib::False);

                    // Remember we removed some
                    bRemovedSome = kCIDLib::True;
                }
                 else
                {
                    // We kept this one so move up the index
                    c4FInd++;
                }
            }
        }

        //
        //  If no fields left, then we mark ourself as in idle state, and
        //  set our 'idle since' stamp, which lets the engine remove us
        //  if we stay idle for a period of time.
        //
        if (m_fiopPoll.bNoFields())
        {
            eState(ESrvStates::Idle);
            return;
        }
    }

    //
    //  Ok, now poll the fields in our field I/O list and store away any
    //  new data or error states.
    //
    try
    {
        // Ask the server for any changes
        tCIDLib::TCard4 c4Bytes;
        m_porbcAdmin->ReadFields(m_fiopPoll, c4Bytes, m_mbufPoll);

        //
        //  Now loop through the results and pull out any results. Check
        //  first for the start object marker and format version.
        //
        TBinMBufInStream strmSrc(&m_mbufPoll, c4Bytes);

        strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);
        tCIDLib::TCard1 c1FmtVersion;
        strmSrc >> c1FmtVersion;

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
                , TString("TCQCPollEngine::TSrvItem")
            );
        }

        // OK, now we ahve to lock while we store the field info
        TLocker lockrSync(&m_mtxSync);

        tCIDLib::TCard1 c1Marker;
        tCIDLib::TCard1 c1State;
        tCIDLib::TCard1 c1Type;
        tCIDLib::TCard2 c2Id;
        tCIDLib::TCard4 c4SerialNum;
        TDrvItem*       pdrviCur = nullptr;
        while (!strmSrc.bEndOfStream())
        {
            // Each new chunk should start with a start object marker
            strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

            // And next we should see a chunk type
            strmSrc >> c1Type;

            // According to the type, do the correct thing
            if (c1Type == kCQCKit::c1FldType_DriverOffline)
            {
                //
                //  Get the id, look up the driver. If we were online before,
                //  then mark it offline.
                //
                strmSrc >> c2Id >> c1State;
                pdrviCur = m_colById[c2Id];
                pdrviCur->SetState(tCQCKit::EDrvStates(c1State));

                //
                //  Now set the driver pointer back to zero, since there
                //  should be no field chunks for this driver.
                //
                pdrviCur = 0;
            }
             else if (c1Type == kCQCKit::c1FldType_DriverOnline)
            {
                //
                //  Get the id, look up the driver, and mark it online if not
                //  already. Leave the driver pointer set, since it will now
                //  become the target driver for subsequent field chunks.
                //
                strmSrc >> c2Id;
                pdrviCur = m_colById[c2Id];
                pdrviCur->SetState(tCQCKit::EDrvStates::Connected);
            }
             else if (c1Type == kCQCKit::c1FldType_Field)
            {
                // We must have seen a previous server
                CIDAssert(pdrviCur != 0, L"Did not see a server before fields");

                // Get the id of the field out and the marker
                strmSrc >> c2Id >> c1Marker;

                // Look up this field
                TFldItem* pfldiCur = pdrviCur->pfldiFind(c2Id);
                if (!pfldiCur)
                {
                    facCQCKit().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcFIOP_FldIdNotFound
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , TCardinal(c2Id)
                    );
                }

                //
                //  If the marker indicates no change, then we don't have
                //  to do anything. If it indicates new data, we have to
                //  store this info.
                //
                if (c1Marker == kCQCKit::c1FldData_Changed)
                {
                    //
                    //  Get the serial number out and update the I/O packet
                    //  for this field to hold the new serial number.
                    //
                    strmSrc >> c4SerialNum;
                    m_fiopPoll.SetSerialNum(pdrviCur->c4DriverId(), c2Id, c4SerialNum);

                    //
                    //  Get the value out, according to the type of field,
                    //  and store the value into the field item.
                    //
                    TCQCFldValue& fvSet = pfldiCur->fvCurrent();
                    switch(pfldiCur->flddInfo().eType())
                    {
                        case tCQCKit::EFldTypes::Boolean :
                        {
                            tCIDLib::TBoolean bValue;
                            strmSrc >> bValue;
                            static_cast<TCQCBoolFldValue&>(fvSet).bSetValue(bValue);
                            break;
                        }

                        case tCQCKit::EFldTypes::Card :
                        {
                            tCIDLib::TCard4 c4Value;
                            strmSrc >> c4Value;
                            static_cast<TCQCCardFldValue&>(fvSet).bSetValue(c4Value);
                            break;
                        }

                        case tCQCKit::EFldTypes::Float :
                        {
                            tCIDLib::TFloat8 f8Value;
                            strmSrc >> f8Value;
                            static_cast<TCQCFloatFldValue&>(fvSet).bSetValue(f8Value);
                            break;
                        }

                        case tCQCKit::EFldTypes::Int :
                        {
                            tCIDLib::TInt4 i4Value;
                            strmSrc >> i4Value;
                            static_cast<TCQCIntFldValue&>(fvSet).bSetValue(i4Value);
                            break;
                        }

                        case tCQCKit::EFldTypes::String :
                        {
                            strmSrc >> m_strTmpPoll;
                            static_cast<TCQCStringFldValue&>(fvSet).bSetValue(m_strTmpPoll);
                            break;
                        }

                        case tCQCKit::EFldTypes::StringList :
                        {
                            strmSrc >> m_colTmpPoll;
                            static_cast<TCQCStrListFldValue&>(fvSet).bSetValue(m_colTmpPoll);
                            break;
                        }

                        case tCQCKit::EFldTypes::Time :
                        {
                            tCIDLib::TCard8 c8Val;
                            strmSrc >> c8Val;
                            static_cast<TCQCTimeFldValue&>(fvSet).bSetValue(c8Val);
                            break;
                        }

                        default :
                            #if CID_DEBUG_ON
                            TPopUp::PopUpMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , L"CQC Intf Engine"
                                , L"Charmed Quark Systems, Ltd"
                                , L"Unknown field type"
                                , 0
                            );
                            #endif
                            break;
                    };
                }
                 else if (c1Marker == kCQCKit::c1FldData_InError)
                {
                    //
                    //  If not already in error state, then set the state. Zero
                    //  the serial number in the field I/O packet to insure we get
                    //  a good read when it comes back online.
                    //
                    if (!pfldiCur->bInError())
                    {
                        pfldiCur->SetErrorState();
                        m_fiopPoll.SetSerialNum(pdrviCur->c4DriverId(), c2Id, 0);
                    }
                }
                 else if (c1Marker != kCQCKit::c1FldData_Unchanged)
                {
                    // Something is wrong
                    // <TBD>
                }
            }
             else
            {
                // Something went wry
                // <TBD>
            }
        }
    }

    catch(const TError& errToCatch)
    {
        //
        //  If its an out of sync error, then we set our state to indicate
        //  that we are unloaded, which will cause us to reload on the
        //  next round.
        //
        //  Else, its a lost of connection or something unexpected, so reset
        //  and start over.
        //
        if (errToCatch.eClass() == tCIDLib::EErrClasses::OutOfSync)
            eState(ESrvStates::NotLoaded);
        else
            Reset();
    }
}



//
//  Goes through the poll list and returns the list of fields currently
//  on the list for this server. We can either replace or append. the caller
//  must lock before calling this.
//
tCIDLib::TVoid TCQCPollEngine::
TSrvItem::QueryActiveFlds(          tCIDLib::TStrCollect&   colToFill
                            , const tCIDLib::TBoolean       bAppend)
{
    if (!bAppend)
        colToFill.RemoveAll();

    TString strFld;
    tCIDLib::TCard4 c4DCount = m_fiopPoll.c4DriverCount();
    for (tCIDLib::TCard4 c4DInd = 0; c4DInd < c4DCount; c4DInd++)
    {
        // Find our driver corresponding to this driver index in the I/o
        TDrvItem* pdrviCur = pdrviFind(m_fiopPoll.c4DriverIdAt(c4DInd));

        // And get the number of fields in the I/O for this index
        tCIDLib::TCard4 c4FCount = m_fiopPoll.c4FieldCountAt(c4DInd);

        for (tCIDLib::TCard4 c4FInd = 0; c4FInd < c4FCount; c4FInd++)
        {
            // Find our druver for the current driver/field index
            const TFldIOData& fiodCur = m_fiopPoll.fiodAt(c4DInd, c4FInd);
            const TFldItem* pfldiCur = pdrviCur->pfldiFind(fiodCur.c4FieldId());

            if (pfldiCur)
            {
                strFld = pdrviCur->strMoniker();
                strFld.Append(L'.');
                strFld.Append(pfldiCur->strName());
                colToFill.objAdd(strFld);
            }
        }
    }
}



//
//  Before this is called, the node name must be set, either via the ctor,
//  or after the fact if the default ctor was used.
//
//  We don't have to lock here since, until we are successful and at the
//  very end we mark ourselves now as idle, any clients will see that we
//  aren't ready and won't try to access anything.
//
tCIDLib::TVoid TCQCPollEngine::TSrvItem::Reload()
{
    //
    //  Make sure the client proxy has been set. If not, then drop back to the
    //  offline state. There is a slight chance this can happen, if we get an
    //  out of sync error because the server is going down and fall back to
    //  unloaded state, but then we get here and it's gone.
    //
    if (!m_porbcAdmin)
    {
        Reset();
        return;
    }

    //
    //  Do an initial reset of the I/O packet to make sure it gets cleaned
    //  out. We'll reset it again below with the real driver list id, if we
    //  can get it. But in case something goes awry before that, we want to
    //  be sure its empty, since this drives the polling system.
    //
    m_fiopPoll.Reset(kCIDLib::c4MaxCard);

    //
    //  Also reset our last prune time as well, since it might otherwise
    //  kick off just after this and do an unnecessary prune cycle.
    //
    m_enctLastPrune = TTime::enctNow();

    //
    //  Reset our driver list id so that we will invalidate any links to
    //  this server, and remove all of the existing drivers.
    //
    m_c4DriverListId = kCIDLib::c4MaxCard;
    m_colDrivers.RemoveAll();
    m_colById.RemoveAll();

    try
    {
        //
        //  Let's use our admin proxy to query our driver list info. First we
        //  get the list of driver monikers and their field ids. We'll then
        //  iterate this list and create a child driver item for each one.
        //
        tCIDLib::TStrList   colMonikers;
        tCIDLib::TCardList  fcolDrvIds;
        tCIDLib::TKVPList   colMakeModel;
        m_c4DriverListId = m_porbcAdmin->c4QueryDriverIdList
        (
            colMonikers, fcolDrvIds, colMakeModel, m_polleOwner->sectUser()
        );

        //
        //  Reset the I/O packet. We know that we won't have any active fields
        //  now, because we are going to freloaded them all. It'll be updated as
        //  GUI widgets relink to it during their polling cycle.
        //
        m_fiopPoll.Reset(m_c4DriverListId);

        const tCIDLib::TCard4   c4Count = colMonikers.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            //
            //  Lets get the fields for this driver. We need to create a
            //  pseudo client proxy for this driver's interface.
            //
            const TString& strCurMoniker = colMonikers[c4Index];
            const TKeyValuePair& kvalCur = colMakeModel[c4Index];

            //
            //  Create a temp driver item and ask it to load. Put a janitor
            //  on it so that it fails, it will get cleaned up. If it survives
            //  we'll orphan it out into our driver list. Tell it what its
            //  moniker is and its driver id, which is the only info we have
            //  at this point. It will figure out the rest.
            //
            TDrvItem* pdrviNew = new TDrvItem
            (
                strCurMoniker
                , fcolDrvIds[c4Index]
                , kvalCur.strKey()
                , kvalCur.strValue()
            );
            TJanitor<TDrvItem> janNew(pdrviNew);
            pdrviNew->Reload
            (
                *m_porbcAdmin, m_c4ServerId, m_c4DriverListId
            );

            //
            //  Ok, we survived, this one, so add it to both our 'by name'
            //  and 'by id' views of the drivers.
            //
            m_colDrivers.Add(janNew.pobjOrphan());
            m_colById.Add(pdrviNew);

            //
            //  Add a driver item to the I/O list, even though we have no
            //  fields being polled yet, so that we'll see any field list
            //  changes during polling.
            //
            m_fiopPoll.c4AddDriver
            (
                pdrviNew->c4DriverId(), pdrviNew->c4FieldListId()
            );
        }

        // It worked so set our state to idle
        eState(ESrvStates::Idle);
    }

    catch(TError& errToCatch)
    {
        // If not one of the common problems, log it
        if ((m_porbcAdmin && !m_porbcAdmin->bCheckForLostConnection(errToCatch))
        &&  (errToCatch.eClass() != tCIDLib::EErrClasses::OutOfSync))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Just panic and reset to force a reconnect
        Reset();
    }
}


tCIDLib::TVoid TCQCPollEngine::TSrvItem::Reset()
{
    m_c4DriverListId = kCIDLib::c4MaxCard;
    m_colById.RemoveAll();
    m_colDrivers.RemoveAll();
    m_colNewFlds.RemoveAll();
    m_fiopPoll.Reset(kCIDLib::c4MaxCard);
    m_enctLastPrune = TTime::enctNow();
    delete m_porbcAdmin;
    m_porbcAdmin = 0;
    m_eState = ESrvStates::Offline;

    // To be completely safe, give us a new server id
    m_c4ServerId = m_polleOwner->c4NextServerId();
}


const TString& TCQCPollEngine::TSrvItem::strNodeName() const
{
    return m_strNodeName;
}


tCIDLib::TVoid TCQCPollEngine::TSrvItem::StartShutdown()
{
    try
    {
        m_thrPoll.ReqShutdownSync(10000);
    }

    catch(TError& errToCatch)
    {
        if (facCQCPollEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


// Just a convenience around the mutex
tCIDLib::TVoid TCQCPollEngine::TSrvItem::Unlock()
{
    m_mtxSync.Unlock();
}


tCIDLib::TVoid
TCQCPollEngine::TSrvItem::UpdateServerId(const tCIDLib::TCard4 c4NewSrvId)
{
    //
    //  Any time a server is removed from the list, we have to reset the
    //  server id and that has to get stored in the server objects and in
    //  the field links of all fields.
    //
    //  So we store our new server id and then tell all our drivers about
    //  the id change so they can update their fields.
    //
    m_c4ServerId = c4NewSrvId;

    const tCIDLib::TCard4 c4DrvCount = m_colById.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DrvCount; c4Index++)
        m_colById[c4Index]->UpdateServerId(c4NewSrvId);
}



tCIDLib::TVoid TCQCPollEngine::TSrvItem::WaitShutdown()
{
    try
    {
        m_thrPoll.eWaitForDeath(4000);
    }

    catch(TError& errToCatch)
    {
        if (facCQCPollEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine::TSrvItem: Private, non-virual methods
// ---------------------------------------------------------------------------

//
//  The poll list will call this before each poll round to see if the clients
//  have added new fields for us to poll. They can't add them directly to
//  our poll I/O list because we are accessing it during the remote poll
//  calls (during which time we cannot be locked.)
//
tCIDLib::TVoid TCQCPollEngine::TSrvItem::LoadNewFields()
{
    TLocker lockrSync(&m_mtxSync);

    tCIDLib::TBoolean bAddedSome = kCIDLib::False;
    const tCIDLib::TCard4 c4Count = m_colNewFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colNewFlds[c4Index];

        // Look up this one
        try
        {
            TDrvItem* pdrviAdd = pdrviFind(kvalCur.strKey());
            if (pdrviAdd)
            {
                TFldItem* pfldiAdd = pdrviAdd->pfldiFind(kvalCur.strValue());
                if (pfldiAdd)
                {
                    m_fiopPoll.c4AddOrFindField
                    (
                        pdrviAdd->c4DriverId()
                        , pdrviAdd->c4FieldListId()
                        , pfldiAdd->c4FieldId()
                        , pfldiAdd->flddInfo().eType()
                    );

                    // Set the last access stamp and on poll list flags
                    pfldiAdd->bOnPollList(kCIDLib::True);
                    pfldiAdd->SetLastAccessStamp();
                    bAddedSome = kCIDLib::True;
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (facCQCPollEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }

    // Flush the list now
    m_colNewFlds.RemoveAll();

    //
    //  If we are in idle state, put us into ready state if we added any
    //  fields, since we now have at least one field to poll.
    //
    if (bAddedSome && (m_eState == ESrvStates::Idle))
        eState(ESrvStates::Ready);
}

