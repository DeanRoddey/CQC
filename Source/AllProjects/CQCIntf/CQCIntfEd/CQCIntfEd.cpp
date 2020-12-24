//
// FILE NAME: CQCIntfEd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2015
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
//  This is the main module of the program. It just defines the facility
//  object and provides a little other stuff that has no other specific
//  home.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCIntfEd_.hpp"



// ----------------------------------------------------------------------------
//  Global functions
// ----------------------------------------------------------------------------
TFacCQCIntfEd& facCQCIntfEd()
{
    static TFacCQCIntfEd* pfacThis = new TFacCQCIntfEd();
    return *pfacThis;
}

