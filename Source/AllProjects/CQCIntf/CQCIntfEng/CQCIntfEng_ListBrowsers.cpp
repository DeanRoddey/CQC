//
// FILE NAME: CQCIntfEng_ListBrowsers.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/28/2005
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
//  This file implements some h/v list browser derivatives, for fields,
//  static lists, etc...
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
AdvRTTIDecls(TCQCIntfFldListBrowser,TCQCIntfHVBrowserBase)
AdvRTTIDecls(TCQCIntfStaticListBrowser,TCQCIntfHVBrowserBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_ListBrowsers
    {
        // -----------------------------------------------------------------------
        //  Our field based persistent storage format version
        //
        //  Version 2 -
        //      We moved field support from the base widget class to a mixin, so
        //      we need to know which way to stream in that stuff, so we bumped
        //      the version. In reality we will not see V1 objects anymore post
        //      3.0.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FldFmtVersion  = 2;


        // -----------------------------------------------------------------------
        //  Our static list persistent storage format version
        //
        //  Version 2 -
        //      We bumped the version because we got rid of an intervening parent
        //      class in 2.4.15. So we now handle reading in its data and toss it
        //      alway on write back out.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2StaticFmtVersion  = 2;


        // -----------------------------------------------------------------------
        //  The caps flags for field based lists
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eFldCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::OptionalAct
            , tCQCIntfEng::ECapFlags::TakesFocus
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );


        // -----------------------------------------------------------------------
        //  The caps flags for static based lists
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eStaticCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::OptionalAct
            , tCQCIntfEng::ECapFlags::TakesFocus
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldListBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldListBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldListBrowser::TCQCIntfFldListBrowser() :

    TCQCIntfHVBrowserBase
    (
        CQCIntfEng_ListBrowsers::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_ListBrowser)
    )
    , MCQCIntfSingleFldIntf
      (
          new TCQCFldFiltTypeName
          (
              tCQCKit::EReqAccess::MReadCWrite, tCQCKit::EFldTypes::StringList
          )
      )
{
}

TCQCIntfFldListBrowser::
TCQCIntfFldListBrowser(const TCQCIntfFldListBrowser& iwdgToCopy) :

    TCQCIntfHVBrowserBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
    , m_colNames(iwdgToCopy.m_colNames)
{
}

TCQCIntfFldListBrowser::~TCQCIntfFldListBrowser()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldListBrowser: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldListBrowser&
TCQCIntfFldListBrowser::operator=(const TCQCIntfFldListBrowser& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
        m_colNames  = iwdgToAssign.m_colNames;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldListBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Call to let us indicate whether we can accept a new field and to optionally
//  update ourself to match it. We just need to check that it's a string list
//  field and don't need to do anything to update ourself.
//
tCIDLib::TBoolean
TCQCIntfFldListBrowser::bCanAcceptField(const   TCQCFldDef& fldTest
                                        , const TString&
                                        , const TString&
                                        , const tCIDLib::TBoolean)
{
    return fldTest.eType() == tCQCKit::EFldTypes::StringList;
}


tCIDLib::TBoolean TCQCIntfFldListBrowser::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldListBrowser& iwdgOther(static_cast<const TCQCIntfFldListBrowser&>(iwdgSrc));

    // We just add the field mixin
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


tCIDLib::TVoid
TCQCIntfFldListBrowser::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldListBrowser&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldListBrowser::eDoCmd( const   TCQCCmdCfg&             ccfgToDo
                                , const tCIDLib::TCard4         c4Index
                                , const TString&                strUserId
                                , const TString&                strEventId
                                ,       TStdVarsTar&            ctarGlobals
                                ,       tCIDLib::TBoolean&      bResult
                                ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LinkToField)
    {
        // Just pass the field name on to our field mixin
        ChangeField(ccfgToDo.piAt(0).m_strValue);
    }
     else
    {
        // Not one of ours, so let our parent try it
        return TParent::eDoCmd
        (
            ccfgToDo
            , c4Index
            , strUserId
            , strEventId
            , ctarGlobals
            , bResult
            , acteTar
        );
    }
    return tCQCKit::ECmdRes::Ok;
}


