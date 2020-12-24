//
// FILE NAME: CQCIntfEng_SActPushButton.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/04/2004
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
//  This is the header for the CQCIntfEng_SActPushButton.cpp file, which
//  implements a simple derivative of the push button class. All we really
//  have to do is to override the TakeClickAction() method and perform the
//  configured action. For this class, that is one of a canned set of
//  special actions, to handle some built in actions, as apposed to user
//  configured actions.
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
//   CLASS: TCQCIntfSActPushButton
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfSActPushButton : public TCQCIntfPushButton
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfSActPushButton();

        TCQCIntfSActPushButton
        (
            const   TCQCIntfSActPushButton& iwdgToCopy
        );

        ~TCQCIntfSActPushButton();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfSActPushButton& operator=
        (
            const   TCQCIntfSActPushButton& iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
        )   override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCIntfEng::ESpecActs eAction() const;

        tCQCIntfEng::ESpecActs eAction
        (
            const   tCQCIntfEng::ESpecActs eToSet
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
        //  m_eAction
        //      The action we are take. Since this button just allows for a
        //      selection from a list of canned actions, we can just store
        //      an enum that indicates which one.
        // -------------------------------------------------------------------
        tCQCIntfEng::ESpecActs  m_eAction;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfSActPushButton,TCQCIntfPushButton)
        DefPolyDup(TCQCIntfSActPushButton)
};

#pragma CIDLIB_POPPACK


