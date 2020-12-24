//
// FILE NAME: ZWaveUSB3Sh_Unit.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  This is the implementation for the core Z-Wave unit class
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveUnit,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TZWaveUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveUnitInfo: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TZWaveUnit::eCompById(  const   TZWaveUnit&     unit1
                        , const TZWaveUnit&     unit2
                        , const tCIDLib::TCard4)
{
    if (unit1.c1Id() < unit2.c1Id())
        return tCIDLib::ESortComps::FirstLess;
    else if (unit1.c1Id() > unit2.c1Id())
        return tCIDLib::ESortComps::FirstLess;
    return tCIDLib::ESortComps::Equal;
}



// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveUnit::TZWaveUnit(TZWUnitInfo* const  punitiOwner) :

    m_colImpls(tCIDLib::EAdoptOpts::Adopt, 8)
    , m_punitiOwner(punitiOwner)
{
}

TZWaveUnit::~TZWaveUnit()
{
    //
    //  We own the impl objects, but explicitly delete them so that we can control
    //  any potential exception out of the dtor.
    //
    tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    while (c4Count)
    {
        try
        {
            delete m_colImpls.pobjOrphanAt(--c4Count);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to give us a chance to parse any extra info and store that data
//  away for subsequent use. This is the first thing called after the unit is
//  created since this is usually the most fundamental stuff.
//
//  Derived classes should call us first and return false if we return false, but
//  for now we have no extra info of our own.
//
tCIDLib::TBoolean TZWaveUnit::bParseExtraInfo()
{
    return kCIDLib::True;
}


//
//  The derived class should call us first, then do any prep. This usually involves
//  calling us to set some impl objects. We will clear the impl list and add any of
//  our own impl objects, then he can add any of his.
//
//  At our level, we will watch for some common command classes, such as battery
//  level, and add some CC impls of our own.
//
//  This is called after bParseExtraInfo() and QueryOptions(), so any extra info and
//  option driven info should be available.
//
tCIDLib::TBoolean TZWaveUnit::bPrepare()
{
    m_colImpls.RemoveAll();

    // If this unit supports the Battery class, then add a handler
    if (bSupportsClass(tZWaveUSB3Sh::ECClasses::Battery))
    {
        TString strFldName(strName());
        strFldName.Append(L"_Battery");
        tCIDLib::TCard4 c4ImplId;
        const tCIDLib::TBoolean bRes = bAddImplObject
        (
            new TZWCCImplBatteryF(m_punitiOwner, strFldName), c4ImplId
        );

        if (!bRes)
            return kCIDLib::False;
    }

    return kCIDLib::True;
}


// The derived class should call us, then if we return false check his own fields
tCIDLib::TBoolean TZWaveUnit::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If this guy handles it, we are done
        if (m_colImpls[c4Index]->bOwnsFld(c4FldId))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  The derived class should handle his fields first. If not one of his, he can
//  call us if he knows he installed any CC impl objects that have writable fields.
//  We will give all of those a chance to handle this.
//
tCIDLib::TBoolean
TZWaveUnit::bBoolFldChanged(const   TString&                strFldName
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TBoolean       bNewValue
                            ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colImpls[c4Index]->bBoolFldChanged(strFldName, c4FldId, bNewValue, eRes))
            {
                bRet = kCIDLib::True;
                break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}

tCIDLib::TBoolean
TZWaveUnit::bCardFldChanged(const   TString&                strFldName
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TCard4         c4NewVal
                            ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colImpls[c4Index]->bCardFldChanged(strFldName, c4FldId, c4NewVal, eRes))
            {
                bRet = kCIDLib::True;
                break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}

tCIDLib::TBoolean
TZWaveUnit::bFloatFldChanged(const  TString&                strFldName
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TFloat8        f8NewVal
                            ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colImpls[c4Index]->bFloatFldChanged(strFldName, c4FldId, f8NewVal, eRes))
            {
                bRet = kCIDLib::True;
                break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}

tCIDLib::TBoolean
TZWaveUnit::bIntFldChanged( const   TString&                strFldName
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TInt4          i4NewVal
                            ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colImpls[c4Index]->bIntFldChanged(strFldName, c4FldId, i4NewVal, eRes))
            {
                bRet = kCIDLib::True;
                break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}

tCIDLib::TBoolean
TZWaveUnit::bStringFldChanged(  const   TString&                strFldName
                                , const tCIDLib::TCard4         c4FldId
                                , const TString&                strNewVal
                                ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colImpls[c4Index]->bStringFldChanged(strFldName, c4FldId, strNewVal, eRes))
            {
                bRet = kCIDLib::True;
                break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


//
//  If the derived class has any CC impl objects installed on us, he must call us
//  to let us pass on this this them. They need to queue up async query messages
//  if they are readable and always listening.
//
//  Some special case ones will handle this themselves since they don't give us
//  the impl objects.
//
tCIDLib::TVoid TZWaveUnit::GetInitVals()
{
    const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWCCImpl* pzwcciCur = m_colImpls[c4Index];
        if (pzwcciCur->bReadAlways())
            pzwcciCur->SendInitValueQuery();
    }
}


//
//  If the derived class installed any CC impl objects on us, he must call us to let
//  us see if any of those are interested in this msg.
//
tCIDLib::TVoid
TZWaveUnit::HandleCCMsg(const   tCIDLib::TCard1     c1Class
                        , const tCIDLib::TCard1     c1Cmd
                        , const TZWInMsg&           zwimIn)
{
    const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWCCImpl* pzwcciCur = m_colImpls[c4Index];

        //
        //  If this msg was not end point encapsulated, or the end points of the two
        //  are the same, or this guy wants to see all end points, we pass it on.
        //
        if (!zwimIn.bWasEncapsulated()
        ||  (pzwcciCur->c1EPId() == zwimIn.c1SrcEPId())
        ||  pzwcciCur->bSeeAllEPs())
        {
            try
            {
                pzwcciCur->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                throw;
            }
        }
    }
}

//
//  When a CC impl enters error state this is called. There's no call for exiting
//  error state. That only happens when a valid value is stored and is reported via
//  ImplValueChanged().
//
tCIDLib::TVoid TZWaveUnit::ImplInErrState(const tCIDLib::TCard4 c4ImplId)
{
}


//
//  When CC impl objects store a new value (or come out of error state) this is called.
//  The owning unit can respond to that as required. Most units don't need to deal with
//  this, mostly its when event triggers need to be sent, or more than one CC impl may
//  contribute to a field value, so we provide an empty default implementation.
//
tCIDLib::TVoid
TZWaveUnit::ImplValueChanged(const  tCIDLib::TCard4
                            , const tZWaveUSB3Sh::EValSrcs
                            , const tCIDLib::TBoolean
                            , const tCIDLib::TInt4)
{
}


//
//  If the derived class has any CC impl objects installed on us, he must call us
//  to let us pass on this to them. This gives them time to do something once in
//  a while. It's called every 5 seconds, but they shouldn't be doing anything that
//  often. They have to maintain their own time stamps to check when it's time for
//  them to actually use this callback to do something.
//
tCIDLib::TVoid TZWaveUnit::Process()
{
    const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colImpls[c4Index]->Process();
}


//
//  The derived class must call us first, so we can handle any stuff that we do
//  on behalf of all derivatives. Though, if they have special needs and are going to
//  manage the field themselves, they can handle this.
//
tCIDLib::TVoid
TZWaveUnit::QueryFldDefs(tCQCKit::TFldDefList& colToFill, TCQCFldDef& flddTmp) const
{
    const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colImpls[c4Index]->QueryFldDefs(colToFill, flddTmp);
}


//
//  The derived class should call us first, then add any of his own options to the
//  list. We let any CC impl objects add theirs. This is done after bParseExtraInfo()
//  info, but before bPrepare is called.
//
tCIDLib::TVoid
TZWaveUnit::QueryUnitAttrs(tCIDMData::TAttrList& colToFill, TAttrData& attrTmp) const
{
    // Let any impl object add their own options
    const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colImpls[c4Index]->QueryCCAttrs(colToFill, attrTmp);
}


//
//  The derived class should look up any fields of his own, then call us so that we
//  can do the same.
//
tCIDLib::TVoid TZWaveUnit::StoreFldIds()
{
    const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colImpls[c4Index]->StoreFldIds();
}





//
//  We let each CC impl know about this. It means we got an info frame from the unit,
//  which generally means it just came online. This is called if we are in Ready state,
//  so it probably means the unit was offline or temporarily offline and is back. We
//  want to let all the CCs know that they should start quickly trying to get in sync
//  again.
//
//  Derived clases can override and inform any CCs that they manage themselves, but
//  they MUST call us as well, so we can do the ones set on us.
//
tCIDLib::TVoid TZWaveUnit::UnitIsOnline()
{
    const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colImpls[c4Index]->UnitIsOnline();
}



// ---------------------------------------------------------------------------
//  TZWaveUnit: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Returns whether the passed class requires secure comms or not
tCIDLib::TBoolean
TZWaveUnit::bSupportsClass(const tZWaveUSB3Sh::ECClasses eClass) const
{
    return m_punitiOwner->bSupportsClass(eClass);
}


//
//  Our unit info object calls us when he is asked to generate a report. We call
//  some protected virtuals to let the derived class format out stuff. The primary
//  reason they would do this is if they manage some CC impls themselves, which some
//  do.
//
//  We call one method to let the derived class format any unit level stuff they might
//  have. Then we format out any CCs we manage. Then we call another virtual to let
//  the derived class format out its managed CC impls. If it doesn't have any of those
//  things, it doesn't have to do anything.
//
tCIDLib::TVoid TZWaveUnit::FormatReport(TTextOutStream& strmTar) const
{
    // Let the derived class add any other unit level info he has
    FormatUnitInfo(strmTar);

    // And format out the CC impls we are managing on behalf of the derived class
    strmTar << L"\nCC Impls : \n"
            << L"-------------------------------\n";

    // Formt out our CC impls
    const tCIDLib::TCard4 c4Count = m_colImpls.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colImpls[c4Index]->FormatReport(strmTar);

    // Let the derived class format out any CC impls he owns
    FormatCCsInfo(strmTar);

    strmTar << kCIDLib::NewLn;
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Protected, virtual methods
// ---------------------------------------------------------------------------

//
//  A good number of unit handlers won't have any extra report info to format, so we
//  provide do-nothing implementations.
//
tCIDLib::TVoid TZWaveUnit::FormatCCsInfo(TTextOutStream&) const
{
}

tCIDLib::TVoid TZWaveUnit::FormatUnitInfo(TTextOutStream&) const
{
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Let's derived classes add CC impl objects to our list for us to process. We take
//  ownership of it.
//
//  We return an id, which is a 1 based index. This lets them user zero as the never
//  installed it indicator. We just sub one to get the index. We set this on the
//  CC impl object also, so that it can include that in calls it might make to the
//  unit.
//
tCIDLib::TBoolean
TZWaveUnit::bAddImplObject(TZWCCImpl* const pzwcciToAdopt, tCIDLib::TCard4& c4ImplId)
{
    //
    //  Give it the next impl id. If it fails, that's fine, the unit will give up
    //  and this way we insure it's set up front while being initialized.
    //
    c4ImplId = m_colImpls.c4ElemCount() + 1;
    pzwcciToAdopt->c4ImplId(c4ImplId);

    // Initialize it. If that fails, return false and delete the impl
    try
    {
        if (!bInitCCImpl(*pzwcciToAdopt))
        {
            delete pzwcciToAdopt;
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        delete pzwcciToAdopt;
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    catch(...)
    {
        delete pzwcciToAdopt;
        return kCIDLib::False;
    }

    // Look ok so store it and give back its one based index
    m_colImpls.Add(pzwcciToAdopt);

    return kCIDLib::True;
}


// A helper to get extra info values from our device info
tCIDLib::TBoolean
TZWaveUnit::bFindExtraVal(const TString& strKey, TString& strToFill) const
{
    return m_punitiOwner->deviUnit().bFindExtraVal(strKey, strToFill);
}

tCIDLib::TBoolean
TZWaveUnit::bFindOptVal(const TString& strKey, TString& strToFill) const
{
    return m_punitiOwner->bFindOptVal(strKey, strToFill);
}


//
//  The bAddImplObject method above has to initial newly added CC impls, and sometimes
//  our derivatives manage them themselves for special concerns. To avoid a lot of
//  redundant code, and to be flexible to add more init later, we provide a helper to
//  do this.
//
tCIDLib::TBoolean TZWaveUnit::bInitCCImpl(TZWCCImpl& zwcciTar)
{
    try
    {
        if (!zwcciTar.bParseExtraInfo())
            return kCIDLib::False;

        if (!zwcciTar.bPrepare())
            return kCIDLib::False;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return kCIDLib::True;
}


// Get the name of a field given its id
tCIDLib::TBoolean
TZWaveUnit::bQueryFldName(const tCIDLib::TCard4 c4FldId, TString& strToFill) const
{
    return m_punitiOwner->bQueryFldName(c4FldId, strToFill);
}


// A convenience to find a boolean option and return its value, or a default if not found
tCIDLib::TBoolean
TZWaveUnit::bQueryOptVal(const TString& strKey, const tCIDLib::TBoolean bDefault) const
{
    return m_punitiOwner->bQueryOptVal(strKey, bDefault);
}


// Helpers for our derivatives to store their field values
tCIDLib::TBoolean
TZWaveUnit::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TBoolean bToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, bToStore);
}

tCIDLib::TBoolean
TZWaveUnit::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TCard4 c4ToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, c4ToStore);
}

tCIDLib::TBoolean
TZWaveUnit::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TFloat8 f8ToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, f8ToStore);
}

tCIDLib::TBoolean
TZWaveUnit::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TInt4 i4ToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, i4ToStore);
}

tCIDLib::TBoolean
TZWaveUnit::bWriteFld(const tCIDLib::TCard4 c4FldId, const TString& strToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, strToStore);
}


tCIDLib::TCard4 TZWaveUnit::c4LookupFldId(const TString& strFldName) const
{
    return m_punitiOwner->c4FindFldId(strFldName);
}

const TString& TZWaveUnit::strDrvMoniker() const
{
    return m_punitiOwner->strDrvMoniker();
}


tCIDLib::TVoid TZWaveUnit::SetFldError(const tCIDLib::TCard4 c4FldId)
{
    m_punitiOwner->SetFldError(c4FldId);
}

