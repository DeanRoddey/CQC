//
// FILE NAME: CQCAppCtrl_AppRecord.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/15/2004
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
//  This this file implements the TAppCtrlRec class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TAppCtrlRec,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCAppCtrl_AppRecord
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Some windows related constants we use here
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4UserBase = 0x400;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TAppCtrlRec
//  PREFIX: acr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppCtrlRec: Public, static methods
// ---------------------------------------------------------------------------
const TString& TAppCtrlRec::strKey(const TAppCtrlRec& acrSrc)
{
    return acrSrc.m_strMoniker;
}


// ---------------------------------------------------------------------------
//  TAppCtrlRec: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppCtrlRec::TAppCtrlRec(const  TString&    strMoniker
                        , const TString&    strTitle
                        , const TString&    strApp
                        , const TString&    strParams
                        , const TString&    strWorkingDir) :

    m_colWndMap(29, TStringKeyOps(), &TAppCtrlWndRec::strGetKey)
    , m_colWndMapIds(tCIDLib::EAdoptOpts::NoAdopt)
    , m_eFlags(tCIDLib::EExtProcFlags::None)
    , m_strFullAppPath(strApp)
    , m_strWorkingDir(strWorkingDir)
    , m_strMoniker(strMoniker)
    , m_strParams(strParams)
    , m_strTitle(strTitle)
{
    // Get out the path/name parts of the app
    TPathStr pathTmp(strApp);
    pathTmp.bQueryPath(m_strAppPath);
    pathTmp.bQueryNameExt(m_strAppName);

    // Do token replacement on the parameter string
    ReplaceTokens(m_strParams);

    // If no working dir, then set it to the root of the app volume
    if (m_strWorkingDir.bIsEmpty())
    {
        pathTmp = m_strFullAppPath;
        pathTmp.bQueryVolume(m_strWorkingDir);
    }

    //
    //  Put in a fake item for the magic 'top window', which always just
    //  refers to the topmost top level window of the application.
    //
    TAppCtrlWndRec acwrNew(L"/", TString::strEmpty(), 0, 0);
    TAppCtrlWndRec* pacwrAdded = &m_colWndMap.objAdd(acwrNew);
    m_colWndMapIds.Add(pacwrAdded);
}

TAppCtrlRec::~TAppCtrlRec()
{
}


// ---------------------------------------------------------------------------
//  TAppCtrlRec: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TAppCtrlRec::Activate()
{
    TProcess::bActivateProcess(m_extpThis);
}


tCIDLib::TBoolean TAppCtrlRec::bIsRunning() const
{
    return m_extpThis.bIsRunning();
}


tCIDLib::TBoolean
TAppCtrlRec::bIsVisible(const   tCIDLib::TCard4 c4Id
                        , const tCIDLib::TCard4 c4ChildId)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    return facCIDCtrls().bIsWindowVisible(hwndTar);
}


tCIDLib::TBoolean TAppCtrlRec::bStart()
{
    // If running, then do nothing, and return false to say we didn't start it
    if (m_extpThis.bIsRunning())
        return kCIDLib::False;

    // Build up the full command line
    TString strFullCmdLine(L"\"");
    strFullCmdLine.Append(m_strFullAppPath);
    strFullCmdLine.Append(L"\" ");
    strFullCmdLine.Append(m_strParams);

    // And start it
    m_extpThis.Start(strFullCmdLine, m_strWorkingDir, m_eFlags);

    //
    //  Wait for it to get up and running before we try to look up the
    //  main window, since otherwise we might look before it gets created.
    //  This will wait until the app starts processing input and takes
    //  care of any pending messages.
    //
    m_extpThis.WaitInit(5000);

    // Return true to indicate that we started it
    return kCIDLib::True;
}

