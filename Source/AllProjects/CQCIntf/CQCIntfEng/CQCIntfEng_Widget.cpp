//
// FILE NAME: CQCIntfEng_Widget.cpp
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
//  This file implements the core user interface widget related classes.
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
RTTIDecls(TCQCIntfWidget,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_Widget
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Persistent format version for the widget class
        //
        //  Version 2 -
        //      We got rid of the old base class in the polling engine facility that
        //      we used to derive from. Now we just derive directly from TCIDObject.
        //      So we have to read in the old base class' few pieces of persistent
        //      info if it's a V1 widget. And we got rid of some stuff that is no
        //      longer required because of the changes.
        //
        //  Version 3 -
        //      Add three new colors for use by widgets for specialized purposes.
        //
        //  Version 4 -
        //      Added the group index to support widget grouping, and the group name
        //      member.
        //
        //  Version 5 -
        //      Added a 'locked' attribute so that widgets can be locked against
        //      selection. This allows you to easily modify widgets on top of them
        //      without accidentally moving the locked widget.
        //
        //  Version 6 -
        //      Removed the old 'direct write' fields which haven't actually been
        //      used for some time. Added a new style flag that allows inertial
        //      scrolling to be supressed, so we need to initialize it off in any
        //      existing widgets.
        //
        //  Version 7 -
        //      We added support for corner rounding, which is mostly used when setting
        //      up the clip area for the widget, to create a rounded corner clip region.
        //
        //  Version 8 -
        //      Added support for anchors, which are used by the dynamic resizing stuff.
        //
        //  Version 9 - 4.4.915
        //      And we get back rid of the anchors since we decided to go with a static
        //      pre-scaling scheme instead of dynamic resizing.
        //
        //  Version 10 - 4.4.919
        //      Added the m_bNegateStates flag, which allow us to use the negation of
        //      the state evaluation if desired.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 10;


        // -----------------------------------------------------------------------
        //  The name under which we put our attributes into an attribute pack
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const   pszAttrPackKey = L"Base Attributes";
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWdgJanitor
//  PREFIX: jan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfWdgJanitor: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfWdgJanitor::TCQCIntfWdgJanitor() :

    m_piwdgToSanitize(0)
{
}

TCQCIntfWdgJanitor::TCQCIntfWdgJanitor(TCQCIntfWidget* const piwdgToSanitize) :

    m_piwdgToSanitize(piwdgToSanitize)
{
    m_piwdgToSanitize->IncRefCount();
}

TCQCIntfWdgJanitor::~TCQCIntfWdgJanitor()
{
    // Just call our own drop method
    DropRef();
}


// ---------------------------------------------------------------------------
//  TCQCIntfWdgJanitor: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfWdgJanitor::bHasRef() const
{
    return (m_piwdgToSanitize != 0);
}


//
//  If we have a current ref, we decrement the ref count. If it goes to zero,
//  we delete the widget. Then we zero our pointer.
//
tCIDLib::TVoid TCQCIntfWdgJanitor::DropRef()
{
    if (m_piwdgToSanitize)
    {
        // Make sure our pointer gets cleared, even if the delete fails
        TCQCIntfWidget* piwdgDrop = m_piwdgToSanitize;
        m_piwdgToSanitize = 0;

        piwdgDrop->DecRefCount();

        // If we've zeroed out, then delete it
        if (!piwdgDrop->c4RefCount())
        {
            try
            {
                delete piwdgDrop;
            }

            catch(...)
            {
            }
        }
    }
}

TCQCIntfWidget* TCQCIntfWdgJanitor::piwdgOrphan()
{
    TCQCIntfWidget* piwdgRet = m_piwdgToSanitize;
    piwdgRet->DecRefCount();
    m_piwdgToSanitize = 0;
    return piwdgRet;
}


// Provide const and non-const access to the widget pointer
const TCQCIntfWidget* TCQCIntfWdgJanitor::piwdgRef() const
{
    return m_piwdgToSanitize;
}

TCQCIntfWidget* TCQCIntfWdgJanitor::piwdgRef()
{
    return m_piwdgToSanitize;
}


//
//  Update us with a new widget. If we have an existing one we decrement
//  the reference count and, if it's zeroed out, we delete it.
//
//  We make sure the new one is the same as the one we have already though.
//
tCIDLib::TVoid
TCQCIntfWdgJanitor::SetNew(TCQCIntfWidget* const piwdgNew)
{
    if (piwdgNew && (piwdgNew == m_piwdgToSanitize))
    {
        //
        //  We didn't get a null, and it's equal to what we have, so we
        //  just do nothing. Presumably they passed us the same widget we
        //  already have. If we called Drop(), we could delete it before we
        //  store the new pointer.
        //
        return;
    }

    // Drop our current ref and store the new one and bump the ref
    DropRef();
    m_piwdgToSanitize = piwdgNew;
    m_piwdgToSanitize->IncRefCount();
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWidget
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfWidget::~TCQCIntfWidget()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We can do some common validation for all derivatives. They call us first. If we return
//  false they return false, else they do their own validation.
//
tCIDLib::TBoolean
TCQCIntfWidget::bValidateParm(  const   TCQCCmd&        cmdSrc
                                , const TCQCCmdCfg&     ccfgTar
                                , const tCIDLib::TCard4 c4Index
                                , const TString&        strValue
                                ,       TString&        strErrText)
{
    // For now, we are just passing it on to the command target mixin for the most basic stuff
    return MCQCCmdTarIntf::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText);
}



//
//  Handle any widget commands that we handle at this level. If it's not
//  one of ours, then assume it's bad because derived classes will have
//  checked it before calling us, so we are the end of the line.
//
tCQCKit::ECmdRes
TCQCIntfWidget::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
                        , const tCIDLib::TCard4     c4Index
                        , const TString&            strUserId
                        , const TString&            strEventId
                        ,       TStdVarsTar&        ctarGlobals
                        ,       tCIDLib::TBoolean&  bResult
                        ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_EnableBorder)
    {
        // Update the border attribute
        bHasBorder(facCQCKit().bCheckBoolVal(ccfgToDo.piAt(0).m_strValue));

        // And force a redraw if relevant
        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_GetWidgetClr)
    {
        const TString& strToGet = ccfgToDo.piAt(0).m_strValue;
        const TString& strFmt = ccfgToDo.piAt(1).m_strValue;
        const TString& strVar = ccfgToDo.piAt(2).m_strValue;

        // Get the current color
        TRGBClr rgbGet;
        if (strToGet == L"Bgn 1")
            rgbGet = m_rgbBgn1;
        else if (strToGet == L"Bgn 2")
            rgbGet = m_rgbBgn2;
        else if (strToGet == L"Fgn 1")
            rgbGet = m_rgbFgn1;
        else if (strToGet == L"Fgn 2")
            rgbGet = m_rgbFgn2;
        else if (strToGet == L"Fgn 3")
            rgbGet = m_rgbFgn3;
        else if (strToGet == L"Extra 1")
            rgbGet = m_rgbExtra1;
        else if (strToGet == L"Extra 2")
            rgbGet = m_rgbExtra2;
        else if (strToGet == L"Extra 3")
            rgbGet = m_rgbExtra3;
        else
            ThrowBadParmVal(ccfgToDo, 0);

        // Convert to the requested format
        TString strVal;
        if (strFmt == L"RGB")
        {
            rgbGet.FormatToText(strVal, kCIDLib::chSpace);
        }
         else if ((strFmt == L"HSV Integer") || (strFmt == L"HSV Float"))
        {
            THSVClr hsvFmt(rgbGet);
            hsvFmt.FormatToText(strVal, kCIDLib::chSpace, strFmt == L"HSV Integer");
        }
         else
        {
            ThrowBadParmVal(ccfgToDo, 1);
        }

        TCQCActVar& varText = TStdVarsTar::varFind
        (
            strVar, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );
        if (varText.bSetValue(strVal) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strVar, varText.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetWdgState)
    {
        //
        //  Don't redraw if we are in the preload. Otherwise, we need to force
        //  this area to redraw so as to show or hide the widget.
        //
        const tCIDLib::TBoolean bRedraw(strEventId != kCQCKit::strEvId_OnPreload);

        if (ccfgToDo.piAt(0).m_strValue == L"Hidden")
            SetDisplayState(tCQCIntfEng::EDispStates::Hidden, bRedraw);
        else if (ccfgToDo.piAt(0).m_strValue == L"Disabled")
            SetDisplayState(tCQCIntfEng::EDispStates::Disabled, bRedraw);
         else
            SetDisplayState(tCQCIntfEng::EDispStates::Normal, bRedraw);
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetWidgetClr)
    {
        const TString& strToSet = ccfgToDo.piAt(0).m_strValue;

        // Get copies of these since we might modify them
        TString strClr = ccfgToDo.piAt(1).m_strValue;
        TString strFmt = ccfgToDo.piAt(2).m_strValue;

        TRGBClr rgbNew;
        if (strFmt == L"RGB")
        {
            if (!rgbNew.bParseFromText(strClr, tCIDLib::ERadices::Auto, L' '))
                ThrowBadParmVal(ccfgToDo, 1);
        }
         else if ((strFmt == L"HSV Integer")
              ||  (strFmt == L"HSV Float")
              ||  (strFmt == L"HS Integer")
              ||  (strFmt == L"V Integer"))
        {
            //
            //  Set zeros for the unprovided values in the partial formats. Then we
            //  can use the standard parsing below.
            //
            if (strFmt == L"HS Integer")
            {
                strClr.Append(L" 0");
                strFmt = L"HSV Integer";
            }
             else if (strFmt == L"V Integer")
            {
                strClr.Prepend(L"0 0 ");
                strFmt = L"HSV Integer";
            }

            THSVClr hsvNew;
            tCIDLib::TBoolean bRes = hsvNew.bParseFromText
            (
                strClr, strFmt != L"HSV Float", tCIDLib::ERadices::Auto, L' '
            );

            if (!bRes)
                ThrowBadParmVal(ccfgToDo, 1);

            //
            //  If it was one of the partials, then we need to keep some parts of the
            //  current value. WE HAVE TO CHECK the original parameter, since we
            //  modified the copy above potentially.
            //
            if ((ccfgToDo.piAt(2).m_strValue == L"HS Integer")
            ||  (ccfgToDo.piAt(2).m_strValue == L"V Integer"))
            {
                // Set up the current value as HSV
                THSVClr hsvPrev;

                if (strToSet == L"Bgn 1")
                    hsvPrev.FromRGB(m_rgbBgn1);
                else if (strToSet == L"Bgn 2")
                    hsvPrev.FromRGB(m_rgbBgn2);
                else if (strToSet == L"Fgn 1")
                    hsvPrev.FromRGB(m_rgbFgn1);
                else if (strToSet == L"Fgn 2")
                    hsvPrev.FromRGB(m_rgbFgn2);
                else if (strToSet == L"Fgn 3")
                    hsvPrev.FromRGB(m_rgbFgn3);
                else if (strToSet == L"Extra 1")
                    hsvPrev.FromRGB(m_rgbExtra1);
                else if (strToSet == L"Extra 2")
                    hsvPrev.FromRGB(m_rgbExtra2);
                else if (strToSet == L"Extra 3")
                    hsvPrev.FromRGB(m_rgbExtra3);
                else
                    ThrowBadParmVal(ccfgToDo, 0);

                // And copy over the bits that were not set
                if (ccfgToDo.piAt(2).m_strValue == L"HS Integer")
                {
                    // Keep the V part
                    hsvNew.f4Value(hsvPrev.f4Value());
                }
                 else if (ccfgToDo.piAt(2).m_strValue == L"V Integer")
                {
                    // Keep the HS parts
                    hsvNew.f4Hue(hsvPrev.f4Hue());
                    hsvNew.f4Saturation(hsvPrev.f4Saturation());
                }
            }

            // Convert the HSV to RGB
            hsvNew.ToRGB(rgbNew);
        }
         else
        {
            ThrowBadParmVal(ccfgToDo, 2);
        }

        if (strToSet == L"Bgn 1")
            m_rgbBgn1 = rgbNew;
        else if (strToSet == L"Bgn 2")
            m_rgbBgn2 = rgbNew;
        else if (strToSet == L"Fgn 1")
            m_rgbFgn1 = rgbNew;
        else if (strToSet == L"Fgn 2")
            m_rgbFgn2 = rgbNew;
        else if (strToSet == L"Fgn 3")
            m_rgbFgn3 = rgbNew;
        else if (strToSet == L"Extra 1")
            m_rgbExtra1 = rgbNew;
        else if (strToSet == L"Extra 2")
            m_rgbExtra2 = rgbNew;
        else if (strToSet == L"Extra 3")
            m_rgbExtra3 = rgbNew;
        else
            ThrowBadParmVal(ccfgToDo, 0);

        // If not in the preload and not hidden, force a redraw
        if (bCanRedraw(strEventId))
            Redraw();
    }
     else
    {
        // No more parent's to call, so it's unknown
        ThrowUnknownCmd(ccfgToDo);
    }
    return tCQCKit::ECmdRes::Ok;
}


