//
// FILE NAME: CQCIntfEng_CmdButton.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/26/2005
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
//  This file implements a derivative of the push button class, which in this
//  case invokes a configured set of CQC commands.
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
#include    "CQCIntfEng_PushButton.hpp"
#include    "CQCIntfEng_CmdButton.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfCmdButton,TCQCIntfPushButton)
AdvRTTIDecls(TCQCIntfFldCmdButton, TCQCIntfCmdButton)
AdvRTTIDecls(TCQCIntfVarCmdButton, TCQCIntfCmdButton)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_CmdButton
    {
        // -----------------------------------------------------------------------
        //  The base class format version
        //
        //  Version 2 -
        //      We added support the Xlats mixin
        //
        //  Version 3 -
        //      We no longer support on press/release, only Onlick, so move any
        //      commands from press release to click.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion        = 3;

        // -----------------------------------------------------------------------
        //  The field based format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FldFmtVersion     = 1;

        // -----------------------------------------------------------------------
        //  The variable based format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2VarFmtVersion     = 1;


        // -----------------------------------------------------------------------
        //  Out capabilities flags
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::TakesFocus
        );


        constexpr tCQCIntfEng::ECapFlags eVarCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::TakesFocus
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCmdButton
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfCmdButton: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfCmdButton::TCQCIntfCmdButton() :

    TCQCIntfPushButton
    (
        CQCIntfEng_CmdButton::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_CmdButton)
    )
{
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnClick)
        , kCQCKit::strEvId_OnClick
        , tCQCKit::EActCmdCtx::Standard
    );

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/CommandButton");
}

TCQCIntfCmdButton::TCQCIntfCmdButton(const  tCQCIntfEng::ECapFlags  eCapFlags
                                    , const TString&                strTypeName) :

    TCQCIntfPushButton(eCapFlags, strTypeName)
{
    // Add the events that we support
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnClick)
        , kCQCKit::strEvId_OnClick
        , tCQCKit::EActCmdCtx::Standard
    );

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/CommandButton");
}

TCQCIntfCmdButton::TCQCIntfCmdButton(const TCQCIntfCmdButton& iwdgSrc) :

    TCQCIntfPushButton(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
{
}

TCQCIntfCmdButton::~TCQCIntfCmdButton()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfCmdButton: Public operators
// ---------------------------------------------------------------------------
TCQCIntfCmdButton&
TCQCIntfCmdButton::operator=(const TCQCIntfCmdButton& iwdgSrc)
{
    if (this == &iwdgSrc)
        return *this;

    TParent::operator=(iwdgSrc);
    MCQCCmdSrcIntf::operator=(iwdgSrc);

    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfCmdButton: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfCmdButton::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfCmdButton& iwdgOther
    (
        static_cast<const TCQCIntfCmdButton&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCCmdSrcIntf::bSameCmds(iwdgOther))
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  We were clicked on. The point doesn't matter since we have no internal
//  structure. This means that being clicked and being invoked does the
//  same thing. So we just call the Invoke() method.
//
tCIDLib::TVoid TCQCIntfCmdButton::Clicked(const TPoint&)
{
    Invoke();
}


tCIDLib::TVoid
TCQCIntfCmdButton::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfCmdButton&>(iwdgSrc);
}


tCIDLib::TVoid TCQCIntfCmdButton::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  At this level, the text is just our caption text. A derived class
    //  may do this again after calling us, but this gets it right for our
    //  level.
    //
    SetText(TString::strEmpty(), kCIDLib::True, kCIDLib::False);
}


//
//  We had the focus and the user invoked us. In our case, this is the same
//  as the Clicked() method above, since we have no internal structure so
//  clicking or hitting enter on us is the same.
//
tCIDLib::TVoid TCQCIntfCmdButton::Invoke()
{
    // Do click feedback
    DoClickFeedback();

    // Post any on click action
    PostOps(*this, kCQCKit::strEvId_OnClick, tCQCKit::EActOrders::NonNested);

    //
    //  Don't do anything else unless you check that we weren't killed by
    //  this action.
    //
}


tCIDLib::TVoid
TCQCIntfCmdButton::QueryWdgAttrs(tCIDMData::TAttrList& colAttrs, TAttrData& adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins. Our command is required
    QueryCmdAttrs(colAttrs, adatTmp, kCIDLib::True, kCIDLib::True, L"Actions");
}


