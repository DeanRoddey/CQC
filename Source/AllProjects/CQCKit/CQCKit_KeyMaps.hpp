//
// FILE NAME: CQCAct_KeyMaps.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/07/2000
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
//  This is the header for the CQCKit_KeyMaps.cpp file, which implements
//  a set of classes that allow the user to map keyboard keys (with various
//  shift combinations) to invoke standard CQC commands.
//
//  We need the TCQCKeyMapItem class to store the information, both in memory
//  and when streamed to disk. It derives from the base standard command
//  source class. All we need to provide is the means to keep up with which
//  key is associated with one.
//
//  And we need the TCQCKeyMap class which provides the smarts to manage the
//  map at runtime, and which streams the contents in and out to disk.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCKeyMapItem
// PREFIX: kmi
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCKeyMapItem : public TCQCStdCmdSrc
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TCard4 c4FindKeyIndex
        (
            const   tCIDCtrls::EExtKeys     eToCheck
        );

        static tCIDLib::TBoolean bComp
        (
            const   TCQCKeyMapItem&         kmi1
            , const TCQCKeyMapItem&         kmi2
            , const tCIDLib::TCard4
        );


        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCKeyMapItem();

        TCQCKeyMapItem
        (
            const   tCIDCtrls::EExtKeys     eKey
        );

        TCQCKeyMapItem
        (
            const   tCIDCtrls::EExtKeys     eKey
            , const TString&                strTitle
        );

        TCQCKeyMapItem
        (
            const   TCQCKeyMapItem&         kmiSrc
        );

        TCQCKeyMapItem(TCQCKeyMapItem&&) = delete;

        ~TCQCKeyMapItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCKeyMapItem& operator=
        (
            const   TCQCKeyMapItem&         kmiSrc
        );

        TCQCKeyMapItem& operator=(TCQCKeyMapItem&&) = delete;

        tCIDLib::TBoolean operator==
        (
            const   TCQCKeyMapItem&         kmiSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCKeyMapItem&         kmiSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EExtKeys eKey() const;

        tCIDCtrls::EExtKeys eKey
        (
            const   tCIDCtrls::EExtKeys     eNew
        );


    protected           :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eKey
        //      The key that this guy is mapped to.
        // -------------------------------------------------------------------
        tCIDCtrls::EExtKeys m_eKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCKeyMapItem,TCQCStdCmdSrc)
        NulObject(TCQCKeyMapItem)
};




// ---------------------------------------------------------------------------
//  CLASS: TCQCKeyMap
// PREFIX: km
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCKeyMap : public TObject, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        //  Public data types
        // -------------------------------------------------------------------
        using TItemList = TVector<TCQCKeyMapItem>;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bIsLegalKey
        (
            const   tCIDCtrls::EExtKeys     eKey
        );

        static tCIDLib::TCard4 c4KeyCount();

        static const TString& strKeyName
        (
            const   tCIDLib::TCard4         c4Index
        );

        static const TString& strKeyName
        (
            const   tCIDCtrls::EExtKeys     eKey
        );


        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCKeyMap();

        TCQCKeyMap
        (
            const   TCQCKeyMap&             kmSrc
        );

        TCQCKeyMap(TCQCKeyMap&&) = delete;

        ~TCQCKeyMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCKeyMap& operator=
        (
            const   TCQCKeyMap&             kmSrc
        );

        TCQCKeyMap& operator=(TCQCKeyMap&&) = delete;

        tCIDLib::TBoolean operator==
        (
            const   TCQCKeyMap&             kmSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCKeyMap&             kmSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TItemList& colForShifts
        (
            const   tCIDLib::TBoolean       bShift
            , const tCIDLib::TBoolean       bCtrlShift
        )   const;

        TItemList& colForShifts
        (
            const   tCIDLib::TBoolean       bShift
            , const tCIDLib::TBoolean       bCtrlShift
        );

        const TCQCKeyMapItem& kmiFind
        (
            const   tCIDCtrls::EExtKeys     eKey
            , const tCIDLib::TBoolean       bShift
            , const tCIDLib::TBoolean       bCtrlShift
        )   const;

        TCQCKeyMapItem& kmiFind
        (
            const   tCIDCtrls::EExtKeys     eKey
            , const tCIDLib::TBoolean       bShift
            , const tCIDLib::TBoolean       bCtrlShift
        );

        tCIDLib::TVoid Reset();


    protected           :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        TItemList& colFindCol
        (
            const   tCIDLib::TBoolean       bShift
            , const tCIDLib::TBoolean       bCtrlShift
        );

        const TItemList& colFindCol
        (
            const   tCIDLib::TBoolean       bShift
            , const tCIDLib::TBoolean       bCtrlShift
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colAllShifted
        //  m_colCtrlShifted
        //  m_colShifted
        //  m_colUnshifted
        //      These are the collections that hold the items for each shift
        //      combination. Each one holds maps for the same keys, in the
        //      same order, just with different shift combinations.
        // -------------------------------------------------------------------
        TItemList m_colAllShifted;
        TItemList m_colCtrlShifted;
        TItemList m_colShifted;
        TItemList m_colUnshifted;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCKeyMap,TObject)
};

#pragma CIDLIB_POPPACK


