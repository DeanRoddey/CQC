//
// FILE NAME: CQCIntfEng_MultiFldIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/26/2012
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
//  based on more than one field.
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
namespace
{
    namespace CQCIntfEng_MultiFldIntf
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfMultiFldIntf::TDFldInfo
//  PREFIX: dfldi
// ---------------------------------------------------------------------------
MCQCIntfMultiFldIntf::TDFldInfo::TDFldInfo() :

    m_pffiltToUse(0)
{
}

MCQCIntfMultiFldIntf::TDFldInfo::
TDFldInfo(const MCQCIntfMultiFldIntf::TDFldInfo& dfldiSrc) :

    m_cfpiAssoc(dfldiSrc.m_cfpiAssoc)
    , m_pffiltToUse(0)
{
    // Dup the source filter if there is one
    if (dfldiSrc.m_pffiltToUse)
        m_pffiltToUse = ::pDupObject<TCQCFldFilter>(dfldiSrc.m_pffiltToUse);
}

MCQCIntfMultiFldIntf::TDFldInfo& MCQCIntfMultiFldIntf::TDFldInfo::
operator=(const MCQCIntfMultiFldIntf::TDFldInfo& dfldiSrc)
{
    if (&dfldiSrc != this)
    {
        m_cfpiAssoc = dfldiSrc.m_cfpiAssoc;
        delete m_pffiltToUse;
        m_pffiltToUse = 0;

        if (dfldiSrc.m_pffiltToUse)
            m_pffiltToUse = ::pDupObject<TCQCFldFilter>(dfldiSrc.m_pffiltToUse);
    }
    return *this;
}

MCQCIntfMultiFldIntf::TDFldInfo::~TDFldInfo()
{
    try
    {
        delete m_pffiltToUse;
    }

    catch(...)
    {
    }
    m_pffiltToUse = 0;
}




// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfMultiFldIntf
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCIntfMultiFldIntf: Destructor
// ---------------------------------------------------------------------------
MCQCIntfMultiFldIntf::~MCQCIntfMultiFldIntf()
{
}


// ---------------------------------------------------------------------------
//  MCQCIntfMultiFldIntf: Public, virtual methods
// ---------------------------------------------------------------------------


//
//  If the derived class has no special requirements as to accepting a field
//  for association, we just say we accept. If they need more, they should
//  override.
//
tCIDLib::TBoolean
MCQCIntfMultiFldIntf::bCanAcceptField( const    TCQCFldDef&
                                        , const tCIDLib::TCard4
                                        , const TString&
                                        , const TString&
                                        , const tCIDLib::TBoolean)
{
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  MCQCIntfMultiFldIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  When the mixing in widget accepts a new field to be associated with, this
//  is called to get us to store it in our field info object.
//
tCIDLib::TVoid
MCQCIntfMultiFldIntf::AssociateField(const  TString&        strMoniker
                                    , const TString&        strField
                                    , const tCIDLib::TCard4 c4FldInd)
{
    m_objaFlds[c4FldInd].m_cfpiAssoc.SetField(strMoniker, strField);
}


// A convenience to check for a good field value being available
tCIDLib::TBoolean
MCQCIntfMultiFldIntf::bGoodFieldValue(const tCIDLib::TCard4 c4FldInd) const
{
    return m_objaFlds[c4FldInd].m_cfpiAssoc.bHasGoodValue();
}


// Indicates if the field has been set
tCIDLib::TBoolean
MCQCIntfMultiFldIntf::bHasField(const tCIDLib::TCard4 c4FldInd) const
{
    return m_objaFlds[c4FldInd].m_cfpiAssoc.bHasField();
}


tCIDLib::TBoolean
MCQCIntfMultiFldIntf::bSameFields(const MCQCIntfMultiFldIntf& miwdgSrc) const
{
    const tCIDLib::TCard4 c4Count = m_objaFlds.tElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_objaFlds[c4Index].m_cfpiAssoc.strFullFldName()
                        != miwdgSrc.m_objaFlds[c4Index].m_cfpiAssoc.strFullFldName())
        {
            return kCIDLib::False;
        }
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
tCIDLib::TVoid
MCQCIntfMultiFldIntf::ChangeField(  const   TString&        strField
                                    , const tCIDLib::TCard4 c4FldInd)
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
    m_objaFlds[c4FldInd].m_cfpiAssoc.SetField(strMon, strFld);
}


// Clears our field association
tCIDLib::TVoid
MCQCIntfMultiFldIntf::ClearFieldAssoc(const tCIDLib::TCard4 c4FldInd)
{
    m_objaFlds[c4FldInd].m_cfpiAssoc.Reset();
}


// A convenience to get the state of the field value
tCQCPollEng::EFldStates
MCQCIntfMultiFldIntf::eFieldState(const tCIDLib::TCard4 c4FldInd) const
{
    return m_objaFlds[c4FldInd].m_cfpiAssoc.eState();
}


// A convenience to get the field type
tCQCKit::EFldTypes
MCQCIntfMultiFldIntf::eFieldType(const tCIDLib::TCard4 c4FldInd) const
{
    return m_objaFlds[c4FldInd].m_cfpiAssoc.flddAssoc().eType();
}


// Return the field filter we were give
const TCQCFldFilter&
MCQCIntfMultiFldIntf::ffiltToUse(const tCIDLib::TCard4 c4FldInd) const
{
    CIDAssert(m_objaFlds[c4FldInd].m_pffiltToUse != 0, L"The field filter never got set");
    return *m_objaFlds[c4FldInd].m_pffiltToUse;
}


// Read access to our field definitions
const TCQCFldDef&
MCQCIntfMultiFldIntf::flddAssoc(const tCIDLib::TCard4 c4FldInd) const
{
    return m_objaFlds[c4FldInd].m_cfpiAssoc.flddAssoc();
}


// Read access to the current value
const TCQCFldValue&
MCQCIntfMultiFldIntf::fvCurrent(const tCIDLib::TCard4 c4FldInd) const
{
    return m_objaFlds[c4FldInd].m_cfpiAssoc.fvCurrent();
}


//
//  This is called regularly to let us update our values. We just ask the
//  polling info object to update. If he indicates that something has
//  changed, then we call a virtual that any of our derivatives will
//  implement.
//
tCIDLib::TVoid
MCQCIntfMultiFldIntf::FieldUpdate(          TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        ctarGlobalVars)
{
    const tCIDLib::TCard4 c4Count = m_objaFlds.tElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TDFldInfo& dfldiCur = m_objaFlds[c4Index];

        // If this oen has no field, nothing to do
        if (!dfldiCur.m_cfpiAssoc.bHasField())
            continue;

        //
        //  Remember the current state before we change it, then ask our
        //  poling interface object to update.
        //
        const tCQCPollEng::EFldStates eOldState = dfldiCur.m_cfpiAssoc.eState();
        if (dfldiCur.m_cfpiAssoc.bUpdateValue(polleToUse))
        {
            //
            //  If before we were waiting for registration and now we are beyond
            //  that state, then we've found the field, so ask the containing class
            //  if he can accept it (it might have changed since the field was
            //  last seen.) If he can't, put it into reject mode. We'll stay there
            //  until something changes that might get us happy again.
            //
            if ((eOldState == tCQCPollEng::EFldStates::WaitReg)
            &&  (dfldiCur.m_cfpiAssoc.eState() > tCQCPollEng::EFldStates::WaitReg))
            {
                if (!bCanAcceptField(dfldiCur.m_cfpiAssoc.flddAssoc()
                                    , c4Index
                                    , dfldiCur.m_cfpiAssoc.strMake()
                                    , dfldiCur.m_cfpiAssoc.strModel()
                                    , kCIDLib::True))
                {
                    dfldiCur.m_cfpiAssoc.RejectField();
                }
            }

            // And tell him about the change one way or another
            FieldChanged(dfldiCur.m_cfpiAssoc, c4Index, bNoRedraw, ctarGlobalVars);
        }
    }
}


