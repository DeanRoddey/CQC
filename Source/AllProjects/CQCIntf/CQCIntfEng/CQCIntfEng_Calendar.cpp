//
// FILE NAME: CQCIntfEng_Calendar.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/07/2011
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
//  This file implements our graphing widgets.
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
#include    "CQCIntfEng_Calendar.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfCalendarBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfStaticCalendar, TCQCIntfCalendarBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_Calendar
    {
        // -----------------------------------------------------------------------
        //  The base class persistent version
        //
        //  Version 2 -
        //      No change in data, but we needed to change some color usage to
        //      make them consistent.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion = 2;


        // -----------------------------------------------------------------------
        //  The static persistent version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2StaticFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The number of rows we display
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4Rows = 6;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::DoesGestures
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCalRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class to expose our couple
//  of values.
// ---------------------------------------------------------------------------
class TCQCIntfCalRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfCalRTV
        (
            const   tCIDLib::TCard4         c4Day
            , const tCIDLib::EMonths        eMonth
            , const tCIDLib::TCard4         c4Year
            , const tCIDLib::TEncodedTime   enctStamp
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TCQCIntfCalRTV();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue
        (
            const   TString&                strId
            ,       TString&                strToFill
        )   const;


    private :

        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4ColCount
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4Day;
        tCIDLib::TCard4         m_c4Year;
        tCIDLib::EMonths        m_eMonth;
        tCIDLib::TEncodedTime   m_enctStamp;

};


// ---------------------------------------------------------------------------
//  TCQCIntfCalRTV: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCIntfCalRTV::
TCQCIntfCalRTV( const   tCIDLib::TCard4         c4Day
                , const tCIDLib::EMonths        eMonth
                , const tCIDLib::TCard4         c4Year
                , const tCIDLib::TEncodedTime   enctStamp
                , const TCQCUserCtx&            cuctxToUse) :

    TCQCCmdRTVSrc(cuctxToUse)
    , m_c4Day(c4Day)
    , m_eMonth(eMonth)
    , m_c4Year(c4Year)
    , m_enctStamp(enctStamp)
{
}

TCQCIntfCalRTV::~TCQCIntfCalRTV()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfCalRTV: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIntfCalRTV::bRTValue(const TString& strId,  TString& strToFill) const
{
    // Call our parent first
    if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
        return kCIDLib::True;

    // See if it's one of ours
    if (strId == kCQCIntfEng::strRTVId_SelDay)
    {
        strToFill.SetFormatted(m_c4Day);
    }
     else if (strId == kCQCIntfEng::strRTVId_SelMonth)
    {
        strToFill.SetFormatted(tCIDLib::TCard4(m_eMonth) + 1);
    }
     else if (strId == kCQCIntfEng::strRTVId_SelMonthName)
    {
        strToFill = TLocale::strMonth(m_eMonth);
    }
     else if (strId == kCQCKit::strRTVId_SelTime)
    {
        strToFill = L"0x";
        strToFill.AppendFormatted(m_enctStamp, tCIDLib::ERadices::Hex);
    }
     else if (strId == kCQCIntfEng::strRTVId_SelYear)
    {
        strToFill.SetFormatted(m_c4Year);
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCalendarBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfCalendarBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfCalendarBase::~TCQCIntfCalendarBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfCalendarBase: Public, inherited methods
// ---------------------------------------------------------------------------

// We cannot size to contents since our contents is runtime only
tCIDLib::TBoolean TCQCIntfCalendarBase::bCanSizeTo() const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean TCQCIntfCalendarBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfCalendarBase& iwdgOther
    (
        static_cast<const TCQCIntfCalendarBase&>(iwdgSrc)
    );

    if (!MCQCCmdSrcIntf::bSameCmds(iwdgOther)
    ||  !MCQCIntfFontIntf::bSameFont(iwdgOther)
    ||  !MCQCIntfImgIntf::bSameImg(iwdgOther))
    {
        return kCIDLib::False;
    }
    return (m_eSelType == iwdgOther.m_eSelType);
}


//
//  We allow horz, single finger flicks only. Anything else, we allow to propogate
//  to the parent.
//
tCIDLib::TBoolean
TCQCIntfCalendarBase::bPerGestOpts( const   TPoint&                 pntAt
                                    , const tCIDLib::EDirs          eDir
                                    , const tCIDLib::TBoolean       bTwoFingers
                                    ,       tCIDCtrls::EGestOpts&   eToSet
                                    ,       tCIDLib::TFloat4&       f4VScale)
{
    if (!bTwoFingers
    &&  (eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right))
    {
        eToSet = tCIDCtrls::EGestOpts::Flick;
        f4VScale = 1.0F;

        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCIntfCalendarBase::Clicked(const TPoint& pntAt)
{
    const tCIDLib::TCard4 c4InvokeInd = c4PointToDay(pntAt);

    if (!c4InvokeInd)
        return;

    // It's a click, see if it's something legal for our mode
    if (m_eSelType != tCQCIntfEng::ECalSelTypes::All)
    {
        TTime tmToday(tCIDLib::ESpecialTimes::CurrentDate);

        // Get the current month/year with the new day
        tCIDLib::TCard4 c4Day, c4Year;
        tCIDLib::EMonths eMonth;
        m_tmCurrent.eAsDateInfo(c4Year, eMonth, c4Day);
        TTime tmSel(c4Year, eMonth, c4InvokeInd);

        if (((m_eSelType == tCQCIntfEng::ECalSelTypes::Reverse) && (tmSel > tmToday))
        ||  ((m_eSelType == tCQCIntfEng::ECalSelTypes::Forward) && (tmSel < tmToday)))
        {
            // Not legal, so return onw
            return;
        }
    }

    // Get the current month/year with the new day
    tCIDLib::TCard4 c4Day, c4Year;
    tCIDLib::EMonths eMonth;
    m_tmCurrent.eAsDateInfo(c4Year, eMonth, c4Day);
    m_tmCurrent = TTime(c4Year, eMonth, c4InvokeInd);

    // And post an onselect now
    PostOps(*this, kCQCKit::strEvId_OnSelect, tCQCKit::EActOrders::NonNested);
}


tCIDLib::TVoid TCQCIntfCalendarBase::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfCalendarBase&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfCalendarBase::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_AdjustMonth)
    {
        //
        //  Call a helper to do this, pass the adjust type, and whether
        //  it's ok to draw in this event handler./
        //
        AdjustMonth(ccfgToDo.piAt(0).m_strValue, bCanRedraw(strEventId));
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetSelType)
    {
        if (ccfgToDo.piAt(0).m_strValue == L"All")
            m_eSelType = tCQCIntfEng::ECalSelTypes::All;
        else if (ccfgToDo.piAt(0).m_strValue == L"Future")
            m_eSelType = tCQCIntfEng::ECalSelTypes::Forward;
        else if (ccfgToDo.piAt(0).m_strValue == L"Past")
            m_eSelType = tCQCIntfEng::ECalSelTypes::Reverse;
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_GetMonth)
    {
        // We get two variables, one for the month and one for the year
        const TString& strMonth = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varMonth = TStdVarsTar::varFind
        (
            strMonth, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );

        const TString& strYear = ccfgToDo.piAt(1).m_strValue;
        TCQCActVar& varYear = TStdVarsTar::varFind
        (
            strYear, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );

        // Break out the date values of the current time stamp
        tCIDLib::TCard4 c4Day, c4Year;
        tCIDLib::EMonths eMonth;
        m_tmCurrent.eAsDateInfo(c4Year, eMonth, c4Day);

        tCIDLib::TBoolean bChanged;
        bChanged = varMonth.bSetValue(tCIDLib::TCard4(eMonth) + 1);
        if (bChanged && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varMonth.strName(), varMonth.strValue());

        bChanged = varYear.bSetValue(c4Year);
        if (bChanged && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varYear.strName(), varYear.strValue());
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


tCIDLib::TVoid
TCQCIntfCalendarBase::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Initialize any mixings that require it
    MCQCIntfImgIntf::InitImage(civOwner(), dsclInit, colErrs, *this);

    // Initialize our info to the current date
    m_tmCurrent.SetTo(tCIDLib::ESpecialTimes::CurrentDate);

    //
    //  Do an intiial size calc. Pass a zero size for the old area, so that it
    //  will do the calculations.
    //
    CalcAreas(areaActual(), TArea::areaEmpty);
}


// We have our own small derivative of the runtime value class
TCQCCmdRTVSrc*
TCQCIntfCalendarBase::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    tCIDLib::TCard4 c4Day, c4Year;
    tCIDLib::EMonths eMonth;
    m_tmCurrent.eAsDateInfo(c4Year, eMonth, c4Day);

    return new TCQCIntfCalRTV
    (
        c4Day, eMonth, c4Year, m_tmCurrent.enctTime(), cuctxToUse
    );
}



//
//  Called for post init loading. We need to send out an initial OnScroll event to
//  tell the template about our initial date stamp info.
//
tCIDLib::TVoid TCQCIntfCalendarBase::PostInit()
{
    PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::Nested);
}


//
//  If flicked, we scroll.
//
tCIDLib::TVoid
TCQCIntfCalendarBase::ProcessFlick( const   tCIDLib::EDirs  eDir
                                    , const TPoint&         )
{
    const tCIDLib::TCh* pszCmd = 0;
    if (eDir == tCIDLib::EDirs::Right)
         pszCmd = L"Previous";
    else if (eDir == tCIDLib::EDirs::Left)
         pszCmd = L"Next";

    //
    //  Invoke the same month adjust method that we use when we get an
    //  action command, passing along the same value those commands use,
    //  based on direction.
    //
    if (pszCmd)
        AdjustMonth(pszCmd, kCIDLib::True);

    //
    //  Dont'd do anything else unless you insure that any actions invoked
    //  above didn't kill us!
    //
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfCalendarBase::QueryCmds(        TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Adjust the month forward or back
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_AdjustMonth
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_AdjustMonth)
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Direction)
            , L"Next, Previous"
        )
    );

    // Set the selection type
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetSelType
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetSelType)
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_SelType)
            , L"All, Future, Past"
        )
    );

    // Get the current month
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_GetMonth
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_GetMonth)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Month)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Year)
        )
    );
}


