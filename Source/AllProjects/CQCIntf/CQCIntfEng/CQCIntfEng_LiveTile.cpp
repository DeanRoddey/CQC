//
// FILE NAME: CQCIntfEng_LiveTile.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/19/2003
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
//  This file implements the 'live tile' class, a derivative of the overlay class
//  for non-interative, automatically loading overlays.
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
#include    "CQCIntfEng_StaticImg.hpp"
#include    "CQCIntfEng_LiveTile.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfLiveTile, TCQCIntfOverlay)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_LiveTile
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        //
        //  Version 2 -
        //      Convert paths to 5.x format if needed.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 2;


        // -----------------------------------------------------------------------
        //  The number of fields we support
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4FldCount      = 2;


        // -----------------------------------------------------------------------
        //  The caps flags that we use.
        //
        //  Note that we don't actually have any associated field/vars or do any
        //  active updates, but we indicate we want such updates. This insures
        //  that our parent (container) class correctly recurses on us during
        //  update scans, and our parent class can in turn then recurse to child
        //  widgets.
        //
        //  Unlike what our parent class would normally do, we do not support
        //  scrolling or gestures.
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
            , tCQCIntfEng::ECapFlags::NameReq
            , tCQCIntfEng::ECapFlags::OptionalAct
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );


        // -----------------------------------------------------------------------
        //  The name of the press effect widget
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const pszPressFX    = L"PressEffect";
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfLiveTiles
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfLiveTile: Public static methods
// ---------------------------------------------------------------------------

//
//  To assist the attribute editor since we have a complex binary chunk of stuff
//  that it has to unpack, edit, and pack back up.
//
tCIDLib::TCard4
TCQCIntfLiveTile::c4FmtSrcTmplsAttr(const   TTmplList&  colSrc
                                    ,       TMemBuf&    mbufTar)
{
    TBinMBufOutStream strmOut(&mbufTar);
    const tCIDLib::TCard4 c4TmplCnt = colSrc.c4ElemCount();
    strmOut << tCIDLib::EStreamMarkers::StartObject
            << c4TmplCnt
            << tCIDLib::EStreamMarkers::Frame;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
    {
        const TTmplInfo& tiCur = colSrc[c4Index];

        strmOut << tiCur.m_expr1
                << tiCur.m_expr2
                << tiCur.m_eLogOp

                << tiCur.m_strTemplate
                << tiCur.m_strVarName
                << tiCur.m_strVarVal

                // Do a frame separator
                << tCIDLib::EStreamMarkers::Frame;
    }
    strmOut << tCIDLib::EStreamMarkers::EndObject;

    strmOut.Flush();
    return strmOut.c4CurPos();
}


//
//  To assist the attribute editor since we have a complex binary chunk of stuff
//  that it has to unpack, edit, and pack back up.
//
tCIDLib::TVoid
TCQCIntfLiveTile::ParseSrcTmplsAttr(const   TMemBuf&        mbufData
                                    , const tCIDLib::TCard4 c4Bytes
                                    ,       TTmplList&      colToFill)
{
    TBinMBufInStream strmSrc(&mbufData, c4Bytes);
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // First should be the fields and a template count
    tCIDLib::TCard4 c4Count;
    strmSrc >> c4Count;
    strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Then that many template info objects
    colToFill.RemoveAll();
    TTmplInfo tiCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmSrc >> tiCur.m_expr1
                >> tiCur.m_expr2
                >> tiCur.m_eLogOp

                >> tiCur.m_strTemplate
                >> tiCur.m_strVarName
                >> tiCur.m_strVarVal;

        colToFill.objAdd(tiCur);

        // There should be a frame marker after each one
        strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);
    }
    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}



// ---------------------------------------------------------------------------
//  TCQCIntfLiveTiles::TTmplInfo Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Set up for a reaonable default scenario. We only support statement type
//  expressions here so make sure the expressions are setup for that.
//
TCQCIntfLiveTile::TTmplInfo::TTmplInfo() :

    m_eLogOp(tCQCKit::ELogOps::AND)
    , m_expr1
      (
        L"Tile Expr 1"
        , tCQCKit::EExprTypes::Statement
        , tCQCKit::EStatements::None
        , TString::strEmpty()
        , kCIDLib::False
      )
    , m_expr2
      (
        L"Tile Expr 2"
        , tCQCKit::EExprTypes::Statement
        , tCQCKit::EStatements::None
        , TString::strEmpty()
        , kCIDLib::False
      )
{
}

