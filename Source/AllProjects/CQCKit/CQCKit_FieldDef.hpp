//
// FILE NAME: CQCKit_FieldDef.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/22/2001
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
//  This file defines a 'field definition' object. All server side drivers
//  make their data available via 'fields', which are named data points with
//  particular ids, data types, access types, and value limits. This class
//  provides that definition of a field.
//
//  We have a simple derivative that adds a driver moniker, for those times
//  when we need to manage or return a list of fields and the associated
//  driver for each field.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCFldDef
//  PREFIX: fldd
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldDef : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eComp
        (
            const   TCQCFldDef&             fldd1
            , const TCQCFldDef&             fldd2
        );

        static tCIDLib::ESortComps eIComp
        (
            const   TCQCFldDef&             fldd1
            , const TCQCFldDef&             fldd2
        );

        static tCIDLib::ESortComps eICompV2
        (
            const   TCQCFldDef&             fldd1
            , const TCQCFldDef&             fldd2
        );

        static const TString& strKey(const TCQCFldDef& flddSrc);


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldDef();

        TCQCFldDef
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const TString&                strLimits
        );

        TCQCFldDef
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strLimits
        );

        TCQCFldDef
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCIDLib::TCh* const     pszLimits
        );

        TCQCFldDef
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCQCKit::EFldSTypes     eSemType
            , const tCIDLib::TCh* const     pszLimits
        );

        TCQCFldDef
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
        );

        TCQCFldDef
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCQCKit::EFldSTypes     eSemType
        );

        TCQCFldDef(const TCQCFldDef&) = default;
        TCQCFldDef(TCQCFldDef&&) = default;

        ~TCQCFldDef();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQCFldDef& operator=(const TCQCFldDef&) = default;
        TCQCFldDef& operator=(TCQCFldDef&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldDef&             flddSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldDef&             flddSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAlwaysWrite() const;

        tCIDLib::TBoolean bAlwaysWrite
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bFlag() const;

        tCIDLib::TBoolean bFlag
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsEnumType() const;

        tCIDLib::TBoolean bIsNumericType() const;

        tCIDLib::TBoolean bIsReadable() const;

        tCIDLib::TBoolean bIsWriteable() const;

        tCIDLib::TBoolean bPrivate() const;

        tCIDLib::TBoolean bPrivate
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bQueuedWrite() const;

        tCIDLib::TBoolean bQueuedWrite
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bNoStoreOnWrite() const;

        tCIDLib::TBoolean bNoStoreOnWrite
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bStoreClientWrite() const;

        tCIDLib::TCard4 c4Id() const;

        tCIDLib::TCard4 c4Id
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCKit::EFldAccess eAccess() const;

        tCQCKit::EFldSTypes eSemType() const;

        tCQCKit::EFldSTypes eSemType
        (
            const   tCQCKit::EFldSTypes     eToSet
        );

        tCQCKit::EFldTypes eType() const;

        tCQCKit::EFldTypes eType
        (
            const   tCQCKit::EFldTypes      eToSet
        );

        tCIDLib::TVoid QueryNumericRange
        (
                    TString&                strMinVal
            ,       TString&                strMaxVal
        )   const;

        tCIDLib::TVoid Reset();

        const TString& strExtraCfg() const;

        const TString& strExtraCfg
        (
            const   TString&                strToSet
        );

        const TString& strLimits() const;

        const TString& strLimits
        (
            const   TString&                strToSet
        );

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strLimits
            , const tCIDLib::TBoolean       bAlwaysWrite = kCIDLib::False
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const TString&                strLimits
            , const tCIDLib::TBoolean       bAlwaysWrite = kCIDLib::False
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCIDLib::TCh* const     pszLimits
            , const tCIDLib::TBoolean       bAlwaysWrite = kCIDLib::False
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCQCKit::EFldSTypes     eSemType
            , const tCIDLib::TCh* const     pszLimits
            , const tCIDLib::TBoolean       bAlwaysWrite = kCIDLib::False
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCIDLib::TBoolean       bAlwaysWrite = kCIDLib::False
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCQCKit::EFldSTypes     eSemType
            , const tCIDLib::TBoolean       bAlwaysWrite = kCIDLib::False
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAlwaysWrite
        //      Some types of fields are used to cause actions, not to set a
        //      value. For these, we need to supress the normal optimization
        //      that doesn't tell the driver when a client writes to a field
        //      the same value that it already had. This setting does that.
        //
        //  m_bFlag
        //      A non-persistent field available for use by code that processes
        //      field definitions and might need to mark some for whatever reason.
        //
        //  m_bNoStoreOnWrite
        //      In some cases, the driver doesn't want a successful field write to
        //      actually update the field, because all it can do it send the msg,
        //      and depends on a later response from the device to determine the
        //      actual value set. It will update the field when that response comes
        //      in. This also sometimes is important because if the field is set
        //      during the field write, the subsequent actual notification with the
        //      value won't cause an event trigger because now the new value coming
        //      back isn't different from the stored value.
        //
        //  m_bPrivate
        //      If marked private, client apps that display lists of available
        //      fields won't show the field, usually because it is used for
        //      for playing some sort of tricks between the client and server
        //      side drivers and isn't really intended for users.
        //
        //  m_bQueuedWrite
        //      Indicates that this field is handled via queued writes. This
        //      means that CQCServer should not store a written value just
        //      because the driver accepts the written value. The driver will
        //      update the value later when the queued event is processed.
        //
        //  m_c4Id
        //      The server side, which creates these objects, will give each
        //      of its fields an id, to make it far more efficient to read
        //      and write them. They only have to be unique for a particular
        //      driver.
        //
        //  m_eAccess
        //      The type of access this field supports.
        //
        //  m_eSemType
        //      The semantic type of the field. Drivers can set this to
        //      provide hints as to what semantic type the field represents,
        //      if any. This can be used to help the development tools to
        //      be smarter. Defaults to 'generic' if not set.
        //
        //  m_eType
        //      The field type for this field. The data is always passed as
        //      text, but this indicates how to interpret it.
        //
        //  m_strExtraCfg
        //      This allows the server side driver to make some extra info
        //      available, mostly to it's client side driver. What the info
        //      put in here means is between consenting drivers.
        //
        //  m_strLimits
        //      The formatted version of the limits set on this field.
        //
        //  m_strName
        //      The name of the field. It should not have any punctuation or
        //      space, to make it easier to use in macros.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bAlwaysWrite;
        tCIDLib::TBoolean       m_bFlag;
        tCIDLib::TBoolean       m_bNoStoreOnWrite;
        tCIDLib::TBoolean       m_bPrivate;
        tCIDLib::TBoolean       m_bQueuedWrite;
        tCIDLib::TCard4         m_c4Id;
        tCQCKit::EFldAccess     m_eAccess;
        tCQCKit::EFldSTypes     m_eSemType;
        tCQCKit::EFldTypes      m_eType;
        TString                 m_strExtraCfg;
        TString                 m_strLimits;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldDef,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvFldDef
//  PREFIX: dfldd
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCDrvFldDef : public TCQCFldDef
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDrvFldDef() = default;

        TCQCDrvFldDef
        (
            const   TString&                strMoniker
            , const TCQCFldDef&             flddSrc
        );

        TCQCDrvFldDef(const TCQCDrvFldDef&) = default;
        TCQCDrvFldDef(TCQCDrvFldDef&&) = default;

        ~TCQCDrvFldDef() = default;


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQCDrvFldDef& operator=(const TCQCDrvFldDef&) = default;
        TCQCDrvFldDef& operator=(TCQCDrvFldDef&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCDrvFldDef&          dflddSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCDrvFldDef&          dflddSrc
        )   const;



        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strMoniker() const;

        const TString& strMoniker
        (
            const   TString&                strToSet
        );


    protected :
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
        //  m_strMoniker
        //      The moniker of the driver the underlying field is associated
        //      with.
        // -------------------------------------------------------------------
        TString     m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDrvFldDef,TCQCFldDef)
};

#pragma CIDLIB_POPPACK