// We just add our moniker (if set) to the list (if not already there)
tCIDLib::TVoid MCQCIntfMultiFldIntf::
QueryFieldMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    const tCIDLib::TCard4 c4Count = m_objaFlds.tElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TDFldInfo& dfldiCur = m_objaFlds[c4Index];

        // If it has a field, add it to the list
        if (dfldiCur.m_cfpiAssoc.bHasField())
        {
            tCIDLib::TBoolean bAdded;
            colToFill.objAddIfNew(dfldiCur.m_cfpiAssoc.strMoniker(), bAdded);
        }
    }
}


//
//  We format our fields out out to a binary blob.
//
//  FOR NOW, we do have a couple widgets that use this mixin, but they directly set their
//  fields based on other info, the user doesn't select them. So, for now, we don't put this
//  into the attribute editor.
//
tCIDLib::TVoid
MCQCIntfMultiFldIntf::QueryMFieldAttrs( tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
#if 0
    adatTmp.Set(L"Field Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Field List"
        , kCQCIntfEng::strAttr_Fields
        , tCIDMData::EAttrTypes::Binary
        , tCIDMData::EAttrEdTypes::Visual
    );

    // Flatten out our data
    tCIDLib::TCard4 c4Bytes;
    THeapBuf mbufData(16 * 1024UL);
    {
        TBinMBufOutStream strmOut(&mbufData);
        WriteOutFields(strmOut);
        strmOut.Flush();
        c4Bytes = strmOut.c4CurPos();
    }

    adatTmp.SetValue(mbufData, c4Bytes);
    colAttrs.objAdd(adatTmp);
#endif
}


