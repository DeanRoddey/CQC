//
// FILE NAME: CQCWBHelper_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2016
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
//  This file implements the facilty class for the web browser helper facility.
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
#include    "CQCWBHelper.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCWBHelper,TGUIFacility)


// ---------------------------------------------------------------------------
//  A little comparator function for queue message
// ---------------------------------------------------------------------------
static tCIDLib::TBoolean
bCompFunc(const TQMsg& qmsg1, const TQMsg& qmsg2, const tCIDLib::TCard4)
{
    return qmsg1.m_eType == qmsg2.m_eType;
}


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCWBHelper
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCWBHelper: Constructors and Destructor
// ---------------------------------------------------------------------------

TFacCQCWBHelper::TFacCQCWBHelper() :

    TGUIFacility
    (
        L"CQCWBHelper"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bSilentMode(kCIDLib::False)
    , m_c4RefreshMins(0)
    , m_colMsgQ(tCIDLib::EMTStates::Safe)
    , m_enctLastMsg(TTime::enctNow())
    , m_porbsCtrl(nullptr)
{
}

TFacCQCWBHelper::~TFacCQCWBHelper()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCWBHelper: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return the next available message, without waiting. If nothing, returns false.
tCIDLib::TBoolean TFacCQCWBHelper::bGetNextMsg(TQMsg& qmsgToFill)
{
    return m_colMsgQ.bGetNext(qmsgToFill, 0);

}


// Called by the main window to see if we have timed out
tCIDLib::TBoolean TFacCQCWBHelper::bTimedOut()
{
    return m_enctLastMsg + kCQCWBHelper::enctTimeout < TTime::enctNow();
}


tCIDLib::TVoid TFacCQCWBHelper::Close()
{
    // Just queue it up
    QueueMsg(TQMsg(tCQCWBHelper::EQMsgTypes::Close));
}


tCIDLib::TVoid TFacCQCWBHelper::DoOp(const tWebBrowser::ENavOps eToDo)
{
    // Just queue it up
    QueueMsg(TQMsg(eToDo));
}


tCIDLib::TVoid
TFacCQCWBHelper::Initialize(const   TArea&              areaPar
                            , const TArea&              areaWidget
                            , const tCIDLib::TBoolean   bVisibility
                            , const TString&            strInitURL)
{
    // Just queue it up
    QueueMsg(TQMsg(areaPar, areaWidget, bVisibility, strInitURL));
}


tCIDLib::EExitCodes TFacCQCWBHelper::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    try
    {
        // Get the standard environmental info
        TString strFailReason;
        if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::False))
        {
            return tCIDLib::EExitCodes::BadEnvironment;
        }

        // Initialize the client side ORB
        facCIDOrb().InitClient();

        // Install a logger
        TModule::InstallLogger
        (
            new TLogSrvLogger(facCQCKit().strLocalLogDir())
            , tCIDLib::EAdoptOpts::Adopt
        );

        if (facCQCWBHelper().bLogInfo())
        {
            facCQCWBHelper().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kWHBMsgs::midStatus_Start
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        //
        //  We get a single command line parameter, which is the unique id the invoking
        //  client assigned us.
        //
        tCIDLib::TCard4 c4Count = TSysInfo::c4CmdLineParmCount();
        if (c4Count != 1)
            return tCIDLib::EExitCodes::BadParameters;

        // Get the first one, which should be the unique id
        TString strUniqueId = TSysInfo::strCmdLineParmAt(0);

        // Open the memory buffer he created for us
        TResourceName rsnBuf(kCIDLib::pszResCompany, L"CQCWBHelper", strUniqueId);
        tCIDLib::TBoolean bCreated;
        TSharedMemBuf mbufInfo
        (
            4096
            , 8192
            , rsnBuf
            , bCreated
            , tCIDLib::EMemAccFlags::ReadWrite
            , tCIDLib::ECreateActs::OpenIfExists
        );

        // And the ready event
        TEvent evDataReady
        (
            rsnBuf
            , bCreated
            , tCIDLib::EEventStates::Reset
            , tCIDLib::ECreateActs::OpenIfExists
            , kCIDLib::True
        );

        //
        //  Stream in the data he passes us. The first Card4 is the overall size, which we
        //  need to set up the input stream.
        //
        tCIDLib::TBoolean bSilentMode;
        tCIDLib::TCard4 c4RefreshMins;
        tCIDCtrls::TWndHandle hwndOwner;
        {
            TBinMBufInStream strmSrc(&mbufInfo, mbufInfo.c4At(0));

            // Skip the size
            tCIDLib::TCard4 c4Size;
            strmSrc >> c4Size;

            // Next should be a start object mark
            strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

            // And now the data
            TCardinal cHandle;
            strmSrc >> bSilentMode
                    >> c4RefreshMins
                    >> cHandle;

            hwndOwner = tCIDCtrls::TWndHandle(cHandle.c4Val());

            // And it should end with an end object marker
            strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
        }

        #if CID_DEBUG_ON
        // thrThis.Sleep(15000);
        #endif

        //
        //  Initialize the server side ORB. We only accept one connection, and let the
        //  system select the port.
        //
        facCIDOrb().InitServer(0, 1);

        //
        //  We have to give it time to get it's listener going, which it does async, because
        //  we have to report the selected port.
        //
        c4Count = 0;
        while (!facCIDOrb().ippnORB() && (c4Count < 20))
        {
            if (!thrThis.bSleep(100))
                break;
        }

        // If we never got it, give up
        if (!facCIDOrb().ippnORB())
        {
            return tCIDLib::EExitCodes::Normal;
        }

        //
        //  Let's construct our faux object id.
        //
        //  NOTE: For some reason the compiler insists this must be done via
        //  move assignment, which this class doesn't support. So I have to
        //  create a temporary and then assign that.
        //
        TOrbObjId ooidTmp
        (
            TCQCWBHelperServerBase::strInterfaceId
            , TUniqueId::strMakeId()
            , L"localhost"
            , facCIDOrb().ippnORB()
            , L"TCQCWBHelperClientProxy"
        );
        m_ooidServer = ooidTmp;

        // Create our control interface with this object id
        m_porbsCtrl = new TWBHelperCtrlImpl(m_ooidServer);

        //
        //  Register it with the ORB. No need for the rebinder here because we don't use
        //  the name server.
        //
        facCIDOrb().RegisterObject(m_porbsCtrl, tCIDLib::EAdoptOpts::Adopt);

        // Create the main frame window
        m_wndMainFrame.Create(hwndOwner, bSilentMode, m_c4RefreshMins);

        // Create an output stream over the shared memory and put out the return info
        tCIDLib::TCard4 c4Size = 0;
        {
            TBinMBufOutStream strmTar(&mbufInfo);

            // Skip the initial Card4 which has the final size
            strmTar << tCIDLib::TCard4(0)
                    << tCIDLib::EStreamMarkers::StartObject
                    << m_ooidServer
                    << tCIDLib::TCard4(m_wndMainFrame.hwndThis())
                    << tCIDLib::EStreamMarkers::EndObject
                    << kCIDLib::FlushIt;

            c4Size = strmTar.c4CurSize();
        }

        // Put the size into the first card4 now.
        mbufInfo.PutCard4(c4Size, 0);

        // And trigger the ready event to let him know the return data is ready
        evDataReady.Trigger();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return tCIDLib::EExitCodes::InitFailed;
    }

    catch(...)
    {
        return tCIDLib::EExitCodes::InitFailed;
    }

    // Process messages until we are told to exit
    facCIDCtrls().uMainMsgLoop(m_wndMainFrame);

    try
    {
        //
        //  First clean up the frame window, which will destroy the browser widget
        //  as well.
        //
        m_wndMainFrame.Destroy();

        // And now we can terminate the ORB
        facCIDOrb().Terminate();
    }

    catch(TError& errToCatch)
    {
        #if CID_DEBUG_ON
        TExceptDlg dlgErr
        (
            TWindow::wndDesktop(), L"Web Browser Helper", L"Termination failed", errToCatch
        );
        #endif
    }

    catch(...)
    {
        #if CID_DEBUG_ON
        TErrBox msgbErr(L"Web Browser Helper", L"Termination failed");
        msgbErr.ShowIt(TWindow::wndDesktop());
        #endif
    }
    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TFacCQCWBHelper::Ping()
{
    // Just update our last activity timer
    m_enctLastMsg = TTime::enctNow();
}


