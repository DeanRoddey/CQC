//
// FILE NAME: CQCIntfEng_Container.cpp
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
//  This file implements the out of line parts of the CQC user drawn
//  interface widget base class.
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


//
//  Because the widget headers are so hugely piggy, we don't want to force
//  them on all our clients. So they are brought in in the handful of places
//  required.
//
#include    "CQCIntfEng_PushButton.hpp"
#include    "CQCIntfEng_AdjustButton.hpp"
#include    "CQCIntfEng_CmdButton.hpp"
#include    "CQCIntfEng_CheckBox.hpp"
#include    "CQCIntfEng_SActPushButton.hpp"

#include    "CQCIntfEng_DynamicText.hpp"
#include    "CQCIntfEng_DynamicMarquee.hpp"

#include    "CQCIntfEng_StaticText.hpp"
#include    "CQCIntfEng_TimeText.hpp"
#include    "CQCIntfEng_NumericText.hpp"
#include    "CQCIntfEng_StaticMarquee.hpp"

#include    "CQCIntfEng_StaticImg.hpp"
#include    "CQCIntfEng_SpecialImg.hpp"
#include    "CQCIntfEng_MediaImage.hpp"

#include    "CQCIntfEng_AdvMItemBrowser.hpp"
#include    "CQCIntfEng_AnimatedImg.hpp"
#include    "CQCIntfEng_AreaFill.hpp"
#include    "CQCIntfEng_BooleanImg.hpp"
#include    "CQCIntfEng_BooleanText.hpp"
#include    "CQCIntfEng_CoverArtBrowser.hpp"
#include    "CQCIntfEng_DigitalClock.hpp"
#include    "CQCIntfEng_EnumImg.hpp"
#include    "CQCIntfEng_LogoImg.hpp"
#include    "CQCIntfEng_MediaCatBrowser.hpp"
#include    "CQCIntfEng_MediaListBrowser.hpp"
#include    "CQCIntfEng_MItemBrowser.hpp"
#include    "CQCIntfEng_MRItemBrowser.hpp"
#include    "CQCIntfEng_ProgressBar.hpp"
#include    "CQCIntfEng_Slider.hpp"
#include    "CQCIntfEng_ToolBar.hpp"
#include    "CQCIntfEng_VolKnob.hpp"
#include    "CQCIntfEng_VUMeter.hpp"
#include    "CQCIntfEng_WeatherChImg.hpp"
#include    "CQCIntfEng_WebBrowser.hpp"
#include    "CQCIntfEng_WebImage.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfContainer,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_Widget
    {
        // -----------------------------------------------------------------------
        //  Persistent format version for the container class
        //
        //  Version 2 -
        //      We made huge refactoring changes and changes to the way that
        //      field translations work and so forth leading up to the 2.5
        //      release, so we bump the version in order to handle those changes.
        //
        //  Version 3 -
        //      We moved to a hierarchical storage system for templates. This
        //      makes the storage of the template name in the template itself
        //      a problem, because now it won't be kept up with movement of the
        //      file within the hierarchy. So we go now purely by the file name
        //      of the template and have to discard the embedded name.
        //
        //  Version 4 -
        //      Added a flag to control whether this container does dynamic
        //      resizing.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2ContFmtVersion    = 4;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfContainer
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfContainer: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfContainer::~TCQCIntfContainer()
{
    // Clean up any states info
    try
    {
        delete m_pistlStates;
    }

    catch(...)
    {
    }

    //
    //  Remove all children. We have to do this, not just let the container
    //  destruct, because they are reference counted.
    //
    RemoveAllChildren();
}



// ---------------------------------------------------------------------------
//  TCQCIntfContainer: Public, virtual mtehods
// ---------------------------------------------------------------------------

//
//  When a hot key is received, a recursive search is done to find a handler. We only deal
//  with children here, so no infinite recursion issues. The objects we invoke will call
//  back into here, but in the context of their own objects, to do their own children.
//
tCIDLib::TBoolean TCQCIntfContainer::bHandleHotKey(const tCQCIntfEng::EHotKeys eKey)
{
    // OK, let's search for any containers and recurse on those that we find
    tCIDLib::TBoolean bRet = kCIDLib::False;
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgCont
            (
                static_cast<TCQCIntfContainer*>(piwdgCur)
            );

            if (piwdgCont->bHandleHotKey(eKey))
            {
                // He (or a child of his) handled it, so we are done
                bRet = kCIDLib::True;
                break;
            }
        }
    }
    return bRet;
}


//
//  This is a recursive call that only involves containers and derivatives of containers
//  (templates and overlays) so we implement it here and those derivatives will override
//  it, then call us. At our level, we just handle recursion, finding any other
//  containers and recursing on them. The derived classes run any triggered events they
//  may have.
//
//  Note that there's no recursion issue since we only invoke children, not our self.
//  So the derived classes calling back into here are calling in the context of their
//  own objects, our children, not us.
//
tCIDLib::TBoolean
TCQCIntfContainer::bInvokeTrgEvents(const   tCIDLib::TBoolean   bIsNight
                                    , const tCQCKit::TCQCEvPtr& cptrEv
                                    , const tCIDLib::TCard4     c4Hour
                                    , const tCIDLib::TCard4     c4Min
                                    ,       TTrigEvCmdTarget&   ctarEv)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgCont
            (
                static_cast<TCQCIntfContainer*>(piwdgCur)
            );
            if (piwdgCont->bInvokeTrgEvents(bIsNight, cptrEv, c4Hour, c4Min, ctarEv))
                bRet = kCIDLib::True;
        }
    }
    return bRet;
}


//
//  Finds the widget at the indicated point (in real coordinates, not relative), at
//  the highest z-order. We work backwards so that the first hit is the highest
//  z-order hit. We call the eHitTest() method on each non-container widget to see
//  if it was hit.
//
//  Normally this is never overridden. But the Live Tile class does so, because it
//  wants to create a non-interactive type of overlay, where the child widgets cannot
//  be interacted with.
//
TCQCIntfWidget*
TCQCIntfContainer::piwdgHit(const   TPoint&                 pntTest
                            , const tCQCIntfEng::EHitFlags  eFlags)
{
    //
    //  We have to go backwards, because we want to the topmost widget
    //  that gets hit, and going the other way would force us to check
    //  them all. This way, we can break out at the first hit.
    //
    tCIDLib::TCard4 c4Index = m_colChildren.c4ElemCount();

    // If none, the we are done now
    if (!c4Index)
        return 0;

    // If searching for widgets, do that first
    if (tCIDLib::bAllBitsOn(eFlags, tCQCIntfEng::EHitFlags::Widgets))
    {
        do
        {
            c4Index--;
            TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

            // If this one is hidden, then skip it
            if (piwdgCur->eCurDisplayState() == tCQCIntfEng::EDispStates::Hidden)
                continue;

            //
            //  If this widget is a container, we need to recurse on it. Else,
            //  just test the current widget.
            //
            if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
            {
                TCQCIntfContainer* piwdgCont
                (
                    static_cast<TCQCIntfContainer*>(piwdgCur)
                );

                //
                //  If the point is within this container's area, then we
                //  need to recurse on it.
                //
                if (piwdgCont->areaActual().bContainsPoint(pntTest))
                {
                    //
                    //  Only look for widgets here. In the case of something like
                    //  the live tile, it will return itself as what was hit, since
                    //  it is acting as widget for hit testing purposes.
                    //
                    TCQCIntfWidget* piwdgHit = piwdgCont->piwdgHit
                    (
                        pntTest, tCQCIntfEng::EHitFlags::Widgets
                    );

                    // If we found it, return it now
                    if (piwdgHit)
                        return piwdgHit;

                    //
                    //  The point is within this container. So, if this is
                    //  not the root container (the base template, so it's
                    //  an overlay or some other future sub-container), there
                    //  is no need to look further.
                    //
                    //  If we didn't find it in here, and this point falls
                    //  into this container, and this container must be above
                    //  any other at this point, then it can't hit anything
                    //  now.
                    //
                    if (piwdgCur->piwdgParent() != nullptr)
                        break;
                }
            }
             else if (piwdgCur->areaActual().bContainsPoint(pntTest))
            {
                //
                //  Ask it if we hit it by it's own criteria. If they indicate
                //  nowhere, we return failure now. If they indicate anything
                //  else other than transparent, we take this one. If transparent,
                //  we keep searching under this widget.
                //
                const tCIDCtrls::EWndAreas eArea = piwdgCur->eHitTest(pntTest);
                if (eArea == tCIDCtrls::EWndAreas::Nowhere)
                    break;
                else if (eArea != tCIDCtrls::EWndAreas::Transparent)
                    return piwdgCur;
            }
        }   while (c4Index);
    }

    // Nothing yet. If we are doing containers, do that.
    if (tCIDLib::bAllBitsOn(eFlags, tCQCIntfEng::EHitFlags::Conts))
    {
        c4Index = m_colChildren.c4ElemCount();

        do
        {
            c4Index--;
            TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

            // If this one is hidden, then skip it
            if (piwdgCur->eCurDisplayState() == tCQCIntfEng::EDispStates::Hidden)
                continue;

            //
            //  If this guy contains the point, then we first need to
            //  recurse, since there may be a nested container above it.
            //  If not, this is our guy.
            //
            if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
            {
                TCQCIntfContainer* piwdgCont
                (
                    static_cast<TCQCIntfContainer*>(piwdgCur)
                );

                //
                //  If the point is within this container's area, then we
                //  need to recurse on it. Only look for containers here.
                //
                if (piwdgCont->areaActual().bContainsPoint(pntTest))
                {
                    TCQCIntfWidget* piwdgHit = piwdgCont->piwdgHit
                    (
                        pntTest, tCQCIntfEng::EHitFlags::Conts
                    );

                    // If we hit it, this is our guy
                    if (piwdgHit)
                        return piwdgHit;

                    // Else it's the current container
                    return piwdgCont;
                }
            }

        }   while (c4Index);
    }

    // Never hit anything
    return 0;
}


//
//  This is for supporting the designer. If the user renames a widget, this will
//  invalidate any commands that are targeting it. So he will call this to have
//  us update any command targets.
//
//  For the most part we can handle this on behalf of all the widgets. We just
//  see which ones implement the command source mixin, and for the others we
//  can call a method that will get references to any embedded command sources
//  (such as the buttons inside toolbars.)
//
//  But some like templates and overlays have to override and deal with some
//  actions that they maintain at their level and then call us, where we'll
//  deal with their regular commands and their children.
//
//  We have to return a list of all of the widgets affected, since the designer
//  has to be able to undo this operation.
//
tCIDLib::TVoid
TCQCIntfContainer::UpdateCmdTarget( const   TString&                    strOldTargetId
                                    , const TString&                    strNewTargetName
                                    , const TString&                    strNewTargetId
                                    ,       tCQCIntfEng::TDesIdList&    fcolAffected)
{
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
            if (pmcsrcCur->bUpdateCmdTarget(strOldTargetId, strNewTargetName, strNewTargetId))
            {
                if (!fcolAffected.bElementPresent(iwdgCur.c8DesId()))
                    fcolAffected.c4AddElement(iwdgCur.c8DesId());
            }
        }
         else
        {
            tCIDLib::TBoolean bChanged = kCIDLib::False;
            colSrcs.RemoveAll();
            const tCIDLib::TCard4 c4EmbCnt = iwdgCur.c4QueryEmbeddedCmdSrcs(colSrcs);
            for (tCIDLib::TCard4 c4EmbInd = 0; c4EmbInd < c4EmbCnt; c4EmbInd++)
            {
                bChanged |= colSrcs[c4EmbInd]->bUpdateCmdTarget
                (
                    strOldTargetId, strNewTargetName, strNewTargetId
                );
            }

            // If any changes, add this one if not already in the list
            if (bChanged)
            {
                if (!fcolAffected.bElementPresent(iwdgCur.c8DesId()))
                    fcolAffected.c4AddElement(iwdgCur.c8DesId());
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfContainer: Public, inherited mtehods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfContainer::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our widget level stuff first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    //
    //  Compare any nested content (only if a template, overlays will be empty
    //  at design time.
    //
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    const TCQCIntfContainer& iwdgOther
    (
        static_cast<const TCQCIntfContainer&>(iwdgSrc)
    );

    // If different child widget count, then obvious different
    if (c4Count != iwdgOther.m_colChildren.c4ElemCount())
        return kCIDLib::False;

    // Compare the children
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget& iwdg1 = *m_colChildren[c4Index];
        const TCQCIntfWidget& iwdg2 = *iwdgOther.m_colChildren[c4Index];

        // If they aren't the same type, then clearly not
        if (iwdg1.clsIsA() != iwdg2.clsIsA())
            return kCIDLib::False;

        // Let them compare
        if (!iwdg1.bIsSame(iwdg2))
            return kCIDLib::False;
    }

    // The child widgets are the same, compare the states
    return (*m_pistlStates == *iwdgOther.m_pistlStates);
}


//
//  We need to distribute this to our children. The derived container will call us first.
//  If we return false, he'll check any other command sources he has.
//
//  Currently only the template would need to override, since the overlay only has these
//  types of other command sources at runtime, getting them from a template loaded into
//  him.
//
tCIDLib::TBoolean TCQCIntfContainer::bReferencesTarId(const TString& strId) const
{
    // Now we can initialize our children
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colChildren[c4Index]->bReferencesTarId(strId))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TVoid
TCQCIntfContainer::Initialize(  TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Now we can initialize our children
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        //
        //  Initialize this guy, which might recurse if this is a container.
        //  Pass us as the parent container this time.
        //
        piwdgCur->Initialize(this, dsclInit, colErrs);
    }
}


// At this level, we just iterate our children and recurse on them
tCIDLib::TVoid
TCQCIntfContainer::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colChildren[c4Index]->QueryMonikers(colToFill);
}


//
//  This is a purely to get attributes on a specific widget, so no recusions in
//  this case. The template name is implicit, so we don't let them edit it here. It's
//  set to whatever the temlate is saved as.
//
tCIDLib::TVoid
TCQCIntfContainer::QueryWdgAttrs(tCIDMData::TAttrList& colAttrs, TAttrData& adatTmp) const
{
    // Call our parent first
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    //
    //  Our states we do as a binary blob. We only do this if it's a template, not
    //  an overlay. That also means we need to put in the template's separator line as
    //  well, otherwise the states would be above it.
    //
    if (clsIsA() == TCQCIntfTemplate::clsThis())
    {
        colAttrs.objPlace
        (
            L"Template Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
        );

        TBinMBufOutStream strmOut(1024UL);
        strmOut << *m_pistlStates << kCIDLib::FlushIt;
        colAttrs.objPlace
        (
            L"Template States"
            , kCQCIntfEng::strAttr_Tmpl_States
            , strmOut.mbufData()
            , strmOut.c4CurPos()
            , TString::strEmpty()
            , tCIDMData::EAttrEdTypes::Visual
        );
    }
}


