//
// FILE NAME: LutronHWC_Item.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/09/2006
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
//  This is the header for the LutronHWC_Item.cpp file, which implements
//  a small class that represents a single configured item.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: THWCItem
//  PREFIX: item
// ---------------------------------------------------------------------------
class THWCItem : public TObject, public MFormattable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eComp
        (
            const   THWCItem&               item1
            , const THWCItem&               item2
        );

        static tCIDLib::TBoolean bTypeNeedsNumber
        (
            const   tLutronHWC::EItemTypes  eType
        );

        static tCIDLib::TBoolean bXlatAddr
        (
            const   TString&                strXlat
            , const tLutronHWC::EItemTypes  eType
            ,       tCIDLib::TCard8&        c8ToFill
            ,       tCIDLib::TCard4&        c4AddrBytes
            ,       TString&                strErrMsg
        );

        static tCIDLib::TCard8 c8MakeComboAddr
        (
            const   tLutronHWC::EItemTypes  eType
            , const tCIDLib::TCard8         c8Addr
            , const tCIDLib::TCard1         c1Num
        );

        static tCIDLib::TVoid FormatAddr
        (
            const   tCIDLib::TCard8         c8Addr
            , const tCIDLib::TCard4         c4AddrBytes
            ,       TString&                strToFill
        );

        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THWCItem() = delete;

        THWCItem
        (
            const   TString&                strName
            , const tLutronHWC::EItemTypes  eType
        );

        THWCItem
        (
            const   THWCItem&               itemSrc
        );

        ~THWCItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        THWCItem& operator=
        (
            const   THWCItem&               itemSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   THWCItem&               itemSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   THWCItem&               itemSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddToList
        (
                    TMultiColListBox&       wndTar
            ,       tCIDLib::TStrList&      colTmp
            , const tCIDLib::TBoolean       bSelect
        );

        tCIDLib::TVoid AddToList
        (
                    TMultiColListBox&       wndTar
            , const tCIDLib::TBoolean       bSelect
        );

        tCIDLib::TBoolean bParseFromData
        (
            const   tCIDLib::TStrList&      colTokens
            ,       TString&                strErrMsg
            ,       tCIDLib::TBoolean&      bFixUpAddrs
        );

        tCIDLib::TBoolean bValidate
        (
                    TString&                strErrMsg
        )   const;

        tCIDLib::TCard1 c1Number() const;

        tCIDLib::TCard4 c4AddrBytes() const;

        tCIDLib::TCard8 c8Addr() const;

        tLutronHWC::EItemTypes eType() const;

        tLutronHWC::EItemTypes eType
        (
            const   tLutronHWC::EItemTypes  eToSet
        );

        const TString& strAddr() const;

        const TString& strName() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatTo
        (
                    TTextOutStream&         strmDest
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c1Number
        //      Used for those modules where the can be more than one thing in a single
        //      module, currently for buttons and LEDs.
        //
        //  m_c4AddrBytes
        //      Indicates how many of the byte sin m_c8Addr are used.
        //
        //  m_c8Addr
        //      The Lutron address, which is a sequence of bytes. In Lutron's format it
        //      is x:y:z type format. We put those in right to left, into the lowest
        //      order byte first, then moving up, so x would be in the 3 lowest bytes,
        //      y in the second, and z in the low order byte.
        //
        //      We store the actual number of bytes used in m_c4AddrBytes, to make it
        //      quicker for us to know what bytes are relevant.
        // -------------------------------------------------------------------
        tCIDLib::TCard1         m_c1Number;
        tCIDLib::TCard4         m_c4AddrBytes;
        tCIDLib::TCard8         m_c8Addr;
        tLutronHWC::EItemTypes  m_eType;
        TString                 m_strAddr;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(THWCItem,TObject)
};


#pragma CIDLIB_POPPACK