// Handle search and replace on our content
tCIDLib::TVoid
TCQCIntfCmdButton::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);
}


tCIDLib::TVoid
TCQCIntfCmdButton::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // Call our parent first
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    SetCmdAttr(adatNew, adatOld);

    //
    //  We have a special case here, in that our parent push button window doesn't
    //  directly use the caption, because it also has to deal with dynamic
    //  derivatives. So we override this just so that we can call the appropriate
    //  set text method on our parent. This makes sure we see the newly set text
    //  at design time.
    //
    if (adatNew.strId() == kCQCIntfEng::strAttr_Widget_Caption)
        SetText(strCaption(), kCIDLib::True, kCIDLib::False);
}


//
//  Validate our content, which currently doesn't require anything at our
//  level so we just pass it through.
//
tCIDLib::TVoid
TCQCIntfCmdButton::Validate(const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent for now
    TParent::Validate(cfcData, colErrs, dsclVal);
}



// ---------------------------------------------------------------------------
//  TCQCIntfCmdButton: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfCmdButton::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // And check the version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_CmdButton::c2FmtVersion))
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

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // Do any mixins
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfCmdButton::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_CmdButton::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // We don't have any of own fields at this time

    // And end up with an end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldCmdButton
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldCmdButton: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldCmdButton::TCQCIntfFldCmdButton() :

    TCQCIntfCmdButton
    (
        CQCIntfEng_CmdButton::eVarCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_CmdButton)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
{
}

TCQCIntfFldCmdButton::TCQCIntfFldCmdButton(const TCQCIntfFldCmdButton& iwdgSrc) :

    TCQCIntfCmdButton(iwdgSrc)
    , MCQCIntfSingleFldIntf(iwdgSrc)
{
}

TCQCIntfFldCmdButton::~TCQCIntfFldCmdButton()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldCmdButton: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldCmdButton&
TCQCIntfFldCmdButton::operator=(const TCQCIntfFldCmdButton& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfSingleFldIntf::operator=(iwdgSrc);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldCmdButton: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfFldCmdButton::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldCmdButton& iwdgOther
    (
        static_cast<const TCQCIntfFldCmdButton&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfSingleFldIntf::bSameField(iwdgOther))
        return kCIDLib::False;

    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfFldCmdButton::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldCmdButton&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldCmdButton::eDoCmd(const  TCQCCmdCfg&             ccfgToDo
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


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfFldCmdButton::QueryCmds(        TCollection<TCQCCmd>&   colCmds
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
tCIDLib::TVoid TCQCIntfFldCmdButton::
QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldCmdButton::QueryWdgAttrs(tCIDMData::TAttrList& colAttrs, TAttrData& adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"State Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldCmdButton::RegisterFields(       TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}



tCIDLib::TVoid TCQCIntfFldCmdButton::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Initialize the text on our parent class. Put it into error state
    //  until we get good data set.
    //
    SetText(TString::strEmpty(), kCIDLib::True, kCIDLib::True);
}


tCIDLib::TVoid
TCQCIntfFldCmdButton::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    // If replacing field names, pass along to the field interface
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocField))
        MCQCIntfSingleFldIntf::ReplaceField(strSrc, strTar, bRegEx, bFull, regxFind);
}


tCIDLib::TVoid
TCQCIntfFldCmdButton::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // Call our parent first
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfFldCmdButton::Validate(const    TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent for now
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Call any mixins
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldCmdButton: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  If our field mixin sees a change in our state or value, then we need
//  to update, so he'll call us here. We pass the text on to the base push
//  button class to let it update the text it's going to display.
//
tCIDLib::TVoid
TCQCIntfFldCmdButton::FieldChanged(         TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();

    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        TString strNewVal;
        fvCurrent().Format(strNewVal);
        SetText(strNewVal, bNoRedraw, kCIDLib::False);
    }
     else
    {
        // We are in error state so show error text
        SetText(facCQCIntfEng().strErrText(), bNoRedraw, kCIDLib::True);
    }
}