// At this level, we just iterate our children and recurse on them
tCIDLib::TVoid TCQCIntfContainer::RefreshImages(TDataSrvClient& dsclToUse)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colChildren[c4Index]->RefreshImages(dsclToUse);
}


//
//  This is called during loading of a template, to make an initial pass
//  where everyone can register any fields that they have, and see if the
//  fields have changed such that the value cannot be used.
//
tCIDLib::TVoid
TCQCIntfContainer::RegisterFields(          TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    // Let our states register
    m_pistlStates->RegisterFields(polleToUse, cfcData);

    // If we have no children, then nothing to do
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    if (!c4Count)
        return;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colChildren[c4Index]->RegisterFields(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfContainer::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFullMatch
                            ,       TRegEx&                 regxFind)
{
    // If the children flag is on, do them
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Children))
    {
        const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colChildren[c4Index]->Replace
            (
                eToRep, strSrc, strTar, bRegEx, bFullMatch, regxFind
            );
        }
    }

    // Do the states
    m_pistlStates->Replace(eToRep, strSrc, strTar, bRegEx, bFullMatch, regxFind);
}


// Reset all our attributes
tCIDLib::TVoid TCQCIntfContainer::ResetWidget()
{
    m_pistlStates->Reset();
    RemoveAllChildren();
    TParent::ResetWidget();
}


// This is design time only, so no recursion
tCIDLib::TVoid
TCQCIntfContainer::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // Call our parent
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Handle stuff we do at this level
    if (adatNew.strId() == kCQCIntfEng::strAttr_Tmpl_States)
    {
        // Stream out the data back to us again
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        strmSrc >> *m_pistlStates;
    }
}


tCIDLib::TVoid
TCQCIntfContainer::Validate(const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Let our states validate
    m_pistlStates->Validate(cfcData, colErrs, dsclVal);

    // Recurse on our children to let them do any of their own validation
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        piwdgCur->Validate(cfcData, colErrs, dsclVal);

        //
        //  If this child implements the command source mixin, then we will do some
        //  action command validation on its behalf. If it has any other nested command
        //  sources, it has to do those itself.
        //
        const MCQCCmdSrcIntf* pmcsrcCur = dynamic_cast<const MCQCCmdSrcIntf*>(piwdgCur);
        if (pmcsrcCur)
            ValidateCmds(L"Widget Commands", *piwdgCur, *pmcsrcCur, colErrs);
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfContainer: Public, non-virtual mtehods
// ---------------------------------------------------------------------------

//
//  Calculates the area that contains all the widgets. We use the relative
//  areas, so it is relative to our origin, not the window origin.
//
TArea TCQCIntfContainer::areaHull() const
{
    TArea areaRet;

    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        areaRet |= m_colChildren[c4Index]->areaRelative();
    return areaRet;
}


// Add a new child widget to our list
tCIDLib::TVoid
TCQCIntfContainer::AddChild(        TCQCIntfWidget* const   piwdgToAdopt
                            , const tCIDLib::EEnds          eEnd)
{
    // Tell it that we are it's parent
    piwdgToAdopt->SetParent(this);

    // Either insert at the start, or append to the end, according to eEnd
    if (eEnd == tCIDLib::EEnds::First)
        m_colChildren.InsertAt(piwdgToAdopt, 0);
    else
        m_colChildren.Add(piwdgToAdopt);

    // Bump its ref count
    piwdgToAdopt->IncRefCount();
}


// Add a new child widget at a particular index
tCIDLib::TVoid
TCQCIntfContainer::AddChildAt(          TCQCIntfWidget* const   piwdgToAdopt
                                , const tCIDLib::TCard4         c4At)
{
    // Tell it that we are it's parent
    piwdgToAdopt->SetParent(this);
    m_colChildren.InsertAt(piwdgToAdopt, c4At);

    // Bump its ref count
    piwdgToAdopt->IncRefCount();
}


//
//  This will pull the children from the source list and put it into ours.
//  We don't adjust the ref count since the caller is assumed to just
//  abandon them. Both lists should be adopting, so this is safe.
//
tCIDLib::TVoid
TCQCIntfContainer::AdoptChildren(tCQCIntfEng::TChildList& colChildren)
{
    // Tell the passed collection to give up all his widgets
    RemoveAllChildren();
    m_colChildren = tCIDLib::ForceMove(colChildren);

    // And loop through them and set us as their parent container
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colChildren[c4Index]->SetParent(this);
}


//
//  See if we contain the indicated widget, either as an immediate child or
//  indirectly. The later only has meaning in the viewer, not during design.
//
tCIDLib::TBoolean
TCQCIntfContainer::bContainsWidget( const   TCQCIntfWidget* const   piwdgFind
                                    , const tCIDLib::TBoolean       bRecurse) const
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        // If this is our guy, we are done
        if (piwdgCur == piwdgFind)
            return kCIDLib::True;

        // If recursing, and the current one is a container, then recurse
        if (bRecurse)
        {
            if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
            {
                //
                //  Cast it to the container interface and recurse. If we find it
                //  in there, then return true, else keep looking.
                //
                const TCQCIntfContainer* piwdgCont
                (
                    static_cast<const TCQCIntfContainer*>(piwdgCur)
                );

                if (piwdgCont->bContainsWidget(piwdgFind, kCIDLib::True))
                    return kCIDLib::True;
            }
        }
    }
    return kCIDLib::False;
}


// Used by the designer, to see if the indicated design time id exists
tCIDLib::TBoolean
TCQCIntfContainer::bDesIdExists(const tCIDLib::TCard8 c8ToCheck) const
{
    //
    //  See if it's this container object, which handles the top level
    //  template itself. For any others, it'll be seen as a child of
    //  the level above first.
    //
    if (c8DesId() == c8ToCheck)
        return kCIDLib::True;

    // Not us, so check the children
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colChildren[c4Index]->c8DesId() == c8ToCheck)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Draw the child widgets. This is separated from the main draw methods
//  below, because the overlay widget has special needs wrt to scrolling
//  that require it to draw the children separately. we allow the widgets
//  to be drawn relative to an arbitrary origin.
//
tCIDLib::TBoolean
TCQCIntfContainer::bDrawChildren(       TGraphDrawDev&          gdevTarget
                                , const TArea&                  areaTest
                                , const TGUIRegion&             grgnClip
                                , const TCQCIntfWidget* const   piwdgStopAt
                                , const tCIDLib::TBoolean       bStopBefore)
{
    // Assume we won't find the stop at
    tCIDLib::TBoolean bFoundStop = kCIDLib::False;

    // And now let's process all our children
    TArea       areaUpdate;
    TGUIRegion  grgnCur;
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        // Never going to happen, but make analzyer happy
        CIDAssert(piwdgCur != nullptr, L"Child widget is null");

        // See if it's the stop at
        if (piwdgCur == piwdgStopAt)
        {
            // Remember we found it
            bFoundStop = kCIDLib::True;

            // If stopping before, then we are done now
            if (bStopBefore)
                break;
        }

        const tCIDLib::TBoolean bIsCont
        (
            piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis())
        );


        //
        //  Create an invalid area for this one, which is the intersection
        //  of its area and the test area.
        //
        areaUpdate = piwdgCur->areaActual();
        areaUpdate &= areaTest;

        //
        //  If this guy is hidden or doesn't intersect the update area, then
        //  we don't need to draw it. But, if it is a container and we have
        //  a stop widget, we still need to search it to see if it contains
        //  the stop widget. If so, we are done.
        //
        //  Else, if it's not hidden and intersects, we draw
        //
        if (((piwdgCur->eCurDisplayState() == tCQCIntfEng::EDispStates::Hidden)
        ||   areaUpdate.bIsEmpty())
        &&  (bIsCont && piwdgStopAt))
        {
            TCQCIntfContainer* piwdgCont
            (
                static_cast<TCQCIntfContainer*>(piwdgCur)
            );

            bFoundStop = piwdgCont->bContainsWidget(piwdgStopAt);
            if (bFoundStop)
                break;
        }
         else if ((piwdgCur->eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden)
              &&  !areaUpdate.bIsEmpty())
        {
            if (bIsCont)
            {
                //
                //  Cast and recurse. If this guy finds the stop widget down
                //  within him or his children, we are done.
                //
                TCQCIntfContainer* piwdgCont
                (
                    static_cast<TCQCIntfContainer*>(piwdgCur)
                );

                bFoundStop = piwdgCont->bDrawUnder
                (
                    gdevTarget, areaUpdate, piwdgStopAt, bStopBefore
                );

                // If this nested guy had the stop widget, we are done
                if (bFoundStop)
                    break;
            }
             else
            {
                //
                //  Save the current state on each round so each widget
                //  starts with the default.
                //
                TGraphicDevJanitor janDev(&gdevTarget);

                try
                {
                    // Get this guy's clip region, set it, and draw
                    piwdgCur->QueryClipRegion(grgnCur);
                    TRegionJanitor janClip
                    (
                        &gdevTarget, grgnCur, tCIDGraphDev::EClipModes::And
                    );
                    piwdgCur->DrawOn(gdevTarget, areaUpdate, grgnCur);
                }

                catch(TError& errToCatch)
                {
                    if (facCQCIntfEng().bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);

                        facCQCIntfEng().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kIEngErrs::errcDbg_WdgUpdateExcept
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , piwdgCur->strTypeName()
                            , piwdgCur->strWidgetId()
                        );
                    }
                }

                catch(...)
                {
                    facCQCIntfEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcDbg_WdgUpdateExcept
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , piwdgCur->strTypeName()
                        , piwdgCur->strWidgetId()
                    );
                }
            }
        }
    }
    return bFoundStop;
}


//
//  Draw all the widgets under the indicated area to the passed target device.
//
//  The can also pass a 'stop at' widget. We'll draw until we either come to
//  it, or finsish drawing it, according to the bStopBefore flag. So we either
//  draw everything under it or draw it and everything under it.
//
//  We return whether or not we hit the stop at widget. If it was zero, then
//  we'll always return false.
//
tCIDLib::TBoolean
TCQCIntfContainer::bDrawUnder(          TGraphDrawDev&          gdevTarget
                                , const TArea&                  areaTest
                                , const TCQCIntfWidget* const   piwdgStopAt
                                , const tCIDLib::TBoolean       bStopBefore)
{
    TArea areaUpdate;
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();

    // If we don't intersect the test area, we are done
    if (!areaTest.bIntersects(areaActual()))
        return kCIDLib::False;

    //
    //  If hidden, we don't need to return anything, but we still need
    //  to return if we contain the stop widget if any.
    //
    if (eCurDisplayState() == tCQCIntfEng::EDispStates::Hidden)
    {
        // If no stop at, just return false now
        if (!piwdgStopAt)
            return kCIDLib::False;

        // Else see if we have it
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colChildren[c4Index] == piwdgStopAt)
                return kCIDLib::True;
        }

        // Never found it
        return kCIDLib::False;
    }

    //
    //  Start off with a clip area that is the test area. There's no need
    //  to draw anything that falls outside of that.
    //
    TRegionJanitor janClip(&gdevTarget, areaTest, tCIDGraphDev::EClipModes::Copy);

    //
    //  Initially we have to draw our own contents. Add our clip to the main test
    //  area, and draw. He may adjust the clip for our border which we will then set
    //  again below before we draw the children.
    //
    TGUIRegion grgnClip;
    QueryClipRegion(grgnClip);
    {
        TGraphicDevJanitor janDev(&gdevTarget);
        TRegionJanitor janClip2(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);
        DrawOn(gdevTarget, areaTest, grgnClip);
    }

    // Add our (possibly updated) clip region to the test clip
    TRegionJanitor janRgn(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Draw the children, return if we hit the stop widget if any
    return bDrawChildren(gdevTarget, areaTest, grgnClip, piwdgStopAt, bStopBefore);
}


//
//  Draws all the content under the passed widget, and optionally the widget
//  itself (or optionally stopping before we draw the target widget.) We just
//  call the version above with the area of the target.
//
tCIDLib::TBoolean
TCQCIntfContainer::bDrawUnder(          TGraphDrawDev&          gdevTarget
                                , const TCQCIntfWidget* const   piwdgStopAt
                                , const tCIDLib::TBoolean       bStopBefore)
{
    return bDrawUnder
    (
        gdevTarget, piwdgStopAt->areaActual(), piwdgStopAt, bStopBefore
    );
}


