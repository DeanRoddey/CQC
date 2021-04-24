//
// FILE NAME: CQCIntfEng_Template.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/16/2001
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
//  This file implements the class that represents a drawn interface. So it
//  is both the top level widget (it derives from TCQCIntfWidget) and it
//  contains members and methods that apply to the interface as a whole.
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
RTTIDecls(TCQCIntfTemplate,TCQCIntfContainer)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_Template
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version.
        //
        //  Version 1 -
        //      The version number sequence was reset for the 2.0 release, since
        //      all data is being explicitly upgraded by the installer and we will
        //      never see any old data.
        //
        //  Version 2 -
        //      Added the load/unload WAV file paths.
        //
        //  Version 3 -
        //      Got rid of the focus colors since there are new focus mechanism
        //      now and they aren't used anymore.
        //
        //  Version 4 -
        //      Added support for a list of triggered events.
        //
        //  Version 5 -
        //      The old Xlats that used to be stored on the container class are now
        //      moves up to here and are implemented in terms of action commands
        //      on an OnPreload event. So we have to pull them up from the base
        //      class and convert them in that case.
        //
        //  Version 6 -
        //      Added new template level actions, so we have to know whether to
        //      stream those in, or just default the list.
        //
        //  Version 7 -
        //      Add a comments field so that the designer of the template can
        //      leave him/herself notes.
        //
        //  Version 8 -
        //      Add support for hot key mapped actions.
        //
        //  Version 9 -
        //      Addition of a new hot key action
        //
        //  Version 10 -
        //      Adding the 'dynamic resize' flag
        //
        //  Version 11 -
        //      And got rid of the dynamic resize flag again, since we ended up going
        //      with a static pre-scaling scheme instead.
        //
        //  Version 12 - 4.7.900
        //      We changed how grouping works, from a group id to a group count that
        //      is set on the first item in the group. Any ungrouped widget (or non-
        //      first group members) are set to 1. So we need to update from the old
        //      scheme to the new upon load.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 12;


        // -----------------------------------------------------------------------
        //  The capabilities flags that templates use. Note that we enable the
        //  transparent background option, but it's ignored except in popup
        //  templates.
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::OptionalAct
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTemplate
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfTemplate: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfTemplate::TCQCIntfTemplate() :

    TCQCIntfContainer
    (
        TString::strEmpty()
        , TArea(0, 0, 640, 480)
        , CQCIntfEng_Template::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Template)
    )
    , MCQCIntfImgIntf(kCIDLib::False)
    , m_bActivityBasedTO(kCIDLib::True)
    , m_bNoFocus(kCIDLib::True)
    , m_bUserFlag(kCIDLib::False)
    , m_c1ConstOpacity(0xFF)
    , m_c4AppData(0)
    , m_c4HandlerId(0)
    , m_c4NextGroupId(1)
    , m_c4TimeoutSecs(0)
    , m_colHotKeyActions()
    , m_enctLastTimeout(TTime::enctNow())
    , m_eMinRole(tCQCKit::EUserRoles::LimitedUser)
    , m_eType(tCQCIntfEng::ETmplTypes::Std)
{
    //
    //  Add our supported events. The exit event (used when popups are closed)
    //  has the same limitations as preload, so we use that context.
    //
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPreload)
        , kCQCKit::strEvId_OnPreload
        , tCQCKit::EActCmdCtx::Preload
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnLoad)
        , kCQCKit::strEvId_OnLoad
        , tCQCKit::EActCmdCtx::NoContChange
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnTimeout)
        , kCQCKit::strEvId_OnTimeout
        , tCQCKit::EActCmdCtx::Standard
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnExit)
        , kCQCKit::strEvId_OnExit
        , tCQCKit::EActCmdCtx::Preload
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnExtClick)
        , kCQCKit::strEvId_OnExtClick
        , tCQCKit::EActCmdCtx::Standard
    );


    //
    //  Initialize all of the entries in the hot keys list. We add a single
    //  event to them which will be invoked when a hot key is received.
    //
    tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
    while (eKey < tCQCIntfEng::EHotKeys::Count)
    {
        TCQCStdCmdSrc& csrcCur = m_colHotKeyActions.objAdd(TCQCStdCmdSrc());
        csrcCur.AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnTrigger)
            , kCQCKit::strEvId_OnTrigger
            , tCQCKit::EActCmdCtx::Standard
        );
        eKey++;
    }

    // Set up our default look
    rgbBgn(kCQCIntfEng_::rgbDef_Bgn);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Template");
}

TCQCIntfTemplate::TCQCIntfTemplate(const TString& strName) :

    TCQCIntfContainer
    (
        strName
        , TArea(0, 0, 640, 480)
        , CQCIntfEng_Template::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Template)
    )
    , MCQCIntfImgIntf(kCIDLib::False)
    , m_bActivityBasedTO(kCIDLib::True)
    , m_bNoFocus(kCIDLib::True)
    , m_bUserFlag(kCIDLib::False)
    , m_c1ConstOpacity(0xFF)
    , m_c4AppData(0)
    , m_c4HandlerId(0)
    , m_c4NextGroupId(1)
    , m_c4TimeoutSecs(0)
    , m_colHotKeyActions()
    , m_enctLastTimeout(TTime::enctNow())
    , m_eMinRole(tCQCKit::EUserRoles::LimitedUser)
    , m_eType(tCQCIntfEng::ETmplTypes::Std)
{
    //
    //  Add our supported events. The exit event (used when popups are closed)
    //  has the same limitations as preload, so we use that context.
    //
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPreload)
        , kCQCKit::strEvId_OnPreload
        , tCQCKit::EActCmdCtx::Preload
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnLoad)
        , kCQCKit::strEvId_OnLoad
        , tCQCKit::EActCmdCtx::NoContChange
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnTimeout)
        , kCQCKit::strEvId_OnTimeout
        , tCQCKit::EActCmdCtx::Standard
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnExit)
        , kCQCKit::strEvId_OnExit
        , tCQCKit::EActCmdCtx::Preload
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnExtClick)
        , kCQCKit::strEvId_OnExtClick
        , tCQCKit::EActCmdCtx::Standard
    );


    //
    //  Initialize all of the entries in the hot keys list. We add a single
    //  OnTrigger event to them which will be invoked when a hot key is received.
    //
    {
        tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
        TCQCStdCmdSrc csrcDef;
        csrcDef.AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnTrigger)
            , kCQCKit::strEvId_OnTrigger
            , tCQCKit::EActCmdCtx::Standard
        );
        while (eKey < tCQCIntfEng::EHotKeys::Count)
        {
            TCQCStdCmdSrc& csrcCur = m_colHotKeyActions.objAdd(csrcDef);
            eKey++;
        }
    }

    // Set up our default look
    rgbBgn(kCQCIntfEng_::rgbDef_Bgn);

    // Set the action command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Template");
}