//
//  Add in the commands that we support at this level. Each class level calls
//  down the hierarchy to get all of the commands supported for a given
//  derived type.
//
//  These can be called any time, even on preload. We check above and don't
//  try to redraw if on preload, we just store the new state.
//
tCIDLib::TVoid
TCQCIntfWidget::QueryCmds(          TCollection<TCQCCmd>&   colToFill
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Enable/disable the border attribute
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_EnableBorder
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_EnableBorder)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            )
        );
    }

    //
    //  Get one of the widget colors. This is a complicated one since each widget can
    //  update the names of the colors (as presented in the attribute editor) to reflect
    //  what that colors means for it (and remove ones it doesn't use.) We want to do the
    //  same here, to make it clear to the user what colors are available and what they
    //  are for.
    //
    //  So we query the supported colors, and set up the command parameter to use its
    //  ability to support both display and internal values.
    //
    {
        tCIDLib::TKVPList colAvail;
        QuerySupportedClrs(colAvail);

        // Sort by display value so that they will be in that order to the user
        colAvail.Sort(TKeyValuePair::eCompKeyI);

        const TString strSep(L", ");
        TString strIntVals;
        TString strDisplayVals;
        const tCIDLib::TCard4 c4Count = colAvail.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (c4Index)
            {
                strIntVals.Append(strSep);
                strDisplayVals.Append(strSep);
            }

            const TKeyValuePair& kvalCur = colAvail[c4Index];
            strIntVals.Append(kvalCur.strValue());
            strDisplayVals.Append(kvalCur.strKey());
        }


        TCQCCmd ccmdNew
        (
            kCQCIntfEng::strCmdId_GetWidgetClr
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_GetWidgetClr)
            , 3
        );

        ccmdNew.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_WidgetColor), strIntVals, strDisplayVals
            )
        );

        ccmdNew.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Format)
                , L"RGB, HSV Float, HSV Integer"
            )
        );

        ccmdNew.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        colToFill.objAdd(ccmdNew);
    }

    // Hide/show the widget
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetWdgState
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetWdgState)
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_WdgState)
                , L"Hidden, Disabled, Normal"
            )
        );
    }

    // Set one of the widget colors
    {
        TCQCCmd ccmdNew
        (
            kCQCIntfEng::strCmdId_SetWidgetClr
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetWidgetClr)
            , 3
        );

        ccmdNew.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_WidgetColor)
                , L"Bgn 1, Bgn 2, Fgn 1, Fgn 2, Fgn 3, Extra 1, Extra 2, Extra 3"
            )
        );

        ccmdNew.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Color)
                , tCQCKit::ECmdPTypes::Color
            )
        );

        ccmdNew.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Format)
                , L"RGB, HSV Float, HSV Integer, HS Integer, V Integer"
            )
        );

        colToFill.objAdd(ccmdNew);
    }
}


// At this level, we don't have to do anything right now
tCIDLib::TVoid TCQCIntfWidget::RefreshImages(TDataSrvClient&)
{
}


tCIDLib::TVoid
TCQCIntfWidget::Replace(const   tCQCIntfEng::ERepFlags  eToRep
                        , const TString&                strSrc
                        , const TString&                strTar
                        , const tCIDLib::TBoolean       bRegEx
                        , const tCIDLib::TBoolean       bFull
                        ,       TRegEx&                 regxFind)
{
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Caption))
    {
        const tCIDLib::TBoolean bRes = facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strCaption, bRegEx, bFull, regxFind
        );

        if (bRes)
            CaptionChanged();
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  Allows each widget type to indicate whether it can support the 'size
//  to contents' operation. By default say no.
//
tCIDLib::TBoolean TCQCIntfWidget::bCanSizeTo() const
{
    return kCIDLib::False;
}


