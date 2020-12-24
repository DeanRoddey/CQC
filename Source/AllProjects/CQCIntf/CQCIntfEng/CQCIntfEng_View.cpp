//
// FILE NAME: CQCIntfEng_View.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/15/2002
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
//  This file implements the TCQCIntfWindow class, which provides the display
//  mechanism for user drawn interfaces.
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
#include    "CQCIntfEng_LiveTile.hpp"
#include    "CQCIntfEng_WebBrowser.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfView,TObject)
RTTIDecls(TCQCIntfStdView,TCQCIntfView)


// ---------------------------------------------------------------------------
//  Local constants and types
// ---------------------------------------------------------------------------
namespace CQCIntfEng_View
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The default popup position info which means 'center it'
        // -----------------------------------------------------------------------
        const TPoint    pntDefPos(-1, -1);


        // -----------------------------------------------------------------------
        //  The number of milliseconds to pause between each round when fading
        //  popups in/out.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4FadeRndMillis(35);


        // -----------------------------------------------------------------------
        //  Ids for the async data callbacks
        // -----------------------------------------------------------------------
        const TString  strACBID_LoadOverAnc(L"OverAncestorLoad");
        const TString  strACBID_ScrollOverlay(L"ScrollOverlay");
    }
}




// ---------------------------------------------------------------------------
//    CLASS: TAsyncDCBBase
//   PREFIX: adcb
// ---------------------------------------------------------------------------
TCQCIntfADCB::TCQCIntfADCB(         TCQCIntfView* const pcivCaller
                            , const TString&            strId) :

    m_pcivCaller(pcivCaller)
    , m_strCBId(strId)
{
}

TCQCIntfADCB::~TCQCIntfADCB()
{
}


// ---------------------------------------------------------------------------
//    CLASS: TAncOverLoad
//   PREFIX: adcb
// ---------------------------------------------------------------------------
TAncOverLoad::TAncOverLoad(         TCQCIntfView* const     pcivCB
                            , const TCQCIntfContainer&      iwdgStart
                            , const TString&                strAncestor
                            , const TString&                strTmplToLoad) :

    TCQCIntfADCB(pcivCB, CQCIntfEng_View::strACBID_LoadOverAnc)
    , m_strAncestor(strAncestor)
    , m_strTmplToLoad(strTmplToLoad)
{
    // Find the target and store the path to it
    const TCQCIntfContainer* piwdgTar = 0;
    if (strAncestor.bCompareI(L"Root"))
    {
        m_strTarPath = L"/";
    }
     else
    {
        //
        //  The passed container is the parent, so start with his parent, since
        //  we only do Grandparent or Great Grandparent.
        //
        piwdgTar = iwdgStart.piwdgParent();

        // If the great grandparent, then move up again
        if (strAncestor.bCompareI(L"Great Grandparent") && piwdgTar)
            piwdgTar = piwdgTar->piwdgParent();

        // If none, then just go with the root
        if (!piwdgTar)
        {
            m_strTarPath = L"/";
        }
         else
        {
            // Build up the name path to the target, including himself
            piwdgTar->QueryNamePath(m_strTarPath, kCIDLib::True);
        }
    }
}

TAncOverLoad::~TAncOverLoad()
{
}



// ---------------------------------------------------------------------------
//    CLASS: TScrollOverADCB
//   PREFIX: adcb
// ---------------------------------------------------------------------------
TScrollOverADCB::TScrollOverADCB(       TCQCIntfView* const pcivCB
                                , const TCQCIntfContainer&  iwdgStart
                                , const TString&            strScrollCmd
                                , const TString&            strTarget) :

    TCQCIntfADCB(pcivCB, CQCIntfEng_View::strACBID_ScrollOverlay)
    , m_strScrollCmd(strScrollCmd)
    , m_strTarget(strTarget)
{
    //
    //  Find the target and store the path to it. Start with the target, which is
    //  the parent of the original widget.
    //
    const TCQCIntfContainer* piwdgTar = &iwdgStart;

    // Make the analyzer happy
    CIDAssert(piwdgTar != nullptr, L"Incoming widget was null");

    //
    //  If the target is Parent, then we have what we need. Else, we have
    //  move up the chain for GrandParent and Great GrandParent, or special
    //  case for Root.
    //
    if (strTarget.bCompareI(L"Root"))
    {
        //
        //  We just move up the parent chain until we hit a non-overlay,
        //  and then keep the last overlay we hit.
        //
        while (piwdgTar)
        {
            const TCQCIntfContainer* piwdgNext = piwdgTar->piwdgParent();

            // If no parent or it's not an overlay, then we are done
            if (!piwdgNext || !piwdgTar->bIsA(TCQCIntfOverlay::clsThis()))
                break;

            // Looks ok, so take this one
            piwdgTar = piwdgNext;
        }
    }
     else if (strTarget.bCompareI(L"Parent"))
    {
        // Do nothing, we already have what we need
    }
     else
    {
        // Not the parent, so at least have to move up one
        piwdgTar = piwdgTar->piwdgParent();
        if (piwdgTar)
        {
            // If they wanted the great grandparent, then move up again if we can
            if (strTarget.bCompareI(L"Great GrandParent"))
                piwdgTar = piwdgTar->piwdgParent();
        }

        // If we hit the root or it's not an overlay, that's bad
        if (!piwdgTar || !piwdgTar->bIsA(TCQCIntfOverlay::clsThis()))
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcLink_NotAnOverlay2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
            );

            // Make analzyer happy
            return;
        }
    }

    // OK, query the path to the target and store it
    piwdgTar->QueryNamePath(m_strTarPath, kCIDLib::True);
}

TScrollOverADCB::~TScrollOverADCB()
{
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStackItem
//  PREFIX: isi
// ---------------------------------------------------------------------------
class TCQCIntfStackItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStackItem();

        ~TCQCIntfStackItem();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize
        (
            const   TCQCIntfTemplate&       iwdgInit
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TVoid Initialize
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TVoid Reset
        (
                    TCQCIntfView* const     pcivOwner
        );


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bBreakFlag
        //      The modal loop requires a break flag. Each popup layer is in
        //      a modal loop, so we keep one for each layer.
        //
        //  m_abProcessingCmd
        //      This is set while we are processing commands, so that we have
        //      a simple way of knowing whether new incoming actions (nested
        //      ones) should just be queued for the existing processing session
        //      or should start up a new one. Each nested template has its
        //      own since it has its own action engine window.
        //
        //  m_abTOResumeFlag
        //      The inactivity timer can invoke a user defined popup that must
        //      be dismissed to continue after the blanker is dismissed.
        //      Levels marked thusly have special concerns. It can't be
        //      dismissed via Escape, it has to return voluntarily, and we
        //      also want to be sure only one of them gets invoked, so if
        //      the timeout kicks back in we don't want to do another one.
        //
        //      It's part of the blanker stuff, and can either be brought
        //      up as the blanker is invoked, so it'll be there when the
        //      blanker is dismissed, or can be popped up by itself, depending
        //      on the blanker mode.
        //
        //      Note we can't have just one flag, since this popup could
        //      invoke other regular popups, which don't have these issues. So
        //      we need a flag per layer.
        //
        //  m_aiwdgTemplate
        //      The actual templates of the stack. The 0th one is the base
        //      template, and the others are popups, up to a small fixed limit
        //      of nesting.
        //
        //  m_apiactxLevel
        //      Each template level gets its own action context object
        //      so that we can keep up with the actions being run by that
        //      level. These have to be pointers because we can't construct
        //      them when the stack manager's ctor first default constructs us.
        //      We'll set this up during Initialize.
        //
        //  m_apntOffset
        //      Each popup can be either centered or indicate a position
        //      relative to the upper left of the parent template. The
        //      caller gives us the offset of the template from its parent
        //      template. This allows us to reposition them if the window
        //      size changes.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bBreakFlag;
        tCIDLib::TBoolean   m_bProcessingCmd;
        tCIDLib::TBoolean   m_bTOResumeFlag;
        TCQCIntfTemplate    m_iwdgTemplate;
        TCQCIntfActCtx*     m_piactxLevel;
        TPoint              m_pntOffset;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTmplMgr
//  PREFIX: itm
// ---------------------------------------------------------------------------
class TCQCIntfTmplMgr
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfTmplMgr
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfTmplMgr
        (
            const   TCQCIntfTemplate&       iwdgInit
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfTmplMgr(const TCQCIntfTmplMgr&) = delete;

        ~TCQCIntfTmplMgr();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfTmplMgr& operator=(const TCQCIntfTmplMgr&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bActiveAction() const;

        tCIDLib::TBoolean bCopyRetVals
        (
            const   tCIDLib::TCard4         c4TopInd
        );

        tCIDLib::TBoolean& bBreakFlag
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TBoolean bBreakFlag
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TBoolean bHasPopups() const;

        tCIDLib::TBoolean bIsValidHandlerId
        (
            const   tCIDLib::TCard4         c4HandlerId
        )   const;

        tCIDLib::TBoolean bProcessingCmds
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TBoolean bTOPopupDisplayed() const;

        tCIDLib::TCard4 c4CurCount() const;

        tCIDLib::TCard4 c4Push
        (
                    TCQCIntfView&           civOwner
            , const tCIDLib::TBoolean       bTOPopup
        );

        tCIDLib::TVoid Cleanup();

        TCQCIntfActCtx& iactxAt
        (
            const   tCIDLib::TCard4         c4At
        );

        TCQCIntfActCtx& iactxCur();

        TCQCIntfStackItem& isiAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TCQCIntfStackItem& isiAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCIntfStackItem& isiNext
        (
                    tCIDLib::TCard4&        c4HandlerId
        );

        TCQCIntfTemplate& iwdgTmplAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TCQCIntfTemplate& iwdgTmplAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TCQCIntfTemplate& iwdgBaseTmpl() const;

        TCQCIntfTemplate& iwdgBaseTmpl();

        const TCQCIntfTemplate& iwdgTopMost() const;

        TCQCIntfTemplate& iwdgTopMost();

        tCIDLib::TVoid NewViewSize
        (
            const   TSize&                  szNew
            , const tCIDLib::TBoolean       bCentered
        );

        TCQCIntfWidget* piwdgFindById
        (
            const   tCIDLib::TCard4         c4WidgetId
            , const tCIDLib::TBoolean       bTopMostOnly
        );

        tCIDLib::TVoid Pop
        (
                    TCQCIntfView&           civOwner
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TBoolean       bFade
        );

        tCIDLib::TVoid QueryCombinedArea
        (
                    TArea&                  areaToFill
        )   const;

        tCIDLib::TVoid SetBaseTemplate
        (
            const   TCQCIntfTemplate&       iwdgToSet
        );

        tCIDLib::TVoid SetBreakFlagAt
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid SetProcessingAt
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid SetTopBreakFlag
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid ShowTop
        (
                    TCQCIntfView&           civOwner
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TCard1         c1Opacity
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckStackIndex
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TCard4         c4Line
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Count
        //      The current count of active templates. It starts out as one
        //      and the base template object is precreated and always kept
        //      around.
        // -------------------------------------------------------------------
        TCQCIntfStackItem   m_aisiMgr[kCQCIntfEng::c4MaxIntfDepth];
        tCIDLib::TCard4     m_c4Count;
};



// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfAppHandler
//  PREFIX: miah
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//  MCQCIntfAppHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
MCQCIntfAppHandler::MCQCIntfAppHandler()
{
}

MCQCIntfAppHandler::MCQCIntfAppHandler(const MCQCIntfAppHandler&)
{
}

MCQCIntfAppHandler::~MCQCIntfAppHandler()
{
}

// ---------------------------------------------------------------------------
//  MCQCIntfAppHandler: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid MCQCIntfAppHandler::operator=(const MCQCIntfAppHandler&)
{
}


// ---------------------------------------------------------------------------
//  MCQCIntfAppHandler: Public, virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
MCQCIntfAppHandler::CreateRemWidget(const   TString&
                                    , const tCQCIntfEng::ERIVAWTypes
                                    , const TArea&
                                    , const tCIDLib::TKVPList&
                                    , const tCIDLib::TBoolean)
{
    // Only needed by the WebRIVA handler in the web server, so default for every else
}

tCIDLib::TVoid
MCQCIntfAppHandler::DestroyRemWidget(const TString&, const tCQCIntfEng::ERIVAWTypes)
{
    // Only needed by the WebRIVA handler in the web server, so default for every else
}

tCIDLib::TVoid
MCQCIntfAppHandler::SetRemWidgetVis(const   TString&
                                    , const tCQCIntfEng::ERIVAWTypes
                                    , const tCIDLib::TBoolean)
{
    // Only needed by the WebRIVA handler in the web server, so default for every else
}

tCIDLib::TVoid
MCQCIntfAppHandler::SetRemWidgetURL(const   TString&
                                    , const tCQCIntfEng::ERIVAWTypes
                                    , const TString&
                                    , const tCIDLib::TKVPList&)
{
    // Only needed by the WebRIVA handler in the web server, so default for every else
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStackItem
//  PREFIX: isi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStackItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStackItem::TCQCIntfStackItem() :

    m_bBreakFlag(kCIDLib::False)
    , m_bProcessingCmd(kCIDLib::False)
    , m_bTOResumeFlag(kCIDLib::False)
    , m_piactxLevel(0)
{
}

TCQCIntfStackItem::~TCQCIntfStackItem()
{
    try
    {
        delete m_piactxLevel;
    }

    catch(...)
    {
    }
}


tCIDLib::TVoid
TCQCIntfStackItem::Initialize(  const   TCQCIntfTemplate&   iwdgInit
                                , const TCQCUserCtx&        cuctxToUse)
{
    if (m_piactxLevel)
        delete m_piactxLevel;
    m_piactxLevel = new TCQCIntfActCtx(cuctxToUse);

    m_iwdgTemplate = iwdgInit;
}

tCIDLib::TVoid TCQCIntfStackItem::Initialize(const TCQCUserCtx& cuctxToUse)
{
    if (m_piactxLevel)
        delete m_piactxLevel;
    m_piactxLevel = new TCQCIntfActCtx(cuctxToUse);
}

tCIDLib::TVoid TCQCIntfStackItem::Reset(TCQCIntfView* const pcivOwner)
{
    m_bBreakFlag = kCIDLib::False;
    m_bProcessingCmd = kCIDLib::False;
    m_bTOResumeFlag = kCIDLib::False;
    m_iwdgTemplate.ResetTemplate(pcivOwner);
    m_pntOffset.Set(0, 0);

    delete m_piactxLevel;
    m_piactxLevel = 0;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTmplMgr
//  PREFIX: ipui
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfTmplMgr: Constructors and Destructor
// ---------------------------------------------------------------------------

TCQCIntfTmplMgr::TCQCIntfTmplMgr(const TCQCUserCtx& cuctxToUse) :

    m_c4Count(0)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCIntfEng::c4MaxIntfDepth; c4Index++)
        m_aisiMgr[c4Index].Initialize(cuctxToUse);
}

TCQCIntfTmplMgr::TCQCIntfTmplMgr(const  TCQCIntfTemplate&   iwdgInit
                                , const TCQCUserCtx&        cuctxToUse) :

    m_c4Count(0)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCIntfEng::c4MaxIntfDepth; c4Index++)
        m_aisiMgr[c4Index].Initialize(cuctxToUse);

    // Store the initial template
    SetBaseTemplate(iwdgInit);
}

TCQCIntfTmplMgr::~TCQCIntfTmplMgr()
{
    // Clean up anything that didn't get cleaned up already
    Cleanup();
}


// ---------------------------------------------------------------------------
//  TCQCIntfTmplMgr: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Returns true if the top-most template is processing an action. The under
//  lying ones will always be set because in order to have a popup or popout,
//  the underlying one would have had to run an action to cause it and that
//  action will be blocked until the popup returns.
//
tCIDLib::TBoolean TCQCIntfTmplMgr::bActiveAction() const
{
    CIDAssert(m_c4Count <= kCQCIntfEng::c4MaxIntfDepth, L"Invalid template stack level");
    return m_aisiMgr[m_c4Count - 1].m_bProcessingCmd;
}


//
//  Provide access to the break flag for a given level. We need one that
//  returns a reference so that it can be passed into the modal loop for
//  each popup level.
//
tCIDLib::TBoolean TCQCIntfTmplMgr::bBreakFlag(const tCIDLib::TCard4 c4At) const
{
    CheckStackIndex(c4At, CID_LINE);
    return m_aisiMgr[c4At].m_bBreakFlag;
}

tCIDLib::TBoolean& TCQCIntfTmplMgr::bBreakFlag(const tCIDLib::TCard4 c4At)
{
    CheckStackIndex(c4At, CID_LINE);
    return m_aisiMgr[c4At].m_bBreakFlag;
}


//
//  This is called as a popup is closing. We copy the return values from the
//  top of stack (which is what we should receive as c4At, into the one
//  under it (the calling template context) and we return the stored return
//  code from it as well.
//
tCIDLib::TBoolean
TCQCIntfTmplMgr::bCopyRetVals(const tCIDLib::TCard4 c4At)
{
    if ((c4At + 1  != m_c4Count) || !c4At)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcView_InvalidTmplIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4At)
        );
    }

    const TCQCIntfActCtx& iacCur = *m_aisiMgr[c4At].m_piactxLevel;
    m_aisiMgr[c4At - 1].m_piactxLevel->TakeRetInfoFrom(iacCur);

    return iacCur.bPopupRes();
}


// If the count is greater than 1, we have popups
tCIDLib::TBoolean TCQCIntfTmplMgr::bHasPopups() const
{
    return m_c4Count > 1;
}


// Indicates whether the passed handler handler is valid
tCIDLib::TBoolean
TCQCIntfTmplMgr::bIsValidHandlerId(const tCIDLib::TCard4 c4HandlerId) const
{
    return (c4HandlerId < m_c4Count);
}


//
//  Indicates whether the template at the indicated index (handler
//  id) is processing a command at this time.
//
tCIDLib::TBoolean
TCQCIntfTmplMgr::bProcessingCmds(const tCIDLib::TCard4 c4At) const
{
    CheckStackIndex(c4At, CID_LINE);
    return m_aisiMgr[c4At].m_bProcessingCmd;
}


//
//  If there is a timeout popup displayed, return true. Note we start at
//  1, since the base template cannot be a popup.
//
tCIDLib::TBoolean TCQCIntfTmplMgr::bTOPopupDisplayed() const
{
    for (tCIDLib::TCard4 c4Index = 1; c4Index < m_c4Count; c4Index++)
    {
        if (m_aisiMgr[c4Index].m_bTOResumeFlag)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  This is called before the modal loop for a new template is entered,
//  to push the new template onto the stack and apply certain operations that
//  need to be done when a new template is pushed. Note that the caller has
//  already set the template contents into to the new stack level. We just
//  move up the count and set up the data for it.
//
//  This doesn't draw the newly pushed guy, it just get's him on the stack.
//  The calling code must subsequently call ShowTop() to display it.
//
//  The caller indicates if this is a timeout popup, which is a special
//  type and needs to be marked as such. Only one can be displayed and it
//  will not dismiss on Escape.
//
tCIDLib::TCard4
TCQCIntfTmplMgr::c4Push(        TCQCIntfView&       civOwner
                        , const tCIDLib::TBoolean   bTOPopup)
{
    // The stack can't be empty now
    CIDAssert(m_c4Count != 0, L"Can't push a popup without a base template");

    // Make sure we don't overflow
    if (m_c4Count >= kCQCIntfEng::c4MaxIntfDepth)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcView_TmplMgrOverflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Overflow
        );
    }

    // Get the current top for offset purposes
    const TCQCIntfStackItem& isiPrev = m_aisiMgr[m_c4Count - 1];
    const TCQCIntfTemplate& iwdgPrev = isiPrev.m_iwdgTemplate;

    // The new handler id is the current count, then we bump the count
    const tCIDLib::TCard4 c4HandlerId = m_c4Count++;

    // And get the new guy that have pushed now
    TCQCIntfStackItem& isiPush = m_aisiMgr[c4HandlerId];

    //
    //  Calculate the offset from the parent template. We have to keep this
    //  so that we can reposition it relative to the parent if the parent
    //  moves.
    //
    const TPoint pntOffset
    (
        isiPush.m_iwdgTemplate.areaActual().pntOrg() - iwdgPrev.areaActual().pntOrg()
    );

    //
    //  Now we can push the new guy on the stack. Note that we don't do any
    //  thing with the action context here. That's set up on each new action
    //  invocation so that it has the most recent information. We assume
    //  that the caller has already put the required info into the template
    //  at this slot.
    //
    isiPush.m_bBreakFlag = kCIDLib::False;
    isiPush.m_bProcessingCmd = kCIDLib::False;
    isiPush.m_pntOffset = pntOffset;

    //
    //  We store the passed timeout popup flag, which lets a popup be marked
    //  as a timeout popup. These have special considerations.
    //
    isiPush.m_bTOResumeFlag = bTOPopup;

    //
    //  Set the handler back on the template, since we overwrote it when we
    //  copied the new incoming template info.
    //
    isiPush.m_iwdgTemplate.c4HandlerId(c4HandlerId);

    return c4HandlerId;
}


//
//  This is called when the template window is being closed down, to clean
//  up any remaining stuff on our stack. There's shouldn't be any, but just
//  in case.
//
tCIDLib::TVoid TCQCIntfTmplMgr::Cleanup()
{
    // Start at the topmost and work down
    if (m_c4Count)
    {
        tCIDLib::TInt4 i4Index = tCIDLib::TInt4(m_c4Count);
        i4Index -= 1;

        while (i4Index >= 0)
        {
            try
            {
                m_aisiMgr[i4Index].Reset(0);
            }

            catch(...)
            {
                // <TBD> Log an error here
            }
            i4Index--;
        }
    }

    // Zero out the count now that we don't need it anymore
    m_c4Count = 0;
}


// Returns the combined area of all the templates
tCIDLib::TVoid TCQCIntfTmplMgr::QueryCombinedArea(TArea& areaComb) const
{
    areaComb = m_aisiMgr[0].m_iwdgTemplate.areaActual();
    for (tCIDLib::TCard4 c4Index = 1; c4Index < m_c4Count; c4Index++)
        areaComb |= m_aisiMgr[c4Index].m_iwdgTemplate.areaActual();
}


// Returns the count of templates currently on the stack
tCIDLib::TCard4 TCQCIntfTmplMgr::c4CurCount() const
{
    return m_c4Count;
}


// Returns the action context at the indicated index
TCQCIntfActCtx& TCQCIntfTmplMgr::iactxAt(const tCIDLib::TCard4 c4At)
{
    CheckStackIndex(c4At, CID_LINE);
    return *m_aisiMgr[c4At].m_piactxLevel;
}