// Add any images that we reference to the passed list
tCIDLib::TVoid TCQCIntfCalendarBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just pass it on to our image mixin
    MCQCIntfImgIntf::QueryImgPath(colToFill, bIncludeSysImgs);
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfCalendarBase::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Delete any underlying attributes we don't honor
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    // Update the names of any colors awe keep that don't default to our needs
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1, L"Today Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2, L"Today Shadow/FX");

    // Do our mixins
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Scroll/Sel Actions"
    );
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, nullptr);
    MCQCIntfImgIntf::QueryImgAttrs(colAttrs, adatTmp, kCIDLib::True);

    // And do our stuff
    adatTmp.Set(L"Calendar Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    TString strLims;
    tCQCIntfEng::FormatECalSelTypes(strLims, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma);
    adatTmp.Set
    (
        L"Select Type"
        , kCQCIntfEng::strAttr_Cal_SelectType
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCQCIntfEng::strXlatECalSelTypes(m_eSelType));
    colAttrs.objAdd(adatTmp);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid TCQCIntfCalendarBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image interface mixin
    MCQCIntfImgIntf::UpdateImage(civOwner(), dsclToUse);
}


// Support search and replace on us
tCIDLib::TVoid
TCQCIntfCalendarBase::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    // Pass on to our parent first
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    // If doing images, pass on to our image mixin
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);
}