//
//  Edit time stuff doesn't matter here. This is used to decide if changes have been
//  made that needs to be saved, i.e. stuff that will affect the widget as it is
//  at viewing time.
//
tCIDLib::TBoolean TCQCIntfWidget::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    //
    //  If debugging, make sure that the two objects are the same runtime type,
    //  so that all of the derived classes can make this assumption. They have to
    //  call us first so we'll throw if anything is wrong. And they should anyway since
    //  we compare the core stuff.
    //
    CIDAssert
    (
        iwdgSrc.clsIsA() == clsIsA()
        , L"The src widget was of a different type than this one"
    );

    if (&iwdgSrc != this)
    {
        if ((m_areaRelative != iwdgSrc.m_areaRelative)
        ||  (m_bNegateStates != iwdgSrc.m_bNegateStates)
        ||  (m_bSingleLine != iwdgSrc.m_bSingleLine)
        ||  (m_bUseStateDisplay != iwdgSrc.m_bUseStateDisplay)
        ||  (m_c4GroupId != iwdgSrc.m_c4GroupId)
        ||  (m_c4Rounding != iwdgSrc.m_c4Rounding)
        ||  (m_c4StyleFlags != iwdgSrc.m_c4StyleFlags)
        ||  (m_eBgnStyle != iwdgSrc.m_eBgnStyle)
        ||  (m_eLogOp != iwdgSrc.m_eLogOp)
        ||  (m_bLocked != iwdgSrc.m_bLocked)
        ||  (m_eStateDisplay != iwdgSrc.m_eStateDisplay)
        ||  (m_strWidgetId != iwdgSrc.m_strWidgetId)
        ||  (m_strCaption != iwdgSrc.m_strCaption)
        ||  (m_rgbBgn1 != iwdgSrc.m_rgbBgn1)
        ||  (m_rgbBgn2 != iwdgSrc.m_rgbBgn2)
        ||  (m_rgbFgn1 != iwdgSrc.m_rgbFgn1)
        ||  (m_rgbFgn2 != iwdgSrc.m_rgbFgn2)
        ||  (m_rgbFgn3 != iwdgSrc.m_rgbFgn3)
        ||  (m_rgbExtra1 != iwdgSrc.m_rgbExtra1)
        ||  (m_rgbExtra2 != iwdgSrc.m_rgbExtra2)
        ||  (m_rgbExtra3 != iwdgSrc.m_rgbExtra3))
        {
            return kCIDLib::False;
        }

        // If we are in a group then the group name has to be the same
        if (m_c4GroupId && (m_strGroupName != iwdgSrc.m_strGroupName))
            return kCIDLib::False;

        // Compare the state names
        const tCIDLib::TCard4 c4StateCnt = m_colStateNames.c4ElemCount();
        if (c4StateCnt != iwdgSrc.m_colStateNames.c4ElemCount())
            return kCIDLib::False;

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4StateCnt; c4Index++)
        {
            if (m_colStateNames[c4Index] != iwdgSrc.m_colStateNames[c4Index])
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  The vast majority of widgets have no internal focal movement, so we
//  provide this default impl that just returns true to say that the focus
//  can be moved off of us if we have it.
//
tCIDLib::TBoolean TCQCIntfWidget::bMoveFocus(const tCQCKit::EScrNavOps)
{
    return kCIDLib::True;
}


//
//  If not overridden, then we return disable for all gestures, and indicate not
//  to let it propogate to the parent container. This is only called for widgets
//  that handle gestures, so they should have to explicitly allow propogation.
//
tCIDLib::TBoolean
TCQCIntfWidget::bPerGestOpts(const  TPoint&
                            , const tCIDLib::EDirs
                            , const tCIDLib::TBoolean
                            ,       tCIDCtrls::EGestOpts& eToSet
                            ,       tCIDLib::TFloat4&   f4VScale)
{
    eToSet = tCIDCtrls::EGestOpts::Disable;
    f4VScale = 1.0F;

    // Let it propogate to the parent
    return kCIDLib::True;
}



//
//  If this widget is the target of a (non-flick type) gesture, this is
//  called to allow us to react to it. We do nothing here by default and should
//  never get called since only widgets that react to such gestures should
//  ever end up being the target of them.
//
tCIDLib::TBoolean
TCQCIntfWidget::bProcessGestEv( const   tCIDCtrls::EGestEvs
                                , const TPoint&
                                , const TPoint&
                                , const TPoint&
                                , const tCIDLib::TBoolean)
{
    return kCIDLib::False;
}


//
//  At this level, we see if this widget implements the command source mixin interface.
//  If so, then it can send commands and might reference the target in one of those
//  commands. So we ask the command source to check the id.
//
//  Most widgets will be OK with just letting us handle this. But if they have embedded
//  actions (like templates or tool bars with their buttons), they need to call us first
//  and then check their own actions if we return false.
//
//  Any widget that implements c4QueryEmbeddedCmdSrcs() should override this since that
//  means he has other command sources that he manages other than just himself.
//
//  Derived classes should call us even if they don't mix in the command source interface,
//  just to be safe for future changes.
//
tCIDLib::TBoolean TCQCIntfWidget::bReferencesTarId(const TString& strId) const
{
    const MCQCCmdSrcIntf* pmcsrcUs = dynamic_cast<const MCQCCmdSrcIntf*>(this);
    if (pmcsrcUs)
        return pmcsrcUs->bReferencesTarId(strId);

    return kCIDLib::False;
}


//
//  Some widgets are not command sources, but they contain a list of them.
//  So we have ot have some way to get at those. Those widgets will override
//  this method and give back a list of pointers to their command srces.
//  They should insure that the passed collection is not adopting!
//
tCIDLib::TCard4 TCQCIntfWidget::c4QueryEmbeddedCmdSrcs(tCQCKit::TCmdSrcList&)
{
    // Default is to do nothing
    return 0;
}


//
//  No particular need for our derivatives to handle these which are called
//  at the start of an action and at the end. So we just no-op them on
//  behalf of our derivatives. They can override them if needed, but not very
//  likely.
//
tCIDLib::TVoid TCQCIntfWidget::CmdTarCleanup()
{
}

tCIDLib::TVoid TCQCIntfWidget::CmdTarInitialize(const TCQCActEngine&)
{
}


// If the widget doesn't handing clicks, we just do nothing
tCIDLib::TVoid TCQCIntfWidget::Clicked(const TPoint&)
{
}


//
//  Called between each gesture inertial move message, mostly to give the target
//  a chance to draw new content ahead of upcoming inertial movement.
//
tCIDLib::TVoid
TCQCIntfWidget::GestInertiaIdle(const   tCIDLib::TEncodedTime   enctEnd
                                , const tCIDLib::TCard4         c4Millis
                                , const tCIDLib::EDirs          eDir)
{

}


//
//  This is called on all widgets in a template when the template is being
//  loaded, to let us initialize. We just have to deal with states at this
//  point.
//
tCIDLib::TVoid
TCQCIntfWidget::Initialize( TCQCIntfContainer* const    piwdgParent
                            , TDataSrvClient&
                            , tCQCIntfEng::TErrList&)
{
    // Store the owner window and parent container
    m_piwdgParent = piwdgParent;

    //
    //  In designer mode, we override any initially invisible setting,
    //  which will have caused the widget to set itself invisible during
    //  the streaming in. This won't change the initially invisible
    //  setting, it'll just put the widget's runtime visiblity state
    //  back to normally visible.
    //
    if (TFacCQCIntfEng::bDesMode())
        SetDisplayState(tCQCIntfEng::EDispStates::Normal, kCIDLib::False);

    //
    //  We need to see if we have any states. If so, we have to look up their indices and
    //  store them away. If any states aren't found, then we remove those states. Clear the
    //  index list first. If we never add any, then it remains empty.
    //
    LookupStates(piwdgParent);
}


// Cause an area of the area of this widget to be redrawn
tCIDLib::TVoid TCQCIntfWidget::Invalidate()
{
    m_pcivOwner->InvalidateArea(m_areaActual);
}

tCIDLib::TVoid TCQCIntfWidget::Invalidate(const TArea& areaTarget)
{
    m_pcivOwner->InvalidateArea(areaTarget);
}


tCIDLib::TVoid TCQCIntfWidget::Invoke()
{
    // Default is is to do nothing
}



//
//  This is called after all widgets are created and initialized, to allow
//  them to do any initial action or data loading or setup that might
//  require accessing other widgets.
//
tCIDLib::TVoid TCQCIntfWidget::PostInit()
{
    // Default is is to do nothing
}


//
//  If not overridden, we do nothing. Generally we shouldn't get called here since
//  they whoudln't enable the flick if they weren't prepared to deal with it.
//
tCIDLib::TVoid
TCQCIntfWidget::ProcessFlick(const  tCIDLib::EDirs  eDirection
                            , const TPoint&         pntStartPos)
{
}


// If the widget can't report it's content size, this default does nothing
tCIDLib::TVoid TCQCIntfWidget::QueryContentSize(TSize&)
{
    // Default is to do nothing
}


//
//  All widgets that references fields or drivers must report the monikers
//  that they reference. Most don't make such references, so we proivde an
//  empty default.
//
tCIDLib::TVoid
TCQCIntfWidget::QueryMonikers(tCIDLib::TStrHashSet&) const
{
    // Default is to do nothing
}


//
//  Most widgets don't have any configured images, so we provide an empty
//  implementation for them.
//
tCIDLib::TVoid TCQCIntfWidget::
QueryReferencedImgs(        tCIDLib::TStrHashSet&
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean
                    , const tCIDLib::TBoolean) const
{
}


//
//  At this level we load up the editable attributes that we control here at
//  the widget level. We add all of them. Derived classes can disable any that
//  are not relevant to them.
//
//  NOTE THAT we do not add the caption attribute. Derived classes will add that if it
//  it is valid for them. But we do handle it in SetAttr and store the caption text
//  away on behalf of the derived class (though they might still intercept and do
//  other things with the set text as well.)
//
tCIDLib::TVoid
TCQCIntfWidget::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    TString strLim;
    TString strVal;

    // -------------------------
    //  Do the base attributes
    // -------------------------
    colAttrs.objPlace
    (
        L"Base Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    //
    //  For most the id isn't required. Derived classes could set the limits to
    //  make it required if needed.
    //
    colAttrs.objPlace
    (
        L"Widget Id", kCQCIntfEng::strAttr_Base_WidgetId, tCIDMData::EAttrTypes::String
    ).SetString(m_strWidgetId);

    // A read-only one for whether this guy is in a group or not
    colAttrs.objPlace
    (
        L"Is Grouped"
        , kCQCIntfEng::strAttr_Base_Grouped
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::None
    ).SetBool(m_c4GroupId != 0);

    // If this guy is the group root, then show the group name.
    if (m_c4GroupId != 0)
    {
        // Get the z-order index of the first of our group and get our own z-order
        const tCIDLib::TCard4 c4FirstGrpZ = m_piwdgParent->c4FindFirstGroupZ(m_c4GroupId);
        const tCIDLib::TCard4 c4OurZ = m_piwdgParent->c4WdgZOrder(this, kCIDLib::False);

        // If they are the same, we are the root
        if (c4FirstGrpZ == c4OurZ)
        {
            colAttrs.objPlace
            (
                L"Group Name"
                , kCQCIntfEng::strAttr_Base_GroupName
                , tCIDMData::EAttrTypes::String
            ).SetString(m_strGroupName);
        }
    }

    //
    //  If we are a template, then set a size limit. Else, set an area limit with the
    //  limit being the area of the parent template.
    //
    if (bIsA(TCQCIntfTemplate::clsThis()))
    {
        // Get the area of the contained widgets, which is our minimum
        const TCQCIntfTemplate* piwdgTmpl = static_cast<const TCQCIntfTemplate*>(this);
        TSize szMin = piwdgTmpl->areaHull().szArea();

        // And in case we are empty, set a hard min minimum
        szMin.TakeLarger(TSize(8, 8));

        // Set a large but arbitrary upper limit
        const TSize szMax(0x4000, 0x4000);

        strLim = kCIDMData::strAttrLim_SizePref;
        szMin.FormatToText(strLim, tCIDLib::ERadices::Dec, kCIDLib::chComma, kCIDLib::True);
        strLim.Append(kCIDLib::chForwardSlash);
        szMax.FormatToText(strLim, tCIDLib::ERadices::Dec, kCIDLib::chComma, kCIDLib::True);

        colAttrs.objPlace
        (
            L"Size", kCQCIntfEng::strAttr_Base_Size, strLim, tCIDMData::EAttrTypes::Size
        ).SetSize(m_areaActual.szArea());
    }
     else
    {
        // Get the area of the containing template as the limit
        const TCQCIntfTemplate& iwdgPar = m_pcivOwner->iwdgBaseTmpl();
        strLim = kCIDMData::strAttrLim_AreaPref;
        iwdgPar.areaActual().FormatToText(strLim, kCIDLib::chComma, kCIDLib::True);

        colAttrs.objPlace
        (
            L"Area", kCQCIntfEng::strAttr_Base_Area, strLim, tCIDMData::EAttrTypes::Area
        ).SetArea(m_areaActual);
    }

    // We need to format out limits fo rthis one
    tCQCIntfEng::FormatEBgnStyles
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    colAttrs.objPlace
    (
        L"Bgn Fill"
        , kCQCIntfEng::strAttr_Base_BgnFill
        , strLim
        , tCIDMData::EAttrTypes::String
    ).SetString(tCQCIntfEng::strXlatEBgnStyles(m_eBgnStyle));

    colAttrs.objPlace
    (
        L"Border", kCQCIntfEng::strAttr_Base_Border, tCIDMData::EAttrTypes::Bool
    ).SetBool(bHasBorder());

    colAttrs.objPlace
    (
        L"Init Invisible"
        , kCQCIntfEng::strAttr_Base_InitInvisible
        , tCIDMData::EAttrTypes::Bool
    ).SetBool(bInitInvisible());

    colAttrs.objPlace
    (
        L"Locked", kCQCIntfEng::strAttr_Base_Locked, tCIDMData::EAttrTypes::Bool
    ).SetBool(bIsLocked());

    colAttrs.objPlace
    (
        L"No Inertia", kCQCIntfEng::strAttr_Base_NoInertia, tCIDMData::EAttrTypes::Bool
    ).SetBool(bNoInertia());

    colAttrs.objPlace
    (
        L"Rounding"
        , kCQCIntfEng::strAttr_Base_Rounding
        , L"Range: 0, 14"
        , tCIDMData::EAttrTypes::Card
    ).SetCard(c4Rounding());

    colAttrs.objPlace
    (
        L"Shadow", kCQCIntfEng::strAttr_Base_Shadow, tCIDMData::EAttrTypes::Bool
    ).SetBool(bShadow());

    colAttrs.objPlace
    (
        L"Trans. Bgn", kCQCIntfEng::strAttr_Base_Transparent, tCIDMData::EAttrTypes::Bool
    ).SetBool(bIsTransparent());

    colAttrs.objPlace
    (
        L"Underline", kCQCIntfEng::strAttr_Base_Underline, tCIDMData::EAttrTypes::Bool
    ).SetBool(bUnderline());

    // -------------------------
    //  Do the standard colors. Not all of these will be used by every widget.
    //  The derived class will remove any it doesn't need. For those it does
    //  keep, it will update the display name with one that indicates the real
    //  usage of the color, though we provide some defaults that are commonly
    //  used.
    // -------------------------
    colAttrs.objPlace
    (
        L"Colors Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    colAttrs.objPlace
    (
        L"Fill 1", kCQCIntfEng::strAttr_Clr_Bgn1, tCIDMData::EAttrTypes::RGBColor
    ).SetRGBColor(m_rgbBgn1);

    colAttrs.objPlace
    (
        L"Fill 2", kCQCIntfEng::strAttr_Clr_Bgn2, tCIDMData::EAttrTypes::RGBColor
    ).SetRGBColor(m_rgbBgn2);

    colAttrs.objPlace
    (
        L"Fgn 1", kCQCIntfEng::strAttr_Clr_Fgn1, tCIDMData::EAttrTypes::RGBColor
    ).SetRGBColor(m_rgbFgn1);

    colAttrs.objPlace
    (
        L"Fgn 2", kCQCIntfEng::strAttr_Clr_Fgn2, tCIDMData::EAttrTypes::RGBColor
    ).SetRGBColor(m_rgbFgn2);

    colAttrs.objPlace
    (
        L"Border", kCQCIntfEng::strAttr_Clr_Fgn3, tCIDMData::EAttrTypes::RGBColor
    ).SetRGBColor(m_rgbFgn3);

    colAttrs.objPlace
    (
        L"Extra 1", kCQCIntfEng::strAttr_Clr_Extra1, tCIDMData::EAttrTypes::RGBColor
    ).SetRGBColor(m_rgbExtra1);

    colAttrs.objPlace
    (
        L"Extra 2", kCQCIntfEng::strAttr_Clr_Extra2, tCIDMData::EAttrTypes::RGBColor
    ).SetRGBColor(m_rgbExtra2);

    colAttrs.objPlace
    (
        L"Extra 3", kCQCIntfEng::strAttr_Clr_Extra3, tCIDMData::EAttrTypes::RGBColor
    ).SetRGBColor(m_rgbExtra3);


    //
    //  As long as it's not a template itself, they can can all optionally support
    //  widget states.
    //
    if (clsIsA() != TCQCIntfTemplate::clsThis())
    {
        colAttrs.objPlace
        (
            L"States:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
        );

        // See if we have any states assigned
        const tCIDLib::TBoolean bHaveStates = !m_colStateNames.bIsEmpty();

        // Flatten out the state names and add those
        {
            TBinMBufOutStream strmOut(1024);
            strmOut << m_colStateNames << kCIDLib::FlushIt;
            colAttrs.objPlace
            (
                L"Associated States"
                , kCQCIntfEng::strAttr_Widget_States
                , strmOut.mbufData()
                , strmOut.c4CurPos()
                , TString::strEmpty()
                , tCIDMData::EAttrEdTypes::Visual
            );
        }

        colAttrs.objPlace
        (
            L"Negate"
            , kCQCIntfEng::strAttr_Widget_StatesNeg
            , tCIDMData::EAttrTypes::Bool
            , bHaveStates ? tCIDMData::EAttrEdTypes::Both : tCIDMData::EAttrEdTypes::None
        ).SetBool(m_bNegateStates);

        tCQCKit::FormatELogOps(strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma);
        colAttrs.objPlace
        (
            L"True If"
            , kCQCIntfEng::strAttr_Widget_StatesLogOp
            , strLim
            , tCIDMData::EAttrTypes::String
            , bHaveStates ? tCIDMData::EAttrEdTypes::Both : tCIDMData::EAttrEdTypes::None
        ).SetString(tCQCKit::strAltXlatELogOps(m_eLogOp));

        //
        //  We only use two of the three display state values for this. The Normal state
        //  makes no sense for this purpose.
        //
        strLim = L"Enum: ";
        strLim.Append(tCQCIntfEng::strXlatEDispStates(tCQCIntfEng::EDispStates::Disabled));
        strLim.Append(L",");
        strLim.Append(tCQCIntfEng::strXlatEDispStates(tCQCIntfEng::EDispStates::Hidden));
        TAttrData& adatDisplay = colAttrs.objPlace
        (
            L"If True:"
            , kCQCIntfEng::strAttr_Widget_StatesDisplay
            , strLim
            , tCIDMData::EAttrTypes::String
            , bHaveStates ? tCIDMData::EAttrEdTypes::Both : tCIDMData::EAttrEdTypes::None
        );
        if (m_eStateDisplay == tCQCIntfEng::EDispStates::Hidden)
            adatDisplay.SetString(tCQCIntfEng::strXlatEDispStates(tCQCIntfEng::EDispStates::Hidden));
        else
            adatDisplay.SetString(tCQCIntfEng::strXlatEDispStates(tCQCIntfEng::EDispStates::Disabled));
    }
}


//
//  This is called after template load, to let all of the users of fields
//  examine their configured fields and see if they still exist, and if
//  so if they've changed in some way that makes them no longer usable.
//
//  This step gets them onto the poll list so that it'll start getting the
//  value.
//
tCIDLib::TVoid
TCQCIntfWidget::RegisterFields(         TCQCPollEngine& polleToUse
                                , const TCQCFldCache&   cfcData)
{
    // Default at this level is to do nothing
}



// Sets our attributes (the base ones for all widgets) back to defaults
tCIDLib::TVoid TCQCIntfWidget::ResetWidget()
{
    //
    //  We have to do this from more than one place, so we have a private.
    //  Some clients don't want to call this, because this is a virtual
    //  and it could affect derived classes.
    //
    SetDefAttrs(0);
}


//
//  When the designer is re-scaling templates to support other resolutions, it will
//  call this method. Unlike normal resizing, which for many widgets doesn't affect
//  the internal contents since they are scrollable and they just show more or less
//  info. But some want to scale some internal setup to make them look right when
//  scaled.
//
//  The default is to do nothing.
//
tCIDLib::TVoid
TCQCIntfWidget::ScaleInternal(  const   tCIDLib::TFloat8
                                , const tCIDLib::TFloat8 )
{
}


//
//  Update ourself with the new attributes during editing.
//
//  All value are validated, so we should be able to assume here that they will have
//  legal values.
//
//  Though our QueryWdgAttrs method doesn't add the caption text, we do handle it here
//  on behalf of any derived classes that add it.
//
tCIDLib::TVoid
TCQCIntfWidget::SetWdgAttr(TAttrEditWnd& wndAttrEd, const TAttrData& adatNew, const TAttrData&)
{
    // ------------------------------
    //  Base attributes
    // ------------------------------
     // Use the setter method because it does important stuff
    if (adatNew.strId() == kCQCIntfEng::strAttr_Base_Area)
        SetArea(adatNew.areaVal());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_BgnFill)
        m_eBgnStyle = tCQCIntfEng::eXlatEBgnStyles(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_Border)
        bHasBorder(adatNew.bVal());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_Locked)
        bIsLocked(adatNew.bVal());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_InitInvisible)
        bInitInvisible(adatNew.bVal());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_NoInertia)
        bNoInertia(adatNew.bVal());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_Rounding)
        c4Rounding(adatNew.c4Val());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_Shadow)
        bShadow(adatNew.bVal());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_Transparent)
        bIsTransparent(adatNew.bVal());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_Underline)
        bUnderline(adatNew.bVal());

    // ------------------------------
    //  We'll only get this is this guy is the root member of a group
    // ------------------------------
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_GroupName)
        strGroupName(adatNew.strValue());

    // ------------------------------
    //  The standard widget colors. The derived class will remove those that are
    //  not applicable to it, but that just means we will only see those here.
    // ------------------------------
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Clr_Bgn1)
        m_rgbBgn1 = adatNew.clrVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Clr_Bgn2)
        m_rgbBgn2 = adatNew.clrVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Clr_Fgn1)
        m_rgbFgn1 = adatNew.clrVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Clr_Fgn2)
        m_rgbFgn2 = adatNew.clrVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Clr_Fgn3)
        m_rgbFgn3 = adatNew.clrVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Clr_Extra1)
        m_rgbExtra1 = adatNew.clrVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Clr_Extra2)
        m_rgbExtra2 = adatNew.clrVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Clr_Extra3)
        m_rgbExtra3 = adatNew.clrVal();


    // ------------------------------
    //  The caption text (see method comments above.) Use the setter method since it
    //  does important stuff
    // ------------------------------
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Widget_Caption)
        strCaption(adatNew.strValue());


    // The states stuff is more complex with some interactive options
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Widget_StatesLogOp)
        m_eLogOp = tCQCKit::eAltXlatELogOps(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Widget_StatesNeg)
        m_bNegateStates = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Widget_States)
    {
        // We have to stream in the state names
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        strmSrc >> m_colStateNames;
    }
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_Size)
    {
        SetSize(adatNew.szVal());
    }
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Widget_StatesDisplay)
        m_eStateDisplay = tCQCIntfEng::eXlatEDispStates(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Base_WidgetId)
        strWidgetId(adatNew.strValue());
}


//
//  Before a popup is closed or the base template is replaced, all the widgets are
//  given a chance to clean up if they have special needs. The web browser widget
//  is one example. It generally wants to kill the embedded browser before the
//  termination is done, because it will interfere with fade out effects and whatnot.
//
//  But most won't bother to do anything with this.
//
tCIDLib::TVoid TCQCIntfWidget::Terminate()
{
}


