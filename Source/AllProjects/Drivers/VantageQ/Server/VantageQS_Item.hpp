//
// FILE NAME: VantageQS_Item.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2006
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
//  This is the header for the VantageQS_Item.cpp file, which implements the
//  classes that represent named items in the Vantage. We represent the
//  nested structure of the items. So we have Floors, Rooms, Modules, and
//  so forth.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TVantageQSItem
//  PREFIX: item
// ---------------------------------------------------------------------------
class TVantageQSItem : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        virtual ~TVantageQSItem();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard8 c8Addr() const;

        tVantageQS::EAddrTypes eAddrType() const;

        const TString& strName() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TVantageQSItem
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4Master
            , const tCIDLib::TCard4         c4Station
            , const tCIDLib::TCard4         c4ItemNum
        );

        TVantageQSItem
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4Master
            , const tCIDLib::TCard4         c4Enclosure
            , const tCIDLib::TCard4         c4Module
            , const tCIDLib::TCard4         c4Load
        );

        TVantageQSItem
        (
            const   TVantageQSItem&         itemToCopy
        );

        TVantageQSItem& operator=
        (
            const   TVantageQSItem&         itemToAssign
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        //
        //  m_ac4Addrs
        //      An array of 4 address values. According to the m_eAddrType
        //      this will be 3 values (master/station/item) or it will be
        //      4 values, (master/enclosure/module/load).
        //
        //  m_c8Addr
        //      An encoded version of the address, to allow for fast compares
        //      of addresses. Each of the 4 possible address values is given
        //      16 bits, the 0ths (master) is in the highest and moving down
        //      through the others.
        //
        //  m_eAddrType
        //      Indicates which of the addressing modes this item uses.
        //
        //  m_strName
        //      The name of this item.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_ac4Addrs[4];
        tCIDLib::TCard8         m_c8Addr;
        tVantageQS::EAddrTypes  m_eAddrType;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TVantageQSItem,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TVantageQSButton
//  PREFIX: item
// ---------------------------------------------------------------------------
class TVantageQSButton : public TVantageQSItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TVantageQSButton
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4Master
            , const tCIDLib::TCard4         c4Station
            , const tCIDLib::TCard4         c4ItemNum
        );

        TVantageQSButton
        (
            const   TVantageQSButton&       itemToCopy
        );

        ~TVantageQSButton();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TVantageQSButton& operator=
        (
            const   TVantageQSButton&       itemToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TVantageQSButton,TVantageQSItem)
};



// ---------------------------------------------------------------------------
//   CLASS: TVantageQSLoad
//  PREFIX: item
//
//  Loads represent things that can be powered on and off. They can be in
//  enclosures or stations, so they have both types of addresses.
// ---------------------------------------------------------------------------
class TVantageQSLoad : public TVantageQSItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TVantageQSLoad
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4Master
            , const tCIDLib::TCard4         c4Station
            , const tCIDLib::TCard4         c4ItemNum
            , const tVantageQS::ELoadTypes  eType
        );

        TVantageQSLoad
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4Master
            , const tCIDLib::TCard4         c4Enclosure
            , const tCIDLib::TCard4         c4Module
            , const tCIDLib::TCard4         c4Load
            , const tVantageQS::ELoadTypes  eType
        );

        TVantageQSLoad
        (
            const   TVantageQSLoad&         itemToCopy
        );

        ~TVantageQSLoad();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TVantageQSLoad& operator=
        (
            const   TVantageQSLoad&         itemToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eType
        //      The type of load this one is, which indicates what type of
        //      operations are valid for it.
        // -------------------------------------------------------------------
        tVantageQS::ELoadTypes  m_eType;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TVantageQSLoad,TVantageQSItem)
};

#pragma CIDLIB_POPPACK

