//
// FILE NAME: CQCDriver_DriverBase.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/22/2001
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
//  This is the implementation file for the base classes for server side
//  drivers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCDriver_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCServerBase,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCDriver_DriverBase
{
    namespace
    {
        // The max sies on the cmd queues
        constexpr tCIDLib::TCard4 c4MaxQSz = 64;
    }
}


// ---------------------------------------------------------------------------
//  A pool for driver commands, which are used and dropped really quickly, so we
//  don't want to pay the price.
// ---------------------------------------------------------------------------
class TSrvDrvCmdPool : public TSimplePool<TCQCServerBase::TDrvCmd>
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TSrvDrvCmdPool() :

            TSimplePool
            (
                1024, L"Driver Cmd Pool", tCIDLib::EMTStates::Safe
            )
        {
        }

        TSrvDrvCmdPool(const TSrvDrvCmdPool&) = delete;

        ~TSrvDrvCmdPool() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSrvDrvCmdPool& operator=(const TSrvDrvCmdPool&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ElemSize(const TCQCServerBase::TDrvCmd& dcmdSrc) override
        {
            return dcmdSrc.m_mbufVal.c4Size();
        }

        TCQCServerBase::TDrvCmd* pelemMakeNew(const tCIDLib::TCard4 c4Size) override
        {
            return new TCQCServerBase::TDrvCmd(c4Size);
        }

        tCIDLib::TVoid
        PrepElement(TCQCServerBase::TDrvCmd& dcmdTar, const tCIDLib::TCard4 c4Size) override
        {
            dcmdTar.Reset(c4Size);
        }
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCServerBase::TDrvCmd
//  PREFIX: dcmd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCServerBase::TDrvCmd: Public, static methods
// ---------------------------------------------------------------------------

//
//  See if the passed command is one of the special ones. These get queued
//  to the special command queue, instead of the regular one.
//
tCIDLib::TBoolean TCQCServerBase::TDrvCmd::bIsASpecialCmd(const EDrvCmds eCmd)
{
    if ((eCmd == TCQCServerBase::EDrvCmds::Pause)
    ||  (eCmd == TCQCServerBase::EDrvCmds::Reconfig)
    ||  (eCmd == TCQCServerBase::EDrvCmds::Resume))
    {
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  We don't really much need the capacity sorting and such. But, we have to
//  provide it, so we use the current allocation size of the memory buffer
//  member.
//
//  This does let us at least try not to unduely allocate buffers.
//
tCIDLib::ESortComps
TCQCServerBase::TDrvCmd::eComp( const   TDrvCmd&        dcmd1
                                , const TDrvCmd&        dcmd2
                                , const tCIDLib::TCard4 )
{
    if (dcmd1.m_mbufVal.c4Size() < dcmd2.m_mbufVal.c4Size())
        return tCIDLib::ESortComps::FirstLess;
    else if (dcmd1.m_mbufVal.c4Size() > dcmd2.m_mbufVal.c4Size())
        return tCIDLib::ESortComps::FirstGreater;

    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TCQCServerBase::TDrvCmd::eKeyComp(  const   tCIDLib::TCard4 c4Val
                                    , const TDrvCmd&        dcmdComp
                                    , const tCIDLib::TCard4 )
{
    if (c4Val < dcmdComp.m_mbufVal.c4Size())
        return tCIDLib::ESortComps::FirstLess;
    else if (c4Val > dcmdComp.m_mbufVal.c4Size())
        return tCIDLib::ESortComps::FirstGreater;

    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//  TCQCServerBase::TDrvCmd: Constructor and Destructor
// ---------------------------------------------------------------------------

// We DO have to set m_pcqcdcNew, because Reset() will try to delete and null it
TCQCServerBase::TDrvCmd::TDrvCmd(const tCIDLib::TCard4 c4BufSz) :

    m_evWait(tCIDLib::EEventStates::Reset)
    , m_mbufVal(c4BufSz ? c4BufSz : 128UL, kCIDLib::c4Sz_2M)
    , m_pcqcdcNew(nullptr)
{
    // Pass the initial bufer size so it doesn't change the buffer
    Reset(m_mbufVal.c4Size());
}

TCQCServerBase::TDrvCmd::~TDrvCmd()
{
    // If the driver config object got created, clean it up
    try
    {
        delete m_pcqcdcNew;
        m_pcqcdcNew = nullptr;
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TCQCServerBase::TDrvCmd: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called to prep us for being set up for the next round. This will be called as
//  we are gotten from the pool. The min size may be zero, since many command don't
//  use the value buffer.
//
tCIDLib::TVoid TCQCServerBase::TDrvCmd::Reset(const tCIDLib::TCard4 c4MinBufSz)
{
    m_eCmd = TCQCServerBase::EDrvCmds::None;
    m_evWait.Reset();
    m_c4RefCnt = 0;

    m_bError = kCIDLib::False;
    m_strMoniker.Clear();

    m_bVal = kCIDLib::False;
    m_c4Count = 0;
    m_c4Id1 = 0;
    m_c4Id2 = 0;
    m_c4Val = 0;
    m_colStrList.RemoveAll();
    m_enctVal = 0;
    m_f8Val = 0;
    m_i4Val = 0;
    m_strName.Clear();
    m_strParams.Clear();
    m_strType.Clear();
    m_strVal.Clear();

    if (m_mbufVal.c4Size() < c4MinBufSz)
        m_mbufVal.Reallocate(c4MinBufSz + 32, kCIDLib::False);

    try
    {
        delete m_pcqcdcNew;
        m_pcqcdcNew = nullptr;
    }

    catch(...)
    {
    }
}


//
//  Called by the driver before it triggers the event, if the command
//  has failed. If a client thread is waiting for it to complete. It
//  will see this and throw back instead of returning.
//
tCIDLib::TVoid TCQCServerBase::TDrvCmd::SetError(const TError& errRet)
{
    m_bError = kCIDLib::True;
    m_errReturn = errRet;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCServerBase
//  PREFIX: sdrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCServerBase: Public, static methods
// ---------------------------------------------------------------------------

//
//  A helper for CQCServer, the CML IDE, etc... to call after they queue up a
//  command. To wait for it to complete and return whether it got an exception
//  or not. If so, to give back the exception.
//
//  If the caller doesn't need to get any other info from the command object,
//  they can just ask us to release it so they don't have to.
//
//  If the command fails, since we are returning the only information that
//  is of use, the exception info, we always release it in that case to save
//  the caller extra work.
//
//  We may get a null pointer because if the caller says he's not going to
//  wait, or ends up not waiting because he says he doesn't care but the
//  driver policy is not to wait, then the cmd object is not returned to the
//  caller so that he can't do something stupid like access it after it's
//  already been put back to the pool by the driver. So watch for that and
//  throw if so.
//
//  There are not really any locking concerns here. The only thing we access
//  is the command, and potentially the pool to release the command. At this
//  point the command is in the queue, and another client thread could update
//  it if they override an existing command, but it won't change anything
//  we are looking at. And we won't continue until the command event is posted.
//
//  If we release the command, the release is thread safe, so we don't have
//  to worry about that. If we release and are the last waiter, then it will
//  be given back to the pool.
//
tCIDLib::TBoolean
TCQCServerBase::bWaitCmd(       TDrvCmd* const      pdcmdWaitFor
                        ,       TError&             errToFill
                        , const tCIDLib::TBoolean   bRelease)
{
    // Shouldn't ever get a null pointer, see method comments above
    if (!pdcmdWaitFor)
    {
        errToFill = TError
        (
            facCQCKit().strName()
            , CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_NoDrvCmdToWait
            , facCQCKit().strMsg(kKitErrs::errcDrv_NoDrvCmdToWait)
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
        );
        return kCIDLib::False;
    }

    //
    //  Wait for the driver to trigger completion of the command. Just in
    //  case the event wait should fail, catch the error and return that.
    //
    try
    {
        pdcmdWaitFor->m_evWait.WaitFor();
    }

    catch(TError& errToCatch)
    {
        // Set the error on the object
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        pdcmdWaitFor->SetError(errToCatch);
    }

    catch(...)
    {
        TError errRet
        (
            facCQCKit().strName()
            , CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_CmdWaitErr
            , facCQCKit().strMsg
              (
                kKitErrs::errcDrv_CmdWaitErr, pdcmdWaitFor->m_strMoniker
              )
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
        );
        pdcmdWaitFor->SetError(errRet);
    }

    //
    //  If it failed, give back the error. Get the return value out now
    //  as well, since we may release the object below.
    //
    const tCIDLib::TBoolean bRet = !pdcmdWaitFor->m_bError;
    if (pdcmdWaitFor->m_bError)
        errToFill = pdcmdWaitFor->m_errReturn;

    //
    //  If they said we can release it or it failed, then release. Indicate
    //  that this is not a release from the driver itself having completed
    //  processing, but from a client thread waiting for the results.
    //
    if (bRelease || !bRet)
        ReleaseCmd(pdcmdWaitFor, kCIDLib::False);

    return bRet;
}


//
//  A helper to reserve a command item from the pool. We don't do it directly,
//  so that we can check for special conditions and throw more helpful
//  exceptions. The driver calling passes his moniker so we can put it into
//  the cmd object.
//
//  We use a local counter to periodically call our facility class to update
//  a stats cache item that tracks how many pool items we have currently
//  used. This is an important thing to track. We don't want to update every
//  time since that would be abusive. Callers have to have locked so it's
//  save to use this type of local static counter.
//
//  There are not locking issues here. The pool is the only thing accessed and
//  it is thread safe. Until the command gets into the queue only the calling
//  thread can see it.
//
TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdReserve(   const   tCIDLib::TCard4 c4Size
                                , const TString&        strMoniker)
{
    CIDAssert(m_psplCmds != nullptr, L"The cmd pool has not been allocated");

    static tCIDLib::TCard4 c4UpdateCnt = 0;

    //
    //  Make sure we have an element to give back. It's always possible that
    //  there was only one left and another thread beats us to the reserve
    //  call below, but this will catch 99% of them easiy. In actual fact it
    //  would require a seriously pathological scenario to make the pool get
    //  full.
    //
    if (!m_psplCmds->c4ElemsAvail())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_DrvBusy
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
            , strMoniker
        );
    }

    // Looks ok, so grab one
    TCQCServerBase::TDrvCmd* pdcmdRet = m_psplCmds->pobjReserveElem(c4Size);
    pdcmdRet->m_strMoniker = strMoniker;

    //
    //  Update the stats counter. This is only for actual incoming commands. That
    //  doesn't include queries for field data or driver status and such, which are
    //  handled without getting the driver involved or by way of direct calls into
    //  the driver. So these don't really happen at a high rate, or only do if
    //  something pathological is going on.
    //
    {
        tCIDLib::TCard4 c4Used, c4Free;
        m_psplCmds->QueryListSizes(c4Used, c4Free);
        facCQCDriver().UpdatePoolItemsStat(c4Used, c4Free);
    }
    return pdcmdRet;
}


//
//  When the driver thread finishes a cmd, it will call this and indicate
//  it's from the driver itself. When CQCServer (or an IDE managing a driver)
//  wakes up from waiting on a command, it will also call this, and indicate
//  it's not from the driver.
//
//  The ref count is reduced since it's being released. If the count goes
//  zero, then the object is released back to the pool.
//
//  If the call is from the driver, and the count hasn't gone zero, i.e.
//  clients are waiting on the command, then we trigger the wait event.
//  Clients should subsequently wake up and call here as well until the
//  count goes zero and we release it.
//
//  We use the lock of the pool itself to sync the reduction of the ref
//  count. See the class header file comments, but basically the ref count
//  can only be increased while the cmd is still in the queue, and that is
//  protected by the locking of the target queue. Once it's taken out of
//  the queue, then the only thing happening is that clients can be waiting
//  on the command completion event until the driver completes the command.
//
//  !! Currently we don't let clients time out of a command before it is
//  marked complete. So there's no way for a client to reduce the count
//  before it's completed.
//
//  Once the driver has it, it can't be increased anymore. The clients are
//  just either blocking waiting for it to complete, or it's a no-wait in
//  which no one is waiting on it. Either the client or the driver dropping
//  the command is done via this call, so the ref count manipulation is
//  protected.
//
tCIDLib::TVoid
TCQCServerBase::ReleaseCmd(         TDrvCmd* const      pdcmdRel
                            , const tCIDLib::TBoolean   bFromDrv)
{
    CIDAssert(m_psplCmds != nullptr, L"The cmd pool has not been allocated");

    //
    //  We use the pool's mutex to sync this since we have to update the
    //  commands ref count and we may need to release the command as
    //  well.
    //
    TLocker mtxSync(m_psplCmds);

    // Decrement the ref count
    if (pdcmdRel->m_c4RefCnt)
        pdcmdRel->m_c4RefCnt--;

    //
    //  If not zero yet, then there is a client waiting, so trigger it and
    //  don't give it back yet. The client thread will do that. If it's
    //  gone zero, then release it back to the pool because there's no
    //  client waiting for it.
    //
    if (pdcmdRel->m_c4RefCnt)
    {
        // Still alive. If this is the driver completing, then trigger
        if (bFromDrv)
            pdcmdRel->m_evWait.Trigger();
    }
     else
    {
        //
        //  Just in case, if the memory buffer got allocated pretty large,
        //  reallocate it, on the assumption that we don't need to keep
        //  it around and such a buffer will be seldom used.
        //
        if (pdcmdRel->m_mbufVal.c4Size() > 1024)
            pdcmdRel->m_mbufVal.Reallocate(128, kCIDLib::False);

        m_psplCmds->ReleaseElem(pdcmdRel);
    }
}




// ---------------------------------------------------------------------------
//  TCQCServerBase: Destructor
// ---------------------------------------------------------------------------
TCQCServerBase::~TCQCServerBase()
{
    //
    //  If we created our own thread, then delete it now. It will have
    //  already long since been stopped.
    //
    if (!m_bOnExtThread)
    {
        try
        {
            delete m_pthrDevPoll;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
        m_pthrDevPoll = nullptr;
    }

    //
    //  Clean up the field storage list. The name list owns the objects, so clear
    //  the id list first.
    //
    m_colFldIdList.RemoveAll();
    m_colFldNameList.RemoveAll();

    //
    //  There shouldn't be any commands in the command queue, but release
    //  them all just in case. We can't take any chances of eating up
    //  these pool objects.
    //
    try
    {
        TError errRet
        (
            facCQCKit().strName()
            , CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_ShuttingDown
            , facCQCKit().strMsg(kKitErrs::errcDrv_ShuttingDown, strMoniker())
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Shutdown
        );
        ReleaseQCmds(errRet);
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TCQCServerBase: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  All of the backdoor commands are optional, so we provide empty impls
//  for them unless a derived driver class overrides.
//
tCIDLib::TBoolean
TCQCServerBase::bQueryData( const   TString&
                            , const TString&
                            ,       tCIDLib::TCard4& c4OutBytes
                            ,       THeapBuf&)
{
    c4OutBytes = 0;
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCServerBase::bQueryData2(const   TString&
                            ,       tCIDLib::TCard4& c4IOBytes
                            ,       THeapBuf&)
{
    c4IOBytes = 0;
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCServerBase::bQueryTextVal(const TString&, const TString&, TString&)
{
    return kCIDLib::False;
}


// These are optional, so we have a do nothing default
tCIDLib::TBoolean TCQCServerBase::bQueryVal(const TString&)
{
    return kCIDLib::False;
}

tCIDLib::TBoolean
TCQCServerBase::bQueryVal2(const TString&, const TString&)
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCServerBase::bSendData(  const   TString&
                            ,       TString&
                            ,       tCIDLib::TCard4& c4Size
                            ,       THeapBuf&)
{
    //
    //  Set the return size zero just in case, so that an unhandled call
    //  doesn't try to return some mondo sized amount data because of a
    //  random value in the size parm.
    //
    c4Size = 0;
    return kCIDLib::False;
}


tCIDLib::TCard4 TCQCServerBase::c4QueryCfg(THeapBuf&)
{
    return 0;
}


tCIDLib::TCard4 TCQCServerBase::c4QueryVal(const TString&)
{
    return 0;
}

tCIDLib::TCard4
TCQCServerBase::c4QueryVal2(const TString&, const TString&)
{
    return 0;
}


tCIDLib::TCard4
TCQCServerBase::c4SendCmd(const TString&, const TString&)
{
    return 0;
}


tCIDLib::TInt4 TCQCServerBase::i4QueryVal(const TString&)
{
    return 0;
}

tCIDLib::TInt4
TCQCServerBase::i4QueryVal2(const TString&, const TString&)
{
    return 0;
}


tCIDLib::TVoid
TCQCServerBase::SetConfig(const tCIDLib::TCard4, const THeapBuf&)
{
}


// ---------------------------------------------------------------------------
//  TCQCServerBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Returns whether the passed value is the same as our field list id. No need to lock
//  since it's just a fundamental type.
//
tCIDLib::TBoolean
TCQCServerBase::bCheckFldListChanged(const tCIDLib::TCard4 c4ToCheck) const
{
    return (c4ToCheck != m_c4FieldListId);
}


// Delete this driver's configuration string
tCIDLib::TBoolean TCQCServerBase::bDeleteCfgData()
{
    //
    //  Build the path for this driver. They have a fixed format with just the moniker
    //  being used as the differentiator.
    //
    TString strKey(kCQCKit::pszOSKey_CQCSrvDriverCfgs);
    strKey.Append(strMoniker());

    tCIDLib::TBoolean bDeleted = kCIDLib::False;
    try
    {
        // Get a local config server proxy
        TCfgServerClient cfgsGet(4000);
        bDeleted = cfgsGet.bDeleteObjectIfExists(strKey);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return bDeleted;
}


tCIDLib::TBoolean TCQCServerBase::bFieldInErr(const tCIDLib::TCard4 c4Id)
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    return pcfsFind(c4Id)->bInError();
}


// Check if a field exists by name
tCIDLib::TBoolean TCQCServerBase::bFieldExists(const TString& strToFind) const
{
    TLocker lockrSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strToFind, kCIDLib::False);
    return (pflddTar != nullptr);
}


//
//  Get a message terminated by a one or two char sequence. It can be gotten
//  from a serial port or socket, which is passed as a generic TObject, and we
//  use runtime typing to figure out what it is.
//
//  We have one version that takes a number of millis to wait, and another
//  that takes an end time. We use the millis to calc the end time and just
//  calls the other version.
//
//  These make mimimal access to driver members, so sync isn't an issue. Just
//  in case we grab the couple values it accesses up front in a lock.
//
tCIDLib::TBoolean
TCQCServerBase::bGetASCIITermMsg(       TObject&        objSrc
                                , const tCIDLib::TCard4 c4WaitFor
                                , const tCIDLib::TCard1 c1Term1
                                , const tCIDLib::TCard1 c1Term2
                                ,       TString&        strToFill)
{
    return bGetASCIITermMsg2
    (
        objSrc, TTime::enctNowPlusMSs(c4WaitFor), c1Term1, c1Term2, strToFill
    );
}

tCIDLib::TBoolean TCQCServerBase
::bGetASCIITermMsg2(        TObject&                objSrc
                    , const tCIDLib::TEncodedTime   enctEndTime
                    , const tCIDLib::TCard1         c1Term1
                    , const tCIDLib::TCard1         c1Term2
                    ,       TString&                strToFill)
{
    // We do a simple state machine here
    enum class EStates
    {
        WaitTerm1
        , WaitTerm2
        , Complete
    };

    // Cast the source object to the correct type
    TStreamSocket* psockSrc = nullptr;
    TCommPortBase* pcommSrc = nullptr;
    if (objSrc.bIsDescendantOf(TCommPortBase::clsThis()))
        pcommSrc = static_cast<TCommPortBase*>(&objSrc);
    else if (objSrc.bIsDescendantOf(TStreamSocket::clsThis()))
        psockSrc = static_cast<TStreamSocket*>(&objSrc);
    else
        return kCIDLib::False;

    // Clear the string for the new output
    strToFill.Clear();

    // Set up the end time and current time stamps
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctEndTime;

    // Make sure the end isn't before now
    if (enctCur >= enctEnd)
        return kCIDLib::False;

    tCIDLib::TBoolean bExtended = kCIDLib::False;
    tCIDLib::TCard1   c1Cur = 0;

    //
    //  Lock and grab the couple members we access during this. We don't
    //  have to worry about them changing while we are in here so this is
    //  fine.
    //
    tCQCKit::EVerboseLvls eVerboseLevel = tCQCKit::EVerboseLvls::Off;
    tCIDLib::TEncodedTime enctExtend = 0;
    {
        TLocker lockrSync(&m_mtxSync);
        enctExtend = m_enctExtend;
        eVerboseLevel = m_eVerboseLevel;
    }

    // Loop until we either get a good message or we time out
    EStates eState = EStates::WaitTerm1;
    while (eState != EStates::Complete)
    {
        // Wait for up to the remaining time for a new character
        tCIDLib::TBoolean bGotOne = kCIDLib::False;
        const tCIDLib::TEncodedTime enctWait = enctEnd - enctCur;
        if (psockSrc)
            bGotOne = psockSrc->c4ReceiveRawTO(&c1Cur, enctWait, 1, tCIDLib::EAllData::OkIfNotAll) == 1;
        else
            bGotOne = pcommSrc->c4ReadRawBuf(&c1Cur, 1, enctWait) == 1;

        //
        //  If we got something, then update the state machine and store the
        //  data.
        //
        if (bGotOne)
        {
            if (eState == EStates::WaitTerm1)
            {
                if (c1Cur == c1Term1)
                {
                    //
                    //  If no term 2, then we are done. Else go to term2
                    //  state.
                    //
                    if (!c1Term2)
                        eState = EStates::Complete;
                    else
                        eState = EStates::WaitTerm2;
                }
                 else
                {
                    //
                    //  Nothing special, just store it. Do a cheap transcode
                    //  since this method is only for ASCII data.
                    //
                    strToFill.Append(tCIDLib::TCh(c1Cur));
                }
            }
             else if (eState == EStates::WaitTerm2)
            {
                if (c1Cur == c1Term2)
                {
                    // It's term2, so we are done
                    eState = EStates::Complete;
                }
                 else
                {
                    // We owe the previous term1 back so store it
                    strToFill.Append(tCIDLib::TCh(c1Term1));

                    //
                    //  If this is a term1, then stay in term2 mode, else
                    //  go back to ground state and store this one.
                    //
                    if (c1Cur != c1Term1)
                    {
                        strToFill.Append(tCIDLib::TCh(c1Cur));
                        eState = EStates::WaitTerm1;
                    }
                }
            }
        }

        // If we got our message, don't bother updating time, just break out
        if (eState == EStates::Complete)
            break;

        // If we are past time, then we may have to give up
        enctCur = TTime::enctNow();
        if (enctCur >= enctEnd)
        {
            //
            //  If we've got some input and we've not already done so, grant
            //  an extension.
            //
            if (!strToFill.bIsEmpty() && !bExtended)
            {
                enctEnd = enctCur + enctExtend;
                bExtended = kCIDLib::True;
            }
             else
            {
                // Oh well, out of luck
                strToFill.Clear();
                break;
            }
        }

        // Watch for shutdown requests
        if (m_pthrDevPoll->bCheckShutdownRequest())
            return kCIDLib::False;
    }

    // If we are in high verbosity, log failures to get terms
    if (eVerboseLevel >= tCQCKit::EVerboseLvls::High)
    {
        // If we got some input, but ended on a wait for term state
        if (!strToFill.bIsEmpty())
        {
            if (facCQCKit().bLogWarnings())
            {
                if (eState == EStates::WaitTerm1)
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Failed to get first msg term char"
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }
                 else if (eState == EStates::WaitTerm2)
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Failed to get second msg term char"
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }
            }
        }
    }
    return (eState == EStates::Complete);
}


//
//  Get an ASCII message that is delimited by start/stop bytes. It can be
//  gotten from a serial port or socket, which is passed as a generic
//  TObject, and we use runtime typing to figure out what it is.
//
//  We have one version that takes a number of millis to wait, and another
//  that takes an end time. We use the millis to calc the end time and just
//  calls the other version.
//
//  These make mimimal access to driver members, so sync isn't an issue. Just
//  in case we grab the couple values it accesses up front in a lock.
//
tCIDLib::TBoolean TCQCServerBase
::bGetASCIIStartStopMsg(        TObject&        objSrc
                        , const tCIDLib::TCard4 c4WaitFor
                        , const tCIDLib::TCard1 c1Term1
                        , const tCIDLib::TCard1 c1Term2
                        ,       TString&        strToFill)
{
    return bGetASCIIStartStopMsg2
    (
        objSrc, TTime::enctNowPlusMSs(c4WaitFor), c1Term1, c1Term2, strToFill
    );
}

tCIDLib::TBoolean TCQCServerBase
::bGetASCIIStartStopMsg2(       TObject&                objSrc
                        , const tCIDLib::TEncodedTime   enctEndTime
                        , const tCIDLib::TCard1         c1StartByte
                        , const tCIDLib::TCard1         c1EndByte
                        ,       TString&                strToFill)
{
    // Clear out the string so that if we return early, it'll be empty
    strToFill.Clear();

    // Cast the source object to the correct type
    TStreamSocket* psockSrc = nullptr;
    TCommPortBase* pcommSrc = nullptr;
    if (objSrc.bIsDescendantOf(TCommPortBase::clsThis()))
        pcommSrc = static_cast<TCommPortBase*>(&objSrc);
    else if (objSrc.bIsDescendantOf(TStreamSocket::clsThis()))
        psockSrc = static_cast<TStreamSocket*>(&objSrc);
    else
        return kCIDLib::False;

    // Set up the end time and current time stamps
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctEndTime;

    // Make sure the end isn't before now
    if (enctCur >= enctEnd)
        return kCIDLib::False;

    // The states for our little state machine
    enum class EStates
    {
        WaitStart
        , WaitEnd
        , Done
    };

    //
    //  Lock and grab the couple members we access during this. We don't
    //  have to worry about them changing while we are in here so this is
    //  fine.
    //
    tCQCKit::EVerboseLvls eVerboseLevel = tCQCKit::EVerboseLvls::Off;
    tCIDLib::TEncodedTime enctExtend = 0;
    {
        TLocker lockrSync(&m_mtxSync);
        enctExtend = m_enctExtend;
        eVerboseLevel = m_eVerboseLevel;
    }

    tCIDLib::TBoolean   bExtended = kCIDLib::False;
    tCIDLib::TBoolean   bGotOne = kCIDLib::False;
    tCIDLib::TCard1     c1Cur = 0;
    EStates             eState = EStates::WaitStart;
    while (eState < EStates::Done)
    {
        const tCIDLib::TEncodedTime enctWait = enctEnd - enctCur;
        if (psockSrc)
            bGotOne = psockSrc->c4ReceiveRawTO(&c1Cur, enctWait, 1, tCIDLib::EAllData::OkIfNotAll) == 1;
        else
            bGotOne = pcommSrc->c4ReadRawBuf(&c1Cur, 1, enctWait) == 1;

        if (bGotOne)
        {
            switch(eState)
            {
                case EStates::WaitStart :
                {
                    //
                    //  If the start byte, store it and move to wait end mode,
                    //  and start accumulating chars.
                    //
                    if (c1Cur == c1StartByte)
                    {
                        strToFill.Clear();
                        strToFill.Append(tCIDLib::TCh(c1StartByte));
                        eState = EStates::WaitEnd;
                    }
                    break;
                }

                case EStates::WaitEnd :
                {
                    // Put the byte into the string
                    strToFill.Append(tCIDLib::TCh(c1Cur));

                    // If the end byte, then we are done
                    if (c1Cur == c1EndByte)
                        eState = EStates::Done;
                    break;
                }
            };

            if (eState == EStates::Done)
                break;
        }

        // If we are past time, then we may have to give up
        enctCur = TTime::enctNow();
        if (enctCur >= enctEnd)
        {
            //
            //  If we've got some input and we've not already done so, grant
            //  an extension.
            //
            if (!strToFill.bIsEmpty() && !bExtended)
            {
                enctEnd = enctCur + enctExtend;
                bExtended = kCIDLib::True;
            }
             else
            {
                // Oh well, out of luck
                strToFill.Clear();
                break;
            }
        }

        // Watch for shutdown requests
        if (m_pthrDevPoll->bCheckShutdownRequest())
            return kCIDLib::False;
    }

    // If we are in high verbosity, log failures to get terms
    if (eVerboseLevel >= tCQCKit::EVerboseLvls::High)
    {
        // If we got partial input, log that
        if (!strToFill.bIsEmpty() && (eState < EStates::Done))
        {
            if (facCQCKit().bLogWarnings())
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Failed to get stop char"
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }
    return (eState == EStates::Done);
}



//
//  Checks to see if a field has a queued up timed write, and if so what the
//  end time is.
//
tCIDLib::TBoolean
TCQCServerBase::bHasQueuedTimedWrite(const  TString&            strField
                                    ,       tCIDLib::TCard8&    c8EndTime)
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    const tCIDLib::TCard4 c4Count = m_colTimedChanges.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TTimedFldChange& tfcCur = m_colTimedChanges[c4Index];
        if (tfcCur.m_strField == strField)
        {
            c8EndTime = tfcCur.m_enctWaitTill;
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  Checks to see if we implement a specific device class. We just look for that
//  class in our driver config.
//
tCIDLib::TBoolean
TCQCServerBase::bImplementsClass(const  tCQCKit::EDevClasses    eClass
                                , const tCIDLib::TBoolean       bThrowIfNotV2) const
{
    TLocker mtxSync(&m_mtxSync);

    // If not a V2 driver, either throw or return false.
    if (m_cqcdcThis.c4ArchVersion() == 1)
    {
        if (bThrowIfNotV2)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcDrv_NotV2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotSupported
                , strMoniker()
            );
        }
        return kCIDLib::False;
    }

    // Return whether we support the indicated class
    return m_cqcdcThis.bImplementsClass(eClass);
}



//
//  Return our async flag. CQCServer uses this to decide whether he should
//  wait for replies to queued up commands or not (in conjunction with
//  explicit wishes on the part of the client sometimes.) No need to
//  lock to just return a simple value.
//
tCIDLib::TBoolean TCQCServerBase::bIsAsyncMode() const
{
    return m_bAsyncMode;
}


//
//  Returns whether the passed value could possibly be a valid field id
//  for this driver, meaning it's not beyond the max index in our field
//  id list.
//
tCIDLib::TBoolean TCQCServerBase::bIsValidFldId(const tCIDLib::TCard4 c4ToCheck) const
{
    TLocker lockrSync(&m_mtxSync);
    return (c4ToCheck < m_colFldIdList.c4ElemCount());
}


// Get this driver's configuration object, if available
tCIDLib::TBoolean TCQCServerBase::bQueryCfgData(MStreamable& mstrmbToFill)
{
    //
    //  Build the path for this driver. They have a fixed format with
    //  just the moniker being used as the differentiator.
    //
    TString strKey(kCQCKit::pszOSKey_CQCSrvDriverCfgs);
    strKey.Append(strMoniker());

    tCIDLib::TBoolean bGotIt = kCIDLib::False;
    try
    {
        // Get a local config server proxy
        TCfgServerClient cfgsGet(4000);

        tCIDLib::TCard4 c4Ver = 0;
        tCIDLib::ELoadRes eRes = cfgsGet.eReadObject(strKey, mstrmbToFill, c4Ver);
        bGotIt = (eRes != tCIDLib::ELoadRes::NotFound);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return bGotIt;
}


//
//  This is mostly to support the C++ driver test harness, though some other things
//  may use it. The caller passes us a list of serial numbers, one for each field.
//  We check those serial numbers, which are in the order of the fields in our by
//  id list, and update fcolChanged, fcolErrors, and colValues to indicate whether
//  they have changed or not. A change into or out of error state is a change, or
//  a change in value is a change.
//
//  *NOTE that we consider both error state or not having ever gotten a value written
//  to the field as an error. The latter is a bit of a temporary thing.
//
//  The return value indicates whether there are any changes at all or not. so that
//  the caller doesn't have to look through the list again.
//
//  If the return is true, bResync may be set. If so, then the caller must query
//  the fields again and start over.
//
tCIDLib::TBoolean
TCQCServerBase::bQueryChangedFlds(  const   tCIDLib::TCard4         c4FldListId
                                    ,       tCIDLib::TCardArray&    fcolSerialNums
                                    ,       tCIDLib::TBoolArray&    fcolChanged
                                    ,       tCIDLib::TBoolArray&    fcolErrors
                                    ,       tCIDLib::TStrList&      colValues
                                    ,       tCIDLib::TBoolean&      bResync)
{
    TLocker lockrSync(&m_mtxSync);

    // If offline, then just return false
    if (eState() != tCQCKit::EDrvStates::Connected)
        return kCIDLib::False;

    // If out of sync, then return true but with the resync flag set
    bResync = c4FldListId != m_c4FieldListId;
    if (bResync)
        return kCIDLib::True;

    // Make sure the incoming serial numbers list is the same size as our field list
    const tCIDLib::TCard4 c4Count = m_colFldIdList.c4ElemCount();
    if (c4Count != fcolSerialNums.c4ElemCount())
    {
        facCQCDriver().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCDrvErrs::errcComm_BadSerNumCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
    }

    // Make sure the outgoing lists are the right size
    if ((fcolChanged.c4ElemCount() != c4Count)
    ||  (fcolErrors.c4ElemCount() != c4Count)
    ||  (colValues.c4ElemCount() != c4Count))
    {
        facCQCDriver().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCDrvErrs::errcComm_BadOutListSize
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
    }

    // Check each field
    tCIDLib::TBoolean bAnyChanges = kCIDLib::False;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldStore* pcfsCur = m_colFldIdList[c4Index];

        if (pcfsCur->bInError() || !pcfsCur->fvThis().bGotFirstVal())
        {
            if (fcolErrors[c4Index])
            {
                // We were already in error state, so not a change
                fcolChanged[c4Index] = kCIDLib::False;
            }
             else
            {
                // We are going into error state, so a change
                fcolErrors[c4Index] = kCIDLib::True;
                fcolChanged[c4Index] = kCIDLib::True;

                bAnyChanges = kCIDLib::True;
            }
        }
         else
        {
            if (pcfsCur->c4SerialNum() != fcolSerialNums[c4Index])
            {
                pcfsCur->FormatValue(colValues[c4Index], m_strmFmt);
                fcolSerialNums[c4Index] = pcfsCur->c4SerialNum();

                // It's a change and we are not in error state, if we were before
                fcolErrors[c4Index] = kCIDLib::False;
                fcolChanged[c4Index] = kCIDLib::True;

                bAnyChanges = kCIDLib::True;
            }
             else
            {
                //
                //  No value change, but if we were in error state before, then that's
                //  a change.
                //
                if (fcolErrors[c4Index])
                {
                    fcolChanged[c4Index] = kCIDLib::True;
                    bAnyChanges = kCIDLib::True;
                }
                 else
                {
                    fcolChanged[c4Index] = kCIDLib::False;
                }

                fcolErrors[c4Index] = kCIDLib::False;
                colValues[c4Index].Clear();
            }
        }
    }
    return bAnyChanges;
}


//
//  Look up a field by name and return its id and type. We can optionally
//  throw if not found, or just return a status. We have one that doesn't
//  return the type, just for convenience.
//
tCIDLib::TBoolean
TCQCServerBase::bQueryFldId(const   TString&                strToFind
                            ,       tCIDLib::TCard4&        c4Id
                            ,       tCQCKit::EFldTypes&     eType
                            , const tCIDLib::TBoolean       bThrowIfNot) const
{
    TLocker lockrSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strToFind, bThrowIfNot);
    if (!pflddTar)
    {
        if (bThrowIfNot)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownFldName
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strMoniker()
                , strToFind
            );
        }
        return kCIDLib::False;
    }

    c4Id = pflddTar->c4Id();
    eType = pflddTar->eType();
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCServerBase::bQueryFldId(const   TString&                strToFind
                            ,       tCIDLib::TCard4&        c4Id
                            , const tCIDLib::TBoolean       bThrowIfNot) const
{
    tCQCKit::EFldTypes eDummy;
    return bQueryFldId(strToFind, c4Id, eDummy, bThrowIfNot);
}


//
//  Query the name of a field by its id, return whether id is valid or
//  not. If so, return the name of the field.
//
tCIDLib::TBoolean
TCQCServerBase::bQueryFldName(  const   tCIDLib::TCard4 c4Id
                                ,       TString&        strToFill) const
{
    TLocker lockrSync(&m_mtxSync);

    const TCQCFldStore* pcfsSrc = pcfsFind(c4Id, kCIDLib::False);
    if (!pcfsSrc)
        return kCIDLib::False;

    strToFill = pcfsSrc->flddInfo().strName();
    return kCIDLib::True;
}


//
//  If we have a trigger on the indicated field, return it and the field
//  definition info.
//
tCIDLib::TBoolean
TCQCServerBase::bQueryFldTrigger(const  TString&            strFldName
                                ,       TCQCFldEvTrigger&   fetToFill
                                ,       TCQCFldDef&         flddToFill) const
{
    TLocker lockrSync(&m_mtxSync);

    // See if we can find this field, throw if not
    const TCQCFldStore* pcfsSrc = pcfsFind(strFldName);

    // Give back the field def info
    flddToFill = pcfsSrc->flddInfo();

    // And now see if there's any field event trigger
    const TCQCFldEvTrigger* pfetTar = m_cqcdcThis.pfetFind(strFldName);
    if (pfetTar)
    {
        fetToFill = *pfetTar;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  This guy returns the field names of all the fields that have triggers
//  associated with them.
//
tCIDLib::TBoolean
TCQCServerBase::bQueryFldTriggers(tCIDLib::TStrList& colToFill) const
{
    TLocker lockrSync(&m_mtxSync);

    // Clean the caller's list so we can start filling it in
    colToFill.RemoveAll();

    // Iterate the field store objects and check each one's trigger
    const tCIDLib::TCard4 c4Count = m_colFldIdList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldStore* pcfsCur = m_colFldIdList[c4Index];
        if (pcfsCur->bHasTrigger())
            colToFill.objAdd(pcfsCur->flddInfo().strName());
    }
    return !colToFill.bIsEmpty();
}


//
//  Methods to read the value of all the various field types and do the
//  grunt work involved in that.
//
tCIDLib::TBoolean
TCQCServerBase::bReadBoolFld(const  tCIDLib::TCard4     c4Id
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TBoolean&  bToFill
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreBool* pcfsRet = pcfsGetBoolStore(c4Id, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    bToFill = pcfsRet->bValue();
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCServerBase::bReadBoolFld(const  TString&            strName
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TBoolean&  bToFill
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreBool* pcfsRet = pcfsGetBoolStore(strName, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    bToFill = pcfsRet->bValue();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCServerBase::bReadCardFld(const  tCIDLib::TCard4     c4Id
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TCard4&    c4ToFill
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreCard* pcfsRet = pcfsGetCardStore(c4Id, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    c4ToFill = pcfsRet->c4Value();
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCServerBase::bReadCardFld(const  TString&            strName
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TCard4&    c4ToFill
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreCard* pcfsRet = pcfsGetCardStore(strName, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    c4ToFill = pcfsRet->c4Value();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCServerBase::bReadField( const   tCIDLib::TCard4     c4FieldListId
                            , const tCIDLib::TCard4     c4FldId
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       TString&            strValue
                            ,       tCQCKit::EFldTypes& eType
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);

    // Make sure the field list hasn't changed since they got their ids
    CheckFldListId(c4FieldListId);

    // Try to find this field by its id, and check for read access
    const TCQCFldStore* pcfsTarget = pcfsFind(c4FldId);
    CheckAccess(*pcfsTarget, tCQCKit::EFldAccess::Read);

    // If it hasn't changed, then return false, else return the value
    if (pcfsTarget->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;

    //
    //  Looks ok, so ask the value object to format it's value. We provide
    //  it with a temp stream to use if they want.
    //
    pcfsTarget->FormatValue(strValue, m_strmFmt);
    c4SerialNum = pcfsTarget->c4SerialNum();
    eType = pcfsTarget->flddInfo().eType();
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCServerBase::bReadField( const   tCIDLib::TCard4     c4FieldListId
                            , const tCIDLib::TCard4     c4FldId
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TCard4&    c4RetBytes
                            ,       TMemBuf&            mbufValue
                            ,       tCQCKit::EFldTypes& eType
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);

    // Make sure the field list hasn't changed since they got their ids
    CheckFldListId(c4FieldListId);

    // Try to find this field by its id, and check for read access
    const TCQCFldStore* pcfsTarget = pcfsFind(c4FldId);
    CheckAccess(*pcfsTarget, tCQCKit::EFldAccess::Read);

    // If it hasn't changed, then return false, else return the value
    if (pcfsTarget->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;

    //
    //  Create a stream over the buffer and stream out the class type of
    //  the value object, and then let the value object stream it's value
    //  the stream.
    //
    {
        TBinMBufOutStream strmOut(&mbufValue);
        strmOut << pcfsTarget->fvThis().clsIsA()
                << tCIDLib::EStreamMarkers::Frame;
        pcfsTarget->fvThis().StreamOutValue(strmOut);
        strmOut << tCIDLib::EStreamMarkers::EndObject
                << kCIDLib::FlushIt;
        c4RetBytes = strmOut.c4CurSize();
    }

    c4SerialNum = pcfsTarget->c4SerialNum();
    eType = pcfsTarget->flddInfo().eType();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCServerBase::bReadFieldByName(const  TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       TString&            strValue
                                ,       tCQCKit::EFldTypes& eType
                                , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);

    // Try to find this field by its name, and check for read access
    const TCQCFldStore* pcfsTarget = pcfsFind(strFldName);
    CheckAccess(*pcfsTarget, tCQCKit::EFldAccess::Read);

    // If it hasn't changed, then return false, else return the value
    if (pcfsTarget->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;

    //
    //  Looks ok, so ask the value object to format it's value. We provide
    //  it with a temp stream to use if they want.
    //
    pcfsTarget->FormatValue(strValue, m_strmFmt);
    c4SerialNum = pcfsTarget->c4SerialNum();
    eType = pcfsTarget->flddInfo().eType();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCServerBase::bReadFieldByName(const  TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       tCIDLib::TCard4&    c4RetBytes
                                ,       TMemBuf&            mbufValue
                                ,       tCQCKit::EFldTypes& eType
                                , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);

    // Try to find this field by its name, and check for read access
    const TCQCFldStore* pcfsTarget = pcfsFind(strFldName);
    CheckAccess(*pcfsTarget, tCQCKit::EFldAccess::Read);

    // If it hasn't changed, then return false, else return the value
    if (pcfsTarget->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;

    //
    //  Create a stream over the buffer and stream out the class type of
    //  the value object, and then let the value object stream it's value
    //  the stream.
    //
    {
        TBinMBufOutStream strmOut(&mbufValue);
        strmOut << pcfsTarget->fvThis().clsIsA()
                << tCIDLib::EStreamMarkers::Frame;
        pcfsTarget->fvThis().StreamOutValue(strmOut);
        strmOut << tCIDLib::EStreamMarkers::EndObject;
        strmOut.Flush();
        c4RetBytes = strmOut.c4CurSize();
    }

    c4SerialNum = pcfsTarget->c4SerialNum();
    eType = pcfsTarget->flddInfo().eType();
    return kCIDLib::True;
}



//
//  Type specific field reading methods by id and name
//
tCIDLib::TBoolean
TCQCServerBase::bReadFloatFld(  const   tCIDLib::TCard4     c4Id
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       tCIDLib::TFloat8&   f8ToFill
                                , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreFloat* pcfsRet = pcfsGetFloatStore(c4Id, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    f8ToFill = pcfsRet->f8Value();
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCServerBase::bReadFloatFld(  const   TString&            strName
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       tCIDLib::TFloat8&   f8ToFill
                                , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreFloat* pcfsRet = pcfsGetFloatStore(strName, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    f8ToFill = pcfsRet->f8Value();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCServerBase::bReadIntFld(const   tCIDLib::TCard4     c4Id
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TInt4&     i4ToFill
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreInt* pcfsRet = pcfsGetIntStore(c4Id, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    i4ToFill = pcfsRet->i4Value();
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCServerBase::bReadIntFld(const   TString&            strName
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TInt4&     i4ToFill
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreInt* pcfsRet = pcfsGetIntStore(strName, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    i4ToFill = pcfsRet->i4Value();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCServerBase::bReadStringFld( const   tCIDLib::TCard4     c4Id
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       TString&            strToFill
                                , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreString* pcfsRet = pcfsGetStringStore(c4Id, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    strToFill = pcfsRet->strValue();
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCServerBase::bReadStringFld( const   TString&            strName
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       TString&            strToFill
                                , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreString* pcfsRet = pcfsGetStringStore(strName, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    strToFill = pcfsRet->strValue();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCServerBase::bReadSListFld(  const   tCIDLib::TCard4     c4Id
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       TVector<TString>&   colToFill
                                , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreSList* pcfsRet = pcfsGetSListStore(c4Id, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    colToFill = pcfsRet->colValue();
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCServerBase::bReadSListFld(  const   TString&            strName
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       TVector<TString>&   colToFill
                                , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreSList* pcfsRet = pcfsGetSListStore(strName, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    colToFill = pcfsRet->colValue();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCServerBase::bReadTimeFld(const  tCIDLib::TCard4     c4Id
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TCard8&    c8ToFill
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreTime* pcfsRet = pcfsGetTimeStore(c4Id, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    c8ToFill = pcfsRet->c8Value();
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCServerBase::bReadTimeFld(const  TString&            strName
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TCard8&    c8ToFill
                            , const tCIDLib::TBoolean   bCheckOnline)
{
    TLocker lockrSync(&m_mtxSync);

    if (bCheckOnline)
        CheckOnline(CID_FILE, CID_LINE);
    TCQCFldStoreTime* pcfsRet = pcfsGetTimeStore(strName, tCQCKit::EFldAccess::Read);
    if (pcfsRet->c4SerialNum() == c4SerialNum)
        return kCIDLib::False;
    c4SerialNum = pcfsRet->c4SerialNum();
    c8ToFill = pcfsRet->c8Value();
    return kCIDLib::True;
}


// Set this driver's config string
tCIDLib::TBoolean
TCQCServerBase::bSetCfgData(const   MStreamable&    mstrmbToSet
                            , const tCIDLib::TCard4 c4Extra)
{
    //
    //  Build the path for this driver. They have a fixed format with just the moniker
    //  being used as the differentiator.
    //
    TString strKey(kCQCKit::pszOSKey_CQCSrvDriverCfgs);
    strKey.Append(strMoniker());

    try
    {
        // Get a local config server proxy
        TCfgServerClient cfgsGet(4000);

        tCIDLib::TCard4 c4Ver = 0;
        cfgsGet.bAddOrUpdate(strKey, c4Ver, mstrmbToSet, c4Extra);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  These methods are all called either from the driver thread to handle a client's
//  queued up field write, or because the driver itself is storing a value. If it's
//  not the driver, and the value changes or the field is marked to always do a
//  field change, then we call into the driver so that it can send something to the
//  device if needed.
//
//  We lock until such time as a we have to call out out to the driver, then we get
//  the lock back upon return. If we throw out while in the callback or when checking
//  the returned status, we don't need to get the lock back.
//
//  In theory these should be protected, but there are special issues with things
//  like the PDL and CML drivers where it's not actually the derived driver class
//  that is calling these methods.
//
//  The checks we do are fairly messy, so we have a private helper that does the bulk
//  of the work. It throws if needed, and otherwise returns whether we should report
//  the value to the driver or not.
//
//  Our return indicates whether a new value was stored (i.e. the value has changed
//  from the previously stored value.) It may get sent to the driver (if not from the
//  driver to begin with) even if it hasn't changed, depending on the field settings.
//  But the caller may want to only do certain things if the value actually changed.
//
//  * We consider the first value to take us out of error state, i.e. first good value,
//  as a change always. So the return is 'new value | was in error'.
//
//  If a new value or was in error, then we give the field storage object a chance to
//  send a field trigger, or at least update his 'last state' which is needed for
//  correct latching trigger types.
//
//  If the value did change or was in error, and this is from the driver or the field
//  is set up to indicate that incoming values from the client should be stored up
//  front (almost always the case), then we update the field storage with the new
//  value.
//
//  HOWEVER, they could have re-registered new fields, in which case the field id is
//  no longer valid by the time we come back. So we save the original field list id
//  and only store if we get a new, changed value and the field list hasn't changed.
//
//  We have to pass an access type when we get the field store. If this is being
//  called from the driver, we can always write it, even if it is not marked as
//  writeable, so we pass None flags which will always pass. Else we pass the
//  write flag to make sure it is writeable.
//
tCIDLib::TBoolean
TCQCServerBase::bStoreBoolFld(  const   tCIDLib::TCard4     c4Id
                                , const tCIDLib::TBoolean   bToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);

    TCQCFldStoreBool* pcfsBool = pcfsGetBoolStore
    (
        c4Id, bFromDrv ? tCQCKit::EFldAccess::None : tCQCKit::EFldAccess::Write
    );

    const tCIDLib::TCard4 c4OrgFldListId = m_c4FieldListId;
    const TString& strName = pcfsBool->flddInfo().strName();
    tCQCKit::EValResults eRes = pcfsBool->eValidate(bToSet);
    if (bFldWriteChecks(*pcfsBool, CID_LINE, eRes, bFromDrv, strName))
    {
        mtxSync.Release();
        CheckCommRes(eBoolFldValChanged(strName, c4Id, bToSet), strName);
        mtxSync.Lock();
    }

    const tCIDLib::TBoolean bWasInErr = pcfsBool->bInError();
    const tCIDLib::TBoolean bGotFirstVal = pcfsBool->fvThis().bGotFirstVal();
    const tCIDLib::TBoolean bChange
    (
        (m_c4FieldListId == c4OrgFldListId)
        && ((eRes == tCQCKit::EValResults::OK) || !bGotFirstVal || bWasInErr)
    );
    if (bChange)
    {
        pcfsBool->bInError(kCIDLib::False);
        if (bFromDrv || pcfsBool->flddInfo().bStoreClientWrite())
            pcfsBool->SetValue(bToSet);

        pcfsBool->SendFldChangeTrig(!bGotFirstVal || bWasInErr);
    }
    return bChange;
}

tCIDLib::TBoolean
TCQCServerBase::bStoreBoolFld(  const   TString&            strName
                                , const tCIDLib::TBoolean   bToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strName);
    return bStoreBoolFld(pflddTar->c4Id(), bToSet, bFromDrv);
}


tCIDLib::TBoolean
TCQCServerBase::bStoreCardFld(  const   tCIDLib::TCard4     c4Id
                                , const tCIDLib::TCard4     c4ToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);

    TCQCFldStoreCard* pcfsCard = pcfsGetCardStore
    (
        c4Id, bFromDrv ? tCQCKit::EFldAccess::None : tCQCKit::EFldAccess::Write
    );

    const tCIDLib::TCard4 c4OrgFldListId = m_c4FieldListId;
    const TString& strName = pcfsCard->flddInfo().strName();
    tCQCKit::EValResults eRes = pcfsCard->eValidate(c4ToSet);
    if (bFldWriteChecks(*pcfsCard, CID_LINE, eRes, bFromDrv, strName))
    {
        mtxSync.Release();
        CheckCommRes(eCardFldValChanged(strName, c4Id, c4ToSet), strName);
        mtxSync.Lock();
    }

    const tCIDLib::TBoolean bWasInErr = pcfsCard->bInError();
    const tCIDLib::TBoolean bGotFirstVal = pcfsCard->fvThis().bGotFirstVal();
    const tCIDLib::TBoolean bChange
    (
        (m_c4FieldListId == c4OrgFldListId)
        && ((eRes == tCQCKit::EValResults::OK) || !bGotFirstVal || bWasInErr)
    );
    if (bChange)
    {
        pcfsCard->bInError(kCIDLib::False);
        if (bFromDrv || pcfsCard->flddInfo().bStoreClientWrite())
            pcfsCard->SetValue(c4ToSet);

        pcfsCard->SendFldChangeTrig(!bGotFirstVal || bWasInErr);
    }
    return bChange;
}

tCIDLib::TBoolean
TCQCServerBase::bStoreCardFld(  const   TString&            strName
                                , const tCIDLib::TCard4     c4ToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strName);
    return bStoreCardFld(pflddTar->c4Id(), c4ToSet, bFromDrv);
}


//
//  In some cases they already have the value in a field value object, and just want to
//  polymorphically store it. So we provide a helper method to do that.
//
//  We don't do the same stuff as the others do, because this can only be called from a
//  driver and if the value is stored in a field value object it's got to be reasonable
//  and we never have to do a callback to the driver. That means we also don't have to
//  so see if it's in error state.
//
tCIDLib::TBoolean
TCQCServerBase::bStoreFldValue( const   tCIDLib::TCard4 c4Id
                                , const TCQCFldValue&   fvToStore)
{
    TLocker mtxSync(&m_mtxSync);

    TCQCFldStore* pcfsTar = pcfsFind(c4Id);
    const TString& strName = pcfsTar->flddInfo().strName();

    const tCIDLib::TBoolean bWasInError = pcfsTar->bInError();
    const tCIDLib::TBoolean bGotFirstVal = pcfsTar->fvThis().bGotFirstVal();
    CancelTimedWrite(strName);
    if (pcfsTar->bSetValue(fvToStore) || bWasInError || !bGotFirstVal)
    {
        pcfsTar->SendFldChangeTrig(bWasInError || !bGotFirstVal);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCServerBase::bStoreFloatFld( const   tCIDLib::TCard4     c4Id
                                , const tCIDLib::TFloat8    f8ToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);

    TCQCFldStoreFloat* pcfsFloat = pcfsGetFloatStore
    (
        c4Id, bFromDrv ? tCQCKit::EFldAccess::None : tCQCKit::EFldAccess::Write
    );

    const tCIDLib::TCard4 c4OrgFldListId = m_c4FieldListId;
    const TString& strName = pcfsFloat->flddInfo().strName();
    tCQCKit::EValResults eRes = pcfsFloat->eValidate(f8ToSet);
    if (bFldWriteChecks(*pcfsFloat, CID_LINE, eRes, bFromDrv, strName))
    {
        mtxSync.Release();
        CheckCommRes(eFloatFldValChanged(strName, c4Id, f8ToSet), strName);
        mtxSync.Lock();
    }

    const tCIDLib::TBoolean bWasInErr = pcfsFloat->bInError();
    const tCIDLib::TBoolean bGotFirstVal = pcfsFloat->fvThis().bGotFirstVal();
    const tCIDLib::TBoolean bChange
    (
        (m_c4FieldListId == c4OrgFldListId)
        && ((eRes == tCQCKit::EValResults::OK) || !bGotFirstVal || bWasInErr)
    );
    if (bChange)
    {
        pcfsFloat->bInError(kCIDLib::False);
        if (bFromDrv || pcfsFloat->flddInfo().bStoreClientWrite())
            pcfsFloat->SetValue(f8ToSet);

        pcfsFloat->SendFldChangeTrig(!bGotFirstVal || bWasInErr);
    }
    return bChange;
}

tCIDLib::TBoolean
TCQCServerBase::bStoreFloatFld( const   TString&            strName
                                , const tCIDLib::TFloat8    f8ToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strName);
    return bStoreFloatFld(pflddTar->c4Id(), f8ToSet, bFromDrv);
}


tCIDLib::TBoolean
TCQCServerBase::bStoreIntFld(const  tCIDLib::TCard4     c4Id
                            , const tCIDLib::TInt4      i4ToSet
                            , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);

    TCQCFldStoreInt* pcfsInt = pcfsGetIntStore
    (
        c4Id, bFromDrv ? tCQCKit::EFldAccess::None : tCQCKit::EFldAccess::Write
    );

    const tCIDLib::TCard4 c4OrgFldListId = m_c4FieldListId;
    const TString& strName = pcfsInt->flddInfo().strName();
    tCQCKit::EValResults eRes = pcfsInt->eValidate(i4ToSet);
    if (bFldWriteChecks(*pcfsInt, CID_LINE, eRes, bFromDrv, strName))
    {
        mtxSync.Release();
        CheckCommRes(eIntFldValChanged(strName, c4Id, i4ToSet), strName);
        mtxSync.Lock();
    }

    const tCIDLib::TBoolean bWasInErr = pcfsInt->bInError();
    const tCIDLib::TBoolean bGotFirstVal = pcfsInt->fvThis().bGotFirstVal();
    const tCIDLib::TBoolean bChange
    (
        (m_c4FieldListId == c4OrgFldListId)
        && ((eRes == tCQCKit::EValResults::OK) || !bGotFirstVal || bWasInErr)
    );
    if (bChange)
    {
        pcfsInt->bInError(kCIDLib::False);
        if (bFromDrv || pcfsInt->flddInfo().bStoreClientWrite())
            pcfsInt->SetValue(i4ToSet);

        pcfsInt->SendFldChangeTrig(!bGotFirstVal || bWasInErr);
    }
    return bChange;
}

tCIDLib::TBoolean
TCQCServerBase::bStoreIntFld(const  TString&            strName
                            , const tCIDLib::TInt4      i4ToSet
                            , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strName);
    return bStoreIntFld(pflddTar->c4Id(), i4ToSet, bFromDrv);
}


tCIDLib::TBoolean
TCQCServerBase::bStoreStringFld(const   tCIDLib::TCard4     c4Id
                                , const TString&            strToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);

    TCQCFldStoreString* pcfsString = pcfsGetStringStore
    (
        c4Id, bFromDrv ? tCQCKit::EFldAccess::None : tCQCKit::EFldAccess::Write
    );

    const tCIDLib::TCard4 c4OrgFldListId = m_c4FieldListId;
    const TString& strName = pcfsString->flddInfo().strName();
    tCQCKit::EValResults eRes = pcfsString->eValidate(strToSet);

    if (bFldWriteChecks(*pcfsString, CID_LINE, eRes, bFromDrv, strName))
    {
        mtxSync.Release();
        CheckCommRes(eStringFldValChanged(strName, c4Id, strToSet), strName);
        mtxSync.Lock();
    }

    const tCIDLib::TBoolean bWasInErr = pcfsString->bInError();
    const tCIDLib::TBoolean bGotFirstVal = pcfsString->fvThis().bGotFirstVal();
    const tCIDLib::TBoolean bChange
    (
        (m_c4FieldListId == c4OrgFldListId)
        && ((eRes == tCQCKit::EValResults::OK) || !bGotFirstVal || bWasInErr)
    );
    if (bChange)
    {
        pcfsString->bInError(kCIDLib::False);

        if (bFromDrv || pcfsString->flddInfo().bStoreClientWrite())
            pcfsString->SetValue(strToSet);

        pcfsString->SendFldChangeTrig(!bGotFirstVal || bWasInErr);
    }
    return bChange;
}

tCIDLib::TBoolean
TCQCServerBase::bStoreStringFld(const   TString&            strName
                                , const TString&            strToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strName);
    return bStoreStringFld(pflddTar->c4Id(), strToSet, bFromDrv);
}


tCIDLib::TBoolean
TCQCServerBase::bStoreSListFld( const   tCIDLib::TCard4     c4Id
                                , const TVector<TString>&   colToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);

    TCQCFldStoreSList* pcfsSList = pcfsGetSListStore
    (
        c4Id, bFromDrv ? tCQCKit::EFldAccess::None : tCQCKit::EFldAccess::Write
    );

    const tCIDLib::TCard4 c4OrgFldListId = m_c4FieldListId;
    const TString& strName = pcfsSList->flddInfo().strName();
    tCQCKit::EValResults eRes = pcfsSList->eValidate(colToSet);
    if (bFldWriteChecks(*pcfsSList, CID_LINE, eRes, bFromDrv, strName))
    {
        mtxSync.Release();
        CheckCommRes(eSListFldValChanged(strName, c4Id, colToSet), strName);
        mtxSync.Lock();
    }

    const tCIDLib::TBoolean bWasInErr = pcfsSList->bInError();
    const tCIDLib::TBoolean bGotFirstVal = pcfsSList->fvThis().bGotFirstVal();
    const tCIDLib::TBoolean bChange
    (
        (m_c4FieldListId == c4OrgFldListId)
        && ((eRes == tCQCKit::EValResults::OK) || !bGotFirstVal || bWasInErr)
    );
    if (bChange)
    {
        pcfsSList->bInError(kCIDLib::False);
        if (bFromDrv || pcfsSList->flddInfo().bStoreClientWrite())
            pcfsSList->SetValue(colToSet);

        pcfsSList->SendFldChangeTrig(!bGotFirstVal || bWasInErr);
    }
    return bChange;
}

tCIDLib::TBoolean
TCQCServerBase::bStoreSListFld( const   TString&            strName
                                , const TVector<TString>&   colToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strName);
    return bStoreSListFld(pflddTar->c4Id(), colToSet, bFromDrv);
}


tCIDLib::TBoolean
TCQCServerBase::bStoreTimeFld(  const   tCIDLib::TCard4     c4Id
                                , const tCIDLib::TCard8&    c8ToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);

    TCQCFldStoreTime* pcfsTime = pcfsGetTimeStore
    (
        c4Id, bFromDrv ? tCQCKit::EFldAccess::None : tCQCKit::EFldAccess::Write
    );

    const tCIDLib::TCard4 c4OrgFldListId = m_c4FieldListId;
    const TString& strName = pcfsTime->flddInfo().strName();
    tCQCKit::EValResults eRes = pcfsTime->eValidate(c8ToSet);
    if (bFldWriteChecks(*pcfsTime, CID_LINE, eRes, bFromDrv, strName))
    {
        mtxSync.Release();
        CheckCommRes(eTimeFldValChanged(strName, c4Id, c8ToSet), strName);
        mtxSync.Lock();
    }

    const tCIDLib::TBoolean bWasInErr = pcfsTime->bInError();
    const tCIDLib::TBoolean bGotFirstVal = pcfsTime->fvThis().bGotFirstVal();
    const tCIDLib::TBoolean bChange
    (
        (m_c4FieldListId == c4OrgFldListId)
        && ((eRes == tCQCKit::EValResults::OK) || !bGotFirstVal || bWasInErr)
    );
    if (bChange)
    {
        pcfsTime->bInError(kCIDLib::False);
        if (bFromDrv || pcfsTime->flddInfo().bStoreClientWrite())
            pcfsTime->SetValue(c8ToSet);

        pcfsTime->SendFldChangeTrig(!bGotFirstVal || bWasInErr);
    }
    return bChange;
}

tCIDLib::TBoolean
TCQCServerBase::bStoreTimeFld(  const   TString&            strName
                                , const tCIDLib::TCard8&    c8ToSet
                                , const tCIDLib::TBoolean   bFromDrv)
{
    TLocker mtxSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strName);
    return bStoreTimeFld(pflddTar->c4Id(), c8ToSet, bFromDrv);
}


//
//  This lets clients wait for the driver thread to die, generally after
//  they call StartShutdown(). If they just want to check, they can pass
//  a zero timeout.
//
//  Note that this may not work in the driver development IDEs, because
//  the driver isn't running on the thread member. It's running on
//  a thread called from the outside.
//
tCIDLib::TBoolean
TCQCServerBase::bWaitTillDead(const tCIDLib::TCard4 c4Millis) const
{
    //
    //  This shouldn't happen, but if it's not set, this is being called
    //  before the driver is even started, so say we are dead.
    //
    if (!m_pthrDevPoll)
        return kCIDLib::True;

    tCIDLib::EExitCodes  eToFill;
    return m_pthrDevPoll->bWaitForDeath(eToFill, c4Millis);
}


// Lock and return the architecture version of this driver (as per the config)
tCIDLib::TCard4 TCQCServerBase::c4ArchVersion() const
{
    TLocker mtxSync(&m_mtxSync);
    return m_cqcdcThis.c4ArchVersion();
}


//
//  Gets a message that starts with a particular start byte, and which has
//  a message length byte embedded in it, and possibly some trailing bytes
//  that aren't counted in the length. LenSub is for those cases where the
//  length includes some bytes already read, and is subtracted from the
//  messgage length value.
//
//  So our sequence of events is:
//
//      Wait for start byte
//      Eat bytes till we get to the c4LenOfs position (length byte)
//      Eat that many bytes minus the lensub value
//      Eat c4TrailBytes bytes
//
//  We have one that takes millis to wait, and another that takes the end
//  time. The first one just calculates the end time and calls the other.
//
//  These make mimimal access to driver members, so sync isn't an issue. Just
//  in case we grab the couple values it accesses up front in a lock.
//
tCIDLib::TCard4
TCQCServerBase::c4GetStartLenMsg(       TObject&        objSrc
                                , const tCIDLib::TCard4 c4WaitFor
                                , const tCIDLib::TCard1 c1StartByte
                                , const tCIDLib::TCard4 c4LenOfs
                                , const tCIDLib::TCard4 c4LenSub
                                , const tCIDLib::TCard4 c4TrailBytes
                                , const tCIDLib::TCard4 c4MaxBytes
                                ,       TMemBuf&        mbufToFill)
{
    return c4GetStartLenMsg2
    (
        objSrc
        , TTime::enctNowPlusMSs(c4WaitFor)
        , c1StartByte
        , c4LenOfs
        , c4LenSub
        , c4TrailBytes
        , c4MaxBytes
        , mbufToFill
    );
}

tCIDLib::TCard4
TCQCServerBase::c4GetStartLenMsg2(          TObject&                objSrc
                                    , const tCIDLib::TEncodedTime   enctEndTime
                                    , const tCIDLib::TCard1         c1StartByte
                                    , const tCIDLib::TCard4         c4LenOfs
                                    , const tCIDLib::TCard4         c4LenSub
                                    , const tCIDLib::TCard4         c4TrailBytes
                                    , const tCIDLib::TCard4         c4MaxBytes
                                    ,       TMemBuf&                mbufToFill)
{
    // Cast the source object to the correct type
    TStreamSocket* psockSrc = nullptr;
    TCommPortBase* pcommSrc = nullptr;
    if (objSrc.bIsDescendantOf(TCommPortBase::clsThis()))
        pcommSrc = static_cast<TCommPortBase*>(&objSrc);
    else if (objSrc.bIsDescendantOf(TStreamSocket::clsThis()))
        psockSrc = static_cast<TStreamSocket*>(&objSrc);
    else
        return 0;

    // Set up the end time and current time stamps
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctEndTime;

    // Make sure the end isn't before now
    if (enctCur >= enctEnd)
        return 0;

    //
    //  Lock and grab the couple members we access during this. We don't
    //  have to worry about them changing while we are in here so this is
    //  fine.
    //
    tCQCKit::EVerboseLvls eVerboseLevel = tCQCKit::EVerboseLvls::Off;
    tCIDLib::TEncodedTime enctExtend = 0;
    {
        TLocker lockrSync(&m_mtxSync);
        enctExtend = m_enctExtend;
        eVerboseLevel = m_eVerboseLevel;
    }

    // Our state machine states
    enum class EStates
    {
        WaitStart
        , WaitLen
        , WaitDBytes
        , WaitTrailBytes
        , Done
    };

    //
    //  This one isn't worth doing a state machine. We just wait for a
    //  start byte, and then wait for the end byte.
    //
    tCIDLib::TBoolean   bExtended = kCIDLib::False;
    tCIDLib::TBoolean   bGotOne = kCIDLib::False;
    tCIDLib::TCard1     c1Cur = 0;
    tCIDLib::TCard4     c4RetBytes = 0;
    tCIDLib::TCard4     c4Counter = 0;
    tCIDLib::TCard4     c4ExpCnt = 0;
    EStates             eState = EStates::WaitStart;
    while (eState < EStates::Done)
    {
        // Wait the remaining time for some more info
        const tCIDLib::TEncodedTime enctWait = enctEnd - enctCur;
        if (psockSrc)
            bGotOne = psockSrc->c4ReceiveRawTO(&c1Cur, enctWait, 1, tCIDLib::EAllData::OkIfNotAll) == 1;
        else
            bGotOne = pcommSrc->c4ReadRawBuf(&c1Cur, 1, enctWait) == 1;

        if (bGotOne)
        {
            // Store this byte. Might toss it, might not
            mbufToFill.PutCard1(c1Cur, c4RetBytes++);

            // If maxed out on input, then we have to stop with what we have
            if (c4RetBytes == c4MaxBytes)
                break;

            switch(eState)
            {
                case EStates::WaitStart :
                {
                    if (c1Cur == c1StartByte)
                    {
                        eState = EStates::WaitLen;
                        c4Counter = 0;
                    }
                     else
                    {
                        // We don't want this one, so reset the counter
                        c4RetBytes = 0;
                    }
                    break;
                }

                case EStates::WaitLen :
                {
                    //
                    //  Wait till we get to the length offset. Bump the counter
                    //  AFTER we check, since it's an offset, not a count.
                    //
                    if (c4Counter == c4LenOfs)
                    {
                        // Reset it to start counting the next thing
                        c4Counter = 0;

                        //
                        //  It's our guy. If any databytes, wait for them.
                        //  Else, go on to the trailing bytes.
                        //
                        c4ExpCnt = 0;
                        if (c1Cur)
                        {
                            eState = EStates::WaitDBytes;
                            c4ExpCnt = c1Cur;

                            // Subtract the length adjustment value if we can
                            if (c4ExpCnt < c4LenSub)
                            {
                                facCQCKit().ThrowErr
                                (
                                    CID_FILE
                                    , CID_LINE
                                    , kKitErrs::errcDbg_BadMsgLenSub
                                    , tCIDLib::ESeverities::Failed
                                    , tCIDLib::EErrClasses::BadParms
                                    , TCardinal(c4LenSub)
                                    , TCardinal(c4ExpCnt)
                                );
                            }
                            c4ExpCnt -= c4LenSub;
                        }

                        // If none expected, then see if we should get trails
                        if (!c4ExpCnt)
                        {
                            if (c4TrailBytes)
                            {
                                eState = EStates::WaitTrailBytes;
                                c4ExpCnt = c4TrailBytes;
                            }
                             else
                            {
                                // There aren't any, so we are done
                                eState = EStates::Done;
                            }
                        }
                    }
                     else
                    {
                        // Not there yet, so bump the counter
                        c4Counter++;
                    }
                    break;
                }

                case EStates::WaitDBytes :
                {
                    // Wait till we've accumulated c4ExpCnt bytes
                    c4Counter++;
                    if (c4Counter == c4ExpCnt)
                    {
                        // If any trail bytes, wait for them, else done
                        if (c4TrailBytes)
                        {
                            eState = EStates::WaitTrailBytes;
                            c4Counter = 0;
                            c4ExpCnt = c4TrailBytes;
                        }
                         else
                        {
                            eState = EStates::Done;
                        }
                    }
                    break;
                }

                case EStates::WaitTrailBytes :
                {
                    // Wait till we've accumulated c4ExpCnt bytes
                    c4Counter++;
                    if (c4Counter == c4ExpCnt)
                        eState = EStates::Done;
                    break;
                }
            };

            if (eState == EStates::Done)
                break;
        }

        // If we are past time, then we may have to give up
        enctCur = TTime::enctNow();
        if (enctCur >= enctEnd)
        {
            //
            //  If we've got some input and we've not already done so, grant
            //  an extension.
            //
            if (c4RetBytes && !bExtended)
            {
                enctEnd = enctCur + enctExtend;
                bExtended = kCIDLib::True;
            }
             else
            {
                // Oh well, out of luck
                c4RetBytes = 0;
                break;
            }
        }

        // Watch for shutdown requests
        if (m_pthrDevPoll->bCheckShutdownRequest())
        {
            c4RetBytes = 0;
            break;
        }
    }

    // If we are in high verbosity, log failures to get terms
    if (eVerboseLevel >= tCQCKit::EVerboseLvls::High)
    {
        // If we got partial input, log that
        if (c4RetBytes && (eState < EStates::Done))
        {
            if (facCQCKit().bLogWarnings())
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Got partial start/len message"
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }
    return c4RetBytes;
}



//
//  Get a message that is delimited by start/stop bytes. It can be gotten
//  from a serial port or socket, which is passed as a generic TObject, and
//  we use runtime typing to figure out what it is.
//
//  We hvae one that takes millis to wait, and another that takes the end
//  time. The first one just calculates the end time and calls the other.
//
//  These make mimimal access to driver members, so sync isn't an issue. Just
//  in case we grab the couple values it accesses up front in a lock.
//
tCIDLib::TCard4
TCQCServerBase::c4GetStartStopMsg(          TObject&            objSrc
                                    , const tCIDLib::TCard4     c4WaitFor
                                    , const tCIDLib::TCard1     c1StartByte
                                    , const tCIDLib::TCard1     c1EndByte
                                    , const tCIDLib::TCard4     c4MaxBytes
                                    ,       TMemBuf&            mbufToFill)
{
    return c4GetStartStopMsg2
    (
        objSrc
        , TTime::enctNowPlusMSs(c4WaitFor)
        , c1StartByte
        , c1EndByte
        , c4MaxBytes
        , mbufToFill
    );
}

tCIDLib::TCard4 TCQCServerBase
::c4GetStartStopMsg2(       TObject&                objSrc
                    , const tCIDLib::TEncodedTime   enctEndTime
                    , const tCIDLib::TCard1         c1StartByte
                    , const tCIDLib::TCard1         c1EndByte
                    , const tCIDLib::TCard4         c4MaxBytes
                    ,       TMemBuf&                mbufToFill)
{
    // Cast the source object to the correct type
    TStreamSocket* psockSrc = nullptr;
    TCommPortBase* pcommSrc = nullptr;
    if (objSrc.bIsDescendantOf(TCommPortBase::clsThis()))
        pcommSrc = static_cast<TCommPortBase*>(&objSrc);
    else if (objSrc.bIsDescendantOf(TStreamSocket::clsThis()))
        psockSrc = static_cast<TStreamSocket*>(&objSrc);
    else
        return 0;

    // Set up the end time and current time stamps
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctEndTime;

    // Make sure the end isn't before now
    if (enctCur >= enctEnd)
        return kCIDLib::False;

    //
    //  Lock and grab the couple members we access during this. We don't
    //  have to worry about them changing while we are in here so this is
    //  fine.
    //
    tCQCKit::EVerboseLvls eVerboseLevel = tCQCKit::EVerboseLvls::Off;
    tCIDLib::TEncodedTime enctExtend = 0;
    {
        TLocker lockrSync(&m_mtxSync);
        enctExtend = m_enctExtend;
        eVerboseLevel = m_eVerboseLevel;
    }

    enum class EStates
    {
        WaitStart
        , WaitEnd
        , Done
    };

    //
    //  This one isn't worth doing a state machine. We just wait for a
    //  start byte, and then wait for the end byte.
    //
    tCIDLib::TBoolean   bExtended = kCIDLib::False;
    tCIDLib::TBoolean   bGotOne = kCIDLib::False;
    tCIDLib::TCard1     c1Cur = 0;
    tCIDLib::TCard4     c4RetBytes = 0;
    EStates             eState = EStates::WaitStart;
    while (eState < EStates::Done)
    {
        // Wait the remaining time for some more info
        const tCIDLib::TEncodedTime enctWait = enctEnd - enctCur;
        if (psockSrc)
            bGotOne = psockSrc->c4ReceiveRawTO(&c1Cur, enctWait, 1, tCIDLib::EAllData::OkIfNotAll) == 1;
        else
            bGotOne = pcommSrc->c4ReadRawBuf(&c1Cur, 1, enctWait) == 1;

        if (bGotOne)
        {
            switch(eState)
            {
                case EStates::WaitStart :
                {
                    //
                    //  If the start byte, store it and move to wait end mode,
                    //  and start accumulating bytes.
                    //
                    if (c1Cur == c1StartByte)
                    {
                        c4RetBytes = 0;
                        mbufToFill.PutCard1(c1StartByte, c4RetBytes++);
                        eState = EStates::WaitEnd;
                    }
                    break;
                }

                case EStates::WaitEnd :
                {
                    // If we've maxed out on input, then we failed
                    if (c4RetBytes >= c4MaxBytes)
                        return 0;

                    // Put the byte into the buffer
                    mbufToFill.PutCard1(c1Cur, c4RetBytes++);

                    // If the end byte, then we are done
                    if (c1Cur == c1EndByte)
                        eState = EStates::Done;
                    break;
                }
            };

            if (eState == EStates::Done)
                break;
        }

        // If we are past time, then we may have to give up
        enctCur = TTime::enctNow();
        if (enctCur >= enctEnd)
        {
            //
            //  If we've got some input and we've not already done so, grant
            //  an extension.
            //
            if (c4RetBytes && !bExtended)
            {
                enctEnd = enctCur + enctExtend;
                bExtended = kCIDLib::True;
            }
             else
            {
                // Oh well, out of luck
                c4RetBytes = 0;
                break;
            }
        }

        // Watch for shutdown requests
        if (m_pthrDevPoll->bCheckShutdownRequest())
        {
            c4RetBytes = 0;
            break;
        }
    }

    // If we are in high verbosity, log failures to get terms
    if (eVerboseLevel >= tCQCKit::EVerboseLvls::High)
    {
        // If we got some input, log that
        if (c4RetBytes && (eState < EStates::Done))
        {
            if (facCQCKit().bLogWarnings())
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Failed to get stop byte"
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }
    return c4RetBytes;
}


//
//  Get a message terminated by a one or two byte sequence. It can be
//  gotten from a serial port or socket, which is passed as a generic
//  TObject, and we use runtime typing to figure out what it is.
//
//  We hvae one that takes millis to wait, and another that takes the end
//  time. The first one just calculates the end time and calls the other.
//
//  These make mimimal access to driver members, so sync isn't an issue. Just
//  in case we grab the couple values it accesses up front in a lock.
//
tCIDLib::TCard4
TCQCServerBase::c4GetTermedMsg(         TObject&            objSrc
                                , const tCIDLib::TCard4     c4WaitFor
                                , const tCIDLib::TCard1     c1Term1
                                , const tCIDLib::TCard1     c1Term2
                                , const tCIDLib::TBoolean   bTwoTerms
                                ,       TMemBuf&            mbufToFill)
{
    return c4GetTermedMsg2
    (
        objSrc
        , TTime::enctNowPlusMSs(c4WaitFor)
        , c1Term1
        , c1Term2
        , bTwoTerms
        , mbufToFill
    );
}

tCIDLib::TCard4
TCQCServerBase::c4GetTermedMsg2(        TObject&                objSrc
                                , const tCIDLib::TEncodedTime   enctEndTime
                                , const tCIDLib::TCard1         c1Term1
                                , const tCIDLib::TCard1         c1Term2
                                , const tCIDLib::TBoolean       bTwoTerms
                                ,       TMemBuf&                mbufToFill)
{
    // We do a simple state machine here
    enum class EStates
    {
        WaitTerm1
        , WaitTerm2
        , Complete
    };

    // Cast the source object to the correct type
    TStreamSocket* psockSrc = nullptr;
    TCommPortBase* pcommSrc = nullptr;
    if (objSrc.bIsDescendantOf(TCommPortBase::clsThis()))
        pcommSrc = static_cast<TCommPortBase*>(&objSrc);
    else if (objSrc.bIsDescendantOf(TStreamSocket::clsThis()))
        psockSrc = static_cast<TStreamSocket*>(&objSrc);
    else
        return 0;

    // Set up the end time and current time stamps
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctEndTime;

    // Make sure the end isn't before now
    if (enctCur >= enctEnd)
        return kCIDLib::False;

    //
    //  Lock and grab the couple members we access during this. We don't
    //  have to worry about them changing while we are in here so this is
    //  fine.
    //
    tCQCKit::EVerboseLvls eVerboseLevel = tCQCKit::EVerboseLvls::Off;
    tCIDLib::TEncodedTime enctExtend = 0;
    {
        TLocker lockrSync(&m_mtxSync);
        enctExtend = m_enctExtend;
        eVerboseLevel = m_eVerboseLevel;
    }

    // Loop until we either get a good message or we time out
    tCIDLib::TBoolean   bExtended = kCIDLib::False;
    tCIDLib::TBoolean   bGotOne = kCIDLib::False;
    tCIDLib::TCard1     c1Cur = 0;
    tCIDLib::TCard4     c4RetBytes = 0;
    EStates             eState = EStates::WaitTerm1;
    while (eState < EStates::Complete)
    {
        // Wait the remaining time for some more info
        const tCIDLib::TEncodedTime enctWait = enctEnd - enctCur;
        if (psockSrc)
            bGotOne = psockSrc->c4ReceiveRawTO(&c1Cur, enctWait, 1, tCIDLib::EAllData::OkIfNotAll) == 1;
        else
            bGotOne = pcommSrc->c4ReadRawBuf(&c1Cur, 1, enctWait) == 1;

        //
        //  If we got something, then update the state machine and store the
        //  data.
        //
        if (bGotOne)
        {
            if (eState == EStates::WaitTerm1)
            {
                if (c1Cur == c1Term1)
                {
                    //
                    //  If no term 2, then we are done. Else go to term2
                    //  state.
                    //
                    if (!bTwoTerms)
                        eState = EStates::Complete;
                    else
                        eState = EStates::WaitTerm2;
                }
                 else
                {
                    // Nothing special, just store it
                    mbufToFill.PutCard1(c1Cur, c4RetBytes++);
                }
            }
             else if (eState == EStates::WaitTerm2)
            {
                if (c1Cur == c1Term2)
                {
                    // It's term2, so we are done
                    eState = EStates::Complete;
                }
                 else
                {
                    // We owe the previous term1 back so store it
                    mbufToFill.PutCard1(c1Term1, c4RetBytes++);

                    //
                    //  If this is a term1, then stay in term2 mode, else
                    //  go back to ground state and store this one.
                    //
                    if (c1Cur != c1Term1)
                    {
                        mbufToFill.PutCard1(c1Cur, c4RetBytes++);
                        eState = EStates::WaitTerm1;
                    }
                }
            }

            // If we got our message, don't bother updating time, just break out
            if (eState == EStates::Complete)
                break;
        }

        //
        //  Update the current time. If past the end time, then the we
        //  timed out, so set the return value to zero bytes and break out.
        //
        enctCur = TTime::enctNow();
        if (enctCur >= enctEnd)
        {
            //
            //  If we have some of the message and haven't already done
            //  so, grant an extension.
            //
            if (c4RetBytes && !bExtended)
            {
                enctEnd = enctCur + enctExtend;
                bExtended = kCIDLib::True;
            }
             else
            {
                // Oh well, out of luck, zero the return bytes and give up
                c4RetBytes = 0;
                break;
            }
        }

        // Watch for a shutdown request
        if (m_pthrDevPoll->bCheckShutdownRequest())
        {
            c4RetBytes = 0;
            break;
        }
    }

    // If we are in high verbosity, log failures to get terms
    if (eVerboseLevel >= tCQCKit::EVerboseLvls::High)
    {
        // If we got partial input, log that
        if (c4RetBytes)
        {
            if (facCQCKit().bLogWarnings())
            {
                if (eState == EStates::WaitTerm1)
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Failed to get first msg term byte"
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }
                 else if (eState == EStates::WaitTerm2)
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Failed to get second msg term byte"
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }
            }
        }
    }
    return c4RetBytes;
}


//
//  Allows CQCServer to ask us for our list of fields. We just give him
//  back a collection of field info objects.
//
tCIDLib::TCard4
TCQCServerBase::c4QueryFields(  TQueryFieldList&    colToFill
                                , tCIDLib::TCard4&  c4FieldListId) const
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  If we have any fields, then copy their field def objects to the
    //  caller's collection.
    //
    colToFill.RemoveAll();
    const tCIDLib::TCard4 c4Count = m_colFldIdList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colToFill.objAdd(m_colFldIdList[c4Index]->flddInfo());

    c4FieldListId = m_c4FieldListId;
    return colToFill.c4ElemCount();
}


//
//  Allows CQCServer to ask us for a list of the names of any of our fields
//  that have the requested access types. And another that uses a regular
//  expression to filter field names.
//
tCIDLib::TCard4
TCQCServerBase::c4QueryFieldNames(          TVector<TString>&   colToFill
                                    , const tCQCKit::EReqAccess eAccess)
{
    colToFill.RemoveAll();

    TLocker lockrSync(&m_mtxSync);
    const tCIDLib::TCard4 c4Count = m_colFldIdList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = m_colFldIdList[c4Index]->flddInfo();
        if (facCQCKit().bCheckFldAccess(eAccess, flddCur.eAccess()))
            colToFill.objAdd(flddCur.strName());
    }
    return colToFill.c4ElemCount();
}

tCIDLib::TCard4
TCQCServerBase::c4QueryFieldNamesRX(        TVector<TCQCFldDef>&    colToFill
                                    , const TString&                strNameRegEx
                                    , const tCQCKit::EReqAccess     eAccess)
{
    // Set up the regular expression
    TRegEx regxName(strNameRegEx);
    colToFill.RemoveAll();

    TLocker lockrSync(&m_mtxSync);
    const tCIDLib::TCard4 c4Count = m_colFldIdList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = m_colFldIdList[c4Index]->flddInfo();
        if (facCQCKit().bCheckFldAccess(eAccess, flddCur.eAccess()))
        {
            if (regxName.bFullyMatches(flddCur.strName()))
                colToFill.objAdd(flddCur);
        }
    }
    return colToFill.c4ElemCount();

}


//
//  Similar to c4QueryFields above, but in this case only returns fields
//  that are marked with one of the indicated semantic types and that have
//  at least the indicated access. This is a new V2 architecture thing,
//  starting with version 4.0.
//
tCIDLib::TCard4 TCQCServerBase::
c4QuerySemFields(const  TFundVector<tCQCKit::EFldSTypes>&   fcolTypes
                , const tCQCKit::EReqAccess                 eReqAccess
                ,       TQueryFieldList&                    colToFill) const
{
    colToFill.RemoveAll();

    // Se how many types to check. If none, then obviously we fail to find any
    const tCIDLib::TCard4 c4STCnt = fcolTypes.c4ElemCount();
    if (!c4STCnt)
        return kCIDLib::False;


    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    // If only one we can do a more efficient check
    const tCQCKit::EFldSTypes eOneType = fcolTypes[0];

    const tCIDLib::TCard4 c4Count = m_colFldIdList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = m_colFldIdList[c4Index]->flddInfo();

        // Take it if it has the right access
        if (facCQCKit().bCheckFldAccess(eReqAccess, flddCur.eAccess()))
        {
            //
            //  If only one type and it matches take it. Else search the types
            //  list for the type of the current field.
            //
            if ((c4STCnt == 1) && (flddCur.eSemType() == eOneType))
                colToFill.objAdd(flddCur);
            else if (fcolTypes.bElementPresent(flddCur.eSemType()))
                colToFill.objAdd(flddCur);
        }
    }
    return colToFill.c4ElemCount();
}


//
//  Return the thread id of our driver thread. We store it away when we
//  the thread starts up, so that we can return it without having to
//  worry about calling into the thread object.
//
//  It never changes after the driver is started, so we don't need to do
//  any locking here.
//
tCIDLib::TCard4 TCQCServerBase::c4ThreadId() const
{
    return m_tidThis;
}


//
//  Provide access to this driver's user context. This never really
//  changes and only CQCServer would access it. So we just allow it
//  to directly access this without locking.
//
const TCQCUserCtx& TCQCServerBase::cuctxDrv() const
{
    return m_cuctxDrv;
}


//
//  This call allows the client to cancel any outstanding timed field write
//  for the indicated field. We just search the list and remove any entry
//  found. Only one per field is allowed into the queue, so we only have
//  to search until we find an entry, then we can break out.
//
tCIDLib::TVoid TCQCServerBase::CancelTimedWrite(const TString& strField)
{
    TLocker lockrSync(&m_mtxSync);

    const tCIDLib::TCard4 c4Count = m_colTimedChanges.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colTimedChanges[c4Index].m_strField == strField)
        {
            // If verbose, log that we are writing the final value
            if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Cancelling timed write for field %(1).%(2)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strMoniker()
                    , strField
                );
            }
            m_colTimedChanges.RemoveAt(c4Index);
            break;
        }
    }
}


//
//  Clears all our stats fields.
//
tCIDLib::TVoid TCQCServerBase::ClearStats()
{
    TLocker lockrSync(&m_mtxSync);

    tCIDLib::ForEachE<tCQCKit::EStdFields>
    (
        [this](const auto eIndex)
        {
            if (eIndex == tCQCKit::EStdFields::LoadTime)
            {
                TCQCFldStoreTime* pcfsFld = pcfsGetTimeStore
                (
                    tCIDLib::c4EnumOrd(eIndex), tCQCKit::EFldAccess::None
                );
                if (pcfsFld)
                    pcfsFld->SetValue(TTime::enctNow());
            }
             else if (eIndex == tCQCKit::EStdFields::Verbosity)
            {
                // Ignore these they are not stats really
            }
             else
            {
                TCQCFldStoreCard* pcfsFld = pcfsGetCardStore
                (
                    tCIDLib::c4EnumOrd(eIndex), tCQCKit::EFldAccess::None
                );
                if (pcfsFld)
                    pcfsFld->SetValue(0);
            }
        }
    );
}



// Get or set the message reading timeout extension value
tCIDLib::TEncodedTime TCQCServerBase::enctExtend() const
{
    TLocker lockrSync(&m_mtxSync);
    return m_enctExtend;
}

tCIDLib::TEncodedTime
TCQCServerBase::enctExtend(const tCIDLib::TEncodedTime enctToSet)
{
    TLocker lockrSync(&m_mtxSync);

    m_enctExtend = enctToSet;
    return m_enctExtend;
}


//
//  In the simulation mode used when developing macro level drivers and when doing
//  testers for some particularly complex drivers, this is called directly from the
//  thread doing the simulating. In the real world under CQCServer, it is started as
//  a separate thread to run that particular driver.
//
//  !!!
//  DO NOT initialize the driver state here. When the driver test harnesses are being
//  used, the initialization of the driver is done by them, not by us here, so the
//  state will already have changed from waiting for init.
//
tCIDLib::EExitCodes
TCQCServerBase::eDevPollThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  if our thread pointer is not set, then we are being called
    //  from an external thread, not via our own normal driver start
    //  method, i.e. in one of the driver IDEs not within CQCServer.
    //
    //  To keep things sane, we will point our driver pointer at this
    //  thread. We have to remember if we own it or not though, so
    //  that we can clean it up in our dtor.
    //
    if (!m_pthrDevPoll)
    {
        m_bOnExtThread = kCIDLib::True;
        m_pthrDevPoll = &thrThis;
    }


    //
    //  Store our thread id for later queries, so that we don't have to
    //  call into the thread object to give it back.
    //
    m_tidThis = thrThis.tidThis();

    // If not in simulator mode, then register us with the rebinder
    if (!m_bSimMode)
        NSRegistration();

    //
    //  And let's enter the main processing loop, where we stay until the driver is
    //  terminated.
    //
    tCIDLib::TBoolean       bExit = kCIDLib::False;
    tCQCKit::ECommResults   eCommRes = tCQCKit::ECommResults::Internal;
    tCQCKit::EDrvStates     ePrevState = tCQCKit::EDrvStates::NotLoaded;
    tCIDLib::TEncodedTime   enctNextTimedCheck = 0;

    while(!bExit)
    {
        try
        {
            //
            //  If verbosity is not Off, see if we need to time it out.
            //  If so, then just set the new level as though a client set
            //  it, then we will pick it up next in the usual way. Only
            //  check if the verbosity time stamp is set.
            //
            if ((m_eVerboseLevel > tCQCKit::EVerboseLvls::Off) && m_enctVerbose)
            {
                if (TTime::enctNow() > m_enctVerbose)
                    m_eVerboseNew = tCQCKit::EVerboseLvls::Off;
            }

            //
            //  If we see a new verbosity level having been set, then
            //  let's take care of that. This will set them back to
            //  same thing again.
            //
            if (m_eVerboseNew != m_eVerboseLevel)
                SetVerboseLevel(m_eVerboseNew);

            //
            //  We have to deal with the issue here of a small number of
            //  special cmds that can be done when we are not connected.
            //  These are put into a special queue.
            //
            //  This guy also handles shutdown requests. If this
            //  guy returns true, then a thread shutdown request was made
            //  on us and it set us to terminated state, so we want to
            //  exit. The CQCServer scavenger thread will see this and
            //  remove us.
            //
            if (bCheckSpecialCmds(ePrevState))
                break;

            //
            //  We have certain time periods we wait between states.
            //  As long as we are moving forward, we do it immediately.
            //  But if we are on the same state again or have moved
            //  backwards, then we wait.
            //
            if (m_eState <= ePrevState)
            {
                // Remember the state before we sleep
                const tCQCKit::EDrvStates eOrgState = m_eState;

                try
                {
                    if (m_eState == tCQCKit::EDrvStates::Connected)
                    {
                        //
                        //  If we are connected, then process waiting
                        //  commands for the length of time that we would
                        //  have waited.
                        //
                        ProcessCmds(m_ac4LoopTimes[m_eState]);
                    }
                     else
                    {
                        //
                        //  Sleep for the indicated time and exit if we
                        //  we asked to while sleeping. We use our own
                        //  sleep method.
                        //
                        bExit = !bSleep(m_ac4LoopTimes[m_eState]);
                        if (bExit)
                            continue;
                    }
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }

            //
            //  Now store the current state as the previous state for next time. We need
            //  to wait until after the sleep check above.
            //
            ePrevState = m_eState;

            //
            //  If we are in paused state, then we don't want to
            //  do anything right now. We've slept our pause interval
            //  above, so we just jump back to the top now.
            //
            if (m_eState == tCQCKit::EDrvStates::Paused)
                continue;

            //
            //  Check for any ready timed writes, if we've not done so in
            //  the last period and we are connected. If we aren't
            //  connected, then the list would have been flushed anyway.
            //
            if ((TTime::enctNow() >= enctNextTimedCheck)
            &&  (m_eState == tCQCKit::EDrvStates::Connected))
            {
                enctNextTimedCheck = TTime::enctNowPlusSecs(15);
                CheckTimedChanges();
            }

            //
            //  Use a switch based on our current state. The job is to
            //  move us forward through the states until we get to the
            //  connected state.
            //
            switch(m_eState)
            {
                case tCQCKit::EDrvStates::WaitInit :
                {
                    if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facCQCKit().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kKitMsgs::midStatus_DrvInitialize
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strMoniker()
                        );
                    }

                    //
                    //  Let's ask the driver to initialize. Generally
                    //  this always works, but it could fail. Note that
                    //  he'll update the
                    //
                    tCQCKit::EDrvInitRes eRes = tCQCKit::EDrvInitRes::Failed;
                    try
                    {
                        eRes = eInitialize();
                    }

                    catch(const TError& errToCatch)
                    {
                        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                        {
                            if (facCQCKit().bShouldLog(errToCatch))
                                TModule::LogEventObj(errToCatch);

                            if (facCQCKit().bLogFailures())
                            {
                                facCQCKit().LogMsg
                                (
                                    CID_FILE
                                    , CID_LINE
                                    , kKitErrs::errcDrv_InitError
                                    , tCIDLib::ESeverities::Failed
                                    , tCIDLib::EErrClasses::InitFailed
                                    , strMoniker()
                                );
                            }
                        }
                        break;
                    }

                    catch(...)
                    {
                        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQCKit().ThrowErr
                            (
                                CID_FILE
                                , CID_LINE
                                , kKitErrs::errcDrv_InitError
                                , TString(L"Unknown exception")
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                                , strMoniker()
                            );
                        }
                    }

                    //
                    //  If he indicated he can't init, then we failed. If
                    //  he reports it worked, then we can move forward.
                    //  Note that the init method sets the state, because
                    //  it's called from other places (the driver test
                    //  harneses and they cannot set it themselves.)
                    //
                    if (eRes == tCQCKit::EDrvInitRes::Failed)
                    {
                        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQCKit().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kKitErrs::errcDrv_InitFailed
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                                , strMoniker()
                            );
                        }
                    }
                    break;
                }

                case tCQCKit::EDrvStates::WaitConfig :
                {
                    if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facCQCKit().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kKitMsgs::midStatus_DrvGetConfig
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strMoniker()
                        );
                    }

                    //
                    //  Some drivers require human config and can't really
                    //  do anything until they get it. So wait for the
                    //  driver to indicate that it has the required config
                    //  before we move forward. Most drivers will just
                    //  immediately return true for this one.
                    //
                    try
                    {
                        if (bWaitConfig(thrThis))
                            m_eState = tCQCKit::EDrvStates::WaitCommRes;
                    }

                    catch(TError& errToCatch)
                    {
                        if (!errToCatch.bLogged()
                        &&  (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium))
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }
                    }

                    if (m_eState == tCQCKit::EDrvStates::WaitCommRes)
                    {
                        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQCKit().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kKitMsgs::midStatus_DrvGotConfig
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                                , strMoniker()
                            );
                        }
                    }
                    break;
                }

                case tCQCKit::EDrvStates::WaitCommRes :
                {
                    if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facCQCKit().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kKitMsgs::midStatus_DrvGetCommRes
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strMoniker()
                        );
                    }

                    //
                    //  The driver hasn't been able to get it's communications
                    //  mechanism opened or allocated for some reason. So we
                    //  just keep calling him to give him a chance to do it.
                    //
                    //  If we get it, move forward to getting connected to
                    //  the device.
                    //
                    try
                    {
                        if (bGetCommResource(thrThis))
                            m_eState = tCQCKit::EDrvStates::WaitConnect;
                    }

                    catch(TError& errToCatch)
                    {
                        if (!errToCatch.bLogged()
                        && (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium))
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }
                    }

                    if (m_eState == tCQCKit::EDrvStates::WaitConnect)
                    {
                        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQCKit().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kKitMsgs::midStatus_DrvGotCommRes
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                                , strMoniker()
                            );
                        }
                    }
                    break;
                }

                case tCQCKit::EDrvStates::WaitConnect :
                {
                    if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facCQCKit().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kKitMsgs::midStatus_DrvConnecting
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strMoniker()
                        );
                    }

                    //
                    //  We just keep giving him a chance to connect to his
                    //  device. If he does, then put us in connected state.
                    //
                    try
                    {
                        eCommRes = eConnectToDevice(thrThis);

                        if (eCommRes == tCQCKit::ECommResults::Success)
                            m_eState = tCQCKit::EDrvStates::Connected;
                        else if (eCommRes == tCQCKit::ECommResults::LostCommRes)
                            m_eState = tCQCKit::EDrvStates::WaitCommRes;

                        //
                        //  Go through all of this driver's fields and make sure
                        //  that they have gotten an initial value. If not, log a
                        //  msg.
                        //
                        if (m_eState == tCQCKit::EDrvStates::Connected)
                            LogBadInitVals();
                    }

                    catch(TError& errToCatch)
                    {
                        if (!errToCatch.bLogged()
                        && (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium))
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }

                        // Assume the worst since an exception leaked out
                        m_eState = tCQCKit::EDrvStates::WaitCommRes;
                    }

                    if (m_eState == tCQCKit::EDrvStates::Connected)
                    {
                        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQCKit().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kKitMsgs::midStatus_DrvConnected
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                                , strMoniker()
                            );
                        }
                    }
                     else if (m_eState == tCQCKit::EDrvStates::WaitCommRes)
                    {
                        // We lost the comm resource
                        LostConnection(ePrevState);
                    }
                    break;
                }

                case tCQCKit::EDrvStates::Connected :
                {
                    //
                    //  We were ok last time we checked, so do a poll cycle.
                    //  If he returns a failure, then knock us either back
                    //  to WaitCommRes or WaitConnect, according to the type
                    //  of error. The call will return a boolean and if it's
                    //  false, the connection was lost. If we get an
                    //  exception that indicates that the comm resource was
                    //  lost, we'll fall back to trying to get the resource
                    //  again.
                    //
                    try
                    {
                        eCommRes = ePollDevice(thrThis);
                        if (eCommRes == tCQCKit::ECommResults::LostConnection)
                            m_eState = tCQCKit::EDrvStates::WaitConnect;
                        else if (eCommRes == tCQCKit::ECommResults::LostCommRes)
                            m_eState = tCQCKit::EDrvStates::WaitCommRes;
                    }

                    catch(TError& errToCatch)
                    {
                        if (!errToCatch.bLogged()
                        && (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium))
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }

                        // Assume the worst since an exception leaked out
                        m_eState = tCQCKit::EDrvStates::WaitCommRes;

                        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQCKit().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , L"Unhandled exception in poll, resetting comm res. Driver=%(1)"
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                                , strMoniker()
                            );
                        }
                    }

                    // If no longer connected, then handle that
                    if (m_eState != tCQCKit::EDrvStates::Connected)
                        LostConnection(ePrevState);
                    break;
                }

                case tCQCKit::EDrvStates::Terminated :
                default :
                {
                    // We should never see any of these here
                }
            };
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged() && (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitMsgs::midStatus_PollCIDExcept
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            //
            //  Assume the worst and drop the comm resource if we got
            //  to the point where it's been obtained.
            //
            try
            {
                if (m_eState > tCQCKit::EDrvStates::WaitCommRes)
                {
                    m_eState = tCQCKit::EDrvStates::WaitCommRes;
                    LostConnection(ePrevState);
                }
            }

            catch(...)
            {
                // <TBD> Should we try to do something here?
            }
        }

        catch(const TDbgExitException&)
        {
            m_pthrDevPoll->ReqShutdownNoSync();
        }

        catch(const TExceptException&)
        {
            //
            //  Gotta let this one pass if in sim mode. In that case we are not
            //  being run as a separate thread, but just called directly. We didn't
            //  register with the name server in this case, so it doesn't matter
            //  if we throw.
            //
            if (m_bSimMode)
                throw;
        }

        catch(...)
        {
            if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitMsgs::midStatus_PollSysExcept
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            //
            //  Assume the worst and drop the comm resource if we got
            //  to the point where it's been obtained.
            //
            try
            {
                if (m_eState > tCQCKit::EDrvStates::WaitCommRes)
                {
                    m_eState = tCQCKit::EDrvStates::WaitCommRes;
                    LostConnection(ePrevState);
                }
            }

            catch(...)
            {
                // <TBD> Should we try to do something here?
            }
        }

        // Reset some stuff again before we start over
        enctNextTimedCheck = 0;
    }

    //
    //  If not in sim mode, we need to deregister ourself from the name server, and
    //  from the rebinder.
    //
    if (!m_bSimMode)
    {
        TString strBinding(TCQCSrvAdminClientProxy::strDrvScope);
        strBinding.Append(kCIDLib::chForwardSlash);
        strBinding.Append(m_strMoniker);

        // First derigster from the rebinder so it won't put it back behind our back
        try
        {
            facCIDOrbUC().bDeregRebindObj(strBinding);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        //
        //  And now try to remove it from the name server. Worst case, if we fail
        //  it will time out because it's not being renewed by the rebinder anymore.
        //
        try
        {
            tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy(3000);
            orbcNS->RemoveBinding(strBinding, kCIDLib::False);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    return tCIDLib::EExitCodes::Normal;
}


//
//  Return a field'stype by id. we can return count if not found, or throw.
//
tCQCKit::EFldTypes
TCQCServerBase::eFldTypeById(const  tCIDLib::TCard4     c4FldId
                            ,       TString&            strName
                            , const tCIDLib::TBoolean   bThrowIfNot) const
{
    TLocker lockrSync(&m_mtxSync);

    if (c4FldId >= m_colFldIdList.c4ElemCount())
    {
        if (bThrowIfNot)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownFldId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strMoniker()
                , TCardinal(c4FldId)
            );
        }
        return tCQCKit::EFldTypes::Count;
    }
    const TCQCFldDef& flddTar = m_colFldIdList[c4FldId]->flddInfo();
    strName = flddTar.strName();
    return flddTar.eType();
}


