//
// FILE NAME: CQCIntfEd_EdView.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/02/2015
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
//  This file implements the TCQCIntfEdView class, which provides a derivative
//  of the interface engine's view class, specialized for our editing needs.
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
#include    "CQCIntfEd_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfEdView,TCQCIntfView)




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfEdView
//  PREFIX: civ
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfEdView::TCQCIntfEdView(         TIntfEditWnd* const pwndOwner
                                , const TCQCUserCtx&        cuctxToUse) :

    TCQCIntfView
    (
        facCQCIntfEd().strMsg(kIEdMsgs::midWnd_AppName), pwndOwner, cuctxToUse
    )
    , m_pgdevCompat(nullptr)
    , m_pwndOwner(pwndOwner)
{
    // Set ourself as the owning view of the template we are editing
    m_iwdgTemplate.SetOwner(this);

    // Disable caching
    bEnableTmplCaching(kCIDLib::False);
}

TCQCIntfEdView::~TCQCIntfEdView()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfEdView: Public, inherited methods
// ---------------------------------------------------------------------------

// Return the full possible area of the display 'device'
TArea TCQCIntfEdView::areaDisplayRes() const
{
    return TArea(TPoint::pntOrigin, TGUIFacility::szPrimaryMonitor());
}


//
//  Return the current area of the actual view area, which for us is the
//  area of the window, at the origin.
//
TArea TCQCIntfEdView::areaView() const
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    return m_pwndOwner->areaWndSize();
}


tCIDLib::TVoid
TCQCIntfEdView::AsyncCmdCallback(const tCQCIntfEng::EAsyncCmds)
{
    // Not used at design time
}


tCIDLib::TVoid TCQCIntfEdView::AsyncDataCallback(TCQCIntfADCB* const)
{
    // Not used at design time
}


tCIDLib::TBoolean
TCQCIntfEdView::bCancelTracking(TCQCIntfContainer&, const tCIDLib::TBoolean)
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfEdView::bActiveAction() const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCIntfEdView::bCreatePopUp(const  TString&
                            , const tCIDLib::TBoolean
                            , const tCIDLib::EDirs
                            , const TPoint&
                            , const TString&
                            , const tCIDLib::TBoolean
                            , const TString&)
{
    // Not used at design time
    return kCIDLib::False;
}