TCQCIntfTemplate::TCQCIntfTemplate(const TCQCIntfTemplate& iwdgSrc) :

    TCQCIntfContainer(iwdgSrc)
    , MCQCIntfImgIntf(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , m_bActivityBasedTO(iwdgSrc.m_bActivityBasedTO)
    , m_bNoFocus(iwdgSrc.m_bNoFocus)
    , m_bUserFlag(iwdgSrc.m_bUserFlag)
    , m_c1ConstOpacity(iwdgSrc.m_c1ConstOpacity)
    , m_c4AppData(iwdgSrc.m_c4AppData)
    , m_c4HandlerId(iwdgSrc.m_c4HandlerId)
    , m_c4NextGroupId(iwdgSrc.m_c4NextGroupId)
    , m_c4TimeoutSecs(iwdgSrc.m_c4TimeoutSecs)
    , m_colActions(iwdgSrc.m_colActions)
    , m_colHotKeyActions(iwdgSrc.m_colHotKeyActions)
    , m_colTrgEvents(iwdgSrc.m_colTrgEvents)
    , m_enctLastTimeout(iwdgSrc.m_enctLastTimeout)
    , m_eMinRole(iwdgSrc.m_eMinRole)
    , m_eType(iwdgSrc.m_eType)
    , m_strNotes(iwdgSrc.m_strNotes)
    , m_strLoadWAV(iwdgSrc.m_strLoadWAV)
    , m_strUnloadWAV(iwdgSrc.m_strUnloadWAV)
{
}

TCQCIntfTemplate::~TCQCIntfTemplate()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfTemplate: Public operators
// ---------------------------------------------------------------------------
TCQCIntfTemplate& TCQCIntfTemplate::operator=(const TCQCIntfTemplate& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgIntf::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);

        m_bActivityBasedTO  = iwdgSrc.m_bActivityBasedTO;
        m_bNoFocus          = iwdgSrc.m_bNoFocus;
        m_bUserFlag         = iwdgSrc.m_bUserFlag;
        m_c1ConstOpacity    = iwdgSrc.m_c1ConstOpacity;
        m_c4AppData         = iwdgSrc.m_c4AppData;
        m_c4HandlerId       = iwdgSrc.m_c4HandlerId;
        m_c4NextGroupId     = iwdgSrc.m_c4NextGroupId;
        m_c4TimeoutSecs     = iwdgSrc.m_c4TimeoutSecs;
        m_colActions        = iwdgSrc.m_colActions;
        m_colHotKeyActions  = iwdgSrc.m_colHotKeyActions;
        m_colTrgEvents      = iwdgSrc.m_colTrgEvents;
        m_enctLastTimeout   = iwdgSrc.m_enctLastTimeout;
        m_eMinRole          = iwdgSrc.m_eMinRole;
        m_eType             = iwdgSrc.m_eType;
        m_strNotes          = iwdgSrc.m_strNotes;
        m_strLoadWAV        = iwdgSrc.m_strLoadWAV;
        m_strUnloadWAV      = iwdgSrc.m_strUnloadWAV;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfTemplate: Public, inherited methods
// ---------------------------------------------------------------------------

// We can size to as long as we have some content
tCIDLib::TBoolean TCQCIntfTemplate::bCanSizeTo() const
{
    return kCIDLib::True;
}


//
//  We override this from our parent class. We call our parent first to see
//  if any nested overlay has a handler. If not, we see if we do, or if we
//  have an Otherwise clause.
//
tCIDLib::TBoolean
TCQCIntfTemplate::bHandleHotKey(const tCQCIntfEng::EHotKeys eKey)
{
    // See if a nested overlay has a handler. If so, we are done
    if (TParent::bHandleHotKey(eKey))
        return kCIDLib::True;

    // Just in case, make sure it's legal for our list of hot keys
    if (tCIDLib::TCard4(eKey) >= m_colHotKeyActions.c4ElemCount())
        return kCIDLib::False;

    // See if we have this key configured
    TCQCStdCmdSrc& csrcKey = m_colHotKeyActions[eKey];
    if (csrcKey.bUsed())
    {
        // It's defined so let's post it and return true
        PostOps(csrcKey, kCQCKit::strEvId_OnTrigger, tCQCKit::EActOrders::NonNested);
        return kCIDLib::True;
    }

    // We don't. See if we have an Otherwise (which is in the None slot)
    TCQCStdCmdSrc& csrcOther = m_colHotKeyActions[tCQCIntfEng::EHotKeys::None];
    if (csrcOther.bUsed())
    {
        // It's defined so let's post it and return true
        PostOps(csrcOther, kCQCKit::strEvId_OnTrigger, tCQCKit::EActOrders::NonNested);
        return kCIDLib::True;
    }

    // Never found anything
    return kCIDLib::False;
}


//
//  We override this from our parent class. This is called to give us a chance
//  to process an incoming triggered event before passing it on for recursive
//  processing.
//
tCIDLib::TBoolean
TCQCIntfTemplate::bInvokeTrgEvents( const   tCIDLib::TBoolean   bIsNight
                                    , const tCQCKit::TCQCEvPtr& cptrEv
                                    , const tCIDLib::TCard4     c4Hour
                                    , const tCIDLib::TCard4     c4Minute
                                    ,       TTrigEvCmdTarget&   ctarEvent)
{
    //
    //  We just call a helper on our parent class to do the work, passing it
    //  our list of triggered events, if any.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (!m_colTrgEvents.bIsEmpty())
        bRet = bRunTrgEvents(m_colTrgEvents, bIsNight, cptrEv, c4Hour, c4Minute, ctarEvent);

    // And now pass it on to our parent to continue recursion
    if (TParent::bInvokeTrgEvents(bIsNight, cptrEv, c4Hour, c4Minute, ctarEvent))
        bRet = kCIDLib::True;

    return bRet;
}


tCIDLib::TBoolean TCQCIntfTemplate::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfTemplate& iwdgOther(static_cast<const TCQCIntfTemplate&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfImgIntf::bSameImg(iwdgOther)
    ||  !MCQCCmdSrcIntf::bSameCmds(iwdgOther))
    {
        return kCIDLib::False;
    }

    // Do our simple bits, and a simple test for hte action lists
    if ((m_bNoFocus         != iwdgOther.m_bNoFocus)
    || (m_c1ConstOpacity    != iwdgOther.m_c1ConstOpacity)
    || (m_eMinRole          != iwdgOther.m_eMinRole)
    || (m_strNotes          != iwdgOther.m_strNotes)
    || (m_strLoadWAV        != iwdgOther.m_strLoadWAV)
    || (m_strUnloadWAV      != iwdgOther.m_strUnloadWAV)
    || (m_colActions.c4ElemCount() != iwdgOther.m_colActions.c4ElemCount())
    || (m_colHotKeyActions.c4ElemCount() != iwdgOther.m_colHotKeyActions.c4ElemCount())
    || (m_colTrgEvents.c4ElemCount() != iwdgOther.m_colTrgEvents.c4ElemCount()))
    {
        return kCIDLib::False;
    }

    // We survived that so compare teha ctual actions
    tCIDLib::TCard4 c4Count;

    c4Count = m_colActions.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colActions[c4Index] != iwdgOther.m_colActions[c4Index])
            return kCIDLib::False;
    }


    tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
    for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
    {
        if (m_colHotKeyActions[eKey] != iwdgOther.m_colHotKeyActions[eKey])
            return kCIDLib::False;
    }

    c4Count = m_colTrgEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colTrgEvents[c4Index] != iwdgOther.m_colTrgEvents[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Call our parent container first to distribute to child widgets. If not, then we'll
//  check our specialized actions.
//
tCIDLib::TBoolean TCQCIntfTemplate::bReferencesTarId(const TString& strId) const
{
    if (TParent::bReferencesTarId(strId))
        return kCIDLib::True;

    // Check ourself. We implement command source for onpreload, onload, etc..
    if (MCQCCmdSrcIntf::bReferencesTarId(strId))
        return kCIDLib::True;

    tCIDLib::TCard4 c4Count = m_colActions.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colActions[c4Index].bReferencesTarId(strId))
            return kCIDLib::True;
    }


    tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
    for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
    {
        if (m_colHotKeyActions[eKey].bReferencesTarId(strId))
            return kCIDLib::True;
    }

    c4Count = m_colTrgEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colTrgEvents[c4Index].bReferencesTarId(strId))
            return kCIDLib::True;
    }

    return kCIDLib::False;
}


tCIDLib::TVoid
TCQCIntfTemplate::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfTemplate&>(iwdgSrc);
}