//
//  Look up the type of a field by name. We can either return count if we
//  don't find it, or throw.
//
tCQCKit::EFldTypes
TCQCServerBase::eFldTypeByName( const   TString&            strToFind
                                ,       tCIDLib::TCard4&    c4FldId
                                , const tCIDLib::TBoolean   bThrowIfNot) const
{
    TLocker lockrSync(&m_mtxSync);
    const TCQCFldDef* pflddTar = pflddFind(strToFind, bThrowIfNot);

    if (!pflddTar)
    {
        if (bThrowIfNot)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownFldName
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strMoniker()
                , strToFind
            );
        }
        return tCQCKit::EFldTypes::Count;
    }

    c4FldId = pflddTar->c4Id();
    return pflddTar->eType();
}


//
//  Handles the initialization of the driver. Note that, unlike most setting
//  of the driver state member, which is mostly done by the driver thread,
//  we set the state ourself in this case, since this method is called
//  directly from the driver test harnesses. So we need to get the state
//  updated if we initialize ok.
//
tCQCKit::EDrvInitRes TCQCServerBase::eInitialize()
{
    // And do the initial driver initialization
    tCQCKit::EDrvInitRes eRes = tCQCKit::EDrvInitRes::Failed;
    try
    {
        //
        //  Register the magic fields that all drivers have. We want them to
        //  already be registered before we even call the driver, so that it
        //  can bump counters during init, before it registers it's fields.
        //
        //  Only do this if we've not done it yet.
        //
        if (m_colFldNameList.bIsEmpty())
            RegDefFields();

        // And let the driver initialize
        eRes = eInitializeImpl();
    }

    catch(TError& errToCatch)
    {
        if (facCQCKit().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facCQCKit().bLogFailures())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitMsgs::midStatus_InitExcept
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Init
            );
        }
    }

    catch(const TExceptException&)
    {
        // It's a CML level exception, so rethrow to IDE
        throw;
    }

    catch(...)
    {
        if (facCQCKit().bLogFailures())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitMsgs::midStatus_InitSysExcept
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Init
            );
        }
    }

    if (eRes == tCQCKit::EDrvInitRes::WaitConfig)
        m_eState = tCQCKit::EDrvStates::WaitConfig;
    else if (eRes == tCQCKit::EDrvInitRes::WaitCommRes)
        m_eState = tCQCKit::EDrvStates::WaitCommRes;
    else
        m_eState = tCQCKit::EDrvStates::WaitInit;

    return eRes;
}


