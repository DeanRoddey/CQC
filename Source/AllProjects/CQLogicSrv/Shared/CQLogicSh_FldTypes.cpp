//
// FILE NAME: CQLogicSh_FldTypes.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2009
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
//  This file implements the abstract bass class for the field types, plus
//  some of the simpler derivatives.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQLogicSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQLSrvFldType,TObject)
AdvRTTIDecls(TCQSLLDMinMaxAvg,TCQLSrvFldType);
AdvRTTIDecls(TCQSLLDPatFmt,TCQLSrvFldType);



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQLogicSh_FldTypes
    {
        // -----------------------------------------------------------------------
        //  The base fld type format version
        //
        //  Version 2 -
        //      In 4.8.917 we bumped the max source fields from 8 to 16. When we read in
        //      the previously stored list of source fields, it's going to have the old
        //      max, so any attempt to add more will fail. So we need to bump the max
        //      size after we read it in.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard1   c1FmtBaseVersion = 2;


        // -----------------------------------------------------------------------
        //  The pattern fld type version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard1   c1FmtPatFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The min/max fld type version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard1   c1FmtMMFmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQLSrvFldType
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQLSrvFldType: Public, static methods
// ---------------------------------------------------------------------------

// For sorting. We just pass it on to the name field of the field def
tCIDLib::ESortComps
TCQLSrvFldType::eComp(  const   TCQLSrvFldType& clsft1
                        , const TCQLSrvFldType& clsft2
                        , const tCIDLib::TCard4 )
{
    return clsft1.strFldName().eCompare(clsft2.strFldName());
}


// For key extraction purposes in keyed collections
const TString& TCQLSrvFldType::strKey(const TCQLSrvFldType& clsftSrc)
{
    return clsftSrc.strFldName();
}


// ---------------------------------------------------------------------------
//  TCQLSrvFldType: Destructor
// ---------------------------------------------------------------------------
TCQLSrvFldType::~TCQLSrvFldType()
{
    // Clean up any value object
    delete m_pfvCurrent;
    m_pfvCurrent = nullptr;
}


// ---------------------------------------------------------------------------
//  TCQLSrvFldType: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  Add a new field to the list of contributing fields. Return indicates
//  whether we added it or not. Generally dups make no sense, so we don't
//  add the same field again.
//
tCIDLib::TBoolean TCQLSrvFldType::bAddField(const TString& strToAdd)
{
    //
    //  The client side configuration interface should honor our max src fields
    //  but verify that in debug mode.
    //
    m_colSrcFields.CheckIsFull(m_c4MaxSrcFlds, L"logic srv field src fields");

    // If already present, don't add again
    tCIDLib::TCard4 c4Index;
    if (bFindByName(strToAdd, c4Index))
        return kCIDLib::False;

    // Add it at the end
    m_colSrcFields.objAdd(strToAdd);
    return kCIDLib::True;
}


// Only a couple of values are compile time values that need to be compared
tCIDLib::TBoolean
TCQLSrvFldType::bIsEqual(const TCQLSrvFldType& clsftRHS) const
{
    // Check the basic field config info
    if (m_flddCfg != clsftRHS.m_flddCfg)
        return kCIDLib::False;

    // Compare the source fields
    return (m_colSrcFields == clsftRHS.m_colSrcFields);
}


//
//  Moves the indicated field up or down in the list. Generally that's not
//  important but some derivatives might be sensitive to this, because they
//  do 'early exit', such as boolean fields which can stop as soon as they
//  know what the result must be.
//
//  The return indicates whether we actually moved it, since it might not
//  be possible if it's already at one end or the other.
//
tCIDLib::TBoolean
TCQLSrvFldType::bMoveField( const   tCIDLib::TCard4     c4At
                            , const tCIDLib::TBoolean   bUp)
{
    // Make sure it's a valid index
    const tCIDLib::TCard4 c4Count = m_colSrcFields.c4ElemCount();
    if (c4At >= c4Count)
    {
        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcCfg_BadFldIndex2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4At)
        );
    }

    // If the count is one, obviously can't do it
    if (c4Count == 1)
        return kCIDLib::False;

    if (bUp)
    {
        // If already at zero position, can't do it
        if (!c4At)
            return kCIDLib::False;
        m_colSrcFields.SwapItems(c4At, c4At - 1);
    }
     else
    {
        // If already at the last position, can't do it
        if (c4At + 1 == c4Count)
            return kCIDLib::False;
        m_colSrcFields.SwapItems(c4At, c4At + 1);
    }
    return kCIDLib::True;
}


