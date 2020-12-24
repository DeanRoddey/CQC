//
// FILE NAME: CQCIntfEng_States.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/28/2004
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
//  This file implements the classes that define the 'states' of a template.
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
#include    "CQCIntfEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfStateItem,TObject)
RTTIDecls(TCQCIntfState,TObject)
RTTIDecls(TCQCIntfStateList,TObject)



// ---------------------------------------------------------------------------
//  Local consts and types
// ---------------------------------------------------------------------------
namespace CQCIntfEng_States
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our state item format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2ItemFmtVersion    = 1;


        // -----------------------------------------------------------------------
        //  Our state list format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2ListFmtVersion    = 1;


        // -----------------------------------------------------------------------
        //  Our state format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2StateFmtVersion   = 1;
    }
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCIntfStateItem
// PREFIX: isti
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TCQCIntfStateItem: Constructors and Destructor
// ---------------------------------------------------------------------------

TCQCIntfStateItem::TCQCIntfStateItem() :

    m_eLastRes(tCQCIntfEng::EExprResults::Error)
{
}

TCQCIntfStateItem::TCQCIntfStateItem(const  TString&    strDescription
                                    , const TString&    strMoniker
                                    , const TString&    strField) :

    m_cfpiAssoc(strMoniker, strField)
    , m_eLastRes(tCQCIntfEng::EExprResults::Error)
    , m_exprEval(strDescription)
{
}

TCQCIntfStateItem::TCQCIntfStateItem(const  TString&        strMoniker
                                    , const TString&        strField
                                    , const TCQCExpression& exprEval) :

    m_cfpiAssoc(strMoniker, strField)
    , m_eLastRes(tCQCIntfEng::EExprResults::Error)
    , m_exprEval(exprEval)
{
}

TCQCIntfStateItem::TCQCIntfStateItem(const TCQCExpression& exprEval) :

    m_exprEval(exprEval)
{
}

TCQCIntfStateItem::TCQCIntfStateItem(const TCQCIntfStateItem& istiToCopy) :

    m_cfpiAssoc(istiToCopy.m_cfpiAssoc)
    , m_eLastRes(tCQCIntfEng::EExprResults::Error)
    , m_exprEval(istiToCopy.m_exprEval)
{
    // Note that the state, result, and serial num go back to default status again
}

TCQCIntfStateItem::~TCQCIntfStateItem()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStateItem: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfStateItem::operator=(const TCQCIntfStateItem& istiSrc)
{
    if (this != &istiSrc)
    {
        m_cfpiAssoc  = istiSrc.m_cfpiAssoc;
        m_exprEval   = istiSrc.m_exprEval;

        // Reset the last result until proven otherwise
        m_eLastRes = tCQCIntfEng::EExprResults::Error;
    }
}