TSize TCQCIntfCalendarBase::szDefaultSize() const
{
    return TSize(244, 160);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfCalendarBase::SetWdgAttr(        TAttrEditWnd&  wndAttrEd
                                , const  TAttrData&     adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfImgIntf::SetImgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_Cal_SelectType)
        m_eSelType = tCQCIntfEng::eXlatECalSelTypes(adatNew.strValue());
}


// Support validation in the designer
tCIDLib::TVoid
TCQCIntfCalendarBase::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image mixin
    ValidateImage(colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfCalendarBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the select type
tCQCIntfEng::ECalSelTypes TCQCIntfCalendarBase::eSelType() const
{
    return m_eSelType;
}

tCQCIntfEng::ECalSelTypes
TCQCIntfCalendarBase::eSelType(const tCQCIntfEng::ECalSelTypes eToSet)
{
    m_eSelType = eToSet;
    return m_eSelType;
}

// ---------------------------------------------------------------------------
//  TCQCIntfCalendarBase: Hidden Constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfCalendarBase::
TCQCIntfCalendarBase(const  TString&                    strTypeName
                    , const tCQCIntfEng::ECalSelTypes   eInitSelType) :

    TCQCIntfWidget(CQCIntfEng_Calendar::eCapFlags, strTypeName)
    , MCQCIntfImgIntf(kCIDLib::False)
    , m_eSelType(eInitSelType)
{
    // Set the RTVs we support on our parent class
    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_SelDay)
        , kCQCIntfEng::strRTVId_SelDay
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_SelMonth)
        , kCQCIntfEng::strRTVId_SelMonth
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_SelMonthName)
        , kCQCIntfEng::strRTVId_SelMonthName
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_SelTime)
        , kCQCKit::strRTVId_SelTime
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_SelYear)
        , kCQCIntfEng::strRTVId_SelYear
        , tCQCKit::ECmdPTypes::Unsigned
    );

    // And the same for the events we support
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnScroll)
        , kCQCKit::strEvId_OnScroll
        , tCQCKit::EActCmdCtx::Standard
    );

    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSelect)
        , kCQCKit::strEvId_OnSelect
        , tCQCKit::EActCmdCtx::Standard
    );

    // Set up our default look
    bBold(kCIDLib::True);
    bShadow(kCIDLib::True);
    eHJustify(tCIDLib::EHJustify::Center);
    rgbBgn(kCQCIntfEng_::rgbDef_Bgn);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);
    rgbExtra(kCQCIntfEng_::rgbDef_EmphText);
    rgbExtra2(kCQCIntfEng_::rgbDef_EmphTextShadow);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Calendar");
}

// Sample data is viewing time only and isn't copied
TCQCIntfCalendarBase::TCQCIntfCalendarBase(const TCQCIntfCalendarBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , MCQCIntfImgIntf(iwdgSrc)
    , m_eSelType(iwdgSrc.m_eSelType)
{
}

TCQCIntfCalendarBase&
TCQCIntfCalendarBase::operator=(const TCQCIntfCalendarBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        MCQCIntfImgIntf::operator=(iwdgSrc);

        m_eSelType      = iwdgSrc.m_eSelType;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfCalendarBase: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We have to update our area oriented info
//
tCIDLib::TVoid
TCQCIntfCalendarBase::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    // Call our parent first
    TParent::AreaChanged(areaNew, areaOld);

    // And recalc info
    CalcAreas(areaNew, areaOld);
}


// Called if our size changes
tCIDLib::TVoid
TCQCIntfCalendarBase::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    // Call our parent first
    TParent::SizeChanged(szNew, szOld);

    // Update our size oriented stuff
    TPoint pntOrg = areaActual().pntOrg();
    TArea areaNew(pntOrg, szNew);
    TArea areaOld(pntOrg, szOld);

    CalcAreas(areaNew, areaOld);
}


