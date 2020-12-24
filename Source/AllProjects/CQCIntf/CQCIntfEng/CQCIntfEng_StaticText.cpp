//
// FILE NAME: CQCIntfEng_StaticText.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/06/2002
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
//  This file implements a derivative of the base single line text display
//  widget, which displays a fixed line of text.
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
#include    "CQCIntfEng_StaticText.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfStaticText, TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfMediaRepoText,TCQCIntfStaticText)
AdvRTTIDecls(TCQCIntfTitleText,TCQCIntfStaticText)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_StaticText
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The static text persistent format version
        //
        //  Version 2 -
        //      Action contexts were moved into the events, so we want to force
        //      a non-standard context on some events for older verions.
        //
        //  Version 3 -
        //      No data change but we need to adjust our use of colors to make them
        //      consistent.
        //
        //  Version 4 -
        //      Allow the secret mode flag to be a persistent attribute, not just something
        //      set at runtime.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2StaticFmtVersion  = 4;


        // -----------------------------------------------------------------------
        //  The media repo text persistent format version
        //
        //  Version 1 -
        //      Original version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2MTextFmtVersion   = 1;


        // -----------------------------------------------------------------------
        //  The title text persistent format version
        //
        //  Version 1 -
        //      Original version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2TitleFmtVersion   = 1;


        // -----------------------------------------------------------------------
        //  The caps flags that we use for normal mode
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::OptionalAct
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticTextRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class, to add our 'last
//  error' runtime value.
// ---------------------------------------------------------------------------
class TCQCIntfStaticTextRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticTextRTV(  const   TString&      strLastError
                                , const TCQCUserCtx&  cuctxToUse) :

            TCQCCmdRTVSrc(cuctxToUse)
            , m_strLastError(strLastError)
        {
        }

        ~TCQCIntfStaticTextRTV() {}


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean
        bRTValue(const TString& strId,  TString& strToFill) const
        {
            // Call our parent first
            if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
                return kCIDLib::True;

            if (strId == kCQCKit::strRTVId_ErrorMsg)
                strToFill = m_strLastError;
            else
                return kCIDLib::False;
            return kCIDLib::True;
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strLastError
        //      This is the last error that occured during the Send command.
        //      This is our one RTV above and beyond the base RTV class.
        // -------------------------------------------------------------------
        TString m_strLastError;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticText::TCQCIntfStaticText() :

    TCQCIntfWidget
    (
        CQCIntfEng_StaticText::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          +  facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Text)
    )
    , m_bMarquee(kCIDLib::False)
    , m_bPatternMode(kCIDLib::False)
    , m_bSecretMode(kCIDLib::False)
    , m_c4AvgCharWidth(0)
    , m_c4MaxChars(kCIDLib::c4MaxCard)
    , m_c4TextWidth(0)
    , m_enctNextScroll(0)
    , m_i4LastOfs(0)
{
    // Add any runtime values we support
    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_ErrorMsg)
        , kCQCKit::strRTVId_ErrorMsg
        , tCQCKit::ECmdPTypes::Text
    );

    //
    //  We can't be marquee mode, so turn off single line mode and allow
    //  multi-line text.
    //
    ResetCaption(kCIDLib::False, TString::strEmpty());

    // Add the events that we suport
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnError)
        , kCQCKit::strEvId_OnError
        , tCQCKit::EActCmdCtx::OnError
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSuccess)
        , kCQCKit::strEvId_OnSuccess
        , tCQCKit::EActCmdCtx::NoContChange
    );

    // Set out default look
    bBold(kCIDLib::True);
    bIsTransparent(kCIDLib::True);
    bShadow(kCIDLib::True);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Text");
}

TCQCIntfStaticText::
TCQCIntfStaticText( const   tCIDLib::TBoolean       bMarqueeMode
                    , const tCQCIntfEng::ECapFlags  eCapFlags
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCapFlags, strTypeName)
    , m_bMarquee(bMarqueeMode)
    , m_bPatternMode(kCIDLib::False)
    , m_bSecretMode(kCIDLib::False)
    , m_c4AvgCharWidth(0)
    , m_c4MaxChars(kCIDLib::c4MaxCard)
    , m_c4TextWidth(0)
    , m_enctNextScroll(0)
    , m_i4LastOfs(0)
{
    // If not marquee mode, then add an RTV
    if (!bMarqueeMode)
    {
        AddRTValue
        (
            facCQCKit().strMsg(kKitMsgs::midRT_ErrorMsg)
            , kCQCKit::strRTVId_ErrorMsg
            , tCQCKit::ECmdPTypes::Text
        );
    }

    //
    //  If not in marquee mode, we can turn off single line mode and allow
    //  multi-line text.
    //
    if (!bMarqueeMode)
        ResetCaption(kCIDLib::False, TString::strEmpty());

    // Add the events that we suport
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnError)
        , kCQCKit::strEvId_OnError
        , tCQCKit::EActCmdCtx::OnError
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSuccess)
        , kCQCKit::strEvId_OnSuccess
        , tCQCKit::EActCmdCtx::NoContChange
    );

    // Set out default look
    bBold(kCIDLib::True);
    bIsTransparent(kCIDLib::True);
    bShadow(kCIDLib::True);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);

    // Set our command help id for our family of widgets
    if (bMarqueeMode)
        strCmdHelpId(L"/Reference/IntfWidgets/Marquee");
    else
        strCmdHelpId(L"/Reference/IntfWidgets/Text");
}

