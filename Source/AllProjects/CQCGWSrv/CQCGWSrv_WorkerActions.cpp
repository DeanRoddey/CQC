//
// FILE NAME: CQCGWSrv_WorkerActions.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/16/2005
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
//  This file implements private methods of the work thread that are related
//  to invoking actions. There is a good bit of code, so these actual
//  message handling methods are split out into a few files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGWSrv.hpp"



// ---------------------------------------------------------------------------
//  TWorkerThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This one allows GW clients to invoke a global action. We pull out the path
//  of the action, and any action parameters, and do the invocation for them.
//
tCIDLib::TVoid TWorkerThread::DoGlobalAct(const TXMLTreeElement& xtnodeReq)
{
    // Get the path and flip any old style back slashes to forward slashes
    TString strActPath = xtnodeReq.xtattrNamed(L"CQCGW:ActPath").strValue();
    strActPath.bReplaceChar(kCIDLib::chBackSlash, kCIDLib::chForwardSlash);

    // OK, now try to download the action
    TCQCStdCmdSrc csrcRun;
    {
        TDataSrvClient dsclLoad;
        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        tCIDLib::TKVPFList colMeta;
        dsclLoad.bReadGlobalAction
        (
            strActPath
            , c4SerialNum
            , enctLastChange
            , csrcRun
            , colMeta
            , m_cuctxClient.sectUser()
        );
    }

    //
    //  That worked, so see if there are any children. If so, they are the
    //  parameters, so pull them out and format them correctly into a quoted
    //  comma list.
    //
    TString strParms;
    const tCIDLib::TCard4 c4ParmCount = xtnodeReq.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ParmCount; c4Index++)
    {
        const TXMLTreeElement& xtnodeParm = xtnodeReq.xtnodeChildAtAsElement(c4Index);

        if (c4Index)
            strParms.Append(kCIDLib::chComma);
        strParms.Append(kCIDLib::chQuotation);
        strParms.Append(xtnodeParm.xtnodeChildAtAsText(0).strText());
        strParms.Append(kCIDLib::chQuotation);
    }

    //
    //  Now create a standard action engine and run the action, after setting
    //  the invocation parameters on it. We have to just gen up a global
    //  variables object on the fly to make the engine happy. We don't have
    //  any persistent global variables context in this server.
    //
    TCQCStdActEngine acteGlobal(m_cuctxClient);
    TStdVarsTar ctarGlobals(tCIDLib::EMTStates::Safe, kCIDLib::False);
    TRefVector<MCQCCmdTarIntf> colExtraTars(tCIDLib::EAdoptOpts::NoAdopt, 1);

    tCQCKit::ECmdRes eRes = acteGlobal.eInvokeOps
    (
        csrcRun, kCQCKit::strEvId_OnTrigger, ctarGlobals, colExtraTars, 0, strParms
    );

    // Based on the result, we send back an ack or nak
    if (eRes == tCQCKit::ECmdRes::Ok)
        SendAckReply();
    else
        SendNakReply(L"An error occured while invoking the global action. See the logs.");
}


//
//  Handles a multi-field write operation. Basically it just contains a
//  list of write field elements. So we just run through them and call
//  WriteField for each one.
//
//  There's not really much way to make it more efficient. We can't know
//  what host each moniker is on ahead of time. We could maintain a multi-map
//  of the client proxies by moniker. But the ORB caches connections for
//  a short period, so in effect we are not closing and re-opening the
//  connection even if we have interleaved fields that are on different
//  hosts.
//
tCIDLib::TVoid TWorkerThread::MWriteField(const TXMLTreeElement& xtnodeReq)
{
    // Get the count of child nodes and loop through them all
    const tCIDLib::TCard4 c4Count = xtnodeReq.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        WriteField(xtnodeReq.xtnodeChildAtAsElement(c4Index), kCIDLib::False);

    // And send an ack since it worked
    SendAckReply();
}


