//
// FILE NAME: CQCIntfEd_Undo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/03/2016
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
//  This file implements our undo framework derivatives.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCIntfEd_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TBaseIntfUndo,TUndoCmd)
RTTIDecls(TIntfAttrUndo, TBaseIntfUndo)
RTTIDecls(TIntfCutUndo,TBaseIntfUndo)
RTTIDecls(TIntfFullUndo,TBaseIntfUndo)
RTTIDecls(TIntfInsertUndo,TBaseIntfUndo)
RTTIDecls(TIntfLockUndo,TBaseIntfUndo)
RTTIDecls(TIntfMultiPosUndo, TBaseIntfUndo)
RTTIDecls(TIntfSelectUndo, TBaseIntfUndo)
RTTIDecls(TIntfZOrderUndo, TBaseIntfUndo)



// ---------------------------------------------------------------------------
//  CLASS: TBaseIntfUndo
// PREFIX: ui
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBaseIntfUndo: Destructor
// ---------------------------------------------------------------------------
TBaseIntfUndo::~TBaseIntfUndo()
{
}


// ---------------------------------------------------------------------------
//  TBaseIntfUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TBaseIntfUndo::bIsSame(const TUndoCmd& uiSrc) const
{
    // For now nothing at this level
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TBaseIntfUndo: Hidden constructors
// ---------------------------------------------------------------------------
TBaseIntfUndo::TBaseIntfUndo()
{
}


// ---------------------------------------------------------------------------
//  TBaseIntfUndo: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TBaseIntfUndo::ThrowIdNotFound( const   tCIDLib::TCard8 c8DesId
                                , const tCIDLib::TCard4 c4Line)
{
    facCQCIntfEd().ThrowErr
    (
        CID_FILE
        , c4Line
        , kIEdErrs::errcDbg_UndoIdNotFound
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , TCardinal64(c8DesId)
    );
}


tCIDLib::TVoid
TBaseIntfUndo::ThrowZNotFound(  const   tCIDLib::TCard4 c4ZOrder
                                , const tCIDLib::TCard4 c4Line)
{
    facCQCIntfEd().ThrowErr
    (
        CID_FILE
        , c4Line
        , kIEdErrs::errcDbg_UndoZNotFound
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , TCardinal(c4ZOrder)
    );
}



// ---------------------------------------------------------------------------
//  CLASS: TIntfAttrUndo
// PREFIX: ui
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfAttrUndo: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfAttrUndo::TIntfAttrUndo(const  tCIDLib::TCard8             c8DesId
                            , const TAttrData&                  adatNew
                            , const TAttrData&                  adatOrg) :

    TBaseIntfUndo()
    , m_adatOrg(adatOrg)
    , m_adatNew(adatNew)
    , m_c8DesId(c8DesId)
{
}

TIntfAttrUndo::~TIntfAttrUndo()
{
}


// ---------------------------------------------------------------------------
//  TIntfAttrUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIntfAttrUndo::bIsSame(const TUndoCmd& uiSrc) const
{
    if (this == &uiSrc)
        return kCIDLib::True;

    if (!TParent::bIsSame(uiSrc))
        return kCIDLib::False;

    const TIntfAttrUndo& uiRealSrc = static_cast<const TIntfAttrUndo&>(uiSrc);
    return
    (
        (m_c8DesId == uiRealSrc.m_c8DesId)
        && (m_adatOrg == uiRealSrc.m_adatOrg)
        && (m_adatNew == uiRealSrc.m_adatNew)
    );
}

tCIDLib::TVoid TIntfAttrUndo::Undo(TAttrEditWnd& wndAttrEd, TCQCIntfTemplate& iwdgTmpl)
{
    // Make sure the target widget exists
    TCQCIntfWidget* piwdgTar = iwdgTmpl.piwdgFindByDesId(m_c8DesId);
    if (!piwdgTar)
        ThrowIdNotFound(m_c8DesId, CID_LINE);

    // Of course we flip the old/new here since we are going backwards
    piwdgTar->SetWdgAttr(wndAttrEd, m_adatOrg, m_adatNew);
}



