//
// FILE NAME: CQCKit_FldInfoCache.hpp
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
//  A number of applications might want to present to the user a list of
//  available devices and their available fields. For instance, the user drawn
//  interface designer needs to get all the fields that meet some criteria
//  and offer them to the user to select from. And, during the load of a user
//  interface we have to be able to have all the fields available quickly so
//  that we can initialize field based widgets.
//
//  The caller provides a field filter, so that only fields that offer
//  that kind of access will be provided.
//
//  Performance is an issue here, because we are getting all the fields of
//  all of the drivers. If there are a large number of fields, then this can
//  be a significant burden. In most cases few of them are actually accessed.
//  So, we fault in each driver's worth of fields as they are accessed.
//
//  Each driver item object is marked as to whether it has been loaded yet,
//  and we query its fields when it is first accessed. This can vastly
//  reduce overhead and speed up things like interface loads a lot when there
//  are large numbers of fields. We load all of the found drivers into the
//  cache upon init without loading their fields, so that we know what all the
//  drivers are. This means we can be used to load a driver list without having
//  to load all the fields. When the user selects a driver, the fields can be
//  faulted in.
//
// CAVEATS/GOTCHAS:
//
//  1)  We always want to access drivers using the pfcdiFindMoniker method,
//      so that the drivers fields can be faulted in if not gotten already.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFICacheDrvItem
//  PREFIX: fcdi
// ---------------------------------------------------------------------------
class CQCKITEXPORT TFICacheDrvItem : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TRefFldList = TRefVector<const TCQCFldDef>;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strGetKey
        (
            const   TFICacheDrvItem&        fcdiSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFICacheDrvItem() = delete;

        TFICacheDrvItem
        (
            const   TString&                strMoniker
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevCls
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCQCKit::TFldDefList&   colFields
        );

        TFICacheDrvItem
        (
            const   TString&                strMoniker
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevCls
            , const TString&                strBinding
        );

        TFICacheDrvItem
        (
            const   TString&                strMoniker
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevCls
            , const TString&                strMake
            , const TString&                strModel
            , const TString&                strBinding
        );

        TFICacheDrvItem
        (
            const   TFICacheDrvItem&        fcdiSrc
        );

        TFICacheDrvItem(TFICacheDrvItem&&) = delete;

        ~TFICacheDrvItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFICacheDrvItem& operator=
        (
            const   TFICacheDrvItem&        fcdiSrc
        );

        TFICacheDrvItem& operator=(TFICacheDrvItem&&);


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFldExists
        (
            const   TString&                strField
            ,       TCQCFldDef&             flddToFill
        )   const;

        tCIDLib::TBoolean bFldWithAccess
        (
            const   tCQCKit::EReqAccess     eReqAcc
            ,       TString&                strMoniker
            ,       TString&                strName
        )   const;

        tCIDLib::TBoolean bImplementsClass
        (
            const   tCQCKit::EDevClasses    eClass
        )   const;

        tCIDLib::TBoolean bIsEmpty() const;

        tCIDLib::TBoolean bIsLoaded() const;

        tCIDLib::TBoolean bIsV2() const;

        tCIDLib::TCard4 c4DriverId() const;

        tCIDLib::TCard4 c4FldListId() const;

        tCQCKit::TFldDefList& colFldList();

        const tCQCKit::TFldDefList& colFldList() const;

        tCQCKit::EDevCats eCategory() const;

        const tCQCKit::TDevClassList& fcolDevClasses() const;

        tCIDLib::TVoid MarkLoaded();

        TCQCFldDef* pflddInfo
        (
            const   TString&                strField
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        const TCQCFldDef* pflddInfo
        (
            const   TString&                strField
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        const TString& strBinding() const;

        const TString& strMake() const;

        const TString& strModel() const;

        const TString& strMoniker() const;

        tCIDLib::TVoid SetFldList
        (
            const   tCQCKit::TFldDefList&   colSrc
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4DriverId
            , const TCQCFldFilter* const    pffiltCriteria
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLoaded
        //      This is used for lazily faulting in the fields for each
        //      driver when the driver is accessed. We could use the driver
        //      and field list id, but it could be that we tried already but
        //      the driver was not found. So weon't want to keep trying over
        //      and over.
        //
        //  m_c4DriverId
        //      The id of this driver that we got when we set up the cache.
        //
        //  m_c4FldListId
        //      This is the field list id for this driver that we got when
        //      we set up the cache.
        //
        //  m_colFields
        //      A vector of field descriptions, which describe the fields
        //      avalable for this interface id.
        //
        //  m_eCategory
        //      The device category of this driver.
        //
        //  m_fcolDevClasses
        //      The device classes implemented by this driver, if any. If not, it
        //      is a V1 type driver. If it does, it's V2.
        //
        //  m_strBinding
        //      The name server binding to use later when we need to fault
        //      this one in, so we don't have to redundantly look it up
        //      again. We'll have it during the init, so it's just stored
        //      away here for later use.
        //
        //  m_strMake
        //  m_strModel
        //      We save away the make and model for future use if desired.
        //
        //  m_strMoniker
        //      This the moniker for the driver that these fields are for.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLoaded;
        tCIDLib::TCard4         m_c4DriverId;
        tCIDLib::TCard4         m_c4FldListId;
        tCQCKit::TFldDefList    m_colFields;
        tCQCKit::EDevCats       m_eCategory;
        tCQCKit::TDevClassList  m_fcolDevClasses;
        TString                 m_strBinding;
        TString                 m_strMake;
        TString                 m_strModel;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TFICacheDrvItem,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldCache
//  PREFIX: cfc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldCache : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Class types. This has to be public since we can return a cursor
        //  for our list of driver items.
        // -------------------------------------------------------------------
        using TDrvList = TKeyedHashSet<TFICacheDrvItem, TString, TStringKeyOps>;
        using TDrvCursor = TDrvList::TCursor;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldCache();

        TCQCFldCache(const TCQCFldCache&) = delete;
        TCQCFldCache(TCQCFldCache&&) = delete;

        ~TCQCFldCache();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldCache& operator=(const TCQCFldCache&) = delete;
        TCQCFldCache& operator=(TCQCFldCache&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDevExists
        (
            const   TString&                strMoniker
        )   const;

        tCIDLib::TBoolean bDevExists
        (
            const   TString&                strMoniker
            ,       TString&                strMake
            ,       TString&                strModel
            ,       tCQCKit::EDevCats&      eDevCat
        )   const;

        tCIDLib::TBoolean bFldExists
        (
            const   TString&                strMoniker
            , const TString&                strField
        )   const;

        tCIDLib::TBoolean bFldExists
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       TCQCFldDef&             flddToFill
        )   const;

        tCIDLib::TBoolean bHasFieldsWithAccess
        (
            const   TString&                strMoniker
            , const tCQCKit::EReqAccess     eAccess
        )   const;

        tCIDLib::TBoolean bIsV2Compliant
        (
            const   TString&                strMoniker
        )   const;

        tCIDLib::TBoolean bQueryDefField
        (
            const   TString&                strOrgMoniker
            ,       TString&                strMoniker
            ,       TString&                strField
            , const tCQCKit::EReqAccess     eAccess = tCQCKit::EReqAccess::ReadOrWrite
        )   const;

        tCIDLib::TBoolean bQueryDevList
        (
                    TCollection<TString>&   colMonikersToFill
            , const tCQCKit::EDevClasses    eDevClass
        )   const;

        tCIDLib::TCard4 c4DevCount() const;

        const tCQCKit::TFldDefList&    colFieldListFor
        (
            const   TString&                strMoniker
        )   const;

        tCQCKit::EDevCats eCategory
        (
            const   TString&                strMoniker
        )   const;

        const tCQCKit::TDevClassList& fcolDevClasses
        (
            const   TString&                strMoniker
        )   const;

        TDrvCursor cursDevs() const;

        const TCQCFldDef& flddFor
        (
            const   TString&                strMoniker
            , const TString&                strField
        )   const;

        TCQCFldDef& flddFor
        (
            const   TString&                strMoniker
            , const TString&                strField
        );

        tCIDLib::TVoid Initialize
        (
                    TCQCFldFilter* const    pffiltToAdopt
            , const tCIDLib::TBoolean       bV2Only = kCIDLib::False
        );

        const TCQCFldDef* pflddFor
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       tCIDLib::TCard4&        c4DriverId
        )   const;

        tCIDLib::TVoid QueryDevInfo
        (
            const   TString&                strMoniker
            ,       tCQCKit::EDevCats&      eCategory
            ,       TString&                strMake
            ,       TString&                strModel
        )   const;

        tCIDLib::TVoid QueryDevList
        (
                    TCollection<TString>&   colMonikersToFill
        )   const;

        tCIDLib::TVoid QueryFieldsFiltered
        (
            const   TString&                strMoniker
            , const tCQCKit::EReqAccess     eAccess
            ,       TFICacheDrvItem::TRefFldList& colToFill
        )   const;

        tCIDLib::TVoid QueryFieldsSorted
        (
            const   TString&                strMoniker
            ,       TFICacheDrvItem::TRefFldList& colToFill
        )   const;

        tCIDLib::TVoid Reload();

        const TString& strDevMake
        (
            const   TString&                strMoniker
        )   const;

        const TString& strDevModel
        (
            const   TString&                strMoniker
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FaultInFields
        (
                    TFICacheDrvItem&        fcdiToLoad
        )   const;

        tCIDLib::TVoid LoadDevices();

        TFICacheDrvItem* pfcdiFindMoniker
        (
            const   TString&                strMoniker
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
            , const tCIDLib::TBoolean       bFaultIn = kCIDLib::True
        );

        const TFICacheDrvItem* pfcdiFindMoniker
        (
            const   TString&                strMoniker
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
            , const tCIDLib::TBoolean       bFaultIn = kCIDLib::True
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bV2Only
        //      The caller can ask us to do only V2 compliant fields or not.
        //
        //  m_colDrivers
        //      This collection is a map of TFICacheDrvItem objects. The key
        //      is the driver moniker, so we can get both the list of device
        //      monikers and their fields from this list.
        //
        //      This must be mutable so that we can fault in the fields of a
        //      driver upon access, which may need to be done from an otherwise
        //      const method.
        //
        //  m_pffiltCriteria
        //      Because we fault in fields upon access, the caller gives us a
        //      field filter object to use, which we adopt. It can be empty
        //      if they want all fields.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bV2Only;
        mutable TDrvList    m_colDrivers;
        TCQCFldFilter*      m_pffiltCriteria;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldCache,TObject)
};

#pragma CIDLIB_POPPACK