// Stream our configuration in from the passed stream
tCIDLib::TVoid TCQCIntfCalendarBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Calendar::c2BaseFmtVersion))
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
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);
    MCQCIntfImgIntf::ReadInImage(strmToReadFrom);

    // Stream our stuff, and sanity check it
    tCIDLib::TInt4 i4Tmp;
    strmToReadFrom  >> i4Tmp;
    if ((i4Tmp < 0) || (i4Tmp >= tCIDLib::i4EnumOrd(tCQCIntfEng::ECalSelTypes::Count)))
        m_eSelType = tCQCIntfEng::ECalSelTypes::All;
    else
        m_eSelType = tCQCIntfEng::ECalSelTypes(i4Tmp);

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    //
    //  If V1, then we need to adjust some colors, so that the F2 color is
    //  always used for shadow/fx. Currently F3 is being used for shadow and
    //  F2 for FX, and F2 is also being used for the border.
    //
    if (c2FmtVersion == 1)
    {
        // Save the original values we are going to change
        TRGBClr rgbF2 = rgbFgn2();
        TRGBClr rgbF3 = rgbFgn3();

        // Put F2 into F3, which is now the border color
        rgbFgn3(rgbF2);

        //
        //  If in not in text fx mode, then move F3 to F2, so that whatever
        //  was being used for the shadow will continue to be.
        //
        if (eTextFX() == tCIDGraphDev::ETextFX::None)
            rgbFgn2(rgbF3);
    }

    // Initialize the current date stuff
    m_tmCurrent.SetTo(tCIDLib::ESpecialTimes::Base);
}


tCIDLib::TVoid TCQCIntfCalendarBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_Calendar::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);
    MCQCIntfImgIntf::WriteOutImage(strmToWriteTo);

    // Stream our stuff
    strmToWriteTo   << tCIDLib::TInt4(m_eSelType)
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfCalendarBase::Update(       TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Let our parent handle the standard stuff
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Add our display area to the clip region
    TRegionJanitor janClip(&gdevTarget, m_areaDisplay, tCIDGraphDev::EClipModes::And);

    DrawCalendar
    (
        gdevTarget, m_areaDisplay, areaInvalid, m_tmCurrent, kCIDLib::False
    );
}



// ---------------------------------------------------------------------------
//  TCQCIntfCalendarBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called from either action commands or gestures, when the user
//  asks to go to the previous or next month.
//
tCIDLib::TVoid
TCQCIntfCalendarBase::AdjustMonth(  const   TString&            strType
                                    , const tCIDLib::TBoolean   bCanRedraw)
{
    // Break out the info for the current display
    tCIDLib::TCard4     c4OrgYear;
    tCIDLib::EMonths    eOrgMonth;
    tCIDLib::TCard4     c4OrgDay;

    const tCIDLib::EWeekDays eOrgWD = m_tmCurrent.eAsDateInfo
    (
        c4OrgYear, eOrgMonth, c4OrgDay
    );

    tCIDLib::EMonths eNewMonth = eOrgMonth;
    tCIDLib::TCard4 c4NewYear = c4OrgYear;

    if (strType == L"Previous")
    {
        if (eNewMonth == tCIDLib::EMonths::Min)
        {
            if (c4NewYear > kCIDLib::c4MinYear)
            {
                eNewMonth = tCIDLib::EMonths::Max;
                c4NewYear--;
            }
        }
         else
        {
            eNewMonth--;
        }
    }
     else
    {
        if (eNewMonth == tCIDLib::EMonths::Max)
        {
            if (c4NewYear < kCIDLib::c4MaxYear)
            {
                eNewMonth = tCIDLib::EMonths::Min;
                c4NewYear++;
            }
        }
         else
        {
            eNewMonth++;
        }
    }

    // If we changed, then update
    if ((c4NewYear != c4OrgYear) || (eNewMonth != eOrgMonth))
    {
        //
        //  We may have to tweak the day, since it could have been on a
        //  day not valid for the current month. It's not really that
        //  important in this case, but we want to keep the day at least
        //  legal for the selected year/month.
        //
        const tCIDLib::TCard4 c4MaxDay = TTime::c4MaxDaysForMonth
        (
            eNewMonth, c4NewYear
        );
        tCIDLib::TCard4 c4NewDay = c4OrgDay;
        if (c4NewDay > c4MaxDay)
            c4NewDay = c4MaxDay;

        // Figure out the new date
        TTime tmNew(c4NewYear, eNewMonth, c4NewDay);

        // Save the current in case we need to scroll, then store
        const TTime tmOrg = m_tmCurrent;
        m_tmCurrent = tmNew;

        //
        //  If we can redraw, then do that. If in remote mode or inertia is
        //  disabled for this widget, just do a redraw. Else we do a sliding
        //  scroll.
        //
        if (bCanRedraw)
        {
            if (TFacCQCIntfEng::bRemoteMode() || bNoInertia())
                Redraw();
            else
                SlideScroll(tmOrg, tmNew);
        }

        // Send an OnScroll event trigger
        PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::Nested);
    }
}


