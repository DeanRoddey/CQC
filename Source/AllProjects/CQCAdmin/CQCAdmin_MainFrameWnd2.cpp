//
// FILE NAME: CQCAdmin_MainFrameWnd2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2015
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
//  This file implements some spill over code from the main file. These are oriented
//  towards loading up files for edit/view.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_DrvMonTab.hpp"
#include    "CQCAdmin_EdSysCfgTab.hpp"
#include    "CQCAdmin_EMailAcctTab.hpp"
#include    "CQCAdmin_EvMonTab.hpp"
#include    "CQCAdmin_GlobalActTab.hpp"
#include    "CQCAdmin_GC100Tab.hpp"
#include    "CQCAdmin_JAPwrTab.hpp"
#include    "CQCAdmin_ImgEditTab.hpp"
#include    "CQCAdmin_ImgViewTab.hpp"
#include    "CQCAdmin_IntfEdTab.hpp"
#include    "CQCAdmin_LogicSrvTab.hpp"
#include    "CQCAdmin_MacroWSTab.hpp"
#include    "CQCAdmin_MonEvTrigsTab.hpp"
#include    "CQCAdmin_QueryLogsTab.hpp"
#include    "CQCAdmin_SchedEvTab.hpp"
#include    "CQCAdmin_TrigEvTab.hpp"
#include    "CQCAdmin_UserAcctTab.hpp"


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called when the user asks to edit an e-mail account. We load up a tab for this
//  account to edit it.
//
tCIDLib::TVoid TMainFrameWnd::EditEMailAcct(const TString& strPath)
{
    //
    //  We need the relative path though it's not really used for this type of data. The
    //  underlying tab class wants it. For us it's just /name.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::EMailAccount, strRelPath);

    // And get the name part which is the name of the account
    TString strName(strPath);
    strName.Cut(0, kCQCRemBrws::strPath_EMailAccts.c4Length() + 1);

    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Let's read it in and create the tab
    try
    {
        TCQCEMailAccount emacctEdit;

        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        tCIDLib::TCard4 c4SerialNum = 0;
        orbcIS->bQueryEmailAccount(strName, emacctEdit, c4SerialNum, facCQCAdmin.sectUser());

        // Create the new tab and load it
        TEMailAcctTab* pwndNewTab = new TEMailAcctTab(strPath, strRelPath);
        pwndNewTab->CreateTab(*m_pwndEditPane, emacctEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText()
            , facCQCAdmin.strMsg
              (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::EMailAccount)
              )
        );
        msgbView.ShowIt(*this);
    }
}


//
//  Called when the user asks to edit an event monitor. We load up a tab for this
//  monitor to edit it.
//
tCIDLib::TVoid TMainFrameWnd::EditEventMon(const TString& strPath)
{
    //
    //  We need to remove the prefix part of the path.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::EvMonitor, strRelPath);

    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Let's read it in
    try
    {
        TDataSrvClient dsclToUse;
        TCQCEvMonCfg emoncEdit;

        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        tCIDLib::TKVPFList  colMeta;
        dsclToUse.bReadEventMon
        (
            strRelPath
            , c4SerialNum
            , enctLastChange
            , emoncEdit
            , colMeta
            , facCQCAdmin.sectUser()
        );

        // Create the new tab and load it
        TEvMonTab* pwndNewTab = new TEvMonTab(strPath, strRelPath);
        pwndNewTab->CreateTab(*m_pwndEditPane, strTabText, emoncEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText()
            , facCQCAdmin.strMsg
              (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::EvMonitor)
              )
        );
        msgbView.ShowIt(*this);
    }
}


//
//  Called when the user asks to edit the GC-100 ports. We load up a tab that let's them
//  edit them. There's no path since it's a single browser item that contains all of the
//  ports.
//
tCIDLib::TVoid TMainFrameWnd::EditGC100Ports()
{
    // Let's read it in and create the tab
    try
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TCard4 c4Bytes = 0;
        THeapBuf mbufData;
        TGC100CfgList gcclEdit;
        const tCIDLib::ELoadRes eRes = orbcIS->eQueryGC100Cfg
        (
            c4SerialNum, c4Bytes, mbufData, facCQCAdmin.sectUser()
        );

        if (eRes == tCIDLib::ELoadRes::NewData)
        {
            TBinMBufInStream strmSrc(&mbufData, c4Bytes);
            strmSrc >> gcclEdit;
        }

        // Create the new tab and load it
        TGC100Tab* pwndNewTab = new TGC100Tab(kCQCRemBrws::strPath_GC100Ports, L"/");
        pwndNewTab->CreateTab(*m_pwndEditPane, L"GC Port Config", gcclEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText()
            , facCQCAdmin.strMsg
              (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::GC100Ports)
              )
        );
        msgbView.ShowIt(*this);
    }
}