tCIDLib::TBoolean TAppCtrlRec::bStart(const TString& strParams)
{
    // If running, then do nothing, and return false to say we didn't start it
    if (m_extpThis.bIsRunning())
        return kCIDLib::False;

    //
    //  Build up the full command line, in this case, taking the parameters
    //  provided instead of the defaults. We have to do the standard token
    //  replacements on it, so we have to get a copy.
    //
    TString strTmpParams(strParams);
    ReplaceTokens(strTmpParams);
    TString strFullCmdLine(L"\"");
    strFullCmdLine.Append(m_strFullAppPath);
    strFullCmdLine.Append(L"\" ");
    strFullCmdLine.Append(strTmpParams);

    // And start it
    m_extpThis.Start(strFullCmdLine, m_strWorkingDir, m_eFlags);

    //
    //  Wait for it to get up and running before we try to look up the
    //  main window, since otherwise we might look before it gets created.
    //  This will wait until the app starts processing input and takes
    //  care of any pending messages.
    //
    m_extpThis.WaitInit(5000);

    // Return true to indicate that we started it
    return kCIDLib::True;
}


tCIDLib::TBoolean
TAppCtrlRec::bStartViaOpen( const   TString&                strDocFile
                            , const tCIDLib::EExtProcShows  eShow)
{
    // If running, then do nothing, and return false to say we didn't start it
    if (m_extpThis.bIsRunning())
        return kCIDLib::False;

    m_extpThis.StartAsOpen(strDocFile, eShow);
    return kCIDLib::True;
}


tCIDLib::TBoolean TAppCtrlRec::bStop()
{
    // If not running, then just return true to say it's stopped
    if (!m_extpThis.bIsRunning())
        return kCIDLib::True;

    //
    //  The first registered window is supposed to be one that we can send
    //  the close message to. If we don't have any, then we just give up.
    //
    //  NOTE:   The 0th item is not the first registered window, it's the
    //          magic 'top window' placeholder.
    //
    if (m_colWndMapIds.c4ElemCount() > 1)
    {
        const TAppCtrlWndRec* pacwrTar = m_colWndMapIds[1];
        facCIDCtrls().SendShutdown(pacwrTar->hwndTarget());
    }

    // Give it a bit to die and see if it does

    return kCIDLib::True;
}


tCIDLib::TBoolean
TAppCtrlRec::bWaitForDeath(         tCIDLib::EExitCodes&    eToFill
                            , const tCIDLib::TCard4         c4MilliSecs) const
{
    return m_extpThis.bWaitForDeath(eToFill, c4MilliSecs);
}


//
//  These methods find the indicated windows and add them to the list of target
//  windows for this app and return the id to the caller.
//
tCIDLib::TCard4 TAppCtrlRec::c4AddWindow(const TString& strPath)
{
    // Make sure this path isn't already in the list
    if (m_colWndMap.bKeyExists(strPath))
    {
        facCQCTrayMon.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kTrayMonErrs::errcApp_WndAlreadyExists
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , strPath
            , m_strMoniker
        );
    }

    //
    //  Put it in the list initially as is. We will fault in the parsing
    //  of the path and storing the handle until it's used.
    //
    TAppCtrlWndRec acwrNew
    (
        strPath, TString::strEmpty(), kCIDCtrls::widInvalid, kCIDCtrls::hwndInvalid
    );
    TAppCtrlWndRec* pacwrAdded = &m_colWndMap.objAdd(acwrNew);

    //
    //  Now take the current id vector element count as the id that we
    //  will return, and then add the pointer to the id vector.
    //
    const tCIDLib::TCard4 c4Ret = m_colWndMapIds.c4ElemCount();
    m_colWndMapIds.Add(pacwrAdded);
    return c4Ret;
}



tCIDLib::TCard4 TAppCtrlRec::c4AddWindowByClass(const TString& strClass)
{
    //
    //  This lets them set the top level window by class. We already have
    //  an entry for it, so we just set the class on it. Otherwise it
    //  will just be a 'by path' type mapping.
    //
    TAppCtrlWndRec* pacwrRoot = m_colWndMap.pobjFindByKey(L"/");
    pacwrRoot->strClass(strClass);

    // The id is always zero
    return 0;
}


tCIDLib::TCard4
TAppCtrlRec::c4QueryListSel(const   tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    return tCIDLib::TCard4(facCIDCtrls().mresSendSyncMsg(hwndTar, 0x0188, 0, 0));
}


tCIDLib::TCard4
TAppCtrlRec::c4QueryText(const  tCIDLib::TCard4 c4Id
                        , const tCIDLib::TCard4 c4ChildId
                        ,       TString&        strToFill)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    return facCIDCtrls().c4QueryWndText(hwndTar, strToFill);
}