// ---------------------------------------------------------------------------
//  CLASS: TIntfCutUndo
// PREFIX: ui
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfCutUndo: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfCutUndo::TIntfCutUndo( const   TCQCIntfTemplate&           iwdgSrc
                            , const tCQCIntfEng::TDesIdList&    fcolCut) :

    TBaseIntfUndo()
    , m_colCut(tCIDLib::EAdoptOpts::Adopt, fcolCut.c4ElemCount())
    , m_fcolZOrders(fcolCut.c4ElemCount())
{
    // We need to store the list by z-order first
    tCIDLib::TCard4 c4Count = fcolCut.c4ElemCount();
    const TCQCIntfWidget* piwdgCur;
    TVector<TSortItem> colSorted;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        tCIDLib::TCard4 c4ZOrder;
        piwdgCur = iwdgSrc.piwdgFindByDesId(fcolCut[c4Index], c4ZOrder);
        if (!piwdgCur)
            ThrowIdNotFound(fcolCut[c4Index], CID_LINE);
        colSorted.objAdd(TSortItem(piwdgCur, c4ZOrder));
    }

    // Sort the list and then load up our two lists
    colSorted.Sort(&eCompSortItem);
    c4Count = colSorted.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSortItem& kobjCur = colSorted[c4Index];
        m_colCut.Add(::pDupObject<TCQCIntfWidget>(kobjCur.objKey()));
        m_fcolZOrders.c4AddElement(kobjCur.objValue());
    }
}

TIntfCutUndo::~TIntfCutUndo()
{
    //
    //  We have copies and own them, and our collection is adopting so they
    //  get cleaned up here when the collection destructs.
    //
}


// ---------------------------------------------------------------------------
//  TIntfCutUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIntfCutUndo::bIsSame(const TUndoCmd& uiSrc) const
{
    if (this == &uiSrc)
        return kCIDLib::True;

    if (!TParent::bIsSame(uiSrc))
        return kCIDLib::False;

    // Compare the design ids and z-orders. First check the list sizes
    const TIntfCutUndo& uiRealSrc = static_cast<const TIntfCutUndo&>(uiSrc);
    const tCIDLib::TCard4 c4Count = m_colCut.c4ElemCount();
    if (uiRealSrc.m_colCut.c4ElemCount() != c4Count)
        return kCIDLib::False;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colCut[c4Index]->c8DesId() != uiRealSrc.m_colCut[c4Index]->c8DesId())
            return kCIDLib::False;

        if (m_fcolZOrders[c4Index] != uiRealSrc.m_fcolZOrders[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TIntfCutUndo::Undo(TAttrEditWnd& wndAttrEd, TCQCIntfTemplate& iwdgTar)
{
    //
    //  We want to put the widget back into the template at the point it was
    //  originally at. By putting them in in order, we insure that each
    //  subsequent index is correct.
    //
    const tCIDLib::TCard4 c4Count = m_colCut.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TCard4 c4ZOrder = m_fcolZOrders[c4Index];

        // Make sure it's valid
        if (c4ZOrder > iwdgTar.c4ChildCount())
            ThrowZNotFound(c4ZOrder, CID_LINE);

        //
        //  Note that we keep using 0 since we are removing them as we go,
        //  so the next one slides into the 0th slot each time around.
        //
        TCQCIntfWidget* piwdgCur = m_colCut.pobjOrphanAt(0);
        iwdgTar.AddChildAt(piwdgCur, c4ZOrder);
    }
}


// ---------------------------------------------------------------------------
//  TIntfCutUndo: Private, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TIntfCutUndo::eCompSortItem(const   TSortItem&      kobj1
                            , const TSortItem&      kobj2)
{
    if (kobj1.objValue() < kobj2.objValue())
        return tCIDLib::ESortComps::FirstLess;
    else if (kobj1.objValue() > kobj2.objValue())
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}



// ---------------------------------------------------------------------------
//  CLASS: TIntfFullUndo
// PREFIX: ui
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfFullUndo: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfFullUndo::TIntfFullUndo(const TCQCIntfTemplate& iwdgSave) :

    TBaseIntfUndo()
    , m_iwdgSave(iwdgSave)
{

}

TIntfFullUndo::~TIntfFullUndo()
{
}


// ---------------------------------------------------------------------------
//  TIntfFullUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIntfFullUndo::bIsSame(const TUndoCmd& uiSrc) const
{
    if (this == &uiSrc)
        return kCIDLib::True;

    if (!TParent::bIsSame(uiSrc))
        return kCIDLib::False;

    const TIntfFullUndo& uiRealSrc = static_cast<const TIntfFullUndo&>(uiSrc);
    return (m_iwdgSave.bIsSame(uiRealSrc.m_iwdgSave));
}