//
//  Used by the scrolling stuff, to gather up the background that is under
//  the area well will be scrolling.
//
//  NOTE: This cannot be called from the regular update, since the big
//  background bitmap is already in use in that case. This is only to
//  be called from the scrolling method.
//
TBitmap TCQCIntfCalendarBase::bmpGatherBgn()
{
    //
    //  Get the stuff underneath our display area. We tell it to stop when
    //  it hits our widget, and not to draw it (stop before the one we
    //  indicate.)
    //
    TBitmap bmpBgn = civOwner().bmpQueryContentUnder
    (
        m_areaDisplay, this, kCIDLib::True
    );

    // Now let's draw our bgn contents on top of that
    TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

    //
    //  Make sure that gradients align, so do the fill on the whole
    //  area, which will make the right bit of any gradients fall int our
    //  area.
    //
    DoBgnFill(gdevBgn, areaActual());

    // Create a bitmap the size of our display area
    TBitmap bmpRet
    (
        m_areaDisplay.szArea()
        , tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
    );

    // And copy the bgn contents over into our return bitmap
    {
        TGraphMemDev gdevRet(civOwner().gdevCompat(), bmpRet);
        TArea areaZDisp(m_areaDisplay);
        areaZDisp.ZeroOrg();
        gdevRet.CopyBits(gdevBgn, m_areaDisplay, areaZDisp);
    }
    return bmpRet;
}


//
//  See if a given point falls within an active day slot in our current
//  month. This is always relative to our actual display area.
//
tCIDLib::TCard4 TCQCIntfCalendarBase::c4PointToDay(const TPoint& pntAt) const
{
    // Get the date info from our current time stamp
    tCIDLib::TCard4 c4Day, c4Year;
    tCIDLib::EMonths eMonth;
    m_tmCurrent.eAsDateInfo(c4Year, eMonth, c4Day);
    const tCIDLib::EWeekDays eCurDay = m_tmCurrent.eDayOfWeek();

    //
    //  Set up another time object with the day set to 1, and get the day of week.
    //  This tells us the day of the week for the first day of the month.
    //
    TTime tmFirst(c4Year, eMonth, 1);
    const tCIDLib::EWeekDays eFirstDay = tmFirst.eDayOfWeek();

    // Get the count of days for this month
    tCIDLib::TCard4 c4DayCount = TTime::c4MaxDaysForMonth(eMonth, c4Year);

    //  Get the first block. Move over to the first day's column
    TArea areaBlock = m_areaDisplay;
    areaBlock.SetSize(m_szBlock);
    areaBlock.AdjustOrg(m_szBlock.c4Width() * tCIDLib::c4EnumOrd(eFirstDay), 0);

    //
    //  If the click point is vertically within this block and to its left, then
    //  it has to be in an unused leading slot.
    //
    if ((pntAt.i4Y() <= areaBlock.i4Bottom()) && (pntAt.i4X() <= areaBlock.i4Left()))
        return 0;

    // Keep in mind that days are 1 based!
    tCIDLib::TCard4     c4Index = 1;
    tCIDLib::EWeekDays  eDay = eFirstDay;
    for (; c4Index <= c4DayCount; c4Index++)
    {
        if (areaBlock.bContainsPoint(pntAt))
            break;

        //
        //  If we are on saturday, then move back to the left and down,
        //  else just move right.
        //
        if (eDay == tCIDLib::EWeekDays::Saturday)
        {
            areaBlock.i4X(m_areaDisplay.i4X());
            areaBlock.Move(tCIDLib::EDirs::Down);
            eDay = tCIDLib::EWeekDays::Sunday;
        }
         else
        {
            areaBlock.Move(tCIDLib::EDirs::Right);
            eDay++;
        }
    }

    // If not found, return zero
    if (c4Index > c4DayCount)
        return 0;

    return c4Index;
}


//
//  This calculates some basic info we need order to draw or to do hit
//  testing and such. This ic called any time our size or area changes.
//
tCIDLib::TVoid
TCQCIntfCalendarBase::CalcAreas(const TArea& areaNew, const TArea& areaOld)
{
    // If the size changed, tehn re-calc the size related stuff
    if (!areaNew.bSameSize(areaOld))
    {
        // Start with our overall size
        m_areaDisplay = areaNew;

        // If we have a border, move in 2, else 1
        if (bHasBorder())
            m_areaDisplay.Deflate(2);
        else
            m_areaDisplay.Deflate(1);

        // If rounding, then move in again
        if (c4Rounding())
            m_areaDisplay.Deflate(1);

        // Now calculate the block sizes
        m_szBlock.Set
        (
            m_areaDisplay.c4Width() / 7
            , m_areaDisplay.c4Height() / CQCIntfEng_Calendar::c4Rows
        );

        //
        //  Since they might not be evenly divisible, resize the grid area
        //  based on multiples of the block size, and center it in our target
        //  area.
        //
        m_areaDisplay.SetSize
        (
            m_szBlock.c4Width() * 7
            , m_szBlock.c4Height() * CQCIntfEng_Calendar::c4Rows
        );
    }

    // Either way, make sure the display area is centered in our new area
    m_areaDisplay.CenterIn(areaNew);
}


