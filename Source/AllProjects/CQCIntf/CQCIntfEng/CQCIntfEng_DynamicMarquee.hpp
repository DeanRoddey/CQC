//
// FILE NAME: CQCIntfEng_DynamicMarquee.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/22/2004
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
//  This is the header for the CQCIntfEng_DynamicMarquee.cpp file, which
//  provides a very simple derivative of the dynamic text widget. The base
//  classes provide all the functionality. All we need to do is just provide
//  new classes for identification purposes, and these guys just pass the
//  correct flags to their parent classes. We create one for field based and
//  one for variable based.
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
//   CLASS: TCQCIntfFldMarquee
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFldMarquee : public TCQCIntfFldText
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFldMarquee();

        TCQCIntfFldMarquee
        (
            const   TCQCIntfFldMarquee&     iwdgToCopy
        );

        ~TCQCIntfFldMarquee();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFldMarquee& operator=
        (
            const   TCQCIntfFldMarquee&     iwdgToAssign
        );


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfFldMarquee,TCQCIntfFldText)
        DefPolyDup(TCQCIntfFldMarquee)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarMarquee
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVarMarquee : public TCQCIntfVarText
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarMarquee();

        TCQCIntfVarMarquee
        (
            const   TCQCIntfVarMarquee&     iwdgToCopy
        );

        ~TCQCIntfVarMarquee();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarMarquee& operator=
        (
            const   TCQCIntfVarMarquee&     iwdgToAssign
        );


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfVarMarquee,TCQCIntfVarText)
        DefPolyDup(TCQCIntfVarMarquee)
};

#pragma CIDLIB_POPPACK