// For now, we just call our parent
tCIDLib::TVoid
TCQCIntfFldListBrowser::Initialize( TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


//
//  Gen up one of our runtime value objects. We give it the text of the
//  selected item, if any.
//
TCQCCmdRTVSrc*
TCQCIntfFldListBrowser::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    const tCIDLib::TCard4 c4Sel = c4SelectedIndex();
    return new TCQCIntfHVBrwRTV
    (
        (c4Sel == kCIDLib::c4MaxCard) ? TString::strEmpty() : m_colNames[c4Sel]
        , c4Sel
        , kCQCIntfEng::strRTVId_ListItemText
        , kCQCKit::strRTVId_SelectIndex
        , cuctxToUse
    );
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfFldListBrowser::QueryCmds(          TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We just support the changing of the field
    {
        TCQCCmd& cmdLinkToFld = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_LinkToField
                , facCQCKit().strMsg(kKitMsgs::midCmd_LinkToField)
                , tCQCKit::ECmdPTypes::Field
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            )
        );

        cmdLinkToFld.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::MReadCWrite);
    }
}


// Report any monikers we refrence
tCIDLib::TVoid
TCQCIntfFldListBrowser::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldListBrowser::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    // Get our parent's attributes
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Str.List Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldListBrowser::RegisterFields(         TCQCPollEngine& polleToUse
                                        , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldListBrowser::Replace(const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocField))
        MCQCIntfSingleFldIntf::ReplaceField(strSrc, strTar, bRegEx, bFull, regxFind);
}


tCIDLib::TVoid
TCQCIntfFldListBrowser::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    // Do the base attrs
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


//
//  Called to ask us to validate any configuration we have. At this level,
//  we call our parent, then validate the field mixin
//
tCIDLib::TVoid
TCQCIntfFldListBrowser::Validate(const  TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Call any mixins
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfFldListBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  If our field mixin sees a change in our state or value, then we need
//  to update, so he'll call us here.
//
tCIDLib::TVoid
TCQCIntfFldListBrowser::FieldChanged(       TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();

    // Clear the list. If we have a good value, then get the new values
    m_colNames.RemoveAll();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        // Get the actual field value type
        const TCQCStrListFldValue& fvStrList
        (
            dynamic_cast<const TCQCStrListFldValue&>(fvCurrent())
        );

        // Now we can get the list of values out
        TCQCStrListFldValue::TValList::TCursor cursList(&fvStrList.colValue());

        // Copy these over to our list
        if (cursList.bReset())
        {
            do
            {
                m_colNames.objAdd(cursList.objRCur());
            }   while(cursList.bNext());
        }
    }

    // Tell the base class that we've reloaded our list
    ReloadedList();

    //
    //  If not hidden, then update with our new contents. We always just
    //  invalidate and cause a full redraw, because this data will tend
    //  to change seldomly and there is the issue of the new list having
    //  fewer values than the old, and therefore we have to clean out the
    //  now unused slots to the right.
    //
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}


// Stream in and out or class
tCIDLib::TVoid TCQCIntfFldListBrowser::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FldFmtVersion;
    strmToReadFrom  >> c2FldFmtVersion;
    if (!c2FldFmtVersion || (c2FldFmtVersion > CQCIntfEng_ListBrowsers::c2FldFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FldFmtVersion)
            , clsThis()
        );
    }

    // Call our parent class
    TParent::StreamFrom(strmToReadFrom);

    //
    //  In theory version 1 objects would require some adjustment to move over
    //  to the single field mixing scheme for association of fields, but we will
    //  never see such versions anymore. They cannot upgrade to anything post
    //  3.0 until they go to 3.0 first and it does all the refactoring and
    //  conversion.
    //
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Clear the list
    m_colNames.RemoveAll();
}