// Pause/resume a scheduled event
tCIDLib::TVoid TWorkerThread::PauseSchEv(const TXMLTreeElement& xtnodeReq)
{
    const tCIDLib::TBoolean bState(xtnodeReq.xtattrNamed(L"CQCGW:State").strValue() == L"Paused");

    // Get the event path. If it's the old style, force it into the /User section
    TString strId = xtnodeReq.xtattrNamed(L"CQCGW:EvId").strValue();
    if (!strId.bStartsWithI(L"/User/"))
        strId.Prepend(L"/User/");

    //
    //  We don't care about tracking changes, so we just pass in a zero serial
    //  number and ignore the return.
    //
    tCIDLib::TCard4 c4SerNum = 0;
    tCIDLib::TEncodedTime enctLastChange;
    facCQCEvCl().SetEvPauseState
    (
        tCQCKit::EEvSrvTypes::SchEvent
        , strId
        , c4SerNum
        , enctLastChange
        , bState
        , m_cuctxClient.sectUser()
    );

    // Send back an ack reply
    SendAckReply();
}


//
//  Handles macro invocation requestions
//
tCIDLib::TVoid TWorkerThread::RunMacro(const TXMLTreeElement& xtnodeReq)
{
    //
    //  Get the name and parms out. The parameters are in optional child
    //  elements, so enumerate them and build them into a string that the
    //  macro engine accepts for text command line invocation.
    //
    const TString& strClassPath = xtnodeReq.xtattrNamed(L"CQCGW:ClassPath").strValue();
    TString strParms;
    const tCIDLib::TCard4 c4ParmCount = xtnodeReq.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ParmCount; c4Index++)
    {
        const TXMLTreeElement& xtnodeParm = xtnodeReq.xtnodeChildAtAsElement(c4Index);

        if (c4Index)
            strParms.Append(kCIDLib::chSpace);

        //
        //  They might pass an empty value, so we have to check for that, since
        //  it will mean that there's no child text node.
        //
        strParms.Append(kCIDLib::chQuotation);
        if (xtnodeParm.c4ChildCount())
            strParms.Append(xtnodeParm.xtnodeChildAtAsText(0).strText());
        strParms.Append(kCIDLib::chQuotation);
    }

    // Log an info level message indicating the macro was invoked
    if (facCQCGWSrv.bLogInfo())
    {
        facCQCGWSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSMsgs::midStatus_MacroInvoked
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , strClassPath
        );
    }

    //
    //  Parse the macro to check it's syntactically ok, and to load up the
    //  engine with the opcodes.
    //
    TCQCMEngClassMgr    mecmParse(m_cuctxClient.sectUser());
    TCQCPrsErrHandler   meehParser;
    TMacroEngParser     meprsDriver;
    TMEngClassInfo*     pmeciNew;
    if (!meprsDriver.bParse(strClassPath, pmeciNew, &m_meTarget, &meehParser, &mecmParse))
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcRun_CantCompile
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strClassPath
        );
    }

    // Reset the output stream we installed as the console stream
    m_strmOutput.Reset();

    //
    //  Looks like it worked, so create the value object and invoke the
    //  default ctor.
    //
    TMEngClassVal* pmecvTarget = pmeciNew->pmecvMakeStorage
    (
        L"$Main$", m_meTarget, tCIDMacroEng::EConstTypes::NonConst
    );
    TJanitor<TMEngClassVal> janClass(pmecvTarget);

    // Call its default constructor
    if (!m_meTarget.bInvokeDefCtor(*pmecvTarget, &m_cuctxClient))
    {
        m_meTarget.Reset();
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcRun_DefCtorFailed
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strClassPath
        );
    }

    // And invoke the macro, passing along the passed parameters
    tCIDLib::TInt4 i4Ret;
    try
    {
        i4Ret = m_meTarget.i4Run(*pmecvTarget, strParms, &m_cuctxClient);
        m_meTarget.Reset();
    }

    catch(const TError& errToCatch)
    {
        SendExceptionReply(errToCatch);
        return;
    }

    //
    //  Catch macro level exceptions specially, since they are not
    //  generally handled and the top level service loop would see it as
    //  an unknown exception and panic.
    //
    catch(const TExceptException&)
    {
        // Log the macro exception
        m_meTarget.LogLastExcept();

        //
        //  Change it into an exception reply. We have a helper that will
        //  return such a reply based on an exception object, so just create
        //  one here locally and pass that to it.
        //
        TString strClass;
        TString strErrText;
        tCIDLib::TCard4 c4Line;
        m_meTarget.bGetLastExceptInfo(strClass, strErrText, c4Line);
        m_meTarget.Reset();
        TError errRet
        (
            facCQCGWSrv.strName()
            , strClass
            , c4Line
            , strErrText
            , tCIDLib::ESeverities::Failed
        );
        SendExceptionReply(errRet);
        return;
    }

    if (facCQCGWSrv.bLogInfo())
    {
        facCQCGWSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSMsgs::midStatus_MacroCompleted
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , strClassPath
            , TInteger(i4Ret)
        );
    }

    // Build up the reply message
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:MacroResults CQCGW:MacroRetVal=\""
                << i4Ret << L"\" CQCGW:Status=\"CQCGW:Success\">";

    //
    //  Flush the macro output stream and dump the macro output to
    //  the reply stream.
    //
    m_strmOutput.Flush();
    m_strmReply << m_strmOutput.strData()
                << L"</CQCGW:MacroResults>\n</CQCGW:Msg>\n"
                << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}