//
//  Given a widget, find all other members of the group. We return them in
//  their z-order within the group. We have one version that returns a list of
//  design ids and one that returns a widget list.
//
//  We also have one that takes a group name. It just finds the widget that
//  has that group name, and then calls the other version.
//
tCIDLib::TBoolean
TCQCIntfContainer::bFindGroupMembers(const  tCIDLib::TCard4             c4GroupId
                                    ,       tCQCIntfEng::TDesIdList&    fcolToFill
                                    ,       tCIDLib::TCard4&            c4FirstZ) const
{
    fcolToFill.RemoveAll();

    // Get the z-order of the first member of the group
    c4FirstZ = c4FindFirstGroupZ(c4GroupId);

    //
    //  It shouldn't ever indicate this guy isn't a child of ours, but if so
    //  return false
    //
    if (c4FirstZ == kCIDLib::c4MaxCard)
        return kCIDLib::False;

    //
    //  OK, we can load all of these up. Add the first one and move forward. Now
    //  we can go forward until we hit another one that doesn't match the group
    //  if of the passed widget.
    //
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    tCIDLib::TCard4 c4GrpInd = c4FirstZ;
    while (c4GrpInd < c4Count)
    {
        const TCQCIntfWidget* piwdgCur = m_colChildren[c4GrpInd];
        if (piwdgCur->c4GroupId() != c4GroupId)
            break;

        fcolToFill.c4AddElement(piwdgCur->c8DesId());
        c4GrpInd++;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCIntfContainer::bFindGroupMembers(const  tCIDLib::TCard4             c4GroupId
                                    ,       tCQCIntfEng::TChildList&    colToFill
                                    ,       tCIDLib::TCard4&            c4FirstZ)
{
    // It can't be an adopting collection
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"The widget list cannot adopt"
    );

    colToFill.RemoveAll();

    // Get the z-order of the first member of the group
    c4FirstZ = c4FindFirstGroupZ(c4GroupId);

    //
    //  It shouldn't ever indicate this guy isn't a child of ours, but if so
    //  return false
    //
    if (c4FirstZ == kCIDLib::c4MaxCard)
        return kCIDLib::False;

    //
    //  OK, we can load all of these up. Add the first one and move forward. Now
    //  we can go forward until we hit another one that doesn't match the group
    //  if of the passed widget.
    //
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    tCIDLib::TCard4 c4GrpInd = c4FirstZ;
    while (c4GrpInd < c4Count)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4GrpInd];
        if (piwdgCur->c4GroupId() != c4GroupId)
            break;

        colToFill.Add(piwdgCur);
        c4GrpInd++;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfContainer::bFindGroupMembers(const  TString&                    strGrpName
                                    ,       tCQCIntfEng::TDesIdList&    fcolToFill
                                    ,       tCIDLib::TCard4&            c4FirstZ)
{
    // Find the widget with the indicated group name first
    TCQCIntfWidget* piwdgGrpMem = nullptr;
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colChildren[c4Index]->strGroupName() == strGrpName)
        {
            piwdgGrpMem = m_colChildren[c4Index];
            break;
        }
    }

    // If not found, then obvious not
    if (!piwdgGrpMem)
        return kCIDLib::False;

    // We found it, so call the other version
    return bFindGroupMembers(piwdgGrpMem->c4GroupId(), fcolToFill, c4FirstZ);
}

tCIDLib::TBoolean
TCQCIntfContainer::bFindGroupMembers(const  TString&                    strGrpName
                                    ,       tCQCIntfEng::TChildList&    colToFill
                                    ,       tCIDLib::TCard4&            c4FirstZ)
{
    // Find the widget with the indicated group name first
    TCQCIntfWidget* piwdgGrpMem = nullptr;
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colChildren[c4Index]->strGroupName() == strGrpName)
        {
            piwdgGrpMem = m_colChildren[c4Index];
            break;
        }
    }

    // If not found, then obvious not
    if (!piwdgGrpMem)
        return kCIDLib::False;

    // We found it, so call the other version
    return bFindGroupMembers(piwdgGrpMem->c4GroupId(), colToFill, c4FirstZ);
}


//
//  Checks to see if the passed group name is used. A specific widget can be
//  provided as an exception, i.e. it will be ignored if it has the id. This is
//  so the existing value of a widget that is being edited want cause its own
//  name to be rejected.
//
tCIDLib::TBoolean
TCQCIntfContainer::bGroupNameUsed(  const  TString&                 strToCheck
                                    , const TCQCIntfWidget* const   piwdgExcept) const
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if ((m_colChildren[c4Index]->strGroupName() == strToCheck)
        &&  (m_colChildren[c4Index] != piwdgExcept))
        {
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  This is a special method that allows for a a break from the usual strict
//  hierarchical nature of action commands. A widget in an overlay can ask to load
//  a new template into a sibling overlay, by calling up to its parent container.
//  We can handle this on behalf of our derived classes easily enough.
//
//  Note that, if we don't have an overlay with the indicated id, then we'll recurse
//  upwards and try it again. We don't allow it ever to reload the
//
tCIDLib::TBoolean
TCQCIntfContainer::bLoadSiblingOverlay( const   TString&                strOverlayId
                                        , const TString&                strToLoad
                                        ,       tCQCIntfEng::TErrList&  colErrs)
{
    // We just see if we have an overlay child with that widget id
    TCQCIntfWidget* piwdgTar = piwdgFindByName(strOverlayId, kCIDLib::False);
    TCQCIntfOverlay* piwdgOver = dynamic_cast<TCQCIntfOverlay*>(piwdgTar);
    if (!piwdgOver)
    {
        // It's not, so that's bad
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcLink_NotAnOverlay
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , strOverlayId
        );
    }

    // Ok, ask his to load the indicated template
    return piwdgOver->bLoadNewTemplate(strToLoad, colErrs);
}


//
//  Find the next/prev insert point. When we move widgets in the z-order, we
//  don't allow them to move into or out of groups. So we have to skip over
//  groups to find the next available spot. The start position is the widget's
//  current position, so we start at the next one before or after that.
//
//  The incoming index is the current z-order of the one to move. It will either
//  be a single item, or a single selected group. That's the only way that the
//  editor should allow this type of operation.
//
//  We get his group id. If he's in a group, we have to get past that in the
//  direction of travel, if we can. If not, we return false.
//
//  Once we are beyond the current widget/group, then we can look for the next
//  possible insertion point. So we move forward/back one more and, if it's a single
//  we take that, else we move to the end/start of that new group.
//
//
//  NOTE: This guy can return the current child count if the next position moving
//  forward is at the end. This is what is expected by the callers of this method,
//  but be careful not to use the insert point returned as an actual index, since
//  it could be invalid in that usage. The collection item movement methods that
//  these values are passed to take a to position that is where it should go wrt
//  the current positions, not after the item is removed from the from position.
//
tCIDLib::TBoolean
TCQCIntfContainer::bNextPrevInsertPnt(  const   tCIDLib::TCard4     c4StartAt
                                        ,       tCIDLib::TCard4&    c4Insert
                                        , const tCIDLib::TBoolean   bForward) const
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();

    // The start cannot be count or beyond
    CIDAssert(c4StartAt < c4Count, L"Start insert point is beyond end of list");

    // Start us off at the start position
    c4Insert = c4StartAt;

    // OK, we can potentially do something
    tCIDLib::TCard4 c4GrpId = m_colChildren[c4Insert]->c4GroupId();

    //
    //  If doing a single widget, then we can test simply for whether we are at the
    //  end already. For groups we don't assume the start point is the first or last
    //  of the group.
    //
    if (!c4GrpId)
    {
        if (bForward && (c4StartAt + 1 >= c4Count))
            return kCIDLib::False;
        else if (!bForward && !c4StartAt)
            return kCIDLib::False;
    }

    if (bForward)
    {
        // If currently in a group, then move to the point just past it.
        if (c4GrpId)
        {
            while (c4Insert < c4Count)
            {
                if (m_colChildren[c4Insert]->c4GroupId() != c4GrpId)
                    break;
                c4Insert++;
            }

            // If we hit the end, then this group goes to the end so nothing to do
            if (c4Insert == c4Count)
                return kCIDLib::False;
        }
         else
        {
            c4Insert++;
        }

        //
        //  If we ended up on a another group, we have to move past it. Else we just move
        //  forward again after the next widget. Either of these may take us to c4Count,
        //  which means we are moving to the end.
        //
        c4GrpId = m_colChildren[c4Insert]->c4GroupId();
        if (c4GrpId)
        {
            while (c4Insert < c4Count)
            {
                if (m_colChildren[c4Insert]->c4GroupId() != c4GrpId)
                    break;
                c4Insert++;
            }
        }
         else if (c4Insert < c4Count)
        {
            c4Insert++;
        }
    }
     else
    {
        //
        //  If the current one is in a group, move back to the first of that group,
        //  so that the code below will work correctly. If we hit zero, then that
        //  group extends to the bottom and we can't do anything.
        //
        if (c4GrpId)
        {
            c4Insert = c4FindFirstGroupZ(c4GrpId);
            if (!c4Insert)
                return kCIDLib::False;

            // Now move back before the current group
            c4Insert--;
        }
         else
        {
            //
            //  Not grouped, so just move back. We know if doing a single that the
            //  start at point was not zero, since we checked above.
            //
            c4Insert--;
        }

        //
        //  If we ended up on another group, then we have to move back to the start
        //  of it. Not past it, the insert point is where the first item of this new
        //  group is. If it's not a group, we are already now on the one before the
        //  starting widget/group.
        //
        c4GrpId = m_colChildren[c4Insert]->c4GroupId();
        if (c4GrpId)
            c4Insert = c4FindFirstGroupZ(c4GrpId);
    }
    return kCIDLib::True;
}


//
//  We check to make sure that all of our child widgets that reference our states
//  are valid, i.e. that we've not deleted any that they reference. If all are good
//  we return true, else false and a list of the names of any states that are
//  referenced but not found
//
tCIDLib::TBoolean
TCQCIntfContainer::bCheckForInvalidStates(tCIDLib::TStrHashSet& colBadList) const
{
    colBadList.RemoveAll();

    tCIDLib::TBoolean bRet = kCIDLib::True;
    const tCIDLib::TCard4 c4ChildCount = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        // The list may be null, in which case he adds all of his states
        if (!m_colChildren[c4Index]->bCheckStateRefs(m_pistlStates, colBadList))
            bRet = kCIDLib::False;
    }
    return bRet;
}


//
//  Checks recursively to see if there are any widgets that would prevent us
//  from allowing a derived overlay from being scrollable. Things like web
//  widgets prevent scrolling.
//
tCIDLib::TBoolean TCQCIntfContainer::bCheckForNonScrollables() const
{

    return kCIDLib::False;
}


// Checked to see if a state exists at the indicated index and returns the name
tCIDLib::TBoolean
TCQCIntfContainer::bStateExists(const   tCIDLib::TCard4 c4Index
                                ,       TString&        strToFill) const
{
    // Just delegate to our state list object
    return m_pistlStates->bGetStateName(c4Index, strToFill);
}


// Sees if this container has a state with the indicated name
tCIDLib::TBoolean TCQCIntfContainer::bStateExists(const TString& strToFind) const
{
    return (m_pistlStates->pistFind(strToFind) != nullptr);
}


//
//  Checks to see if the passed widget id is used. A specific widget can be
//  provided as an exception, i.e. it will be ignored if it has the id.
//
tCIDLib::TBoolean
TCQCIntfContainer::bWidgetIdUsed(const  TString&                strToCheck
                                , const TCQCIntfWidget* const   piwdgExcept) const
{
    // Search for any widgets with this name
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if ((m_colChildren[c4Index]->strWidgetId() == strToCheck)
        &&  (m_colChildren[c4Index] != piwdgExcept))
        {
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// Assigns new designer ids to our widgets starting at the passed id value
tCIDLib::TCard4
TCQCIntfContainer::c4AssignDesIdsFrom(const tCIDLib::TCard4 c4From)
{
    //
    //  The id is the combination of our id shifted to the high 32 bits
    //  plus the sequential ids in the low 32 bits. So get our id by
    //  taking the low 32 bits of our own id.
    //
    const tCIDLib::TCard4 c4OurId = tCIDLib::TCard4(c8DesId() >> 32);

    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    tCIDLib::TCard4 c4Cur = c4From;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colChildren[c4Index]->c8DesId(TRawBits::c8From32(c4Cur++, c4OurId));

    return c4Cur;
}


// Just returns our count of children
tCIDLib::TCard4 TCQCIntfContainer::c4ChildCount() const
{
    return m_colChildren.c4ElemCount();
}


//
//  Find the zorder of the first member of the group that has the passed group
//  id. If it's not in a group, it returns max card.
//
tCIDLib::TCard4
TCQCIntfContainer::c4FindFirstGroupZ(const tCIDLib::TCard4 c4GroupId) const
{
    //
    //  If it's not a group member, then we are done. We don't want to search
    //  the list because any ungrouped item has a zero id, so we'd match the
    //  first ungrouped one we found, which would be bad.
    //
    if (!c4GroupId)
        return kCIDLib::c4MaxCard;

    //
    //  We have to just search forward till we either find a widget with this
    //  group Id, or hit the end.
    //
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colChildren[c4Index]->c4GroupId() == c4GroupId)
            return c4Index;
    }
    return kCIDLib::c4MaxCard;
}


//
//  Looks up the passed widget and returns it's position in the list of
//  children. If not found, it returns c4MaxCard, or throws if indicated.
//
tCIDLib::TCard4
TCQCIntfContainer::c4WdgZOrder( const   TCQCIntfWidget* const   piwdgToFind
                                , const tCIDLib::TBoolean       bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colChildren[c4Index] == piwdgToFind)
            return c4Index;
    }

    if (bThrowIfNot)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcTmpl_NotTmplChild
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }
    return kCIDLib::c4MaxCard;
}


//
//  Move the indicated widget to another z-order position in the child list.
//
//
//  NOTE:
//
//  This is for the editor's use when it's actually manipulating groups,
//  and knows it is going to deal with group ids and such. For normal z-order
//  movement use eWidgetZOrder() which will handle all the complex issues of
//  moving groups and widgets. This noe is used really when doing grouping, to
//  get get the selected widgets into successive z-order.
//
tCIDLib::TVoid
TCQCIntfContainer::ChangeZOrder(        TCQCIntfWidget* const   piwdgToMove
                                , const tCIDLib::TCard4         c4To)
{
    // Find the zorder of the widget to move
    const tCIDLib::TCard4 c4CurOrder = c4WdgZOrder(piwdgToMove);

    // If not the same as the requested target order, then let's move it
    if (c4CurOrder != c4To)
        m_colChildren.MoveItem(c4CurOrder, c4To);
}


// Just remove any states associated with this container
tCIDLib::TVoid TCQCIntfContainer::ClearStates()
{
    CIDAssert(m_pistlStates != nullptr, L"The states list should not be zero here");
    m_pistlStates->Reset();
}


// Ask all widgets to remove any references to the passed states
tCIDLib::TVoid TCQCIntfContainer::DeleteStates(const tCIDLib::TStrCollect& colToRemove)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        piwdgCur->DeleteStates(colToRemove);
    }
}


//
//  This is called after a template is loaded and the widgets are initialized.
//  We recursively call the PostInit() method on all our widgets and their
//  widgets and so on.
//
tCIDLib::TVoid TCQCIntfContainer::DoPostInits()
{
    // We call this on containers and non-containers, including ourself
    PostInit();

    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        //
        //  If this widget is a container, we need to recurse on it.
        //  Otherwise, call PostInit on it. The container will get called
        //  above when we recurse.
        //
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgCont
            (
                static_cast<TCQCIntfContainer*>(piwdgCur)
            );
            piwdgCont->DoPostInits();
        }
         else
        {
            piwdgCur->PostInit();
        }
    }
}