TCQCIntfLiveTile::TTmplInfo::TTmplInfo(const TCQCIntfLiveTile::TTmplInfo& tiSrc) :

    m_expr1(tiSrc.m_expr1)
    , m_expr2(tiSrc.m_expr2)
    , m_eLogOp(tiSrc.m_eLogOp)
    , m_strTemplate(tiSrc.m_strTemplate)
    , m_strVarName(tiSrc.m_strVarName)
    , m_strVarVal(tiSrc.m_strVarVal)
{
}

TCQCIntfLiveTile::TTmplInfo::~TTmplInfo()
{
}


tCIDLib::TVoid
TCQCIntfLiveTile::TTmplInfo::operator=(const TCQCIntfLiveTile::TTmplInfo& tiSrc)
{
    if (this != &tiSrc)
    {
        m_expr1         = tiSrc.m_expr1;
        m_expr2         = tiSrc.m_expr2;
        m_eLogOp        = tiSrc.m_eLogOp;
        m_strTemplate   = tiSrc.m_strTemplate;
        m_strVarName    = tiSrc.m_strVarName;
        m_strVarVal     = tiSrc.m_strVarVal;
    }
}




// ---------------------------------------------------------------------------
//  TCQCIntfLiveTiles: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfLiveTile::TCQCIntfLiveTile() :

    TCQCIntfOverlay
    (
        CQCIntfEng_LiveTile::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_LiveTile)
    )
    , MCQCIntfMultiFldIntf(CQCIntfEng_LiveTile::c4FldCount)
    , m_bFldsRegistered(kCIDLib::False)
    , m_bLastReady(kCIDLib::False)
    , m_bUpdateOK(kCIDLib::False)
{
    // We add an OnClick event
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnClick)
        , kCQCKit::strEvId_OnClick
        , tCQCKit::EActCmdCtx::Standard
    );

    // Set default apppearance
    rgbBgn(kCQCIntfEng_::rgbDef_Bgn2);
}

TCQCIntfLiveTile::TCQCIntfLiveTile(const TCQCIntfLiveTile& iwdgSrc) :

    TCQCIntfOverlay(iwdgSrc)
    , MCQCIntfMultiFldIntf(iwdgSrc)
    , m_bFldsRegistered(kCIDLib::False)
    , m_bLastReady(kCIDLib::False)
    , m_bUpdateOK(kCIDLib::False)
    , m_colTmpls(iwdgSrc.m_colTmpls)
    , m_strFld1(iwdgSrc.m_strFld1)
    , m_strFld2(iwdgSrc.m_strFld2)
{
    // We don't copy runtime only stuff
}

TCQCIntfLiveTile::~TCQCIntfLiveTile()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfLiveTile: Public operators
// ---------------------------------------------------------------------------
TCQCIntfLiveTile&
TCQCIntfLiveTile::operator=(const TCQCIntfLiveTile& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfMultiFldIntf::operator=(iwdgSrc);

        m_colTmpls = iwdgSrc.m_colTmpls;
        m_strFld1  = iwdgSrc.m_strFld1;
        m_strFld2  = iwdgSrc.m_strFld2;

        // Runtime only stuff
        m_bFldsRegistered   = kCIDLib::False;
        m_bLastReady        = kCIDLib::False;
        m_bUpdateOK         = kCIDLib::False;
        m_strCurTemplate.Clear();
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfLiveTile: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfLiveTile::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfLiveTile& iwdgOther(static_cast<const TCQCIntfLiveTile&>(iwdgSrc));

    // Handle our mixins
    if (!MCQCIntfMultiFldIntf::bSameFields(iwdgOther))
        return kCIDLib::False;

    // Do our stuff
    if ((m_strFld1 != iwdgOther.m_strFld1)
    ||  (m_strFld2 != iwdgOther.m_strFld2))
    {
        return kCIDLib::False;
    }

    // If we don't have the same count of templates to load, then not equal
    const tCIDLib::TCard4 c4Count = m_colTmpls.c4ElemCount();
    if (c4Count != iwdgOther.m_colTmpls.c4ElemCount())
        return kCIDLib::False;

    // We have the same count, so compare them
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TTmplInfo& tiUs = m_colTmpls[c4Index];
        const TTmplInfo& tiThem = iwdgOther.m_colTmpls[c4Index];

        if ((tiUs.m_expr1       != tiThem.m_expr1)
        ||  (tiUs.m_expr2       != tiThem.m_expr2)
        ||  (tiUs.m_eLogOp      != tiThem.m_eLogOp)
        ||  (tiUs.m_strTemplate != tiThem.m_strTemplate)
        ||  (tiUs.m_strVarName  != tiThem.m_strVarName)
        ||  (tiUs.m_strVarVal   != tiThem.m_strVarVal))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCIntfLiveTile::Clicked(const TPoint& pntAt)
{
    // We do our click feedback effect
    DoClickFeedback();

    // Post any on click action
    PostOps(*this, kCQCKit::strEvId_OnClick, tCQCKit::EActOrders::NonNested);

    //
    //  Don't do anything else unless you check that we weren't killed by
    //  this action.
    //
}


tCIDLib::TVoid TCQCIntfLiveTile::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfLiveTile&>(iwdgSrc);
}