TCQCIntfStaticText::TCQCIntfStaticText(const TCQCIntfStaticText& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , m_bMarquee(iwdgSrc.m_bMarquee)
    , m_bPatternMode(iwdgSrc.m_bPatternMode)
    , m_bSecretMode(iwdgSrc.m_bSecretMode)
    , m_c4AvgCharWidth(iwdgSrc.m_c4AvgCharWidth)
    , m_c4MaxChars(iwdgSrc.m_c4MaxChars)
    , m_c4TextWidth(iwdgSrc.m_c4TextWidth)
    , m_enctNextScroll(iwdgSrc.m_enctNextScroll)
    , m_i4LastOfs(iwdgSrc.m_i4LastOfs)
{
}

TCQCIntfStaticText::~TCQCIntfStaticText()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticText: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticText&
TCQCIntfStaticText::operator=(const TCQCIntfStaticText& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);

        m_bMarquee       = iwdgSrc.m_bMarquee;
        m_bPatternMode   = iwdgSrc.m_bPatternMode;
        m_bSecretMode    = iwdgSrc.m_bSecretMode;
        m_c4AvgCharWidth = iwdgSrc.m_c4AvgCharWidth;
        m_c4MaxChars     = iwdgSrc.m_c4MaxChars;
        m_c4TextWidth    = iwdgSrc.m_c4TextWidth;
        m_enctNextScroll = iwdgSrc.m_enctNextScroll;
        m_i4LastOfs      = iwdgSrc.m_i4LastOfs;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticText: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfStaticText::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfStaticText::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticText& iwdgOther(static_cast<const TCQCIntfStaticText&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfFontIntf::bSameFont(iwdgOther)
    ||  !MCQCCmdSrcIntf::bSameCmds(iwdgOther))
    {
        return kCIDLib::False;
    }

    // Do our own stuff
    return
    (
        (m_bMarquee == iwdgOther.m_bMarquee)
        && (m_bPatternMode == iwdgOther.m_bPatternMode)
        && (m_bSecretMode == iwdgOther.m_bSecretMode)
        && (m_c4MaxChars == iwdgOther.m_c4MaxChars)
    );
}