//
//  This is called after a popup's modal loop is exited, to give it a chance
//  to clean up.
//
tCIDLib::TVoid TCQCIntfContainer::DoOnExits(TCQCIntfView& civOwner)
{
    // Recurse first if needed
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        // If this widget is a container, we need to recurse on it.
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgCont
            (
                static_cast<TCQCIntfContainer*>(piwdgCur)
            );
            piwdgCont->DoOnExits(civOwner);
        }
    }

    // Invoke our own last
    DoOnExit(civOwner);
}



//
//  This is called after load, init, and post init, to do the OnLoad
//  acions.
//
tCIDLib::TVoid TCQCIntfContainer::DoOnLoads(TCQCIntfView& civOwner)
{
    // Invoke our own onload first
    DoOnLoad(civOwner);

    // Then recurse if needed
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        // If this widget is a container, we need to recurse on it.
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgCont
            (
                static_cast<TCQCIntfContainer*>(piwdgCur)
            );
            piwdgCont->DoOnLoads(civOwner);
        }
    }
}


//
//  This is called after the load and basic initialization is done. It gives
//  all containers a chance to do their OnPreload actions, which are all
//  done synchronously.
//
tCIDLib::TVoid
TCQCIntfContainer::DoPreloads(          TCQCIntfView&   civOwner
                                , const TString&        strInvokeParms)
{
    //  Invoke any preload action at this level first
    DoOnPreload(civOwner, strInvokeParms);

    // And now recurse
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        // If this widget is a container, we need to recurse on it.
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgCont
            (
                static_cast<TCQCIntfContainer*>(piwdgCur)
            );
            piwdgCont->DoPreloads(civOwner, strInvokeParms);
        }
    }
}


//
//  This method will adjust a whole group in the Z-Order.
//
tCIDLib::TBoolean
TCQCIntfContainer::bChangeGroupZOrder(  const   tCIDLib::TCard4         c4GroupId
                                        , const tCQCIntfEng::EZOrder    eMove
                                        ,       tCIDLib::TCard4&        c4NewZ)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    CIDAssert(c4Count > 1, L"Group operation called when widget count is < 2");
    if (c4Count < 2)
        return kCIDLib::False;

    // Get the members of the group and find the index of the first member
    tCQCIntfEng::TChildList colGroup(tCIDLib::EAdoptOpts::NoAdopt);
    tCIDLib::TCard4 c4OrgZ;
    if (!bFindGroupMembers(c4GroupId, colGroup, c4OrgZ))
    {
        CIDAssert2(L"No group members were found");
        return kCIDLib::False;
    }
    const tCIDLib::TCard4 c4GrpCnt = colGroup.c4ElemCount();
    CIDAssert(c4GrpCnt > 1, L"The group has < 2 members");
    if (c4GrpCnt < 2)
        return kCIDLib::False;

    // Now decide the new index for the first member of the group
    c4NewZ = c4OrgZ;
    switch(eMove)
    {
        case tCQCIntfEng::EZOrder::ToBack :
            c4NewZ = 0;
            break;

        case tCQCIntfEng::EZOrder::ToFront  :
            //
            //  If this group is not already at the end, then move it to
            //  the end. Just set it to count, which will cause them to
            //  go to the end.
            //
            if (c4OrgZ + c4GrpCnt != c4Count)
                c4NewZ = c4Count;
            break;

        case tCQCIntfEng::EZOrder::Back :
        case tCQCIntfEng::EZOrder::Forward :
            //
            //  Find the next or previous available insert point. This will skip over
            //  groups as though they were single widgets, so we won't move a group into
            //  a group.
            //
            if (!bNextPrevInsertPnt(c4OrgZ, c4NewZ, eMove == tCQCIntfEng::EZOrder::Forward))
                c4NewZ = c4OrgZ;
            break;

        default :
            CIDAssert2(L"Unknown z-order movement type")
            return kCIDLib::False;
    };

    //
    //  If we need to do it, then orphan all the group members out
    //  of the list (we have them in the group list already), and
    //  then insert the group list back at the new z order.
    //
    if (c4NewZ != c4OrgZ)
    {
        //
        //  Not we orphan at the start index each time, since we are removing
        //  widgets from the list, which would make the actual indices invalid.
        //
        tCIDLib::TCard4 c4GrpInd = c4GrpCnt;
        while (c4GrpInd)
        {
            m_colChildren.OrphanElemAt(c4OrgZ);
            c4GrpInd--;
        }

        //
        //  If we are moving to the end, then we just append them back.
        //  If moving to a lower index, we just insert them all back at
        //  the lower index. If moving to a higher index, we have to
        //  adjust for the number we removed.
        //
        //  As we go we accumlate an area that covers them all so we
        //  can redraw.
        //
        TArea areaRefresh;
        if (c4NewZ == c4Count)
        {
            for (c4GrpInd = 0; c4GrpInd < c4GrpCnt; c4GrpInd++)
            {
                TCQCIntfWidget* piwdgAdd = colGroup[c4GrpInd];
                if (c4GrpInd)
                    areaRefresh |= piwdgAdd->areaActual();
                else
                    areaRefresh = piwdgAdd->areaActual();
                m_colChildren.Add(piwdgAdd);
            }
        }
         else
        {
            //
            //  If moving to a higher z-order, then we affected the
            //  indices when we removed them above. So we need to adjust
            //  the new z down by that much.
            //
            if (c4NewZ > c4OrgZ)
                c4NewZ -= c4GrpCnt;

            for (c4GrpInd = 0; c4GrpInd < c4GrpCnt; c4GrpInd++)
            {
                TCQCIntfWidget* piwdgAdd = colGroup[c4GrpInd];
                if (c4GrpInd)
                    areaRefresh |= piwdgAdd->areaActual();
                else
                    areaRefresh = piwdgAdd->areaActual();
                m_colChildren.InsertAt(piwdgAdd, c4NewZ + c4GrpInd);
            }
        }

        // Redraw the affected area
        civOwner().Redraw(areaRefresh);

        return kCIDLib::True;
    }

    // We didn't affect the group's position
    return kCIDLib::False;
}


//
//  This method will adjust a single widget's z-order in the direction
//  indicated in the eMove parameter. The z-order just follows the order of
//  the widgets in the vector we store them in (with the 0th element being
//  the furthest back, and then moving fowards.)
//
//  There are essentially only two scenarios.
//
//  1.  The widget is in a group, in which case we move it within the group only.
//  2.  It's a non-grouped widget, so we move it, skipping groups as though they
//      were single widgets.
//
//  We have to keep the group name on the first widget in the group. This only
//  can be affected in scenario #2.
//
tCIDLib::TBoolean
TCQCIntfContainer::bChangeWidgetZOrder(         TCQCIntfWidget* const   piwdgToMove
                                        , const tCQCIntfEng::EZOrder    eMove
                                        ,       tCIDLib::TCard4&        c4NewZ)
{
    // Get the count, and if there's zero or one, we can't do anything
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    if (c4Count < 2)
        return kCIDLib::False;

    // Find the widget's current z-order
    tCIDLib::TCard4 c4CurZ = 0;
    for (; c4CurZ < c4Count; c4CurZ++)
    {
        if (m_colChildren[c4CurZ] == piwdgToMove)
            break;
    }

    // If not found, then that's really bad
    if (c4CurZ == c4Count)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcTmpl_NotTmplChild
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    // Start off the new z order at the incoming
    c4NewZ = c4CurZ;

    // Remember if it's in a group
    const tCIDLib::TCard4 c4GroupId = piwdgToMove->c4GroupId();


    //
    //  Set our max movement range. If in a group, it's the range of the group
    //  members (non-inclusive, so end index is the one beyond the group. If
    //  not in a group, then max range is zero to the count. This way we can do
    //  the movement code generically.
    //
    tCIDLib::TCard4 c4MaxInd;
    tCIDLib::TCard4 c4MinInd;
    if (c4GroupId)
    {
        c4MinInd = c4FindFirstGroupZ(c4GroupId);
        c4MaxInd  = c4MinInd + 1;
        while (c4MaxInd < c4Count)
        {
            if (m_colChildren[c4MaxInd]->c4GroupId() != c4GroupId)
                break;
            c4MaxInd++;
        }
    }
     else
    {
        c4MinInd = 0;
        c4MaxInd = c4Count;
    }

    // Now based on the requested move, adjust the index
    switch(eMove)
    {
        case tCQCIntfEng::EZOrder::ToBack :
        {
            c4NewZ = c4MinInd;
            break;
        }

        case tCQCIntfEng::EZOrder::ToFront  :
            c4NewZ = c4MaxInd;
            break;

        case tCQCIntfEng::EZOrder::Back :
        {
            if (c4GroupId)
            {
                // It's in the group, so just move back if possible
                if (c4NewZ > c4MinInd)
                    c4NewZ--;
            }
             else
            {
                //
                //  Find the previous available insert point. If none is
                //  available, then return false.
                //
                if (!bNextPrevInsertPnt(c4CurZ, c4NewZ, kCIDLib::False))
                    c4NewZ = c4CurZ;
            }
            break;
        }

        case tCQCIntfEng::EZOrder::Forward :
        {
            if (c4GroupId)
            {
                // It's in the group, so just move back if possible
                if (c4NewZ + 1 < c4MaxInd)
                    c4NewZ++;
            }
             else
            {
                if (!bNextPrevInsertPnt(c4CurZ, c4NewZ, kCIDLib::True))
                    c4NewZ = c4CurZ;
            }
            break;
        }

        default :
            CIDAssert2(L"Unknown z-order movement type")
            break;
    };

    // If we changed order, then do what is necessary
    if (c4NewZ != c4CurZ)
    {
        //
        //  Deal with the group name. We don't even bother to worry too much
        //  about how it gets done. Save the name from the min index. Later we'll
        //  put it back on the min index. If it wasn't grouped, then it'll be
        //  empty anyway.
        //
        TString strGroupName = m_colChildren[c4MinInd]->strGroupName();
        m_colChildren.MoveItem(c4CurZ, c4NewZ);
        m_colChildren[c4MinInd]->strGroupName(strGroupName);

        return kCIDLib::True;
    }

    // Never changed the index
    return kCIDLib::False;
}


//
//  Given a path to a template, see if it's fully qualified. If not, then expand
//  it by making it relative to our path. We have to have a fully qualified path,
//  since it's expanded out when loaded and couldn't have loaded if it wasn't
//  successfully expanded.
//
tCIDLib::TVoid
TCQCIntfContainer::ExpandTmplPath(  const   TString&    strOrg
                                    ,       TString&    strExpPath) const
{
    // We call a lower level, generic helper to do this
    facCQCKit().bExpandResPath(m_strTemplateName, strOrg, strExpPath);
}


//
//  Finds all of our immediate children and descendants which can accept the
//  focus, i.e. we recursively search.
//
tCIDLib::TVoid
TCQCIntfContainer::FindAllFocusable(tCQCIntfEng::TChildList& colToFill)
{
    // If in debug mode, make sure the collection does not adopt
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"The widget list cannot adopt"
    );

    //
    //  We just use a more generic helper that will find all widgets with
    //  a given capabilities flag, in this case the flag that indicates it
    //  can take focus.
    //
    QueryAllWithCapFlags
    (
        tCQCIntfEng::ECapFlags::TakesFocus
        , tCQCIntfEng::ECapFlags::TakesFocus
        , colToFill
    );
}


//
//  Add all our named children to the passed collection. Don't flush the
//  list first because it's used to add widgets to a list that already has
//  stuff in it usually. They can tell us to ignore a particular widget,
//  so that we don't add that one to the list.
//
//  They can ask us to do a recursive search, else we just search our own
//  immediate children.
//
tCIDLib::TVoid TCQCIntfContainer::
FindAllTargets(         tCQCKit::TCmdTarList&   colToFill
                , const tCIDLib::TBoolean       bRecurse
                , const TCQCIntfWidget* const   piwdgIgnore)
{
    // If in debug mode, make sure the collection does not adopt
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"The widget list cannot adopt"
    );

    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (!piwdgCur->strWidgetId().bIsEmpty() && (piwdgCur != piwdgIgnore))
            colToFill.Add(dynamic_cast<MCQCCmdTarIntf*>(piwdgCur));

        // Recurse if asked to
        if (bRecurse)
        {
            TCQCIntfContainer* piwdgCont = dynamic_cast<TCQCIntfContainer*>(piwdgCur);
            if (piwdgCont)
                piwdgCont->FindAllTargets(colToFill, bRecurse, piwdgIgnore);
        }
    }
}

tCIDLib::TVoid TCQCIntfContainer::
FindAllTargets(         tCQCKit::TCCmdTarList&  colToFill
                , const tCIDLib::TBoolean       bRecurse
                , const TCQCIntfWidget* const   piwdgIgnore) const
{
    // If in debug mode, make sure the collection does not adopt
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"The widget list cannot adopt"
    );

    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (!piwdgCur->strWidgetId().bIsEmpty() && (piwdgCur != piwdgIgnore))
            colToFill.Add(dynamic_cast<const MCQCCmdTarIntf*>(piwdgCur));

        // Recurse if asked to
        if (bRecurse)
        {
            const TCQCIntfContainer* piwdgCont
            (
                dynamic_cast<const TCQCIntfContainer*>(piwdgCur)
            );
            if (piwdgCont)
                piwdgCont->FindAllTargets(colToFill, bRecurse, piwdgIgnore);
        }
    }
}


// Get or set the states associated with this container
const TCQCIntfStateList& TCQCIntfContainer::istlStates() const
{
    return *m_pistlStates;
}

const TCQCIntfStateList&
TCQCIntfContainer::istlStates(const TCQCIntfStateList& istlToSet)
{
    *m_pistlStates = istlToSet;
    return *m_pistlStates;
}


// Provides indexed access to our children
TCQCIntfWidget& TCQCIntfContainer::iwdgAt(const tCIDLib::TCard4 c4At)
{
    return *m_colChildren[c4At];
}

const TCQCIntfWidget&
TCQCIntfContainer::iwdgAt(const tCIDLib::TCard4 c4At) const
{
    return *m_colChildren[c4At];
}


//
//  This is called upon load of a template (or a template into an overlay.)
//  It will just force an initial, non-redrawing, value scan on all of the
//  widgets. It will only be called on the template or overlay. We then turn
//  around and call the value scan protected virtual on ourself, which will
//  cause the recursive update of all widgets below us.
//
tCIDLib::TVoid
TCQCIntfContainer::InitialScan( TCQCPollEngine&     polleToUse
                                , TStdVarsTar&      ctarGlobals)
{
    TGUIRegion grgnClip;
    QueryClipRegion(grgnClip);
    ValueUpdate
    (
        polleToUse, kCIDLib::True, kCIDLib::True, ctarGlobals, grgnClip
    );
}


