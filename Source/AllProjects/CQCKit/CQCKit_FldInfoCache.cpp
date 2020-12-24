//
// FILE NAME: CQCKit_FldInfoCache.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/11/2001
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
RTTIDecls(TCQCFldCache,TObject)
RTTIDecls(TFICacheDrvItem,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TFICacheDrvItem
//  PREFIX: fcdi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFICacheDrvItem: Public, static methods
// ---------------------------------------------------------------------------
const TString& TFICacheDrvItem::strGetKey(const TFICacheDrvItem& fcdiSrc)
{
    return fcdiSrc.m_strMoniker;
}


// ---------------------------------------------------------------------------
//  TFICacheDrvItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TFICacheDrvItem::TFICacheDrvItem(const  TString&                strMoniker
                                , const tCQCKit::EDevCats       eCategory
                                , const tCQCKit::TDevClassList& fcolDevCls
                                , const tCIDLib::TCard4         c4FldListId
                                , const tCIDLib::TCard4         c4DriverId
                                , const tCQCKit::TFldDefList&   colFields) :

    m_bLoaded(kCIDLib::True)
    , m_c4DriverId(c4DriverId)
    , m_c4FldListId(c4FldListId)
    , m_colFields(colFields)
    , m_eCategory(eCategory)
    , m_fcolDevClasses(fcolDevCls)
    , m_strMoniker(strMoniker)
{
}

TFICacheDrvItem::TFICacheDrvItem(const  TString&                strMoniker
                                , const tCQCKit::EDevCats       eCategory
                                , const tCQCKit::TDevClassList& fcolDevCls
                                , const TString&                strBinding) :

    m_bLoaded(kCIDLib::False)
    , m_c4DriverId(0)
    , m_c4FldListId(0)
    , m_eCategory(eCategory)
    , m_fcolDevClasses(fcolDevCls)
    , m_strBinding(strBinding)
    , m_strMoniker(strMoniker)
{
}

TFICacheDrvItem::TFICacheDrvItem(const  TString&                strMoniker
                                , const tCQCKit::EDevCats       eCategory
                                , const tCQCKit::TDevClassList& fcolDevCls
                                , const TString&                strMake
                                , const TString&                strModel
                                , const TString&                strBinding) :

    m_bLoaded(kCIDLib::False)
    , m_c4DriverId(0)
    , m_c4FldListId(0)
    , m_eCategory(eCategory)
    , m_fcolDevClasses(fcolDevCls)
    , m_strBinding(strBinding)
    , m_strMake(strMake)
    , m_strModel(strModel)
    , m_strMoniker(strMoniker)
{
}

TFICacheDrvItem::TFICacheDrvItem(const TFICacheDrvItem& fcdiSrc) :

    m_bLoaded(fcdiSrc.m_bLoaded)
    , m_c4DriverId(fcdiSrc.m_c4DriverId)
    , m_c4FldListId(fcdiSrc.m_c4FldListId)
    , m_colFields(fcdiSrc.m_colFields)
    , m_eCategory(fcdiSrc.m_eCategory)
    , m_fcolDevClasses(fcdiSrc.m_fcolDevClasses)
    , m_strBinding(fcdiSrc.m_strBinding)
    , m_strMake(fcdiSrc.m_strMake)
    , m_strModel(fcdiSrc.m_strModel)
    , m_strMoniker(fcdiSrc.m_strMoniker)
{
}

TFICacheDrvItem::~TFICacheDrvItem()
{
}


// ---------------------------------------------------------------------------
//  TFICacheDrvItem: Public operators
// ---------------------------------------------------------------------------
TFICacheDrvItem& TFICacheDrvItem::operator=(const TFICacheDrvItem& fcdiSrc)
{
    if (this != &fcdiSrc)
    {
        m_bLoaded           = fcdiSrc.m_bLoaded;
        m_c4DriverId        = fcdiSrc.m_c4DriverId;
        m_c4FldListId       = fcdiSrc.m_c4FldListId;
        m_colFields         = fcdiSrc.m_colFields;
        m_eCategory         = fcdiSrc.m_eCategory;
        m_fcolDevClasses    = fcdiSrc.m_fcolDevClasses;
        m_strBinding        = fcdiSrc.m_strBinding;
        m_strMake           = fcdiSrc.m_strMake;
        m_strModel          = fcdiSrc.m_strModel;
        m_strMoniker        = fcdiSrc.m_strMoniker;
    }
    return *this;
}