tCIDLib::TVoid
TCQCIntfStaticText::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticText&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfStaticText::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Append)
    {
        const TString& strCap = strCaption();

        TString strNew = strCap;
        strNew.Append(ccfgToDo.piAt(0).m_strValue);

        // Cap to the max length if needed
        if (strNew.c4Length() > m_c4MaxChars)
            strNew.CapAt(m_c4MaxChars);
        strCaption(strNew);

        // Redraw if not hidden and not in a preload event
        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Clear)
    {
        strCaption(TString::strEmpty());

        // Redraw if not hidden and not in a preload event
        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_DeleteLast)
    {
        TString strNew = strCaption();
        bResult = !strNew.bIsEmpty();
        if (bResult)
        {
            strNew.DeleteLast();
            strCaption(strNew);

            // Redraw if not hidden and not in a preload event
            if (bCanRedraw(strEventId))
                Redraw();
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetText)
    {
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varText = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );
        if (varText.bSetValue(strCaption()) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strKey, varText.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SendValue)
    {
        const TString& strCap = strCaption();

        //
        //  If in pattern mode, see if the value matches the pattern. If not
        //  then set the error message and issue an OnError and don't
        //  send the message.
        //
        if (m_bPatternMode)
        {
            if (!m_regxPattern.bFullyMatches(strCap, kCIDLib::True))
            {
                m_strLastError = facCQCIntfEng().strMsg(kIEngErrs::errcFld_InvalidSendVal);
                PostOps(*this, kCQCKit::strEvId_OnError, tCQCKit::EActOrders::Nested);

                //
                //  Don't do anything else unless you check that we weren't
                //  killed by this action.
                //
                return tCQCKit::ECmdRes::Ok;
            }
        }

        //
        //  If there is a format string, then get it and format the current
        //  value into it. Else, just take the current value.
        //
        TString strWrite = ccfgToDo.piAt(1).m_strValue;
        if (strWrite.bIsEmpty())
            strWrite = strCap;
        else
            strWrite.eReplaceToken(strCaption(), kCIDLib::chLatin_v);

        // Pull the driver moniker out of the field name
        TString strFld;
        TString strMon;
        facCQCKit().ParseFldName(ccfgToDo.piAt(0).m_strValue, strMon, strFld);

        // show the fully expanded command
        if (acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActDebug(TString(L"Sending value: ") + strWrite);

        // And write the field. Catch any error
        try
        {
            facCQCIntfEng().WriteField(civOwner(), strMon, strFld, strWrite);
        }

        catch(const TError& errToCatch)
        {
            // Log the error first, if not already
            TModule::LogEventObj(errToCatch);

            // Set the text of the error as our last error text
            m_strLastError = errToCatch.strErrText();

            // Since it failed, post an OnError event and return
            PostOps(*this, kCQCKit::strEvId_OnError, tCQCKit::EActOrders::Nested);

            //
            //  Don't do anything else unless you check that we weren't
            //  killed by this action.
            //

            return tCQCKit::ECmdRes::Ok;
        }

        // It worked so post an OnSuccess event
        PostOps(*this, kCQCKit::strEvId_OnSuccess, tCQCKit::EActOrders::Nested);

        //
        //  Don't do anything else unless you check that we weren't
        //  killed by this action.
        //
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetMaxChars)
    {
        m_c4MaxChars = ccfgToDo.piAt(0).m_strValue.c4Val(tCIDLib::ERadices::Auto);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetPattern)
    {
        //
        //  If the pattern is empty, we clear pattern mode, else we
        //  set the regular expression and set the flag.
        //
        const TString& strVal = ccfgToDo.piAt(0).m_strValue;
        if (strVal.bIsEmpty())
        {
            m_bPatternMode = kCIDLib::False;
        }
         else
        {
            // If this fails, we allow the exception to propogate
            m_regxPattern.SetExpression(strVal);
            m_bPatternMode = kCIDLib::True;
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetSecretMode)
    {
        m_bSecretMode = facCQCKit().bCheckBoolVal(ccfgToDo.piAt(0).m_strValue);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetText)
    {
        const TString& strText = ccfgToDo.piAt(0).m_strValue;

        // Recalculate our text size and reset the scroll, if in marquee mode
        if (m_bMarquee)
        {
            tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
            TCQCIntfFontJan janFont(cptrGraphDev.pobjData(), *this);
            m_c4TextWidth = cptrGraphDev->areaString(strText, kCIDLib::False).szArea().c4Width();
            m_i4LastOfs = 0;
        }

        // Pass the new text on to our parent
        strCaption(strText);

        // Redraw if not hidden and not in a preload event
        if (bCanRedraw(strEventId))
            Redraw();
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
TCQCIntfStaticText::Initialize( TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Set our font and get the width our text, and the average character
    //  width. If we are a marguee mode static text widget, we'll use this
    //  in the scrolling code.
    //
    if (m_bMarquee)
    {
        tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
        TCQCIntfFontJan janFont(cptrGraphDev.pobjData(), *this);
        TFontMetrics fmtrCur(*cptrGraphDev);
        m_c4AvgCharWidth = fmtrCur.c4AverageWidth();
        m_c4TextWidth = cptrGraphDev->areaString
        (
            strCaption(), kCIDLib::False
        ).szArea().c4Width();
    }
}


//
//  We return a simple derivative of the RTV source since we have some RTVs
//  of our own.
//
TCQCCmdRTVSrc*
TCQCIntfStaticText::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    // Give it the last error value, which is our only class specific RTV
    return new TCQCIntfStaticTextRTV(m_strLastError, cuctxToUse);
}


tCIDLib::TVoid TCQCIntfStaticText::QueryContentSize(TSize& szToFill)
{
    //
    //  Set our font and then get the area of the string. Get the size out
    //  of that as the basic size.
    //
    tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
    TCQCIntfFontJan janFont(cptrGraphDev.pobjData(), *this);

    // If no text wrap, then do single line calculation
    const TString* pstrContent = &strCaption();
    if (pstrContent->bIsEmpty())
        pstrContent = &facCQCIntfEng().strErrText();

    if (bNoTextWrap())
    {
        szToFill = cptrGraphDev->areaString(*pstrContent, kCIDLib::False).szArea();
    }
     else
    {
        TArea areaTmp = areaActual();
        szToFill = cptrGraphDev->areaMLText(*pstrContent, areaTmp.c4Width()).szArea();
    }

    // If we have a border, then increase by two
    if (bHasBorder())
        szToFill.Adjust(2);

    // Add a little margin around the text
    szToFill.Adjust(6, 2);
}


tCIDLib::TVoid
TCQCIntfStaticText::QueryCmds(          TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Append new text to our text
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Append
            , facCQCKit().strMsg(kKitMsgs::midCmd_Append)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_NewText)
        )
    );

    // Clear our text
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Clear
            , facCQCKit().strMsg(kKitMsgs::midCmd_Clear)
            , 0
        )
    );

    // Delete the last character
    {
        TCQCCmd& cmdDelLast = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_DeleteLast
                , facCQCKit().strMsg(kKitMsgs::midCmd_DeleteLast)
                , 0
            )
        );
        cmdDelLast.eType(tCQCKit::ECmdTypes::Both);
    }

    // We put our current text into the passed variable
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetText
            , facCQCKit().strMsg(kKitMsgs::midCmd_GetText)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );

    //
    //  Set our current value to the indicated field, if not an IV event
    //  or preload action.
    //
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd& cmdSend = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SendValue
                , facCQCKit().strMsg(kKitMsgs::midCmd_SendValue)
                , tCQCKit::ECmdPTypes::Field
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_FmtStr)
            )
        );
        cmdSend.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::CReadMWrite);
        cmdSend.SetParmReqAt(1, kCIDLib::False);
    }

    // Set the max chars we can take
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetMaxChars
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetMaxChars)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        )
    );

    //
    //  Set the input pattern. The pattern parm can be empty, to clear
    //  it, so we make it optional.
    //
    {
        TCQCCmd& cmdSetPat = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetPattern
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetPattern)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
            )
        );
        cmdSetPat.SetParmReqAt(0, kCIDLib::False);
    }

    // Set the secret mode flag
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetSecretMode
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetSecretMode)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Mode)
        )
    );

    // Set our text to the passed text, which can be an empty string
    {
        TCQCCmd& cmdSetText = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetText
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetText)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_NewText)
            )
        );
        cmdSetText.SetParmReqAt(0, kCIDLib::False);
    }
}


