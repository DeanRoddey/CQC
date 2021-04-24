//
// FILE NAME: CQCIGKit_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/18/2004
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
#include    "CQCIGKit_.hpp"
#include    "CQCIGKit_AboutDlg_.hpp"
#include    "CQCIGKit_AddToPackDlg_.hpp"
#include    "CQCIGKit_CfgRemPortDlg_.hpp"
#include    "CQCIGKit_EditCmdDlg_.hpp"
#include    "CQCIGKit_EditKeyMapDlg_.hpp"
#include    "CQCIGKit_EdTimeToksDlg_.hpp"
#include    "CQCIGKit_ImportPackDlg_.hpp"
#include    "CQCIGKit_LatLongDlg_.hpp"
#include    "CQCIGKit_SelDrvDlg_.hpp"
#include    "CQCIGKit_SelFldDlg_.hpp"
#include    "CQCIGKit_SelIRCmdDlg_.hpp"
#include    "CQCIGKit_MngVarsDlg_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCIGKit,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIGKit
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCIGKit: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCIGKit::TFacCQCIGKit() :

    TGUIFacility
    (
        L"CQCIGKit"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_bVarsLoaded(kCIDLib::False)
    , m_c4KeyMapVer(0)
    , m_ctarGlobals(tCIDLib::EMTStates::Safe, kCIDLib::False)
    , m_enctNextKMQ(0)
{
}

TFacCQCIGKit::~TFacCQCIGKit()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCIGKit: Constructors and Destructor
// ---------------------------------------------------------------------------

// Displays our generic about dialog
tCIDLib::TVoid TFacCQCIGKit::AboutDlg(TWindow& wndOwner)
{
    TAboutDlg dlgAbout;
    dlgAbout.Run(wndOwner);
}


//
//  This is a helper called to actually add a single file to a package. We have to read the
//  contents and then add it to the package with the appropriate info. This is used by our
//  own internal package creation dialog, but the client application also generally adds
//  the initial files to the package (which are protected from removal), and we don't want
//  him to replicate this code.
//
//  We'll also try to make sure nothing incorrect is done while building the package.
//
//  OBVIOUSLY this only is for data server oriented files. Any others are of the type
//  mentioned above, special stuff added by the caller initially depending on the type
//  of package.
//
tCIDLib::TVoid
TFacCQCIGKit::AddFileToPack(            TDataSrvClient&         dsclToUse
                                ,       TCQCPackage&            packTar
                                , const tCQCKit::EPackFlTypes   eType
                                , const TString&                strRelPath
                                , const tCQCRemBrws::EDTypes    eDType
                                , const TCQCSecToken&           sectUser
                                , const tCIDLib::TBoolean       bEncrypt)
{
    try
    {
        tCIDLib::TCard4         c4SerialNum = 0;
        tCIDLib::TEncodedTime   enctLastChange;
        TChunkedFile            chflData;
        const tCIDLib::TBoolean bRes = dsclToUse.bReadFile
        (
            strRelPath
            , eDType
            , c4SerialNum
            , enctLastChange
            , chflData
            , sectUser
        );

        // We passed zero serial number so we should get data
        CIDAssert(bRes, L"Did not get new data with a zero serial number");

        //
        //  If a macro or template, compress it since there's a lot to be gained. The
        //  others are either too small (or already compressed in the case of images)
        //  to really get much gain for the overhead.
        //
        tCIDLib::TBoolean bCompress
        (
            (eDType == tCQCRemBrws::EDTypes::Macro)
            || (eDType == tCQCRemBrws::EDTypes::Template)
        );

        // Since we used a chunked stream, we have to read the data out to a buffer
        packTar.c4AddFile(eType, strRelPath, chflData, bCompress, bEncrypt);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TBoolean
TFacCQCIGKit::bCfgRemotePort(const  TWindow&            wndOwner
                            ,       tCIDLib::TBoolean&  bEnable
                            ,       TIPEndPoint&        ipepInOut)
{
    TCfgRemPortDlg dlgRemPort;
    return dlgRemPort.bRunDlg(wndOwner, bEnable, ipepInOut);
}



//
//  Connects to the security server on the master server and asks it for
//  the CQC version number is is. Then checks that against our own. If it's
//  not the same, tells the user and returns false so that the program
//  can exit.
//
tCIDLib::TBoolean
TFacCQCIGKit::bCheckCQCVersion(const TWindow& wndOwner, const TString& strTitle)
{
    // Get a security server proxy
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy(15000);

    tCIDLib::TCard8 c8SrvVer = orbcSS->c8QueryVersion();
    if (c8SrvVer != kCQCKit::c8Ver_Current)
    {
        TString strOurVer;
        TString strSrvVer;
        facCQCKit().FormatVersionStr(strOurVer, kCQCKit::c8Ver_Current);
        facCQCKit().FormatVersionStr(strSrvVer, c8SrvVer);

        TErrBox msgbVer
        (
            strTitle
            , facCQCKit().strMsg(kKitErrs::errcApp_BadClientVer, strOurVer, strSrvVer)
        );
        msgbVer.ShowIt(wndOwner);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// A wrapper around the key map editor
tCIDLib::TBoolean
TFacCQCIGKit::bEditKeyMap(  const   TWindow&                wndOwner
                            , const TString&                strAccount
                            , const tCQCIGKit::EKMActModes  eMode
                            , const TCQCUserCtx&            cuctxToUse
                            ,       TCQCKeyMap&             kmToEdit)
{
    TCQCKeyMapDlg dlgEdit(eMode, cuctxToUse);
    return dlgEdit.bRun(wndOwner, strAccount, kmToEdit);
}


// Invokes the action editor dialog to edict the passed action
tCIDLib::TBoolean
TFacCQCIGKit::bEditAction(  const   TWindow&                    wndOwner
                            ,       MCQCCmdSrcIntf&             mcsrcEdit
                            , const tCIDLib::TBoolean           bRequired
                            , const TCQCUserCtx&                cuctxToUse
                            ,       tCQCKit::TCmdTarList* const pcolExtraTars
                            , const TString* const              pstrBasePath)
{
    TCQCEditCmdDlg dlgEdit;

    // If we got a relative path, set it
    if (pstrBasePath)
        dlgEdit.SetBasePath(*pstrBasePath);

    return dlgEdit.bRun(wndOwner, mcsrcEdit, bRequired, cuctxToUse, pcolExtraTars);
}


// A wrapper around the time token editor dialog
tCIDLib::TBoolean
TFacCQCIGKit::bEditTimeTokens(const TWindow& wndOwner, TString& strToEdit)
{
    TEdTimeToksDlg dlgEdit;
    return dlgEdit.bRun(wndOwner, strToEdit);
}



//
//  Gets the lat/long information and if not found, or the force parm is
//  set, asks the user to set it.
//
tCIDLib::TBoolean
TFacCQCIGKit::bGetLocInfo(  const   TWindow&            wndOwner
                            ,       tCIDLib::TFloat8&   f8Lat
                            ,       tCIDLib::TFloat8&   f8Long
                            , const tCIDLib::TBoolean   bForce
                            , const TCQCSecToken&       sectUser)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;
    f8Lat = 0.0;
    f8Long = 0.0;
    try
    {
        // Get an installation server proxy and ask it for the location info
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        bRes = orbcIS->bQueryLocationInfo(f8Lat, f8Long, sectUser);

        // If not there or bad, or forced, then get it from the user
        if (!bRes || bForce)
        {
            TLatLongDlg dlgLatLong;
            bRes = dlgLatLong.bRunDlg(wndOwner, f8Lat, f8Long);
            if (bRes)
            {
                // Update the installation server
                orbcIS->SetLocationInfo(f8Lat, f8Long, sectUser);

                //
                //  And try to tell the scheduled event server about it. In
                //  this case, we only want to do this if the event server is
                //  actually running, but it could be on the way up since this
                //  method gets called on startup if no info is set.
                //
                //  So, we use a short timeout, and catch any error.
                //
                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
                tCQCEvCl::TEventSrvProxy orbcES;
                try
                {
                    orbcES = facCQCEvCl().orbcEventSrvProxy(3000);
                }

                catch(...)
                {
                    // Just eat it
                }

                //
                //  If we got it, then try to set it. Let any error here
                //  go to the catch below.
                //
                if (orbcES.pobjData())
                    orbcES->SetLocInfo(f8Lat, f8Long, sectUser);
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        facCQCGKit().ShowError
        (
            wndOwner
            , wndOwner.strWndText()
            , facCQCIGKit().strMsg(kIGKitErrs::errcCfg_GetLatLong)
            , errToCatch
        );
        return kCIDLib::False;
    }
    return bRes;
}


// A wrapper around the package import dialog
tCIDLib::TBoolean
TFacCQCIGKit::bImportPackage(const   TWindow&        wndOwner
                            , const TString&        strPackFile
                            , const TCQCUserCtx&    cuctxToUse)
{
    TImportPackDlg dlgImpPack;
    return dlgImpPack.bRunDlg(wndOwner, strPackFile, cuctxToUse);
}


//
//  This is a standard action invoking method for GUI apps that don't have any
//  needs beyond the standard command target types. This also means that there
//  will be no nested actions invoked, since none of the standard targets do
//  that, and there is no need to call back into the GUI thread. So we can
//  use the standard GUI orientd action engine.
//
tCIDLib::TBoolean
TFacCQCIGKit::bInvokeAction(const   TWindow&                wndOwner
                            ,       TWindow&                wndModal
                            , const MCQCCmdSrcIntf&         csrcToDo
                            , const TString&                strEventId
                            , const TString&                strTitle
                            , const TCQCUserCtx&            cuctxToUse
                            ,       MCQCCmdTracer* const    pcmdtDebug)
{
    // We don't have any extras, so just reate an empty collection
    TRefVector<MCQCCmdTarIntf> colExtraTars(tCIDLib::EAdoptOpts::NoAdopt, 1);

    // Create a standard GUI based action engine
    TCQCStdGUIActEngine acteInvoke
    (
        &wndOwner, &wndModal, strTitle, cuctxToUse
    );

    // And invoke it in the background mode
    const tCQCKit::ECmdRes eRes = acteInvoke.eInvokeOpsInBgn
    (
        csrcToDo
        , strEventId
        , m_ctarGlobals
        , colExtraTars
        , pcmdtDebug
        , TString::strEmpty()
    );
    return (eRes == tCQCKit::ECmdRes::Ok);
}


//
//  This is called to invoke a mapped key. We get the extended key and the
//  shift states and the key mapping list. We look up the mapping for the
//  passed key and shift states. This gets us a key map item, which is mixes
//  in the command source interface, so we then call bInvokeAction() which
//  can invoke anything that implements the standard command targets, which
//  is all that key maps can currently do.
//
//  These are always invoked via us, so we provide the global variables
//  target for them, using a member. Key mapped actions are global to the
//  app so it's find for us to provide a single one for the whole app.
//
tCIDLib::TBoolean
TFacCQCIGKit::bInvokeKeyMap(const   tCIDCtrls::EExtKeys     eExtKey
                            , const TWindow&                wndOwner
                            ,       TWindow&                wndModal
                            , const TString&                strTitle
                            , const tCIDLib::TBoolean       bAltShift
                            , const tCIDLib::TBoolean       bCtrlShift
                            , const tCIDLib::TBoolean       bShift
                            , const TCQCUserCtx&            cuctxToUse
                            ,       MCQCCmdTracer* const    pcmdtDebug)
{
    // Check for keymap data. If we can't get any, then fail now
    if (!bLoadKeyMapData(wndOwner, cuctxToUse))
        return kCIDLib::False;

    // Now call the other version with the current key map data
    return bInvokeKeyMap
    (
        eExtKey
        , wndOwner
        , wndModal
        , m_kmData
        , strTitle
        , bAltShift
        , bCtrlShift
        , bShift
        , cuctxToUse
        , pcmdtDebug
    );
}

tCIDLib::TBoolean
TFacCQCIGKit::bInvokeKeyMap(const   tCIDCtrls::EExtKeys     eExtKey
                            , const TWindow&                wndOwner
                            ,       TWindow&                wndModal
                            , const TCQCKeyMap&             kmData
                            , const TString&                strTitle
                            , const tCIDLib::TBoolean       bAltShift
                            , const tCIDLib::TBoolean       bCtrlShift
                            , const tCIDLib::TBoolean       bShift
                            , const TCQCUserCtx&            cuctxToUse
                            ,       MCQCCmdTracer* const    pcmdtDebug)
{
    // Make sure its a legal keymap key
    if (!TCQCKeyMap::bIsLegalKey(eExtKey))
        return kCIDLib::False;

    // If the Alt shift is pressed, then it can't be one of ours
    if (bAltShift)
        return kCIDLib::False;

    // Try to find this key in the map, with the current shift states
    const TCQCKeyMapItem& kmiInvoke = kmData.kmiFind(eExtKey, bShift, bCtrlShift);

    // If this one isn't used, then we don't have anything to do
    if (!kmiInvoke.bUsed())
        return kCIDLib::False;
    return bInvokeAction
    (
        wndOwner
        , wndModal
        , kmiInvoke
        , kCQCKit::strEvId_OnPress
        , strTitle
        , cuctxToUse
        , pcmdtDebug
    );
}



//
//  This method allows the key mapped actions of a particular user account to
//  be queried.
//
tCIDLib::TBoolean
TFacCQCIGKit::bQueryKeyMapData( const   TWindow&        wndOwner
                                ,       TCQCKeyMap&     kmToFill
                                , const TCQCUserCtx&    cuctxToUse
                                , const TString&        strUserName) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::ELoadRes eRes = tCIDLib::ELoadRes::Count;
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        bRet = orbcIS->bQueryKeyMap(c4SerialNum, eRes, strUserName, kmToFill, cuctxToUse.sectUser());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndOwner, strMsg(kIGKitMsgs::midStatus_CantAccessKMData), errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// A wrapper around the manage package dialog
tCIDLib::TBoolean
TFacCQCIGKit::bManagePackage(const  TWindow&                wndParent
                            ,       TCQCPackage&            packTar
                            , const TString&                strInitPath
                            , const tCIDLib::TBoolean       bEncrypt
                            , const tCQCRemBrws::EDTypes    eDType
                            , const TCQCUserCtx&            cuctxToUse)
{
    TAddToPackDlg dlgPack;
    return dlgPack.bRunDlg(wndParent, packTar, strInitPath, bEncrypt, eDType, cuctxToUse);
}


// A wrapper around the variables cheat sheet
tCIDLib::TBoolean
TFacCQCIGKit::bManageVars(  const   TWindow&            wndOwner
                            , const TString&            strUserName
                            ,       TString&            strSelected
                            ,       tCIDLib::TBoolean&  bByValue
                            , const tCIDLib::TBoolean   bNameOnly
                            , const TCQCUserCtx&        cuctxToUse)
{
    // Fault in the variables if not done so far
    if (!m_bVarsLoaded)
        LoadVarCheatSheet(wndOwner, strUserName, cuctxToUse);

    TMngVarsDlg dlgVars;
    const tCIDLib::TBoolean bRet = dlgVars.bRun
    (
        wndOwner, m_cavlCheatSheet, strSelected, bByValue, bNameOnly
    );

    // If any changes, store them and clear the flag
    if (m_cavlCheatSheet.bChanges())
        StoreVarCheatSheet(wndOwner, cuctxToUse);

    return bRet;
}


//
//  This method handles a special case where the caller wants to display all of
//  of the possible instances of a device class. This is more complex because a
//  given driver may implement more than one instance of a device class. So, in
//  the case of, say, the Audio device class, that would mean that there could be
//  multiple instances of a field defined by that class:
//
//      AUD#LR~Volume
//      AUD#MBR~Volume
//
//  and so forth. To select one, we have to display both the drivers that implement
//  that class, and for each driver as many instances as there are of that class.
//
//  So, we go through the caller's provided field cache and we find all of the
//  drivers that implement the desired device class. Then, for each of those, we
//  put in an entry for each instance. If there is no AUD# field, for instance, that
//  has a tilde, then there can only be one instance. If some do, then we know there
//  are multiple instances and will display them as:
//
//  moniker (subname1)
//  moniker (subname2)
//
//  and so forth. When a selection is made we can return both the moniker and the
//  subname if there is one, else the sub name is empty.
//
//  We are given the device class and the suffix bit, and that's enough to find what
//  we need.
//
tCIDLib::TBoolean
TFacCQCIGKit::bSelectDevSub(const   TWindow&                wndOwner
                            ,       TString&                strDevToFill
                            ,       TString&                strSubToFill
                            , const tCQCKit::EDevClasses    eDevClass
                            , const TString&                strSuffix
                            , const TCQCFldCache&           cfcSrc)
{
    //
    //  Query any devices that implement requested device class. If it failed
    //  or found none, then do nothing.
    //
    tCIDLib::TBoolean bRes = kCIDLib::False;
    tCIDLib::TStrList colDevs;
    if (!cfcSrc.bQueryDevList(colDevs, eDevClass) || colDevs.bIsEmpty())
        return kCIDLib::False;

    // Build the device prefix we'll look for
    TString strPref = TString::strConcat(tCQCKit::strAltXlatEDevClasses(eDevClass), L'#');

    // Build the suffix we need if there are multiple units
    TString strSuf = TString::strConcat(L"~", strSuffix);

    // Build the full field name if there are no multi-units
    TString strSingleFld = TString::strConcat(strPref, strSuffix);

    //
    //  For each one, look for the fields that start with the device class prefix and
    //  end with the indicated suffix.
    //
    //  If there's only one match, then we just add the moniker to the list. If more
    //  than one, we pull out the multi-unit suffix of each one and add.
    //
    TString strSub;
    tCIDLib::TKVPList colMatches;
    const tCIDLib::TCard4 c4Count = colDevs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the field list for this driver
        const tCQCKit::TFldDefList& colFlds
        (
            cfcSrc.colFieldListFor(colDevs[c4Index])
        );

        const tCIDLib::TCard4 c4FCnt = colFlds.c4ElemCount();
        for (tCIDLib::TCard4 c4FInd = 0; c4FInd < c4FCnt; c4FInd++)
        {
            const TCQCFldDef& flddCur = colFlds[c4FInd];
            const TString& strFld = flddCur.strName();

            //
            //  If it matches the full field, then there can be only one instance
            //  so we don't have to look further. No suffix in this case.
            //
            if (strFld == strSingleFld)
            {
                colMatches.objPlace(colDevs[c4Index], L"");
                break;
            }

            // If not one of the fields we care about it, then ignore it
            if (!strFld.bStartsWith(strPref) || !strFld.bEndsWith(strSuf))
                continue;

            //
            //  It is so remember it. We need to pull the content out between the
            //  pound and the tilde.
            //
            strSub = strFld;
            strSub.CutUpTo(kCIDLib::chPoundSign);
            strSub.bCapAtChar(kCIDLib::chTilde);

            colMatches.objPlace(colDevs[c4Index], strSub);
        }
    }

    // OK, we can invoke the list selection window
    TKeyValuePair kvalSel;
    bRes = facCIDWUtils().bKVPListSelect
    (
        wndOwner
        , facCQCIGKit().strMsg(kIGKitMsgs::midTitle_SelDevice)
        , colMatches
        , kvalSel
        , kCIDLib::True
    );

    if (bRes)
    {
        strDevToFill = kvalSel.strKey();
        strSubToFill = kvalSel.strValue();
    }
    return bRes;
}


//
//  A wrapper around the driver selection dialog.
//
tCIDLib::TBoolean
TFacCQCIGKit::bSelectDriver(const   TWindow&            wndOwner
                            ,       TString&            strDriver
                            , const tCQCKit::EDevCats   eDevCatFilt
                            , const TString&            strMakeFilt
                            , const TString&            strModelFilt)
{
    TCQCSelDrvDlg dlgSel;
    return dlgSel.bRun(wndOwner, strDriver, eDevCatFilt, strMakeFilt, strModelFilt);
}

tCIDLib::TBoolean
TFacCQCIGKit::bSelectDriver(const   TWindow&        wndOwner
                            , const TCQCFldCache&   cfcToUse
                            ,       TString&        strDriver)
{
    TCQCSelDrvDlg dlgSel;
    return dlgSel.bRun(wndOwner, cfcToUse, strDriver);
}



//
//  A wrapper around the field selection dialog. We have one that takes an existing
//  field info cache, and another that just takes a filter, sets up a cache object
//  and calls the other version.
//
tCIDLib::TBoolean
TFacCQCIGKit::bSelectField( const   TWindow&            wndOwner
                            ,       TCQCFldCache&       cfcFldInfo
                            ,       TString&            strDriver
                            ,       TString&            strField
                            ,       tCIDLib::TBoolean&  bByValue
                            , const tCIDLib::TBoolean   bNameOnly)
{
    TCQCSelFldDlg dlgSel;
    return dlgSel.bRun(wndOwner, cfcFldInfo, strDriver, strField, bByValue, bNameOnly);
}

tCIDLib::TBoolean
TFacCQCIGKit::bSelectField( const   TWindow&            wndOwner
                            ,       TCQCFldCache&       cfcFldInfo
                            ,       TString&            strDriver
                            ,       TString&            strField)
{
    TCQCSelFldDlg dlgSel;
    return dlgSel.bRun(wndOwner, cfcFldInfo, strDriver, strField);
}

tCIDLib::TBoolean
TFacCQCIGKit::bSelectField( const   TWindow&                wndOwner
                            ,       TCQCFldFilter* const    pffiltToAdopt
                            ,       TString&                strDriver
                            ,       TString&                strField
                            ,       tCIDLib::TBoolean&      bByValue
                            , const tCIDLib::TBoolean       bNameOnly)
{
    TCQCFldCache cfcToUse;
    cfcToUse.Initialize(pffiltToAdopt);
    return bSelectField(wndOwner, cfcToUse, strDriver, strField, bByValue, bNameOnly);
}

tCIDLib::TBoolean
TFacCQCIGKit::bSelectField( const   TWindow&                wndOwner
                            ,       TCQCFldFilter* const    pffiltToAdopt
                            ,       TString&                strDriver
                            ,       TString&                strField)
{
    TCQCFldCache cfcToUse;
    cfcToUse.Initialize(pffiltToAdopt);
    return bSelectField(wndOwner, cfcToUse, strDriver, strField);
}

tCIDLib::TBoolean
TFacCQCIGKit::bSelectField( const   TWindow&                wndOwner
                            ,       TCQCFldFilter* const    pffiltToAdopt
                            ,       TString&                strSelectedFld
                            ,       tCIDLib::TBoolean&      bByValue
                            , const tCIDLib::TBoolean       bNameOnly)
{
    //
    //  If any previus values, we are getting the driver.field syntax, so we need
    //  to split it out. If it fails, we just clear the values and go with no selected
    //  initial field.
    //
    TString strDriver, strField;
    if (!strSelectedFld.bIsEmpty())
    {
        try
        {
            facCQCKit().ParseFldName(strSelectedFld, strDriver, strField);
        }

        catch(...)
        {
            // Just clear the values and fall through
            strDriver.Clear();
            strField.Clear();
        }
    }

    if (bSelectField(wndOwner, pffiltToAdopt, strDriver, strField, bByValue, bNameOnly))
    {
        // Build up the full field to return
        strSelectedFld= strDriver;
        strSelectedFld.Append(kCIDLib::chPeriod);
        strSelectedFld.Append(strField);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}

tCIDLib::TBoolean
TFacCQCIGKit::bSelectField( const   TWindow&                wndOwner
                            ,       TCQCFldFilter* const    pffiltToAdopt
                            ,       TString&                strSelectedFld)
{
    //
    //  If any previus values, we are getting the driver.field syntax, so we need
    //  to split it out. If it fails, we just clear the values and go with no selected
    //  initial field.
    //
    TString strDriver, strField;
    if (!strSelectedFld.bIsEmpty())
    {
        try
        {
            facCQCKit().ParseFldName(strSelectedFld, strDriver, strField);
        }

        catch(...)
        {
            // Just clear the values and fall through
            strDriver.Clear();
            strField.Clear();
        }
    }

    if (bSelectField(wndOwner, pffiltToAdopt, strDriver, strField))
    {
        // Build up the full field to return
        strSelectedFld = strDriver;
        strSelectedFld.Append(kCIDLib::chPeriod);
        strSelectedFld.Append(strField);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// A wrapper around the IR command selection dialog
tCIDLib::TBoolean
TFacCQCIGKit::bSelectIRCmd( const   TWindow&    wndOwner
                            , const TString&    strDriver
                            ,       TString&    strToFill)
{
    TCQCSelIRCmdDlg dlgSel;
    return dlgSel.bRunDlg(wndOwner, strDriver, strToFill);
}



//
//  This method is called by client apps. For more advanced users, who can
//  get around the system quickly and for systems in kiosk mode, it is
//  possible for the client apps to come up before the back end has come
//  up completely.
//
//  So they can call us here and give us a name server binding of some
//  object that they need to talk to to get started (usually the config
//  server they are using.) We will wait for up to the time they indicate,
//  periodically checking for the name server to arrive and then for the
//  indicated binding to become available.
//
//  If the wait goes beyond 6 seconds, we'll pop up a little dialog that
//  indicates we are waiting.
//
//  NOTE:   They obviously must have initialized the ORB client services
//          before calling this or its going to fail for sure.
//
tCIDLib::TBoolean
TFacCQCIGKit::bWaitForCQC(  const   TWindow&        wndOwner
                            , const TString&        strTitle
                            , const TString&        strBinding
                            , const tCIDLib::TCard4 c4WaitFor)
{
    // We will wait a minimum of 10 seconds
    tCIDLib::TCard4 c4ActualWait = c4WaitFor;
    if (c4ActualWait < 10000)
        c4ActualWait = 10000;

    tCIDLib::TBoolean   bOk         = kCIDLib::False;
    tCIDLib::TCard4     c4CurTime   = TTime::c4Millis();
    tCIDLib::TCard4     c4EndTime1  = c4CurTime + 4000;
    tCIDLib::TCard4     c4EndTime2  = c4CurTime + c4ActualWait;
    TString             strName;
    TString             strDescr;

    while (c4CurTime < c4EndTime1)
    {
        try
        {
            tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy(1000);
            orbcNS->QueryNameInfo(strBinding, strName, strDescr);
            bOk = kCIDLib::True;
            break;
        }

        catch(const TError&)
        {
            // Just eat it
        }

        // Get the new current time to see if it's time to give up
        c4CurTime = TTime::c4Millis();
    }

    //
    //  If we didn't get it, and the time isn't up, then it's because we've
    //  waited more than 10 seconds and we need to pop up the dialog that
    //  indicates we are waiting. But don't bother if we are within a second
    //  of the end time anyway.
    //
    if (!bOk && (c4CurTime < c4EndTime2))
    {
        const tCIDLib::TCard4 c4Remaining = c4EndTime2 - c4CurTime;
        if (c4Remaining > 1000)
        {
            // Do the dialog box. It will return true if it worked
            TWaitCQCDlg dlgWait;
            bOk = dlgWait.bRunDlg(wndOwner, strTitle, strBinding, c4Remaining);
        }
    }
    return bOk;
}


//
//  This is called from the client driver tab, when it needs to load a new client side
//  driver. We return the driver's window, which will be created as a child of the passed
//  parent window, at the indicated coordinates, with the indicated window id.
//
//  The caller has to pass a valid user context, which will be provided to the driver
//  so that it can do calls as required on behalf of the user logged in. The driver
//  will have whatever rights the logged in user does.
//
//  We have a test mode flag that can be used during development, so that it will load
//  the DLLs from the result directory, where normally it would load them from the Bin
//  directory of the installed system.
//
TCQCDriverClient*
TFacCQCIGKit::pwndLoadNewDriver(        TWindow&            wndParent
                                , const TArea&              areaInit
                                , const tCIDCtrls::TWndId   widToUse
                                , const TCQCDriverObjCfg&   cqcdcInfo
                                , const TCQCUserCtx&        cuctxToUse
                                , const tCIDLib::TBoolean   bTestMode)
{
    //
    //  Create a new driver info object and store it if it succeeds in creating. If
    //  it doesn't then the throw out of the constructor will prevent the Add call
    //  from occuring. If that occurs, and we created a new state object, we need to
    //  remove that item.
    //
    TFacility* pfacDriver = nullptr;
    TCQCDriverClient* pwndRet = nullptr;
    try
    {
        // Use a different base path depending on test mode
        TString strLoadPath;
        if (bTestMode)
        {
            TString strResDir;
            if (!TProcEnvironment::bFind(L"CID_RESDIR", strResDir))
            {
                ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kIGKitErrs::errcDrv_NoResDir
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                );
            }
            strLoadPath = strResDir;
        }
        else
        {
            strLoadPath = facCQCKit().strBinDir();
        }

        //
        //  Figure out the flags. We just look for the files. So we need to build
        //  the module names. This is sort of redundant since it'll have to be done
        //  again in the module load, but since we have to create the facility object
        //  for these guys up front, we can't know without looking.
        //
        tCIDLib::EModFlags eActualFlags = tCIDLib::EModFlags::None;
        {
            TString strPortName, strLoadName;
            TModule::BuildModName
            (
                cqcdcInfo.strClientLibName()
                , kCQCKit::c4MajVersion
                , kCQCKit::c4MinVersion
                , tCIDLib::EModTypes::SharedLib
                , strPortName, strLoadName
            );

            TPathStr pathTest;
            pathTest = strLoadPath;
            pathTest.AddLevel(strPortName);
            pathTest.AppendExt(L"CIDRes");
            if (TFileSys::bExists(pathTest, tCIDLib::EDirSearchFlags::AllFiles))
                eActualFlags |= tCIDLib::EModFlags::HasResFile;

            pathTest = strLoadPath;
            pathTest.AddLevel(strPortName);
            pathTest.Append(L"_*");
            pathTest.AppendExt(L"CIDMsg");
                eActualFlags |= tCIDLib::EModFlags::HasMsgFile;
        }

        // OK, now we can try to load it
        pfacDriver = new TFacility
        (
            cqcdcInfo.strClientLibName()
            , strLoadPath
            , tCIDLib::EModTypes::SharedLib
            , kCQCKit::c4MajVersion
            , kCQCKit::c4MinVersion
            , 0
            , eActualFlags
        );

        // Use it to dynamically load the factory function
        tCQCIGKit::TClientDriverFuncPtr pfnDriverFunc = nullptr;
        try
        {
            pfnDriverFunc = tCQCIGKit::TClientDriverFuncPtr
            (
                pfacDriver->pfnGetFuncPtr(kCQCIGKit::pszClientFunc)
            );
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIGKitErrs::errcDrv_FactoryFuncErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , cqcdcInfo.strMoniker()
                );
            }

            delete pfacDriver;
            return nullptr;
        }

        catch(...)
        {
            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIGKitErrs::errcDrv_FactoryFuncErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , cqcdcInfo.strMoniker()
                );
            }

            delete pfacDriver;
            return nullptr;
        }

        //
        //  And now call the factory function to create the window. It will create
        //  itself as a child of the passed parent, at the indicated position.
        //
        try
        {
            pwndRet = (pfnDriverFunc)
            (
                wndParent, areaInit, widToUse, cqcdcInfo, cuctxToUse
            );
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIGKitErrs::errcDrv_CallFactoryFuncErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , cqcdcInfo.strMoniker()
                );
            }

            delete pfacDriver;
            return nullptr;
        }

        catch(...)
        {
            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIGKitErrs::errcDrv_CallFactoryFuncErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , cqcdcInfo.strMoniker()
                );
            }

            delete pfacDriver;
            return nullptr;
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        delete pfacDriver;
        return nullptr;
    }

    return pwndRet;
}


