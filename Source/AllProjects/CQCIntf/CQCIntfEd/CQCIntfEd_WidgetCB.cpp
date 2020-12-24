//
// FILE NAME: CQCIntfEd_WidgetCB.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2004
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
//  This class is used for cutting/pasting selected widgets.
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
//  Magic macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfAttrPack,TObject)
AdvRTTIDecls(TCQCIntfEdWdgCB,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEd_WdgCB
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our attribute pack persistent format version. Generally not required since
        //  these are only flattened in memory, but just in case.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2APFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  Our widget persistent format version. Generally not required since these are
        //  only flattened in memory, but just in case.
        //
        //  2.  Added the type map
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2WdgFmtVersion = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfAttrPack
//  PREFIX: iap
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfAttrPack: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfAttrPack::TCQCIntfAttrPack()
{
}

TCQCIntfAttrPack::~TCQCIntfAttrPack()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfAttrPack: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCIntfAttrPack::c4ElemCount() const
{
    return m_colAttrs.c4ElemCount();
}


const tCIDMData::TAttrList& TCQCIntfAttrPack::colAttrs() const
{
    return m_colAttrs;
}

tCIDMData::TAttrList& TCQCIntfAttrPack::colAttrs()
{
    return m_colAttrs;
}

const TAttrData& TCQCIntfAttrPack::attrAt(const tCIDLib::TCard4 c4At) const
{
    return m_colAttrs[c4At];
}


tCIDLib::TVoid TCQCIntfAttrPack::Reset()
{
    m_colAttrs.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TCQCIntfAttrPack: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfAttrPack::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // And next should be the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEd_WdgCB::c2APFmtVersion))
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

    // And get our stuff out
    tCIDLib::TCard4 c4Size, c4XSize;
    strmToReadFrom >> c4Size >> c4XSize;
    if ((c4Size ^ kCIDLib::c4MaxCard) != c4XSize)
    {
        facCQCIntfEd().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEdErrs::errcGen_BadCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    strmToReadFrom >> m_colAttrs;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfAttrPack::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEd_WdgCB::c2APFmtVersion
                    << m_colAttrs.c4ElemCount()
                    << tCIDLib::TCard4(m_colAttrs.c4ElemCount() ^ kCIDLib::c4MaxCard)
                    << m_colAttrs
                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfEdWdgCB
//  PREFIX: wcb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfEdWdgCB: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfEdWdgCB::TCQCIntfEdWdgCB(const  TCQCIntfTemplate&        iwdgSrc
                                , const tCQCIntfEng::TDesIdList& fcolSrc) :

    m_colWidgets(tCIDLib::EAdoptOpts::Adopt, fcolSrc.c4ElemCount())
{
    // Make copies of all the widgets in the id list
    tCIDLib::TCard4 c4Count = fcolSrc.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgNew = ::pDupObject<TCQCIntfWidget>
        (
            iwdgSrc.piwdgFindByDesId(fcolSrc[c4Index])
        );

        //
        //  This guy is no longer in the template, so clear the owning view and
        //  container else it might do something bad.
        //
        piwdgNew->SetOwner(nullptr);
        piwdgNew->SetParent(nullptr);

        m_colWidgets.Add(piwdgNew);
    }

    //
    //  And load up our type map as well. We can use the find all targets
    //  method, which just finds all of the named targets. We tell it not to
    //  recurse, though it really makes no difference since we never load
    //  up any overlays in the designer. But, just in case...
    //
    TRefVector<const MCQCCmdTarIntf> colTars(tCIDLib::EAdoptOpts::NoAdopt);
    iwdgSrc.FindAllTargets(colTars, kCIDLib::False);
    c4Count = colTars.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const MCQCCmdTarIntf* pmctarCur = colTars[c4Index];
        if (!pmctarCur->strCmdTargetName().bIsEmpty())
        {
            m_colTypeMap.objAdd
            (
                TKeyValuePair
                (
                    pmctarCur->strCmdTargetName(), pmctarCur->strCmdTargetType()
                )
            );
        }
    }

    //
    //  Calculate the source point, which is the origin of the largest area
    //  that will hold them all.
    //
    CalcOrigin();
}

TCQCIntfEdWdgCB::~TCQCIntfEdWdgCB()
{
}