//
//  Some commands will be remapped at viewing time from the interface window
//  target to us here, only at viewing time. So they are configured on him,
//  but we process them.
//
//  We do this re-mapping in our initialization call.
//
tCQCKit::ECmdRes
TCQCIntfTemplate::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                        , const tCIDLib::TCard4     // c4CmdIndex
                        , const TString&            strUserId
                        , const TString&            strEventId
                        ,       TStdVarsTar&        ctarGlobals
                        ,       tCIDLib::TBoolean&  bResult
                        ,       TCQCActEngine&      acteTar)
{
    tCQCKit::ECmdRes eRet = tCQCKit::ECmdRes::Ok;

    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_LoadSiblingOverlay)
    {
        //
        //  This is an error. This is only callable when the template is
        //  loaded into an overlay.
        //
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcTmpl_InvalidActCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , kCQCIntfEng::strCmdId_LoadSiblingOverlay
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_RunTmplAction)
    {
        const TString& strTitle = ccfgToDo.piAt(0).m_strValue;

        //
        //  Look up the list of template level actions and see if the
        //  indicated one is present. If so, invoke it as a nested action.
        //
        const tCIDLib::TCard4 c4Count = m_colActions.c4ElemCount();
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            if (m_colActions[c4Index].strTitle() == strTitle)
                break;
            c4Index++;
        }

        // If we found it, invoke it
        if (c4Index < c4Count)
        {
            civOwner().PostOps
            (
                *this
                , m_colActions[c4Index]
                , kCQCKit::strEvId_OnTrigger
                , iwdgRootTemplate().c4HandlerId()
                , tCQCKit::EActOrders::Nested
            );
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetTimeout)
    {
        //
        //  Convert the value to seconds and store it on ourselves as the
        //  auto-timeout period.
        //
        m_c4TimeoutSecs = ccfgToDo.piAt(0).m_strValue.c4Val();

        // Adjust if it's stupid
        if (m_c4TimeoutSecs)
        {
            if (m_c4TimeoutSecs < 2)
                m_c4TimeoutSecs = 2;
            else if (m_c4TimeoutSecs > 600)
                m_c4TimeoutSecs = 600;
        }

        // Get the activity based flag
        m_bActivityBasedTO = facCQCKit().bCheckBoolVal(ccfgToDo.piAt(1).m_strValue);

        //
        //  Reset the last timeout stamp so we start counting from here if
        //  non-activity based.
        //
        m_enctLastTimeout = TTime::enctNow();
    }
     else
    {
        //
        //  Since this is a remapped command, if it's not one we handle,
        //  then that's obviously an error
        //
        ThrowUnknownCmd(ccfgToDo);
    }
    return eRet;
}


//
//  We just call our parent first, to recursively initialize all of our
//  contained child widgets. Then if we have any background image, we
//  load it up by calling the backgroung image mixin.
//
tCIDLib::TVoid
TCQCIntfTemplate::Initialize(TCQCIntfContainer* const   piwdgParent
                            , TDataSrvClient&           dsclInit
                            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Load any background image we have. The image is handled by the
    //  image mixin, so we just delegate to it.
    //
    InitImage(civOwner(), dsclInit, colErrs, *this);

    //
    //  Now that the image stuff is initiallized, set up pre-scaling on our image.
    //  The size will end up getting changed again likely as we are sized to our
    //  final size, but just in case we need to set it up now.
    //
    SetImgPrescale(civOwner(), !TFacCQCIntfEng::bRemoteMode(), areaActual().szArea());

    // Fix up some commands to map their targets differently at viewing time
    if (!TFacCQCIntfEng::bDesMode())
        SwizzleCmds();
}


tCIDLib::TVoid TCQCIntfTemplate::PostInit()
{
    TParent::PostInit();
}


// We return the combined area of our children
tCIDLib::TVoid TCQCIntfTemplate::QueryContentSize(TSize& szToFill)
{
    //
    //  If we have no children, then see if we have an image and use that. If
    //  neither, return our current size. If we have children, then size to
    //  them first, Then if the image is larger, size up to that.
    //

    const tCIDLib::TCard4 c4CCount = c4ChildCount();
    if (c4CCount)
    {
        szToFill = areaHull().szArea();
        if (bUseImage())
            szToFill.TakeLarger(cptrImage()->szImage());
    }
     else if (bUseImage())
    {
        szToFill = cptrImage()->szImage();
    }
     else
    {
        szToFill = areaActual().szArea();
    }
}


tCIDLib::TVoid TCQCIntfTemplate::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&   colScopes
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       bIncludeScope) const
{
    // Call our parent to handle recursion on children
    TParent::QueryReferencedImgs
    (
        colToFill, colScopes, bIncludeSysImgs, bIncludeScope
    );

    // And call our image mixin
    MCQCIntfImgIntf::QueryImgPath(colToFill, bIncludeSysImgs);
}


//
//  We call our parent first, then add a few bits of our own
//
//  OUR STATES are handled at the container level.
//
tCIDLib::TVoid
TCQCIntfTemplate::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    // Do our parent's stuff
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove the ones we don't use
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_InitInvisible);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Locked);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Rounding);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Shadow);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    // Do our images mixin
    MCQCIntfImgIntf::QueryImgAttrs(colAttrs, adatTmp, kCIDLib::True);
    QueryCmdAttrs(colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Actions");

    // Load/unload WAV files
    colAttrs.objPlace
    (
        L"Template Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    //
    //  Note that the parent class will put in our separator for us, since he also
    //  has some template settings to insert and otherwise they would come out before
    //  our separator.
    //
    //  So let's start with our basic settings.
    //
    colAttrs.objPlace
    (
        L"Focus", kCQCIntfEng::strAttr_Tmpl_Focus, tCIDMData::EAttrTypes::Bool
    ).SetBool(!m_bNoFocus);

    TString strLim;
    tCQCKit::FormatEUserRoles
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    colAttrs.objPlace
    (
        L"Min Role"
        , kCQCIntfEng::strAttr_Tmpl_MinRole
        , strLim
        , tCIDMData::EAttrTypes::String
    ).SetString(tCQCKit::strLoadEUserRoles(m_eMinRole));

    colAttrs.objPlace
    (
        L"Notes"
        , kCQCIntfEng::strAttr_Tmpl_Notes
        , tCIDMData::EAttrTypes::String
    ).SetString(m_strNotes);

    colAttrs.objPlace
    (
        L"Popup Opacity"
        , kCQCIntfEng::strAttr_Tmpl_PopupOpacity
        , tCIDMData::EAttrTypes::Card
    ).SetCard(tCIDLib::TCard4(m_c1ConstOpacity));

    // Do the hot keys, which we have to flatten out as binary
    TBinMBufOutStream strmOut(1024UL);
    strmOut << m_colHotKeyActions << kCIDLib::FlushIt;
    colAttrs.objPlace
    (
        L"Hot Keys"
        , kCQCIntfEng::strAttr_Tmpl_HotKeys
        , strmOut.mbufData()
        , strmOut.c4CurPos()
        , TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    );


    // Do the template actions, which we have to flatten out as binary
    strmOut.Reset();
    strmOut << m_colActions << kCIDLib::FlushIt;
    colAttrs.objPlace
    (
        L"Template Actions"
        , kCQCIntfEng::strAttr_Tmpl_TmplActions
        , strmOut.mbufData()
        , strmOut.c4CurPos()
        , TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    );


    // These are also flattened
    strmOut.Reset();
    strmOut << m_colTrgEvents << kCIDLib::FlushIt;
    colAttrs.objPlace
    (
        L"Trig. Events"
        , kCQCIntfEng::strAttr_Tmpl_TrigEvents
        , strmOut.mbufData()
        , strmOut.c4CurPos()
        , TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    );


    // Load/unload WAV files
    colAttrs.objPlace
    (
        L"Template Audio:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    colAttrs.objPlace
    (
        L"Load WAV"
        , kCQCIntfEng::strAttr_Tmpl_LoadWAV
        , L"FileSel:WAV File:*.wav"
        , tCIDMData::EAttrTypes::File
    ).SetString(m_strLoadWAV);

    colAttrs.objPlace
    (
        L"Unload WAV"
        , kCQCIntfEng::strAttr_Tmpl_UnloadWAV
        , L"FileSel:WAV File:*.wav"
        , tCIDMData::EAttrTypes::File
    ).SetString(m_strUnloadWAV);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid TCQCIntfTemplate::RefreshImages(TDataSrvClient& dsclInit)
{
    // Call our parent to do children
    TParent::RefreshImages(dsclInit);

    // And update our image if any
    MCQCIntfImgIntf::UpdateImage(civOwner(), dsclInit);
}


tCIDLib::TVoid
TCQCIntfTemplate::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    // Let our parent classes do anything they need to do
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    // Do our image interface if doing images
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);

    tCIDLib::TCard4 c4Count;
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
    {
        // Do any template level actions
        c4Count = m_colActions.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colActions[c4Index].CmdSearchNReplace
            (
                strSrc, strTar, bRegEx, bFull, regxFind
            );
        }

        // Do any hot key actions
        tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
        for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
        {
            m_colHotKeyActions[eKey].CmdSearchNReplace
            (
                strSrc, strTar, bRegEx, bFull, regxFind
            );
        }

        // Do any event driven action commands
        c4Count = m_colTrgEvents.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colTrgEvents[c4Index].CmdSearchNReplace
            (
                strSrc, strTar, bRegEx, bFull, regxFind
            );
        }

        // And do any on us ourself
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);
    }

    // If doing event filters, go back through the events
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::FilterParms))
    {
        c4Count = m_colTrgEvents.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colTrgEvents[c4Index].FilterSearchNReplace
            (
                strSrc, strTar, bRegEx, bFull, regxFind
            );
        }
    }
}