// Returns the variable at the top of the cheat sheet
const TString& TFacCQCIGKit::strTopOfCheatSheet() const
{
    // The caller should have accessed the cheat sheet before calling this
    CIDAssert(m_bVarsLoaded, L"Variable cheat sheet has not been loaded");
    return m_cavlCheatSheet.strTopKey();
}


// This method allows you to set the key map data for a specific user
tCIDLib::TVoid
TFacCQCIGKit::SetKeyMapData(const   TWindow&        wndOwner
                            , const TCQCKeyMap&     kmToSet
                            , const TCQCUserCtx&    cuctxToUse
                            , const TString&        strUserName)
{
    tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
    orbcIS->SetKeyMap(strUserName, kmToSet, cuctxToUse.sectUser());

    // If it's for our current user, then store it there as well
    if (strUserName == m_strUser)
        m_kmData = kmToSet;
}


tCIDLib::TVoid
TFacCQCIGKit::StoreVarCheatSheet(const TWindow& wndOwner, const TCQCUserCtx& cuctxToUse)
{
    // If we never loaded it, or nothing has changed, do nothing
    if (!m_bVarsLoaded || !m_cavlCheatSheet.bChanges())
        return;

    try
    {
        tCIDLib::TCard4 c4SerialNum = 0;
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        orbcIS->bAddOrUpdateCheatSheet
        (
            cuctxToUse.strUserName(), c4SerialNum, m_cavlCheatSheet, cuctxToUse.sectUser()
        );

        // Clear the changes flag since we stored it successfully
        m_cavlCheatSheet.ClearChanges();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            wndOwner
            , wndOwner.strWndText()
            , strMsg(kIGKitErrs::errcCfg_StoreVarList)
            , errToCatch
        );
    }
}