// ---------------------------------------------------------------------------
//  TFICacheDrvItem: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TFICacheDrvItem::bFldExists(const TString& strField, TCQCFldDef& flddToFill) const
{
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = m_colFields[c4Index];
        if (flddCur.strName() == strField)
        {
            flddToFill = flddCur;
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  See if this driver has a field with the indicated access. If so, return
//  it. Skip ones that start with $, so that we don't use any of the standard
//  statistics fields.
//
tCIDLib::TBoolean
TFICacheDrvItem::bFldWithAccess(const   tCQCKit::EReqAccess eReqAcc
                                ,       TString&            strMoniker
                                ,       TString&            strName) const
{
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = m_colFields[c4Index];

        if (facCQCKit().bCheckFldAccess(eReqAcc, flddCur.eAccess()))
        {
            strMoniker = m_strMoniker;
            strName = flddCur.strName();
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  Return whether we have any fields or not. Generally this would never
//  return false since the field cache will not store any that have no fields
//  matching the provided criteria, but it could happen if the outside world
//  forced a driver into the list.
//
tCIDLib::TBoolean TFICacheDrvItem::bIsEmpty() const
{
    return m_colFields.bIsEmpty();
}


// Returns whether this device implements the indicated device class
tCIDLib::TBoolean
TFICacheDrvItem::bImplementsClass(const tCQCKit::EDevClasses eClass) const
{
    return m_fcolDevClasses.bElementPresent(eClass);
}


// Indicates if our field list has been faulted in yet
tCIDLib::TBoolean TFICacheDrvItem::bIsLoaded() const
{
    return m_bLoaded;
}


// If this one implements any device classes, it's V2
tCIDLib::TBoolean TFICacheDrvItem::bIsV2() const
{
    return !m_fcolDevClasses.bIsEmpty();
}


tCIDLib::TCard4 TFICacheDrvItem::c4DriverId() const
{
    return m_c4DriverId;
}


tCIDLib::TCard4 TFICacheDrvItem::c4FldListId() const
{
    return m_c4FldListId;
}


// Provide access to the fields we have
tCQCKit::TFldDefList& TFICacheDrvItem::colFldList()
{
    return m_colFields;
}

const tCQCKit::TFldDefList& TFICacheDrvItem::colFldList() const
{
    return m_colFields;
}


// Return the device category of our device
tCQCKit::EDevCats TFICacheDrvItem::eCategory() const
{
    return m_eCategory;
}


// Provide read only access to our device class list
const tCQCKit::TDevClassList& TFICacheDrvItem::fcolDevClasses() const
{
    return m_fcolDevClasses;
}


//
//  If we try to load the field list but can't, then we need to marke the
//  item loaded without setting the field list, whichis normally how it
//  gets marked. This will prevent us from redundantly trying to load it.
//
tCIDLib::TVoid TFICacheDrvItem::MarkLoaded()
{
    m_bLoaded = kCIDLib::True;
}


//
//  Find a field in our list with the indicate name. We can optionalyl throw
//  if not found, or return a boolean result.
//
TCQCFldDef*
TFICacheDrvItem::pflddInfo( const   TString&            strField
                            , const tCIDLib::TBoolean   bThrowIfNot)
{
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCFldDef& flddCur = m_colFields[c4Index];
        if (flddCur.strName() == strField)
            return &flddCur;
    }

    if (bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFlC_FldNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , m_strMoniker
            , strField
        );
    }
    return 0;
}

const TCQCFldDef*
TFICacheDrvItem::pflddInfo( const   TString&            strField
                            , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = m_colFields[c4Index];
        if (flddCur.strName() == strField)
            return &flddCur;
    }

    if (bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFlC_FldNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , m_strMoniker
            , strField
        );
    }
    return 0;
}


const TString& TFICacheDrvItem::strBinding() const
{
    return m_strBinding;
}