// Create a path relative to us for the passed original path
tCIDLib::TVoid
TCQCIntfContainer::MakeRelTmplPath(const TString& strOrg, TString& strRelPath) const
{
    facCQCKit().bMakeRelResPath(m_strTemplateName, strOrg, strRelPath);
}


//
//  This method allows the editor to orphan a member of a group. If it only leaves
//  one widget in the group, we disband it.
//
//  The caller is responsible for the widget after this, since it is no longer
//  in this container's list of widgets.
//
tCIDLib::TVoid
TCQCIntfContainer::OrphanGrpMember( TCQCIntfWidget* const   piwdgToOrphan
                                    , tCIDLib::TCard4&      c4GrpZ
                                    , tCIDLib::TBoolean&    bGrpDestroyed)
{
    tCQCIntfEng::TChildList colGrp(tCIDLib::EAdoptOpts::NoAdopt);
    if (!bFindGroupMembers(piwdgToOrphan->c4GroupId(), colGrp, c4GrpZ))
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcGrp_NotGroupMember
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
        );
    }

    const tCIDLib::TCard4 c4GrpCnt = colGrp.c4ElemCount();

    // Look forward from the group z-order to find this target one's z-order
    tCIDLib::TCard4 c4WdgZ = c4GrpZ;
    {
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4GrpCnt)
        {
            if (colGrp[c4Index] == piwdgToOrphan)
                break;
            c4Index++;
        }
        CIDAssert(c4Index < c4GrpCnt, L"The widget was not in its reported group");
        c4WdgZ += c4Index;
    }

    // Remember if removing the first group member
    const tCIDLib::TBoolean bFirstMember(piwdgToOrphan == colGrp[0]);

    //
    //  Let's orphan it out now from both the container's list and the group
    //  list we just looked up.
    //
    m_colChildren.OrphanElemAt(c4WdgZ);
    colGrp.OrphanElem(piwdgToOrphan);

    if (colGrp.c4ElemCount() == 1)
    {
        // We are destroying the group
        bGrpDestroyed = kCIDLib::True;

        colGrp[0]->c4GroupId(0);
        colGrp[0]->strGroupName(TString::strEmpty());
    }
     else
    {
        bGrpDestroyed = kCIDLib::False;

        // If we removed the first one, then copy over the group name
        if (bFirstMember)
            colGrp[0]->strGroupName(piwdgToOrphan->strGroupName());
    }
}


//
//  Looks up a widget by the design time id. This is only useful by the
//  designer since it drives the assignment of these ids. We have const
//  and non-const versions and versions that return the z-order of the
//  located widget.
//
TCQCIntfWidget*
TCQCIntfContainer::piwdgFindByDesId(const tCIDLib::TCard8 c8ToFind)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (piwdgCur->c8DesId() == c8ToFind)
            return piwdgCur;
    }

    // If it's our id, return us
    if (c8ToFind == c8DesId())
        return this;
    return 0;
}

TCQCIntfWidget*
TCQCIntfContainer::piwdgFindByDesId(const   tCIDLib::TCard8     c8ToFind
                                    ,       tCIDLib::TCard4&    c4ZOrder)
{
    c4ZOrder = kCIDLib::c4MaxCard;
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (piwdgCur->c8DesId() == c8ToFind)
        {
            c4ZOrder = c4Index;
            return piwdgCur;
        }
    }

    // If it's our id, return us
    if (c8ToFind == c8DesId())
        return this;
    return 0;
}

const TCQCIntfWidget*
TCQCIntfContainer::piwdgFindByDesId(const tCIDLib::TCard8 c8ToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (piwdgCur->c8DesId() == c8ToFind)
            return piwdgCur;
    }

    // If it's our id, return us
    if (c8ToFind == c8DesId())
        return this;
    return 0;
}

const TCQCIntfWidget*
TCQCIntfContainer::piwdgFindByDesId(const   tCIDLib::TCard8     c8ToFind
                                    ,       tCIDLib::TCard4&    c4ZOrder) const
{
    c4ZOrder = kCIDLib::c4MaxCard;
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (piwdgCur->c8DesId() == c8ToFind)
        {
            c4ZOrder = c4Index;
            return piwdgCur;
        }
    }

    // If it's our id, return us
    if (c8ToFind == c8DesId())
        return this;
    return nullptr;
}


//
//  Find a widget by it's name. They can ask us to recurse or just check our
//  immediate children.
//
const TCQCIntfWidget*
TCQCIntfContainer::piwdgFindByName( const   TString&            strName
                                    , const tCIDLib::TBoolean   bRecurse) const
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    if (!c4Count)
        return 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (piwdgCur->m_strWidgetId == strName)
            return piwdgCur;

        // If it's a container, and the recurse flag is set, then recurse
        if (bRecurse && piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            const TCQCIntfContainer* piwdgNest
            (
                static_cast<const TCQCIntfContainer*>(piwdgCur)
            );
            piwdgCur = piwdgNest->piwdgFindByName(strName, bRecurse);
            if (piwdgCur)
                return piwdgCur;
        }
    }
    return nullptr;
}

TCQCIntfWidget*
TCQCIntfContainer::piwdgFindByName( const   TString&            strName
                                    , const tCIDLib::TBoolean   bRecurse)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    if (!c4Count)
        return 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (piwdgCur->m_strWidgetId == strName)
            return piwdgCur;

        // If it's a container, and the recurse flag is set, then recurse
        if (bRecurse && piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgNest = static_cast<TCQCIntfContainer*>(piwdgCur);
            piwdgCur = piwdgNest->piwdgFindByName(strName, bRecurse);
            if (piwdgCur)
                return piwdgCur;
        }
    }
    return nullptr;
}


//
//  Looks up a widget by it's unique id.
//
TCQCIntfWidget*
TCQCIntfContainer::piwdgFindByUID(  const   tCIDLib::TCard4     c4ToFind
                                    , const tCIDLib::TBoolean   bRecurse)
{
    // If it's our id, return us
    if (c4ToFind == c4UniqueId())
        return this;

    // It's not, so search further
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        if (piwdgCur->c4UniqueId() == c4ToFind)
            return piwdgCur;

        // If it's a container, and the recurse flag is set, then recurse
        if (bRecurse && piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgNest = static_cast<TCQCIntfContainer*>(piwdgCur);
            piwdgCur = piwdgNest->piwdgFindByUID(c4ToFind, bRecurse);
            if (piwdgCur)
                return piwdgCur;
        }
    }

    // Never found it as this
    return nullptr;
}


//
//  Finds the first widget that has the passed flags. We have second version
//  that returns the parent container as well, if any. We provide a dummy
//  version that provides and eats that parameter, so they don't have to
//  provide it.
//
TCQCIntfWidget*
TCQCIntfContainer::piwdgFindRunFlags(const  tCQCIntfEng::ERunFlags  eToFind
                                    , const tCQCIntfEng::ERunFlags  eMask)
{
    TCQCIntfContainer*  piwdgEat;
    return piwdgFindRunFlags(eToFind, eMask, piwdgEat);
}

TCQCIntfWidget*
TCQCIntfContainer::piwdgFindRunFlags(const  tCQCIntfEng::ERunFlags  eToFind
                                    , const tCQCIntfEng::ERunFlags  eMask
                                    ,       TCQCIntfContainer*&     piwdgPar)
{
    // Test us first
    const tCIDLib::TCard4 c4Mask = tCIDLib::c4EnumOrd(eMask);
    const tCIDLib::TCard4 c4ToFind = tCIDLib::c4EnumOrd(eToFind);
    tCIDLib::TCard4 c4CurFlags = tCIDLib::c4EnumOrd(eRunFlags());

    //
    //  Turn off all non-relevant bits in the current flags. If that now
    //  matches the to find bit set, then we matched.
    //
    if ((c4CurFlags & c4Mask) == c4ToFind)
        return this;

    //
    //  It wasn't us, so iterate our children and check them, recursing if
    //  we hit a container. If we have no children, just return null.
    //
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    if (!c4Count)
        return 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        // Check this guy first, even if he's a container
        if ((tCIDLib::TCard4(piwdgCur->eRunFlags()) & c4Mask) == c4ToFind)
            return piwdgCur;

        // Not him, so see if he's a container and recurse if so
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgCont
            (
                static_cast<TCQCIntfContainer*>(piwdgCur)
            );

            //
            //  If he returns a non-zero, he found one, so return it, else
            //  just keep going.
            //
            TCQCIntfWidget* piwdgFind = piwdgCont->piwdgFindRunFlags(eToFind, eMask);
            if (piwdgFind)
            {
                piwdgPar = piwdgCont;
                return piwdgFind;
            }
        }
    }

    // Never found one at this level and below
    return nullptr;
}


const TCQCIntfWidget*
TCQCIntfContainer::piwdgFindRunFlags(const  tCQCIntfEng::ERunFlags  eToFind
                                    , const tCQCIntfEng::ERunFlags  eMask) const
{
    const TCQCIntfContainer* piwdgEat;
    return piwdgFindRunFlags(eToFind, eMask, piwdgEat);
}

const TCQCIntfWidget*
TCQCIntfContainer::piwdgFindRunFlags(const  tCQCIntfEng::ERunFlags  eToFind
                                    , const tCQCIntfEng::ERunFlags  eMask
                                    , const TCQCIntfContainer*&     piwdgPar) const
{
    // Test us first
    const tCIDLib::TCard4 c4Mask = tCIDLib::c4EnumOrd(eMask);
    const tCIDLib::TCard4 c4ToFind = tCIDLib::c4EnumOrd(eToFind);
    tCIDLib::TCard4 c4CurFlags = tCIDLib::c4EnumOrd(eRunFlags());

    //
    //  Turn off all non-relevant bits in the current flags. If that now
    //  matches the to find bit set, then we matched.
    //
    if ((c4CurFlags & c4Mask) == c4ToFind)
        return this;

    //
    //  It wasn't us, so iterate our children and check them, recursing if
    //  we hit a container. If we have no children, just return null.
    //
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    if (!c4Count)
        return 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        // Check this guy first, even if he's a container
        if ((tCIDLib::TCard4(piwdgCur->eRunFlags()) & c4Mask) == c4ToFind)
            return piwdgCur;

        // Not him, so see if he's a container and recurse if so
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            const TCQCIntfContainer* piwdgCont
            (
                static_cast<const TCQCIntfContainer*>(piwdgCur)
            );

            //
            //  If he returns a non-zero, he found one, so return it, else
            //  just keep going.
            //
            const TCQCIntfWidget* piwdgFind = piwdgCont->piwdgFindRunFlags
            (
                eToFind, eMask
            );

            if (piwdgFind)
            {
                piwdgPar = piwdgCont;
                return piwdgFind;
            }
        }
    }

    // Never found one at this level and below
    return 0;
}


//
//  Looks for those widgets that have a specific runtime flag set on them.
//  We have set of flags that indicate focus, mouse over, tracking, etc..
//  These all do a recursive search, using a helper method on our parent
//  container class that will find the first widget with a given set of run
//  flags. If no widget has the indicate flag, then we return zero.
//
//  We do it this way because widgets can get blown away at any time. Using
//  pointers is very dangerous since we can end up referencing a now
//  destroyed widget.
//
TCQCIntfWidget* TCQCIntfContainer::piwdgFindCurFocus()
{
    return piwdgFindRunFlags
    (
        tCQCIntfEng::ERunFlags::HasFocus, tCQCIntfEng::ERunFlags::HasFocus
    );
}

TCQCIntfWidget*
TCQCIntfContainer::piwdgFindCurFocus(TCQCIntfContainer*& piwdgParent)
{
    //
    //  Pass the focus flag for both the value and the mask, since we want
    //  to find one with just that bit set.
    //
    return piwdgFindRunFlags
    (
        tCQCIntfEng::ERunFlags::HasFocus
        , tCQCIntfEng::ERunFlags::HasFocus
        , piwdgParent
    );
}


//
//  Find the next or previous widget that can accept the focus, handling
//  recursion downwards, but not upwards, i.e. we don't wrap here. That's
//  handled at a higher level.
//
TCQCIntfWidget*
TCQCIntfContainer::piwdgNextPrevFocus(          TCQCIntfWidget* const piwdgStart
                                        , const tCIDLib::TBoolean     bNext)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();

    // If we have no children, give up now
    if (!c4Count)
    {
        //
        //  If the start widget isn't null, this cannot be good since he
        //  could not be one of ours.
        //
        CIDAssert(piwdgStart != nullptr, L"Passed start widget is not one of ours");
        return nullptr;
    }

    //
    //  If the start widget is null, start with our first (or last) one, else
    //  it must be one of ours, so run up to it, and then start at the one
    //  after (or before) it. If it's the last (or first) one, we give up
    //  immediately.
    //
    tCIDLib::TCard4 c4Index = 0;
    TCQCIntfWidget* piwdgRet = nullptr;
    if (piwdgStart)
    {
        for (; c4Index < c4Count; c4Index++)
        {
            piwdgRet = m_colChildren[c4Index];
            if (piwdgRet == piwdgStart)
                break;
        }

        if (c4Index >= c4Count)
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcTmpl_NotAChild
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , piwdgStart->clsIsA()
            );
        }

        //
        //  We need to start at the next (or previous) one. If this is the
        //  last (or first) one, then we return false now.
        //
        if (bNext)
        {
            c4Index++;
            if (c4Index >= c4Count)
                return nullptr;
        }
         else
        {
            if (!c4Index)
                return nullptr;
            c4Index--;
        }
    }
     else
    {
        //
        //  No starting item, and we are going backwards, so we set the
        //  start index to the last index.
        //
        if (!bNext)
            c4Index = c4Count - 1;
    }

    //
    //  Ok, now look forward (or back) for one that takes the focus. If we
    //  hit a container, we recurse. If he gives us one back, we return
    //  that, else we keep looking.
    //
    while (kCIDLib::True)
    {
        piwdgRet = m_colChildren[c4Index];
        if (piwdgRet->bAcceptsFocus())
            break;

        if (piwdgRet->bIsDescendantOf(clsThis()))
        {
            //
            //  It's a container, so recurse. Pass in zero to have it search
            //  all it's children.
            //
            TCQCIntfContainer* piwdgNewCont
            (
                static_cast<TCQCIntfContainer*>(piwdgRet)
            );
            piwdgRet = piwdgNewCont->piwdgNextPrevFocus(0, bNext);
            if (piwdgRet)
                break;
        }

        if (bNext)
        {
            c4Index++;
            if (c4Index >= c4Count)
                return nullptr;
        }
         else
        {
            if (!c4Index)
                return nullptr;
            c4Index--;
        }
    }

    return piwdgRet;
}