// Return configuration summary info about us
tCIDLib::TVoid
TCQCIntfStaticText::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove base attributes we don't make use of
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Update color names as required to suit us
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");

    // Do our mixins
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, &strCaption());
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Text Actions"
    );

    // Do our attributes
    colAttrs.objPlace
    (
        L"Static Text Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    colAttrs.objPlace
    (
        L"Marquee Mode", kCQCIntfEng::strAttr_Text_Marquee, tCIDMData::EAttrTypes::Bool
    ).SetBool(m_bMarquee);

    colAttrs.objPlace
    (
        L"Secret Mode", kCQCIntfEng::strAttr_Text_Secret, tCIDMData::EAttrTypes::Bool
    ).SetBool(m_bSecretMode);
}


tCIDLib::TVoid
TCQCIntfStaticText::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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


TSize TCQCIntfStaticText::szDefaultSize() const
{
    TGraphDesktopDev gdevToUse;
    TCQCIntfFontJan janFont(&gdevToUse, *this);
    TArea areaText = gdevToUse.areaString(L"A default string");
    areaText.AdjustSize(16, 4);
    return areaText.szArea();
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfStaticText::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);

    // Do our attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_Text_Marquee)
        m_bMarquee = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Text_Secret)
        m_bSecretMode = adatNew.bVal();
}


tCIDLib::TVoid
TCQCIntfStaticText::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticText: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfStaticText::ActiveUpdate(const  tCIDLib::TBoolean   bNoRedraw
                                , const TGUIRegion&         grgnClip
                                , const tCIDLib::TBoolean   bInTopLayer)
{
    // If no text, do nothing
    if (strCaption().bIsEmpty())
        return;

    // We only scroll if the text is shorter than the available area
    if (m_c4TextWidth + 4 > areaActual().c4Width())
    {
        //
        //  See if it's time to scroll again yet. If so, then do it and
        //  update the time for the next scroll time (before we do it.)
        //
        const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        if (enctCur >= m_enctNextScroll)
        {
            m_enctNextScroll = enctCur + (kCIDLib::enctOneMilliSec * 50);
            const TArea& areaDraw = areaActual();
            tCIDLib::TInt4 i4Adjustment(m_c4AvgCharWidth / 6);
            if (!i4Adjustment)
                i4Adjustment = 1;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < 2; c4Index++)
            {
                m_i4LastOfs -= i4Adjustment;
                if (m_i4LastOfs < 0)
                {
                    //
                    //  We've pushed the whole thing off, so back to the far right,
                    //  and start with 1 char visible.
                    //
                    if (tCIDLib::TCard4(m_i4LastOfs * -1) > m_c4TextWidth)
                        m_i4LastOfs = tCIDLib::TInt4(areaDraw.c4Width() - m_c4AvgCharWidth);
                }

                // Redraw if we can
                if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden)
                &&  !bNoRedraw
                &&  bInTopLayer)
                {
                    //
                    //  Only invalidate the clip area, not our area, since we may be partially
                    //  outside our parent container.
                    //
                    Redraw(grgnClip);
                }
            }
        }
    }
}


tCIDLib::TVoid TCQCIntfStaticText::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_StaticText::c2StaticFmtVersion))
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
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    //  And now stream in our own stuff.
    strmToReadFrom >> m_bMarquee;

    // If V1, then force some action context
    if (c2FmtVersion == 1)
    {
        SetEventContext(kCQCKit::strEvId_OnError, tCQCKit::EActCmdCtx::OnError);
        SetEventContext(kCQCKit::strEvId_OnSuccess, tCQCKit::EActCmdCtx::NoContChange);
    }

    //
    //  If V1/2, then we need to adjust some colors, so that the F2 color is
    //  always used for shadow/fx. Currently F3 is being used for shadow and
    //  F2 for FX, and F2 is also being used for the border.
    //
    if (c2FmtVersion < 3)
    {
        // Save the original values we are going to change
        TRGBClr rgbF2 = rgbFgn2();
        TRGBClr rgbF3 = rgbFgn3();

        // Put F2 into F3, which is now the border color
        rgbFgn3(rgbF2);

        //
        //  If not in text fx mode, then move F3 to F2, so that whatever
        //  was being used for the shadow will continue to be.
        //
        if (eTextFX() == tCIDGraphDev::ETextFX::None)
            rgbFgn2(rgbF3);
    }

    // If less than V4, default the secret mode, else read it in
    if (c2FmtVersion < 4)
        m_bSecretMode = kCIDLib::False;
    else
        strmToReadFrom >> m_bSecretMode;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset the runtime members
    m_bPatternMode = kCIDLib::False;
    m_c4AvgCharWidth = 0;
    m_c4TextWidth = 0;
    m_i4LastOfs = 0;
}