tCIDLib::TVoid TCQCIntfTemplate::ResetWidget()
{
    // Pass it on to our parent
    TParent::ResetWidget();

    //
    //  Set a good default size for a template. Tell it not to try to report the
    //  size change, since it's meaningless to do so at this point.
    //
    SetArea(TArea(0, 0, 640, 480), kCIDLib::False);

    // Reset any mixins
    ResetImage();
    ResetAllOps(kCIDLib::False);
}


TSize TCQCIntfTemplate::szDefaultSize() const
{
    // Should never be called for this class
    CIDAssert(kCIDLib::False, L"szDefaultSize should not be called on the template");

    // Make compiler happy
    return TSize(10, 10);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfTemplate::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&      adatOld)
{
    // Do base class ones first
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    //
    //  Watch for the size to change. In our case that's special since we need to
    //  adjust the containing view's size to match our size plus some, and to
    //  redraw.
    //
    if(adatNew.strId() == kCQCIntfEng::strAttr_Base_Size)
    {
        //
        //  The view doesn't really have a command to adjust the size from below like
        //  this. So we sort of cheat and ask the containing window to resize, which
        //  will in turn resize the view.
        //
        TSize szNew = adatNew.szVal();
        szNew.Adjust(256,256);
        civOwner().wndOwner().SetSize(szNew,kCIDLib::False);
        civOwner().wndOwner().Redraw(tCIDCtrls::ERedrawFlags::ImmedErase);
    }


    // Do our mixins
    SetCmdAttr(adatNew, adatOld);
    SetImgAttr(wndAttrEd, adatNew, adatOld);

    // Handle our own attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_Focus)
    {
        m_bNoFocus = !adatNew.bVal();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_MinRole)
    {
        m_eMinRole = tCQCKit::eXlatEUserRoles(adatNew.strValue());
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_Notes)
    {
        m_strNotes = adatNew.strValue();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_PopupOpacity)
    {
        m_c1ConstOpacity = tCIDLib::TCard1(adatNew.c4Val());
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_LoadWAV)
    {
        m_strLoadWAV = adatNew.strValue();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_UnloadWAV)
    {
        m_strUnloadWAV = adatNew.strValue();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_HotKeys)
    {
        //
        //  For undo purposes we have to stream this back in, even though it's already been
        //  updated during editing.
        //
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        strmSrc >> m_colHotKeyActions;
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_TmplActions)
    {
        //
        //  For undo purposes we have to stream this back in, even though it's already been
        //  updated during editing.
        //
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        strmSrc >> m_colActions;
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_TrigEvents)
    {
        //
        //  For undo purposes we have to stream this back in, even though it's already been
        //  updated during editing.
        //
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        strmSrc >> m_colTrgEvents;
    }
}


//
//  When the designer lets the user rename a widget, it needs to update any
//  commands that target that widget id. So we have to override this in order
//  to handle any actions taht only exist at our level. The container parent
//  class will handle any that are done via our command source mixin
//  interface, and all of our child widgets.
//
tCIDLib::TVoid
TCQCIntfTemplate::UpdateCmdTarget(  const   TString&                    strOldTargetId
                                    , const TString&                    strNewTargetName
                                    , const TString&                    strNewTargetId
                                    ,       tCQCIntfEng::TDesIdList&    fcolAffected)
{
    CIDAssert
    (
        TFacCQCIntfEng::bDesMode()
        , L"UpdateCmdTarget should only be called at design time"
    );

    // Do any template level actions
    tCIDLib::TBoolean bChanged = kCIDLib::False;
    tCIDLib::TCard4 c4Count = m_colActions.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        bChanged |= m_colActions[c4Index].bUpdateCmdTarget
        (
            strOldTargetId, strNewTargetName, strNewTargetId
        );
    }

    // Do any hot key actions
    tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
    for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
    {
        if (m_colHotKeyActions[eKey].bUsed())
        {
            bChanged |= m_colHotKeyActions[eKey].bUpdateCmdTarget
            (
                strOldTargetId, strNewTargetName, strNewTargetId
            );
        }
    }

    //
    //  Add oursef to the list of affected widgets if we changed anything and are
    //  not already in the list.
    //
    if (bChanged)
    {
        if (!fcolAffected.bElementPresent(c8DesId()))
            fcolAffected.c4AddElement(c8DesId());
    }

    //
    //  Now call our parent class to handle any child widgets and our own
    //  regular commands.
    //
    TParent::UpdateCmdTarget(strOldTargetId, strNewTargetName, strNewTargetId, fcolAffected);
}


// Handle validating data at our level
tCIDLib::TVoid
TCQCIntfTemplate::Validate( const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image mixin
    ValidateImage(colErrs, dsclVal, *this);

    // Validate our own commands
    ValidateCmds(L"Tmpl Commands", *this, *this, colErrs);

    // Check our non-standard actions
    tCIDLib::TCard4 c4Count = m_colActions.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        ValidateCmds(L"Tmpl Actions", *this, m_colActions[c4Index], colErrs);

    tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
    for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
        ValidateCmds(L"Hot Keys", *this, m_colHotKeyActions[eKey], colErrs);

    c4Count = m_colTrgEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        ValidateCmds(L"Trig Evs", *this, m_colTrgEvents[c4Index], colErrs);
}



// ---------------------------------------------------------------------------
//  TCQCIntfTemplate: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return the timeout type flag
tCIDLib::TBoolean TCQCIntfTemplate::bActivityBasedTO() const
{
    return m_bActivityBasedTO;
}