//
//  Note that any time the overlay is reloaded, that this is called again. So
//  we can't do anything here that would reset our state, or we could get caught
//  in an infinite recursion.
//
tCIDLib::TVoid
TCQCIntfLiveTile::Initialize(TCQCIntfContainer* const   piwdgParent
                            , TDataSrvClient&           dsclInit
                            , tCQCIntfEng::TErrList&    colErrs)
{
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  If we end up doing any init, then we need a flag to only have it happen
    //  once, and not ever time a template is loaded.
    //
}


//
//  We override this in order to suppress the scrolling related RTVs that our
//  parent provides. Those aren't relevant to us. We do this by just forcing a
//  call to the command source's version of the method.
//
TCQCCmdRTVSrc* TCQCIntfLiveTile::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    return MCQCCmdSrcIntf::pcrtsMakeNew(cuctxToUse);
}


//
//  We override this so that this overlay will not be recursed into for hit testing.
//  If the point lies within our area, and they are searching for widgets, then we
//  return ourself as the hit widget, since we act as a widget for hit testing
//  purposes.
//
TCQCIntfWidget*
TCQCIntfLiveTile::piwdgHit( const   TPoint&                 pntTest
                            , const tCQCIntfEng::EHitFlags  eFlags)
{
    if (tCIDLib::bAllBitsOn(eFlags, tCQCIntfEng::EHitFlags::Widgets)
    &&  areaActual().bContainsPoint(pntTest))
    {
        return this;
    }

    // Not us, so return no hit
    return 0;
}



//
//  Note that any time the overlay is reloaded, that this is called again. So
//  we can't do anything here that would reset our state, or we could get caught
//  in an infinite recursion.
//
//  To handle this we have an update ok flag that is set to false when this object
//  is streamed in. We use it to delay the processing of our fields until after
//  the first initialization is done. The value update callback doesn't call through
//  to the field mixin until this flag is set.
//
//  When a new template is loaded, it's reset again, to insure that we don't start
//  processing fields again until all of the init is done on the new template load.
//  We pass on the post init to our parent first, so all children are post-
//  initialized before we set the flag.
//
tCIDLib::TVoid TCQCIntfLiveTile::PostInit()
{
    // Call our parent first
    TParent::PostInit();

    // Set the update ok flag now, which will allow field change callbacks to start
    m_bUpdateOK = kCIDLib::True;
}


//
//  NOTE THAT we don't call the multi-field mixin' version of this, because of our
//  special needs we store the fields separately and only set them on the field
//  interface at a safe time.
//
tCIDLib::TVoid
TCQCIntfLiveTile::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    //
    //  Get our parent's attributes. But we don't want to expose our parent overlay's
    //  attributes. We need more control. So we purposefully skip it! It's attributes
    //  need to either stay at the defaults or at what we set them to.
    //
    TCQCIntfContainer::QueryWdgAttrs(colAttrs, adatTmp);

    //
    //  However we do need to expose his actions, so we do that ourself. When they are
    //  later stored, they will go directly to him and he'll store them. This will include
    //  the OnClick that we forced on him.
    //
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Actions"
    );

    // Remove those common ones that are not applicable to us
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3);

    colAttrs.objPlace
    (
        L"Tile Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    //
    //  Our two control fields we can do as separate attributes. One is required, and
    //  the other is optional.
    //
    colAttrs.objPlace
    (
        L"Field 1"
        , kCQCIntfEng::strAttr_LiveTile_Fld1
        , kCIDMData::strAttrLim_Required
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_strFld1);

    colAttrs.objPlace
    (
        L"Field 2"
        , kCQCIntfEng::strAttr_LiveTile_Fld2
        , TString::strEmpty()
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_strFld2);

    //
    //  For the other stuff we have to provide a general configuration attribute that
    //  we will do visually and store as binary. It's the list of template and the
    //  info associated with each template that indicates whether it's the one to
    //  display.
    //
    THeapBuf mbufData(1024UL);
    tCIDLib::TCard4 c4Bytes = c4FmtSrcTmplsAttr(m_colTmpls, mbufData);
    colAttrs.objPlace
    (
        L"Source Templates"
        , kCQCIntfEng::strAttr_LiveTile_SrcTemplates
        , mbufData
        , c4Bytes
        , TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    );
}