//
//  In the designer, our collection is enabled to push changes. But it cannot know
//  if we modified a widget directly. So the widgets will call us here if any changes
//  we care about for publishing purposes change. We can then ask the collection to
//  publish an 'element change' msg for us. This then covers all the bases required
//  to keep things like the widget palette up to date.
//
//  In some cases this will get called during editing for widgets that are not in
//  the template, because they are copies of ones that are. So if the widget isn't
//  one of our children, we just ignore it. Though this could in theory let a bug through,
//  by not reporting a bad widget, other things will catch that.
//
//  If the passed widget is null, then that means it is us that was changed. In that
//  case we pass max card for the index.
//
tCIDLib::TVoid
TCQCIntfContainer::PublishWidgetChanged(const TCQCIntfWidget* const piwdgChanged)
{
    if (piwdgChanged)
    {
        const tCIDLib::TCard4 c4At = c4WdgZOrder(piwdgChanged, kCIDLib::False);
        if (c4At != kCIDLib::c4MaxCard)
            m_colChildren.PublishElemChanged(c4At);
    }
     else
    {
        m_colChildren.PublishElemChanged(kCIDLib::c4MaxCard);
    }
}


//
//  Just gives back a list of the design time ids for all of the current
//  widgets (but not ourself.) We don't return locked widgets unless we
//  are forced to.
//
tCIDLib::TVoid TCQCIntfContainer::
QueryAllDesIds(TFundVector<         tCIDLib::TCard8>&   fcolToFill
                            , const tCIDLib::TBoolean   bIncludeLocked)
{
    fcolToFill.RemoveAll();
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (bIncludeLocked || !m_colChildren[c4Index]->bIsLocked())
            fcolToFill.c4AddElement(m_colChildren[c4Index]->c8DesId());
    }
}


//
//  At this level we just call our children's version of this method, to
//  do the required recursion.
//
tCIDLib::TVoid TCQCIntfContainer::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&   colScopes
                    , const tCIDLib::TBoolean       bDoSysImages
                    , const tCIDLib::TBoolean       bIncludeScope) const
{
    // Just iterate our children and call their version
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_colChildren[c4Index]->QueryReferencedImgs
        (
            colToFill, colScopes, bDoSysImages, bIncludeScope
        );
    }
}


//
//  Remove the widget with the passed design time id. This is a design
//  time only thing, so it doesn't have to worry about reference counts or
//  anything.
//
tCIDLib::TVoid TCQCIntfContainer::RemoveByDesId(const tCIDLib::TCard8 c8ToFind)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colChildren[c4Index]->c8DesId() == c8ToFind)
        {
            m_colChildren.RemoveAt(c4Index);
            break;
        }
    }
}

//
//  Remove the widget with the passed widget name. This is a design
//  time only thing, so it doesn't have to worry about reference counts or
//  anything. It wouldn't work at viewing time since the id is modified
//  then to include runtime specific stuff.
//
//  This is primarily to help the theme system. The designer would always
//  otherwise use the design time id. But the theme guy is often updating
//  widgets and templates before they've been added, and we want to minimize
//  the amount of code involved in theme generation as much as possible so
//  having a quick and easy way to remove widgets it's not going to be able
//  to use is important.
//
tCIDLib::TVoid TCQCIntfContainer::RemoveByName(const TString& strName)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colChildren[c4Index]->strWidgetId() == strName)
        {
            m_colChildren.RemoveAt(c4Index);
            break;
        }
    }
}


//
//  Remove the widget at the indicated index position, or the passed
//  widget. In the later, the passed widget will be invalid upon return.
//
tCIDLib::TVoid TCQCIntfContainer::RemoveWidget(const tCIDLib::TCard4 c4At)
{
    TCQCIntfWidget* piwdgRem = m_colChildren.pobjOrphanAt(c4At);
    piwdgRem->DecRefCount();
    if (piwdgRem->c4RefCount())
        piwdgRem->bIsDead(kCIDLib::True);
    else
        delete piwdgRem;
}

tCIDLib::TVoid
TCQCIntfContainer::RemoveWidget(TCQCIntfWidget* const piwdgToRemove)
{
    m_colChildren.OrphanElem(piwdgToRemove);
    piwdgToRemove->DecRefCount();
    if (piwdgToRemove->c4RefCount())
        piwdgToRemove->bIsDead(kCIDLib::True);
    else
        delete piwdgToRemove;
}


//
//  This method will find the widget with the indicated id and replace it with
//  the passed one. This is a designer only thing so it doesn't have to deal
//  with reference counts.
//
tCIDLib::TVoid
TCQCIntfContainer::ReplaceWidget(const  tCIDLib::TCard8 c8DesId
                                , const TCQCIntfWidget& iwdgReplacement)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        if (piwdgCur->c8DesId() == c8DesId)
        {
            // This will delete the old one, since the collection is adopting
            m_colChildren.SetAt
            (
                ::pDupObject<TCQCIntfWidget>(iwdgReplacement)
                , c4Index
            );
            break;
        }
    }
}


//
//  This is to support the undo system. Changing zorder can get messy so it just saves
//  all of the design ids in their original zorder. To undo, it passes that list back to
//  us.
//
tCIDLib::TVoid TCQCIntfContainer::SetAllZOrder(const tCQCIntfEng::TDesIdList& fcolOrder)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();

    // If the size isn't the same as our child list, then obviously bad
    if (fcolOrder.c4ElemCount() != c4Count)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcDbg_BadZOrderList
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(fcolOrder.c4ElemCount())
            , TCardinal(c4Count)
        );
    }

    //
    //  For each of the incoming design ids, look it up in our list. Then move it to
    //  the current index. That should always work out correctly. We can never find one
    //  at a zorder below our current working index.
    //
    tCIDLib::TCard4 c4CurZ;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // This get us the current zorder for use below
        TCQCIntfWidget* piwdgTmp = piwdgFindByDesId(fcolOrder[c4Index], c4CurZ);

        // Shouldn't happen but if not found, skip it
        if (!piwdgTmp)
        {
            CIDAssert2(L"Could not find des id in the current child list")
            continue;
        }

        // If not already in the right position, then move it
        if (c4CurZ != c4Index)
            m_colChildren.MoveItem(c4CurZ, c4Index);
    }
}


//
//  Allows the outside world to update specific runtime flags on all widgets.
//  This is mostly just for the editor's use though it might come in handy
//  at some other point.
//
tCIDLib::TVoid
TCQCIntfContainer::SetGlobalRunFlags(const  tCQCIntfEng::ERunFlags  eBitsToSet
                                    , const tCQCIntfEng::ERunFlags  eMask)
{
    // Set the flags on us first
    eSetRunFlags(eBitsToSet, eMask);

    // Now do our children and recurse if needed
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        piwdgCur->eSetRunFlags(eBitsToSet, eMask);

        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            // If its a container, then recurse
            static_cast<TCQCIntfContainer*>(piwdgCur)->SetGlobalRunFlags
            (
                eBitsToSet, eMask
            );
        }
    }
}


//
//  Get or set the name of the template that this container represents. We
//  are either the main template, in which case it's the name of the loaded
//  top level template, or we are an overlay and it's the name of the template
//  currently loaded into us.
//
//  It will be the full repository path of the template.
//
const TString& TCQCIntfContainer::strTemplateName() const
{
    return m_strTemplateName;
}

const TString& TCQCIntfContainer::strTemplateName(const TString& strToSet)
{
    m_strTemplateName = strToSet;
    return m_strTemplateName;
}


//
//  Calculate the extent of the child widgets, so we just run through them
//  and take the larger of each widget's right corner. We don't have to
//  recurse because we know that the overlay areas will contain any widgets
//  that get loaded into them.
//
TSize TCQCIntfContainer::szMaxChildExtent()
{
    TPoint pntMax(0, 0);

    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        pntMax.TakeLarger(m_colChildren[c4Index]->areaActual().pntLR());

    // Covert the point to a size. We know it will be positive
    return TSize
    (
        tCIDLib::TCard4(pntMax.i4X()), tCIDLib::TCard4(pntMax.i4Y())
    );
}


//
//  This is called when a template is loaded up into an overlay. The template
//  data is streamed into a temp template object, and then the overlay calls
//  this to steal his states and child widgets.
//
//  We also set all the child widgets with our own owner view and set us as their
//  parent, since they now are owned by our view and are under us. We don't want them
//  to access the original. This may turn out to be redundant in some cases since
//  it might be also done during the loading process, but it's safe to do it now.
//
tCIDLib::TVoid
TCQCIntfContainer::TakeFrom(        TCQCIntfContainer&  iwdgSrc
                            , const tCIDLib::TBoolean   bSetParOrg)
{
    // Take his states
    delete m_pistlStates;
    m_pistlStates = iwdgSrc.m_pistlStates;
    iwdgSrc.m_pistlStates = new TCQCIntfStateList;

    // Clean up our children and take all his
    RemoveAllChildren();
    m_colChildren = tCIDLib::ForceMove(iwdgSrc.m_colChildren);

    //
    //  If asked, update all the children to be relative to our origin. Tell them
    //  not to call AreaChanged(), because when the widgets aren't set up yet. We
    //  aslso set our
    //
    TCQCIntfView* const pcivSet = &civOwner();
    const TPoint pntUs = areaActual().pntOrg();
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        piwdgCur->SetOwner(pcivSet);
        piwdgCur->SetParent(this);
        if (bSetParOrg)
            piwdgCur->SetParentOrg(pntUs, kCIDLib::False);
    }
}


//
//  This is called when a popup is about to be closed, and when the main template or
//  popup is about to be reloaded, to give special widgets a chance to clean up
//  first.
//
tCIDLib::TVoid TCQCIntfContainer::TermWidgets()
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        try
        {
            m_colChildren[c4Index]->Terminate();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfContainer: Hidden Constructors and Operators
// ---------------------------------------------------------------------------
TCQCIntfContainer::TCQCIntfContainer(const  tCQCIntfEng::ECapFlags  eCaps
                                    , const TString&                strTypeName
                                    , const tCIDLib::TCard4         c4InitStyle) :

    TCQCIntfWidget(eCaps, strTypeName, c4InitStyle)
    , m_colChildren(tCIDLib::EAdoptOpts::Adopt, 32)
    , m_pistlStates(nullptr)
{
    // Create an empty initial states object
    m_pistlStates = new TCQCIntfStateList;

    CommonInit();
}

TCQCIntfContainer::TCQCIntfContainer(const  TString&                strName
                                    , const TArea&                  areaInit
                                    , const tCQCIntfEng::ECapFlags  eCaps
                                    , const TString&                strTypeName
                                    , const tCIDLib::TCard4         c4InitStyle) :

    TCQCIntfWidget(areaInit, eCaps, strTypeName, c4InitStyle)
    , m_colChildren(tCIDLib::EAdoptOpts::Adopt, 32)
    , m_pistlStates(nullptr)
    , m_strTemplateName(strName)
{
    // Create an empty initial states object
    m_pistlStates = new TCQCIntfStateList;

    CommonInit();
}

TCQCIntfContainer::TCQCIntfContainer(const TCQCIntfContainer& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , m_colChildren(tCIDLib::EAdoptOpts::Adopt, iwdgSrc.m_colChildren.c4ElemCount())
    , m_pistlStates(nullptr)
    , m_strTemplateName(iwdgSrc.m_strTemplateName)
{
    // Copy the states info
    m_pistlStates = new TCQCIntfStateList(*iwdgSrc.m_pistlStates);

    CommonInit();

    //
    //  Use the bulk mode janitor since if we are in designer mode this is going
    //  to publish change msgs and we just want a single one at the end.
    //
    TColBlockModeJan janLoad(&m_colChildren, kCIDLib::True);

    // And copy duplicate the widgets
    const tCIDLib::TCard4 c4Count = iwdgSrc.m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgNew = ::pDupObject<TCQCIntfWidget>
        (
            *iwdgSrc.m_colChildren[c4Index]
        );

        // Set us as the new guy's parent and set the ref count to 1
        piwdgNew->SetParent(this);
        piwdgNew->IncRefCount();
        m_colChildren.Add(piwdgNew);
    }
}

TCQCIntfContainer& TCQCIntfContainer::operator=(const TCQCIntfContainer& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TCQCIntfWidget::operator=(iwdgSrc);

        *m_pistlStates      = *iwdgSrc.m_pistlStates;
        m_strTemplateName   = iwdgSrc.m_strTemplateName;

        // Flush our child list and dup the source's list
        RemoveAllChildren();

        //
        //  Use the bulk mode janitor since if we are in designer mode this is going
        //  to publish change msgs and we just want a single one at the end.
        //
        TColBlockModeJan janLoad(&m_colChildren, kCIDLib::True);

        const tCIDLib::TCard4 c4Count = iwdgSrc.m_colChildren.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCIntfWidget* piwdgNew = ::pDupObject<TCQCIntfWidget>
            (
                *iwdgSrc.m_colChildren[c4Index]
            );

            // Set us as the new guy's parent and set the ref count to 1
            piwdgNew->SetParent(this);
            piwdgNew->IncRefCount();

            m_colChildren.Add(piwdgNew);
        }
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfContainer: Protected, virtual methods
// ---------------------------------------------------------------------------

//
//  If not overridden, just copy from the new point that is to be applied to
//  the adjusted area.
//
tCIDLib::TVoid
TCQCIntfContainer::AdjustChildPos(const TPoint& pntNew, TPoint& pntAdj)
{
    pntAdj = pntNew;
}


// ---------------------------------------------------------------------------
//  TCQCIntfContainer: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  This method is called periodically from the user interface window that
//  owns us, via a timer. Our job is to iterate all the widgets and for
//  any that asked for active updates, call them so that they have a chance
//  to update.
//
//  The caller can ask us not to redraw. We pass this NoRedraw flag on to
//  the widgets, to ask them not to redraw based on the new value. This is
//  usually done during start up, to supress the initial redrawing until
//  everyone has been set up and has data, then we can draw it all at once.
//
//  The overlay class marks himself as wanting active updates, which insures
//  that we correctly recurse into overlays.
//
//  We skip any that are marked as in a gesture, which would only ever be one
//  at a time. This one we don't want to update until the gesture is over.
//
tCIDLib::TVoid
TCQCIntfContainer::ActiveUpdate(const   tCIDLib::TBoolean   bNoRedraw
                                , const TGUIRegion&         grgnClip
                                , const tCIDLib::TBoolean   bInTopLayer)
{
    // If we have no children, then nothing to do
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    if (!c4Count)
        return;

    TThread* pthrCaller = TThread::pthrCaller();
    TGUIRegion grgnCur;
    TArea      areaUpdate;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (pthrCaller->bCheckShutdownRequest())
            break;

        TCQCIntfWidget& iwdgCur = *m_colChildren[c4Index];
        if (iwdgCur.bWantsActiveUpdate() && !iwdgCur.bInGesture())
        {
            try
            {
                //
                //  Get this guy's clip region. We get its most restricted clip
                //  region which takes into account the fact that his may be an overlay
                //  and scrolled. It may not be visible. We combine the visibility
                //  flag with the no-redraw flag we got.
                //
                //  We could just not call him, but he may need to update information
                //  on each round.
                //
                const tCIDLib::TBoolean bVisible(iwdgCur.bFindMostRestrictiveClip(grgnCur, 0, 0));
                iwdgCur.ActiveUpdate(bNoRedraw || !bVisible, grgnCur, bInTopLayer);
            }

            catch(const TError& errToCatch)
            {
                if (facCQCIntfEng().bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);
            }
        }
    }
}


