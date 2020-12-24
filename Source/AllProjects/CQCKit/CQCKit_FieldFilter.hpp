//
// FILE NAME: CQCKit_FieldFilter.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2008
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
//  This is the header for the CQCKit_FieldFilter.cpp file, which implements
//  a set of classes that provide filtering of fields. There are various
//  scenarios field selection where fields are selected and we only want
//  the user to be presented with those that meet particular critera.
//
//  These filters are accepted by various dialog boxes and whatnot to allow
//  you to filter for fields of various types. We provide  base class and
//  some obvious derivatives but you can create any specific filter derivatives
//  you want.
//
//  At the base filter class level only the field access is filtered for.
//  Derived classes add any other filtering they want, call the base class
//  first and then if that worked, then apply their own filtering to further
//  restrict the selection.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFilter
//  PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFilter : public TObject, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFilter
        (
            const   tCQCKit::EReqAccess     eReqAccess
        );

        TCQCFldFilter
        (
            const   TCQCFldFilter&          ffiltSrc
        );

        ~TCQCFldFilter();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFilter& operator=
        (
            const   TCQCFldFilter&          fffiltSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldFilter&          fffiltSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldFilter&          fffiltSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetReject
        (
            const   TString&                strMoniker
            , const TString&                strField
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eReqAccess
        //      The requested field access. This is the most fundamental
        //      filter type and is always applied.
        //
        //  m_strFldRej
        //  m_strMonRej
        //      We allow any of them to support rejecting anything that has
        //      a particular moniker, field name, or both by setting one or
        //      both of these. If empty they will have no effect.
        // -------------------------------------------------------------------
        tCQCKit::EReqAccess m_eReqAccess;
        TString             m_strFldRej;
        TString             m_strMonRej;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFilter,TObject)
        DefPolyDup(TCQCFldFilter)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltNull
//  PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFiltNull : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFiltNull();

        TCQCFldFiltNull
        (
            const   TCQCFldFiltNull&        ffiltSrc
        );

        ~TCQCFldFiltNull();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFiltNull& operator=
        (
            const   TCQCFldFiltNull&        ffiltSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldFiltNull&        fffiltSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldFiltNull&        fffiltSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFiltNull, TCQCFldFilter)
        DefPolyDup(TCQCFldFiltNull)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltNumeric
//  PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFiltNumeric : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFiltNumeric
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCIDLib::TBoolean       bAllowFloat
        );

        TCQCFldFiltNumeric
        (
            const   TCQCFldFiltNumeric&     ffiltSrc
        );

        ~TCQCFldFiltNumeric();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFiltNumeric& operator=
        (
            const   TCQCFldFiltNumeric&     ffiltSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldFiltNumeric&     ffiltSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldFiltNumeric&     ffiltSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAllowFloat
        //      Indicates whehter we should only allow int/card, or whether
        //      float is OK as well.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bAllowFloat;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFiltNumeric, TCQCFldFilter)
        DefPolyDup(TCQCFldFiltNumeric)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltNumRange
//  PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFiltNumRange : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFiltNumRange
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCIDLib::TFloat8        f8MinRange
            , const tCIDLib::TFloat8        f8MaxRange
            , const tCIDLib::TBoolean       bAllowFloat
        );

        TCQCFldFiltNumRange
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCIDLib::TFloat8        f8MinRange
            , const tCIDLib::TFloat8        f8MaxRange
            , const tCQCKit::EFldSTypes     eSemType
            , const tCIDLib::TBoolean       bAllowFloat
        );

        TCQCFldFiltNumRange
        (
            const   TCQCFldFiltNumRange&    ffiltSrc
        );

        ~TCQCFldFiltNumRange();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFiltNumRange& operator=
        (
            const   TCQCFldFiltNumRange&    ffiltSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldFiltNumRange&    ffiltSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldFiltNumRange&    ffiltSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAllowFloat
        //      Indicates whehter we should only allow int/card, or whether
        //      float is OK as well.
        //
        //  m_eSemType
        //      We allow optional filtering on semantic field type. If it's
        //      not provided (equal to the _Count value), we ignore it.
        //
        //  m_f8MaxRange
        //  m_f8MinRange
        //      The max and min spread we will accept for the range. Only the
        //      max is used for Floating point values, since the ranges for
        //      floats can be very small (e.g. 0.0 to 0.5 or something like
        //      that.)
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bAllowFloat;
        tCQCKit::EFldSTypes m_eSemType;
        tCIDLib::TFloat8    m_f8MaxRange;
        tCIDLib::TFloat8    m_f8MinRange;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFiltNumRange, TCQCFldFilter)
        DefPolyDup(TCQCFldFiltNumRange)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltLimTypes
//  PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFiltLimTypes : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFiltLimTypes
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCIDLib::TStrList&      colLimitTypes
        );

        TCQCFldFiltLimTypes
        (
            const   TCQCFldFiltLimTypes&    ffiltSrc
        );

        ~TCQCFldFiltLimTypes();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFiltLimTypes& operator=
        (
            const   TCQCFldFiltLimTypes&    ffiltSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldFiltLimTypes&    ffiltSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldFiltLimTypes&    ffiltSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colLimitTypes
        //      A list of the limit types we'll accept, i.e. Range, Enum, and
        //      for forth.
        // -------------------------------------------------------------------
        tCIDLib::TStrList   m_colLimitTypes;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFiltLimTypes, TCQCFldFilter)
        DefPolyDup(TCQCFldFiltLimTypes)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltTypeName
//  PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFiltTypeName : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFiltTypeName
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
        );

        TCQCFldFiltTypeName
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCQCKit::EFldTypes      eType
        );

        TCQCFldFiltTypeName
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCQCKit::TFldSTypeList& fcolSTypes
        );

        TCQCFldFiltTypeName
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCQCKit::EFldTypes      eType
            , const TString&                strName1
            , const TString&                strName2
        );

        TCQCFldFiltTypeName
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strName1
            , const TString&                strName2
        );

        TCQCFldFiltTypeName
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCQCKit::TFldSTypeList& fcolSTypes
            , const TString&                strName1
            , const TString&                strName2
        );

        TCQCFldFiltTypeName
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strName1
            , const TString&                strName2
        );

        TCQCFldFiltTypeName
        (
            const   tCQCKit::EReqAccess     eReqAccess
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::TFldSTypeList& fcolSTypes
            , const TString&                strName1
            , const TString&                strName2
        );

        TCQCFldFiltTypeName
        (
            const   TCQCFldFiltTypeName&    ffiltSrc
        );

        ~TCQCFldFiltTypeName();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFiltTypeName& operator=
        (
            const   TCQCFldFiltTypeName&    ffiltSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldFiltTypeName&    ffiltSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldFiltTypeName&    ffiltSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eType
        //      The field type we filter for. If set to the _Count value,
        //      it is ignored.
        //
        //  m_fcolSTypes
        //      The semantic field types we filter for, if any.
        //
        //  m_strName1
        //  m_strName2
        //      If either of these is non-empty, then we only accept fields
        //      of the indicated type which have one of these names.
        // -------------------------------------------------------------------
        tCQCKit::EFldTypes      m_eType;
        tCQCKit::TFldSTypeList  m_fcolSTypes;
        TString                 m_strName1;
        TString                 m_strName2;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFiltTypeName, TCQCFldFilter)
        DefPolyDup(TCQCFldFiltTypeName)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltDevCat
//  PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFiltDevCat : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFiltDevCat
        (
            const   TString&                strName1
            , const TString&                strName2
            , const tCQCKit::EDevCats       eDevCat
            , const tCQCKit::EFldTypes      eType = tCQCKit::EFldTypes::Count
            , const tCQCKit::EReqAccess     eReqAccess = tCQCKit::EReqAccess::ReadOrWrite
        );

        TCQCFldFiltDevCat
        (
            const   TCQCFldFiltDevCat&      ffiltSrc
        );

        ~TCQCFldFiltDevCat();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFiltDevCat& operator=
        (
            const   TCQCFldFiltDevCat&      ffiltSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldFiltDevCat&      ffiltSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldFiltDevCat&      ffiltSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eDevCat
        //      The device category that we require the driver to be.
        //
        //  m_eType
        //      The field type we filter for. If set to the _Count value,
        //      it is ignored.
        //
        //  m_strName1
        //      We can look for one or two fields by name. If either of these
        //      is non-empty, then the field must match one of them.
        // -------------------------------------------------------------------
        tCQCKit::EDevCats   m_eDevCat;
        tCQCKit::EFldTypes  m_eType;
        TString             m_strName1;
        TString             m_strName2;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFiltDevCat, TCQCFldFilter)
        DefPolyDup(TCQCFldFiltDevCat)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltDevCls
//  PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFiltDevCls : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFiltDevCls
        (
            const   tCQCKit::EDevClasses    eDevClass
            , const tCQCKit::EFldSTypes     eSType = tCQCKit::EFldSTypes::Count
            , const tCQCKit::EFldTypes      eType = tCQCKit::EFldTypes::Count
            , const tCQCKit::EReqAccess     eReqAccess = tCQCKit::EReqAccess::ReadOrWrite
        );

        TCQCFldFiltDevCls
        (
            const   tCQCKit::EDevClasses    eDevClass
            , const tCQCKit::TFldSTypeList& fcolSTypes
            , const tCQCKit::EFldTypes      eType = tCQCKit::EFldTypes::Count
            , const tCQCKit::EReqAccess     eReqAccess = tCQCKit::EReqAccess::ReadOrWrite
        );

        TCQCFldFiltDevCls
        (
            const   tCQCKit::TDevClassList& fcolList
            , const tCIDLib::TBoolean       bMatchAll
            , const tCQCKit::TFldSTypeList& fcolSTypes
            , const tCQCKit::EFldTypes      eType = tCQCKit::EFldTypes::Count
            , const tCQCKit::EReqAccess     eReqAccess = tCQCKit::EReqAccess::ReadOrWrite
        );

        TCQCFldFiltDevCls
        (
            const   TCQCFldFiltDevCls&      ffiltSrc
        );

        ~TCQCFldFiltDevCls();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFiltDevCls& operator=
        (
            const   TCQCFldFiltDevCls&      ffiltSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldFiltDevCls&      ffiltSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldFiltDevCls&      ffiltSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strMoniker() const;

        const TString& strMoniker
        (
            const   TString&                strToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMatchAll
        //      The caller indicates if we have to match all or just match one
        //      of the classes in the list in order to accept.
        //
        //  m_eType
        //      The field type we filter for. If set to the _Count value,
        //      it is ignored.
        //
        //  m_fcolClasses
        //      The list of device classes that caller wants to include in the
        //      filtering.
        //
        //  m_fcolSTypes
        //      If this list is not empty, then the field must match one of the
        //      semantic fields types in this list.
        //
        //  m_strMoniker
        //      We can filter on moniker. If empty, we don't use it.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bMatchAll;
        tCQCKit::EFldTypes      m_eType;
        tCQCKit::TDevClassList  m_fcolClasses;
        tCQCKit::TFldSTypeList  m_fcolSTypes;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFiltDevCls, TCQCFldFilter)
        DefPolyDup(TCQCFldFiltDevCls)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltEnumLim
//  PREFIX: ffilt
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFiltEnumLim : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFiltEnumLim
        (
            const   tCIDLib::TCard4         c4MinVals
            , const tCIDLib::TCard4         c4MaxVals
            , const tCQCKit::EReqAccess     eReqAccess
        );

        TCQCFldFiltEnumLim
        (
            const   TCQCFldFiltEnumLim&     ffiltSrc
        );

        ~TCQCFldFiltEnumLim();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFiltEnumLim& operator=
        (
            const   TCQCFldFiltEnumLim&     ffiltSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCFldFiltEnumLim&     ffiltSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCFldFiltEnumLim&     ffiltSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const tCQCKit::EDevCats       eCategory
            , const tCQCKit::TDevClassList& fcolDevClasses
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4MaxVals
        //  m_c4MinVals
        //      Indicates the minimum and maximum number of enumerated values
        //      the field must have in order to be acceptable. They can be
        //      max card if you don't want one or both of them checked.
        //
        //  m_fldlTmp
        //      A temp enumerated field limit we use to parse the out the
        //      field limit value if the caller provides max/min values we
        //      have to check. It has to be mutable since we use it in the
        //      bCanAcceptField callback.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4MaxVals;
        tCIDLib::TCard4             m_c4MinVals;
        mutable TCQCFldEnumLimit    m_fldlTmp;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFiltEnumLim, TCQCFldFilter)
        DefPolyDup(TCQCFldFiltEnumLim)
};

#pragma CIDLIB_POPPACK