tCIDLib::TVoid TCQCIntfStaticText::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out our marker and version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_StaticText::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // And do our stuff and our end marker
    strmToWriteTo   << m_bMarquee

                    // V4 stuff
                    << m_bSecretMode

                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfStaticText::Update(         TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Move in a bit more, clip and draw
    grgnClip.Deflate(3, 2);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    TArea areaDraw = grgnClip.areaBounds();
    DrawText(gdevTarget, areaInvalid, areaDraw);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticText: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfStaticText::DrawText(       TGraphDrawDev&      gdevTarget
                            , const TArea&              areaInvalid
                            ,       TArea&              areaDraw)
{
    //
    //  Now move the clip in further for the drawing part. If the text area
    //  intersects the invalid area, then draw it.
    //
    //  The 'size to' calcs above must reflect any such margins that we use
    //  here, so update those calcs if you chage this.
    //
    if (areaDraw.bIntersects(areaInvalid))
    {
        // Set the new clip on the text output area
        TRegionJanitor janClip(&gdevTarget, areaDraw, tCIDGraphDev::EClipModes::And);

        // Set our font
        TCQCIntfFontJan janFont(&gdevTarget, *this);

        //
        //  According to whether we are in marquee mode or normal mode, we
        //  will do different drawing.
        //
        //  And if we are in secret mode, we have to generate a string on
        //  the fly that has the same number of asterisks as there are chars
        //  in the current caption.
        //
        const TString& strCap = strCaption();
        const TString* pstrText;
        if (m_bSecretMode)
        {
            pstrText = &m_strSecretText;
            const tCIDLib::TCard4 c4SLen = m_strSecretText.c4Length();
            const tCIDLib::TCard4 c4CLen = strCap.c4Length();
            if (c4SLen > c4CLen)
            {
                m_strSecretText.CapAt(c4CLen);
            }
             else
            {
                const tCIDLib::TCard4 c4Add = (c4CLen - c4SLen);
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Add; c4Index++)
                    m_strSecretText.Append(L'*');
            }
        }
         else
        {
            pstrText = &strCap;
            if (strCap.bIsEmpty() && TFacCQCIntfEng::bDesMode())
                pstrText = &facCQCIntfEng().strErrText();
        }
        gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);
        if (m_bMarquee)
        {
            //
            //  We are a marquee style static text, so adjust the origin and
            //  size by the last offset that our active update callback
            //  calculated, and draw. Marquees can only be single line text,
            //  so we can just call the non-multi-line text drawing method.
            //
            areaDraw.AdjustOrg(m_i4LastOfs, 0);
            areaDraw.AdjustSize(-m_i4LastOfs, 0);
            gdevTarget.SetTextColor(rgbFgn());
            gdevTarget.DrawString
            (
                *pstrText, areaDraw, tCIDLib::EHJustify::Left, eVJustify()
            );
        }
         else if (eTextFX() != tCIDGraphDev::ETextFX::None)
        {
            civOwner().DrawTextFX
            (
                gdevTarget
                , *pstrText
                , areaDraw
                , eTextFX()
                , rgbFgn()
                , rgbFgn2()
                , eHJustify()
                , eVJustify()
                , bNoTextWrap()
                , pntOffset()
            );
        }
         else
        {
            //
            //  Now draw our text. If the shadow style is enabled, then draw
            //  first a drop version in fgn3, then draw the regular one in
            //  fgn1 in the normal position.
            //
            if (bShadow())
            {
                tCIDLib::TCard4 c4Drop = 1;
                if (c4FontHeight() > 24)
                      c4Drop = 2;
                TArea areaDrop(areaDraw);
                areaDrop.AdjustOrg(c4Drop, c4Drop);
                gdevTarget.SetTextColor(rgbFgn2());
                if (bNoTextWrap())
                {
                    gdevTarget.DrawString
                    (
                        *pstrText
                        , areaDrop
                        , eHJustify()
                        , eVJustify()
                    );
                }
                 else
                {
                    gdevTarget.DrawMText
                    (
                        *pstrText
                        , areaDrop
                        , eHJustify()
                        , eVJustify()
                        , kCIDLib::True
                    );
                }
            }

            gdevTarget.SetTextColor(rgbFgn());
            if (bNoTextWrap())
            {
                gdevTarget.DrawString
                (
                    *pstrText
                    , areaDraw
                    , eHJustify()
                    , eVJustify()
                );
            }
             else
            {
                gdevTarget.DrawMText
                (
                    *pstrText
                    , areaDraw
                    , eHJustify()
                    , eVJustify()
                    , kCIDLib::True
                );
            }
        }
    }

    // If underline is enabled, then draw a line along the bottom of the area
    if (bUnderline())
    {
        const TArea areaLine = areaActual();
        gdevTarget.DrawLine(areaLine.pntLL(), areaLine.pntLR(), rgbFgn());
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTitleText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfTitleText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfTitleText::TCQCIntfTitleText() :

    TCQCIntfStaticText
    (
        kCIDLib::False
        , CQCIntfEng_StaticText::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_TitleText)
    )
{
    // Default our font height to 24 and turn on underlining
    c4FontHeight(24);
    bUnderline(kCIDLib::True);
}