tCIDLib::TVoid TFacCQCWBHelper::SetArea(const TArea& areaPar, const TArea& areaWidget)
{
    QueueMsg(TQMsg(areaPar, areaWidget));
}


tCIDLib::TVoid TFacCQCWBHelper::SetURL(const TString& strToSet)
{
    // Just queue it up
    QueueMsg(TQMsg(tCQCWBHelper::EQMsgTypes::SetURL, strToSet));
}


tCIDLib::TVoid TFacCQCWBHelper::SetVisibility(const tCIDLib::TBoolean bNewState)
{
    // Just queue it up
    QueueMsg(TQMsg(tCQCWBHelper::EQMsgTypes::SetVisibility, bNewState));
}

// ---------------------------------------------------------------------------
//  TFacCQCWBHelper: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TFacCQCWBHelper::QueueMsg(const TQMsg& qmsgToDo)
{
    m_enctLastMsg = TTime::enctNow();

    //
    //  To avoid extra work, we will update an existing command of the same type in the
    //  queue   if possible, and only add one if needed. This way, we don't do
    //  unnecessary operations   and avoid overwhelming the queue if we get behind
    //  somehow.
    //
    const tCIDLib::TBoolean bSend = m_colMsgQ.bAddOrUpdate
    (
        qmsgToDo
        , [](const TQMsg& qmsg1, const TQMsg& qmsg2)
          {
            if (qmsg1.m_eType > qmsg2.m_eType)
                return tCIDLib::ESortComps::FirstGreater;
            if (qmsg1.m_eType < qmsg2.m_eType)
                return tCIDLib::ESortComps::FirstLess;
            return tCIDLib::ESortComps::Equal;
          }
    );
    if (bSend)
        m_wndMainFrame.SendCode(kCQCWBHelper::i4NewMsg, 0);
}