tCIDLib::TBoolean
TCQCIntfStateItem::operator==(const TCQCIntfStateItem& istiSrc) const
{
    if (this != &istiSrc)
    {
        //
        //  For the field assocation all that matter is the field name, since that's
        //  all that is persisted.
        //
        if ((m_cfpiAssoc.strFullFldName() != istiSrc.m_cfpiAssoc.strFullFldName())
        ||  (m_exprEval != istiSrc.m_exprEval))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCIntfStateItem::operator!=(const TCQCIntfStateItem& istiSrc) const
{
    return !operator==(istiSrc);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStateItem: Public, non-virtual methods
// ---------------------------------------------------------------------------


// Get/set the negated setting. This is just passed on to the expression object
tCIDLib::TBoolean TCQCIntfStateItem::bNegated() const
{
    return m_exprEval.bNegated();
}

tCIDLib::TBoolean TCQCIntfStateItem::bNegated(const tCIDLib::TBoolean bToSet)
{
    m_exprEval.bNegated(bToSet);
    return bToSet;
}


const TCQCExpression& TCQCIntfStateItem::exprEval() const
{
    return m_exprEval;
}


tCQCIntfEng::EExprResults
TCQCIntfStateItem::eEvaluate(       TCQCPollEngine& polleToUse
                            , const TStdVarsTar&    ctarGlobalVars)
{
    //
    //  Ask the field info object to try to update the value. If anything
    //  changed, then deal with it. If nothing changed, we just return
    //  the previous value.
    //
    if (m_cfpiAssoc.bUpdateValue(polleToUse))
    {
        const tCQCPollEng::EFldStates eNewState = m_cfpiAssoc.eState();

        if (eNewState == tCQCPollEng::EFldStates::Ready)
        {
            //
            //  Something changed, so if the new state is ready, then that
            //  means we got a new value, or we got a good initial value
            //  after being offline before.
            //
            //  So we evalute ourself based on the new value
            //
            if (m_exprEval.bEvaluate(m_cfpiAssoc.fvCurrent(), &ctarGlobalVars))
                m_eLastRes = tCQCIntfEng::EExprResults::True;
            else
                m_eLastRes = tCQCIntfEng::EExprResults::False;
        }
         else
        {
            // Something is wrong, so put us into error result state
            m_eLastRes = tCQCIntfEng::EExprResults::Error;
        }
    }

    // And return the new stored result
    return m_eLastRes;
}


const TCQCFldDef& TCQCIntfStateItem::flddAssoc() const
{
    return m_cfpiAssoc.flddAssoc();
}


tCIDLib::TVoid
TCQCIntfStateItem::QueryAttrs(tCIDMData::TAttrList& colAttrs, TAttrData& adatTmp) const
{
    // Some are really at our level, so add those
    adatTmp.Set
    (
        L"Field"
        , kCQCIntfEng::strAttr_StateItem_Field
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );

    // If we have current info, set that as the current value
    if (!m_cfpiAssoc.strFullFldName().bIsEmpty())
        adatTmp.SetString(m_cfpiAssoc.strFullFldName());
    colAttrs.objAdd(adatTmp);

    // The rest are in the expression
    m_exprEval.QueryExprAttrs(colAttrs, adatTmp, kCIDLib::True);
}


tCIDLib::TVoid
TCQCIntfStateItem::RegisterFields(          TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    //
    //  Ask the field polling info object to try to register the field.
    //  Whether he succeeds or not we set our result to error until we
    //  get a good value at some point later during updates.
    //
    m_cfpiAssoc.bRegister(polleToUse, cfcData);
    m_eLastRes = tCQCIntfEng::EExprResults::Error;
}


tCIDLib::TVoid
TCQCIntfStateItem::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFullMatch
                            ,       TRegEx&                 regxFind)
{
    // If doing text, then do the comparison value of our expression
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Caption))
    {
        if (!m_exprEval.strCompVal().bIsEmpty())
        {
            TString strVal = m_exprEval.strCompVal();
            const tCIDLib::TBoolean bRes = facCQCKit().bDoSearchNReplace
            (
                strSrc, strTar, strVal, bRegEx, bFullMatch, regxFind
            );

            if (bRes)
                m_exprEval.strCompVal(strVal);
        }
    }

    // If doing fields, then do our associated field
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocField))
    {
        // Get the full field text
        TString strVal = m_cfpiAssoc.strFullFldName();
        const tCIDLib::TBoolean bRes = facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, strVal, bRegEx, bFullMatch, regxFind
        );

        if (bRes)
        {
            // Break it back apart and set it back on the poll info object
            TString strFld;
            if (strVal.bSplit(strFld, kCIDLib::chPeriod, kCIDLib::True))
                m_cfpiAssoc.SetField(strVal, strFld);
        }
    }
}


const TString& TCQCIntfStateItem::strDescription() const
{
    return m_exprEval.strDescription();
}

const TString& TCQCIntfStateItem::strDescription(const TString& strToSet)
{
    m_exprEval.strDescription(strToSet);
    return m_exprEval.strDescription();
}


const TString& TCQCIntfStateItem::strMoniker() const
{
    return m_cfpiAssoc.strMoniker();
}


const TString& TCQCIntfStateItem::strField() const
{
    return m_cfpiAssoc.strFieldName();
}


// Set up a different (or iniital if the def ctor was used) association
tCIDLib::TVoid
TCQCIntfStateItem::Set( const   TString&        strMoniker
                        , const TString&        strField
                        , const TCQCExpression& exprEval)
{
    m_exprEval = exprEval;
    m_cfpiAssoc.SetField(strMoniker, strField);
}



tCIDLib::TVoid
TCQCIntfStateItem::SetAttr(const TAttrData& adatNew, const TAttrData& adatOld)
{
    if (adatNew.strId() == kCQCIntfEng::strAttr_StateItem_Field)
        SetField(adatNew.strValue());
    else
        m_exprEval.SetExprAttr(adatNew, adatOld);
}