tCIDLib::TVoid
TCQCIntfWidget::Validate(const  TCQCFldCache&
                        ,       tCQCIntfEng::TErrList& colErrs
                        ,       TDataSrvClient&) const
{
    // If we have any states, make sure they exist
    if (!m_colStateNames.bIsEmpty())
    {
        const TCQCIntfTemplate& iwdgPar = civOwner().iwdgBaseTmpl();

        const tCIDLib::TCard4 c4Count = m_colStateNames.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (!iwdgPar.bStateExists(m_colStateNames[c4Index]))
            {
                colErrs.objAdd
                (
                    TCQCIntfValErrInfo
                    (
                        c4UniqueId()
                        , L"Template states are referenced that no longer exist in the parent template"
                        , kCIDLib::True
                        , strWidgetId()
                    )
                );
                break;
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get the actual area (where it is as created) of the widget
const TArea& TCQCIntfWidget::areaActual() const
{
    return m_areaActual;
}


// Get our original area, before any dynamic resizing
const TArea& TCQCIntfWidget::areaOrg() const
{
    return m_areaOrg;
}


// Get the relative area, relative to our parent
const TArea& TCQCIntfWidget::areaRelative() const
{
    return m_areaRelative;
}


// Add a template state to our list of states we monitor
tCIDLib::TVoid TCQCIntfWidget::AddState(const TString& strToAdd)
{
    m_colStateNames.objAdd(strToAdd);
}


// A convenience to check the focus capabilities flag
tCIDLib::TBoolean TCQCIntfWidget::bAcceptsFocus() const
{
    return tCIDLib::bAnyBitsOn(m_eCapFlags, tCQCIntfEng::ECapFlags::TakesFocus);
}


//
//  A helper that saves a lot of grunt work, when widgets are responding to
//  commands. They only want to redraw if the widget is not hidden and if
//  the event they are responding to is not happening in a preload event.
//
tCIDLib::TBoolean TCQCIntfWidget::bCanRedraw(const TString& strEventId) const
{
    return
    (
        (eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden)
        && (strEventId != kCQCKit::strEvId_OnPreload)
    );
}


//
//  We check to see if all of the states we reference are in the passed list. The list
//  can be null if none have been set, in which case we just add all our states to the
//  list of bad states.
//
tCIDLib::TBoolean
TCQCIntfWidget::bCheckStateRefs(const   TCQCIntfStateList* const    pistlStates
                                ,       tCIDLib::TStrHashSet&       colBadList) const
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    tCIDLib::TCard4 c4At;
    const tCIDLib::TCard4 c4Count = m_colStateNames.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TString& strCur = m_colStateNames[c4Index];
        if (!pistlStates || !pistlStates->bFindState(strCur, c4At))
        {
            // If not already in the list, then add it
            if (!colBadList.bHasElement(strCur))
            {
                colBadList.objAdd(strCur);
                bRet = kCIDLib::False;
            }
        }
    }
    return bRet;
}


// Check whether this widget's actual area contains the passed point
tCIDLib::TBoolean
TCQCIntfWidget::bContainsPoint(const TPoint& pntToTest) const
{
    return m_areaActual.bContainsPoint(pntToTest);
}


// Get whether this widget indicates it can handle gestures
tCIDLib::TBoolean TCQCIntfWidget::bDoesGestures() const
{
    return tCIDLib::bAnyBitsOn(m_eCapFlags, tCQCIntfEng::ECapFlags::DoesGestures);
}



//
//  Normally, during drawing, each container overlay adds its clip area
//  to the clip region, so that any widgets inside them are automatically
//  restricted to the most restrictive clip area of all containers up its
//  parentage chain.
//
//  But, sometimes, special case stuff like scrolling or drawing that is
//  done practively during user interaction and whatnot need to set the
//  most restrictive clip before they draw. So they need to figure out
//  what that area is. We will work up the parentage chain and find
//  the smallest area that contains them all, possibly empting the area.
//
//  We return true if there's any area left, else false.
//
//  They can give us an initial deflation value if they need to, to move us
//  inside some border or whatnot.
//
//  We have another version that will just check to see if this widget has
//  a border and call the first version with either 0 or 1 deflates.
//
tCIDLib::TBoolean
TCQCIntfWidget::bFindMostRestrictiveClip(       TGUIRegion&     grgnToSet
                                        , const tCIDLib::TCard4 c4HDeflate
                                        , const tCIDLib::TCard4 c4VDeflate)
{
    // Start with our own clip region, letting it do the default set
    QueryClipRegion(grgnToSet);

    //
    //  Do any initial deflation, and an extra one if we have a border.
    //
    const tCIDLib::TCard4 c4BorderDef = bHasBorder() ? 1 : 0;
    grgnToSet.Deflate(c4HDeflate + c4BorderDef, c4VDeflate + c4BorderDef);

    // Then get our parent. We break out when no more parents
    TCQCIntfContainer* piwdgCont = m_piwdgParent;
    TGUIRegion grgnCur;
    while (piwdgCont)
    {
        // Add to our existing region
        piwdgCont->QueryClipRegion(grgnCur);
        grgnToSet.CombineWith(grgnCur, tCIDGraphDev::EClipModes::And);

        // If we've emptied the region, return false
        if (grgnToSet.bIsEmpty())
            return kCIDLib::False;

        piwdgCont = piwdgCont->piwdgParent();
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCIntfWidget::bFindMostRestrictiveClip(       TGUIRegion& grgnToSet
                                        , const TArea&      areaDisplay)
{
    //
    //  Start with the passed display area, which we assume the caller is
    //  smart enough to insure is within any border or rounding.
    //
    grgnToSet.Set(areaDisplay);

    //
    //  Then get our parent an iterate up to the root container, adding
    //  their clip regions. We break out when no more parents.
    //
    TCQCIntfContainer* piwdgCont = m_piwdgParent;
    TGUIRegion grgnCur;
    while (piwdgCont)
    {
        // Add to our existing region
        piwdgCont->QueryClipRegion(grgnCur);
        grgnToSet.CombineWith(grgnCur, tCIDGraphDev::EClipModes::And);

        // If we've emptied the region, return false
        if (grgnToSet.bIsEmpty())
            return kCIDLib::False;

        piwdgCont = piwdgCont->piwdgParent();
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCIntfWidget::bFindMostRestrictiveClip(TGUIRegion& rgnToSet)
{
    tCIDLib::TCard4 c4Deflate = bHasBorder() ? 1 : 0;
    return bFindMostRestrictiveClip(rgnToSet, c4Deflate, c4Deflate);
}


// Get/set the border attribute
tCIDLib::TBoolean TCQCIntfWidget::bHasBorder() const
{
    return (m_c4StyleFlags & kCQCIntfEng::c4StyleFlag_HasBorder) != 0;
}

tCIDLib::TBoolean TCQCIntfWidget::bHasBorder(const tCIDLib::TBoolean bToSet)
{
    const tCIDLib::TCard4 c4Old = m_c4StyleFlags;

    m_c4StyleFlags &= ~kCQCIntfEng::c4StyleFlag_HasBorder;
    if (bToSet)
        m_c4StyleFlags |= kCQCIntfEng::c4StyleFlag_HasBorder;

    // Tell the derived class a style flag changed
    if (c4Old != m_c4StyleFlags)
        StyleFlagChanged(c4Old, m_c4StyleFlags);

    return bToSet;
}


// Get/set the focus runtime flag
tCIDLib::TBoolean TCQCIntfWidget::bHasFocus(const tCIDLib::TBoolean bToSet)
{
    const tCIDLib::TCard4 c4Old = m_c4StyleFlags;

    if (bToSet)
        m_eRunFlags = tCIDLib::eOREnumBits(m_eRunFlags, tCQCIntfEng::ERunFlags::HasFocus);
    else
        m_eRunFlags = tCIDLib::eClearEnumBits(m_eRunFlags, tCQCIntfEng::ERunFlags::HasFocus);

    return bToSet;
}

tCIDLib::TBoolean TCQCIntfWidget::bHasFocus() const
{
    return tCIDLib::bAnyBitsOn(m_eRunFlags, tCQCIntfEng::ERunFlags::HasFocus);
}



// Get or set the locked flag
tCIDLib::TBoolean TCQCIntfWidget::bIsLocked() const
{
    return m_bLocked;
}

tCIDLib::TBoolean TCQCIntfWidget::bIsLocked(const tCIDLib::TBoolean bToSet)
{
    m_bLocked = bToSet;
    return m_bLocked;
}


// Return true if we have a non-zero group id, else false
tCIDLib::TBoolean TCQCIntfWidget::bIsGrouped() const
{
    return (m_c4GroupId != 0);
}


// See if our actual area intersects the passed area
tCIDLib::TBoolean TCQCIntfWidget::bIntersects(const TArea& areaToTest) const
{
    return m_areaActual.bIntersects(areaToTest);
}


// Get/set the initially invisible style
tCIDLib::TBoolean TCQCIntfWidget::bInitInvisible() const
{
    return (m_c4StyleFlags & kCQCIntfEng::c4StyleFlag_InitInvisible) != 0;
}

tCIDLib::TBoolean
TCQCIntfWidget::bInitInvisible(const tCIDLib::TBoolean bToSet)
{
    const tCIDLib::TCard4 c4Old = m_c4StyleFlags;

    m_c4StyleFlags &= ~kCQCIntfEng::c4StyleFlag_InitInvisible;
    if (bToSet)
        m_c4StyleFlags |= kCQCIntfEng::c4StyleFlag_InitInvisible;

    // Tell the derived class a style flag changed
    if (c4Old != m_c4StyleFlags)
        StyleFlagChanged(c4Old, m_c4StyleFlags);

    return bToSet;
}


//
//  A convenience method to get/set the in-gesture flag. This is set any time the
//  widget is the current target of a gesture, from the time it starts until any
//  inertia is complete. It is used by the container class to no dispatch any of the
//  timer driven callbacks to this widget, while still allowing others to update
//  since gestures can go on for a while.
//
tCIDLib::TBoolean TCQCIntfWidget::bInGesture() const
{
    return tCIDLib::bAnyBitsOn(m_eRunFlags, tCQCIntfEng::ERunFlags::InGesture);
}

tCIDLib::TBoolean TCQCIntfWidget::bInGesture(const tCIDLib::TBoolean bToSet)
{
    if (bToSet)
        m_eRunFlags = tCIDLib::eOREnumBits(m_eRunFlags, tCQCIntfEng::ERunFlags::InGesture);
    else
        m_eRunFlags = tCIDLib::eClearEnumBits(m_eRunFlags, tCQCIntfEng::ERunFlags::InGesture);
    return bToSet;
}


// Get/set the dead flag
tCIDLib::TBoolean TCQCIntfWidget::bIsDead() const
{
    return tCIDLib::bAnyBitsOn(m_eRunFlags, tCQCIntfEng::ERunFlags::Dead);
}

tCIDLib::TBoolean
TCQCIntfWidget::bIsDead(const tCIDLib::TBoolean bToSet)
{
    const tCIDLib::TCard4 c4Old = m_c4StyleFlags;
    if (bToSet)
        m_eRunFlags = tCIDLib::eOREnumBits(m_eRunFlags, tCQCIntfEng::ERunFlags::Dead);
    else
        m_eRunFlags = tCIDLib::eClearEnumBits(m_eRunFlags, tCQCIntfEng::ERunFlags::Dead);
    return bToSet;
}


// Get/set the transparency style
tCIDLib::TBoolean TCQCIntfWidget::bIsTransparent() const
{
    return (m_c4StyleFlags & kCQCIntfEng::c4StyleFlag_Transparent) != 0;
}

tCIDLib::TBoolean
TCQCIntfWidget::bIsTransparent(const tCIDLib::TBoolean bToSet)
{
    const tCIDLib::TCard4 c4Old = m_c4StyleFlags;

    m_c4StyleFlags &= ~kCQCIntfEng::c4StyleFlag_Transparent;
    if (bToSet)
        m_c4StyleFlags |= kCQCIntfEng::c4StyleFlag_Transparent;

    // Tell the derived class a style flag changed
    if (c4Old != m_c4StyleFlags)
        StyleFlagChanged(c4Old, m_c4StyleFlags);

    return bToSet;
}


// Set or get the negate states flag
tCIDLib::TBoolean TCQCIntfWidget::bNegateStates() const
{
    return m_bNegateStates;
}

tCIDLib::TBoolean TCQCIntfWidget::bNegateStates(const tCIDLib::TBoolean bToSet)
{
    m_bNegateStates = bToSet;
    return m_bNegateStates;
}


// Set or get the no inertia style
tCIDLib::TBoolean TCQCIntfWidget::bNoInertia() const
{
    return (m_c4StyleFlags & kCQCIntfEng::c4StyleFlag_NoInertia) != 0;
}

tCIDLib::TBoolean TCQCIntfWidget::bNoInertia(const tCIDLib::TBoolean bToSet)
{
    const tCIDLib::TCard4 c4Old = m_c4StyleFlags;

    m_c4StyleFlags &= ~kCQCIntfEng::c4StyleFlag_NoInertia;
    if (bToSet)
        m_c4StyleFlags |= kCQCIntfEng::c4StyleFlag_NoInertia;

    // Tell the derived class a style flag changed
    if (c4Old != m_c4StyleFlags)
        StyleFlagChanged(c4Old, m_c4StyleFlags);

    return bToSet;
}


//
//  Returns whether the background is scroll safe in the indicated direction.
//  For the most part this doesn't matter but for a couple widget types, they
//  care about it. And others are able to do more efficient scrolls if so.
//
tCIDLib::TBoolean
TCQCIntfWidget::bScrollSafeBgn(const tCIDLib::TBoolean bVert) const
{
    // If transparent, then definitely not
    tCIDLib::TBoolean bRet = !bIsTransparent();

    // If we are filled, see if the fill is reasonable for the direction
    if (bRet)
    {
        if (m_eBgnStyle == tCQCIntfEng::EBgnStyles::Fill)
        {
            // Either direction is ok
            bRet = kCIDLib::True;
        }
         else if ((m_eBgnStyle == tCQCIntfEng::EBgnStyles::HGradient)
              ||  (m_eBgnStyle == tCQCIntfEng::EBgnStyles::LGradient)
              ||  (m_eBgnStyle == tCQCIntfEng::EBgnStyles::CGradient))
        {
            // We can scroll vertically
            bRet = bVert;
        }
         else if (m_eBgnStyle == tCQCIntfEng::EBgnStyles::VGradient)
        {
            // We can scroll horizontally
            bRet = !bVert;
        }
    }
    return bRet;
}


// Get/set the simple drop shadow style
tCIDLib::TBoolean TCQCIntfWidget::bShadow() const
{
    return (m_c4StyleFlags & kCQCIntfEng::c4StyleFlag_Shadow) != 0;
}

tCIDLib::TBoolean TCQCIntfWidget::bShadow(const tCIDLib::TBoolean bToSet)
{
    const tCIDLib::TCard4 c4Old = m_c4StyleFlags;

    m_c4StyleFlags &= ~kCQCIntfEng::c4StyleFlag_Shadow;
    if (bToSet)
        m_c4StyleFlags |= kCQCIntfEng::c4StyleFlag_Shadow;

    // Tell the derived class a style flag changed
    if (c4Old != m_c4StyleFlags)
        StyleFlagChanged(c4Old, m_c4StyleFlags);

    return bToSet;
}


// Return true if the 'track beyond' capabilities flag is set
tCIDLib::TBoolean TCQCIntfWidget::bTrackBeyond() const
{
    return tCIDLib::bAnyBitsOn(m_eCapFlags, tCQCIntfEng::ECapFlags::TrackBeyond);
}


// Get/set the underline style
tCIDLib::TBoolean TCQCIntfWidget::bUnderline() const
{
    return (m_c4StyleFlags & kCQCIntfEng::c4StyleFlag_Underline) != 0;
}

tCIDLib::TBoolean TCQCIntfWidget::bUnderline(const tCIDLib::TBoolean bToSet)
{
    const tCIDLib::TCard4 c4Old = m_c4StyleFlags;

    m_c4StyleFlags &= ~kCQCIntfEng::c4StyleFlag_Underline;
    if (bToSet)
        m_c4StyleFlags |= kCQCIntfEng::c4StyleFlag_Underline;

    // Tell the derived class a style flag changed
    if (c4Old != m_c4StyleFlags)
        StyleFlagChanged(c4Old, m_c4StyleFlags);

    return bToSet;
}


// Update based on the passed state info
tCIDLib::TBoolean
TCQCIntfWidget::bUpdateDisplayState(const TCQCIntfStateList& istlData)
{
    // If no states, then return false now
    if (m_fcolStates.bIsEmpty())
        return kCIDLib::False;

    //
    //  We reference any states, so run through them and get the values of the
    //  states we use. If they resolve to true, then we set the m_bUseStateDisplay
    //  flag to indicate that the state display state is active display state.
    //
    tCIDLib::TBoolean bOrgVal = m_bUseStateDisplay;
    m_bUseStateDisplay = istlData.bGetStateResult(m_fcolStates, m_eLogOp);

    // If we are negating states, then do do that
    if (m_bNegateStates)
        m_bUseStateDisplay = !m_bUseStateDisplay;

    //
    //  Return whether it needs to redraw. It only needs to if the flag changed and
    //  the state based display state is different from the current display state.
    //  Otherwise we still have the same display state.
    //
    return
    (
        (bOrgVal != m_bUseStateDisplay) && (m_eStateDisplay != m_eCurDisplayState)
    );
}


// Indicates if we use the passed state
tCIDLib::TBoolean TCQCIntfWidget::bUsesState(const TString& strToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colStateNames.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colStateNames[c4Index] == strToFind)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Conveniences to check the update caps falgs
tCIDLib::TBoolean TCQCIntfWidget::bWantsActiveUpdate() const
{
    return tCIDLib::bAnyBitsOn(m_eCapFlags, tCQCIntfEng::ECapFlags::ActiveUpdate);
}

tCIDLib::TBoolean TCQCIntfWidget::bWantsAnyUpdate() const
{
    return tCIDLib::bAnyBitsOn(m_eCapFlags, tCQCIntfEng::ECapFlags::AnyUpdateFlags);
}

tCIDLib::TBoolean TCQCIntfWidget::bWantsValueUpdate() const
{
    return tCIDLib::bAnyBitsOn(m_eCapFlags, tCQCIntfEng::ECapFlags::ValueUpdate);
}


// Get/set the group id
tCIDLib::TCard4 TCQCIntfWidget::c4GroupId() const
{
    return m_c4GroupId;
}

tCIDLib::TCard4 TCQCIntfWidget::c4GroupId(const tCIDLib::TCard4 c4ToSet)
{
    //
    //  If setting to zero, clear the group name. This will only be set on the
    //  first widget in the group, but we don't know which one that is here. So
    //  just always clear it.
    //
    if (!c4ToSet)
        m_strGroupName.Clear();

    m_c4GroupId = c4ToSet;
    return m_c4GroupId;
}


// Returns the current ref count
tCIDLib::TCard4 TCQCIntfWidget::c4RefCount() const
{
    return m_c4RefCount;
}


// Get or set our rounding
tCIDLib::TCard4 TCQCIntfWidget::c4Rounding() const
{
    return m_c4Rounding;
}

tCIDLib::TCard4 TCQCIntfWidget::c4Rounding(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Rounding = c4ToSet;
    return m_c4Rounding;
}


//
//  The design time id. It is assigned to widgets by the designer for its own
//  needs. Primarily it is to uniquely identify widgets for undo purposes.
//
tCIDLib::TCard8 TCQCIntfWidget::c8DesId() const
{
    return m_c8DesId;
}

tCIDLib::TCard8 TCQCIntfWidget::c8DesId(const tCIDLib::TCard8 c8ToSet)
{
    m_c8DesId = c8ToSet;
    return m_c8DesId;
}


TCQCIntfView& TCQCIntfWidget::civOwner()
{
    CIDAssert(m_pcivOwner != 0, L"The owning view has not been set");
    return *m_pcivOwner;
}

const TCQCIntfView& TCQCIntfWidget::civOwner() const
{
    CIDAssert(m_pcivOwner != 0, L"The owning view has not been set");
    return *m_pcivOwner;
}


// Decrements the ref count, checking for underflow
tCIDLib::TVoid TCQCIntfWidget::DecRefCount()
{
    if (!m_c4RefCount)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcTmpl_RefCountUnderflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
    m_c4RefCount--;
}


// Deletes the indicated state from our list of states, if found
tCIDLib::TVoid TCQCIntfWidget::DeleteState(const TString& strToDel)
{
    const tCIDLib::TCard4 c4Count = m_colStateNames.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colStateNames[c4Index] == strToDel)
        {
            m_colStateNames.RemoveAt(c4Index);
            break;
        }
    }
}


// Deletes a list of states
tCIDLib::TVoid TCQCIntfWidget::DeleteStates(const tCIDLib::TStrCollect& colToRemove)
{
    TColCursor<TString>* pcursDel = colToRemove.pcursNew();
    TJanitor<TColCursor<TString> > janCursor(pcursDel);

    if (pcursDel->bReset())
    {
        const tCIDLib::TCard4 c4Count = m_colStateNames.c4ElemCount();
        do
        {
            const TString& strToDel = pcursDel->objRCur();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                if (m_colStateNames[c4Index] == strToDel)
                {
                    m_colStateNames.RemoveAt(c4Index);
                    break;
                }
            }
        }   while (pcursDel->bNext());
    }
}


//
//  Draws this widget on the passed target. We just check the hidden state
//  and just pass it on to a virtual of the widget if not hidden.
//
tCIDLib::TVoid
TCQCIntfWidget::DrawOn(         TGraphDrawDev&  gdevTarget
                        , const TArea&          areaInvalid
                        ,       TGUIRegion&     grgnClip)
{
    //
    //  For now just call the protected virtual so that the derived class
    //  can update itself. We have this interceding public method so that
    //  we will have some flexibility in the future to do some other
    //  processing here.
    //
    //  Don't do this if this widget's display state is hidden.
    //
    if (eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden)
        Update(gdevTarget, areaInvalid, grgnClip);
}


// Set/get the background style for this widget
tCQCIntfEng::EBgnStyles TCQCIntfWidget::eBgnStyle() const
{
    return m_eBgnStyle;
}

tCQCIntfEng::EBgnStyles
TCQCIntfWidget::eBgnStyle(const tCQCIntfEng::EBgnStyles eToSet)
{
    m_eBgnStyle = eToSet;
    return m_eBgnStyle;
}


// Get the caps flags for this widget
tCQCIntfEng::ECapFlags TCQCIntfWidget::eCapFlags() const
{
    return m_eCapFlags;
}


//
//  Get the display state for this widget. We need a separate style setter
//  because we cannot just return what was passed in since which display
//  state is active depends on whether we are controlled by the template
//  states.
//
tCQCIntfEng::EDispStates TCQCIntfWidget::eCurDisplayState() const
{
    // If states are controlling us, then return that, else the normal one
    if (m_bUseStateDisplay)
        return m_eStateDisplay;
    return m_eCurDisplayState;
}


// Get/set runtime values for this widget
tCQCIntfEng::ERunFlags TCQCIntfWidget::eRunFlags() const
{
    return m_eRunFlags;
}

tCQCIntfEng::ERunFlags
TCQCIntfWidget::eSetRunFlags(const  tCQCIntfEng::ERunFlags  eToSet
                            , const tCQCIntfEng::ERunFlags  eMask)
{
    m_eRunFlags = tCIDLib::eMaskEnumBits(m_eRunFlags, eToSet, eMask);
    return m_eRunFlags;
}


// Return the root template of this widget
TCQCIntfTemplate& TCQCIntfWidget::iwdgRootTemplate()
{
    //
    //  Work our way up the parent chain until we get to one that doesn't
    //  have a parent. That guy must be a template, since it must be the
    //  root.
    //
    TCQCIntfWidget* piwdgCur = this;
    TCQCIntfWidget* piwdgPar = m_piwdgParent;
    while(piwdgPar)
    {
        piwdgCur = piwdgPar;
        piwdgPar = piwdgCur->m_piwdgParent;
    }
    CIDAssert(piwdgCur != 0, L"No root template widget was found");
    return *dynamic_cast<TCQCIntfTemplate*>(piwdgCur);
}

const TCQCIntfTemplate& TCQCIntfWidget::iwdgRootTemplate() const
{
    //
    //  Work our way up the parent chain until we get to one that doesn't
    //  have a parent. That guy must be a template, since it must be the
    //  root.
    //
    const TCQCIntfWidget* piwdgCur = this;
    const TCQCIntfWidget* piwdgPar = m_piwdgParent;
    while(piwdgPar)
    {
        piwdgCur = piwdgPar;
        piwdgPar = piwdgCur->m_piwdgParent;
    }

    CIDAssert(piwdgCur != 0, L"No root template widget was found");
    return *dynamic_cast<const TCQCIntfTemplate*>(piwdgCur);
}


// Increments the ref count on this widget
tCIDLib::TVoid TCQCIntfWidget::IncRefCount()
{
    m_c4RefCount++;
}


//
//  For any states we have, look them up in the parent and save the index, in the same order
//  as the state names. If we don't find any, then remove them from our list.
//
tCIDLib::TVoid TCQCIntfWidget::LookupStates(const TCQCIntfContainer* const piwdgParent)
{
    // Clear out our state index list first
    m_fcolStates.RemoveAll();
    if (piwdgParent)
    {
        const TCQCIntfStateList& istlPar = piwdgParent->istlStates();
        tCIDLib::TCard4 c4Count = m_colStateNames.c4ElemCount();

        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            tCIDLib::TCard4 c4StateInd;
            if (istlPar.bFindState(m_colStateNames[c4Index], c4StateInd))
            {
                m_fcolStates.c4AddElement(c4StateInd);
                c4Index++;
            }
             else
            {
                m_colStateNames.RemoveAt(c4Index);
                c4Count--;
            }
        }
    }
     else
    {
        // Remove all states
        m_colStateNames.RemoveAll();
    }
}


//
//  Return a pointer to the owning view. In some cases code may have to deal with
//  the fact that the view isn't set. Otherwise they should call the civOwner() version
//  which will throw if not set.
//
TCQCIntfView* TCQCIntfWidget::pcivOwner()
{
    return m_pcivOwner;
}

const TCQCIntfView* TCQCIntfWidget::pcivOwner() const
{
    return m_pcivOwner;
}


//
//  A helper method that any widget can call to post an action it wants
//  to invoke. We look up the handler id for the template we belong to
//  and call the method on our window that actually posts the action,
//  passing this widget along as the sender.
//
tCIDLib::TVoid
TCQCIntfWidget::PostOps(        MCQCCmdSrcIntf&         mcsrcAction
                        , const TString&                strEvent
                        , const tCQCKit::EActOrders     eOrder)
{
    civOwner().PostOps
    (
        *this
        , mcsrcAction
        , strEvent
        , iwdgRootTemplate().c4HandlerId()
        , eOrder
    );
}


//
//  Set or add our clip region to the passed region. If adding, we are
//  doing an AND, so that we end up with the overlap of ours and the
//  existing content.
//
tCIDLib::TVoid
TCQCIntfWidget::QueryClipRegion(        TGUIRegion&         grgnToUpdate
                                , const tCIDLib::TBoolean   bSet)
{
    if (bSet)
    {
        grgnToUpdate.Set(areaActual(), m_c4Rounding);
    }
     else
    {
        TGUIRegion grgnUs(areaActual(), m_c4Rounding);
        grgnToUpdate.CombineWith(grgnUs, tCIDGraphDev::EClipModes::And);
    }
}


//
//  To allow the action editor to present the user with color names based on what the
//  actual widgets use them for, we need a way to get the colors supported by a widget
//  and the names it gives to them. There may be other uses for this.
//
//  There is already a mechanism in place that does that, the attribute system that the
//  attribute editor uses. We don't want to duplicate that stuff or create a whole new
//  scheme. So, though it's a little less efficient because it returns all of the supported
//  attributes, we provide method, which does an attribute query, and then looks through
//  the list to find the color attributes.
//
//  We then return a KVP list where the keys are the display names and the values are the
//  original names that this class uses internally to represent the colors.
//
tCIDLib::TVoid TCQCIntfWidget::QuerySupportedClrs(tCIDLib::TKVPCollect& colToFill) const
{
    // Ask ourself for our attributes
    tCIDMData::TAttrList    colAttrs;
    TAttrData               adatTmp;
    QueryWdgAttrs(colAttrs, adatTmp);

    //
    //  Find the ones that are colors and update the caller's list
    tCIDLib::TBoolean bKeeper;
    TString strClrName;
    colToFill.RemoveAll();
    TKeyValuePair kvalCur;
    const tCIDLib::TCard4 c4Count = colAttrs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TAttrData& adatCur = colAttrs[c4Index];

        bKeeper = kCIDLib::True;
        if (adatCur.strId() == kCQCIntfEng::strAttr_Clr_Bgn1)
            strClrName = L"Bgn 1";
        else if (adatCur.strId() == kCQCIntfEng::strAttr_Clr_Bgn2)
            strClrName = L"Bgn 2";
        else if (adatCur.strId() == kCQCIntfEng::strAttr_Clr_Fgn1)
            strClrName = L"Fgn 1";
        else if (adatCur.strId() == kCQCIntfEng::strAttr_Clr_Fgn2)
            strClrName = L"Fgn 2";
        else if (adatCur.strId() == kCQCIntfEng::strAttr_Clr_Fgn3)
            strClrName = L"Fgn 3";
        else if (adatCur.strId() == kCQCIntfEng::strAttr_Clr_Extra1)
            strClrName = L"Extra 1";
        else if (adatCur.strId() == kCQCIntfEng::strAttr_Clr_Extra2)
            strClrName = L"Extra 2";
        else if (adatCur.strId() == kCQCIntfEng::strAttr_Clr_Extra3)
            strClrName = L"Extra 3";
        else
            bKeeper = kCIDLib::False;

        if (bKeeper)
        {
            kvalCur.Set(adatCur.strName(), strClrName);
            colToFill.objAdd(kvalCur);
        }
    }
}


