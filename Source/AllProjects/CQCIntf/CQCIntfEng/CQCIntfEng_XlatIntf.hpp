//
// FILE NAME: CQCIntfEng_XlatIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/17/2013
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
//  This is the header for the CQCIntfEng_XlatIntf.cpp file, which implements
//  a simple mixin interface that some widgets use. It allows them to support
//  a list of translation values, which is generally used to translate a field
//  or variable value into something else for display.
//
//  We provide a method to do the standard formatting that would be normally
//  desired by a widget that uses this mixin.
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
//   CLASS: MCQCIntfXlatIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfXlatIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~MCQCIntfXlatIntf();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddXlatItem
        (
            const   TString&                strKey
            , const TString&                strValue
        );

        tCIDLib::TBoolean bHaveXlats() const;

        tCIDLib::TBoolean bRemoveXlatKey
        (
            const   TString&                strToRemove
        );

        tCIDLib::TBoolean bSameXlats
        (
            const   MCQCIntfXlatIntf&       miwdgSrc
        )   const;

        tCIDLib::TCard4 c4QueryXlatKeys
        (
                    tCIDLib::TStrCollect&   colToFill
        )   const;

        tCIDLib::TVoid FmtXlatText
        (
            const   TString&                strCaption
            , const TString&                strValue
            , const tCIDLib::TBoolean       bErrorState
            ,       TString&                strToFill
        );

        [[nodiscard]] TColCursor<TKeyValuePair>* pcursMakeNew() const;

        const TKeyValuePair* pkvalElseXlatVal() const;

        const TKeyValuePair* pkvalEmptyXlatVal() const;

        const TKeyValuePair* pkvalFindXlat
        (
            const   TString&                strKey
            , const tCIDLib::TBoolean       bUseElse = kCIDLib::True
        )   const;

        tCIDLib::TVoid QueryXlatAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const;

        tCIDLib::TVoid ReadInXlats
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SetXlatAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid SetXlatValue
        (
            const   TString&                strKey
            , const TString&                strNewVal
        );

        tCIDLib::TVoid UpdateXlatItem
        (
            const   TString&                strOldKey
            , const TString&                strNewKey
            , const TString&                strValue
        );

        tCIDLib::TVoid WriteOutXlats
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


        //
        //  Just to support a refactor in 4.2.918 to create this xlat stuff
        //  as a separate mixin. Used to be just in the dynamic text widget.
        //
        tCIDLib::TVoid RefactorRead
        (
                    TBinInStream&           strmToReadFrom
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        MCQCIntfXlatIntf();

        MCQCIntfXlatIntf
        (
            const   MCQCIntfXlatIntf&     iwdgToCopy
        );

        MCQCIntfXlatIntf& operator=
        (
            const   MCQCIntfXlatIntf&     iwdgToAssign
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colXlats
        //      The list of translation values, in a hash set, keyed on the
        //      key value for fast lookup.
        //
        //  m_strXXXKey
        //      Some special keys we support specifically if the user wants
        //      to use them.
        // -------------------------------------------------------------------
        tCIDLib::TKVHashSet m_colXlats;
        const TString       m_strElseKey;
        const TString       m_strEmptyKey;
};

#pragma CIDLIB_POPPACK