//
//  At this level we insure that at least one source field is defined, since
//  it makes no sense to do otherwise.
//
tCIDLib::TBoolean
TCQLSrvFldType::bValidate(          TString&            strErr
                            ,       tCIDLib::TCard4&    c4SrcFldInd
                            , const TCQCFldCache&       cfcData)
{
    const tCIDLib::TCard4 c4SrcCnt = m_colSrcFields.c4ElemCount();

    // Make sure we have at least one source field
    if (!c4SrcCnt)
    {
        c4SrcFldInd = kCIDLib::c4MaxCard;
        strErr.LoadFromMsg(kCQLShErrs::errcCfg_NoSrcFields, facCQLogicSh());
        return kCIDLib::False;
    }

    // Make sure all the source fields are readable
    tCIDLib::TCard4 c4DriverId;
    TString         strField;
    TString         strMoniker;

    c4SrcFldInd = kCIDLib::c4MaxCard;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcCnt; c4Index++)
    {
        // Look up this source field
        facCQCKit().bParseFldName(strSrcFldAt(c4Index), strMoniker, strField);
        const TCQCFldDef* pflddSrc = cfcData.pflddFor(strMoniker, strField, c4DriverId);

        if (!pflddSrc)
        {
            strErr.LoadFromMsg
            (
                kCQLShErrs::errcCfg_SrcFldNotFound
                , facCQLogicSh()
                , strFldName()
                , strSrcFldAt(c4Index)
            );
            return kCIDLib::False;
        }
         else if (!tCIDLib::bAllBitsOn(pflddSrc->eAccess(), tCQCKit::EFldAccess::Read))
        {
            strErr.LoadFromMsg
            (
                kKitErrs::errcFld_FieldNotReadable
                , facCQCKit()
                , strMoniker
                , strField
            );
            c4SrcFldInd = c4Index;
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Do any initialization required to get us ready. For now we do nothing
//  but all our derived classes should call us first just in case we do
//  need to later.
//
tCIDLib::TVoid TCQLSrvFldType::Initialize(TCQCFldCache&)
{
}


// Default is to just get any readable field
TCQCFldFilter* TCQLSrvFldType::pffiltToUse()
{
    return new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite);
}


// Removes the source field at the indicated index
tCIDLib::TVoid TCQLSrvFldType::RemoveField(const tCIDLib::TCard4 c4At)
{
    // Make sure it's a valid index
    const tCIDLib::TCard4 c4Count = m_colSrcFields.c4ElemCount();
    if (c4At >= c4Count)
    {
        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcCfg_BadFldIndex2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4At)
        );
    }

    // Remove this guy
    m_colSrcFields.RemoveAt(c4At);
}



// ---------------------------------------------------------------------------
//  TCQLSrvFldType: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return the always store flag for the server to use
tCIDLib::TBoolean TCQLSrvFldType::bAlwaysStore() const
{
    return m_bAlwaysStore;
}


// Look up a field name by name and return the index if found
tCIDLib::TBoolean
TCQLSrvFldType::bFindByName(const   TString&            strName
                            ,       tCIDLib::TCard4&    c4Index) const
{
    // Make sure it's not already added
    const tCIDLib::TCard4 c4Count = m_colSrcFields.c4ElemCount();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (strName == m_colSrcFields[c4Index])
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Return the normal field flag for the server to use
tCIDLib::TBoolean TCQLSrvFldType::bNormalFld() const
{
    return m_bNormalFld;
}


// Get/set the field id
tCIDLib::TCard4 TCQLSrvFldType::c4FldId() const
{
    return m_c4FldId;
}

tCIDLib::TCard4 TCQLSrvFldType::c4FldId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId = c4ToSet;
    return m_c4FldId;
}


//
//  Indicates how many source fields this logic server field can have. It
//  defaults to the hard maximum. This is a compile time only thing, not
//  something that changes based on configuration. The derived classes pass
//  it in during ctor and it never changes after that.
//
tCIDLib::TCard4 TCQLSrvFldType::c4MaxSrcFields() const
{
    return m_c4MaxSrcFlds;
}


// Get the count of src fields configured for us
tCIDLib::TCard4 TCQLSrvFldType::c4SrcFldCount() const
{
    return m_colSrcFields.c4ElemCount();
}