//
//  Get the current state. We don't bother locking to return a simple
//  value.
//
tCQCKit::EDrvStates TCQCServerBase::eState() const
{
    return m_eState;
}


//
//  Conveniences for V2 drivers. We'll build up the name, in a few common V2 formats,
//  and call the basic version indicating throw if not found.
//
//  If the caller is not the driver thread, it just lock while it has access to the
//  field definition, since it could change if the driver re-registers fields.
//
const TCQCFldDef&
TCQCServerBase::flddFind(const  tCQCKit::EDevClasses    eClass
                        , const TString&                strName) const
{
    TString strFld(64UL);
    strFld = tCQCKit::strAltXlatEDevClasses(eClass);
    strFld.Append(kCIDLib::chPoundSign);
    strFld.Append(strName);

    return *pflddFind(strFld, kCIDLib::True);
}

const TCQCFldDef&
TCQCServerBase::flddFind(const  tCQCKit::EDevClasses    eClass
                        , const TString&                strSubUnit
                        , const TString&                strName) const
{
    TString strFld(64UL);
    strFld = tCQCKit::strAltXlatEDevClasses(eClass);
    strFld.Append(kCIDLib::chPoundSign);
    if (!strSubUnit.bIsEmpty())
    {
        strFld.Append(strSubUnit);
        strFld.Append(kCIDLib::chTilde);
    }
    strFld.Append(strName);

    return *pflddFind(strFld, kCIDLib::True);
}

