//
// FILE NAME: CQCWebSrvC_WebSockCMLHandler.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/17/2014
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
//  This file implements a derivative of the base Websockets handler. In this case it
//  is a generalized one that works in terms of an abstract CML class interface. So
//  this lets users create their own Websocket handlers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"
#include    "CIDMacroEng_NetClasses_.hpp"


// ---------------------------------------------------------------------------
//   CLASS: TWebsockCMLThread
//  PREFIX: thr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWebsockCMLThread: Constructors and destructor
// ---------------------------------------------------------------------------
TWebsockCMLThread::TWebsockCMLThread(const tCIDLib::TBoolean bSimMode) :

    TWebsockThread(tCQCWebSrvC::EWSockTypes::CML, bSimMode)
    , m_pmecvHandler(0)
    , m_pmeciHandler(0)
    , m_pmefrData(0)
    , m_pmeTarget(0)
{
}

TWebsockCMLThread::~TWebsockCMLThread()
{
}


// ---------------------------------------------------------------------------
//  TWebsockCMLThread: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Just a pass through to the protected method in the base class that handles sending a
//  text message from a string, so that our CML web socket base class can have access to
//  a message sending method on behalf of the CML code he runs.
//
tCIDLib::TVoid TWebsockCMLThread::SendTextMsg(const TString& strToSend)
{
    QueueTextMsg(strToSend);
}


//
//  This is called when we are run within the IDE. We get the macro engine, and the
//  info and value objects for the handler class. We also get the class path of the
//  class running in the IDE. We store it as the class path. Above, when the base
//  class calls our init, we verify that the URL resolves to this class or it is
//  rejected.
//
tCIDLib::TVoid
TWebsockCMLThread::Simulate(        TCIDMacroEngine&    meTarget
                            ,       TMEngClassVal&      mecvHandler
                            , const TString&            strClassPath)
{
    m_pmecvHandler = &mecvHandler;
    m_pmeciHandler = &meTarget.meciFind(strClassPath);
    m_pmeTarget    = &meTarget;
    m_strClassPath = strClassPath;
}