//
//  We have a special concern here. We only want ot register our own fields once,
//  but this will get called every time a new template is loaded. So we have to use
//  a one time flag to allow us to only do it once.
//
tCIDLib::TVoid
TCQCIntfLiveTile::RegisterFields(       TCQCPollEngine& polleToUse
                                , const TCQCFldCache&   cfcData)
{
    TParent::RegisterFields(polleToUse, cfcData);

    if (!m_bFldsRegistered)
    {
        m_bFldsRegistered = kCIDLib::True;
        MCQCIntfMultiFldIntf::RegisterMultiFields(polleToUse, cfcData);
    }
}


// Handle search and replace requests
tCIDLib::TVoid
TCQCIntfLiveTile::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    // Our parent overlay class will handle actions and image and such
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    // If templates are included, then do all our included templates
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Template))
    {
        // We have hit each of the configured templates
        const tCIDLib::TCard4 c4TmplCnt = m_colTmpls.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
        {
            TTmplInfo& tiCur = m_colTmpls[c4Index];
            facCQCKit().bDoSearchNReplace
            (
                strSrc, strTar, tiCur.m_strTemplate, bRegEx, bFull, regxFind
            );
        }
    }
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfLiveTile::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&      adatOld)
{
    // Do the base attrs
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Handle our own attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_LiveTile_Fld1)
    {
        m_strFld1 = adatNew.strValue();

        // Do this to make the validation happy though it's not used during design
        if (m_strFld1.bIsEmpty())
        {
            ClearFieldAssoc(0);
        }
         else
        {
            TString strMoniker;
            TString strField;
            facCQCKit().ParseFldName(m_strFld1, strMoniker, strField);
            AssociateField(strMoniker, strField, 0);
        }
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_LiveTile_Fld2)
    {
        m_strFld2 = adatNew.strValue();

        // Do this to make the validation happy though it's not used during design
        if (m_strFld2.bIsEmpty())
        {
            ClearFieldAssoc(1);
        }
         else
        {
            TString strMoniker;
            TString strField;
            facCQCKit().ParseFldName(m_strFld2, strMoniker, strField);
            AssociateField(strMoniker, strField, 1);
        }
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_LiveTile_SrcTemplates)
    {
        // This one is binary, so we have to stream it back in
        ParseSrcTmplsAttr(adatNew.mbufVal(), adatNew.c4Bytes(), m_colTmpls);
    }
}


//
//  At our level, the designer won't allow any illegal content, but our associated
//  fields may not be available anymore, so we pass it on to the field mixin, then
//  pass it to our parent.
//
tCIDLib::TVoid
TCQCIntfLiveTile::Validate( const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclInit) const
{

    ValidateFields(cfcData, colErrs, dsclInit, *this);
    TParent::Validate(cfcData, colErrs, dsclInit);
}