const TCQCFldDef&
TCQCServerBase::flddFind(const  tCQCKit::EDevClasses    eClass
                        , const TString&                strPrefix
                        , const TString&                strName
                        , const TString&                strSuffix) const
{
    TString strFld(64UL);
    strFld = tCQCKit::strAltXlatEDevClasses(eClass);
    strFld.Append(kCIDLib::chPoundSign);

    if (!strPrefix.bIsEmpty())
    {
        strFld.Append(strPrefix);
        strFld.Append(kCIDLib::chUnderscore);
    }

    strFld.Append(strName);

    if (!strSuffix.bIsEmpty())
    {
        strFld.Append(kCIDLib::chUnderscore);
        strFld.Append(strSuffix);
    }
    return *pflddFind(strFld, kCIDLib::True);
}

const TCQCFldDef&
TCQCServerBase::flddFind(const  tCQCKit::EDevClasses    eClass
                        , const tCIDLib::TCh* const     pszPrefix
                        , const TString&                strName
                        , const tCIDLib::TCh* const     pszSuffix) const
{
    TString strFld(64UL);
    strFld = tCQCKit::strAltXlatEDevClasses(eClass);
    strFld.Append(kCIDLib::chPoundSign);

    if (pszPrefix && *pszPrefix)
    {
        strFld.Append(pszPrefix);
        strFld.Append(kCIDLib::chUnderscore);
    }

    strFld.Append(strName);

    if (pszSuffix && *pszSuffix)
    {
        strFld.Append(kCIDLib::chUnderscore);
        strFld.Append(pszSuffix);
    }
    return *pflddFind(strFld, kCIDLib::True);
}


//
//  Find a field by it's id. This is almost always called from the driver,
//  but the CML driver wrapper needs it as well and some other drivers
//  that use helper classes.
//
//  If this is called from any thread other than the driver thread, the
//  caller should lock while accessing the returned field def reference, since
//  it could change.
//
const TCQCFldDef& TCQCServerBase::flddFromId(const tCIDLib::TCard4 c4Id) const
{
    return pcfsFind(c4Id)->flddInfo();
}


//
//  Get the currenty verbosity level. Don't bother locking to return a simple
//  value.
//
tCQCKit::EVerboseLvls TCQCServerBase::eVerboseLevel() const
{
    return m_eVerboseLevel;
}


//
//  A convenience method to pull the contents of a string list field out and
//  format it into a string in the standard single line format (a comma
//  separated list of quoted values. This is the form that such strings are
//  provided in for actions and places where the collection of strings form
//  isn't practical.
//
tCIDLib::TVoid
TCQCServerBase::FormatStrListFld(const  tCIDLib::TCard4 c4Id
                                ,       TString&        strToFill) const
{
    TLocker mtxSync(&m_mtxSync);

    //
    //  Get the field store for this guy, which will also verify that it's really
    //  a readable string list. He provides a method to do this formatting, sowe
    //  can just pass the call through to him.
    //
    const TCQCFldStoreSList* pcfsSrc = pcfsGetSListStore
    (
        c4Id, tCQCKit::EFldAccess::Read
    );

    //
    //  We have to provide him with a temp string out stream to use for any
    //  formatting needs. We happen to know that this particular guy doesn't
    //  use it, so we just allocate a very minimally sized one.
    //
    TTextStringOutStream strmTar(8UL);
    pcfsSrc->FormatValue(strToFill, strmTar);
}



//
//  These are only called from the driver itself, so we don't have to
//  worry about checking for being online or any of that stuff.
//
tCIDLib::TVoid TCQCServerBase::IncBadMsgCounter()
{
    TLocker lockrSync(&m_mtxSync);
    TCQCFldStoreCard* pcfsBadMsgs = pcfsGetCardStore
    (
        tCIDLib::c4EnumOrd(tCQCKit::EStdFields::BadMsgs), tCQCKit::EFldAccess::None
    );
    pcfsBadMsgs->Inc();
}


tCIDLib::TVoid TCQCServerBase::IncFailedWriteCounter()
{
    TLocker lockrSync(&m_mtxSync);
    TCQCFldStoreCard* pcfsFailedWrt = pcfsGetCardStore
    (
        tCIDLib::c4EnumOrd(tCQCKit::EStdFields::FailedWrites), tCQCKit::EFldAccess::None
    );
    pcfsFailedWrt->Inc();
}


tCIDLib::TVoid TCQCServerBase::IncNakCounter()
{
    TLocker lockrSync(&m_mtxSync);
    TCQCFldStoreCard* pcfsNaks = pcfsGetCardStore
    (
        tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Naks), tCQCKit::EFldAccess::None
    );
    pcfsNaks->Inc();
}


tCIDLib::TVoid TCQCServerBase::IncReconfigCounter()
{
    TLocker lockrSync(&m_mtxSync);
    TCQCFldStoreCard* pcfsReconf = pcfsGetCardStore
    (
        tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Reconfigs), tCQCKit::EFldAccess::None
    );
    pcfsReconf->Inc();
}


tCIDLib::TVoid TCQCServerBase::IncTimeoutCounter()
{
    TLocker lockrSync(&m_mtxSync);
    TCQCFldStoreCard* pcfsTOs = pcfsGetCardStore
    (
        tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Timeouts), tCQCKit::EFldAccess::None
    );
    pcfsTOs->Inc();
}


tCIDLib::TVoid TCQCServerBase::IncUnknownMsgCounter()
{
    TLocker lockrSync(&m_mtxSync);
    TCQCFldStoreCard* pcfsUnkMsgs = pcfsGetCardStore
    (
        tCIDLib::c4EnumOrd(tCQCKit::EStdFields::UnknownMsgs), tCQCKit::EFldAccess::None
    );
    pcfsUnkMsgs->Inc();
}


tCIDLib::TVoid TCQCServerBase::IncUnknownWriteCounter()
{
    TLocker lockrSync(&m_mtxSync);
    TCQCFldStoreCard* pcfsUnkWrt = pcfsGetCardStore
    (
        tCIDLib::c4EnumOrd(tCQCKit::EStdFields::UnknownWrites), tCQCKit::EFldAccess::None
    );
    pcfsUnkWrt->Inc();
}