//
//  This is the high level init call that's done on a template after it is
//  loaded up from the repository. It sets up the template level info and
//  recursively sets up all of the child widgets, resolves field references,
//  initializes state info, does an initial value scane, and so forth. Once
//  we exit here, we have a working template ready to start doing periodic
//  value scans.
//
tCIDLib::TBoolean
TCQCIntfTemplate::bInitTemplate(        TCQCIntfView* const     pcivOwner
                                ,       TCQCPollEngine&         polleToUse
                                ,       TDataSrvClient&         dsclInit
                                , const TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                , const TString&                strInvokeParms)
{
    // Reset all the runtime flags
    SetGlobalRunFlags(tCQCIntfEng::ERunFlags::None, tCQCIntfEng::ERunFlags::All);

    try
    {
        //
        //  The first thing we want to do is a recursive load. This will scan
        //  all our children if if any are overlays set up to do initial loads
        //  of templates, they will be loaded. This will leave us with the
        //  whole initial tree of widgets in memory.
        //
        //  If in designer mode, it won't actually recurse, it'll just do our
        //  immediate children since there is no recursion at design time.
        //
        RecursiveLoad(pcivOwner, dsclInit, cfcData);

        //
        //  Now kick off the recursive initialization of all the widgets we
        //  loaded. Pass a null owning container for this first level.
        //
        Initialize(0, dsclInit, colErrs);

        // Kick off the recursive OnPreload pass if not in designer mode
        if (!TFacCQCIntfEng::bDesMode())
            DoPreloads(*pcivOwner, strInvokeParms);

        //
        //  Do an initial force of everyone to register their fields and get initial values
        //  if they are available already (because the fields were already in the polling
        //  engine.)
        //
        if (!TFacCQCIntfEng::bDesMode())
            RegisterFields(polleToUse, cfcData);
    }

    catch(TError& errToCatch)
    {
        ResetTemplate(pcivOwner);

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        pcivOwner->ShowErr
        (
            pcivOwner->strTitle()
            , facCQCIntfEng().strMsg(kIEngErrs::errcTmpl_LoadFailed, strTemplateName())
            , errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// A helper to see if this guy is a popup or popout
tCIDLib::TBoolean TCQCIntfTemplate::bIsPopupType() const
{
    return (m_eType == tCQCIntfEng::ETmplTypes::Popup)
           || (m_eType == tCQCIntfEng::ETmplTypes::Popout);
}


// Set or get the no focus flag for this template
tCIDLib::TBoolean TCQCIntfTemplate::bNoFocus() const
{
    return m_bNoFocus;
}

tCIDLib::TBoolean TCQCIntfTemplate::bNoFocus(const tCIDLib::TBoolean bToSet)
{
    m_bNoFocus = bToSet;
    return m_bNoFocus;
}


// Get or set the constant opacity set on us. Only meaningful in a popup
tCIDLib::TCard1 TCQCIntfTemplate::c1ConstOpacity() const
{
    return m_c1ConstOpacity;
}

tCIDLib::TCard1 TCQCIntfTemplate::c1ConstOpacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1ConstOpacity = c1ToSet;
    return m_c1ConstOpacity;
}


// Get the next group id and bump it for the next time around
tCIDLib::TCard4 TCQCIntfTemplate::c4GetNextGroupId()
{
    return m_c4NextGroupId++;
}


// Provide access to our template level actions list
const tCQCIntfEng::TActList& TCQCIntfTemplate::colActions() const
{
    return m_colActions;
}

tCQCIntfEng::TActList& TCQCIntfTemplate::colActions()
{
    return m_colActions;
}


// Provide access to our hot key mapped actions list
const tCQCIntfEng::THKActList& TCQCIntfTemplate::colHotKeyActions() const
{
    return m_colHotKeyActions;
}

tCQCIntfEng::THKActList& TCQCIntfTemplate::colHotKeyActions()
{
    return m_colHotKeyActions;
}


// Provide access to our triggered event list
const TVector<TCQCTrgEvent>& TCQCIntfTemplate::colTrgEvents() const
{
    return m_colTrgEvents;
}

TVector<TCQCTrgEvent>& TCQCIntfTemplate::colTrgEvents()
{
    return m_colTrgEvents;
}


//
//  The view calls this for the top level template when there is a click outside
//  of its area. If we have any external click commands, we post that event.
//
tCIDLib::TVoid TCQCIntfTemplate::DoExtClick(const TPoint& ntAt)
{
    if (c4OpCount(kCQCKit::strEvId_OnExtClick))
        PostOps(*this, kCQCKit::strEvId_OnExtClick, tCQCKit::EActOrders::NonNested);
}


//
//  This is mainly for the installer. It will call this as it updates the template data
//  during an upgrade. It reads/writes back out all templates in order to get them up
//  to the latest persistent version. It calls this to give us a chance to do any sort
//  of fixups we don't want to do on every load of the templates.
//
tCIDLib::TVoid TCQCIntfTemplate::DoUpdateFixes()
{
    //
    //  There is an error that happened back some time, where the template itself was incorrectly
    //  offered as an action target. This should happen, only the Intf.Viewer target should be
    //  available. Where appropriate, such commands are mapped to the template itself, or to the
    //  overlay the template was loaded into.
    //
    //  So we need to try to find any such commands. They could only exist if the template itself
    //  has a widget id, else any such commands now just will cause a missing target error. We
    //  could try to figure those out as well, by exhaustively checking all commands that could
    //  go to the intfviewer target, but we don't do that.
    //
    if (!strWidgetId().bIsEmpty())
    {
        //
        //  It could have happened. So let's iterate all commands this guy sends and check
        //  them to see if they are targeting this widget id. Build up what the actual bad
        //  and correct target ids would be and just check each command to see if he is
        //  targeting the bad one. The parent container class has a helper that will do
        //  this sort of target id update for all widgets.
        //
        const TString strOldTargetId = TString::strConcat(kCQCKit::strTarPref_Widgets, strWidgetId());
        TString strNewTargetName;
        TString strNewTargetId;
        tCQCIntfEng::TDesIdList fcolAffected;

        UpdateCmdTarget
        (
            strOldTargetId
            , facCQCIntfEng().strMsg(kIEngMsgs::midTar_Viewer)
            , kCQCIntfEng::strCTarId_IntfViewer
            , fcolAffected
        );

        //
        //  And there have been times where the designer mode flag was not set during
        //  upgrades of templates. This causes default ids to be assigned to all widgets
        //  that don't have explicit ones, because they always need to have an id when it
        //  viewer mode. So for each widget that has a default generated id, we see if
        //  any actions actually reference this id. If not, we clear it.
        //
        //  We have to build back up the target id again, since it might have been
        //  changed before.
        //
        const tCIDLib::TCard4 c4Count = c4ChildCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCIntfWidget& iwdgCur = iwdgAt(c4Index);

            if (iwdgCur.strWidgetId().bStartsWith(kCQCIntfEng::strDefWidgetIdPref))
            {
                strNewTargetId = kCQCKit::strTarPref_Widgets;
                strNewTargetId.Append(iwdgCur.strWidgetId());

                //
                //  Call ourself, which check us and all children. That will call on
                //  the current one as well, but he could reference himself in a command.
                //
                if (!bReferencesTarId(strNewTargetId))
                    iwdgCur.strWidgetId(TString::strEmpty());
            }
        }
    }
}



// Get or set the last timeout event invocation time stamp
tCIDLib::TEncodedTime TCQCIntfTemplate::enctLastTimeout() const
{
    return m_enctLastTimeout;
}

tCIDLib::TEncodedTime
TCQCIntfTemplate::enctLastTimeout(const tCIDLib::TEncodedTime enctToSet)
{
    m_enctLastTimeout = enctToSet;
    return m_enctLastTimeout;
}


//
//  This is called to ask us to do a validation pass and let the user deal with any errors.
//
tCQCIntfEng::EValTypes
TCQCIntfTemplate::eDoValidation(tCQCIntfEng::TErrList& colErrs)
{
    colErrs.RemoveAll();

    // Do a pass and get any errors
    try
    {
        TDataSrvClient dsclVal;
        TCQCFldCache cfcData;
        cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));
        Validate(cfcData, colErrs, dsclVal);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        civOwner().ShowErr
        (
            civOwner().strTitle()
            , facCQCIntfEng().strMsg(kIEngErrs::errcVal_Failed)
            , errToCatch
        );
        return tCQCIntfEng::EValTypes::Success;
    }

    // If no errors, return success now
    const tCIDLib::TCard4 c4Count = colErrs.c4ElemCount();
    if (!c4Count)
        return tCQCIntfEng::EValTypes::Success;

    // Else see if all the errors are warnings, or if we have some real errors
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!colErrs[c4Index].bWarning())
            return tCQCIntfEng::EValTypes::Errors;
    }
    return tCQCIntfEng::EValTypes::Warnings;
}