//
//  Handles requests to set gateway connection options for the calling
//  client's session.
//
tCIDLib::TVoid TWorkerThread::SetOption(const TXMLTreeElement& xtnodeReq)
{
    // Get the name of the option and the new state
    const TString& strOpt = xtnodeReq.xtattrNamed(L"CQCGW:OptName").strValue();
    const TString& strState = xtnodeReq.xtattrNamed(L"CQCGW:State").strValue();

    tCQCGWSrv::EOptFlags eFlag = tCQCGWSrv::EOptFlags::None;
    if (strOpt == L"NoAlpha")
    {
        eFlag = tCQCGWSrv::EOptFlags::NoAlpha;
    }
     else if (strOpt == L"ImgInfo")
    {
        eFlag = tCQCGWSrv::EOptFlags::SendImgInfo;
    }
     else
    {
        // We don't recognize it
        if (facCQCGWSrv.bLogFailures())
        {
            facCQCGWSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCGWSErrs::errcProto_BadOptName
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , strOpt
            );
        }
    }

    // Set or clear the flag
    if (strState == L"True")
        m_eOptFlags = tCIDLib::eOREnumBits(m_eOptFlags, eFlag);
    else
        m_eOptFlags = tCIDLib::eClearEnumBits(m_eOptFlags, eFlag);

    // Send back an ack reply
    SendAckReply();
}


// Set the options for a periodic scheduled event
tCIDLib::TVoid TWorkerThread::SetPerEvInfo(const TXMLTreeElement& xtnodeReq)
{
    // Get the event path. If it's the old style, force it into the /User section
    TString strPath = xtnodeReq.xtattrNamed(L"CQCGW:EvId").strValue();
    if (!strPath.bStartsWithI(L"/User/"))
        strPath.Prepend(L"/User/");

    // Get the period and start time out
    const tCIDLib::TCard4 c4Period = xtnodeReq.xtattrNamed(L"CQCGW:Period").c4ValueAs();
    const tCIDLib::TEncodedTime enctStart = xtnodeReq.xtattrNamed(L"CQCGW:FromTime").c8ValueAs();

    //
    //  Make sure the start time is either now or beyond, or at least not more
    //  than a couple days in the past.
    //
    if (enctStart < TTime::enctNow() - (kCIDLib::enctOneDay * 2))
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_InvalidEvStartTime
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    //
    //  We don't care about tracking changes, so we just pass in a zero serial
    //  number and ignore the return.
    //
    tCIDLib::TCard4 c4SerNum = 0;
    tCIDLib::TEncodedTime enctLastChange;
    facCQCEvCl().SetPeriodicEvTime
    (
        strPath, c4SerNum, enctLastChange, enctStart, c4Period, m_cuctxClient.sectUser()
    );

    // Send an ack back
    SendAckReply();
}