//
//  Methods for folks like CQCServer, and the CML driver class, to queue
//  up field write commands for processing. It avoids a lot of duplicated
//  grunt work in clients, and let's us control how it is done.
//
//  We have one for each type, which handles either by name or by id
//  writes. The caller provides one or the other, so we just store them.
//
//  We throw no exceptions back form these, except in the case where
//  we cannot allocate a command object from the pool.
//
//  See the class header comments for details on the queueing of commands
//  and how it all works in detail.
//
//  In these we have to lock the command queue always because we always
//  want to see if we have an existing one to update.
//
TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQBoolFld(  const   TString&                strName
                                , const tCIDLib::TCard4         c4FldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TBoolean       bToSet
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    // Lock and get a command object
    tCIDLib::TBoolean bWait = kCIDLib::False;
    TDrvCmd* pdcmdRet = nullptr;
    {
        // Lock the queue while we do this
        TLocker lockrQ(&m_colCmdQ);

        // Call a helper to set up the command other than value
        tCIDLib::TBoolean bNew;
        pdcmdRet = pdcmdGetFldWrtCmd
        (
            strName
            , c4FldListId
            , c4FldId
            , eWait
            , EDrvCmds::WrtBoolByName
            , EDrvCmds::WrtBoolById
            , bNew
            , bWait
        );

        pdcmdRet->m_bVal = bToSet;

        if (bNew)
            QueueCmd(m_colCmdQ, pdcmdRet);
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}

TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQCardFld(  const   TString&                strName
                                , const tCIDLib::TCard4         c4FldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TCard4         c4ToSet
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    // Lock and get a command object
    tCIDLib::TBoolean bWait = kCIDLib::False;
    TDrvCmd* pdcmdRet = nullptr;
    {
        // Lock the queue while we do this
        TLocker lockrQ(&m_colCmdQ);

        // Call a helper to set up the command other than value
        tCIDLib::TBoolean bNew;
        pdcmdRet = pdcmdGetFldWrtCmd
        (
            strName
            , c4FldListId
            , c4FldId
            , eWait
            , EDrvCmds::WrtCardByName
            , EDrvCmds::WrtCardById
            , bNew
            , bWait
        );

        // Put the value in it
        pdcmdRet->m_c4Val = c4ToSet;

        // Queue if if new
        if (bNew)
            QueueCmd(m_colCmdQ, pdcmdRet);
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}

TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQFloatFld( const   TString&                strName
                                , const tCIDLib::TCard4         c4FldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TFloat8        f8ToSet
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    // Lock and get a command object
    tCIDLib::TBoolean bWait = kCIDLib::False;
    TDrvCmd* pdcmdRet = nullptr;
    {
        // Lock the queue while we do this
        TLocker lockrQ(&m_colCmdQ);

        // Call a helper to set up the command other than value
        tCIDLib::TBoolean bNew;
        pdcmdRet = pdcmdGetFldWrtCmd
        (
            strName
            , c4FldListId
            , c4FldId
            , eWait
            , EDrvCmds::WrtFloatByName
            , EDrvCmds::WrtFloatById
            , bNew
            , bWait
        );

        pdcmdRet->m_f8Val = f8ToSet;

        if (bNew)
            QueueCmd(m_colCmdQ, pdcmdRet);
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}

TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQIntFld(const  TString&                strName
                            , const tCIDLib::TCard4         c4FldListId
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TInt4          i4ToSet
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    // Lock and get a command object
    tCIDLib::TBoolean bWait = kCIDLib::False;
    TDrvCmd* pdcmdRet = nullptr;
    {
        // Lock the queue while we do this
        TLocker lockrQ(&m_colCmdQ);

        // Call a helper to set up the command other than value
        tCIDLib::TBoolean bNew;
        pdcmdRet = pdcmdGetFldWrtCmd
        (
            strName
            , c4FldListId
            , c4FldId
            , eWait
            , EDrvCmds::WrtIntByName
            , EDrvCmds::WrtIntById
            , bNew
            , bWait
        );

        pdcmdRet->m_i4Val = i4ToSet;

        if (bNew)
            QueueCmd(m_colCmdQ, pdcmdRet);
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}


TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQStringFld(const   TString&                strName
                                , const tCIDLib::TCard4         c4FldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const TString&                strToSet
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    // Lock and get a command object queued up
    tCIDLib::TBoolean bWait = kCIDLib::False;
    TDrvCmd* pdcmdRet = nullptr;
    {
        // Lock the queue while we do this
        TLocker lockrQ(&m_colCmdQ);

        tCIDLib::TBoolean bNew;
        pdcmdRet = pdcmdGetFldWrtCmd
        (
            strName
            , c4FldListId
            , c4FldId
            , eWait
            , EDrvCmds::WrtStrByName
            , EDrvCmds::WrtStrById
            , bNew
            , bWait
        );

        // Whether new or overriding a previous one, update the value
        pdcmdRet->m_strVal = strToSet;

        // If new, then queue it up
        if (bNew)
            QueueCmd(m_colCmdQ, pdcmdRet);
    }

    // If not waiting, don't return the command
    if (!bWait)
        return nullptr;
    return pdcmdRet;
}

TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQSListFld( const   TString&                strName
                                , const tCIDLib::TCard4         c4FldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TStrList&      colToSet
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    // Lock and get a command object
    tCIDLib::TBoolean bWait = kCIDLib::False;
    TDrvCmd* pdcmdRet = nullptr;
    {
        // Lock the queue while we do this
        TLocker lockrQ(&m_colCmdQ);

        tCIDLib::TBoolean bNew;
        pdcmdRet = pdcmdGetFldWrtCmd
        (
            strName
            , c4FldListId
            , c4FldId
            , eWait
            , EDrvCmds::WrtStrListByName
            , EDrvCmds::WrtStrListById
            , bNew
            , bWait
        );

        pdcmdRet->m_colStrList = colToSet;

        if (bNew)
            QueueCmd(m_colCmdQ, pdcmdRet);
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}


TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQTimeFld(  const   TString&                strName
                                , const tCIDLib::TCard4         c4FldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TCard8&        c8ToSet
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    // Lock and get a command object
    tCIDLib::TBoolean bWait = kCIDLib::False;
    TDrvCmd* pdcmdRet = nullptr;
    {
        // Lock the queue while we do this
        TLocker lockrQ(&m_colCmdQ);

        tCIDLib::TBoolean bNew;
        pdcmdRet = pdcmdGetFldWrtCmd
        (
            strName
            , c4FldListId
            , c4FldId
            , eWait
            , EDrvCmds::WrtTimeByName
            , EDrvCmds::WrtTimeById
            , bNew
            , bWait
        );

        pdcmdRet->m_enctVal = c8ToSet;

        if (bNew)
            QueueCmd(m_colCmdQ, pdcmdRet);
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}


TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQWriteFld( const   TString&                strName
                                , const tCIDLib::TCard4         c4FldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const TString&                strToSet
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    // Lock and get a command object
    tCIDLib::TBoolean bWait = kCIDLib::False;
    TDrvCmd* pdcmdRet = nullptr;
    {
        // Lock the queue while we do this
        TLocker lockrQ(&m_colCmdQ);

        tCIDLib::TBoolean bNew;
        pdcmdRet = pdcmdGetFldWrtCmd
        (
            strName
            , c4FldListId
            , c4FldId
            , eWait
            , EDrvCmds::WrtFieldByName
            , EDrvCmds::WrtFieldById
            , bNew
            , bWait
        );

        pdcmdRet->m_strVal = strToSet;

        if (bNew)
            QueueCmd(m_colCmdQ, pdcmdRet);
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}


//
//  Like the field write queuing methods above, these queue up backdoor
//  commands.
//
//  For query type commands, there's no option to wait or not, so the initial
//  ref count is always set to 2, because they have to wait. And we don't
//  have to lock the queue while we get the command set up because we aren't
//  going to try to update an existing one.
//
//  For outgoing commands, we always try to replace existing queued
//  commands, and waiting is an option. So the init ref count may be 1 or
//  2, and we have to lock the target queue because we have to search for
//  an existing one to update.
//
//  The reservation of a command is from the global pool, which is separately
//  thread safe, so we don't have to sync that at the driver level. When
//  we put the command into the queue, the qeue is thread safe, so we don't
//  have to lock explicitly for that.
//
TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQQueryBoolVal( const   TString&            strValId
                                    , const TString&            strValName
                                    , const tCIDLib::TBoolean   bTwoVer)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    TDrvCmd* pdcmdRet = nullptr;
    {
        pdcmdRet = pdcmdReserve(0, strMoniker());
        pdcmdRet->m_eCmd = EDrvCmds::QueryBoolVal;
        pdcmdRet->m_c4RefCnt = 2;
        pdcmdRet->m_bVal = bTwoVer;
        pdcmdRet->m_strType = strValId;
        pdcmdRet->m_strName = strValName;

        QueueCmd(m_colCmdQ, pdcmdRet);
    }
    return pdcmdRet;
}

TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQQueryCardVal( const   TString&            strValId
                                    , const TString&            strValName
                                    , const tCIDLib::TBoolean   bTwoVer)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    TDrvCmd* pdcmdRet = nullptr;
    {
        pdcmdRet = pdcmdReserve(0, strMoniker());
        pdcmdRet->m_eCmd = EDrvCmds::QueryCardVal;
        pdcmdRet->m_c4RefCnt = 2;
        pdcmdRet->m_bVal = bTwoVer;
        pdcmdRet->m_strType = strValId;
        pdcmdRet->m_strName = strValName;

        QueueCmd(m_colCmdQ, pdcmdRet);
    }
    return pdcmdRet;
}

TCQCServerBase::TDrvCmd* TCQCServerBase::pdcmdQQueryConfig()
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    TDrvCmd* pdcmdRet = nullptr;
    {
        pdcmdRet = pdcmdReserve(0, strMoniker());
        pdcmdRet->m_eCmd = EDrvCmds::QueryConfig;
        pdcmdRet->m_c4RefCnt = 2;

        QueueCmd(m_colCmdQ, pdcmdRet);
    }
    return pdcmdRet;
}

TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQQueryData(const   TString&            strType
                                , const TString&            strName
                                , const tCIDLib::TCard4&    c4Bytes
                                , const THeapBuf&           mbufData
                                , const tCIDLib::TBoolean   bTwoVer)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    TDrvCmd* pdcmdRet = nullptr;
    {
        pdcmdRet = pdcmdReserve(bTwoVer ? c4Bytes : 0, strMoniker());

        pdcmdRet->m_eCmd = EDrvCmds::QueryData;
        pdcmdRet->m_c4RefCnt = 2;
        pdcmdRet->m_bVal = bTwoVer;
        pdcmdRet->m_strType = strType;
        pdcmdRet->m_strName = strName;

        // If the 2 version, then we have incoming data
        if (bTwoVer)
        {
            pdcmdRet->m_c4Count = c4Bytes;
            if (c4Bytes)
                pdcmdRet->m_mbufVal.CopyIn(mbufData, c4Bytes);
        }
        QueueCmd(m_colCmdQ, pdcmdRet);
    }
    return pdcmdRet;
}

TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQQueryIntVal(  const   TString&            strValId
                                    , const TString&            strValName
                                    , const tCIDLib::TBoolean   bTwoVer)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    TDrvCmd* pdcmdRet = nullptr;
    {
        pdcmdRet = pdcmdReserve(0, strMoniker());
        pdcmdRet->m_eCmd = EDrvCmds::QueryIntVal;
        pdcmdRet->m_c4RefCnt = 2;
        pdcmdRet->m_bVal = bTwoVer;
        pdcmdRet->m_strType = strValId;
        pdcmdRet->m_strName = strValName;

        QueueCmd(m_colCmdQ, pdcmdRet);
    }

    return pdcmdRet;
}

TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQQueryTextVal( const   TString&    strValId
                                    , const TString&    strValName)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    TDrvCmd* pdcmdRet = nullptr;
    {
        pdcmdRet = pdcmdReserve(0, strMoniker());
        pdcmdRet->m_eCmd = EDrvCmds::QueryTextVal;
        pdcmdRet->m_c4RefCnt = 2;
        pdcmdRet->m_strType = strValId;
        pdcmdRet->m_strName = strValName;

        QueueCmd(m_colCmdQ, pdcmdRet);
    }
    return pdcmdRet;
}

TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQSendCmd(  const   TString&                strCmd
                                , const TString&                strParms
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    tCIDLib::TCard4 c4InitRef;
    const tCIDLib::TBoolean bWait = bDefCmdRefCnt(eWait, c4InitRef);

    TDrvCmd* pdcmdRet = nullptr;
    {
        pdcmdRet = pdcmdReserve(0, strMoniker());
        pdcmdRet->m_eCmd = EDrvCmds::SendCmd;
        pdcmdRet->m_c4RefCnt = c4InitRef;
        pdcmdRet->m_strType = strCmd;
        pdcmdRet->m_strParams = strParms;

        QueueCmd(m_colCmdQ, pdcmdRet);
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}

//
//  This calls sends and can receive bytes, but at this level we just have to
//  put them into queued command, so our data parms are in only. The caller
//  will get the data out again if desired from the command.
//
TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQSendData( const   TString&                strType
                                , const TString&                strName
                                , const tCIDLib::TCard4&        c4Bytes
                                , const THeapBuf&               mbufData
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    tCIDLib::TCard4 c4InitRef;
    const tCIDLib::TBoolean bWait = bDefCmdRefCnt(eWait, c4InitRef);

    TDrvCmd* pdcmdRet = nullptr;
    {
        pdcmdRet = pdcmdReserve(c4Bytes, strMoniker());
        pdcmdRet->m_eCmd = EDrvCmds::SendData;
        pdcmdRet->m_c4RefCnt = c4InitRef;
        pdcmdRet->m_strType = strType;
        pdcmdRet->m_strName = strName;
        pdcmdRet->m_c4Count = c4Bytes;
        if (c4Bytes)
            pdcmdRet->m_mbufVal.CopyIn(mbufData, c4Bytes);

        QueueCmd(m_colCmdQ, pdcmdRet);
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}



//
//  Some special case commands, for pause or resume, or to update driver
//  config. Don't have to be connected for some of these.
//
//  We only need to have one of these in the queue, so if we find one already
//  there, since it's nothing but a command, we don't even have to update
//  anything, other than the ref count which will be bumped for us by the
//  lookup command.
//
TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQPauseResumeCmd(const  tCIDLib::TBoolean       bPause
                                    , const tCQCKit::EDrvCmdWaits   eWait)
{
    tCIDLib::TCard4 c4InitRef;
    const tCIDLib::TBoolean bWait = bDefCmdRefCnt(eWait, c4InitRef);

    TDrvCmd* pdcmdRet = nullptr;
    {
        // Lock the queue while we do this
        TLocker lockrQ(&m_colCmdQSpec);

        // See if we already one queued that we can just update
        if (bPause)
            pdcmdRet = pdcmdFindQCmd(m_colCmdQSpec, EDrvCmds::Pause, bWait);
        else
            pdcmdRet = pdcmdFindQCmd(m_colCmdQSpec, EDrvCmds::Resume, bWait);

        //
        //  There's nothing to update, so all we need to check for is
        //  whether we found a previous one or not. If not, create a
        //  new one and queue it.
        //
        if (!pdcmdRet)
        {
            pdcmdRet = pdcmdReserve(0, strMoniker());
            pdcmdRet->m_c4RefCnt = c4InitRef;

            // Queue up the appropriate command
            if (bPause)
                pdcmdRet->m_eCmd = EDrvCmds::Pause;
            else
                pdcmdRet->m_eCmd = EDrvCmds::Resume;

            QueueCmd(m_colCmdQSpec, pdcmdRet);
        }
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}


TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQReconfig( const   TCQCDriverObjCfg&       cqcdcToSet
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    tCIDLib::TCard4 c4InitRef;
    const tCIDLib::TBoolean bWait = bDefCmdRefCnt(eWait, c4InitRef);

    TDrvCmd* pdcmdRet = nullptr;
    {
        TLocker lockrQ(&m_colCmdQSpec);

        // See if there's already one queued on the special queue
        pdcmdRet = pdcmdFindQCmd(m_colCmdQSpec, EDrvCmds::Reconfig, bWait);

        if (pdcmdRet)
        {
            //
            //  Just update the config data. The pointer has to have been allocated
            //  in order for the original to have gotten queued up.
            //
            *pdcmdRet->m_pcqcdcNew = cqcdcToSet;
        }
         else
        {
            pdcmdRet = pdcmdReserve(0, strMoniker());
            pdcmdRet->m_eCmd = EDrvCmds::Reconfig;
            pdcmdRet->m_c4RefCnt = c4InitRef;

            pdcmdRet->m_pcqcdcNew = new TCQCDriverObjCfg();
            *pdcmdRet->m_pcqcdcNew = cqcdcToSet;

            QueueCmd(m_colCmdQSpec, pdcmdRet);
        }
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}


TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQSetConfig(const   tCIDLib::TCard4&        c4Bytes
                                , const THeapBuf&               mbufCfg
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    tCIDLib::TCard4 c4InitRef;
    const tCIDLib::TBoolean bWait = bDefCmdRefCnt(eWait, c4InitRef);

    TDrvCmd* pdcmdRet = nullptr;
    {
        TLocker lockrQ(&m_colCmdQ);

        //
        //  Make no sense to write one then another, so replace an
        //  existing one if we can.
        //
        pdcmdRet = pdcmdFindQCmd(m_colCmdQ, EDrvCmds::SetConfig, bWait);
        if (pdcmdRet)
        {
            // Just update the data
            pdcmdRet->m_c4Count = c4Bytes;
            pdcmdRet->m_mbufVal.CopyIn(mbufCfg, c4Bytes);
        }
         else
        {
            pdcmdRet = pdcmdReserve(c4Bytes, strMoniker());
            pdcmdRet->m_eCmd = EDrvCmds::SetConfig;
            pdcmdRet->m_c4RefCnt = c4InitRef;
            pdcmdRet->m_c4Count = c4Bytes;
            pdcmdRet->m_mbufVal.CopyIn(mbufCfg, c4Bytes);

            QueueCmd(m_colCmdQ, pdcmdRet);
        }
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}


//
//  Queues up a timed field write command.
//
//  In this case we will look for an existng command of this type for
//  this field and replace it, since it makes no sense to do the already
//  queued on.
//
TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdQTimedFldWrt(  const   TString&                strField
                                    , const TString&                strNewValue
                                    , const TString&                strEndValue
                                    , const tCIDLib::TCard4         c4Minutes
                                    , const tCQCKit::EDrvCmdWaits   eWait)
{

    // We have to be online
    CheckOnline(CID_FILE, CID_LINE);

    tCIDLib::TCard4 c4InitRef;
    const tCIDLib::TBoolean bWait = bDefCmdRefCnt(eWait, c4InitRef);

    TDrvCmd* pdcmdRet = nullptr;
    {
        TLocker lockrQ(&m_colCmdQ);

        // See if we can find an existing one
        pdcmdRet = pdcmdFindQFldWrt
        (
            m_colCmdQ, EDrvCmds::TimedFldWrt, 0, strField, kCIDLib::True, bWait
        );

        // If so, update the values, else create our own
        if (pdcmdRet)
        {
            pdcmdRet->m_strVal = strNewValue;
            pdcmdRet->m_strParams = strEndValue;
            pdcmdRet->m_c4Count = c4Minutes;
        }
         else
        {
            pdcmdRet = pdcmdReserve(0, strMoniker());
            pdcmdRet->m_eCmd = EDrvCmds::TimedFldWrt;
            pdcmdRet->m_c4RefCnt = c4InitRef;
            pdcmdRet->m_strName = strField;
            pdcmdRet->m_strVal = strNewValue;
            pdcmdRet->m_strParams = strEndValue;
            pdcmdRet->m_c4Count = c4Minutes;
            QueueCmd(m_colCmdQ, pdcmdRet);
        }
    }

    if (!bWait)
        return nullptr;
    return pdcmdRet;
}


//
//  Provide const access to the limits object for a field by id. If it
//  doesn't have one, the return is null. If not called from the driver
//  thread, the caller should lock while accessing this info.
//
const TCQCFldLimit* TCQCServerBase::pfldlLimsFor(const tCIDLib::TCard4 c4FldId) const
{
    // Find the field storage. If not found, return null
    const TCQCFldStore* pcfsFor = pcfsFind(c4FldId);
    if (!pcfsFor)
        return nullptr;

    // Return the limits object held by the field storage
    return pcfsFor->pfldlLimits();
}

const TCQCFldLimit*
TCQCServerBase::pfldlLimsFor(const  tCIDLib::TCard4 c4FldId
                            , const TClass&         clsType) const
{
    // Find the field storage. If not found, return null
    const TCQCFldStore* pcfsFor = pcfsFind(c4FldId);
    if (!pcfsFor)
        return nullptr;

    // Get the limit object. If none, return null
    const TCQCFldLimit* pfldlRet = pcfsFor->pfldlLimits();
    if (!pfldlRet)
        return nullptr;

    // See if it's of the indicated type. If not, return null
    if (!pfldlRet->bIsA(clsType))
        return nullptr;

    // Looks ok, so return it
    return pfldlRet;
}


//
//  Find the field definition for a named field. If not called from the
//  driver thread, the caller should lock while accessing this info.
//
const TCQCFldDef*
TCQCServerBase::pflddFind(  const   TString&            strNameToFind
                            , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const TCQCFldStore* pcfsRet = m_colFldNameList.pobjFindByKey
    (
        strNameToFind, kCIDLib::False
    );

    if (!pcfsRet)
    {
        if (bThrowIfNot)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownFldName
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strMoniker()
                , strNameToFind
            );
        }
        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::High)
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownFldName
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strMoniker()
                , strNameToFind
            );
        }
        return nullptr;
    }
    return &pcfsRet->flddInfo();
}


//
//  Derived classes call this to queue up an event trigger. We just pass it on, but
//  this let's us suppress triggers during the connect driver phase when the fields
//  are being set initially.
//
//  It would be way too messy to force drivers to do this, and they wouldn't ever
//  always do it right. They use the same code to store field values generally for
//  the initial driver setup as they do later on during regular back and forth
//  driver comms.
//
//  This is public because the CML driver engine needs to be able to call this
//  on behalf if the CML drivers, which are not derivatives of us. But we assume
//  any required locking has been done.
//
tCIDLib::TVoid
TCQCServerBase::QueueEventTrig( const   tCQCKit::EStdDrvEvs eEvent
                                , const TString&            strFld
                                , const TString&            strVal1
                                , const TString&            strVal2
                                , const TString&            strVal3
                                , const TString&            strVal4)
{
    // Ignore it if not in connected state
    if (m_eState == tCQCKit::EDrvStates::Connected)
    {
        // Lock long enough to get our moniker out then we can unlock
        TString strOurMon;
        {
            TLocker lockrSync(&m_mtxSync);
            strOurMon = m_strMoniker;
        }

        // The rest we just pass through to the CQCKit facility
        facCQCKit().QueueStdEventTrig
        (
            eEvent
            , strOurMon
            , strFld
            , strVal1
            , strVal2
            , strVal3
            , strVal4
        );
    }
}


// Return a copy of our driver config
tCIDLib::TVoid
TCQCServerBase::QueryDrvConfigObj(TCQCDriverObjCfg& cqcdcToFill) const
{
    TLocker lockrSync(&m_mtxSync);
    cqcdcToFill = m_cqcdcThis;
}


// Return some info about this driver
tCIDLib::TVoid
TCQCServerBase::QueryDrvInfo(TString&               strDrvMoniker
                            , TString&              strMake
                            , TString&              strModel
                            , tCIDLib::TCard4&      c4MajVersion
                            , tCIDLib::TCard4&      c4MinVersion
                            , tCQCKit::EDevCats&    eDevCat) const
{
    TLocker lockrSync(&m_mtxSync);

    c4MajVersion = m_cqcdcThis.c4MajVersion();
    c4MinVersion = m_cqcdcThis.c4MinVersion();
    eDevCat = m_cqcdcThis.eCategory();
    strMake = m_cqcdcThis.strMake();
    strModel = m_cqcdcThis.strModel();
    strDrvMoniker = strMoniker();
}

tCIDLib::TVoid
TCQCServerBase::QueryDrvInfo(   tCQCKit::EDrvStates&        eStatus
                                , tCIDLib::TCard4&          c4ArchVer
                                , tCQCKit::TDevClassList&   fcolDevClasses
                                , TString&                  strMake
                                , TString&                  strModel) const
{
    TLocker lockrSync(&m_mtxSync);

    eStatus = m_eState;
    c4ArchVer = m_cqcdcThis.c4ArchVersion();
    fcolDevClasses = m_cqcdcThis.fcolDevClasses();
    strMake = m_cqcdcThis.strMake();
    strModel = m_cqcdcThis.strModel();
}


// Query the definition of a field, throw if not found
tCIDLib::TVoid
TCQCServerBase::QueryFldDef(const   TString&    strFldName
                            ,       TCQCFldDef& flddToFill) const
{
    TLocker lockrSync(&m_mtxSync);

    // Find the field, throw if not found
    const TCQCFldStore* pcfsSrc = pcfsFind(strFldName);
    flddToFill = pcfsSrc->flddInfo();
}


// Query some id info about a given field
tCIDLib::TVoid
TCQCServerBase::QueryFldInfo(const  TString&            strFldName
                            ,       tCIDLib::TCard4&    c4FieldListId
                            ,       tCIDLib::TCard4&    c4FieldId
                            ,       tCQCKit::EFldTypes& eType) const
{
    TLocker lockrSync(&m_mtxSync);

    const TCQCFldStore* pcfsSrc = pcfsFind(strFldName);
    const TCQCFldDef& flddTar = pcfsSrc->flddInfo();

    c4FieldListId = m_c4FieldListId;
    c4FieldId = flddTar.c4Id();
    eType = flddTar.eType();
}


//
//  Query the name of a field by its id. Throw if it's a bad it
//
tCIDLib::TVoid
TCQCServerBase::QueryFldName(const  tCIDLib::TCard4     c4Id
                            ,       tCQCKit::EFldTypes& eType
                            ,       TString&            strToFill) const
{
    TLocker lockrSync(&m_mtxSync);

    const TCQCFldStore* pcfsSrc = pcfsFind(c4Id);
    const TCQCFldDef& flddTar = pcfsSrc->flddInfo();

    eType = flddTar.eType();
    strToFill = flddTar.strName();
}


// Get some basic driver state info
tCIDLib::TVoid
TCQCServerBase::QueryState( tCQCKit::EDrvStates&            eStatus
                            ,       tCQCKit::EVerboseLvls&  eVerbose
                            ,       tCIDLib::TCard4&        c4ThreadId) const
{
    TLocker mtxSync(&m_mtxSync);

    c4ThreadId = m_tidThis;
    eStatus = m_eState;
    eVerbose = m_eVerboseLevel;
}


//
//  CQCServer can call this to get stats for this driver to return to clients. It also
//  returns the stuff that QueryState above does.
//
tCIDLib::TVoid TCQCServerBase::QueryStats(TCQCDrvStats& cdstatsToFill) const
{
    TLocker mtxSync(&m_mtxSync);

    cdstatsToFill.m_c4ThreadId = m_tidThis;
    cdstatsToFill.m_eStatus = m_eState;
    cdstatsToFill.m_eVerboseLvl = m_eVerboseLevel;

    cdstatsToFill.m_c4BadMsgs = c4FldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::BadMsgs));
    cdstatsToFill.m_c4FailedWrites = c4FldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::FailedWrites));
    cdstatsToFill.m_c8LoadTime = c8TimeFldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::LoadTime));
    cdstatsToFill.m_c4LostCommRes = c4FldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::LostCommRes));
    cdstatsToFill.m_c4LostConns = c4FldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::LostConn));
    cdstatsToFill.m_c4Naks = c4FldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Naks));
    cdstatsToFill.m_c4Reconfigures = c4FldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Reconfigs));
    cdstatsToFill.m_c4Timeouts = c4FldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Timeouts));
    cdstatsToFill.m_c4UnknownMsgs = c4FldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::UnknownMsgs));
    cdstatsToFill.m_c4UnknownWrites = c4FldValue(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::UnknownWrites));
}


//
//  Called by CQCServer when a new trigger is removed from one (or all) of
//  our fields by the user via the admin interface. We need to remove the
//  same from our live event list.
//
tCIDLib::TVoid TCQCServerBase::RemoveAllTriggers()
{
    TLocker mtxSync(&m_mtxSync);

    const tCIDLib::TCard4 c4Count = m_colFldIdList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colFldIdList[c4Index]->fetTrigger().eType(tCQCKit::EEvTrTypes::Unused);
    m_cqcdcThis.RemoveAllTriggers();
}

tCIDLib::TVoid TCQCServerBase::RemoveTrigger(const TString& strField)
{
    TLocker mtxSync(&m_mtxSync);

    // Default is to throw if not found, which is what we want
    TCQCFldStore* pcfsTar = pcfsFind(strField);
    pcfsTar->fetTrigger().eType(tCQCKit::EEvTrTypes::Unused);

    //
    //  Update our driver config object as well, so that if anyone comes asking
    //  about our event trigger configuration, they'll get accurate info.
    //
    m_cqcdcThis.RemoveTrigger(strField);
}


//
//  Drivers can pre-emptively set all fields to error state up front, so that any
//  taht don't get values set will be naturally left in error state, but having
//  gotten their first 'value' change.
//
//  We skill write only fields, which don't apply here.
//
//  <TBD> Some point, once we have rehabilited all the drivers to get rid of those
//  that accidentally don't set all their fields, we can get rid of this and just
//  default the error state to true.
//
//  - We skip the stats fields
//  - We assume the caller has locked if that is appropriate (after connected state
//      a lock is required.
//
tCIDLib::TVoid TCQCServerBase::SetAllErrStates()
{
    tCIDLib::TCard4 c4Index = tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Count);
    tCIDLib::TCard4 c4Count = m_colFldIdList.c4ElemCount();
    while (c4Index < c4Count)
    {
        TCQCFldStore& cfsCur = *m_colFldIdList[c4Index];
        if (cfsCur.flddInfo().bIsReadable())
            cfsCur.fvThis().bInError(kCIDLib::True);
        c4Index++;
    }
}


//
//  Sets or clears the error status for the indicated field, by field id.
//  We just look up the field storage object by id and pass the call on to
//  him.
//
tCIDLib::TVoid
TCQCServerBase::SetFieldErr(const   tCIDLib::TCard4     c4Id
                            , const tCIDLib::TBoolean   bToSet)
{
    TLocker mtxSync(&m_mtxSync);
    pcfsFind(c4Id)->bInError(bToSet);
}


//
//  This is called to set a new verbosity. We just store it in the new
//  verbosity level member. If the driver thread sees the new one is
//  different from the current level, it will process the new one.
//
tCIDLib::TVoid
TCQCServerBase::SetNewVerbosity(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerboseNew = eToSet;
}


// Called by the driver to set the poll and reconnect times
tCIDLib::TVoid
TCQCServerBase::SetPollTimes(const  tCIDLib::TCard4 c4PollTime
                            , const tCIDLib::TCard4 c4ReconnTime)
{
    // Validate the values, and clip to minimums
    if (c4PollTime < 50)
        m_ac4LoopTimes[tCQCKit::EDrvStates::Connected] = 50;
    else
        m_ac4LoopTimes[tCQCKit::EDrvStates::Connected] = c4PollTime;

    if (c4ReconnTime < 2000)
        m_ac4LoopTimes[tCQCKit::EDrvStates::WaitConnect] = 2000;
    else
        m_ac4LoopTimes[tCQCKit::EDrvStates::WaitConnect] = c4ReconnTime;
}


//
//  If we are running under CQCServer, it will call this immediately upon creating
//  us, to tell us the object id that we should register ourself under in the name
//  server. All of the drivers register themselves to point to this same interface.
//
//  If running under the C++ test harness, it will do the same, but it will point us
//  at a faux object that he creates, which emulates the CQCServer interface, or as
//  much of it as he requires.
//
tCIDLib::TVoid TCQCServerBase::SetSrvAdminOId(const TOrbObjId& ooidToSet)
{
    m_ooidSrvAdmin = ooidToSet;
}


//
//  Called by CQCServer when new triggers have been configured. The admin client will
//  have saved them to the config server. CQCServer loads up that new config and
//  passes it to us. We let our config object copy over the triggers from it. Then
//  we find the field for each trigger and update it.
//
//  NOTE that it is possible that there are triggers for fields that don't exist anymore.
//  The client tries to remove them, but may have not been able to. If so, it'll try
//  again next time. But we need to be tolerant and skip any fields that we don't have.
//
tCIDLib::TVoid
TCQCServerBase::SetTriggers(const TCQCDriverObjCfg::TTriggerList& colToSet)
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    // Remove existing triggers, and then set the new set on us
    RemoveAllTriggers();
    m_cqcdcThis.SetTriggers(colToSet);

    //
    //  Now go through the passed list and find each field and set their passed in triggers
    //  on them.
    //
    if (!colToSet.bIsEmpty())
    {
        const tCIDLib::TCard4 c4Count = colToSet.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCFldEvTrigger& fetCur = colToSet[c4Index];

            TCQCFldStore* pcfsTar = pcfsFind(fetCur.strFldName(), kCIDLib::False);
            if (pcfsTar)
                pcfsTar->fetTrigger(fetCur);
        }
    }
}


//
//  CQCServer will call this after it loads a driver. It lets us make a copy
//  of his security credentials which will be used in any commands that the
//  driver might do that requires credentials. The name is purposefully a bit
//  misleading.
//
//  No need for locking here since it's called long before clients can access
//  the driver.
//
tCIDLib::TVoid TCQCServerBase::SetUC(const TCQCUserCtx& cuctxToSet)
{
    m_cuctxDrv = cuctxToSet;

    // Do the default loading of environmental vars from the host
    m_cuctxDrv.LoadEnvRTVsFromHost();
}


//
//  This is called to start the driver thread processing. This is typically
//  only called in CQCServer, where the driver runs in its own thread. In
//  the drive development IDEs, that may not be the case. They may run the
//  driver on a thread they create.
//
//  In this case, we create our own thread object.
//
tCIDLib::TVoid TCQCServerBase::StartPoll()
{
    if (!m_pthrDevPoll)
    {
        m_pthrDevPoll = new TThread
        (
            facCIDLib().strNextThreadName(TString(L"CQCDrv_") + strMoniker())
            , TMemberFunc<TCQCServerBase>(this, &TCQCServerBase::eDevPollThread)
        );

        //
        //  We are not on an external thread. Remember that because we have
        //  to clean up this thread object.
        //
        m_bOnExtThread = kCIDLib::False;
    }

    // If not running, then start it
    if (!m_pthrDevPoll->bIsRunning())
        m_pthrDevPoll->Start();
}


//
//  Do a non-blocking thread shutdown request on the driver thread. This
//  will set the thread's shutdown request flag and any looping code
//  should see it and exit. Our main loop will look for it as well and
//  start the shutdown process.
//
tCIDLib::TVoid TCQCServerBase::StartShutdown()
{
    //
    //  If the thread isn't stopped, then this must be getting called before
    //  the driver has even been started, so do nothing, but set the status
    //  to terminated, to insure we'll be seen as done. If we have a thread
    //  then do a non-blocking shutdown request here since this could be
    //  called with the CQCServer driver list locked if removing a single
    //  driver.
    //
    if (m_pthrDevPoll)
        m_pthrDevPoll->ReqShutdownNoSync();
    else
        m_eState = tCQCKit::EDrvStates::Terminated;
}


//
//  Streams out the value (or status) of the indicated field in a standard
//  format that is used to transmit the values to clients.
//
tCIDLib::TVoid
TCQCServerBase::StreamValue(const   tCIDLib::TCard4 c4FieldId
                            , const tCIDLib::TCard4 c4SerialNum
                            ,       TBinOutStream&  strmOut) const
{
    TLocker mtxSync(&m_mtxSync);

    CheckOnline(CID_FILE, CID_LINE);

    // Try to find this field by its id, and check for read access
    const TCQCFldStore* pcfsTarget = pcfsFind(c4FieldId);
    CheckAccess(*pcfsTarget, tCQCKit::EFldAccess::Read);

    //
    //  Check the serial number and if not changed, then stream out the
    //  'unchanged' indicator, and return false.
    //
    //  If the field is in error, then we just put out an error indicator,
    //  else we stream out the new data value.
    //
    if (pcfsTarget->bInError())
    {
        strmOut << kCQCKit::c1FldData_InError;
    }
     else if (c4SerialNum == pcfsTarget->c4SerialNum())
    {
        strmOut << kCQCKit::c1FldData_Unchanged;
    }
     else
    {
        //
        //  We have new data, so get the field as it's real type and add it to
        //  the results. Stream out the 'changed' indicator and serial number
        //  first, then ask the storage object to stream the actual value.
        //
        strmOut << kCQCKit::c1FldData_Changed
                << pcfsTarget->c4SerialNum();
        pcfsTarget->StreamOut(strmOut);
    }
}

tCIDLib::TVoid
TCQCServerBase::StreamValue(const   TString&        strName
                            ,       TBinOutStream&  strmOut) const
{
    TLocker mtxSync(&m_mtxSync);

    CheckOnline(CID_FILE, CID_LINE);

    // Try to find this field by its id, and check for read access
    const TCQCFldStore* pcfsTarget = pcfsFind(strName);
    CheckAccess(*pcfsTarget, tCQCKit::EFldAccess::Read);

    //
    //  In this one, we don't get a serial number, since it's used for one
    //  time queries by name almost all the time. So we always put out a
    //  changed marker and the value. We dont' return a serial number.
    //
    //  If the field is in error, then we just put out an error indicator.
    //
    if (pcfsTarget->bInError())
    {
        strmOut << kCQCKit::c1FldData_InError;
    }
     else
    {
        strmOut << kCQCKit::c1FldData_Changed;
        pcfsTarget->StreamOut(strmOut);
    }
}


//
//  This is a helper mostly for the CML driver. C++ drivers will do this themselves
//  directly in many cases. We'll throw if the passed field id is not found. If
//  the field isn't readable, we just ignore this since it has no meaning.
//
//  We find the passed field and tell it that the default value is ok as the
//  valid initial value.
//
//  This can be (and should be) called before we are connected, so we don't need
//  to check that.
//
tCIDLib::TVoid TCQCServerBase::UseDefFldValue(const tCIDLib::TCard4 c4FldId)
{
    TLocker mtxSync(&m_mtxSync);
    TCQCFldStore* pcfsTarget = pcfsFind(c4FldId);

    if (pcfsTarget->flddInfo().bIsReadable())
        pcfsTarget->fvThis().UseDefValue();
}


// ---------------------------------------------------------------------------
//  TCQCServerBase: Hidden constructor
// ---------------------------------------------------------------------------

//
//  This will only be used in 'simulator' mode, i.e. when in the CML/PDL driver
//  development test harnesses. The moniker and driver config won't be provided
//  until later when the macro or protocol level driver class is compiled and
//  kicked off.
//
//  NOTE that CQCServer always currently calls the one that takes the config
//  info. This is important because it will hold our persistent pause/resume flag.
//  Otherwise, we wouldn't get the config until we are initialized and by that
//  point we are running. So, if we were supposed to still be paused, we would
//  have ignored that setting.
//
//  The C++ driver harness also calls the second one since it knows what driver it
//  is running before it starts it.
//
TCQCServerBase::TCQCServerBase() :

    m_ac4LoopTimes(0UL)
    , m_bSimMode(kCIDLib::False)
    , m_colCmdQ(tCIDLib::EAdoptOpts::NoAdopt, tCIDLib::EMTStates::Safe)
    , m_colCmdQSpec(tCIDLib::EAdoptOpts::NoAdopt, tCIDLib::EMTStates::Safe)
    , m_colFldIdList(tCIDLib::EAdoptOpts::NoAdopt, kCQCKit::c4MaxDrvFields)
    , m_colFldNameList
      (
        tCIDLib::EAdoptOpts::Adopt, 109, TStringKeyOps(), &TCQCFldStore::strKey
      )
    , m_colTmp(32)
    , m_mbufTmp(4096)
    , m_strmFmt(1024UL)
{
    CommonInit(kCIDLib::True);
}