//
//  Called when the user asks to edit the Just Add Power ports. We load up a tab that
//  lets him edit them. There's no path since it's a single browser item that contains
//  all of the ports.
//
tCIDLib::TVoid TMainFrameWnd::EditJAPwrPorts()
{
    // Let's read it in and create the tab
    try
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TCard4 c4Bytes = 0;
        THeapBuf mbufData;

        TJAPwrCfgList japlEdit;
        const tCIDLib::ELoadRes eRes = orbcIS->eQueryJAPCfg
        (
            c4SerialNum, c4Bytes, mbufData, facCQCAdmin.sectUser()
        );
        if (eRes == tCIDLib::ELoadRes::NewData)
        {
            TBinMBufInStream strmSrc(&mbufData, c4Bytes);
            strmSrc >> japlEdit;
        }

        // Create the new tab and load it
        TJAPwrTab* pwndNewTab = new TJAPwrTab(kCQCRemBrws::strPath_JAPwrPorts, L"/");
        pwndNewTab->CreateTab(*m_pwndEditPane, L"JAP Port Config", japlEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText()
            , facCQCAdmin.strMsg
              (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::JAPwrPorts)
              )
        );
        msgbView.ShowIt(*this);
    }
}

//
//  Called when the user asks to edit a global action. We load up a tab for this
//  global action to edit it.
//
tCIDLib::TVoid TMainFrameWnd::EditGlobalAction(const TString& strPath)
{
    //
    //  We need to remove the prefix part of the path, since the IDE works in terms
    //  of type relative paths.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::GlobalAct, strRelPath);

    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Let's read it in
    try
    {
        TDataSrvClient dsclToUse;
        TCQCStdCmdSrc csrcEdit;

        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        tCIDLib::TKVPFList  colMeta;
        dsclToUse.bReadGlobalAction
        (
            strRelPath
            , c4SerialNum
            , enctLastChange
            , csrcEdit
            , colMeta
            , facCQCAdmin.sectUser()
        );

        // Create the new tab and load it
        TGlobalActTab* pwndNewTab = new TGlobalActTab(strPath, strRelPath);
        pwndNewTab->CreateTab(*m_pwndEditPane, strTabText, csrcEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText()
            , facCQCAdmin.strMsg
              (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::GlobalAct)
              )
        );
        msgbView.ShowIt(*this);
    }
}


//
//  Called when the user asks to edit an image. We just ask for the image and load
//  into an image edit tab.
//
tCIDLib::TVoid TMainFrameWnd::EditImage(const TString& strPath)
{
    //
    //  We need to remove the prefix part of the path, since the data server client
    //  works in terms of file type relative paths.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::Image, strRelPath);

    TPNGImage imgToLoad;
    try
    {
        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        TDataSrvClient dsclRead;
        tCIDLib::TKVPFList  colMeta;
        const tCIDLib::TBoolean bRes = dsclRead.bReadImage
        (
            strRelPath
            , c4SerialNum
            , enctLastChange
            , imgToLoad
            , colMeta
            , facCQCAdmin.sectUser()
        );

        if (!bRes)
        {
            TErrBox msgbView
            (
                strWndText()
                , facCQCAdmin.strMsg(kCQCAMsgs::midStatus_CantReadFile, strPath)
            );
            msgbView.ShowIt(*this);
            return;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText(), facCQCAdmin.strMsg(kCQCAMsgs::midStatus_CantReadFile, strPath)
        );
        msgbView.ShowIt(*this);
        return;
    }

    // Get the part of the path we want to be displayed in the tab text
    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Create the new tab and load it
    TImgEditTab* pwndNewTab = new TImgEditTab(strPath, strRelPath);
    pwndNewTab->CreateTab(*m_pwndEditPane, strTabText, imgToLoad);
}