tCIDLib::TVoid TCQCIntfFldCmdButton::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // And check the version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_CmdButton::c2FldFmtVersion))
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

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // Stream in any mixins
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // We don't have any of own persistent data at this time

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
TCQCIntfFldCmdButton::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_CmdButton::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any of own persistent data at this time

    // And end up with an end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}



// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldCmdButton::ValueUpdate(          TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarCmdButton
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarCmdButton: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarCmdButton::TCQCIntfVarCmdButton() :

    TCQCIntfCmdButton
    (
        CQCIntfEng_CmdButton::eVarCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_CmdButton)
    )
    , MCQCIntfVarIntf()
{
}

TCQCIntfVarCmdButton::TCQCIntfVarCmdButton(const TCQCIntfVarCmdButton& iwdgSrc) :

    TCQCIntfCmdButton(iwdgSrc)
    , MCQCIntfVarIntf(iwdgSrc)
{
}

TCQCIntfVarCmdButton::~TCQCIntfVarCmdButton()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarCmdButton: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarCmdButton&
TCQCIntfVarCmdButton::operator=(const TCQCIntfVarCmdButton& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfVarIntf::operator=(iwdgSrc);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarCmdButton: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfVarCmdButton::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarCmdButton& iwdgOther
    (
        static_cast<const TCQCIntfVarCmdButton&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfVarIntf::bSameVariable(iwdgOther))
        return kCIDLib::False;

    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfVarCmdButton::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarCmdButton&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarCmdButton::eDoCmd(const  TCQCCmdCfg&             ccfgToDo
                            , const tCIDLib::TCard4         c4Index
                            , const TString&                strUserId
                            , const TString&                strEventId
                            ,       TStdVarsTar&            ctarGlobals
                            ,       tCIDLib::TBoolean&      bResult
                            ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LinkToVar)
    {
        SetVarInfo(ccfgToDo.piAt(0).m_strValue);
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


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfVarCmdButton::QueryCmds(        TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We just support the changing of the variable
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_LinkToVar
            , facCQCKit().strMsg(kKitMsgs::midCmd_LinkToVar)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );
}



tCIDLib::TVoid TCQCIntfVarCmdButton::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Initialize the text on our parent class. Put it into error state
    //  until we get good data set.
    //
    SetText(TString::strEmpty(), kCIDLib::True, kCIDLib::True);
}


tCIDLib::TVoid
TCQCIntfVarCmdButton::QueryWdgAttrs(tCIDMData::TAttrList& colAttrs, TAttrData& adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarCmdButton::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    // Do variable names if indicated
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocVar))
        MCQCIntfVarIntf::ReplaceVar(strSrc, strTar, bRegEx, bFull, regxFind);
}


tCIDLib::TVoid
TCQCIntfVarCmdButton::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // Call our parent first
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarCmdButton::Validate(const    TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent for now
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarCmdButton: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  When our variable shows up, our variable mixin will call us here to see
//  if we can accept it, so that we can set ourselves up to match it's
//  type and limits.
//
//  For us, we just say yeh, because we can handle any field type.
//
tCIDLib::TBoolean TCQCIntfVarCmdButton::bCanAcceptVar(const TCQCActVar&)
{
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCIntfVarCmdButton::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // And check the version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_CmdButton::c2VarFmtVersion))
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

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // Stream in any mixins
    MCQCIntfVarIntf::ReadInVar(strmToReadFrom);

    // We don't have any info of our own right now

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
TCQCIntfVarCmdButton::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_CmdButton::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // We don't have any of own fields at this time

    // And end up with an end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}



// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfVarCmdButton::ValueUpdate(          TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean   bVarsUpdate
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    //
    //  If variables have changed, pass it on the variables mixin to update
    //  the value.
    //
    if (bVarsUpdate)
        VarUpdate(ctarGlobalVars, bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the variable state
//  changes out of ready state, to let us know we need to update our display
//  to indicate we don't have a good value.
//
tCIDLib::TVoid
TCQCIntfVarCmdButton::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    SetText(TString::strEmpty(), bNoRedraw, kCIDLib::True);
}



//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that we are in ready state, either
//  already were or have just transitioned into it.
//
tCIDLib::TVoid
TCQCIntfVarCmdButton::VarValChanged(const   TCQCActVar&         varNew
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        )
{
    // Tell our parent class about it
    SetText(varNew.strValue(), bNoRedraw, kCIDLib::False);
}