//
//  We recursively pass along area changes to our child widgets, to let
//  them update for a change in our area.
//
//  We give the derived container class a chance to adjust the point before
//  we set it on the child. This lets, for instance, the overlay class lie
//  to its child widgets to allow its contents to be scrolled.
//
tCIDLib::TVoid
TCQCIntfContainer::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    // Update all our children's parent origin
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    const TPoint& pntNew = areaNew.pntOrg();
    TPoint pntAdj;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        try
        {
            AdjustChildPos(pntNew, pntAdj);
            piwdgCur->SetParentOrg(pntAdj);
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().eLogThreshold() <= tCIDLib::ESeverities::Warn)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }
}


// We call ParentDisplayStateChanged() on all our children
tCIDLib::TVoid TCQCIntfContainer::DisplayStateChanged()
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        try
        {
            piwdgCur->ParentDisplayStateChanged(eCurDisplayState());
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().eLogThreshold() <= tCIDLib::ESeverities::Warn)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }
}


//
//  We override this and just pass it on to our children. This is primarily for those
//  widgets that maintain an external helper that they position over themselves (e.g. web
//  cam and web browser widgets.)
//
tCIDLib::TVoid
TCQCIntfContainer::ScrPosChanged(const TArea& areaNew)
{
    // Update all our children's parent origin
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];
        try
        {
            piwdgCur->ScrPosChanged(areaNew);
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().eLogThreshold() <= tCIDLib::ESeverities::Warn)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }
}



//
//  We stream in our contents and that of any of the child widgets we contain.
//  At the persistent level, there are nested containers, so we only have to
//  streamin immediate children, no recursion.
//
tCIDLib::TVoid TCQCIntfContainer::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Get rid of any existing children
    RemoveAllChildren();

    // Our stuff must start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version for this level
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion > CQCIntfEng_Widget::c2ContFmtVersion)
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

    // Call our parent class
    TParent::StreamFrom(strmToReadFrom);

    //
    //  If before V3, we have to read in and discard the template name.
    //  This is no longer an embedded thing, it's a run time thing driven
    //  by the path from which the file was loaded.
    //
    if (c2FmtVersion < 3)
    {
        TString strTmp;
        strmToReadFrom >> strTmp;
    }

    // Now get the child count and its XORd version and check them
    tCIDLib::TCard4 c4Count;
    tCIDLib::TCard4 c4XORCount;
    strmToReadFrom >> c4Count >> c4XORCount;
    if (c4Count != (c4XORCount ^ kCIDLib::c4MaxCard))
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcTmpl_BadChildCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strTemplateName
        );
    }

    //
    //  Use the bulk mode janitor since if we are in designer mode this is going
    //  to publish change msgs and we just want a single one at the end.
    //
    TColBlockModeJan janLoad(&m_colChildren, kCIDLib::True);

    //
    //  Ok, lets get that many children out. We have to stream them in
    //  polymorphically. We do our own polymorphic streaming here so that
    //  we can have more control to deal with refactoring.
    //
    const TArea& areaOurRel = areaRelative();
    TArea areaFix;
    TClass clsType;
    TCQCIntfWidget* piwdgCur = nullptr;
    tCIDLib::TCard4 c4CurInd;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // We should have an index number first, which matches our current ind
        strmToReadFrom >> c4CurInd;
        if (c4CurInd != c4Index)
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcTmpl_BadChildIndex
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , m_strTemplateName
                , TCardinal(c4CurInd)
                , TCardinal(c4Index)
            );
        }

        // Next should be a class type object. Read it in.
        clsType = strmToReadFrom.clsReadClassInfo();

        //
        //  Let this object read itself in. Deal with any refactoring of
        //  classes.
        //
        try
        {
            if (clsType == TCQCIntfWebImg::clsThis())
            {
                //
                //  There was a refactoring here. So we read in the old class,
                //  which is just there in stripped down form, enough to read
                //  itself in. We then create the new class and pass it along
                //  the data.
                //
                TCQCIntfWebImg iwdgTmp;
                strmToReadFrom >> iwdgTmp;
                piwdgCur = new TCQCIntfStaticWebImg(iwdgTmp);
            }
             else
            {
                //
                //  Nothing special, so ask the class type to make a new object
                //  of that type, and cast it to the base widget type. Then
                //  we can let it stream itself in. Check that it really is
                //  something derived from the widget class just to be sure.
                //
                TObject* pobjNew = clsType.pobjMakeNew();
                TBinInStream::CheckRelationship(pobjNew, TCQCIntfWidget::clsThis());

                piwdgCur = static_cast<TCQCIntfWidget*>(pobjNew);
                strmToReadFrom >> *piwdgCur;
            }

            //
            //  Do a sanity check here. If a widget manages to get outside of its
            //  container, weirdness occurs. So check it and force it back in if it
            //  has managed (through probably some past error) to drift. We force it
            //  directly into the member because we aren't in a position to deal
            //  with the normal process that occurs when a widget's position changes.
            //
            if (!facCQCIntfEng().bIsValidWdgArea(areaOurRel, piwdgCur->areaRelative(), areaFix))
            {
                // Set both actual and relative which are the same at this point
                piwdgCur->m_areaActual = areaFix;
                piwdgCur->m_areaRelative = areaFix;

                if (facCQCIntfEng().bLogWarnings())
                {
                    facCQCIntfEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcTmpl_ForcedChildPos
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::Format
                    );
                }
            }
        }

        catch(...)
        {
            delete piwdgCur;
            throw;
        }

        piwdgCur->SetParent(this);
        piwdgCur->IncRefCount();
        m_colChildren.Add(piwdgCur);
    }

    // Stream in our states;
    strmToReadFrom  >> *m_pistlStates;

    //
    //  If version 1, then read in the old xlats stuff. The derived class
    //  if it's a template, is going to grab these and convert them to an
    //  action based thing on itself. So we don't write the back out again.
    //
    if (c2FmtVersion < 2)
        strmToReadFrom >> m_colFXlats;

    // And our stuff must end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfContainer::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_Widget::c2ContFmtVersion;

    // Call our parent class
    TParent::StreamTo(strmToWriteTo);

    //
    //  Write out the count of elements we are going to stream, and
    //  the XORed version of it, for safety.
    //
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    strmToWriteTo << c4Count << tCIDLib::TCard4(c4Count ^ kCIDLib::c4MaxCard);

    //
    //  And iterate the children and stream them each out, with the index
    //  value preceding it, again for safety so we can be sure we are reading
    //  back in good data later.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        //
        //  Write out this index, then do our own polymorphic streaming. We
        //  stream out the class type, then the contents of the object.
        //
        strmToWriteTo << c4Index;
        strmToWriteTo.WriteClassInfo(piwdgCur->clsIsA());
        strmToWriteTo << *piwdgCur;
    }

    // Stream out our own data and the end marker
    strmToWriteTo   << *m_pistlStates
                    << tCIDLib::EStreamMarkers::EndObject;
}


//
//  This is from the periodic timer of the owning IV window, to give all
//  widgets who want it a chance to update their current value. We just
//  recurse on any children marked as wanting such updates.
//
//  The overlay class marks himself as wanting value scans. He doesn't
//  have any associated fields, but this makes sure that he gets called
//  for correct recursion here.
//
//  We also re-evaluate all states defined at this level.
//
//  The caller tells us whether the widgets can actually redraw or not.
//  Individual widgets are expected to correctly honor their visibility
//  states and not redraw regardless if they are hidden.
//
//  If one is marked as in a gesture, we don't process it. We don't want any
//  updates to interfere with the gesture processing.
//
tCIDLib::TVoid
TCQCIntfContainer::ValueUpdate(         TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean   bVarsUpdate
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    EvaluateStates(polleToUse, bNoRedraw, ctarGlobalVars);

    // If we have no children, then nothing to do
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    if (!c4Count)
        return;

    TThread* pthrCaller = TThread::pthrCaller();
    TGUIRegion grgnCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (pthrCaller->bCheckShutdownRequest())
            break;

        TCQCIntfWidget& iwdgCur = *m_colChildren[c4Index];

        if (iwdgCur.bWantsValueUpdate() && !iwdgCur.bInGesture())
        {
            try
            {
                //
                //  Get this guy's clip region. We get its most restricted clip region
                //  which takes into account the fact that his may be an overlay and
                //  scrolled. It may not be visible.
                //
                //  We still need to call the widget since it has to store this info
                //  in most cases. But we combine the visibility flag with the passed in
                //  no redraw to prevent any actual redraw if it's not visible.
                //
                const tCIDLib::TBoolean bVisible(iwdgCur.bFindMostRestrictiveClip(grgnCur, 0, 0));
                iwdgCur.ValueUpdate
                (
                    polleToUse
                    , bNoRedraw || !bVisible
                    , bVarsUpdate
                    , ctarGlobalVars
                    , grgnCur
                );
            }

            catch(const TError& errToCatch)
            {
                if (facCQCIntfEng().bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfContainer: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called with a list of triggered events and some info that we need
//  to process them. We run through them and process their filters. If any
//  pass, we invoke them.
//
//  We provide a public, virtual method, InvokeTrgEvents() that will recurse
//  throgh all the containers and invoke this at any level where triggered
//  events are loaded.
//
//  We MASSIVELY simplify things for ourself by using a standard action
//  engine, which just runs the action in our thread context directly, and
//  will directly call back to even GUI targets.
//
//  This is OK because we heavily restrict the operations that can be done
//  within a triggered event in the GUI. There can't be any loading of new
//  overlays or popups or any of that complex stuff.
//
//  We return True if we ran any events, else False.
//
tCIDLib::TBoolean
TCQCIntfContainer::bRunTrgEvents(const  tCQCEvCl::TTrgEvList&   colEvents
                                , const tCIDLib::TBoolean       bIsNight
                                , const tCQCKit::TCQCEvPtr&     cptrEv
                                , const tCIDLib::TCard4         c4Hour
                                , const tCIDLib::TCard4         c4Minute
                                ,       TTrigEvCmdTarget&       ctarEvent)
{
    // Get his list count of events and loop through them
    tCIDLib::TBoolean bAnyRun = kCIDLib::False;
    const tCIDLib::TCard4 c4EvCnt = colEvents.c4ElemCount();
    TCQCIntfView& civPar = civOwner();
    for (tCIDLib::TCard4 c4EvInd = 0; c4EvInd < c4EvCnt; c4EvInd++)
    {
        const TCQCTrgEvent& csrcCur = colEvents[c4EvInd];
        try
        {
            //
            //  Evaluate the filters on this one to see if it
            //  matches our event. If so, then invoke it.
            //
            const tCIDLib::TBoolean bFiltRes = csrcCur.bEvaluate
            (
                *cptrEv.pobjData(), bIsNight, TTime::enctNow(), c4Hour, c4Minute
            );

            if (bFiltRes)
            {
                //
                //  Looks ok. Build up a list of non-standard targets, which
                //  will be all our children that implement the command target
                //  mixin. Add our view as well, and ourself.
                //
                tCQCKit::TCmdTarList colOtherTars(tCIDLib::EAdoptOpts::NoAdopt);
                FindAllTargets(colOtherTars, kCIDLib::True);
                colOtherTars.Add(&civPar);
                colOtherTars.Add(this);

                //
                //  We know it's a triggered event, so set up a triggered event target.
                //  We give it the incoming counted pointer to the event that triggered
                //  us.
                //
                // <TBD> We already get one of these passed to us, and we aren't using it?
                TTrigEvCmdTarget ctarEvent;
                ctarEvent.SetEvent(cptrEv);
                colOtherTars.Add(&ctarEvent);

                //
                //  Create a wrapper around the triggered event which
                //  will provide the correct runtime value object when the
                //  action engine asks for it. This is what we pass as the
                //  action source.
                //
                TCQCTrgEventEx csrcActual(csrcCur, cptrEv);

                // Now create the engine and run it
                bAnyRun = kCIDLib::True;
                TCQCStdActEngine actePreload(civPar.cuctxToUse());
                const tCQCKit::ECmdRes eRes = actePreload.eInvokeOps
                (
                    csrcActual
                    , kCQCKit::strEvId_OnTrigger
                    , civPar.ctarGlobalVars()
                    , colOtherTars
                    , civPar.pmcmdtTrace()
                    , TString::strEmpty()
                );

                if (eRes > tCQCKit::ECmdRes::Exit)
                {
                    if (facCQCIntfEng().bLogFailures())
                    {
                        facCQCKit().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , TString(L"IV based triggered event failed")
                            , csrcCur.strTitle()
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::AppStatus
                        );
                    }
                }
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
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitMsgs::midStatus_ExceptInEval
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , csrcCur.strTitle()
                );
            }
        }

        catch(...)
        {
            if (facCQCIntfEng().bLogFailures())
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitMsgs::midStatus_ExceptInEval
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , csrcCur.strTitle()
                );
            }
        }
    }
    return bAnyRun;
}