// Get or set the minimum user role that can access this template
tCQCKit::EUserRoles TCQCIntfTemplate::eMinRole() const
{
    return m_eMinRole;
}

tCQCKit::EUserRoles TCQCIntfTemplate::eMinRole(const tCQCKit::EUserRoles eToSet)
{
    m_eMinRole = eToSet;
    return m_eMinRole;
}


// Get or set the template type
tCQCIntfEng::ETmplTypes TCQCIntfTemplate::eType() const
{
    return m_eType;
}

tCQCIntfEng::ETmplTypes
TCQCIntfTemplate::eType(const tCQCIntfEng::ETmplTypes eType)
{
    m_eType = eType;
    return m_eType;
}


//
//  This is called to find the next widget to take the focus when focus is
//  enabled. We get the existing focus widget, if any and a navigation
//  option that tells us how to move.
//
TCQCIntfWidget*
TCQCIntfTemplate::piwdgMoveFocus(const  tCQCKit::EScrNavOps eOp
                                ,       TCQCIntfWidget*&    piwdgOld)
{
    // Find the current focus
    piwdgOld = piwdgFindRunFlags
    (
        tCQCIntfEng::ERunFlags::HasFocus, tCQCIntfEng::ERunFlags::HasFocus
    );

    //
    //  If none, then find the first one that can take it. This might also
    //  be null if there are no focusable widgets.
    //
    if (!piwdgOld)
        return piwdgNextPrevFocus(0, kCIDLib::True);

    //
    //  Before we do anything, if it's not one of the previous/next ops,
    //  give the old widget an opportunity to move the focus internally
    //  within itself, which is necessary for some types of widgets that
    //  have multiple internal focus locations. If he returns false, then
    //  he kept the focus and just moved internally and we just return it
    //  again.
    //
    //  The previous/next operations force movement to be external only,
    //  to provide a way to get immediately out of a multi-position widget.
    //
    if ((eOp != tCQCKit::EScrNavOps::Prev)
    &&  (eOp != tCQCKit::EScrNavOps::Next))
    {
        // If an internal move, then return the same one
        if (!piwdgOld->bMoveFocus(eOp))
            return piwdgOld;
    }

    //
    //  We only support forward/back at this level, so down or right is
    //  considered next and up/left is considered previous.
    //
    const tCIDLib::TBoolean bForward
    (
        (eOp == tCQCKit::EScrNavOps::Down)
        || (eOp == tCQCKit::EScrNavOps::Right)
        || (eOp == tCQCKit::EScrNavOps::Next)
    );

    //
    //  We have to find the next/prev widget that can take the focus,
    //  accounting for nesting of overlays.
    //
    //  Get the parent container of the passed widget and ask it for
    //  the previous or next widget of his that can take the focus.
    //  If he returns none, that's because we are at the end of him in
    //  the direction of travel and need to recurse up or down (or
    //  wrap if the parent is us.)
    //
    //  Note that he will recurse downwards himself to find the next
    //  available focus under him. We just have to deal with coming back
    //  up out again.
    //
    TCQCIntfContainer* piwdgParent = piwdgOld->piwdgParent();
    TCQCIntfWidget* piwdgNew = nullptr;
    #if CID_DEBUG_ON
    if (!piwdgParent)
    {
        // <TBD>...
        return nullptr;
    }
    #endif
    piwdgNew = piwdgParent->piwdgNextPrevFocus(piwdgOld, bForward);

    //
    //  If we've hit the end of whatever container the focus was in, we
    //  need to loop now, moving upwards until we find a container that
    //  can move in the direction we want, or we hit ourself.
    //
    while(!piwdgNew)
    {
        // Break out when we hit ourself
        if (piwdgParent == this)
            break;

        // Not us yet, so move up another level
        TCQCIntfWidget* piwdgPrevPar = piwdgParent;
        piwdgParent = piwdgParent->piwdgParent();

        // And try again
        piwdgNew = piwdgParent->piwdgNextPrevFocus(piwdgPrevPar, bForward);
    }

    //
    //  If we still don't have one, then we need to wrap around at our
    //  top level.
    //
    if (!piwdgNew)
    {
        if (bForward)
            piwdgNew = piwdgNextPrevFocus(0, kCIDLib::True);
        else
            piwdgNew = piwdgNextPrevFocus(0, kCIDLib::False);
    }
    return piwdgNew;
}


// Resets everything back to a default empty state
tCIDLib::TVoid TCQCIntfTemplate::ResetTemplate(TCQCIntfView* const pcivOwner)
{
    // Store the new owner
    SetOwner(pcivOwner);

    // Call the virtual reset that will do all the other resetting
    ResetWidget();
}


// Get/set the comments
const TString& TCQCIntfTemplate::strNotes() const
{
    return m_strNotes;
}

const TString& TCQCIntfTemplate::strNotes(const TString& strToSet)
{
    m_strNotes = strToSet;
    return m_strNotes;
}


// Get/set the load/unload WAV file paths
const TString& TCQCIntfTemplate::strLoadWAV() const
{
    return m_strLoadWAV;
}

const TString& TCQCIntfTemplate::strLoadWAV(const TString& strToSet)
{
    m_strLoadWAV = strToSet;
    return m_strLoadWAV;
}

const TString& TCQCIntfTemplate::strUnloadWAV() const
{
    return m_strUnloadWAV;
}

const TString& TCQCIntfTemplate::strUnloadWAV(const TString& strToSet)
{
    m_strUnloadWAV = strToSet;
    return m_strUnloadWAV;
}


// ---------------------------------------------------------------------------
//  TCQCIntfTemplate: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCQCIntfTemplate::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    // Call our parent first
    TParent::AreaChanged(areaNew, areaOld);

    // Update the pre-scaling on our image mixin, if our size changed.
    if (!areaNew.bSameSize(areaOld))
        SetImgPrescale(civOwner(), !TFacCQCIntfEng::bRemoteMode(), areaNew.szArea());
}

// Invoke any OnExit action if we have one configured
tCIDLib::TVoid TCQCIntfTemplate::DoOnExit(TCQCIntfView& civOwner)
{
    if (c4OpCount(kCQCKit::strEvId_OnExit))
        PostOps(*this, kCQCKit::strEvId_OnExit, tCQCKit::EActOrders::NonNested);
}

// Invoke any OnLoad action if we have one configured
tCIDLib::TVoid TCQCIntfTemplate::DoOnLoad(TCQCIntfView& civOwner)
{
    if (c4OpCount(kCQCKit::strEvId_OnLoad))
        PostOps(*this, kCQCKit::strEvId_OnLoad, tCQCKit::EActOrders::NonNested);
}