//
//  This is called to get the area of the block that a given day is in. The
//  day is a 1 based value.
//
//  Starting from the first day column, we then run forward and down to get
//  to the correct row/col indices, then we can calc the area.
//
tCIDLib::TVoid
TCQCIntfCalendarBase::CalcDayArea(  const   tCIDLib::TCard4     c4Day
                                    , const TArea&              areaTar
                                    , const tCIDLib::EWeekDays  eFirstDay
                                    ,       TArea&              areaToFill) const
{

    tCIDLib::TCard4 c4Col = tCIDLib::c4EnumOrd(eFirstDay);
    tCIDLib::TCard4 c4Row = 0;
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Day ; c4Index++)
    {
        if (c4Col == 6)
        {
            c4Col = 0;
            c4Row++;
        }
         else
        {
            c4Col++;
        }
    }

    areaToFill = areaTar;
    areaToFill.SetSize(m_szBlock);
    areaToFill.AdjustOrg(m_szBlock.c4Width() * c4Col, m_szBlock.c4Height() * c4Row);
}



//
//  Calculates the area for a given row, based on a given target display area
//  and the stored block size.
//
tCIDLib::TVoid
TCQCIntfCalendarBase::CalcRowArea(  const   tCIDLib::TCard4 c4Index
                                    , const TArea&          areaTar
                                    ,       TArea&          areaToFill) const
{
    // Start with the whoel target
    areaToFill = areaTar;

    // Set the height to one block height
    areaToFill.c4Height(m_szBlock.c4Height());

    // And update the y origin
    areaToFill.AdjustOrg(0, c4Index * m_szBlock.c4Height());
}


