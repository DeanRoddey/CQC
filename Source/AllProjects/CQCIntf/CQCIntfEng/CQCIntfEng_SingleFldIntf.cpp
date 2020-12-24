//
// FILE NAME: CQCIntfEng_SingleFldIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/20/2008
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
//  This file implements a mixin that is used by all the widgets that are
//  based on a single field.
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
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_SingleFldIntf
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        //
        //  Version 2 -
        //      We added the 'no auto write' flag. We don't use it but allow
        //      classes that use us to store it and use it themselves.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfSingleFldIntf
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCIntfSingleFldIntf: Destructor
// ---------------------------------------------------------------------------
MCQCIntfSingleFldIntf::~MCQCIntfSingleFldIntf()
{
    try
    {
        delete m_pffiltToUse;
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  MCQCIntfSingleFldIntf: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  If the derived class has no special requirements as to accepting a field
//  for association, we just say yeh we'll take it.
//
tCIDLib::TBoolean
MCQCIntfSingleFldIntf::bCanAcceptField( const   TCQCFldDef&
                                        , const TString&
                                        , const TString&
                                        , const tCIDLib::TBoolean)
{
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  MCQCIntfSingleFldIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  When the mixing in widget accepts a new field to be associated with, this
//  is called to get us to store it in our field info object.
//
tCIDLib::TVoid
MCQCIntfSingleFldIntf::AssociateField(  const   TString&    strMoniker
                                        , const TString&    strField)
{
    m_cfpiAssoc.SetField(strMoniker, strField);
}


//
//  At design time our field definition info isn't set, and even the field may not have
//  been associated yet. So this is a helper to get that info at design time, if it can
//  be gotten.
//
tCIDLib::TBoolean MCQCIntfSingleFldIntf::bDesQueryFldDef(TCQCFldDef& flddTar) const
{
    // If we don't have a selected field, then obviously not
    if (!m_cfpiAssoc.bHasField())
        return kCIDLib::False;

    // Let's try to query the info
    return facCQCKit().bQueryFldDef
    (
        m_cfpiAssoc.strMoniker(), m_cfpiAssoc.strFieldName(), flddTar, kCIDLib::True
    );
}

tCIDLib::TBoolean
MCQCIntfSingleFldIntf::bDesQueryFldInfo(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    // If we don't have a selected field, then obviously not
    if (!m_cfpiAssoc.bHasField())
        return kCIDLib::False;

    TCQCFldDef flddInfo;
    const tCIDLib::TBoolean bRet = facCQCKit().bQueryFldDef
    (
        m_cfpiAssoc.strMoniker(), m_cfpiAssoc.strFieldName(), flddInfo, kCIDLib::True
    );

    if (bRet)
    {
        eType = flddInfo.eType();
        strLimits = flddInfo.strLimits();
    }
    return bRet;
}


// A convenience to check for a good field value being available
tCIDLib::TBoolean MCQCIntfSingleFldIntf::bGoodFieldValue() const
{
    return m_cfpiAssoc.bHasGoodValue();
}


// Indicates if the field has been set
tCIDLib::TBoolean MCQCIntfSingleFldIntf::bHasField() const
{
    return m_cfpiAssoc.bHasField();
}


// Get/set the no auto-write flag
tCIDLib::TBoolean MCQCIntfSingleFldIntf::bNoAutoWrite() const
{
    return m_bNoAutoWrite;
}

tCIDLib::TBoolean
MCQCIntfSingleFldIntf::bNoAutoWrite(const tCIDLib::TBoolean bToSet)
{
    m_bNoAutoWrite = bToSet;
    return m_bNoAutoWrite;
}


tCIDLib::TBoolean
MCQCIntfSingleFldIntf::bSameField(const MCQCIntfSingleFldIntf& miwdgSrc) const
{
    if ((m_bNoAutoWrite != miwdgSrc.m_bNoAutoWrite)
    ||  (m_cfpiAssoc.strFullFldName() != miwdgSrc.m_cfpiAssoc.strFullFldName()))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// We make sure that a field has been set
tCIDLib::TBoolean
MCQCIntfSingleFldIntf::bValidateField(  const   TCQCFldCache&           cfcData
                                        ,       tCQCIntfEng::TErrList&  colErrs
                                        ,       TDataSrvClient&         dsclInit
                                        , const TCQCIntfWidget&         iwdgThis) const
{
    //
    //  See if a field was ever set. If not, complain about that. If so,
    //  see if it's in the current field list. If not, complain about that.
    //
    if (m_cfpiAssoc.bHasField())
    {
        if (!cfcData.bFldExists(m_cfpiAssoc.strMoniker(), m_cfpiAssoc.strFieldName()))
        {
            TString strErr
            (
                kIEngErrs::errcVal_FldNotFound
                , facCQCIntfEng()
                , m_cfpiAssoc.strFullFldName()
            );
            colErrs.objAdd
            (
                TCQCIntfValErrInfo
                (
                    iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId()
                )
            );
            return kCIDLib::False;
        }
    }
     else
    {
        TString strErr(kIEngErrs::errcFld_NoFieldSelected, facCQCIntfEng());
        colErrs.objAdd
        (
            TCQCIntfValErrInfo
            (
                iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId()
            )
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is provided so that widgets can set their field at viewing time.
//  We use this despite it's being almost the same as AssociateField()
//  above, in case we want to in the future be able to do something
//  different when it changes dynamically at viewing time, whereas the
//  one above is a design time thing.
//
tCIDLib::TVoid MCQCIntfSingleFldIntf::ChangeField(const TString& strField)
{
    TString strMon;
    TString strFld;
    if (!facCQCKit().bParseFldName(strField, strMon, strFld))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_BadFldNameFmt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strField
        );
    }
    m_cfpiAssoc.SetField(strMon, strFld);
}


// Clears our field association
tCIDLib::TVoid MCQCIntfSingleFldIntf::ClearFieldAssoc()
{
    m_cfpiAssoc.Reset();
}


// A convenience to get the state of the field value
tCQCPollEng::EFldStates MCQCIntfSingleFldIntf::eFieldState() const
{
    return m_cfpiAssoc.eState();
}


// A convenience to get the field type
tCQCKit::EFldTypes MCQCIntfSingleFldIntf::eFieldType() const
{
    return m_cfpiAssoc.flddAssoc().eType();
}


// Return the field filter we were give
const TCQCFldFilter& MCQCIntfSingleFldIntf::ffiltToUse() const
{
    CIDAssert(m_pffiltToUse != 0, L"The field filter never got set");
    return *m_pffiltToUse;
}


//
//  Read access to our field definition. This is for viewing time. It's not set during
//  design time, just the moniker and field name are available (assuming one has been
//  set.) At design time, we have bDesQueryFldDef() to get the info if its needed during
//  design.
//
const TCQCFldDef& MCQCIntfSingleFldIntf::flddAssoc() const
{
    return m_cfpiAssoc.flddAssoc();
}


// Read access to the current value
const TCQCFldValue& MCQCIntfSingleFldIntf::fvCurrent() const
{
    return m_cfpiAssoc.fvCurrent();
}


//
//  This is called regularly to let us update our value. We just ask the
//  polling info object to update. If he indicates that something has
//  changed, then we call a virtual that any of our derivatives will
//  implement.
//
tCIDLib::TVoid
MCQCIntfSingleFldIntf::FieldUpdate(         TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        ctarGlobalVars)
{
    //
    //  Remember the current state before we change it, then ask our
    //  poling interface object to update.
    //
    const tCQCPollEng::EFldStates eOldState = m_cfpiAssoc.eState();
    if (m_cfpiAssoc.bUpdateValue(polleToUse))
    {
        //
        //  If before we were waiting for registration and now we are beyond
        //  that state, then we've found the field, so ask the containing class
        //  if he can accept it (it might have changed since the field was
        //  last seen.) If he can't, put it into reject mode. We'll stay there
        //  until something changes that might get us happy again.
        //
        if ((eOldState == tCQCPollEng::EFldStates::WaitReg)
        &&  (m_cfpiAssoc.eState() > tCQCPollEng::EFldStates::WaitReg))
        {
            if (!bCanAcceptField(m_cfpiAssoc.flddAssoc()
                                , m_cfpiAssoc.strMake()
                                , m_cfpiAssoc.strModel()
                                , kCIDLib::True))
            {
                m_cfpiAssoc.RejectField();
            }
        }

        // And tell him about the change one way or another
        FieldChanged(m_cfpiAssoc, bNoRedraw, ctarGlobalVars);
    }
}


// We just add our moniker (if set) to the list (if not already there)
tCIDLib::TVoid MCQCIntfSingleFldIntf::
QueryFieldMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    if (!m_cfpiAssoc.strMoniker().bIsEmpty())
    {
        tCIDLib::TBoolean bAdded;
        colToFill.objAddIfNew(m_cfpiAssoc.strMoniker(), bAdded);
    }
}


// Return some summary info about us
tCIDLib::TVoid
MCQCIntfSingleFldIntf::QueryFieldAttrs(         tCIDMData::TAttrList&   colAttrs
                                        ,       TAttrData&              adatTmp
                                        , const TString&                strItemText) const
{
    colAttrs.objPlace
    (
        L"Field Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    colAttrs.objPlace
    (
        strItemText
        , kCQCIntfEng::strAttr_Field
        , kCIDMData::strAttrLim_Required
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_cfpiAssoc.strFullFldName());
}


// Read in our persistent data
tCIDLib::TVoid
MCQCIntfSingleFldIntf::ReadInField(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_SingleFldIntf::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCIntfSingleFieldIntf")
        );
    }

    // Read in our field info and set up the poll info object
    TString     strField;
    TString     strMoniker;
    strmToReadFrom >> strMoniker >> strField;
    if (!strMoniker.bIsEmpty())
        m_cfpiAssoc.SetField(strMoniker, strField);

    // If V2 or above, read in the no autowrite flag, else default it
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_bNoAutoWrite;
    else
        m_bNoAutoWrite = kCIDLib::False;

    // And it should all end with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


//
//  Classes that mix us in pass on the RegisterField() call that they get
//  to us here, so that we can register our field. This is justa one shot
//  thing at first to get them all registered quickly. After that it happens
//  via the polling engine as they poll the field for changes.
//
tCIDLib::TVoid
MCQCIntfSingleFldIntf::RegisterSingleField(         TCQCPollEngine& polleToUse
                                            , const TCQCFldCache&   cfcData)
{
    //
    //  If it got registered, see if the client can accept this field. If
    //  not, then reject it. Once rejected, it'll stay that way until the
    //  poll info object sees a change that could affect the field def,
    //  which will put us back into register mode. We pass true for the
    //  'store' parameter to make the target update himself to match the
    //  field if he accepts it.
    //
    if (m_cfpiAssoc.bRegister(polleToUse, cfcData))
    {
        if (!bCanAcceptField(m_cfpiAssoc.flddAssoc()
                            , m_cfpiAssoc.strMake()
                            , m_cfpiAssoc.strModel()
                            , kCIDLib::True))
        {
            m_cfpiAssoc.RejectField();
        }
    }
}


//
//  This is called from the enclosing class when it is asked to do a moniker
//  replacement, which is used during template imports.
//
tCIDLib::TVoid
MCQCIntfSingleFldIntf::ReplaceField(const   TString&            strSrc
                                    , const TString&            strTar
                                    , const tCIDLib::TBoolean   bRegEx
                                    , const tCIDLib::TBoolean   bFull
                                    ,       TRegEx&             regxFind)
{
    // We need to do this on the full field name
    TString strTmp = m_cfpiAssoc.strFullFldName();
    if (facCQCKit().bDoSearchNReplace(strSrc, strTar, strTmp, bRegEx, bFull, regxFind))
    {
        // Make sure it parses ok, and we need the two parts to set it back again
        TString strField;
        TString strMoniker;
        if (!facCQCKit().bParseFldName(strTmp, strMoniker, strField))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_BadFldNameFmt
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , strTmp
            );
        }
        m_cfpiAssoc.SetField(strMoniker, strField);
    }
}


// Return our field name
const TString& MCQCIntfSingleFldIntf::strFieldName() const
{
    return m_cfpiAssoc.strFieldName();
}


// Return our full field name
const TString& MCQCIntfSingleFldIntf::strFullFieldName() const
{
    return m_cfpiAssoc.strFullFldName();
}


// Return our moniker
const TString& MCQCIntfSingleFldIntf::strMoniker() const
{
    return m_cfpiAssoc.strMoniker();
}


tCIDLib::TVoid
MCQCIntfSingleFldIntf::SetFieldAttr(        TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    if (adatNew.strId() == kCQCIntfEng::strAttr_Field)
    {
        // Parse out the field parts
        TString strMoniker;
        TString strFldName;
        if (facCQCKit().bParseFldName(adatNew.strValue(), strMoniker, strFldName))
            m_cfpiAssoc.SetField(strMoniker, strFldName);
    }
}


// Set a new (or initial) field filter
tCIDLib::TVoid
MCQCIntfSingleFldIntf::SetFieldFilter(TCQCFldFilter* const pffiltToAdopt)
{
    delete m_pffiltToUse;
    m_pffiltToUse = pffiltToAdopt;
}



// Stream out our persistent data
tCIDLib::TVoid
MCQCIntfSingleFldIntf::WriteOutField(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEng_SingleFldIntf::c2FmtVersion;

    // Do our stuff and the end marker
    strmToWriteTo   << m_cfpiAssoc.strMoniker()
                    << m_cfpiAssoc.strFieldName()
                    << m_bNoAutoWrite
                    << tCIDLib::EStreamMarkers::Frame;
}



// ---------------------------------------------------------------------------
//  MCQCIntfSingleFldIntf: Hidden constructors and operators
// ---------------------------------------------------------------------------
MCQCIntfSingleFldIntf::MCQCIntfSingleFldIntf() :

    m_bNoAutoWrite(kCIDLib::False)
    , m_pffiltToUse(0)
{
}

MCQCIntfSingleFldIntf::MCQCIntfSingleFldIntf(TCQCFldFilter* const pffiltToAdopt) :

    m_bNoAutoWrite(kCIDLib::False)
    , m_pffiltToUse(pffiltToAdopt)
{
}

MCQCIntfSingleFldIntf::
MCQCIntfSingleFldIntf(const MCQCIntfSingleFldIntf& iwdgToCopy) :

    m_bNoAutoWrite(iwdgToCopy.m_bNoAutoWrite)
    , m_cfpiAssoc(iwdgToCopy.m_cfpiAssoc)
    , m_pffiltToUse(::pDupObject<TCQCFldFilter>(iwdgToCopy.m_pffiltToUse))
{
}

MCQCIntfSingleFldIntf&
MCQCIntfSingleFldIntf::operator=(const MCQCIntfSingleFldIntf& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        m_bNoAutoWrite  = iwdgToAssign.m_bNoAutoWrite;
        m_cfpiAssoc     = iwdgToAssign.m_cfpiAssoc;

        // Clean up any current filter and copy the incoming one if any
        delete m_pffiltToUse;
        m_pffiltToUse = 0;
        if (iwdgToAssign.m_pffiltToUse)
            m_pffiltToUse = ::pDupObject<TCQCFldFilter>(iwdgToAssign.m_pffiltToUse);
    }
    return *this;
}


