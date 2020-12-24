//
// FILE NAME: CQCIntfEd_ThisFacility.hpp
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
//  This is the header for the CQCIntfEd_ThisFacility.cpp file, which
//  implements the facility class for this program. Since this is a GUI
//  based program, it derives from the TGUIFacility class, instead of the
//  base TFacility class.
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
//   CLASS: TFacCQCIntfEd
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCIntfEd : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCIntfEd();

        ~TFacCQCIntfEd();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TFacCQCIntfEd(const TFacCQCIntfEd&);
        tCIDLib::TVoid operator=(const TFacCQCIntfEd&);


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCIntfEd,TGUIFacility)
};

#pragma CIDLIB_POPPACK