// Read in our persistent data
tCIDLib::TVoid
MCQCIntfMultiFldIntf::ReadInFields(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MultiFldIntf::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCIntfMultiFieldIntf")
        );
    }

    // Read in the count and XOR'd count and validate them
    tCIDLib::TCard4 c4Count, c4XORCount;
    strmToReadFrom >> c4Count >> c4XORCount;
    if (c4Count != (c4XORCount ^ kCIDLib::c4MaxCard))
    {
    }

    // Reallocate our array
    m_objaFlds.Realloc(c4Count);

    // Read in our field info and set up the poll info objects
    TString     strField;
    TString     strMoniker;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToReadFrom >> strMoniker >> strField;

        if (!strMoniker.bIsEmpty())
            m_objaFlds[c4Index].m_cfpiAssoc.SetField(strMoniker, strField);
    }

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
MCQCIntfMultiFldIntf::RegisterMultiFields(          TCQCPollEngine& polleToUse
                                            , const TCQCFldCache&   cfcData)
{
    const tCIDLib::TCard4 c4Count = m_objaFlds.tElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TDFldInfo& dfldiCur = m_objaFlds[c4Index];

        // If this one has no field, then skip it
        if (!dfldiCur.m_cfpiAssoc.bHasField())
            continue;

        //
        //  If it got registered, see if the client can accept this field. If
        //  not, then reject it. Once rejected, it'll stay that way until the
        //  poll info object sees a change that could affect the field def,
        //  which will put us back into register mode. We pass true for the
        //  'store' parameter to make the target update himself to match the
        //  field if he accepts it.
        //
        if (dfldiCur.m_cfpiAssoc.bRegister(polleToUse, cfcData))
        {
            if (!bCanAcceptField(dfldiCur.m_cfpiAssoc.flddAssoc()
                                , c4Index
                                , dfldiCur.m_cfpiAssoc.strMake()
                                , dfldiCur.m_cfpiAssoc.strModel()
                                , kCIDLib::True))
            {
                dfldiCur.m_cfpiAssoc.RejectField();
            }
        }
    }
}


//
//  This is called from the enclosing class when it is asked to do a moniker
//  replacement, which is used during template imports.
//
tCIDLib::TVoid
MCQCIntfMultiFldIntf::ReplaceMultiFields(const  TString&            strSrc
                                        , const TString&            strTar
                                        , const tCIDLib::TBoolean   bRegEx
                                        , const tCIDLib::TBoolean   bFull
                                        ,       TRegEx&             regxFind)
{
    const tCIDLib::TCard4 c4Count = m_objaFlds.tElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TDFldInfo& dfldiCur = m_objaFlds[c4Index];

        // We need to do this on the full field name
        TString strTmp = dfldiCur.m_cfpiAssoc.strFullFldName();
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
            dfldiCur.m_cfpiAssoc.SetField(strMoniker, strField);
        }
    }
}