TCQCIntfTitleText::TCQCIntfTitleText(const TCQCIntfTitleText& iwdgToCopy) :

    TCQCIntfStaticText(iwdgToCopy)
{
}

TCQCIntfTitleText::~TCQCIntfTitleText()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfTitleText: Public operators
// ---------------------------------------------------------------------------
TCQCIntfTitleText&
TCQCIntfTitleText::operator=(const TCQCIntfTitleText& iwdgToAssign)
{
    if (this == &iwdgToAssign)
        return *this;

    TParent::operator=(iwdgToAssign);
    return *this;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaRepoText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMediaRepoText::TCQCIntfMediaRepoText() :

    TCQCIntfStaticText
    (
        kCIDLib::False
        , CQCIntfEng_StaticText::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_MediaRepoText)
    )
    , m_eAttr(tCQCMedia::EMTextAttrs::Title)
{
    // Default to no text wrap
    bNoTextWrap(kCIDLib::True);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/MediaRepoText");
}

TCQCIntfMediaRepoText::
TCQCIntfMediaRepoText(const TCQCIntfMediaRepoText& iwdgSrc) :

    TCQCIntfStaticText(iwdgSrc)
    , m_eAttr(iwdgSrc.m_eAttr)
    , m_strRepoMoniker(iwdgSrc.m_strRepoMoniker)
{
}

TCQCIntfMediaRepoText::~TCQCIntfMediaRepoText()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoText: Public operators
// ---------------------------------------------------------------------------
TCQCIntfMediaRepoText&
TCQCIntfMediaRepoText::operator=(const TCQCIntfMediaRepoText& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        m_eAttr = iwdgSrc.m_eAttr;
        m_strRepoMoniker = iwdgSrc.m_strRepoMoniker;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoText: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfMediaRepoText::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfMediaRepoText& iwdgOther(static_cast<const TCQCIntfMediaRepoText&>(iwdgSrc));

    // Do our own stuff
    return
    (
        (m_eAttr == iwdgOther.m_eAttr)
        && (m_strRepoMoniker == iwdgOther.m_strRepoMoniker)
    );
}