// ---------------------------------------------------------------------------
//  TCQCIntfEdWdgCB: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCIntfEdWdgCB::c4ElemCount() const
{
    return m_colWidgets.c4ElemCount();
}


const TCQCIntfEdWdgCB::TTypeMap& TCQCIntfEdWdgCB::colTypeMap() const
{
    return m_colTypeMap;
}


const tCQCIntfEng::TChildList& TCQCIntfEdWdgCB::colWidgets() const
{
    return m_colWidgets;
}


TCQCIntfWidget* TCQCIntfEdWdgCB::piwdgAt(const tCIDLib::TCard4 c4At)
{
    return m_colWidgets[c4At];
}

const TCQCIntfWidget*
TCQCIntfEdWdgCB::piwdgAt(const tCIDLib::TCard4 c4At) const
{
    return m_colWidgets[c4At];
}


TCQCIntfWidget* TCQCIntfEdWdgCB::piwdgOrphanNext()
{
    return m_colWidgets.pobjOrphanAt(0);
}


tCIDLib::TVoid TCQCIntfEdWdgCB::Reset()
{
    m_colWidgets.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TCQCIntfEdWdgCB: Hidden Constructors
// ---------------------------------------------------------------------------
TCQCIntfEdWdgCB::TCQCIntfEdWdgCB() :

    m_colWidgets(tCIDLib::EAdoptOpts::Adopt)
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfEdWdgCB: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfEdWdgCB::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // And next should be the format version
    tCIDLib::TCard2 c2WdgFmtVersion;
    strmToReadFrom  >> c2WdgFmtVersion;
    if (!c2WdgFmtVersion || (c2WdgFmtVersion > CQCIntfEd_WdgCB::c2WdgFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2WdgFmtVersion)
            , clsThis()
        );
    }

    // And get our stuff out
    tCIDLib::TCard4 c4Count;
    tCIDLib::TCard4 c4XorCount;
    strmToReadFrom  >> c4Count >> c4XorCount;
    if ((c4Count ^ kCIDLib::c4MaxCard) != c4XorCount)
    {
        facCQCIntfEd().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEdErrs::errcGen_BadCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Get rid of any current content
    m_colWidgets.RemoveAll();

    TCQCIntfWidget* piwdgNew;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Polymorphically stream out a widget and put it in our list
        ::PolymorphicRead(piwdgNew, strmToReadFrom);
        m_colWidgets.Add(piwdgNew);
    }

    // Stream in the type map
    if (c2WdgFmtVersion > 1)
        strmToReadFrom >> m_colTypeMap;
    else
        m_colTypeMap.RemoveAll();

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfEdWdgCB::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEd_WdgCB::c2WdgFmtVersion
                    << m_colWidgets.c4ElemCount()
                    << tCIDLib::TCard4(m_colWidgets.c4ElemCount() ^ kCIDLib::c4MaxCard);

    // Polymorphically stream out the widgets
    const tCIDLib::TCard4 c4Count = m_colWidgets.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        ::PolymorphicWrite(m_colWidgets[c4Index], strmToWriteTo);

    // And stream out the type map and end marker
    strmToWriteTo   << m_colTypeMap
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCIntfEdWdgCB: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfEdWdgCB::CalcOrigin()
{
    //
    //  First we want to get the bounding area for all the widgets. If we have
    //  none selected, then we really shouldn't have been called, so just
    //  return.
    //
    const tCIDLib::TCard4 c4Count = m_colWidgets.c4ElemCount();
    if (!c4Count)
        return;

    // We have widgets, so get the containing area
    TArea areaTmp(m_colWidgets[0]->areaActual());
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        areaTmp |= m_colWidgets[c4Index]->areaActual();

    // Create a negated version of the common origin
    TPoint pntNegOrg(areaTmp.pntOrg());
    pntNegOrg.Negate();

    //
    //  Now go back and adjust each widget's origin to be relative to this
    //  bounding area, by adjusting the origin by the negated bounding origin.
    //
    TArea areaNew;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget& iwdgCur = *m_colWidgets[c4Index];
        areaNew = iwdgCur.areaActual();
        areaNew.AdjustOrg(pntNegOrg);
        iwdgCur.SetArea(areaNew);
    }
}