// Returns the current top-most action context
TCQCIntfActCtx& TCQCIntfTmplMgr::iactxCur()
{
    // Gotta have at least one template
    CheckStackIndex(0, CID_LINE);
    return *m_aisiMgr[m_c4Count - 1].m_piactxLevel;
}


// Proivde access to the stack item at a specific index
const TCQCIntfStackItem&
TCQCIntfTmplMgr::isiAt(const tCIDLib::TCard4 c4At) const
{
    CheckStackIndex(0, CID_LINE);
    return m_aisiMgr[c4At];
}

TCQCIntfStackItem& TCQCIntfTmplMgr::isiAt(const tCIDLib::TCard4 c4At)
{
    CheckStackIndex(0, CID_LINE);
    return m_aisiMgr[c4At];
}


//
//  Provide access to the next stack item that will be used if another
//  popup is pushed. They cannot call iwdgTmplAt() since he does index
//  validation, and this one isn't valid yet. This method is used by the
//  popup pushing code.
//
TCQCIntfStackItem& TCQCIntfTmplMgr::isiNext(tCIDLib::TCard4& c4HandlerId)
{
    // Make sure we don't overflow
    if (m_c4Count >= kCQCIntfEng::c4MaxIntfDepth)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcView_TmplMgrOverflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Overflow
        );
    }
    c4HandlerId = m_c4Count;
    return m_aisiMgr[m_c4Count];
}


// Returns the base template which we often specially need to get to
const TCQCIntfTemplate& TCQCIntfTmplMgr::iwdgBaseTmpl() const
{
    CIDAssert(m_c4Count != 0, L"The template stack is empty");
    return m_aisiMgr[0].m_iwdgTemplate;
}

TCQCIntfTemplate& TCQCIntfTmplMgr::iwdgBaseTmpl()
{
    CIDAssert(m_c4Count != 0, L"The template stack is empty");
    return m_aisiMgr[0].m_iwdgTemplate;
}


// Get the template at a given index
TCQCIntfTemplate& TCQCIntfTmplMgr::iwdgTmplAt(const tCIDLib::TCard4 c4At)
{
    CheckStackIndex(c4At, CID_LINE);
    return m_aisiMgr[c4At].m_iwdgTemplate;
}

const TCQCIntfTemplate&
TCQCIntfTmplMgr::iwdgTmplAt(const tCIDLib::TCard4 c4At) const
{
    CheckStackIndex(c4At, CID_LINE);
    return m_aisiMgr[c4At].m_iwdgTemplate;
}


// Returns the current topmost template
const TCQCIntfTemplate& TCQCIntfTmplMgr::iwdgTopMost() const
{
    return m_aisiMgr[m_c4Count - 1].m_iwdgTemplate;
}

TCQCIntfTemplate& TCQCIntfTmplMgr::iwdgTopMost()
{
    return m_aisiMgr[m_c4Count - 1].m_iwdgTemplate;
}


//
//  The containing template view has changed size, so we need to re-calc all our
//  template positions. We set the base template position, and that kicks off a
//  recursive adjustment any contained content.
//
tCIDLib::TVoid
TCQCIntfTmplMgr::NewViewSize(const TSize& szNew, const tCIDLib::TBoolean bCentered)
{
    TCQCIntfStackItem& isiBase = m_aisiMgr[0];

    //
    //  Unless the size changes, at this level, we don't care about this. Movement of
    //  the view doesn't affect us.
    //
    if (szNew == isiBase.m_iwdgTemplate.areaActual().szArea())
        return;

    //
    //  Either center or upper-left justify. If the view wants us to center then center
    //  (as best we can.) Else we just set to the origin of the view.
    //
    TArea areaView(TPoint::pntOrigin, szNew);
    TArea areaToSet = isiBase.m_iwdgTemplate.areaActual();
    if (bCentered)
    {
        areaToSet.CenterIn(areaView);

        //
        //  Even if centering, if we are bigger than the new view area, then
        //  upper/left justify on either axis that is too small.
        //
        if (areaToSet.i4X() < 0)
            areaToSet.i4X(0);
        if (areaToSet.i4Y() < 0)
            areaToSet.i4Y(0);
    }
     else
    {
        areaToSet.SetOrg(areaView.pntOrg());
    }

    // Kick off the position update pass
    isiBase.m_iwdgTemplate.SetPositions(areaToSet.pntOrg(), areaToSet.pntOrg());
}


//
//  Finds the widget with the indicated unique widget id. It can search
//  the topmost only, or all the way down.
//
TCQCIntfWidget*
TCQCIntfTmplMgr::piwdgFindById( const   tCIDLib::TCard4     c4WidgetId
                                , const tCIDLib::TBoolean   bTopmostOnly)
{
    CIDAssert(m_c4Count != 0, L"The template manager stack is empty");

    TCQCIntfWidget* piwdgRet = 0;
    tCIDLib::TInt4 i4Level = tCIDLib::TInt4(m_c4Count - 1);
    while (i4Level > 0)
    {
        TCQCIntfTemplate& iwdgParent = m_aisiMgr[i4Level].m_iwdgTemplate;
        piwdgRet = iwdgParent.piwdgFindByUID(c4WidgetId, kCIDLib::True);
        if (piwdgRet)
            break;

        // Not found. If topmost only, then we've failed already
        if (bTopmostOnly)
            break;

        // Keep searching
        i4Level--;
    }
    return piwdgRet;
}