//
//  This is only useful at viewing time. It will build up a 'name path' which is
//  in the form /name/name/name/etc... Where the first name is the root-most container
//  of this widget, and so on down the nesting hierarchy.
//
//  We can optionally not include our own name at the end, i.e. it becomes a path
//  to our containing parent. A final target widget may not have a widget id anyway.
//
tCIDLib::TVoid
TCQCIntfWidget::QueryNamePath(          TString&            strToFill
                                , const tCIDLib::TBoolean   bIncludeMe) const
{
    // Start it off with the top-most container, the template usually
    strToFill = L"/";

    //
    //  Get the parent of this widget. If there is none, we can be much more
    //  efficient and just return this guy's id if he has one.
    //
    const TCQCIntfContainer* piwdgCur = piwdgParent();
    if (!piwdgCur)
    {
        strToFill.Append(m_strWidgetId);
        return;
    }

    //
    //  First thing we do is to work our way up the nesting hiearchy until we hit
    //  the top, then start working back down. So we use a simple stack to push
    //  the names onto. We don't push the template itself. It's represented by the
    //  / we already put into the path above.
    //
    TStack<TString> colNames;
    while (piwdgCur->piwdgParent())
    {
        piwdgCur = piwdgCur->piwdgParent();
        colNames.objPush(piwdgCur->strWidgetId());
    }

    // OK, we can now start popping them off and build up the path
    while (!colNames.bIsEmpty())
    {
        strToFill.Append(colNames.objPeek());
        strToFill.Append(kCIDLib::chForwardSlash);
        colNames.TrashTop();
    }

    // Now add this guy to the end if asked to
    if (bIncludeMe)
        strToFill.Append(m_strWidgetId);
}