TCQCServerBase::TCQCServerBase(const TCQCDriverObjCfg& cqcdcInfo) :

    m_ac4LoopTimes(0UL)
    , m_bSimMode(kCIDLib::False)
    , m_colCmdQ(tCIDLib::EAdoptOpts::NoAdopt, tCIDLib::EMTStates::Safe)
    , m_colCmdQSpec(tCIDLib::EAdoptOpts::NoAdopt, tCIDLib::EMTStates::Safe)
    , m_colFldIdList(tCIDLib::EAdoptOpts::NoAdopt, kCQCKit::c4MaxDrvFields)
    , m_colFldNameList
      (
        tCIDLib::EAdoptOpts::Adopt, 109, TStringKeyOps(), &TCQCFldStore::strKey
      )
    , m_colTmp(32)
    , m_mbufTmp(4096UL)
    , m_cqcdcThis(cqcdcInfo)
    , m_strmFmt(1024UL)
{
    CommonInit(kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TCQCServerBase: Protected, virtual methods
// ---------------------------------------------------------------------------

//
//  The second generation driver architecture allows for probing for devices
//  in order to support automatic location of what port a device is on. Not
//  all drivers support this, so it's optional and we provide a default
//  implementation that just says we found nothing. But it shouldn't even
//  get called if the driver doesn't indicate it's capable of this type of
//  thing.
//
tCIDLib::TBoolean TCQCServerBase::bProbeDevice(const TCQCDriverObjCfg&)
{
    return kCIDLib::False;
}


//
//  A helper method for derived drivers to call to see if they should log
//  an exception. If it's not been logged and the current driver verbosity
//  level is at or beyond the indicated minimum level, then it returns true.
//
//  It's virtual because some drivers may want to override it, such as when
//  CML or PDL drivers are run inside their respective IDEs.
//
tCIDLib::TBoolean
TCQCServerBase::bShouldLog( const   TError&                 errToCatch
                            , const tCQCKit::EVerboseLvls   eMinLevel) const
{
    // If already logged, then no need to do it
    return (!errToCatch.bLogged() && (m_eVerboseLevel >= eMinLevel));
}


//
//  These we don't make pure, since some drivers have no writeable fields,
//  and we want to make sure that the appopriate instrumentation field gets
//  bumped if they don't handle any that they should.
//
tCQCKit::ECommResults
TCQCServerBase::eBoolFldValChanged( const   TString&
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TBoolean)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::Unhandled;
}

tCQCKit::ECommResults
TCQCServerBase::eCardFldValChanged(const   TString&
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TCard4)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::Unhandled;
}

tCQCKit::ECommResults
TCQCServerBase::eFloatFldValChanged(const   TString&
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TFloat8)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::Unhandled;
}

tCQCKit::ECommResults
TCQCServerBase::eIntFldValChanged(const   TString&
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TInt4)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::Unhandled;
}

tCQCKit::ECommResults
TCQCServerBase::eStringFldValChanged(const   TString&
                                    , const tCIDLib::TCard4
                                    , const TString&)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::Unhandled;
}

tCQCKit::ECommResults
TCQCServerBase::eSListFldValChanged(const   TString&
                                    , const tCIDLib::TCard4
                                    , const TVector<TString>&)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::Unhandled;
}

tCQCKit::ECommResults
TCQCServerBase::eTimeFldValChanged(const   TString&
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TCard8&)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::Unhandled;
}



// Most drivers don't handle this. It's mostly for the generic driver
tCIDLib::TVoid TCQCServerBase::Idle()
{
}


//
//  Helper methods for derived drivers to log errors if they have not
//  already been logged and the current verbosity level is at or beyond
//  the indicated minimum level.
//
//  We have one that will add the caller's info to the stack trace of the
//  error object, which is typically desired.
//
tCIDLib::TVoid
TCQCServerBase::LogError(const  TError&                 errToCatch
                        , const tCQCKit::EVerboseLvls   eMinLevel) const
{
    if (!errToCatch.bLogged() && (m_eVerboseLevel >= eMinLevel))
        TModule::LogEventObj(errToCatch);
}

tCIDLib::TVoid
TCQCServerBase::LogError(       TError&                 errToCatch
                        , const tCQCKit::EVerboseLvls   eMinLevel
                        , const tCIDLib::TCh* const     pszSrcFile
                        , const tCIDLib::TCard4         c4SrcLine) const
{
    if (!errToCatch.bLogged() && (m_eVerboseLevel >= eMinLevel))
    {
        errToCatch.AddStackLevel(pszSrcFile, c4SrcLine);
        TModule::LogEventObj(errToCatch);
    }
}


//
//  Allows derived classes to react to a verbose mode change. Most don't
//  care and just look at the mode flag when they need to, so we provide
//  a default empty implementation.
//
tCIDLib::TVoid TCQCServerBase::VerboseModeChanged(const tCQCKit::EVerboseLvls)
{
}



// ---------------------------------------------------------------------------
//  TCQCServerBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Return the value of a boolean field by id
tCIDLib::TBoolean TCQCServerBase::bFldValue(const tCIDLib::TCard4 c4Id) const
{
    TLocker mtxSync(&m_mtxSync);
    return pcfsGetBoolStore(c4Id, tCQCKit::EFldAccess::Read)->bValue();
}


//
//  Our own internal sleep method for drivers, so that we can call the
//  Idle callback periodically while the driver isn't doing anything else.
//
tCIDLib::TBoolean TCQCServerBase::bSleep(const tCIDLib::TCard4 c4Sleep)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;

    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(c4Sleep);
    tCIDLib::TEncodedTime enctNow = TTime::enctNow();

    // Loop until we run out of time or are asked to shutdown
    while ((enctNow < enctEnd)
    &&     !m_pthrDevPoll->bCheckShutdownRequest())
    {
        // Call the idle callback at least once
        {
            try
            {
                Idle();
            }

            catch(TError& errToCatch)
            {
                if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::High)
                {
                    if (facCQCKit().bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcDbg_ExceptInIdle
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strMoniker()
                    );
                }
            }

            catch(const TDbgExitException&)
            {
                m_pthrDevPoll->ReqShutdownNoSync();
            }

            catch(const TExceptException&)
            {
                //
                //  Gotta let this one pass, because it goes back to the IDE
                //  when in sim mode, who will handle it.
                //
                if (m_bSimMode)
                    throw;
            }

            catch(...)
            {
                if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::High)
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcDbg_ExceptInIdle
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strMoniker()
                    );
                }
            }
        }

        //
        //  We want to call Idle at least once, and it might take the
        //  time we have, so we don't check for sleep time left until
        //  we get down here. So update now first and see if we are done.
        //
        enctNow = TTime::enctNow();
        if (enctNow >= enctEnd)
            break;

        // Not done yet, so calculate the millis left
        tCIDLib::TCard4 c4Millis = tCIDLib::TCard4
        (
            (enctEnd - enctNow) / kCIDLib::enctOneMilliSec
        );

        // If not at least a small number of millis left, we are done
        if (c4Millis < 10)
            break;

        // Sleep up to 250ms of the millis left
        if (c4Millis > 250)
            c4Millis = 250;
        TThread::Sleep(c4Millis);

        // Update the now time again, for the top of loop check
        enctNow = TTime::enctNow();
    }
    return bRet;
}


// Return the current value of a card field by id
tCIDLib::TCard4 TCQCServerBase::c4FldValue(const tCIDLib::TCard4 c4Id) const
{
    TLocker mtxSync(&m_mtxSync);
    return pcfsGetCardStore(c4Id, tCQCKit::EFldAccess::Read)->c4Value();
}


// Return the current value of a time field by id
tCIDLib::TCard8 TCQCServerBase::c8TimeFldValue(const tCIDLib::TCard4 c4Id) const
{
    TLocker mtxSync(&m_mtxSync);
    return pcfsGetTimeStore(c4Id, tCQCKit::EFldAccess::Read)->c8Value();
}


//
//  In this case the driver must lock before calling and hold the lock until
//  he is done with the collection.
//
const TVector<TString>&
TCQCServerBase::colFldValue(const tCIDLib::TCard4 c4Id) const
{
    return pcfsGetSListStore(c4Id, tCQCKit::EFldAccess::Read)->colValue();
}


//
//  Check if we are online, throw if not. We check the current state to see
//  if it's connected or not.
//
tCIDLib::TVoid
TCQCServerBase::CheckOnline(const   TString&        strFile
                            , const tCIDLib::TCard4 c4Line) const
{
    if (m_eState != tCQCKit::EDrvStates::Connected)
    {
        facCQCKit().ThrowErr
        (
            strFile
            , c4Line
            , kKitErrs::errcDrv_NotOnline
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , strMoniker()
        );
    }
}


// Return the vlaue of a float field by id
tCIDLib::TFloat8 TCQCServerBase::f8FldValue(const tCIDLib::TCard4 c4Id) const
{
    TLocker mtxSync(&m_mtxSync);
    return pcfsGetFloatStore(c4Id, tCQCKit::EFldAccess::Read)->f8Value();
}


// Return the value of an int field by id
tCIDLib::TInt4 TCQCServerBase::i4FldValue(const tCIDLib::TCard4 c4Id) const
{
    TLocker mtxSync(&m_mtxSync);
    return pcfsGetIntStore(c4Id, tCQCKit::EFldAccess::Read)->i4Value();
}


//
//  Helper methods for derived drivers to log on the fly high verbosity
//  messages, optionally with replacement tokens. The driver moniker is
//  automatically added to the message. We have versions that don't take
//  a severity and just assume status level, and another set that takes
//  the severity.
//
tCIDLib::TVoid
TCQCServerBase::LogMsg( const   TString&                strMsg
                        , const tCQCKit::EVerboseLvls   eMinLevel
                        , const tCIDLib::TCh* const     pszSrcFile
                        , const tCIDLib::TCard4         c4SrcLine
                        , const MFormattable&           fmtblToken1
                        , const MFormattable&           fmtblToken2
                        , const MFormattable&           fmtblToken3
                        , const MFormattable&           fmtblToken4) const
{
    if (m_eVerboseLevel >= eMinLevel)
    {
        TString strToLog(L"(Driver=", 256UL);
        strToLog.Append(strMoniker());
        strToLog.Append(L") - ");
        strToLog.Append(strMsg);

        facCQCKit().LogMsg
        (
            pszSrcFile
            , c4SrcLine
            , strToLog
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , fmtblToken1
            , fmtblToken2
            , fmtblToken3
            , fmtblToken4
        );
    }
}

tCIDLib::TVoid
TCQCServerBase::LogMsg( const   TString&                strMsg
                        , const tCQCKit::EVerboseLvls   eMinLevel
                        , const tCIDLib::ESeverities    eSev
                        , const tCIDLib::TCh* const     pszSrcFile
                        , const tCIDLib::TCard4         c4SrcLine
                        , const MFormattable&           fmtblToken1
                        , const MFormattable&           fmtblToken2
                        , const MFormattable&           fmtblToken3
                        , const MFormattable&           fmtblToken4) const
{
    if (m_eVerboseLevel >= eMinLevel)
    {
        TString strToLog(L"(Driver=", 256UL);
        strToLog.Append(strMoniker());
        strToLog.Append(L") - ");
        strToLog.Append(strMsg);

        facCQCKit().LogMsg
        (
            pszSrcFile
            , c4SrcLine
            , strToLog
            , eSev
            , tCIDLib::EErrClasses::AppStatus
            , fmtblToken1
            , fmtblToken2
            , fmtblToken3
            , fmtblToken4
        );
    }
}

tCIDLib::TVoid
TCQCServerBase::LogMsg( const   tCIDLib::TCh* const     pszMsg
                        , const tCQCKit::EVerboseLvls   eMinLevel
                        , const tCIDLib::TCh* const     pszSrcFile
                        , const tCIDLib::TCard4         c4SrcLine
                        , const MFormattable&           fmtblToken1
                        , const MFormattable&           fmtblToken2
                        , const MFormattable&           fmtblToken3
                        , const MFormattable&           fmtblToken4) const
{
    if (m_eVerboseLevel >= eMinLevel)
    {
        TString strToLog(L"(Driver=", 256UL);
        strToLog.Append(strMoniker());
        strToLog.Append(L") - ");
        strToLog.Append(pszMsg);

        facCQCKit().LogMsg
        (
            pszSrcFile
            , c4SrcLine
            , strToLog
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , fmtblToken1
            , fmtblToken2
            , fmtblToken3
            , fmtblToken4
        );
    }
}

tCIDLib::TVoid
TCQCServerBase::LogMsg( const   tCIDLib::TCh* const     pszMsg
                        , const tCQCKit::EVerboseLvls   eMinLevel
                        , const tCIDLib::ESeverities    eSev
                        , const tCIDLib::TCh* const     pszSrcFile
                        , const tCIDLib::TCard4         c4SrcLine
                        , const MFormattable&           fmtblToken1
                        , const MFormattable&           fmtblToken2
                        , const MFormattable&           fmtblToken3
                        , const MFormattable&           fmtblToken4) const
{
    if (m_eVerboseLevel >= eMinLevel)
    {
        TString strToLog(L"(Driver=", 256UL);
        strToLog.Append(strMoniker());
        strToLog.Append(L") - ");
        strToLog.Append(pszMsg);

        facCQCKit().LogMsg
        (
            pszSrcFile
            , c4SrcLine
            , strToLog
            , eSev
            , tCIDLib::EErrClasses::AppStatus
            , fmtblToken1
            , fmtblToken2
            , fmtblToken3
            , fmtblToken4
        );
    }
}


// Get the value of a string field by id
TString TCQCServerBase::strFldVal(const tCIDLib::TCard4 c4Id) const
{
    return pcfsGetStringStore(c4Id, tCQCKit::EFldAccess::Read)->strValue();
}


//
//  Only called by the driver. We lock and update the config. Note that the
//  moniker cannot be changed. The only time it could be changed here is if
//  it was not previously set and this is the first setting of config on the
//  driver. This would never happen in a live CQCServer based environment,
//  so effectively the moniker cannot be changed once set, so we don't have
//  to worry about locking when exposing the copy of the moniker we store.
//
tCIDLib::TVoid TCQCServerBase::SetConfigObj(const TCQCDriverObjCfg& cqcdcInfo)
{
    //
    //  The moniker cannot be changed, so check that if we currently have a
    //  moniker.
    //
    if (!m_strMoniker.bIsEmpty() && (cqcdcInfo.strMoniker() != m_strMoniker))
    {
        facCQCDriver().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCDrvErrs::errcDrv_MonikerChanged
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::InitFailed
            , cqcdcInfo.strMoniker()
            , m_strMoniker
        );
    }

    // Looks ok, so lock and store the new info
    TLocker mtxSync(&m_mtxSync);
    m_cqcdcThis = cqcdcInfo;

    //
    //  Get out the async flags, so that we don't have to constantly lock the
    //  config to look at these flags.
    //
    m_bAsyncMode = cqcdcInfo.bAsyncMode();
    m_bAsyncFldOver = cqcdcInfo.bAsyncFldOver();

    //
    //  And we store a copy of the moniker, so that we can provide access to
    //  it to the public without locking issues. The config can change, but
    //  the moniker cannot, so this is safe and avoids a lot of annoying
    //  locking issues for this very commonly accessed info.
    //
    //  Only update it if it's not already set, since otherwise we could
    //  modify it while others are using it, and we know nothing has changed
    //  otherwise since we checked above.
    //
    if (m_strMoniker.bIsEmpty())
        m_strMoniker = cqcdcInfo.strMoniker();
}


//
//  The derived class will call us here during field initialization so that we
//  can set up the definitions of the fields and the storage supported by this
//  driver. They pass us a list of TCQCFldDef objects, and we use that to
//  create our list of TCQCFldStore derived objects.
//
//  If the derived class doesn't have a fixed set of fields, i.e. the set can
//  change according to user configuration, then this will be called other than
//  at startup.
//
tCIDLib::TVoid
TCQCServerBase::SetFields(          TSetFieldList&      colList
                            , const tCIDLib::TBoolean   bInternal)
{
    // We'll need the count of incoming fields a number of times below
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();

    // Make sure it's not too many (when the standard ones are included)
    const tCIDLib::TCard4 c4StdFldsCnt(tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Count));
    if (colList.c4ElemCount() + c4StdFldsCnt > kCQCKit::c4MaxDrvFields)
    {
        facCQCDriver().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCDrvErrs::errcDrv_MaxFields
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
            , TCardinal(kCQCKit::c4MaxDrvFields - c4StdFldsCnt)
        );
    }

    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    //
    //  If more than the base fields have been registered, then remove those,
    //  leaving the base fields (which are the ones we add on their behalf.)
    //
    TString strCurName;
    const tCIDLib::TCard4 c4At = c4StdFldsCnt;
    tCIDLib::TCard4 c4Removed = 0;
    while (m_colFldIdList.c4ElemCount() > c4At)
    {
        // Get the name of the current item before we toast it
        strCurName = m_colFldIdList[c4At]->flddInfo().strName();

        //
        //  Kill this item in the by id list. This guy is non-adopting, so
        //  it doesn't matter that we've not removed it from the by name list
        //  yet.
        //
        m_colFldIdList.RemoveAt(c4StdFldsCnt);

        //
        //  This will destroy it, so we had to get a copy of the name out
        //  above before we do this.
        //
        m_colFldNameList.bRemoveKey(strCurName);
        c4Removed++;
    }

    // If we removed any, then bump the field list id
    if (c4Removed)
        m_c4FieldListId++;

    // If no fields passed in, then we are done now
    if (!c4Count)
        return;

    //
    //  We'll use this to assign field ids as we go, bumping it for each one.
    //  Start it past the first magic field id.
    //
    tCIDLib::TCard4 c4CurId = m_colFldIdList.c4ElemCount();

    const TString& strOurMon = strMoniker();
    try
    {
        //
        //  Now iterate the source collection. For each one, create the
        //  appropriate type of storage object, giving it a copy of the field
        //  def of the field it will be the storage for. Add the object to
        //  the name list, and give the id list a copy of the pointer also.
        //
        //  We also set the field ids as we go, to insure that the ids of the
        //  fields are mappable directly into the 'by id' list for fast id
        //  lookup.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            //
            //  Get a copy of the field def object and set the id of our copy,
            //  which is the one we'll put into the object we create.
            //
            TCQCFldDef& flddCur = colList[c4Index];
            flddCur.c4Id(c4CurId);

            // Check up front for a dup
            const TString& strFldName = flddCur.strName();
            if (m_colFldNameList.bKeyExists(strFldName))
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcFld_DupFldName
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::Already
                    , strFldName
                );
                continue;
            }

            // Make sure it's a valid field name
            if (!facCQCKit().bIsValidFldName(strFldName, bInternal))
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcFld_FldInitFailed
                    , facCQCKit().strMsg(kKitErrs::errcFld_BadNameChars)
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::InitFailed
                    , strFldName
                    , strOurMon
                );
            }

            // Looks like we'll keep it (or fail), so bump the next id value
            c4CurId++;

            TCQCFldStore* pcfsNew = nullptr;
            try
            {
                //
                //  Create the field limits object for this field. The storage
                //  objects we create will adopt it, and clean it up even if they
                //  throw, so it's safe that it will get taken care of.
                //
                TCQCFldLimit* pfldlCur = TCQCFldLimit::pfldlMakeNew(flddCur);

                switch(flddCur.eType())
                {
                    case tCQCKit::EFldTypes::Boolean :
                        pcfsNew = new TCQCFldStoreBool
                        (
                            strOurMon
                            , flddCur
                            , dynamic_cast<TCQCFldBoolLimit*>(pfldlCur)
                        );
                        break;

                    case tCQCKit::EFldTypes::Card :
                        pcfsNew = new TCQCFldStoreCard
                        (
                            strOurMon
                            , flddCur
                            , dynamic_cast<TCQCFldCardLimit*>(pfldlCur)
                        );
                        break;

                    case tCQCKit::EFldTypes::Float :
                        pcfsNew = new TCQCFldStoreFloat
                        (
                            strOurMon
                            , flddCur
                            , dynamic_cast<TCQCFldFloatLimit*>(pfldlCur)
                        );
                        break;

                    case tCQCKit::EFldTypes::Int :
                        pcfsNew = new TCQCFldStoreInt
                        (
                            strOurMon
                            , flddCur
                            , dynamic_cast<TCQCFldIntLimit*>(pfldlCur)
                        );
                        break;

                    case tCQCKit::EFldTypes::String :
                        pcfsNew = new TCQCFldStoreString
                        (
                            strOurMon
                            , flddCur
                            , dynamic_cast<TCQCFldStrLimit*>(pfldlCur)
                        );
                        break;

                    case tCQCKit::EFldTypes::Time :
                        pcfsNew = new TCQCFldStoreTime
                        (
                            strOurMon
                            , flddCur
                            , dynamic_cast<TCQCFldTimeLimit*>(pfldlCur)
                        );
                        break;

                    case tCQCKit::EFldTypes::StringList :
                        pcfsNew = new TCQCFldStoreSList
                        (
                            strOurMon
                            , flddCur
                            , dynamic_cast<TCQCFldStrListLimit*>(pfldlCur)
                        );
                        break;

                    default :
                    {
                        // Clean up the limit object since it never got stored
                        delete pfldlCur;

                        #if CID_DEBUG_ON
                        facCQCKit().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kKitErrs::errcFld_UnknownType
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Internal
                            , TInteger(tCIDLib::i4EnumOrd(flddCur.eType()))
                        );
                        #endif
                    }
                };

                //
                //  See if there is a configured trigger for this field. If so, set
                //  it on the storage object.
                //
                try
                {
                    const TCQCFldEvTrigger* pfetFld = m_cqcdcThis.pfetFind(strFldName);
                    if (pfetFld)
                        pcfsNew->fetTrigger(*pfetFld);
                }

                catch(TError& errToCatch)
                {
                    delete pcfsNew;
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }

                // Now add the new storage object to our two lists
                m_colFldNameList.Add(pcfsNew);
                m_colFldIdList.Add(pcfsNew);

                // It's stored so make sure it's not referenced again
                pcfsNew = nullptr;
            }

            catch(TError& errToCatch)
            {
                if (!errToCatch.bLogged()
                &&  (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Low))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcFld_FldInitFailed
                    , errToCatch.strErrText()
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::InitFailed
                    , strFldName
                    , strOurMon
                );
            }

            catch(...)
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcFld_FldInitFailed
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::InitFailed
                    , strFldName
                    , strOurMon
                );
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        if (!errToCatch.bLogged() && (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Low))
            TModule::LogEventObj(errToCatch);

        // Bump the field list id and clear the list back to the standard ones
        m_c4FieldListId++;
        m_colFldNameList.RemoveAll();
        m_colFldIdList.RemoveAll();
        RegDefFields();

        // And now rethrow
        throw;
    }

    // Bump the field list id, to invalidate any current field info in clients
    m_c4FieldListId++;
}


tCIDLib::TVoid TCQCServerBase::SetState(const tCQCKit::EDrvStates eToSet)
{
    TLocker lockrSync(&m_mtxSync);
    m_eState = eToSet;
}




// ---------------------------------------------------------------------------
//  TCQCServerBase: Private, static data members
// ---------------------------------------------------------------------------
TSrvDrvCmdPool* TCQCServerBase::m_psplCmds = nullptr;



// ---------------------------------------------------------------------------
//  TCQCServerBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called from the main driver thread method, to see if any
//  special commands have showed up and process them. Most of these
//  will cause us to cycle the driver, or to at least stop it. We update
//  the thread state, which will become the new state upon our return.
//
//  We also watch for the shutdown request flag here. If we see that
//  we go through the same shutdown steps, but in this case we leave us
//  in the terminated state so that we'll be cleaned up.
//
tCIDLib::TBoolean
TCQCServerBase::bCheckSpecialCmds(tCQCKit::EDrvStates& ePrevState)
{
    //
    //  In this case, we only ever process one of these at a time, since
    //  they all cause a cycle or stop of the driver. So we can do one
    //  up front check.
    //
    const tCIDLib::TBoolean bShutdown = m_pthrDevPoll->bCheckShutdownRequest();

    //
    //  If the shutdown flag isn't set, then see if we have any commands
    //  to process. If shutting down, no need to do that since we are going
    //  to do a shutdown.
    //
    TDrvCmd* pdcmdNew = nullptr;
    if (!bShutdown)
    {
        while (kCIDLib::True)
        {
            // Check for a new command, with no wait
            pdcmdNew = m_colCmdQSpec.pobjGetNext(0, kCIDLib::False);

            // If we didn't get one, break out now
            if (!pdcmdNew)
                break;

            //
            //  Check for any special cases where we really don't
            //  want to honor the command. If so, then just release
            //  it now and zero the pointer, and try again.
            //
            //  1. If they ask to resume and we aren't paused
            //  2. If they ask to pause and we already are paused
            //
            if (((pdcmdNew->m_eCmd == EDrvCmds::Resume)
            &&   (m_eState != tCQCKit::EDrvStates::Paused))
            ||  ((pdcmdNew->m_eCmd == EDrvCmds::Pause)
            &&   (m_eState == tCQCKit::EDrvStates::Paused)))
            {
                // Indicate it's the driver releasing
                ReleaseCmd(pdcmdNew, kCIDLib::True);
                pdcmdNew = nullptr;
            }

            // Break out with this one
            break;
        }
    }

    // If we got a command or a shutdown request
    if (pdcmdNew || bShutdown)
    {
        //
        //  Note that we do not want to lock here, because these are calls
        //  into the derived class. We will update the driver state, but
        //  it's a basic value so we don't bother locking for that.
        //
        //  To be safe we want to release any command we got right now. So
        //  get the bit of info out and release it if we got one.
        //
        EDrvCmds eCmd = EDrvCmds::None;
        TCQCDriverObjCfg cqcdcReconfig;
        if (pdcmdNew)
        {
            eCmd = pdcmdNew->m_eCmd;

            // If a reconfig, get the new config data out
            if (eCmd == EDrvCmds::Reconfig)
                cqcdcReconfig = *pdcmdNew->m_pcqcdcNew;

            // Now we can release it
            ReleaseCmd(pdcmdNew, kCIDLib::True);
            pdcmdNew = nullptr;
        }

        //
        //  If we have a comm resource, then clean it up. We still
        //  the lock currently, so we can get cleaned up without
        //  anyone else getting in. By the time they get in, we
        //  will be offline and they'll be rejected.
        //
        try
        {
            if ((m_eState == tCQCKit::EDrvStates::WaitConnect)
            ||  (m_eState == tCQCKit::EDrvStates::Connected))
            {
                ReleaseCommResource();
            }
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(const TDbgExitException&)
        {
            m_pthrDevPoll->ReqShutdownNoSync();
        }

        catch(const TExceptException&)
        {
            //
            //  Gotta let this one pass if in sim mode. In that case we
            //  are not being run as a separate thread, but just called
            //  directly.
            //
            if (m_bSimMode)
                throw;
        }

        catch(...)
        {
        }

        // Fall back to wait comm res state now
        m_eState = tCQCKit::EDrvStates::WaitCommRes;

        try
        {
            //
            //  If the driver is anywhere between not loaded and
            //  terminated, then terminate.
            //
            if ((m_eState > tCQCKit::EDrvStates::NotLoaded)
            &&  (m_eState < tCQCKit::EDrvStates::Terminated))
            {
                TerminateImpl();
            }
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
        }

        // Fall back to not loaded state now
        m_eState = tCQCKit::EDrvStates::NotLoaded;

        //
        //  Now do the command, or shutdown request. Get a lock here since
        //  we are going to update driver state. We error out any queued
        //  commands, release the command we are processing, if any, and
        //  any timed field writes.
        //
        try
        {
            TLocker lockrSync(&m_mtxSync);

            if (eCmd != EDrvCmds::None)
            {
                if (eCmd == EDrvCmds::Reconfig)
                {
                    // Store our new config and reset to waiting for init
                    SetConfigObj(cqcdcReconfig);
                    ePrevState = tCQCKit::EDrvStates::NotLoaded;
                    m_eState = tCQCKit::EDrvStates::WaitInit;

                    // Bump the reconfigured counter field
                    TCQCFldStoreCard* pcfsReconf = pcfsGetCardStore
                    (
                        tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Reconfigs)
                        , tCQCKit::EFldAccess::None
                    );
                    pcfsReconf->Inc();
                }
                 else if (eCmd == EDrvCmds::Pause)
                {
                    ePrevState = tCQCKit::EDrvStates::Paused;
                    m_eState = tCQCKit::EDrvStates::Paused;
                }
                 else if (eCmd == EDrvCmds::Resume)
                {
                    ePrevState = tCQCKit::EDrvStates::NotLoaded;
                    m_eState = tCQCKit::EDrvStates::WaitInit;
                }

                // Reset the load time field if starting back up
                if ((eCmd == EDrvCmds::Reconfig) || (eCmd == EDrvCmds::Resume))
                {
                    TCQCFldStoreTime* pcfsFld = pcfsGetTimeStore
                    (
                        tCIDLib::c4EnumOrd(tCQCKit::EStdFields::LoadTime)
                        , tCQCKit::EFldAccess::None
                    );
                    if (pcfsFld)
                        pcfsFld->SetValue(TTime::enctNow());
                }
            }
             else
            {
                // It's a shutdown request, so terminate us
                ePrevState = tCQCKit::EDrvStates::Terminated;
                m_eState = tCQCKit::EDrvStates::Terminated;
            }

            // Clean up any timed field changess
            m_colTimedChanges.RemoveAll();

            //
            //  We don't need to honor any other currently queued commands.
            //  Release them all back to the pool and let any waiting client
            //  threads go. Set up an exception to return to the clients.
            //
            //  We return a different command based on whether this is a
            //  shutdown or not.
            //
            if (eCmd == EDrvCmds::None)
            {
                TError errRet
                (
                    facCQCKit().strName()
                    , CID_FILE
                    , CID_LINE
                    , kKitErrs::errcDrv_ShuttingDown
                    , facCQCKit().strMsg
                      (
                        kKitErrs::errcDrv_ShuttingDown, strMoniker()
                      )
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Shutdown
                );
                ReleaseQCmds(errRet);
            }
             else
            {
                TError errRet
                (
                    facCQCKit().strName()
                    , CID_FILE
                    , CID_LINE
                    , L"Driver was paused, resumed or reconfigured"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotReady
                );
                ReleaseQCmds(errRet);
            }
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }

    // Return true if the shutdown flag is set
    return bShutdown;
}


//
//  A small helper for the command queuing code. We look at the caller's
//  desired wait mode and the driver's policy, and indicate a simple boolean
//  wait/nowait flag, and the correct initial ref count for the command.
//
//  If they indicate they don't care, we return !async mode, which will be
//  true if they aren't in async mode, i.e. want to wait.
//
tCIDLib::TBoolean
TCQCServerBase::bDefCmdRefCnt(  const   tCQCKit::EDrvCmdWaits   eWait
                                ,       tCIDLib::TCard4&        c4DefRef) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;

    if (eWait == tCQCKit::EDrvCmdWaits::DontCare)
        bRet = !m_bAsyncMode;
    else if (eWait == tCQCKit::EDrvCmdWaits::Wait)
        bRet = kCIDLib::True;

    if (bRet)
        c4DefRef = 2;
    else
        c4DefRef = 1;

    return bRet;
}


