//
// FILE NAME: CQCMedia_WMPAudioPlayer.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/14/2007
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
//  This file implements a wrapper around the Windows Media Player SDK, for
//  doing simple headless audio playback.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Bring in the WMP SDK header and a platform specific helper header
// ---------------------------------------------------------------------------
#include    <Dshow.h>
#include    "CIDKernel_InternalHelpers_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCWMPAudioPlayer,TCQCAudioPlayer)



// ---------------------------------------------------------------------------
//  Local types and data, continued
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCMedia_WMPAudioPlayer
    {
        // -----------------------------------------------------------------------
        //  A unique name object to give unique names to our threads that we spin
        //  up for each new instance.
        // -----------------------------------------------------------------------
        TUniqueName unamThreads(L"WMPAudioPlayerThread_%(1)", 1);


        // -----------------------------------------------------------------------
        //  We will wait for up to this long for a queued command to be processed
        //  by the player thread.
        // -----------------------------------------------------------------------
        #if CID_DEBUG_ON
        constexpr tCIDLib::TCard4   c4WaitCmd = 600000;
        #else
        constexpr tCIDLib::TCard4   c4WaitCmd = 10000;
        #endif
    }
}



// ---------------------------------------------------------------------------
//  Local helper functions
// ---------------------------------------------------------------------------
static tCIDLib::TVoid XlatSysErr(tCIDLib::TCard4 c4Err, TString& strToFill)
{
    const tCIDLib::TCard4 c4BufSz = 1023;
    tCIDLib::TCh achBuf[c4BufSz + 1];

    achBuf[0] = kCIDLib::chNull;
    ::FormatMessage
    (
        FORMAT_MESSAGE_FROM_SYSTEM
        , NULL
        , c4Err
        , MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
        , achBuf
        , c4BufSz
        , NULL
    );
    achBuf[c4BufSz] = kCIDLib::chNull;

    //
    //  If it came out empty, then format the actual error code into the
    //  string.
    //
    strToFill = achBuf;
    strToFill.StripWhitespace();
    if (strToFill.bIsEmpty())
    {
        strToFill = L"DirectShow error: ";
        strToFill.AppendFormatted(c4Err, tCIDLib::ERadices::Hex);
    }
}


//
//  All of the COM APIs has to be checked for errors and the code is the
//  same for all of them here, so they just call this method and provide the
//  facility error and the underlying COM error.
//
static tCIDLib::TVoid
ThrowDSError(const  tCIDLib::TErrCode   errcToThrow
            , const tCIDLib::TCard4     c4HRes
            , const tCIDLib::TCard4     c4Line)
{
    TString strSysErr;
    XlatSysErr(c4HRes, strSysErr);
    facCQCMedia().ThrowErr
    (
        CID_FILE
        , c4Line
        , errcToThrow
        , strSysErr
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::CantDo
    );
}

static tCIDLib::TVoid
ThrowDSError(const  tCIDLib::TErrCode   errcToThrow
            , const tCIDLib::TCard4     c4HRes
            , const tCIDLib::TCard4     c4Line
            , const MFormattable&       mfmtblTok1)
{
    TString strSysErr;
    XlatSysErr(c4HRes, strSysErr);
    facCQCMedia().ThrowErr
    (
        CID_FILE
        , c4Line
        , errcToThrow
        , strSysErr
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::CantDo
        , mfmtblTok1
    );
}






// ---------------------------------------------------------------------------
//  CLASS: TCQCWMPAudioPlayer
// PREFIX: cwmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCWMPAudioPlayer: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCWMPAudioPlayer::TCQCWMPAudioPlayer(const tCIDLib::TCard4 c4DefVolume) :

    TCQCAudioPlayer(c4DefVolume)
    , m_bMuted(kCIDLib::False)
    , m_c4CurVolume(c4DefVolume)
    , m_colCmdQ(tCIDLib::EMTStates::Safe)
    , m_eEndState(tCQCMedia::EEndStates::Ended)
    , m_eState(tCQCMedia::EMediaStates::Stopped)
    , m_enctCurPos(0)
    , m_enctWatchDog(0)
    , m_mtxSync()
    , m_thrPlayer
      (
        CQCMedia_WMPAudioPlayer::unamThreads.strQueryNewName()
        , TMemberFunc<TCQCWMPAudioPlayer>(this, &TCQCWMPAudioPlayer::ePlayerThread)
      )
    , m_pBasicAudio(nullptr)
    , m_pGraphBuilder(nullptr)
    , m_pMediaControl(nullptr)
    , m_pMediaEvent(nullptr)
    , m_pMediaSeeking(nullptr)
{
}

