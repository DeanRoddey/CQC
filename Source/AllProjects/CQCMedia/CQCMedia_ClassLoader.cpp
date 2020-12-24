//
// FILE NAME: CQCMedia_ClassLoader.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/30/2006
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
//  This is the implementation of our media related CML class loader.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  CLASS: TCQCMediaRTClassLoader
// PREFIX: mecl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCMediaRTClassLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCMediaRTClassLoader::TCQCMediaRTClassLoader() noexcept
{
}

TCQCMediaRTClassLoader::~TCQCMediaRTClassLoader()
{
}


// ---------------------------------------------------------------------------
//  TCQCMediaRTClassLoader: Public, inherited methods
// ---------------------------------------------------------------------------
TMEngClassInfo*
TCQCMediaRTClassLoader::pmeciLoadClass(         TCIDMacroEngine& meTarget
                                        , const TString&         strClassPath)
{
    TMEngClassInfo* pmeciRet = nullptr;

    if (strClassPath == TCQCMediaInfoInfo::strPath())
        pmeciRet = new TCQCMediaInfoInfo(meTarget);
    else if (strClassPath == TCQCPLItemInfo::strPath())
        pmeciRet = new TCQCPLItemInfo(meTarget);
    else if (strClassPath == TCQCPLMgrInfo::strPath())
        pmeciRet = new TCQCPLMgrInfo(meTarget);
    return pmeciRet;
}