tCIDLib::TBoolean
TCQCIntfMediaRepoText::bValidateParm(const  TCQCCmd&        cmdSrc
                                    , const TCQCCmdCfg&     ccfgTar
                                    , const tCIDLib::TCard4 c4Index
                                    , const TString&        strValue
                                    ,       TString&        strErrText)
{
    // Let our parent go first
    if (!TParent::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    if ((ccfgTar.strCmdId() == kCQCMedia::strCmdId_SetTextType) && !c4Index)
    {
        // It has to be a valid member of the enumeration
        try
        {
            tCQCMedia::eXlatEMTextAttrs(strValue);
        }

        catch(...)
        {
            strErrText.LoadFromMsg(kMedErrs::errcCmd_BadTextType, facCQCMedia(), strValue);
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfMediaRepoText::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfMediaRepoText&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfMediaRepoText::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                                , const tCIDLib::TCard4     c4Index
                                , const TString&            strUserId
                                , const TString&            strEventId
                                ,       TStdVarsTar&        ctarGlobals
                                ,       tCIDLib::TBoolean&  bResult
                                ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetTextType)
    {
        m_eAttr = tCQCMedia::eXlatEMTextAttrs(ccfgToDo.piAt(0).m_strValue);
    }
     else if ((ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetColCookie)
          ||  (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetTitleCookie))
    {
        // See if we have this repo's database in our cache
        TString strDBSerialNum;
        TFacCQCMedia::TMDBPtr cptrMetaData;
        facCQCMedia().eGetMediaDB
        (
            m_strRepoMoniker, strDBSerialNum, cptrMetaData
        );

        // If we got the database, then let's set up
        TString strVal;
        if (cptrMetaData.pobjData())
        {
            // Parse the cookie so we know what to look up
            tCIDLib::TCard2         c2CatId;
            tCIDLib::TCard2         c2ColId;
            tCIDLib::TCard2         c2ItemId;
            tCIDLib::TCard2         c2TitleId;
            tCQCMedia::EMediaTypes  eMType;

            const tCQCMedia::ECookieTypes eCType = facCQCMedia().eCheckCookie
            (
                ccfgToDo.piAt(0).m_strValue
                , eMType
                , c2CatId
                , c2TitleId
                , c2ColId
                , c2ItemId
            );

            //
            //  If a title cookie, then we need to just use the first collection.
            //  Else it has to be a collection cookie.
            //
            tCIDLib::TCard4 c4ColInd;
            if (eCType == tCQCMedia::ECookieTypes::Title)
            {
                c4ColInd = 0;
            }
             else if (eCType == tCQCMedia::ECookieTypes::Collect)
            {
                CIDAssert(c2ColId != 0, L"The collection id was zero");
                c4ColInd = c2ColId - 1;
            }
             else
            {
                facCQCMedia().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kMedErrs::errcDB_BadCookieType
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                    , m_strRepoMoniker
                );

                // Make analyzer happy
                return tCQCKit::ECmdRes::Unknown;
            }

            // Look up the title set and collection, throw if either fails
            const TMediaDB& mdbData = cptrMetaData->mdbData();
            const TMediaTitleSet* pmtsSrc = mdbData.pmtsById
            (
                eMType, c2TitleId, kCIDLib::True
            );

            const TMediaCollect* pmcolCur = mdbData.pmcolById
            (
                eMType, pmtsSrc->c2ColIdAt(c4ColInd), kCIDLib::True
            );

            //
            //  According to our set attribute, pull out the appropriate
            //  value.
            //
            switch(m_eAttr)
            {
                case tCQCMedia::EMTextAttrs::Artist :
                    strVal = pmcolCur->strArtist();
                    break;

                case tCQCMedia::EMTextAttrs::AspectRatio :
                    strVal = pmcolCur->strAspectRatio();
                    break;

                case tCQCMedia::EMTextAttrs::BitDepth :
                    strVal.AppendFormatted(pmtsSrc->c4BitDepth());
                    break;

                case tCQCMedia::EMTextAttrs::BitRate :
                    strVal.AppendFormatted(pmtsSrc->c4BitRate());
                    break;

                case tCQCMedia::EMTextAttrs::Cast :
                    strVal = pmcolCur->strCast();
                    break;

                case tCQCMedia::EMTextAttrs::Channels :
                    strVal.AppendFormatted(pmtsSrc->c4Channels());
                    break;

                case tCQCMedia::EMTextAttrs::DateAdded :
                {
                    TTime tmFmt(pmtsSrc->enctAdded());
                    tmFmt.FormatToStr(strVal, L"%(m) %(D), %(Y)");
                    break;
                }

                case tCQCMedia::EMTextAttrs::Description :
                    strVal = pmcolCur->strDescr();
                    break;

                case tCQCMedia::EMTextAttrs::Label :
                    strVal = pmcolCur->strLabel();
                    break;

                case tCQCMedia::EMTextAttrs::LeadActor :
                    strVal = pmcolCur->strLeadActor();
                    break;

                case tCQCMedia::EMTextAttrs::MediaFmt :
                    strVal = pmcolCur->strMediaFormat();
                    break;

                case tCQCMedia::EMTextAttrs::Name :
                    strVal = pmcolCur->strName();
                    break;

                case tCQCMedia::EMTextAttrs::Rating :
                    strVal = pmcolCur->strRating();
                    break;

                case tCQCMedia::EMTextAttrs::RawRunTime :
                    strVal.SetFormatted(pmcolCur->c4Duration());
                    break;

                case tCQCMedia::EMTextAttrs::RunTime :
                {
                    const tCIDLib::TCard4 c4Hours = (pmcolCur->c4Duration() / 3600);
                    const tCIDLib::TCard4 c4Mins
                    (
                        (pmcolCur->c4Duration() - (c4Hours * 3600)) / 60
                    );

                    if (!c4Hours)
                    {
                        // There's no hours, so format it as xx mins
                        strVal.SetFormatted(c4Mins);
                        strVal.Append(L" mins");
                    }
                     else
                    {
                        // We have at least one hours so do H:MM format
                        strVal.SetFormatted(c4Hours);
                        strVal.Append(TLocale::chTimeSeparator());
                        if (c4Mins < 10)
                            strVal.Append(kCIDLib::chDigit0);
                        strVal.AppendFormatted(c4Mins);
                    }
                    break;
                }

                case tCQCMedia::EMTextAttrs::SampleRate :
                    strVal.AppendFormatted(pmtsSrc->c4SampleRate());
                    break;

                case tCQCMedia::EMTextAttrs::SeqNum :
                    strVal.AppendFormatted(pmtsSrc->c4SeqNum());
                    break;

                case tCQCMedia::EMTextAttrs::Title :
                    strVal = pmtsSrc->strName();

                    // If the collection name isn't the same, then append it
                    if (pmtsSrc->strName() != pmcolCur->strName())
                    {
                        strVal.Append(L" - ");
                        strVal.Append(pmcolCur->strName());
                    }
                    break;

                case tCQCMedia::EMTextAttrs::UserRating :
                    strVal.AppendFormatted(pmtsSrc->c4UserRating());
                    break;

                case tCQCMedia::EMTextAttrs::Year :
                    strVal.SetFormatted(pmcolCur->c4Year());
                    break;

                default :
                    strVal = L"???";
                    break;
            };
        }

        strCaption(strVal);

        // Force an update if not hidden
        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetRepository)
    {
        //
        //  Just store the new repository. It'll be used the next time they
        //  set a new cookie on us.
        //
        m_strRepoMoniker = ccfgToDo.piAt(0).m_strValue;
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
TCQCIntfMediaRepoText::Initialize(  TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


tCIDLib::TVoid
TCQCIntfMediaRepoText::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    //
    //  We don't support anything in an event context. The other stuff is fine
    //  since we just store the info away until the next time a cookie is set
    //  on us. And we only redraw on a new cookie if not in a preload.
    //
    if (eContext == tCQCKit::EActCmdCtx::IVEvent)
        return;

    // Set a new collection cookie
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCMedia::strCmdId_SetColCookie
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetColCookie)
            , tCQCKit::ECmdPTypes::Text
            , facCQCMedia().strMsg(kMedMsgs::midCmdP_CollectCookie)
        )
    );

    // They can associate us with another repository
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCMedia::strCmdId_SetRepository
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetRepository)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
        )
    );

    //
    //  Dynamically change what value we display. Note that the names
    //  in the enumerated limit must match the trailing parts of the
    //  actual enum we use, since we use that to translate to the actual
    //  enumerateion.
    //
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCMedia::strCmdId_SetTextType
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetTextType)
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Type)
            , L"Artist, AspectRatio, BitDepth, BitRate, Cast, Channels, DateAdded, "
              L"Description, Label, LeadActor, MediaFmt, Rating, RawRunTime, "
              L"RunTime, SampleRate, SeqNum, Title, UserRating, Year"
        )
    );

    // Set a new title cookie
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCMedia::strCmdId_SetTitleCookie
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetTitleCookie)
            , tCQCKit::ECmdPTypes::Text
            , facCQCMedia().strMsg(kMedMsgs::midCmdP_TitleCookie)
        )
    );
}