tCIDLib::TVoid TCQCIntfStateItem::SetField(const TString& strToSet)
{
    // Make sure it parses, else this will throw
    TString strMon, strFld;
    facCQCKit().ParseFldName(strToSet, strMon, strFld);

    m_cfpiAssoc.SetField(strMon, strFld);
}


//
//  This is called when the client wants to validate a template. We report
//  an errors that we have.
//
tCIDLib::TVoid
TCQCIntfStateItem::Validate(const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    const TString& strMon = m_cfpiAssoc.strMoniker();
    const TString& strFld = m_cfpiAssoc.strFieldName();

    TString     strErr;
    TCQCFldDef  flddState;
    if (strMon.bIsEmpty() || strFld.bIsEmpty())
    {
        // No field defined
        strErr = facCQCIntfEng().strMsg
        (
            kIEngErrs::errcVal_NoStateFld, m_exprEval.strDescription()
        );

        colErrs.objAdd
        (
            TCQCIntfValErrInfo
            (
                tCQCIntfEng::EValErrTypes::State, strErr, kCIDLib::False, TString::strEmpty()
            )
        );
    }
     else
    {
        // There is a field, so check it
        if (!cfcData.bFldExists(strMon, strFld, flddState))
        {
            strErr = facCQCIntfEng().strMsg
            (
                kIEngErrs::errcVal_BadStateFld
                , m_exprEval.strDescription()
                , strMon
                , strFld
            );

            colErrs.objAdd
            (
                TCQCIntfValErrInfo
                (
                    tCQCIntfEng::EValErrTypes::State, strErr, kCIDLib::True, TString::strEmpty()
                )
            );
        }
    }

    // Check the expression as well
    if (!m_exprEval.bValidate(strErr, flddState.eType()))
    {
        colErrs.objAdd
        (
            TCQCIntfValErrInfo
            (
                tCQCIntfEng::EValErrTypes::State, strErr, kCIDLib::False, TString::strEmpty()
            )
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfStateItem: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfStateItem::StreamFrom(TBinInStream& strmToReadFrom)
{
    // We should have a state marker at the start of our stuff
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != CQCIntfEng_States::c2ItemFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Stream in our fields
    TString strField;
    TString strMoniker;
    strmToReadFrom  >> strMoniker >> strField;

    // Bring in the expression object
    strmToReadFrom  >> m_exprEval;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Set up the poll info object.
    m_cfpiAssoc.SetField(strMoniker, strField);

    // Reset any run time only stuff
    m_eLastRes = tCQCIntfEng::EExprResults::Error;
}


tCIDLib::TVoid TCQCIntfStateItem::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_States::c2ItemFmtVersion
                    << m_cfpiAssoc.strMoniker()
                    << m_cfpiAssoc.strFieldName()
                    << m_exprEval
                    << tCIDLib::EStreamMarkers::EndObject;
}






// ---------------------------------------------------------------------------
//  CLASS: TCQCIntfState
// PREFIX: stt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TCQCIntfState: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfState::TCQCIntfState() :

    m_bNegated(kCIDLib::False)
    , m_bValue(kCIDLib::False)
    , m_eLogOp(tCQCKit::ELogOps::AND)
{
}

TCQCIntfState::TCQCIntfState(const  TString&    strName
                            , const TString&    strDescription) :

    m_bNegated(kCIDLib::False)
    , m_bValue(kCIDLib::False)
    , m_eLogOp(tCQCKit::ELogOps::AND)
    , m_strDescription(strDescription)
    , m_strName(strName)
{
}

TCQCIntfState::TCQCIntfState(const TCQCIntfState& istToCopy) :

    m_bNegated(istToCopy.m_bNegated)
    , m_bValue(istToCopy.m_bValue)
    , m_colItems(istToCopy.m_colItems)
    , m_eLogOp(istToCopy.m_eLogOp)
    , m_strDescription(istToCopy.m_strDescription)
    , m_strName(istToCopy.m_strName)
{
}

TCQCIntfState::~TCQCIntfState()
{
}


// ---------------------------------------------------------------------------
// TCQCIntfState: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfState::operator=(const TCQCIntfState& istSrc)
{
    if (this != &istSrc)
    {
        m_bNegated       = istSrc.m_bNegated;
        m_bValue         = istSrc.m_bValue;
        m_colItems       = istSrc.m_colItems;
        m_eLogOp         = istSrc.m_eLogOp;
        m_strDescription = istSrc.m_strDescription;
        m_strName        = istSrc.m_strName;
    }
}

tCIDLib::TBoolean TCQCIntfState::operator==(const TCQCIntfState& istSrc) const
{
    if (this != &istSrc)
    {
        if ((m_bNegated != istSrc.m_bNegated)
        ||  (m_bValue != istSrc.m_bValue)
        ||  (m_eLogOp != istSrc.m_eLogOp)
        ||  (m_strDescription != istSrc.m_strDescription)
        ||  (m_strName != istSrc.m_strName))
        {
            return kCIDLib::False;
        }

        const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
        if (c4Count != istSrc.m_colItems.c4ElemCount())
            return kCIDLib::False;

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colItems[c4Index] != istSrc.m_colItems[c4Index])
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCIntfState::operator!=(const TCQCIntfState& istSrc) const
{
    return !operator==(istSrc);
}


// ---------------------------------------------------------------------------
// TCQCIntfState: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIntfState::bEvaluate(       TCQCPollEngine& polleToUse
                        , const TStdVarsTar&    ctarGlobalVars)
{
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();

    // If no state items, then return false now
    if (!c4Count)
        return kCIDLib::False;

    //
    //  We have to combine the results according to the logical op, so
    //  we need to set an initial value that will cause that to work
    //  correctly, and then we'll just combine each subsequent value
    //
    tCIDLib::TBoolean bNewVal;
    if (m_eLogOp == tCQCKit::ELogOps::AND)
        bNewVal = kCIDLib::True;
    else
        bNewVal = kCIDLib::False;

    //
    //  Loop through all of the state items and ask each one to evaluate
    //  itself, passing it the value object for the field it is associated
    //  with. We do the logical operations here, and we need to do smart
    //  evaluation, such that as soon as we find a scenario that would
    //  cause a failure, we should stop evaluating and return false.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfStateItem& istiCur = m_colItems[c4Index];
        const tCQCIntfEng::EExprResults eRes = istiCur.eEvaluate
        (
            polleToUse, ctarGlobalVars
        );

        //
        //  If the value is in error, we can set false right now, since
        //  it doesn't matter what else is going on. We don't honor the
        //  negation flag either.
        //
        if (eRes == tCQCIntfEng::EExprResults::Error)
        {
            // Return true (changes) if the previous state was true
            bRet = m_bValue;
            m_bValue = kCIDLib::False;
            return bRet;
        }

        //
        //  Else, combine the result with the running new value, using
        //  the set logical operator.
        //
        if (eRes == tCQCIntfEng::EExprResults::False)
        {
            //
            //  The only thing that matters here is for an AND, in which
            //  case we can break out with a false value now. For OR or
            //  XOR, it won't have any effect.
            //
            if (m_eLogOp == tCQCKit::ELogOps::AND)
            {
                bNewVal = kCIDLib::False;
                break;
            }
        }
         else
        {
            //
            //  Don't need to do AND, since this wouldn't change it's
            //  state. Any false with an AND we fall out immediately.
            //
            if (m_eLogOp == tCQCKit::ELogOps::OR)
            {
                //
                //  If any one is true, it's going to be true, so we can
                //  break out with a true value as soon as we see one.
                //
                bNewVal = kCIDLib::True;
                break;
            }
             else if (m_eLogOp == tCQCKit::ELogOps::XOR)
            {
                //
                //  If we already saw one true one, then it cannot be
                //  true, so set a false and break out. Else, set it
                //  true to remember we saw one true so far.
                //
                if (bNewVal)
                {
                    bNewVal = kCIDLib::False;
                    break;
                }
                bNewVal = kCIDLib::True;
            }
        }
    }


    // If we are negated, then flip the result
    if (m_bNegated)
        bNewVal = !bNewVal;

    // And store the new value, remembering if this is a change
    bRet = (m_bValue != bNewVal);
    m_bValue = bNewVal;

    // Return whether we changed states
    return bRet;
}


tCIDLib::TBoolean TCQCIntfState::bNegated() const
{
    return m_bNegated;
}

tCIDLib::TBoolean TCQCIntfState::bNegated(const tCIDLib::TBoolean bToSet)
{
    m_bNegated = bToSet;
    return m_bNegated;
}


tCIDLib::TBoolean TCQCIntfState::bValue() const
{
    return m_bValue;
}


tCIDLib::TCard4 TCQCIntfState::c4AddItem(const TCQCIntfStateItem& istiToAdd)
{
    tCIDLib::TCard4 c4Ret = m_colItems.c4ElemCount();
    m_colItems.objAdd(istiToAdd);
    return c4Ret;
}


tCIDLib::TCard4 TCQCIntfState::c4ItemCount() const
{
    return m_colItems.c4ElemCount();
}


tCIDLib::TVoid TCQCIntfState::DeleteAt(const tCIDLib::TCard4 c4At)
{
    m_colItems.RemoveAt(c4At);
}

tCIDLib::TVoid TCQCIntfState::DeleteExpr(const TString& strNameToDel)
{
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfStateItem& istiCur = m_colItems[c4Index];
        if (istiCur.strDescription() == strNameToDel)
        {
            m_colItems.RemoveAt(c4Index);
            break;
        }
    }
}