tCIDLib::TVoid
TCQCIntfFldListBrowser::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format verson
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_ListBrowsers::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any of our own stuff right now, so just do the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldListBrowser::ValueUpdate(        TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldListBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Lets the base horz list browser class know how many display values we have
//  available.
//
tCIDLib::TCard4 TCQCIntfFldListBrowser::c4NameCount() const
{
    return m_colNames.c4ElemCount();
}


//
//  Lets the base horz list browser class get access to our display values
//  when he needs them.
//
const TString&
TCQCIntfFldListBrowser::strDisplayTextAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colNames[c4Index];
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticListRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------
class TCQCIntfStaticListRTV : public TCQCIntfHVBrwRTV
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticListRTV() = delete;

        TCQCIntfStaticListRTV
        (
            const   TString&                strItemText
            , const tCIDLib::TCard4         c4Index
            , const TString&                strTextKey
            , const TString&                strIndexKey
            , const TString&                strUserData
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfStaticListRTV(const TCQCIntfStaticListRTV&) = delete;

        ~TCQCIntfStaticListRTV();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticListRTV& operator=(const TCQCIntfStaticListRTV&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue
        (
            const   TString&                strId
            ,       TString&                strToFill
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strUserData
        //      The user can set some context info on each item. This wil be
        //      set to the value of the user data for the selected item.
        // -------------------------------------------------------------------
        TString         m_strUserData;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticListRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfHVBrwRTV: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticListRTV::
TCQCIntfStaticListRTV(  const   TString&        strItemText
                        , const tCIDLib::TCard4 c4SelectIndex
                        , const TString&        strTextKey
                        , const TString&        strIndexKey
                        , const TString&        strUserData
                        , const TCQCUserCtx&    cuctxToUse) :

    TCQCIntfHVBrwRTV
    (
        strItemText
        , c4SelectIndex
        , strTextKey
        , strIndexKey
        , cuctxToUse
    )
    , m_strUserData(strUserData)
{
}

TCQCIntfStaticListRTV::~TCQCIntfStaticListRTV()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfHVBrwRTV: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIntfStaticListRTV::bRTValue(const TString& strId,  TString& strToFill) const
{
    // Call our parent first
    if (TCQCIntfHVBrwRTV::bRTValue(strId, strToFill))
        return kCIDLib::True;

    // If it's one of ours, then handle it
    if (strId == kCQCIntfEng::strRTVId_UserData)
    {
        strToFill = m_strUserData;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticListBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticListBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticListBrowser::TCQCIntfStaticListBrowser() :

    TCQCIntfHVBrowserBase
    (
        CQCIntfEng_ListBrowsers::eStaticCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_ListBrowser)
    )
{
    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_UserData)
        , kCQCIntfEng::strRTVId_UserData
        , tCQCKit::ECmdPTypes::Text
    );
}

TCQCIntfStaticListBrowser::
TCQCIntfStaticListBrowser(const TCQCIntfStaticListBrowser& iwdgToCopy) :

    TCQCIntfHVBrowserBase(iwdgToCopy)
    , m_colValues(iwdgToCopy.m_colValues)
{
}

TCQCIntfStaticListBrowser::~TCQCIntfStaticListBrowser()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticListBrowser: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticListBrowser&
TCQCIntfStaticListBrowser::operator=(const TCQCIntfStaticListBrowser& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        m_colValues  = iwdgToAssign.m_colValues;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticListBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfStaticListBrowser::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticListBrowser& iwdgOther
    (
        static_cast<const TCQCIntfStaticListBrowser&>(iwdgSrc)
    );

    // For now, nothing else
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfStaticListBrowser::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticListBrowser&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfStaticListBrowser::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                                    , const tCIDLib::TCard4     c4Index
                                    , const TString&            strUserId
                                    , const TString&            strEventId
                                    ,       TStdVarsTar&        ctarGlobals
                                    ,       tCIDLib::TBoolean&  bResult
                                    ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetUserData)
    {
        // The index is the first parm
        const tCIDLib::TCard4 c4PIndex = ccfgToDo.piAt(0).c4Val(tCIDLib::ERadices::Auto);

        // If the index is good, get the user data and return true, else false
        if (c4PIndex < m_colUserData.c4ElemCount())
        {
            const TString& strKey = ccfgToDo.piAt(1).m_strValue;
            TCQCActVar& varText = TStdVarsTar::varFind
            (
                strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );
            if (varText.bSetValue(m_colUserData[c4PIndex]) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varText.strName(), varText.strValue());
            bResult = kCIDLib::True;
        }
         else
        {
            bResult = kCIDLib::False;
            if (acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActDebug(L"Bad browser index, no user data returned");
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetBrowserList)
    {
        // Parse out the list values and store them
        tCIDLib::TCard4 c4ErrIndex;
        const tCIDLib::TBoolean bRes = TStringTokenizer::bParseQuotedCommaList
        (
            ccfgToDo.piAt(0).m_strValue, m_colValues, c4ErrIndex
        );

        if (!bRes)
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcVal_BadQList
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4ErrIndex)
            );
        }

        // Reset the user data list to the same size with all empty values
        m_colUserData.RemoveAll();
        m_colUserData.AddXCopies(TString::strEmpty(), m_colValues.c4ElemCount());

        // Tell our parent class that we reloaded our list
        ReloadedList();

        // Redraw with the new contents if not on preload or hidden
        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if ((ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetBrowserList2)
          ||  (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetDblBrowserList))

    {
        //
        //  We either will get two separate quoted comma lists as separate
        //  parameters, or we'll get one parameter that holds two such lists,
        //  separated by a new line.
        //
        const TString* pstrVals = 0;
        const TString* pstrData = 0;
        TString strVals;
        TString strData;
        if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetBrowserList2)
        {
            // We got
            pstrVals = &ccfgToDo.piAt(0).m_strValue;
            pstrData = &ccfgToDo.piAt(1).m_strValue;
        }
         else
        {
            // Break out the two separate lines to locals
            TTextStringInStream strmLists(&ccfgToDo.piAt(0).m_strValue);
            strmLists.c4GetLine(strVals);
            strmLists.c4GetLine(strData);

            pstrVals = &strVals;
            pstrData = &strData;
        }

        // OK, we have the strings, so parse them out to temp lists
        tCIDLib::TStrList colVals;
        tCIDLib::TStrList colData;
        tCIDLib::TCard4 c4ErrIndex;
        const tCIDLib::TBoolean bLRes = TStringTokenizer::bParseQuotedCommaList
        (
            *pstrVals, colVals, c4ErrIndex
        );

        // If the first one worked, try the second one
        tCIDLib::TBoolean bURes = kCIDLib::False;
        if (bLRes)
        {
            bURes = TStringTokenizer::bParseQuotedCommaList
            (
                *pstrData, colData, c4ErrIndex
            );
        }

        // If either failed, then throw
        if (!bLRes || !bURes)
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcVal_BadQList2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , bLRes ? TString(L"user data") : TString(L"visible values")
                , TCardinal(c4ErrIndex)
            );
        }

        // If the lists aren't the same size, then that's bad
        if (m_colValues.c4ElemCount() != m_colUserData.c4ElemCount())
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcVal_ListSizeMatch
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        //
        //  It worked so store the results and tell our parent class that we
        //  reloaded our list.
        //
        m_colValues = colVals;
        m_colUserData = colData;
        ReloadedList();

        // Redraw with the new contents
        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if ((ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetFromEnumVar)
          ||  (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetFromEnumFld))
    {
        const TString& strName = ccfgToDo.piAt(0).m_strValue;

        // It's either a variable or a field
        if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetFromEnumVar)
        {
            // Look up the variable
            const TCQCActVar& varSrc = TStdVarsTar::varFind
            (
                strName
                , acteTar.ctarLocals()
                , ctarGlobals
                , CID_FILE
                , CID_LINE
                , kCIDLib::False
            );

            // Make sure it's an enumerated type
            if (!varSrc.bQueryEnumVals(m_colValues))
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCmd_MustBeEnum
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , strName
                );
            }
        }
         else
        {
            // Break out the separate parts of he field name
            TString strMon;
            TString strFld;
            facCQCKit().ParseFldName(strName, strMon, strFld);

            // Get the CQCServer proxy taht hosts this driver
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strMon);

            // Now query the field info
            TCQCFldDef flddSrc;
            orbcSrv->QueryFieldDef(strMon, strFld, flddSrc);

            // Make sure it's an enum limit
            const TString& strLims = flddSrc.strLimits();
            if (!strLims.bStartsWith(L"Enum"))
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCmd_MustBeEnum
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , strName
                );
            }

            //
            //  It is the right type, so create a limit object and let it
            //  parse out the values.
            //
            TCQCFldEnumLimit fldlTmp;
            tCIDLib::IgnoreRet(fldlTmp.bParseLimits(strLims));

            // And now query them into our list
            fldlTmp.c4QueryValues(m_colValues);
        }

        // Reset the user data list to the same size with all empty values
        m_colUserData.RemoveAll();
        m_colUserData.AddXCopies(TString::strEmpty(), m_colValues.c4ElemCount());

        // Tell our parent class that we reloaded our list
        ReloadedList();

        // Redraw with the new contents
        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetText)
    {
        // Just store the first param on the item at the second parm
        const tCIDLib::TCard4 c4PIndex = ccfgToDo.piAt(1).m_strValue.c4Val();
        m_colValues[c4PIndex] = ccfgToDo.piAt(0).m_strValue;

        // Redraw this item if it's visible
        if (bCanRedraw(strEventId))
            RedrawItem(c4PIndex);
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetUserData)
    {
        // Just store the first param on the item at the second parm
        const tCIDLib::TCard4 c4PIndex = ccfgToDo.piAt(1).m_strValue.c4Val();
        m_colUserData[c4PIndex] = ccfgToDo.piAt(0).m_strValue;
    }
     else
    {
        // Not one of ours, so let our parent try it
        return TParent::eDoCmd
        (
            ccfgToDo
            , c4Index
            , strUserId
            , strEventId
            , ctarGlobals
            , bResult
            , acteTar
        );
    }

    return tCQCKit::ECmdRes::Ok;
}



