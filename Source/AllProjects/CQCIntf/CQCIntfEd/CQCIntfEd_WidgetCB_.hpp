//
// FILE NAME: CQCIntfEd_WidgetCB_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2004
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
//  This file implements some classes used to manage widgets and widget attributes on the
//  clipboard. TCQCIntfEdWdgCB is used to copy/paste widgets. TCQCIntfAttrPack is used to
//  copy/paste widget attributes.
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
//   CLASS: TCQCIntfAttrPack
//  PREFIX: iap
// ---------------------------------------------------------------------------
class TCQCIntfAttrPack : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfAttrPack();

        TCQCIntfAttrPack(const TCQCIntfAttrPack&) = delete;

        ~TCQCIntfAttrPack();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfAttrPack& operator=(const TCQCIntfAttrPack&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ElemCount() const;

        const tCIDMData::TAttrList& colAttrs() const;

        tCIDMData::TAttrList& colAttrs();

        const TAttrData& attrAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid Reset();


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
        //  m_colAttrs
        //      The selected widget is just allowed to fill in this list with all of his
        //      attributes when a copy is done.
        // -------------------------------------------------------------------
        tCIDMData::TAttrList    m_colAttrs;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        BefriendFactory(TCQCIntfAttrPack)
        RTTIDefs(TCQCIntfAttrPack,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfEdWdgCB
//  PREFIX: wcb
// ---------------------------------------------------------------------------
class TCQCIntfEdWdgCB : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TVector<TKeyValuePair>      TTypeMap;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfEdWdgCB
        (
            const   TCQCIntfTemplate&        iwdgSrc
            , const tCQCIntfEng::TDesIdList& fcolSrc
        );

        TCQCIntfEdWdgCB(const TCQCIntfEdWdgCB&) = delete;

        ~TCQCIntfEdWdgCB();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfEdWdgCB& operator=(const TCQCIntfEdWdgCB&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ElemCount() const;

        const TTypeMap& colTypeMap() const;

        const tCQCIntfEng::TChildList& colWidgets() const;

        TCQCIntfWidget* piwdgAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TCQCIntfWidget* piwdgAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCIntfWidget* piwdgOrphanNext();

        tCIDLib::TVoid Reset();


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQCIntfEdWdgCB();


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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CalcOrigin();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colTypeMap
        //      If widgets are cut and pasted to another template, the original
        //      target names may not be valid. In order to help fix this, we
        //      store the names and cmd target types of each user-named widget
        //      in the source template.
        //
        //      In the target, when pasted, we'll look for any non-system
        //      targets that aren't present and try to find another widget
        //      of the same type to attach it to.
        //
        //  m_colWidgets
        //      The collection of widgets added. It is by reference since we
        //      hold a homogenous list, and we adopt since we make copies of
        //      the added widgets.
        // -------------------------------------------------------------------
        TTypeMap                m_colTypeMap;
        tCQCIntfEng::TChildList m_colWidgets;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        BefriendFactory(TCQCIntfEdWdgCB)
        RTTIDefs(TCQCIntfEdWdgCB,TObject)
};

#pragma CIDLIB_POPPACK