//
//  Handles the actual work of drawing the graph. We don't bother to try to
//  optimize. We are mostly just doing a poly line, which the OS can easily
//  clip to the invalid output area.
//
tCIDLib::TVoid
TCQCIntfCalendarBase::DrawCalendar(         TGraphDrawDev&      gdevTarget
                                    , const TArea&              areaTar
                                    , const TArea&              areaInvalid
                                    , const TTime&              tmTarget
                                    , const tCIDLib::TBoolean   bForceAlpha)
{
    // Calculate some info we need
    tCIDLib::TBoolean   bShowImg = bUseImage();
    tCIDLib::TCard4     c4DayCount;
    tCIDLib::TCard4     c4ThisDay = 0;
    tCIDLib::EWeekDays  eFirstDay;
    {
        // Get the target time info and get the day count for this month
        tCIDLib::TCard4 c4Day, c4Year;
        tCIDLib::EMonths eMonth;
        tmTarget.eAsDateInfo(c4Year, eMonth, c4Day);
        c4DayCount = TTime::c4MaxDaysForMonth(eMonth, c4Year);

        // Figure out the first day of the month for this month.
        TTime tmFirst(c4Year, eMonth, 1);
        eFirstDay = tmFirst.eDayOfWeek();

        //
        //  Save the current day info. If we are showing the current year,
        //  and month, then we need to show current day emphasis info.
        //
        TTime tmNow(tCIDLib::ESpecialTimes::CurrentDate);
        tCIDLib::EMonths eCurMonth;
        tCIDLib::TCard4 c4CurYear;
        tmNow.eAsDateInfo(c4CurYear, eCurMonth, c4ThisDay);

        // If not current year and month, clear the day back out
        if ((c4Year != c4CurYear) || (eMonth != eCurMonth))
            c4ThisDay = 0;
    }

    //
    //  And now draw the actual contents. Start off by calculating the area
    //  for the top row.
    //
    TArea areaRow;
    CalcRowArea(0, areaTar, areaRow);

    const tCIDGraphDev::ETextFmts eFmt = tCIDGraphDev::ETextFmts::None;
    TCQCIntfFontJan janFont(&gdevTarget, *this);
    gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

    //
    //  Figure out which index to start in, based on the first day of
    //  the month. We can then just run through the rest, wrapping
    //  down to the next row as required. We'll do the daycount number
    //  of days in the loop.
    //
    TArea areaBlock(areaRow);
    areaBlock.c4Width(m_szBlock.c4Width());
    areaBlock.AdjustOrg(m_szBlock.c4Width() * tCIDLib::c4EnumOrd(eFirstDay), 0);

    // And now let's display the text for each day
    tCIDLib::EWeekDays eDay = eFirstDay;
    TString strNum;
    TRGBClr rgb1 = rgbFgn();
    TRGBClr rgb2 = rgbFgn2();
    TRGBClr rgb3 = rgbFgn3();

    TCQCIntfView& civUs = civOwner();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DayCount; c4Index++)
    {
        //
        //  If this is the current day, we need to show the emphasis image
        //  if one is set. And we switch to the extra colors for text
        //  display.
        //
        if (c4Index + 1 == c4ThisDay)
        {
            if (bShowImg)
            {
                // Temporarily clip to this day block and draw the image
                TRegionJanitor janClip
                (
                    &gdevTarget, areaBlock, tCIDGraphDev::EClipModes::And
                );

                facCQCIntfEng().DrawImage
                (
                    gdevTarget
                    , bForceAlpha
                    , cptrImage()
                    , areaBlock
                    , areaInvalid
                    , ePlacement()
                    , 0
                    , 0
                    , c1Opacity()
                );
            }

            rgb1 = rgbExtra();
            rgb2 = rgbExtra2();
            rgb3 = rgbExtra3();
        }

        // Format out the number for display
        strNum.SetFormatted(c4Index + 1);

        // And display it according to the text settings
        if (eTextFX() != tCIDGraphDev::ETextFX::None)
        {
            civUs.DrawTextFX
            (
                gdevTarget
                , strNum
                , areaBlock
                , eTextFX()
                , rgb1
                , rgb2
                , eHJustify()
                , eVJustify()
                , bNoTextWrap()
                , pntOffset()
            );
        }
         else
        {
            if (bShadow())
            {
                tCIDLib::TCard4 c4Drop = 1;
                if (c4FontHeight() > 24)
                      c4Drop = 2;
                TArea areaDrop(areaBlock);
                areaDrop.AdjustOrg(c4Drop, c4Drop);
                gdevTarget.SetTextColor(rgb2);

                // If no wrap, use single line draw, else multi-line
                if (bNoTextWrap())
                {
                    if (bForceAlpha)
                    {
                        civUs.DrawTextAlpha
                        (
                            gdevTarget, strNum, areaDrop, eHJustify(), eVJustify(), eFmt
                        );
                    }
                     else
                    {
                        gdevTarget.DrawString
                        (
                            strNum, areaDrop, eHJustify(), eVJustify(), eFmt
                        );
                    }
                }
                 else
                {
                    if (bForceAlpha)
                    {
                        civUs.DrawMTextAlpha
                        (
                            gdevTarget
                            , strNum
                            , areaDrop
                            , eHJustify()
                            , eVJustify()
                            , kCIDLib::True
                        );
                    }
                     else
                    {
                        gdevTarget.DrawMText
                        (
                            strNum, areaDrop, eHJustify(), eVJustify(), kCIDLib::True
                        );
                    }
                }
            }

            //
            //  Now draw our text
            //
            //  <TBD> When mutlti-text supports format flags, we need to use
            //  the format flag set above to show disabled status.
            //
            gdevTarget.SetTextColor(rgb1);

            // If no wrap, use single line draw, else multi-line
            if (bNoTextWrap())
            {
                if (bForceAlpha)
                {
                    civUs.DrawTextAlpha
                    (
                        gdevTarget
                        , strNum
                        , areaBlock
                        , eHJustify()
                        , eVJustify()
                        , eFmt
                    );
                }
                 else
                {
                    gdevTarget.DrawString
                    (
                        strNum, areaBlock, eHJustify(), eVJustify(), eFmt
                    );
                }
            }
             else
            {
                if (bForceAlpha)
                {
                    civUs.DrawMTextAlpha
                    (
                        gdevTarget
                        , strNum
                        , areaBlock
                        , eHJustify()
                        , eVJustify()
                        , kCIDLib::True
                    );
                }
                 else
                {
                    gdevTarget.DrawMText
                    (
                        strNum, areaBlock, eHJustify(), eVJustify(), kCIDLib::True
                    );
                }
            }
        }

        //
        //  If we are on saturday, then move back to the left and down,
        //  else just move right.
        //
        if (eDay == tCIDLib::EWeekDays::Saturday)
        {
            areaBlock.i4X(areaRow.i4X());
            areaBlock.Move(tCIDLib::EDirs::Down);
            eDay = tCIDLib::EWeekDays::Sunday;
        }
         else
        {
            areaBlock.Move(tCIDLib::EDirs::Right);
            eDay++;
        }

        // If we switched to the emphasis colors, switch back to the normal ones
        if (c4Index + 1 == c4ThisDay)
        {
            rgb1 = rgbFgn();
            rgb2 = rgbFgn2();
            rgb3 = rgbFgn3();
        }
    }
}