//
//  The user asked to edit our location info. This means changing it after the fact, which
//  is not something lightly done, so we warn about it first.
//
tCIDLib::TVoid TMainFrameWnd::EditLocation()
{
    TYesNoBox msgbAsk(facCQCAdmin.strMsg(kCQCAMsgs::midQ_ChangeLocation));
    if (msgbAsk.bShowIt(*this))
    {
        // Run the location selection dialog
        tCIDLib::TFloat8    f8Lat;
        tCIDLib::TFloat8    f8Long;
        const tCIDLib::TBoolean bRes = facCQCIGKit().bGetLocInfo
        (
            *this, f8Lat, f8Long, kCIDLib::True, facCQCAdmin.sectUser()
        );

        // If it was set, then warn them again
        if (bRes)
        {
            TOkBox msgbSet(facCQCAdmin.strMsg(kCQCAMsgs::midStatus_LocationChanged));
            msgbSet.ShowIt(*this);
        }
    }
}


//
//  The user asked to edit the logic server configuration. We load the tab to do that.
//  There's no path in this one, since it's a fixed path.
//
tCIDLib::TVoid TMainFrameWnd::EditLogicSrv()
{
    // Let's read it in and create the tab
    try
    {
        tCQLogicSh::TLogicSrvProxy orbcLogic = facCQLogicSh().orbcLogicSrvProxy();

        TCQLogicSrvCfg lscfgEdit;
        tCIDLib::TCard4 c4SerialNum;
        orbcLogic->QueryConfig(lscfgEdit, c4SerialNum);

        // Create the new tab and load it
        TLogicSrvTab* pwndNewTab = new TLogicSrvTab(kCQCRemBrws::strPath_LogicSrv, L"/");
        pwndNewTab->CreateTab(*m_pwndEditPane, L"Logic Server", lscfgEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText()
            , facCQCAdmin.strMsg
              (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::LogicSrv)
              )
        );
        msgbView.ShowIt(*this);
    }
}


//
//  Called when the user asks to edit a macro. We load up a macro workshop tab
//  to let the edit/debug the macro.
//
tCIDLib::TVoid TMainFrameWnd::EditMacro(const TString& strPath)
{
    //
    //  We need to remove the prefix part of the path, since the IDE works in terms
    //  of type relative paths.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::Macro, strRelPath);

    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Create the new tab and load it.
    TMacroWSTab* pwndNewTab = new TMacroWSTab(strPath, strRelPath);
    pwndNewTab->CreateTab(*m_pwndEditPane, strTabText);
}


//
//  Called when the user asks to edit a scheduled event. We load up a tab for this
//  event to edit it.
//
tCIDLib::TVoid TMainFrameWnd::EditSchedEvent(const TString& strPath)
{
    // Get the lat/long information. If we can't, then give up
    tCIDLib::TFloat8 f8Lat, f8Long;
    if (!facCQCIGKit().bGetLocInfo( *this
                                    , f8Lat
                                    , f8Long
                                    , kCIDLib::False
                                    , facCQCAdmin.sectUser()))
    {
        return;
    }

    //
    //  We need to remove the prefix part of the path, since the IDE works in terms
    //  of type relative paths.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::SchEvent, strRelPath);

    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Let's read it in
    try
    {
        TDataSrvClient  dsclToUse;
        TCQCSchEvent    csrcEdit;
        tCIDLib::TKVPFList  colMeta;
        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        dsclToUse.bReadSchedEvent
        (
            strRelPath
            , c4SerialNum
            , enctLastChange
            , csrcEdit
            , colMeta
            , facCQCAdmin.sectUser()
        );

        // Create the new tab and load it
        //
        TSchedEvTab* pwndNewTab = new TSchedEvTab(strPath, strRelPath);
        pwndNewTab->CreateTab(*m_pwndEditPane, strTabText, csrcEdit, f8Lat, f8Long);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText()
            , facCQCAdmin.strMsg
              (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::SchEvent)
              )
        );
        msgbView.ShowIt(*this);
    }
}


