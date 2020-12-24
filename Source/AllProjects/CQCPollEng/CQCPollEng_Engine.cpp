//
// FILE NAME: CQCPollEng_Engine.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/25/2008
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
//  This file implements the CQC system's user drawn interface engine. The
//  main body of the class is implemented here, but there is some spillage
//  from this class over into auxillary .cpp files, becuase there is a good
//  bit of code.
//
//  The nested classes that represent fields, drivers, and servers are all
//  in the CQCPollEng_Engine2.cpp overflow file, to keep this file from
//  getting too large.
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


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCPollEngFldLink,TObject)
RTTIDecls(TCQCFldPollInfo,TObject)
RTTIDecls(TCQCPollEngine,TObject)



// ---------------------------------------------------------------------------
//  A little class that we use to keep track of failed driver to server lookups
//  so that we can limit how often the lookup will be done. So that an unloaded
//  driver won't cause constant calls to the name server. We keep a keyed hash
//  set of these guys.
// ---------------------------------------------------------------------------
class TSrvLookupFail : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey(const TSrvLookupFail& slfSrc)
        {
            return slfSrc.m_strMoniker;
        }


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TSrvLookupFail(const TString& strMoniker) :

            m_enctLast(TTime::enctNow())
            , m_strMoniker(strMoniker)
        {
        }

        ~TSrvLookupFail() {}


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        tCIDLib::TEncodedTime   m_enctLast;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSrvLookupFail, TObject)
};

RTTIDecls(TSrvLookupFail, TObject)



// ---------------------------------------------------------------------------
//  Local types and data
// ---------------------------------------------------------------------------
namespace CQCPollEng_Engine
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Normally, only one of these will exist per process. But, if that's
        //  not true, we need to give each instances' poll thread a unique name.
        // -----------------------------------------------------------------------
        TUniqueName unamEngInstance(L"CQCPollEngPollThread%(1)");


        // -----------------------------------------------------------------------
        //  We throttle calls to find the server for a given driver, and this is
        //  the time limit between subsequent checks.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TEncodedTime enctDrvCheckPeriod(kCIDLib::enctOneSecond * 5);


        // -----------------------------------------------------------------------
        //  We drop fields that haven't been accessed in a specific time. This is
        //  the default timeout, that we set initially. But the user can set a
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TEncodedTime enctFldDropInterval(kCIDLib::enctOneSecond * 60);


        // -----------------------------------------------------------------------
        //  The server lookup throttle list. We just keep one for the whole
        //  process, since if a driver isn't loaded, it's not loaded and it's the
        //  same for any interface engines in the same process.
        //
        //  NOTE:   It is created type safe because it is used by multiple interface
        //          engine instances potentially.
        // -----------------------------------------------------------------------
        TKeyedHashSet<TSrvLookupFail,TString,TStringKeyOps> colFailList
        (
             29, TStringKeyOps(), &TSrvLookupFail::strKey
        );
    }
}


// -------------------------------------------------------------------
//   CLASS: TPESrvJanitor
//  PREFIX: jan
//
//  We need a janitor to handle locking server items while we access
//  it's contents.
// -------------------------------------------------------------------
class TPESrvJanitor
{
    public :
        // -----------------------------------------------------------
        //  Constructors and Destructor
        // -----------------------------------------------------------
        TPESrvJanitor();

        ~TPESrvJanitor();


        // -----------------------------------------------------------
        //  Public, non-virtual methods
        // -----------------------------------------------------------
        tCIDLib::TBoolean bFindDriver
        (
                    TCQCPollEngine&         polleSrc
            , const TString&                strMoniker
        );

        tCIDLib::TBoolean bFindField
        (
                    TCQCPollEngine&         polleSrc
            , const TString&                strMoniker
            , const TString&                strField
            , const tCIDLib::TBoolean       bCreateIfNew
            , const tCIDLib::TBoolean       bForceOnPoll
        );

        tCIDLib::TBoolean bFindField
        (
                    TCQCPollEngine&         polleSrc
            , const TCQCPollEngFldLink&     flnkToFind
            , const tCIDLib::TBoolean       bForceOnPoll
        );

        tCIDLib::TBoolean bFindSrvByMoniker
        (
                    TCQCPollEngine&         polleSrc
            , const TString&                strMoniker
            , const tCIDLib::TBoolean       bCreateIfNew
            ,       tCIDLib::TBoolean&      bCreated
        );


        // -----------------------------------------------------------
        //  Data members
        // -----------------------------------------------------------
        TCQCPollEngine::TDrvItem*   m_pdrviTar;
        TCQCPollEngine::TFldItem*   m_pfldiTar;
        TCQCPollEngine::TSrvItem*   m_psrviTar;
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCPollEngFldLink
//  PREFIX: flnk
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPollEngFldLink: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCPollEngFldLink::TCQCPollEngFldLink() :