// ---------------------------------------------------------------------------
//  TIntfFullUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfFullUndo::Undo(TAttrEditWnd& wndAttrEd, TCQCIntfTemplate& iwdgTmpl)
{
    iwdgTmpl = m_iwdgSave;
}


// ---------------------------------------------------------------------------
//  CLASS: TIntfInsertUndo
// PREFIX: ui
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfInsertUndo: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfInsertUndo::TIntfInsertUndo(const  tCQCIntfEng::TDesIdList& fcolInserted) :

    TBaseIntfUndo()
    , m_fcolInserted(fcolInserted)
{
}

TIntfInsertUndo::TIntfInsertUndo(const TCQCIntfWidget& iwdgInserted) :

    TBaseIntfUndo()
    , m_fcolInserted(1)
{
    m_fcolInserted.c4AddElement(iwdgInserted.c8DesId());
}

TIntfInsertUndo::~TIntfInsertUndo()
{
    // Our collection is non-adopting, since we don't own the widgets
}


// ---------------------------------------------------------------------------
//  TIntfInsertUndo: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Always say it's different because we can legitimately have more than one instance
//  that's the same in a row (if they inserted the same widgets more than once.)
//
tCIDLib::TBoolean TIntfInsertUndo::bIsSame(const TUndoCmd&) const
{
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TIntfInsertUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfInsertUndo::Undo(TAttrEditWnd& wndAttrEd, TCQCIntfTemplate& iwdgTar)
{
    // We want to go back and find the referenced widgets and remove them
    const tCIDLib::TCard4 c4Count = m_fcolInserted.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TCard8 c8DesId = m_fcolInserted[c4Index];
        iwdgTar.RemoveByDesId(c8DesId);
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TIntfLockUndo
// PREFIX: ui
//
//  Saves and restores the lock state of widgets.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfLockUndo: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfLockUndo::TIntfLockUndo(const  tCQCIntfEng::TDesIdList&    fcolChanged
                            , const tCIDLib::TBoolean           bState) :

    TBaseIntfUndo()
    , m_bState(bState)
    , m_fcolChanged(fcolChanged)
{
}

TIntfLockUndo::TIntfLockUndo(const  tCIDLib::TCard8     c8Changed
                            , const tCIDLib::TBoolean   bState) :
    TBaseIntfUndo()
    , m_bState(bState)
    , m_fcolChanged(1UL)
{
    m_fcolChanged.c4AddElement(c8Changed);
}

TIntfLockUndo::~TIntfLockUndo()
{
}


// ---------------------------------------------------------------------------
//  TIntfLockUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIntfLockUndo::bIsSame(const TUndoCmd& uiSrc) const
{
    if (this == &uiSrc)
        return kCIDLib::True;

    if (!TParent::bIsSame(uiSrc))
        return kCIDLib::False;

    const TIntfLockUndo& uiRealSrc = static_cast<const TIntfLockUndo&>(uiSrc);
    return ((m_fcolChanged == uiRealSrc.m_fcolChanged) && (m_bState == uiRealSrc.m_bState));
}


tCIDLib::TVoid TIntfLockUndo::Undo(TAttrEditWnd& wndAttrEd, TCQCIntfTemplate& iwdgTar)
{
    // Put back the lock states back
    const tCIDLib::TCard4 c4Count = m_fcolChanged.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = iwdgTar.piwdgFindByDesId(m_fcolChanged[c4Index]);
        piwdgCur->bIsLocked(m_bState);
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TIntfMultiPosUndo
// PREFIX: ui
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfMultiPosUndo: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfMultiPosUndo::TIntfMultiPosUndo(       TCQCIntfTemplate&           iwdgTar
                                    , const tCQCIntfEng::TDesIdList&    fcolChanged) :

    TBaseIntfUndo()
    , m_colAreas(fcolChanged.c4ElemCount())
    , m_fcolChanged(fcolChanged)
{
    // Store the areas of the changed widgets
    const tCIDLib::TCard4 c4Count = fcolChanged.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgTar = iwdgTar.piwdgFindByDesId(fcolChanged[c4Index]);
        m_colAreas.objAdd(piwdgTar->areaActual());
    }
}

TIntfMultiPosUndo::~TIntfMultiPosUndo()
{
}


// ---------------------------------------------------------------------------
//  TIntfMultiPosUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIntfMultiPosUndo::bIsSame(const TUndoCmd& uiSrc) const
{
    if (this == &uiSrc)
        return kCIDLib::True;

    if (!TParent::bIsSame(uiSrc))
        return kCIDLib::False;

    // Compare the areas and ids
    const TIntfMultiPosUndo& uiRealSrc = static_cast<const TIntfMultiPosUndo&>(uiSrc);
    const tCIDLib::TCard4 c4Count = m_colAreas.c4ElemCount();
    if (uiRealSrc.m_colAreas.c4ElemCount() != c4Count)
        return kCIDLib::False;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colAreas[c4Index] != uiRealSrc.m_colAreas[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TIntfMultiPosUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfMultiPosUndo::Undo(TAttrEditWnd& wndAttrEd, TCQCIntfTemplate& iwdgTmpl)
{
    const tCIDLib::TCard4 c4Count = m_colAreas.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = iwdgTmpl.piwdgFindByDesId(m_fcolChanged[c4Index]);
        if (piwdgCur)
            piwdgCur->SetArea(m_colAreas[c4Index]);
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TIntfSelectUndo
// PREFIX: ui
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfSelectUndo: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfSelectUndo::TIntfSelectUndo(const tCQCIntfEng::TDesIdList& fcolSelected) :

    TBaseIntfUndo()
    , m_fcolSelected(fcolSelected)
{
}

TIntfSelectUndo::~TIntfSelectUndo()
{
}


// ---------------------------------------------------------------------------
//  TIntfSelectUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIntfSelectUndo::bIsSame(const TUndoCmd& uiSrc) const
{
    if (this == &uiSrc)
        return kCIDLib::True;

    if (!TParent::bIsSame(uiSrc))
        return kCIDLib::False;

    // Compare the design time ids
    const TIntfSelectUndo& uiRealSrc = static_cast<const TIntfSelectUndo&>(uiSrc);
    const tCIDLib::TCard4 c4Count = m_fcolSelected.c4ElemCount();
    if (uiRealSrc.m_fcolSelected.c4ElemCount() != c4Count)
        return kCIDLib::False;

    return (m_fcolSelected == uiRealSrc.m_fcolSelected);
}


tCIDLib::TVoid TIntfSelectUndo::Undo(TAttrEditWnd& wndAttrEd, TCQCIntfTemplate&)
{
    // Nothing to do since the editor window restore the list our parent class stores
}


// ---------------------------------------------------------------------------
//  TIntfSelectUndo: Public, non-virtual methods
// ---------------------------------------------------------------------------
const tCQCIntfEng::TDesIdList& TIntfSelectUndo::fcolSelected() const
{
    return m_fcolSelected;
}



// ---------------------------------------------------------------------------
//  CLASS: TIntfZOrderUndo
// PREFIX: ui
//
//  Saves and restores the zorder of widgets.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfZOrderUndo: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfZOrderUndo::TIntfZOrderUndo(const TCQCIntfTemplate& iwdgTmpl) :

    TBaseIntfUndo()
    , m_fcolOrgOrder(iwdgTmpl.c4ChildCount())
{
    // Store the current set of design ids in z-order
    const tCIDLib::TCard4 c4Count = iwdgTmpl.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_fcolOrgOrder.c4AddElement(iwdgTmpl.iwdgAt(c4Index).c8DesId());
}

TIntfZOrderUndo::~TIntfZOrderUndo()
{
}


// ---------------------------------------------------------------------------
//  TIntfZOrderUndo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIntfZOrderUndo::bIsSame(const TUndoCmd& uiSrc) const
{
    if (this == &uiSrc)
        return kCIDLib::True;

    if (!TParent::bIsSame(uiSrc))
        return kCIDLib::False;

    const TIntfZOrderUndo& uiRealSrc = static_cast<const TIntfZOrderUndo&>(uiSrc);
    return (m_fcolOrgOrder == uiRealSrc.m_fcolOrgOrder);
}


tCIDLib::TVoid TIntfZOrderUndo::Undo(TAttrEditWnd& wndAttrEd, TCQCIntfTemplate& iwdgTar)
{
    //
    //  What we do is just tell the template to restore the previous order. It will
    //  do all of the dirty work.
    //
    iwdgTar.SetAllZOrder(m_fcolOrgOrder);
}