tCIDLib::TVoid TCQCIntfStaticListBrowser::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Nothing of our own for now
}


//
//  Gen up one of our runtime value objects. We give it the text of the
//  selected item, if any.
//
TCQCCmdRTVSrc*
TCQCIntfStaticListBrowser::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    const tCIDLib::TCard4 c4Sel = c4SelectedIndex();
    return new TCQCIntfStaticListRTV
    (
        (c4Sel == kCIDLib::c4MaxCard) ? TString::strEmpty() : m_colValues[c4Sel]
        , c4Sel
        , kCQCIntfEng::strRTVId_ListItemText
        , kCQCKit::strRTVId_SelectIndex
        , (c4Sel == kCIDLib::c4MaxCard) ? TString::strEmpty() : m_colUserData[c4Sel]
        , cuctxToUse
    );
}


// Return our list of supported commands
tCIDLib::TVoid
TCQCIntfStaticListBrowser::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Get the user data of given slot
    {
        TCQCCmd& cmdGetText = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetUserData
                , facCQCKit().strMsg(kKitMsgs::midCmd_GetUserData)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
        cmdGetText.eType(tCQCKit::ECmdTypes::Both);
    }

    // Load up the list with items, no user data
    {
        TCQCCmd& cmdSetList = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetBrowserList
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetBrowserList)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_List)
            )
        );

        // Don't make the parameter required, so it can set an empty list
        cmdSetList.cmdpAt(0).bRequired(kCIDLib::False);
    }

    //
    //  Let's the set list items and the user data in one shot. Both are
    //  comma separated quoted strings.
    //
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetBrowserList2
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetBrowserList2)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_List)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_UserData)
        )
    );

    //
    //  Simliar to above but the two lists are in one block of text, with
    //  the two lists being new line separated. This allows driver to return
    //  the visible and user data lists atomically in one shot, which may
    //  often be a requirement.
    //
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetDblBrowserList
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetDblBrowserList)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
        )
    );

    //
    //  Sets the list values from the enumerated values of an enumerated
    //  variable or an enumerated field.
    //
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetFromEnumVar
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetFromEnumVar)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );

    {
        TCQCCmd& cmdSetFrom = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetFromEnumFld
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetFromEnumFld)
                , tCQCKit::ECmdPTypes::Field
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            )
        );
        cmdSetFrom.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::ReadOrWrite);
    }

    // Set the text on a paritcular list item by index
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetText
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetText)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
        )
    );

    // Set the user data on a particular list item by index
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetUserData
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetUserData)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
        )
    );
}