// ---------------------------------------------------------------------------
//  TCQCIntfLiveTile: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  On any field change, we run down the template list and evaluate each one. When
//  we hit one that matches, we load that one.
//
//  First though we check to see if any of our fields are not ready. If so, then we
//  clear the overlay contents and display an empty template.
//
tCIDLib::TVoid
TCQCIntfLiveTile::FieldChanged(         TCQCFldPollInfo&    cfpiAssoc
                                , const tCIDLib::TCard4     c4FldIndex
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        ctarGlobalVars)
{
    tCIDLib::TBoolean bReady = kCIDLib::True;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < CQCIntfEng_LiveTile::c4FldCount; c4Index++)
    {
        const TString& strCur = c4Index ? m_strFld2 : m_strFld1;

        // If we have this field, and it doesn't have a good value, then not ready
        if (!strCur.bIsEmpty() && !bGoodFieldValue(c4Index))
        {
            bReady = kCIDLib::False;
            break;
        }
    }

    if (bReady != m_bLastReady)
    {
        //
        //  The ready state has changed. If it changed to not ready, then clear out
        //  out our displayed contents.
        //
        m_bLastReady = bReady;
        if (!m_bLastReady)
        {
            m_strCurTemplate.Clear();
            ResetOverlay();
            Invalidate();
        }
    }

    // If we are ready, then see if we need to do anything
    if (m_bLastReady)
    {
        //
        //  Evaluate the template expressions and see if we have a match. If so,
        //  and that's not the same template we are already displaying, let's load
        //  it.
        //
        const tCIDLib::TCard4 c4TmplCnt = m_colTmpls.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
        {
            const TTmplInfo& tiCur = m_colTmpls[c4Index];

            //
            //  If we are in XOR mode, then default to false, so that unset ones
            //  won't cause a false failure. Else to true, so unset ones don't cause
            //  a failure.
            //
            tCIDLib::TBoolean bMatch1;
            tCIDLib::TBoolean bMatch2;

            if (tiCur.m_eLogOp == tCQCKit::ELogOps::XOR)
            {
                bMatch1 = kCIDLib::False;
                bMatch2 = kCIDLib::False;
            }
             else
            {
                bMatch1 = kCIDLib::True;
                bMatch2 = kCIDLib::True;
            }

            if (tiCur.m_expr1.eStatement() != tCQCKit::EStatements::None)
                bMatch1 = tiCur.m_expr1.bEvaluate(fvCurrent(0), &ctarGlobalVars);

            if (tiCur.m_expr2.eStatement() != tCQCKit::EStatements::None)
                bMatch2 = tiCur.m_expr2.bEvaluate(fvCurrent(1), &ctarGlobalVars);

            // Combine the two results via the indicated logical operator
            if (tiCur.m_eLogOp == tCQCKit::ELogOps::AND)
                bMatch1 &= bMatch2;
            else if (tiCur.m_eLogOp == tCQCKit::ELogOps::OR)
                bMatch1 |= bMatch2;
            else if (tiCur.m_eLogOp == tCQCKit::ELogOps::XOR)
                bMatch1 ^= bMatch2;

            if (bMatch1 && (m_strCurTemplate != tiCur.m_strTemplate))
            {
                //
                //  Store the new template. If it's relative expand it, but remember
                //  the actual path from the template info object. Else, we'd have
                //  to expand the template path above every time we wanted to compare.
                //
                m_strCurTemplate = tiCur.m_strTemplate;
                TString strToLoad;
                piwdgParent()->ExpandTmplPath(m_strCurTemplate, strToLoad);

                // Disable updates again until we get past the post init
                m_bUpdateOK = kCIDLib::False;

                // If it has a global variable defined, then set it now
                if (!tiCur.m_strVarName.bIsEmpty())
                {
                    // It's always a global, so local vars don't matter
                    TStdVarsTar ctarLocals(tCIDLib::EMTStates::Unsafe, kCIDLib::True);
                    TCQCActVar& varTar = TStdVarsTar::varFind
                    (
                        tiCur.m_strVarName
                        , ctarLocals
                        , civOwner().ctarGlobalVars()
                        , CID_FILE
                        , CID_LINE
                        , kCIDLib::True
                    );
                    varTar.bSetValue(tiCur.m_strVarVal);
                }

                // And now load the new template
                tCQCIntfEng::TErrList colErrs;
                if (!bLoadNewTemplate(strToLoad, colErrs))
                {
                    facCQCIntfEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcLTile_LoadFailed
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                        , strWidgetId()
                        , strToLoad
                    );

                    m_strCurTemplate.Clear();
                    ResetOverlay();
                    Invalidate();
                }
                break;
            }
        }
    }
}