//
//  Given the path to a source image file, and a target scope within the
//  image repository, and the transparency info for the image, this guy will
//  upload the image, and create a thumbnail for it and upload that. They have
//  to provide us with an image repository client proxy, and with memory
//  buffers to use for reading in the image and for creating the thumb to
//  upload. The reason being that this is often called multiple times in a
//  row so we avoid creating them over and over.
//
//  It assumes that the caller already made sure that an overwrite was ok,
//  it will just blindly update any existing target of the same name.
//
//  We have a second version that doesn't take a transparency color but takes
//  a point within the image where the transparency color can be found. They
//  both turn around can call a helper that will do the rest.
//
tCIDLib::TVoid
TFacCQCIGKit::UploadImage(          TDataSrvClient&         dsclUp
                            ,       TPNGImage&              imgToUpload
                            , const TString&                strTargetFile
                            ,       TBinMBufOutStream&      strmImg
                            ,       TBinMBufOutStream&      strmThumb
                            , const tCIDLib::TBoolean       bNoDataCache
                            , const tCIDLib::TKVPFList&     colXMeta
                            , const TCQCUserCtx&            cuctxToUse)
{
    // Load up the image data
    TSize szThumb;
    facCQCKit().PackageImg(imgToUpload, strmImg, strmThumb, szThumb);

    const TPixelArray&      pixaImg = imgToUpload.pixaBits();
    tCIDLib::TCard4         c4NewSerNum;
    tCIDLib::TEncodedTime   enctLastChange;
    dsclUp.WriteImage
    (
        strTargetFile
        , c4NewSerNum
        , enctLastChange
        , strmImg.mbufData()
        , strmImg.c4CurSize()
        , strmThumb.mbufData()
        , strmThumb.c4CurSize()
        , pixaImg.ePixFmt()
        , pixaImg.eBitDepth()
        , pixaImg.szImage()
        , szThumb
        , kCQCRemBrws::c4Flag_OverwriteOK | (bNoDataCache ? kCQCRemBrws::c4Flag_NoDataCache : 0)
        , colXMeta
        , cuctxToUse.sectUser()
    );
}