// Invoke any OnPreload action if we have one configured
tCIDLib::TVoid
TCQCIntfTemplate::DoOnPreload(          TCQCIntfView&   civOwner
                                , const TString&        strInvokeParms)
{
    TOpcodeBlock* pcolPre = pcolOpsForEvent(kCQCKit::strEvId_OnPreload);
    if (!pcolPre->bIsEmpty())
    {
        //
        //  Looks ok. Build up a list of non-standard targets, which
        //  will be all our children that implement the command target
        //  mixin.
        //
        tCQCKit::TCmdTarList colWdgTars(tCIDLib::EAdoptOpts::NoAdopt);
        FindAllTargets(colWdgTars, kCIDLib::False);

        // Add ourself
        colWdgTars.Add(this);

        // And the view
        colWdgTars.Add(&civOwner);

        // Now create the engine and run
        TCQCStdActEngine actePreload(civOwner.cuctxToUse());
        const tCQCKit::ECmdRes eRes = actePreload.eInvokeOps
        (
            *this
            , kCQCKit::strEvId_OnPreload
            , civOwner.ctarGlobalVars()
            , colWdgTars
            , civOwner.pmcmdtTrace()
            , strInvokeParms
        );

        if (eRes > tCQCKit::ECmdRes::Exit)
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcTmpl_PreloadFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strTemplateName()
            );
        }
    }
}


tCIDLib::TVoid
TCQCIntfTemplate::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    // Call our parent first
    TParent::SizeChanged(szNew, szOld);

    // Update the pre-scaling on our image mixin
    SetImgPrescale(civOwner(), !TFacCQCIntfEng::bRemoteMode(), szNew);
}


// Stream us in from the passed stream
tCIDLib::TVoid TCQCIntfTemplate::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Template::c2FmtVersion))
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

    // Let our parent stream his stuff
    TParent::StreamFrom(strmToReadFrom);

    // Stream any mixins
    MCQCIntfImgIntf::ReadInImage(strmToReadFrom);
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // Pull in our stuff
    strmToReadFrom  >> m_bNoFocus
                    >> m_eMinRole;

    // If version 2, then read in and discard the focus colors
    if (c2FmtVersion == 2)
    {
        TRGBClr rgbTmp;
        strmToReadFrom >> rgbTmp >> rgbTmp;
    }

    // If version 1, then default the WAV paths, else load them
    if (c2FmtVersion < 2)
    {
        m_strLoadWAV.Clear();
        m_strUnloadWAV.Clear();
    }
     else
    {
        strmToReadFrom >> m_strLoadWAV >> m_strUnloadWAV;
    }

    // If version V3 or less, default the triggered events, else read them
    if (c2FmtVersion < 4)
        m_colTrgEvents.RemoveAll();
     else
        strmToReadFrom >> m_colTrgEvents;

    // If V5 or less, default the actions list, else read them in
    if (c2FmtVersion < 6)
        m_colActions.RemoveAll();
    else
        strmToReadFrom >> m_colActions;

    // If V6 or less default the comments, else read them in
    if (c2FmtVersion < 7)
        m_strNotes.Clear();
    else
        strmToReadFrom >> m_strNotes;

    //
    //  If V7 or less, default the hot key actions list and mark them all
    //  unused.
    //
    //  If version 8, then we need to add another item,so we read into a
    //  temp collection, then copy the old ones over then add the ne one.
    //
    //  Else just read them in.
    //
    if (c2FmtVersion < 8)
    {
        tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
        for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
            m_colHotKeyActions[eKey].bUsed(kCIDLib::False);
    }
     else if (c2FmtVersion == 8)
    {
        // Read into a temp list
        tCQCIntfEng::THKActList colTmp;
        strmToReadFrom >> colTmp;

        CIDAssert
        (
            colTmp.c4ElemCount() + 1 == tCIDLib::c4EnumOrd(tCQCIntfEng::EHotKeys::Count)
            , L"The V8 hot key actionlist was not the correct size"
        );

        //  Copy over the old ones, then add the new extra one at the end
        m_colHotKeyActions = colTmp;
        m_colHotKeyActions.objAdd(TCQCStdCmdSrc());
    }
     else
    {
        // Nothing special, just read it in
        strmToReadFrom >> m_colHotKeyActions;
    }

    //
    //  If V10, read in an discard the dynamic resizing flag that we introduced
    //  for that version but then removed in V11. So it's only ever present on V10.
    //
    if (c2FmtVersion == 10)
    {
        tCIDLib::TBoolean bDummy;
        strmToReadFrom >> bDummy;
    }

    // And we should have a final end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    //
    //  If it was a pre-5 object, then convert the Xlats that our parent
    //  used to own (and read into a protected member for us to get and
    //  translate.) We also will force the new OnPreload event in as well.
    //
    if (c2FmtVersion < 5)
        ConvertXlats(m_colFXlats);

    //
    //  Command sources have to tell the command editor dialog what action
    //  context they use. Since triggered events are used in different
    //  contexts (here vs. event server) it has to report different contexts,
    //  so we have to set a specific context on them. It's not persisted
    //  with the command configuration since it never actually changes. It's
    //  an attribute of the situation the events are configured in and it's
    //  only needed at design time.
    //
    tCIDLib::TCard4 c4Count = m_colTrgEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colTrgEvents[c4Index].SetContext(tCQCKit::EActCmdCtx::IVEvent);

    //
    //  We have to add some new events to older stored templates, since these
    //  we added after the initial implementation. If we don't find the event
    //  add it.
    //
    //  OnExit was added in 4.0.13. We give it the same restrictions as OnPreload,
    //  so we use that command context.
    //
    //  OnExtClick was added in 4.4.915, and has no special restrictions.
    //
    if (!pcolOpsForEvent(kCQCKit::strEvId_OnTimeout))
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnTimeout)
            , kCQCKit::strEvId_OnTimeout
            , tCQCKit::EActCmdCtx::Standard
        );
    }
    if (!pcolOpsForEvent(kCQCKit::strEvId_OnExit))
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnExit)
            , kCQCKit::strEvId_OnExit
            , tCQCKit::EActCmdCtx::Preload
        );
    }

    if (!pcolOpsForEvent(kCQCKit::strEvId_OnExtClick))
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnExtClick)
            , kCQCKit::strEvId_OnExtClick
            , tCQCKit::EActCmdCtx::Standard
        );
    }

    //
    //  Somewhere along the line, we managed to get two onextclick events into some
    //  templates, because the event got spelled wrong. So find the bad one and remove
    //  it.
    //
    DeleteEvent(L"StdEvent:OnExtClick");

    //
    //  If version pre-version 12, then we need to update the grouping scheme. It
    //  used to be an index scheme, where the first item was 0, and the subsequent
    //  group members were 1, 2, etc... Ungrouped widgets were max card.
    //
    //  Now it's that we assign a unique group id to grouped members.
    //
    //  If it's V12 or beyond, we just need to find the next available group id
    //  and remember that.
    //
    c4Count = c4ChildCount();
    if (c2FmtVersion < 12)
    {
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            TCQCIntfWidget& iwdgCur = iwdgAt(c4Index);

            //
            //  If not grouped, set it to zero. If it's a group leader, then let's
            //  loop through that group and set them all to the next available
            //  group id.
            //
            if (iwdgCur.c4GroupId() == kCIDLib::c4MaxCard)
            {
                iwdgCur.c4GroupId(0);
                c4Index++;
            }
             else
            {
                const tCIDLib::TCard4 c4NewId = c4GetNextGroupId();

                //
                //  Do the current one and move forward before we hit the loop,
                //  so that we can know if we hit another group, not just our own
                //  first member.
                //
                iwdgCur.c4GroupId(c4NewId);
                c4Index++;

                while (c4Index < c4Count)
                {
                    TCQCIntfWidget& iwdgMember = iwdgAt(c4Index);

                    //
                    //  If we hit the start of another group, or an ungrouped one
                    //  we are done.
                    //
                    if ((iwdgMember.c4GroupId() == 0)
                    ||  (iwdgMember.c4GroupId() == kCIDLib::c4MaxCard))
                    {
                        break;
                    }

                    // Else it's a member, so update it
                    iwdgMember.c4GroupId(c4NewId);
                    c4Index++;
                }
            }
        }
    }
     else
    {
        m_c4NextGroupId = 1;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCIntfWidget& iwdgCur = iwdgAt(c4Index);
            if (iwdgCur.c4GroupId() >= m_c4NextGroupId)
                m_c4NextGroupId = iwdgCur.c4GroupId() + 1;
        }
    }

    // Make sure our group id is zero since we are never grouped, just in case
    c4GroupId(0);

    // Reset any non-persistent stuff
    m_bActivityBasedTO = kCIDLib::True;
    m_bUserFlag = kCIDLib::False;
    m_c1ConstOpacity = 0xFF;
    m_c4AppData = 0;
    m_c4TimeoutSecs = 0;
    m_eType = tCQCIntfEng::ETmplTypes::Std;
    m_c4HandlerId = 0;
    m_enctLastTimeout = TTime::enctNow();
}