tCIDLib::TVoid
TCQCIntfStaticListBrowser::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                        , TAttrData&            adatTmp) const
{
    // Get our parent's attributes
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Nothing else here for now
}


tCIDLib::TVoid
TCQCIntfStaticListBrowser::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                        , const TAttrData&      adatNew
                                        , const TAttrData&      adatOld)
{
    // Do the base attrs
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticListBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

// Stream our type in and out
tCIDLib::TVoid TCQCIntfStaticListBrowser::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2StaticFmtVersion;
    strmToReadFrom  >> c2StaticFmtVersion;
    if (!c2StaticFmtVersion || (c2StaticFmtVersion > CQCIntfEng_ListBrowsers::c2StaticFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2StaticFmtVersion)
            , clsThis()
        );
    }

    // Call our parent class
    TParent::StreamFrom(strmToReadFrom);

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid
TCQCIntfStaticListBrowser::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format verson
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_ListBrowsers::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // We don't have any of our own stuff right now, so just do the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//  TCQCIntfStaticListBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Lets the base horz list browser class know how many display values we have
//  available.
//
tCIDLib::TCard4 TCQCIntfStaticListBrowser::c4NameCount() const
{
    return m_colValues.c4ElemCount();
}


//
//  Lets the base horz list browser class get access to our display values
//  when he needs them.
//
const TString&
TCQCIntfStaticListBrowser::strDisplayTextAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colValues[c4Index];
}