// A convenience to get to the full field name
const TString&
MCQCIntfMultiFldIntf::strFieldName(const tCIDLib::TCard4 c4FldInd) const
{
    return m_objaFlds[c4FldInd].m_cfpiAssoc.strFieldName();
}


// A convenience to get to the moniker of a field
const TString&
MCQCIntfMultiFldIntf::strMoniker(const tCIDLib::TCard4 c4FldInd) const
{
    return m_objaFlds[c4FldInd].m_cfpiAssoc.strMoniker();
}


tCIDLib::TVoid
MCQCIntfMultiFldIntf::SetMFieldAttr(        TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    if (adatNew.strId() == kCQCIntfEng::strAttr_Fields)
    {
        // Stream out the data back to us again
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        ReadInFields(strmSrc);
    }
}


// Set a new (or initial) field filter
tCIDLib::TVoid
MCQCIntfMultiFldIntf::SetFieldFilter(       TCQCFldFilter* const    pffiltToAdopt
                                    , const tCIDLib::TCard4         c4FldInd)
{
    TDFldInfo& dfldiCur = m_objaFlds[c4FldInd];
    delete dfldiCur.m_pffiltToUse;
    dfldiCur.m_pffiltToUse = pffiltToAdopt;
}


// We make sure that our fields have been set
tCIDLib::TVoid
MCQCIntfMultiFldIntf::ValidateFields(const  TCQCFldCache&           cfcData
                                    ,       tCQCIntfEng::TErrList&  colErrs
                                    ,       TDataSrvClient&         dsclVal
                                    , const TCQCIntfWidget&         iwdgThis) const
{
    const tCIDLib::TCard4 c4Count = m_objaFlds.tElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TDFldInfo& dfldiCur = m_objaFlds[c4Index];

        //
        //  See if a field was set. If so, see if it's in the current field list.
        //  If not, complain about that.
        //
        if (dfldiCur.m_cfpiAssoc.bHasField())
        {
            if (!cfcData.bFldExists(dfldiCur.m_cfpiAssoc.strMoniker()
                                    , dfldiCur.m_cfpiAssoc.strFieldName()))
            {
                TString strErr
                (
                    kIEngErrs::errcVal_FldNotFound
                    , facCQCIntfEng()
                    , dfldiCur.m_cfpiAssoc.strFullFldName()
                );
                colErrs.objAdd
                (
                    TCQCIntfValErrInfo
                    (
                        iwdgThis.c4UniqueId()
                        , strErr
                        , kCIDLib::True
                        , iwdgThis.strWidgetId()
                    )
                );
            }
        }
        #if 0
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
        }
        #endif
    }
}


// Stream out our persistent data
tCIDLib::TVoid
MCQCIntfMultiFldIntf::WriteOutFields(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEng_MultiFldIntf::c2FmtVersion;

    const tCIDLib::TCard4 c4Count = m_objaFlds.tElemCount();
    strmToWriteTo   << c4Count << (c4Count ^ kCIDLib::c4MaxCard);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToWriteTo   << m_objaFlds[c4Index].m_cfpiAssoc.strMoniker()
                        << m_objaFlds[c4Index].m_cfpiAssoc.strFieldName();
    }
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame;
}



// ---------------------------------------------------------------------------
//  MCQCIntfMultiFldIntf: Hidden constructors and operators
// ---------------------------------------------------------------------------
MCQCIntfMultiFldIntf::MCQCIntfMultiFldIntf(const tCIDLib::TCard4 c4Count) :

    m_objaFlds(c4Count)
{
}


MCQCIntfMultiFldIntf::
MCQCIntfMultiFldIntf(const MCQCIntfMultiFldIntf& iwdgSrc) :

    m_objaFlds(iwdgSrc.m_objaFlds)
{
}

MCQCIntfMultiFldIntf&
MCQCIntfMultiFldIntf::operator=(const MCQCIntfMultiFldIntf& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        m_objaFlds = iwdgSrc.m_objaFlds;
    }
    return *this;
}