tCIDLib::TVoid
TFacCQCIGKit::UploadImage(          TDataSrvClient&         dsclUp
                            , const TString&                strSrcFile
                            , const TPoint&                 pntTransAt
                            , const tCIDLib::TBoolean       bTrans
                            , const TString&                strTargetFile
                            ,       TBinMBufOutStream&      strmImg
                            ,       TBinMBufOutStream&      strmThumb
                            , const tCIDLib::TBoolean       bNoDataCache
                            , const tCIDLib::TKVPFList&     colXMeta
                            , const TCQCUserCtx&            cuctxToUse)
{
    // Package up the file
    tCIDImage::EBitDepths   eDepth;
    tCIDImage::EPixFmts     eFmt;
    TSize                   szImg;
    TSize                   szThumb;
    facCQCKit().PackageImg
    (
        strSrcFile, pntTransAt, bTrans, strmImg, strmThumb, eFmt, eDepth, szImg, szThumb
    );

    // And send it off
    tCIDLib::TCard4 c4NewSerNum;
    tCIDLib::TEncodedTime enctLastChange;

    dsclUp.WriteImage
    (
        strSrcFile
        , c4NewSerNum
        , enctLastChange
        , strmImg.mbufData()
        , strmImg.c4CurSize()
        , strmThumb.mbufData()
        , strmThumb.c4CurSize()
        , eFmt
        , eDepth
        , szImg
        , szThumb
        , kCQCRemBrws::c4Flag_OverwriteOK | (bNoDataCache ? kCQCRemBrws::c4Flag_NoDataCache : 0)
        , colXMeta
        , cuctxToUse.sectUser()
    );
}


