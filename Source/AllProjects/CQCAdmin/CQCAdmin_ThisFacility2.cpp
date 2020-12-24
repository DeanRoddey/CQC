//
// FILE NAME: CQCAdmin_ThisFacility2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/21/2016
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
//  This file handles spillover from the main facility cpp file, to keep it from
//  getting too large.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"


// ---------------------------------------------------------------------------
//  TFacCQCAdmin: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We need to, early in the process, initialize the local object store,
//  which is used to store machine (and user account) specific info locally
//  on this machine. It's provided by the GCKit facility.
//
tCIDLib::TVoid
TFacCQCAdmin::InitObjectStore(          TCQCCAppState&      fstInit
                                , const tCIDLib::TBoolean   bNoState)
{
    //
    //  Do the init. He will use this key to build up the the actual file path for
    //  our object store, including the current user name in the file name as well.
    //  Pass the no state command line param value as the 'reset' indicator. If they
    //  say 'no state', then reset will be true and this guy will delete any previous
    //  file and start us fresh.
    //
    facCQCGKit().InitObjectStore
    (
        kCQCAdmin::pszAppStoreKey, m_cuctxToUse.strUserName(), bNoState
    );

    // Check to see if we have info for the main window size/pos
    tCIDLib::TCard4 c4Ver = 0;
    tCIDLib::ELoadRes eRes = facCQCGKit().eReadStoreObj
    (
        kCQCAdmin::pszCfgKey_MainFrame, c4Ver, fstInit, kCIDLib::False
    );

    if (eRes == tCIDLib::ELoadRes::NotFound)
    {
        // We don't have it, so just write out default info
        fstInit.areaFrame(facCIDCtrls().areaDefWnd());
        facCQCGKit().AddStoreObj(kCQCAdmin::pszCfgKey_MainFrame, fstInit, 128);
    }
     else if (bNoState)
    {
        //
        //  We found it, but they don't want to use it, so set to defaults. Not need
        //  to write it out. It'll get written back out after it gets used and converted
        //  to real screen coordinates.
        //
        fstInit.areaFrame(facCIDCtrls().areaDefWnd());
    }
     else
    {
        //
        //  We found it and can use it. But make sure it's still reasonable. The
        //  screen res may have changed.
        //
        TArea areaCheck = fstInit.areaFrame();
        facCIDCtrls().ValidatePrevWindowArea(areaCheck);
        fstInit.areaFrame(areaCheck);
    }
}


//
//  Called at startup. We register COM port factories with CIDComm as required.
//
tCIDLib::TVoid TFacCQCAdmin::RegisterCOMFactories()
{
    //
    //  Watch for a CQC_REMPORTSRV= environment variable. This tells us to
    //  register a remote port server port factory, which will make those
    //  serial ports available for selection when loading drivers. This
    //  is not advertised and is for development purposes.
    //
    TString strRemPort;
    if (TProcEnvironment::bFind(L"CQC_REMPORTSRV", strRemPort))
    {
        try
        {
            TIPEndPoint ipepRemSrv(strRemPort, tCIDSock::EAddrTypes::Unspec);
            facCIDComm().RegisterFactory(new TCQCRemSrvPortFactory(ipepRemSrv));
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }
        }
    }

    //  Register any local port factories.
    facGC100Ser().bUpdateFactory(m_cuctxToUse.sectUser());
    facJAPwrSer().bUpdateFactory(m_cuctxToUse.sectUser());
}