const TString& TFICacheDrvItem::strMake() const
{
    return m_strMake;
}


const TString& TFICacheDrvItem::strModel() const
{
    return m_strModel;
}


const TString& TFICacheDrvItem::strMoniker() const
{
    return m_strMoniker;
}


//
//  Our owning cache object calls us here duing loading and gives us a list
//  of all of the fields for our driver and it's make/model, plus a filter
//  to filter out any that the calling code doesn't want. The filter may
//  be null if no filtering is required.
//
tCIDLib::TVoid
TFICacheDrvItem::SetFldList(const   tCQCKit::TFldDefList&   colSrc
                            , const tCIDLib::TCard4         c4FldListId
                            , const tCIDLib::TCard4         c4DriverId
                            , const TCQCFldFilter* const    pffiltCriteria)
{
    // Store the ids away
    m_c4DriverId  = c4DriverId;
    m_c4FldListId = c4FldListId;

    //
    //  Clear the list in preparation for loading and mark us loaded now,
    //  so that even if something should go wrong below, we won't keep doing
    //  this over and over.
    //
    m_colFields.RemoveAll();
    m_bLoaded = kCIDLib::True;

    //
    //  Iterate the source list and, for those that pass the filter, we
    //  keep them.
    //
    const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colSrc[c4Index];
        if (pffiltCriteria)
        {
            //
            //  We have a filter, so filter it. We pass our own mmoniker, make,
            //  model and device category for it to filter against, and the
            //  field def of course.
            //
            if (pffiltCriteria->bCanAcceptField(flddCur
                                                , m_strMoniker
                                                , m_strMake
                                                , m_strModel
                                                , m_eCategory
                                                , m_fcolDevClasses))
            {
                m_colFields.objAdd(flddCur);
            }
        }
         else
        {
            // No filter, just take it as is
            m_colFields.objAdd(flddCur);
        }
    }
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCFldCache
//  PREFIX: cfc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldCache: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldCache::TCQCFldCache() :

    m_bV2Only(kCIDLib::False)
    , m_colDrivers(53, TStringKeyOps(), &TFICacheDrvItem::strGetKey)
    , m_pffiltCriteria(nullptr)
{
}

