//
// FILE NAME: MediaRepoMgr.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/01/2006
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
//  This is the main header of the
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDEncode.hpp"
#include    "CIDJPEG.hpp"
#include    "CIDImgFact.hpp"
#include    "CIDDAE.hpp"
#include    "CIDCrypto.hpp"

#if         !defined(CIDMETAEXT_HPP)
#include    "CIDMetaExtr.hpp"
#endif

#include    "CIDOrb.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDNet.hpp"
#include    "CIDWUtils.hpp"
#include    "CIDXML.hpp"
#include    "CIDZLib.hpp"

#include    "CQCKit.hpp"
#include    "CQCGKit.hpp"
#include    "CQCIGKit.hpp"
#include    "CQCMedia.hpp"
#include    "CQCMedia_RepoUpload.hpp"


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kMediaRepoMgr
{
    // -----------------------------------------------------------------------
    //  Keys used in our local config store.  The last path ones are
    //  not full paths, just ids that are put into a standard last path scope
    //  by a helper method.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszCfgKey_MainFrame   = L"/Wnd/MainFrameState";
    const tCIDLib::TCh* const pszStoreKey           = L"MediaRepoMgr";
    const tCIDLib::TCh* const pszCfgKey_LastImgPath = L"LastImgPath";


    // -----------------------------------------------------------------------
    //  The key on the master server config repository where we store the per repo
    //  driver configuration settings. The replace parameter is the repository driver
    //  moniker.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszOSKey_Settings = L"/CQC/MediaRepo/%(1)/Settings";


    // -----------------------------------------------------------------------
    //  The max image sizes. Anything larger than this, we'll down-scale.
    //  And the JPEG image quality setting.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxH          = 1024;
    const tCIDLib::TCard4   c4MaxV          = 1024;
    const tCIDLib::TCard4   c4JPEGQuality   = 75;


    // -----------------------------------------------------------------------
    //  How long we wait in a deferred list load
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4 c4ListDeferTime   = 250;
}


// ---------------------------------------------------------------------------
//  And our own fundamental headers
// ---------------------------------------------------------------------------
#include    "MediaRepoMgr_ErrorIds.hpp"
#include    "MediaRepoMgr_MessageIds.hpp"
#include    "MediaRepoMgr_ResourceIds.hpp"
#include    "MediaRepoMgr_CQSLRepoClientProxy.hpp"


// ---------------------------------------------------------------------------
//  Some types that need to see the headers above
// ---------------------------------------------------------------------------
namespace tMediaRepoMgr
{
    // -----------------------------------------------------------------------
    //  A counted pointer to one of our repository manager client proxies
    // -----------------------------------------------------------------------
    using TRepoMgrProxyPtr = TCntPtr<TCQSLRepoMgrClientProxy>;


    // -----------------------------------------------------------------------
    //  The possible primary status returns from a My Movies metadata query.
    // -----------------------------------------------------------------------
    enum class EMMStatus
    {
        OK
        , Failed
        , None
    };
}


// ---------------------------------------------------------------------------
//  And the rest of our own headers
// ---------------------------------------------------------------------------
#include    "MediaRepoMgr_RepoSettings.hpp"
#include    "MediaRepoMgr_RepoSettingsDlg.hpp"
#include    "MediaRepoMgr_MetaSrc.hpp"
#include    "MediaRepoMgr_MyMoviesMetaSrc.hpp"

#include    "MediaRepoMgr_ThisFacility.hpp"
#include    "MediaRepoMgr_MainFrame.hpp"


// ---------------------------------------------------------------------------
//  Some stuff that needs to be seen throughout this facility
// ---------------------------------------------------------------------------
extern TFacMediaRepoMgr facMediaRepoMgr;