//
//  A helper that all of the bStoreXXX() methods calls to do a set of otherwise
//  messy checks. We return true if the new value written needs to be reported to
//  the driver, else false. The reasons are:
//
//  1. If the value is from the driver itself, we never report it it back to the
//     driver.
//  2. Else, if the value changed, we always report it
//  3. Or, if the value hasn't changed, we call bShouldSendCurVal() to let the field
//     storage object tell us whether to send an unchanged value or not, such as
//     being marked 'write always'.
//
//  Note that the call to CheckValRes() should throw for anything other than OK or
//  Unchanged status. So if we get past that, we know we have one of those two
//  values.
//
tCIDLib::TBoolean
TCQCServerBase::bFldWriteChecks(        TCQCFldStore&           cfsCheck
                                , const tCIDLib::TCard4         c4Line
                                , const tCQCKit::EValResults    eRes
                                , const tCIDLib::TBoolean       bFromDrv
                                , const TString&                strName)
{
    //
    //  Check the validation result. We prefer to report to the client about it
    //  sending bad values before we complain about the driver being offline and
    //  such.
    //
    CheckValRes(eRes, strName, CID_LINE);
    CIDAssert
    (
        (eRes == tCQCKit::EValResults::OK) || (eRes == tCQCKit::EValResults::Unchanged)
        , L"Got unexpected field value result after call to CheckValRes()"
    );

    //
    //  If not from the driver, make sure that we are online at the driver level
    //  and the field is not in error state.
    //
    if (!bFromDrv)
    {
        CheckOnline(CID_FILE, c4Line);
        CheckErrState(cfsCheck);
    }

    //
    //  If it's a good, and new, value, then cancel any queued timed write. We don't
    //  do this if it's the same value being written, even though that might be
    //  passed to the driver for processing in some cases.
    //
    if (eRes == tCQCKit::EValResults::OK)
        CancelTimedWrite(strName);

    // If from the driver, we never report it back to the driver
    if (bFromDrv)
        return kCIDLib::False;

    //
    //  If it's a changed value, or the field says send it always, we return true. We
    //  know here that the result is either Ok or Unchanged, so if it's not OK, we
    //  call the check to see if unchanged values should be sent. If either is true
    //  we send it to the driver.
    //
    return ((eRes == tCQCKit::EValResults::OK) || cfsCheck.bShouldSendCurVal());
}


//
//  Our field read methods call this to make sure the indicated field is readable
//  or not. It might be called from other things if needed to check for read or write
//  access being available. If not, it throws.
//
//  This isn't called for field writes because the driver can legally do writes to
//  fields even if clients aren't allowed. So those do their own check.
//
tCIDLib::TVoid
TCQCServerBase::CheckAccess(const   TCQCFldStore&       cfsToCheck
                            , const tCQCKit::EFldAccess eAccess) const
{
    TLocker mtxSync(&m_mtxSync);

    const TCQCFldDef& flddToCheck = cfsToCheck.flddInfo();
    if (!tCIDLib::bAllBitsOn(flddToCheck.eAccess(), eAccess))
    {
        tCIDLib::TErrCode errcToThrow = 0;
        if (eAccess == tCQCKit::EFldAccess::Read)
            errcToThrow = kKitErrs::errcFld_FieldNotReadable;
        else
            errcToThrow = kKitErrs::errcFld_FieldNotWriteable;

        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , errcToThrow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , flddToCheck.strName()
            , strMoniker()
        );
    }
}


//
//  Checks the comm res value returned from a number of the callbacks. If
//  it's a 'value rejected' then an exception is thrown back to the client.
//  Else if the driver reports lost connection or lost comm res, we update
//  the driver's state.
//
tCIDLib::TVoid
TCQCServerBase::CheckCommRes(const  tCQCKit::ECommResults   eRes
                            , const TString&                strFldName)
{
    TLocker mtxSync(&m_mtxSync);

    if (eRes == tCQCKit::ECommResults::LostConnection)
        m_eState = tCQCKit::EDrvStates::WaitConnect;
    else if (eRes == tCQCKit::ECommResults::LostCommRes)
        m_eState = tCQCKit::EDrvStates::WaitCommRes;
    else if (eRes >= tCQCKit::ECommResults::ValueRejected)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_ValueRejected
            , tCQCKit::strXlatECommResults(eRes)
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker()
            , strFldName
            , tCQCKit::strAltXlatECommResults(eRes)
        );
    }
}


//
//  Checks to see if the passed field is in error state and throws if so.
//
//  Only the driver thread calls this, and will have locked in order to have
//  the field store to pass here.
//
tCIDLib::TVoid TCQCServerBase::CheckErrState(const TCQCFldStore& cfsToCheck) const
{
    if (cfsToCheck.bInError())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_FieldInErr
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , cfsToCheck.flddInfo().strName()
        );
    }
}


tCIDLib::TVoid
TCQCServerBase::CheckFldListId(const tCIDLib::TCard4 c4ToCheck) const
{
    // No need to lock. This is only called by the driver thread
    if (c4ToCheck != m_c4FieldListId)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadFldListId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutOfSync
            , strMoniker()
        );
    }
}



//
//  This is called from the driver thread. We check for timed field changes
//  to have expired. If the driver is not online, we just discard any that
//  would need ot be written now.
//
//  If we can do one, we only do one at a time. Any others will get caught
//  one at a time as this gets periodically called.
//
//  We have to lock while looking up the field write, but then we have to
//  unlock before we call the field write callback.
//
tCIDLib::TVoid TCQCServerBase::CheckTimedChanges()
{
    // Lock while we do this
    TLocker mtxSync(&m_mtxSync);

    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TCard4 c4Count = m_colTimedChanges.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Count)
    {
        const TCQCServerBase::TTimedFldChange& tfcCur = m_colTimedChanges[c4Index];
        if (tfcCur.m_enctWaitTill < enctCur)
        {
            //
            //  Get out the field and value and remove it and reduce the count.
            //  MAKE COPIES sine we are going to delete it.
            //
            TString strField = tfcCur.m_strField;
            TString strVal = tfcCur.m_strFinalValue;
            m_colTimedChanges.RemoveAt(c4Index);
            c4Count--;

            // And, if we are connected, get a field lock and write it
            if (m_eState == tCQCKit::EDrvStates::Connected)
            {
                try
                {
                    // If verbose, log that we are writing the final value
                    if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facCQCKit().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Writing final value to field %(1).%(2)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strMoniker()
                            , strField
                        );
                    }

                    //
                    //  The write by name always removes any queued field writes, but we've
                    //  already removed it ourself, so it won't find one in this case. Note
                    //  that we indicate this write is from the client, not from the driver,
                    //  so that those rules will be applied, since it technically is from the
                    //  client.
                    //
                    mtxSync.Release();
                    WriteFieldByName(strField, strVal, kCIDLib::False);
                }

                catch(TError& errToCatch)
                {
                    if (!errToCatch.bLogged()
                    &&  (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }
                }

                //
                //  We've done one, so break out
                //
                //  >>>NOTE:
                //
                //  We assume we don't have to reaquire the lock
                //  above after doing the field write, since we are going
                //  to break out and be done. If this becomes no longer
                //  true and we do more than one, we will need to get the
                //  lock again upon return or exception!
                //
                break;
            }
             else
            {
                if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Low)
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Final timed write to field %(1).%(2) was dropped, driver offline"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strMoniker()
                        , strField
                    );
                }
            }
        }
         else
        {
            // Not ready yet, so bump the index
            c4Index++;
        }
    }
}


tCIDLib::TVoid
TCQCServerBase::CheckTypeAccess(const   TCQCFldStore&       cfsToCheck
                                , const tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldAccess eAccess) const
{
    TLocker mtxSync(&m_mtxSync);

    const TCQCFldDef& flddToCheck = cfsToCheck.flddInfo();
    if (flddToCheck.eType() != eType)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_TypeMismatch
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , strMoniker()
            , flddToCheck.strName()
            , tCQCKit::strXlatEFldTypes(eType)
            , tCQCKit::strXlatEFldTypes(flddToCheck.eType())
        );
    }

    if (!tCIDLib::bAllBitsOn(flddToCheck.eAccess(), eAccess))
    {
        tCIDLib::TErrCode errcToThrow = 0;
        if (eAccess == tCQCKit::EFldAccess::Read)
            errcToThrow = kKitErrs::errcFld_FieldNotReadable;
        else
            errcToThrow = kKitErrs::errcFld_FieldNotWriteable;

        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , errcToThrow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , flddToCheck.strName()
            , strMoniker()
        );
    }
}


//
//  A helper that the field write handling methods call to check the the result of
//  trying to store the value, to avoid redundancy. No need to lock, though we are
//  accessing the moniker. This is only called by the driver thread, and so the
//  moniker can't change while we are in here.
//
//  We convert these to ValueRejected errors, so that we get a non-generic error
//  msg on the client side.
//
tCIDLib::TVoid
TCQCServerBase::CheckValRes(const   tCQCKit::EValResults    eRes
                            , const TString&                strName
                            , const tCIDLib::TCard4         c4Line) const
{
    if (eRes == tCQCKit::EValResults::Unconvertable)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , c4Line
            , kKitErrs::errcFld_CannotConvertVal
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strMoniker()
            , strName
        );
    }
     else if (eRes == tCQCKit::EValResults::LimitViolation)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , c4Line
            , kKitErrs::errcFld_LimitViolation
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strMoniker()
            , strName
        );
    }
}


//
//  We have a couple ctors and too much init code to want to replicate it
//  all. Some we have to do at the ctor, but the rest here.
//
tCIDLib::TVoid TCQCServerBase::CommonInit(const tCIDLib::TBoolean bSimMode)
{
    //
    //  Fault in the static command pool if not already. It has to be thread
    //  safe since it's not protected by the driver mutex. It's global to
    //  all drivers.
    //
    if (!m_psplCmds)
    {
        TLocker lockrSync(&m_mtxSync);
        if (!m_psplCmds)
            m_psplCmds = new TSrvDrvCmdPool();

        //
        //  Do an initial call to update stats that the facility class
        //  maintains for us. This will initialize it.
        //
        tCIDLib::TCard4 c4Used = 0, c4Free = 0;
        m_psplCmds->QueryListSizes(c4Used, c4Free);
        facCQCDriver().UpdatePoolItemsStat(c4Used, c4Free);
    }

    //
    //  Get out the async flags from the driver info. When this is called,
    //  from the ctor, locking isn't an issue. The config (if we got any
    //  in the ctor) will have already been stored.
    //
    m_bAsyncMode = m_cqcdcThis.bAsyncMode();
    m_bAsyncFldOver = m_cqcdcThis.bAsyncFldOver();

    //
    //  And we store a copy of the moniker, so that we can provide access to
    //  it to the public without locking issues. The config can change, but
    //  the moniker cannot, so this is safe and avoids a lot of annoying
    //  locking issues for this very commonly accessed info.
    //
    //  This could be empty if the default ctor is called. We'll update it
    //  later when the config is set. That never happens in real driver
    //  scenarios, where their factory methods always construct the driver
    //  object using the ctor that takes the config object.
    //
    m_strMoniker = m_cqcdcThis.strMoniker();

    // Based on the paused flag, we either start in wait init or paused state
    if (m_cqcdcThis.bPaused())
        m_eState = tCQCKit::EDrvStates::Paused;
    else
        m_eState = tCQCKit::EDrvStates::WaitInit;

    // Init the other simple stuff
    m_bSimMode          = bSimMode;
    m_bOnExtThread      = kCIDLib::False;
    m_c4FieldListId     = 1;
    m_eVerboseLevel     = tCQCKit::EVerboseLvls::Off;
    m_eVerboseNew       = tCQCKit::EVerboseLvls::Off;
    m_enctExtend        = 250 * kCIDLib::enctOneMilliSec;
    m_enctVerbose       = 0;
    m_tidThis           = 0;
    m_pthrDevPoll       = nullptr;

    // Set our temp formatting stream to use 5 decimal digits for floats
    m_strmFmt.c4Precision(5);

    //
    //  Register the default fields. This way, if paused, or if init fails, we
    //  will at least have these registered and we won't have to special case
    //  them.
    //
    RegDefFields();

    //
    //  Set up the default loop times list one per possible driver state.
    //  most of these aren't changeable but some are.
    //
    m_ac4LoopTimes[tCQCKit::EDrvStates::NotLoaded] = 1000;
    m_ac4LoopTimes[tCQCKit::EDrvStates::Paused] = 2000;
    m_ac4LoopTimes[tCQCKit::EDrvStates::WaitInit] = 10000;
    m_ac4LoopTimes[tCQCKit::EDrvStates::WaitConfig] = 5000;
    m_ac4LoopTimes[tCQCKit::EDrvStates::WaitCommRes] = 5000;
    m_ac4LoopTimes[tCQCKit::EDrvStates::WaitConnect] = 3000;
    m_ac4LoopTimes[tCQCKit::EDrvStates::Connected] = 1000;
    m_ac4LoopTimes[tCQCKit::EDrvStates::Terminated] = 1000;
}


//
//  This is where we queue up timed writes in response to calls from
//  clients. The caller must get a call and field access lock for this.
//  It's effectively the same as a field write call, with just the added
//  bit that we also queue up a deferred write for later.
//
tCIDLib::TVoid
TCQCServerBase::DoTimedWrite(const  TString&        strField
                            , const TString&        strNewValue
                            , const TString&        strEndValue
                            , const tCIDLib::TCard4 c4Seconds)
{
    //
    //  Make sure the seconds count is reasonable. We limit the eventual
    //  final write to 24 hours, and at least 10 seconds.
    //
    if ((c4Seconds < 10) || (c4Seconds > ((60 * 60) * 24)))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_TimedWrtInterval
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
    }

    if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Low)
    {
        TString strMsg(L"Writing initial timed fld write value for field ");
        strMsg.Append(strMoniker());
        strMsg.Append(kCIDLib::chPeriod);
        strMsg.Append(strField);
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , strMsg
            , strNewValue
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  Look up the field and do the initial field write. If we are offline,
    //  or the field is in error state, this will throw back the client and
    //  we'll not queue up anything.
    //
    //  If there is an outstanding timed write, this will remove it, so we
    //  won't end up with two of them. It doesn't matter that we pass true
    //  for the last value (which indicates a call from within the driver
    //  itself.)
    //
    //
    //  !!!!!
    //
    //  This one is tricky. The WriteField method will do any locking
    //  required and release it if it needs to call the driver's field
    //  change callback. By the time we get back here it will be
    //  released again.
    //
    //  WE CANNOT lock here before we call this, since that would leave
    //  us locked across a potential driver callback.
    //
    WriteFieldByName(strField, strNewValue, kCIDLib::False);

    //
    //  Now queue up a timed change for later. We need to lock now while
    //  we do this, since we couldn't do it above.
    //
    TLocker lockrSync(&m_mtxSync);

    //
    //  Make sure any previous timed field write is cancelled. The field write
    //  command above might do it, but only if the value written is different from
    //  the current value.
    //
    CancelTimedWrite(strField);

    // And now queue up a new one
    m_colTimedChanges.objPlace(strField, strEndValue, TTime::enctNowPlusSecs(c4Seconds));

    if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Low)
    {
        TString strMsg(L"Final timed fld write value queued up for field ");
        strMsg.Append(strMoniker());
        strMsg.Append(kCIDLib::chPeriod);
        strMsg.Append(strField);
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , strMsg
            , strEndValue
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This is a termporary thing. When the driver reports that he has connected, this
//  is called. We will run through the fields and see if any have not gotten an
//  initial value. If so, we'll collect the names of up to 8 of them and log those.
//
//  We only do this for readable fields, writeables aren't ever in error state.
//
tCIDLib::TVoid TCQCServerBase::LogBadInitVals()
{
    TLocker mtxSync(&m_mtxSync);

    tCIDLib::TCard4 c4Count = 0;
    TString strBadFlds;
    m_colFldIdList.bForEach
    (
        [&strBadFlds, &c4Count](const TCQCFldStore& cfsCur) -> tCIDLib::TBoolean
        {
            if (cfsCur.flddInfo().bIsReadable() && !cfsCur.fvThis().bGotFirstVal())
            {
                strBadFlds.Append(cfsCur.flddInfo().strName());
                strBadFlds.Append(kCIDLib::chSpace);
                c4Count++;
            }
            return (c4Count < 8);
        }
    );

    if (!strBadFlds.bIsEmpty())
    {
        facCQCDriver().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCDrvMsgs::midStatus_UnsetFields
            , strBadFlds
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}



//
//  If the driver thread sees that it's lost the connection to the device
//  and needs to deal with that. To avoid replicating this code in possibly
//  multiple places, we split it out here.
//
//  We know that we were at least past the point of having gotten the
//  comm resource. The caller passes us the previous state, and the current
//  state has been updated to the new status. So based on what the previous
//  and new states are, we back out things.
//
tCIDLib::TVoid
TCQCServerBase::LostConnection(const tCQCKit::EDrvStates ePrevState)
{
    // Shouldn't happen, but if no change, then do nothing
    if (m_eState == ePrevState)
        return;

    //
    //  Since we are no longer connected, flush the timed list, and the
    //  command queues are no longer required either, since we are
    //  recycling the connection and any queued commands are meaningless
    //  now.
    //
    {
        TLocker mtxSync(&m_mtxSync);
        m_colTimedChanges.RemoveAll();

        //
        //  Set up an error to return back to any waiting client threads
        //  and release all queued commands.
        //
        TError errRet
        (
            facCQCKit().strName()
            , CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_NotOnline
            , facCQCKit().strMsg(kKitErrs::errcDrv_NotOnline, strMoniker())
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
        );
        ReleaseQCmds(errRet);
    }

    //
    //  We had been connected and now we aren't, so log a message
    //  that we lost connection.
    //
    if ((ePrevState == tCQCKit::EDrvStates::Connected)
    &&  (m_eState < tCQCKit::EDrvStates::Connected))
    {
        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitMsgs::midStatus_DrvNowOffline
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::LostConnection
                , strMoniker()
            );
        }

        // Bump the lost connection counter
        {
            TLocker mtxSync(&m_mtxSync);
            TCQCFldStoreCard* pcfsLostConn = pcfsGetCardStore
            (
                tCIDLib::c4EnumOrd(tCQCKit::EStdFields::LostConn)
                , tCQCKit::EFldAccess::None
            );
            pcfsLostConn->Inc();
        }
    }

    //
    //  We had the comm resource before, and now we don't, so log a message
    //  and release the comm resource.
    //
    if (((ePrevState > tCQCKit::EDrvStates::WaitCommRes)
    &&   (ePrevState <= tCQCKit::EDrvStates::Connected))
    &&  (m_eState < tCQCKit::EDrvStates::WaitConnect))
    {
        // We lost the connection, so log a message
        if (m_eVerboseLevel >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitMsgs::midStatus_DrvLostCommRes
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::LostConnection
                , strMoniker()
            );
        }

        // Tell the derived class to release the resource
        ReleaseCommResource();

        // Bump the lost comm res counter
        {
            TLocker mtxSync(&m_mtxSync);
            TCQCFldStoreCard* pcfsLostComm = pcfsGetCardStore
            (
                tCIDLib::c4EnumOrd(tCQCKit::EStdFields::LostCommRes)
                , tCQCKit::EFldAccess::None
            );
            pcfsLostComm->Inc();
        }
    }
}


//
//  We provide public helper methods to queue up commands. According to
//  client request or driver policy, we may want to just replace an existing
//  command with new data already in the queue. So we provide these to
//  see if we have an equivalent command in the queue.
//
//  If we do, and the caller indicates that the new waiter is going to want
//  to wait on the command, then we bump the reference count, since there
//  will now be another owner of the object.
//
//  The public helpers will have locked the passed queue before they call
//  us here.
//
//  For field write type searchs, if the id is non-zero we assume that this
//  is a by id command, else that the name is to be compared.
//
TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdFindQCmd(          TCmdQ&              colSrcQ
                                , const EDrvCmds            eToFind
                                , const tCIDLib::TBoolean   bWaiting)
{
    // If the queue is empty, then we can just return null now
    if (colSrcQ.bIsEmpty())
        return nullptr;

    //
    //  See if already have one in the queue. If so, just update it, else
    //  add a new one.
    //
    TCmdQ::TNCCursor cursSearch(&colSrcQ);
    cursSearch.bReset();

    TCQCServerBase::TDrvCmd* pdcmdRet = nullptr;
    {
        do
        {
            TDrvCmd& dcmdCur = cursSearch.objWCur();
            if (dcmdCur.m_eCmd == eToFind)
            {
                // Bump the ref count if the new client is waiting
                if (bWaiting)
                    dcmdCur.m_c4RefCnt++;

                pdcmdRet = &dcmdCur;
                break;
            }
        }   while (cursSearch.bNext());
    }
    return pdcmdRet;
}


TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdFindQFldWrt(       TCmdQ&              colSrcQ
                                , const EDrvCmds            eToFind
                                , const tCIDLib::TCard4     c4FldId
                                , const TString&            strName
                                , const tCIDLib::TBoolean   bByName
                                , const tCIDLib::TBoolean   bWaiting)
{
    // If the queue is empty, then we can just return null now
    if (colSrcQ.bIsEmpty())
        return nullptr;

    // Search the queue for this command for this id
    TCmdQ::TNCCursor cursSearch(&colSrcQ);
    cursSearch.bReset();

    TCQCServerBase::TDrvCmd* pdcmdRet = nullptr;
    do
    {
        TDrvCmd& dcmdCur = cursSearch.objWCur();
        if (dcmdCur.m_eCmd == eToFind)
        {
            if ((bByName && (dcmdCur.m_strName == strName))
            ||  (c4FldId && (dcmdCur.m_c4Id1 == c4FldId)))
            {
                // Bump the ref count if the new client is waiting
                if (bWaiting)
                    dcmdCur.m_c4RefCnt++;

                pdcmdRet = &dcmdCur;
                break;
            }
        }
    }   while (cursSearch.bNext());

    return pdcmdRet;
}


//
//  A helper to do the grunt work of setting up a command for a field
//  write. It's the same for all of the field types, except for a little
//  stuff related to the actual value being set on the command. So this
//  avoids a lot of redundant code and aids maintenance.
//
//  We tell the caller if it we created a new one or not, so he knows
//  wehther to queue it (and only he can do that since he has to get the
//  outgoing value into it first.) We tell him if this is a wait mode
//  operation or not as well, so he knows whether to return the command
//  to the caller or not.
//
//  These always use the regular command queue, so they don't have to
//  pass it. The caller already locked it.
//
//  If appropriate we try to find a previously queued field write for the
//  same field an djust update it. This doesn't matter to the caller. If
//  we do so, and the caller is going to wait, we will update the ref count.
//
TCQCServerBase::TDrvCmd*
TCQCServerBase::pdcmdGetFldWrtCmd(  const   TString&                strName
                                    , const tCIDLib::TCard4         c4FldListId
                                    , const tCIDLib::TCard4         c4FldId
                                    , const tCQCKit::EDrvCmdWaits   eWait
                                    , const EDrvCmds                eNameCmd
                                    , const EDrvCmds                eIdCmd
                                    ,       tCIDLib::TBoolean&      bNew
                                    ,       tCIDLib::TBoolean&      bWait)
{
    TCQCServerBase::TDrvCmd* pdcmdRet = nullptr;

    // Decide if by name or by id and use the right command
    EDrvCmds eCmd = eNameCmd;
    if (strName.bIsEmpty())
    {
        CheckFldListId(c4FldListId);
        eCmd = eIdCmd;
    }


    //
    //  Decide if waiting or not and get the right init ref count if we
    //  we end up creating a new one.
    //
    tCIDLib::TCard4 c4InitRef;
    bWait = bDefCmdRefCnt(eWait, c4InitRef);

    //
    //  If we can replace, try that. If the driver policy says do it, then
    //  we always do. Or if the caller is not not waiting we always do.
    //
    if (m_bAsyncFldOver || !bWait)
    {
        pdcmdRet = pdcmdFindQFldWrt
        (
            m_colCmdQ, eCmd, c4FldId, strName, !strName.bIsEmpty(), bWait
        );
    }

    if (pdcmdRet)
    {
        // Not a new command
        bNew = kCIDLib::False;
    }
     else
    {
        pdcmdRet = pdcmdReserve(0, strMoniker());
        pdcmdRet->m_eCmd = eCmd;
        pdcmdRet->m_c4RefCnt = c4InitRef;

        if (strName.bIsEmpty())
        {
            pdcmdRet->m_c4Id1 = c4FldListId;
            pdcmdRet->m_c4Id2 = c4FldId;
        }
         else
        {
            pdcmdRet->m_strName = strName;
        }

        // A new command, so the caller should queue up
        bNew = kCIDLib::True;
    }

    return pdcmdRet;
}


//
//  Get the storage object as the base storage interface, via the field name
//  or id.
//
//  This is only called by the driver and it must lock before calling and keep
//  the lock while it accesses this data directly.
//
const TCQCFldStore*
TCQCServerBase::pcfsFind(const  TString&            strName
                        , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const TCQCFldStore* pcfsRet = m_colFldNameList.pobjFindByKey(strName, kCIDLib::False);
    if (!pcfsRet && bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_UnknownFldName
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker()
            , strName
        );
    }
    return pcfsRet;
}

TCQCFldStore*
TCQCServerBase::pcfsFind(const  TString&            strName
                        , const tCIDLib::TBoolean   bThrowIfNot)
{
    TCQCFldStore* pcfsRet = m_colFldNameList.pobjFindByKey(strName, kCIDLib::False);
    if (!pcfsRet && bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_UnknownFldName
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker()
            , strName
        );
    }
    return pcfsRet;
}


const TCQCFldStore*
TCQCServerBase::pcfsFind(const  tCIDLib::TCard4     c4FldId
                        , const tCIDLib::TBoolean   bThrowIfNot) const
{
    if (c4FldId >= m_colFldIdList.c4ElemCount())
    {
        if (bThrowIfNot)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownFldId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strMoniker()
                , TCardinal(c4FldId)
            );
        }
        return nullptr;
    }
    return m_colFldIdList[c4FldId];
}

TCQCFldStore*
TCQCServerBase::pcfsFind(const  tCIDLib::TCard4     c4FldId
                        , const tCIDLib::TBoolean   bThrowIfNot)
{
    if (c4FldId >= m_colFldIdList.c4ElemCount())
    {
        // We never found it
        if (bThrowIfNot)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownFldId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strMoniker()
                , TCardinal(c4FldId)
            );
        }
        return nullptr;
    }
    return m_colFldIdList[c4FldId];
}


//
//  These methods handle the grunt work of checking a passed field id for
//  legality, getting the associated storage object out for it, making sure
//  that it's of the indicated type, and if so, casting it for return.
//
//  We first get it as a generic object, which will also check that the
//  id is valid.
//
//  This is only called by the driver, who must lock before calling and keep
//  the lock while it accesses the returned value.
//
TCQCFldStoreBool*
TCQCServerBase::pcfsGetBoolStore(const  tCIDLib::TCard4     c4Id
                                , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Boolean, eAccess);
    return static_cast<TCQCFldStoreBool*>(pcfsRet);
}

TCQCFldStoreBool*
TCQCServerBase::pcfsGetBoolStore(const  TString&            strName
                                , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Boolean, eAccess);
    return static_cast<TCQCFldStoreBool*>(pcfsRet);
}

const TCQCFldStoreBool*
TCQCServerBase::pcfsGetBoolStore(const  tCIDLib::TCard4     c4Id
                                , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Boolean, eAccess);
    return static_cast<const TCQCFldStoreBool*>(pcfsRet);
}

const TCQCFldStoreBool*
TCQCServerBase::pcfsGetBoolStore(const  TString&            strName
                                , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Boolean, eAccess);
    return static_cast<const TCQCFldStoreBool*>(pcfsRet);
}


TCQCFldStoreCard*
TCQCServerBase::pcfsGetCardStore(const  tCIDLib::TCard4     c4Id
                                , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Card, eAccess);
    return static_cast<TCQCFldStoreCard*>(pcfsRet);
}

TCQCFldStoreCard*
TCQCServerBase::pcfsGetCardStore(const TString&             strName
                                , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Card, eAccess);
    return static_cast<TCQCFldStoreCard*>(pcfsRet);
}

const TCQCFldStoreCard*
TCQCServerBase::pcfsGetCardStore(const  tCIDLib::TCard4     c4Id
                                , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Card, eAccess);
    return static_cast<const TCQCFldStoreCard*>(pcfsRet);
}

const TCQCFldStoreCard*
TCQCServerBase::pcfsGetCardStore(const TString&             strName
                                , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Card, eAccess);
    return static_cast<const TCQCFldStoreCard*>(pcfsRet);
}


TCQCFldStoreFloat*
TCQCServerBase::pcfsGetFloatStore(  const   tCIDLib::TCard4     c4Id
                                    , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Float, eAccess);
    return static_cast<TCQCFldStoreFloat*>(pcfsRet);
}

TCQCFldStoreFloat*
TCQCServerBase::pcfsGetFloatStore(  const   TString&            strName
                                    , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Float, eAccess);
    return static_cast<TCQCFldStoreFloat*>(pcfsRet);
}

const TCQCFldStoreFloat*
TCQCServerBase::pcfsGetFloatStore(  const   tCIDLib::TCard4     c4Id
                                    , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Float, eAccess);
    return static_cast<const TCQCFldStoreFloat*>(pcfsRet);
}

const TCQCFldStoreFloat*
TCQCServerBase::pcfsGetFloatStore(  const   TString&            strName
                                    , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Float, eAccess);
    return static_cast<const TCQCFldStoreFloat*>(pcfsRet);
}



TCQCFldStoreInt*
TCQCServerBase::pcfsGetIntStore(const   tCIDLib::TCard4     c4Id
                                , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Int, eAccess);
    return static_cast<TCQCFldStoreInt*>(pcfsRet);
}

TCQCFldStoreInt*
TCQCServerBase::pcfsGetIntStore(const   TString&            strName
                                , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Int, eAccess);
    return static_cast<TCQCFldStoreInt*>(pcfsRet);
}

const TCQCFldStoreInt*
TCQCServerBase::pcfsGetIntStore(const   tCIDLib::TCard4     c4Id
                                , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Int, eAccess);
    return static_cast<const TCQCFldStoreInt*>(pcfsRet);
}

const TCQCFldStoreInt*
TCQCServerBase::pcfsGetIntStore(const   TString&            strName
                                , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Int, eAccess);
    return static_cast<const TCQCFldStoreInt*>(pcfsRet);
}


TCQCFldStoreString*
TCQCServerBase::pcfsGetStringStore( const   tCIDLib::TCard4     c4Id
                                    , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::String, eAccess);
    return static_cast<TCQCFldStoreString*>(pcfsRet);
}

TCQCFldStoreString*
TCQCServerBase::pcfsGetStringStore( const   TString&            strName
                                    , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::String, eAccess);
    return static_cast<TCQCFldStoreString*>(pcfsRet);
}

const TCQCFldStoreString*
TCQCServerBase::pcfsGetStringStore( const   tCIDLib::TCard4     c4Id
                                    , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::String, eAccess);
    return static_cast<const TCQCFldStoreString*>(pcfsRet);
}

const TCQCFldStoreString*
TCQCServerBase::pcfsGetStringStore( const   TString&            strName
                                    , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::String, eAccess);
    return static_cast<const TCQCFldStoreString*>(pcfsRet);
}