TCQCFldCache::~TCQCFldCache()
{
    // Clean up any filter we were given
    if (m_pffiltCriteria)
    {
        try
        {
            delete m_pffiltCriteria;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCFldCache: Public, non-virtual methods
// ---------------------------------------------------------------------------

// See if a particular driver is in the cache
tCIDLib::TBoolean TCQCFldCache::bDevExists(const TString& strMoniker) const
{
    // Don't throw if not found, don't fault in fields
    return (pfcdiFindMoniker(strMoniker, kCIDLib::False, kCIDLib::False) != 0);
}

tCIDLib::TBoolean
TCQCFldCache::bDevExists(const  TString&            strMoniker
                        ,       TString&            strMake
                        ,       TString&            strModel
                        ,       tCQCKit::EDevCats&  eDevCat) const
{
    // Don't throw if not found, don't fault in fields
    const TFICacheDrvItem* pfcdiTar = pfcdiFindMoniker
    (
        strMoniker, kCIDLib::False, kCIDLib::False
    );

    if (!pfcdiTar)
        return kCIDLib::False;

    eDevCat = pfcdiTar->eCategory();
    strMake = pfcdiTar->strMake();
    strModel = pfcdiTar->strModel();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCFldCache::bFldExists(const  TString&    strMoniker
                        , const TString&    strField) const
{
    //
    //  Look up the driver item for the moniker. Don't throw if not found,
    //  but fault in fields if found and not already loaded.
    //
    const TFICacheDrvItem* pfcdiDev = pfcdiFindMoniker(strMoniker, kCIDLib::False);
    if (!pfcdiDev)
        return kCIDLib::False;

    // See if it has the field
    return (pfcdiDev->pflddInfo(strField, kCIDLib::False) != 0);
}


tCIDLib::TBoolean
TCQCFldCache::bFldExists(const  TString&    strMoniker
                        , const TString&    strField
                        ,       TCQCFldDef& flddToFill) const
{
    //
    //  Look up the driver item for the moniker. Don't throw if not found,
    //  but fault in fields if found and not already loaded.
    //
    const TFICacheDrvItem* pfcdiDev = pfcdiFindMoniker(strMoniker, kCIDLib::False);
    if (!pfcdiDev)
        return kCIDLib::False;

    // See if it has the field
    return pfcdiDev->bFldExists(strField, flddToFill);
}


//
//  See returns true if the indicated driver has any fields that can meet
//  the requested access.
//
tCIDLib::TBoolean TCQCFldCache::
bHasFieldsWithAccess(const  TString&            strMoniker
                    , const tCQCKit::EReqAccess eAccess) const
{
    //
    //  Look up the driver item for the moniker. Throw if not found and
    //  fault in fields if not already done.
    //
    const TFICacheDrvItem& fcdiDev = *pfcdiFindMoniker(strMoniker);
    const tCQCKit::TFldDefList& colFields = fcdiDev.colFldList();

    // Iterate the list and look for fields that match the criteria
    tCIDLib::TCard4 c4Count = colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFields[c4Index];
        const tCQCKit::EFldAccess eCurAcc = flddCur.eAccess();

        if (facCQCKit().bCheckFldAccess(eAccess, eCurAcc))
            return kCIDLib::True;
    }

    // Never found one
    return kCIDLib::False;
}


//
//  Returns true if the indcated driver is V2 comlaint. No need to fault in the
//  fields for this, it's part of the stuff we get during init.
//
tCIDLib::TBoolean TCQCFldCache::bIsV2Compliant(const TString& strMoniker) const
{
    const TFICacheDrvItem& fcdiDev = *pfcdiFindMoniker
    (
        strMoniker, kCIDLib::True, kCIDLib::False
    );
    return fcdiDev.bIsV2();
}


//
//  A convenience method that is used when some default initial driver.field
//  is desired. Just prevents the outside world from having to do the grunt
//  work itself. Usually called when some design tool cannot find a previously
//  configured field and needs to reset to a default.
//
//  It provides the original name so that we can first check for another field
//  in the same driver.
//
tCIDLib::TBoolean
TCQCFldCache::bQueryDefField(const  TString&            strOrgMoniker
                            ,       TString&            strMoniker
                            ,       TString&            strField
                            , const tCQCKit::EReqAccess eAccess) const
{
    //
    //  First see if we can find the original moniker and search it, faulting
    //  in fields if needed.
    //
    const TFICacheDrvItem* pfcdiOrg = pfcdiFindMoniker(strMoniker, kCIDLib::False);
    if (pfcdiOrg)
    {
        if (pfcdiOrg->bFldWithAccess(eAccess, strMoniker, strField))
            return kCIDLib::True;
    }


    //
    //  Couldn't find that guy, so let's iterate what we have. We use a non
    //  const cursor because we may have to fault in fields. The driver list
    //  is mutable so we can do this.
    //
    TDrvList::TNCCursor cursFind(&m_colDrivers);

    // If no drivers, then return false now
    if (!cursFind.bReset())
        return kCIDLib::False;

    //
    //  Search the list of drivers a first time, only checking drivers whose
    //  fields have already been faulted in.
    //
    do
    {
        const TFICacheDrvItem& fcdiDef = cursFind.objRCur();

        // If this guy has has been faultd in already, let's check it
        if (!fcdiDef.bIsLoaded())
        {
            if (fcdiDef.bFldWithAccess(eAccess, strMoniker, strField))
                return kCIDLib::True;
        }
    }   while (cursFind.bNext());

    //
    //  Didn't find one, so we have to start faultin in fields. This is
    //  sort of piggy but this is only used in design tiem tools, so not
    //  that bad.
    //
    //  Not we only have to check drivers this time that have not been
    //  faulted in. Any others we did above.
    //
    cursFind.bReset();
    do
    {
        TFICacheDrvItem& fcdiDef = cursFind.objWCur();

        // If not faulted in yet, do so. If it works, then search it
        if (!fcdiDef.bIsLoaded())
        {
            FaultInFields(fcdiDef);
            if (fcdiDef.bFldWithAccess(eAccess, strMoniker, strField))
                return kCIDLib::True;
        }
    }   while (cursFind.bNext());

    return kCIDLib::False;
}


// Return a list of monikers that implement the indicated device class
tCIDLib::TBoolean
TCQCFldCache::bQueryDevList(        TCollection<TString>&   colMonikersToFill
                            , const tCQCKit::EDevClasses    eClass) const
{
    // Flush the caller's list just in case
    colMonikersToFill.RemoveAll();

    // Iterate our device list and copy the key part out to the caller's list
    TDrvList::TCursor cursDrv(&m_colDrivers);
    if (!cursDrv.bReset())
        return kCIDLib::False;

    do
    {
        const TFICacheDrvItem& fcdiCur = cursDrv.objRCur();

        if (fcdiCur.bImplementsClass(eClass))
            colMonikersToFill.objAdd(fcdiCur.strMoniker());
    }   while (cursDrv.bNext());

    return !colMonikersToFill.bIsEmpty();
}


// Tell the caller if we found any devices
tCIDLib::TCard4 TCQCFldCache::c4DevCount() const
{
    return m_colDrivers.c4ElemCount();
}


// Allow direct access to the field list for a given driver
const tCQCKit::TFldDefList&
TCQCFldCache::colFieldListFor(const TString& strMoniker) const
{
    // Look up the driver item for the moniker.
    const TFICacheDrvItem& fcdiDev = *pfcdiFindMoniker(strMoniker);

    // And return a const reference to its field list
    return fcdiDev.colFldList();
}


// Return the device category of the indicate driver
tCQCKit::EDevCats TCQCFldCache::eCategory(const TString& strMoniker) const
{
    // Throw if not found, don't need to fault in fields if not already done
    const TFICacheDrvItem* pfcdiDev = pfcdiFindMoniker
    (
        strMoniker, kCIDLib::True, kCIDLib::False
    );
    return pfcdiDev->eCategory();
}


//
//  Provide read only access to the device classes implemented by the passed moniker.
//  No need to fault in the fields here since this is stuff we get during init.
//
const tCQCKit::TDevClassList&
TCQCFldCache::fcolDevClasses(const TString& strMoniker) const
{
    const TFICacheDrvItem* pfcdiDev = pfcdiFindMoniker
    (
        strMoniker, kCIDLib::True, kCIDLib::False
    );
    return pfcdiDev->fcolDevClasses();
}


// Provide read access to the driver objects
TCQCFldCache::TDrvCursor TCQCFldCache::cursDevs() const
{
    return TCQCFldCache::TDrvCursor(&m_colDrivers);
}


//
//  Find the field def for a given field, const or non-const access. In
//  this case it has to exist or we throw.
//
const TCQCFldDef&
TCQCFldCache::flddFor(const TString& strMoniker, const TString& strField) const
{
    //
    //  Look up the driver item for the moniker. We throw if not found and
    //  fault in fields is not already done.
    //
    const TFICacheDrvItem& fcdiDev = *pfcdiFindMoniker(strMoniker);
    return *fcdiDev.pflddInfo(strField);
}

TCQCFldDef&
TCQCFldCache::flddFor(const TString& strMoniker, const TString& strField)
{
    //
    //  Look up the driver item for the moniker. We throw if not found and
    //  fault in fields is not already done.
    //
    TFICacheDrvItem& fcdiDev = *pfcdiFindMoniker(strMoniker);
    return *fcdiDev.pflddInfo(strField);
}



//
//  Initialize the cache. What we want to do here is to get a driver info object
//  into place for all available drivers. We can do this fairly efficiently by just
//  getting all of the bindings in the name server scope where all of the drivers
//  are registered. We set up the list and initially mark them all as not faulted
//  in yet. We'll fault them in upon access.
//
//  We can do V2 only filtering here if asked. We just pass the V2 only flag on to
//  the query we make to each unique host.
//
tCIDLib::TVoid
TCQCFldCache::Initialize(       TCQCFldFilter* const    pffiltToAdopt
                        , const tCIDLib::TBoolean       bV2Only)
{
    //
    //  Adopt the filter. Just for safely remove any previous one though there
    //  shouldn't be one.
    //
    delete m_pffiltCriteria;
    m_pffiltCriteria = pffiltToAdopt;

    m_bV2Only = bV2Only;
    LoadDevices();
}


//
//  See if there is a field def for the indicated moniker.field. If so, return
//  a pointer to it, else zero.
//
const TCQCFldDef*
TCQCFldCache::pflddFor( const   TString&            strMoniker
                        , const TString&            strField
                        ,       tCIDLib::TCard4&    c4DriverId) const
{
    // Look up the driver item for the moniker
    const TFICacheDrvItem* pfcdiDev = pfcdiFindMoniker(strMoniker, kCIDLib::False);
    if (!pfcdiDev)
        return 0;
    c4DriverId = pfcdiDev->c4DriverId();
    return pfcdiDev->pflddInfo(strField, kCIDLib::False);
}


//
//  Provide access to the non-field info of a driver. Along with QueryDevList()
//  below, this will allow for iteration of the drivers to get basic info, without
//  faulting in any field info.
//
tCIDLib::TVoid
TCQCFldCache::QueryDevInfo( const   TString&            strMoniker
                            ,       tCQCKit::EDevCats&  eCategory
                            ,       TString&            strMake
                            ,       TString&            strModel) const
{
    // Look for the driver, throw if not found, don't fault in fields
    const TFICacheDrvItem* pfcdiDev = pfcdiFindMoniker
    (
        strMoniker, kCIDLib::True, kCIDLib::False
    );

    eCategory = pfcdiDev->eCategory();
    strMake = pfcdiDev->strMake();
    strModel = pfcdiDev->strModel();
}



//
//  Query all of the device monikers in the cache. Here of course we don't
//  need or want to fault in fields, so we directly iterate the driver list
//  and get just monikers.
//
tCIDLib::TVoid
TCQCFldCache::QueryDevList(TCollection<TString>& colMonikersToFill) const
{
    // Flush the caller's list just in case
    colMonikersToFill.RemoveAll();

    // Iterate our device list and copy monikers out to the caller's list
    m_colDrivers.bForEach
    (
        [&colMonikersToFill](const TFICacheDrvItem& fcdiCur)
        {
            colMonikersToFill.objAdd(fcdiCur.strMoniker());
            return kCIDLib::True;
        }
    );
}


//
//  Return refs to all of the fields of the indicated device that match
//  the requested field access.
//
//  NOTE:   This cache is initialized for a particular access and will only
//          contain fields that match that. So generally you will want to
//          initialize for unrestricted access if you are going to use this
//          filtered query, so that the cache holds all available fields.
//
tCIDLib::TVoid TCQCFldCache::
QueryFieldsFiltered(const   TString&                        strMoniker
                    , const tCQCKit::EReqAccess             eAccess
                    ,       TFICacheDrvItem::TRefFldList&   colToFill) const
{
    // If debugging, make sure the passed collection is non-adopting
    #if CID_DEBUG_ON
    if (colToFill.eAdopt() != tCIDLib::EAdoptOpts::NoAdopt)
    {
        TPopUp::PopUpMsg
        (
            CID_FILE
            , CID_LINE
            , L"Filtered Field Cache Query"
            , 0
            , L"The passed collection must be non-adopting"
        );
        return;
    }
    #endif

    // Look up the driver item for the moniker.
    const TFICacheDrvItem& fcdiDev = *pfcdiFindMoniker(strMoniker);
    const tCQCKit::TFldDefList& colFields = fcdiDev.colFldList();

    // Iterate the list and look for fields that match the criteria
    tCIDLib::TCard4 c4Count = colFields.c4ElemCount();
    colToFill.RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFields[c4Index];
        const tCQCKit::EFldAccess eCurAcc = flddCur.eAccess();

        // If it matches the requested access, then take it
        if (facCQCKit().bCheckFldAccess(eAccess, eCurAcc))
            colToFill.Add(&flddCur);
    }
}


tCIDLib::TVoid TCQCFldCache::
QueryFieldsSorted(  const   TString&                        strMoniker
                    ,       TFICacheDrvItem::TRefFldList&   colToFill) const
{
    // If debugging, make sure the passed collection is non-adopting
    #if CID_DEBUG_ON
    if (colToFill.eAdopt() != tCIDLib::EAdoptOpts::NoAdopt)
    {
        TPopUp::PopUpMsg
        (
            CID_FILE
            , CID_LINE
            , L"Sorted Field Cache Query"
            , 0
            , L"The passed collection must be non-adopting"
        );
        return;
    }
    #endif

    // Look up the driver item for the moniker.
    const TFICacheDrvItem& fcdiDev = *pfcdiFindMoniker(strMoniker);
    const tCQCKit::TFldDefList& colFields = fcdiDev.colFldList();

    // Iterate the list and look for fields that match the criteria
    tCIDLib::TCard4 c4Count = colFields.c4ElemCount();
    colToFill.RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colToFill.Add(&colFields[c4Index]);

    //
    //  Sort them using our special sorting comparitor that deals with V2 vs. V1
    //  fields and such.
    //
    colToFill.Sort(&TCQCFldDef::eICompV2);
}


//
//  We just call the load devices private method, who in this case will just use
//  the info we have stored already.
//
tCIDLib::TVoid TCQCFldCache::Reload()
{
    LoadDevices();
}

//
//  Return a reference to the make or model of the indicated driver. We throw
//  if not found.
//
const TString& TCQCFldCache::strDevMake(const TString& strMoniker) const
{
    const TFICacheDrvItem* pfcdiDev = pfcdiFindMoniker(strMoniker, kCIDLib::True);
    return pfcdiDev->strMake();
}

const TString& TCQCFldCache::strDevModel(const TString& strMoniker) const
{
    const TFICacheDrvItem* pfcdiDev = pfcdiFindMoniker(strMoniker, kCIDLib::True);
    return pfcdiDev->strModel();
}


// ---------------------------------------------------------------------------
//  TCQCFldCache: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Faults in a driver object's list of fields.
//
tCIDLib::TVoid TCQCFldCache::FaultInFields(TFICacheDrvItem& fcdiToLoad) const
{
    // If already loaded, or the binding wasn't set, do nothing
    if (fcdiToLoad.bIsLoaded() || fcdiToLoad.strBinding().bIsEmpty())
        return;

    try
    {
        // See if we can get a client proxy for this one's CQCServer
        TCQCSrvAdminClientProxy* porbcAdmin
        (
            facCQCKit().porbcCQCSrvAdminProxy(fcdiToLoad.strBinding())
        );

        if (!porbcAdmin)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcRem_DrvNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , fcdiToLoad.strMoniker()
            );
        }

        // We got it so make sure it gets cleaned up
        TJanitor<TCQCSrvAdminClientProxy> janProxy(porbcAdmin);

        // Query this guy for his list of fields
        tCQCKit::TFldDefList colCurFlds;
        tCIDLib::TCard4 c4DriverId;
        tCIDLib::TCard4 c4DriverListId;
        tCIDLib::TCard4 c4FldListId;
        tCQCKit::EDrvStates eState;
        porbcAdmin->c4QueryFields
        (
            fcdiToLoad.strMoniker()
            , eState
            , colCurFlds
            , c4FldListId
            , c4DriverId
            , c4DriverListId
        );

        //
        //  Filter and store. Even if the list is empty, still we need
        //  to set it so that it gets marked loaded, and any previous
        //  contents get cleared out and the ids get stored.
        //
        fcdiToLoad.SetFldList
        (
            colCurFlds, c4DriverId, c4FldListId, m_pffiltCriteria
        );
    }

    catch(TError& errToCatch)
    {
        // We couldn't do it for some reason
        if (facCQCKit().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facCQCKit().bLogFailures())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcDrv_RegButNoAccess
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotReady
                , fcdiToLoad.strMoniker()
                , fcdiToLoad.strBinding()
            );
        }

        // Mark this guy as loaded so we won't try again
        fcdiToLoad.MarkLoaded();
    }
}


