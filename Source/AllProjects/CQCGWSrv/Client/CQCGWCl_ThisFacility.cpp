//
// FILE NAME: CQCGWCl_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2013
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGWCl_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCGWCl,TFacility)



// ---------------------------------------------------------------------------
//  CLASS: TGWClClassLoader
// PREFIX: mecl
// ---------------------------------------------------------------------------
class TGWClClassLoader : public MMEngExtClassLoader
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGWClClassLoader() noexcept {}

        TGWClClassLoader(const TGWClClassLoader&) = delete;
        TGWClClassLoader(TGWClClassLoader&&) = delete;

        ~TGWClClassLoader() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGWClClassLoader& operator=(const TGWClClassLoader&) = delete;
        TGWClClassLoader& operator=(TGWClClassLoader&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TMEngClassInfo* pmeciLoadClass
        (
                    TCIDMacroEngine&        meTarget
            , const TString&                strClassPath
        )   final;
};

TMEngClassInfo*
TGWClClassLoader::pmeciLoadClass(       TCIDMacroEngine&    meTarget
                                , const TString&            strClassPath)
{
    TMEngClassInfo* pmeciRet = nullptr;
    if (strClassPath == TCQCGWSrvCInfo::strPath())
        pmeciRet = new TCQCGWSrvCInfo(meTarget);

    return pmeciRet;
}



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCGWCl
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCGWCl: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCGWCl::TFacCQCGWCl() :

    TFacility
    (
        L"CQCGWCl"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}

TFacCQCGWCl::~TFacCQCGWCl()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCGWCl: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Client apps have to call this to get the CML runtime classes register
//  with the macro engine facility, so that these classes are available to
//  any macros they might load. We just allocate one of our standard runtime
//  class loader objects and install it on the macro engine.
//
//  Unfortunately there is no way to have this fault in, so containing apps
//  must call this during init, before they start up other threads.
//
tCIDLib::TVoid TFacCQCGWCl::InitCMLRuntime()
{
    facCIDMacroEng().AddClassLoader(new TGWClClassLoader);
}

