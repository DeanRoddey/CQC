//
// FILE NAME: CQCRPortSrv.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 19/14/2005
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
//  This is the main implementation file of the remote port server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRPortSrv.hpp"


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"CQCRPortSrvMainThread"
        , TMemberFunc<TFacCQCRPortSrv>(&facCQCRPortSrv(), &TFacCQCRPortSrv::eMainThread)
    )
)


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacCQCRPortSrv& facCQCRPortSrv()
{
    static TFacCQCRPortSrv* pfacThis = new TFacCQCRPortSrv();
    return *pfacThis;
}