// Provide read only access to the list of src fields
const tCQLogicSh::TSrcList& TCQLSrvFldType::colSrcFields() const
{
    return m_colSrcFields;
}


// A convenience to get/set the field type, which is in the field def member
tCQCKit::EFldTypes TCQLSrvFldType::eType() const
{
    return m_flddCfg.eType();
}

tCQCKit::EFldTypes TCQLSrvFldType::eType(const tCQCKit::EFldTypes eToSet)
{
    m_flddCfg.eType(eToSet);

    // Create or recreate the value object if needed
    MakeValueObj();
    return m_flddCfg.eType();
}


//
//  This is called by the server periodically. We run through our configured
//  source fields and evaluate their current state and value. If the state
//  or value of any of the fields have changed, then we call the derived class,
//  who can use all those values to create it's composite value and return it.
//
tCIDLib::TVoid
TCQLSrvFldType::Evaluate(       TCQCPollEngine& polleToUse
                        , const tCIDLib::TCard4 c4Hour
                        , const tCIDLib::TCard4 c4Minute)
{
    CIDAssert
    (
        m_pfvCurrent->eFldType() == m_flddCfg.eType()
        , L"The value object is not of the correct type"
    )

    //
    //  Start with no change as the assumed default. As we check each
    //  one, we'll take the new result if it's lower. So we'll move to
    //  new value or error state.
    //
    tCQLogicSh::EEvalRes eResult = tCQLogicSh::EEvalRes::NoChange;

    //
    //  Run through them all and check their current state/value. If we
    //  hit any one that's in error, that's as far as we need to go.
    //
    const tCIDLib::TCard4 c4Count = m_colPollInfo.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCFldPollInfo& cfpiCur = m_colPollInfo[c4Index];

        if (cfpiCur.bUpdateValue(polleToUse))
        {
            //
            //  Get the new field state of this one. If that translates into
            //  a lesser state of ours, then keep that as the new state.
            //
            const tCQCPollEng::EFldStates eNewState = cfpiCur.eState();
            if (eNewState == tCQCPollEng::EFldStates::Ready)
            {
                //
                //  Something changes, and it's in ready state, so that has
                //  to mean that there's a new value for this one. If not
                //  already below or in new value state, then move to that.
                //
                if (eResult > tCQLogicSh::EEvalRes::NewValue)
                    eResult = tCQLogicSh::EEvalRes::NewValue;
            }
             else
            {
                //
                //  Either not up to the point of being able to get the value
                //  yet, or in error, so set us to error state if not already.
                //
                if (eResult > tCQLogicSh::EEvalRes::Error)
                    eResult = tCQLogicSh::EEvalRes::Error;
            }
        }
         else
        {
            // No change, so see if it's in error. If so, set us to error state
            if (!cfpiCur.bHasGoodValue())
                eResult = tCQLogicSh::EEvalRes::Error;
        }
    }

    //
    //
    //
    if ((eResult == tCQLogicSh::EEvalRes::NewValue)
    ||  (m_bAlwaysStore && (eResult != tCQLogicSh::EEvalRes::Error)))
    {
        try
        {
            eResult = eBuildValue(m_colPollInfo, *m_pfvCurrent, c4Hour, c4Minute);
        }

        catch(const TError& errToCatch)
        {
            eResult = tCQLogicSh::EEvalRes::Error;

            if (facCQLogicSh().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            if (facCQLogicSh().bLogFailures())
            {
                facCQLogicSh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"An exception occured during field scan. Name=%(1)"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , m_flddCfg.strName()
                );
            }
        }
    }

    //
    //  If error state, then set that flag on the value object
    //
    if (eResult == tCQLogicSh::EEvalRes::Error)
        m_pfvCurrent->bInError(kCIDLib::True);
}



// Read only access to the field definition for our field
const TCQCFldDef& TCQLSrvFldType::flddCfg() const
{
    return m_flddCfg;
}


// Provide access to our value object
const TCQCFldValue& TCQLSrvFldType::fvCurrent() const
{
    CIDAssert(m_pfvCurrent != 0, L"The value object has not been set");
    return *m_pfvCurrent;
}

TCQCFldValue& TCQLSrvFldType::fvCurrent()
{
    CIDAssert(m_pfvCurrent != 0, L"The value object has not been set");
    return *m_pfvCurrent;
}