//
//  Called when the user asks to edit a the system configuration. We load up a tab for
//  this job. We don't bother trying to keep a serial number and caching a copy. We just
//  get a new copy when they ask to edit it.
//
tCIDLib::TVoid TMainFrameWnd::EditSystemCfg(const TString& strPath)
{
    // We don't need this but we have to pass it to the tab
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::SystemCfg, strRelPath);

    // Let's try to read in the system configuration data
    TCQCSysCfg scfgEdit;
    try
    {
        TCQCSysCfgClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TCQCSysCfgClientProxy>
        (
            TCQCSysCfgClientProxy::strBinding, 4000
        );
        TCntPtr<TCQCSysCfgClientProxy> orbcProxy(porbcProxy);

        tCIDLib::TCard4 c4SerialNum = 0;
        orbcProxy->bQueryRoomCfg(c4SerialNum, scfgEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            facCQCAdmin.strMsg
            (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::SystemCfg)
            )
        );
        msgbView.ShowIt(*this);
    }

    // We got it so load up the tab
    TEdSysCfgTab* pwndNewTab = new TEdSysCfgTab(strPath, strRelPath);
    pwndNewTab->CreateTab(*m_pwndEditPane, scfgEdit);
}

//
//  Called when the user asks to edit a template. We load up an interface editor
//  tab and pass it the provided path.
//
tCIDLib::TVoid TMainFrameWnd::EditTemplate(const TString& strPath)
{
    //
    //  We need to remove the prefix part of the path, since the IDE works in terms
    //  of type relative paths.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::Template, strRelPath);

    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Create the new tab and load it
    TIntfEdTab* pwndNewTab = new TIntfEdTab(strPath, strRelPath);
    pwndNewTab->CreateTab(*m_pwndEditPane, strTabText);
}


//
//  Called when the user asks to edit a triggered event. We load up a tab for this
//  event to edit it.
//
tCIDLib::TVoid TMainFrameWnd::EditTrigEvent(const TString& strPath)
{
    //
    //  We need to remove the prefix part of the path, since the IDE works in terms
    //  of type relative paths.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::TrgEvent, strRelPath);

    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Let's read it in
    try
    {
        TDataSrvClient  dsclToUse;
        TCQCTrgEvent    csrcEdit;

        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        tCIDLib::TKVPFList  colMeta;
        dsclToUse.bReadTrigEvent
        (
            strRelPath
            , c4SerialNum
            , enctLastChange
            , csrcEdit
            , colMeta
            , facCQCAdmin.sectUser()
        );

        // Create the new tab and load it
        TTrigEvTab* pwndNewTab = new TTrigEvTab(strPath, strRelPath);
        pwndNewTab->CreateTab(*m_pwndEditPane, strTabText, csrcEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText()
            , facCQCAdmin.strMsg
              (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::TrgEvent)
              )
        );
        msgbView.ShowIt(*this);
    }
}



//
//  Called when the user asks to edit a user account.
//
tCIDLib::TVoid TMainFrameWnd::EditUserAccount(const TString& strPath)
{
    try
    {
        // Get the relative path
        TString strRelPath;
        facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::User, strRelPath);

        // Get the last bit of the path, which is the user login name
        TString strLoginName;
        facCQCRemBrws().QueryNamePart(strPath, strLoginName);

        // And query the user account info
        tCQCKit::TSecuritySrvProxy orbcSrc = facCQCKit().orbcSecuritySrvProxy();
        TCQCUserAccount uaccEdit;
        orbcSrc->QueryAccount(strLoginName, facCQCAdmin.sectUser(), uaccEdit);

        //
        //  If the current user's user role is less than the selected account, then
        //  tell them they cannot do it.
        //
        if (facCQCAdmin.eUserRole() < uaccEdit.eRole())
        {
            TOkBox msgbView
            (
                strWndText(), facCQCAdmin.strMsg(kCQCAErrs::errcUAcc_SubPrivLevel)
            );
            msgbView.ShowIt(*this);
            return;
        }

        // Create the new tab and load it
        TUserAcctTab* pwndNewTab = new TUserAcctTab(strPath, strRelPath);
        pwndNewTab->CreateTab(*m_pwndEditPane, uaccEdit.strLoginName(), uaccEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText()
            , facCQCAdmin.strMsg
              (
                kCQCAMsgs::midStatus_CantEdit
                , tCQCRemBrws::strLoadEDTypes(tCQCRemBrws::EDTypes::User)
              )
        );
        msgbView.ShowIt(*this);
    }

}



