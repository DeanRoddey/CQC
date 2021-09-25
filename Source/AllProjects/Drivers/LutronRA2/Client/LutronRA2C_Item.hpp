//
// FILE NAME: LutronRA2C_Item.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/06/2012
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
//  This is the header for the LutronRA2C_Item.cpp file, which implements a small class
//  that represents a dimmer, button, keypad, etc... . We just need something to keep
//  lists of the configured items so that we can reasononably check for changes.
//
//  Then we have another class that holds the overall configuration, which is a set of
//  lists, once for each type we support.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TRA2CItem
//  PREFIX: item
// ---------------------------------------------------------------------------
class TRA2CItem : public TObject, public MFormattable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bParseAddr
        (
            const   TString&                strToParse
            ,       tCIDLib::TCard4&        c4IntID
            ,       tCIDLib::TCard4&        c4CompID
            , const tLutronRA2C::EItemTypes eType
            ,       TString&                strErrMsg
        );

        static tCIDLib::ESortComps eComp
        (
            const   TRA2CItem&              item1
            , const TRA2CItem&              item2
        );

        static tCIDLib::TVoid FormatAddr
        (
            const   tCIDLib::TCard4         c4IntID
            , const tCIDLib::TCard4         c4CompID
            ,       TString&                strToFill
        );



        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRA2CItem();

        TRA2CItem
        (
            const   TString&                strName
            , const tLutronRA2C::EItemTypes eType
        );

        TRA2CItem(const TRA2CItem&) = default;
        TRA2CItem(TRA2CItem&&) = default;

        ~TRA2CItem() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRA2CItem& operator=(const TRA2CItem&) = default;
        TRA2CItem& operator=(TRA2CItem&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TRA2CItem&              itemSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TRA2CItem&              itemSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddToList
        (
                    TMultiColListBox&       wndTar
            ,       tLutronRA2C::TColStrs&  colTmp
        );

        tCIDLib::TVoid AddToList
        (
                    TMultiColListBox&       wndTar
        );

        tCIDLib::TBoolean bIsValid
        (
                    TString&                strErrMsg
        )   const;

        tCIDLib::TBoolean bParseFromCfgLine
        (
            const   tLutronRA2C::EItemTypes eType
            , const TString&                strSrc
            ,       TString&                strErrMsg
        );

        tCIDLib::TCard4 c4CompID() const;

        tCIDLib::TCard4 c4IntID() const;

        tLutronRA2C::EItemTypes eType() const;

        tCIDLib::TVoid ParseFromLB
        (
                    TMultiColListBox&       wndTar
            , const tCIDLib::TCard4         c4Index
        );

        const TString& strAddr() const
        {
            return m_strAddr;
        }

        const TString& strExtra() const;

        const TString& strExtra
        (
            const   TString&                strToSet
        );

        const TString& strName() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatTo
        (
                    TTextOutStream&         strmDest
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSendTrig
        //      Only applies to switches and occupancy sensors and indicates if they
        //      should send a standard trigger.
        //
        //  m_c4CompID
        //  m_c4IntID
        //      The Lutron ids of the item.
        //
        //  m_eType
        //      The item type. We just keep one big list, so this tells us what type
        //      it is.
        //
        //  m_strAddr
        //      The formatted out address. Any time either of the ID values above are
        //      changed, this is updated for convenience.
        //
        //  m_strExtra
        //      Some have an extra value only applicable to that type.
        //
        //  m_strName
        //      The base name of the item, as give
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bSendTrig;
        tCIDLib::TCard4         m_c4CompID;
        tCIDLib::TCard4         m_c4IntID;
        tLutronRA2C::EItemTypes m_eType;
        TString                 m_strAddr;
        TString                 m_strExtra;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TRA2CItem,TObject)
};

#pragma CIDLIB_POPPACK