// Set the options for a scheduled scheduled event
tCIDLib::TVoid TWorkerThread::SetSchEvInfo(const TXMLTreeElement& xtnodeReq)
{
    // Get the event path. If it's the old style, force it into the /User section
    TString strPath = xtnodeReq.xtattrNamed(L"CQCGW:EvId").strValue();
    if (!strPath.bStartsWithI(L"/User/"))
        strPath.Prepend(L"/User/");

    // Parse the mask out
    tCIDLib::TCard4 c4BitCount;
    tCIDLib::TCard4 c4BitMask;
    const TString&  strMask = xtnodeReq.xtattrNamed(L"CQCGW:Mask").strValue();
    if (!TCQCSchEvent::bParseMask(strMask, c4BitMask, c4BitCount))
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_InvalidEvMask
            , strMask
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    // Parse out the day, hour, minute value
    const TString&    strDHM = xtnodeReq.xtattrNamed(L"CQCGW:DHM").strValue();
    tCIDLib::TCard4   c4Day;
    tCIDLib::TCard4   c4Hour;
    tCIDLib::TCard4   c4Min;
    if (!TCQCSchEvent::bParseDHM(strDHM, c4Day, c4Hour, c4Min))
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_InvalidEvDHM
            , xtnodeReq.xtattrNamed(L"CQCGW:DHM").strValue()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    //
    //  We don't care about tracking changes, so we just pass in a zero serial
    //  number and ignore the return.
    //
    tCIDLib::TCard4 c4SerNum = 0;
    tCIDLib::TEncodedTime enctLast;
    facCQCEvCl().SetScheduledEvTime
    (
        strPath, c4SerNum, enctLast, c4Day, c4Hour, c4Min, c4BitMask, m_cuctxClient.sectUser()
    );

    // Send an ack back
    SendAckReply();
}


// Set the options for a sun-based scheduled event
tCIDLib::TVoid TWorkerThread::SetSunEvInfo(const TXMLTreeElement& xtnodeReq)
{
    // Get the event path. If it's the old style, force it into the /User section
    TString strPath = xtnodeReq.xtattrNamed(L"CQCGW:EvId").strValue();
    if (!strPath.bStartsWithI(L"/User/"))
        strPath.Prepend(L"/User/");

    // Get the offset out
    const tCIDLib::TInt4 i4Offset = xtnodeReq.xtattrNamed(L"CQCGW:Offset").i4ValueAs();

    if ((i4Offset < -60) || (i4Offset > 60))
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_InvalidSunOfs
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    //
    //  We don't care about tracking changes, so we just pass in a zero serial
    //  number and ignore the return.
    //
    tCIDLib::TCard4 c4SerNum = 0;
    tCIDLib::TEncodedTime enctLastChange;
    facCQCEvCl().SetSunBasedEvOffset
    (
        strPath, c4SerNum, enctLastChange, i4Offset, m_cuctxClient.sectUser()
    );

    // Send an ack back
    SendAckReply();
}


//
//
//  Handles the write of a single field. We get the moniker.field and the
//  value to write. The send ack thing lets the multi-write guy just call
//  this one to do the work and tell us not to send an ack and he'll send
//  one at the end.
//
tCIDLib::TVoid
TWorkerThread::WriteField(  const   TXMLTreeElement&    xtnodeReq
                            , const tCIDLib::TBoolean   bSendAck)
{
    // Split out the moniker and field name
    const TString& strField = xtnodeReq.xtattrNamed(L"CQCGW:Field").strValue();
    TString strMon;
    TString strFld;
    facCQCKit().ParseFldName(strField, strMon, strFld);

    //
    //  Create a client side admin proxy for the CQCServer that is hosting
    //  this driver, and do the write.
    //
    tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMon));
    orbcAdmin->WriteFieldByName
    (
        strMon
        , strFld
        , xtnodeReq.xtattrNamed(L"CQCGW:Value").strValue()
        , m_cuctxClient.sectUser()
        , tCQCKit::EDrvCmdWaits::DontCare
    );

    // Just send an ack back if asked to
    if (bSendAck)
        SendAckReply();
}


