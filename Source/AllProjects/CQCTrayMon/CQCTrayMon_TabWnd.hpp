//
// FILE NAME: CQCTrayMon_TabWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2012
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
//  This is the header for the CQCTrayMon_TabWnd.cpp file, which implements
//  the TCQCTrayMonTab class. This provides a bass class from which any
//  tabs we put into the main frame window's tabbed window will derive. It
//  allows us to provide common functionality where needed.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TCQCTrayMonTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCTrayMonTab : public TTabWindow
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        ~TCQCTrayMonTab();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid Cleanup() = 0;

        virtual tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
        ) = 0;

        virtual tCIDLib::TVoid RemoveBindings
        (
                    tCIDOrbUC::TNSrvProxy&  orbcNS
        ) = 0;



    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQCTrayMonTab
        (
            const   TString&                strName
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TCQCTrayMonTab();
        TCQCTrayMonTab(const TCQCTrayMonTab&);
        tCIDLib::TVoid operator=(const TCQCTrayMonTab&);


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTrayMonTab, TTabWindow)
};