//
//  This is called on the server driver side, upon startup or changing of
//  the field configuration, to let us get our fields registered with the
//  polling engine.
//
tCIDLib::TVoid
TCQLSrvFldType::RegisterFields(         TCQCPollEngine& polleToUse
                                , const TCQCFldCache&   cfcData)
{
    //
    //  First we remove any entries from the poll list, and go backand load
    //  it up with the moniker/name of our currently configured fields.
    //
    TString strMoniker;
    TString strField;
    m_colPollInfo.RemoveAll();
    const tCIDLib::TCard4 c4SrcCount = m_colSrcFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcCount; c4Index++)
    {
        facCQCKit().ParseFldName(m_colSrcFields[c4Index], strMoniker, strField);
        m_colPollInfo.objPlace(strMoniker, strField);
    }

    // Now go back and register them
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcCount; c4Index++)
        m_colPollInfo[c4Index].bRegister(polleToUse, cfcData);
}


// A convenience to get/set the field name, which is in the field def member
const TString& TCQLSrvFldType::strFldName() const
{
    return m_flddCfg.strName();
}

const TString& TCQLSrvFldType::strFldName(const TString& strToSet)
{
    m_flddCfg.strName(strToSet);
    return m_flddCfg.strName();
}


const TString& TCQLSrvFldType::strSrcFldAt(const tCIDLib::TCard4 c4At) const
{
    return m_colSrcFields[c4At];
}


// ---------------------------------------------------------------------------
//  TCQLSrvFldType: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQLSrvFldType::TCQLSrvFldType( const   tCIDLib::TBoolean   bNormalFld
                                , const tCIDLib::TBoolean   bAlwaysStore
                                , const tCIDLib::TCard4     c4MaxSrcFlds) :
    m_bAlwaysStore(bAlwaysStore)
    , m_bNormalFld(bNormalFld)
    , m_c4FldId(0)
    , m_c4MaxSrcFlds(c4MaxSrcFlds)
    , m_colPollInfo(c4MaxSrcFlds)
    , m_colSrcFields(c4MaxSrcFlds)
    , m_flddCfg
      (
        TString::strEmpty(), tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read
      )
    , m_pfvCurrent(nullptr)
{
    //
    //  We provided a some dummy field def info, so that we can make the value
    //  object even in this case and not have to worry about it not being
    //  there.
    //
    MakeValueObj();
}

TCQLSrvFldType::TCQLSrvFldType( const   TString&            strName
                                , const tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldAccess eAccess
                                , const TString&            strLimits
                                , const tCIDLib::TBoolean   bNormalFld
                                , const tCIDLib::TBoolean   bAlwaysStore
                                , const tCIDLib::TCard4     c4MaxSrcFlds) :

    m_bAlwaysStore(bAlwaysStore)
    , m_bNormalFld(bNormalFld)
    , m_c4FldId(0)
    , m_c4MaxSrcFlds(c4MaxSrcFlds)
    , m_colPollInfo(c4MaxSrcFlds)
    , m_colSrcFields(c4MaxSrcFlds)
    , m_flddCfg(strName, eType, eAccess, strLimits)
    , m_pfvCurrent(nullptr)
{
    MakeValueObj();
}

TCQLSrvFldType::TCQLSrvFldType( const   TString&            strName
                                , const tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldAccess eAccess
                                , const tCIDLib::TBoolean   bNormalFld
                                , const tCIDLib::TBoolean   bAlwaysStore
                                , const tCIDLib::TCard4     c4MaxSrcFlds) :

    m_bAlwaysStore(bAlwaysStore)
    , m_bNormalFld(bNormalFld)
    , m_c4FldId(0)
    , m_c4MaxSrcFlds(c4MaxSrcFlds)
    , m_colPollInfo(c4MaxSrcFlds)
    , m_colSrcFields(c4MaxSrcFlds)
    , m_flddCfg(strName, eType, eAccess)
    , m_pfvCurrent(nullptr)
{
    MakeValueObj();
}