//
//  Undoes the stuff pushed by the push above. If a popout, the eDir indicates
//  the direction it popped out (and we have to pop back in the other way.)
//  If not a popout, the bFade indicates whether we actually got it successfully
//  displayed and therefore want to do a regular fadeout to close it.
//
//  In both cases we don't do those things in RIVA mode.
//
tCIDLib::TVoid
TCQCIntfTmplMgr::Pop(       TCQCIntfView&       civOwner
                    , const tCIDLib::EDirs      eDir
                    , const tCIDLib::TBoolean   bFade)
{
    // Make sure we don't underflow. We cannot drop the base template
    if (m_c4Count < 2)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcView_TmplMgrUnderflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Underflow
        );
    }

    // Get a ref to the one we are going to lose, then drop the count
    TCQCIntfStackItem& isiPop = m_aisiMgr[m_c4Count - 1];
    TCQCIntfTemplate& iwdgPop = isiPop.m_iwdgTemplate;
    const TArea areaPopup = iwdgPop.areaActual();
    m_c4Count--;

    // Get a convenience copy of the transparency setting
    const tCIDLib::TBoolean bIsTrans = iwdgPop.bIsTransparent();

    // Get a target graphics device to write to
    tCQCIntfEng::TGraphIntfDev cptrGDev = civOwner.cptrNewGraphDev();

    // Stop timers while we do this
    TCQCIntfTimerJan janTimers(&civOwner);

    if ((iwdgPop.eType() == tCQCIntfEng::ETmplTypes::Popout)
    &&  !TFacCQCIntfEng::bRemoteMode())
    {
        // Create a zero based area the size of our popup
        TArea areaZPopup = areaPopup;
        areaZPopup.ZeroOrg();

        //
        //  We are going to need a few memory devices, required by the
        //  easing scroll method we are going to use. We need one to
        //  capture the background content in and one for double buffering
        //  purposes, both of which need to be the size of the popup. ANd
        //  we need one that is twice as larger in the direction of travel.
        //
        tCQCIntfEng::TGraphIntfDev cptrCompat = civOwner.cptrNewGraphDev();

        TBitmap bmpBgn
        (
            areaPopup.szArea()
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevBgn(*cptrCompat, bmpBgn);

        // Grab the contents under the popup and copy to the bgn buffer.
        {
            TBitmap& bmpDB = civOwner.bmpQueryContentUnder(areaPopup);
            TGraphMemDev gdevUnder(*cptrCompat, bmpDB);
            gdevBgn.CopyBits(gdevUnder, areaPopup, areaZPopup);
        }

        // Now let any special widgets terminate early
        iwdgPop.TermWidgets();

        //
        //  Adjust the direction. The original choices were unfortunate.
        //  From the left and right are Left and Right, so they came out
        //  by going in the opposite directions and now the direction is
        //  the direction we want to go back. But from the top and bottom
        //  are down and up, so now we want to go the opopsite direction to
        //  put them back.
        //
        //  So flip if up or down.
        //
        tCIDLib::EDirs ePopDir = eDir;
        if (eDir == tCIDLib::EDirs::Up)
            ePopDir = tCIDLib::EDirs::Down;
        else if (eDir == tCIDLib::EDirs::Down)
            ePopDir = tCIDLib::EDirs::Up;

        //
        //  Now we need to draw into the scroll buffer at the correct
        //  spot.
        //
        if (bIsTrans)
        {
            TArea areaScroll(areaZPopup);
            if ((eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right))
                areaScroll.ScaleSize(2.0, 1.0);
            else
                areaScroll.ScaleSize(1.0, 2.0);

            TBitmap bmpScroll
            (
                areaScroll.szArea()
                , tCIDImage::EPixFmts::TrueAlpha
                , tCIDImage::EBitDepths::Eight
                , kCIDLib::True
            );
            TGraphMemDev gdevScroll(*cptrCompat, bmpScroll);

            //
            //  Set up the target area where we want to draw the contents
            //
            TArea areaTar(areaZPopup);
            if (ePopDir == tCIDLib::EDirs::Right)
               areaTar.Move(tCIDLib::EDirs::Right);
            else if (ePopDir == tCIDLib::EDirs::Down)
               areaTar.Move(tCIDLib::EDirs::Down);

            const TArea areaSave = iwdgPop.areaActual();
            iwdgPop.SetOrg(areaTar.pntOrg());
            iwdgPop.bDrawUnder(gdevScroll, areaTar);
            iwdgPop.SetArea(areaSave);

            TBitmap bmpBuf
            (
                areaPopup.szArea()
                , tCIDImage::EPixFmts::TrueAlpha
                , tCIDImage::EBitDepths::Eight
                , kCIDLib::True
            );
            TGraphMemDev gdevBuf(*cptrCompat, bmpBuf);

            //
            //  If the popup is transparent, then we have to deal with a
            //  special case issue. The popup is going to have a background
            //  image, which will have an alpha channel. But any drawing of
            //  text into the target device above is going to clear the
            //  alpha channel, which causes problems. So, we need to go back
            //  and re-apply the image's alpha channel.
            //
            //  It doesn't have to be the same size as the template, and any
            //  pixels not covered by the image will be set to fully
            //  tranparent. The image placement has to be one of the non-
            //  scaling types, so centered or upper left.
            //
            if (iwdgPop.bUseImage() || (iwdgPop.c1Opacity() < 0xFF))
            {
                //
                //  Calculate where in the popup the image actually is displayed
                //  so that we can reapply at the right position.
                //
                TArea areaImg(TPoint::pntOrigin, iwdgPop.szImgPrescaled());
                TArea areaImgTar;
                const tCIDGraphDev::EBltTypes eBltType = facCIDGraphDev().eCalcPlacement
                (
                    areaTar
                    , areaImg
                    , areaImgTar
                    , iwdgPop.ePlacement()
                    , kCIDLib::False
                );

                //
                //  If tiling, then ignore the fact that it might have alpha because
                //  we can't really handle that. It would cause an error because the
                //  image isn't the same size as the target area.
                //
                if (iwdgPop.bmpPreImage().bHasAlpha()
                &&  (eBltType != tCIDGraphDev::EBltTypes::Tile))
                {
                    bmpScroll.ReapplyAlphaMask
                    (
                        iwdgPop.bmpPreImage()
                        , *cptrCompat
                        , areaImg.pntOrg()
                        , areaImgTar.pntOrg()
                        , kCIDLib::False
                    );
                }
                 else
                {
                    bmpScroll.pixaData().SetAlphaAt
                    (
                        tCIDLib::MinVal(tCIDLib::TCard1(0xFF), iwdgPop.c1Opacity())
                        , areaImgTar
                    );
                }
            }

            TCIDEaseScroller escrShow;
            escrShow.Scroll1
            (
                *cptrGDev
                , areaPopup
                , areaScroll
                , gdevBuf
                , gdevBgn
                , gdevScroll
                , kCQCIntfEng_::c4EaseScrollMSs
                , ePopDir
                , kCQCIntfEng_::c4EaseOrder
            );
        }
         else
        {
            // Capture the popup contents in a zero origined buffer
            TBitmap bmpPopup
            (
                areaPopup.szArea()
                , tCIDImage::EPixFmts::TrueClr
                , tCIDImage::EBitDepths::Eight
            );
            TGraphMemDev gdevPop(*cptrCompat, bmpPopup);

            const TArea areaSave = iwdgPop.areaActual();
            iwdgPop.SetOrg(TPoint::pntOrigin);
            iwdgPop.bDrawUnder(gdevPop, areaZPopup);
            iwdgPop.SetArea(areaSave);

            //
            //  OK, now we can do the easing scroll operation. We use the simplest
            //  variation here. We pass it the popup contents and the bgn, and tell
            //  it in this case that this is a pop, not a push. It doesn't use the
            //  alpha channel, so it doesn't matter what happened to the alpha during
            //  the drawing.
            //
            TCIDEaseScroller escrShow;
            escrShow.Scroll4
            (
                *cptrGDev
                , areaPopup
                , gdevPop
                , gdevBgn
                , kCQCIntfEng_::c4EaseScrollMSs
                , ePopDir
                , kCQCIntfEng_::c4EaseOrder
                , kCIDLib::False
            );
        }
    }
     else if (bFade
          &&  civOwner.bDoFades()
          &&  !TFacCQCIntfEng::bRemoteMode())
    {
        //
        //  We can do a fade out and aren't in remote mode. So we'll fade it
        //  down to almost gone, and let the regular redraw of the underlying
        //  area below handle the final wipeout.
        //
        tCQCIntfEng::TGraphIntfDev cptrCompat = civOwner.cptrNewGraphDev();

        //
        //  Create a bitmap the size of the popup. Force it into memory
        //  so that we can reset it before each round.
        //
        TBitmap bmpPopup
        (
            areaPopup.szArea()
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
            , kCIDLib::True
        );
        TGraphMemDev gdevPopup(*cptrCompat, bmpPopup);

        // Create another for the background
        TBitmap bmpBgn
        (
            areaPopup.szArea()
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
            , kCIDLib::True
        );
        TGraphMemDev gdevBgn(*cptrCompat, bmpBgn);

        // Create an area for the popup size, at the origin
        const TArea areaSrcBlit(TPoint::pntOrigin, areaPopup.szArea());

        // Let any special widgets terminate early
        iwdgPop.TermWidgets();

        //
        //  Temporarily set the popout origin to zero and draw it into
        //  our popup memory device. We'll now have an image of the
        //  content to display. Set up an area that represents the
        //  zero origin'd image.
        //
        {
            const TArea areaSave = iwdgPop.areaActual();
            iwdgPop.SetOrg(TPoint::pntOrigin);
            iwdgPop.bDrawUnder(gdevPopup, areaSrcBlit);
            iwdgPop.SetArea(areaSave);
        }

        //
        //  If the popup has tranparency, we have to go back and update the alpha
        //  channel of the captured popup image. The reason being that any text
        //  drawn into the image will have messed up the alpha channel, and when
        //  we alpha blit it, it'll be a mess because the text bits will be full
        //  opacity, even if not drawn that way.
        //
        if (bIsTrans && iwdgPop.bUseImage() || (iwdgPop.c1Opacity() < 0xFF))
        {
            //
            //  Calculate where in the popup the image actually is displayed
            //  so that we can reapply at the right position.
            //
            TArea areaImg(TPoint::pntOrigin, iwdgPop.szImgPrescaled());
            TArea areaImgTar;
            const tCIDGraphDev::EBltTypes eBltType = facCIDGraphDev().eCalcPlacement
            (
                areaSrcBlit
                , areaImg
                , areaImgTar
                , iwdgPop.ePlacement()
                , kCIDLib::False
            );

            if (iwdgPop.bmpPreImage().bHasAlpha()
            &&  (eBltType != tCIDGraphDev::EBltTypes::Tile))
            {
                bmpPopup.ReapplyAlphaMask
                (
                    iwdgPop.bmpPreImage()
                    , *cptrCompat
                    , areaImg.pntOrg()
                    , areaImgTar.pntOrg()
                    , kCIDLib::False
                );
            }
             else
            {
                bmpPopup.pixaData().SetAlphaAt
                (
                    tCIDLib::MinVal(tCIDLib::TCard1(0xFF), iwdgPop.c1Opacity())
                    , areaImgTar
                );
            }
        }

        //
        //  Get the contents under where the popup will be, and put a device
        //  over it. The curernt layer has already been removed, so we set the
        //  last parm to false, otherwise we'd end up skipping the layer under
        //  us that we are trying to put back.
        //
        TBitmap& bmpDB = civOwner.bmpQueryContentUnder(areaPopup, kCIDLib::False);
        TGraphMemDev gdevBuf(*cptrCompat, bmpDB);

        // And now get a copy of that, so we have the underlying content
        gdevBgn.CopyBits(gdevBuf, areaPopup, areaSrcBlit);

        // Create a third popop sized one for mixing during the rounds
        TBitmap bmpMix
        (
            areaPopup.szArea()
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
            , kCIDLib::True
        );
        TGraphMemDev gdevMix(*cptrCompat, bmpMix);

        //
        //  Set up a simple opacity map for an optimal fade out. We will do a final
        //  redraw below, so we don't need to take it all the way down to zero.
        //
        const tCIDLib::TCard1 ac1FadeMap[] =
        {
            0xFF, 0xD0, 0xA0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x30
        };
        const tCIDLib::TCard4 c4Rounds = tCIDLib::c4ArrayElems(ac1FadeMap);

        for (tCIDLib::TCard4 c4FadeInd = 0; c4FadeInd < c4Rounds; c4FadeInd++)
        {
            // Copy the background to the mix
            gdevMix.CopyBits(gdevBgn, areaSrcBlit, areaSrcBlit);

            // Alpha blit the popup contents over that
            gdevMix.AlphaBlit
            (
                gdevPopup
                , areaSrcBlit
                , areaSrcBlit
                , ac1FadeMap[c4FadeInd]
                , bIsTrans
            );

            //
            //  And then do a regular blit of that to the screen. Tell it to
            //  wait for the vertical retrace. This both makes it smoother and
            //  prevents it from going too fast.
            //
            cptrCompat->CopyBits
            (
                gdevMix
                , areaSrcBlit
                , areaPopup
                , tCIDGraphDev::EBmpModes::SrcCopy
                , kCIDLib::True
            );
        }
    }

    //
    //  If in remote mode, this is all that will happen. If not, this insures we
    //  get the updated content behind the popup displayed. Something may have
    //  changed while we were fading or sliding.
    //
    //  Putting this down here insures that the main double buffering bmp is
    //  released.
    //
    civOwner.Redraw(areaPopup);

    // Now clean up any content of the one we popped
    try
    {
        iwdgPop.ResetTemplate(&civOwner);
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facCQCIntfEng().bLogFailures())
        {
            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngMsgs::midStatus_ExceptInTmplCleanup
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(...)
    {
        if (facCQCIntfEng().bLogFailures())
        {
            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngMsgs::midStatus_ExceptInTmplCleanup
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
}


//
//  We have a special method to set the base template. This one can only be
//  set if the stackis empty, and we need to nitialize stuff stuff since
//  this is the first widget to be displayed.
//
tCIDLib::TVoid
TCQCIntfTmplMgr::SetBaseTemplate(const TCQCIntfTemplate& iwdgToSet)
{
    CIDAssert(m_c4Count == 0, L"Can't set base template unless stack is empty");

    m_aisiMgr[0].m_iwdgTemplate = iwdgToSet;
    m_c4Count++;
}


// Set the break flag for a given template
tCIDLib::TVoid
TCQCIntfTmplMgr::SetBreakFlagAt(const   tCIDLib::TCard4     c4At
                                , const tCIDLib::TBoolean   bToSet)
{
    CheckStackIndex(c4At, CID_LINE);
    m_aisiMgr[c4At].m_bBreakFlag = bToSet;
}


// Set the processing commands flag for a given template
tCIDLib::TVoid
TCQCIntfTmplMgr::SetProcessingAt(const  tCIDLib::TCard4     c4At
                                , const tCIDLib::TBoolean   bToSet)
{
    CheckStackIndex(c4At, CID_LINE);
    m_aisiMgr[c4At].m_bProcessingCmd = bToSet;
}


//
//  Sets the top-most break flag, for those cases where the caller doesn't
//  know or care and just wants to break out of the top-most loop.
//
tCIDLib::TVoid TCQCIntfTmplMgr::SetTopBreakFlag(const tCIDLib::TBoolean bToSet)
{
    CIDAssert(m_c4Count != 0, L"The template manager stack is empty");
    m_aisiMgr[m_c4Count - 1].m_bBreakFlag = bToSet;
}


//
//  This is called before the modal loop for a new template is entered,
//  but after it has been pushed and any post init has been done. This
//  guy will do the work required to get the newly pushed (top) popup/popout
//  displayed.
//
tCIDLib::TVoid
TCQCIntfTmplMgr::ShowTop(       TCQCIntfView&   civOwner
                        , const tCIDLib::EDirs  eDir
                        , const tCIDLib::TCard1 c1Opacity)
{
    // Get the top one, that was just pushed and that we need to display
    TCQCIntfStackItem& isiPush = m_aisiMgr[m_c4Count - 1];
    TCQCIntfTemplate& iwdgPush = isiPush.m_iwdgTemplate;

    //
    //  Get the final target area for the popup out for convenience. Get
    //  a copy since we might move the widget below.
    //
    const TArea areaTo = iwdgPush.areaActual();

    //  Get a convenience copy of the transparency setting
    const tCIDLib::TBoolean bIsTrans = iwdgPush.bIsTransparent();

    // Get a target graphics device to write to
    tCQCIntfEng::TGraphIntfDev cptrGDev = civOwner.cptrNewGraphDev();

    //
    //  We have to process them differently based on whether it's a popup
    //  or popout. Supress popout stuff if in remote viewer mode.
    //
    if ((iwdgPush.eType() == tCQCIntfEng::ETmplTypes::Popout)
    &&  !TFacCQCIntfEng::bRemoteMode())
    {
        //
        //  It's a popup type, so we have to slide the new guy out. The caller
        //  has already set up the template and done an initial scan.
        //

        // Create a zero based area the size of our popup
        TArea areaZPopup = areaTo;
        areaZPopup.ZeroOrg();

        //
        //  We are going to need a few memory devices, required by the
        //  easing scroll method we are going to use. We need one to
        //  capture the background content in and one for double buffering
        //  purposes, both of which need to be the size of the popup. ANd
        //  we need one that is twice as larger in the direction of travel.
        //
        tCQCIntfEng::TGraphIntfDev cptrCompat = civOwner.cptrNewGraphDev();

        //
        //  Adjust the direction. The original choices were unfortunate.
        //  From the left and right are Left and Right. But that means
        //  the source, not the direction. So we need to flip right and
        //  left. Up and down are correct.
        //
        tCIDLib::EDirs ePushDir = eDir;
        if (ePushDir == tCIDLib::EDirs::Left)
            ePushDir = tCIDLib::EDirs::Right;
        else if (eDir == tCIDLib::EDirs::Right)
            ePushDir = tCIDLib::EDirs::Left;

        //
        //  Grab the contents under the popup and copy to the bgn buffer.
        //  In this case, we are already pushed, so we have to tell it to
        //  not include the top level.
        //
        TBitmap bmpBgn
        (
            areaTo.szArea(), tCIDImage::EPixFmts::TrueAlpha, tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevBgn(*cptrCompat, bmpBgn);
        {
            TBitmap& bmpDB = civOwner.bmpQueryContentUnder
            (
                areaTo, kCIDLib::True
            );
            TGraphMemDev gdevUnder(*cptrCompat, bmpDB);
            gdevBgn.CopyBits(gdevUnder, areaTo, areaZPopup);
        }

        //
        //  Set up the target area where we want to draw the contents
        //
        {
            TArea areaTar(areaZPopup);

            //
            //  Now we need to draw into the scroll buffer at the correct
            //  spot.
            //
            if (bIsTrans)
            {
                if (ePushDir == tCIDLib::EDirs::Left)
                    areaTar.Move(tCIDLib::EDirs::Right);
                else if (ePushDir == tCIDLib::EDirs::Up)
                    areaTar.Move(tCIDLib::EDirs::Down);

                TBitmap bmpBuf
                (
                    areaTo.szArea()
                    , tCIDImage::EPixFmts::TrueAlpha
                    , tCIDImage::EBitDepths::Eight
                    , kCIDLib::True
                );
                TGraphMemDev gdevBuf(*cptrCompat, bmpBuf);

                TArea areaScroll(areaZPopup);
                if ((eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right))
                    areaScroll.ScaleSize(2.0, 1.0);
                else
                    areaScroll.ScaleSize(1.0, 2.0);

                TBitmap bmpScroll
                (
                    areaScroll.szArea()
                    , tCIDImage::EPixFmts::TrueAlpha
                    , tCIDImage::EBitDepths::Eight
                    , kCIDLib::True
                );
                TGraphMemDev gdevScroll(*cptrCompat, bmpScroll);

                const TArea areaSave = iwdgPush.areaActual();
                iwdgPush.SetOrg(areaTar.pntOrg());
                iwdgPush.bDrawUnder(gdevScroll, areaTar);
                iwdgPush.SetArea(areaSave);

                //
                //  If the popup is transparent, then we have to deal with a
                //  special case issue. The popup is going to have a bgn
                //  image, which will have an alpha channel. But any drawing
                //  of text into the target device above is going to clear the
                //  alpha channel, which causes problems. So, we need to go
                //  back and re-apply the image's alpha channel.
                //
                //  It doesn't have to be the same size as the template, and
                //  any pixels not covered by the image will be set to fully
                //  tranparent.
                //
                if (iwdgPush.bUseImage() || (iwdgPush.c1Opacity() < 0xFF))
                {
                    //
                    //  Calculate where in the popup the image actually is displayed
                    //  so that we can reapply at the right position.
                    //
                    TArea areaImg(TPoint::pntOrigin, iwdgPush.szImgPrescaled());
                    TArea areaImgTar;
                    const tCIDGraphDev::EBltTypes eBltType = facCIDGraphDev().eCalcPlacement
                    (
                        areaTar
                        , areaImg
                        , areaImgTar
                        , iwdgPush.ePlacement()
                        , kCIDLib::False
                    );

                    //
                    //  If tiling, then ignore the fact that it might have alpha because
                    //  we can't really handle that. It would cause an error because the
                    //  image isn't the same size as the target area.
                    //
                    if (iwdgPush.bmpPreImage().bHasAlpha()
                    &&  (eBltType != tCIDGraphDev::EBltTypes::Tile))
                    {
                        bmpScroll.ReapplyAlphaMask
                        (
                            iwdgPush.bmpPreImage()
                            , *cptrCompat
                            , areaImg.pntOrg()
                            , areaImgTar.pntOrg()
                            , kCIDLib::False
                        );
                    }
                     else
                    {
                        bmpScroll.pixaData().SetAlphaAt
                        (
                            tCIDLib::MinVal(tCIDLib::TCard1(0xFF), iwdgPush.c1Opacity())
                            , areaImgTar
                        );
                    }
                }


                // OK, now we can do the easing scroll operation
                TCIDEaseScroller escrShow;
                escrShow.Scroll1
                (
                    *cptrGDev
                    , areaTo
                    , areaScroll
                    , gdevBuf
                    , gdevBgn
                    , gdevScroll
                    , kCQCIntfEng_::c4EaseScrollMSs
                    , ePushDir
                    , kCQCIntfEng_::c4EaseOrder
                );
            }
             else
            {
                //
                //  We need the zero origined, captured new contents, so we create a
                //  bitmap the size of the popout and draw the popup contents into
                //  that. The background was already captured above.
                //
                TBitmap bmpPush
                (
                    areaTo.szArea()
                    , tCIDImage::EPixFmts::TrueClr
                    , tCIDImage::EBitDepths::Eight
                );

                TGraphMemDev gdevPush(*cptrCompat, bmpPush);
                const TArea areaSave = iwdgPush.areaActual();
                iwdgPush.SetOrg(TPoint::pntOrigin);
                iwdgPush.bDrawUnder(gdevPush, areaTar);
                iwdgPush.SetArea(areaSave);

                //
                //  OK, now we can do the easing scroll operation. We use the simplest
                //  variation. We provide it the new content and the background under
                //  it and tell it this is a push operation. It doesn't require any
                //  alpha manipulation, so it doesn't matter what happened to the
                //  alpha channel during the drawing.
                //
                TCIDEaseScroller escrShow;
                escrShow.Scroll4
                (
                    *cptrGDev
                    , areaTo
                    , gdevPush
                    , gdevBgn
                    , kCQCIntfEng_::c4EaseScrollMSs
                    , ePushDir
                    , kCQCIntfEng_::c4EaseOrder
                    , kCIDLib::True
                );
            }
        }
    }
     else
    {
        //
        //  If not in remote mode and the view has fades enabled,
        //  then do a fade. Else just redraw the area of the popup.
        //
        if (!TFacCQCIntfEng::bRemoteMode() && civOwner.bDoFades())
        {
            //
            //  Let's do a fade in. We get the whole popup to draw into a
            //  bitmap, which we then do a quick set of alpha blend steps
            //  on in a loop to fade in.
            //
            tCQCIntfEng::TGraphIntfDev cptrCompat = civOwner.cptrNewGraphDev();

            //
            //  Create a bitmap the size of the popup. Force it into memory
            //  so that we can reset it before each round.
            //
            TBitmap bmpPopup
            (
                areaTo.szArea()
                , tCIDImage::EPixFmts::TrueAlpha
                , tCIDImage::EBitDepths::Eight
                , kCIDLib::True
            );
            TGraphMemDev gdevPopup(*cptrCompat, bmpPopup);

            // Create another for the background
            TBitmap bmpBgn
            (
                areaTo.szArea()
                , tCIDImage::EPixFmts::TrueAlpha
                , tCIDImage::EBitDepths::Eight
                , kCIDLib::True
            );
            TGraphMemDev gdevBgn(*cptrCompat, bmpBgn);

            // Create an area for the popup size, at the origin
            const TArea areaSrcBlit(TPoint::pntOrigin, areaTo.szArea());

            //
            //  Temporarily set the popout origin to zero and draw it into
            //  our popup memory device. We'll now have an image of the
            //  content to display.
            //
            {
                // Set up an area that represents the zero origin'd image
                const TArea areaSave = iwdgPush.areaActual();
                iwdgPush.SetOrg(TPoint::pntOrigin);
                iwdgPush.bDrawUnder(gdevPopup, areaSrcBlit);
                iwdgPush.SetArea(areaSave);
            }

            //
            //  If the popup has tranparency, we have to go back and update the
            //  alpha channel of the captured popup image. The reason being that
            //  any text drawn into the image will have messed up the alpha
            //  channel, and when we alpha blit it, it'll be a mess because the
            //  text bits will be full opacity, even if not drawn that way.
            //
            if (bIsTrans && iwdgPush.bUseImage() || (iwdgPush.c1Opacity() < 0xFF))
            {
                //
                //  Calculate where in the popup the image actually is displayed
                //  so that we can reapply at the right position.
                //
                TArea areaImg(TPoint::pntOrigin, iwdgPush.szImgPrescaled());
                TArea areaImgTar;
                const tCIDGraphDev::EBltTypes eBltType = facCIDGraphDev().eCalcPlacement
                (
                    areaSrcBlit
                    , areaImg
                    , areaImgTar
                    , iwdgPush.ePlacement()
                    , kCIDLib::False
                );

                //
                //  If tiling, then ignore the fact that it might have alpha because
                //  we can't really handle that. It would cause an error because the
                //  image isn't the same size as the target area.
                //
                if (iwdgPush.bmpPreImage().bHasAlpha()
                &&  (eBltType != tCIDGraphDev::EBltTypes::Tile))
                {
                    bmpPopup.ReapplyAlphaMask
                    (
                        iwdgPush.bmpPreImage()
                        , *cptrCompat
                        , areaImg.pntOrg()
                        , areaImgTar.pntOrg()
                        , kCIDLib::False
                    );
                }
                 else
                {
                    bmpPopup.pixaData().SetAlphaAt
                    (
                        tCIDLib::MinVal(tCIDLib::TCard1(0xFF), iwdgPush.c1Opacity())
                        , areaImgTar
                    );
                }
            }

            //
            //  Get the contents under where the popup will be, and put a
            //  device over it. The last parm says ignore the top-most layer,
            //  which is this new popup.
            //
            TBitmap& bmpDB = civOwner.bmpQueryContentUnder(areaTo, kCIDLib::True);
            {
                // And now get a copy of that, so we have the underlying content
                TGraphMemDev gdevBuf(*cptrCompat, bmpDB);
                gdevBgn.CopyBits(gdevBuf, areaTo, areaSrcBlit);
            }

            // Create a third popop sized one for mixing during the rounds
            TBitmap bmpMix
            (
                areaTo.szArea()
                , tCIDImage::EPixFmts::TrueAlpha
                , tCIDImage::EBitDepths::Eight
                , kCIDLib::True
            );
            TGraphMemDev gdevMix(*cptrCompat, bmpMix);

            // Set up an array of optimal opacities for the fade
            const tCIDLib::TCard1 ac1FadeMap[] =
            {
                0x20, 0x30, 0x40, 0x50, 0x60, 0x80, 0xAA, 0xD8, 0xFF
            };
            const tCIDLib::TCard4 c4Rounds = tCIDLib::c4ArrayElems(ac1FadeMap);

            for (tCIDLib::TCard4 c4FadeInd = 0; c4FadeInd < c4Rounds; c4FadeInd++)
            {
                // Copy the background to the mix
                gdevMix.CopyBits(gdevBgn, areaSrcBlit, areaSrcBlit);

                // Alpha blit the popup contents over that.
                gdevMix.AlphaBlit
                (
                    gdevPopup
                    , areaSrcBlit
                    , areaSrcBlit
                    , ac1FadeMap[c4FadeInd]
                    , bIsTrans
                );

                //
                //  And then do a regular blit of that to the screen. Tell it to
                //  wait for the vertical retrace. This both makes it smoother and
                //  prevents it from going too fast.
                //
                cptrCompat->CopyBits
                (
                    gdevMix
                    , areaSrcBlit
                    , areaTo
                    , tCIDGraphDev::EBmpModes::SrcCopy
                    , kCIDLib::True
                );
            }
        }
         else
        {
            civOwner.Redraw(iwdgPush.areaActual());
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfTmplMgr: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Check a popup stack index
tCIDLib::TVoid
TCQCIntfTmplMgr::CheckStackIndex(const  tCIDLib::TCard4 c4At
                                , const tCIDLib::TCard4 c4Line) const
{
    if (c4At >= m_c4Count)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , c4Line
            , kIEngErrs::errcView_InvalidTmplIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4At)
        );
    }
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfView
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfView: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfView::~TCQCIntfView()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfView: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  We may end up handling some of these at our level, so we wnat the derived class
//  to call us for any it doesn't do itself, so we ned an implementation.
//
tCIDLib::TVoid
TCQCIntfView::AsyncDataCallback(TCQCIntfADCB* const)
{
}


// If not overridden, we return false and timeout events will never happen
tCIDLib::TBoolean TCQCIntfView::bCheckTimeout()
{
    return kCIDLib::False;
}

// If not overridden, we just return a dummy 1x1 bitmap
TBitmap&
TCQCIntfView::bmpQueryContentUnder(const TArea&, const tCIDLib::TBoolean)
{
    // A dummy op for us if not overridden
    static TBitmap bmpDummy
    (
        TSize(1,1), tCIDImage::EPixFmts::GrayScale, tCIDImage::EBitDepths::One
    );
    return bmpDummy;
}

TBitmap&
TCQCIntfView::bmpQueryContentUnder( const   TArea&
                                    , const TCQCIntfWidget* const
                                    , const tCIDLib::TBoolean)
{
    // A dummy op for us if not overridden
    static TBitmap bmpDummy
    (
        TSize(1,1), tCIDImage::EPixFmts::GrayScale, tCIDImage::EBitDepths::One
    );
    return bmpDummy;
}



tCIDLib::TVoid TCQCIntfView::EndBranchReplace(TCQCIntfContainer&)
{
    // Empty, default impl
}


// A helper to safely update a variable outside the action system
tCIDLib::TVoid
TCQCIntfView::SetVariable(const TString&, const TString&)
{
    // Default is no-op if not overridden
}


tCIDLib::TVoid TCQCIntfView::StartBranchReplace(TCQCIntfContainer&)
{
    // Empty, default impl
}


tCIDLib::TVoid TCQCIntfView::Validate()
{
    // Default is to do nothing. The designer's editor window overrides this
}


// ---------------------------------------------------------------------------
//  TCQCIntfView: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfView::bInitialize(TDataSrvClient&, tCQCIntfEng::TErrList&)
{
    // For now, nothign to do
    return kCIDLib::True;
}


//
//  Default is to say no. This is used in some apps to allow the view to
//  intercept commands that would normally be directly dispatched by the
//  interface action engine, or to modify the values before they are.
//
tCIDLib::TBoolean
TCQCIntfView::bInterceptCmd(const   TString&
                            , const TString&
                            ,       TCQCCmdCfg&) const
{
    return kCIDLib::False;
}


//
//  This is overridden from the command target mixin. It allows us to tell
//  the action editor if we support global actions, and if so what their
//  names are, so that he can present them to the user for selection. In our
//  case, we don't use the command id or context for any per-event/cmd type
//  filtering.
//
tCIDLib::TBoolean TCQCIntfView::
bQueryTarSubActNames(       TCollection<TString>& colToFill
                    , const TString&
                    , const tCQCKit::EActCmdCtx) const
{
    colToFill.RemoveAll();

    // If we have any template actions, give him the names, else return false
    const TCQCIntfTemplate& iwdgEdit = iwdgTopMost();
    const tCIDLib::TCard4 c4Count = iwdgEdit.colActions().c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colToFill.objAdd(iwdgEdit.colActions()[c4Index].strTitle());
    return (c4Count != 0);
}


// We do some class specific parameter validation
tCIDLib::TBoolean
TCQCIntfView::bValidateParm(const   TCQCCmd&        cmdSrc
                            , const TCQCCmdCfg&     ccfgTar
                            , const tCIDLib::TCard4 c4Index
                            , const TString&        strValue
                            ,       TString&        strErrText)
{
    // Let the command mixin do the most basic stuff
    if (!MCQCCmdTarIntf::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    if (((ccfgTar.strCmdId() == kCQCIntfEng::strCmdId_InvokePopup) && (c4Index == 1))
    ||  ((ccfgTar.strCmdId() == kCQCIntfEng::strCmdId_InvokePopout) && (c4Index == 3))
    ||  ((ccfgTar.strCmdId() == kCQCIntfEng::strCmdId_SetBlankOpts) && (c4Index == 2)))
    {
        // Make sure we can parse this as a quoted comma list if not empty
        if (!strValue.bIsEmpty())
        {
            tCIDLib::TCard4  c4ErrInd;
            TVector<TString> colDummy;
            if (!TStringTokenizer::bParseQuotedCommaList(strValue, colDummy, c4ErrInd))
            {
                strErrText.LoadFromMsg
                (
                    kKitErrs::errcCmd_BadQuotedList, facCQCKit(), TCardinal(c4ErrInd)
                );
                return kCIDLib::False;
            }
        }
    }
     else if ((ccfgTar.strCmdId() == kCQCIntfEng::strCmdId_SetBlankOpts) && !c4Index)
    {
        // The blank period can be 0 (disable) or one minute or greater
        tCIDLib::TCard4 c4Val = 0;
        if (!strValue.bToCard4(c4Val) || (c4Val && (c4Val < 60000)))
        {
            strErrText = facCQCIntfEng().strMsg(kIEngMsgs::midStatus_BadBlankTime);
            return kCIDLib::False;
        }
    }
     else if ((ccfgTar.strCmdId() == kCQCIntfEng::strCmdId_SetTimeout)
          &&  (c4Index == 0))
    {
        // The blank period can be 0 (disable) or from 2 to 600 seconds
        tCIDLib::TCard4 c4Val = 0;
        if (!strValue.bToCard4(c4Val) || (c4Val && ((c4Val < 2) || (c4Val > 600))))
        {
            strErrText = facCQCIntfEng().strMsg(kIEngMsgs::midStatus_BadTimeout);
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  These are currently no-ops for us. They are called at the start and end of
//  an action (from the action engine thread that's running the action.)
//
tCIDLib::TVoid TCQCIntfView::CmdTarCleanup()
{
}

tCIDLib::TVoid TCQCIntfView::CmdTarInitialize(const TCQCActEngine&)
{
}


//
//  This part of the command target interface we can do here since it's the
//  same whether in view or design mode and it's needed in both. The actual
//  handling of commands is a viewing time thing and is handled in the view
//  derivatives, or in some cases they are remapped to the template.
//
tCIDLib::TVoid
TCQCIntfView::QueryCmds(        TCollection<TCQCCmd>&   colToFill
                        , const tCQCKit::EActCmdCtx     eContext) const
{
    // Wait for a list of drivers to reach a particular power state
    #if 0
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_CheckPowerTrans
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_CheckPowerTrans)
                , tCQCKit::ECmdPTypes::Text
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_WaitList)
            )
        );
    }
    #endif

    // Dismiss the blanker
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_DismissBlanker
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_DismissBlanker)
            , 0
        )
    );

    // Set/clear the do fades flag
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_DoFades
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_DoFades)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            )
        );
    }

    //
    //  Load a new template. Its a condition and has side effects so set the
    //  type. Within an overlay, this reloads the overlay. we only allow this
    //  command in certain contexts.
    //
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        TCQCCmd cmdDoLink
        (
            kCQCIntfEng::strCmdId_DoLink
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_DoLink)
            , tCQCKit::ECmdPTypes::Template
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
        );

        cmdDoLink.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdDoLink);
    }

    // We don't support these a preload or event context
    if ((eContext != tCQCKit::EActCmdCtx::Preload)
    &&  (eContext != tCQCKit::EActCmdCtx::IVEvent))
    {
        TCQCCmd& cmdExit = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_Exit
                , facCQCKit().strMsg(kKitMsgs::midCmd_Exit)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Result)
            )
        );
    }

    //
    //  Get an overlay parameter value at a given number. This will get mapped to
    //  to the overlay when a template is loaded into one. If it's not, then it will
    //  come to us and we'll ignore it.
    //
    {
        TCQCCmd cmdGetOverParam
        (
            kCQCIntfEng::strCmdId_GetOverlayParam
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_GetOverlayParam)
            , 2
        );

        cmdGetOverParam.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        cmdGetOverParam.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Number), TString(L"1, 2, 3, 4")
            )
        );

        colToFill.objAdd(cmdGetOverParam);
    }

    // Get a return value
    {
        TCQCCmd& cmdGetRetVal = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_GetRetVal
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_GetRetVal)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Name)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MustExist)
            )
        );
        cmdGetRetVal.eType(tCQCKit::ECmdTypes::Both);
    }

    // Get a template value
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        TCQCCmd& cmdGetTmplVal = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_GetTmplVal
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_GetTmplVal)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Name)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MustExist)
            )
        );
        cmdGetTmplVal.eType(tCQCKit::ECmdTypes::Both);
    }

    // Check if we are running in RIVA mode, or normal IV
    {
        TCQCCmd& cmdInRIVAMode = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_InRIVAMode
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_InRIVAMode)
                , 0UL
            )
        );
        cmdInRIVAMode.eType(tCQCKit::ECmdTypes::Conditional);
    }

    //
    //  This will get remapped in overlays, ignored in templates
    //
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdLoadAnc
        (
            kCQCIntfEng::strCmdId_LoadAncestor
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_LoadAncestor)
            , 2
        );

        cmdLoadAnc.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
                , tCQCKit::ECmdPTypes::Template
            )
        );

        cmdLoadAnc.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Target)
                , L"GrandParent, Great Grandparent, Root"
            )
        );

        cmdLoadAnc.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdLoadAnc);
    }


    // Invoke a template as a popup
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        TCQCCmd cmdPopup
        (
            kCQCIntfEng::strCmdId_InvokePopup
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_InvokePopup)
            , 4
        );

        cmdPopup.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
                , tCQCKit::ECmdPTypes::Template
            )
        );

        cmdPopup.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Parms)
                , tCQCKit::ECmdPTypes::QuotedList
            )
        );

        cmdPopup.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Origin)
                , tCQCKit::ECmdPTypes::Point
            )
        );

        cmdPopup.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_Alignment)
                , L"Center, Lower Left, Lower Right, Upper Left, Upper Right"
            )
        );

        // The params are optional and it can be conditional or not
        cmdPopup.SetParmReqAt(1, kCIDLib::False);
        cmdPopup.eType(tCQCKit::ECmdTypes::Both);

        colToFill.objAdd(cmdPopup);
    }

    // Invoke a template as a slideing popup (popout)
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        TCQCCmd cmdPopout
        (
            kCQCIntfEng::strCmdId_InvokePopout
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_InvokePopout)
            , 4
        );
        cmdPopout.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
                , tCQCKit::ECmdPTypes::Template
            )
        );

        cmdPopout.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Direction)
                , L"From Top, From Bottom, From Left, From Right"
            )
        );

        cmdPopout.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Origin)
                , tCQCKit::ECmdPTypes::Point
            )
        );

        cmdPopout.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Parms)
                , tCQCKit::ECmdPTypes::QuotedList
            )
        );

        // The params param is optional, and it can be conditional or not
        cmdPopout.SetParmReqAt(3, kCIDLib::False);
        cmdPopout.eType(tCQCKit::ECmdTypes::Both);

        colToFill.objAdd(cmdPopout);
    }

    // Load the default template into the target container
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        TCQCCmd& cmdLoadDef = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_LoadDefTmpl
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_LoadDefTmpl)
                , 0
            )
        );
        cmdLoadDef.eType(tCQCKit::ECmdTypes::Both);
    }

    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        //
        //  In this case, the target parameter has to be just text because
        //  we don't know what the parent container will be. They have to
        //  just type in the name of the target overlay that they know will
        //  be there.
        //
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_LoadSiblingOverlay
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_LoadSiblingOverlay)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Target)
                , tCQCKit::ECmdPTypes::Template
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
            )
        );
    }

    //
    //  Allow them to tell us to suppress mouse movement messages to child
    //  windows, which causes a problem in some ActiveX objects in web
    //  widgets. This will be used to set a flag in the modal loop of a
    //  derived view class that deals with window based views, and hence can
    //  support web widgets.
    //
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_NoChildMouse
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_NoChildMouse)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            )
        );
    }

    // Copy a return value to a template value
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        TCQCCmd& cmdRVToTV = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_RetValToTmplVal
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_RetValToTmplVal)
                , tCQCKit::ECmdPTypes::Text
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_RetVal)
                , tCQCKit::ECmdPTypes::Text
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_TmplVal)
            )
        );
    }

    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        TCQCCmd& cmdRIVACmd = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_RIVACmd
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_RIVACmd)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Command)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value1)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value2)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value3)
            )
        );

        // The parameters aren't required
        cmdRIVACmd.SetParmReqAt(1, kCIDLib::False);
        cmdRIVACmd.SetParmReqAt(2, kCIDLib::False);
        cmdRIVACmd.SetParmReqAt(3, kCIDLib::False);
    }

    //
    //  Run a template level action on the containing template or
    //  a parent template if loaded into an overlay. We only allow
    //  it in standard action contexts.
    //
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_RunTmplAction
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_RunTmplAction)
                , tCQCKit::ECmdPTypes::TarSubAction
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Target)
            )
        );
    }

    //
    //  This will get remapped in overlays, ignored in templates. It's only
    //  supported in standard command contexts.
    //
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        TCQCCmd cmdScrPar
        (
            kCQCIntfEng::strCmdId_ScrollParent
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_ScrollParent)
            , 2
        );

        cmdScrPar.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_ScrollDir), L"Left, Right"
            )
        );

        cmdScrPar.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Target)
                , L"Parent, GrandParent, Great Grandparent, Root"
            )
        );

        colToFill.objAdd(cmdScrPar);
    }

    // Set the auto-blanker mode
    {
        TCQCCmd cmdSetAB
        (
            kCQCIntfEng::strCmdId_SetAutoBlankMode
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetAutoBlankMode)
            , 1
        );

        cmdSetAB.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Mode)
                , L"Blanker, Blank/Blanker, ClockBlanker, DisplayOff, ShowBlanker"
            )
        );
        colToFill.objAdd(cmdSetAB);
    }

    // Set blanker options
    {
        TCQCCmd& cmdSetBOpts = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetBlankOpts
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetBlankOpts)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MilliSecs)
                , tCQCKit::ECmdPTypes::Template
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
                , tCQCKit::ECmdPTypes::QuotedList
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Parms)
            )
        );

        // The template parm and its parms are optional
        cmdSetBOpts.SetParmReqAt(1, kCIDLib::False);
        cmdSetBOpts.SetParmReqAt(2, kCIDLib::False);
    }

    // Set the image for the background, not supported in event handlers
    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetImage
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetImage)
                , tCQCKit::ECmdPTypes::ImgPath
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_ImagePath)
            )
        );
    }

    // Allows a widget to hide it's parent. Only payed attention to by overlays
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetParentState
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetParentState)
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_WdgState)
                , L"Hidden, Disabled, Normal"
            )
        );
    }

    // Set a return value
    {
        TCQCCmd& cmdSetRetVal = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetRetVal
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetRetVal)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Name)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
            )
        );

        // The value can be empty
        cmdSetRetVal.SetParmReqAt(1, kCIDLib::False);
    }

    // Set a template value
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        TCQCCmd& cmdSetTmplVal = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetTmplVal
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetTmplVal)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Name)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
            )
        );

        // The value can be empty
        cmdSetTmplVal.SetParmReqAt(1, kCIDLib::False);
    }

    //
    //  Set a timeout timer on the template. It can be regularly or after
    //  a given period of inactivity by the user.
    //
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetTimeout
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetTimeout)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Seconds)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_Inactivity)
            )
        );
    }

    // Set the color used by the viewer to surround the template contents
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetViewBorderClr
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetViewBorderClr)
                , tCQCKit::ECmdPTypes::Color
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Color)
            )
        );
    }

    // Copy a template value to a return value
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        TCQCCmd& cmdTVToRV = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_TmplValToRetVal
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_TmplValToRetVal)
                , tCQCKit::ECmdPTypes::Text
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_TmplVal)
                , tCQCKit::ECmdPTypes::Text
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_RetVal)
            )
        );
    }

    // Wait for a list of drivers to reach a particular power state
    #if 0
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_WaitPowerTrans
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_WaitPowerTrans)
                , tCQCKit::ECmdPTypes::Text
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_WaitList)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Milliseconds)
                , tCQCKit::ECmdPTypes::Template
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_WaitTmplPath)
            )
        );
    }
    #endif
}