tCIDLib::TCard4
TAppCtrlRec::c4QueryWndStyles(const tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    return tCIDLib::TCard4(facCIDCtrls().c4QueryWndStyles(hwndTar));
}


tCIDLib::EExtProcFlags TAppCtrlRec::eFlags() const
{
    return m_eFlags;
}

tCIDLib::EExtProcFlags
TAppCtrlRec::eFlags(const tCIDLib::EExtProcFlags eToSet)
{
    m_eFlags = eToSet;
    return m_eFlags;
}


TExternalProcess& TAppCtrlRec::extpThis()
{
    return m_extpThis;
}

const TExternalProcess& TAppCtrlRec::extpThis() const
{
    return m_extpThis;
}


tCIDLib::EPrioClasses TAppCtrlRec::ePriorityClass() const
{
    return m_extpThis.ePriorityClass();
}

tCIDLib::EPrioClasses
TAppCtrlRec::ePriorityClass(const tCIDLib::EPrioClasses eNewClass)
{
    return m_extpThis.ePriorityClass(eNewClass);
}


tCIDLib::EExitCodes
TAppCtrlRec::eWaitForDeath(const tCIDLib::TCard4 c4MilliSecs) const
{
    return m_extpThis.eWaitForDeath(c4MilliSecs);
}


tCIDCtrls::TWndHandle
TAppCtrlRec::hwndFindWindow(const   tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId)
{
    //
    //  If the id zero, then it's the magic top window id, so we need to
    //  find the top window of the application. Else, it's a reference to
    //  one of the registered windows.
    //
    TAppCtrlWndRec* pacwrTar = m_colWndMapIds[c4Id ? c4Id : 0];
    tCIDCtrls::TWndHandle hwndTar = hwndTestWindow(*pacwrTar);

    if (hwndTar && (c4ChildId != kCIDCtrls::widInvalid))
    {
        hwndTar = facCIDCtrls().hwndFindChild(hwndTar, c4ChildId, kCIDLib::True);
        if (!hwndTar)
        {
            facCQCTrayMon.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kTrayMonErrs::errcApp_ChildWndNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(c4ChildId)
            );
        }
    }
    return hwndTar;
}


tCIDLib::TInt4
TAppCtrlRec::i4QueryTrackBar(const  tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    return facCIDCtrls().mresSendSyncMsg
    (
        hwndTar, CQCAppCtrl_AppRecord::c4UserBase, 0, 0
    );
}


tCIDLib::TInt4
TAppCtrlRec::i4SendMsg( const   tCIDLib::TCard4     c4Id
                        , const tCIDLib::TCard4     c4ChildId
                        , const tCIDLib::TCard4     c4ToSend
                        , const tCIDLib::TCard4     c4Param1
                        , const tCIDLib::TInt4      i4Param2
                        , const tCIDLib::TBoolean   bAsync)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    if (bAsync)
        return facCIDCtrls().mresSendAsyncMsg(hwndTar, c4ToSend, c4Param1, i4Param2);
    return facCIDCtrls().mresSendSyncMsg(hwndTar, c4ToSend, c4Param1, i4Param2);
}


const TString& TAppCtrlRec::strAppName() const
{
    return m_strAppName;
}


const TString& TAppCtrlRec::strAppPath() const
{
    return m_strAppPath;
}


const TString& TAppCtrlRec::strFullAppPath() const
{
    return m_strFullAppPath;
}


const TString& TAppCtrlRec::strMoniker() const
{
    return m_strMoniker;
}


const TString& TAppCtrlRec::strParams() const
{
    return m_strParams;
}


const TString& TAppCtrlRec::strTitle() const
{
    return m_strTitle;
}


const TString& TAppCtrlRec::strWorkingDir() const
{
    return m_strWorkingDir;
}


tCIDLib::TVoid
TAppCtrlRec::SendExtKey(const   tCIDLib::TCard4     c4Id
                        , const tCIDLib::TCard4     c4ChildId
                        , const tCIDCtrls::EExtKeys eKeyToSend
                        , const tCIDLib::TBoolean   bAltShifted
                        , const tCIDLib::TBoolean   bCtrlShifted
                        , const tCIDLib::TBoolean   bShifted)
{
    facCIDCtrls().SendExtKey(eKeyToSend, bAltShifted, bCtrlShifted, bShifted);
}