TCQCFldStoreSList*
TCQCServerBase::pcfsGetSListStore(  const   tCIDLib::TCard4     c4Id
                                    , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::StringList, eAccess);
    return static_cast<TCQCFldStoreSList*>(pcfsRet);
}

TCQCFldStoreSList*
TCQCServerBase::pcfsGetSListStore(  const   TString&            strName
                                    , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::StringList, eAccess);
    return static_cast<TCQCFldStoreSList*>(pcfsRet);
}

const TCQCFldStoreSList*
TCQCServerBase::pcfsGetSListStore(  const   tCIDLib::TCard4     c4Id
                                    , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::StringList, eAccess);
    return static_cast<const TCQCFldStoreSList*>(pcfsRet);
}

const TCQCFldStoreSList*
TCQCServerBase::pcfsGetSListStore(  const   TString&            strName
                                    , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::StringList, eAccess);
    return static_cast<const TCQCFldStoreSList*>(pcfsRet);
}


TCQCFldStoreTime*
TCQCServerBase::pcfsGetTimeStore(const  tCIDLib::TCard4     c4Id
                                , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Time, eAccess);
    return static_cast<TCQCFldStoreTime*>(pcfsRet);
}

TCQCFldStoreTime*
TCQCServerBase::pcfsGetTimeStore(const  TString&            strName
                                , const tCQCKit::EFldAccess eAccess)
{
    TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Time, eAccess);
    return static_cast<TCQCFldStoreTime*>(pcfsRet);
}

const TCQCFldStoreTime*
TCQCServerBase::pcfsGetTimeStore(const  tCIDLib::TCard4     c4Id
                                , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(c4Id);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Time, eAccess);
    return static_cast<const TCQCFldStoreTime*>(pcfsRet);
}

const TCQCFldStoreTime*
TCQCServerBase::pcfsGetTimeStore(const  TString&            strName
                                , const tCQCKit::EFldAccess eAccess) const
{
    const TCQCFldStore* pcfsRet = pcfsFind(strName);
    CheckTypeAccess(*pcfsRet, tCQCKit::EFldTypes::Time, eAccess);
    return static_cast<const TCQCFldStoreTime*>(pcfsRet);
}


//
//  Register us with the name server to make it visible to clients.
//
//  In order to allow for fast translation by clients of moniker to the CQCServer
//  that hosts it, we put in an 'alias' entry into the name server, that just maps
//  back to our admin interface.
//
//  We set the text description string to our host name, and we use the three extra
//  strings in the binding to store the make/model, the device category, the moniker,
//  and the list of device classes the driver implements (empty if a V1 driver.)
//
//  Note that we create a very tiny possible race condition here where the rebinder
//  gets the binding registered before we get the driver into the list, since this is
//  done first. But it's so unlikely that it's safer to make sure we can register for
//  rebinding before we set up the driver in the list.
//
tCIDLib::TVoid TCQCServerBase::NSRegistration()
{
    try
    {
        TString strBinding(TCQCSrvAdminClientProxy::strDrvScope);
        strBinding.Append(kCIDLib::chForwardSlash);
        strBinding.Append(m_strMoniker);

        //
        //  Format out the device classes, comma separated. If there are none,
        //  it's just empty and that implies a V1 driver.
        //
        TString strDevCls;
        const tCQCKit::TDevClassList& fcolDC = m_cqcdcThis.fcolDevClasses();
        const tCIDLib::TCard4 c4DCCnt = fcolDC.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DCCnt; c4Index++)
        {
            if (c4Index)
                strDevCls.Append(kCIDLib::chComma);
            strDevCls.Append(tCQCKit::strAltXlatEDevClasses(fcolDC[c4Index]));
        }

        facCIDOrbUC().RegRebindObj
        (
            m_ooidSrvAdmin
            , strBinding
            , TSysInfo::strIPHostName()
            , m_cqcdcThis.strMake()
            , m_cqcdcThis.strModel()
            , tCQCKit::strXlatEDevCats(m_cqcdcThis.eCategory())
            , strDevCls
        );
    }

    catch(TError& errToCatch)
    {
        if (facCQCDriver().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        facCQCDriver().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCDrvErrs::errcDrv_CantCreateDrvAlias
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , m_strMoniker
        );
    }

    catch(...)
    {
        facCQCDriver().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCDrvErrs::errcDrv_CantCreateDrvAlias
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , m_strMoniker
        );
    }
}


//
//  When in connected mode, instead of just doing the usual inter-poll
//  callback sleep interval, this is called so that we can watch for cmds
//  to process.
//
//  We watch for one of the commands having changed the driver state. If
//  we are no longer in connected mode, we do the lost connection stuff
//  and return.
//
//  We also watch for the shutdown flag here, so that we'll break out as
//  soon as possible when it's seen. We go back and the main thread loop
//  will see it and shut us down.
//
tCIDLib::TVoid TCQCServerBase::ProcessCmds(const tCIDLib::TCard4 c4WaitMSs)
{
    //
    //  We will process commands up until the indicated time or we see the
    //  shutdown flag. We may well go over if a command takes a while.
    //
    tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctNow + (c4WaitMSs * kCIDLib::enctOneMilliSec);

    //
    //  Calculate an extended end time we'll use if needed. It'll be 1/8th
    //  of the wait time. We just shift down by three to divide by 8. It's
    //  not const. Once we'ev extended once, we'll zero it and that'll serve
    //  as the 'already extended' flag.
    //
    tCIDLib::TEncodedTime enctExtEnd
    (
        enctNow + ((c4WaitMSs >> 3) * kCIDLib::enctOneMilliSec)
    );


    //
    //  While we haven't been asked to stop, loop. We'll do the out of time
    //  check at the bottom because it'll allow us to deal with extending
    //  the timeout if there are still commands to process. That means we'll
    //  always at least enter (unles we were already asked to stop.) But
    //  the wait time can never be less than the minimum allowed driver
    //  poll time anyway, so it can't be zero.
    //
    while (!m_pthrDevPoll->bCheckShutdownRequest())
    {
        //
        //  Check the special commands queue each time. If something shows
        //  up there, break out and go back to the main loop to process it.
        //  It's going to whack us, so there's no point in processing any
        //  more queued messages now.
        //
        if (!m_colCmdQSpec.bIsEmpty())
            break;

        Idle();

        //
        //  Wait a bit for a command to show up. If a shutdown request
        //  happens, we'll wake up with no command. Wait up to 250ms for
        //  something to show up.
        //
        tCIDLib::TCard4 c4WaitCur = tCIDLib::TCard4
        (
            (enctEnd - enctNow) / kCIDLib::enctOneMilliSec
        );

        // If we didn't have even 1 ms left, just give up
        if (!c4WaitCur)
            break;

        // Clip to 250ms
        if (c4WaitCur > 250)
            c4WaitCur = 250;

        // And now wait that long for the new command
        TCQCServerBase::TDrvCmd* pdcmdNew = m_colCmdQ.pobjGetNext
        (
            c4WaitCur, kCIDLib::False
        );

        //
        //  If no command this time, go back to the top and try again. We
        //  have to update and check the time though, since we aren't checking
        //  it at the top.
        //
        if (!pdcmdNew)
        {
            enctNow = TTime::enctNow();
            if (enctNow > enctEnd)
                break;

            // Not done yet, go back
            continue;
        }

        //
        //  Put a janitor on the command to insure it's cleaned up. Indicate
        //  we want to do a 'from the driver' release.
        //
        TCQCSrvCmdJan janCmd(pdcmdNew, kCIDLib::True);

        // OK, let's process this command
        tCIDLib::TBoolean bRes = kCIDLib::False;
        tCIDLib::TCard4   c4Cnt = 0;
        try
        {
            switch(pdcmdNew->m_eCmd)
            {
                // Field write commands
                case EDrvCmds::WrtBoolById :
                    CheckFldListId(pdcmdNew->m_c4Id1);
                    bRes = bStoreBoolFld(pdcmdNew->m_c4Id2, pdcmdNew->m_bVal, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtBoolByName :
                    bRes = bStoreBoolFld(pdcmdNew->m_strName, pdcmdNew->m_bVal, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtCardById :
                    CheckFldListId(pdcmdNew->m_c4Id1);
                    bRes = bStoreCardFld(pdcmdNew->m_c4Id2, pdcmdNew->m_c4Val, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtCardByName :
                    bRes = bStoreCardFld(pdcmdNew->m_strName, pdcmdNew->m_c4Val, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtFieldById :
                    WriteField
                    (
                        pdcmdNew->m_c4Id1
                        , pdcmdNew->m_c4Id2
                        , pdcmdNew->m_strVal
                        , kCIDLib::False
                    );
                    break;

                case EDrvCmds::WrtFieldByName :
                    WriteFieldByName(pdcmdNew->m_strName, pdcmdNew->m_strVal, kCIDLib::False);
                    break;

                case EDrvCmds::WrtFloatById :
                    CheckFldListId(pdcmdNew->m_c4Id1);
                    bRes = bStoreFloatFld(pdcmdNew->m_c4Id2, pdcmdNew->m_f8Val, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtFloatByName :
                    bRes = bStoreFloatFld(pdcmdNew->m_strName, pdcmdNew->m_f8Val, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtIntById :
                    CheckFldListId(pdcmdNew->m_c4Id1);
                    bRes = bStoreIntFld(pdcmdNew->m_c4Id2, pdcmdNew->m_i4Val, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtIntByName :
                    bRes = bStoreIntFld(pdcmdNew->m_strName, pdcmdNew->m_i4Val, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtStrById :
                    CheckFldListId(pdcmdNew->m_c4Id1);
                    bRes = bStoreStringFld(pdcmdNew->m_c4Id2, pdcmdNew->m_strVal, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtStrByName :
                    bRes = bStoreStringFld(pdcmdNew->m_strName, pdcmdNew->m_strVal, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtStrListById :
                    CheckFldListId(pdcmdNew->m_c4Id1);
                    bRes = bStoreSListFld(pdcmdNew->m_c4Id2, pdcmdNew->m_colStrList, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtStrListByName :
                    bRes = bStoreSListFld(pdcmdNew->m_strName, pdcmdNew->m_colStrList, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtTimeById :
                    CheckFldListId(pdcmdNew->m_c4Id1);
                    bRes = bStoreTimeFld(pdcmdNew->m_c4Id2, pdcmdNew->m_enctVal, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::WrtTimeByName :
                    bRes = bStoreTimeFld(pdcmdNew->m_strName, pdcmdNew->m_enctVal, kCIDLib::False);
                    pdcmdNew->m_bVal = bRes;
                    break;


                // Specialized commands
                case EDrvCmds::TimedFldWrt :
                    DoTimedWrite
                    (
                        pdcmdNew->m_strName
                        , pdcmdNew->m_strVal
                        , pdcmdNew->m_strParams
                        , pdcmdNew->m_c4Count
                    );
                    break;



                // Backdoor commands
                case EDrvCmds::QueryBoolVal :
                    if (pdcmdNew->m_bVal)
                        bRes = bQueryVal2(pdcmdNew->m_strType, pdcmdNew->m_strName);
                    else
                        bRes = bQueryVal(pdcmdNew->m_strType);
                    pdcmdNew->m_bVal = bRes;
                    break;

                case EDrvCmds::QueryCardVal :
                    if (pdcmdNew->m_bVal)
                        pdcmdNew->m_c4Val = c4QueryVal2(pdcmdNew->m_strType, pdcmdNew->m_strName);
                    else
                        pdcmdNew->m_c4Val = c4QueryVal(pdcmdNew->m_strType);
                    break;

                case EDrvCmds::QueryConfig :
                    pdcmdNew->m_c4Count = c4QueryCfg(pdcmdNew->m_mbufVal);
                    break;

                case EDrvCmds::QueryData :
                    if (pdcmdNew->m_bVal)
                    {
                        // There's incoming data on this one
                        c4Cnt = pdcmdNew->m_c4Count;
                        bRes = bQueryData2(pdcmdNew->m_strType, c4Cnt, pdcmdNew->m_mbufVal);
                    }
                     else
                    {
                        c4Cnt = 0;
                        bRes = bQueryData(pdcmdNew->m_strType, pdcmdNew->m_strName, c4Cnt, pdcmdNew->m_mbufVal);
                    }
                    pdcmdNew->m_bVal = bRes;
                    pdcmdNew->m_c4Count = c4Cnt;
                    break;

                case EDrvCmds::QueryIntVal :
                    if (pdcmdNew->m_bVal)
                        pdcmdNew->m_i4Val = i4QueryVal2(pdcmdNew->m_strType, pdcmdNew->m_strName);
                    else
                        pdcmdNew->m_i4Val = i4QueryVal(pdcmdNew->m_strType);
                    break;

                case EDrvCmds::QueryTextVal :
                    pdcmdNew->m_bVal = bQueryTextVal
                    (
                        pdcmdNew->m_strType
                        , pdcmdNew->m_strName
                        , pdcmdNew->m_strVal
                    );
                    break;

                case EDrvCmds::SendCmd :
                    pdcmdNew->m_c4Val = c4SendCmd(pdcmdNew->m_strType, pdcmdNew->m_strParams);
                    break;

                case EDrvCmds::SendData :
                    pdcmdNew->m_bVal = bSendData
                    (
                        pdcmdNew->m_strType
                        , pdcmdNew->m_strName
                        , pdcmdNew->m_c4Count
                        , pdcmdNew->m_mbufVal
                    );
                    break;

                case EDrvCmds::SetConfig :
                    SetConfig(pdcmdNew->m_c4Count, pdcmdNew->m_mbufVal);
                    break;

                default :
                {
                    // Return an unknown command error
                    TError errRet
                    (
                        facCQCKit().strName()
                        , CID_FILE
                        , CID_LINE
                        , L"The command was not understood by the driver"
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Unknown
                    );
                    pdcmdNew->SetError(errRet);
                    break;
                }
            };
        }


        catch(TError& errToCatch)
        {
            // Set up the command for an error return
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
                TModule::LogEventObj(errToCatch);
            pdcmdNew->SetError(errToCatch);
        }

        catch(const TDbgExitException&)
        {
            m_pthrDevPoll->ReqShutdownNoSync();
        }

        catch(const TExceptException&)
        {
            //
            //  Gotta let this one pass if in sim mode. In that case we
            //  are not being run as a separate thread, but just called
            //  directly.
            //
            if (m_bSimMode)
                throw;
        }

        catch(...)
        {
            // Set up the command for an unknown exception error return
            TError errRet
            (
                facCQCKit().strName()
                , CID_FILE
                , CID_LINE
                , L"An unknown exception occurred while processing the command"
                , strMoniker()
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
            );
            pdcmdNew->SetError(errRet);
        }

        //
        //  If we have lost our connection, then clean up and return.
        //  This will error out any currently queued up commands. Pass
        //  in connected as the previous state, since we have to be
        //  connected in order to into this method.
        //
        if (m_eState != tCQCKit::EDrvStates::Connected)
        {
            LostConnection(tCQCKit::EDrvStates::Connected);
            break;
        }

        // Update our time for another round
        enctNow = TTime::enctNow();

        //
        //  If we are over. See if there are still commands to process. If
        //  we are not too far over yet, and haven't already done so, extend
        //  the end time a bit. We calculated what our extended end time
        //  will be up at the top. If it's not zero, then we've not extended
        //  yet.
        //
        if (enctNow > enctEnd)
        {
            // If no more commands, or we already extended, we are done
            if (!enctExtEnd || m_colCmdQ.bIsEmpty())
                break;

            // Store the extended time and zero it
            enctEnd = enctExtEnd;
            enctExtEnd = 0;

            // If we are still over, we are done
            if (enctNow > enctExtEnd)
                break;

            // Else let's go back to the top
        }
    }
}


//
//  This is called by our public helper methods to queue up command
//  objectss for later processing.
//
//  NOTE:
//
//  These are all queued up by public helper methods. These will check for
//  any opportunity to just update an existing queued command and hook up
//  the client thread to the same command object instead of queueing a new
//  one, if that is appropriate. If they do create a new one they call
//  here to queue it.
//
//  We do a couple checks and may error out the command and release it
//  without even queueing it. If we do release them, we indicate the release
//  is from the driver, so that the client will be woken up immediately
//  upon return and sned the error back.
//
//  The caller doesn't have to lock the queue or the driver to call
//  this. The target queue is thread safe, and releasing commands only
//  has to update the command, nothing to do with the driver or the
//  target queue.
//
//  However the caller may have a lock if he needed to search for a previous
//  instance of a command to replace, but it's not required to just queue
//  up a new command.
//
tCIDLib::TVoid
TCQCServerBase::QueueCmd(TCmdQ& colTarQ, TDrvCmd* const pdcmdToAdopt)
{
    // Should happen but just in case
    if (!pdcmdToAdopt)
    {
        facCQCDriver().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCDrvErrs::errcDrv_NullCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
            , strMoniker()
        );

        // Won't get here, just making the analyzer happy
        return;
    }

    // Reject if the queue is full
    if (colTarQ.bIsFull(CQCDriver_DriverBase::c4MaxQSz))
    {
        TError errRet
        (
            facCQCKit().strName()
            , CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_CmdQFull
            , facCQCKit().strMsg(kKitErrs::errcDrv_CmdQFull, strMoniker())
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
        );

        pdcmdToAdopt->SetError(errRet);
        ReleaseCmd(pdcmdToAdopt, kCIDLib::True);
        return;
    }

    //
    //  If we aren't in connected state, reject it unless it's one of the
    //  special ones.
    //
    if ((m_eState != tCQCKit::EDrvStates::Connected)
    &&  !pdcmdToAdopt->bIsSpecial())
    {
        TError errRet
        (
            facCQCKit().strName()
            , CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_NotOnline
            , facCQCKit().strMsg(kKitErrs::errcDrv_NotOnline, strMoniker())
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
        );

        pdcmdToAdopt->SetError(errRet);
        ReleaseCmd(pdcmdToAdopt, kCIDLib::True);
        return;
    }

    // Looks like we want to keep it, so queue it up
    colTarQ.Add(pdcmdToAdopt);
}



//
//  We add magic fields that all drivers will have, without having to
//  register them. These are 'instrumentation data' fields, which are
//  used to keep up with counts of things. We store the ids of each one
//  as we register it, so that we can get to it quickly at runtime.
//
//  NOTE:   There is an enum, tCQCKit::EStdFields, that provides the
//          ids that we want these fields to have. So we have to add
//          them into the collection in that order!
//
//  These are added during init, so that that they are there before the
//  derived driver actually loads his field list, since some of them
//  must connect to the device to get the field list, and therefore they
//  will want to bump counters if they get failures when doing that work.
//
//  If the field count equals the number of default fields, then we assume
//  that they are already registered and no others are, and don't wipe
//  them out by doing them again.
//
//  This only called when client threads cannot be accessing the driver,
//  since we aren't connected yet. So no need to lock.
//
tCIDLib::TVoid TCQCServerBase::RegDefFields()
{
    if (m_colFldIdList.c4ElemCount() != tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Count))
    {
        TString strLims(L"Enum: ");
        tCQCKit::EVerboseLvls eVerbosity = tCQCKit::EVerboseLvls::Min;
        while (eVerbosity < tCQCKit::EVerboseLvls::Max)
        {
            if (eVerbosity > tCQCKit::EVerboseLvls::Min)
                strLims.Append(kCIDLib::chComma);

            strLims.Append(tCQCKit::strXlatEVerboseLvls(eVerbosity));
            eVerbosity++;
        }

        const tCIDLib::TCard4 c4Count = tCIDLib::TCard4(tCQCKit::EStdFields::Count);
        TVector<TCQCFldDef> colFlds(c4Count);
        const tCQCKit::EFldAccess eStdAcc = tCQCKit::EFldAccess::Read;
        tCQCKit::EStdFields eCur = tCQCKit::EStdFields::Min;
        while (eCur <= tCQCKit::EStdFields::Max)
        {
            tCQCKit::EFldTypes eType = tCQCKit::EFldTypes::Card;
            if (eCur == tCQCKit::EStdFields::LoadTime)
                eType = tCQCKit::EFldTypes::Time;
            else if (eCur == tCQCKit::EStdFields::Verbosity)
                eType = tCQCKit::EFldTypes::String;

            const TString& strFldName = tCQCKit::strAltXlatEStdFields(eCur);
            if (eCur == tCQCKit::EStdFields::Verbosity)
                colFlds.objPlace(strFldName, eType, eStdAcc, strLims);
            else
                colFlds.objPlace(strFldName, eType, eStdAcc);

            eCur++;
        }

        SetFields(colFlds, kCIDLib::True);

        eCur = tCQCKit::EStdFields::Min;
        while (eCur <= tCQCKit::EStdFields::Max)
        {
            switch(eCur)
            {
                case tCQCKit::EStdFields::LoadTime :
                {
                    TCQCFldStoreTime* pcfsLoad = pcfsGetTimeStore
                    (
                        tCIDLib::c4EnumOrd(eCur), tCQCKit::EFldAccess::None
                    );
                    pcfsLoad->SetToNow();
                    break;
                }

                case tCQCKit::EStdFields::Verbosity :
                {
                    TCQCFldStoreString* pcfsVerbosity = pcfsGetStringStore
                    (
                        tCIDLib::c4EnumOrd(eCur), tCQCKit::EFldAccess::None
                    );
                    pcfsVerbosity->SetValue(tCQCKit::strXlatEVerboseLvls(m_eVerboseLevel));
                    break;
                }

                default :
                {
                    // The rest are Card and the default value is fine
                    TCQCFldStoreCard* pcfsCur = pcfsGetCardStore
                    (
                        tCIDLib::c4EnumOrd(eCur), tCQCKit::EFldAccess::None
                    );
                    pcfsCur->SetValue(0);
                    break;
                }
            };
            eCur++;
        }
    }
}


//
//  Any time we need to flush the command queue, this is called. We just
//  go through the whole queue, releasing each command. This lets any
//  waiting clients go as well. The caller provides an exception to put
//  in the commands, which will be returned to any waiting clients.
//
//  We do both the regular queue and the special commands queue.
//
//  To simplify things, we lock the driver while doing this. Though the
//  two queues are thread safe, this just makes it easer to insure that
//  new commands won't be getting queued up while we are doing this.
//
tCIDLib::TVoid TCQCServerBase::ReleaseQCmds(const TError& errRet)
{
    TLocker mtxSync(&m_mtxSync);

    while (kCIDLib::True)
    {
        try
        {
            TCQCServerBase::TDrvCmd* pdcmdCur = m_colCmdQ.pobjGetNext(0, kCIDLib::False);
            if (!pdcmdCur)
                break;

            pdcmdCur->SetError(errRet);
            ReleaseCmd(pdcmdCur, kCIDLib::True);
        }

        catch(...)
        {
        }
    }

    while (kCIDLib::True)
    {
        try
        {
            TCQCServerBase::TDrvCmd* pdcmdCur = m_colCmdQSpec.pobjGetNext(0, kCIDLib::False);
            if (!pdcmdCur)
                break;

            pdcmdCur->SetError(errRet);
            ReleaseCmd(pdcmdCur, kCIDLib::True);
        }

        catch(...)
        {
        }
    }

}


//
//  We have to lock initially, but release if we end up doing the
//  call back. This is only called if it actually changes.
//
tCIDLib::TVoid
TCQCServerBase::SetVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    // Lock while we update
    {
        TLocker mtxSync(&m_mtxSync);

        // Clear or reset the auto-reset time stamp
        if (eToSet == tCQCKit::EVerboseLvls::Off)
            m_enctVerbose = 0;
        else
            m_enctVerbose = TTime::enctNow() + (24 * kCIDLib::enctOneHour);

        // Store the new value now
        m_eVerboseLevel = eToSet;

        // Update our verbosity level field
        TCQCFldStoreString* pcfsVerbose = pcfsGetStringStore
        (
            tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Verbosity)
            , tCQCKit::EFldAccess::None
        );
        pcfsVerbose->SetValue(tCQCKit::strXlatEVerboseLvls(m_eVerboseLevel));
    }

    // Let the derived class know about it
    try
    {
        VerboseModeChanged(m_eVerboseLevel);
    }

    catch(TError& errToCatch)
    {
        if (facCQCKit().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    catch(...)
    {
    }
}


tCIDLib::TVoid TCQCServerBase::UnhandledFldId(const tCIDLib::TCard4 c4FldId)
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kKitErrs::errcFld_UnhandledFldId
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , TCardinal(c4FldId)
        , strMoniker()
    );
}


tCIDLib::TVoid TCQCServerBase::UnknownFldId(const tCIDLib::TCard4 c4FldId)
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kKitErrs::errcFld_UnknownFldId
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TCardinal(c4FldId)
    );
}



//
//  This handles a generic field write, to any type of field via a text formatted
//  value. In order to avoid making this into a LOT of redundant code, what we do
//  is just manage the conversion and then call the typed field write methods. This
//  way, the complicated code that is involved in field writes is kept at least
//  reasonably in bounds. We use a validation method available on the field storage
//  object, which will return a result and (if valid) the converted value. So we
//  can do the conversion cheaply, then just call the other version.
//
//  We have to lock up to the point where we are done with the sensitive data, but not
//  to the point where we would remain locked across the driver callback. So we release
//  the locker in each of the case statements. At that point we are just referencing data
//  we'lve stored in locals.
//
tCIDLib::TVoid
TCQCServerBase::WriteField( const   tCIDLib::TCard4     c4FieldListId
                            , const tCIDLib::TCard4     c4FldId
                            , const TString&            strValue
                            , const tCIDLib::TBoolean   bFromDriver)
{
    TLocker mtxSync(&m_mtxSync);

    //
    //  Make sure the field list hasn't changed since they got their ids. If it has
    //  not worthg bothering.
    //
    CheckFldListId(c4FieldListId);

    // Try to find this field's storage object, generically via the base class
    TCQCFldStore* pcfsTarget = pcfsFind(c4FldId);

    // HAS TO BE a copy since we use it after we release the mutex
    const TString strName = pcfsTarget->flddInfo().strName();

    // And now, based on the type, convert to binary and call the main storage methods

    tCQCKit::EValResults eRes = tCQCKit::EValResults::Count;
    switch(pcfsTarget->flddInfo().eType())
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            tCIDLib::TBoolean bTmp = kCIDLib::False;
            TCQCFldStoreBool* pcfsTar = static_cast<TCQCFldStoreBool*>(pcfsTarget);
            eRes = pcfsTar->eValidate(strValue, bTmp);

            mtxSync.Release();
            CheckValRes(eRes, strName, CID_LINE);
            bStoreBoolFld(c4FldId, bTmp, bFromDriver);
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            tCIDLib::TCard4 c4Tmp = 0;
            TCQCFldStoreCard* pcfsTar = static_cast<TCQCFldStoreCard*>(pcfsTarget);
            eRes = pcfsTar->eValidate(strValue, c4Tmp);

            mtxSync.Release();
            CheckValRes(eRes, strName, CID_LINE);
            bStoreCardFld(c4FldId, c4Tmp, bFromDriver);
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            tCIDLib::TFloat8 f8Tmp = 0;
            TCQCFldStoreFloat* pcfsTar = static_cast<TCQCFldStoreFloat*>(pcfsTarget);
            eRes = pcfsTar->eValidate(strValue, f8Tmp);

            mtxSync.Release();
            CheckValRes(eRes, strName, CID_LINE);
            bStoreFloatFld(c4FldId, f8Tmp, bFromDriver);
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            tCIDLib::TInt4 i4Tmp = 0;
            TCQCFldStoreInt* pcfsTar = static_cast<TCQCFldStoreInt*>(pcfsTarget);
            eRes = pcfsTar->eValidate(strValue, i4Tmp);

            mtxSync.Release();
            CheckValRes(eRes, strName, CID_LINE);
            bStoreIntFld(c4FldId, i4Tmp, bFromDriver);
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            // We use a member here to avoid having to alloc one every time
            TCQCFldStoreString* pcfsTar = static_cast<TCQCFldStoreString*>(pcfsTarget);
            eRes = pcfsTar->eValidate(strValue);

            mtxSync.Release();
            CheckValRes(eRes, strName, CID_LINE);
            bStoreStringFld(c4FldId, strValue, bFromDriver);
            break;
        }

        case tCQCKit::EFldTypes::StringList :
        {
            // We use a member here to avoid having to alloc one every time
            TCQCFldStoreSList* pcfsTar = static_cast<TCQCFldStoreSList*>(pcfsTarget);
            eRes = pcfsTar->eValidate(strValue, m_colTmp);

            mtxSync.Release();
            CheckValRes(eRes, strName, CID_LINE);
            bStoreSListFld(c4FldId, m_colTmp, bFromDriver);
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCFldStoreTime* pcfsTar = static_cast<TCQCFldStoreTime*>(pcfsTarget);
            tCIDLib::TCard8 c8Val;
            eRes = pcfsTar->eValidate(strValue, c8Val);

            mtxSync.Release();
            CheckValRes(eRes, strName, CID_LINE);
            bStoreTimeFld(c4FldId, c8Val, bFromDriver);
            break;
        }

        default :
        {
            #if CID_DEBUG_ON
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TInteger(tCIDLib::i4EnumOrd(pcfsTarget->flddInfo().eType()))
            );
            #endif
        }
    };
}


//
//  Just find the field id and call the other version to the work. We lock long enough to
//  get the info that we need to call the other version. It will also have to do some
//  locking as well, but we CANNOT leave it locked here, or it would remain locked across
//  a potential call into the driver.
//
tCIDLib::TVoid
TCQCServerBase::WriteFieldByName(const  TString&            strFldName
                                , const TString&            strValue
                                , const tCIDLib::TBoolean   bFromDriver)
{
    tCIDLib::TCard4 c4FldId = 0;
    tCIDLib::TCard4 c4ListId = 0;
    {
        TLocker mtxSync(&m_mtxSync);
        const TCQCFldDef* pflddTar = pflddFind(strFldName, kCIDLib::True);
        c4FldId = pflddTar->c4Id();
        c4ListId = m_c4FieldListId;
    }
    WriteField(c4ListId, c4FldId, strValue, bFromDriver);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCSrvCmdJan
//  PREFIX: jan
// ---------------------------------------------------------------------------
TCQCSrvCmdJan::TCQCSrvCmdJan(       TCQCServerBase::TDrvCmd* const  pdcmdWait
                            , const tCIDLib::TBoolean               bFromDriver) :

    m_bFromDriver(bFromDriver)
    , m_pdcmdWait(pdcmdWait)
{
}

TCQCSrvCmdJan::~TCQCSrvCmdJan()
{
    // Release if we have a command
    if (m_pdcmdWait)
        TCQCServerBase::ReleaseCmd(m_pdcmdWait, m_bFromDriver);
}

TCQCServerBase::TDrvCmd* TCQCSrvCmdJan::pdcmdWait()
{
    return m_pdcmdWait;
}

const TCQCServerBase::TDrvCmd* TCQCSrvCmdJan::pdcmdWait() const
{
    return m_pdcmdWait;
}