tCIDLib::TBoolean TCQCIntfEdView::bHasPopups() const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCIntfEdView::bInitialize(TDataSrvClient&, tCQCIntfEng::TErrList&)
{

    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfEdView::bIsTracking() const
{
    // We do use this one at design time, so just say not tracking
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCIntfEdView::bLoadOverlay(const  TString&
                            , const TString&
                            ,       tCQCIntfEng::TErrList&)
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCIntfEdView::bLoadNewTemplate(       TDataSrvClient&
                                , const TCQCIntfTemplate&
                                ,       tCQCIntfEng::TErrList&)
{
    return kCIDLib::False;
}


tCIDLib::TCard4 TCQCIntfEdView::c4StackSize() const
{
    return 0;
}


tCQCIntfEng::TGraphIntfDev TCQCIntfEdView::cptrNewGraphDev() const
{
    return tCQCIntfEng::TGraphIntfDev(new TGraphWndDev(*m_pwndOwner));
}


tCIDLib::TVoid TCQCIntfEdView::Destroyed()
{
    // We don't use this one at edit time
}


tCIDLib::TVoid TCQCIntfEdView::DoActEvent(tCQCIntfEng::TIntfCmdEv&)
{
    // We don't use this one at edit time
}


tCIDLib::TVoid TCQCIntfEdView::DoActiveUpdatePass()
{
    // We don't use this one at edit time
}


tCIDLib::TVoid TCQCIntfEdView::DoEventProcessingPass()
{
    // We don't use this one at edit time
}


tCIDLib::TVoid
TCQCIntfEdView::DoModalLoop(tCIDLib::TBoolean&, const tCIDLib::TBoolean)
{
}
    // We don't use this one at edit time


tCIDLib::TVoid TCQCIntfEdView::DoUpdatePass()
{
    // We don't use this one at edit time
}


tCIDLib::TVoid
TCQCIntfEdView::DrawUnder(          TGraphDrawDev&
                            , const TArea&
                            , const tCIDLib::TBoolean)
{
    // We don't use this one at edit time
}


tCQCKit::ECmdRes
TCQCIntfEdView::eDoCmd( const   TCQCCmdCfg&
                        , const tCIDLib::TCard4
                        , const TString&
                        , const TString&
                        ,       TStdVarsTar&
                        ,       tCIDLib::TBoolean&
                        ,       TCQCActEngine&)
{
    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid TCQCIntfEdView::ExitCurrentProcessingLoop()
{
    // We don't use this one at edit time
}


tCIDLib::TVoid TCQCIntfEdView::ForceRedraw()
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    m_pwndOwner->ForceRepaint();
}


const TGraphDrawDev& TCQCIntfEdView::gdevCompat() const
{
    // Fault it in upon first use
    if (!m_pgdevCompat)
    {
        CIDAssert
        (
            m_pwndOwner && m_pwndOwner->bIsValid()
            , L"The owner window has not been created yet"
        );
        m_pgdevCompat = new TGraphWndDev(*m_pwndOwner);
    }

    return *m_pgdevCompat;
}


// These all just return our single template that we are editing
const TCQCIntfTemplate& TCQCIntfEdView::iwdgTopMost() const
{
    return m_iwdgTemplate;
}

TCQCIntfTemplate& TCQCIntfEdView::iwdgTopMost()
{
    return m_iwdgTemplate;
}

const TCQCIntfTemplate& TCQCIntfEdView::iwdgBaseTmpl() const
{
    return m_iwdgTemplate;
}

TCQCIntfTemplate& TCQCIntfEdView::iwdgBaseTmpl()
{
    return m_iwdgTemplate;
}


tCIDLib::TVoid TCQCIntfEdView::InvalidateArea(const TArea& areaToRedraw)
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    m_pwndOwner->InvalidateArea(areaToRedraw);
}


MCQCCmdTarIntf*
TCQCIntfEdView::pmctarFind(const TString&, const tCIDLib::TBoolean)
{
    return 0;
}


const TCQCIntfWidget* TCQCIntfEdView::piwdgFindTracking() const
{
    return 0;
}

TCQCIntfWidget* TCQCIntfEdView::piwdgFindTracking()
{
    return 0;
}


TPoint TCQCIntfEdView::pntTouchPos() const
{
    return facCIDCtrls().pntPointerPos(*m_pwndOwner);
}


tCIDLib::TVoid TCQCIntfEdView::PauseTimers(const tCIDLib::TBoolean)
{
    // We don't use this one at edit time
}


tCIDLib::TVoid
TCQCIntfEdView::PlayWAV(const TString&, const tCIDLib::EWaitModes)
{
}


tCIDLib::TVoid
TCQCIntfEdView::QueryCombinedAreas(TArea& areaToFill) const
{
    // Just return the template relative area
    areaToFill = m_iwdgTemplate.areaRelative();
}


//
//  Redraw the whole template area or a specific area or region. Note that,
//  in theory we only need the immediate flag. But, if we are in a composited
//  parent (and we will be in the admin client), it requires children or nothing
//  happens, even though we have no children.
//
tCIDLib::TVoid TCQCIntfEdView::Redraw()
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    m_pwndOwner->Redraw(m_pwndOwner->areaWnd(), tCIDCtrls::ERedrawFlags::ImmedChildren);
}


tCIDLib::TVoid TCQCIntfEdView::Redraw(const TArea& areaToRedraw)
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    m_pwndOwner->Redraw(areaToRedraw, tCIDCtrls::ERedrawFlags::ImmedChildren);
}

tCIDLib::TVoid TCQCIntfEdView::Redraw(const TGUIRegion& grgnToRedraw)
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    m_pwndOwner->Redraw(grgnToRedraw, tCIDCtrls::ERedrawFlags::ImmedChildren);
}


tCIDLib::TVoid TCQCIntfEdView::ScrPosChanged(const TArea& areaTmplWnd)
{
    // A no-op in design mode
}


tCIDLib::TVoid
TCQCIntfEdView::SetVariable(const   TString&    strKey
                            , const TString&    strValue)
{
    // A no-op in design mode
}


tCIDLib::TVoid
TCQCIntfEdView::ShowErr(const   TString&    strTitle
                        , const TString&    strText
                        , const TError&     errToShow)
{
    TExceptDlg dlgShow(*m_pwndOwner, strTitle, strText, errToShow);
}


