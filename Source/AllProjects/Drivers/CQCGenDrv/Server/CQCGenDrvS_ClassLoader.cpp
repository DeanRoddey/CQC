//
// FILE NAME: CQCGenDrvS_ClassLoader.cpp
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
//  This file implements a derivative of the CIDLib Macro Engine's class
//  loader interface that loads any external classes supported by this
//  facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGenDrvS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCGenDrvClassLoader,TObject)


// ---------------------------------------------------------------------------
//  CLASS: TCQCGenDrvClassLoader
// PREFIX: mecl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCGenDrvClassLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCGenDrvClassLoader::TCQCGenDrvClassLoader()
{
}

TCQCGenDrvClassLoader::~TCQCGenDrvClassLoader()
{
}


// ---------------------------------------------------------------------------
//  TCQCGenDrvClassLoader: Public, inherited methods
// ---------------------------------------------------------------------------
TMEngClassInfo*
TCQCGenDrvClassLoader::pmeciLoadClass(          TCIDMacroEngine& meTarget
                                        , const TString&         strClassPath)
{
    if (strClassPath == TCQCAppCtrlInfo::strPath())
        return new TCQCAppCtrlInfo(meTarget);
    else if (strClassPath == TCQCDriverInfo::strPath())
        return new TCQCDriverInfo(meTarget);
    else if (strClassPath == TCQCDriverGlueInfo::strPath())
        return new TCQCDriverGlueInfo(meTarget);

    return 0;
}