TCQCWMPAudioPlayer::~TCQCWMPAudioPlayer()
{
    //
    //  Clean up if they didn't do the right thing and call Terminate
    //  themselves, We can only hope that they call it in the same thread
    //  that they created us in, because of the studid COM thread issues.
    //
    try
    {
        Terminate();
    }

    catch(const TError& errToCatch)
    {
        if (facCQCMedia().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TCQCWMPAudioPlayer: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Mute or unmute our player. It doesn't support a mute per se, so we just
//  have to set the volume to the minimum value and remember that we are
//  muted. We don't actually change our m_c4CurVolume member, which stays at
//  whatever the last set volume was. We use a separate boolean to remember
//  if we are in mute state or not.
//
tCIDLib::TBoolean TCQCWMPAudioPlayer::bMute() const
{
    // We don't bother to lock for a simple value like this
    return m_bMuted;
}

tCIDLib::TBoolean TCQCWMPAudioPlayer::bMute(const tCIDLib::TBoolean bToSet)
{
    // Queue up a command
    TQElem cptrEv(new TEvData(EEvTypes::Mute));
    cptrEv->m_bVal = bToSet;
    PostCmdAndWait(cptrEv);

    return m_bMuted;
}


//
//  Get/set the volume, We may not be able to actually set the volume, since
//  it can only be done when the graph is set up. So we store it away and
//  only actually set it when that's doable. The loading method will, after
//  setting up a new filter graph, reinstate the volume/mute status on the
//  graph. If we are in mute mode, then it won't do anything, and our newly
//  stored value will just be used whenever it's finally time to actually
//  set the volume.
//
tCIDLib::TCard4 TCQCWMPAudioPlayer::c4Volume() const
{
    // We don't bother to lock for a simple value like this
    return m_c4CurVolume;
}

tCIDLib::TCard4 TCQCWMPAudioPlayer::c4Volume(const tCIDLib::TCard4 c4Percent)
{
    // Store the volume for later use
    m_c4CurVolume = c4Percent;
    if (m_c4CurVolume > 100)
        m_c4CurVolume = 100;

    TQElem cptrEv(new TEvData(EEvTypes::Volume));
    cptrEv->m_c4Val = c4Percent;

    PostCmdAndWait(cptrEv);
    return m_c4CurVolume;
}


// Proivdes access to the current playback position
tCIDLib::TEncodedTime TCQCWMPAudioPlayer::enctCurPos() const
{
    // This one is 64 bit, so we lock while returning it
    TLocker lockrSync(&m_mtxSync);
    return m_enctCurPos;
}


//
//  A way to get all the status of the player quickly with a single lock
//  operation.
//
tCIDLib::TVoid
TCQCWMPAudioPlayer::GetStatusInfo(  tCQCMedia::EMediaStates&    eState
                                    , tCQCMedia::EEndStates&    eEndState
                                    , tCIDLib::TEncodedTime&    enctPlayPos)
{
    TLocker lockrSync(&m_mtxSync);
    eEndState = m_eEndState;
    eState = m_eState;
    enctPlayPos = m_enctCurPos;
}


//
//  The client should call this after constructing the player object. We
//  just use it to start up the player thread.
//
tCIDLib::TVoid TCQCWMPAudioPlayer::Initialize()
{
    m_thrPlayer.Start();
}


// We just queue up a pause command on the player's queue
tCIDLib::TVoid TCQCWMPAudioPlayer::Pause()
{
    TQElem cptrEv(new TEvData(EEvTypes::Pause));
    PostCmdAndWait(cptrEv);
}


// We just queue up a play command on the player's queue
tCIDLib::TVoid TCQCWMPAudioPlayer::Play()
{
    TQElem cptrEv(new TEvData(EEvTypes::Play));
    PostCmdAndWait(cptrEv);
}


//
//  Loads a new file to play. We either create a new filter graph or
//  clean out the old one and start a new one. We will set the volume/mute
//  settings to whatever the last values set on us are, since they can
//  only be set once the filter graph is loaded.
//
//  They can indicate a specific renderer filter, which the player thread
//  will try to use if possible. If not, he'll error out.
//
tCIDLib::TVoid
TCQCWMPAudioPlayer::LoadFile(const  TString&    strToPlay
                            , const TString&    strTargetDev)
{
    TQElem cptrEv(new TEvData(EEvTypes::Load, strToPlay, strTargetDev));
    PostCmdAndWait(cptrEv);
}


//
//  This is called to clear out any current loaded media, and it clears
//  the end of media flag. We just queue up a reset command on the player's
//  queue.
//
tCIDLib::TVoid TCQCWMPAudioPlayer::Reset()
{
    TQElem cptrEv(new TEvData(EEvTypes::Reset));
    PostCmdAndWait(cptrEv);
}


// We just queue up a stop command on the player's queue
tCIDLib::TVoid TCQCWMPAudioPlayer::Stop()
{
    TQElem cptrEv(new TEvData(EEvTypes::Stop));
    PostCmdAndWait(cptrEv);
}


// The client should call this before destructing the player object
tCIDLib::TVoid TCQCWMPAudioPlayer::Terminate()
{
    // Ask our player thread to stop
    if (m_thrPlayer.bIsRunning())
    {
        m_thrPlayer.ReqShutdownSync(10000);
        m_thrPlayer.eWaitForDeath(5000);
    }
}


// ---------------------------------------------------------------------------
//  TCQCWMPAudioPlayer: Private, static methods
// ---------------------------------------------------------------------------

//
//  This is called by the player thread, which calls LoadNewURL to load a
//  new file/URL. If the caller also provided a desired target output
//  device, then we are called with the device's friendly name and try to
//  find it and instantiate that filter and return it. Else we return a
//  null pointer.
//
IBaseFilter* TCQCWMPAudioPlayer::pFindDevFilter(const TString& strName)
{
    HRESULT hRes;

    // Create a device enumeration interface
    ICreateDevEnum* pCreateDevEnum = nullptr;
    hRes = CoCreateInstance
    (
        CLSID_SystemDeviceEnum
        , NULL
        , CLSCTX_INPROC_SERVER
        , IID_ICreateDevEnum
        , tCIDLib::pToVoidPP(&pCreateDevEnum)
    );
    if (FAILED(hRes))
        ThrowDSError(kMedErrs::errcWMP_CreateDevEnum, hRes, CID_LINE);

    // And use that to create an enumerator for audio devices
    IEnumMoniker* pEnumMoniker;
    hRes = pCreateDevEnum->CreateClassEnumerator
    (
        CLSID_AudioRendererCategory, &pEnumMoniker, 0
    );

    // At this point, work or fail we can release the device enumerator
    SafeCOMFree(pCreateDevEnum);

    // If the class enumerator failed, then give up
    if (FAILED(hRes))
        ThrowDSError(kMedErrs::errcWMP_CreateEnumMoniker, hRes, CID_LINE);

    // It worked, so put a janitor on it and reset it for iteration
    TCOMJanitor<IEnumMoniker> janEnumMoniker(&pEnumMoniker);
    pEnumMoniker->Reset();

    //
    //  The name we get is the basic name, which we get from doing the
    //  simpler iteration to get the list to present to the user. At this
    //  level we prefer the ones that start with "DirectShow: ", so we
    //  create a version of the name like that and look for that first. If
    //  we don't find it, then we look for one with the specific name.
    //
    const TString strPrefName = TString::strConcat(L"DirectSound: ", strName);

    tCIDLib::TCard4 c4GotCnt;
    IMoniker*       pMoniker;
    IBaseFilter*    pPrefFilter = nullptr;
    IBaseFilter*    pAltFilter = nullptr;
    while(kCIDLib::True)
    {
        hRes = pEnumMoniker->Next(1, &pMoniker, &c4GotCnt);
        if (hRes == VFW_E_ENUM_OUT_OF_SYNC)
        {
            // The list has changed, so we want to start over
            pEnumMoniker->Reset();
            continue;
        }

        // If no more, then we are done
        if (!c4GotCnt)
            break;

        // If we got an error, then we are done
        if (FAILED(hRes))
            break;

        // We got one, so put a janitor on it and get the name out
        TCOMJanitor<IMoniker> janMoniker(&pMoniker);

        IPropertyBag* pPropertyBag;
        #pragma warning(suppress : 6387) // For moniker filters the first parm MUST be null
        pMoniker->BindToStorage(0, 0, IID_IPropertyBag, tCIDLib::pToVoidPP(&pPropertyBag));
        TCOMJanitor<IPropertyBag> janBag(&pPropertyBag);
        VARIANT NameVar;
        NameVar.vt = VT_BSTR;
        hRes = pPropertyBag->Read(L"FriendlyName", &NameVar, 0);
        if (FAILED(hRes))
        {
            if (facCQCMedia().bLogFailures())
            {
                facCQCMedia().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kMedErrs::errcWMP_GetDeviceName
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , TCardinal(hRes, tCIDLib::ERadices::Hex)
                );
            }
        }
         else
        {
            //
            //  If it matches the prefered name, then it's our guy. If not,
            //  but it matches the basic name, then remember it as the
            //  fallback.
            //
            const tCIDLib::TBoolean bPrefMatch
            (
                TRawStr::bCompareStr(strPrefName.pszBuffer(), NameVar.bstrVal)
            );
            const tCIDLib::TBoolean bMatch
            (
                TRawStr::bCompareStr(strName.pszBuffer(), NameVar.bstrVal)
            );

            if (bPrefMatch || bMatch)
            {
                ::SysFreeString(NameVar.bstrVal);

                IBindCtx* pBindCtx = nullptr;
                if (::CreateBindCtx(0, &pBindCtx) != S_OK)
                    return nullptr;

                TCOMJanitor<IBindCtx> janBindCtx(&pBindCtx);

                if (pBindCtx)
                {
                    if (bPrefMatch)
                    {
                        hRes = pMoniker->BindToObject(pBindCtx, 0, IID_IBaseFilter, tCIDLib::pToVoidPP(&pPrefFilter));
                    }
                     else
                    {
                        // If there was a previous match, then do nothing
                        hRes = 0;
                        if (!pAltFilter)
                        {
                            hRes = pMoniker->BindToObject(pBindCtx, 0, IID_IBaseFilter, tCIDLib::pToVoidPP(&pAltFilter));
                        }

                        if (FAILED(hRes))
                        {
                            // If we had an alt filter, clean it up and throw
                            if (pAltFilter)
                                SafeCOMFree(pAltFilter);
                            ThrowDSError
                            (
                                kMedErrs::errcWMP_BindToDevFilter, hRes, CID_LINE
                            );
                        }
                    }

                    // We found our guy, so break out
                    if (bPrefMatch)
                        break;
                }
            }
             else
            {
                ::SysFreeString(NameVar.bstrVal);
            }
        }
    }

    // If we got the preferred filter, then clean up any alt and return it
    if (pPrefFilter)
    {
        if (pAltFilter)
            SafeCOMFree(pAltFilter);
        return pPrefFilter;
    }

    // Else return the alt filter if we got one
    return pAltFilter;
}


// ---------------------------------------------------------------------------
//  TCQCWMPAudioPlayer: Private, non-virtual methods
// ---------------------------------------------------------------------------

// A helper to keep the cleanup of all our objects in one place
tCIDLib::TVoid
TCQCWMPAudioPlayer::CleanupGraph(const tCIDLib::TBoolean bErr)
{
    // Stop current playback, if any
    HRESULT hRes;
    if (m_pMediaControl)
    {
        if (m_pMediaControl->Stop() == S_FALSE)
        {
            // We need to wait for a state transition
            OAFilterState State;
            hRes = m_pMediaControl->GetState(5000, &State);
            if (hRes == S_OK)
                m_pMediaControl->Stop();
        }
    }

    // Whether the above worked or not, kill every
    if (m_pMediaSeeking)
        SafeCOMFree(m_pMediaSeeking);

    if (m_pBasicAudio)
        SafeCOMFree(m_pBasicAudio);

    if (m_pMediaControl)
        SafeCOMFree(m_pMediaControl);

    if (m_pMediaEvent)
        SafeCOMFree(m_pMediaEvent);

    if (m_pGraphBuilder)
        SafeCOMFree(m_pGraphBuilder);

    if (bErr)
        m_eEndState = tCQCMedia::EEndStates::Failed;
    else
        m_eEndState = tCQCMedia::EEndStates::Ended;

    // Reset some status fields
    m_eState = tCQCMedia::EMediaStates::Stopped;
    m_enctCurPos = 0;
    m_enctWatchDog = 0;
    m_strCurFile.Clear();
}


//
//  This is the thread that manages the player. It does all the COM'isms and
//  whatnot, since COM is very picky about objects only being accessed by the
//  thread that created them.
//
//  NOTE: Once we get an
//
tCIDLib::EExitCodes
TCQCWMPAudioPlayer::ePlayerThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let the calling thread go now since we initialized ok
    thrThis.Sync();

    m_pBasicAudio = nullptr;
    m_pGraphBuilder = nullptr;
    m_pMediaControl = nullptr;
    m_pMediaEvent = nullptr;
    m_pMediaSeeking = nullptr;

    //
    //  We use a double loop here, so that we can recover from any exception
    //  that might occur and keep the thread running.
    //
    //  We do short, blocking checks on the command queue and on the filter
    //  graph event queue, which provides our throttling. Probably should
    //  be changed to a more event driven scheme in the future, but this
    //  is simple and the overhead will be very small, since we will just
    //  usually make two small checks every 100ms.
    //
    tCIDLib::TBoolean       bDone = kCIDLib::False;
    tCIDLib::TCard4         c4LoopCnt = 0;
    TQElem                  cptrEv;
    while (!bDone)
    {
        try
        {
            // Watch for shutdown requests
            while (!thrThis.bCheckShutdownRequest())
            {
                //
                //  Wait a bit for a command to come in, using a short time
                //  out.
                //
                if (m_colCmdQ.bGetNext(cptrEv, 100))
                    ProcessCmd(cptrEv);

                //
                //  Process any events from the filter graph, using a short
                //  timeout here also.
                //
                long EvCode;
                long Parm1;
                long Parm2;
                tCQCMedia::EMediaStates eState = tCQCMedia::EMediaStates::Count;
                while (m_pMediaEvent
                &&     (m_pMediaEvent->GetEvent(&EvCode, &Parm1, &Parm2, 100) == S_OK))
                {
                    switch(EvCode)
                    {
                        case EC_COMPLETE :
                            // Set the end state
                            if (Parm1 == S_OK)
                                m_eEndState = tCQCMedia::EEndStates::Ended;
                            else
                                m_eEndState = tCQCMedia::EEndStates::Failed;

                            // We have to stop the filter graph ourselves
                            if (m_pMediaControl)
                                m_pMediaControl->Stop();

                            m_strCurFile.Clear();
                            break;

                        case EC_BUFFERING_DATA :
                            //
                            //  We don't deal with the end of buffering
                            //  event. We update the status manually about
                            //  once a second and if no other event comes
                            //  in to change the status before then, we'll
                            //  update it them.
                            //
                            if (Parm1)
                                eState = tCQCMedia::EMediaStates::Buffering;
                            break;

                        case EC_ERRORABORT :
                        case EC_STREAM_ERROR_STOPPED :
                        case EC_SNDDEV_OUT_ERROR :
                            // Log that we are giving up on this one
                            facCQCMedia().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kMedErrs::errcWMP_SkippedBadFile
                                , m_strCurFile
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                            );

                            eState = tCQCMedia::EMediaStates::Stopped;
                            if (m_pMediaControl)
                                m_pMediaControl->Stop();

                            // Set the failed end state
                            m_eEndState = tCQCMedia::EEndStates::Failed;
                            break;

                        case EC_USERABORT :
                            // Not an error, but end of media
                            eState = tCQCMedia::EMediaStates::Stopped;
                            if (m_pMediaControl)
                                m_pMediaControl->Stop();

                            // Set the media end state
                            m_eEndState = tCQCMedia::EEndStates::Ended;
                            break;

                        case EC_ERROR_STILLPLAYING :
                        case EC_STREAM_ERROR_STILLPLAYING :
                            // eState = tCQCMedia::EEndStates::Error;
                            break;

                        case EC_OPENING_FILE :
                            eState = tCQCMedia::EMediaStates::Loading;
                            break;

                        case EC_STATE_CHANGE :
                            eState = eXlatFilterState(Parm1);
                            break;

                        default :
                            // Not one we care about
                            break;
                    };

                    // And free the parms
                    m_pMediaEvent->FreeEventParams(EvCode, Parm1, Parm2);
                }

                //
                //  If we got something that we care about and it's not what
                //  we already have, then store it. We just store one at the
                //  end, since if there was more than one, it wouldn't be
                //  there long enough for anyone to see it.
                //
                //  If it hasn't changed, then every 5th time through,
                //  update the state manually.
                //
                c4LoopCnt++;
                if ((eState != tCQCMedia::EMediaStates::Count)
                &&  (eState != m_eState))
                {
                    TLocker lockrSync(&m_mtxSync);
                    m_eState = eState;
                }
                 else if (m_pMediaControl && !(c4LoopCnt % 5))
                {
                    OAFilterState State;
                    if (m_pMediaControl->GetState(0, &State) != E_FAIL)
                    {
                        eState = eXlatFilterState(State);
                        if (eState != m_eState)
                        {
                            TLocker lockrSync(&m_mtxSync);
                            m_eState = eState;
                        }
                    }
                }

                //
                //  Every 2 times through, get the current time and store
                //  it, if we have the seeking interface. We might not if
                //  we couldn't get the desired time format set or if we
                //  are currenty idle.
                //
                if ((c4LoopCnt & 0x1) && m_pMediaSeeking)
                {
                    TLocker lockrSync(&m_mtxSync);
                    LONGLONG CurTime;
                    m_pMediaSeeking->GetCurrentPosition(&CurTime);
                    m_enctCurPos = tCIDLib::TEncodedTime(CurTime);
                }

                //
                //  If the state is playing but the current position is
                //  zero, then see how long since we got the file loading
                //  event. If it's been more than 10 seconds, then kill
                //  playback since it's probably hung.
                //
                if ((eState == tCQCMedia::EMediaStates::Playing) && !m_enctCurPos)
                {
                    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
                    if ((enctCur - m_enctWatchDog) > kCIDLib::enctTenSeconds)
                    {
                        m_enctWatchDog = enctCur;
                        if (m_pMediaControl)
                            m_pMediaControl->Stop();

                        m_eEndState = tCQCMedia::EEndStates::Failed;
                        m_eState = tCQCMedia::EMediaStates::Stopped;

                        facCQCMedia().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kMedErrs::errcWMP_StartupTimeout
                            , m_strCurFile
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                        );
                    }
                }
            }

            // We are stopping, so clean up any objects we have left
            bDone = kCIDLib::True;

            // Clean up any remaining stuff and release any commands
            ReleaseCmds(0);
            CleanupGraph(kCIDLib::False);
        }

        catch(const TError& errToCatch)
        {
            try
            {
                if (facCQCMedia().bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);

                if (facCQCMedia().bLogFailures())
                {
                    facCQCMedia().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kMedErrs::errcMP_LoopExcept
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }
                CleanupGraph(kCIDLib::True);
            }

            catch(...)
            {
            }

            //
            //  Release any queued up commands, passing back the
            //  exception we got.
            //
            ReleaseCmds(&errToCatch);
        }

        catch(...)
        {
            try
            {
                if (facCQCMedia().bLogFailures())
                {
                    facCQCMedia().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kMedErrs::errcMP_LoopExcept
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }
                CleanupGraph(kCIDLib::True);
            }

            catch(...)
            {
            }

            //
            //  Release any queued up commands, giving back an appropriate
            //  exception.
            //
            TError errToRet
            (
                facCQCMedia().strName()
                , CID_FILE
                , CID_LINE
                , kMedErrs::errcMP_LoopExcept
                , facCQCMedia().strMsg(kMedErrs::errcMP_LoopExcept)
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
            ReleaseCmds(&errToRet);
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  This is called any time we need to store a filter state, which needs to
//  be stored as our player state format.
//
tCQCMedia::EMediaStates
TCQCWMPAudioPlayer::eXlatFilterState(const tCIDLib::TSInt sStatus)
{
    tCQCMedia::EMediaStates eRet;
    switch((FILTER_STATE)sStatus)
    {
        case State_Stopped :
            eRet = tCQCMedia::EMediaStates::Stopped;
            break;

        case State_Paused :
            eRet = tCQCMedia::EMediaStates::Paused;
            break;

        case State_Running :
            eRet = tCQCMedia::EMediaStates::Playing;
            break;

        default :
            eRet = tCQCMedia::EMediaStates::Undefined;
            break;
    };
    return eRet;
}


//
//  This is called to load up a new URL. We undo any stuff from any
//  currently loaded graph, and create new stuff. Once we have the graph
//  setup, we set the last set volume on it if we are not in mute state.
//
//  If the strDevice parm is not an empty string, we'll look up the
//  audio renderer filter with that name and use it. If we cannot find
//  it, we fail.
//
tCIDLib::TVoid
TCQCWMPAudioPlayer::LoadNewURL( const   TString&    strToLoad
                                , const TString&    strDevice)
{
    // Clean up any incoming old objects
    CleanupGraph(kCIDLib::False);

    // Reset the playback time
    m_enctCurPos = 0;
    m_enctWatchDog = TTime::enctNow();

    //
    //  Assume we'll start some new media playing. If we fail and clear
    //  the graph, it'll get set again. And clear the end type again now
    //  that we are loading a new file.
    //
    m_eEndState = tCQCMedia::EEndStates::Running;
    m_eState = tCQCMedia::EMediaStates::Loading;

    try
    {
        // Create a new filter graph
        HRESULT hRes = ::CoCreateInstance
        (
            CLSID_FilterGraph
            , 0
            , CLSCTX_ALL
            , IID_IGraphBuilder
            , tCIDLib::pToVoidPP(&m_pGraphBuilder)
        );

        if (FAILED(hRes))
            ThrowDSError(kMedErrs::errcWMP_CreateFilterGraph, hRes, CID_LINE);

        //
        //  If they indicated a specific renderer, then we want to find that
        //  filter, instantiate one, and add it to the graph so that it will
        //  be used instead of the default renderer.
        //
        if (!strDevice.bIsEmpty())
        {
            IBaseFilter* pTarFilter = pFindDevFilter(strDevice);
            if (pTarFilter)
            {
                TCOMJanitor<IBaseFilter> janFilter(&pTarFilter);
                m_pGraphBuilder->AddFilter(pTarFilter, strDevice.pszBuffer());
            }
        }

        // Get the media event interface
        hRes = m_pGraphBuilder->QueryInterface(IID_IMediaEvent, tCIDLib::pToVoidPP(&m_pMediaEvent));
        if (FAILED(hRes))
            ThrowDSError(kMedErrs::errcWMP_GetEventInterface, hRes, CID_LINE);

        // Get the control interface
        hRes = m_pGraphBuilder->QueryInterface(IID_IMediaControl, tCIDLib::pToVoidPP(&m_pMediaControl));
        if (FAILED(hRes))
            ThrowDSError(kMedErrs::errcWMP_GetCtrlInterface, hRes, CID_LINE);

        // Get the basic audio interface
        hRes = m_pGraphBuilder->QueryInterface(IID_IBasicAudio, tCIDLib::pToVoidPP(&m_pBasicAudio));
        if (FAILED(hRes))
            ThrowDSError(kMedErrs::errcWMP_GetAudioInterface, hRes, CID_LINE);

        //
        //  Get the seeking interface, which we use for seeking and for getting
        //  the current playback position.
        //
        hRes = m_pGraphBuilder->QueryInterface(IID_IMediaSeeking, tCIDLib::pToVoidPP(&m_pMediaSeeking));
        if (FAILED(hRes))
            ThrowDSError(kMedErrs::errcWMP_GetSeekInterface, hRes, CID_LINE);

        //
        //  See what the position format is. If it's not the standard time
        //  stamp format, then set that format. If that fails, then we will
        //  just report invalid time info.
        //
        GUID TimeFmt;
        m_pMediaSeeking->GetTimeFormat(&TimeFmt);
        if (TimeFmt != TIME_FORMAT_MEDIA_TIME)
        {
            if (m_pMediaSeeking->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME) == S_OK)
                m_pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
        }

        // Turn off default handling for any that we can
        m_pMediaEvent->CancelDefaultHandling(EC_BUFFERING_DATA);
        m_pMediaEvent->CancelDefaultHandling(EC_ERROR_STILLPLAYING);
        m_pMediaEvent->CancelDefaultHandling(EC_ERRORABORT);
        m_pMediaEvent->CancelDefaultHandling(EC_OPENING_FILE);
        m_pMediaEvent->CancelDefaultHandling(EC_STATE_CHANGE);
        m_pMediaEvent->CancelDefaultHandling(EC_STREAM_ERROR_STILLPLAYING);
        m_pMediaEvent->CancelDefaultHandling(EC_STREAM_ERROR_STOPPED);
        m_pMediaEvent->CancelDefaultHandling(EC_USERABORT);

        //
        //  Store the current file name at this point, so it'll be there
        //  as soon as events start coming back. If we fail, it'll get
        //  cleared back up during the graph cleanup.
        //
        m_strCurFile = strToLoad;

        // Now try to get the filter graph set up
        hRes = m_pGraphBuilder->RenderFile(strToLoad.pszBuffer(), NULL);
        if (FAILED(hRes))
            ThrowDSError(kMedErrs::errcWMP_BuildGraph, hRes, CID_LINE, strToLoad);

        //
        //  Wait a little bit for it to get to playing state. Otherwise, it
        //  may undo our setting of the volume.
        //
        tCIDLib::TCard4 c4LoopCnt = 0;
        TThread* pthrCaller = TThread::pthrCaller();
        while ((m_eState != tCQCMedia::EMediaStates::Playing) && (c4LoopCnt < 20))
        {
            if (!pthrCaller->bSleep(50))
                break;
            c4LoopCnt++;
        }

        //
        //  If not muted, set the current volume, If muted, set it to zero.
        //  If muted, we have to indicate in the 2nd parm that we want to
        //  override the normal ignoring of the new volume when in mute mode.
        //
        if (m_bMuted)
            SetNewVolume(0, kCIDLib::True);
        else
            SetNewVolume(m_c4CurVolume, kCIDLib::False);

        // And finally start it playing
        if (m_pMediaControl)
        {
            hRes = m_pMediaControl->Run();
            if (FAILED(hRes))
                ThrowDSError(kMedErrs::errcMP_StartPlayback, hRes, CID_LINE, strToLoad);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        if (facCQCMedia().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        CleanupGraph(kCIDLib::True);
        throw;
    }

    catch(...)
    {
        CleanupGraph(kCIDLib::True);
        throw;
    }
}


//
//  A helper to wait for a command to complete. The incoming calls post
//  commands to the DirectShow player thread and wait for it to post them
//  as done.
//
tCIDLib::TVoid TCQCWMPAudioPlayer::PostCmdAndWait(TQElem& cptrWait)
{
    if (m_colCmdQ.bIsFull(16))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcWMP_CmdQFull
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
        );
    }

    // We have space so post the command and wait
    m_colCmdQ.objAdd(cptrWait);
    cptrWait->m_evWait.WaitFor(CQCMedia_WMPAudioPlayer::c4WaitCmd);

    // If we get an error back, then throw it
    if (cptrWait->m_perrFailure)
        throw *cptrWait->m_perrFailure;
}


//
//  This is called from the player thread, when a command comes in from
//  the outside. One way or another we have to trigger the event to let
//  the caller go once we've either completed the command or failed. We
//  return exception if it fails.
//
tCIDLib::TVoid TCQCWMPAudioPlayer::ProcessCmd(TQElem& cptrEv)
{
    HRESULT hRes;
    try
    {
        // We got one, so process it
        switch(cptrEv->m_eType)
        {
            case EEvTypes::Load :
            {
                LoadNewURL(*cptrEv->m_pstrVal1, *cptrEv->m_pstrVal2);
                break;
            }

            case EEvTypes::Mute :
            {
                if (cptrEv->m_bVal)
                {
                    m_bMuted = kCIDLib::True;
                    SetNewVolume(0, kCIDLib::True);
                }
                 else
                {
                    // Put the volume back to where it was before
                    m_bMuted = kCIDLib::False;
                    SetNewVolume(m_c4CurVolume, kCIDLib::True);
                }
                break;
            }

            case EEvTypes::Pause :
            {
                if (m_pMediaControl
                &&  (m_pMediaControl->Pause() == S_FALSE))
                {
                    OAFilterState State;
                    hRes = m_pMediaControl->GetState(5000, &State);
                    if (hRes != S_OK)
                    {
                    }
                }
                break;
            }

            case EEvTypes::Play :
            {
                if (m_pMediaControl
                &&  (m_pMediaControl->Run() == S_FALSE))
                {
                    OAFilterState State;
                    hRes = m_pMediaControl->GetState(5000, &State);
                    if (hRes != S_OK)
                    {
                    }
                }
                break;
            }

            case EEvTypes::Reset :
            {
                // Just clean up the filter graph
                CleanupGraph(kCIDLib::False);
                break;
            }

            case EEvTypes::Volume :
            {
                //
                //  Set the new volume on our filter graph if
                //  that's doable right now and if we are not
                //  in mute state.
                //
                SetNewVolume(cptrEv->m_c4Val, kCIDLib::False);
                break;
            }

            case EEvTypes::Stop :
            {
                if (m_pMediaControl)
                {
                    hRes = m_pMediaControl->Stop();
                    if (FAILED(hRes))
                    {
                    }
                }
                break;
            }

            default :
                // <TBD> Dunno what this is, should log something
                break;
        };

        // Trigger the event, and then drop the reference.
        cptrEv->m_evWait.Trigger();
        cptrEv.DropRef();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        cptrEv->m_perrFailure = new TError(errToCatch);
        cptrEv->m_evWait.Trigger();
        cptrEv.DropRef();
        throw;
    }

    catch(...)
    {
        cptrEv->m_perrFailure = new TError
        (
            facCQCMedia().strName()
            , CID_FILE
            , CID_LINE
            , kMedErrs::errcMP_CmdError
            , facCQCMedia().strMsg
              (
                kMedErrs::errcMP_CmdError
                , TCardinal(tCIDLib::c4EnumOrd(cptrEv->m_eType))
              )
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
        cptrEv->m_evWait.Trigger();
        cptrEv.DropRef();
        throw;
    }
}


//
//  Called to release any pending commands from the queue, giving back an
//  exception if that is needed.
//
tCIDLib::TVoid TCQCWMPAudioPlayer::ReleaseCmds(const TError* const perrToRet)
{
    TQElem cptrEv;
    while (m_colCmdQ.bGetNext(cptrEv, 1))
    {
        if (perrToRet)
            cptrEv->m_perrFailure = new TError(*perrToRet);
        cptrEv->m_evWait.Trigger();
        cptrEv.DropRef();
    }
}


//
//  This is a helper that will do the actual work to set a volume percent on
//  the player, if we have a basic audio interface and we are not currently
//  muted (or the override parm is true.)
//
tCIDLib::TVoid
TCQCWMPAudioPlayer::SetNewVolume(const  tCIDLib::TCard4     c4Percent
                                , const tCIDLib::TBoolean   bOverride)
{
    // If it's not legal, do nothing
    if (c4Percent > 100)
        return;

    if (m_pBasicAudio && (!m_bMuted || bOverride))
    {
        tCIDLib::TInt4 i4NewVal;

        // Special case zero and 100, since we can do them easily
        if (!c4Percent)
        {
            i4NewVal = -10000;
        }
         else if (c4Percent == 100)
        {
            i4NewVal = 0;
        }
         else
        {
            //
            //  Get the log10 of the percent. This should get us a value
            //  from 0.0 to 2.0.
            //
            tCIDLib::TFloat8 f8LogVol = c4Percent;
            f8LogVol = TMathLib::f8Log10(f8LogVol);

            //
            //  Multiply this times 20 to a value in the 40dB range that
            //  we support, and invert since we are subtracting down
            //  from 0.
            //
            i4NewVal = 40 - tCIDLib::TInt4(f8LogVol * 20);

            // And convert to the Windows format
            i4NewVal *= -100;

            // Clip just in case
            if (i4NewVal > 0)
                i4NewVal = 0;
            else if (i4NewVal < -10000)
                i4NewVal = -10000;
        }

        HRESULT hRes = m_pBasicAudio->put_Volume(i4NewVal);
        if (FAILED(hRes))
            ThrowDSError(kMedErrs::errcMP_SetVolume, hRes, CID_LINE);
    }
}

