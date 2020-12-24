//
// FILE NAME: CQCGenDrvS_ClassLoader_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/15/2003
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
//  This is the header for the CQCGenDrvS_ClassLoader.cpp file, which
//  implements a derivative of the macro engine's external class loader,
//  where external means implemented in C++ but not an intrinsic. This
//  makes our CML driver base class available to the engine.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCGenDrvClassLoader
// PREFIX: mecl
// ---------------------------------------------------------------------------
class CQCGENDRVSEXPORT TCQCGenDrvClassLoader

    : public TObject, public MMEngExtClassLoader
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCGenDrvClassLoader();

        ~TCQCGenDrvClassLoader();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TMEngClassInfo* pmeciLoadClass
        (
                    TCIDMacroEngine&        meTarget
            , const TString&                strClassPath
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TCQCGenDrvClassLoader(const TCQCGenDrvClassLoader&);
        tCIDLib::TVoid operator=(const TCQCGenDrvClassLoader&);


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCGenDrvClassLoader,TObject)
};

#pragma CIDLIB_POPPACK