    m_c4DriverId(kCIDLib::c4MaxCard)
    , m_c4DriverListId(kCIDLib::c4MaxCard)
    , m_c4FieldId(kCIDLib::c4MaxCard)
    , m_c4FieldListId(kCIDLib::c4MaxCard)
    , m_c4ServerId(kCIDLib::c4MaxCard)
{
}

TCQCPollEngFldLink::TCQCPollEngFldLink( const   tCIDLib::TCard4 c4ServerId
                                        , const tCIDLib::TCard4 c4DriverListId
                                        , const tCIDLib::TCard4 c4DriverId
                                        , const tCIDLib::TCard4 c4FieldListId
                                        , const tCIDLib::TCard4 c4FieldId) :
    m_c4DriverId(c4DriverId)
    , m_c4DriverListId(c4DriverListId)
    , m_c4FieldId(c4FieldId)
    , m_c4FieldListId(c4FieldListId)
    , m_c4ServerId(c4ServerId)
{
}

TCQCPollEngFldLink::~TCQCPollEngFldLink()
{
}


// ---------------------------------------------------------------------------
//  TCQCPollEngFldLink: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCPollEngFldLink::operator==(const TCQCPollEngFldLink& flnkSrc) const
{
    if (this == &flnkSrc)
        return kCIDLib::True;

    return  (m_c4DriverId == flnkSrc.m_c4DriverId)
            && (m_c4DriverListId == flnkSrc.m_c4DriverListId)
            && (m_c4FieldId == flnkSrc.m_c4FieldId)
            && (m_c4FieldListId == flnkSrc.m_c4FieldListId)
            && (m_c4ServerId == flnkSrc.m_c4ServerId);
}


tCIDLib::TBoolean
TCQCPollEngFldLink::operator!=(const TCQCPollEngFldLink& flnkSrc) const
{
    return !operator==(flnkSrc);
}


// ---------------------------------------------------------------------------
//  TCQCPollEngFldLink: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Make sure that all ids are invalid
tCIDLib::TVoid TCQCPollEngFldLink::Reset()
{
    m_c4DriverId     = kCIDLib::c4MaxCard;
    m_c4DriverListId = kCIDLib::c4MaxCard;
    m_c4FieldId      = kCIDLib::c4MaxCard;
    m_c4FieldListId  = kCIDLib::c4MaxCard;
    m_c4ServerId     = kCIDLib::c4MaxCard;
}


// Reset this object with a whole new set of ids
tCIDLib::TVoid TCQCPollEngFldLink::Set( const   tCIDLib::TCard4 c4ServerId
                                        , const tCIDLib::TCard4 c4DriverListId
                                        , const tCIDLib::TCard4 c4DriverId
                                        , const tCIDLib::TCard4 c4FieldListId
                                        , const tCIDLib::TCard4 c4FieldId)
{
    m_c4DriverId     = c4DriverId;
    m_c4DriverListId = c4DriverListId;
    m_c4FieldId      = c4FieldId;
    m_c4FieldListId  = c4FieldListId;
    m_c4ServerId     = c4ServerId;
}


//
//  This is an optimization, since each TDrvItem in the polling engine will
//  create one of these per field, and the only thing that changes is the
//  field id, so he can just create a local with the other ids set, and then
//  just update the field id and set it on each new field.
//
tCIDLib::TVoid TCQCPollEngFldLink::SetFieldId(const tCIDLib::TCard4 c4FieldId)
{
    // Just update the field id and rehash
    m_c4FieldId = c4FieldId;
}


//
//  The engine will occasionally prune inactive servers out of it's poll
//  list, in which case the server id will have to be updated if it gets
//  added back later.
//
tCIDLib::TVoid
TCQCPollEngFldLink::SetServerId(const tCIDLib::TCard4 c4ServerId)
{
    m_c4ServerId = c4ServerId;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldPollInfo
//  PREFIX: cfpe
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldPollInfo: Constructors and Destructor
// ---------------------------------------------------------------------------

TCQCFldPollInfo::TCQCFldPollInfo() :

    m_c4SerialNum(0)
    , m_eLastState(tCQCPollEng::EFldStates::WaitReg)
    , m_pfvCurrent(nullptr)
{
}

TCQCFldPollInfo::TCQCFldPollInfo(const  TString&    strDevice
                                , const TString&    strField) :
    m_c4SerialNum(0)
    , m_eLastState(tCQCPollEng::EFldStates::WaitReg)
    , m_pfvCurrent(nullptr)
    , m_strField(strField)
    , m_strMoniker(strDevice)
{
    // Build up the full name
    m_strFullFldName = strDevice;
    m_strFullFldName.Append(kCIDLib::chPeriod);
    m_strFullFldName.Append(strField);
}

TCQCFldPollInfo::TCQCFldPollInfo(const TCQCFldPollInfo& cfpiSrc) :

    m_c4SerialNum(0)
    , m_eLastState(tCQCPollEng::EFldStates::WaitReg)
    , m_flddAssoc(cfpiSrc.m_flddAssoc)
    , m_pfvCurrent(nullptr)
    , m_strField(cfpiSrc.m_strField)
    , m_strFullFldName(cfpiSrc.m_strFullFldName)
    , m_strMake(cfpiSrc.m_strMake)
    , m_strModel(cfpiSrc.m_strModel)
    , m_strMoniker(cfpiSrc.m_strMoniker)
{
    // Note that the serial number, state, and link info go back to defaults
    m_flnkAssoc.Reset();

    //
    //  Allocate the appropriate storage. If the source has a value, then
    //  we know it's been set up, so we can also. Else, leave it null for
    //  now.
    //
    if (cfpiSrc.m_flddAssoc.eType() != tCQCKit::EFldTypes::Count)
        m_pfvCurrent = facCQCKit().pfvMakeNewFor(0, 0, m_flddAssoc.eType());
}

TCQCFldPollInfo::~TCQCFldPollInfo()
{
    // Clean up the field storage if it got allocated
    delete m_pfvCurrent;
}


// ---------------------------------------------------------------------------
//  TCQCFldPollInfo: Public operators
// ---------------------------------------------------------------------------
TCQCFldPollInfo& TCQCFldPollInfo::operator=(const TCQCFldPollInfo& cfpiSrc)
{
    if (this != &cfpiSrc)
    {
        m_flddAssoc = cfpiSrc.m_flddAssoc;

        // If we have a current value and it's not the new type, delete it
        if (m_pfvCurrent && (m_pfvCurrent->eFldType() != m_flddAssoc.eType()))
        {
            delete m_pfvCurrent;
            m_pfvCurrent = nullptr;
        }

        // If we don't have a value and the new type is set, then create one
        if (!m_pfvCurrent && (m_flddAssoc.eType() != tCQCKit::EFldTypes::Count))
            m_pfvCurrent = facCQCKit().pfvMakeNewFor(0, 0, m_flddAssoc.eType());

        // And store any other stuff
        m_strField      = cfpiSrc.m_strField;
        m_strFullFldName= cfpiSrc.m_strFullFldName;
        m_strMake       = cfpiSrc.m_strMake;
        m_strModel      = cfpiSrc.m_strModel;
        m_strMoniker    = cfpiSrc.m_strMoniker;

        // Note that the state, serial, and link inof go back to default status again
        m_c4SerialNum = 0;
        m_eLastState = tCQCPollEng::EFldStates::WaitReg;
        m_flnkAssoc.Reset();
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCFldPollInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Indicate whether we have a field assocation set on us
tCIDLib::TBoolean TCQCFldPollInfo::bHasField() const
{
    return !m_strMoniker.bIsEmpty() && !m_strField.bIsEmpty();
}


// A conveient shortcut to find out if we have a good value
tCIDLib::TBoolean TCQCFldPollInfo::bHasGoodValue() const
{
    return (m_eLastState == tCQCPollEng::EFldStates::Ready);
}


//
//  Tries to get our field registered with the polling engine, if it exists
//  currently (as indicated by the field cache.) If not, it puts us into
//  waiting for registration state so we'll keep trying. It returns true
//  if we got registered.
//
tCIDLib::TBoolean
TCQCFldPollInfo::bRegister(         TCQCPollEngine& polleToUse
                            , const TCQCFldCache&   cfcData)
{
    //
    //  Watch for a pathological issue. If the moniker or field is empty, which
    //  shouldn't happen but if it does, mark this one as invalid, so it will just
    //  be left alone and never get good data.
    //
    if (m_strMoniker.bIsEmpty() || m_strField.bIsEmpty())
    {
        m_eLastState = tCQCPollEng::EFldStates::Invalid;
        return kCIDLib::False;
    }

    // If it exists, try to do the field registration, else say still waiting
    if (cfcData.bFldExists(m_strMoniker, m_strField))
        DoFieldReg(polleToUse);
    else
        m_eLastState = tCQCPollEng::EFldStates::WaitReg;

    // Return true if we got it registered
    return (m_eLastState > tCQCPollEng::EFldStates::WaitReg);
}


//
//  The client must call this periodically to keep the field up to date. THey
//  have to call it more than once per minute to keep the field registered in
//  the poll engine, else he'll drop it.
//
//  We return true if the state or value of the field changed. The caller
//  should get the state out first, then call this method. We return true
//  if either the state or the value changed. So if we return true, they
//  should look at how the state changed and what that means to them.
//
//  If the return is true and the new state is ready, then we've either just
//  come online and got a value or we were on line and got a new value, so the
//  caller should react accordingly.
//
tCIDLib::TBoolean TCQCFldPollInfo::bUpdateValue(TCQCPollEngine& polleToUse)
{
    //
    //  If invalid, just do nothing. We decided during registration our info
    //  was never going to work.
    //
    if (m_eLastState == tCQCPollEng::EFldStates::Invalid)
        return kCIDLib::False;

    //
    //  If it was rejected, then we need to see if any change has occurred
    //  that could have changed the field definition, in which case we'll
    //  given them another chance.
    //
    if (m_eLastState == tCQCPollEng::EFldStates::Rejected)
    {
        //
        //  If the link info has changed since we registered the field, then
        //  the field definition could have changed so we want to try to
        //  register again and get new info. But we don't want to just
        //  do both in one shot here. We need the client to see this change,
        //  so return. We'll start re-registering on the next round.
        //
        if (polleToUse.bCheckLinkChange(*this))
        {
            m_eLastState = tCQCPollEng::EFldStates::WaitReg;
            m_flnkAssoc.Reset();
            return kCIDLib::True;
        }

        // Nothing we can do, so stay in rejected mode
        return kCIDLib::False;
    }

    //
    //  If we've not gotten our field registered yet, then keep trying to
    //  do that. Note we handled invalid explicitly above, so though it's less
    //  than WaitReg, it'll never get us here.
    //
    if (m_eLastState <= tCQCPollEng::EFldStates::WaitReg)
        DoFieldReg(polleToUse);

    //
    //  And now if we are ready to get data, then do that. So if our state
    //  is field error (which is set intially upon getting registered or if
    //  we lose the value) or greater, then try to get a value.
    //
    //  We remember the original state and if we get a new value, so that at
    //  the end we can indicate whether something changed.
    //
    tCIDLib::TBoolean       bNewValue = kCIDLib::False;
    tCQCPollEng::EFldStates eOrgState = m_eLastState;

    if (m_eLastState >= tCQCPollEng::EFldStates::FldError)
    {
        try
        {
            // Make sure we've allocated the value object
            CIDAssert(m_pfvCurrent != nullptr, L"The field has not been set yet");

            const tCQCKit::EValQRes eRes = polleToUse.eQueryValue
            (
                m_strMoniker, m_strField, m_c4SerialNum, *m_pfvCurrent, m_flnkAssoc
            );

            if (eRes == tCQCKit::EValQRes::NewValue)
            {
                // Make sure our state is ready now if not already
                m_eLastState = tCQCPollEng::EFldStates::Ready;

                // We've got a new value
                bNewValue = kCIDLib::True;
            }
             else if (eRes == tCQCKit::EValQRes::NoChange)
            {
                // We better be in ready state already
                CIDAssert
                (
                    m_eLastState == tCQCPollEng::EFldStates::Ready
                    , L"The field was not in ready state"
                );
            }
             else if ((eRes == tCQCKit::EValQRes::NotFound)
                  ||  (eRes == tCQCKit::EValQRes::Reconfig))
            {
                //
                //  We've lost the field or it's been reconfigured, so go
                //  back to wait reg. It may not have actually gone away,
                //  but a reconfiguration of the driver's fields may have
                //  occured, which changed the link info, and possibly the
                //  field definition so we have to recycle.
                //
                m_eLastState = tCQCPollEng::EFldStates::WaitReg;
                m_flnkAssoc.Reset();
            }
             else if (eRes == tCQCKit::EValQRes::InError)
            {
                // The individual field is in error
                m_c4SerialNum = 0;
                m_eLastState = tCQCPollEng::EFldStates::FldError;
            }
             else
            {
                // We got some unknown query result, so that's bad
                if (facCQCPollEng().bLogWarnings())
                {
                    facCQCPollEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Field '%(1)' got an unknown query result, resetting it"
                        , tCIDLib::ESeverities::Info
                        , tCIDLib::EErrClasses::AppStatus
                        , m_strFullFldName
                    );
                }

                m_c4SerialNum = 0;
                m_eLastState = tCQCPollEng::EFldStates::WaitReg;
                m_flnkAssoc.Reset();
            }
        }

        catch(TError& errToCatch)
        {
            if (facCQCPollEng().bLogFailures())
            {
                facCQCPollEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Failure querying value of field '%(1)', resetting it"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strFullFldName
                );
            }

            // Something went awry, so go back wait value state
            m_c4SerialNum = 0;
            m_eLastState = tCQCPollEng::EFldStates::WaitValue;

            if (facCQCPollEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        if ((eOrgState != m_eLastState) && facCQCPollEng().bLogInfo())
        {
            facCQCPollEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Field '%(1)' changed status to %(2)"
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , m_strFullFldName
                , tCQCPollEng::strLoadEFldStates(m_eLastState)
            );
        }
    }

    // If we got a new value or the state changed, return true
    return bNewValue || (eOrgState != m_eLastState);
}


// Give the client access to the current state
tCQCPollEng::EFldStates TCQCFldPollInfo::eState() const
{
    return m_eLastState;
}


// Provide read only access to our field definition
const TCQCFldDef& TCQCFldPollInfo::flddAssoc() const
{
    return m_flddAssoc;
}


// Provide read only access to our field link info
const TCQCPollEngFldLink& TCQCFldPollInfo::flnkAssoc() const
{
    return m_flnkAssoc;
}


//
//  Give the client access to the current value. It's their responsibilty
//  to know what the type is and cast it appropriately in order to get the
//  value out.
//
const TCQCFldValue& TCQCFldPollInfo::fvCurrent() const
{
    // Make sure the field has been set
    CIDAssert(m_pfvCurrent != nullptr, L"The field storage has not been created");
    return *m_pfvCurrent;
}


// Format out the field info to XML
tCIDLib::TVoid
TCQCFldPollInfo::FormatToXML(TTextOutStream& strmTarget) const
{
    strmTarget  << L"\n<FldInfo Mon='" << m_strMoniker << L"' Fld='"
                << m_strField << L"'/>";
}


//
//  Client code can call this to put a field into reject mode. Until we
//  see some change that could have modified the field definition, we'll
//  stay in rejected mode.
//
tCIDLib::TVoid TCQCFldPollInfo::RejectField()
{
    CIDAssert(m_pfvCurrent != nullptr, L"The field has not been set yet");
    m_eLastState = tCQCPollEng::EFldStates::Rejected;
}


// Clear the field association
tCIDLib::TVoid TCQCFldPollInfo::Reset()
{
    delete m_pfvCurrent;
    m_pfvCurrent = nullptr;

    m_c4SerialNum = 0;
    m_eLastState = tCQCPollEng::EFldStates::WaitReg;
    m_flddAssoc.Reset();
    m_flnkAssoc.Reset();
    m_strMake.Clear();
    m_strModel.Clear();
    m_strMoniker.Clear();
    m_strField.Clear();
    m_strFullFldName.Clear();
}


// Provide access to our configured moniker, make, model, and field name
const TString& TCQCFldPollInfo::strFieldName() const
{
    return m_strField;
}

const TString& TCQCFldPollInfo::strFullFldName() const
{
    return m_strFullFldName;
}

const TString& TCQCFldPollInfo::strMake() const
{
    return m_strMake;
}

const TString& TCQCFldPollInfo::strModel() const
{
    return m_strModel;
}

const TString& TCQCFldPollInfo::strMoniker() const
{
    return m_strMoniker;
}


//
//  If they don't set the field during the ctor, they need to subsequently
//  call this to set it. It must be set before attempting to use this object.
//
tCIDLib::TVoid
TCQCFldPollInfo::SetField(const TString& strMoniker, const TString& strField)
{
    CIDAssert
    (
        !strMoniker.bIsEmpty() && !strField.bIsEmpty()
        , L"Empty moniker or field passed to polling engine"
    );

    // Cleanup the value object until we know the type again
    delete m_pfvCurrent;
    m_pfvCurrent = nullptr;

    m_flddAssoc.Reset();
    m_flnkAssoc.Reset();
    m_strField = strField;
    m_strMake.Clear();
    m_strModel.Clear();
    m_strMoniker = strMoniker;

    // Note that the state and serial go back to default status again
    m_c4SerialNum = 0;
    m_eLastState = tCQCPollEng::EFldStates::WaitReg;

    // Build up the full name
    m_strFullFldName = m_strMoniker;
    m_strFullFldName.Append(kCIDLib::chPeriod);
    m_strFullFldName.Append(m_strField);
}


//
//  During some types of configuration import operations, drivers of the same
//  sort are found in the target system, and a moniker replacement is done.
//  This is to support that kind of operation. In theory, that would mean
//  that the field type is the same, but we don't assume that.
//
tCIDLib::TVoid TCQCFldPollInfo::SetMoniker(const TString& strMoniker)
{
    CIDAssert(!strMoniker.bIsEmpty(), L"Empty moniker passed to polling engine");

    // Cleanup the value object until we know the type again
    delete m_pfvCurrent;
    m_pfvCurrent = nullptr;

    m_flddAssoc.Reset();
    m_flnkAssoc.Reset();
    m_strMake.Clear();
    m_strModel.Clear();
    m_strMoniker = strMoniker;

    //Note that the state and serial go back to default status again. But we check for
    m_c4SerialNum = 0;
    m_eLastState = tCQCPollEng::EFldStates::WaitReg;

    // Build up the full name
    m_strFullFldName = m_strMoniker;
    m_strFullFldName.Append(kCIDLib::chPeriod);
    m_strFullFldName.Append(m_strField);
}



// ---------------------------------------------------------------------------
//  TCQCFldPollInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This functionality is needed both in our evaluation code and in the
//  field registration pass, so it's broken out here. The caller passes in
//  the poll engine, and we attempt the registration and set an appropriate
//  expression state and field state. We'll keep getting called here until
//  we get our field registered.
//
tCIDLib::TVoid TCQCFldPollInfo::DoFieldReg(TCQCPollEngine& polleToUse)
{
    const tCQCPollEng::EFldRegRes eRegRes = polleToUse.eRegisterField
    (
        m_strMoniker
        , m_strField
        , tCQCKit::EReqAccess::MReadCWrite
        , m_flddAssoc
        , m_flnkAssoc
        , m_strMake
        , m_strModel
    );

    tCQCPollEng::EFldStates eState = m_eLastState;
    if (eRegRes == tCQCPollEng::EFldRegRes::OK)
    {
        // OK, we found it, so allocate the storage and wait for a value
        eState = tCQCPollEng::EFldStates::WaitValue;

        //
        //  If we have a current field value and it's not the same type as
        //  our field is now, then delete it.
        //
        if (m_pfvCurrent && (m_pfvCurrent->eFldType() != m_flddAssoc.eType()))
        {
            delete m_pfvCurrent;
            m_pfvCurrent = nullptr;
        }

        // If we don't have storage, create one for our new type
        if (!m_pfvCurrent)
            m_pfvCurrent = facCQCKit().pfvMakeNewFor(0, 0, m_flddAssoc.eType());
    }
     else if (eRegRes == tCQCPollEng::EFldRegRes::NotFound)
    {
        // It's not there, so just stay in wait reg state
        eState = tCQCPollEng::EFldStates::WaitReg;
    }
     else
    {
        //
        //  Otherwise, since we can't get the name parsing rejecting
        //  because we pass separate moniker/field, then it has to be a
        //  change in the field access, so not much we can do but put it
        //  into rejected state.
        //
        eState = tCQCPollEng::EFldStates::Rejected;
    }

    if (eState != m_eLastState)
    {
        m_eLastState = eState;
        if (facCQCPollEng().bLogInfo())
        {
            facCQCPollEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Field '%(1).%(2)' changes status to %(3)"
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , m_strMoniker
                , m_strField
                , tCQCPollEng::strLoadEFldStates(m_eLastState)
            );
        }
    }

    // Make sure the serial number gets reset
    m_c4SerialNum = 0;
}




// ---------------------------------------------------------------------------
//   CLASS: TPESrvJanitor
//  PREFIX: jan
//
//  We need a janitor to handle locking server items while we access its
//  contents. This is called by the polling engine but in the context of
//  incoming client calls. It must lock the main list mutex before using
//  this janitor.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TPESrvJanitor: Constructors and Destructor
// ---------------------------------------------------------------------------
TPESrvJanitor::TPESrvJanitor() :

    m_pdrviTar(nullptr)
    , m_pfldiTar(nullptr)
    , m_psrviTar(nullptr)
{
}

TPESrvJanitor::~TPESrvJanitor()
{
    // If we got the server, then unlock it
    if (m_psrviTar)
        m_psrviTar->Unlock();
}


// ---------------------------------------------------------------------------
//  TPESrvJanitor: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Find a driver and the server that hosts it, via the moniker. If the
//  server isn't in our list but exists, it'll be added to the list. If
//  it works, then we set the driver and server pointers and zero the field
//  pointer and return with the serer locked.
//
tCIDLib::TBoolean
TPESrvJanitor::bFindDriver(         TCQCPollEngine& polleSrc
                            , const TString&        strMoniker)
{
    // Assume the worst
    m_psrviTar = nullptr;
    m_pdrviTar = nullptr;
    m_pfldiTar = nullptr;

    //
    //  Find the server with this moniker, adding it to the list if not
    //  already there.
    //
    tCIDLib::TBoolean bCreated;
    TCQCPollEngine::TSrvItem* psrviTmp = polleSrc.psrviFindSrvByMoniker
    (
        strMoniker, kCIDLib::True, bCreated
    );

    //
    //  If we didn't find and could not add the server, or if we created
    //  the server just now (so our driver couldn't be available yet), or
    //  if the server isn't up and going yet so it couldn't have the info
    //  yet, just return false.
    //
    if (!psrviTmp
    ||  bCreated
    ||  (psrviTmp->eState() < TCQCPollEngine::ESrvStates::Idle))
    {
        return kCIDLib::False;
    }

    // Temporarily lock it while we search for the driver
    TLocker lockrSrv(&psrviTmp->mtxSync());
    TCQCPollEngine::TDrvItem* pdrviTmp = psrviTmp->pdrviFind(strMoniker);

    // Didn't find it so let the mutex unlock and we failed
    if (!pdrviTmp)
        return kCIDLib::False;

    //
    //  We found it, so store the values and orphan the mutex out of
    //  the mutex locker so that it stays locked.
    //
    m_psrviTar = psrviTmp;
    m_pdrviTar = pdrviTmp;
    m_pfldiTar = nullptr;
    lockrSrv.Orphan();

    return kCIDLib::True;
}


//
//  Find a particular field, the driver that contains it, and the server
//  that hosts the driver, based on the moniker and field name.
//
tCIDLib::TBoolean
TPESrvJanitor::bFindField(          TCQCPollEngine&     polleSrc
                            , const TString&            strMoniker
                            , const TString&            strField
                            , const tCIDLib::TBoolean   bCreateIfNew
                            , const tCIDLib::TBoolean   bForceOnPoll)
{
    // Assume the worst
    m_psrviTar = nullptr;
    m_pdrviTar = nullptr;
    m_pfldiTar = nullptr;

    //
    //  First, try to find the server that has the driver with this moniker.
    //  If we don't find one, then we will go to the name server to see if
    //  one is out there, if asked to do so.
    //
    tCIDLib::TBoolean bCreated;
    TCQCPollEngine::TSrvItem* psrviTmp = polleSrc.psrviFindSrvByMoniker
    (
        strMoniker, bCreateIfNew, bCreated
    );

    //
    //  If not found, then we failed. Either we are not creating if new
    //  and it doesn't exist, or we are but it couldn't be added to the
    //  list. Also, if we created the server object, then it's not going
    //  to be ready yet, so just return false for now. They'll come back
    //  and try again later. Or, if it's not up and running yet, then it
    //  cannot have any useful info for us.
    //
    if (!psrviTmp
    ||  bCreated
    ||  (psrviTmp->eState() < TCQCPollEngine::ESrvStates::Idle))
    {
        return kCIDLib::False;
    }

    //
    //  We already have the server, so in theory if all is going well
    //  it should be ready now, so let's lock it and try to find the
    //  driver.
    //
    TLocker lockrSrv(&psrviTmp->mtxSync());
    TCQCPollEngine::TDrvItem* pdrviTmp = psrviTmp->pdrviFind(strMoniker);

    // If we didn't find it, we failed
    if (!pdrviTmp)
        return 0;

    //
    //  Ask the driver to find the field, and to force it onto the poll
    //  list if asked to. If that fails, we return false now and are
    //  done.
    //
    TCQCPollEngine::TFldItem* pfldiTmp = psrviTmp->pfldiFind
    (
        *pdrviTmp, strField, bForceOnPoll
    );
    if (!pfldiTmp)
        return kCIDLib::False;

    // It worked, so store the info and orphan the mutex so it doesn't unlock
    m_psrviTar = psrviTmp;
    m_pdrviTar = pdrviTmp;
    m_pfldiTar = pfldiTmp;
    lockrSrv.Orphan();

    return kCIDLib::True;
}


tCIDLib::TBoolean
TPESrvJanitor::bFindField(          TCQCPollEngine&     polleSrc
                            , const TCQCPollEngFldLink& flnkToFind
                            , const tCIDLib::TBoolean   bForceOnPoll)
{
    // Assume the worst
    m_psrviTar = nullptr;
    m_pdrviTar = nullptr;
    m_pfldiTar = nullptr;

    //
    //  Try to find the server referenced by by the link. We have to lock
    //  each server object as we check it so that, if we find the one with
    //  this id, that it won't be changed before we can get it locked later.
    //  If the server isn't up and running yet, then it can't have the info
    //  we are interested in, so just return false for now and try again
    //  next time.
    //
    TCQCPollEngine::TSrvItem* psrviTmp = polleSrc.psrviLockById(flnkToFind.c4ServerId());
    if (!psrviTmp || (psrviTmp->eState() < TCQCPollEngine::ESrvStates::Idle))
        return kCIDLib::False;

    // We did find it, so it's locked now, so make sure we unlock if needed
    try
    {
        // See if the driver id list is good for this server, else we failed
        if (flnkToFind.c4DriverListId() != psrviTmp->c4DriverListId())
        {
            psrviTmp->Unlock();
            return kCIDLib::False;
        }

        // That's good so get the driver
        TCQCPollEngine::TDrvItem* pdrviTmp = psrviTmp->pdrviFind(flnkToFind.c4DriverId());

        //
        //  See if the field list id is good, else we failed so unlock. Also
        //  verify we really did find the driver. We should since the list has
        //  not changed. But if the caller got an invalid id into the link
        //  object somehow we don't want to freak out here.
        //
        if (!pdrviTmp || (flnkToFind.c4FieldListId() != pdrviTmp->c4FieldListId()))
        {
            psrviTmp->Unlock();
            return kCIDLib::False;
        }

        //
        //  And the field should be good. Pass along the force on poll flag,
        //  to force it onto the poll list if they asked us to.
        //
        TCQCPollEngine::TFldItem* pfldiTmp = psrviTmp->pfldiFind
        (
            *pdrviTmp, flnkToFind.c4FieldId(), bForceOnPoll
        );

        // If field not found, then we failed, so unlock
        if (!pfldiTmp)
        {
            psrviTmp->Unlock();
            return kCIDLib::False;
        }

        // It worked, so store the info and leave it locked
        m_psrviTar = psrviTmp;
        m_pdrviTar = pdrviTmp;
        m_pfldiTar = pfldiTmp;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        // Something went wrong, so unlock and rethrow
        psrviTmp->Unlock();
        throw;
    }
    return kCIDLib::True;
}


//
//  If we already have the server that hosts the indicated driver, then
//  return it. We won't create it if it doesn't already exist.
//
tCIDLib::TBoolean
TPESrvJanitor::bFindSrvByMoniker(       TCQCPollEngine&     polleSrc
                                , const TString&            strMoniker
                                , const tCIDLib::TBoolean   bCreateIfNew
                                ,       tCIDLib::TBoolean&  bCreated)
{
    m_pfldiTar = nullptr;
    m_pdrviTar = nullptr;

    m_psrviTar = polleSrc.psrviFindSrvByMoniker(strMoniker, bCreateIfNew, bCreated);
    if (m_psrviTar)
    {
        // We found it, so lock it and return success
        m_psrviTar->Lock();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCPollEngine
//  PREFIX: intfe
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPollEngine: Public, static methods
// ---------------------------------------------------------------------------
TCQCPollEngine& TCQCPollEngine::Nul_TCQCPollEngine()
{
    static TCQCPollEngine intfeNull;
    return intfeNull;
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we give the poll thread a name that includes an instance count,
//  just in case more than one instance of the engine gets created in the same
//  process.
//
TCQCPollEngine::TCQCPollEngine(const tCIDLib::TEncodedTime   enctDropInterval) :

    m_colServers(tCIDLib::EAdoptOpts::Adopt)
    , m_enctFldDropInterval(enctDropInterval)
    , m_enctLastNSCheck(0)
    , m_scntNextSrvId(1)
    , m_thrPrune
      (
        CQCPollEng_Engine::unamEngInstance.strQueryNewName()
        , TMemberFunc<TCQCPollEngine>(this, &TCQCPollEngine::ePruneThread)
      )
{
    //
    //  If the drop interval passed was zero, set a default. If not zero, then
    //  make sure it's nothing stupid and correct if so.
    //
    if (!m_enctFldDropInterval)
        m_enctFldDropInterval = kCIDLib::enctOneSecond * 60;
    else if (m_enctFldDropInterval < kCIDLib::enctOneSecond * 15)
        m_enctFldDropInterval = kCIDLib::enctOneSecond * 15;
    else if (m_enctFldDropInterval > kCIDLib::enctOneMinute * 10)
        m_enctFldDropInterval = kCIDLib::enctOneMinute * 10;
}

TCQCPollEngine::~TCQCPollEngine()
{
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Given a driver moniker, we'll see if we have it in our list. If so, we'll
//  see if the driver, driver list, or field list ids have changed since the
//  caller last got them. If so, we return true to indicate a change. We also
//  return true to indicate a change if it's not in our list, since it must
//  have been dropped by us or unloaded from the server.
//
tCIDLib::TBoolean
TCQCPollEngine::bCheckDrvChange(const   TString&        strMoniker
                                , const tCIDLib::TCard4 c4FldListId
                                , const tCIDLib::TCard4 c4DriverId
                                , const tCIDLib::TCard4 c4DrvListId)
{
    TLocker lockrSync(&m_mtxSync);

    // Lock this server if we have it. If not just return true
    TPESrvJanitor janSrv;
    if (!janSrv.bFindDriver(*this, strMoniker))
        return kCIDLib::True;

    // We do, so compare the ids
    return ((c4FldListId != janSrv.m_pdrviTar->c4FieldListId())
           || (c4DriverId != janSrv.m_pdrviTar->c4DriverId())
           || (c4DrvListId != janSrv.m_psrviTar->c4DriverListId()));
}


//
//  If the passed moniker is in our list of drivers, indicates whether that
//  driver is online or not. If it's not in our list, we return false and
//  the output is not updated.
//
tCIDLib::TBoolean
TCQCPollEngine::bCheckDrvState( const   TString&                strMoniker
                                ,       tCQCKit::EDrvStates&    eToFill)
{
    TLocker lockrSync(&m_mtxSync);

    // Lock this server if we have it. If not just return false
    TPESrvJanitor janSrv;
    if (!janSrv.bFindDriver(*this, strMoniker))
        return kCIDLib::False;

    eToFill = janSrv.m_pdrviTar->eState();
    return kCIDLib::True;
}


//
//  A helper to test whether any changes have occured that could affect
//  the definition of the field represented by the passed poll info object.
//
tCIDLib::TBoolean
TCQCPollEngine::bCheckLinkChange(const TCQCFldPollInfo& cfpiToTest)
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  Look up the passed field. If we donn't find the field, then that's
    //  fine. We'd want the caller to go back to trying to register it.
    //
    TPESrvJanitor janSrv;
    if (!janSrv.bFindField(*this, cfpiToTest.flnkAssoc(), kCIDLib::False))
        return kCIDLib::False;

    //
    //  If the field links are not the same, then something may have changed
    //  so return true. We update the last access time as well, since clearly
    //  someone is interested. This is a const method that updates a mutable
    //  member, so we can call it from const methods.
    //
    janSrv.m_pfldiTar->SetLastAccessStamp();
    return janSrv.m_pfldiTar->flnkThis() != cfpiToTest.flnkAssoc();
}


//
//  Looks up in our list for the host name of the host whose CQCServer is
//  running the driver with the passed moniker. If the moniker is not in
//  our list, we return false and the output is not updated.
//
tCIDLib::TBoolean
TCQCPollEngine::bHostForMoniker(const   TString&            strMoniker
                                ,       TString&            strHost
                                , const tCIDLib::TBoolean   bCreateIfNew
                                ,       tCIDLib::TBoolean&  bCreated)
{
    TLocker lockrSync(&m_mtxSync);

    TPESrvJanitor janSrv;
    if (!janSrv.bFindSrvByMoniker(*this, strMoniker, bCreateIfNew, bCreated))
        return kCIDLib::False;

    strHost = janSrv.m_psrviTar->strNodeName();
    return kCIDLib::True;
}


//
//  This method will try to find the passed field in the engine's cache of
//  info, without ever trying to force anything in, i.e. it just works on
//  what's already there, and returns the field info on the field if found.
//
tCIDLib::TBoolean
TCQCPollEngine::bQueryFldInfo(  const   TString&        strMoniker
                                , const TString&        strField
                                ,       TCQCFldDef&     flddToFill)
{
    TLocker lockrSync(&m_mtxSync);

    // Try to find the indicated field
    TPESrvJanitor janSrv;
    if (!janSrv.bFindField(*this, strMoniker, strField, kCIDLib::False, kCIDLib::False))
        return kCIDLib::False;

    //
    //  Fill in the caller's buffer with the field info, and update the
    //  last access stamp on the field.
    //
    janSrv.m_pfldiTar->SetLastAccessStamp();
    flddToFill = janSrv.m_pfldiTar->flddInfo();

    return kCIDLib::True;
}


//
//  This method will look up the passed field by name, never trying to force
//  the field into the engine, so it only works on what's already there and
//  being polled. It will return the value as a formatted string and the type
//  of the field. We have another version that doesn't take the type, which
//  just calls the other version.
//
//  We also have a version that returns a flattened field value object. It's
//  in the same format as what the CQCServer returns for a similar sort of call.
//  It's the class type, a frame marker, the flattened value, and an end object
//  marker.
//
tCIDLib::TBoolean
TCQCPollEngine::bReadValue( const   TString&            strMoniker
                            , const TString&            strField
                            ,       TString&            strToFill)
{
    // Just call the other version and toss the type value
    tCQCKit::EFldTypes eType;
    return bReadValue(strMoniker, strField, strToFill, eType);
}

tCIDLib::TBoolean
TCQCPollEngine::bReadValue( const   TString&            strMoniker
                            , const TString&            strField
                            ,       TString&            strToFill
                            ,       tCQCKit::EFldTypes& eType)
{
    TLocker lockrSync(&m_mtxSync);

    // Try to find the indicated field
    TPESrvJanitor janSrv;
    if (!janSrv.bFindField(*this, strMoniker, strField, kCIDLib::False, kCIDLib::False))
        return kCIDLib::False;

    // Bump the last access stamp
    janSrv.m_pfldiTar->SetLastAccessStamp();

    //
    //  If this field's driver is offline or the field itself has an error
    //  status, then return error
    //
    if (!janSrv.m_pdrviTar->bOnline() || janSrv.m_pfldiTar->bInError())
        return kCIDLib::False;

    //
    //  This will fault a new formatted value in if necessary, and it will
    //  bump the last access time stamp.
    //
    strToFill = janSrv.m_pfldiTar->strValue();
    eType = janSrv.m_pfldiTar->flddInfo().eType();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCPollEngine::bReadValue( const   TString&            strMoniker
                            , const TString&            strField
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       TMemBuf&            mbufData
                            ,       tCQCKit::EFldTypes& eType)
{
    TLocker lockrSync(&m_mtxSync);

    // Try to find the indicated field
    TPESrvJanitor janSrv;
    if (!janSrv.bFindField(*this, strMoniker, strField, kCIDLib::False, kCIDLib::False))
        return kCIDLib::False;

    // Bump the last access stamp
    janSrv.m_pfldiTar->SetLastAccessStamp();

    //
    //  If this field's driver is offline or the field itself has an error
    //  status, then return error
    //
    if (!janSrv.m_pdrviTar->bOnline() || janSrv.m_pfldiTar->bInError())
        return kCIDLib::False;

    // Create a stream over the caller's buffer and straem out the info
    {
        TBinMBufOutStream strmOut(&mbufData);
        strmOut << janSrv.m_pfldiTar->fvCurrent().clsIsA()
                << tCIDLib::EStreamMarkers::Frame;
        janSrv.m_pfldiTar->fvCurrent().StreamOutValue(strmOut);
        strmOut << tCIDLib::EStreamMarkers::EndObject
                << kCIDLib::FlushIt;

        c4Bytes = strmOut.c4CurSize();
    }

    //
    //  Set the last access since it won't happen automatically when we access
    //  the field this way. And give back the type.
    //
    janSrv.m_pfldiTar->SetLastAccessStamp();
    eType = janSrv.m_pfldiTar->flddInfo().eType();
    return kCIDLib::True;
}


//
//  Sometimes widgets will need to talk directly to the driver that owns
//  their field, to make a backdoor query of non-field data or some other
//  such thing. We have the info to get them a CQCServer admin proxy
//  object fairly quickly, so we provide a method for that.
//
tCIDLib::TBoolean
TCQCPollEngine::bGetSrvProxy(const  TString&                strMoniker
                            ,       tCQCKit::TCQCSrvProxy&  orbcToSet)
{
    // Lock to get the server name
    TString strHost;
    {
        TLocker lockrSync(&m_mtxSync);

        TPESrvJanitor janSrv;
        tCIDLib::TBoolean bCreated;
        if (!janSrv.bFindSrvByMoniker(*this, strMoniker, kCIDLib::False, bCreated))
            return kCIDLib::False;

        strHost = janSrv.m_psrviTar->strNodeName();
    }

    TCQCSrvAdminClientProxy* porbcAdmin = nullptr;
    try
    {
        porbcAdmin = facCQCKit().porbcCQCSrvAdminProxy(strHost);
    }

    catch(TError& errToCatch)
    {
        if (facCQCPollEng().bLogInfo() && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    orbcToSet.SetPointer(porbcAdmin);
    return kCIDLib::True;
}


//
//  Clients call this to get the latest value of a field. They pass us a
//  serial number (max card initially) so that we can tell them if there has
//  been any change since the last read or not.
//
//  We return a status that indicates either nothing has changed, or that
//  either a new value is being returned, or a new state has been set
//  on the widget.
//
tCQCKit::EValQRes
TCQCPollEngine::eQueryValue(const   TString&            strToRead
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       TCQCFldValue&       fvToFill
                            ,       TCQCPollEngFldLink& flnkInfo)
{
    // Break apart the field name
    TString strMoniker;
    TString strField;
    if (!facCQCKit().bParseFldName(strToRead, strMoniker, strField))
        return tCQCKit::EValQRes::NotFound;

    // And call the other version
    return eQueryValue(strMoniker, strField, c4SerialNum, fvToFill, flnkInfo);
}


tCQCKit::EValQRes
TCQCPollEngine::eQueryValue(const   TString&            strMoniker
                            , const TString&            strField
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       TCQCFldValue&       fvToFill
                            ,       TCQCPollEngFldLink& flnkInfo)
{
    // Lock while we are searching the engine's data
    TLocker lockrSync(&m_mtxSync);

    //
    //  Look up the field. Tell the janitor to create the server object
    //  if it's not already loaded. If it has to create it, it will still
    //  return false though, since the value won't be ready yet, so we'll
    //  come back again next time and try again, by which time is probably
    //  will be ready.
    //
    TPESrvJanitor janSrv;
    if (!janSrv.bFindField(*this, flnkInfo, kCIDLib::True))
        return tCQCKit::EValQRes::Reconfig;

    // It's there so update the last access stamp
    janSrv.m_pfldiTar->SetLastAccessStamp();

    // The driver is offline, so force reconfig
    if (!janSrv.m_pdrviTar->bOnline())
        return tCQCKit::EValQRes::Reconfig;

    // The field is just in error state, so return error
    if (janSrv.m_pfldiTar->bInError())
        return tCQCKit::EValQRes::InError;

    //
    //  If the serial number hasn't changed, indicate no change. We
    //  alreayd updated the access stamp above, so even if it's not
    //  changing, the engine knows we are still interested.
    //
    if (c4SerialNum == janSrv.m_pfldiTar->c4SerialNum())
        return tCQCKit::EValQRes::NoChange;

    // It has changed, so return the new serial number
    c4SerialNum = janSrv.m_pfldiTar->c4SerialNum();

    // Do the actual query of the value into the caller's buffer
    janSrv.m_pfldiTar->QueryValue(fvToFill);

    //  Indicate we have a new value
    return tCQCKit::EValQRes::NewValue;
}


//
//  This guy will handle registering a single field with the polling engine.
//  They just pass us a 'moniker.field' type of string. If it's already in
//  the engine, then we don't have to do anything. Else we add it.
//
//  Note that we look up the field such that the server is created if it
//  doesn't exist, but we don't force it onto the poll list immediately.
//  We check other things and then add it after we are sure it's reasonable.
//
tCQCPollEng::EFldRegRes
TCQCPollEngine::eRegisterField( const   TString&            strToReg
                                , const tCQCKit::EReqAccess eReqAccess
                                ,       TCQCFldDef&         flddInfo
                                ,       TCQCPollEngFldLink& flnkInfo
                                ,       TString&            strMake
                                ,       TString&            strModel)
{
    // Break apart the field name
    TString strMoniker;
    TString strField;
    if (!facCQCKit().bParseFldName(strToReg, strMoniker, strField))
        return tCQCPollEng::EFldRegRes::BadName;

    // Call the other version
    return eRegisterField
    (
        strMoniker, strField, eReqAccess, flddInfo, flnkInfo, strMake, strModel
    );
}

tCQCPollEng::EFldRegRes
TCQCPollEngine::eRegisterField( const   TString&            strMoniker
                                , const TString&            strField
                                , const tCQCKit::EReqAccess eReqAccess
                                ,       TCQCFldDef&         flddInfo
                                ,       TCQCPollEngFldLink& flnkInfo
                                ,       TString&            strMake
                                ,       TString&            strModel)
{
    // If the widget doesn't want any read access, then makes no sense
    if (!tCIDLib::bAllBitsOn(eReqAccess, tCQCKit::EReqAccess::ReadOnly))
        return tCQCPollEng::EFldRegRes::NoReadAccess;

    // Lock while we are searching the engine's data
    TLocker lockrSync(&m_mtxSync);

    //
    //  Note that this guy will call into us and try to find the tarver server.
    //  If it doesn't, we tell it to create the server object. If will do that
    //  but it will still return false since the field won't be ready until the
    //  server object comes online. We can't keep the list locked and wait for
    //  it come online. By the time the caller comes back to check again it
    //  should be ready if all is going well.
    //
    TPESrvJanitor janSrv;
    if (!janSrv.bFindField(*this, strMoniker, strField, kCIDLib::True, kCIDLib::False))
        return tCQCPollEng::EFldRegRes::NotFound;

    //
    //  Make sure the basic field info gets stored one way or another, even
    //  if we end up rejecting the field for some reason, so that the caller
    //  can see what the field is and get the type and so forth.
    //
    flddInfo = janSrv.m_pfldiTar->flddInfo();
    strMake = janSrv.m_pdrviTar->strMake();
    strModel = janSrv.m_pdrviTar->strModel();

    //
    //  Make sure that the access that the widget needs is available for this
    //  field, since they can be affected by the field def changing after the
    //  client originally looked up the field. If it's not available, then put
    //  it into the field rejected state.
    //
    const TCQCFldDef& flddLink = janSrv.m_pfldiTar->flddInfo();
    if (!facCQCKit().bCheckFldAccess(eReqAccess, flddLink.eAccess()))
        return tCQCPollEng::EFldRegRes::WrongAccess;

    //
    //  Ok, we found it, so force it onto the polling list if not already.
    //  This will also update its last access stamp.
    //
    if (!janSrv.m_pfldiTar->bOnPollList())
        janSrv.m_psrviTar->AddNewField(strMoniker, strField);

    // Update the last access stamp since it's been registered OK
    janSrv.m_pfldiTar->SetLastAccessStamp();

    // Looks good so now store the link info since we are going to use it
    flnkInfo = janSrv.m_pfldiTar->flnkThis();

    return tCQCPollEng::EFldRegRes::OK;
}



//
//  For diagnostic purposes, this method will return a list of fields
//  currently being polled.
//
tCIDLib::TVoid TCQCPollEngine::QueryFlds(tCIDLib::TStrList& colDrvs)
{
    colDrvs.RemoveAll();

    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    // Iterate ourself through the whole tree
    TString strFld;
    const tCIDLib::TCard4 c4SrvCnt = m_colServers.c4ElemCount();
    TDrvItem* pdrviRet = nullptr;
    for (tCIDLib::TCard4 c4SrvInd = 0; c4SrvInd < c4SrvCnt; c4SrvInd++)
    {
        TSrvItem* psrviCur = m_colServers[c4SrvInd];

        // Lock this server and ask it for its active polling fields
        {
            TLocker lockrSrv(&psrviCur->mtxSync());
            psrviCur->QueryActiveFlds(colDrvs, kCIDLib::True);
        }
    }
}



// Called by clients to start up our background polling engine
tCIDLib::TVoid TCQCPollEngine::StartEngine(const TCQCSecToken& sectUser)
{
    // Just start up the pruning thread if not already running
    if (!m_thrPrune.bIsRunning())
    {
        m_sectUser = sectUser;
        m_thrPrune.Start();
    }
}


//
//  Called by clients to stop our engine. It will stop the prunning
//  engine and clean up the server items and their threads.
//
tCIDLib::TVoid TCQCPollEngine::StopEngine()
{
    if (m_thrPrune.bIsRunning())
    {
        try
        {
            m_thrPrune.ReqShutdownSync(5000);
            m_thrPrune.eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            if (facCQCPollEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQCPollEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kPEngErrs::errcEng_ShutdownFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }
    }

    //
    //  Clean up our servers, which will clean up their drivers, and their
    //  fields and threads.
    //
    try
    {
        // Make sure all clients are out first, then clean up
        TLocker lockrSync(&m_mtxSync);

        const tCIDLib::TCard4 c4Count = m_colServers.c4ElemCount();

        // Make one pass to start them shutting down
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TSrvItem* psrviCur = m_colServers[c4Index];
            psrviCur->StartShutdown();
        }

        // And another pass to wait for them to end
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TSrvItem* psrviCur = m_colServers[c4Index];
            psrviCur->WaitShutdown();
        }

        // And now we can clean them all up
        m_colServers.RemoveAll();
    }

    catch(TError& errToCatch)
    {
        if (facCQCPollEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    m_sectUser.Reset();
}


//
//  These are called by the interface (or other client of this engine) to
//  write a value to a field. We let any exceptions propogate out of here
//  if we cannot get the admin proxy or the moniker or field doesn't
//  exist.
//
tCIDLib::TVoid TCQCPollEngine::WriteField(  const   TString&        strMoniker
                                            , const TString&        strFldName
                                            , const TString&        strValue
                                            , const TCQCSecToken&   sectUser)
{
    //
    //  Use the moniker name to get a CQC server admin proxy, which we will
    //  then use to write the value.
    //
    tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
    orbcAdmin->WriteFieldByName
    (
        strMoniker, strFldName, strValue, sectUser, tCQCKit::EDrvCmdWaits::DontCare
    );
}

tCIDLib::TVoid TCQCPollEngine::WriteField(  const   TString&        strFullFldName
                                            , const TString&        strValue
                                            , const TCQCSecToken&   sectUser)
{
    //
    //  Break the name out into the parts, driver moniker and field
    //  name.
    //
    TString strFldName;
    TString strMoniker;
    facCQCKit().ParseFldName(strFullFldName, strMoniker, strFldName);

    //
    //  Use the moniker name to get a CQC server admin proxy, which we will
    //  then use to write the value.
    //
    tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
    orbcAdmin->WriteFieldByName
    (
        strMoniker, strFldName, strValue, sectUser, tCQCKit::EDrvCmdWaits::DontCare
    );
}


// ---------------------------------------------------------------------------
//  TCQCPollEngine: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is used to hand out the sequential ids we assign to new servers or
//  reassign to servers that have lost connection and regained it, since we
//  don't know what might have changed, so we want to invalidate any links
//  we might have given out before.
//
//  The server items call this any time they need to give themselves a new
//  id. We use a safe counter to handle the fact that multiple server item
//  poll threads could call it at once.
//
tCIDLib::TCard4 TCQCPollEngine::c4NextServerId()
{
    return m_scntNextSrvId++;
}


//
//  This thread just does a little maintenance of the server list, throwing
//  out any that have not been accessed for a period of time.
//
tCIDLib::EExitCodes
TCQCPollEngine::ePruneThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    tCIDLib::EExitCodes eRet = tCIDLib::EExitCodes::Normal;
    while (kCIDLib::True)
    {
        try
        {
            //
            //  Sleep for a while. If we wake up due to a shutdown
            //  request, break out.
            //
            if (!thrThis.bSleep(5000))
                break;

            // Do a pruning pass. He'll lock while doing this
            PruneServers();
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
    }
    return eRet;
}


//
//  Looks through our list of servers and sees if one is for the indicated
//  host. We don't have to lock them here since the host name never changes
//  for a given server item.
//
TCQCPollEngine::TSrvItem* TCQCPollEngine::psrviFindSrvByHost(const TString& strHost)
{
    TCQCPollEngine::TSrvItem* psrviRet = nullptr;
    const tCIDLib::TCard4 c4Count = m_colServers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        psrviRet = m_colServers[c4Index];
        if (psrviRet->strNodeName() == strHost)
            return psrviRet;
    }
    return nullptr;
}


//
//  This method will try to find a server item for the indicated moniker.
//
//  This method throttles on a per-moniker basis. The clients are not
//  coordinated in their calls to register fields. They continuously try to
//  register until they succeed, fairly rapidly. We can't bang on the name
//  server like that. So we keep a list of monikers that have been asked
//  for and failed. If it's for one of those, we see how long it's been since
//  we last checked, and will just return immediate failure if it's not been
//  at least the minimum cool off time.
//
//  This way clients can just try to re-register every time they try to query
//  a field value and find they've not been able to register it yet, but we
//  won't induce undue network overhead.
//
//  In order to avoid further overhead, we first check our list of servers
//  to see if one of them has the driver. This is something we have to
//  careful about. A driver, for instance, might be unloaded from one server
//  and loaded to another. But, that's ok, the server will see the change
//  and the moniker lookup would subsquently fail on that server point and
//  we would go to the name server again.
//
//  There's no problem searching the servers for the moniker. No deadlock
//  can occur since the caller has to lock the main mutex before calling
//  this method, therefore no other server items can be locked. We do though
//  have to lock each server item before we ask it is if has the driver, since
//  otherwise the server's poll thread might be changing the data. A little
//  overhead there but way less than going to the name server.
//
//  The caller has to have locked the main mutex before calling here, so
//  any server we find cannot go away before we get back to the caller. Once
//  the caller returns the caller will lock the server before attemping to
//  look for any drivers/fields inside it.
//
TCQCPollEngine::TSrvItem*
TCQCPollEngine::psrviFindSrvByMoniker(  const   TString&            strMoniker
                                        , const tCIDLib::TBoolean   bCreateIfNew
                                        ,       tCIDLib::TBoolean&  bCreated)
{
    // Watch for a pathological scenario
    if (strMoniker.bIsEmpty())
    {
        CIDAssert2(L"The moniker passed to the polling engine was empty");
        return nullptr;
    }

    bCreated = kCIDLib::False;
    TSrvItem* psrviRet = nullptr;

    //
    //  First see if we already have a server that claims to own this driver.
    //  If so, then we are done.
    //
    psrviRet = psrviSearchForMoniker(strMoniker);
    if (psrviRet)
        return psrviRet;

    // Oh well, not there so lock the fail list while we do this
    TLocker lockrFailList(&CQCPollEng_Engine::colFailList);

    //
    //  See if this moniker is in the list of servers that we've recently
    //  failed on. If so, then only check again once the throttling period
    //  is up.
    //
    TSrvLookupFail* pslfSrv = CQCPollEng_Engine::colFailList.pobjFindByKey(strMoniker);
    if (pslfSrv)
    {
        //
        //  We've failed on this one before, so see if the check period is
        //  up for this driver. If not, just return. If so, update it's
        //  time and fall through to try it.
        //
        const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
        if (pslfSrv->m_enctLast + CQCPollEng_Engine::enctDrvCheckPeriod > enctNow)
            return nullptr;

        pslfSrv->m_enctLast = enctNow;
    }

    // Hasn't failed lately, so let's go to the name server
    tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

    // Build up the name it would have, and see if it exists
    TString strBinding(TCQCSrvAdminClientProxy::strDrvScope);
    strBinding.Append(kCIDLib::pszTreeSepChar);
    strBinding.Append(strMoniker);

    //
    //  See if this moniker really does exist. If not, then we have to give
    //  up. If it does, then we now have the host address.
    //
    TString strName;
    TString strDescr;
    if (!orbcNS->bQueryNameInfo(strBinding, strName, strDescr))
    {
        // Didn't find it, so add it to the fail list if not already
        if (!pslfSrv)
            CQCPollEng_Engine::colFailList.objAdd(TSrvLookupFail(strMoniker));
        return nullptr;
    }

    // If it was in the fail list before, remove it now
    if (pslfSrv)
        CQCPollEng_Engine::colFailList.bRemoveKey(strMoniker);

    //
    //  See if we have a server item for this host name. If not, and they
    //  want to create one, then we create a new one. If we find it, then
    //  we'll fall through and return that one.
    //
    psrviRet = psrviFindSrvByHost(strDescr);
    if (!psrviRet && bCreateIfNew)
    {
        psrviRet = new TSrvItem
        (
            this, m_thrPrune.strName(), strDescr, m_enctFldDropInterval
        );
        m_colServers.Add(psrviRet);

        // Indicate to the caller we created it
        bCreated = kCIDLib::True;
    }
    return psrviRet;
}


//
//  Looks for the server item with the indicated id and, if found, locks
//  it and returns it locked so that the id cannot lock before the caller
//  gets to it.
//
TCQCPollEngine::TSrvItem* TCQCPollEngine::psrviLockById(const tCIDLib::TCard4 c4ToFind)
{
    TCQCPollEngine::TSrvItem* psrviRet = nullptr;
    const tCIDLib::TCard4 c4Count = m_colServers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        psrviRet = m_colServers[c4Index];
        psrviRet->Lock();
        if (psrviRet->c4ServerId() == c4ToFind)
            return psrviRet;
        psrviRet->Unlock();
    }
    return nullptr;
}


//
//  Searchs our existing list of servers to see if one of them owns the
//  indicated driver moniker. We have to lock each one as we make this
//  call since otherwise the server might be in the process of updating
//  the driver list.
//
TCQCPollEngine::TSrvItem*
TCQCPollEngine::psrviSearchForMoniker(const TString& strMoniker)
{
    const tCIDLib::TCard4 c4Count = m_colServers.c4ElemCount();
    TDrvItem* pdrviRet = nullptr;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TSrvItem* psrviCur = m_colServers[c4Index];
        TLocker lockrDrv(&psrviCur->mtxSync());
        if (psrviCur->bHasDriver(strMoniker))
            return psrviCur;
    }
    return nullptr;
}



//
//  This is called periodically by the pruning thread to see if we have
//  any servers that we can dump because it's been idle (or offline) for
//  a good while.
//
tCIDLib::TVoid TCQCPollEngine::PruneServers()
{
    // Lock the list while we do this
    TLocker lockrSync(&m_mtxSync);

    //
    //  Run through the list and see if any have been in pre-ready state for
    //  more than the threshold, remove them.
    //
    TSrvList::TNCCursor cursSrvs(&m_colServers);
    while (cursSrvs.bIsValid())
    {
        TSrvItem& srviCur = cursSrvs.objWCur();

        //
        //  If this guy is not ready, and has been in it's current
        //  state for over the threshold, then kill it.
        //
        const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
        if ((srviCur.eState() < ESrvStates::Ready)
        &&  srviCur.bStateChangeThreshold(enctNow))
        {
            //
            //  Shut this one down and remove it from the list. In
            //  this case we only have one so we just call the start
            //  and wait back to back.
            //
            cursSrvs.objWCur().StartShutdown();
            cursSrvs.objWCur().WaitShutdown();

            m_colServers.RemoveAt(cursSrvs);
        }
         else
        {
            cursSrvs.bNext();
        }
    }
}