//
//  Handles a left/right flick, to slide to the new month. We know which way
//  to go based on the relative magnitudes of the two time stamps.
//
tCIDLib::TVoid
TCQCIntfCalendarBase::SlideScroll(const TTime& tmOrg, const TTime& tmNew)
{
    // Create a zero origined display area
    TArea areaZDisp = m_areaDisplay;
    areaZDisp.ZeroOrg();

    // Set up a scroll bitmap of twice the size
    TArea areaScroll(areaZDisp);
    areaScroll.ScaleSize(2.0, 1.0);
    areaScroll.ZeroOrg();

    //
    //  Figure out the direction of the scroll, and the areas where we'll
    //  display the blocks for the old and new pages.
    //
    TArea areaOld = areaZDisp;
    TArea areaNew = areaZDisp;
    tCIDLib::EDirs  eDir;
    if (tmOrg.enctTime() < tmNew.enctTime())
    {
        // We are moving forward in time, so to the left
        eDir = tCIDLib::EDirs::Left;

        // The old on the left and the new on the right
        areaNew.Move(tCIDLib::EDirs::Right);
    }
     else
    {
        // We are moving backwards in time, so to the right
        eDir = tCIDLib::EDirs::Right;

        // The old on the right and the new on the left
        areaOld.Move(tCIDLib::EDirs::Right);
    }

    // Make sure this stuff is cleaned up so we can do a redraw at the end
    {
        //
        //  Set up the bitmap to draw the scrollable contents into. We have
        //  to force this one into memory.
        //
        TBitmap bmpScroll
        (
            areaScroll.szArea()
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
            , kCIDLib::True
        );
        TGraphMemDev gdevScroll(civOwner().gdevCompat(), bmpScroll);

        //
        //  If our background is safe horizontally, then put the bgn fill
        //  into the sroll area. We have to make sure any gradient's align,
        //  so get our full area and center it on the scroll area and fill
        //  that.
        //
        const tCIDLib::TBoolean bForceAlpha = !bScrollSafeBgn(kCIDLib::False);
        if (!bForceAlpha)
        {
            TArea areaFill(areaScroll);
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevScroll, areaFill);
        }

        //
        //  And now let's draw both pages. Tell it whether to force alpha
        //  on if not bgn safe.
        //
        DrawCalendar(gdevScroll, areaOld, areaOld, tmOrg, bForceAlpha);
        DrawCalendar(gdevScroll, areaNew, areaNew, tmNew, bForceAlpha);

        //
        //  We have a special case here since we are doing this scroll. We need
        //  to make sure our display area is clipped to within all our parent's
        //  areas, since the content of the template we are in may be bigger
        //  than the overlay we are in. We start with our display area and
        //  add the containing container clips.
        //
        TGUIRegion grgnClip;
        if (!bFindMostRestrictiveClip(grgnClip, m_areaDisplay))
        {
            //
            //  We are completely clipped. Wierd since we shouldn't have seen
            //  the gesture, but anyway...
            //
            return;
        }

        tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
        TRegionJanitor janClip
        (
            cptrGraphDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::And
        );

        TCIDEaseScroller escrSlide;
        if (bForceAlpha)
        {
            // Get the stuff underneath us
            TBitmap bmpBgn = bmpGatherBgn();
            TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

            // Create a buffer bitmap for the scroll
            TBitmap bmpBuf
            (
                m_areaDisplay.szArea()
                , tCIDImage::EPixFmts::TrueAlpha
                , tCIDImage::EBitDepths::Eight
                , kCIDLib::True
            );
            TGraphMemDev gdevBuf(civOwner().gdevCompat(), bmpBuf);

            escrSlide.Scroll1
            (
                *cptrGraphDev
                , m_areaDisplay
                , areaScroll
                , gdevBuf
                , gdevBgn
                , gdevScroll
                , kCQCIntfEng_::c4EaseScrollMSs
                , eDir
                , kCQCIntfEng_::c4EaseOrder
            );
        }
         else
        {
            // WE can do the more efficient scroll
            escrSlide.Scroll2
            (
                *cptrGraphDev
                , m_areaDisplay
                , areaScroll
                , gdevScroll
                , kCQCIntfEng_::c4EaseScrollMSs
                , eDir
                , kCQCIntfEng_::c4EaseOrder
            );
        }
    }

    // Do a final redraw
    Redraw();
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticCalendar
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticCalendar: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticCalendar::TCQCIntfStaticCalendar() :

    TCQCIntfCalendarBase(L"Static Calendar", tCQCIntfEng::ECalSelTypes::All)
{
}

// Only the name value is something that needs to be copied
TCQCIntfStaticCalendar::TCQCIntfStaticCalendar(const TCQCIntfStaticCalendar& iwdgToCopy) :

    TCQCIntfCalendarBase(iwdgToCopy)
{
}

TCQCIntfStaticCalendar::~TCQCIntfStaticCalendar()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticCalendar: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticCalendar&
TCQCIntfStaticCalendar::operator=(const TCQCIntfStaticCalendar& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticCalendar: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfStaticCalendar::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticCalendar&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfStaticCalendar::Initialize( TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfStaticCalendar::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfStaticCalendar::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfStaticCalendar::Validate(const  TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticCalendar: Protected, inherited methods
// ---------------------------------------------------------------------------

// Stream ourself in
tCIDLib::TVoid TCQCIntfStaticCalendar::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Calendar::c2StaticFmtVersion))
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

    // Stream our stuff (we have none of our own yet)

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


// Stream ourself out
tCIDLib::TVoid TCQCIntfStaticCalendar::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_Calendar::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff (we don't have any yet) and and end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