TCQLSrvFldType::TCQLSrvFldType(const TCQLSrvFldType& clsftToCopy) :

    m_bAlwaysStore(clsftToCopy.m_bAlwaysStore)
    , m_bNormalFld(clsftToCopy.m_bNormalFld)
    , m_c4FldId(clsftToCopy.m_c4FldId)
    , m_c4MaxSrcFlds(clsftToCopy.m_c4MaxSrcFlds)
    , m_colPollInfo(clsftToCopy.m_c4MaxSrcFlds)
    , m_colSrcFields(clsftToCopy.m_colSrcFields)
    , m_flddCfg(clsftToCopy.m_flddCfg)
    , m_pfvCurrent(nullptr)
{
    //
    //  The poll info list is not copied. It's a runtime value only, and will
    //  be set up after these objects are loaded and in place.
    //
    MakeValueObj();
}

TCQLSrvFldType& TCQLSrvFldType::operator=(const TCQLSrvFldType& clsftToAssign)
{
    if (this != &clsftToAssign)
    {
        m_bAlwaysStore  = clsftToAssign.m_bAlwaysStore;
        m_bNormalFld    = clsftToAssign.m_bNormalFld;
        m_c4FldId       = clsftToAssign.m_c4FldId;
        m_c4MaxSrcFlds  = clsftToAssign.m_c4MaxSrcFlds;
        m_colSrcFields  = clsftToAssign.m_colSrcFields;
        m_flddCfg       = clsftToAssign.m_flddCfg;

        //
        //  The poll info list is not copied. It's a runtime value only, and
        //  will be set up after these objects are loaded and in place, during
        //  the field registration call. So just reset those.
        //
        m_colPollInfo.RemoveAll();

        MakeValueObj();
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQLSrvFldType: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQLSrvFldType::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_FldTypes::c1FmtBaseVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    // Stream our stuff
    strmToReadFrom  >> m_colSrcFields
                    >> m_flddCfg;

    //
    //  If pre-V2, then we need to adjust the max size of the src field list. This can't
    //  be done directly, so we have to copy to another one, reset ours, then copy the
    //  elements back over.
    //
    if (c1FmtVersion < 2)
    {
        tCQLogicSh::TSrcList colTmp = m_colSrcFields;
        m_colSrcFields.Reallocate(m_c4MaxSrcFlds, kCIDLib::False);

        const tCIDLib::TCard4 c4Count = colTmp.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_colSrcFields.objAdd(colTmp[c4Index]);
    }

    // Reset any runtime only stuff
    m_c4FldId = 0;
    m_colPollInfo.RemoveAll();
    MakeValueObj();

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQLSrvFldType::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_FldTypes::c1FmtBaseVersion
                    << m_colSrcFields
                    << m_flddCfg
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQLSrvFldType: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Creates or recreates the value object appropriate for our field type
tCIDLib::TVoid TCQLSrvFldType::MakeValueObj()
{
    //
    //  Make sure that the value object is there, and if so that it's of
    //  the correct type for this field. If not, create it or re-create
    //  it.
    //
    if (!m_pfvCurrent)
    {
        m_pfvCurrent = facCQCKit().pfvMakeNewFor(0, 0, m_flddCfg.eType());
    }
     else if (m_pfvCurrent->eFldType() != m_flddCfg.eType())
    {
        delete m_pfvCurrent;
        m_pfvCurrent = nullptr;
        m_pfvCurrent = facCQCKit().pfvMakeNewFor(0, 0, m_flddCfg.eType());
    }
}





// ---------------------------------------------------------------------------
//   CLASS: CQSLLDPatFmt
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLLDPatFmt: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLLDPatFmt::TCQSLLDPatFmt(const TString& strName) :

    TCQLSrvFldType(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
{
}

TCQSLLDPatFmt::TCQSLLDPatFmt(const TCQSLLDPatFmt& clsftToCopy) :

    TCQLSrvFldType(clsftToCopy)
    , m_strPattern(clsftToCopy.m_strPattern)
{
}

TCQSLLDPatFmt::~TCQSLLDPatFmt()
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDPatFmt: Public oeprators
// ---------------------------------------------------------------------------
TCQSLLDPatFmt& TCQSLLDPatFmt::operator=(const TCQSLLDPatFmt& clsftToAssign)
{
    if (this != &clsftToAssign)
    {
        TParent::operator=(clsftToAssign);
        m_strPattern = clsftToAssign.m_strPattern;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQSLLDPatFmt: Public, inherited methods
// ---------------------------------------------------------------------------

// See if we are equal to the passed one
tCIDLib::TBoolean
TCQSLLDPatFmt::bIsEqual(const TCQLSrvFldType& clsftRHS) const
{
    if (!TParent::bIsEqual(clsftRHS))
        return kCIDLib::False;

    // Make sure it's of our type or beyond
    if (!clsftRHS.bIsDescendantOf(clsThis()))
        return kCIDLib::False;

    // It is, so cast it to our type and compare
    const TCQSLLDPatFmt& clsftAct = static_cast<const TCQSLLDPatFmt&>(clsftRHS);
    return (m_strPattern == clsftAct.m_strPattern);
}


tCIDLib::TBoolean TCQSLLDPatFmt::bIsValidSrcFld(const TCQCFldDef&) const
{
    // We can do any type that is currently supported
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQSLLDPatFmt::bValidate(       TString&            strErr
                        ,       tCIDLib::TCard4&    c4SrcFldInd
                        , const TCQCFldCache&       cfcData)
{
    if (!TParent::bValidate(strErr, c4SrcFldInd, cfcData))
        return kCIDLib::False;

    // We don't have any source field related stuff that could be in error
    c4SrcFldInd = kCIDLib::c4MaxCard;

    // And we don't have anything to validate per se
    return kCIDLib::True;
}


tCIDLib::TVoid TCQSLLDPatFmt::Initialize(TCQCFldCache& cfcInit)
{
    // Call our parent first
    TParent::Initialize(cfcInit);

    // A no-op for us
}


// ---------------------------------------------------------------------------
//  TCQSLLDPatFmt: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the pattern string
const TString& TCQSLLDPatFmt::strPattern() const
{
    return m_strPattern;
}

const TString& TCQSLLDPatFmt::strPattern(const TString& strToSet)
{
    m_strPattern = strToSet;
    return m_strPattern;
}


// ---------------------------------------------------------------------------
//  TCQSLLDPatFmt: Hidden Constructors
// ---------------------------------------------------------------------------

// Just needed for polymorphic streaming
TCQSLLDPatFmt::TCQSLLDPatFmt() :

    TCQLSrvFldType()
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDPatFmt: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We just run through the pattern string and build up a new value from the
//  source fields. We assume that any given number token is only present once
//  within the pattern, and that contiguous token numbers are used. So once
//  we fail to find one, we stop.
//
tCQLogicSh::EEvalRes
TCQSLLDPatFmt::eBuildValue( const   tCQLogicSh::TInfoList&  colVals
                            ,       TCQCFldValue&           fldvToFill
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4         )
{
    // Start our format string with the pattern
    m_strFmt = m_strPattern;

    //
    //  And do tokens in numeric order until we don't find one, or we run out
    //  of source values to format.
    //
    const tCIDLib::TCard4 c4ValCnt = colVals.c4ElemCount();
    tCIDLib::TCh chToken = kCIDLib::chDigit1;
    tCIDLib::TCard4 c4ValInd = 0;
    while ((chToken < kCIDLib::chDigit9) && (c4ValInd < c4ValCnt))
    {
        //
        //  Get the first field value out. These guys can format themselves
        //  to text, so we don't have to deal with the polymorphism here.
        //
        colVals[c4ValInd].fvCurrent().Format(m_strFmt2);

        // And try the token replacement. If none found, we are done early
        if (m_strFmt.eReplaceToken(m_strFmt2, chToken) != tCIDLib::EFindRes::Found)
            break;

        c4ValInd++;
        chToken++;
    }

    //
    //  Now we cast the field value to it's correct type and store the new
    //  value. The base class verifies it's of the right type before calling
    //  us.
    //
    if (static_cast<TCQCStringFldValue&>(fldvToFill).bSetValue(m_strFmt))
        return tCQLogicSh::EEvalRes::NewValue;
    return tCQLogicSh::EEvalRes::NoChange;
}


tCIDLib::TVoid TCQSLLDPatFmt::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_FldTypes::c1FmtPatFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    // Do our parent's stuff
    TParent::StreamFrom(strmToReadFrom);

    // Stream our stuff
    strmToReadFrom >> m_strPattern;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQSLLDPatFmt::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_FldTypes::c1FmtPatFmtVersion;

    // Do our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_strPattern
                    << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//   CLASS: CQSLLDMinMaxAvg
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLLDMinMaxAvg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLLDMinMaxAvg::TCQSLLDMinMaxAvg( const   TString&            strName
                                    , const tCQCKit::EFldTypes  eType) :

    TCQLSrvFldType(strName, eType, tCQCKit::EFldAccess::Read)
    , m_eValType(tCQLogicSh::EMMATypes::Minimum)
{
}

TCQSLLDMinMaxAvg::TCQSLLDMinMaxAvg(const TCQSLLDMinMaxAvg& clsftToCopy) :

    TCQLSrvFldType(clsftToCopy)
    , m_eValType(clsftToCopy.m_eValType)
{
}

TCQSLLDMinMaxAvg::~TCQSLLDMinMaxAvg()
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDMinMaxAvg: Public oeprators
// ---------------------------------------------------------------------------
TCQSLLDMinMaxAvg& TCQSLLDMinMaxAvg::operator=(const TCQSLLDMinMaxAvg& clsftToAssign)
{
    if (this != &clsftToAssign)
    {
        TParent::operator=(clsftToAssign);
        m_eValType = clsftToAssign.m_eValType;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQSLLDMinMaxAvg: Public, inherited methods
// ---------------------------------------------------------------------------

// See if we are equal to the passed one
tCIDLib::TBoolean
TCQSLLDMinMaxAvg::bIsEqual(const TCQLSrvFldType& clsftRHS) const
{
    if (!TParent::bIsEqual(clsftRHS))
        return kCIDLib::False;

    // Make sure it's of our type or beyond
    if (!clsftRHS.bIsDescendantOf(clsThis()))
        return kCIDLib::False;

    // It is, so cast it to our type and compare
    const TCQSLLDMinMaxAvg& clsftAct = static_cast<const TCQSLLDMinMaxAvg&>(clsftRHS);

    return (m_eValType == clsftAct.m_eValType);
}


tCIDLib::TBoolean
TCQSLLDMinMaxAvg::bIsValidSrcFld(const TCQCFldDef& flddTest) const
{
    // It has to be the same field type as us
    return (flddTest.eType() == eType());
}


tCIDLib::TBoolean
TCQSLLDMinMaxAvg::bValidate(        TString&            strErr
                            ,       tCIDLib::TCard4&    c4SrcFldInd
                            , const TCQCFldCache&       cfcData)
{
    if (!TParent::bValidate(strErr, c4SrcFldInd, cfcData))
        return kCIDLib::False;

    // We don't have any source field related stuff that could be in error
    c4SrcFldInd = kCIDLib::c4MaxCard;

    // And we don't really have anything to valid
    return kCIDLib::True;
}


tCIDLib::TVoid TCQSLLDMinMaxAvg::Initialize(TCQCFldCache& cfcInit)
{
    // Call our parent first
    TParent::Initialize(cfcInit);

    // A no-op for us
}


TCQCFldFilter* TCQSLLDMinMaxAvg::pffiltToUse()
{
    // We only can accept numeric fields
    return new TCQCFldFiltNumeric
    (
        tCQCKit::EReqAccess::MReadCWrite, kCIDLib::True
    );
}


// ---------------------------------------------------------------------------
//  TCQSLLDMinMaxAvg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the value type we create
tCQLogicSh::EMMATypes TCQSLLDMinMaxAvg::eValType() const
{
    return m_eValType;
}

tCQLogicSh::EMMATypes
TCQSLLDMinMaxAvg::eValType(const tCQLogicSh::EMMATypes eToSet)
{
    m_eValType = eToSet;
    return m_eValType;
}


// ---------------------------------------------------------------------------
//  TCQSLLDMinMaxAvg: Hidden Constructors
// ---------------------------------------------------------------------------

// Just needed for polymorophic streaming
TCQSLLDMinMaxAvg::TCQSLLDMinMaxAvg() :

    TCQLSrvFldType()
    , m_eValType(tCQLogicSh::EMMATypes::Minimum)
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDMinMaxAvg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We just run through the pattern string and build up a new value from the
//  source fields.
//
tCQLogicSh::EEvalRes
TCQSLLDMinMaxAvg::eBuildValue(  const   tCQLogicSh::TInfoList&  colVals
                                ,       TCQCFldValue&           fldvToFill
                                , const tCIDLib::TCard4
                                , const tCIDLib::TCard4         )
{
    const tCIDLib::TCard4 c4ValCnt = colVals.c4ElemCount();
    const tCQCKit::EFldTypes eFType = eType();

    tCIDLib::TBoolean bChanged = kCIDLib::False;
    if (eFType == tCQCKit::EFldTypes::Card)
    {
        tCIDLib::TCard4 c4Accum = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCnt; c4Index++)
        {
            const TCQCCardFldValue& fvCur = static_cast<const TCQCCardFldValue&>
            (
                colVals[c4Index].fvCurrent()
            );
            const tCIDLib::TCard4 c4Cur = fvCur.c4Value();

            if (c4Index)
            {
                switch(m_eValType)
                {
                    case tCQLogicSh::EMMATypes::Minimum :
                        if (c4Cur < c4Accum)
                            c4Accum = c4Cur;
                        break;

                    case tCQLogicSh::EMMATypes::Maximum :
                        if (c4Cur > c4Accum)
                            c4Accum = c4Cur;
                        break;

                    case tCQLogicSh::EMMATypes::Average :
                        c4Accum += c4Cur;
                        break;
                };
            }
             else
            {
                c4Accum = c4Cur;
            }
        }

        if (m_eValType == tCQLogicSh::EMMATypes::Average)
            c4Accum /= c4ValCnt;

        TCQCCardFldValue& fvTar = static_cast<TCQCCardFldValue&>(fldvToFill);
        bChanged = fvTar.bSetValue(c4Accum);
    }
     else if (eFType == tCQCKit::EFldTypes::Float)
    {
        tCIDLib::TFloat8 f8Accum = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCnt; c4Index++)
        {
            const TCQCFloatFldValue& fvCur = static_cast<const TCQCFloatFldValue&>
            (
                colVals[c4Index].fvCurrent()
            );
            const tCIDLib::TFloat8 f8Cur = fvCur.f8Value();

            if (c4Index)
            {
                switch(m_eValType)
                {
                    case tCQLogicSh::EMMATypes::Minimum :
                        if (f8Cur < f8Accum)
                            f8Accum = f8Cur;
                        break;

                    case tCQLogicSh::EMMATypes::Maximum :
                        if (f8Cur > f8Accum)
                            f8Accum = f8Cur;
                        break;

                    case tCQLogicSh::EMMATypes::Average :
                        f8Accum += f8Cur;
                        break;
                };
            }
             else
            {
                f8Accum = f8Cur;
            }
        }

        if (m_eValType == tCQLogicSh::EMMATypes::Average)
            f8Accum /= tCIDLib::TFloat8(c4ValCnt);

        TCQCFloatFldValue& fvTar = static_cast<TCQCFloatFldValue&>(fldvToFill);
        bChanged = fvTar.bSetValue(f8Accum);
    }
     else if (eFType == tCQCKit::EFldTypes::Int)
    {
        tCIDLib::TInt4 i4Accum = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCnt; c4Index++)
        {
            const TCQCIntFldValue& fvCur = static_cast<const TCQCIntFldValue&>
            (
                colVals[c4Index].fvCurrent()
            );
            const tCIDLib::TInt4 i4Cur = fvCur.i4Value();

            if (c4Index)
            {
                switch(m_eValType)
                {
                    case tCQLogicSh::EMMATypes::Minimum :
                        if (i4Cur < i4Accum)
                            i4Accum = i4Cur;
                        break;

                    case tCQLogicSh::EMMATypes::Maximum :
                        if (i4Cur > i4Accum)
                            i4Accum = i4Cur;
                        break;

                    case tCQLogicSh::EMMATypes::Average :
                        i4Accum += i4Cur;
                        break;
                };
            }
             else
            {
                i4Accum = i4Cur;
            }
        }

        if (m_eValType == tCQLogicSh::EMMATypes::Average)
            i4Accum /= tCIDLib::TInt4(c4ValCnt);

        TCQCIntFldValue& fvTar = static_cast<TCQCIntFldValue&>(fldvToFill);
        bChanged = fvTar.bSetValue(i4Accum);
    }
     else
    {
        // Something is wrong, since we don't understand the type
        return tCQLogicSh::EEvalRes::Error;
    }

    if (bChanged)
        return tCQLogicSh::EEvalRes::NewValue;
    return tCQLogicSh::EEvalRes::NoChange;

}


tCIDLib::TVoid TCQSLLDMinMaxAvg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_FldTypes::c1FmtMMFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    // Do our parent's stuff
    TParent::StreamFrom(strmToReadFrom);

    // Stream our stuff
    strmToReadFrom >> m_eValType;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQSLLDMinMaxAvg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_FldTypes::c1FmtMMFmtVersion;

    // Do our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_eValType
                    << tCIDLib::EStreamMarkers::EndObject;
}