// ---------------------------------------------------------------------------
//  TFacCQCIGKit: Constructors and Destructor
// ---------------------------------------------------------------------------

// A shared helper method to load up the keymaping data
tCIDLib::TBoolean
TFacCQCIGKit::bLoadKeyMapData(  const   TWindow&        wndOwner
                                , const TCQCUserCtx&    cuctxToUse) const
{
    // Store the user name
    m_strUser = cuctxToUse.strUserName();

    if (!m_c4KeyMapVer || (TTime::enctNow() > m_enctNextKMQ))
    {
        try
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

            tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
            tCIDLib::ELoadRes eRes = tCIDLib::ELoadRes::Count;
            if (!orbcIS->bQueryKeyMap(m_c4KeyMapVer, eRes, m_strUser, m_kmData, cuctxToUse.sectUser()))
            {
                m_c4KeyMapVer = 0;
                m_kmData.Reset();
            }

            // Set the next time that we should check for new data
            m_enctNextKMQ = TTime::enctNowPlusSecs(10);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            if (bShouldLog(errToCatch))
                LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                wndOwner, strMsg(kIGKitMsgs::midStatus_CantAccessKMData), errToCatch
            );
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Called to fault in the current user's variable cheat sheet. It's const
//  because it's called from both the const and non-const access methods,
//  and we have to fault in from both. The flag and cheat sheet are both
//  mutable for this reason.
//
tCIDLib::TVoid
TFacCQCIGKit::LoadVarCheatSheet(const   TWindow&        wndOwner
                                , const TString&        strUserName
                                , const TCQCUserCtx&    cuctxToUse) const
{
    // Just in case, make sure it's not already been done
    if (m_bVarsLoaded)
        return;

    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        tCIDLib::ELoadRes eRes = tCIDLib::ELoadRes::Count;

        // If not created yet, that's ok, we'll just have an empty list
        tCIDLib::TCard4 c4Ver = 0;
        orbcIS->bQueryCheatSheet(c4Ver, eRes, strUserName, m_cavlCheatSheet, cuctxToUse.sectUser());
        m_bVarsLoaded = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndOwner, strMsg(kIGKitErrs::errcCfg_LoadVarList), errToCatch
        );
    }
}