tCIDLib::TVoid
TAppCtrlRec::SendKey(const  tCIDLib::TCard4     c4Id
                    , const tCIDLib::TCard4     c4ChildId
                    , const tCIDLib::TCh        chToSend
                    , const tCIDLib::TBoolean   bAltShifted
                    , const tCIDLib::TBoolean   bCtrlShifted
                    , const tCIDLib::TBoolean   bShifted)
{
    facCIDCtrls().SendKey(chToSend, bAltShifted, bCtrlShifted, bShifted);
}


tCIDLib::TVoid
TAppCtrlRec::SetFocus(  const   tCIDLib::TCard4 c4Id
                        , const tCIDLib::TCard4 c4ChildId)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    facCIDCtrls().SetFocus(hwndTar, kCIDLib::True);
}


tCIDLib::TVoid
TAppCtrlRec::SetListSel(const   tCIDLib::TCard4 c4Id
                        , const tCIDLib::TCard4 c4ChildId
                        , const tCIDLib::TCard4 c4Index)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    facCIDCtrls().mresSendSyncMsg(hwndTar, 0x0186, c4Index, 0);
}


tCIDLib::TVoid
TAppCtrlRec::SetTrackBar(const  tCIDLib::TCard4 c4Id
                        , const tCIDLib::TCard4 c4ChildId
                        , const tCIDLib::TInt4  i4ToSet)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    facCIDCtrls().mresSendSyncMsg
    (
        hwndTar, CQCAppCtrl_AppRecord::c4UserBase + 5, 1, i4ToSet
    );
}


tCIDLib::TVoid
TAppCtrlRec::SetWindowPos(  const   tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId
                            , const tCIDLib::TInt4  i4X
                            , const tCIDLib::TInt4  i4Y)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    facCIDCtrls().SetWndPos(hwndTar, i4X, i4Y);
}


tCIDLib::TVoid
TAppCtrlRec::SetWindowSize( const   tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId
                            , const tCIDLib::TCard4 c4Width
                            , const tCIDLib::TCard4 c4Height)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    facCIDCtrls().SetWndSize(hwndTar, c4Width, c4Height);
}


tCIDLib::TVoid
TAppCtrlRec::SetWindowText( const   tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId
                            , const TString&        strText)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    facCIDCtrls().SetWndText(hwndTar, strText);
}


tCIDLib::TVoid
TAppCtrlRec::ShowWindow(const   tCIDLib::TCard4     c4Id
                        , const tCIDLib::TCard4     c4ChildId
                        , const tCIDLib::TBoolean   bState)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);
    facCIDCtrls().ShowWindow(hwndTar, bState);
}


tCIDLib::TVoid
TAppCtrlRec::StandardOp(const   tCIDLib::TCard4     c4Id
                        , const tCIDLib::TCard4     c4ChildId
                        , const tCQCKit::EStdACOps  eOp)
{
    tCIDCtrls::TWndHandle hwndTar = hwndFindWindow(c4Id, c4ChildId);

    switch(eOp)
    {
        case tCQCKit::EStdACOps::Maximize :
            facCIDCtrls().SetPosState(hwndTar, tCIDCtrls::EPosStates::Maximized);
            break;

        case tCQCKit::EStdACOps::Minimize :
            facCIDCtrls().SetPosState(hwndTar, tCIDCtrls::EPosStates::Minimized);
            break;

        case tCQCKit::EStdACOps::Restore :
            facCIDCtrls().SetPosState(hwndTar, tCIDCtrls::EPosStates::Restored);
            break;

        case tCQCKit::EStdACOps::ToTop :
            facCIDCtrls().SetForeground(hwndTar, kCIDLib::True);
            break;

        case tCQCKit::EStdACOps::ToBack :
            facCIDCtrls().ToBottom(hwndTar);
            break;

        default :
            break;
    };
}