// Stream ourself in from the source stream
tCIDLib::TVoid TCQCIntfLiveTile::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_LiveTile::c2FmtVersion))
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
    MCQCIntfMultiFldIntf::ReadInFields(strmToReadFrom);

    // Read in our stuff. The two fields are first
    strmToReadFrom  >> m_strFld1
                    >> m_strFld2;

    // Get the count and XOR'd count
    tCIDLib::TCard4 c4TmplCnt, c4XORCnt;

    strmToReadFrom  >> c4TmplCnt
                    >> c4XORCnt;

    if (c4TmplCnt != (c4XORCnt ^ kCIDLib::c4MaxCard))
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcLTile_BadPerCnt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Clear our list and read in the template info objects
    m_colTmpls.RemoveAll();

    TTmplInfo tiCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
    {
        strmToReadFrom  >> tiCur.m_expr1
                        >> tiCur.m_expr2
                        >> tiCur.m_eLogOp

                        >> tiCur.m_strTemplate
                        >> tiCur.m_strVarName
                        >> tiCur.m_strVarVal;

        // Convert the path to 5.x format if needed
        if (c2FmtVersion < 2)
            facCQCKit().Make50Path(tiCur.m_strTemplate);

        m_colTmpls.objAdd(tiCur);

        // There should be a frame marker after each one
        strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    }

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Init any runtime stuff
    m_bFldsRegistered = kCIDLib::False;
    m_bLastReady = kCIDLib::False;
    m_bUpdateOK = kCIDLib::False;
    m_strCurTemplate.Clear();

    //
    //  If not in designer mode, then set up our configured fields with the
    //  multi-field mixin. There's no other safe place to do this that would risk
    //  causing constant recycling. Every time we do this, it would cause us to
    //  get a field change of no good value, then subsequently a field with a value,
    //  so we could get caught in an infinite loop of loading and unload.
    //
    if (!TFacCQCIntfEng::bDesMode())
    {
        TString strMon, strFld;
        if (!m_strFld1.bIsEmpty())
        {
            if (facCQCKit().bParseFldName(m_strFld1, strMon, strFld))
                AssociateField(strMon, strFld, 0);
        }

        if (!m_strFld2.bIsEmpty())
        {
            if (facCQCKit().bParseFldName(m_strFld2, strMon, strFld))
                AssociateField(strMon, strFld, 1);
        }
    }
}


// Stream ourself out to the target stream
tCIDLib::TVoid TCQCIntfLiveTile::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_LiveTile::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfMultiFldIntf::WriteOutFields(strmToWriteTo);

    // Do our stuff. Store the two fields first, then loop through the tmpl infos
    strmToWriteTo   << m_strFld1
                    << m_strFld2;

    const tCIDLib::TCard4 c4TmplCnt = m_colTmpls.c4ElemCount();
    strmToWriteTo   << c4TmplCnt
                    << (c4TmplCnt ^ kCIDLib::c4MaxCard);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
    {
        const TTmplInfo& tiCur = m_colTmpls[c4Index];

        strmToWriteTo   << tiCur.m_expr1
                        << tiCur.m_expr2
                        << tiCur.m_eLogOp

                        << tiCur.m_strTemplate
                        << tiCur.m_strVarName
                        << tiCur.m_strVarVal

                        // Do a frame separator
                        << tCIDLib::EStreamMarkers::Frame;
    }

    // And the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


//
//  We call our parent first, which will recurse through the child widgets to
//  let them update. Then we let our field mixin update.
//
//  We supress our own field updates until we get to post init, so that we don't
//  allow the field change handler to load a new template before we have finished
//  setup.
//
tCIDLib::TVoid
TCQCIntfLiveTile::ValueUpdate(          TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean   bVarsUpdate
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&         grgnClip)
{
    TParent::ValueUpdate(polleToUse, bNoRedraw, bVarsUpdate, ctarGlobalVars, grgnClip);
    if (m_bUpdateOK)
        FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}


// ---------------------------------------------------------------------------
//  TCQCIntfLiveTile: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  In order to support a clicked effect, we allow them to name a static image with
//  the name PressEffect, which would be normally hidden. So we look for such a named
//  image. If we find it (as an immediate child), we unhide it, redraw, pause, then
//  hide it and redraw.
//
tCIDLib::TVoid TCQCIntfLiveTile::DoClickFeedback()
{
    TCQCIntfWidget* piwdgPE = piwdgFindByName
    (
        CQCIntfEng_LiveTile::pszPressFX, kCIDLib::False
    );

    // Make sure we found it and it's a static image. If not, do nothing
    if (!piwdgPE || !piwdgPE->bIsA(TCQCIntfStaticImg::clsThis()))
        return;

    piwdgPE->SetDisplayState(tCQCIntfEng::EDispStates::Normal);
    TThread::Sleep(kCQCIntfEng_::c4PressEmphMSs / 2);
    piwdgPE->SetDisplayState(tCQCIntfEng::EDispStates::Hidden);
}