// Get/set the logical operation on this state
tCQCKit::ELogOps TCQCIntfState::eLogOp() const
{
    return m_eLogOp;
}

tCQCKit::ELogOps TCQCIntfState::eLogOp(const tCQCKit::ELogOps eToSet)
{
    m_eLogOp = eToSet;
    return m_eLogOp;
}


// Get const/non-const access to a state item at a given index
const TCQCIntfStateItem& TCQCIntfState::istiAt(const tCIDLib::TCard4 c4At) const
{
    return m_colItems[c4At];
}

TCQCIntfStateItem& TCQCIntfState::istiAt(const tCIDLib::TCard4 c4At)
{
    return m_colItems[c4At];
}


// Look up a particular state item by name
TCQCIntfStateItem* TCQCIntfState::pistiFind(const TString& strNameToFind)
{
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfStateItem& istiCur = m_colItems[c4Index];
        if (istiCur.strDescription() == strNameToFind)
            return &istiCur;
    }
    return 0;
}


//
//  This is called on startup to do initial checking and registration of
//  the fields. We just pass it on to each of our state objects.
//
tCIDLib::TVoid
TCQCIntfState::RegisterFields(          TCQCPollEngine& polleToUse
                                , const TCQCFldCache&   cfcData)
{
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colItems[c4Index].RegisterFields(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfState::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                        , const TString&                strSrc
                        , const TString&                strTar
                        , const tCIDLib::TBoolean       bRegEx
                        , const tCIDLib::TBoolean       bFull
                        ,       TRegEx&                 regxFind)
{
    // Iterate the items and pass the command on
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colItems[c4Index].Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
}


// Get/set the state description
const TString& TCQCIntfState::strDescription() const
{
    return m_strDescription;
}

const TString& TCQCIntfState::strDescription(const TString& strToSet)
{
    m_strDescription = strToSet;
    return m_strDescription;
}


// Get/set the state name
const TString& TCQCIntfState::strName() const
{
    return m_strName;
}

const TString& TCQCIntfState::strName(const TString& strToSet)
{
    m_strName = strToSet;
    return m_strName;
}


//
//  This is called to do any validation of our configuration and fields and
//  so forth and report any errors.
//
tCIDLib::TVoid
TCQCIntfState::Validate(const   TCQCFldCache&           cfcData
                        ,       tCQCIntfEng::TErrList&  colErrs
                        ,       TDataSrvClient&         dsclVal) const
{
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colItems[c4Index].Validate(cfcData, colErrs, dsclVal);
}



// ---------------------------------------------------------------------------
//  TCQCIntfState: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfState::StreamFrom(TBinInStream& strmToReadFrom)
{
    // We should have a state marker at the start of our stuff
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCIntfEng_States::c2StateFmtVersion)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_bNegated
                    >> m_eLogOp
                    >> m_strName
                    >> m_strDescription
                    >> m_colItems;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfState::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_States::c2StateFmtVersion
                    << m_bNegated
                    << m_eLogOp
                    << m_strName
                    << m_strDescription
                    << m_colItems
                    << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//  CLASS: TCQCIntfStateList
// PREFIX: istl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TCQCIntfStateList: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStateList::TCQCIntfStateList() :

    m_bFirstTime(kCIDLib::True)
{
}

TCQCIntfStateList::TCQCIntfStateList(const TCQCIntfStateList& istlToCopy) :

    m_bFirstTime(kCIDLib::True)
    , m_colStates(istlToCopy.m_colStates)
{
    // We always reset the first time flag to true
}

TCQCIntfStateList::~TCQCIntfStateList()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStateList: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStateList& TCQCIntfStateList::operator=(const TCQCIntfStateList& istlSrc)
{
    // We always reset the first time flag to true
    if (this != &istlSrc)
    {
        m_bFirstTime = kCIDLib::True;
        m_colStates = istlSrc.m_colStates;
    }
    return *this;
}


tCIDLib::TBoolean
TCQCIntfStateList::operator==(const TCQCIntfStateList& istlSrc) const
{
    // We always reset the first time flag to true
    if (this != &istlSrc)
    {
        const tCIDLib::TCard4 c4Count = m_colStates.c4ElemCount();
        if (c4Count != istlSrc.m_colStates.c4ElemCount())
            return kCIDLib::False;

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colStates[c4Index] != istlSrc.m_colStates[c4Index])
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCIntfStateList::operator!=(const TCQCIntfStateList& istlSrc) const
{
    return !operator==(istlSrc);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStateList: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4
TCQCIntfStateList::c4AddState(const TCQCIntfState& istNew)
{
    const tCIDLib::TCard4 c4Ret = m_colStates.c4ElemCount();
    m_colStates.objAdd(istNew);
    return c4Ret;
}


//
//  Some change was reported by the polling engine, so the timer that watches
//  for change notifications called us with the notification. We need to
//  re-evalutae our states and store new true/false values for them all. So
//  we just pass this on to each of them.
//
tCIDLib::TBoolean
TCQCIntfStateList::bEvaluate(       TCQCPollEngine& polleToUse
                            , const TStdVarsTar&    ctarGlobalVars)
{
    //
    //  Run through all of the states and let them re-evaluate of the
    //  change was for their associated field.
    //
    tCIDLib::TBoolean bChanges = kCIDLib::False;
    const tCIDLib::TCard4 c4StateCount = m_colStates.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4StateCount; c4Index++)
    {
        // Get the current state and ask it to evaluate itself.
        TCQCIntfState& istCur = m_colStates[c4Index];
        if (istCur.bEvaluate(polleToUse, ctarGlobalVars))
            bChanges = kCIDLib::True;
    }

    // If the first time, always say we had changes
    if (m_bFirstTime)
    {
        m_bFirstTime = kCIDLib::False;
        return kCIDLib::True;
    }

    // Return whether any states changed
    return bChanges;
}


tCIDLib::TBoolean
TCQCIntfStateList::bFindState(  const   TString&            strToFind
                                ,       tCIDLib::TCard4&    c4StateInd) const
{
    const tCIDLib::TCard4 c4Count = m_colStates.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colStates[c4Index].strName() == strToFind)
        {
            c4StateInd = c4Index;
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCIntfStateList::bGetStateName(const  tCIDLib::TCard4 c4At
                                ,       TString&        strToFill) const
{
    if (c4At >= m_colStates.c4ElemCount())
        return kCIDLib::False;

    strToFill = m_colStates[c4At].strName();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfStateList::bGetStateResult( const   TIndexList&         fcolIndices
                                    , const tCQCKit::ELogOps    eLogOp) const
{
    //
    //  Create an intial state that is appropriate for the logical operator
    //  we are going to do.
    //
    tCIDLib::TBoolean bNewVal;
    if (eLogOp == tCQCKit::ELogOps::AND)
        bNewVal = kCIDLib::True;
    else
        bNewVal = kCIDLib::False;

    const tCIDLib::TCard4 c4Count = fcolIndices.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colStates[fcolIndices[c4Index]].bValue())
        {
            //
            //  Don't need to do AND, since this wouldn't change it's
            //  state. Any false with an AND we fall out immediately.
            //
            if (eLogOp == tCQCKit::ELogOps::OR)
            {
                //
                //  If any one is true, it's going to be true, so we can
                //  break out with a true value as soon as we see one.
                //
                bNewVal = kCIDLib::True;
                break;
            }
             else if (eLogOp == tCQCKit::ELogOps::XOR)
            {
                //
                //  If we already saw one true one, then it cannot be
                //  true, so set a false and break out. Else, set it
                //  true to remember we saw one true so far.
                //
                if (bNewVal)
                {
                    bNewVal = kCIDLib::False;
                    break;
                }
                bNewVal = kCIDLib::True;
            }
        }
         else
        {
            //
            //  The only thing that matters here is for an AND, in which
            //  case we can break out with a false value now. For OR or
            //  XOR, it won't have any effect.
            //
            if (eLogOp == tCQCKit::ELogOps::AND)
            {
                bNewVal = kCIDLib::False;
                break;
            }
        }
    }
    return bNewVal;
}


tCIDLib::TCard4 TCQCIntfStateList::c4StateCount() const
{
    return m_colStates.c4ElemCount();
}


// Delete a state by name or at an index
tCIDLib::TVoid TCQCIntfStateList::DeleteAt(const tCIDLib::TCard4 c4At)
{
    m_colStates.RemoveAt(c4At);
}

tCIDLib::TVoid TCQCIntfStateList::DeleteState(const TString& strToDelete)
{
    const tCIDLib::TCard4 c4Count = m_colStates.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colStates[c4Index].strName() == strToDelete)
        {
            m_colStates.RemoveAt(c4Index);
            break;
        }
    }
}