// Get/set the background colors. The queries are inlined for speed
const TRGBClr& TCQCIntfWidget::rgbBgn() const
{
    return m_rgbBgn1;
}

const TRGBClr& TCQCIntfWidget::rgbBgn2() const
{
    return m_rgbBgn2;
}

const TRGBClr& TCQCIntfWidget::rgbBgn(const TRGBClr& rgbToSet)
{
    m_rgbBgn1 = rgbToSet;
    return m_rgbBgn1;
}

const TRGBClr& TCQCIntfWidget::rgbBgn2(const TRGBClr& rgbToSet)
{
    m_rgbBgn2 = rgbToSet;
    return m_rgbBgn2;
}


// Get/set the extra colors colors
const TRGBClr& TCQCIntfWidget::rgbExtra() const
{
    return m_rgbExtra1;
}

const TRGBClr& TCQCIntfWidget::rgbExtra2() const
{
    return m_rgbExtra2;
}

const TRGBClr& TCQCIntfWidget::rgbExtra3() const
{
    return m_rgbExtra3;
}

const TRGBClr& TCQCIntfWidget::rgbExtra(const TRGBClr& rgbToSet)
{
    m_rgbExtra1 = rgbToSet;
    return m_rgbExtra1;
}

const TRGBClr& TCQCIntfWidget::rgbExtra2(const TRGBClr& rgbToSet)
{
    m_rgbExtra2 = rgbToSet;
    return m_rgbExtra2;
}

const TRGBClr& TCQCIntfWidget::rgbExtra3(const TRGBClr& rgbToSet)
{
    m_rgbExtra3 = rgbToSet;
    return m_rgbExtra3;
}


// Get/set the foreground colors colors
const TRGBClr& TCQCIntfWidget::rgbFgn() const
{
    return m_rgbFgn1;
}

const TRGBClr& TCQCIntfWidget::rgbFgn2() const
{
    return m_rgbFgn2;
}

const TRGBClr& TCQCIntfWidget::rgbFgn3() const
{
    return m_rgbFgn3;
}

const TRGBClr& TCQCIntfWidget::rgbFgn(const TRGBClr& rgbToSet)
{
    m_rgbFgn1 = rgbToSet;
    return m_rgbFgn1;
}

const TRGBClr& TCQCIntfWidget::rgbFgn2(const TRGBClr& rgbToSet)
{
    m_rgbFgn2 = rgbToSet;
    return m_rgbFgn2;
}

const TRGBClr& TCQCIntfWidget::rgbFgn3(const TRGBClr& rgbToSet)
{
    m_rgbFgn3 = rgbToSet;
    return m_rgbFgn3;
}


// Resets our caption fields back to defaults
tCIDLib::TVoid
TCQCIntfWidget::ResetCaption(const  tCIDLib::TBoolean   bSingleLine
                            , const TString&            strText)
{
    m_bSingleLine = bSingleLine;
    m_strCaption = strText;

    CaptionChanged();
}


// Get/set the caption text
const TString& TCQCIntfWidget::strCaption() const
{
    return m_strCaption;
}

const TString& TCQCIntfWidget::strCaption(const TString& strToSet)
{
    m_strCaption = strToSet;
    CaptionChanged();
    return m_strCaption;
}


// Provide access to our human readable type name
const TString& TCQCIntfWidget::strTypeName() const
{
    return m_strTypeName;
}


//  Get/set the group name
const TString& TCQCIntfWidget::strGroupName() const
{
    return m_strGroupName;
}

const TString& TCQCIntfWidget::strGroupName(const TString& strToSet)
{
    if (!strToSet.bIsEmpty())
    {
        CIDAssert(m_c4GroupId != 0, L"Group name set on non-group root member");
    }
    m_strGroupName = strToSet;
    return m_strGroupName;
}


//  Get/set the widget id. If setting, we have some extra work to do
const TString& TCQCIntfWidget::strWidgetId() const
{
    return m_strWidgetId;
}

const TString& TCQCIntfWidget::strWidgetId(const TString& strToSet)
{
    //
    //  Set the passed value as the widget id, which is a human provided
    //  name for interface widgets.
    //
    m_strWidgetId = strToSet;

    //
    //  Use that to build a unique target id. The name is unique as enforced
    //  by the editor. We just add it to a widget prefix to make it a
    //  unique id.
    //
    TString strTarId;
    if (!m_strWidgetId.bIsEmpty())
    {
        strTarId = kCQCKit::strTarPref_Widgets;
        strTarId.Append(m_strWidgetId);
    }

    //
    //  And set up the command target mixin ids. We give it the target id we
    //  just built, the widget id as the human readable target name, and
    //  the class name of the ultimate derived class as the target type.
    //
    SetCmdTargetIds(strTarId, m_strWidgetId, clsIsA().strClassName());

    // Publish a change msg
    if (TFacCQCIntfEng::bDesMode())
    {
        if (piwdgParent())
            piwdgParent()->PublishWidgetChanged(this);
        else if (clsIsA() == TCQCIntfTemplate::clsThis())
            static_cast<TCQCIntfTemplate*>(this)->PublishWidgetChanged(nullptr);
    }

    return m_strWidgetId;
}


tCIDLib::TVoid
TCQCIntfWidget::SetArea(const   TArea&              areaNew
                        , const tCIDLib::TBoolean   bReport)
{
    if (areaNew == m_areaActual)
        return;

    const TArea areaOld = m_areaActual;

    // We get a new actual area
    m_areaActual = areaNew;
    SetRelativeArea();

    if (bReport)
    {
        // If only the size really changed, that's all we report
        if (m_areaActual.bSameOrg(areaOld))
            SizeChanged(m_areaActual.szArea(), areaOld.szArea());
        else
            AreaChanged(m_areaActual, areaOld);
    }
}

tCIDLib::TVoid
TCQCIntfWidget::SetArea(const   TPoint&             pntNewOrg
                        , const TSize&              szNew
                        , const tCIDLib::TBoolean   bReport)

{
    TArea areaNew (pntNewOrg, szNew);
    if (areaNew == m_areaActual)
        return;

    const TArea areaOld = m_areaActual;

    // We get a new actual origin
    m_areaActual.Set(pntNewOrg, szNew);
    SetRelativeArea();

    // If only the size really changed, that's all we report
    if (bReport)
    {
        if (m_areaActual.bSameOrg(areaOld))
            SizeChanged(m_areaActual.szArea(), areaOld.szArea());
        else
            AreaChanged(m_areaActual, areaOld);
    }
}


//
//  Set the display state, and optionally redraw if needed. Note that this
//  doesn't necessarily affect the widget's active display state because we
//  can be under teh control of template states right now and they will
//  override what is set here. But we store it for future use when the
//  template states release us back to our normal display state.
//
tCIDLib::TVoid
TCQCIntfWidget::SetDisplayState(const   tCQCIntfEng::EDispStates eToSet
                                , const tCIDLib::TBoolean        bRedraw)
{
    if (m_eCurDisplayState != eToSet)
    {
        m_eCurDisplayState = eToSet;

        //
        //  If we are not under control of template states, then let the
        //  derived class know that the state has changed and redraw if
        //  the caller asked us to.
        //
        if (!m_bUseStateDisplay)
        {
            DisplayStateChanged();
            if (bRedraw)
                Redraw();
        }
    }
}


//
//  Set our actual and relative origin. Inform the derived classes that the
//  origin changed if they care.
//
tCIDLib::TVoid
TCQCIntfWidget::SetOrg( const   TPoint&             pntNew
                        , const tCIDLib::TBoolean   bReport)
{
    if (m_areaActual.pntOrg() == pntNew)
        return;

    const TArea areaOld = m_areaActual;

    m_areaActual.SetOrg(pntNew);
    SetRelativeArea();
    if (bReport)
        AreaChanged(m_areaActual, areaOld);
}


// Set our owning view
tCIDLib::TVoid TCQCIntfWidget::SetOwner(TCQCIntfView* const pcivOwner)
{
    m_pcivOwner = pcivOwner;
}


// Set our parent container
tCIDLib::TVoid TCQCIntfWidget::SetParent(TCQCIntfContainer* const piwdgToSet)
{
    m_piwdgParent = piwdgToSet;
}


// Set the point of our parent container
tCIDLib::TVoid
TCQCIntfWidget::SetParentOrg(const  TPoint&             pntToSet
                            , const tCIDLib::TBoolean   bReport)
{
    if (pntToSet == m_pntParent)
        return;

    m_pntParent = pntToSet;
    const TArea areaOld = m_areaActual;

    //
    //  We need to update the absolute position in this case, based on the
    //  relative position plus the new parent origin.
    //
    m_areaActual = m_areaRelative;
    m_areaActual.AdjustOrg(pntToSet);

    if (bReport)
        AreaChanged(m_areaActual, areaOld);
}


// Change our widget position info
tCIDLib::TVoid
TCQCIntfWidget::SetPositions(const  TPoint&             pntParentOrg
                            , const TPoint&             pntNew
                            , const tCIDLib::TBoolean   bReport)
{
    if ((pntParentOrg == m_pntParent) && (pntNew == m_areaActual.pntOrg()))
        return;

    const TArea areaOld = m_areaActual;

    m_pntParent = pntParentOrg;
    m_areaActual.SetOrg(pntNew);
    SetRelativeArea();

    if (bReport)
        AreaChanged(m_areaActual, areaOld);
}