tCIDLib::TVoid TCQCIntfTemplate::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_Template::c2FmtVersion;

    // Let the parent stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Handle any mixins
    MCQCIntfImgIntf::WriteOutImage(strmToWriteTo);
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // And strem stream our stuff and our end marker
    strmToWriteTo   << m_bNoFocus
                    << m_eMinRole
                    << m_strLoadWAV
                    << m_strUnloadWAV
                    << m_colTrgEvents
                    << m_colActions
                    << m_strNotes

                    // V8 stuff
                    << m_colHotKeyActions

                    << tCIDLib::EStreamMarkers::EndObject;
}


//
//  This is called to get us to update when we need to redraw. We just have
//  to deal with the background fill or image.
//
tCIDLib::TVoid
TCQCIntfTemplate::Update(       TGraphDrawDev&  gdevTarget
                        , const TArea&          areaInvalid
                        ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Set up the remaining area as the clip region
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();

    //
    //  If the image interface mixin indicates we are using an image, then ask it
    //  to draw. It will take advantage of any pre-scaling to optimize the drawing.
    //
    if (bUseImage())
    {
        DrawImgOn
        (
            gdevTarget
            , kCIDLib::False
            , areaDraw
            , areaInvalid
            , 0
            , 0
            , kCIDLib::False
            , kCIDLib::False // (m_eType == tCQCIntfEng::ETmplTypes::Std)
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfTmplate: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Some cmds are actually targeted at the interface view target, since their actual
//  target isn't known until viewing time (since the template could be loaded as a
//  top level template or into an overlay or popup.)
//
//  So we want to find any of those commands and map their target to us instead of
//  to the viewer target.
//
//  NOTE:   This is only called at viewing time. It has no meaning at design time!
//
tCIDLib::TVoid TCQCIntfTemplate::SwizzleCmds()
{
    CIDAssert
    (
        !TFacCQCIntfEng::bDesMode()
        , L"SwizzleCmds should only be called at viewing time"
    );

    // Build up a path to us
    TString strTarId = kCQCKit::strTarPref_Widgets;
    strTarId.Append(strWidgetId());

    //
    //  Do ourself, since these could be called from OnPreload or OnLoad.
    //  This one is a special case, we only do the set timeout in
    //  this case. The others make no sense to be called from those
    //  events, so we want to let them go to the view and fail if they
    //  were called.
    //
    UpdateOps
    (
        kCQCIntfEng::strCmdId_SetTimeout
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_SetTimeout
    );

    tCQCKit::TCmdSrcList colSrcs(tCIDLib::EAdoptOpts::NoAdopt);
    tCIDLib::TCard4 c4Count = c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget& iwdgCur = iwdgAt(c4Index);

        //
        //  See if it implements the command source interface. If not, then
        //  ask it for a list of targets it might contain, since sometimes
        //  they contain targets but aren't one themselves.
        //
        MCQCCmdSrcIntf* pmcsrcCur = dynamic_cast<MCQCCmdSrcIntf*>(&iwdgCur);
        if (pmcsrcCur)
        {
            SwizzleOne(strTarId, *pmcsrcCur);
        }
         else
        {
            colSrcs.RemoveAll();
            const tCIDLib::TCard4 c4EmbCnt = iwdgCur.c4QueryEmbeddedCmdSrcs(colSrcs);
            for (tCIDLib::TCard4 c4EmbInd = 0; c4EmbInd < c4EmbCnt; c4EmbInd++)
                SwizzleOne(strTarId, *colSrcs[c4EmbInd]);
        }
    }

    // Do any template level actions
    c4Count = m_colActions.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        SwizzleOne(strTarId, m_colActions[c4Index]);

    // Do any hot key actions that are configured
    tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
    for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
    {
        if (m_colHotKeyActions[eKey].bUsed())
            SwizzleOne(strTarId, m_colHotKeyActions[eKey]);
    }

    // And do any event driven actions
    c4Count = m_colTrgEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        SwizzleOne(strTarId, m_colTrgEvents[c4Index]);
}


// A helper for SwizzleCmds above, to avoid a lot of redundancy
tCIDLib::TVoid
TCQCIntfTemplate::SwizzleOne(const  TString&        strTarId
                            ,       MCQCCmdSrcIntf& mcsrcToUpdate)
{
    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_LoadSiblingOverlay
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_LoadSiblingOverlay
    );

    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_RunTmplAction
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_RunTmplAction
    );

    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_SetTimeout
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_SetTimeout
    );
}


//
//  Temporary to support translation of the old Xlats stuff into the new
//  action based form that was introduced in the run up to 3.0.
//
tCIDLib::TVoid
TCQCIntfTemplate::ConvertXlats(const tCIDLib::TKVPList& colFXlats)
{
    // Force the new preload event in since it won't be in olders ones
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPreload)
        , kCQCKit::strEvId_OnPreload
        , tCQCKit::EActCmdCtx::Preload
    );

    // If no xlates, then nothing more to do here
    if (colFXlats.bIsEmpty())
        return;

    //
    //  Set up a link to field command. It's always the same for all
    //  targets that support it.
    //
    TCQCCmd cmdSetField
    (
        kCQCKit::strCmdId_LinkToField
        , facCQCKit().strMsg(kKitMsgs::midCmd_LinkToField)
        , tCQCKit::ECmdPTypes::Field
        , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
    );
    cmdSetField.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::MReadCWrite);

    // We'll fill in a list of opcodes and set them at the end
    TVector<TActOpcode> colOps;

    const tCIDLib::TCard4 c4Count = m_colFXlats.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = colFXlats[c4Index];

        // See if this target exists. If not go to the next one
        TCQCIntfWidget* piwdgCur = piwdgFindByName(kvalCur.strKey(), kCIDLib::False);
        if (!piwdgCur)
            continue;

        //
        //  OK, generate an opcode in the OnPreload. The Xlat value becomes
        //  the parameter to the command.
        //
        {
            TActOpcode& aocNew = colOps.objAdd(TActOpcode(tCQCKit::EActOps::Cmd));
            aocNew.m_ccfgStep.SetFrom(cmdSetField, *this, piwdgCur);
            aocNew.m_ccfgStep.SetParmAt(0, kvalCur.strValue());
        }
    }

    // Now set those ops on our preload event
    SetEventOps(kCQCKit::strEvId_OnPreload, colOps);
}