tCIDLib::TVoid
TAppCtrlRec::Update(const   TString&    strTitle
                    , const TString&    strApp
                    , const TString&    strParams
                    , const TString&    strWorkingDir)
{
    m_strFullAppPath = strApp;
    m_strParams      = strParams;
    m_strTitle       = strTitle;
    m_strWorkingDir  = strWorkingDir;

    // Get out the path/name parts of the app
    TPathStr pathTmp(strApp);
    pathTmp.bQueryPath(m_strAppPath);
    pathTmp.bQueryNameExt(m_strAppName);

    // Flush the window list
    m_colWndMap.RemoveAll();
    m_colWndMapIds.RemoveAll();

    // If no working dir, then set it to the root of the app volume
    if (m_strWorkingDir.bIsEmpty())
    {
        pathTmp = m_strFullAppPath;
        pathTmp.bQueryVolume(m_strWorkingDir);
    }

    //
    //  Put in a fake item for the magic 'top window', which always just
    //  refers to the topmost top level window of the application.
    //
    TAppCtrlWndRec acwrNew(L"/", TString::strEmpty(), 0, 0);
    TAppCtrlWndRec* pacwrAdded = &m_colWndMap.objAdd(acwrNew);
    m_colWndMapIds.Add(pacwrAdded);
}


// ---------------------------------------------------------------------------
//  TAppCtrlRec: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TAppCtrlRec::ReplaceTokens(TString& strTarget)
{
    while (strTarget.bTokenExists(L'h'))
        strTarget.eReplaceToken(TSysInfo::strIPHostName(), L'h');

    while (strTarget.bTokenExists(L'u'))
        strTarget.eReplaceToken(TSysInfo::strUserName(), L'u');

    while (strTarget.bTokenExists(L't'))
    {
        TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
        tmCur.strDefaultFormat(TTime::strHHMMSS());
        strTarget.eReplaceToken(tmCur, L't');
    }
}


