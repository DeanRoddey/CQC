//
// FILE NAME: CQCIntfEng_StaticIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/07/2008
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
//  This is the header for the CQCIntfEng_StaticIntf.cpp file, which implements
//  a simple mixin interface that static based widgets use if they need to have
//  a configurable numeric range or need to know what data type the value will
//  be (or limit it to a set of data types), and a most do.
//
//  This let's us have a standard configuration tab for all those widgets to
//  get this info. The including class overrides some methods and provides
//  the possible configuration options to the config tab, which gets it through
//  this interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfStaticIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfStaticIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~MCQCIntfStaticIntf();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid SetStaticInfo
        (
            const   tCIDLib::TFloat8        f8Min
            , const tCIDLib::TFloat8        f8Max
            , const tCQCKit::EFldTypes      eType
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSameStatic
        (
            const   MCQCIntfStaticIntf&     miwdgSrc
        )   const;

        tCQCKit::EFldTypes eDataType() const;

        tCIDLib::TFloat8 f8RangeMax() const;

        tCIDLib::TFloat8 f8RangeMin() const;

        tCIDLib::TVoid QueryStaticAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const;

        tCIDLib::TVoid ReadInStatic
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid SetStaticAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid WriteOutStatic
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        MCQCIntfStaticIntf
        (
            const   tCQCKit::EFldTypes      eType
        );

        MCQCIntfStaticIntf
        (
            const   MCQCIntfStaticIntf&     iwdgToCopy
        );

        MCQCIntfStaticIntf& operator=
        (
            const   MCQCIntfStaticIntf&     iwdgToAssign
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eType
        //      The data type that the value will have, which controls the
        //      legal values of the range.
        //
        //  m_f8CfgMin
        //  m_f8CfgMax
        //      The configured min/max values. They are floating point because
        //      we have to be able to deal with either float or int/card based
        //      ranges.
        // -------------------------------------------------------------------
        tCQCKit::EFldTypes  m_eType;
        tCIDLib::TFloat8    m_f8RangeMax;
        tCIDLib::TFloat8    m_f8RangeMin;
};

#pragma CIDLIB_POPPACK