// Access a state object at the indicated index
const TCQCIntfState& TCQCIntfStateList::istAt(const tCIDLib::TCard4 c4At) const
{
    return m_colStates[c4At];
}

TCQCIntfState& TCQCIntfStateList::istAt(const tCIDLib::TCard4 c4At)
{
    return m_colStates[c4At];
}


// Find a state object with the indicated name
TCQCIntfState* TCQCIntfStateList::pistFind(const TString& strNameToFind)
{
    const tCIDLib::TCard4 c4Count = m_colStates.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfState& istCur = m_colStates[c4Index];
        if (istCur.strName() == strNameToFind)
            return &istCur;
    }
    return 0;
}


// Update the contents of a state
tCIDLib::TVoid
TCQCIntfStateList::SetStateAt(  const   tCIDLib::TCard4 c4At
                                , const TCQCIntfState&  istToSet)
{
    m_colStates[c4At] = istToSet;
}


//
//  This is called on startup to do initial checking and registration of
//  the fields. We just pass it on to each of our state objects.
//
tCIDLib::TVoid
TCQCIntfStateList::RegisterFields(          TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    const tCIDLib::TCard4 c4StateCount = m_colStates.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4StateCount; c4Index++)
        m_colStates[c4Index].RegisterFields(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfStateList::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    // We only care about field and text replacements
    static const tCQCIntfEng::ERepFlags eFlags = tCIDLib::eOREnumBits
    (
        tCQCIntfEng::ERepFlags::AssocField
        , tCQCIntfEng::ERepFlags::Caption
    );
    if (!tCIDLib::bAnyBitsOn(eToRep, eFlags))
        return;

    // Iterate the states and pass on the command to them
    const tCIDLib::TCard4 c4Count = m_colStates.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colStates[c4Index].Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
}


// Resets any information we have, to leave us in default state
tCIDLib::TVoid TCQCIntfStateList::Reset()
{
    m_colStates.RemoveAll();

    // Reset the first time flag as well
    m_bFirstTime = kCIDLib::True;
}


//
//  Looks up all of our fields and sees if they have changed in any way
//  that would invalidate the previous configuration. We just pass on the
//  call to each state object.
//
//
//  This is called to do any validation of our configuration and fields and
//  so forth and report any errors.
//
tCIDLib::TVoid
TCQCIntfStateList::Validate(const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    const tCIDLib::TCard4 c4Count = m_colStates.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colStates[c4Index].Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStateList: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfStateList::StreamFrom(TBinInStream& strmToReadFrom)
{
    // We should have a state marker at the start of our stuff
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCIntfEng_States::c2ListFmtVersion)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Read in our list of states
    strmToReadFrom  >> m_colStates;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any runtime only stuff
    m_bFirstTime = kCIDLib::True;
}


tCIDLib::TVoid TCQCIntfStateList::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_States::c2ListFmtVersion
                    << m_colStates
                    << tCIDLib::EStreamMarkers::EndObject;
}