// ---------------------------------------------------------------------------
//  TAppCtrlRec: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::TWndHandle
TAppCtrlRec::hwndParsePath( const   TString&            strPath
                            , const TString&            strClass
                            ,       tCIDCtrls::TWndId&  widToFill)
{
    // It has to start with a slash
    if (strPath[0] != kCIDLib::chForwardSlash)
        return kCIDCtrls::hwndInvalid;

    // Ok, set up a string tokenizer to break out the parts
    TStringTokenizer stokPath(&strPath, L"/");

    tCIDCtrls::TWndHandle hwndRet = kCIDCtrls::hwndInvalid;
    TString strCurTok;
    if (strPath == L"/")
    {
        hwndRet = facCIDCtrls().hwndFindTopAppWnd
        (
            m_extpThis, strClass, kCIDLib::False
        );
    }
     else
    {
        // The first token must be the window text of a top level window
        if (stokPath.bGetNextToken(strCurTok))
        {
            hwndRet = facCIDCtrls().hwndFindAppWnd
            (
                m_extpThis, strCurTok, strClass, kCIDLib::False
            );
        }
    }

    // If not found, then give up
    if (hwndRet == kCIDCtrls::hwndInvalid)
    {
        facCQCTrayMon.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kTrayMonErrs::errcApp_TopWndNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strPath
        );

        // Won't happen, but makes analyzer happy
        return kCIDCtrls::hwndInvalid;
    }

    // If this ends up being the guy, we need to have set this
    widToFill = facCIDCtrls().widFromHandle(hwndRet);

    //
    //  Ok, now start looping till we've worked our way down the list of
    //  ids in the rest of the path, if any.
    //

    tCIDLib::TCard4     c4Count = 1;
    while (stokPath.bGetNextToken(strCurTok))
    {
        //
        //  The first char indicates what we should expect, so get it out
        //  and cut it off the token string.
        //
        const tCIDLib::TCh chType = strCurTok.chFirst();
        strCurTok.Cut(0, 1);

        const tCIDLib::TCh* const pszBuf = strCurTok.pszBuffer();

        tCIDCtrls::TWndId widNew = kCIDCtrls::widInvalid;
        if ((chType == kCIDLib::chPlusSign)
        ||  (chType == kCIDLib::chHyphenMinus))
        {
            // It must be 8 chars long
            if (strCurTok.c4Length() != 8)
            {
                hwndRet = kCIDCtrls::hwndInvalid;
                break;
            }

            //
            //  We find the next child via it's id. If it's a plus sign
            //  we search the whole underlying tree. Else, it's a direct
            //  child.
            //
            //  So convert the remaining token text to a binary value and
            //  try to find a child or descendant with that id.
            //
            tCIDLib::TBoolean bOk;
            widNew = TRawStr::c4AsBinary(pszBuf, bOk, tCIDLib::ERadices::Hex);
            if (!bOk)
            {
                hwndRet = kCIDCtrls::hwndInvalid;
                break;
            }

            //
            //  If it's 0xFFFFFFFF, then they want us to find a default top
            //  level window, else find the child with the indicated id.
            //
            if (widNew == kCIDLib::c4MaxCard)
            {
                hwndRet = facCIDCtrls().hwndFindTopAppWnd
                (
                    m_extpThis, strClass, kCIDLib::False
                );
                widNew = facCIDCtrls().widFromHandle(hwndRet);
            }
             else
            {
                hwndRet = facCIDCtrls().hwndFindChild
                (
                    hwndRet
                    , widNew
                    , chType == kCIDLib::chHyphenMinus
                );
            }
        }
         else if ((chType == kCIDLib::chDollarSign)
              ||  (chType == kCIDLib::chAmpersand))
        {
            //
            //  The token is window text that we should look for in the
            //  child windows.
            //
            hwndRet = facCIDCtrls().hwndChildFromText
            (
                hwndRet
                , widNew
                , strCurTok
                , kCIDLib::False
                , chType == kCIDLib::chDollarSign
            );
        }
         else if ((chType == kCIDLib::chLatin_f)
              ||  (chType == kCIDLib::chLatin_l))
        {
            //
            //  These indicate the first or last child window and an offset
            //  thereof. The offset means down (in z-order) from the first
            //  (previous) orup from the last (next). It must be 2 chars
            //  long and be a decimal value.
            //
            if (strCurTok.c4Length() != 2)
            {
                hwndRet = kCIDCtrls::hwndInvalid;
                break;
            }

            tCIDLib::TBoolean bOk;
            const tCIDLib::TCard4 c4Ofs = TRawStr::c4AsBinary(pszBuf, bOk, tCIDLib::ERadices::Dec);
            if (!bOk)
            {
                hwndRet = kCIDCtrls::hwndInvalid;
                break;
            }

            hwndRet = facCIDCtrls().hwndNthChildFrom(hwndRet, (chType == kCIDLib::chLatin_f), c4Ofs);
            if (hwndRet != kCIDCtrls::hwndInvalid)
                widNew = facCIDCtrls().widFromHandle(hwndRet);
        }
         else
        {
            hwndRet = kCIDCtrls::hwndInvalid;
            break;
        }

        if (hwndRet == kCIDCtrls::hwndInvalid)
        {
            facCQCTrayMon.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kTrayMonErrs::errcApp_NthChildNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(c4Count)
                , TCardinal(widNew)
            );

            // Won't happen, but makes analyzer happy
            return kCIDCtrls::hwndInvalid;
        }

        widToFill = widNew;
        c4Count++;
    }
    return hwndRet;
}


//
//  This is called to make sure that a window item has been parsed and it's
//  window handle stored, since we fault in the processing of window items.
//
tCIDCtrls::TWndHandle TAppCtrlRec::hwndTestWindow(TAppCtrlWndRec& acwrTest)
{
    // If the window is already set and is valid, then return it
    tCIDCtrls::TWndHandle hwndTar = acwrTest.hwndTarget();
    if (hwndTar && facCIDCtrls().bIsWindowValid(hwndTar))
        return hwndTar;

    //
    //  Call a helper method that will parse the path and find the window
    //  in question.
    //
    tCIDCtrls::TWndId      widNew;
    tCIDCtrls::TWndHandle  hwndNew;
    hwndNew = hwndParsePath(acwrTest.strPath(), acwrTest.strClass(), widNew);
    if (hwndNew == kCIDCtrls::hwndInvalid)
    {
        // Make sure it's marked as invalid
        acwrTest.Set(kCIDCtrls::widInvalid, kCIDCtrls::hwndInvalid);

        // Throw an badly formed path error
        facCQCTrayMon.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kTrayMonErrs::errcApp_InvalidWndPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , acwrTest.strPath()
        );
    }

    // It worked so update the record
    acwrTest.Set(widNew, hwndNew);
    return hwndNew;
}