//
//  This is broken out because we need to access it from both the Initialize() and
//  Reload() methods.
//
tCIDLib::TVoid TCQCFldCache::LoadDevices()
{
    // Clear out any previous stuff just in case
    m_colDrivers.RemoveAll();

    try
    {
        // Get a name server client proxy so we can get the moniker list
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

        //
        //  Enumerate the CQCServer driver objects into our collection. If we don't
        //  get anything, we can just return right now since we are done.
        //
        TVector<TNameServerInfo> colNSItems;
        if (!orbcNS->c4EnumObjects(TCQCSrvAdminClientProxy::strDrvScope, colNSItems, kCIDLib::False))
            return;

        //
        //  Let's iterate the list of items we got and for each one add a driver item to
        //  our list (except in the case of V2 Only mode where we skip V1 drivers.)
        //
        TString strTok;
        TStringTokenizer stokDC;
        tCQCKit::TDevClassList fcolDevClasses;
        const tCIDLib::TCard4 c4Count = colNSItems.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TNameServerInfo& nsviCur = colNSItems[c4Index];

            // Handle device classes if we have any
            fcolDevClasses.RemoveAll();
            if (nsviCur.strExtra4().bIsEmpty())
            {
                // No device classes. If V2 only skip this one
                if (m_bV2Only)
                    continue;

                // Else all through with an empty device class list
            }
            else
            {
                // We have device classes so parse them out
                stokDC.Reset(&nsviCur.strExtra4(), L",");
                while (stokDC.bGetNextToken(strTok))
                {
                    strTok.StripWhitespace();
                    tCQCKit::EDevClasses eCls = tCQCKit::eAltXlatEDevClasses(strTok);
                    if (eCls != tCQCKit::EDevClasses::Count)
                        fcolDevClasses.c4AddElement(eCls);
                }
            }

            // Translate the device category
            tCQCKit::EDevCats eCat = tCQCKit::eXlatEDevCats(nsviCur.strExtra3());
            if (eCat == tCQCKit::EDevCats::Count)
                eCat = tCQCKit::EDevCats::Miscellaneous;

            m_colDrivers.objPlace
            (
                nsviCur.strNodeName()
                , eCat
                , fcolDevClasses
                , nsviCur.strExtra1()
                , nsviCur.strExtra2()
                , nsviCur.strDescription()
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Just clear our list of drivers out
        m_colDrivers.RemoveAll();
    }
}


//
//  Find the driver item for the indicated moniker, const and non-const versions.
//  We can throw if not found or just return a null pointer. We can also be
//  told whether to fault in fields if not already done so, or to just leave
//  it alone.
//
TFICacheDrvItem*
TCQCFldCache::pfcdiFindMoniker( const   TString&            strMoniker
                                , const tCIDLib::TBoolean   bThrowIfNot
                                , const tCIDLib::TBoolean   bFaultInFlds)
{
    TDrvList::TNCCursor cursDrv(&m_colDrivers);
    if (cursDrv.bReset())
    {
        do
        {
            if (cursDrv.objRCur().strMoniker() == strMoniker)
            {
                // If we are to fault in fields and they aren't already
                if (bFaultInFlds && !cursDrv.objWCur().bIsLoaded())
                    FaultInFields(cursDrv.objWCur());

                return &cursDrv.objWCur();
            }
        }   while (cursDrv.bNext());
    }

    if (bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFlC_MonikerNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker
        );
    }
    return 0;
}

const TFICacheDrvItem*
TCQCFldCache::pfcdiFindMoniker( const   TString&            strMoniker
                                , const tCIDLib::TBoolean   bThrowIfNot
                                , const tCIDLib::TBoolean   bFaultInFlds) const
{
    //
    //  We can do a non-const cursor here because the driver list is mutable.
    //  This allows us to fault in fields if needed.
    //
    TDrvList::TNCCursor cursDrv(&m_colDrivers);
    if (cursDrv.bReset())
    {
        do
        {
            if (cursDrv.objRCur().strMoniker() == strMoniker)
            {
                // If we are to fault in fields and they aren't already
                if (bFaultInFlds && !cursDrv.objWCur().bIsLoaded())
                    FaultInFields(cursDrv.objWCur());
                return &cursDrv.objRCur();
            }
        }   while (cursDrv.bNext());
    }

    if (bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFlC_MonikerNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker
        );
    }
    return 0;
}