// Report any monikers we reference
tCIDLib::TVoid
TCQCIntfMediaRepoText::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    if (!m_strRepoMoniker.bIsEmpty())
    {
        tCIDLib::TBoolean bAdded;
        colToFill.objAddIfNew(m_strRepoMoniker, bAdded);
    }
}



// Return configuration summary info about us
tCIDLib::TVoid
TCQCIntfMediaRepoText::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Add our own attributes
    TString strLim;
    tCQCMedia::FormatEMTextAttrs
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    colAttrs.objPlace
    (
        L"Media Text"
        , kCQCIntfEng::strAttr_Media_Text
        , strLim
        , tCIDMData::EAttrTypes::String
    ).SetString(tCQCMedia::strXlatEMTextAttrs(m_eAttr));

    adatTmp.Set
    (
        L"Repo Moniker"
        , kCQCIntfEng::strAttr_Media_Repo
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_RepoMoniker);
    adatTmp.SetString(m_strRepoMoniker);
    colAttrs.objAdd(adatTmp);
}


// Handle search and replace at our level in the hierarchy
tCIDLib::TVoid
TCQCIntfMediaRepoText::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocDevice))
    {
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strRepoMoniker, bRegEx, bFull, regxFind
        );
    }
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfMediaRepoText::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Set our attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_Media_Repo)
        m_strRepoMoniker = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Media_Text)
        m_eAttr = tCQCMedia::eXlatEMTextAttrs(adatNew.strValue());
}


tCIDLib::TVoid TCQCIntfMediaRepoText::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCMedia::strCmdId_SetColCookie)
    {
        // Default it to the collection cookie source value
        ccfgToSet.SetParmAtAsRTV(0, kCQCMedia::strRTVId_ColCookie);
    }
     else if (ccfgToSet.strCmdId() == kCQCMedia::strCmdId_SetTitleCookie)
    {
        // Default it to the collection cookie source value
        ccfgToSet.SetParmAtAsRTV(0, kCQCMedia::strRTVId_TitleCookie);
    }
     else
    {
        // Not one of ours, pass long to our parent
        TParent::SetDefParms(ccfgToSet);
    }
}


tCIDLib::TVoid
TCQCIntfMediaRepoText::Validate(const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Make sure our referenced media repo is available
    if (m_strRepoMoniker.bIsEmpty() || !facCQCKit().bDrvIsLoaded(m_strRepoMoniker))
    {
        TString strErr
        (
            kIEngErrs::errcVal_DrvNotFound
            , facCQCIntfEng()
            , m_strRepoMoniker
        );
        colErrs.objAdd
        (
            TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::True, strWidgetId())
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoText: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCMedia::EMTextAttrs TCQCIntfMediaRepoText::eAttr() const
{
    return m_eAttr;
}

tCQCMedia::EMTextAttrs
TCQCIntfMediaRepoText::eAttr(const tCQCMedia::EMTextAttrs eToSet)
{
    m_eAttr = eToSet;
    return m_eAttr;
}


const TString& TCQCIntfMediaRepoText::strRepoMoniker() const
{
    return m_strRepoMoniker;
}

const TString& TCQCIntfMediaRepoText::strRepoMoniker(const TString& strToSet)
{
    m_strRepoMoniker = strToSet;
    return m_strRepoMoniker;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoText: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfMediaRepoText::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff starts with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_StaticText::c2MTextFmtVersion))
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

    //  And now stream in our own stuff
    strmToReadFrom >> m_eAttr >> m_strRepoMoniker;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfMediaRepoText::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_StaticText::c2MTextFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and our end marker
    strmToWriteTo   << m_eAttr
                    << m_strRepoMoniker
                    << tCIDLib::EStreamMarkers::EndObject;
}