tCIDLib::TVoid
TCQCIntfEdView::ShowMsg(const   TString&            strTitle
                        , const TString&            strText
                        , const tCIDLib::TBoolean   bIsError
                        , const MFormattable&       mfmtbl1
                        , const MFormattable&       mfmtbl2
                        , const MFormattable&       mfmtbl3)
{
    TString strMsg(strText);
    if (!MFormattable::bIsNullObject(mfmtbl1))
        strMsg.eReplaceToken(mfmtbl1, L'1');
    if (!MFormattable::bIsNullObject(mfmtbl2))
        strMsg.eReplaceToken(mfmtbl2, L'2');
    if (!MFormattable::bIsNullObject(mfmtbl3))
        strMsg.eReplaceToken(mfmtbl3, L'3');

    if (bIsError)
    {
        TErrBox msgbErr(strTitle, strMsg);
        msgbErr.ShowIt(*m_pwndOwner);
    }
     else
    {
        TOkBox msgbOk(strTitle, strMsg);
        msgbOk.ShowIt(*m_pwndOwner);
    }
}


tCIDLib::TVoid TCQCIntfEdView::TakeFocus()
{
    // A no-op for us
}


tCIDLib::TVoid TCQCIntfEdView::Validate()
{
    // A no-op for us
}


TWindow& TCQCIntfEdView::wndOwner()
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    return *m_pwndOwner;
}

const TWindow& TCQCIntfEdView::wndOwner() const
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    return *m_pwndOwner;
}



// ---------------------------------------------------------------------------
//  TCQCIntfEdView: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Some pass-throughs to the template, to make life simpler for the editor
//  window.
//
tCIDLib::TBoolean
TCQCIntfEdView::bFindGroupMembers(  const   tCIDLib::TCard4             c4GroupId
                                    ,       tCQCIntfEng::TDesIdList&    fcolToFill
                                    ,       tCIDLib::TCard4&            c4FirstZ)
{
    return m_iwdgTemplate.bFindGroupMembers(c4GroupId, fcolToFill, c4FirstZ);
}

tCIDLib::TBoolean
TCQCIntfEdView::bFindGroupMembers(  const   tCIDLib::TCard4             c4GroupId
                                    ,       tCQCIntfEng::TChildList&    colToFill
                                    ,       tCIDLib::TCard4&            c4FirstZ)
{
    return m_iwdgTemplate.bFindGroupMembers(c4GroupId, colToFill, c4FirstZ);
}


TCQCIntfWidget*
TCQCIntfEdView::piwdgFindByDesId(const tCIDLib::TCard8 c8ToFind)
{
    return m_iwdgTemplate.piwdgFindByDesId(c8ToFind);
}


const TCQCIntfWidget*
TCQCIntfEdView::piwdgFindByDesId(const tCIDLib::TCard8 c8ToFind) const
{
    return m_iwdgTemplate.piwdgFindByDesId(c8ToFind);
}


TCQCIntfWidget*
TCQCIntfEdView::piwdgFindByDesId(const  tCIDLib::TCard8     c8ToFind
                                ,       tCIDLib::TCard4&    c4ZOrder)
{
    return m_iwdgTemplate.piwdgFindByDesId(c8ToFind, c4ZOrder);
}


const TCQCIntfWidget*
TCQCIntfEdView::piwdgFindByName(const   TString&            strName
                                , const tCIDLib::TBoolean   bRecurse) const
{
    return m_iwdgTemplate.piwdgFindByName(strName, bRecurse);
}


TCQCIntfWidget*
TCQCIntfEdView::piwdgFindByName(const   TString&            strName
                                , const tCIDLib::TBoolean   bRecurse)
{
    return m_iwdgTemplate.piwdgFindByName(strName, bRecurse);
}


// Provide access to the edtior window
TIntfEditWnd* TCQCIntfEdView::pwndEditor()
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    return m_pwndOwner;
}

const TIntfEditWnd* TCQCIntfEdView::pwndEditor() const
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    return m_pwndOwner;
}


// Pass on these to the editor window
const TString& TCQCIntfEdView::strEdUpdateTopic() const
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    return m_pwndOwner->strEdUpdateTopic();
}

const TString& TCQCIntfEdView::strWdgPalTopic() const
{
    CIDAssert(m_pwndOwner != 0, L"Owner window has not been set")
    return m_pwndOwner->strWdgPalTopic();
}
