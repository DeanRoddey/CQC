//
// FILE NAME: CQCIntfEng_ButtonBase.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/13/2002
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
//  This is the header for the CQCIntfEng_ButtonBase.cpp file, which
//  implements the TCQCIntfButtonBase class. This is a base class that does
//  the grunt work portions of a pushable button style widget. Each derived
//  class just has to know how to display itself with a couple of different
//  styles that represent normal, pushed, and disabled/enabled versions, and
//  what to do when the button is clicked. Otherwise, this class handles the
//  reset of the work.
//
//  At this level, since we don't know anything about display issues, we don't
//  store any text or anything like that. There will be text based, icon
//  based, bitmap based, etc... buttons that derive from this class and they
//  all have to know how to display themselves.
//
// CAVEATS/GOTCHAS:
//
//  1)  All the button derivatives are required to accept focus, so we handle
//      the bAcceptsFocus() method for them. They must handle the got/lost
//      focus methods and show/clear their focus emphasis appropriately.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfButtonBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfButtonBase : public TCQCIntfWidget
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TCQCIntfButtonBase() = delete;

        ~TCQCIntfButtonBase();


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfButtonBase
        (
            const   tCQCIntfEng::ECapFlags  eCapFlags
            , const TString&                strTypeName
        );

        TCQCIntfButtonBase
        (
            const   TCQCIntfButtonBase&     iwdgToCopy
        );

        TCQCIntfButtonBase& operator=
        (
            const   TCQCIntfButtonBase&     iwdgToAssign
        );


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


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPressed() const;

        tCIDLib::TBoolean bPressed
        (
            const   tCIDLib::TBoolean       bPressed
        );

        tCIDLib::TVoid DoClickFeedback();

        tCQCIntfEng::EAutoReps eAutoRepeat() const;

        tCQCIntfEng::EAutoReps eAutoRepeat
        (
            const   tCQCIntfEng::EAutoReps eToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bPressed
        //      This used by derived classes to mark themselves as pressed
        //      and they can choose to display differnetly if so. Some many
        //      not use it, and some may only set it, redraw, then clear it
        //      and redraw, to provide clicked feedback. This is runtime only.
        //
        //  m_eAutoRepeat
        //      Not used anymore. It was primarily for the inc/dec button, and that
        //      was dropped once we moved to 4.5.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bPressed;
        tCQCIntfEng::EAutoReps   m_eAutoRepeat;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfButtonBase,TCQCIntfWidget)
};

#pragma CIDLIB_POPPACK