// ---------------------------------------------------------------------------
//  TWebsockCMLThread: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TCard4
TWebsockCMLThread::c4WSInitialize(  const   TURL&       urlReq
                                    ,       TString&    strRepText
                                    ,       TString&    strErrText)
{
    // Expand the path component of the URL, so we can examine it
    TString strExpPath;
    TURL::ExpandTo
    (
        urlReq.strPath()
        , strExpPath
        , TURL::EExpTypes::Path
        , tCIDLib::EAppendOver::Overwrite
    );

    // We'll build up the path to the CML macro
    TPathStr pathToUse(L"MEng.");

    //
    //  First make sure it starts with the right path. We require this and it's used
    //  to create a CML class path to invoke.
    //
    tCIDLib::TCard4 c4BaseLen;
    if (strExpPath.bStartsWithI(L"/WebSock/User/"))
    {
        pathToUse.Append(L"User.Websock.");
        c4BaseLen = 14;
    }
     else if (strExpPath.bStartsWithI(L"/WebSock/System/"))
    {
        pathToUse.Append(L"System.Websock.");
        c4BaseLen = 16;
    }
     else
    {
        strRepText = L"Bad Request";
        strErrText = facCQCWebSrvC().strMsg(kCQCWSCErrs::errcMacro_WSBaseURL);
        return kCIDNet::c4HTTPStatus_BadRequest;
    }

    //
    //  We have the starting bit, so append the part of the path after the prefix
    //  we figured out above. after converting the slashes to periods.
    //
    strExpPath.bReplaceChar(L'/', L'.');
    pathToUse.AppendSubStr(strExpPath, 14);

    // Make sure there's no trailing one, which could be the case for the URL
    if (pathToUse.chLast() == L'.')
        pathToUse.DeleteLast();

    //
    //  They can put an .html extension on it, in order to keep stupid browsers like
    //  IE from thinking it should download a file. So strip that off.
    //
    TString strExt;
    if (pathToUse.bQueryExt(strExt))
    {
        if ((strExt == L"html") || (strExt == L"htm"))
            pathToUse.bRemoveExt();
    }

    //
    //  If in simulator mode, then the class has already been parsed and we already
    //  have the macro engine and all the other info. So we can basically just return
    //  success. We do need to check that the URL maps to the class path that the
    //  IDE is currently running.
    //
    if (bSimMode())
    {
        if (!pathToUse.bCompareI(m_strClassPath))
        {
            strRepText = L"Unknown CML handler class";
            strErrText = facCQCWebSrvC().strMsg(kCQCWSCErrs::errcMacro_SimClass, pathToUse);
            return kCIDNet::c4HTTPStatus_NotFound;
        }

        strRepText = L"Success";
        return kCIDNet::c4HTTPStatus_OK;
    }


    //
    //  Not in sim mode, so we need to parse the class and set up the value/info objects.
    //  Make sure they are initially zero.
    //
    m_pmecvHandler = 0;
    m_pmeciHandler = 0;
    try
    {
        // This is non-sim mode, so we have to allocate our own macro engine
        m_pmeTarget = new TCIDMacroEngine;

        // Store the path now for later use
        m_strClassPath = pathToUse;

        //
        //  Set up our macro engine and the handlers we need and get them plugged
        //  into the macro engine.
        //
        TCQCMEngErrHandler          meehLogger;
        TMacroEngParser             meprsToUse;
        m_pmeTarget->SetErrHandler(&meehLogger);

        //
        //  Set up a file loading handler to load from our standard macro root path.
        //  This has to be kept around since it will be used as the macro runs.
        //
        m_pmefrData = new TMEngFixedBaseFileResolver(facCQCKit().strMacroRootPath());
        m_pmeTarget->SetFileResolver(m_pmefrData);

        //
        //  Try to download and parse the macro. If this fails, we return an error page.
        //  The class object will be owned by the macro engine, added to it by the
        //  parsing process.
        //
        TMEngClassInfo* pmeciTar;
        {
            TCQCMEngClassMgr    mecmCQC(facCQCWebSrvC().sectUser());
            TCQCPrsErrHandler   meehParse;
            if (!meprsToUse.bParse(m_strClassPath, pmeciTar, m_pmeTarget, &meehParse, &mecmCQC))
            {
                strRepText = L"Macro Load Failed";
                strErrText = facCQCWebSrvC().strMsg(kCQCWSCErrs::errcMacro_CantLoad, m_strClassPath);
                return kCIDNet::c4HTTPStatus_NotFound;
            }
        }

        //
        //  Make sure it derives from the correct base class, else we can't
        //  use it. If not, then return an error page.
        //
        TCMLWSockBaseInfo* pmeciParent = m_pmeTarget->pmeciFindAs<TCMLWSockBaseInfo>
        (
            TCMLWSockBaseInfo::strPath(), kCIDLib::True
        );

        if (pmeciTar->c2ParentClassId() != pmeciParent->c2Id())
        {
            strRepText = L"Incorrect base class";
            strErrText = facCQCWebSrvC().strMsg
            (
                kCQCWSCErrs::errcMacro_WSBaseClass, TCMLWSockBaseInfo::strPath()
            );
            return kCIDNet::c4HTTPStatus_BadRequest;
        }

        //
        //  Looks ok, so create the value object for this class. We own this guy so
        //  make sure it gets cleaned up if we throw before we we get out of here.
        //
        TMEngClassVal* pmecvTarget = pmeciTar->pmecvMakeStorage
        (
            L"WebSockHandler", *m_pmeTarget, tCIDMacroEng::EConstTypes::NonConst
        );
        TJanitor<TMEngClassVal> janVal(pmecvTarget);

        //
        //  Store us on the value object, in the extra storage. This provides the
        //  needed link back when the CML class makes outgoing calls.
        //
        pmecvTarget->SetExtra(this);

        //
        //  Store the value and info classes in members for later use when invoking
        //  callbacks. We have to cast the value object to the base type
        //
        //  NOTE: We need to set these before we call the ctor, but they are also
        //  in janitors, and so we need to be sure they get cleared if we throw or
        //  exist early, else our dtor will try to delete them after the janitors
        //  already did.
        //
        m_pmecvHandler = pmecvTarget;
        m_pmeciHandler = pmeciTar;

        // Call its default constructor. If it fails, return an error page
        if (!m_pmeTarget->bInvokeDefCtor(*pmecvTarget, &facCQCWebSrvC().cuctxToUse()))
        {
            m_pmecvHandler = 0;
            m_pmeciHandler = 0;

            strRepText = L"Macro ctor failed";
            strErrText = facCQCWebSrvC().strMsg(kCQCWSCErrs::errcMacro_DefCtor, m_strClassPath);
            return kCIDNet::c4HTTPStatus_BadRequest;
        }

        // Release the value object janitor
        janVal.Orphan();
    }

    catch(TError& errToCatch)
    {
        // Clear the pointers
        m_pmecvHandler = 0;
        m_pmeciHandler = 0;

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcMacro_ExceptLoad
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
        return kCIDNet::c4HTTPStatus_SrvError;
    }

    catch(...)
    {
        // Clear the two pointers
        m_pmecvHandler = 0;
        m_pmeciHandler = 0;

        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcMacro_ExceptLoad
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
        return kCIDNet::c4HTTPStatus_SrvError;
    }

    // Tell him it all went well
    strRepText = L"Success";
    return kCIDNet::c4HTTPStatus_OK;
}