// Finds all the widget that have specific capabilities flags
tCIDLib::TVoid TCQCIntfContainer::
QueryAllWithCapFlags(const  tCQCIntfEng::ECapFlags     eToFind
                    , const tCQCIntfEng::ECapFlags     eMask
                    ,       tCQCIntfEng::TChildList&   colToFill)
{
    // If in debug mode, make sure the collection does not adopt
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"The widget list cannot adopt"
    );

    // Test us first
    const tCIDLib::TCard4 c4Mask = tCIDLib::TCard4(eMask);
    const tCIDLib::TCard4 c4ToFind = tCIDLib::TCard4(eToFind);
    tCIDLib::TCard4 c4CurFlags = tCIDLib::TCard4(eCapFlags());

    //
    //  Turn off all non-relevant bits in the current flags. If that now
    //  matches the to find bit set, then we matched.
    //
    if ((c4CurFlags & c4Mask) == c4ToFind)
        colToFill.Add(this);

    //
    //  It wasn't us, so iterate our children and check them, recursing if
    //  we hit a container. If we have no children, just return null.
    //
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        // Check this guy and add if a match
        if ((tCIDLib::TCard4(piwdgCur->eCapFlags()) & c4Mask) == c4ToFind)
            colToFill.Add(piwdgCur);

        // See if he's a container and recurse if so
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            TCQCIntfContainer* piwdgCont
            (
                static_cast<TCQCIntfContainer*>(piwdgCur)
            );

            piwdgCont->QueryAllWithCapFlags(eToFind, eMask, colToFill);
        }
    }
}


//
//  Upon either loading a top level template or popup, or loading a template into
//  an overlay, this is called on that top level template, or on the overlay itself
//  after the newly loaded template's contents have been copied over to the overlay
//  itself.
//
//  Note that this process will discard any overlay buttons or overlays with initial
//  loads that could cause a recursive reference.
//
//  It will also initialize states at each level, and associate widgets with the
//  states they reference.
//
//  And it will set the passed owner view as the owner of all of the widgets as it
//  goes. This gives them a back-link to their view.
//
//  If in designer mode, it won't actually recurse, it'll just do our immediate
//  children since there is no recursion at design time.
//
//
//  At the end, all of the widgets are in memory and ready to be initialized.
//
tCIDLib::TVoid
TCQCIntfContainer::RecursiveLoad(       TCQCIntfView* const     pcivOwner
                                ,       TDataSrvClient&         dsclLoad
                                , const TCQCFldCache&           cfcData)
{
    //
    //  Set the owner window on ourself. This will be reundant in that we
    //  might have already set it as a child of a previous level. But this
    //  prevents the top level guy from having to separately set it on himself
    //  before calling here. Otherwise, the initial incoming container wouldn't
    //  get set.
    //
    SetOwner(pcivOwner);

    // And loop through our children and do any other recursive loads.
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget& iwdgCur = *m_colChildren[c4Index];

        // Set the owner on this one
        iwdgCur.SetOwner(pcivOwner);

        //
        //  It this guy is an overlay, and is configured to initially load,
        //  and we aren't in designer mode, then do it.
        //
        if (iwdgCur.bIsDescendantOf(TCQCIntfOverlay::clsThis())
        &&  !TFacCQCIntfEng::bDesMode())
        {
            //
            //  Cast it to its real time, and call the other version, which
            //  will do the load and call us back here to continue the
            //  work.
            //
            TCQCIntfOverlay& iwdgOver = static_cast<TCQCIntfOverlay&>(iwdgCur);
            if (!iwdgOver.strInitTemplate().bIsEmpty())
            {
                //
                //  If it fails, that's fine. The last parm says not to throw
                //  but to tell the user it failed. We'll just not go any
                //  further in the load on this branch and this overlay will
                //  not have any initial contents, or might only have partial
                //  contents.
                //
                //  This method will expand the path if required, so we can just
                //  pass what we have.
                //
                iwdgOver.bLoadNew
                (
                    pcivOwner
                    , dsclLoad
                    , cfcData
                    , iwdgOver.strInitTemplate()
                    , kCIDLib::False
                );
            }
        }
    }
}


//
//  Loop through all our children and delete them. We do it this way
//  so that if any of them were to throw, we wouldn't fail to destroy
//  the subsequent ones.
//
//  If the ref count is still non-zero after we decrement, then a janitor
//  has the widget so we don't delete it, but we mark it dead.
//
tCIDLib::TVoid TCQCIntfContainer::RemoveAllChildren()
{
    //
    //  Use the bulk mode janitor since if we are in designer mode this is going
    //  to publish change msgs and we just want a single one at the end.
    //
    TColBlockModeJan janLoad(&m_colChildren, kCIDLib::False);

    tCIDLib::TInt4 i4Count = tCIDLib::TInt4(m_colChildren.c4ElemCount());
    i4Count--;
    while (i4Count >= 0)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren.pobjOrphanAt(i4Count);
        piwdgCur->DecRefCount();

        try
        {
            //
            //  If we were the last reference, then destroy it. Else,
            //  mark at as dead.
            //
            if (piwdgCur->c4RefCount())
                piwdgCur->bIsDead(kCIDLib::True);
            else
                delete piwdgCur;
        }

        catch(const TError& errToCatch)
        {
            if (facCQCIntfEng().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
            // <TBD> Log something
        }
        i4Count--;
    }
}


//
//  This guy will iterate all our children. If any of them are overlays that
//  are set up to initially load any template that is currently in our
//  parent chain, then we will remove that widget, since that would cause
//  a recursive freakout.
//
//  <TBD> Maybe we should just create a local hash set, pre-scan the parent
//  chain and get the list of loaded templates up the chain? Then we could
//  do a very fast recursion check by just seeing if a linked template is
//  in the hash set, instead of re-interating the parent chain each time.
//
tCIDLib::TVoid TCQCIntfContainer::RemoveRecursiveLinks()
{
    tCIDLib::TCard4     c4Count = m_colChildren.c4ElemCount();
    tCIDLib::TCard4     c4Index = 0;
    TString             strToCheck;
    TString             strToLoad;
    TCQCIntfOverlay*    piwdgOver;

    while (c4Index < c4Count)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        // See if this guy is an overlay
        piwdgOver = dynamic_cast<TCQCIntfOverlay*>(piwdgCur);

        if (piwdgOver)
        {
            // Get the initial template and expand the path
            ExpandTmplPath(piwdgOver->strInitTemplate(), strToLoad);

            // If it never got set, then don't bother
            TCQCIntfContainer* piwdgCont = this;
            if (!strToLoad.bIsEmpty())
            {
                //
                //  Ok, we have to check. So starting with us, run up our
                //  parent chain and see if this template is in there.
                //
                TCQCIntfTemplate*  piwdgTmpl;
                TCQCIntfOverlay*   piwdgOvrly;
                while (piwdgCont)
                {
                    // It's either an overlay or the parent template
                    if (piwdgCont->clsIsA() == TCQCIntfTemplate::clsThis())
                    {
                        piwdgTmpl = static_cast<TCQCIntfTemplate*>(piwdgCont);
                        if (piwdgTmpl->strTemplateName() == strToLoad)
                            break;
                    }
                     else
                    {
                        piwdgOvrly = static_cast<TCQCIntfOverlay*>(piwdgCont);
                        if (piwdgOvrly->strTemplateName() == strToLoad)
                            break;
                    }

                    piwdgCont = piwdgCont->piwdgParent();
                }

                if (piwdgCont)
                {
                    //
                    //  We hit one, so remove this guy and dec the count.
                    //  This happens during the loading of a template so
                    //  no actions could be involved and so the ref count
                    //  could never be > 1. So we can safely just remove.
                    //
                    m_colChildren.RemoveAt(c4Index);
                    c4Count--;
                }
                 else
                {
                    // We didn't remove it, so move forward
                    c4Index++;

                    // If it's an overlay, then recurse
                    if (piwdgOver)
                        piwdgOver->RemoveRecursiveLinks();
                }
            }
             else
            {
                c4Index++;
            }
        }
         else
        {
            c4Index++;
        }
    }
}


//
//  A helper that validates the commands in a command source. Various widgets can call
//  this from their validation method if they provide action commands. We automatically
//  handle any that implement the command source mixin, and call this method on their
//  behalf, but some widgets have other command sources as well.
//
tCIDLib::TVoid
TCQCIntfContainer::ValidateCmds(const   TString&                strActType
                                , const TCQCIntfWidget&         iwdgSrc
                                , const MCQCCmdSrcIntf&         mcsrcVal
                                ,       tCQCIntfEng::TErrList&  colErrs) const
{
    // Loop through all of the events of the source
    TString strTarName;
    const tCIDLib::TCard4 c4EvCount = mcsrcVal.c4EventCount();
    for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EvCount; c4EvIndex++)
    {
        const MCQCCmdSrcIntf::TOpcodeBlock& colOps = mcsrcVal.colOpsAt(c4EvIndex);
        const tCIDLib::TCard4 c4OpCount = colOps.c4ElemCount();

        // Loop through all of the opcodes of this event
        for (tCIDLib::TCard4 c4OpIndex = 0; c4OpIndex < c4OpCount; c4OpIndex++)
        {
            const TActOpcode& aocCur = colOps[c4OpIndex];

            // If disabled or a comment, ignore this one
            if (aocCur.m_bDisabled || (aocCur.m_eOpcode == tCQCKit::EActOps::Comment))
                continue;

            // Otherwise let's check it
            const TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;

            // Get the target id. They are all prefix:name type strings
            strTarName = ccfgCur.strTargetId();

            // If it's a command type that has a target, let's check it
            if (!strTarName.bIsEmpty())
            {
                tCIDLib::TBoolean bFound = kCIDLib::True;
                if (strTarName.bStartsWith(kCQCKit::strTarPref_Widgets))
                {
                    //
                    //  It's got to a widget. So we need to remove the widget target
                    //  prefix and the remainder should be the widget id.
                    //
                    strTarName.Cut(0, kCQCKit::strTarPref_Widgets.c4Length());
                    bFound = piwdgFindByName(strTarName, kCIDLib::False) != nullptr;
                }
                 else if ((strTarName == kCQCKit::strCTarId_Fields)
                      ||  (strTarName == kCQCKit::strCTarId_System)
                      ||  (strTarName == kCQCKit::strCTarId_GVars)
                      ||  (strTarName == kCQCKit::strCTarId_LVars)
                      ||  (strTarName == kCQCKit::strCTarId_TrigEvent)
                      ||  (strTarName == kCQCKit::strCTarId_EventSrv)
                      ||  (strTarName == kCQCKit::strCTarId_MacroEngine)
                      ||  (strTarName == kCQCIntfEng::strCTarId_IntfViewer))
                {
                    //
                    //  It's a standard target. Cut off the prefix part to leave a human
                    //  readable value.
                    //
                    strTarName.CutUpTo(kCIDLib::chColon);
                }
                 else
                {
                    // Never found it
                    bFound = kCIDLib::False;
                }

                if (!bFound)
                {
                    // It's not found, so add an error
                    TString strEvName = mcsrcVal.strEventIdAt(c4EvIndex);
                    strEvName.CutUpTo(kCIDLib::chColon);

                    TString strErr = facCQCIntfEng().strMsg
                    (
                        kIEngErrs::errcVal_CmdTarNotFound
                        , strTarName
                        , strEvName
                        , strActType
                    );

                    colErrs.objPlace
                    (
                        iwdgSrc.c4UniqueId()
                        , strErr
                        , kCIDLib::False
                        , iwdgSrc.strWidgetId()
                    );
                }
            }
        }
    }

}


// ---------------------------------------------------------------------------
//  TCQCIntfContainer: Private, non-virtual methods
// ---------------------------------------------------------------------------


// Some common init since we have multiple ctors
tCIDLib::TVoid TCQCIntfContainer::CommonInit()
{
    // If in designer mode, set up our publishing
    if (TFacCQCIntfEng::bDesMode())
    {
        TString strTopicId(L"/CQC/IntfEng/TemplateChanges/");
        strTopicId.AppendFormatted(facCIDLib().c4NextId());
        m_colChildren.EnablePubSub(strTopicId);
    }
}


//
//  This is called to get a container to evaluate it's states, and then to
//  update any of it's widgets that would be affected by any state changes.
//  We recursive when we see a container child. This is called when the
//  timer that watches for change notifications from the polling engine
//  sees a change that could affect the state evaluations.
//
tCIDLib::TVoid
TCQCIntfContainer::EvaluateStates(          TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        ctarGlobalVars)
{
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    if (!c4Count)
        return;

    //
    //  Ask the states object for our level to evaluate it's states based
    //  on curernt field values. If he tells us that any states changed, then
    //  we need to run through all the widgets and give them a chance to
    //  update based on state info.
    //
    const tCIDLib::TBoolean bNewStates = m_pistlStates->bEvaluate(polleToUse, ctarGlobalVars);
    if (!bNewStates)
        return;

    // Something changed, so let's update
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_colChildren[c4Index];

        //
        //  In this case, we both recurse and apply, because we want to be able to
        //  hide overlays as well. So we do any children, then we update the state.
        //
        if (piwdgCur->bIsDescendantOf(TCQCIntfContainer::clsThis()))
        {
            static_cast<TCQCIntfContainer*>(piwdgCur)->EvaluateStates
            (
                polleToUse, bNoRedraw, ctarGlobalVars
            );
        }

        if (piwdgCur->bUpdateDisplayState(*m_pistlStates))
        {
            //
            //  Force everything under this guy to redraw. Note that we cannot
            //  just call DrawUnder() because we could be below the top level
            //  template here, and so we woudln't draw anything in overlaying
            //  popups.
            //
            //  We also need to let the widget know that the display state has
            //  changed, since it might manage a real window and will need to
            //  hide/show it.
            //
            if (!bNoRedraw)
            {
                piwdgCur->DisplayStateChanged();
                piwdgCur->Invalidate();
            }
        }
    }
}


// Some out of line throwing methods called from templatized methods
tCIDLib::TVoid
TCQCIntfContainer::ThrowCastErr(const   TString&    strName
                                , const TString&    strClassName)
{
    facCQCIntfEng().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIEngErrs::errcDbg_BadWidgetCast
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::TypeMatch
        , strName
        , strClassName
    );
}


tCIDLib::TVoid TCQCIntfContainer::ThrowNotFound(const TString& strName)
{
    facCQCIntfEng().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIEngErrs::errcTmpl_WidgetNotFound
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strName
    );
}