tCIDLib::TVoid
TCQCIntfWidget::SetPositions(const  TPoint&             pntParentOrg
                            , const TArea&              areaNew
                            , const tCIDLib::TBoolean   bReport)
{
    if ((pntParentOrg == m_pntParent) && (areaNew == m_areaActual))
        return;

    const TArea areaOld = m_areaActual;

    m_pntParent = pntParentOrg;
    m_areaActual = areaNew;
    SetRelativeArea();

    if (bReport)
        AreaChanged(m_areaActual, areaOld);
}


// Change our widget size info
tCIDLib::TVoid
TCQCIntfWidget::SetSize(const   TSize&              szNew
                        , const tCIDLib::TBoolean   bReport)
{
    if (szNew == m_areaActual.szArea())
        return;

    const TSize szOld = m_areaActual.szArea();

    m_areaActual.SetSize(szNew);
    m_areaRelative.SetSize(szNew);

    // Publish a change msg
    if(TFacCQCIntfEng::bDesMode())
    {
        if(piwdgParent())
            piwdgParent()->PublishWidgetChanged(this);
        else if (clsIsA() == TCQCIntfTemplate::clsThis())
            static_cast<TCQCIntfTemplate*>(this)->PublishWidgetChanged(nullptr);
    }

    if (bReport)
        SizeChanged(szNew, szOld);
}

tCIDLib::TVoid
TCQCIntfWidget::SetSize(const   tCIDLib::TCard4     c4Width
                        , const tCIDLib::TCard4     c4Height
                        , const tCIDLib::TBoolean   bReport)
{
    if ((c4Width == m_areaActual.c4Width()) && (c4Height == m_areaActual.c4Height()))
        return;

    const TSize szOld = m_areaActual.szArea();

    m_areaActual.SetSize(c4Width, c4Height);
    m_areaRelative.SetSize(c4Width, c4Height);

    // Publish a change msg
    if(TFacCQCIntfEng::bDesMode())
    {
        if(piwdgParent())
            piwdgParent()->PublishWidgetChanged(this);
        else if(clsIsA() == TCQCIntfTemplate::clsThis())
            static_cast<TCQCIntfTemplate*>(this)->PublishWidgetChanged(nullptr);
    }

    if (bReport)
        SizeChanged(m_areaActual.szArea(), szOld);
}


// Reset the group info on this widget
tCIDLib::TVoid TCQCIntfWidget::Ungroup()
{
    m_c4GroupId = 0;
    m_strGroupName.Clear();
}


// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfWidget::TCQCIntfWidget( const   tCQCIntfEng::ECapFlags  eCaps
                                , const TString&                strTypeName
                                , const tCIDLib::TCard4         c4InitStyle) :
    m_bLocked(kCIDLib::False)
    , m_bNegateStates(kCIDLib::False)
    , m_bSingleLine(kCIDLib::True)
    , m_bUseStateDisplay(kCIDLib::False)
    , m_c4GroupId(0)
    , m_c4RefCount(0)
    , m_c4Rounding(0)
    , m_c4StyleFlags(0)
    , m_c8DesId(kCIDLib::c8MaxCard)
    , m_eCapFlags(eCaps)
    , m_eCurDisplayState(tCQCIntfEng::EDispStates::Normal)
    , m_eLogOp(tCQCKit::ELogOps::AND)
    , m_eRunFlags(tCQCIntfEng::ERunFlags::None)
    , m_eStateDisplay(tCQCIntfEng::EDispStates::Disabled)
    , m_pcivOwner(nullptr)
    , m_piwdgParent(nullptr)
    , m_strCaption()
    , m_strGroupName(1UL)
    , m_strTypeName(strTypeName)
{
    SetDefAttrs(c4InitStyle);

    // We require GUI based command dispatch
    bGUIDispatch(kCIDLib::True);
}

// Note that the ref count goes to zero until added to some container
TCQCIntfWidget::TCQCIntfWidget( const   TArea&                  areaInit
                                , const tCQCIntfEng::ECapFlags  eCaps
                                , const TString&                strTypeName
                                , const tCIDLib::TCard4         c4InitStyle) :
    m_areaActual(areaInit)
    , m_areaRelative(TPoint::pntOrigin, areaInit.szArea())
    , m_bLocked(kCIDLib::False)
    , m_bNegateStates(kCIDLib::False)
    , m_bSingleLine(kCIDLib::True)
    , m_bUseStateDisplay(kCIDLib::False)
    , m_c4GroupId(0)
    , m_c4RefCount(0)
    , m_c4Rounding(0)
    , m_c4StyleFlags()
    , m_c8DesId(kCIDLib::c8MaxCard)
    , m_eCapFlags(eCaps)
    , m_eCurDisplayState(tCQCIntfEng::EDispStates::Normal)
    , m_eLogOp(tCQCKit::ELogOps::AND)
    , m_eRunFlags(tCQCIntfEng::ERunFlags::None)
    , m_eStateDisplay(tCQCIntfEng::EDispStates::Disabled)
    , m_pcivOwner(nullptr)
    , m_piwdgParent(nullptr)
    , m_strCaption()
    , m_strGroupName(1UL)
    , m_strTypeName(strTypeName)
{
    SetDefAttrs(c4InitStyle);

    // We require GUI based command dispatch
    bGUIDispatch(kCIDLib::True);
}

//
//  Note that the ref count goes to zero until added to some container. We also
//  do not set the owner because this one is not in the template, it's a copy of
//  one that was at best. Until it is added back, it's not a member.
//
TCQCIntfWidget::TCQCIntfWidget(const TCQCIntfWidget& iwdgSrc) :

    MCQCCmdTarIntf(iwdgSrc)
    , m_areaActual(iwdgSrc.m_areaActual)
    , m_areaOrg(iwdgSrc.m_areaOrg)
    , m_areaRelative(iwdgSrc.m_areaRelative)
    , m_bLocked(iwdgSrc.m_bLocked)
    , m_bNegateStates(iwdgSrc.m_bNegateStates)
    , m_bSingleLine(iwdgSrc.m_bSingleLine)
    , m_bUseStateDisplay(iwdgSrc.m_bUseStateDisplay)
    , m_c4GroupId(iwdgSrc.m_c4GroupId)
    , m_c4RefCount(0)
    , m_c4Rounding(iwdgSrc.m_c4Rounding)
    , m_c4StyleFlags(iwdgSrc.m_c4StyleFlags)
    , m_c8DesId(iwdgSrc.m_c8DesId)
    , m_colStateNames(iwdgSrc.m_colStateNames)
    , m_eBgnStyle(iwdgSrc.m_eBgnStyle)
    , m_eCapFlags(iwdgSrc.m_eCapFlags)
    , m_eCurDisplayState(iwdgSrc.m_eCurDisplayState)
    , m_eLogOp(iwdgSrc.m_eLogOp)
    , m_eRunFlags(iwdgSrc.m_eRunFlags)
    , m_eStateDisplay(iwdgSrc.m_eStateDisplay)
    , m_fcolStates(iwdgSrc.m_fcolStates)
    , m_pcivOwner(iwdgSrc.m_pcivOwner)
    , m_piwdgParent(nullptr)
    , m_pntParent(iwdgSrc.m_pntParent)
    , m_rgbBgn1(iwdgSrc.m_rgbBgn1)
    , m_rgbBgn2(iwdgSrc.m_rgbBgn2)
    , m_rgbExtra1(iwdgSrc.m_rgbExtra1)
    , m_rgbExtra2(iwdgSrc.m_rgbExtra2)
    , m_rgbExtra3(iwdgSrc.m_rgbExtra3)
    , m_rgbFgn1(iwdgSrc.m_rgbFgn1)
    , m_rgbFgn2(iwdgSrc.m_rgbFgn2)
    , m_rgbFgn3(iwdgSrc.m_rgbFgn3)
    , m_strCaption(iwdgSrc.m_strCaption)
    , m_strGroupName(iwdgSrc.m_strGroupName)
    , m_strTypeName(iwdgSrc.m_strTypeName)
    , m_strWidgetId(iwdgSrc.m_strWidgetId)
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Protected operators
// ---------------------------------------------------------------------------
TCQCIntfWidget& TCQCIntfWidget::operator=(const TCQCIntfWidget& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        // !!!!
        //  We keep our own ref count. We don't take the source's.
        // !!!!

        MCQCCmdTarIntf::operator=(iwdgSrc);
        m_areaActual        = iwdgSrc.m_areaActual;
        m_areaOrg           = iwdgSrc.m_areaOrg;
        m_areaRelative      = iwdgSrc.m_areaRelative;
        m_bLocked           = iwdgSrc.m_bLocked;
        m_bNegateStates     = iwdgSrc.m_bNegateStates;
        m_bSingleLine       = iwdgSrc.m_bSingleLine;
        m_bUseStateDisplay  = iwdgSrc.m_bUseStateDisplay;
        m_c4GroupId         = iwdgSrc.m_c4GroupId;
        m_c4Rounding        = iwdgSrc.m_c4Rounding;
        m_c4StyleFlags      = iwdgSrc.m_c4StyleFlags;
        m_c8DesId           = iwdgSrc.m_c8DesId;
        m_colStateNames     = iwdgSrc.m_colStateNames;
        m_eBgnStyle         = iwdgSrc.m_eBgnStyle;
        m_eCapFlags         = iwdgSrc.m_eCapFlags;
        m_eCurDisplayState  = iwdgSrc.m_eCurDisplayState;
        m_eLogOp            = iwdgSrc.m_eLogOp;
        m_eRunFlags         = iwdgSrc.m_eRunFlags;
        m_eStateDisplay     = iwdgSrc.m_eStateDisplay;
        m_fcolStates        = iwdgSrc.m_fcolStates;
        m_pcivOwner         = iwdgSrc.m_pcivOwner;
        m_piwdgParent       = iwdgSrc.m_piwdgParent;
        m_pntParent         = iwdgSrc.m_pntParent;
        m_rgbBgn1           = iwdgSrc.m_rgbBgn1;
        m_rgbBgn2           = iwdgSrc.m_rgbBgn2;
        m_rgbExtra1         = iwdgSrc.m_rgbExtra1;
        m_rgbExtra2         = iwdgSrc.m_rgbExtra2;
        m_rgbExtra3         = iwdgSrc.m_rgbExtra3;
        m_rgbFgn1           = iwdgSrc.m_rgbFgn1;
        m_rgbFgn2           = iwdgSrc.m_rgbFgn2;
        m_rgbFgn3           = iwdgSrc.m_rgbFgn3;
        m_strCaption        = iwdgSrc.m_strCaption;
        m_strGroupName      = iwdgSrc.m_strGroupName;
        m_strTypeName       = iwdgSrc.m_strTypeName;
        m_strWidgetId       = iwdgSrc.m_strWidgetId;

        //
        //  The collection won't report this so, in case we are in designer mode
        //  publish an element change.
        //
        if (TFacCQCIntfEng::bDesMode())
        {
            if (piwdgParent())
                piwdgParent()->PublishWidgetChanged(this);
            else if (clsIsA() == TCQCIntfTemplate::clsThis())
                static_cast<TCQCIntfTemplate*>(this)->PublishWidgetChanged(nullptr);
        }
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Protected, virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCQCIntfWidget::ActiveUpdate(const  tCIDLib::TBoolean   bNoRedraw
                            , const TGUIRegion&         grgnClip
                            , const tCIDLib::TBoolean   bInTopLayer)
{
    // Default empty implementation
}


tCIDLib::TVoid TCQCIntfWidget::AreaChanged(const TArea&, const TArea&)
{
    // Default empty implementation
}


// Move widgets won't need to react to this, so we do nothing by default
tCIDLib::TVoid TCQCIntfWidget::CaptionChanged()
{
}


//
//  Called when the widget's display state changes, to give the derived class
//  a chance to react to that other than just being redrawn.
//
tCIDLib::TVoid TCQCIntfWidget::DisplayStateChanged()
{
    // Default is to do nothing
}


// The default implementation is a simple area test
tCIDCtrls::EWndAreas TCQCIntfWidget::eHitTest(const TPoint& pntTest) const
{
    // We can use a generic area hit tester for this
    return facCIDCtrls().eGenericHitCalc
    (
        pntTest, m_areaActual, kCQCIntfEng::c4SizeBorderWidth
    );
}


// Called when this widget gets focus. Only used if focus is enabled
tCIDLib::TVoid TCQCIntfWidget::GotFocus()
{
    // Default is to just redraw
    Redraw();
}


// Called when this widget loses the focus. Only used if focus is enabled
tCIDLib::TVoid TCQCIntfWidget::LostFocus()
{
    // Default is to just redraw
    Redraw();
}


//
//  Called when the widget's parent's display state changes. The parent will call
//  this on each child when its state changes.
//
tCIDLib::TVoid TCQCIntfWidget::ParentDisplayStateChanged(const tCQCIntfEng::EDispStates)
{
    // Default is to do nothing
}

//
//  This one is only called at viewing time. It allows any widget that is maintaining some
//  external window relative to itself to move it when the containing application changes
//  position. SizeChanged or areaChanged() doesn't get called because the widget itself has
//  not changed it's position relative to the template window.
//
//  It's gets the area of the template in screen coordinates. This widget is that plus the
//  widget's relative area.
//
tCIDLib::TVoid TCQCIntfWidget::ScrPosChanged(const TArea&)
{
    // Default is to do nothing
}


//
//  This one is only called during design mode. At viewing time the size never
//  changes. This is to allow widgets to react to changes in their size during
//  design. They get the old size, and the new size is already set.
//
tCIDLib::TVoid TCQCIntfWidget::SizeChanged(const TSize&, const TSize&)
{
    // Default is to just to do nothing
}


//
//  This is called when one of the style flags changes, to allow widget to
//  re-calc stuff if needed.
//
tCIDLib::TVoid
TCQCIntfWidget::StyleFlagChanged(const tCIDLib::TCard4, const tCIDLib::TCard4)
{
    // Default is just ot do nothing
}


//
//  Called on widgets when it's time for them to redraw. Generally derived
//  classes call us here to do the background fill and border stuff, then
//  they draw any remaining stuff over that. If we are hidden, we do nothing.
//  If not transparent, we do a standard bgn fill. If we have a border, that
//  is done using the almost always used Fgn3 color.
//
//  If the derived class needs anything different from this, it has to
//  override and do it itself.
//
//  We move the passed clip region in if we update it due to a border. The
//  derived method can set this new region before doing it's own drawing.
//
tCIDLib::TVoid
TCQCIntfWidget::Update(         TGraphDrawDev&      gdevTarget
                        , const TArea&              areaInvalid
                        ,       TGUIRegion&         grgnClip)
{
    // Only do this if not hidden
    if (eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden)
    {
        // Fill the invalid area if we aren't transparent
        if (!(m_c4StyleFlags & kCQCIntfEng::c4StyleFlag_Transparent))
        {
            TRegionJanitor janClip
            (
                &gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And
            );

            TArea areaFill = grgnClip.areaBounds();
            facCQCIntfEng().StdBgnFill
            (
                gdevTarget
                , areaFill
                , m_eBgnStyle
                , m_rgbBgn1
                , m_rgbBgn2
            );
        }
    }

    if (bHasBorder())
    {
        gdevTarget.Stroke(grgnClip, m_rgbFgn3);
        grgnClip.Deflate(1, 1);
    }
}


// Periodically called on widgets who ask to be scanned for value updates.
tCIDLib::TVoid
TCQCIntfWidget::ValueUpdate(        TCQCPollEngine&
                            , const tCIDLib::TBoolean
                            , const tCIDLib::TBoolean
                            , const TStdVarsTar&
                            , const TGUIRegion&)
{
    // Default empty implementation
}



// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfWidget::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff must start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version for this level
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Widget::c2FmtVersion))
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

    //
    //  If it's a V1, then the old widget base class is no longer our parent,
    //  and no long exists. So we have to read in his fields and discard
    //  them. Else, nothing to do since our parent is TObject.
    //
    if (c2FmtVersion < 2)
    {
        strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);
        tCIDLib::TCard2 c2BaseFmtVersion;
        strmToReadFrom  >> c2BaseFmtVersion;
        if (!c2BaseFmtVersion || (c2BaseFmtVersion > 2))
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

        if (c2BaseFmtVersion < 2)
        {
            tCQCKit::EReqAccess eDummy;
            strmToReadFrom >> eDummy;
        }

        TString strDummy;
        strmToReadFrom >> strDummy;

        // And our stuff must end with a marker
        strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    }

    //
    //  Stream in our stuff. In V6 we removed the direct write stuff, which
    //  hadn't been used in a while. If prior to V6, we have to read them
    //  into temps.
    //
    strmToReadFrom  >> m_areaActual
                    >> m_areaRelative;

    if (c2FmtVersion < 6)
    {
        tCIDLib::TBoolean bTmp;
        strmToReadFrom >> bTmp;
    }

    strmToReadFrom  >> m_c4StyleFlags;

    if (c2FmtVersion < 6)
    {
        tCQCKit::EUserRoles eTmp;
        strmToReadFrom >> eTmp;
    }

    strmToReadFrom  >> m_rgbBgn1
                    >> m_rgbBgn2
                    >> m_rgbFgn1
                    >> m_rgbFgn2
                    >> m_strWidgetId
                    >> m_colStateNames
                    >> m_eLogOp
                    >> m_eStateDisplay
                    >> m_rgbFgn3
                    >> m_eBgnStyle
                    >> m_strCaption;

    // If V3 or beyond, read in the extra colors, else default them
    if (c2FmtVersion > 2)
    {
        strmToReadFrom  >> m_rgbExtra1
                        >> m_rgbExtra2
                        >> m_rgbExtra3;
    }
     else
    {
        m_rgbExtra1.ToBlack();
        m_rgbExtra2.ToBlack();
        m_rgbExtra3.ToBlack();
    }

    //
    //  If V4 or beyond, read in the group stuff, else set to no group. Note
    //  that the template will handle upgrading us from the group index to
    //  the group id scheme we use now, so we don't make any attempt to change
    //  them here.
    //
    if (c2FmtVersion > 3)
    {
        strmToReadFrom  >> m_c4GroupId
                        >> m_strGroupName;
    }
     else
    {
        m_c4GroupId = 0;
        m_strGroupName.Clear();
    }

    // If V5 or beyond, read in the locked flag, else set to unlocked
    if (c2FmtVersion > 4)
        strmToReadFrom >> m_bLocked;
    else
        m_bLocked = kCIDLib::False;

    //
    //  If less than V6, then make sure that the 'no inertia' bit is set to
    //  zero. This was added in 4.3.927, and we want to insure that any
    //  existing widgets have it forced off.
    //
    if (c2FmtVersion < 6)
        m_c4StyleFlags &= ~kCQCIntfEng::c4StyleFlag_NoInertia;

    // If less than 7, default the rounding value, else reading it
    if (c2FmtVersion < 7)
        m_c4Rounding = 0;
    else
        strmToReadFrom >> m_c4Rounding;

    //
    //  If version 8 read in the anchor value and throw it away since we no longer
    //  use that. It wasn't present before or after V8.
    //
    if (c2FmtVersion == 8)
    {
        tCIDLib::TInt4 i4Dummy;
        strmToReadFrom >> i4Dummy;
    }

    //
    //  V9 is just get us beyond V8 once we've done the discard of the anchors
    //  above. It doesn't have any changes of its own.
    //

    // If > V9, then read in the state negation flag, else default
    if (c2FmtVersion > 9)
        strmToReadFrom >> m_bNegateStates;
    else
        m_bNegateStates = kCIDLib::False;

    //
    //  Build the target id based on the base widget id. If it doesn't
    //  have one, and we aren't in designer mode, then give it an on the
    //  fly id for use in action target resolution (since some widgets can
    //  be targetd without being given an id, such as overlays when they
    //  are reloaded.
    //
    TString strTarId;
    if (m_strWidgetId.bIsEmpty() && !TFacCQCIntfEng::bDesMode())
    {
        m_strWidgetId = kCQCIntfEng::strDefWidgetIdPref;
        m_strWidgetId.AppendFormatted(c4UniqueId());
    }

    if (!m_strWidgetId.bIsEmpty())
    {
        strTarId = kCQCKit::strTarPref_Widgets;
        strTarId.Append(m_strWidgetId);
    }

    //
    //  And set up the command target mixin ids. We give it the target id we
    //  just built, the widget id as the human readable target name, and
    //  the class name of the ultimate derived class as the target type.
    //
    SetCmdTargetIds(strTarId, m_strWidgetId, clsIsA().strClassName());

    // And our stuff must end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Clear out/reset the stuff that isn't persisted
    m_bUseStateDisplay = kCIDLib::False;
    m_fcolStates.RemoveAll();
    m_c4RefCount = 0;

    //  Set the display state based on the initial invisible style
    if (m_c4StyleFlags & kCQCIntfEng::c4StyleFlag_InitInvisible)
        SetDisplayState(tCQCIntfEng::EDispStates::Hidden, kCIDLib::False);
    else
        SetDisplayState(tCQCIntfEng::EDispStates::Normal, kCIDLib::False);

    // Remember our original relative area
    m_areaOrg = m_areaRelative;
}