//
//  Called when the user asks to 'view' a driver. We open a tab that lets the user
//  see the fields and monitors their values and so forth.
//
tCIDLib::TVoid TMainFrameWnd::ViewDriver(const TString& strPath)
{
    // Get the part of the path we want to be displayed in the tab text
    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Get the relative path
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::Driver, strRelPath);

    // Create the new tab and load it
    TDrvMonTab* pwndNewTab = new TDrvMonTab(strPath, strRelPath);
    pwndNewTab->CreateTab(*m_pwndEditPane, strTabText);
}


//
//  Called when the user asks to view help. We have a main documentation path, and we have
//  an 'about' path, to show system info. For the documentation, we just invoke the default
//  browser with the right URL.
//
tCIDLib::TVoid TMainFrameWnd::ViewHelp(const TString& strPath)
{
    // Get the relative path
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::Help, strRelPath);

    // Create the new tab and load it
    if (strPath.bStartsWith(kCQCRemBrws::strPath_Help_Docs))
    {
        tCIDLib::TBoolean bSecure;
        tCIDLib::TIPPortNum ippnWS;
        TString strWSHost;
        if (!facCQCKit().bFindHelpWS(strWSHost, ippnWS, bSecure))
        {
            // Tell the user we can't load any content
            TErrBox msgbErr
            (
                L"The address of the Master Server's Web Server could not be gotten"
            );
            msgbErr.ShowIt(*this);
            return;
        }

        //
        //  OK, looks like we have it, so build up the appropriate URL and set it on the
        //  web browser. We always have the same base URL. If it's one of the direct links
        //  then we add some query parameters.
        //
        TString strURL(bSecure ? L"https://" : L"http://");
        strURL.Append(strWSHost);
        strURL.Append(kCIDLib::chColon);
        strURL.AppendFormatted(ippnWS);
        strURL.Append(L"/Web2/CQCDocs/CQCDocs.html");

        if (strPath == kCQCRemBrws::strPath_Help_Acts_BigPicture)
            strURL.Append(L"?topic=/BigPicture/&page=/Actions");
        else if (strPath == kCQCRemBrws::strPath_Help_Acts_Reference)
            strURL.Append(L"?topic=/Reference/Actions&page=/Overview");
        else if (strPath == kCQCRemBrws::strPath_Help_CML_BigPicture)
            strURL.Append(L"?topic=/BigPicture/&page=/CMLLanguage");
        else if (strPath == kCQCRemBrws::strPath_Help_CML_Reference)
            strURL.Append(L"?topic=/Reference/CMLLanguage/&page=/Overview");

        else if (strPath == kCQCRemBrws::strPath_Help_Drivers_BigPicture)
            strURL.Append(L"?topic=/AdvancedTopics/DriverDev&page=/Overview");
        else if (strPath == kCQCRemBrws::strPath_Help_Drivers_Reference)
            strURL.Append(L"?topic=/Reference/Drivers&page=/Overview");

        else if (strPath == kCQCRemBrws::strPath_Help_Interface_BigPicture)
            strURL.Append(L"?topic=/BigPicture/&page=/TouchScreens");
        else if (strPath == kCQCRemBrws::strPath_Help_Interface_Reference)
            strURL.Append(L"?topic=/Reference/IntfWidgets&page=/Overview");

        else if (strPath == kCQCRemBrws::strPath_Help_Intro)
            strURL.Append(L"?topic=/AutomationIntro&page=/Overview");
        else if (strPath == kCQCRemBrws::strPath_Help_Install)
            strURL.Append(L"?topic=/Installation&page=/Overview");
        else if (strPath == kCQCRemBrws::strPath_Help_Tools)
            strURL.Append(L"?topic=/Tools&page=/Overview");
        else if (strPath == kCQCRemBrws::strPath_Help_Videos)
            strURL.Append(L"?topic=/Videos&page=/Overview");
        else if (strPath == kCQCRemBrws::strPath_Help_V2Classes)
            strURL.Append(L"?topic=/Reference/DevClasses&page=/Overview");

        else if (strPath == kCQCRemBrws::strPath_Help_VoiceControl_BigPicture)
            strURL.Append(L"?topic=/BigPicture/&page=/VoiceCtrl");
        else if (strPath == kCQCRemBrws::strPath_Help_VoiceControl_Echo)
            strURL.Append(L"?topic=/AdvancedTopics/AmazonEcho&page=/Overview");
        else if (strPath == kCQCRemBrws::strPath_Help_VoiceControl_CQCVoice)
            strURL.Append(L"?topic=/Reference/CQCVoice&page=/Overview");

        TGUIShell::InvokeDefBrowser(*this, strURL);
    }
     else if (strPath == kCQCRemBrws::strPath_Help_About)
    {
        // Show our generic about dialog
        facCQCIGKit().AboutDlg(*this);
    }
}



