//
// FILE NAME: GenProtoS_Maps.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the header file for the GenProtoS_Maps.cpp file, which defines
//  the simple classes that are used by the 'Maps' block of a protocol
//  file. Maps map between text and numeric values. So each entry in a map is
//  a text key and a numeric value.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMapItem
//  PREFIX: mapi
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoMapItem : public TObject, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoMapItem();

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const tCIDLib::TBoolean       bValue
        );

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const tCIDLib::TCard1         c1Value
        );

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const tCIDLib::TCard2         c2Value
        );

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const tCIDLib::TCard4         c4Value
        );

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const tCIDLib::TFloat4        f4Value
        );

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const tCIDLib::TFloat8        f8Value
        );

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const tCIDLib::TInt1          i1Value
        );

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const tCIDLib::TInt2          i2Value
        );

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const tCIDLib::TInt4          i4Value
        );

        TGenProtoMapItem
        (
            const   TString&                strKey
            , const TString&                strValue
        );

        TGenProtoMapItem
        (
            const   TGenProtoMapItem&       mapiToCopy
        );

        ~TGenProtoMapItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoMapItem& operator=
        (
            const   TGenProtoMapItem&       mapiToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TGenProtoExprVal& evalItem() const;

        tGenProtoS::ETypes eType() const;

        tCIDLib::THashVal hshKey() const;

        const TString& strKey() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fvalItem
        //      This is the value for this item. It uses the smart union class
        //      that lets us store any of our valid expression types.
        //
        //  m_hshKey
        //      We pre-hash the key to speed up searches.
        //
        //  m_strKey
        //      The key value for this item, which is the string portion that
        //      we map to.
        // -------------------------------------------------------------------
        TGenProtoExprVal    m_evalItem;
        tCIDLib::THashVal   m_hshKey;
        TString             m_strKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoMapItem,TObject)
        DefPolyDup(TGenProtoMapItem)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMap
//  PREFIX: map
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoMap : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TGenProtoMap&           mapSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoMap
        (
            const   TString&                strName
            , const tGenProtoS::ETypes      eType
        );

        TGenProtoMap
        (
            const   TGenProtoMap&           mapToCopy
        );

        ~TGenProtoMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoMap& operator=
        (
            const   TGenProtoMap&           mapToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddItem
        (
            const   TGenProtoMapItem&       mapiToAdd
        );

        tCIDLib::TVoid AddElseItem
        (
            const   TGenProtoMapItem&       mapiToAdd
        );

        tCIDLib::TVoid BuildLimString
        (
                    TString&                strToFill
        );

        tCIDLib::TBoolean bItemValue
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TCard1 c1ItemValue
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TCard2 c2ItemValue
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TCard4 c4ItemValue
        (
            const   TString&                strKey
        )   const;

        tGenProtoS::ETypes eItemType() const;

        tCIDLib::TFloat4 f4ItemValue
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TFloat8 f8ItemValue
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TInt1 i1ItemValue
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TInt2 i2ItemValue
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TInt4 i4ItemValue
        (
            const   TString&                strKey
        )   const;

        TGenProtoMapItem* pmapiFind
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );

        const TGenProtoMapItem* pmapiFind
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        )   const;

        TGenProtoMapItem* pmapiFindByValue
        (
            const   TGenProtoExprVal&       evalToFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );

        const TGenProtoMapItem* pmapiFindByValue
        (
            const   TGenProtoExprVal&       evalToFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        )   const;

        const TString& strItemValue
        (
            const   TString&                strKey
        )   const;

        const TString& strName() const;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented ctors and operators
        // -------------------------------------------------------------------
        TGenProtoMap();


        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef TVector<TGenProtoMapItem>   TItemList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckType
        (
            const   TGenProtoMapItem&       mapiToCheck
            , const tGenProtoS::ETypes      eToCheck
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bHaveElseClause
        //      Used to indidate of we had an else clause, which means that
        //      the m_mapiElse member is valid.
        //
        //  m_colItems
        //      The collection of map items that holds our map data. Its a
        //      by ref vector of map items. We don't have a by ref hash set
        //      which would otherwise be the appropriate way to do it.
        //
        //  m_eItemType
        //      This guy polymorphically holds all the item types (though only
        //      one type in any one instance), so we have to keep up with what
        //      type we are holding.
        //
        //  m_mapiElse
        //      A map can have an 'else' value which is used if no match is
        //      found. It's only valid if m_bHaveElseClause is set.
        //
        //  m_strName
        //      The name of the map itself.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bHaveElseClause;
        TItemList           m_colItems;
        tGenProtoS::ETypes  m_eItemType;
        TGenProtoMapItem    m_mapiElse;
        TString             m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoMap,TObject)
};

#pragma CIDLIB_POPPACK