tCIDLib::TVoid TCQCIntfWidget::StreamTo(TBinOutStream& strmToWriteTo) const
{
    //  Stream out a start object marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_Widget::c2FmtVersion;

    // Stream out our stuff and a frame marker
    strmToWriteTo   << m_areaActual
                    << m_areaRelative
                    << m_c4StyleFlags
                    << m_rgbBgn1
                    << m_rgbBgn2
                    << m_rgbFgn1
                    << m_rgbFgn2
                    << m_strWidgetId
                    << m_colStateNames
                    << m_eLogOp
                    << m_eStateDisplay
                    << m_rgbFgn3
                    << m_eBgnStyle
                    << m_strCaption

                    // V3 stuff
                    << m_rgbExtra1
                    << m_rgbExtra2
                    << m_rgbExtra3

                    // V4 stuff
                    << m_c4GroupId
                    << m_strGroupName

                    // V5 stuff
                    << m_bLocked

                    // V7 stuff
                    << m_c4Rounding

                    // V8 changes were discarded, leading to 9 as a place holder

                    // V10 stuff
                    << m_bNegateStates

                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Just a convenience that passes through to our owner window
tCQCIntfEng::TGraphIntfDev TCQCIntfWidget::cptrNewGraphDev() const
{
    return m_pcivOwner->cptrNewGraphDev();
}


//
//  Does the standard background field that all widgets support. This one
//  is for use when setting up zero based memory images for scrollable
//  background content. So it has to take an arbitrary area. The area should
//  always be the whole widget area, so that gradients and such come out
//  correctly (though it might be used in some special cases to do a fill on
//  some sub-area of the widget.)
//
//  There's no issue here with rounding or clipping. The target widget that
//  the scrollable content is being displayed within will provide the
//  clipping.
//
tCIDLib::TVoid
TCQCIntfWidget::DoBgnFill(TGraphDrawDev& gdevTarget, const TArea& areaTarget)
{
    // Fill the invalid area if we aren't transparent
    if (!(m_c4StyleFlags & kCQCIntfEng::c4StyleFlag_Transparent))
    {
        facCQCIntfEng().StdBgnFill
        (
            gdevTarget
            , areaTarget
            , m_eBgnStyle
            , m_rgbBgn1
            , m_rgbBgn2
        );
    }
}


//
//  Just a covenience that passes through to our owner view, to provide
//  access to a compatible graphic device that any of our widgets can use
//  during drawing. It's not for drawing, but for things like creating
//  bitmaps.
//
const TGraphDrawDev& TCQCIntfWidget::gdevCompat() const
{
    return m_pcivOwner->gdevCompat();
}


//
//  These are used by derived widget classes to ask to have areas or
//  regions of themselves redrawn. They cannot just draw themselves
//  directly, since we must deal with popups that overlay us.
//
tCIDLib::TVoid TCQCIntfWidget::Redraw(const TArea& areaToRedraw)
{
    CIDAssert(m_pcivOwner != 0, L"The owning view has not be set");
    m_pcivOwner->Redraw(areaToRedraw);
}

tCIDLib::TVoid TCQCIntfWidget::Redraw(const TGUIRegion& grgnToRedraw)
{
    CIDAssert(m_pcivOwner != 0, L"The owning view has not be set");
    m_pcivOwner->Redraw(grgnToRedraw);
}

tCIDLib::TVoid TCQCIntfWidget::Redraw()
{
    CIDAssert(m_pcivOwner != 0, L"The owning view has not be set");
    m_pcivOwner->Redraw(m_areaActual);
}


//
//  Though it's rare, sometimes widgets have to update a capabilities flag
//  based on circumstances, such as whether it supports gestures or not.
//
tCIDLib::TVoid
TCQCIntfWidget::SetCapFlags(const   tCQCIntfEng::ECapFlags eFlags
                            , const tCQCIntfEng::ECapFlags eMask)
{
    //
    //  Do the usual thing. Mask out the mask bits from the current value,
    //  then OR in the AND of the toset and mask bits.
    //
    const tCIDLib::TCard4 c4Mask = tCIDLib::c4EnumOrd(eMask);
    tCIDLib::TCard4 c4Tmp = tCIDLib::c4EnumOrd(m_eCapFlags);

    c4Tmp &= ~c4Mask;
    c4Tmp |= tCIDLib::TCard4(eFlags) & c4Mask;

    m_eCapFlags = tCQCIntfEng::ECapFlags(c4Tmp);
}


//
//  A helper that just sees if this class type (polymorhically) is the same
//  as the one passed and throws if not.
//
tCIDLib::TVoid TCQCIntfWidget::ValidateTypes(const TClass& clsSrc) const
{
    if (clsIsA() != clsSrc)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcDbg_BadSrcType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , clsSrc
            , clsIsA()
        );
    }
}


// A convenience method for derived widgets to write a field value
tCIDLib::TVoid
TCQCIntfWidget::WriteValue( const   TString& strMoniker
                            , const TString& strFld
                            , const TString& strValue)
{
    facCQCIntfEng().WriteField(civOwner(), strMoniker, strFld, strValue);
}


// ---------------------------------------------------------------------------
//  TCQCIntfWidget: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfWidget::SetDefAttrs(const tCIDLib::TCard4 c4DefStyles)
{
    m_c4Rounding = 0;
    m_c4StyleFlags = c4DefStyles;
    m_eBgnStyle = tCQCIntfEng::EBgnStyles::Fill;
    m_rgbBgn1 = facCIDGraphDev().rgbWhite;
    m_rgbBgn2 = facCIDGraphDev().rgbWhite;
    m_rgbFgn1.ToBlack();
    m_rgbFgn2 = facCIDGraphDev().rgbWhite;
    m_rgbFgn3.ToBlack();

    m_rgbExtra1.ToBlack();
    m_rgbExtra2 = facCIDGraphDev().rgbWhite;
    m_rgbExtra3.ToBlack();
}


tCIDLib::TVoid TCQCIntfWidget::SetRelativeArea()
{
    // Start with the actual, then subtract the parent origin
    m_areaRelative = m_areaActual;
    TPoint pntTmp(m_pntParent);
    pntTmp.Negate();
    m_areaRelative.AdjustOrg(pntTmp);

    // Publish a change msg
    if (TFacCQCIntfEng::bDesMode())
    {
        if (piwdgParent())
            piwdgParent()->PublishWidgetChanged(this);
        else if (clsIsA() == TCQCIntfTemplate::clsThis())
            static_cast<TCQCIntfTemplate*>(this)->PublishWidgetChanged(nullptr);
    }
}