//
//  Called when the user asks to view an image. We just ask for the image and load
//  into an image view tab.
//
tCIDLib::TVoid TMainFrameWnd::ViewImage(const TString& strPath)
{
    //
    //  We need to remove the prefix part of the path, since the data server client
    //  works in terms of file type relative paths.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::Image, strRelPath);

    TBitmap bmpDisplay;
    try
    {
        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        TDataSrvClient dsclRead;
        tCIDLib::TKVPFList  colMeta;
        TPNGImage imgToLoad;

        const tCIDLib::TBoolean bRes = dsclRead.bReadImage
        (
            strRelPath
            , c4SerialNum
            , enctLastChange
            , imgToLoad
            , colMeta
            , facCQCAdmin.sectUser()
        );

        if (!bRes)
        {
            TErrBox msgbView
            (
                strWndText()
                , facCQCAdmin.strMsg(kCQCAMsgs::midStatus_CantReadFile, strPath)
            );
            msgbView.ShowIt(*this);
            return;
        }

        // Create a bitmap from it
        TGraphWndDev gdevCompat(*this);
        bmpDisplay = TBitmap(gdevCompat, imgToLoad);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText(), facCQCAdmin.strMsg(kCQCAMsgs::midStatus_CantReadFile, strPath)
        );
        msgbView.ShowIt(*this);
        return;
    }

    // Get the part of the path we want to be displayed in the tab text
    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Create the new tab and load it
    TImgViewTab* pwndNewTab = new TImgViewTab(strPath, strRelPath);
    pwndNewTab->CreateTab(*m_pwndEditPane, strTabText, bmpDisplay);
}



//
//  Called when the user asks to view a macro. We just ask for the text of the
//  macro and load it into a text view tab.
//
tCIDLib::TVoid TMainFrameWnd::ViewMacro(const TString& strPath)
{
    //
    //  We need to remove the prefix part of the path, since the data server client
    //  works in terms of file type relative paths.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::Macro, strRelPath);

    TString strText;
    try
    {
        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        TDataSrvClient dsclRead;
        tCIDLib::TKVPFList  colMeta;
        const tCIDLib::TBoolean bRes = dsclRead.bReadMacro
        (
            strRelPath
            , c4SerialNum
            , enctLastChange
            , strText
            , kCIDLib::False
            , colMeta
            , facCQCAdmin.sectUser()
        );

        if (!bRes)
        {
            TErrBox msgbView
            (
                strWndText()
                , facCQCAdmin.strMsg(kCQCAMsgs::midStatus_CantReadFile, strPath)
            );
            msgbView.ShowIt(*this);
            return;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbView
        (
            strWndText(), facCQCAdmin.strMsg(kCQCAMsgs::midStatus_CantReadFile, strPath)
        );
        msgbView.ShowIt(*this);
        return;
    }

    // The tab display text is the base part of the name, not the whole path
    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    // Create the new tab and load it
    TTextViewTab* pwndNewTab = new TTextViewTab(strPath, strRelPath, L"CML Macro");
    pwndNewTab->CreateTab(*m_pwndEditPane, strTabText, strText, kCIDLib::True);
}



//
//  Called when the user invokes any of the stuff in the /System part of the browser
//  hiearchy. This is all for system type info and tools. They all get passed here and
//  look at the path to decide what we need to do.
//
tCIDLib::TVoid TMainFrameWnd::ViewSystemInfo(const TString& strPath)
{
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::System, strRelPath);

    if (strPath == kCQCRemBrws::strPath_ExploreLogs)
    {
        // Load up the load query tab
        TQueryLogsTab* pwndNewTab = new TQueryLogsTab(strPath, strRelPath);
        pwndNewTab->CreateTab(*m_pwndEditPane, L"Query Logs");
    }
     else if (strPath == kCQCRemBrws::strPath_MonitorEvTriggers)
    {
        TMonEvTrigsTab* pwndNewTab = new TMonEvTrigsTab(strPath, strRelPath);
        pwndNewTab->CreateTab(*m_pwndEditPane, L"Event Triggers");
    }
}