// A no op here, provided by the viewer window to run actions
tCIDLib::TVoid
TCQCIntfView::PostOps(          TCQCIntfWidget&
                        ,       MCQCCmdSrcIntf&
                        , const TString&
                        , const tCIDLib::TCard4
                        , const tCQCKit::EActOrders)
{
}

tCIDLib::TVoid
TCQCIntfView::PostOps(          TCQCIntfWidget&
                        ,       MCQCCmdSrcIntf&
                        , const TString&
                        , const tCIDLib::TCard4
                        ,       TCQCIntfActCtx&
                        ,       MCQCCmdTarIntf* const
                        , const tCQCKit::EActOrders)
{
}


// Set up default parameter values for some commands
tCIDLib::TVoid TCQCIntfView::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_InvokePopup)
    {
        ccfgToSet.piAt(2).m_strValue = L"-1, -1";
        ccfgToSet.piAt(3).m_strValue = L"Center";
    }
     else if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_InvokePopout)
    {
        ccfgToSet.piAt(1).m_strValue = L"From Top";
    }
     else if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_SetBlankOpts)
    {
        ccfgToSet.piAt(0).m_strValue = L"60000";
    }
     else if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_SetTimeout)
    {
        ccfgToSet.piAt(0).m_strValue = L"5";
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfView: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get /setthe do fades flag
tCIDLib::TBoolean TCQCIntfView::bDoFades() const
{
    return m_bDoFades;
}

tCIDLib::TBoolean TCQCIntfView::bDoFades(const tCIDLib::TBoolean bToSet)
{
    m_bDoFades = bToSet;
    return m_bDoFades;
}


// Get/set the template caching setting for this view
tCIDLib::TBoolean TCQCIntfView::bEnableTmplCaching() const
{
    return m_bEnableTmplCaching;
}

tCIDLib::TBoolean
TCQCIntfView::bEnableTmplCaching(const tCIDLib::TBoolean bToSet)
{
    m_bEnableTmplCaching = bToSet;
    return m_bEnableTmplCaching;
}



//
//  This indicates to the caller whether there's anything going that would mean
//  they shouldn't kick off an action right now.
//
//  If there's an active action, or we are tracking a gesture widget (or in a click
//  handler), then we are busy from our point of view. That mostly is used to prevent
//  starting any event or timer based actions. The janitor used to insure a widget
//  doesn't get destroyed while it's being tracked is checked to see if it currently
//  has a reference. The click handler also sets it during the processing of the/
//  click.
//
tCIDLib::TBoolean TCQCIntfView::bIsBusy() const
{
    return (bActiveAction()) || m_janGesture.bHasRef();
}


//
//  This is called by the containing program to let us respond to gestures.
//  On start of a gesture we remember the target widget, and pass the
//  subsequent move/end reports to that widget. We mark the widget being
//  dragged as 'no redraw' so it won't update while we are dragging it.
//
tCIDLib::TBoolean
TCQCIntfView::bProcessGestEv(const  tCIDCtrls::EGestEvs   eEv
                            , const TPoint&             pntStart
                            , const TPoint&             pntAt
                            , const TPoint&             pntDelta
                            , const tCIDLib::TBoolean   bTwoFinger)
{
    // Update the last activity time
    UpdateLastActivityStamp();

    //
    //  See if a widget accepted the gesture in the per-gesture options
    //  setup. If not, nothing to do.
    //
    TCQCIntfWidget* piwdgTar = m_janGesture.piwdgRef();
    if (!piwdgTar)
        return kCIDLib::False;

    // If we do, but it's since been killed, then drop it and we are done
    if (piwdgTar->bIsDead())
    {
        m_janGesture.DropRef();
        return kCIDLib::False;
    }

    tCIDLib::TBoolean bRet;
    try
    {
        // On start, mark it as in a gseture before we make the first call
        if (eEv == tCIDCtrls::EGestEvs::Start)
            piwdgTar->bInGesture(kCIDLib::True);

        //
        //  Let the widget see the getsure. If it's a drag and the point is not
        //  in the widget's area, only let it be seen if the widget has set the
        //  'track beyond' cap flag.
        //
        if ((eEv == tCIDCtrls::EGestEvs::Drag)
        &&  !piwdgTar->areaActual().bContainsPoint(pntAt)
        &&  !piwdgTar->bTrackBeyond())
        {
            // Don't let the widget see this move
            bRet = kCIDLib::True;
        }
         else
        {
            bRet = piwdgTar->bProcessGestEv
            (
                eEv, pntStart, pntAt, pntDelta, bTwoFinger
            );
        }

        //
        //  If this was the end of gesture, or the widget was killed during
        //  this processing, then drop it, which will cause the subsequent
        //  events for this gesture to be ignored.
        //
        //  Remove the no-redraw flag as well.
        //
        if ((eEv == tCIDCtrls::EGestEvs::End) || piwdgTar->bIsDead())
        {
            piwdgTar->bInGesture(kCIDLib::False);
            m_janGesture.DropRef();
        }
    }

    catch(TError& errToCatch)
    {
        //
        //  If this was the end of gesture, or the widget was killed during
        //  this processing, then drop it, which will cause the subsequent
        //  events for this gesture to be ignored.
        //
        if (piwdgTar)
        {
            piwdgTar->bInGesture(kCIDLib::False);
            if ((eEv == tCIDCtrls::EGestEvs::End) || piwdgTar->bIsDead())
                m_janGesture.DropRef();
        }

        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        bRet = kCIDLib::False;
    }

    catch(...)
    {
        //
        //  If this was the end of gesture, or the widget was killed during
        //  this processing, then drop it, which will cause the subsequent
        //  events for this gesture to be ignored.
        //
        if (piwdgTar)
        {
            piwdgTar->bInGesture(kCIDLib::False);
            if ((eEv == tCIDCtrls::EGestEvs::End) || piwdgTar->bIsDead())
                m_janGesture.DropRef();
        }
        bRet = kCIDLib::False;
    }
    return bRet;
}


// Just provides access to the global variables target for this view instance
TStdVarsTar& TCQCIntfView::ctarGlobalVars()
{
    return m_ctarGlobalVars;
}


tCIDLib::TVoid TCQCIntfView::Clicked(const TPoint& pntAt)
{
    // See if we hit a widget
    TCQCIntfTemplate& iwdgCont = iwdgTopMost();

    //
    //  If the point is outside of the area of the top-most template, then it can't
    //  be a regular hit type event. However we do allow them react to this.
    //
    if (!iwdgCont.areaActual().bContainsPoint(pntAt))
    {
        // Tell the template to invoke any actions it has for its external click event
        iwdgCont.DoExtClick(pntAt);
        return;
    }

    // Not that special case so do hit testing
    TCQCIntfWidget* piwdgTar = iwdgCont.piwdgHit
    (
        pntAt, tCQCIntfEng::EHitFlags::Widgets
    );

    //
    //  If the widget state is not normal, then do nothing. We special case this
    //  here so as to avoid every widget having to implement this check.
    //
    //  Note that this is separate from a gesture, so the PerGestOpts() method's
    //  checking of the display state doesn't happen. So we have to check it
    //  here as well.
    //
    if (piwdgTar && (piwdgTar->eCurDisplayState() != tCQCIntfEng::EDispStates::Normal))
        piwdgTar = 0;

    // If so, call his clicked method
    if (piwdgTar)
    {
        try
        {
            // Make sure it can't go away while we are processing this cmd.
            TCQCIntfWdgJanitor janClick(piwdgTar);

            //
            //  Get the old focus widget and tell it it lost and then tell
            //  new guy he got it. Only do this if focus is enabled.
            //
            if (!iwdgCont.bNoFocus())
            {
                TCQCIntfWidget* piwdgOld = iwdgCont.piwdgFindCurFocus();
                if (piwdgOld != piwdgTar)
                {
                    if (piwdgOld)
                    {
                        piwdgOld->bHasFocus(kCIDLib::False);
                        piwdgOld->LostFocus();
                    }
                }

                piwdgTar->bHasFocus(kCIDLib::True);
                piwdgTar->GotFocus();
            }

            // And now click it
            piwdgTar->Clicked(pntAt);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }
    }
}


//
//  These are helpers that we provide for text drawing that requires temporary
//  buffers, f/x based or alpha based text. To avoid having to constantly allocate
//  and delete these possibly fairly large buffers, we allocate a set and provide
//  these methods for widgets to use. No given view is ever used from more than one
//  thread at a time. In the RIVA server each session has its own view. So we can
//  easily get away with this and provide a lot of performance benefit.
//
//  We just pass the call on to the caller's provided device target, passing allong
//  all of the parameters plus the required buffers and pixel array.
//
//  The buffers are allocated to reasonable sizes initially, and we will size
//  them up more if required. If we do need to size up, we go further than needed
//  so as to possibly avoid subsequent resizing.
//
tCIDLib::TVoid
TCQCIntfView::DrawMTextAlpha(       TGraphDrawDev&      gdevTar
                            , const TString&            strText
                            , const TArea&              areaFormat
                            , const tCIDLib::EHJustify  eHJustify
                            , const tCIDLib::EVJustify  eVJustify
                            , const tCIDLib::TBoolean   bWordBreak
                            , const tCIDLib::TCard1     c1ConstAlpha)
{
    // If the string is empty, do nothing
    if (strText.bIsEmpty())
        return;

    // Check the buffer size and make sure it's reasonable
    CheckTextBufSz(areaFormat.c4Width(), areaFormat.c4Height());

    gdevTar.DrawMTextAlpha
    (
        strText
        , areaFormat
        , m_bmpTBuf
        , m_bmpTMask
        , eHJustify
        , eVJustify
        , bWordBreak
        , c1ConstAlpha
    );
}

tCIDLib::TVoid
TCQCIntfView::DrawTextAlpha(        TGraphDrawDev&          gdevTar
                            , const TString&                strText
                            , const tCIDLib::TCard4         c4StartInd
                            , const tCIDLib::TCard4         c4Len
                            , const TArea&                  areaFormat
                            , const tCIDLib::EHJustify      eHJustify
                            , const tCIDLib::EVJustify      eVJustify
                            , const tCIDGraphDev::ETextFmts eFormat
                            , const tCIDLib::TCard1         c1ConstAlpha
                            , const TRGBClr&                rgbBgnFill)
{
    // If the string is empty, do nothing
    if (strText.bIsEmpty())
        return;

    // Check the buffer size and make sure it's reasonable
    CheckTextBufSz(areaFormat.c4Width(), areaFormat.c4Height());

    gdevTar.DrawStringAlpha
    (
        strText
        , c4StartInd
        , c4Len
        , areaFormat
        , m_bmpTBuf
        , m_bmpTMask
        , eHJustify
        , eVJustify
        , eFormat
        , c1ConstAlpha
        , rgbBgnFill
    );
}

tCIDLib::TVoid
TCQCIntfView::DrawTextAlpha(        TGraphDrawDev&          gdevTar
                            , const TString&                strText
                            , const TArea&                  areaFormat
                            , const tCIDLib::EHJustify      eHJustify
                            , const tCIDLib::EVJustify      eVJustify
                            , const tCIDGraphDev::ETextFmts eFormat
                            , const tCIDLib::TCard1         c1ConstAlpha
                            , const TRGBClr&                rgbBgnFill)
{
    // If the string is empty, do nothing
    if (strText.bIsEmpty())
        return;

    // Check the buffer size and make sure it's reasonable
    CheckTextBufSz(areaFormat.c4Width(), areaFormat.c4Height());

    // Pass 0 and the string length
    gdevTar.DrawStringAlpha
    (
        strText
        , 0
        , strText.c4Length()
        , areaFormat
        , m_bmpTBuf
        , m_bmpTMask
        , eHJustify
        , eVJustify
        , eFormat
        , c1ConstAlpha
        , rgbBgnFill
    );
}

tCIDLib::TVoid
TCQCIntfView::DrawTextFX(       TGraphDrawDev&          gdevTar
                        , const TString&                strText
                        , const TArea&                  areaDraw
                        , const tCIDGraphDev::ETextFX  eEffect
                        , const TRGBClr&                rgbClr1
                        , const TRGBClr&                rgbClr2
                        , const tCIDLib::EHJustify      eHJustify
                        , const tCIDLib::EVJustify      eVJustify
                        , const tCIDLib::TBoolean       bNoTextWrap
                        , const TPoint&                 pntOffset
                        , const tCIDLib::TCard1         c1ConstAlpha)
{
    // Check the buffer size and make sure it's reasonable
    CheckTextBufSz(areaDraw.c4Width(), areaDraw.c4Height());

    gdevTar.DrawStringFX
    (
        strText
        , areaDraw
        , eEffect
        , rgbClr1
        , rgbClr2
        , eHJustify
        , eVJustify
        , bNoTextWrap
        , pntOffset
        , m_bmpTBuf
        , m_bmpTMask
        , m_pixaTBlur
        , c1ConstAlpha
    );
}


// Get/set the auto-blanker mode
tCQCIntfEng::ESpecActs TCQCIntfView::eAutoBlankMode() const
{
    return m_eAutoBlankMode;
}

tCQCIntfEng::ESpecActs
TCQCIntfView::eAutoBlankMode(const tCQCIntfEng::ESpecActs eToSet)
{
    m_eAutoBlankMode = eToSet;
    return m_eAutoBlankMode;
}


tCIDLib::TVoid
TCQCIntfView::GestInertiaIdle(  const   tCIDLib::TEncodedTime   enctEnd
                                , const tCIDLib::TCard4         c4Millis
                                , const tCIDLib::EDirs          eDir)
{
    //
    //  See if a widget accepted the gesture in the per-gesture options
    //  setup. If not, nothing to do.
    //
    TCQCIntfWidget* piwdgTar = m_janGesture.piwdgRef();
    if (!piwdgTar)
        return;

    // If we do, but it's since been killed, then drop it and we are done
    if (piwdgTar->bIsDead())
    {
        m_janGesture.DropRef();
        return;
    }

    try
    {
        piwdgTar->GestInertiaIdle(enctEnd, c4Millis, eDir);
    }

    catch(TError& errToCatch)
    {
        //
        //  If the widget was killed during this processing, then drop it, which
        //  will cause the subsequentevents for this gesture to be ignored.
        //
        if (piwdgTar->bIsDead())
            m_janGesture.DropRef();

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



// Provide access to our installed application handler, throwing if not set
MCQCIntfAppHandler& TCQCIntfView::miahOwner() const
{
    CIDAssert(m_pmiahToUse != 0, L"The interface view app handler has not been set");
    return *m_pmiahToUse;
}


//
//  Return the action trace provide by the containing apps interface action
//  handler if he provided one.
//
MCQCCmdTracer* TCQCIntfView::pmcmdtTrace()
{
    CIDAssert(m_pmiahToUse != 0, L"The interface view app handler has not been set");
    return m_pmiahToUse->pmcmdtTrace();
}





tCIDLib::TVoid
TCQCIntfView::PerGestOpts(  const   TPoint&                 pntAt
                            , const tCIDLib::EDirs          eDir
                            , const tCIDLib::TBoolean       bTwoFingers
                            ,       tCIDCtrls::EGestOpts&     eToSet
                            ,       tCIDLib::TFloat4&       f4VScale)
{
    // Make sure the widget gesture target is zeroed to start
    m_janGesture.DropRef();

    //
    //  If it's two fingered, then look for the container this is in.
    //  Else, look for a widget. If no widget hit, or the hit widget
    //  doesn't accept gestures, then pass it on to the parent container
    //  to see if he wants it.
    //
    TCQCIntfWidget* piwdgTar = 0;
    if (bTwoFingers)
    {
        piwdgTar = iwdgTopMost().piwdgHit(pntAt, tCQCIntfEng::EHitFlags::Conts);
    }
     else
    {
        piwdgTar = iwdgTopMost().piwdgHit(pntAt, tCQCIntfEng::EHitFlags::Widgets);

        if (!piwdgTar || (piwdgTar && !piwdgTar->bDoesGestures()))
        {
            piwdgTar = iwdgTopMost().piwdgHit
            (
                pntAt, tCQCIntfEng::EHitFlags::Conts
            );
        }
    }

    //
    //  If we hit someone, ask it for options. If it doesn't accept it, and
    //  returns true, then we allow it to pass to the parent container, until
    //  we get to the root of that layer.
    //
    while (piwdgTar)
    {
        // If the widget state is disabled, we just do nothing.
        if (piwdgTar->eCurDisplayState() == tCQCIntfEng::EDispStates::Disabled)
        {
            piwdgTar = 0;
            break;
        }

        //
        //  If it's in normal state, it gets a change to do the gesture. Otherwise,
        //  it's hidden, and we just keep drilling down.
        //
        if (piwdgTar->eCurDisplayState() == tCQCIntfEng::EDispStates::Normal)
        {
            const tCIDLib::TBoolean bPropogate = piwdgTar->bPerGestOpts
            (
                pntAt, eDir, bTwoFingers, eToSet, f4VScale
            );

            // If they accepted, break out
            if (eToSet != tCIDCtrls::EGestOpts::Disable)
                break;

            // They didn't accept. If they don't want to propogate, give up
            if (!bPropogate)
                break;
        }

        // Else see if we have a parent to pass it to
        piwdgTar = piwdgTar->piwdgParent();
    }

    //
    //  Just to be sure, if we propogated to the limit, make sure the gesture
    //  option is disable, since we don't have a target widget now. It shouldn't
    //  be possible for it to be anything but disabled at this point, but don't
    //  bother standing principle.
    //
    if (!piwdgTar)
        eToSet = tCIDCtrls::EGestOpts::Disable;

    //
    //  If he we hit something and he accepted the gesture, then it's going to
    //  happen, so put this widget into the gesture janitor, to make sure he will
    //  not be destroyed until the gesture is completed.
    //
    if (eToSet != tCIDCtrls::EGestOpts::Disable)
        m_janGesture.SetNew(piwdgTar);
}


tCIDLib::TVoid
TCQCIntfView::ProcessFlick(const tCIDLib::EDirs eDir, const TPoint& pntStart)
{
    // Update the last activity time
    UpdateLastActivityStamp();

    //
    //  See if we have a gesture widget from the gesture opts setup. If not,
    //  then there's nothing to do.
    //
    TCQCIntfWidget* piwdgTar = m_janGesture.piwdgRef();
    if (!piwdgTar)
        return;

    // If we do, but it's since been killed, then drop it
    if (piwdgTar->bIsDead())
    {
        m_janGesture.DropRef();
        return;
    }

    //
    //  Pass it the gesture, and then drop our reference, since we don't
    //  need it anymore.
    //
    try
    {
        // Mark the target as being in a gesture, then invoke the flick
        piwdgTar->bInGesture(kCIDLib::True);
        piwdgTar->ProcessFlick(eDir, pntStart);

        // Unmark the target widget again now and drop the reference
        piwdgTar->bInGesture(kCIDLib::False);
        m_janGesture.DropRef();
    }

    catch(TError& errToCatch)
    {
        piwdgTar->bInGesture(kCIDLib::False);
        m_janGesture.DropRef();

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



// Get or set the view background border color
const TRGBClr& TCQCIntfView::rgbBorder() const
{
    return m_rgbBorder;
}

const TRGBClr& TCQCIntfView::rgbBorder(const TRGBClr& rgbToSet)
{
    m_rgbBorder = rgbToSet;
    return m_rgbBorder;
}


//
//  Return the name of the base template. We can do this in terms of the
//  virtual method above that gets the base template.
//
const TString& TCQCIntfView::strBaseTmplName() const
{
    return iwdgBaseTmpl().strTemplateName();
}


//
//  Return the name of the currently active template. We can do this in terms
//  of the virtual method above that gets the active template.
//
const TString& TCQCIntfView::strCurTmplName() const
{
    return iwdgTopMost().strTemplateName();
}


// Proivde access to the title string we are given to use in any error popups
const TString& TCQCIntfView::strTitle() const
{
    return m_strTitle;
}


// Provides access to the generated template id that identifies us at runtime
const TString& TCQCIntfView::strTmplId() const
{
    return m_strTmplId;
}



//
//  Derived views call this in response to user input activity, so that
//  bCheckTimeout() can tell the caller if it's time for the topmost
//  popup, if any, should get an inactivity timeout event.
//
tCIDLib::TVoid TCQCIntfView::UpdateLastActivityStamp()
{
    m_enctLastActivity = TTime::enctNow();
}


// ---------------------------------------------------------------------------
//  TCQCIntfView: Hidden Constructors
// ---------------------------------------------------------------------------
TCQCIntfView::TCQCIntfView( const   TString&                    strTitle
                            ,       MCQCIntfAppHandler* const   pmiahToUse
                            , const TCQCUserCtx&                cuctxToUse) :

    MCQCCmdTarIntf
    (
        kCQCIntfEng::strCTarId_IntfViewer
        , facCQCIntfEng().strMsg(kIEngMsgs::midTar_Viewer)
        , kCQCIntfEng::strCTarId_IntfViewer
        , L"/Reference/IntfWidgets/IntfViewer"
    )
    , m_bDoFades(kCIDLib::True)
    , m_bEnableTmplCaching(!TFacCQCIntfEng::bDesMode())
    , m_bmpTBuf
      (
        TSize(512, 256)
        , tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
        , kCIDLib::True
      )
    , m_bmpTMask
      (
        TSize(512, 256)
        , tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
        , kCIDLib::True
      )
    , m_ctarGlobalVars(tCIDLib::EMTStates::Safe, kCIDLib::False)
    , m_cuctxToUse(cuctxToUse)
    , m_enctLastActivity(TTime::enctNow())
    , m_eAutoBlankMode(tCQCIntfEng::ESpecActs::BlankAndBlanker)
    , m_pixaTBlur
      (
        tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
        , tCIDImage::ERowOrders::TopDown
        , TSize(512, 512)
      )
    , m_pmiahToUse(pmiahToUse)
    , m_rgbBorder(facCIDGraphDev().rgbBlack)
    , m_strTitle(strTitle)
    , m_strTmplId(TUniqueId::strMakeId())
{
    // Set the 'special' command target flag on us
    bIsSpecialCmdTar(kCIDLib::True);

    // We also require GUI based command dispatch
    bGUIDispatch(kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TCQCIntfView: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper derived views can call to see if it's time to do issue a timeout
//  event on the passed template (which should be a popup.)
//
tCIDLib::TBoolean
TCQCIntfView::bTOEventTime(const TCQCIntfTemplate& iwdgToCheck) const
{
    // If zero seconds timeout, then it's not enabled, so return false
    const tCIDLib::TCard4 c4Secs = iwdgToCheck.c4TimeoutSecs();
    if (!c4Secs)
        return kCIDLib::False;

    //
    //  If it's not activity based, then check relative to the last timeout.
    //  Else, check relative to the last activity.
    //
    const tCIDLib::TBoolean bActBased = iwdgToCheck.bActivityBasedTO();
    const tCIDLib::TEncodedTime enctBase
    (
         bActBased ? m_enctLastActivity : iwdgToCheck.enctLastTimeout()
    );

    // See if it's been at least that many seconds
    return
    (
        (enctBase + (c4Secs * kCIDLib::enctOneSecond)) <= TTime::enctNow()
    );
}


//
//  If it's been the indicated length of time or more beyond the last activity,
//  then return true and set the last activity time to now. Else return false.
//
tCIDLib::TBoolean
TCQCIntfView::bSinceLastActivity(const tCIDLib::TEncodedTime enctCheck)
{
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if ((enctCur - m_enctLastActivity) > enctCheck)
    {
        m_enctLastActivity = enctCur;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}



// ---------------------------------------------------------------------------
//  TCQCIntfWView: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called by the alpha/fx text drawing methods to make sure that our buffer bmps
//  and pixel array are large enough. They are all the same size, so we only need
//  to check one.
//
//  We actually check against the new sizes plus 32, so that there's always enough
//  to handle any potential blurring.
//
tCIDLib::TVoid
TCQCIntfView::CheckTextBufSz(const  tCIDLib::TCard4 c4Width
                            , const tCIDLib::TCard4 c4Height)
{
    if ((m_pixaTBlur.c4Width() < c4Width + 32)
    ||  (m_pixaTBlur.c4Height() < c4Height + 32))
    {
        // Start with our current size
        TSize szNew(m_pixaTBlur.szImage());

        //
        //  For each axis that has been exceeded, bump it up to the new high water
        //  mark plus some extra. We do each one separately so that one axis doesn't
        //  keep creeping when it's only the other that's getting bigger.
        //
        if (m_pixaTBlur.c4Width() < c4Width + 32)
            szNew.c4Width(c4Width + 128);

        if (m_pixaTBlur.c4Height() < c4Height + 32)
            szNew.c4Height(c4Height + 128);

        m_bmpTBuf = TBitmap
        (
            szNew
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
            , kCIDLib::True
        );

        m_bmpTMask = TBitmap
        (
            szNew
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
            , kCIDLib::True
        );

        m_pixaTBlur.Reset
        (
            tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
            , tCIDImage::ERowOrders::TopDown
            , szNew
        );
    }
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStdView
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStdView: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStdView::TCQCIntfStdView(const  TString&                    strTitle
                                ,       MCQCIntfAppHandler* const   pmiahToUse
                                , const TRGBClr&                    rgbBgn
                                , const tCIDLib::TFloat8            f8Lat
                                , const tCIDLib::TFloat8            f8Long
                                , const tCIDLib::TBoolean           bCenteredView
                                , const TCQCUserCtx&                cuctxToUse) :

    TCQCIntfView(strTitle, pmiahToUse, cuctxToUse)
    , m_bCenteredView(bCenteredView)
    , m_bInEventAction(kCIDLib::False)
    , m_bNoChildMouse(kCIDLib::False)
    , m_bmpDBuf
      (
        TSize(16, 16)
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
      )
    , m_enctBlankTime(0)
    , m_f8Lat(f8Lat)
    , m_f8Long(f8Long)
    , m_pitmStack(new TCQCIntfTmplMgr(cuctxToUse))
    , m_psasubEvTrigs(kCIDLib::False)
    , m_rgbBgn(rgbBgn)
{
    // Subscribe to incoming event triggers from CQCKit
    m_psasubEvTrigs.SubscribeTo(kCQCKit::strPubTopic_EvTriggers, kCIDLib::False);
}

TCQCIntfStdView::TCQCIntfStdView(const  TString&                    strTitle
                                , const TCQCIntfTemplate&           iwdgTemplate
                                ,       MCQCIntfAppHandler* const   pmiahToUse
                                , const TRGBClr&                    rgbBgn
                                , const tCIDLib::TFloat8            f8Lat
                                , const tCIDLib::TFloat8            f8Long
                                , const tCIDLib::TBoolean           bCenteredView
                                , const TCQCUserCtx&                cuctxToUse) :

    TCQCIntfView(strTitle, pmiahToUse, cuctxToUse)
    , m_bCenteredView(bCenteredView)
    , m_bInEventAction(kCIDLib::False)
    , m_bNoChildMouse(kCIDLib::False)
    , m_bmpDBuf(TSize(16, 16), tCIDImage::EPixFmts::TrueClr, tCIDImage::EBitDepths::Eight)
    , m_enctBlankTime(0)
    , m_f8Lat(f8Lat)
    , m_f8Long(f8Long)
    , m_pitmStack(new TCQCIntfTmplMgr(iwdgTemplate, cuctxToUse))
    , m_psasubEvTrigs(kCIDLib::False)
    , m_rgbBgn(rgbBgn)
{

    // Subscribe to incoming event triggers from CQCKit
    m_psasubEvTrigs.SubscribeTo(kCQCKit::strPubTopic_EvTriggers, kCIDLib::False);

    //
    //  In this special case we have to do a call to the app handler's new
    //  template callback, because a new template won't be loaded via the
    //  regular means. It's being passed in to us for initial load.
    //
    if (pmiahToUse)
        pmiahToUse->NewTemplate(iwdgTemplate, *this, 0);
}

TCQCIntfStdView::~TCQCIntfStdView()
{
    // Unsubscribe from event reporting
    m_psasubEvTrigs.UnsubscribeFrom(kCQCKit::strPubTopic_EvTriggers);

    // Clean up the stack if we created it
    try
    {
        delete m_pitmStack;
        m_pitmStack = 0;
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfStdView: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If we request the containing app to do an async command for us, he will
//  call us back here with the command we asked for. He will post it async
//  internally and call us back after the original method that posted it has
//  returned.
//
tCIDLib::TVoid
TCQCIntfStdView::AsyncCmdCallback(const tCQCIntfEng::EAsyncCmds eCmd)
{
    if (eCmd == tCQCIntfEng::EAsyncCmds::RunTOPopup)
    {
        //
        //  If the timeout popup path is not empty, let's invoke it, and
        //  indicate it is a timeout popup, so that it will do the right
        //  thing.
        //
        if (!m_strTOPopup.bIsEmpty())
        {
            TCQCIntfStdView::bCreatePopUp
            (
                m_strTOPopup
                , kCIDLib::False
                , tCIDLib::EDirs::Count
                , CQCIntfEng_View::pntDefPos
                , L"Center"
                , kCIDLib::True
                , m_strTOPopupParms
            );
        }
    }
}


//
//  Handle async data callbacks from the containing app. The application will make
//  sure the object gets cleaned up when we return.
//
tCIDLib::TVoid TCQCIntfStdView::AsyncDataCallback(TCQCIntfADCB* const padcbData)
{
    try
    {
        if (padcbData->m_strCBId == CQCIntfEng_View::strACBID_LoadOverAnc)
        {
            TAncOverLoad* padcbInfo = static_cast<TAncOverLoad*>(padcbData);

            //
            //  If the target path is the root, then we just do a load template.
            //  Else we need to find the target via the root and load into it.
            //
            tCIDLib::TBoolean       bRes = kCIDLib::False;
            tCQCIntfEng::TErrList   colErrs;
            TDataSrvClient          dsclLoad;

            if (padcbInfo->m_strTarPath == L"/")
            {
                // Query the template contents
                TCQCIntfTemplate iwdgToLoad;
                facCQCIntfEng().QueryTemplate
                (
                    padcbInfo->m_strTmplToLoad
                    , dsclLoad
                    , iwdgToLoad
                    , cuctxToUse()
                    , bEnableTmplCaching()
                );

                // This will reload the top-most template in teh stack
                bRes = bLoadNewTemplate(dsclLoad, iwdgToLoad, colErrs);
            }
             else
            {
                // Find the target, if it still exists, and load it
                TCQCIntfWidget* piwdgTar = facCQCIntfEng().piwdgFindByNamePath
                (
                    padcbInfo->m_strTarPath, iwdgTopMost()
                );

                if (piwdgTar && piwdgTar->bIsA(TCQCIntfOverlay::clsThis()))
                {
                    TCQCIntfOverlay* piwdgOver = static_cast<TCQCIntfOverlay*>(piwdgTar);
                    bRes = piwdgOver->bLoadNewTemplate
                    (
                        padcbInfo->m_strTmplToLoad, colErrs
                    );
                }
            }

            if (!bRes)
            {
                // <TBD>
            }
        }
         else if (padcbData->m_strCBId == CQCIntfEng_View::strACBID_ScrollOverlay)
        {
            // Cast it to its actual type
            TScrollOverADCB* padcbInfo = static_cast<TScrollOverADCB*>(padcbData);

            // Find the target, if it still exists
            TCQCIntfWidget* piwdgTar = facCQCIntfEng().piwdgFindByNamePath
            (
                padcbInfo->m_strTarPath, iwdgTopMost()
            );

            // If we didn't end up on an overlay, do nothing
            if (!piwdgTar || !piwdgTar->bIsA(TCQCIntfOverlay::clsThis()))
            {
                facCQCIntfEng().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kIEngErrs::errcLink_NotAnOverlay2
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::TypeMatch
                );
                return;
            }

            // Fake a flick gesture on it
            tCIDLib::EDirs eDir = tCIDLib::EDirs::Count;
            if (padcbInfo->m_strScrollCmd == L"Left")
                eDir = tCIDLib::EDirs::Left;
            else if (padcbInfo->m_strScrollCmd == L"Right")
                eDir = tCIDLib::EDirs::Right;

            if (eDir != tCIDLib::EDirs::Count)
            {
                TPoint pntAt = piwdgTar->areaActual().pntCenter();
                piwdgTar->ProcessFlick(eDir, pntAt);
            }
        }
         else
        {
            // Pass to our parent who may handle some of these
            TParent::AsyncDataCallback(padcbData);
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        ShowErr
        (
            strTitle()
            , facCQCIntfEng().strMsg
              (
                kIEngMsgs::midStatus_AsyncDCBFailed, padcbData->m_strCBId
              )
            , errToCatch
        );
    }

    catch(...)
    {
        ShowMsg
        (
            strTitle()
            , facCQCIntfEng().strMsg(kIEngMsgs::midStatus_AsyncDCBFailed)
            , kCIDLib::True
            , padcbData->m_strCBId
        );
    }
}


//
//  Let's the world know if we have any active actions right now. This includes any user
//  driven action, or if one of the triggered events associated with one of the templates.
//
tCIDLib::TBoolean TCQCIntfStdView::bActiveAction() const
{
    return  m_pitmStack->bActiveAction() || m_bInEventAction;
}


//
//  If we are not busy, and if the top template has a timeout set, then we'll see if
//  the timeout period has expired (since the last activity if it's of that type.) If
//  so, we return true and we post an OnTimeout event.
//
tCIDLib::TBoolean TCQCIntfStdView::bCheckTimeout()
{
    const tCIDLib::TCard4 c4Cnt = m_pitmStack->c4CurCount();
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (!bIsBusy())
    {
        TCQCIntfTemplate& iwdgTop = m_pitmStack->iwdgTopMost();
        bRet = bTOEventTime(iwdgTop);

        if (bRet)
        {
            //
            //  Reset the last activity stamp just in case. If it's not
            //  activity based, it won't matter. If it is, we need to
            //  reset it, because this might not do anything and it
            //  wouldn't get reset otherwise.
            //
            UpdateLastActivityStamp();

            // Post the OnTimeout event
            try
            {
                // Update the widget's last timeout stamp
                iwdgTop.enctLastTimeout(TTime::enctNow());

                PostOps
                (
                    iwdgTop
                    , iwdgTop
                    , kCQCKit::strEvId_OnTimeout
                    , c4Cnt - 1
                    , tCQCKit::EActOrders::NonNested
                );
            }

            catch(TError& errToCatch)
            {
                // Disable the timeout so that they won't get stuck
                iwdgTop.c4TimeoutSecs(0);

                if (facCQCIntfEng().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                // And display the error
                ShowErr
                (
                    strTitle()
                    , facCQCIntfEng().strMsg(kIEngMsgs::midStatus_TOActionFailed)
                    , errToCatch
                );
            }

            catch(...)
            {
                ShowMsg
                (
                    strTitle()
                    , facCQCIntfEng().strMsg(kIEngMsgs::midStatus_TOActionFailed)
                    , kCIDLib::True
                    , TString(L"A system error occurred")
                );
            }
        }
    }

    // And tell them if we did or not, just in case they care
    return bRet;
}


//
//  Creates a popup or popout template. They are basically the same except
//  that popouts are 'slid out' like tray, at a specific point, while popups
//  are just displayed normally and centered.
//
//  The gotcha is that we have to have a second memory bitmap for our
//  double buffering since we have to render the layered ones into their own
//  bitmap, and then alpa blit them over the contents of the main double
//  buffering bitmap, and finally the main bitmap is blitted to the screen.
//
tCIDLib::TBoolean
TCQCIntfStdView::bCreatePopUp(  const   TString&            strTemplate
                                , const tCIDLib::TBoolean   bSlideOut
                                , const tCIDLib::EDirs      eDir
                                , const TPoint&             pntAt
                                , const TString&            strAlignment
                                , const tCIDLib::TBoolean   bTOPopup
                                , const TString&            strPOParms)
{
    // Get the position of the parent template
    TCQCIntfTemplate& iwdgParent = m_pitmStack->iwdgTopMost();
    const TArea& areaParPos = iwdgParent.areaActual();

    // Disable timers while we do this
    TCQCIntfTimerJan janTimers(this);

    TArea                       areaNew;
    tCQCIntfEng::TGraphIntfDev  cptrGDev = cptrNewGraphDev();
    TDataSrvClient              dsclLoad;

    //
    //  We can't copy the template once we've done initilaization, because
    //  it will create pointers to things that aren't easily copyable. So
    //  we get a ref to the next available template in the stack, the one
    //  that we will take if we are successful. We can load directly into
    //  it, but if we fail, then we can just reset it and not have committed
    //  to anything. The handler id will be the current count of templates,
    //  and that will be the index of the next available one.
    //
    tCIDLib::TCard4 c4HandlerId;
    TCQCIntfStackItem& isiPopup = m_pitmStack->isiNext(c4HandlerId);
    TCQCIntfTemplate& iwdgPopup = isiPopup.m_iwdgTemplate;

    //
    //  Take the incoming position. Adjust it for the origin of the parent
    //  template if not -1,-1.
    //
    TPoint pntAdjAt(pntAt);
    if (pntAdjAt != CQCIntfEng_View::pntDefPos)
        pntAdjAt += areaParPos.pntOrg();

    try
    {
        facCQCIntfEng().QueryTemplate
        (
            strTemplate, dsclLoad, iwdgPopup, cuctxToUse(), bEnableTmplCaching()
        );

        // Inherit the parent template's focus setting
        iwdgPopup.bNoFocus(iwdgParent.bNoFocus());

        // Set the popup type
        if (bSlideOut)
            iwdgPopup.eType(tCQCIntfEng::ETmplTypes::Popout);
        else
            iwdgPopup.eType(tCQCIntfEng::ETmplTypes::Popup);

        //
        //  Calc the actual position. We'll either center it in the parent
        //  template, position it for a slideout movement, or position it
        //  relative to the parent template, according to how the command
        //  was invoked.
        //
        areaNew = iwdgPopup.areaActual();
        if (bSlideOut)
        {
            areaNew.SetOrg(pntAdjAt);
            if (eDir == tCIDLib::EDirs::Up)
            {
                areaNew.Move(tCIDLib::EDirs::Up);
                areaNew.Move(tCIDLib::EDirs::Left);
            }
             else if (eDir == tCIDLib::EDirs::Right)
            {
                areaNew.Move(tCIDLib::EDirs::Left);
            }
        }
         else
        {
            //
            //  It's a popup. So they either gave us a position relative
            //  to the parent template origin, or asked us to just center
            //  it in the parent template.
            //
            if (pntAdjAt == CQCIntfEng_View::pntDefPos)
            {
                areaNew.CenterIn(areaParPos);
            }
            else
            {
                // Start off assuming upper left corner alignment
                areaNew.SetOrg(pntAdjAt);

                // And adjust if necessary
                if (strAlignment == L"Center")
                {
                    // Center it on the point
                    areaNew.CenterOn(pntAdjAt);
                }
                 else if (strAlignment == L"Upper Right")
                {
                    // Move the area left by its width
                    areaNew.Move(tCIDLib::EDirs::Left);
                }
                 else if (strAlignment == L"Lower Left")
                {
                    // Move the area up by its height
                    areaNew.Move(tCIDLib::EDirs::Up);
                }
                 else if (strAlignment == L"Lower Right")
                {
                    // Move the area up and left
                    areaNew.Move(tCIDLib::EDirs::Left);
                    areaNew.Move(tCIDLib::EDirs::Up);
                }
            }
        }

        // Store the template name into the template
        iwdgPopup.strTemplateName(strTemplate);

        //
        //  Initialize the template now. This will recursively initialize all the
        //  widgets, allowing them to load images and so forth, find all of the
        //  widget associated with fields and let them update themselves with field
        //  id info, and then it will register all the associated field-using widgets
        //  with our interface engine.
        //
        //  We pass in any invocation parameters, which will get passed on
        //  to the pre-load action engine (which is done using a synchronous
        //  engine, not the asynd one that we normally use.
        //
        TCQCFldCache cfcData;
        cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));
        tCQCIntfEng::TErrList colErrs;
        if (!iwdgPopup.bInitTemplate(this
                                    , facCQCIntfEng().polleThis()
                                    , dsclLoad
                                    , cfcData
                                    , colErrs
                                    , strPOParms))
        {
            return kCIDLib::False;
        }

        // Move it to it's target location now
        iwdgPopup.SetArea(areaNew);

        //
        //  See if we can find an initial widget that will accept the focus.
        //  If we find one, then give it the focus emphasis. If we don't find
        //  one, it will return zero, and we just won't display one.
        //
        //  The base template controls whether focus is used or not, not the
        //  popup.
        //
        if (!m_pitmStack->iwdgBaseTmpl().bNoFocus())
        {
            TCQCIntfWidget* piwdgInitFocus = iwdgPopup.piwdgNextPrevFocus(0, kCIDLib::True);
            if (piwdgInitFocus)
                piwdgInitFocus->bHasFocus(kCIDLib::True);
        }

        // If there's a load WAV file, then start it playing
        if (!iwdgPopup.strLoadWAV().bIsEmpty())
            PlayWAV(iwdgPopup.strLoadWAV(), tCIDLib::EWaitModes::NoWait);
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }


        //
        //  Reset the template in case we got stuff created that will not
        //  now be used.
        //
        try
        {
            iwdgPopup.ResetTemplate(this);
        }

        catch(TError& errToCatch2)
        {
            if (facCQCIntfEng().bShouldLog(errToCatch2))
            {
                errToCatch2.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch2);
            }

            if (facCQCIntfEng().bLogFailures())
            {
                facCQCIntfEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIEngMsgs::midStatus_ExceptInTmplCleanup
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(...)
        {
            if (facCQCIntfEng().bLogFailures())
            {
                facCQCIntfEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIEngMsgs::midStatus_ExceptInTmplCleanup
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        ShowErr
        (
            strTitle()
            , facCQCIntfEng().strMsg(kIEngMsgs::midStatus_CantLoadPopup, strTemplate)
            , errToCatch
        );
        return kCIDLib::False;
    }

    //
    //  Pause slightly to give the polling engine some time to get any
    //  new fields polled.
    //
    TThread::Sleep(250);

    //
    //  It survived so push it onto template manager's stack. This will
    //  not display it yet.
    //
    //  At this point, if anything goes wrong and we get an exception before
    //  we get into the modal loop, we have to get it back off the stack
    //  before we exit.
    //
    //  We pass along the 'timeout popup' flag so that this one will be
    //  marked appropriately.
    //
    const tCIDLib::TCard4 c4NewId = m_pitmStack->c4Push(*this, bTOPopup);
    CIDAssert(c4NewId == c4HandlerId, L"The new popup handler ID is incorrect");

    tCIDLib::TBoolean bFailed = kCIDLib::False;
    try
    {
        //
        //  Now set any invocation parameters on the action context at this
        //  new level. Which action parameter it is depends on the command. This
        //  will get the parms into the OnLoad and reset template and return
        //  values.
        //
        m_pitmStack->iactxAt(c4NewId).Init(strPOParms);

        //
        //  Force an initial value scan so that we can at least get the values
        //  of any fields that are already in the cache and have the widgets
        //  that access those fields valid before we even show up.
        //
        iwdgPopup.InitialScan
        (
            facCQCIntfEng().polleThis(), ctarGlobalVars()
        );

        //
        //  Now let's display it
        //
        //  NOTE:   It would be better if we could delay this until after the
        //          OnLoad command processing below. But it's not possible. The
        //          widgets assume that they can redraw during command processing
        //          and stopping them from doing so would be a lot of work. So
        //          they'd update in their final target position before the actual
        //          template background got there.
        //
        //          This is why the push and show commands were done separately,
        //          to allow for this, but for now we can't.
        //
        m_pitmStack->ShowTop(*this, eDir, 0xFF);

        //
        //  Go back and do a post-init init. This allows them to do any work
        //  that can't be during init such as loading data or anything that
        //  might invoke an action.
        //
        iwdgPopup.DoPostInits();

        //
        //  Get the break flag for our (new) level and clear it. We have to do
        //  this before we do any OnLoad, because it might issue an Exit cmd
        //  and in that case we don't even want to enter the modal loop below.
        //
        tCIDLib::TBoolean& bBreak = m_pitmStack->bBreakFlag(c4NewId);
        bBreak = kCIDLib::False;

        // Do any nested onloads
        iwdgPopup.DoOnLoads(*this);

        // Now let the timers go again while we are in the modal loop
        janTimers.Release();

        //
        //  Now we have to start a new processing loop, because we don't want to
        //  return until this popup completes. If we return, then the calling
        //  action would continue to run and we need to block it until this
        //  popup exits.
        //
        //  Also note that the OnLoad above may have already set the break flag
        //  if the exit command was issued, in which case we don't even bother
        //  entering the loop.
        //
        if (!bBreak)
            DoModalLoop(bBreak, bTOPopup);

        // Do any nested onexits
        iwdgPopup.DoOnExits(*this);
    }

    //
    //  If we have to pop back off due to an error, we'll force the type
    //  to be just a regular popup, so that we don't try to do the slide
    //  back closed popout style. That might not go well if something went
    //  wrong.
    //
    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        bFailed = kCIDLib::True;
    }

    catch(...)
    {
        bFailed = kCIDLib::True;
    }

    // Reaquire the timers lock to stop the timers while we clean up
    janTimers.Reacquire();

    //
    //  Get the result info. This will copy the return values from the
    //  context we are closing into the parent context so that they will
    //  be available, and it'll return us the result value from the one
    //  we are closing, which will be our return value.
    //
    tCIDLib::TBoolean bRet = m_pitmStack->bCopyRetVals(c4NewId);

    //
    //  If there's an unload WAV file, then start it playing async. Only
    //  do it if the template actually loaded successfully.
    //
    if (!iwdgPopup.strUnloadWAV().bIsEmpty() && !bFailed)
    {
        try
        {
            PlayWAV(iwdgPopup.strUnloadWAV(), tCIDLib::EWaitModes::NoWait);
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().bLogInfo() && !errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }

    // Undo the push above, to clean up the pushed template
    m_pitmStack->Pop(*this, eDir, !bFailed);

    // If we failed, show an error message and set the return to false
    if (bFailed)
    {
        ShowMsg
        (
            strTitle()
            , facCQCIntfEng().strMsg(kIEngMsgs::midStatus_CantLoadPopup, strTemplate)
            , kCIDLib::True
        );
        bRet = kCIDLib::False;
    }
    return bRet;
}



// Lets the outside world know if we have any popups displayed
tCIDLib::TBoolean TCQCIntfStdView::bHasPopups() const
{
    return (m_pitmStack->c4CurCount() > 1);
}


tCIDLib::TBoolean
TCQCIntfStdView::bInitialize(TDataSrvClient& dsclInit, tCQCIntfEng::TErrList& colErrs)
{
    CIDAssert
    (
        m_pitmStack->c4CurCount() == 1
        , L"The template stack was not 1 when view was initialized"
    );

    // Call our parent first
    if (!TParent::bInitialize(dsclInit, colErrs))
        return kCIDLib::False;

    // Get the current area of this view
    const TArea& areaUs = areaView();

    //
    //  Do an initial creation of our double buffering bitmap, to fit the view size.
    //  It will be reallocated as required as the view changes size, when the
    //  containing app tells us about size changes. Note it might be zero here so
    //  don't try it if so. If it is, we'll get size change event later.
    //
    if (!areaUs.bIsEmpty() && (m_bmpDBuf.szBitmap() != areaUs.szArea()))
    {
        m_bmpDBuf = TBitmap
        (
            areaUs.szArea(), tCIDImage::EPixFmts::TrueClr, tCIDImage::EBitDepths::Eight
        );
    }

    //
    //  At this point we just have the base template since we are just now being set
    //  up.
    //
    TArea areaInit;
    TCQCIntfStackItem& isiBase = m_pitmStack->isiAt(0);
    TCQCIntfTemplate& iwdgMain = isiBase.m_iwdgTemplate;

    // Call the main template loading method and return his return
    return bLoadNewTemplate(dsclInit, iwdgMain, colErrs);
}


//
//  This is provided to support direct loads of an overlay via the IV's external ctrl
//  interface, and also in the cases of loading an ancestor, which has to be done
//  async via the app handler which calls us back. The caller provides us with the name
//  of a top level overlay, and the path to a template to load into it.
//
tCIDLib::TBoolean
TCQCIntfStdView::bLoadOverlay(const TString&                strOvrName
                            , const TString&                strToLoad
                            ,       tCQCIntfEng::TErrList&  colErrs)
{
    //
    //  Now let's find the overlay, which we assume to be in the current
    //  top-most template. If not, we do nothing. Tell it not to recurse,
    //  since we only support loading an overlay directly under the template
    //  itself.
    //
    TCQCIntfTemplate& iwdgTar = m_pitmStack->iwdgTopMost();
    TCQCIntfWidget* piwdgTar = iwdgTar.piwdgFindByName(strOvrName, kCIDLib::False);

    // We didn't find it
    if (!piwdgTar)
    {
        TString strErr
        (
            kIEngErrs::errcTmpl_WidgetNotFound, facCQCIntfEng(), strOvrName
        );
        colErrs.objPlace
        (
            tCQCIntfEng::EValErrTypes::Template, strErr, kCIDLib::True, TString::strEmpty()
        );
        return kCIDLib::False;
    }

    // Make sure it's an overlay
    if (!piwdgTar->bIsA(TCQCIntfOverlay::clsThis()))
    {
        TString strErr(kIEngErrs::errcOvrly_NotAnOverlay, facCQCIntfEng(), strOvrName);
        colErrs.objPlace
        (
            piwdgTar->c4UniqueId(), strErr, kCIDLib::True, piwdgTar->strWidgetId()
        );
        return kCIDLib::False;
    }

    //
    //  Ok, we found it and it's an overlay, so let's ask it to load this
    //  template.
    //
    TCQCIntfOverlay* piwdgOvr = static_cast<TCQCIntfOverlay*>(piwdgTar);
    return piwdgOvr->bLoadNewTemplate(strToLoad, colErrs);
}


//
//  This can be called to make us replace our current template contents with
//  another template. This can be the base template or a popup. At ths point
//  we expect a base template to be in place, even if just a dummy that we can
//  overwrite.
//
tCIDLib::TBoolean
TCQCIntfStdView::bLoadNewTemplate(          TDataSrvClient&         dsclLoad
                                    , const TCQCIntfTemplate&       iwdgToLoad
                                    ,       tCQCIntfEng::TErrList&  colErrs)
{
    CIDAssert(m_pitmStack->c4CurCount(), L"The template stack can't be empty here");

    // Get the top-most template
    TCQCIntfTemplate& iwdgCur = m_pitmStack->iwdgTopMost();
    const tCIDLib::TCard4 c4HandlerId = iwdgCur.c4HandlerId();

    //
    //  If this is the base template, then we need to clear out the double
    //  buffer, because the new one might be smaller than the old one.
    //
    if (!c4HandlerId)
    {
        TGraphMemDev gdevTmp(gdevCompat(), m_bmpDBuf);
        gdevTmp.Fill(areaDisplayRes(), m_rgbBgn);
    }

    //
    //  When we are done, we'll have to redraw to show the new contents. We need to
    //  redraw the larger of the current or the new one, so that we correctly redraw
    //  any of the affected areas. So save the cur widget area are before we change
    //  anything.
    //
    TArea areaRedraw(iwdgCur.areaActual());

    try
    {
        //
        //  If this is a new base level template, then reset the auto-blanker
        //  timer flag. We don't want the setting from a previous template to
        //  hang around. If this new top level template wants to have auto-
        //  blanking, it will set it during its OnLoad.
        //
        m_enctBlankTime = 0;

        //
        //  Assign the new temp one to the top most stack template. We have
        //  to save the current handler id and put it back since the incoming
        //  one will have a default 0.
        //
        StartBranchReplace(iwdgCur);
        iwdgCur = iwdgToLoad;
        iwdgCur.c4HandlerId(c4HandlerId);

        // !From this point, iwdgToLoad is no longer used!


        //
        //  Initialize the template now. This will recursively initialize
        //  all the widgets, allowing them to load images and so forth,
        //  find all of the widget associated with fields and let them
        //  update themselves with field id info, and then it will
        //  register all the associated field-using widgets with our
        //  interface engine.
        //
        TCQCFldCache cfcData;
        cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));
        if (!iwdgCur.bInitTemplate( this
                                    , facCQCIntfEng().polleThis()
                                    , dsclLoad
                                    , cfcData
                                    , colErrs
                                    , TString::strEmpty()))
        {
            return kCIDLib::False;
        }

        //
        //  Position the template. We are told by our parent class whether he
        //  wants templates centered or upper left justified. We do the scheme
        //  requested.
        //
        if (m_bCenteredView)
        {
            const TArea areaTarget(areaView());
            TArea areaTmpl(iwdgCur.areaActual());
            areaTmpl.CenterIn(areaTarget);
            if (areaTmpl.c4Width() > areaTarget.c4Width())
                areaTmpl.i4X(0);
            if (areaTmpl.c4Height() > areaTarget.c4Height())
                areaTmpl.i4Y(0);
            iwdgCur.SetPositions(areaTmpl.pntOrg(), areaTmpl.pntOrg());
        }
         else
        {
            iwdgCur.SetPositions(TPoint::pntOrigin, TPoint::pntOrigin);
        }

        //
        //  Pause slightly to give the polling engine some time to get any
        //  new fields polled.
        //
        TThread::Sleep(200);

        // Let the containing app know about this new load
        miahOwner().NewTemplate(iwdgCur, *this, m_pitmStack->c4CurCount());

        //
        //  Force an initial value scan so that we can at least get the values
        //  of any fields that are already in the cache and have the widgets
        //  that access those fields valid before we even show up.
        //
        iwdgCur.InitialScan(facCQCIntfEng().polleThis(), ctarGlobalVars());

        //
        //  Go back and do a post-init init. This allows them to do any work
        //  that can't be during init such as loading data or anything that
        //  might invoke an action.
        //
        iwdgCur.DoPostInits();

        //
        //  If this is the base template, get focus initialized
        //
        //  See if we can find an initial widget that will accept the focus. If
        //  we find one, then give it the focus emphasis. If we don't find one,
        //  it will return zero, and we just won't display one.
        //
        if (!c4HandlerId)
        {
            TCQCIntfWidget* piwdgInitFocus = 0;
            if (!iwdgCur.bNoFocus())
            {
                piwdgInitFocus = iwdgCur.piwdgNextPrevFocus(0, kCIDLib::True);

                // If we set an initial focus, tell it it is the focus now
                if (piwdgInitFocus)
                {
                    piwdgInitFocus->bHasFocus(kCIDLib::True);
                    piwdgInitFocus->GotFocus();
                }
            }
        }

        //
        //  Do an initial redraw now. Take the larger of the old and new areas.
        //  So just take the combination of the previously saved old area and
        //  the new area.
        //
        areaRedraw |= iwdgCur.areaActual();
        Redraw(areaRedraw);

        //
        //  And do the end branch replace. We can't do this until after post
        //  init, because some widgets won't load the data they need to
        //  determine if the current mouse position is a 'hit' within them
        //  and this guy does the mouse and focus setup.
        //
        EndBranchReplace(iwdgCur);

        // And do any onloads
        iwdgCur.DoOnLoads(*this);
    }

    catch(TError& errToCatch)
    {
        // Make sure it's at least a valid empty template
        iwdgCur.ResetTemplate(this);
        ForceRedraw();

        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        ShowErr
        (
            strTitle()
            , facCQCIntfEng().strMsg
              (
                kIEngErrs::errcTmpl_LoadFailed, iwdgCur.strTemplateName()
              )
            , errToCatch
        );
        return kCIDLib::False;
    }

    catch(...)
    {
        // Make sure it's at least a valid empty template
        iwdgCur.ResetTemplate(this);
        ForceRedraw();

        ShowMsg
        (
            strTitle()
            , facCQCIntfEng().strMsg
              (
                kIEngErrs::errcTmpl_LoadFailed, iwdgCur.strTemplateName()
              )
            , kCIDLib::True
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Does a double buffered draw, returning a ref to our double buffering
//  bitmap, which the caller can then blit somewhere else. We have one that
//  is used to build up all the content, or all of the content minus the
//  current top popup.
//
//  And we have one that will stop once it gets to a particular widget, either
//  before drawing it or after drawing it.
//
TBitmap&
TCQCIntfStdView::bmpQueryContentUnder(  const   TArea&            areaInvalid
                                        , const tCIDLib::TBoolean bIgnoreTop)
{
    // Create a device context over our buffered bitmap
    TGraphMemDev gdevTmp(gdevCompat(), m_bmpDBuf);

    // And call the other version that draws everything under an area
    DrawUnder(gdevTmp, areaInvalid, bIgnoreTop);

    // It'll get released from the device on exit
    return m_bmpDBuf;
}


TBitmap&
TCQCIntfStdView::bmpQueryContentUnder(  const   TArea&                  areaInvalid
                                        , const TCQCIntfWidget* const   piwdgStop
                                        , const tCIDLib::TBoolean       bBefore)
{
    // Create a device context over our buffered bitmap
    TGraphMemDev gdevTarget(gdevCompat(), m_bmpDBuf);

    //
    //  Set a clip region on it so that Windows will avoid some drawing
    //  that isn't needed.
    //
    TRegionJanitor janClip(&gdevTarget, areaInvalid, tCIDGraphDev::EClipModes::Copy);

    //
    //  Fill the part of the invalid area that intersects the template area with the
    //  configured background color.
    //
    TArea areaTmplBgn = m_pitmStack->iwdgBaseTmpl().areaActual();
    {
        areaTmplBgn &= areaInvalid;
        gdevTarget.Fill(areaTmplBgn, m_rgbBgn);
    }

    //
    //  Remove the template area from the clip region and fill with the border color. Only
    //  do this if not in remote mode.
    //
    if (!facCQCIntfEng().bRemoteMode())
    {
        TRegionJanitor janFill
        (
            &gdevTarget, m_pitmStack->iwdgBaseTmpl().areaActual(), tCIDGraphDev::EClipModes::Diff
        );
        gdevTarget.Fill(areaInvalid, rgbBorder());
    }

    // Now draw the stuf
    TArea areaCur;
    const tCIDLib::TCard4 c4Count = m_pitmStack->c4CurCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfTemplate& iwdgTmpl = m_pitmStack->iwdgTmplAt(c4Index);

        //
        //  Confine the update area to the intersection of the invalid area
        //  and this layer's current area.
        //
        //  NOTE: We cannot just not draw if there is no intersection, since
        //  we have to stop when we hit the stop widget. So we have to do
        //  the draw and let it tell us if it found the stop widget or not.
        //  If will do intersection checking itself, so it's only slightly
        //  less efficient.
        //
        //  If this guy hits the stop widget, then we are done so break out.
        //
        areaCur = iwdgTmpl.areaActual();
        areaCur &= areaInvalid;
        if (iwdgTmpl.bDrawUnder(gdevTarget, areaCur, piwdgStop, bBefore))
            break;
    }

    // It'll get released from the device on exit
    return m_bmpDBuf;
}


// Return the current stack count
tCIDLib::TCard4 TCQCIntfStdView::c4StackSize() const
{
    if (m_pitmStack)
        return m_pitmStack->c4CurCount();
    return 0;
}


// We need to stop the
tCIDLib::TVoid TCQCIntfStdView::Destroyed()
{
    // Clean up the template manager
    m_pitmStack->Cleanup();
}



//
//  This must be called regularly by the client code to give us a chance to update
//  any widgets that do active updates, like animation widgets, clocks, etc...
//
//  Note that if there's a widget currently processing a gesture, it will not be
//  called, even if it wants active updates. This prevents it from being interfered
//  with. And we don't have to do anything special here to deal with that.
//
tCIDLib::TVoid TCQCIntfStdView::DoActiveUpdatePass()
{
    // If we are in an active action then don't do anything
    if (bActiveAction())
        return;

    //
    //  Do an active update pass, where we give any widgets who want to a chance to
    //  do active updates.
    //
    const tCIDLib::TCard4 c4Count = m_pitmStack->c4CurCount();
    try
    {
        TGUIRegion grgnClip;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCIntfWidget& iwdgTmpl = m_pitmStack->iwdgTmplAt(c4Index);
            iwdgTmpl.QueryClipRegion(grgnClip);
            iwdgTmpl.ActiveUpdate(kCIDLib::False, grgnClip, (c4Index + 1) == c4Count);
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facCQCIntfEng().bLogFailures())
        {
            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngMsgs::midStatus_ActivePassErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
            );
        }
    }

    catch(...)
    {
        if (facCQCIntfEng().bLogFailures())
        {
            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngMsgs::midStatus_ActivePassErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
            );
        }
    }
}


//
//  Draws all of the children that fall at least partially under a given area. It's
//  typically used to refresh an area of the screen. We clip to the invalid area so
//  that we only update the part that matters. We can optimally ignore the top-most
//  one, since it's often used to gather the content under a widget, to provide a
//  background for compositing during scrolls and such. Since we take a target, we
//  can draw into a buffer or to the screen. However, we do draw at the actual
//  widget positions, so we can't draw into a zero origined bitmap. This is typically
//  called with our large double buffer bitmap, which is the size of the window.
//
tCIDLib::TVoid
TCQCIntfStdView::DrawUnder(         TGraphDrawDev&      gdevTarget
                            , const TArea&              areaInvalid
                            , const tCIDLib::TBoolean   bIgnoreTop)
{
    TCQCIntfTemplate& iwdgBase = m_pitmStack->iwdgBaseTmpl();

    //
    //  Set a clip region on it so that Windows will avoid some drawing
    //  that isn't needed.
    //
    TRegionJanitor janClip(&gdevTarget, areaInvalid, tCIDGraphDev::EClipModes::Copy);

    //
    //  Fill the part of the invalid area and the template area with the
    //  configured background color.
    //
    TArea areaTmplBgn = iwdgBase.areaActual();
    {
        areaTmplBgn &= areaInvalid;
        gdevTarget.Fill(areaTmplBgn, m_rgbBgn);
    }

    //
    //  Remove the template area from the clip region and fill with the border color. Only
    //  do this if not in remote mode.
    //
    if (!facCQCIntfEng().bRemoteMode())
    {
        TRegionJanitor janFill
        (
            &gdevTarget, m_pitmStack->iwdgBaseTmpl().areaActual(), tCIDGraphDev::EClipModes::Diff
        );
        gdevTarget.Fill(areaInvalid, rgbBorder());
    }

    // Do the base which always has to be done
    m_pitmStack->iwdgBaseTmpl().bDrawUnder(gdevTarget, areaInvalid);

    // And do any popups
    tCIDLib::TCard4 c4Count = m_pitmStack->c4CurCount();
    if (c4Count > 1)
    {
        // If ignoring the top-most layer, then drop the count down
        if (bIgnoreTop)
            c4Count--;

        TArea areaPO;
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            TCQCIntfTemplate& iwdgTmpl = m_pitmStack->iwdgTmplAt(c4Index);
            areaPO = iwdgTmpl.areaActual();

            // If this one doesn't intersect the invalid area, skip it
            if (!areaPO.bIntersects(areaInvalid))
                continue;

            // Confine the update area to the intersection
            areaPO &= areaInvalid;

            // And draw this guy into the target bitmap if it intersected
            if (!areaPO.bIsEmpty())
                iwdgTmpl.bDrawUnder(gdevTarget, areaPO);
        }
    }
}


//
//  This is called periodically by the containing application to process any
//  pending events. Note that this is very different from normal actions. It
//  isn't started by the user, so it could happen any time. So we are very
//  picky about when we allow it to happen.
//
//  We have a flag that we use to prevent starting a new event action while
//  another is happening. That flag is also part of the bActiveAction()
//  method above, which the client code will check before allowing any new
//  action to start.
//
//  So we lock ourself out and any user driven action as well. And we also
//  refuse to do one of these if a regular action is active of if we are
//  tracking the mouse.
//
//  The client will give some sort of visual feedback to the user if the
//  active action method returns true, and refuse to start another.
//
tCIDLib::TVoid TCQCIntfStdView::DoEventProcessingPass()
{
    //  We only do this if not busy
    if (bIsBusy())
        return;

    //
    //  Nothing going on currently, so let's put a janitor on the flag to
    //  prevent reentrance.
    //
    TBoolJanitor janEvAction(&m_bInEventAction, kCIDLib::True);

    //
    //  See if any events have come in. If so, then process one of them
    //  and see if it needs to be run. We just check, with a zero time
    //  out.
    //
    //  Note that, in this case, we don't need to post an event back to
    //  ourself to invoke an event. We are not being invoked by a widget
    //  that might get destroyed.
    //
    if (m_psasubEvTrigs.bGetNextMsg(m_psmsgTmp, 0))
    {
        //
        //  We got one, so let's see if it needs to be invoked. Put it in a counted
        //  pointer, which is how it's passed on to the action. Unfortunately that
        //  means we have to duplicate it because it's already in a ref counted
        //  object. This event data goes down through a bunch of levels that we just
        //  don't want to tie to the pub/sub system, and we can't allow two different
        //  ref counting objects to point to one thing.
        //
        tCQCKit::TCQCEvPtr cptrEvent(new TCQCEvent(m_psmsgTmp.objMsgAs<TCQCEvent>()));

        //
        //  We need to create a command target for the event so that
        //  the action can access it. WE set the event on it so that
        //  will provide access to our event. We'll pass this in as
        //  an extra target below.
        //
        TTrigEvCmdTarget ctarEvent;
        ctarEvent.SetEvent(cptrEvent);

        // Get the current time info
        tCIDLib::TCard4  c4Hour;
        tCIDLib::TCard4  c4Min;
        tCIDLib::TCard4  c4Sec;
        m_tmNow.SetToNow();
        m_tmNow.c4AsTimeInfo(c4Hour, c4Min, c4Sec);

        //
        //  Figure out if it's nighttime, which is something we have
        //  to provide to the filter evaluation.
        //
        tCIDLib::TBoolean bIsNight;
        {
            m_tmNoon.SetTo(tCIDLib::ESpecialTimes::NoonToday);
            if (m_tmNow < m_tmNoon)
            {
                //
                //  It is before noon today, so we are in 0 to 12 part
                //  of the day. So get the sunrise time. If the current
                //  time is before that time, then it's night.
                //
                m_tmTmp = m_tmNoon;
                m_tmTmp.SetToSunrise(m_f8Lat, m_f8Long);
                bIsNight = (m_tmNow < m_tmTmp);
            }
             else
            {
                //
                //  It is after noon, so we are in the 12 to 23 part of
                //  the day. So get the sunset time. If the current time
                //  is after that time, then it's night.
                //
                m_tmTmp = m_tmNoon;
                m_tmTmp.SetToSunset(m_f8Lat, m_f8Long);
                bIsNight = (m_tmNow > m_tmTmp);
            }
        }

        //
        //  We have to do a double loop here. We have to go through all of the stacked up
        //  templates, and for each one we have to process any events he has defined. We
        //  call a recursive method at each stack layer, which will go through any nested
        //  containers and process triggers as it goes.
        //
        tCIDLib::TBoolean bRanSome = kCIDLib::False;
        const tCIDLib::TCard4 c4TmplCnt = m_pitmStack->c4CurCount();
        for (tCIDLib::TCard4 c4TmplInd = 0; c4TmplInd < c4TmplCnt; c4TmplInd++)
        {
            TCQCIntfTemplate& iwdgCur = m_pitmStack->iwdgTmplAt(c4TmplInd);
            if (iwdgCur.bInvokeTrgEvents(bIsNight, cptrEvent, c4Hour, c4Min, ctarEvent))
                bRanSome = kCIDLib::True;
        }

        //
        //  If any ran, then we may have changed variables, so force a
        //  variable update pass.
        //
        if (bRanSome)
            m_c4VarUpdate++;
    }
}


//
//  This is called periodically by the client app to give those widget that
//  are associated with fields or variables to update themselves to match
//  the value of the field or variable.
//
tCIDLib::TVoid TCQCIntfStdView::DoUpdatePass()
{
    //  We only do this if not in an active action
    if (bActiveAction())
        return;

    //
    //  See if we need to do variable based ones as well and then reset
    //  the variable update counter for the next round.
    //
    const tCIDLib::TBoolean bDoVars(m_c4VarUpdate != 0);
    m_c4VarUpdate = 0;

    //
    //  We tell them where the pointer is in case that makes a difference
    //  to them (some support flyover emphasis.)
    //
    TGUIRegion grgnClip;
    const tCIDLib::TCard4 c4Count = m_pitmStack->c4CurCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget& iwdgTmpl = m_pitmStack->iwdgTmplAt(c4Index);

        try
        {
            iwdgTmpl.QueryClipRegion(grgnClip);
            iwdgTmpl.ValueUpdate
            (
                facCQCIntfEng().polleThis()
                , kCIDLib::False
                , bDoVars
                , ctarGlobalVars()
                , grgnClip
            );
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().bLogWarnings())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCIntfEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIEngMsgs::midStatus_UpdatePassErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                );
            }
        }
    }

    //
    //  Do blanking if needed and not currently busy. Note that these get
    //  passed along to the containing app. So if we are in the remote
    //  viewer mode, it will go to the remote viewer server's handler, and
    //  he will pass it on to the client.
    //
    if (m_enctBlankTime && !bIsBusy())
    {
        if (bSinceLastActivity(m_enctBlankTime))
        {
            try
            {
                miahOwner().DoSpecialAction(eAutoBlankMode(), kCIDLib::False, *this);

                //
                //  If that worked, and we have a timeout popup defined
                //  and we don't already have one up, then we want to
                //  invoke that. But we can't do it here, since it is a
                //  blocking loop. So we use the app handler to ask the
                //  containing app to call us back asynchronously.
                //
                if (!m_strTOPopup.bIsEmpty() && !m_pitmStack->bTOPopupDisplayed())
                    miahOwner().AsyncCallback(tCQCIntfEng::EAsyncCmds::RunTOPopup, *this);
            }

            catch(TError& errToCatch)
            {
                if (facCQCIntfEng().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }
        }
    }
}


//
//
//  We get called here to handle commands that we reported we handle, actually
//  our base class above reports the supported commands, but the derived
//  classes need to actually handle them.
//
//  NOTE:   Some of our commands are remapped. We are the target for some
//          commands because we don't know their actual target until viewing
//          time (related to whether a template loads as a template or into
//          an overlay.) So we don't handle them here because we'll never
//          actually see them at viewing time.
//
tCQCKit::ECmdRes
TCQCIntfStdView::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
                        , const tCIDLib::TCard4     c4Index
                        , const TString&            strUserId
                        , const TString&            strEventId
                        ,       TStdVarsTar&        ctarGlobals
                        ,       tCIDLib::TBoolean&  bResult
                        ,       TCQCActEngine&      acteTar)
{
    tCQCKit::ECmdRes eRet = tCQCKit::ECmdRes::Ok;

    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_CheckPowerTrans)
    {
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_DismissBlanker)
    {
        //
        //  This is done via the application handler that the host app must install
        //  and handle these special operations. Our parent class owns the handler
        //  and provides us with a method to call to pass this on to the host app.
        //
        miahOwner().DismissBlanker(*this);
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_DoFades)
    {
        // Just set the fades flag based on the boolean value
        bDoFades(facCQCKit().bCheckBoolVal(ccfgToDo.piAt(0).m_strValue));
    }
     else if ((ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_DoLink)
          ||  (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_LoadDefTmpl))
    {
        //
        //  If loading the default template, then look that up. If there
        //  isn't any for the current user, then return false result.
        //
        //  In our case here, we ignore the 'target' parameter of the DoLink
        //  command. That's only used by overlays.
        //
        TString strTmpl;
        if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_DoLink)
            strTmpl = ccfgToDo.piAt(0).m_strValue;
        else
            strTmpl = cuctxToUse().strDefTemplate();

        if (strTmpl.bIsEmpty())
        {
            bResult = kCIDLib::False;
            return eRet;
        }

        //
        //  In this case, it has to be a fully qualified path. Only if it gets
        //  redirected to a template or overlay can it be relative.
        //
        TDataSrvClient dsclLoad;
        TCQCIntfTemplate iwdgTmpl;
        facCQCIntfEng().QueryTemplate
        (
            strTmpl, dsclLoad, iwdgTmpl, cuctxToUse(), bEnableTmplCaching()
        );

        //
        //  In this case, we eat any errors, but we return his return as
        //  the status of the operation.
        //
        tCQCIntfEng::TErrList colErrs;
        bResult = bLoadNewTemplate(dsclLoad, iwdgTmpl, colErrs);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Exit)
    {
        if (acteTar.pcmdtDebug())
        {
            TString strMsg(L"Exiting with status: ");
            strMsg.Append(ccfgToDo.piAt(0).m_strValue);
            acteTar.pcmdtDebug()->ActDebug(strMsg);
        }

        //
        //  Store the passed exit status on the action context object for our
        //  invocation level. The current one has to be us in order for us
        //  to be running commands right now. If this is a popup, then the
        //  modal popup runner (bCreatePopup), will pull the value out and
        //  give it back as the invoke popup/popupout command result.
        //
        m_pitmStack->iactxCur().bPopupRes
        (
            facCQCKit().bCheckBoolVal(ccfgToDo.piAt(0).m_strValue)
        );

        //
        //  Just return an exit status. We'll deal with it once
        //  we unwind back out of the action.
        //
        eRet = tCQCKit::ECmdRes::Exit;
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_GetOverlayParam)
    {
        //
        //  Only valid when called from a template that was loaded into an overlay. ANd
        //  it will be remapped to go to that overlay in that case. If we see it, then
        //  it's configured on a template that is not loaded into an overlay and just
        //  ignore it.
        //
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_GetRetVal)
    {
        // See if we have a value by that name
        tCIDLib::TBoolean bMustExist(facCQCKit().bCheckBoolVal(ccfgToDo.piAt(2).m_strValue));

        bResult = kCIDLib::True;
        TString strValue;
        if (bMustExist)
            strValue = m_pitmStack->iactxCur().strRetValue(ccfgToDo.piAt(0).m_strValue);
        else
            bResult = m_pitmStack->iactxCur().bQueryRetValue(ccfgToDo.piAt(0).m_strValue, strValue);

        if (bResult)
        {
            TCQCActVar& varTar = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(1).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars()
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );
            if (varTar.bSetValue(strValue) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_GetTmplVal)
    {
        // See if we have a value by that name
        tCIDLib::TBoolean bMustExist(facCQCKit().bCheckBoolVal(ccfgToDo.piAt(2).m_strValue));

        TString strValue;
        bResult = kCIDLib::True;
        if (bMustExist)
            strValue = m_pitmStack->iactxCur().strTmplValue(ccfgToDo.piAt(0).m_strValue);
        else
            bResult = m_pitmStack->iactxCur().bQueryTmplValue(ccfgToDo.piAt(0).m_strValue, strValue);

        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars()
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varTar.bSetValue(strValue) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_InRIVAMode)
    {
        bResult = TFacCQCIntfEng::bWebRIVAMode();
    }
     else if ((ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_InvokePopup)
          ||  (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_InvokePopout))
    {
        //
        //  Call the helper that does all the work to create a new popup. We
        //  have to get some info out of the command first.
        //
        tCIDLib::TBoolean bSlideOut
        (
            ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_InvokePopout
        );
        tCIDLib::EDirs eDir = tCIDLib::EDirs::Count;
        if (bSlideOut)
        {
            // Set the direction enum
            TString strDir = ccfgToDo.piAt(1).m_strValue;
            if (strDir == L"From Top")
                eDir = tCIDLib::EDirs::Down;
            else if (strDir == L"From Bottom")
                eDir = tCIDLib::EDirs::Up;
            else if (strDir == L"From Left")
                eDir = tCIDLib::EDirs::Left;
            else if (strDir == L"From Right")
                eDir = tCIDLib::EDirs::Right;
            else
                eDir = tCIDLib::EDirs::Left;
        }

        //
        //  There's an origin point in either case. It used to use a space, so we
        //  have to check for both formats.
        //
        TPoint pntOrigin;
        if (!pntOrigin.bParseFromText(  ccfgToDo.piAt(2).m_strValue
                                        , tCIDLib::ERadices::Auto
                                        , kCIDLib::chComma))
        {
            if (!pntOrigin.bParseFromText(  ccfgToDo.piAt(2).m_strValue
                                            , tCIDLib::ERadices::Auto
                                            , kCIDLib::chSpace))
            {
                //
                //  It's bad, so default it. The alignment will be ignored in
                //  that case.
                //
                pntOrigin = CQCIntfEng_View::pntDefPos;
            }
        }

        //
        //  Get the current top of stack and use that as the base to expand the
        //  path.
        //
        TString strToLoad;
        m_pitmStack->iwdgTopMost().ExpandTmplPath(ccfgToDo.piAt(0).m_strValue, strToLoad);

        // And now invoke it and get the result back
        bResult = bCreatePopUp
        (
            strToLoad
            , bSlideOut
            , eDir
            , pntOrigin
            , ccfgToDo.piAt(3).m_strValue
            , kCIDLib::False
            , ccfgToDo.piAt(bSlideOut ? 3 : 1).m_strValue
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_LoadAncestor)
    {
        //
        //  In an overlay this will get remapped to the overlay. If this happens
        //  to be configured on a template run as the main template or root of a
        //  popup, then we just ignore it.
        //
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_NoChildMouse)
    {
        m_bNoChildMouse = facCQCKit().bCheckBoolVal(ccfgToDo.piAt(0).m_strValue);
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_RetValToTmplVal)
    {
        // Get the return value referenced, has to exist or an error
        const TString& strValue = m_pitmStack->iactxCur().strRetValue
        (
            ccfgToDo.piAt(0).m_strValue
        );

        // And set it as a template value
        m_pitmStack->iactxCur().SetTmplValue(ccfgToDo.piAt(1).m_strValue, strValue);
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_RIVACmd)
    {
        //
        //  Don't need to do anything. It's only used in the RIVA server,
        //  and it will be intercepted by the RIVA's special view class and
        //  just passed on to the client.
        //
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_ScrollParent)
    {
        //
        //  In an overlay this will get remapped to the overlay. If this happens
        //  to be configured on a template run as the main template or root of a
        //  popup, then we just ignore it.
        //
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetAutoBlankMode)
    {
        // Get the value and check it. We set the mode on our parent
        const TString& strMode = ccfgToDo.piAt(0).m_strValue;
        if (strMode == L"Blanker")
            eAutoBlankMode(tCQCIntfEng::ESpecActs::Blanker);
        else if (strMode == L"Blank/Blanker")
            eAutoBlankMode(tCQCIntfEng::ESpecActs::BlankAndBlanker);
        else if (strMode == L"ClockBlanker")
            eAutoBlankMode(tCQCIntfEng::ESpecActs::ClockBlanker);
        else if (strMode == L"DisplayOff")
            eAutoBlankMode(tCQCIntfEng::ESpecActs::DisplayOff);
        else if (strMode == L"ShowBlanker")
            eAutoBlankMode(tCQCIntfEng::ESpecActs::ShowBlanker);
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetBlankOpts)
    {
        // Store the new timeout value
        m_enctBlankTime = ccfgToDo.piAt(0).m_strValue.c4Val()
                          * kCIDLib::enctOneMilliSec;

        //
        //  Get the current top of stack and use that as the base to expand the
        //  path.
        //
        m_pitmStack->iwdgTopMost().ExpandTmplPath
        (
            ccfgToDo.piAt(1).m_strValue, m_strTOPopup
        );

        // And store the popup template and parms as well
        m_strTOPopupParms = ccfgToDo.piAt(2).m_strValue;
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetImage)
    {
        // We need a data server client for this
        TDataSrvClient dsclLoad;

        // This always addresses the base template!
        TCQCIntfTemplate& iwdgTop = m_pitmStack->iwdgBaseTmpl();
        iwdgTop.SetNewImage
        (
            *this, dsclLoad, ccfgToDo.piAt(0).m_strValue, kCIDLib::False
        );

        if (iwdgTop.bCanRedraw(strEventId))
            Redraw(iwdgTop.areaActual());
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetParentState)
    {
        // If not redirected, ignore this at this level
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetRetVal)
    {
        m_pitmStack->iactxCur().SetRetValue
        (
            ccfgToDo.piAt(0).m_strValue, ccfgToDo.piAt(1).m_strValue
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetTmplVal)
    {
        m_pitmStack->iactxCur().SetTmplValue
        (
            ccfgToDo.piAt(0).m_strValue, ccfgToDo.piAt(1).m_strValue
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetViewBorderClr)
    {
        TRGBClr rgbNew;
        if (!rgbNew.bParseFromText(ccfgToDo.piAt(0).m_strValue, tCIDLib::ERadices::Auto, L' '))
            ThrowBadParmVal(ccfgToDo, 0);
        rgbBorder(rgbNew);
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_TmplValToRetVal)
    {
        // Get the template value referenced, has to exist or an error
        const TString& strValue = m_pitmStack->iactxCur().strTmplValue
        (
            ccfgToDo.piAt(0).m_strValue
        );

        // And set it as a return value
        m_pitmStack->iactxCur().SetRetValue(ccfgToDo.piAt(1).m_strValue, strValue);
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_WaitPowerTrans)
    {

    }
     else
    {
        // No more parent's to call, so it's unknown
        ThrowUnknownCmd(ccfgToDo);
    }
    return eRet;
}


//
//  Overlays call this after they are (re)loaded, so we can deal with init
//  of focus and possibly other things later.
//
tCIDLib::TVoid TCQCIntfStdView::EndBranchReplace(TCQCIntfContainer& iwdgCont)
{
    //
    //  If we now have no focus item and focus support isn't suppressed in
    //  this interface, then it may have been inside the overlay that was
    //  replaced, so just do the default focus selection to find a new one.
    //
    const TCQCIntfTemplate& iwdgBase = m_pitmStack->iwdgBaseTmpl();
    TCQCIntfTemplate& iwdgCur = m_pitmStack->iwdgTopMost();
    if (!iwdgBase.bNoFocus() && !iwdgCur.piwdgFindCurFocus())
    {
        TCQCIntfWidget* piwdgInitFocus = iwdgCur.piwdgNextPrevFocus(0, kCIDLib::True);
        if (piwdgInitFocus)
        {
            piwdgInitFocus->bHasFocus(kCIDLib::True);
            Redraw(piwdgInitFocus->areaActual());
        }
    }
}


//
//  These return the topmost root container, which will be either the main
//  template of this window, or the topmost popup.
//
const TCQCIntfTemplate& TCQCIntfStdView::iwdgTopMost() const
{
    return m_pitmStack->iwdgTopMost();
}

TCQCIntfTemplate& TCQCIntfStdView::iwdgTopMost()
{
    return m_pitmStack->iwdgTopMost();
}


// Provide access to the base template of this window
const TCQCIntfTemplate& TCQCIntfStdView::iwdgBaseTmpl() const
{
    return m_pitmStack->iwdgBaseTmpl();
}

TCQCIntfTemplate& TCQCIntfStdView::iwdgBaseTmpl()
{
    return m_pitmStack->iwdgBaseTmpl();
}


//
//  Find a widget via it's command target id, and return it as the command
//  target interface. It does a recursive search. It throws if the target
//  is not found. We search the top-most template by default but they can tell
//  us to search the base instead if that's desired.
//
MCQCCmdTarIntf*
TCQCIntfStdView::pmctarFind(const   TString&            strTarId
                            , const tCIDLib::TBoolean   bBaseOnly)
{
    //
    //  Do a search on the template for a widget with a target id that
    //  matches the passed target id. Second parm tells it to recurse.
    //
    if (bBaseOnly)
        return m_pitmStack->iwdgBaseTmpl().piwdgFindByName(strTarId, kCIDLib::True);
    return m_pitmStack->iwdgTopMost().piwdgFindByName(strTarId, kCIDLib::True);
}


//
//  When a widget needs to invoke it's configured opcodes, it calls here. We
//  collect up the ones that are for the event to invoke and then post them
//  to our action processing engine. We get the action context object for
//  the top-most template in the template manager (the base template or the
//  top-most popup) and set it up with all the info.
//
//  We have another one for invoking event driver actions, which has to
//  provide it's own action context. Because of the way they work, the first
//  one can just call the second one, providing the canned action context
//  for the target template.
//
//  The reason for this is that the template's context is often blocked inside
//  an action that is invoked a popup. So we cannot dispatch the event
//  driven actions on it. They would just get queued and not processed until
//  the popup exited. The event driven ones use a fresh context each time
//  so that it will always be a new action, that will get processed
//  synchronously, then the context will be tossed after we return.
//
//  If there is no currently active action, then we start up the action
//  engine and wait for it to complete. If there is already an action that
//  is going on, then we just queue up the new stuff to be processed in
//  the same engine run. The caller tells us what order the new action is,
//  which tells us whether to add them to the head of the queue (to be
//  done after any current ones complete) or to be done now before going
//  back to whatever is currently going on.
//
tCIDLib::TVoid
TCQCIntfStdView::PostOps(       TCQCIntfWidget&         iwdgSender
                        ,       MCQCCmdSrcIntf&         mcsrcAction
                        , const TString&                strEventId
                        , const tCIDLib::TCard4         c4HandlerId
                        , const tCQCKit::EActOrders     eOrder)
{
    // Just in case, make sure the handler is still available
    if (!m_pitmStack->bIsValidHandlerId(c4HandlerId))
    {
        // For some reason, the template is already gone.
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcDbg_TmplHandlerIdBad
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4HandlerId)
        );
    }

    // Call the other version, and provide the context for the passed handler id
    TCQCIntfActCtx& iactxTar = m_pitmStack->iactxAt(c4HandlerId);
    PostOps
    (
        iwdgSender, mcsrcAction, strEventId, c4HandlerId, iactxTar, 0, eOrder
    );
}

tCIDLib::TVoid
TCQCIntfStdView::PostOps(       TCQCIntfWidget&         iwdgSender
                        ,       MCQCCmdSrcIntf&         mcsrcAction
                        , const TString&                strEventId
                        , const tCIDLib::TCard4         c4HandlerId
                        ,       TCQCIntfActCtx&         iactxTar
                        ,       MCQCCmdTarIntf* const   pctarExtra
                        , const tCQCKit::EActOrders     eOrder)
{
    // Just in case, make sure the handler is still available
    if (!m_pitmStack->bIsValidHandlerId(c4HandlerId))
    {
        // For some reason, the template is already gone.
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcDbg_TmplHandlerIdBad
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4HandlerId)
        );
    }

    //
    //  We also have to deal with the fact that a previous action that
    //  was pushed as part of action processing didn't already kill the
    //  widget. They are supposed to check this themselves if they invoke
    //  more than one event in a row (such as OnRelease then OnPress.) But,
    //  just in case, see if the widget is dead and do nothing if so.
    //
    if (iwdgSender.bIsDead())
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcDbg_ActSrcIsDead
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    //
    //  Stop timers for a bit here. We need to make sure, from the point where
    //  we start the process until the point where we get ourselves marked as
    //  in an action, that no one else can start one, and that could only happen
    //  via user input or one of the incoming timer callbacks from the derived
    //  view. The user action can't happen because we are off doing something
    //  in the GUI thread context right now. We'd only see it after we get
    //  back to processing messages, by which time we'll have marked ourselves
    //  as working and the click or key will be ignored.
    //
    TCQCIntfTimerJan janTimers(this);

    // Get the template for the indicated handler id
    TCQCIntfTemplate& iwdgTar = m_pitmStack->iwdgTmplAt(c4HandlerId);

    // See if we have any commands for the passed event. If not, we are done
    if (!mcsrcAction.c4OpCount(strEventId))
        return;

    //
    //  Lock the tracer so it can't go away. Since we have a pointer to an
    //  action trace option (possibly, it could be null), we have to insure
    //  that it stays alive until the action completes.
    //
    TActTraceJan janTrace(pmcmdtTrace());

    tCQCKit::EActPostRes ePostRes = tCQCKit::EActPostRes::NoAction;
    try
    {
        //
        //  We need to get a list of all our widget based targets. He doesn't
        //  keep these, he just resolves all of the command target ids for
        //  use in the running of the action, storing away their unique
        //  runtime assigned numeric ids for later lookup.
        //
        //  If the passed widget is a container, we take it as the parent
        //  container and make him and all his children available. If it
        //  isn't we get its parent and use that.
        //
        //  We have to sort of kludge a special case here. If it's an overlay,
        //  and the event is OnScroll/OnFlick, or it's a Live Tile and OnClick,
        //  then it's really an action configured the overlay/Tile itself, so
        //  we don't treat it as the parent in that case.
        //
        tCQCKit::TCmdTarList colWdgTars(tCIDLib::EAdoptOpts::NoAdopt, 256);
        TCQCIntfContainer* piwdgCont = nullptr;
        if ((iwdgSender.bIsA(TCQCIntfOverlay::clsThis())
        &&   ((strEventId == kCQCKit::strEvId_OnFlick)
        ||    (strEventId == kCQCKit::strEvId_OnScroll)))

        ||  (iwdgSender.bIsA(TCQCIntfLiveTile::clsThis())
        &&   (strEventId == kCQCKit::strEvId_OnClick)))
        {
            // Make the container the overlay's parent
            piwdgCont = iwdgSender.piwdgParent();
        }
         else
        {
            // Else see if it's a container itself
            piwdgCont = dynamic_cast<TCQCIntfContainer*>(&iwdgSender);
            if (!piwdgCont)
                piwdgCont = iwdgSender.piwdgParent();
        }

        CIDAssert(piwdgCont != 0, L"No container found for action sender widget");
        piwdgCont->FindAllTargets(colWdgTars, kCIDLib::False);

        //
        //  Add the container itself, since the above only finds children and
        //  descendants of the container.
        //
        colWdgTars.Add(piwdgCont);

        //
        //  Add the parent container also, since there are a couple cmds
        //  that can target the parent.
        //
        TCQCIntfContainer* piwdgPar = piwdgCont->piwdgParent();
        if (piwdgPar)
            colWdgTars.Add(piwdgPar);

        // We have to provide one extra standard target, which is the IV target
        tCQCKit::TCmdTarList colExtraStdTars(tCIDLib::EAdoptOpts::NoAdopt, 1);
        colExtraStdTars.Add(this);

        // And they can provide one extra standard target besides
        if (pctarExtra)
            colExtraStdTars.Add(pctarExtra);

        //
        //  Calculate the area of the sending widget. This needs to be relative to
        //  the parent template.
        //
        TArea areaWdgSrc = iwdgSender.areaActual();
        if (piwdgPar)
            areaWdgSrc -= iwdgTar.areaActual().pntOrg();

        // Now we can post the command opcodes
        ePostRes = iactxTar.ePostOps
        (
            this
            , &ctarGlobalVars()
            , colExtraStdTars
            , pmcmdtTrace()
            , mcsrcAction
            , colWdgTars
            , strEventId
            , TString::strEmpty()
            , c4HandlerId
            , kCIDLib::False
            , eOrder
            , areaWdgSrc
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        if (facCQCIntfEng().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        if (facCQCIntfEng().bLogFailures())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_CouldNotPost
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }
        throw;
    }

    catch(...)
    {
        if (facCQCIntfEng().bLogFailures())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_CouldNotPost
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }
        throw;
    }

    //
    //  !!!!!!!!
    //
    //  At this point, we've gotten all the info we need from the action
    //  source object and that info has been posted in the action object.
    //  The action may destroy that src object, so we cannot reference it
    //  anymore after this!
    //
    //  !!!!!!!
    //

    //
    //  If this is a fresh batch, then process them. Else they just
    //  got queued up and will be processed, since the engine is already
    //  processing commands. We'll just fall through so that we can
    //  continue to process commands that it's spitting out. The user
    //  is locked out by the modal loop, so they still cannot start
    //  another action until this one is done.
    //
    if (ePostRes == tCQCKit::EActPostRes::Process)
    {
        // Mark us as being processing commands now
        m_pitmStack->SetProcessingAt(c4HandlerId, kCIDLib::True);

        //
        //  Release the time janitor now that everything is in place. We will
        //  now show upas busy and lock out any other actions from happening
        //  until we've processed this one.
        //
        janTimers.Release();

        try
        {
            // Update the last activity time before we enter
            UpdateLastActivityStamp();

            //
            //  And now invoke the action. This will block until this and
            //  any nested actions that occur in response are done, or
            //  something fails.
            //
            tCQCKit::ECmdRes eRes = iactxTar.eInvokeCmds();

            //
            //  There is a possibility here of a trailing action, when
            //  the mouse button was released as the thread was winding
            //  down. If so, then there are will commands queued up, so
            //  we need to invoke it again.
            //
            //  We do it even if the one above returned a failure status
            //  or asked to stop, since we always want to get the release
            //  commands.  A stop request only applies to the original
            //  action anyway.
            //
            //  If the above invocation threw an exception somehow, then we
            //  can't run this guy since we will never get here, but otherwise
            //  we do.
            //
            //  We get a new command result and if it's worse than the
            //  original, we replace the original result with it.
            //
            //  Note that all of the original context info is still set
            //  up, so we can just invoke again.
            //
            if (iactxTar.bGotTrailingAction())
            {
                // Clear the flag first, just to be safe
                iactxTar.bGotTrailingAction(kCIDLib::False);
                const tCQCKit::ECmdRes eTrailRes = iactxTar.eInvokeCmds();

                // The new result is worse than the other, take it
                if (eTrailRes > eRes)
                    eRes = eTrailRes;
            }

            // Update the last activity time again
            UpdateLastActivityStamp();

            // Clear the processing flag for the current template
            m_pitmStack->SetProcessingAt(c4HandlerId, kCIDLib::False);

            // Clean up the context again
            iactxTar.Reset();

            //
            //  Bump the variable update counter to force a variable update
            //  pass now that the action is done.
            //
            m_c4VarUpdate++;

            // Take focus back now in case something happened to steal it
            TakeFocus();

            // If they indicated they want to exit, then do that.
            if (eRes == tCQCKit::ECmdRes::Exit)
            {
                if (iwdgTar.bIsPopupType())
                {
                    // It's a popup, so just force it out of the current loop
                    m_pitmStack->SetBreakFlagAt(c4HandlerId, kCIDLib::True);
                }
                 else
                {
                    // This is the base template, so do an exit special action
                    miahOwner().DoSpecialAction
                    (
                        tCQCIntfEng::ESpecActs::Exit, kCIDLib::True, *this
                    );
                }
            }

            // If it failed, then throw
            if (eRes >= tCQCKit::ECmdRes::Except)
            {
                facCQCIntfEng().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kIEngMsgs::midStatus_UserActionFailed
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            if (facCQCIntfEng().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            // Do the same cleanup as above
            iactxTar.Reset();
            m_pitmStack->SetProcessingAt(c4HandlerId, kCIDLib::False);

            // Take focus back now in case something happened to steal it
            TakeFocus();

            // Just in case we got far enough to change variables
            m_c4VarUpdate++;

            throw;
        }

        catch(...)
        {

            // Do the same cleanup as above
            iactxTar.Reset();
            m_pitmStack->SetProcessingAt(c4HandlerId, kCIDLib::False);

            // Take focus back now in case something happened to steal it
            TakeFocus();

            // Just in case we got far enough to change variables
            m_c4VarUpdate++;

            throw;
        }
    }
}


// Return the combined area of the base template and any popups
tCIDLib::TVoid TCQCIntfStdView::QueryCombinedAreas(TArea& areaToFill) const
{
    // The template manager has a method to do this
    m_pitmStack->QueryCombinedArea(areaToFill);
}


// We just pass this information on to all the widgets
tCIDLib::TVoid TCQCIntfStdView::ScrPosChanged(const TArea& areaTmplWnd)
{
    TArea areaTmpl;
    const tCIDLib::TCard4 c4SCount = m_pitmStack->c4CurCount();
    for (tCIDLib::TCard4 c4SIndex = 0; c4SIndex < c4SCount; c4SIndex++)
    {
        TCQCIntfTemplate& iwdgTmpl = m_pitmStack->iwdgTmplAt(c4SIndex);

        const tCIDLib::TCard4 c4WCount = iwdgTmpl.c4ChildCount();
        for (tCIDLib::TCard4 c4WIndex = 0; c4WIndex < c4WCount; c4WIndex++)
            iwdgTmpl.iwdgAt(c4WIndex).ScrPosChanged(areaTmplWnd);
    }
}


//
//  Sometimes variables have to be set outside of an action, such as when
//  a variables based slider or volume knob is moved. So we provide this
//  helper method for them to use to do that, which insures that we bump
//  the action counter to make it look like
//
tCIDLib::TVoid
TCQCIntfStdView::SetVariable(const  TString&    strKey
                            , const TString&    strValue)
{
    const tCIDLib::TBoolean bChanged = ctarGlobalVars().bAddOrUpdateVar
    (
        strKey
        , strValue
        , tCQCKit::EFldTypes::String
        , TString::strEmpty()
    );

    // If any change, then force a value update pass
    if (bChanged)
        m_c4VarUpdate++;
}


//
//  Overlays call this before they are about to replace their contents. If the focus
//  item is wihtin this container, we don't have to do anything since we'll set a new
//  focus item if we end up without one after the load is completed (which means we
//  blew the old one away.)
//
//  Since we don't do tracking anymore in the gesture world the way we used to (i.e.
//  clicks are not tracked, we get them after the fact and the tracking is already
//  over), we don't have to anymore worry about cancelling any tracking. Tracking
//  really only happens during scrolls, and we don't allow any commands during
//  scrolls that would replace contents.
//
//  We also call it when replace the whole template contents during a link operation,
//  in which case the passed container is the template itself.
//
//  When the process is complete, EndBrachReplace is called.
//
tCIDLib::TVoid TCQCIntfStdView::StartBranchReplace(TCQCIntfContainer& iwdgCont)
{
}



// ---------------------------------------------------------------------------
//  TCQCIntfStdView: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Get/set the no child mouse flag
//
tCIDLib::TBoolean TCQCIntfStdView::bNoChildMouse() const
{
    return m_bNoChildMouse;
}

tCIDLib::TBoolean TCQCIntfStdView::bNoChildMouse(const tCIDLib::TBoolean bToSet)
{
    m_bNoChildMouse = bToSet;
    return m_bNoChildMouse;
}


//
//  Queries field definition info about a field that is expected to be in
//  the cache of our interface polling engine.
//
tCIDLib::TBoolean
TCQCIntfStdView::bQueryFldInfo( const   TString&    strMoniker
                                , const TString&    strField
                                ,       TCQCFldDef& flddToFill)
{
    return facCQCIntfEng().polleThis().bQueryFldInfo(strMoniker, strField, flddToFill);
}


//
//  We handle various extended keys and map them to focus movement type
//  operations or a few other special operations. This is something only
//  invoked when the containing application has a way to get such key strokes
//  from the user, and it can also be faked in by remote commands that the
//  containinging app receives as a remote control mechanism.
//
//  We shouldn't get any keyboard input while we are processing an action. If
//  it happens, we reject the key.
//
tCIDLib::TBoolean
TCQCIntfStdView::bSpecialKey(const  tCIDCtrls::EExtKeys eKey
                            , const tCIDLib::TBoolean   bAltShift
                            , const tCIDLib::TBoolean   bCtrlShift
                            , const tCIDLib::TBoolean   bShift)
{
    // Update the last activity time
    UpdateLastActivityStamp();

    //
    //  If the current top-most template is processing commands, then we ignore
    //  keys, since we don't want to let them start anything new until the action
    //  is complete.
    //
    if (bActiveAction())
    {
        if (facCQCIntfEng().bLogInfo())
        {
            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Key press ignored because an action is being processed"
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return kCIDLib::True;
    }

    //
    //  If it's Ctrl-Shift-F1, then it's a request to exit fullscreen mode,
    //  which we have to pass on to the containing app, by way of a virtual
    //  that its view class will override.
    //
    if (!bAltShift && bCtrlShift && bShift && (eKey == tCIDCtrls::EExtKeys::F1))
    {
        miahOwner().DoSpecialAction(tCQCIntfEng::ESpecActs::ExitFS, kCIDLib::False, *this);
        return kCIDLib::True;
    }

    //
    //  Otherwise, if any shift keys and it's not the backtab, we don't deal
    //  with it.
    //
    if (bAltShift || bCtrlShift || bShift)
    {
        if ((eKey != tCIDCtrls::EExtKeys::BackTab) || bAltShift || bCtrlShift)
            return kCIDLib::False;
    }

    // Get the currently active template
    TCQCIntfTemplate& iwdgCur = iwdgTopMost();

    //
    //  If we are in a popup, then we want to exit this popup layer on an
    //  escape key. This may be ignored if the popup is a secure one.
    //
    if (eKey == tCIDCtrls::EExtKeys::Esc)
    {
        if (iwdgCur.bIsPopupType())
            m_pitmStack->SetTopBreakFlag(kCIDLib::True);
        return kCIDLib::True;
    }

    //
    //  All of this stuff is related to moving focus or is targeting the
    //  widget with the focus. So if focus is suppressed in this template,
    //  we don't need to do anything.
    //
    if (!iwdgCur.bNoFocus())
    {
        if ((eKey == tCIDCtrls::EExtKeys::Enter)
        ||  (eKey == tCIDCtrls::EExtKeys::Space))
        {
            //
            //  If we have a focus widget, then call it's Invoke method to give it
            //  a chance to do something in response. We just call it on the release.
            //
            TCQCIntfWidget* piwdgFocus = iwdgCur.piwdgFindCurFocus();
            if (piwdgFocus)
            {
                //
                //  Put a janitor on the widget so that it cannot be destroyed if ay
                //  action we kick off here were to replace its parent's contents.
                //  If it does, then this janitor will clean it up in the end.
                //
                //  This insures that the target widget won't get destroyed until
                //  we've unwound the stack back out of him.
                //
                TCQCIntfWdgJanitor janTracking(piwdgFocus);
                try
                {
                    // Send it an invoke command.
                    piwdgFocus->Invoke();
                }

                catch(TError& errToCatch)
                {
                    if (facCQCIntfEng().bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }
                    ShowErr
                    (
                        strTitle()
                        , facCQCIntfEng().strMsg(kIEngMsgs::midStatus_UserActionFailed)
                        , errToCatch
                    );
                }
            }
        }
         else
        {
            tCQCKit::EScrNavOps eOp = tCQCKit::EScrNavOps::Count;
            switch(eKey)
            {
                case tCIDCtrls::EExtKeys::BackSpace :
                    eOp = tCQCKit::EScrNavOps::Back;
                    break;

                case tCIDCtrls::EExtKeys::BackTab :
                    eOp = tCQCKit::EScrNavOps::Prev;
                    break;

                case tCIDCtrls::EExtKeys::Down :
                    eOp = tCQCKit::EScrNavOps::Down;
                    break;

                case tCIDCtrls::EExtKeys::End :
                    eOp = tCQCKit::EScrNavOps::End;
                    break;

                case tCIDCtrls::EExtKeys::Home :
                    eOp = tCQCKit::EScrNavOps::Home;
                    break;

                case tCIDCtrls::EExtKeys::Left :
                    eOp = tCQCKit::EScrNavOps::Left;
                    break;

                case tCIDCtrls::EExtKeys::PageDown :
                    eOp = tCQCKit::EScrNavOps::NextPage;
                    break;

                case tCIDCtrls::EExtKeys::PageUp :
                    eOp = tCQCKit::EScrNavOps::PrevPage;
                    break;

                case tCIDCtrls::EExtKeys::Right :
                    eOp = tCQCKit::EScrNavOps::Right;
                    break;

                case tCIDCtrls::EExtKeys::Tab :
                    eOp = tCQCKit::EScrNavOps::Next;
                    break;

                case tCIDCtrls::EExtKeys::Up :
                    eOp = tCQCKit::EScrNavOps::Up;
                    break;

                default :
                    return kCIDLib::False;
            };

            if (eOp != tCQCKit::EScrNavOps::Count)
            {
                TCQCIntfWidget* piwdgOld;
                TCQCIntfWidget* piwdgNew;

                //
                //  If it moved, then update loser and winner. If the old
                //  widget just moved the focus internally or did a page op
                //  but kept the focus, then it just redrew itself as
                //  required during the focus move call. This allows it to
                //  do the most optimal internal focus movement redraw.
                //
                piwdgNew = iwdgCur.piwdgMoveFocus(eOp, piwdgOld);
                if (piwdgOld != piwdgNew)
                {
                    if (piwdgOld)
                    {
                        piwdgOld->bHasFocus(kCIDLib::False);
                        piwdgOld->LostFocus();
                    }

                    if (piwdgNew)
                    {
                        piwdgNew->bHasFocus(kCIDLib::True);
                        piwdgNew->GotFocus();
                    }
                }
            }
        }
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// We just return some status info
tCIDLib::TCard4
TCQCIntfStdView::c4QueryStatus(TString& strBaseTmpl, TString& strTopTmpl)
{
    const tCIDLib::TCard4 c4Ret = m_pitmStack->c4CurCount();
    if (c4Ret)
    {
        strBaseTmpl = m_pitmStack->iwdgBaseTmpl().strTemplateName();
        strTopTmpl = m_pitmStack->iwdgTopMost().strTemplateName();
    }
     else
    {
        strBaseTmpl.Clear();
        strTopTmpl.Clear();
    }
    return c4Ret;
}


//
//  Our containing app has to tell us if the display resolution changes. For now
//  we don't do anything with this.
//
tCIDLib::TVoid TCQCIntfStdView::DisplayResChanged(const TSize&)
{
}



//
//  We've gotten a hot key, which generally comes from the pressing of a hard
//  button on a RIVA client, which gets mapped in whatever way to one of these
//  special keys and sent to us. We need to find out if we have a runnable
//  action mapped to one of these keys. It might be there, but not runnable
//  because it's not in the top-most popup.
//
tCIDLib::TVoid TCQCIntfStdView::HotKey(const tCQCIntfEng::EHotKeys eKey)
{
    // If we are busy, we just ignore this
    if (bIsBusy())
        return;

    //
    //  Looks reasonable, so get the top level template and pass this on
    //  to it. It'll search itself for a handler and if found it'll post the
    //  action. We don't care about the return value, it's just used for
    //  dealing with recursion.
    //
    TCQCIntfTemplate& iwdgTop = m_pitmStack->iwdgTopMost();
    iwdgTop.bHandleHotKey(eKey);
}


// Provide access to a specific level in the template stack
TCQCIntfTemplate& TCQCIntfStdView::iwdgAt(const tCIDLib::TCard4 c4HandlerId)
{
    return m_pitmStack->iwdgTmplAt(c4HandlerId);
}

const TCQCIntfTemplate&
TCQCIntfStdView::iwdgAt(const tCIDLib::TCard4 c4HandlerId) const
{
    return m_pitmStack->iwdgTmplAt(c4HandlerId);
}


//
//  We don't need to do anything per se, but we treat this as activity, so
//  we set the last activity time stamp.
//
tCIDLib::TVoid TCQCIntfStdView::GettingFocus()
{
    UpdateLastActivityStamp();
}


tCIDLib::TVoid TCQCIntfStdView::LosingFocus()
{
}


//
//  We don't have to lock during this because it won't happen while an
//  action is being processed. This is called when the window (or whatever
//  it is) that we are being displayed in changes size.
//
tCIDLib::TVoid TCQCIntfStdView::NewSize(const TSize& szNew)
{
    // Resize our double buffering bitmap to fit
    if (m_bmpDBuf.szBitmap() != szNew)
    {
        m_bmpDBuf = TBitmap
        (
            szNew
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );
    }

    //
    //  Get all of the templates repositioned as required. It will handle
    //  dynamic resizing if enabled, or just adjust origins to center or
    //  justify the template in the view area otherwise.
    //
    m_pitmStack->NewViewSize(szNew, m_bCenteredView);

    //
    //  We have to just redraw the whole area since we have to get the bgn
    //  updated (because we could be moving the template off of some bgn
    //  that needs to be filled.
    //
    InvalidateArea(areaView());
}