//
//  The base class calls this regularly so derived classes can check to see if there's
//  some shutdown request that they are aware of other than the usual one (which would cause
//  a thread shutdown request in the normal sort of way.)
//
//  In our case, we call the check IDE request method. If we are in the IDE and it's been
//  asked to stop, this will throw an debugger exit exception, which will propagate back to
//  the base class and stop it.
//
tCIDLib::TVoid TWebsockCMLThread::CheckShutdownReq() const
{
    try
    {
        m_pmeTarget->CheckIDEReq();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    catch(const TDbgExitException&)
    {
        throw;
    }

    catch(const TExceptException&)
    {
        // A CML level exception leaked out
        m_pmeTarget->LogLastExcept();

        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcMacro_CMLExcept
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
}


tCIDLib::TVoid TWebsockCMLThread::Connected()
{
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // No return value so push a void object
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::NonConst);

        tCIDLib::TCard2 c2MethId = m_pmeciHandler->c2FindMethod(L"Connected");
        m_pmeTarget->meciPushMethodCall(m_pmeciHandler->c2Id(), c2MethId);
        m_pmeciHandler->Invoke
        (
            *m_pmeTarget, *m_pmecvHandler, c2MethId, tCIDMacroEng::EDispatch::Poly
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    catch(const TExceptException&)
    {
        // A CML level exception leaked out
        m_pmeTarget->LogLastExcept();

        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcMacro_CMLExcept
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
}


tCIDLib::TVoid TWebsockCMLThread::Disconnected()
{
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // No return value so push a void object
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::NonConst);

        tCIDLib::TCard2 c2MethId = m_pmeciHandler->c2FindMethod(L"Disconnected");
        m_pmeTarget->meciPushMethodCall(m_pmeciHandler->c2Id(), c2MethId);
        m_pmeciHandler->Invoke
        (
            *m_pmeTarget, *m_pmecvHandler, c2MethId, tCIDMacroEng::EDispatch::Poly
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    catch(const TExceptException&)
    {
        // A CML level exception leaked out
        m_pmeTarget->LogLastExcept();

        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcMacro_CMLExcept
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
}


tCIDLib::TVoid
TWebsockCMLThread::FieldChanged(const   TString&            strMon
                                , const TString&            strField
                                , const tCIDLib::TBoolean   bGoodValue
                                , const TCQCFldValue&       fvValue)
{
    try
    {
        // Get the value formatted out if a good value, else an empty string
        TString strVal;
        if (bGoodValue)
            fvValue.Format(strVal);

        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // No return value so push a void object
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::NonConst);

        // Push the four values
        m_pmeTarget->PushString(strMon, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushString(strField, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushString(strVal, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushBoolean(bGoodValue, tCIDMacroEng::EConstTypes::Const);

        tCIDLib::TCard2 c2MethId = m_pmeciHandler->c2FindMethod(L"FieldChanged");
        m_pmeTarget->meciPushMethodCall(m_pmeciHandler->c2Id(), c2MethId);
        m_pmeciHandler->Invoke
        (
            *m_pmeTarget, *m_pmecvHandler, c2MethId, tCIDMacroEng::EDispatch::Poly
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    catch(const TExceptException&)
    {
        // A CML level exception leaked out
        m_pmeTarget->LogLastExcept();

        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcMacro_CMLExcept
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
}


//
//  We don't currently pass this to the CML class, but we could
//
tCIDLib::TVoid TWebsockCMLThread::Idle()
{
}


//
//  We pass the message on to the derived CML classes.
//
tCIDLib::TVoid
TWebsockCMLThread::ProcessMsg(const TString& strMsg)
{
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // No return value so push a void object
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::NonConst);

        // Get the msg text into a CML string and push it
        m_pmeTarget->PushString(strMsg, tCIDMacroEng::EConstTypes::Const);

        tCIDLib::TCard2 c2MethId = m_pmeciHandler->c2FindMethod(L"ProcessMsg");
        m_pmeTarget->meciPushMethodCall(m_pmeciHandler->c2Id(), c2MethId);
        m_pmeciHandler->Invoke
        (
            *m_pmeTarget, *m_pmecvHandler, c2MethId, tCIDMacroEng::EDispatch::Poly
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    catch(const TExceptException&)
    {
        // A CML level exception leaked out
        m_pmeTarget->LogLastExcept();

        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcMacro_CMLExcept
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
}


//
//  This is a method called by the base TThread class when the thread has ended.
//  We override it to get a chance to clean up.
//
tCIDLib::TVoid TWebsockCMLThread::WSTerminate()
{
    m_strClassPath.Clear();

    if (!bSimMode())
    {
        //
        //  Clean up any per-session stuff we allocate. Note we don't own the class
        //  info object, the macro engine does, in all cases. The value object we
        //  own if not simulating, since we created it. If simulating then the macro
        //  engine created it as well.
        //
        try
        {
            delete m_pmecvHandler;
            m_pmecvHandler = 0;
        }

        catch(...)
        {
        }

        try
        {
            delete m_pmefrData;
            m_pmefrData = 0;
        }

        catch(...)
        {
        }

        try
        {
            delete m_pmeTarget;
            m_pmeTarget = 0;
        }

        catch(...)
        {
        }
    }

    // Call our parent class as well
    TWebsockThread::WSTerminate();
}

