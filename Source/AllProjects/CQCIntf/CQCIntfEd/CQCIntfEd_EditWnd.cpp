//
// FILE NAME: CQCIntfEd_EditWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2015
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
//  This file implements the main container window of the editor.
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
#include "CQCIntfEd_.hpp"
#include "CQCIntfEd_PasteAttrsDlg_.hpp"
#include "CQCIntfEd_SearchDlg_.hpp"
#include "CQCIntfEd_TmplStatesDlgs_.hpp"
#include "CQCIntfEd_Tools_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIntfEditWnd,TWindow)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEd_EditWnd
    {
        // -----------------------------------------------------------------------
        //  The next template level design time id to apply. We set a new one on
        //  the template of each new opened window.
        // -----------------------------------------------------------------------
        tCIDLib::TCard4 c4NextTmplDesId = 1;


        // -----------------------------------------------------------------------
        //  Some faux menu items that we use for our accellerator table. Keep these at
        //  high numbers that won't be used in the context menu, but below the ones used
        //  to dynamically load up the New submenu. So we just subtract values from the
        //  first dynamic id.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TResId   ridMenu_Faux_FontSize_Dec   = kCQCIntfEd_::c4FirstNewId - 1;
        constexpr tCIDLib::TResId   ridMenu_Faux_FontSize_Inc   = kCQCIntfEd_::c4FirstNewId - 2;
        constexpr tCIDLib::TResId   ridMenu_Faux_HJustify       = kCQCIntfEd_::c4FirstNewId - 3;
        constexpr tCIDLib::TResId   ridMenu_Faux_VJustify       = kCQCIntfEd_::c4FirstNewId - 4;
    }
}



// ---------------------------------------------------------------------------
//  TIntfEditWnd::TAlignWrapper: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfEditWnd::TAlignWrapper::TAlignWrapper() :

    m_colList(tCIDLib::EAdoptOpts::NoAdopt)
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfEditWnd::TAlignWrapper::Complete()
{
    // Set up the full area of the widgets we hold
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    if (c4Count == 1)
    {
        m_areaFull = m_colList[0]->areaActual();
    }
     else
    {
        m_areaFull = m_colList[0]->areaActual();
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
            m_areaFull |= m_colList[c4Index]->areaActual();
    }
}


//
//  If a single widget, just set the passed point as the origin. Else, move the group
//  origin to this point, which means adjusting all of the widgets relative to their
//  new origin.
//
tCIDLib::TVoid
TIntfEditWnd::TAlignWrapper::SetOrg(const TPoint& pntSet, TIntfEditWnd& iwdgOwner)
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();

    // Shouldn't happen, but just in case
    if (!c4Count)
        return;

    if (c4Count == 1)
    {
        m_colList[0]->SetOrg(pntSet);
    }
     else
    {
        TPoint pntOfs;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // Get the current widget and it's offset from the original group area
            TCQCIntfWidget* piwdgCur = m_colList[c4Index];
            pntOfs = piwdgCur->areaActual().pntOrg() - m_areaFull.pntOrg();

            // Now place it this offset from the new area origin
            piwdgCur->SetOrg(pntSet + pntOfs);
        }
    }
}


//
//  We got a notification from our tools tab and it was one of the alignment tools. So we
//  look at the op requested and apply it. We create our own little sorter, so that we can
//  sort the list by various criteria, which we pass in via the user data value. The values
//  are:
//
//      1 - Left
//      2 - Right
//      3 - Top
//      4 - Bottom
//
//      5 - Horizontal center line
//      6 - Vertical center line
//
//  What we are sorting are our alignment wrappers, which may contain a single widget or
//  a full group.
//
//  If in right/bottom mode, we will reverse sort.
//
class TAlignListSorter
{
    public :
        TAlignListSorter() = delete;
        TAlignListSorter(const  TString                     strToDo
                        ,       TIntfEditWnd::TAlignList&   colToSort
                        , const tCIDLib::TBoolean           bReverse) :

            m_bReverse(bReverse)
            , m_colToSort(colToSort)
            , m_strToDo(strToDo)
        {
        }

        tCIDLib::ESortComps operator()( const  TIntfEditWnd ::TAlignWrapper&    aw1
                                        , const TIntfEditWnd::TAlignWrapper&    aw2) const
        {
            tCIDLib::TInt4 i41;
            tCIDLib::TInt4 i42;
            if (m_strToDo == kCQCIntfEd_::strTool_LeftAlign)
            {
                i41 = aw1.m_areaFull.i4Left();
                i42 = aw2.m_areaFull.i4Left();
            }
            else if (m_strToDo == kCQCIntfEd_::strTool_RightAlign)
            {
                i41 = aw1.m_areaFull.i4Right();
                i42 = aw2.m_areaFull.i4Right();
            }
            else if (m_strToDo == kCQCIntfEd_::strTool_TopAlign)
            {
                i41 = aw1.m_areaFull.i4Top();
                i42 = aw2.m_areaFull.i4Top();
            }
            else if (m_strToDo == kCQCIntfEd_::strTool_BottomAlign)
            {
                i41 = aw1.m_areaFull.i4Bottom();
                i42 = aw2.m_areaFull.i4Bottom();
            }
            else if ((m_strToDo == kCQCIntfEd_::strTool_HorzSpace)
                ||   (m_strToDo == kCQCIntfEd_::strTool_VertCenter))
            {
                i41 = aw1.m_areaFull.pntCenter().i4X();
                i42 = aw2.m_areaFull.pntCenter().i4X();
            }
            else if ((m_strToDo == kCQCIntfEd_::strTool_HorzCenter)
                 ||  (m_strToDo == kCQCIntfEd_::strTool_VertSpace))
            {
                i41 = aw1.m_areaFull.pntCenter().i4Y();
                i42 = aw2.m_areaFull.pntCenter().i4Y();
            }

            // If right/bottom type, then reverse sort
            if (m_bReverse)
            {
                if (i42 < i41)
                    return tCIDLib::ESortComps::FirstLess;
                else if (i42 > i41)
                    return tCIDLib::ESortComps::FirstGreater;
            }
             else
            {
                if (i41 < i42)
                    return tCIDLib::ESortComps::FirstLess;
                else if (i41 > i42)
                    return tCIDLib::ESortComps::FirstGreater;
            }

            return tCIDLib::ESortComps::Equal;
        }

        tCIDLib::TBoolean           m_bReverse;
        TIntfEditWnd::TAlignList&   m_colToSort;
        TString                     m_strToDo;
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfEditWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfEditWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfEditWnd::TIntfEditWnd() :

    TWindow(kCIDLib::False)
    , m_bmpChecker(facCIDCtrls(), kCIDCtrls::ridBmp_Checker)
    , m_c4NextWId(1)
    , m_eDragArea(tCIDCtrls::EWndAreas::Nowhere)
    , m_eDragMode(EDragModes::None)
    , m_gpenDrag(facCIDGraphDev().rgbWhite, tCIDGraphDev::ELineStyles::Dot)
    , m_menuEdit(L"IE Edit Menu")
    , m_pcivEdit(nullptr)
    , m_pwndAttrEd(nullptr)
    , m_pwndErrList(nullptr)
{
    //
    //  Create the widget palette publish topic id. The widget palette in our editor
    //  tab will get this from us and use it to publish. This way we both can see it.
    //
    m_strWdgPalTopic = L"/CQC/IntfEd/WdgPalChanges/";
    m_strWdgPalTopic.AppendFormatted(facCIDLib().c4NextId());

    // Create our own update topic id
    m_strUpdateTopic = L"/CQC/IntfEd/Updates/";
    m_strUpdateTopic.AppendFormatted(facCIDLib().c4NextId());
}

TIntfEditWnd::~TIntfEditWnd()
{
}


// ---------------------------------------------------------------------------
//  TIntfEdWindow: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TIntfEditWnd::AsyncCallback(const tCQCIntfEng::EAsyncCmds, TCQCIntfView&)
{
    // No-op at design time
}


tCIDLib::TVoid TIntfEditWnd::AsyncDataCallback(TCQCIntfADCB* const)
{
    // No-op at design time
}


//
//  We implement the in place editor's interface, and pass ourself to the attribute
//  editor windows, which in turn pass us to the IPE for it to use for validation.
//  So we get called back here when the IPE needs to validate an entered value. Most
//  of them we will pass to the base mixin interface, but we handle those that require
//  extra checking.
//
//  Keep in mind that we will we only get stuff here that can be edited in place. Other
//  validation has to be done by the widgets themselves.
//
//  The user id here will be the design time id.
//
tCIDLib::TBoolean
TIntfEditWnd::bIPEValidate( const   TString&            strSrc
                            ,       TAttrData&          adatVal
                            , const TString&            strNewVal
                            ,       TString&            strErrMsg
                            ,       tCIDLib::TCard8&    c8UserId) const
{
    //
    //  Call the base mixin class to do basic validation. If that fails, then
    //  no need to look further.
    //
    if (!MIPEIntf::bIPEValidate(strSrc, adatVal, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    // Do any extra checking that we need to do.
    if (adatVal.strId() == kCQCIntfEng::strAttr_Field)
    {
        //
        //  Make sure the field exists if they enter it manually. If it's empty, and the
        //  attribute is marked as required, we reject that.
        //
        if (strNewVal.bIsEmpty())
        {
            if (adatVal.strLimits() == kCIDMData::strAttrLim_Required)
            {
                strErrMsg = facCQCIntfEd().strMsg(kIEdErrs::errcGen_ReqField);
                return kCIDLib::False;
            }
        }
         else
        {
            if (!facCQCKit().bCheckFldExists(strNewVal))
            {
                strErrMsg = facCQCKit().strMsg(kKitErrs::errcFld_NotFound, strNewVal);
                return kCIDLib::False;
            }
        }
    }
     else if (adatVal.strId() == kCQCIntfEng::strAttr_Base_Area)
    {
        // Don't let any widget get smaller than one pixel
        const TArea& areaCheck = adatVal.areaVal();

        if (!areaCheck.c4Width() || !areaCheck.c4Height())
        {
            strErrMsg = facCQCIntfEd().strMsg(kIEdErrs::errcVal_ZeroSize);
            return kCIDLib::False;
        }
    }
     else if (adatVal.strId() == kCQCIntfEng::strAttr_Base_WidgetId)
    {
        //
        //  If we find it, and it's not the original, then a dup. Check for empty first, since
        //  we can have multiples that are empty.
        //
        if (!strNewVal.bIsEmpty())
        {
            const TCQCIntfWidget* piwdgTest = iwdgEdit().piwdgFindByName(strNewVal, kCIDLib::False);
            if (piwdgTest && (piwdgTest->c8DesId() != c8UserId))
            {
                strErrMsg = facCQCIntfEd().strMsg(kIEdErrs::errcTmpl_DupId);
                return kCIDLib::False;
            }
        }
    }
     else if (adatVal.strId() == kCQCIntfEng::strAttr_WebCam_Params)
    {
        tCIDLib::TKVPList colParams;
        return TCQCIntfWebCamera::bValidateParms(adatVal.strValue(), strErrMsg, colParams);
    }
     else if ((adatVal.strSpecType() == kCQCIntfEng::strAttrType_Template)
          ||  (adatVal.strSpecType() == kCQCIntfEng::strAttrType_Image))
    {
        //
        //  These ae can do via the special type. We watch for those that refer to
        //  remote browser files and just make sure it's a syntactically valid
        //  remote browser, type relative, path. Relative paths only valid if it's
        //  a template.
        //
        const tCIDLib::TBoolean bRelOK
        (
            adatVal.strSpecType() == kCQCIntfEng::strAttrType_Template
        );
        if (!facCQCKit().bIsValidRemTypeRelPath(adatVal.strValue(), strErrMsg, bRelOK))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TIntfEditWnd::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    // Just pass it to our accelerator table
    return m_accelEdit.bProcessAccel(*this, pMsgData);
}


tCIDLib::TVoid
TIntfEditWnd::DismissBlanker(TCQCIntfView&)
{
    // No-op at design time
}

tCIDLib::TVoid
TIntfEditWnd::DoSpecialAction( const   tCQCIntfEng::ESpecActs
                                , const tCIDLib::TBoolean
                                ,       TCQCIntfView&)
{
    // No-op at design time
}


// Always just report that we are in designer mode
tCQCIntfEng::EAppFlags TIntfEditWnd::eAppFlags() const
{
    return tCQCIntfEng::EAppFlags::DesMode;
}


//
//  This is used to validate in place edited values. There are a few special cases we
//  need to react to. We do the same in the attribute editor notification handler so
//  we call a common method.
//
tCIDLib::TVoid
TIntfEditWnd::IPEValChanged(const   tCIDCtrls::TWndId   widSrc
                            , const TString&            strSrc
                            , const TAttrData&          adatNew
                            , const TAttrData&          adatOld
                            , const tCIDLib::TCard8     c8UserId)
{
    // Look up the target widget and store the new data
    tCIDLib::TCard4 c4ZOrder;
    TCQCIntfWidget* piwdgTar = iwdgEdit().piwdgFindByDesId(c8UserId, c4ZOrder);
    CIDAssert(piwdgTar != nullptr, L"The target widget could not be found");

    CheckAttrChange(*piwdgTar, adatOld, adatNew);
}


tCIDLib::TVoid
TIntfEditWnd::NewTemplate(  const   TCQCIntfTemplate&
                            ,       TCQCIntfView&
                            , const tCIDLib::TCard4)
{
    // No-op at design time
}


MCQCCmdTracer* TIntfEditWnd::pmcmdtTrace()
{
    // No-op at design time
    return 0;
}


// ---------------------------------------------------------------------------
//  TIntfEditWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Compare the current contents to the last saved (or original) contents.
tCIDLib::TBoolean TIntfEditWnd::bChanges() const
{
    return !iwdgEdit().bIsSame(m_iwdgOrg);
}


// We save our current template contents to the data server
tCIDLib::TBoolean TIntfEditWnd::bSaveChanges(TString& strErrMsg)
{
    //
    //  Run a validation pass first, and load up the errors to the error list window if
    //  we fail. If we do, then just give back a 'there are ourstanding errors' error
    //  since we already have the errors displayed.
    //
    tCQCIntfEng::TErrList colList;
    const tCQCIntfEng::EValTypes eValType = iwdgEdit().eDoValidation(colList);
    m_pwndErrList->LoadErrors(colList);
    if (eValType == tCQCIntfEng::EValTypes::Errors)
    {
        strErrMsg = facCQCIntfEd().strMsg(kIEdErrs::errcVal_ValErrors);
        return kCIDLib::False;
    }

    try
    {
        // Flatten it to a buffer
        THeapBuf mbufTmpl(32* 1024UL);
        tCIDLib::TCard4 c4TmplBytes = 0;
        {
            TBinMBufOutStream strmTar(&mbufTmpl);
            strmTar << iwdgEdit() << kCIDLib::FlushIt;
            c4TmplBytes = strmTar.c4CurSize();
        }

        tCIDLib::TCard4         c4SerialNum = 0;
        tCIDLib::TEncodedTime   enctLastChange;
        TDataSrvClient          dsclWrite;
        tCIDLib::TKVPFList      colXMeta;
        dsclWrite.WriteTemplate
        (
            iwdgEdit().strTemplateName()
            , c4SerialNum
            , enctLastChange
            , mbufTmpl
            , c4TmplBytes
            , iwdgEdit().eMinRole()
            , iwdgEdit().strNotes()
            , kCQCRemBrws::c4Flag_OverwriteOK
            , colXMeta
            , m_pcivEdit->cuctxToUse().sectUser()
        );

        // Save the current stuff as the original content
        m_iwdgOrg = iwdgEdit();

        // And clear the undo stack, so they can't under past this
        m_uhEditor.Reset();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TIntfEditWnd::CreateEditWnd(        TScrollArea&            wndParent
                            ,       TCQCIntfEdView* const   pcivEdit
                            ,       TIntfAttrEdWnd* const   pwndAttrEd
                            ,       TIntfErrListWnd* const  pwndErrList
                            ,       TIntfWidgetPalTab*const pwndWdgPal
                            , const tCIDCtrls::TWndId       widToUse)
{
    // Store the view object and attr editor for later
    m_pcivEdit = pcivEdit;
    m_pwndAttrEd = pwndAttrEd;
    m_pwndErrList = pwndErrList;
    m_pwndWdgPal = pwndWdgPal;

    // Assign this guy the next available template design time id
    pcivEdit->iwdgBaseTmpl().c8DesId
    (
        TRawBits::c8From32(0, CQCIntfEd_EditWnd::c4NextTmplDesId++)
    );

    //
    //  And ask it to number the current widgets sequentially from our
    //  next available widget id. It will return the next unused id
    //  which we save away for the next time.
    //
    m_c4NextWId = pcivEdit->iwdgBaseTmpl().c4AssignDesIdsFrom(m_c4NextWId);

    // The widget palette can now load his list now that the design time ids are set
    m_pwndWdgPal->LoadWidgets();

    //
    //  We need to register a custom class so that we can get double clicks. Only
    //  do it once, even if more than one editor window is created.
    //
    static const tCIDLib::TCh* pszClass = L"CQCEditWnd";
    static TAtomicFlag atomInitDone;
    if (!atomInitDone)
    {
        // Lock while we do this
        TBaseLock lockInit;
        if (!atomInitDone)
        {
            const TRGBClr rgbBgn = facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window);
            RegWndClass
            (
                pszClass
                , kCIDLib::False
                , kCIDLib::True
                , 0
                , rgbBgn
                , kCIDLib::False
            );
            atomInitDone.Set();
        }
    }


    //
    //  Set our size to fit the template, plus some extra to allow for sizing up
    //  via drag. Initially invisible until we get the tempalte set up. We are in
    //  a scroll area so it'll make the extra bits available.
    //
    TArea areaNew(pcivEdit->iwdgBaseTmpl().areaRelative());
    areaNew.AdjustSize(256, 256);
    TWindow::CreateWnd
    (
        wndParent.hwndThis()
        , pszClass
        , L""
        , areaNew
        , tCIDCtrls::EWndStyles::ClippingTabChild
        , tCIDCtrls::EExWndStyles::None
        , widToUse
    );

    // Set us as the scroll window child
    wndParent.SetChild(this);
    wndParent.SetChildSize(areaNew.szArea());

    //
    //  Initialize the template, which will recursively initialize the children.
    //  This is it's chance to load up any resources or do other things that
    //  require access to the window, and it will set us as the owner window
    //  and we'll set the designer mode.
    //
    //  NOTE:   We can pass a null polling engine here, since in designer mode
    //          no field registration is done.
    //
    //  NOTE:   We have to pass in a data server client, but it won't get used
    //          for loading any more templates since we don't do that in
    //          designer mode. It'll be used for images where needed.
    //
    TCQCFldCache cfcData;
    cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));

    TDataSrvClient dsclLoad;
    tCQCIntfEng::TErrList colErrs;
    const tCIDLib::TBoolean bRes = pcivEdit->iwdgBaseTmpl().bInitTemplate
    (
        m_pcivEdit
        , TCQCPollEngine::Nul_TCQCPollEngine()
        , dsclLoad
        , cfcData
        , colErrs
        , TString::strEmpty()
    );

    //
    //  Force the origin to zero since we stopped allowing them move the interface off
    //  the origin during design. DO THIS AFTER we initialize above, so that the view
    //  is set on the widgets.
    //
    if (bRes)
        pcivEdit->iwdgBaseTmpl().SetPositions(TPoint::pntOrigin, TPoint::pntOrigin);

    // Do an initial validation pass
    Validate();

    // We want to register for events from the editor window
    pwndAttrEd->pnothRegisterHandler(this, &TIntfEditWnd::eAttrEditHandler);

    // Now show us
    SetVisibility(kCIDLib::True);

    //
    //  Save a copy of the template for later changes checking.
    //
    //  DO THIS AT THE END, so that all of the changes made during init will be captured
    //  when we copy it.
    //
    m_iwdgOrg = iwdgEdit();

    // Select the template
    AddToSelection(&iwdgEdit(), kCIDLib::True, kCIDLib::True);
}


//
//  Any tools that send click events are sent here. Since we could get them from multiple
//  windows that we don't control the ids of, they all have to set a window name and we
//  use that to figure out what we got.
//
//  The helpers we call set up their own undos.
//
tCIDCtrls::EEvResponses TIntfEditWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    const TString& strSrcName = wnotEvent.strSrcName();

    if ((strSrcName == kCQCIntfEd_::strTool_FontBold)
    ||  (strSrcName == kCQCIntfEd_::strTool_FontDecrease)
    ||  (strSrcName == kCQCIntfEd_::strTool_FontFace)
    ||  (strSrcName == kCQCIntfEd_::strTool_FontIncrease)
    ||  (strSrcName == kCQCIntfEd_::strTool_FontItalic))
    {
        ApplyFontTool(strSrcName);
    }
     else
    {
        ApplyAlignTool(strSrcName);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// If the passed designer id is still valid, make that the selected widget
tCIDLib::TVoid TIntfEditWnd::SelectByDesId(const tCIDLib::TCard8 c8DesId)
{
    TCQCIntfWidget* piwdgTar = m_pcivEdit->piwdgFindByDesId(c8DesId);
    if (piwdgTar)
        AddToSelection(piwdgTar, kCIDLib::True, kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TIntfEditWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TIntfEditWnd::bCreated()
{
    if (!TParent::bCreated())
        return kCIDLib::False;

    //
    //  Load our popup menu. We just keep it around and enable and disable items
    //  as appropriate for what's selected.
    //
    m_menuEdit.Create(facCQCIntfEd(), kCQCIntfEd::ridMenu_Edit);

    //
    //  The items in the New sub-menu are faulted in since they will change
    //  based on what is implemented down in the engine.
    //
    {
        TSubMenu menuNew(m_menuEdit, kCQCIntfEd::ridMenu_Edit_New);
        facCQCIntfEng().FillNewMenu(menuNew, kCQCIntfEd_::c4FirstNewId);
    }

    // Set up our accelerator keys
    tCIDLib::TStrList colKeyStrs;
    tCIDLib::TCardList fcolCmds;

    colKeyStrs.objAdd(L"Delete");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Delete);

    colKeyStrs.objAdd(L"Ctrl-C");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Copy);
    colKeyStrs.objAdd(L"Ctrl-X");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Cut);
    colKeyStrs.objAdd(L"Ctrl-V");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Paste);

    colKeyStrs.objAdd(L"Ctrl-A");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_SelectAll);

    colKeyStrs.objAdd(L"Ctrl-H");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Replace);

    colKeyStrs.objAdd(L"PageDn");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_MoveBack);
    colKeyStrs.objAdd(L"PageUp");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_MoveForward);

    colKeyStrs.objAdd(L"Ctrl--");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Ungroup);
    colKeyStrs.objAdd(L"Ctrl-Minus");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Ungroup);
    colKeyStrs.objAdd(L"Ctrl-+");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Group);
    colKeyStrs.objAdd(L"Ctrl-Plus");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Group);

    colKeyStrs.objAdd(L"Ctrl-Z");
    fcolCmds.c4AddElement(kCQCIntfEd::ridMenu_Edit_Undo);

    colKeyStrs.objAdd(L"Ctrl-Alt--");
    fcolCmds.c4AddElement(CQCIntfEd_EditWnd::ridMenu_Faux_FontSize_Dec);
    colKeyStrs.objAdd(L"Ctrl-Alt-Minus");
    fcolCmds.c4AddElement(CQCIntfEd_EditWnd::ridMenu_Faux_FontSize_Dec);
    colKeyStrs.objAdd(L"Ctrl-Alt-+");
    fcolCmds.c4AddElement(CQCIntfEd_EditWnd::ridMenu_Faux_FontSize_Inc);
    colKeyStrs.objAdd(L"Ctrl-Alt-Plus");
    fcolCmds.c4AddElement(CQCIntfEd_EditWnd::ridMenu_Faux_FontSize_Inc);

    colKeyStrs.objAdd(L"Ctrl-Alt-H");
    fcolCmds.c4AddElement(CQCIntfEd_EditWnd::ridMenu_Faux_HJustify);
    colKeyStrs.objAdd(L"Ctrl-Alt-V");
    fcolCmds.c4AddElement(CQCIntfEd_EditWnd::ridMenu_Faux_VJustify);

    // Load the table now
    m_accelEdit.Create(colKeyStrs, fcolCmds);

    // Indicate we want to see arrows and keys
    SetWantInput(tCIDCtrls::EWantInputs::CharsArrows);

    //
    //  Register for the palette window's change msgs. It may not exist yet, so don't
    //  require it.
    //
    SubscribeToTopic(m_strWdgPalTopic, kCIDLib::False);

    // Subscribe to changes from the template's widget list
    SubscribeToTopic(m_pcivEdit->iwdgBaseTmpl().strPublishTopic(), kCIDLib::False);

    // Subscribe to selects in the error list
    SubscribeToTopic(m_pwndErrList->strPublishTopic(), kCIDLib::True);

    //
    //  Set up the selection list to publish changes and subscribe to that for our own
    //  internal use. We'll turn around and publish something else in return.
    //
    m_strSelectionTopic = L"/CQC/IntfEng/SelChange/";
    m_strSelectionTopic.AppendFormatted(facCIDLib().c4NextId());
    m_selList.SetPubTopic(m_strSelectionTopic);
    SubscribeToTopic(m_strSelectionTopic, kCIDLib::True);

    // And create our own update topic that we use to report changes
    m_pstopUpdates = TPubSubTopic::pstopCreateTopic
    (
        m_strUpdateTopic, TIntfEdUpdateTopic::clsThis()
    );

    return kCIDLib::True;
}


//
//  No need for this. Our paint draws the whole window contents, so we just
//  override this and say we did it. This reduces flicker as well.
//
tCIDLib::TBoolean TIntfEditWnd::bEraseBgn(TGraphDrawDev& gdevToUse)
{
    return kCIDLib::True;
}


//
//  We just update the pointer to reflect where it is over any given widget,
//  indicating the drag/move operations possible at that point.
//
tCIDLib::TBoolean
TIntfEditWnd::bMouseMove(const  TPoint&             pntAt
                        , const tCIDLib::TBoolean   bControlDown
                        , const tCIDLib::TBoolean   bShiftDown)
{
    tCIDCtrls::ESysPtrs ePtr = tCIDCtrls::ESysPtrs::Arrow;

    // See if we are over anything and publish a mouse over if needed
    tCIDLib::TCard8 c8DesId;
    TCQCIntfWidget* piwdgTarget = piwdgHitTest(pntAt, c8DesId);
    PublishMouseOverMsg(piwdgTarget);

    if (m_eDragMode == EDragModes::Size)
    {
        // Adjust our tracking area based on the drag area
        TArea areaPrev = m_areaDrag;

        //
        //  Note that in all of the ones where we adjust the left/top, we
        //  tell it to lock the bottom/right, since we aren't moving, we are
        //  sizing.
        //
        switch(m_eDragArea)
        {
            case tCIDCtrls::EWndAreas::Bottom :
                m_areaDrag.i4Bottom(pntAt.i4Y());
                break;

            case tCIDCtrls::EWndAreas::Left :
                m_areaDrag.i4X(pntAt.i4X(), kCIDLib::True);
                break;

            case tCIDCtrls::EWndAreas::BottomLeft :
                m_areaDrag.i4X(pntAt.i4X(), kCIDLib::True);
                m_areaDrag.i4Bottom(pntAt.i4Y());
                break;

            case tCIDCtrls::EWndAreas::TopLeft :
                m_areaDrag.i4X(pntAt.i4X(), kCIDLib::True);
                m_areaDrag.i4Top(pntAt.i4Y(), kCIDLib::True);
                break;

            case tCIDCtrls::EWndAreas::Right :
                m_areaDrag.i4Right(pntAt.i4X());
                break;

            case tCIDCtrls::EWndAreas::BottomRight :
                m_areaDrag.i4Right(pntAt.i4X());
                m_areaDrag.i4Bottom(pntAt.i4Y());
                break;

            case tCIDCtrls::EWndAreas::TopRight :
                m_areaDrag.i4Top(pntAt.i4Y(), kCIDLib::True);
                m_areaDrag.i4Right(pntAt.i4X());
                break;

            case tCIDCtrls::EWndAreas::Top :
                m_areaDrag.i4Top(pntAt.i4Y(), kCIDLib::True);
                break;

            default :
                CancelDrag(kCIDLib::True);
                CIDAssert2(L"Unknown drag area")
                return kCIDLib::True;
        };

        // Force a redraw of the larger of the two areas
        areaPrev.TakeLarger(m_areaDrag);
        m_pcivEdit->Redraw(areaPrev);
    }
     else if (m_eDragMode == EDragModes::Select)
    {
        //
        //  We have to adjust potentially any corner, based on where the new
        //  point is relative to the starting point.
        //
        TArea areaPrev = m_areaDrag;
        if (pntAt.i4X() <= m_pntDragOfs.i4X())
        {
            m_areaDrag.i4X(pntAt.i4X());
            m_areaDrag.i4Right(m_pntDragOfs.i4X());
        }
         else
        {
            m_areaDrag.i4X(m_pntDragOfs.i4X());
            m_areaDrag.i4Right(pntAt.i4X());
        }

        if (pntAt.i4Y() <= m_pntDragOfs.i4Y())
        {
            m_areaDrag.i4Y(pntAt.i4Y());
            m_areaDrag.i4Bottom(m_pntDragOfs.i4Y());
        }
         else
        {
            m_areaDrag.i4Y(m_pntDragOfs.i4Y());
            m_areaDrag.i4Bottom(pntAt.i4Y());
        }

        // Redraw the area that contains old and new rectangles
        areaPrev |= m_areaDrag;
        m_pcivEdit->Redraw(areaPrev);
    }
     else if (m_eDragMode == EDragModes::Move)
    {
        //
        //  Adjust the origin of the drag area. Basically we just take the
        //  current area and set its origin to the current point minus the
        //  drag offset. Remember the previous area first so that we can
        //  redraw the minimum area.
        //
        TArea areaPrev = m_areaDrag;
        m_areaDrag.SetOrg(pntAt - m_pntDragOfs);

        //
        //  Calc the new area so we know what to redraw. It's the combination
        //  of the two areas.
        //
        TArea areaUpdate(areaPrev);
        areaUpdate |= m_areaDrag;

        // Adjust the drag area now and then redraw
        m_pcivEdit->Redraw(areaUpdate);
    }
     else
    {
        // If we hit a widget and it's not locked, hit test it
        if (piwdgTarget && !piwdgTarget->bIsLocked())
        {
            // We hit something, so see what pointer to use
            const tCIDCtrls::EWndAreas eArea = piwdgTarget->eHitTest(pntAt);

            //
            //  If it's the main template, only change the cursor for the
            //  lower and right edges.
            //
            if ((piwdgTarget != &iwdgEdit())
            ||  (eArea == tCIDCtrls::EWndAreas::Right)
            ||  (eArea == tCIDCtrls::EWndAreas::Bottom)
            ||  (eArea == tCIDCtrls::EWndAreas::BottomRight))
            {
                ePtr = ePtrForArea(eArea);
            }
        }
         else
        {
            c8DesId = kCIDLib::c8MaxCard;
        }
    }

    facCIDCtrls().SetPointer(ePtr);
    return kCIDLib::True;
}


tCIDLib::TBoolean
TIntfEditWnd::bPaint(TGraphDrawDev& gdevTarget, const TArea& areaUpdate)
{
    //
    //  If in a drag mode we handle those specially, else we just redraw the
    //  affected widgets.
    //
    if (m_eDragMode != EDragModes::None)
    {
        // Copy back over the affected original window contents
        {
            TGraphMemDev gdevTrack(gdevTarget, m_bmpDrag);
            gdevTarget.CopyBits(gdevTrack, areaUpdate, areaUpdate);
        }

        // And draw the selection rectangle if not empty
        if (!m_areaDrag.bIsEmpty())
        {
            TPenJanitor  janPen(&gdevTarget, &m_gpenDrag);
            gdevTarget.eMixMode(tCIDGraphDev::EMixModes::XorPen);
            gdevTarget.Stroke(m_areaDrag);
        }
    }
     else
    {
        const TArea& areaBgn = areaClient();

        //
        //  If the template we are doing has a transparent background setting,
        //  then fill the overlap of the update area and the template area with
        //  a checkerboard pattern.
        //
        const TArea& areaTmpl(iwdgEdit().areaActual());
        TArea areaClip;
        TGUIRegion grgnClip;
        if (iwdgEdit().bIsTransparent())
        {
            areaClip = areaTmpl;
            areaClip &= areaUpdate;
            if (!areaClip.bIsEmpty())
            {
                grgnClip.Set(areaClip);
                TRegionJanitor janClip
                (
                    &gdevTarget, grgnClip, tCIDGraphDev::EClipModes::Copy
                );
                gdevTarget.FillWithBmpPattern
                (
                    areaBgn, m_bmpChecker, tCIDGraphDev::EBmpModes::PatCopy
                );
            }
        }

        //
        //  Fill any area outside of the template with the workspace color if
        //  the template is smaller than the window in either direction.
        //
        if ((areaTmpl.c4Width() < areaBgn.c4Width())
        ||  (areaTmpl.c4Height() < areaBgn.c4Height()))
        {
            grgnClip.Set(areaTmpl);
            TRegionJanitor janClip
            (
                &gdevTarget, grgnClip, tCIDGraphDev::EClipModes::Xor
            );
            gdevTarget.Fill
            (
                areaUpdate
                , facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::AppWorkspace)
            );
        }

        //
        //  Draw a contrast line just outside the area, so that the flyover
        //  border doesn't get lost.
        //
        {
            TArea areaContrast(areaTmpl);
            areaContrast.Inflate(1);
            gdevTarget.Stroke(areaContrast, facCIDGraphDev().rgbWhite);
        }

        //
        //  Clip the output to the intersection of the update area and the
        //  template area.
        //
        areaClip = areaTmpl;
        areaClip &= areaUpdate;
        grgnClip.Set(areaClip);
        TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::Copy);

        //
        //  Ask the main widget to update itself. Put a janitor on the device
        //  so that we get back to the original settings.
        //
        {
            TGraphicDevJanitor janDev(&gdevTarget);
            iwdgEdit().QueryClipRegion(grgnClip);
            iwdgEdit().DrawOn(gdevTarget, areaUpdate, grgnClip);

            //
            //  If this guy is in the selection list, then, draw the
            //  selection handles on it.
            //
            DrawSelHandlesIfSelected(gdevTarget, iwdgEdit().c8DesId());
        }

        //
        //  And ask all of the child widgets that intersect the update area to
        //  update. We create an update area for each one that is the interaction
        //  of the update area and its area.
        //
        const tCIDLib::TCard4 c4Count = iwdgEdit().c4ChildCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCIntfWidget& iwdgCur = iwdgEdit().iwdgAt(c4Index);

            //
            //  If it intersects the invalid area and the template area (it
            //  could be off the edge if they sized the template down, then
            //  update it. We have to create an area that represents the part
            //  of the actual widget's area that is invalid. We save and
            //  restore the content of the device each time, so that each
            //  one gets a device with default values.
            //
            if (iwdgCur.bIntersects(areaUpdate) && iwdgCur.bIntersects(areaTmpl))
            {
                TArea areaInvalid(areaUpdate);
                areaInvalid &= iwdgCur.areaActual();

                if (!areaInvalid.bIsEmpty())
                {
                    TGraphicDevJanitor janDev(&gdevTarget);
                    iwdgCur.QueryClipRegion(grgnClip);
                    iwdgCur.DrawOn(gdevTarget, areaInvalid, grgnClip);

                    //
                    //  If this guy is in the selection list, then, draw the
                    //  selection handles on it.
                    //
                    DrawSelHandlesIfSelected(gdevTarget, iwdgCur.c8DesId());
                }
            }
        }
    }
    return kCIDLib::True;
}


//
//  Handle click events. This is pretty gnarly because of all of the possible states we
//  can be in, limitations on what can be done in certain states, dealing with groups,
//  and so forth. We don't even try to be clever and so we have some duplicate code. But
//  it would be too hard to do otherwise.
//
tCIDLib::TBoolean
TIntfEditWnd::bClick(const  tCIDCtrls::EMouseButts  eButton
                    , const tCIDCtrls::EMouseClicks eClickType
                    , const TPoint&                 pntAt
                    , const tCIDLib::TBoolean       bCtrlShift
                    , const tCIDLib::TBoolean       bShift)
{
    // Take the focus on any down click
    if (eClickType == tCIDCtrls::EMouseClicks::Down)
        TakeFocus();

    //
    //  If any shifts, can't be right click. Eat it so it doesn't invoke our context menu
    //
    const tCIDLib::TBoolean bAltShift = facCIDCtrls().bAltShifted();
    if ((eButton == tCIDCtrls::EMouseButts::Right) && (bCtrlShift || bShift || bAltShift))
           return kCIDLib::True;

    // If any shifts can't be any double click
    if ((bAltShift || bCtrlShift || bShift) && (eClickType == tCIDCtrls::EMouseClicks::Double))
        return kCIDLib::True;

    // If all shifts can't be anything valid, but let it pass in case it's some system thing
    if (bAltShift && bCtrlShift && bShift)
        return kCIDLib::False;

    //
    //  Push an undo separator to bracket the various undos that might happen. It's
    //  possible we may never push anything, but that's fine. This will just end up
    //  getting ignored.
    //
    m_uhEditor.PushSep();

    //
    //  If an up click, we only use those for one thing, which is to sense the end of
    //  a drag operation. So let's handle those up front, and it doesn't depend on what
    //  was hit.
    //
    TCQCIntfTemplate& iwdgBase = iwdgEdit();
    if (eClickType == tCIDCtrls::EMouseClicks::Up)
    {
        if (m_eDragMode != EDragModes::None)
        {
            //
            //  Save the incoming drag mode then reset just in case something
            //  goes wrong. We don't want to get stuck in this mode.
            //
            const EDragModes eMode = m_eDragMode;
            CancelDrag(kCIDLib::False);

            // Based on the drag type, do what is required
            if (eMode == EDragModes::Move)
            {
                //
                //  Call a helper to do this. The area of the selection is the original
                //  area against which the new drag area will be compared to figure out
                //  the movement.
                //
                const TArea areaOrg = areaSelection();
                AdjustWidgetAreas(areaOrg, m_areaDrag);

                const TArea areaNew = areaSelection();
                if (areaNew.bIntersects(areaOrg))
                {
                    m_pcivEdit->Redraw(areaOrg | areaNew);
                }
                 else
                {
                    m_pcivEdit->Redraw(areaOrg);
                    m_pcivEdit->Redraw(areaNew);
                }
            }
             else if (eMode == EDragModes::Select)
            {
                // Redraw the drag area to get rid of the marquee
                m_pcivEdit->Redraw(m_areaDrag);

                // Select the contained widgets, replacing any current selection
                SelectContained(m_areaDrag, kCIDLib::True);
            }
             else if (eMode == EDragModes::Size)
            {
                //
                //  The target widget is the single selected one, so we need grab it and
                //  change it's size/position.
                //
                TCQCIntfWidget* piwdgSize = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(0));

                //
                //  In this case we have to manually push an area change attribute undo,
                //  since it didn't come from the attribute editor. Though we only have
                //  one widget changed, we use the multi-pos undo since it's the most
                //  convenient for us and works fine for a onsie.
                //
                m_uhEditor.Push(new TIntfMultiPosUndo(iwdgBase, m_selList.fcolIds()));

                //
                //  If the template is selected, then we cannot let it get smaller
                //  than the contained widgets. And we have to size our window to fit.
                //  If a regular widget, we just don't allow it to be reduced below
                //  a minimum size.
                //
                if (piwdgSize->c8DesId() == iwdgBase.c8DesId())
                {
                    const TArea areaMin = iwdgBase.areaHull();
                    m_areaDrag.TakeLarger(areaMin);
                    piwdgSize->SetArea(m_areaDrag, kCIDLib::True);

                    //
                    //  Make the actual window a bit larger, so that the user can size
                    //  the template up.
                    //
                    TSize szWnd = m_areaDrag.szArea();
                    szWnd.Adjust(256, 256);
                    SetSize(szWnd, kCIDLib::False);
                }
                 else
                {
                    if (m_areaDrag.c4Width() < kCQCIntfEd_::c4MinSize)
                        m_areaDrag.c4Width(kCQCIntfEd_::c4MinSize);
                    if (m_areaDrag.c4Height() < kCQCIntfEd_::c4MinSize)
                        m_areaDrag.c4Height(kCQCIntfEd_::c4MinSize);

                    const TArea areaOrg(piwdgSize->areaActual());
                    piwdgSize->SetArea(m_areaDrag, kCIDLib::True);

                    if (m_areaDrag.bIntersects(areaOrg))
                    {
                        m_pcivEdit->Redraw(areaOrg | m_areaDrag);
                    }
                     else
                    {
                        m_pcivEdit->Redraw(areaOrg);
                        m_pcivEdit->Redraw(m_areaDrag);
                    }
                }
            }

            return kCIDLib::True;
        }

        // Wasn't ours so let it propogate
        return kCIDLib::False;
    }

    // Figure out which widget we hit, if any
    tCIDLib::TCard8 c8HitId;
    TCQCIntfWidget* piwdgHit = piwdgHitTest(pntAt, c8HitId);

    //
    //  If they didn't hit anything, it's easy. Just remove any selection
    //  and we are done. Don't do this if we are dragging (in case they
    //  hit the other button by mistake.)
    //
    if (!piwdgHit)
    {
        if (m_eDragMode == EDragModes::None)
            RemoveSelection();
        return kCIDLib::True;
    }

    // Do a hit test on it
    const tCIDCtrls::EWndAreas eArea = piwdgHit->eHitTest(pntAt);

    //
    //  If no viable hit area, do nothing. Shouldn't generally happen, but if
    //  a widget doesn't support sizing in a particular direction it might report
    //  no area being hit.
    //
    if (eArea == tCIDCtrls::EWndAreas::Nowhere)
        return kCIDLib::True;


    //
    //  If there is a single widget selected and it is the one that was hit, and it
    //  has associated actions, then invoke the action editor on double left click
    //  as a quick way to access that very common functionality. The initial click
    //  of the two would typically have selected the widget.
    //
    if (eClickType == tCIDCtrls::EMouseClicks::Double)
    {
        if ((eButton == tCIDCtrls::EMouseButts::Left) && m_selList.bSingleSel())
        {
            TCQCIntfWidget* piwdgSel = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(0));
            if (piwdgSel != piwdgHit)
                return kCIDLib::True;

            MCQCCmdSrcIntf* pcsrcTar = dynamic_cast<MCQCCmdSrcIntf*>(piwdgHit);
            if (pcsrcTar)
            {
                // Gather up a list of named widgets as the extra action cmd targets
                tCQCKit::TCmdTarList colExtra(tCIDLib::EAdoptOpts::NoAdopt);
                m_pcivEdit->iwdgBaseTmpl().FindAllTargets(colExtra, kCIDLib::False);

                //
                //  Always add the owning view, since he represents the interface viewer
                //  target (for link operations and such.)
                //
                colExtra.Add(m_pcivEdit);

                // And invoke the action editor dialog.
                const tCIDLib::TBoolean bChanged = facCQCIGKit().bEditAction
                (
                    *this
                    , *pcsrcTar
                    , kCIDLib::False
                    , m_pcivEdit->cuctxToUse()
                    , &colExtra
                    , &m_pcivEdit->iwdgBaseTmpl().strTemplateName()
                );
            }
        }
        return kCIDLib::True;
    }


    // Remember various bits of information for later use below
    const tCIDLib::TBoolean bHitTmpl(piwdgHit->c8DesId() == iwdgBase.c8DesId());
    const tCIDLib::TBoolean bTmplSelected
    (
        m_selList.bSingleSel() && (m_selList.c8At(0) == iwdgBase.c8DesId())
    );
    const tCIDLib::TBoolean bInGroup = piwdgHit->bIsGrouped();
    const tCIDLib::TBoolean bWdgInSel = m_selList.bIsSelected(piwdgHit->c8DesId());

    //
    //  The big split is left vs. right button. Those are the most different
    //  in terms of how we react.
    //
    if (eButton == tCIDCtrls::EMouseButts::Left)
    {
        if (bHitTmpl)
        {
            //
            //  If the template isn't the already selected one, then remove any
            //  existing selection.
            //
            if (!bTmplSelected)
                RemoveSelection(kCIDLib::False);

            if (bShift && !bCtrlShift)
            {
                // We are toggling it in/out of the selection
                if (bTmplSelected)
                    RemoveSelection();
                else
                    AddToSelection(piwdgHit, kCIDLib::True, kCIDLib::False);
            }
             else if (!bShift && !bCtrlShift)
            {
                //
                //  If on the bottom and/or right, then we can size the template.
                //  Else, let's just watch for drag detect and do a widget
                //  selection if so.
                //
                if ((eArea == tCIDCtrls::EWndAreas::Bottom)
                ||  (eArea == tCIDCtrls::EWndAreas::Right)
                ||  (eArea == tCIDCtrls::EWndAreas::BottomRight))
                {
                    // Select the template if not already
                    if (!bTmplSelected)
                        AddToSelection(piwdgHit, kCIDLib::True, kCIDLib::False);

                    // Allow the template to be resized
                    StartSize(pntAt, *piwdgHit, eArea);
                }
                 else if (bDragDetect(pntAt))
                {
                    // Put us into marquee select mode
                    StartMarqueeSel(pntAt);
                }
                 else if (!bTmplSelected)
                {
                    // If the template wasn't already selected, do so
                    AddToSelection(piwdgHit, kCIDLib::True, kCIDLib::True);

                    //
                    //  Let this one propagate so that we can also see a double
                    //  click to invoke the action editor potentially.
                    //
                    return kCIDLib::False;
                }
            }
        }
         else if (piwdgHit->bIsLocked())
        {
            //
            //  This is a special case, we only support the start of a lasso select
            //  here. They've clicked on a locked widget, so we don't select it, but we
            //  do unselect anything else. So we just watch for drag start.
            //
            RemoveSelection();
            if (bDragDetect(pntAt))
                StartMarqueeSel(pntAt);
        }
         else if (bAltShift && !bCtrlShift && !bShift)
        {
            //
            //  It's the special drill down scenario. We know it's not the template
            //  in this case, since that was checked for above.
            //
            DrillDown(pntAt, *piwdgHit);
        }
         else if (bInGroup)
        {
            // If the template is selected, deselect it
            if (bTmplSelected)
                RemoveSelection();

            if (!bCtrlShift && !bShift)
            {
                if (bWdgInSel
                &&  (eArea != tCIDCtrls::EWndAreas::Nowhere)
                &&  bDragDetect(pntAt))
                {
                    //
                    //  It's unshifted and they hit a selected one, so we deal wihh
                    //  a move or size.
                    //
                    if (eArea == tCIDCtrls::EWndAreas::Client)
                        StartMove(pntAt);
                    else if (m_selList.bSingleSel())
                        StartSize(pntAt, *piwdgHit, eArea);
                }
                 else
                {
                    // Just add the group to selection
                    AddGrpToSelection(piwdgHit->c4GroupId(), kCIDLib::True);
                }
            }
             else if (bShift && !bCtrlShift)
            {
                // Toggle the whole group
                if (bWdgInSel)
                    DeselectGrp(piwdgHit->c4GroupId());
                else
                    AddGrpToSelection(piwdgHit->c4GroupId(), kCIDLib::False);
            }
             else if (bCtrlShift && bShift)
            {
                // Toggle the hit one
                if (bWdgInSel)
                    Deselect(piwdgHit);
                else
                    AddToSelection(piwdgHit, kCIDLib::False, kCIDLib::False);
            }
             else if (bCtrlShift && !bShift)
            {
                // Just select the hit one, removing any previous
                AddToSelection(piwdgHit, kCIDLib::True, kCIDLib::True);
            }
        }
         else if (!bShift && !bCtrlShift)
        {
            //
            //  If the widget is in the selection, and we got a hit, then let's watch
            //  for a drag. If so, then we start a move or size.
            //
            if (bWdgInSel && (eArea != tCIDCtrls::EWndAreas::Nowhere) && bDragDetect(pntAt))
            {
                if (eArea == tCIDCtrls::EWndAreas::Client)
                    StartMove(pntAt);
                else if (m_selList.bSingleSel())
                    StartSize(pntAt, *piwdgHit, eArea);
            }
             else
            {
                //
                //  At this point we have to just be selecting the target, so remove
                //  any current and select the target.
                //
                AddToSelection(piwdgHit, kCIDLib::True, kCIDLib::True);

                // Let it propagate so that a double click can occur as well
                return kCIDLib::False;
            }
        }
         else if (bShift)
        {
            //
            //  We ignore Ctrl here so that they can shift-select both grouped and non-
            //  grouped widgets without having to make the distinction. Though, in the
            //  grouped one above, no Ctrl means the whole group is selected/toggled.
            //

            // I the template is selected, then deselect it
            if (bTmplSelected)
                RemoveSelection();

            //
            //  We are just toggling the hit one into or out of the selection. We
            //  know it's not in a group since that was handled separately above.
            //
            if (bWdgInSel)
                Deselect(piwdgHit);
            else
                AddToSelection(piwdgHit, kCIDLib::False, kCIDLib::False);
        }
    }
     else if (eButton == tCIDCtrls::EMouseButts::Right)
    {
        //
        //  If they hit something that's not already selected, then remove any current
        //  selection. If the new thing is not locked, make it the selected widget.
        //
        const tCIDLib::TBoolean bWasSel = bIsSelected(piwdgHit->c8DesId());
        if (!bWasSel)
        {
            RemoveSelection();
            if (!piwdgHit->bIsLocked())
            {
                //
                //  If it's a group member, and they not control shifted, then we need
                //  to select the whole group, or at least any not locked. Else just
                //  select the new guy.
                //
                if (piwdgHit->c4GroupId() && !bCtrlShift)
                    AddGrpToSelection(piwdgHit->c4GroupId(), kCIDLib::False);
                else
                    AddToSelection(piwdgHit, kCIDLib::False, kCIDLib::True);
            }
        }

        // Let it propagate so we get the context menu invocation
        return kCIDLib::False;
    }
     else
    {
        // Don't use anything left or right
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  We handle sizing/moving selected widgets via the arrow keys. Some other things are
//  handled via the accelerator table.
//
tCIDLib::TBoolean
TIntfEditWnd::bExtKey(  const   tCIDCtrls::EExtKeys     eKey
                        , const tCIDLib::TBoolean       bAlt
                        , const tCIDLib::TBoolean       bCtrl
                        , const tCIDLib::TBoolean       bShift
                        , const TWindow* const          )
{
    // Get the selected item count. If none, then nothing to do
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();

    //
    //  To make things simpler, we first do the character procoessing and
    //  set an enumerated value to indicate the operation to do.
    //
    enum ECharOps
    {
        EOp_None

        // Move operations
        , EOp_Down
        , EOp_Left
        , EOp_Right
        , EOp_Up

        // Size operations
        , EOp_HGrow
        , EOp_HShrink
        , EOp_VGrow
        , EOp_VShrink
    };

    // Based on the key and shifts, set the operation code
    ECharOps eOp = EOp_None;
    switch(eKey)
    {
        case tCIDCtrls::EExtKeys::Down :
            if (!bAlt && !bShift)
            {
                if (bCtrl)
                    eOp = EOp_VGrow;
                else
                    eOp = EOp_Down;
            }
            break;

        case tCIDCtrls::EExtKeys::Left :
            if (!bAlt && !bShift)
            {
                if (bCtrl)
                    eOp = EOp_HShrink;
                else
                    eOp = EOp_Left;
            }
            break;

        case tCIDCtrls::EExtKeys::Right :
            if (!bAlt && !bShift)
            {
                if (bCtrl)
                    eOp = EOp_HGrow;
                else
                    eOp = EOp_Right;
            }
            break;

        case tCIDCtrls::EExtKeys::Up :
            if (!bAlt && !bShift)
            {
                if (bCtrl)
                    eOp = EOp_VShrink;
                else
                    eOp = EOp_Up;
            }
            break;

        default :
            break;
    };

    // If not one of our guys or no selection, then let it propogate
    if ((eOp == EOp_None) || !c4Count)
        return kCIDLib::False;

    // See if the template is the selected widget.
    const tCIDLib::TBoolean bTmplSelected
    (
        m_selList.bSingleSel() && (m_selList.c8At(0) == iwdgEdit().c8DesId())
    );

    // If so, we can only honor size type operations
    if (bTmplSelected && (eOp < EOp_HGrow))
        return kCIDLib::True;

    // Push an undo sepator to brack any operations that might happen here
    m_uhEditor.PushSep();

    //
    //  Get the original area of the selected widgets, and a copy we can adjust
    const TArea areaOrg = areaSelection();
    TArea areaAdjust = areaOrg;

    TCQCIntfWidget* piwdgCur;
    if ((eOp >= EOp_Down) && (eOp <= EOp_Up))
    {
        // The limit of movement is the template area in this case
        const TArea areaLimit = iwdgEdit().areaActual();

        // Figure out the adjustment amount
        TPoint pntAdjust;
        switch(eOp)
        {
            case EOp_Right :
                pntAdjust.Set(1, 0);
                break;

            case EOp_Left :
                pntAdjust.Set(-1, 0);
                break;

            case EOp_Up :
                pntAdjust.Set(0, -1);
                break;

            case EOp_Down :
                pntAdjust.Set(0, 1);
                break;

            default :
                CIDAssert2(L"Unknown move operation")
                break;
        };

        // Adjust the move area and see if it's still legal
        areaAdjust.AdjustOrg(pntAdjust);
        if (areaLimit.bContainsArea(areaAdjust))
        {
            // Push a multi-undo before we change the positions
            m_uhEditor.Push(new TIntfMultiPosUndo(iwdgEdit(), m_selList.fcolIds()));

            //
            //  It is, so loop through the selected items and adjust them all by
            //  the adjustment.
            //
            TArea areaNew;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));

                // Get this guy's area and adjust the origin
                areaNew = piwdgCur->areaActual();
                areaNew += pntAdjust;

                // Update the area of the widget
                piwdgCur->SetArea(areaNew);
            }

            // We may have moved it out from under the mouse
            PublishMouseOverMsg();

            //
            //  Now we need to update the area that combines the original area and
            //  the new overall move area.
            //
            areaAdjust |= areaOrg;
            m_pcivEdit->Redraw(areaAdjust);
        }
    }
     else if ((eOp >= EOp_HGrow) && (eOp <= EOp_VShrink))
    {
        //
        //  The limit of movement is either the template (if it itself isn't
        //  selected, or this window area if the template itself is selected.)
        //
        //  We also have a minimum size. If the template is selected, then it's
        //  the smallest it can be without clipping any widgets. If it's not, then
        //  we set a small minimum size that we won't shrink any widgets below.
        //
        TArea areaLimit;
        TSize szMin;
        if (bTmplSelected)
        {
            areaLimit = areaWndSize();
            szMin = iwdgEdit().areaHull().szArea();
        }
         else
        {
            areaLimit = iwdgEdit().areaActual();
            szMin.Set(kCQCIntfEd_::c4MinSize, kCQCIntfEd_::c4MinSize);
        }

        // Figure out the adjustment amount
        TPoint pntAdjust;
        switch(eOp)
        {
            case EOp_HGrow :
                pntAdjust.Set(1, 0);
                break;

            case EOp_HShrink :
                pntAdjust.Set(-1, 0);
                break;

            case EOp_VGrow :
                pntAdjust.Set(0, 1);
                break;

            case EOp_VShrink :
                pntAdjust.Set(0, -1);
                break;

            default :
                CIDAssert2(L"Unknown size operation")
                break;
        };

        //
        //  Adjust the move area and see if it's still legal. We have a special case
        //  for shrinking where we don't allow the stuff to get too small.
        //
        areaAdjust.AdjustSize(pntAdjust);
        if (areaLimit.bContainsArea(areaAdjust))
        {
            // Push a multi-undo before we change the positions
            m_uhEditor.Push(new TIntfMultiPosUndo(iwdgEdit(), m_selList.fcolIds()));

            //
            //  It is, so loop through the selected items and adjust them all by
            //  the adjustment.
            //
            TArea areaNew;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));

                // Get this guy's area and adjust
                areaNew = piwdgCur->areaActual();
                areaNew.AdjustSize(pntAdjust);

                // If it hasn't gotten too small, then re-apply it
                if ((areaNew.c4Width() >= szMin.c4Width())
                &&  (areaNew.c4Height() >= szMin.c4Height()))
                {
                    piwdgCur->SetArea(areaNew);
                }
            }

            // We may have sized it down out from under the mouse
            PublishMouseOverMsg();

            //
            //  If it's the template, we have to resize the window. We force a redraw
            //  of the contents.
            //
            //  Else we redraw the combination of the old and new areas.
            //
            if (bTmplSelected)
            {
                areaAdjust.Inflate(1);
                TSize szWnd = areaAdjust.szArea();
                szWnd.Adjust(256, 256);
                SetSize(szWnd, kCIDLib::False);

                Redraw(tCIDCtrls::ERedrawFlags::ImmedErase);
            }
             else
            {
                areaAdjust |= areaOrg;
                m_pcivEdit->Redraw(areaAdjust);
            }
        }
    }

    return kCIDLib::True;
}


//
//  If there is a single widget selected, we show the documentation for that widget
//  type.
//
tCIDLib::TBoolean
TIntfEditWnd::bShowHelp(const tCIDCtrls::TWndId widCtrl, const TPoint& pntMouse)
{
    // Get the master server web server info. If we can't get that, can't do anything
    tCIDLib::TBoolean bSecure = kCIDLib::False;
    tCIDLib::TIPPortNum ippnWS = 0;
    TString strWSHost;
    if (!facCQCKit().bFindHelpWS(strWSHost, ippnWS, bSecure))
    {
        TErrBox msgbErr(L"The Master Server's Web Server information could not be obtained");
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }

    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (c4Count != 1)
        return kCIDLib::False;

    // Get the selected one
    const TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(0));

    // We can use its command target help id to build the URL
    TString strURL(bSecure ? L"https://" : L"http://");
    strURL.Append(strWSHost);
    strURL.Append(kCIDLib::chColon);
    strURL.AppendFormatted(ippnWS);
    strURL.Append(L"/Web2/CQCDocs/CQCDocs.html?");

    //
    //  We have to break out the last part as the page id and the other part
    //  is the reference.
    //
    TString strHelpRef = piwdgCur->strCmdHelpId();
    tCIDLib::TCard4 c4DivAt;
    TString strPageId;
    if (!strHelpRef.bLastOccurrence(kCIDLib::chForwardSlash, c4DivAt)
    ||  !c4DivAt)
    {
        // It can't be correct
        TErrBox msgbErr(L"The action target's help reference was invalid");
        msgbErr.ShowIt(*this);
        return kCIDLib::True;
    }

    strHelpRef.CopyOutSubStr(strPageId, c4DivAt);
    strHelpRef.CapAt(c4DivAt);

    strURL.Append(L"topic=");
    strURL.Append(strHelpRef);
    strURL.Append(L"&page=");
    strURL.Append(strPageId);

    // Now let's invoke the browser
    TGUIShell::InvokeDefBrowser(*this, strURL);

    return kCIDLib::True;
}


//
//  This is called when the user invokes the context menu. It can happen two ways:
//
//  1.  The standard F10 key. If we get here with a -1,-1 point, then it was via the
//      keyboard. If there's a single widget selected, we pop it up for that guy.
//  2.  In the bClick, if they let the right click pass through, we'll get here. So it
//      insures that something is selected, and if so let's it go through.
//
//  In the later case it can be multiple things selected
//
tCIDCtrls::ECtxMenuOpts
TIntfEditWnd::eShowContextMenu(const TPoint& pntAt, const tCIDCtrls::TWndId)
{
    //
    //  If the point is -1, -1, then it was via keyboard, so we look for a single
    //  widget selected and get its center point.
    //
    TPoint pntMenu = pntAt;
    TPoint pntRel;
    TCQCIntfWidget* piwdgHit = nullptr;
    if (pntMenu == TPoint(-1, -1))
    {
        // If not a single thing, then we are done
        if (m_selList.bSingleSel())
        {
            piwdgHit = iwdgEdit().piwdgFindByDesId(m_selList.c8At(0));
            if (piwdgHit)
                pntRel = piwdgHit->areaActual().pntCenter();
        }

        // Get the screen based coordinates if we got a hit
        if (piwdgHit)
            ToScreenCoordinates(pntRel, pntMenu);
    }
     else
    {
        // Convert the coordinates to our relative coordinates and see if we hit anything
        FromScreenCoordinates(pntAt, pntRel);
        tCIDLib::TCard8 c8DesId;
        piwdgHit = piwdgHitTest(pntRel, c8DesId);
    }

    // If we got nothing, do nothing
    if (!piwdgHit)
        return tCIDCtrls::ECtxMenuOpts::Done;

    //
    //  If there's something on the clipboard, remember it for below. It can fail if
    //  someone else hasn't released it, so deal with that.
    //
    tCIDLib::TBoolean bCBData = kCIDLib::False;
    TString strCBClass;
    {
        try
        {
            TGUIClipboard gclipTmp(*this);
            bCBData = gclipTmp.bGetAsClass(strCBClass);
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEd().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }

    //
    //  Now let's enable/disable items as appropriate. Note that, the template
    //  can only ever be selected by itself, so if we hit the template, it's
    //  the only thing that is selected.
    //
    if (piwdgHit == &iwdgEdit())
    {
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_CopyAttrs, kCIDLib::True);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Delete, kCIDLib::False);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_States, kCIDLib::True);

        // We cannot copy or cut the template, or do z-order
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Cut, kCIDLib::False);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Copy, kCIDLib::False);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_ToFront, kCIDLib::False);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_ToBack, kCIDLib::False);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_MoveBack, kCIDLib::False);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_MoveForward, kCIDLib::False);

        // We can size the template
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_SizeTo, kCIDLib::True);

        // We can refresh images
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_RefreshImgs, kCIDLib::True);

        // Can't group/ungroup or lock
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Group, kCIDLib::False);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Ungroup, kCIDLib::False);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Lock, kCIDLib::False);

        //
        //  We might be able to paste a widget, so lets see if some object derived from
        //  our widget class is on the clipboard.
        //
        m_menuEdit.SetItemEnable
        (
            kCQCIntfEd::ridMenu_Edit_Paste
            , bCBData && (TCQCIntfEdWdgCB::clsThis() == strCBClass)
        );
    }
     else
    {
        //
        //  Some stuff we'll do differently depending on if there are more than one widget
        //  selected or not, or if there are no partial group content selected.
        //
        const tCIDLib::TBoolean bMultiSel = m_selList.c4Count() > 1;
        const tCIDLib::TBoolean bNoPartials = !bPartialGrpSel();

        if (piwdgHit->bIsLocked())
        {
            //
            //  Disable almost everything. A few are common whether locked or unlocked and
            //  are done below after this if/else block.
            //
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Copy, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Cut, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Delete, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Group, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_ToFront, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_ToBack, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_MoveBack, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_MoveForward, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_PasteAttrs, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_SetSize, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_SizeTo, kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Ungroup, kCIDLib::False);

            // And of course the lock oriented operations
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Lock, kCIDLib::True);

            // Set the lock item's text to actually show Unlock
            m_menuEdit.SetItemText(kCQCIntfEd::ridMenu_Edit_Lock, L"Unlock");
        }
         else
        {
            // Make sure the lock/unlock item is back to Lock text
            m_menuEdit.SetItemText(kCQCIntfEd::ridMenu_Edit_Lock, L"Lock");
            if (bMultiSel)
            {
                //
                //  We can do size to contents multiples at a time. Some may not
                //  support it, but we don't bother to check them all at this point.
                //
                m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_SizeTo, kCIDLib::True);
            }
             else
            {
                // Only enable this one if the one widget supports this operation
                m_menuEdit.SetItemEnable
                (
                    kCQCIntfEd::ridMenu_Edit_SizeTo, piwdgHit->bCanSizeTo()
                );
            }

            // For front/back it has to be a single or a full group
            tCIDLib::TBoolean bZOrderOK = bSingleSelected(kCIDLib::True);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_ToFront, bZOrderOK);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_ToBack, bZOrderOK);

            // For these a single one can be moved within the group
            bZOrderOK = bSingleSelected(kCIDLib::False);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_MoveBack, bZOrderOK);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_MoveForward, bZOrderOK);

            // These things only if there are no partial groups
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Delete, bNoPartials);
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Cut, bNoPartials);

            //
            //  If no partial groups and there is at least one group selected, then
            //  we allow ungroup.
            //
            const tCIDLib::TBoolean bSomeGroups = bAnyGroupedSel();
            m_menuEdit.SetItemEnable
            (
                kCQCIntfEd::ridMenu_Edit_Ungroup, bSomeGroups && bNoPartials
            );

            //
            //  If no partial groups and there's more than one widget selected and
            //  at least one of them is ungrouped, then we enable group.
            //
            m_menuEdit.SetItemEnable
            (
                kCQCIntfEd::ridMenu_Edit_Group
                , bNoPartials && bMultiSel && bAnyUngroupedSel()
            );

            //
            //  We always support lock. We allow widgets within groups to be locked
            //  separately. They just won't be selectable, which means that anything
            //  that isn't legal for partial groups won't be doable.
            //
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Lock, kCIDLib::True);

            // Set size is always ok if the target isn't locked
            m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_SetSize, kCIDLib::True);
        }

        // These things only if there are no partial groups
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Copy, bNoPartials);

        // Only available if a single widget is selected
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_CopyAttrs, !bMultiSel);

        // Only valid on the template itself
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_States, kCIDLib::False);
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_RefreshImgs, kCIDLib::False);

        // All this stuff we'll always enable if non-template is selected
        m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Replace, kCIDLib::True);

        // If an attribute pack is on the clipboard, enable the paste attrs
        m_menuEdit.SetItemEnable
        (
            kCQCIntfEd::ridMenu_Edit_PasteAttrs
            , bCBData && (TCQCIntfAttrPack::clsThis() == strCBClass)
        );
    }

    // This can be done any time and aren't targeting a specific widget
    m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_UnlockAll, kCIDLib::True);
    m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_CopyPoint, kCIDLib::True);
    m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_DeselectAll, kCIDLib::True);
    m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Replace, kCIDLib::True);
    m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_SelectAll, kCIDLib::True);
    m_menuEdit.SetItemEnable(kCQCIntfEd::ridMenu_Edit_Undo, kCIDLib::True);
    m_menuEdit.SetItemEnable
    (
        kCQCIntfEd::ridMenu_Edit_Paste
        , bCBData && (TCQCIntfEdWdgCB::clsThis() == strCBClass)
    );


    //
    //  And now finally we can pop up the menu. We pass in the point that we figured
    //  out above.
    //
    const tCIDLib::TCard4 c4Cmd = m_menuEdit.c4Process
    (
        *this, pntMenu, tCIDLib::EVJustify::Bottom
    );

    // If they made a selection, then process it
    if (c4Cmd)
    {
        // Bracket any undo operations we do here
        m_uhEditor.PushSep();

        //
        //  Some we only handle here. The rest could be done via hot key also so
        //  we do them in MenuCommand().
        //
        if (c4Cmd == kCQCIntfEd::ridMenu_Edit_CopyAttrs)
        {
            //
            //  Just get all of the attributes of the selected widget into a simple wrapper
            //  object, which we'll flatten that and set it on the clipboard.
            //
            TCQCIntfAttrPack iapCB;

            //
            //  Ask the single selected widget for it's attributes. We let it load
            //  straight into the attribute pack object's collection, for efficiency.
            //
            TAttrData attrTmp;
            piwdgHit->QueryWdgAttrs(iapCB.colAttrs(), attrTmp);

            // Remove some we never want copied
            facCIDMData().RemoveAttr(iapCB.colAttrs(), kCQCIntfEng::strAttr_Base_Area);
            facCIDMData().RemoveAttr(iapCB.colAttrs(), kCQCIntfEng::strAttr_Base_Grouped);
            facCIDMData().RemoveAttr(iapCB.colAttrs(), kCQCIntfEng::strAttr_Base_Locked);
            facCIDMData().RemoveAttr(iapCB.colAttrs(), kCQCIntfEng::strAttr_Base_WidgetId);

            // And let's get it onto the clipboard
            TGUIClipboard gclipTmp(*this);
            gclipTmp.Clear();
            gclipTmp.WriteObj<TCQCIntfAttrPack>(iapCB);
        }
         else if (c4Cmd == kCQCIntfEd::ridMenu_Edit_CopyPoint)
        {
            //
            //  Copy the window relative version of the point onto the clipboard,
            //  so that it can be pasted into something later.
            //
            try
            {
                TGUIClipboard gclipTmp(*this);
                gclipTmp.Clear();

                TString strFmt(pntRel);
                gclipTmp.StoreText(strFmt);
            }

            catch(TError& errToCatch)
            {
                if (facCQCIntfEd().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                TExceptDlg dlgErr
                (
                    *this
                    , facCQCIntfEd().strMsg(kIEdMsgs::midWnd_AppName)
                    , errToCatch.strErrText()
                    , errToCatch
                );
            }
        }
         else if (c4Cmd == kCQCIntfEd::ridMenu_Edit_Delete)
        {
            DeleteSelected(kCIDLib::True);
        }
         else if (c4Cmd == kCQCIntfEd::ridMenu_Edit_Paste)
        {
            //
            //  In this case, we have the actual click point available, so we pass that.
            //  We also implement this command over in the menu command handler for when
            //  it's invoked via hot key. In that case, it has to just use the current
            //  position of the mouse.
            //
            PasteWidgets(pntRel);
        }
         else if (c4Cmd == kCQCIntfEd::ridMenu_Edit_PasteAttrs)
        {
            // Get the attribute pack from the clipboard
            TCQCIntfAttrPack* piapNew = nullptr;
            try
            {
                TGUIClipboard gclipPaste(*this);
                if (!gclipPaste.bReadObj<TCQCIntfAttrPack>(piapNew))
                    return tCIDCtrls::ECtxMenuOpts::Done;
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                TExceptDlg dlgErr
                (
                    *this
                    , facCQCIntfEd().strMsg(kIEdMsgs::midWnd_AppName)
                    , L"An error occurred while getting the attributes from the clip board"
                    , errToCatch
                );
                return tCIDCtrls::ECtxMenuOpts::Done;
            }

            // Make sure the attributes get cleaned up
            TJanitor<TCQCIntfAttrPack> janNew(piapNew);

            // Display the dialog box that lets them choose the attributes to apply
            tCIDMData::TAttrList colApply;
            TPasteAttrsDlg dlgPaste;
            if (dlgPaste.bRun(*this, *piapNew, colApply))
                PasteAttrs(colApply);
        }
         else if (c4Cmd == kCQCIntfEd::ridMenu_Edit_Lock)
        {
            //
            //  If the hit widget is locked, then this is really an unlock operation.
            //  Else, go through all of the selected widgets and lock or unlock them. If
            //  it's in a group, unlock the whole group.
            //
            if (piwdgHit->bIsLocked())
            {
                if (piwdgHit->bIsGrouped())
                {
                    tCQCIntfEng::TChildList colList(tCIDLib::EAdoptOpts::NoAdopt);
                    tCIDLib::TCard4 c4FirstZ;
                    if (m_pcivEdit->bFindGroupMembers(piwdgHit->c4GroupId(), colList, c4FirstZ))
                    {
                        RemoveSelection(kCIDLib::True);
                        const tCIDLib::TCard4 c4GrpCnt = colList.c4ElemCount();
                        for (tCIDLib::TCard4 c4GrpInd = 0; c4GrpInd < c4GrpCnt; c4GrpInd++)
                        {
                            colList[c4GrpInd]->bIsLocked(kCIDLib::False);
                            AddToSelection(colList[c4GrpInd], kCIDLib::False, kCIDLib::False);
                        }
                    }
                     else
                    {
                        CIDAssert2(L"No members of group were found")
                    }
                }
                 else
                {
                    // Unlock it and make it the selected widget
                    piwdgHit->bIsLocked(kCIDLib::False);
                    AddToSelection(piwdgHit, kCIDLib::True, kCIDLib::True);
                }
            }
             else
            {
                LockSelected();
            }
        }
         else if (c4Cmd == kCQCIntfEd::ridMenu_Edit_UnlockAll)
        {
            UnlockAll();
        }
         else if (c4Cmd == kCQCIntfEd::ridMenu_Edit_SetSize)
        {
            SetWidgetSizes();
        }
         else if (c4Cmd >= kCQCIntfEd_::c4FirstNewId)
        {
            //
            //  Has to be one of the dynamic New menu items. Pass it the relative
            //  click point, which will be the origin of the new widget.
            //
            CreateNewWidget(c4Cmd, pntRel);
        }
         else if (c4Cmd != kCIDLib::c4MaxCard)
        {
            MenuCommand(c4Cmd, kCIDLib::False, kCIDLib::True);
        }
    }
    return tCIDCtrls::ECtxMenuOpts::Done;
}


tCIDLib::TVoid TIntfEditWnd::GettingFocus()
{
    TParent::GettingFocus();

    // If we have any selected widgets, update the selection handles
    if (!m_selList.bIsEmpty())
        DrawAllSelHandles();
}

tCIDLib::TVoid TIntfEditWnd::LosingFocus()
{
    TParent::LosingFocus();

    // If we have any selected widgets, update the selection handles
    if (!m_selList.bIsEmpty())
        DrawAllSelHandles();
}


//
//  This is called when we get an accelerated hot key, and we also call it from
//  our context menu handler above, so that it's all in one place. The menu
//  disables items that aren't currently valid, but that's not the case for hot
//  keys, so we still have to check here that it's ok to do the requested
//  operation.
//
tCIDLib::TVoid
TIntfEditWnd::MenuCommand(  const   tCIDLib::TResId     ridItem
                            , const tCIDLib::TBoolean   bChecked
                            , const tCIDLib::TBoolean   bEnabled)
{
    // Bracket any undo operations we do here, except for the undo itself
    if (ridItem != kCQCIntfEd::ridMenu_Edit_Undo)
        m_uhEditor.PushSep();

    // Based on the selection, let's do the requested operation
    try
    {
        switch(ridItem)
        {
            case kCQCIntfEd::ridMenu_Edit_Copy :
            case kCQCIntfEd::ridMenu_Edit_Cut :
            case kCQCIntfEd::ridMenu_Edit_Delete :
            {
                //
                //  All of these require that of course there be something
                //  selected, that it not include the template, and there
                //  are no partial groups selected.

                if (!m_selList.bIsEmpty()
                &&  (m_selList.c8At(0) != iwdgEdit().c8DesId())
                &&  !bPartialGrpSel())
                {
                    if ((ridItem == kCQCIntfEd::ridMenu_Edit_Copy)
                    ||  (ridItem == kCQCIntfEd::ridMenu_Edit_Cut))
                    {
                        CopyCutSelected(ridItem == kCQCIntfEd::ridMenu_Edit_Cut);
                    }
                     else
                    {
                        DeleteSelected(kCIDLib::True);
                    }
                }
                break;
            }

            case kCQCIntfEd::ridMenu_Edit_DeselectAll :
            {
                RemoveSelection();
                break;
            }

            case kCQCIntfEd::ridMenu_Edit_ToBack :
            case kCQCIntfEd::ridMenu_Edit_ToFront :
                // Requires a single widget or full group
                if (bSingleSelected())
                    ChangeZOrder(ridItem);
                break;

            case kCQCIntfEd::ridMenu_Edit_MoveBack :
            case kCQCIntfEd::ridMenu_Edit_MoveForward :
                // Requires a single widget. Can move one inside a group in this case
                if (bSingleSelected(kCIDLib::False))
                    ChangeZOrder(ridItem);
                break;

            case kCQCIntfEd::ridMenu_Edit_RefreshImgs :
            {
                RefreshImages();
                break;
            }

            case kCQCIntfEd::ridMenu_Edit_States :
            {
                // The selected widget must be the template
                TCQCIntfWidget* piwdgSel = iwdgEdit().piwdgFindByDesId(m_selList.c8At(0));
                CIDAssert
                (
                    (piwdgSel != nullptr) && piwdgSel->bIsA(TCQCIntfTemplate::clsThis())
                    , L"Selected widget not found or not the template"
                );

                TCQCIntfTemplate* piwdgTmpl = static_cast<TCQCIntfTemplate*>(piwdgSel);
                TCQCIntfStateList istlTmp = piwdgTmpl->istlStates();
                TTmplStatesDlg dlgStates;
                if (dlgStates.bRun(*this, istlTmp))
                {
                    piwdgTmpl->istlStates(istlTmp);

                    //
                    //  In this case, we may have removed states that are referenced by
                    //  widgets. So let's check that and give the user a chance to
                    //  remove all such invalidated states.
                    //
                    tCIDLib::TStrHashSet colBadStates(109, TStringKeyOps());
                    if (!piwdgTmpl->bCheckForInvalidStates(colBadStates))
                    {
                        TString strMsg
                        (
                            L"The following states are referenced but no longer defined. They"
                            L" will be removed."
                        );

                        tCIDLib::TStrHashSet::TCursor cursBad(&colBadStates);
                        if (cursBad.bReset())
                        {
                            strMsg.Append(cursBad.objRCur());
                            while (cursBad.bNext())
                            {
                                strMsg.Append(L", ");
                                strMsg.Append(cursBad.objRCur());
                            }
                        }
                         else
                        {
                            CIDAssert2(L"No bad states was found in the bad state list")
                        }

                        TOkBox msgbBadStates(strMsg);
                        msgbBadStates.ShowIt(*this);
                        piwdgTmpl->DeleteStates(colBadStates);
                    }
                }
                break;
            }

            case kCQCIntfEd::ridMenu_Edit_Group :
            {
                //
                //  We need to gather up all of the widgets that are selected and
                //  get them into a group together. This can be invoked either
                //  from hot keys or the menu. In the hot key case, we have to
                //  check now whether it's valid to do.
                //
                //  Has to be more than one widget selected (which also insures
                //  the template isn't selected.) If so, and there are no partial
                //  groups selected, we can do it.
                //
                if ((m_selList.c4Count() > 1) && !bPartialGrpSel())
                    DoGrouping();
                break;
            }

            case kCQCIntfEd::ridMenu_Edit_Paste :
            {
                //
                //  In this case, it's hot key driven so we have to use the current
                //  mouse position. We also implement this over in the context menu
                //  invocation handler, where the actual mouse position at the click
                //  point is available, which is what we want in that case.
                //
                PasteWidgets(facCIDCtrls().pntPointerPos(*this));
                break;
            };

            case kCQCIntfEd::ridMenu_Edit_Replace :
            {
                DoSearchAndReplace();
                break;
            };

            case kCQCIntfEd::ridMenu_Edit_SelectAll :
            {
                //
                //  Iterate all widgets and add them to the selection list. Tell the all
                //  ids query not to return locked widgets.
                //
                m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));
                tCQCIntfEng::TDesIdList fcolSel;
                iwdgEdit().QueryAllDesIds(fcolSel, kCIDLib::False);
                m_selList.SetList(fcolSel, kCIDLib::False);
                ForceRepaint();
                break;
            }

            case kCQCIntfEd::ridMenu_Edit_SizeTo :
            {
                SizeSelectedToContents();
                break;
            }

            case kCQCIntfEd::ridMenu_Edit_Undo :
            {
                Undo();
                break;
            }

            case kCQCIntfEd::ridMenu_Edit_Ungroup :
            {
                //
                //  This can be done from a hot key, so we have to check after
                //  the fact that it's OK to do it. If there's at least one
                //  group selected, and no partial groups.
                //
                if (bAnyGroupedSel() && !bPartialGrpSel())
                    DoUngrouping();
                break;
            }

            case CQCIntfEd_EditWnd::ridMenu_Faux_FontSize_Dec :
            {
                ApplyFontTool(kCQCIntfEd_::strTool_FontDecrease);
                break;
            }

            case CQCIntfEd_EditWnd::ridMenu_Faux_FontSize_Inc :
            {
                ApplyFontTool(kCQCIntfEd_::strTool_FontIncrease);
                break;
            }

            case CQCIntfEd_EditWnd::ridMenu_Faux_HJustify :
            case CQCIntfEd_EditWnd::ridMenu_Faux_VJustify :
            {
                ToggleTextJustification(ridItem == CQCIntfEd_EditWnd::ridMenu_Faux_HJustify);
                break;
            }

            default:
                CIDAssert2(L"Unknown menu command")
                break;
        };
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEd().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Oh well, didn't work. Display the error to the user
        TExceptDlg dlgbFail
        (
            *this
            , facCQCIntfEd().strMsg(kIEdMsgs::midWnd_AppName)
            , facCQCIntfEd().strMsg(kIEdMsgs::midGen_UnhandledException)
            , errToCatch
        );
    }
}


tCIDLib::TVoid TIntfEditWnd::MouseExit()
{
}


//
//  We handle callbacks from the subscription mixin. These are all sync, so we get
//  directly called back when changes happen.
//
tCIDLib::TVoid TIntfEditWnd::ProcessPubMsg(const TPubSubMsg& psmsgIn)
{
    try
    {
        // Based on the topic, process it appropriately
        if (psmsgIn.strSrcTopicPath().bCompareI(m_strWdgPalTopic))
        {
            const TIntfWidgetPalPub& wppubMsg = psmsgIn.objMsgAs<TIntfWidgetPalPub>();

            //
            //  Push an undo separator. If we never end up actually pushing any undo
            //  operations below, this will just get ignored.
            //
            m_uhEditor.PushSep();

            switch(wppubMsg.m_eEvent)
            {
                case TIntfWidgetPalPub::EEvents::Del :
                    DeleteSelected(kCIDLib::True);
                    break;

                case TIntfWidgetPalPub::EEvents::MoveBack :
                    ChangeZOrder(kCQCIntfEd::ridMenu_Edit_MoveBack);
                    break;

                case TIntfWidgetPalPub::EEvents::MoveForward :
                    ChangeZOrder(kCQCIntfEd::ridMenu_Edit_MoveForward);
                    break;

                case TIntfWidgetPalPub::EEvents::SelChanged :
                    WdgPalSelUpdate(wppubMsg.m_fcolSel);
                    break;

                default :
                    // Dunno what this is
                    break;
            };
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }


    //
    //  Now get any selection list changes. These come from our m_fcolSelected collection
    //  which we enable for publishing. We don't even care what they are. We assume they
    //  represent a change in the selection list so we turn around and issue one of our
    //  own published selection list msgs with the current selection list.
    //
    try
    {
        // Based on the topic, process it appropriately
        if (psmsgIn.strSrcTopicPath().bCompareI(m_strSelectionTopic))
            m_pstopUpdates.Publish(new TIntfEdUpdateTopic(m_selList.fcolIds()));
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }


    //
    //  And get any from the template's widget list. We have to do some work here to
    //  convert these to our own msgs.
    //
    try
    {
        const TCQCIntfTemplate& iwdgTmpl = m_pcivEdit->iwdgBaseTmpl();
        const TString& strWdgListTopic = iwdgTmpl.strPublishTopic();
        if (psmsgIn.strSrcTopicPath().bCompareI(strWdgListTopic))
        {
            const TColPubSubInfo& colpsiCur = psmsgIn.objMsgAs<TColPubSubInfo>();

            TIntfEdUpdateTopic* pieutNew = nullptr;
            switch(colpsiCur.eEvent())
            {
                case TColPubSubInfo::EEvents::BlockAdded :
                    pieutNew = new TIntfEdUpdateTopic();
                    pieutNew->m_eType = TIntfEdUpdateTopic::ETypes::BlockAdded;
                    pieutNew->m_c4Index1 = colpsiCur.c4Index1();
                    pieutNew->m_c4Count = colpsiCur.c4Index2();
                    break;

                case TColPubSubInfo::EEvents::BlockRemoved :
                    pieutNew = new TIntfEdUpdateTopic();
                    pieutNew->m_eType = TIntfEdUpdateTopic::ETypes::BlockRemoved;
                    pieutNew->m_c4Index1 = colpsiCur.c4Index1();
                    pieutNew->m_c4Count = colpsiCur.c4Index2();
                    break;

                case TColPubSubInfo::EEvents::Cleared :
                    pieutNew = new TIntfEdUpdateTopic();
                    pieutNew->m_eType = TIntfEdUpdateTopic::ETypes::Cleared;
                    break;

                case TColPubSubInfo::EEvents::ElemAdded :
                    pieutNew = new TIntfEdUpdateTopic
                    (
                        kCIDLib::True
                        , iwdgTmpl.iwdgAt(colpsiCur.c4Index1())
                        , colpsiCur.c4Index1()
                    );
                    break;

                case TColPubSubInfo::EEvents::ElemChanged :
                    //
                    //  In this case, the index can be max card, which means that it
                    //  was the template itself that was changed.
                    //
                    pieutNew = new TIntfEdUpdateTopic
                    (
                        kCIDLib::False
                        , (colpsiCur.c4Index1() == kCIDLib::c4MaxCard)
                                ? iwdgTmpl : iwdgTmpl.iwdgAt(colpsiCur.c4Index1())
                        , colpsiCur.c4Index1()
                    );
                    break;

                case TColPubSubInfo::EEvents::ElemMoved :
                    pieutNew = new TIntfEdUpdateTopic();
                    pieutNew->m_eType = TIntfEdUpdateTopic::ETypes::Moved;
                    pieutNew->m_c4Index1 = colpsiCur.c4Index1();
                    pieutNew->m_c4Index2 = colpsiCur.c4Index2();
                    break;

                case TColPubSubInfo::EEvents::ElemRemoved :
                    pieutNew = new TIntfEdUpdateTopic();
                    pieutNew->m_eType = TIntfEdUpdateTopic::ETypes::Removed;
                    pieutNew->m_c4Index1 = colpsiCur.c4Index1();
                    pieutNew->m_c8DesId = iwdgTmpl.iwdgAt(colpsiCur.c4Index1()).c8DesId();
                    break;

                case TColPubSubInfo::EEvents::Reordered :
                    pieutNew = new TIntfEdUpdateTopic();
                    pieutNew->m_eType = TIntfEdUpdateTopic::ETypes::Reordered;
                    break;

                case TColPubSubInfo::EEvents::Swapped :
                    pieutNew = new TIntfEdUpdateTopic();
                    pieutNew->m_eType = TIntfEdUpdateTopic::ETypes::Swapped;
                    pieutNew->m_c4Index1 = colpsiCur.c4Index1();
                    pieutNew->m_c4Index2 = colpsiCur.c4Index2();
                    break;

                default :
                    #if CID_DEBUG_ON
                    TErrBox msgbErr(L"Unknown collection change msg published");
                    msgbErr.ShowIt(*this);
                    #endif

                    // Panic and reload to make sure we don't get out of sync
                    pieutNew = new TIntfEdUpdateTopic();
                    pieutNew->m_eType = TIntfEdUpdateTopic::ETypes::Reordered;
                    break;
            };

            // If we created a new msg, publish it
            if (pieutNew)
                m_pstopUpdates.Publish(pieutNew);
        }


        // To be safe publish a mouse over msg in case this affected what is under the mouse
        PublishMouseOverMsg();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }


    //
    //  If we get any from the error list, we need to select the widget in question.
    //
    try
    {
        // Based on the topic, process it appropriately
        if (psmsgIn.strSrcTopicPath().bCompareI(m_pwndErrList->strPublishTopic()))
        {
            const TIntfErrListTopic& ieltMsg = psmsgIn.objMsgAs<TIntfErrListTopic>();

            // See if the indicated widget exists. If so, select it if it isn't locked.
            TCQCIntfWidget* piwdgSel = nullptr;
            if (ieltMsg.m_c4WidgetId == kCIDLib::c4MaxCard)
                piwdgSel = &iwdgEdit();
            else
                piwdgSel = iwdgEdit().piwdgFindByUID(ieltMsg.m_c4WidgetId, kCIDLib::False);

            if (piwdgSel && !piwdgSel->bIsLocked())
            {
                // Push a selection undo first, with separator
                m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()), kCIDLib::True);
                AddToSelection(piwdgSel, kCIDLib::True, kCIDLib::False);
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TIntfEditWnd: Private, non-virtaul methods
// ---------------------------------------------------------------------------

// Calculates the area that bounds all of the selected items.
TArea TIntfEditWnd::areaSelection()
{
    TArea areaSel;
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count)
        return areaSel;

    areaSel = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(0))->areaActual();
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        areaSel |= m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index))->areaActual();
    return areaSel;
}


//
//  This method is called to add a widget to the selection list. They can ask
//  that we clear the list first, or just add to what is there.
//
//  We have another version that will add all the members of a group to the
//  selection. Note that only unlocked group members will be added.
//
tCIDLib::TVoid
TIntfEditWnd::AddGrpToSelection(const   tCIDLib::TCard4     c4GroupId
                                , const tCIDLib::TBoolean   bClearFirst)
{
    // Find all of the other group widgets and add them to the selection.
    tCQCIntfEng::TChildList colList(tCIDLib::EAdoptOpts::NoAdopt);
    tCIDLib::TCard4 c4FirstZ;
    if (!m_pcivEdit->bFindGroupMembers(c4GroupId, colList, c4FirstZ))
    {
        CIDAssert2(L"No members of group were found")
        return;
    }

    // Push a selection undo
    m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));

    //
    //  If we clear, tell him not to update the attribute editor, since we
    //  going to do it at the end. This will help avoid blinkies. This will also
    //  tell him not to publish a selection change msg, we'll do it below after the
    //  change.
    //
    if (bClearFirst)
        RemoveSelection(kCIDLib::False);

    // Enable block mode for publishing, so we don't send a bunch of individual changes
    TColBlockModeJan janBlock(&m_selList.fcolIds(), kCIDLib::True);

    TArea areaUpdate;
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // For each one in the group, if not selected and not locked, then add it
        TCQCIntfWidget* piwdgCur = colList[c4Index];
        if (!piwdgCur->bIsLocked()
        &&  !m_selList.bIsSelected(piwdgCur->c8DesId()))
        {
            m_selList.c4AddNew(piwdgCur->c8DesId(), kCIDLib::False);
            areaUpdate |= piwdgCur->areaActual();
        }
    }

    // Redraw the area affected, if any
    if (!areaUpdate.bIsEmpty())
        m_pcivEdit->Redraw(areaUpdate);

    // Update the attribute editor since we affected the selection
    UpdateAttrEd();
}


tCIDLib::TVoid
TIntfEditWnd::AddToSelection(       TCQCIntfWidget* const   piwdgTar
                            , const tCIDLib::TBoolean       bClearFirst
                            , const tCIDLib::TBoolean       bManualSel)
{
    // Push a selection undo
    m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));

    //
    //  If we clear, tell him not to update the attribute editor, since we
    //  going to do it at the end. This will help avoid blinkies.
    //
    if (bClearFirst)
        RemoveSelection(kCIDLib::False);

    // If this one is locked, then we do nothing
    if (!piwdgTar->bIsLocked())
    {
        if (!m_selList.bIsSelected(piwdgTar->c8DesId()))
        {
            m_selList.c4AddNew(piwdgTar->c8DesId(), bManualSel);

            // Redraw this guy's area to show the changes
            m_pcivEdit->Redraw(piwdgTar->areaActual());
        }
    }

    // Update the attribute editor since we affected the selection
    UpdateAttrEd();
}


// This is called when the user drags
tCIDLib::TVoid
TIntfEditWnd::AdjustWidgetAreas(const TArea& areaOld, const TArea& areaNew)
{
    // Push a multi-position undo
    m_uhEditor.Push(new TIntfMultiPosUndo(iwdgEdit(), m_selList.fcolIds()));

    //
    //  One or more selected items were moved. We get the old and new areas,
    //  each of which is the area that contains all of the widgets that were
    //  moved. So we just adjust each of them by the difference between the
    //  old and new origins.
    //
    TPoint pntAdjust(areaNew.pntOrg());
    pntAdjust -= areaOld.pntOrg();

    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    TArea areaCur;
    TArea areaUpdate;
    TCQCIntfWidget* piwdgCur = nullptr;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));

        // Get the current area and add it to the update area
        areaCur = piwdgCur->areaActual();
        TSize szOld(areaCur.szArea());
        areaUpdate |= areaCur;

        // Update the current area origin and do auto-snap if it's enabled
        areaCur.AdjustOrg(pntAdjust);
        bSnapArea(areaCur, iwdgEdit().areaActual(), kCIDLib::False);

        // Set the new area on the widget and update it
        piwdgCur->SetArea(areaCur);
        piwdgCur->SizeChanged(areaCur.szArea(), szOld);
    }

    // And redraw the affected parts
    m_pcivEdit->Redraw(areaUpdate);
}


tCIDLib::TVoid TIntfEditWnd::ApplyAlignTool(const TString& strToDo)
{
    //
    //  We have a special issue here wrt to groups. For fully selected groups, we need
    //  to treat them as a whole, with singles or individually selected members of a
    //  group being treated as singles.
    //
    //  So we use a small wrapper class and actually use that to do all of the alignment
    //  and sorting and such. All members of fully selected groups are put into a single
    //  wrapper object.
    //
    //  We have a helper to build us up a list of these. If he doesn't end up keeping
    //  anything, then just return.
    //
    TAlignList colList(tCIDLib::EAdoptOpts::Adopt);
    if (!bBuildAlignList(colList))
        return;

    //
    //  Push a multi-pos undo for the selected widgets. We may toss it in the end if
    //  we never end up moving anything. Push a separator
    //
    m_uhEditor.Push(new TIntfMultiPosUndo(iwdgEdit(), m_selList.fcolIds()), kCIDLib::True);

    // Remember if we actually moved any
    tCIDLib::TBoolean bMoved = kCIDLib::False;

    //
    //  If this was not a manual selection, or it's one of the spacing type commands
    //  then we want to sort them along the axis of alignment, in reverse order
    //  if ctrl shifted and not one of the spacing options.
    //
    if (!m_selList.bAlignToFirst()
    ||  (strToDo == kCQCIntfEd_::strTool_HorzSpace)
    ||  (strToDo == kCQCIntfEd_::strTool_VertSpace))
    {
        //
        //  If control shifted and it's not one of the spacing types, we go from the
        //  right/bottom so tell the sorter to reverse sort.
        //
        const tCIDLib::TBoolean bReverse
        (
            facCIDCtrls().bCtrlShifted()
            && (strToDo != kCQCIntfEd_::strTool_HorzSpace)
            && (strToDo != kCQCIntfEd_::strTool_VertSpace)
        );

        // Sort the list along the alignment access (reverse if right/bottom type)
        colList.Sort(TAlignListSorter(strToDo, colList, bReverse));
    }

    TArea           areaCur;
    TPoint          pntOrg;
    const TArea     areaBase;
    tCIDLib::TInt4  i4Align;

    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    const TArea& areaTmpl(iwdgEdit().areaActual());
    if (strToDo == kCQCIntfEd_::strTool_BottomAlign)
    {
        i4Align = colList[0]->m_areaFull.i4Bottom();
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            areaCur = awCur.m_areaFull;
            pntOrg = areaCur.pntOrg();
            areaCur.BottomAlign(i4Align);
            areaCur.ForceWithin(areaTmpl);
            if (pntOrg != areaCur.pntOrg())
            {
                awCur.SetOrg(areaCur.pntOrg(), *this);
                bMoved = kCIDLib::True;
            }
        }
    }
     else if (strToDo == kCQCIntfEd_::strTool_HorzCenter)
    {
        // Figure out the horz center point of the first one
        i4Align = colList[0]->m_areaFull.pntCenter().i4Y();

        // And center align the rest of them on this point
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            areaCur = awCur.m_areaFull;
            pntOrg = areaCur.pntOrg();
            areaCur.i4Y(i4Align - tCIDLib::TInt4(areaCur.c4Height() / 2));
            areaCur.ForceWithin(areaTmpl);
            if (pntOrg != areaCur.pntOrg())
            {
                awCur.SetOrg(areaCur.pntOrg(), *this);
                bMoved = kCIDLib::True;
            }
        }
    }
     else if (strToDo == kCQCIntfEd_::strTool_HorzSpace)
    {
        //
        //  Find the left-most and right-most of the selected widgets, and get the horz
        //  centers of both of them. These provide the two brackets for the distribution.
        //
        tCIDLib::TInt4 i4Min = kCIDLib::i4MaxInt;
        tCIDLib::TInt4 i4Max = kCIDLib::i4MinInt;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            const TArea& areaCur = awCur.m_areaFull;
            i4Align = areaCur.pntCenter().i4X();
            if (i4Align < i4Min)
                i4Min = i4Align;

            if (i4Align > i4Max)
                i4Max = i4Align;
        }

        //
        //  Now we can calculate the distance between each center point,
        //  which is the difference between the max and min, divided by
        //  the count of widgets.
        //
        const tCIDLib::TInt4 i4Ofs = (i4Max - i4Min) / tCIDLib::TInt4(c4Count - 1);

        //
        //  Now we can start at the min value and center each widget on
        //  that point, then move forward by the offset.
        //
        i4Min += i4Ofs;
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            areaCur = awCur.m_areaFull;
            pntOrg = areaCur.pntOrg();
            areaCur.i4X(i4Min - tCIDLib::TInt4(areaCur.c4Width() / 2));
            areaCur.ForceWithin(areaTmpl);
            if (pntOrg != areaCur.pntOrg())
            {
                awCur.SetOrg(areaCur.pntOrg(), *this);
                bMoved = kCIDLib::True;
            }
            i4Min += i4Ofs;
        }
    }
     else if (strToDo == kCQCIntfEd_::strTool_LeftAlign)
    {
        // Use left of first one as the align point
        i4Align = colList[0]->m_areaFull.i4Left();
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            areaCur = awCur.m_areaFull;
            pntOrg = areaCur.pntOrg();
            areaCur.i4Left(i4Align);
            areaCur.ForceWithin(areaTmpl);
            if (pntOrg != areaCur.pntOrg())
            {
                awCur.SetOrg(areaCur.pntOrg(), *this);
                bMoved = kCIDLib::True;
            }
        }
    }
     else if (strToDo == kCQCIntfEd_::strTool_RightAlign)
    {
        // Use right of first one as the align point
        i4Align = colList[0]->m_areaFull.i4Right();
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            areaCur = awCur.m_areaFull;
            pntOrg = areaCur.pntOrg();
            areaCur.RightAlign(i4Align);
            areaCur.ForceWithin(areaTmpl);
            if (pntOrg != areaCur.pntOrg())
            {
                awCur.SetOrg(areaCur.pntOrg(), *this);
                bMoved = kCIDLib::True;
            }
        }
    }
     else if (strToDo == kCQCIntfEd_::strTool_TopAlign)
    {
        // Use top of first one as the align point
        i4Align = colList[0]->m_areaFull.i4Top();
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            areaCur = awCur.m_areaFull;
            pntOrg = areaCur.pntOrg();
            areaCur.i4Top(i4Align);
            areaCur.ForceWithin(areaTmpl);
            if (pntOrg != areaCur.pntOrg())
            {
                awCur.SetOrg(areaCur.pntOrg(), *this);
                bMoved = kCIDLib::True;
            }
        }
    }
     else if (strToDo == kCQCIntfEd_::strTool_VertCenter)
    {
        // Figure out the horizontal center point of the first widget
        i4Align = colList[0]->m_areaFull.pntCenter().i4X();

        // And center align the rest of them on this point
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            areaCur = awCur.m_areaFull;
            pntOrg = areaCur.pntOrg();
            areaCur.i4X(i4Align - tCIDLib::TInt4(areaCur.c4Width() / 2));
            areaCur.ForceWithin(areaTmpl);
            if (pntOrg != areaCur.pntOrg())
            {
                awCur.SetOrg(areaCur.pntOrg(), *this);
                bMoved = kCIDLib::True;
            }
        }
    }
     else if (strToDo == kCQCIntfEd_::strTool_VertSpace)
    {
        //
        //  Find the topmost and bottom-most of the selected widgets, and get the
        //  vertical centers of both of them. These provide the two brackets for
        //  the distribution.
        //
        tCIDLib::TInt4 i4Min = kCIDLib::i4MaxInt;
        tCIDLib::TInt4 i4Max = kCIDLib::i4MinInt;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            const TArea& areaCur = awCur.m_areaFull;
            i4Align = areaCur.pntCenter().i4Y();

            if (i4Align < i4Min)
                i4Min = i4Align;

            if (i4Align > i4Max)
                i4Max = i4Align;
        }

        //
        //  Now we can calculate the distance between each center point,
        //  which is the difference between the max and min, divided by
        //  the count of widgets.
        //
        const tCIDLib::TInt4 i4Ofs = (i4Max - i4Min) / tCIDLib::TInt4(c4Count - 1);

        //
        //  Now we can start at the min value and center each widget on
        //  that point, then move forward by the offset.
        //
        i4Min += i4Ofs;
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            TAlignWrapper& awCur = *colList[c4Index];
            areaCur = awCur.m_areaFull;
            pntOrg = areaCur.pntOrg();
            areaCur.i4Y(i4Min - tCIDLib::TInt4(areaCur.c4Height() / 2));
            areaCur.ForceWithin(areaTmpl);
            if (pntOrg != areaCur.pntOrg())
            {
                awCur.SetOrg(areaCur.pntOrg(), *this);
                bMoved = kCIDLib::True;
            }
            i4Min += i4Ofs;
        }
    }

    // If we moved any, then deal with that, else pop back to the last undo separator
    if (bMoved)
        ForceRepaint();
    else
        m_uhEditor.DiscardToSep();
}



//
//  We got a notification from our tools tab and it was one of the font tools. So we
//  look at the op requested and apply it.
//
tCIDLib::TVoid TIntfEditWnd::ApplyFontTool(const TString& strToDo)
{
    tCIDLib::TCard4 c4Count = m_selList.c4Count();

    // If none selected, then nothing to do
    if (!c4Count)
        return;

    //
    //  Go through though them and see if any implement the font mixin. If not, then
    //  there's nothing to do. To be efficient, remember them as we go and just go
    //  through those at the end, via the font mixin. Our list must be non-adopting
    //  of course!
    //
    TRefVector<TCQCIntfWidget> colKeepers(tCIDLib::EAdoptOpts::NoAdopt);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));
        MCQCIntfFontIntf* pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(piwdgCur);
        if (pmiwdgFont)
            colKeepers.Add(piwdgCur);
    }

    // If we didn't find any, then we are done
    c4Count = colKeepers.c4ElemCount();
    if (c4Count)
    {
        // Push a full undo for the selected widgets. Push a separator
        m_uhEditor.Push(new TIntfFullUndo(iwdgEdit()), kCIDLib::True);

        //
        //  If it's font face, we have to get the face. The others are implicit in the
        //  operation.
        //
        MCQCIntfFontIntf* pmiwdgFont;
        TString strFace;
        if (strToDo == kCQCIntfEd_::strTool_FontBold)
        {
            // For each one we will just flip the bold attribute
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(colKeepers[c4Index]);
                const tCIDLib::TBoolean bBold = pmiwdgFont->bBold();
                pmiwdgFont->bBold(!bBold);
            }
        }
         else if (strToDo == kCQCIntfEd_::strTool_FontDecrease)
        {
            // For each one, if the font size is greater than 5, we'll reduce it by one
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(colKeepers[c4Index]);
                tCIDLib::TCard4 c4Size = pmiwdgFont->c4FontHeight();
                if (c4Size > 5)
                    pmiwdgFont->c4FontHeight(c4Size - 1);
            }
        }
         else if (strToDo == kCQCIntfEd_::strTool_FontFace)
        {
            //
            //  Get a font face to apply. Use the first one in the list as the default
            //  settings.
            //
            pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(colKeepers[0]);
            TString strFace = pmiwdgFont->strFaceName();
            const tCIDLib::TBoolean bRes = facCIDWUtils().bSelectFontFace
            (
                *this
                , strFace
                , pmiwdgFont->c4FontHeight()
                , pmiwdgFont->bBold()
                , pmiwdgFont->bItalic()
            );

            if (bRes)
            {
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(colKeepers[c4Index]);
                    pmiwdgFont->strFaceName(strFace);
                }
            }
        }
         else if (strToDo == kCQCIntfEd_::strTool_FontIncrease)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(colKeepers[c4Index]);
                tCIDLib::TCard4 c4Size = pmiwdgFont->c4FontHeight();
                if (c4Size < 128)
                    pmiwdgFont->c4FontHeight(c4Size + 1);
            }
        }
         else if (strToDo == kCQCIntfEd_::strTool_FontItalic)
        {
            // For each one we will just flip the italic attribute
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(colKeepers[c4Index]);
                const tCIDLib::TBoolean bItalic = pmiwdgFont->bItalic();
                pmiwdgFont->bItalic(!bItalic);
            }
        }

        // Build up a region that includes the affected widgets and invalidate it
        TGUIRegion grgnRedraw(colKeepers[0]->areaActual());
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        {
            grgnRedraw.CombineWith
            (
                colKeepers[c4Index]->areaActual(), tCIDGraphDev::EClipModes::Or
            );
        }
        m_pcivEdit->Redraw(grgnRedraw);
    }
}


//
//  Returns true if any group members are in the selection list.
//
tCIDLib::TBoolean TIntfEditWnd::bAnyGroupedSel() const
{
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index))->c4GroupId())
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Returns true if any ungrouped members are in the selection list.
//
tCIDLib::TBoolean TIntfEditWnd::bAnyUngroupedSel() const
{
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index))->c4GroupId())
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  A helper to build up the list of align wrapper objects used during alignment
//  operations. We go through the list of selected widgets. If we hit singles or
//  non-complete groups, we store those as separate align wrapper objects. For fully
//  selected groups we store them as a single align wrapper object.
//
tCIDLib::TBoolean TIntfEditWnd::bBuildAlignList(TAlignList& colToFill)
{
    // If we have no widgets selected or a single one, then obviously not
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (c4Count < 2)
        return kCIDLib::False;

    //
    //  If we have just a single group, then we treat them singles so that they can
    //  align the group members. We know we have more than just one widget now, so if
    //  this returns true, we have a single, full group selected.
    //
    if (bSingleSelected())
    {
        for (tCIDLib::TCard4 c4GrpInd = 0; c4GrpInd < c4Count; c4GrpInd++)
        {
            TAlignWrapper* pawCur = new TAlignWrapper();
            colToFill.Add(pawCur);
            pawCur->m_colList.Add(m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4GrpInd)));
            pawCur->Complete();
        }
        return kCIDLib::True;
    }

    //
    //  Get a copy of the selection list, so that we can remove those we have taken
    //  care of, so as not to redundantly process them. Group members may not be in
    //  the list in group order necessarily, for instance, so we want to remove them
    //  all once that group is processed.
    //
    tCQCIntfEng::TDesIdList fcolSel(m_selList.fcolIds());

    //
    //  Ok, let's start. We run through the widgets and break out singles, partials,
    //  and full groups.
    //
    colToFill.RemoveAll();
    tCIDLib::TCard4 c4Index = 0;
    tCQCIntfEng::TDesIdList fcolGrpIds;
    while (!fcolSel.bIsEmpty())
    {
        TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(fcolSel[0]);

        // See if this guy is in a group
        const tCIDLib::TCard4 c4CurGrpId = piwdgCur->c4GroupId();
        if (c4CurGrpId)
        {
            //
            //  See if this guy's entire group is selected. If so, then we treat it as
            //  a group, else just as singles. This helper will give us back all of the
            //  ids of the grouped widgets if it's fully selected.
            //
            if (bGrpFullySelected(c4CurGrpId, fcolGrpIds))
            {
                // Add all of this group to a single wrapper
                TAlignWrapper* pawCur = new TAlignWrapper();
                colToFill.Add(pawCur);
                const tCIDLib::TCard4 c4GrpCnt = fcolGrpIds.c4ElemCount();
                for (tCIDLib::TCard4 c4GrpInd = 0; c4GrpInd < c4GrpCnt; c4GrpInd++)
                {
                    piwdgCur = m_pcivEdit->piwdgFindByDesId(fcolGrpIds[c4GrpInd]);
                    pawCur->m_colList.Add(piwdgCur);
                    fcolSel.bDeleteIfPresent(fcolGrpIds[c4GrpInd]);
                }
                pawCur->Complete();
            }
             else
            {
                //
                //  Find the selected ones from this group and add them as singles and
                //  remove them from the selection list.
                //
                tCIDLib::TCard4 c4At;
                const tCIDLib::TCard4 c4GrpCnt = fcolGrpIds.c4ElemCount();
                for (tCIDLib::TCard4 c4GrpInd = 0; c4GrpInd < c4GrpCnt; c4GrpInd++)
                {
                    if (!fcolSel.bElementPresent(fcolGrpIds[c4GrpInd], c4At))
                        continue;

                    piwdgCur = m_pcivEdit->piwdgFindByDesId(fcolGrpIds[c4GrpInd]);
                    TAlignWrapper* pawCur = new TAlignWrapper();
                    colToFill.Add(pawCur);
                    pawCur->m_colList.Add(piwdgCur);
                    pawCur->Complete();
                    fcolSel.Delete(c4At);
                }
            }
        }
         else
        {
            // Just a single, so keep it and remove from the selection list
            TAlignWrapper* pawCur = new TAlignWrapper();
            colToFill.Add(pawCur);
            pawCur->m_colList.Add(piwdgCur);
            pawCur->Complete();
            fcolSel.Delete(0);
        }
    }

    // If we enede dup with anything return true
    return !colToFill.bIsEmpty();
}


//
//  This is a helper that will build up a region that represents all of the
//  selection handles of the selected widgets. This is used to then redraw
//  just those small areas, to get rid of the selection without having to
//  redraw the whole widgets and cause a bunch of blinkiness.
//
tCIDLib::TBoolean TIntfEditWnd::bBuildSelHandleRgn(TGUIRegion& grgnToFill)
{
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count)
        return kCIDLib::False;

    TArea areaHandle(0, 0, 5, 5);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));

        //
        //  Get the area of the widget and calc the area of each of the
        //  selection handles for it and add them.
        //
        const TArea& areaCur = piwdgCur->areaActual();

        // Do the left side one
        areaHandle.LeftJustifyIn(areaCur, kCIDLib::True);
        if (grgnToFill.bIsEmpty())
            grgnToFill.Set(areaHandle);
        else
            grgnToFill.CombineWith(areaHandle, tCIDGraphDev::EClipModes::Or);

        // Do the right side one
        areaHandle.RightJustifyIn(areaCur, kCIDLib::True);
        grgnToFill.CombineWith(areaHandle, tCIDGraphDev::EClipModes::Or);

        // Do the top one
        areaHandle.TopJustifyIn(areaCur, kCIDLib::True);
        grgnToFill.CombineWith(areaHandle, tCIDGraphDev::EClipModes::Or);

        // Do the bottom one
        areaHandle.BottomJustifyIn(areaCur, kCIDLib::True);
        grgnToFill.CombineWith(areaHandle, tCIDGraphDev::EClipModes::Or);

        // Do the upper left corner
        areaHandle.JustifyIn(areaCur, tCIDLib::EHJustify::Left, tCIDLib::EVJustify::Top);
        grgnToFill.CombineWith(areaHandle, tCIDGraphDev::EClipModes::Or);

        // Do the upper right corner
        areaHandle.JustifyIn(areaCur, tCIDLib::EHJustify::Right, tCIDLib::EVJustify::Top);
        grgnToFill.CombineWith(areaHandle, tCIDGraphDev::EClipModes::Or);

        // Do the bottom left
        areaHandle.JustifyIn(areaCur, tCIDLib::EHJustify::Left, tCIDLib::EVJustify::Bottom);
        grgnToFill.CombineWith(areaHandle, tCIDGraphDev::EClipModes::Or);

        // Do the bottom right
        areaHandle.JustifyIn(areaCur, tCIDLib::EHJustify::Right, tCIDLib::EVJustify::Bottom);
        grgnToFill.CombineWith(areaHandle, tCIDGraphDev::EClipModes::Or);
    }
    return kCIDLib::True;
}


//
//  Returns true if all of the wigets in the indicated group are in the selection
//  list. Either way we return all of the ids of the grouped widgets.
//
tCIDLib::TBoolean
TIntfEditWnd::bGrpFullySelected(const   tCIDLib::TCard4             c4GrpId
                                ,       tCQCIntfEng::TDesIdList&    fcolGrpIds) const
{
    // Find the first widget in this group
    const TCQCIntfTemplate& iwdgTmpl = iwdgEdit();
    tCIDLib::TCard4 c4ZOrder = iwdgTmpl.c4FindFirstGroupZ(c4GrpId);
    CIDAssert(c4ZOrder != kCIDLib::c4MaxCard, L"The passed group id was not found");

    //
    //  Go through the widgets until we hit something not in this group or the end of
    //  the list. If any of them aren't found, return false.
    //
    tCIDLib::TBoolean bRet = kCIDLib::True;
    fcolGrpIds.RemoveAll();
    const tCIDLib::TCard4 c4Count = iwdgTmpl.c4ChildCount();
    for (; c4ZOrder < c4Count; c4ZOrder++)
    {
        const TCQCIntfWidget& iwdgCur = iwdgTmpl.iwdgAt(c4ZOrder);

        // If a different group id, then we are done
        if (iwdgCur.c4GroupId() != c4GrpId)
            break;

        // If this guy isn't in the selection list, then we are going to fail
        if (!m_selList.bIsSelected(iwdgCur.c8DesId()))
            bRet = kCIDLib::False;

        // Add this guy's design time id to the caller's list
        fcolGrpIds.c4AddElement(iwdgCur.c8DesId());
    }

    // Return whether all fo them were selected
    return bRet;
}


//
//  Indicate if the passed widget it selected or not. If so, return the index
//  within the selection list.
//
tCIDLib::TBoolean
TIntfEditWnd::bIsSelected(  const   tCIDLib::TCard8     c8ToCheck
                            ,       tCIDLib::TCard4&    c4Index)
{
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count)
    {
        c4Index = kCIDLib::c4MaxCard;
        return kCIDLib::False;
    }

    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_selList.c8At(c4Index) == c8ToCheck)
            return kCIDLib::True;
    }

    c4Index = kCIDLib::c4MaxCard;
    return kCIDLib::False;
}


tCIDLib::TBoolean TIntfEditWnd::bIsSelected(const tCIDLib::TCard8 c8ToCheck)
{
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count)
        return kCIDLib::False;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_selList.c8At(c4Index) == c8ToCheck)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  There are various things that we don't allow if there are any groups that
//  are partially selected. We have to operate on a full group or not at all.
//  So this checks all of the selected widgets. If any are in a group, it insures
//  that all of that group's members are selected as well. It can include multiple
//  groups and singles.
//
//  This is as compared wtih bSingleSelected, which insures that the selection is
//  either a single widget or a single full group.
//
tCIDLib::TBoolean TIntfEditWnd::bPartialGrpSel() const
{
    //
    //  We keep up with the groups we've already checked, so as not to keep
    //  checking the same group over and over.
    //
    tCQCIntfEng::TDesIdList fcolGrpsDone;
    tCQCIntfEng::TDesIdList fcolGroupIds;

    const TCQCIntfTemplate& iwdgTmpl = iwdgEdit();
    const tCIDLib::TCard4 c4SelCnt = m_selList.c4Count();
    for (tCIDLib::TCard4 c4SelIndex = 0; c4SelIndex < c4SelCnt; c4SelIndex++)
    {
        const TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4SelIndex));

        // If not grouped, skip this one
        const tCIDLib::TCard4 c4GroupId = piwdgCur->c4GroupId();
        if (!c4GroupId)
            continue;

        // If we've already done this group, skip this one
        if (fcolGrpsDone.bElementPresent(c4GroupId))
            continue;

        //
        //  We need to check it. Add it to the list of groups checked, then
        //  let's check it out.
        //
        fcolGrpsDone.c4AddElement(c4GroupId);

        //
        //  Query all of the widgets in this group. Should not return false, but
        //  just in case, keep moving forward.
        //
        tCIDLib::TCard4 c4FirstZ;
        if (!iwdgTmpl.bFindGroupMembers(c4GroupId, fcolGroupIds, c4FirstZ))
            continue;

        // Make sure that all of these ids are in the selection list
        const tCIDLib::TCard4 c4GrpCnt = fcolGroupIds.c4ElemCount();
        for (tCIDLib::TCard4 c4GrpIndex = 0; c4GrpIndex < c4GrpCnt; c4GrpIndex++)
        {
            // If this one isn't then we have a partial group
            if (!m_selList.bIsSelected(fcolGroupIds[c4GrpIndex]))
                return kCIDLib::True;
        }
    }

    // Looks good, no partial groups
    return kCIDLib::False;
}


//
//  This method will look at the selected widgets and return true if a single
//  widget is selected, or all of the members of a single group is are selected.
//  There are some things we only allow in those circumstances.
//
//  They can override the full group requirement and return true if it's just a
//  single widget selected, regardless. Sometimes that's all that's important.
//
tCIDLib::TBoolean
TIntfEditWnd::bSingleSelected(const tCIDLib::TBoolean bFullGrpReq) const
{
    tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count)
        return kCIDLib::False;

    //
    //  Get the first one and see if it is grouped or not. If so, all the others
    //  have to have the same group id, and then we have to insure that there are
    //  no others before or after this set that have the same id (which would mean
    //  not all of the group members were selected.
    //
    //  If it's not, then it has to be a single widget.
    //
    tCIDLib::TCard4 c4ZOrder;
    const TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId
    (
        m_selList.c8At(0), c4ZOrder
    );

    const tCIDLib::TCard4 c4GrpId = piwdgCur->c4GroupId();
    if (c4Count == 1)
    {
        // If it has to be a full group, test that
        if (bFullGrpReq)
            return (c4GrpId == 0);

        // Else it is a single
        return kCIDLib::True;
    }

    // Has to be a group, else we know we've failed.
    if (!c4GrpId)
        return kCIDLib::False;

    // Get the index of first one of this group
    const TCQCIntfTemplate& iwdgTmpl = iwdgEdit();
    c4ZOrder = iwdgTmpl.c4FindFirstGroupZ(c4GrpId);

    //
    //  Run through the widgets from here forward that are part of this group.
    //  Check that each one is in the selection list, and keep a count.
    //
    c4Count = iwdgTmpl.c4ChildCount();
    tCIDLib::TCard4 c4GrpCnt = 0;

    for (tCIDLib::TCard4 c4Index = c4ZOrder; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget& iwdgCur = iwdgTmpl.iwdgAt(c4Index);

        // If not a member of this group, then we are done
        if (iwdgCur.c4GroupId() != c4GrpId)
            break;

        // If not in the selection list we are done
        if (!m_selList.bIsSelected(iwdgCur.c8DesId()))
            break;

        c4GrpCnt++;
    }

    // If we accounted for all the selected widgets, then we are good
    return (c4GrpCnt == m_selList.c4Count());
}



//
//  If auto-snap is turned on, these will adjust the passed area or point to
//  the nearest grid node.
//
tCIDLib::TBoolean
TIntfEditWnd::bSnapArea(        TArea&              areaToAdjust
                        , const TArea&              areaLimit
                        , const tCIDLib::TBoolean   bForce)
{
    // If and not forced, then just return false now
    if (!bForce)
    {
        //
        //  We still make sure it's within the template area, so that the
        //  caller doesn't have to special case this when the snap doesn't
        //  change the origin.
        //
        areaToAdjust.ForceWithin(iwdgEdit().areaActual());
        return kCIDLib::False;
    }

    // Call the other version to adjust the origin. If not changed, return false
    TPoint pntOrg = areaToAdjust.pntOrg();
    if (!bSnapPoint(pntOrg, kCIDLib::True))
    {
        //
        //  We still make sure it's within the template area, so that the
        //  caller doesn't have to special case this when the snap doesn't
        //  change the origin.
        //
        areaToAdjust.ForceWithin(iwdgEdit().areaActual());
        return kCIDLib::False;
    }

    // Put the new origin in, and then make sure it stays within the limits
    areaToAdjust.SetOrg(pntOrg);
    areaToAdjust.ForceWithin(iwdgEdit().areaActual());

    return kCIDLib::True;
}

tCIDLib::TBoolean
TIntfEditWnd::bSnapPoint(       TPoint&             pntToAdjust
                        , const tCIDLib::TBoolean   bForce)
{
    // If not forced, then just return false now
    if (!bForce)
        return kCIDLib::False;

#if 0
    // Set up the values we'll need to do this calculation
    const tCIDLib::TInt4 i4XDiv = tCIDLib::TInt4(m_cfgoCur.szSnapGrid().c4Width());
    const tCIDLib::TInt4 i4YDiv = tCIDLib::TInt4(m_cfgoCur.szSnapGrid().c4Height());

    const tCIDLib::TInt4 i4XDiv2 = i4XDiv / 2;
    const tCIDLib::TInt4 i4YDiv2 = i4YDiv / 2;

    // Save the original, so that we can remember if we changed it
    TPoint pntSave = pntToAdjust;

    //
    //  We want to take them to the next closest point, so add half
    //  the grid size to each and then divide by the grid size, then
    //  multiply back out by the grid size.
    //
    tCIDLib::TInt4 i4X = ((pntToAdjust.i4X() + i4XDiv2) / i4XDiv) * i4XDiv;
    tCIDLib::TInt4 i4Y = ((pntToAdjust.i4Y() + i4YDiv2) / i4YDiv) * i4YDiv;

    // Update original with the new values
    pntToAdjust.Set(i4X, i4Y);

    // And return true if we changed the value
    return (pntSave != pntToAdjust);
#endif

    return kCIDLib::False;
}


//
//  Undoes any drag operation. This is called at the legitimate end of a drag,
//  if some error occurs while dragging, or we lose capture of the mouse for
//  some reason.
//
tCIDLib::TVoid TIntfEditWnd::CancelDrag(const tCIDLib::TBoolean bRedraw)
{
    m_eDragMode = EDragModes::None;
    m_eDragArea = tCIDCtrls::EWndAreas::Nowhere;

    // Release the mouse and the mouse confinement
    facCIDCtrls().ReleasePointer();
    ReleaseMouse();

    //
    //  Force a full redraw to get rid of any drag outline, if asked to. The
    //  caller may have to already redraw for some reason, and will therefore
    //  not ask us to.
    //
    if (bRedraw)
        m_pcivEdit->Redraw();
}


//
//  Adjust the z-order of the selected widget or a selected group
//
//  NOTE that these are only enabled in the popup menu if either a single widget is
//  selected or a full group is selected. So we never have to worry about breaking a
//  group here.
//
//  NOTE that this may have come from the widget palette. But we still send him a
//  msg, since he doesn't make the changes himself. To keep things sane, he just sends
//  us a msg, and then reacts to the change the same as if the user had done it.
//
//  This is also just practicality since it's the underlying vector collection that
//  is sending out stuff from our side, and it would be really annoying to have to try
//  to selectively have him send msgs or not.
//
tCIDLib::TVoid TIntfEditWnd::ChangeZOrder(const tCIDLib::TCard4 c4Cmd)
{
    TCQCIntfTemplate& iwdgTmpl = iwdgEdit();

    // If we only have one or none, do nothing
    const tCIDLib::TCard4 c4CurCount = iwdgTmpl.c4ChildCount();
    if (c4CurCount < 2)
        return;

    // Convert the command to our zorder change enum
    tCQCIntfEng::EZOrder eOrder;
    switch(c4Cmd)
    {
        case kCQCIntfEd::ridMenu_Edit_ToBack :
            eOrder = tCQCIntfEng::EZOrder::ToBack;
            break;

        case kCQCIntfEd::ridMenu_Edit_ToFront :
            eOrder = tCQCIntfEng::EZOrder::ToFront;
            break;

        case kCQCIntfEd::ridMenu_Edit_MoveBack :
            eOrder = tCQCIntfEng::EZOrder::Back;
            break;

        case kCQCIntfEd::ridMenu_Edit_MoveForward :
            eOrder = tCQCIntfEng::EZOrder::Forward;
            break;

        default :
            CIDAssert2(L"Unknown Z-order change command")
            break;
    };

    // Get the selected widget (or first group member) for use below
    TCQCIntfWidget* piwdgTar = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(0));

    //
    //  Provisionally push a z-order undo, though we may end up poping it if we don't
    //  actually end up changing the z-order.
    //
    m_uhEditor.Push(new TIntfZOrderUndo(iwdgEdit()));

    //
    //  NOTE:
    //
    //  This guy can return the actual count as the new z-order, if this operation
    //  moves a single item to the end.
    //
    //  DO NOT use this returned value as an actual index since it could be invalid.
    //
    tCIDLib::TBoolean bRes;
    tCIDLib::TCard4 c4NewZO;
    if (m_selList.c4Count() > 1)
    {
        //
        //  This can happen via hot key as well as menu. For the menu we disable it if
        //  there isn't a single group selected. But we need to check again in case of
        //  the hot key scenario. We know there's more than one selected; so, if this
        //  returns true, it has to be a single group and the group order change is
        //  valid.
        //
        if (bSingleSelected(kCIDLib::True))
            bRes = iwdgTmpl.bChangeGroupZOrder(piwdgTar->c4GroupId(), eOrder, c4NewZO);
    }
     else
    {
        bRes = iwdgTmpl.bChangeWidgetZOrder(piwdgTar, eOrder, c4NewZO);
    }

    //
    //  If we actually did anything, redraw. Else toss the undo we pushed above. If we
    //  did something, and there's a single item selected, update the attribute editor
    //  since the z-order is displayed for read only purposes.
    //
    if (bRes)
    {
        m_pcivEdit->Redraw();

        if (m_selList.bSingleSel())
            UpdateAttrEd();
    }
     else
    {
        m_uhEditor.DiscardToSep();
    }
}


//
//  This is called from the IPE value change callback and the attribute editor window's
//  notification handler. We watch for some attributes to change that we need to do
//  something in response to, other than just the storing away of the change.
//
tCIDLib::TVoid
TIntfEditWnd::CheckAttrChange(          TCQCIntfWidget& iwdgTar
                                , const TAttrData&      adatOld
                                , const TAttrData&      adatNew)
{
    if (adatNew.strId() ==  kCQCIntfEng::strAttr_Base_Area)
    {
        //
        //  We need to redraw the combination of the old and new widget area so that
        //  any selection handles get redrawn. When done directly by the user via mouse
        //  or keyboard it's already handled. But here it's being forced via the attribute
        //  editor.
        //
        TArea areaUpdate(adatOld.areaVal());
        areaUpdate.TakeLarger(adatNew.areaVal());
        areaUpdate.Inflate(1);
        m_pcivEdit->Redraw(areaUpdate);
    }
     else if (adatNew.strId() ==  kCQCIntfEng::strAttr_Base_Size)
    {
        if (iwdgTar.bIsA(TCQCIntfTemplate::clsThis()))
        {
            //
            //  When it's the template, make the actual window a bit larger than the actual
            //  size, so that the user can size the template up from there. And then do
            //  a full redraw so that we get rid of any unused area when down-sizing and
            //  we make sure the selection handles get redrawn correctly.
            //
            //  Note that the attribute editor insures we don't get sized too small to
            //  hold our child widgets, but if debugging we'll verify that.
            //
            TSize szWnd = adatNew.szVal();

            #if CID_DEBUG_ON
            TCQCIntfTemplate& iwdgTmpl = static_cast<TCQCIntfTemplate&>(iwdgTar);
            TArea areaMin = iwdgTmpl.areaHull();
            if ((szWnd.c4Width() < areaMin.c4Width())
            ||  (szWnd.c4Height() < areaMin.c4Height()))
            {
                TErrBox msgbTooSmall(L"The template was sized too small to hold its children");
                msgbTooSmall.ShowIt(*this);
            }
            #endif

            szWnd.Adjust(256, 256);
            SetSize(szWnd, kCIDLib::True);

            m_pcivEdit->Redraw();
        }
    }
     else if ((adatNew.strId() != kCQCIntfEng::strAttr_Base_Grouped)
          &&  (adatNew.strId() != kCQCIntfEng::strAttr_Base_WidgetId))
    {
        //
        //  If not an obviously non-visual attribute, redraw the target to insure the
        //  changes are made visible.
        //
        m_pcivEdit->Redraw(iwdgTar.areaRelative());
    }
}


//
//  Do a copy or cut operation on the selected widget
//
tCIDLib::TVoid TIntfEditWnd::CopyCutSelected(const tCIDLib::TBoolean bCut)
{
    // If nothing selected, or it's the template itself, then do nothing
    if (m_selList.bIsEmpty() || (m_selList.c8At(0) == iwdgEdit().c8DesId()))
        return;

    //
    //  Clear the clipboard, and put the target object(s) on it. We
    //  use a special class, which is just small wrapper around a
    //  memory buffer, to flatten the objects into and then to put
    //  onto the clipboard. We can only put a single object, and there
    //  could be multiple objects selected, so we do it this way with
    //  the simple wrapper.
    //
    try
    {
        TGUIClipboard gclipTmp(*this);
        gclipTmp.Clear();

        TCQCIntfEdWdgCB wcbTmp(iwdgEdit(), m_selList.fcolIds());
        gclipTmp.WriteObj<TCQCIntfEdWdgCB>(wcbTmp);

        //
        //  If its a cut, then then delete the selected items and push a cut undo onto
        //  the undo handler for these widgets.
        //
        if (bCut)
            DeleteSelected(kCIDLib::False);
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEd().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgErr
        (
            *this
            , facCQCIntfEd().strMsg(kIEdMsgs::midWnd_AppName)
            , errToCatch.strErrText()
            , errToCatch
        );
    }
}


//
//  This is called when the user selects one of the New popup submenu items, to
//  create a new widget. The command id is the menu id of the item under the New
//  sub-menu.
//
tCIDLib::TVoid
TIntfEditWnd::CreateNewWidget(  const   tCIDLib::TCard4 c4CmdId
                                , const TPoint&         pntAt)
{
    //
    //  It's one of the dynamically added items under the New submenu. So get
    //  the text of the selected item and use it to create a new widget.
    //
    TString strType;
    TSubMenu menuNew(m_menuEdit, kCQCIntfEd::ridMenu_Edit_New);
    menuNew.QueryItemText(c4CmdId, strType);
    TCQCIntfWidget* piwdgNew = facCQCIntfEng().piwdgMakeInstanceOf(strType);

    // Tell it we are its owning window
    piwdgNew->SetOwner(m_pcivEdit);

    //
    //  Now set the initial area to the placement point and the new widget's
    //  desired initial size, and tell it about our origin, which it is relative
    //  to.
    //
    //  Make sure that new area won't exceed the parent template
    //  size. If so, make it fit.
    //
    const TArea& areaTar = iwdgEdit().areaActual();
    TArea areaNew(pntAt, piwdgNew->szDefaultSize());
    areaNew.ForceWithin(areaTar, kCIDLib::True);

    //
    //  If auto-snap is turned on, then we need to snap this guy
    //  to the grid. Give the widget area as the limit so that the
    //  snap won't push it off the widget.
    //
    bSnapArea(areaNew, iwdgEdit().areaActual(), kCIDLib::False);

    //
    //  And now we can set it's position and tell it the area of
    //  the parent widget (the template), so that it can calc its
    //  relative position.
    //
    piwdgNew->SetPositions(areaTar.pntOrg(), areaNew);

    //
    //  Initialize the object, which requires a data server client, since it may
    //  need to load images or other resources.
    //
    TDataSrvClient dsclNew;
    tCQCIntfEng::TErrList colErrs;

    //
    //  Assign this guy a design time id. We take the template
    //  level part of the template's id and add the next available
    //  widget id to it.
    //
    piwdgNew->c8DesId
    (
        TRawBits::c8From32(m_c4NextWId++, tCIDLib::TCard4(iwdgEdit().c8DesId() >> 32))
    );

    // And now initialize the widget
    piwdgNew->Initialize(&iwdgEdit(), dsclNew, colErrs);

    // Push an insert undo operation
    m_uhEditor.Push(new TIntfInsertUndo(*piwdgNew));

    // And finally add it to the template as a child widget
    iwdgEdit().AddChild(piwdgNew);

    // Make it the single selection
    AddToSelection(piwdgNew, kCIDLib::True, kCIDLib::False);

    // Do a validation pass
    Validate();

    // And force a redraw of its area to show it and the selection handles on it
    m_pcivEdit->Redraw(areaNew);
}


//
//  This is called when the user chooses to delete objects from the template. We go
//  through all of the currently selected objects and remove them. We keep a running
//  region for the removed objects, and redraw those areas at the end.
//
//  This cannot be called if any group is partially selected, so we don't have to
//  worry about breaking up a group.
//
tCIDLib::TVoid TIntfEditWnd::DeleteSelected(const tCIDLib::TBoolean bConfirm)
{
    // If nothing selected, or it is the template itself, then do nothing.
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count || (m_selList.c8At(0) == iwdgEdit().c8DesId()))
        return;

    // Make sure that they want to do it if we were asked to
    if (bConfirm)
    {
        TYesNoBox msgbAsk
        (
            facCQCIntfEd().strMsg(kIEdMsgs::midWnd_AppName)
            , facCQCIntfEd().strMsg(kIEdMsgs::midQ_DeleteSel)
        );

        if (!msgbAsk.bShowIt(*this))
            return;
    }

    // Push a cut undo for the selected widgets
    m_uhEditor.Push(new TIntfCutUndo(iwdgEdit(), m_selList.fcolIds()));

    TCQCIntfTemplate& iwdgUs = iwdgEdit();
    TGUIRegion grgnUpdate;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));

        // Add this guy to the update region, then remove it
        if (c4Index)
            grgnUpdate.CombineWith(piwdgCur->areaActual(), tCIDGraphDev::EClipModes::Or);
        else
            grgnUpdate.Set(piwdgCur->areaActual());

        iwdgUs.RemoveWidget(piwdgCur);
    }

    //
    //  Clear the selection list, since we just deleted them all. Since we are doing it
    //  manually, we have to push a select undo.
    //
    m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));
    m_selList.Clear();

    // Update the attribute editor since we affected the selection
    UpdateAttrEd();

    // Do a validation pass
    Validate();

    // And redraw the affected parts
    Redraw(grgnUpdate, tCIDCtrls::ERedrawFlags::ImmedChildren);
}



//
//  Remove the passed widget or group from the selection list
//
tCIDLib::TVoid TIntfEditWnd::Deselect(const TCQCIntfWidget* const piwdgTar)
{
    const tCIDLib::TCard8 c8DesId = piwdgTar->c8DesId();
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (c8DesId == m_selList.c8At(c4Index))
        {
            TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(c8DesId);
            m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));

            m_selList.Remove(c8DesId);
            if (piwdgCur)
                m_pcivEdit->Redraw(piwdgCur->areaActual());
            break;
        }
    }

    // Update the attribute editor since we affected the selection
    UpdateAttrEd();
}


//
//  Deselect any widgets that are in the indicated group
//
tCIDLib::TVoid TIntfEditWnd::DeselectGrp(const tCIDLib::TCard4 c4GroupId)
{
    // Get all of the widgets in this group
    tCIDLib::TCard4 c4FirstZ;
    tCQCIntfEng::TChildList colGrp(tCIDLib::EAdoptOpts::NoAdopt);
    if (!m_pcivEdit->bFindGroupMembers(c4GroupId, colGrp, c4FirstZ))
        return;

    // We may toss it if we never deselect any
    m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));

    // Enable block mode so that we don't get a bunch of individual published changes
    TColBlockModeJan janBlock(&m_selList.fcolIds(), kCIDLib::False);
    TArea areaUpdate;
    const tCIDLib::TCard4 c4Count = colGrp.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_selList.bIsSelected(colGrp[c4Index]->c8DesId()))
        {
            areaUpdate |= colGrp[c4Index]->areaActual();
            m_selList.Remove(colGrp[c4Index]->c8DesId());
        }
    }

    if (areaUpdate.bIsEmpty())
        m_uhEditor.DiscardTop();
    else
        m_pcivEdit->Redraw(areaUpdate);

    // Update the attribute editor in case we affected the selection
    UpdateAttrEd();
}


//
//  We have to take all of the selected widgets and group them. This involves
//  a few potential steps. We know that if we have any members of a group selected
//  thenwe have them all, so we know that we won't split a group or anything.
//
//  We have to insure that all of the widgets are moved together in z-order,
//  so we find the one with the lowest z-order, then we move the rest of them
//  into the z-order after that, in their original order.
//
tCIDLib::TVoid TIntfEditWnd::DoGrouping()
{
    //
    //  Do a full template undo push. In this case we are saving the whole template,
    //  because this could change zorders and group indexes and all that.
    //
    m_uhEditor.Push(new TIntfFullUndo(iwdgEdit()));

    TCQCIntfTemplate& iwdgTmpl = iwdgEdit();

    // Get a list of the zorders
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    TFundVector<tCIDLib::TCard4> fcolZOrders(c4Count);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        tCIDLib::TCard4 c4ZOrder;
        if (m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index), c4ZOrder))
            fcolZOrders.c4AddElement(c4ZOrder);
    }

    // Make sure we got them all
    CIDAssert
    (
        fcolZOrders.c4ElemCount() == c4Count
        , L"Didn't find all selected widgets by design id"
    );

    //
    //  Sort it, which now gives us a list of indices into the widget list
    //  and the first one is the lowest z-order, and that's were we'll start
    //  moving the others down to get them all contiguous.
    //
    fcolZOrders.Sort(tCIDLib::eComp<tCIDLib::TCard4>);

    //
    //  Now let's get a list of pointers to the widget in z-order. They may not
    //  be either contiguous or in order at this point.
    //
    tCQCIntfEng::TChildList colGroup(tCIDLib::EAdoptOpts::NoAdopt);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget& iwdgCur = iwdgTmpl.iwdgAt(fcolZOrders[c4Index]);
        colGroup.Add(&iwdgCur);
    }

    //
    //  Now we will go through, starting at the second one, and move them
    //  into z-orders following the first one. As we go, we also set the
    //  group id. We replace any existing group ids (if they are grouping
    //  groups) with a new one. Set the first one as well of course.
    //
    //  We also clear out any group names along the way. The user will have
    //  to provide a new name for this new group.
    //
    const tCIDLib::TCard4 c4NewGrpId = iwdgTmpl.c4GetNextGroupId();
    colGroup[0]->c4GroupId(c4NewGrpId);

    tCIDLib::TBoolean bChanges = kCIDLib::False;
    tCIDLib::TCard4 c4ZOrder = fcolZOrders[0];
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = colGroup[c4Index];
        c4ZOrder++;

        if (piwdgCur != &iwdgTmpl.iwdgAt(c4ZOrder))
        {
            iwdgEdit().ChangeZOrder(piwdgCur, c4ZOrder);
            bChanges = kCIDLib::True;
        }

        piwdgCur->c4GroupId(c4NewGrpId);
        piwdgCur->strGroupName(TString::strEmpty());
    }

    // If we affected the z-order, then redraw
    if (bChanges)
        ForceRepaint();
}


// Does the work of a search and replace operation
tCIDLib::TVoid TIntfEditWnd::DoSearchAndReplace()
{
    //
    //  If nothing is selected, do nothing. This can happen if they invoke
    //  us via hot key, which they can do at any time. If invoked from the
    //  edit menu, it'll disable this option if nothing is selected.
    //
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count)
        return;

    // See if just the template itself is selected
    tCIDLib::TBoolean bTmplSelected = kCIDLib::False;
    if (c4Count == 1)
    {
        bTmplSelected = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(0))->bIsA
        (
            TCQCIntfTemplate::clsThis()
        );
    }

    tCIDLib::TBoolean        bFull;
    tCIDLib::TBoolean        bRegEx;
    TString                  strFindStr;
    TString                  strRepStr;
    tCQCIntfEng::ERepFlags   eFlags;

    TIntfSearchDlg dlgSearch;
    const tCIDLib::TBoolean bSearch = dlgSearch.bRun
    (
        *this
        , bFull
        , bRegEx
        , strFindStr
        , strRepStr
        , eFlags
        , bTmplSelected
    );

    if (bSearch)
    {
        //
        //  If it's a regular expression, we have to set that up now since we
        //  pass it into the search and replace.
        //
        TRegEx regxFind;
        if (bRegEx)
            regxFind.SetExpression(strFindStr);

        //
        //  Push a full undo for the selected widgets so that this whole
        //  operation can be undone.
        //
        m_uhEditor.Push(new TIntfFullUndo(iwdgEdit()));

        //
        //  We just run through all the selected widgets and invoke the search
        //  and replace on them. If the template it selected, it can only ever
        //  be selected by itself, and the 'children' option will control
        //  whether contained widgets are searched.
        //
        TCQCIntfWidget* piwdgCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));
            piwdgCur->Replace
            (
                eFlags, strFindStr, strRepStr, bRegEx, bFull, regxFind
            );
        }

        // If images were enabled, then force a refresh of any images
        try
        {
            TDataSrvClient dsclRefresh;
            m_pcivEdit->iwdgBaseTmpl().RefreshImages(dsclRefresh);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"An error occurred during the search and replace operation"
                , errToCatch
            );
        }

        // Force a full repaint to display any changes
        ForceRepaint();
    }
}


//
//  Go through all of the selected widgets and just clear the group id and group
//  name. We know we can't have any partial groups here, so this is safe. This
//  has no visible side effects.
//
tCIDLib::TVoid TIntfEditWnd::DoUngrouping()
{
    // We have to have at least one widget selected
    const tCIDLib::TCard4 c4SelCnt = m_selList.c4Count();
    if (!c4SelCnt)
        return;

    // Do a full template undo push
    m_uhEditor.Push(new TIntfFullUndo(iwdgEdit()));

    TCQCIntfWidget* piwdgCur = nullptr;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SelCnt; c4Index++)
    {
         piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));
         piwdgCur->c4GroupId(0);
         piwdgCur->strGroupName(TString::strEmpty());
    }
}


//
//  Draws the selection handles on all selected widgets. This is primarly for the
//  focus get/lose since we need to change the handle colors based on whether we have
//  focus or not.
//
tCIDLib::TVoid TIntfEditWnd::DrawAllSelHandles()
{
    // If no selected widgets, nothing to do
    const tCIDLib::TCard4 c4SelCnt = m_selList.c4Count();
    if (!c4SelCnt)
        return;

    // Else loop through them and update each one
    TGraphWndDev gdevToUse(*this);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SelCnt; c4Index++)
    {
         TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));

         // Should always be found, but just in case
         if (piwdgCur)
             DrawSelHandles(gdevToUse, *piwdgCur);
     }
}


//
//  Draw selection handles on a given widget, or on all of teh selected widgets. We draw
//  one in each corner and one in the middle of each edge
//
tCIDLib::TVoid
TIntfEditWnd::DrawSelHandles(       TGraphDrawDev&  gdevToUse
                            , const TCQCIntfWidget& iwdgHit)
{
    TArea areaAt(iwdgHit.areaActual());

    //
    //  We need a solid brush to fill the handle blocks, which we fill with
    //  either black if we have the focus or dark grey if we don't.
    //
    //  And we use a white stroke around it, so that no matter what type of
    //  background, one of them should show up.
    //
    TSolidBrush gbrFill(bHasFocus() ? facCIDGraphDev().rgbBlack : facCIDGraphDev().rgbPaleGrey);

    // Draw the handles just inside the widget, on each border
    TArea areaHandle(areaAt);
    areaHandle.SetSize(5, 5);

    // Do the left edge
    areaHandle.LeftJustifyIn(areaAt, kCIDLib::True);
    gdevToUse.Fill(areaHandle, gbrFill);
    gdevToUse.Stroke(areaHandle, facCIDGraphDev().rgbWhite);

    // Do the right edge
    areaHandle.RightJustifyIn(areaAt, kCIDLib::True);
    gdevToUse.Fill(areaHandle, gbrFill);
    gdevToUse.Stroke(areaHandle, facCIDGraphDev().rgbWhite);

    // Do the top edge
    areaHandle.TopJustifyIn(areaAt, kCIDLib::True);
    gdevToUse.Fill(areaHandle, gbrFill);
    gdevToUse.Stroke(areaHandle, facCIDGraphDev().rgbWhite);

    // Do the bottom edge
    areaHandle.BottomJustifyIn(areaAt, kCIDLib::True);
    gdevToUse.Fill(areaHandle, gbrFill);
    gdevToUse.Stroke(areaHandle, facCIDGraphDev().rgbWhite);

    // Do the upper left corner
    areaHandle.JustifyIn(areaAt, tCIDLib::EHJustify::Left, tCIDLib::EVJustify::Top);
    gdevToUse.Fill(areaHandle, gbrFill);
    gdevToUse.Stroke(areaHandle, facCIDGraphDev().rgbWhite);

    // Do the upper right corner
    areaHandle.JustifyIn(areaAt, tCIDLib::EHJustify::Right, tCIDLib::EVJustify::Top);
    gdevToUse.Fill(areaHandle, gbrFill);
    gdevToUse.Stroke(areaHandle, facCIDGraphDev().rgbWhite);

    // Do the bottom left
    areaHandle.JustifyIn(areaAt, tCIDLib::EHJustify::Left, tCIDLib::EVJustify::Bottom);
    gdevToUse.Fill(areaHandle, gbrFill);
    gdevToUse.Stroke(areaHandle, facCIDGraphDev().rgbWhite);

    // Do the bottom right
    areaHandle.JustifyIn(areaAt, tCIDLib::EHJustify::Right, tCIDLib::EVJustify::Bottom);
    gdevToUse.Fill(areaHandle, gbrFill);
    gdevToUse.Stroke(areaHandle, facCIDGraphDev().rgbWhite);
}


tCIDLib::TVoid
TIntfEditWnd::DrawSelHandlesIfSelected(         TGraphDrawDev&  gdevToUse
                                        , const tCIDLib::TCard8 c8ToCheck)
{
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_selList.c8At(c4Index) == c8ToCheck)
        {
            DrawSelHandles(gdevToUse, *m_pcivEdit->piwdgFindByDesId(c8ToCheck));
            break;
        }
    }
}


//
//  This guy will draw all the widgets (in z-order so that overlapping works
//  correctly) that intersect the passed area.
//
//  NOTE:   We call an underlying method on the template class, but we have
//          our own little wrapper to handle insuring that selection handles
//          get updated, which only we understand.
//
tCIDLib::TVoid
TIntfEditWnd::DrawUnder(        TGraphDrawDev&  gdevToUse
                        , const TArea&          areaUnder)
{
    iwdgEdit().bDrawUnder(gdevToUse, areaUnder);

    //
    //  Go back through and redraw the selection handles for any widgets that
    //  intersect the area.
    //
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));
        if (!piwdgCur)
            continue;
        if (piwdgCur->areaActual().bIntersects(areaUnder))
            DrawSelHandles(gdevToUse, *piwdgCur);
    }
}


//
//  This is a special case. If the user does a Alt-Shift click, this is used to drill down
//  through layered widgets, to allow them to be selected.
//
//  If there's a single widget selected, and the click point is within its area, then we
//  look for another one below it and select that. If neither of those things is true, then
//  we just remove any previous selection and select the new guy hit.
//
//  Of course we have to skip locked ones. We know we never get called if they clicked on
//  the template itself. However they might have previously drilled down to the template.
//  We check if the template is selected and just select the hit one, which effectively
//  takes us back to the top.
//
tCIDLib::TVoid TIntfEditWnd::DrillDown(const TPoint& pntAt, TCQCIntfWidget& iwdgHit)
{
    // If nothing selected, or the template is, then just select the new guy and return
    if ((m_selList.c4Count() != 1)
    ||  m_selList.c8At(0) == iwdgEdit().c8DesId())
    {
        if (!bIsSelected(iwdgHit.c8DesId()))
            AddToSelection(&iwdgHit, kCIDLib::True, kCIDLib::True);
        return;
    }

    //
    //  Get the one selected guy. If it doesn't contain the hit point, then select the
    //  new guy and we are done.
    //
    TCQCIntfWidget* piwdgSel = iwdgEdit().piwdgFindByDesId(m_selList.c8At(0));
    if (!piwdgSel->areaActual().bContainsPoint(pntAt))
    {
        if (piwdgSel->c8DesId() != iwdgHit.c8DesId())
            AddToSelection(&iwdgHit, kCIDLib::True, kCIDLib::True);
        return;
    }

    //
    //  Work backwards down the z-order to zero until we either hit the end or hit
    //  another widget. We have to have started on something past the lowest z-order
    //  since we need to move down below the starting one.
    //
    //  If we never find one, we go to the hit one, which will be back at the top level
    //  at this click point.
    //
    tCIDLib::TCard4 c4ZInd = iwdgEdit().c4WdgZOrder(piwdgSel);
    while (c4ZInd)
    {
        c4ZInd--;
        TCQCIntfWidget& iwdgCur = iwdgEdit().iwdgAt(c4ZInd);
        if (!iwdgCur.bIsLocked() && iwdgCur.areaActual().bContainsPoint(pntAt))
        {
            // Select this new guy, replacing any previous and return
            AddToSelection(&iwdgCur, kCIDLib::True, kCIDLib::True);
            return;
        }
    }

    //
    //  Never found one, so take the template. We know the template is not the selected
    //  one currently since we checked for that above.
    //
    AddToSelection(&iwdgEdit(), kCIDLib::True, kCIDLib::True);
}



//
//  We handle notifications from the attribute editor window.
//
tCIDCtrls::EEvResponses TIntfEditWnd::eAttrEditHandler(TAttrEditInfo& wnotInfo)
{
    tCIDLib::TCard4 c4ZOrder;
    TCQCIntfWidget* piwdgTar = iwdgEdit().piwdgFindByDesId(wnotInfo.c8UserId(), c4ZOrder);
    CIDAssert(piwdgTar != nullptr, L"The target widget could not be found");

    //
    //  Push an attribute undo for this one if it's a change made by the user, not one
    //  made by us undoing a change or other playing of tricks. Tell it to push a
    //  separator.
    //
    if (wnotInfo.bFromUser())
    {
        m_uhEditor.Push
        (
            new TIntfAttrUndo(piwdgTar->c8DesId(), wnotInfo.adatNew(), wnotInfo.adatOld())
            , kCIDLib::True
        );
    }

    //
    //  And now set it on the widget. The widget will store it away. Any mixins and derived
    //  classes along the way can watch for things it needs to react to as well.
    //
    piwdgTar->SetWdgAttr(*m_pwndAttrEd, wnotInfo.adatNew(), wnotInfo.adatOld());

    // Push an attribute undo for this one. Tell it to push a separator
    m_uhEditor.Push
    (
        new TIntfAttrUndo(piwdgTar->c8DesId(), wnotInfo.adatNew(), wnotInfo.adatOld())
        , kCIDLib::True
    );

    // Do a validation to display an error/warning messages that this change might cause
    Validate();

    const TString& strId = wnotInfo.adatNew().strId();

    if (strId == kCQCIntfEng::strAttr_Base_Locked)
    {
        // If it was the lock state, make sure it's not selected
        if (piwdgTar->bIsLocked())
            Deselect(piwdgTar);
    }
     else if (strId == kCQCIntfEng::strAttr_Img_UseImage)
    {
        //
        //  If they change the 'use image' attribute, we have to enable/disable editing
        //  on the other image related attributes, and we clear the image path. We let
        //  these come back to us as notifications so we'll update the widget.
        //
        const tCIDMData::EAttrEdTypes eNewEdType
        (
            wnotInfo.adatNew().bVal() ? tCIDMData::EAttrEdTypes::Both
                                      : tCIDMData::EAttrEdTypes::None
        );

        m_pwndAttrEd->ChangeEditType(kCQCIntfEng::strAttr_Img_Image, eNewEdType);
        m_pwndAttrEd->ChangeEditType(kCQCIntfEng::strAttr_Img_Placement, eNewEdType);
        m_pwndAttrEd->ChangeEditType(kCQCIntfEng::strAttr_Img_HitTransparent, eNewEdType);

        m_pwndAttrEd->ChangeEditType
        (
            kCQCIntfEng::strAttr_Img_Opacity
            , wnotInfo.adatNew().bVal() ? tCIDMData::EAttrEdTypes::InPlace
                                        : tCIDMData::EAttrEdTypes::None
        );

        // If disabling, then clear out the string path attribute
        if (!wnotInfo.adatNew().bVal())
            m_pwndAttrEd->UpdateString(kCQCIntfEng::strAttr_Img_Image, TString::strEmpty());
    }
     else if ((strId == kCQCIntfEng::strAttr_Slider_UseLimits)
          ||  (strId == kCQCIntfEng::strAttr_VKnob_UseLimits))
    {
        //
        //  If they change the 'use limits' attribute of a widget that supports them
        //  then we need to enable/disable the limits attribute. The widget itself
        //  will update its own state as required. The limits are just binary so it
        //  doesn't have to actually update the attribute display info or anything.
        //
        TString strChangeAttr;
        if (strId == kCQCIntfEng::strAttr_Slider_UseLimits)
            strChangeAttr = kCQCIntfEng::strAttr_Slider_Limits;
        else
            strChangeAttr = kCQCIntfEng::strAttr_VKnob_Limits;

        m_pwndAttrEd->ChangeEditType
        (
            strChangeAttr
            , wnotInfo.adatNew().bVal() ? tCIDMData::EAttrEdTypes::Visual
                                        : tCIDMData::EAttrEdTypes::None
        );
    }
     else if (strId == kCQCIntfEng::strAttr_Img_Image)
    {
        MCQCIntfImgIntf* pmiwdgImg = dynamic_cast<MCQCIntfImgIntf*>(piwdgTar);
        CIDAssert(pmiwdgImg != nullptr, L"The widget does not implement the image mixin");

        //
        //  The image interface handles clearing the image, but it can't handle
        //  setting a new image, so we deal with that here.
        //
        if (pmiwdgImg)
        {
            const TString& strImg = wnotInfo.adatNew().strValue();
            if (!strImg.bIsEmpty())
            {
                try
                {
                    TDataSrvClient dsclImg;
                    pmiwdgImg->SetNewImage(*m_pcivEdit, dsclImg, strImg, kCIDLib::True);
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }
        }
    }
     else if (strId == kCQCIntfEng::strAttr_NumText_FormatStyle)
    {
        const tCQCIntfEng::ENumTxtStyles eStyle
        (
            tCQCIntfEng::eXlatENumTxtStyles(wnotInfo.adatNew().strValue())
        );

        // Update the edit type of the groups and decimal digits based on the style
        m_pwndAttrEd->ChangeEditType
        (
            kCQCIntfEng::strAttr_NumText_UseGroups
            , (eStyle == tCQCIntfEng::ENumTxtStyles::Custom) ? tCIDMData::EAttrEdTypes::InPlace
                                                           : tCIDMData::EAttrEdTypes::None
        );

        m_pwndAttrEd->ChangeEditType
        (
            kCQCIntfEng::strAttr_NumText_DecDigits
            , (eStyle == tCQCIntfEng::ENumTxtStyles::Custom) ? tCIDMData::EAttrEdTypes::InPlace
                                                           : tCIDMData::EAttrEdTypes::None
        );
    }
     else if ((strId == kCQCIntfEng::strAttr_Static_UseRange)
          ||  (strId == kCQCIntfEng::strAttr_Static_Type))
    {
        //
        //  If the 'use range' flag of the static mixin is disabled, or the data type
        //  is changed, then the range will have been forced to the max values. The
        //  widget will have updated the actual values, but we need to update the
        //  attributes.
        //
        MCQCIntfStaticIntf* pmiwdgStatic = dynamic_cast<MCQCIntfStaticIntf*>(piwdgTar);
        CIDAssert(pmiwdgStatic != nullptr, L"The widget does not implement the static mixin");

        //
        //  If the type changed, then update the type of the min/max attributes. We
        //  have to do that before we do the below, because if the type changed it
        //  would fail below with a wrong type error.
        //
        if (strId == kCQCIntfEng::strAttr_Static_Type)
        {
            tCIDMData::EAttrTypes eAttrType;
            if (pmiwdgStatic->eDataType() == tCQCKit::EFldTypes::Card)
                eAttrType = tCIDMData::EAttrTypes::Card;
            else if (pmiwdgStatic->eDataType() == tCQCKit::EFldTypes::Float)
                eAttrType = tCIDMData::EAttrTypes::Float;
            else if (pmiwdgStatic->eDataType() == tCQCKit::EFldTypes::Int)
                eAttrType = tCIDMData::EAttrTypes::Int;

            m_pwndAttrEd->ChangeAttrType(kCQCIntfEng::strAttr_Static_Min, eAttrType);
            m_pwndAttrEd->ChangeAttrType(kCQCIntfEng::strAttr_Static_Max, eAttrType);
        }

        if (pmiwdgStatic->eDataType() == tCQCKit::EFldTypes::Card)
        {
            m_pwndAttrEd->UpdateCard
            (
                kCQCIntfEng::strAttr_Static_Min
                , tCIDLib::TCard4(pmiwdgStatic->f8RangeMin())
                , kCIDLib::False
            );
            m_pwndAttrEd->UpdateCard
            (
                kCQCIntfEng::strAttr_Static_Max
                , tCIDLib::TCard4(pmiwdgStatic->f8RangeMax())
                , kCIDLib::False
            );
        }
         else if (pmiwdgStatic->eDataType() == tCQCKit::EFldTypes::Float)
        {
            m_pwndAttrEd->UpdateFloat
            (
                kCQCIntfEng::strAttr_Static_Min
                , pmiwdgStatic->f8RangeMin()
                , kCIDLib::False
            );
            m_pwndAttrEd->UpdateFloat
            (
                kCQCIntfEng::strAttr_Static_Max
                , pmiwdgStatic->f8RangeMax()
                , kCIDLib::False
            );
        }
         else if (pmiwdgStatic->eDataType() == tCQCKit::EFldTypes::Int)
        {
            m_pwndAttrEd->UpdateInt
            (
                kCQCIntfEng::strAttr_Static_Min
                , tCIDLib::TInt4(pmiwdgStatic->f8RangeMin())
                , kCIDLib::False
            );
            m_pwndAttrEd->UpdateInt
            (
                kCQCIntfEng::strAttr_Static_Max
                , tCIDLib::TInt4(pmiwdgStatic->f8RangeMax())
                , kCIDLib::False
            );
        }

        //
        //  Make sure the min/max values are enabled/disabled appropriately. If this is
        //  the actual use range being set, use that since the value won't have been
        //  stored yet.
        //
        const tCIDLib::TBoolean bUse
        (
            m_pwndAttrEd->adatFindById(kCQCIntfEng::strAttr_Static_UseRange).bVal()
        );

        m_pwndAttrEd->ChangeEditType
        (
            kCQCIntfEng::strAttr_Static_Min
            , bUse ? tCIDMData::EAttrEdTypes::InPlace : tCIDMData::EAttrEdTypes::None
        );

        m_pwndAttrEd->ChangeEditType
        (
            kCQCIntfEng::strAttr_Static_Max
            , bUse ? tCIDMData::EAttrEdTypes::InPlace : tCIDMData::EAttrEdTypes::None
        );
    }
     else if (strId == kCQCIntfEng::strAttr_Widget_States)
    {
        // Get the states info out
        tCIDLib::TStrList colCurStates;
        if (wnotInfo.adatNew().c4Bytes())
        {
            TBinMBufInStream strmSrc
            (
                &wnotInfo.adatNew().mbufVal(), wnotInfo.adatNew().c4Bytes()
            );
            strmSrc >> colCurStates;
        }

        const tCIDLib::TBoolean bHaveStates = !colCurStates.bIsEmpty();

        //
        //  Enable or disable the other related states. Also, if disabled, we remove
        //  all of the state names from the widget.
        //
        tCIDMData::EAttrEdTypes eNewEdType
        (
            bHaveStates ? tCIDMData::EAttrEdTypes::Both : tCIDMData::EAttrEdTypes::None
        );

        // Change the edit type on the related attributes
        m_pwndAttrEd->ChangeEditType
        (
            kCQCIntfEng::strAttr_Widget_StatesNeg, eNewEdType
        );

        m_pwndAttrEd->ChangeEditType
        (
            kCQCIntfEng::strAttr_Widget_StatesLogOp, eNewEdType
        );

        m_pwndAttrEd->ChangeEditType
        (
            kCQCIntfEng::strAttr_Widget_StatesDisplay, eNewEdType
        );
    }

    // Do a validation pass
    Validate();

    //
    //  React to some special cases which we do here and in the IPE value changed callback.
    //
    //  <TBD> The IPE changes don't come through here, which is really wrong. We should
    //  be handling them in our attr editor derivative and faking calls to this guy
    //  instead of having to handle them twice.
    //
    CheckAttrChange(*piwdgTar, wnotInfo.adatOld(), wnotInfo.adatNew());

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We need to set the pointer according to the position of the mouse over
//  the edges or corners of a widget.
//
tCIDCtrls::ESysPtrs
TIntfEditWnd::ePtrForArea(const tCIDCtrls::EWndAreas eArea) const
{
    tCIDCtrls::ESysPtrs ePtr = tCIDCtrls::ESysPtrs::Arrow;
    switch(eArea)
    {
        case tCIDCtrls::EWndAreas::Left :
        case tCIDCtrls::EWndAreas::Right :
            ePtr = tCIDCtrls::ESysPtrs::SizeWE;
            break;

        case tCIDCtrls::EWndAreas::Top :
        case tCIDCtrls::EWndAreas::Bottom :
            ePtr = tCIDCtrls::ESysPtrs::SizeNS;
            break;

        case tCIDCtrls::EWndAreas::TopLeft :
        case tCIDCtrls::EWndAreas::BottomRight :
            ePtr = tCIDCtrls::ESysPtrs::SizeNWSE;
            break;

        case tCIDCtrls::EWndAreas::TopRight :
        case tCIDCtrls::EWndAreas::BottomLeft :
            ePtr = tCIDCtrls::ESysPtrs::SizeNESW;
            break;

        default :
            ePtr = tCIDCtrls::ESysPtrs::Arrow;
            break;
    }
    return ePtr;
}


//
//  Provide convenient access to the template being edited, which is in the view
//  object.
//
const TCQCIntfTemplate& TIntfEditWnd::iwdgEdit() const
{
    CIDAssert(m_pcivEdit != nullptr, L"The view object is not set on the edit window");
    return m_pcivEdit->iwdgBaseTmpl();
}

TCQCIntfTemplate& TIntfEditWnd::iwdgEdit()
{
    CIDAssert(m_pcivEdit != nullptr, L"The view object is not set on the edit window");
    return m_pcivEdit->iwdgBaseTmpl();
}


// Lock the selected items
tCIDLib::TVoid TIntfEditWnd::LockSelected()
{
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (c4Count)
    {
        // Push a lock undo
        m_uhEditor.Push(new TIntfLockUndo(m_selList.fcolIds(), kCIDLib::False));

        TCQCIntfWidget* piwdgCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));
            if (!piwdgCur)
                continue;
            piwdgCur->bIsLocked(kCIDLib::True);
        }

        // Remove any selection, because locked items cannot be selected
        RemoveSelection();
    }
}


//
//  This is used in 'drill down' selection, where we let them select progressively
//  down into the z-order via Shift-Right clicks. We get the current selection id
//  which we find. Then we search further down for another widget under it in the
//  z-order that contains the point.
//
//  If we are currently on the template itself, then we go back to the top-most
//  one again.
//
//  If we find it we return the new designer id and widget pointer.
//
TCQCIntfWidget*
TIntfEditWnd::piwdgHitNext(const TPoint& pntTest, tCIDLib::TCard8& c8DesId)
{

    // If its not within the template at all, then return zero now
    if (!iwdgEdit().areaActual().bContainsPoint(pntTest))
    {
        c8DesId = kCIDLib::c8MaxCard;
        return 0;
    }

    //
    //  We have to find the current one, and we'll leave the index on that one.
    //  If the current one is the template, then we leave it at the top to start
    //  over from the top again, else we search down for the incoming one.
    //
    tCIDLib::TInt4 i4Index = tCIDLib::TInt4(iwdgEdit().c4ChildCount());
    i4Index--;

    TCQCIntfWidget* piwdgTarget = nullptr;
    if (c8DesId != iwdgEdit().c8DesId())
    {
        while (i4Index >= 0)
        {
            TCQCIntfWidget& iwdgCur = iwdgEdit().iwdgAt(i4Index);
            if (iwdgCur.c8DesId() == c8DesId)
            {
                piwdgTarget = &iwdgCur;
                break;
            }
            i4Index--;
        }
    }

    // If we found the current one, then continue the search
    if (piwdgTarget)
    {
        //
        //  Reset the search stuff so we have to find another one. We want to
        //  explictly skip locked ones here, since otherwise we'll hit one and
        //  return it, but it will refuse to be selected. So we won't be able to
        //  get past it.
        //
        piwdgTarget = nullptr;
        i4Index--;

        while (i4Index >= 0)
        {
            TCQCIntfWidget& iwdgCur = iwdgEdit().iwdgAt(i4Index);
            if (!iwdgCur.bIsLocked() && iwdgCur.bContainsPoint(pntTest))
            {
                piwdgTarget = &iwdgCur;
                break;
            }
            i4Index--;
        }
    }

    // If not a widget, then it hit the main template background
    if (!piwdgTarget)
        piwdgTarget = &iwdgEdit();
    c8DesId = piwdgTarget->c8DesId();
    return piwdgTarget;
}


//
//  Sees if the passed point falls within one of the widgets, which can be the template
//  itserlf. If not wtihin the template area, it return null.
//
TCQCIntfWidget*
TIntfEditWnd::piwdgHitTest(const TPoint& pntTest, tCIDLib::TCard8& c8DesId)
{

    // If its not within the template at all, then return zero now
    if (!iwdgEdit().areaActual().bContainsPoint(pntTest))
    {
        c8DesId = kCIDLib::c8MaxCard;
        return nullptr;
    }

    // We have to go backwards to get the top-most one
    TCQCIntfWidget* piwdgTarget = nullptr;
    tCIDLib::TInt4 i4Count = tCIDLib::TInt4(iwdgEdit().c4ChildCount());
    i4Count--;
    while (i4Count >= 0)
    {
        TCQCIntfWidget& iwdgCur = iwdgEdit().iwdgAt(i4Count);
        if (iwdgCur.bContainsPoint(pntTest))
        {
            piwdgTarget = &iwdgCur;
            break;
        }
        i4Count--;
    }

    // If not a widget, then it hit the main template background
    if (!piwdgTarget)
        piwdgTarget = &iwdgEdit();
    c8DesId = piwdgTarget->c8DesId();
    return piwdgTarget;
}


//
//  Applies the indicated attributes to the selected widgets, or the ones that each widget
//  can accept.
//
tCIDLib::TVoid TIntfEditWnd::PasteAttrs(const tCIDMData::TAttrList& colToApply)
{
    const tCIDLib::TCard4 c4AvailCnt = colToApply.c4ElemCount();

    //
    //  To avoid a combinatorial sort of overhead, we create a hash list of the attributes
    //  we are going to apply, sorted by id. Then we can quickly check to see if we have
    //  a given attribute in the list.
    //
    TRefKeyedHashSet<const TAttrData, TString, TStringKeyOps> colApply
    (
        tCIDLib::EAdoptOpts::NoAdopt
        , 109
        , TStringKeyOps()
        , &TAttrData::strId
    );

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4AvailCnt; c4Index++)
        colApply.Add(&colToApply[c4Index]);


    // Push a full undo for the selected widgets. Push a separator
    m_uhEditor.Push(new TIntfFullUndo(iwdgEdit()), kCIDLib::True);

    // We may need a data server client for this
    TDataSrvClient dsclLoad;

    //
    //  And go through all the selected widgets. For each one we get the attributes that
    //  the widget supports. We then go through each attribute and see if it is in the
    //  passed list. If so, we set it.
    //
    tCIDMData::TAttrList colCurAttrs;
    TAttrData attrTmp;
    const tCIDLib::TCard4 c4SelCount = m_selList.c4Count();
    TGUIRegion grgnRedraw;
    for (tCIDLib::TCard4 c4SelIndex = 0; c4SelIndex < c4SelCount; c4SelIndex++)
    {
        TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4SelIndex));

        tCIDLib::TCard4 c4ApplyCnt = 0;
        colCurAttrs.RemoveAll();
        piwdgCur->QueryWdgAttrs(colCurAttrs, attrTmp);
        MCQCIntfImgListIntf* pmwdgTmp;
        MCQCCmdSrcIntf* pmcsrcTmp;
        const tCIDLib::TCard4 c4CurAttrCnt = colCurAttrs.c4ElemCount();
        for (tCIDLib::TCard4 c4CurAttrInd = 0; c4CurAttrInd < c4CurAttrCnt; c4CurAttrInd++)
        {
            const TAttrData& attrCur = colCurAttrs[c4CurAttrInd];

            // Skip separators
            if (attrCur.eType() == tCIDMData::EAttrTypes::Separator)
                continue;

            //
            //  See if this one is available in the list to apply. If so then let's pass
            //  it along to the widget.
            //
            const TAttrData* padatApply = colApply.pobjFindByKey(attrCur.strId(), kCIDLib::False);
            if (padatApply)
            {
                //
                //  Watch for some special case ones. Some can't just be applied as is.
                //  They are the same attribute but the contents is specific to the widget
                //  type.
                //
                if (padatApply->strId() == kCQCIntfEng::strAttr_Images)
                {
                    //
                    //  We don't want to take the image list, we want to find any images
                    //  with the same key and take those. So stream the image list in,
                    //  using a dummy class that just implements the image list mixin,
                    //  and call a method that will take the matching keys.
                    //
                    TIEImgListWrap iewTmp;
                    TBinMBufInStream strmSrc(&padatApply->mbufVal(), padatApply->c4Bytes());
                    iewTmp.ReadInImgList(strmSrc);

                    // We have to cast it to the image list interface
                    pmwdgTmp= dynamic_cast<MCQCIntfImgListIntf*>(piwdgCur);
                    if (pmwdgTmp)
                    {
                        if (pmwdgTmp->bTakeMatchingImgKeys(iewTmp))
                        {
                            c4ApplyCnt++;
                            pmwdgTmp->UpdateAllImgs(*m_pcivEdit, dsclLoad);
                        }
                    }
                }
                 else if (padatApply->strId() == kCQCKit::strAttr_Actions)
                {
                    // Stream in the attribute data
                    TBasicCmdSrc csrcEdit;
                    TBinMBufInStream strmSrc(&padatApply->mbufVal(), padatApply->c4Bytes());
                    strmSrc >> csrcEdit;

                    // We have to cast it to the command source
                    pmcsrcTmp = dynamic_cast<MCQCCmdSrcIntf*>(piwdgCur);
                    if (pmcsrcTmp)
                    {
                        if (pmcsrcTmp->bTakeMatchingEvents(csrcEdit))
                            c4ApplyCnt++;
                    }
                }
                 else
                {
                    piwdgCur->SetWdgAttr(*m_pwndAttrEd, *padatApply, attrCur);
                    c4ApplyCnt++;
                }
            }
        }

        if (c4ApplyCnt)
        {
            if (grgnRedraw.bIsEmpty())
                grgnRedraw.Set(piwdgCur->areaActual());
            else
                grgnRedraw.CombineWith(piwdgCur->areaActual(), tCIDGraphDev::EClipModes::Or);
        }
    }

    // Redraw the region that contains the affected widgets
    m_pcivEdit->Redraw(grgnRedraw);
}


//
//  If there's interface widget clipboard content, then we need to paste those in. They
//  go at the end, so there's no real z-order issues, but we have to deal with group ids.
//
//  It's possible that the group ids of any grouped widgets are ok, but we don't want
//  to worry about it. We just go through and, for each group of widgets, we update them
//  to use the next available group id in this template. They could have been pasted
//  from another template.
//
//  Similarly we have to update the design time ids. We just assume the worst and update
//  all of the design time ids to references our template id and re-numbering them with
//  this template's next available design time ids.
//
tCIDLib::TVoid TIntfEditWnd::PasteWidgets(const TPoint& pntAt)
{
    TGraphWndDev gdevToUse(*this);
    TCQCIntfTemplate& iwdgTmpl = iwdgEdit();
    const TArea& areaTmpl = iwdgTmpl.areaRelative();

    // Try to read a designer clipboard object off the clipboard
    TCQCIntfEdWdgCB* pwcbTmp = nullptr;
    try
    {
        TGUIClipboard gclipPaste(*this);
        if (!gclipPaste.bReadObj<TCQCIntfEdWdgCB>(pwcbTmp))
        {
            // Clear the clipboard since something isn't right, and return
            gclipPaste.Clear();
            return;
        }
    }

    catch(const TError& errToCatch)
    {
        if (facCQCIntfEd().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , facCQCIntfEd().strMsg(kIEdMsgs::midWnd_AppName)
            , errToCatch.strErrText()
            , errToCatch
        );
        return;
    }

    // Make sure it get's cleaned up
    TJanitor<TCQCIntfEdWdgCB> janTmp(pwcbTmp);

    // If no widgets are in the object, then we are done
    const tCIDLib::TCard4 c4Count = pwcbTmp->c4ElemCount();
    if (!c4Count)
        return;

    //
    //  Figure out the full size of the widgets. They were moved relative to their common
    //  origin when put on the clipboard. So we just accumulate their areas.
    //
    TArea areaHull;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        areaHull |= pwcbTmp->piwdgAt(c4Index)->areaRelative();

    //
    //  Now adjust this by the click point. This will be the area where the widgets will
    //  be going back. If this is bigger than the template area in either direction, tell
    //  the user we can't paste them.
    //
    if ((areaHull.c4Width() > areaTmpl.c4Width())
    ||  (areaHull.c4Height() > areaTmpl.c4Height()))
    {
        TYesNoBox msgbTooBig
        (
            L"The widgets cannot fit within the template's area without modifying their "
            L"relative positions or sizes. Do you want to allow that?"
        );

        if (!msgbTooBig.bShowIt(*this))
            return;

        //
        //  Let's adjust them. We move the hull area back to the origin in the problem
        //  direction(s). Below we will use this as the origin, and will adjust those that
        //  don't fit.
        //
        if (areaHull.c4Width() > areaTmpl.c4Width())
            areaHull.i4X(0);
        else
            areaHull.i4X(pntAt.i4X());

        if (areaHull.c4Height() > areaTmpl.c4Height())
            areaHull.i4Y(0);
        else
            areaHull.i4Y(pntAt.i4Y());

    }
     else
    {
        //
        //  Force the hull back within the template area. Tell it not to adjust the size,
        //  since we checked above it's OK.
        //
        areaHull.SetOrg(pntAt);
        areaHull.ForceWithin(areaTmpl, kCIDLib::False);
    }

    // We have to initialize the widgets, which requires a data server client
    TDataSrvClient dsclToUse;

    //
    //  We have to deal with the design time ids. They will have lost any ids
    //  they had originally since it's not streamed and the had to be flattened
    //  in order to be put on the clip board.
    //
    const tCIDLib::TCard4 c4OurTmplId = TRawBits::c4High32From64(iwdgTmpl.c8DesId());
    tCQCIntfEng::TDesIdList fcolIds(c4Count);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = pwcbTmp->piwdgAt(c4Index);
        fcolIds.c4AddElement
        (
            piwdgCur->c8DesId(TRawBits::c8From32(m_c4NextWId++, c4OurTmplId))
        );
    }

    //
    //  Do the group ids. We know that all grouped widgets are adjacent, so we
    //  just watch for a group id. If we get one, we assign the next available
    //  new id until we hit a new group or ungrouped.
    //
    {
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            TCQCIntfWidget* piwdgCur = pwcbTmp->piwdgAt(c4Index);
            if (piwdgCur->c4GroupId())
            {
                const tCIDLib::TCard4 c4OldGrpId = piwdgCur->c4GroupId();
                const tCIDLib::TCard4 c4NewGrpId = iwdgTmpl.c4GetNextGroupId();

                while (piwdgCur->c4GroupId() == c4OldGrpId)
                {
                    piwdgCur->c4GroupId(c4NewGrpId);

                    // Bump the index and break out if we hit the count available
                    c4Index++;
                    if (c4Index >= c4Count)
                        break;

                    // Else move forward
                    piwdgCur = pwcbTmp->piwdgAt(c4Index);
                }
            }
             else
            {
                c4Index++;
            }
        }
    }

    //
    //  Push an insert undo. It just needs the list of ids so that it can go back and
    //  find and remove them later if needed.
    //
    m_uhEditor.Push(new TIntfInsertUndo(fcolIds));

    //
    //  We want these new guys to be selected after a paste, so clean out
    //  the selection list and add each new one to the list as we add them
    //  successfully to the template. We don't need to redraw here since we
    //  will do it at the end, so just empty the selection list.
    //
    m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));
    RemoveSelection();

    //
    //  Loop through the widgets in the list and add of each of them to the template.
    //
    //  Note that we maintain their relative positions, so we set them relative to the
    //  click position, using their previous position relative to the origin of the
    //  rectangle that enclosed them all, but we force them back within the template
    //  area if this would push them out.
    //
    tCQCIntfEng::TErrList colErrs;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        //
        //  The objects were copied onto the clipboard, so that we don't have
        //  to make copies and can just put them directly into the template
        //  by orphaning them from the designer clipboard object.
        //
        TCQCIntfWidget* piwdgNew = pwcbTmp->piwdgOrphanNext();
        TJanitor<TCQCIntfWidget> janCur(piwdgNew);

        // Process any states
        piwdgNew->LookupStates(&iwdgTmpl);

        //
        //  We have to deal with widget ids here. If this guy has an id, then
        //  see if that id is used already. If so, then start appending 1, 2,
        //  etc... onto it until we get a unique id.
        //
        if (!piwdgNew->strWidgetId().bIsEmpty())
        {
            //
            //  Note that we don't do a recursive search, since there are
            //  no loaded child overlays during design time. At runtime,
            //  only dups at the same level are a problem anyway.
            //
            TCQCIntfWidget* piwdgDup = m_pcivEdit->piwdgFindByName
            (
                piwdgNew->strWidgetId(), kCIDLib::False
            );

            if (piwdgDup)
            {
                TString strNew(piwdgNew->strWidgetId());
                const tCIDLib::TCard4 c4OrgLen = strNew.c4Length();
                tCIDLib::TCard4 c4Suffix = 2;

                while (piwdgDup)
                {
                    strNew.CapAt(c4OrgLen);
                    strNew.AppendFormatted(c4Suffix);
                    piwdgDup = m_pcivEdit->piwdgFindByName(strNew, kCIDLib::False);

                    //
                    //  Just as a sanity check, if we do 1024, just empty the
                    //  id out and be done with it.
                    //
                    if (c4Suffix > 1024)
                    {
                        if (facCQCIntfEd().bLogFailures())
                        {
                            facCQCIntfEd().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kIEdErrs::errcGen_MakePasteId
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::Internal
                                , piwdgNew->strWidgetId()
                            );
                        }
                        strNew.Clear();
                        break;
                    }
                    c4Suffix++;
                }
                piwdgNew->strWidgetId(strNew);
            }
        }

        //
        //  Tell it we are its owning view
        //
        //  DO THIS before we resize it below, since it may try to do things that
        //  require access to the view when that happens.
        //
        piwdgNew->SetOwner(m_pcivEdit);

        //
        //  Adjust the new widget's area by the hull origin, which we set up above. If this
        //  would make it too big, then first try to adjust its origin, then adjust its
        //  size if needed.
        //
        TArea areaNew(piwdgNew->areaActual());
        areaNew.AdjustOrg(areaHull.pntOrg());

        // If it doesn't fit the template as is, adjust
        if (!areaTmpl.bContainsArea(areaNew))
        {
            if (areaNew.i4Right() > areaTmpl.i4Right())
            {
                if (areaNew.c4Width() > areaTmpl.c4Width())
                {
                    areaNew.i4X(0);
                    areaNew.c4Width(areaTmpl.c4Width());
                }
                 else
                {
                    areaNew.i4X(areaTmpl.i4Right() - tCIDLib::TInt4(areaNew.c4Width()));
                }
            }

            if (areaNew.i4Bottom() > areaTmpl.i4Bottom())
            {
                if (areaNew.c4Height() > areaTmpl.c4Height())
                {
                    areaNew.i4Y(0);
                    areaNew.c4Height(areaTmpl.c4Height());
                }
                 else
                {
                    areaNew.i4Y(areaTmpl.i4Bottom() - tCIDLib::TInt4(areaNew.c4Height()));
                }
            }
        }

        // Now take this (possibly adjusted) area
        piwdgNew->SetArea(areaNew);

        //
        //  This is a flattened widget that we copied, so we have to
        //  to initialize it, so that it can load images and such
        //  that it is configured for.
        //
        colErrs.RemoveAll();
        piwdgNew->Initialize(&iwdgTmpl, dsclToUse, colErrs);

        //
        //  And now add it to ourself as a child, letting it go from
        //  the janitor.
        //
        iwdgTmpl.AddChild(janCur.pobjOrphan());

        // And add it to the selection list
        AddToSelection(piwdgNew, kCIDLib::False, kCIDLib::False);
    }

    // Do a validation pass
    Validate();

    // Redraw to update everything
    m_pcivEdit->Redraw();
}


//
//  Helpers to support publishing msgs when the mouse moves from one widget to another. We
//  have a main one that takes the widget hit (or null if nothing under the mouse), one
//  that takes a point and does a hit test and calls the first, and one that takes a point
//  and calls the hit testing version.
//
tCIDLib::TVoid TIntfEditWnd::PublishMouseOverMsg(const TCQCIntfWidget* const piwdgUnder)
{
    if (piwdgUnder)
    {
        // If we are in a new one, then publish and remember the new one
        if (m_c8LastMouseOver != piwdgUnder->c8DesId())
        {
            m_c8LastMouseOver = piwdgUnder->c8DesId();
            m_pstopUpdates.Publish(new TIntfEdUpdateTopic(piwdgUnder));
        }
    }
     else
    {
        // Not over one now
        m_c8LastMouseOver = 0;
        m_pstopUpdates.Publish(new TIntfEdUpdateTopic(nullptr));
    }
}

tCIDLib::TVoid TIntfEditWnd::PublishMouseOverMsg(const TPoint& pntAt)
{
    // See if this hits a widget
    tCIDLib::TCard8 c8HitId;
    const TCQCIntfWidget* piwdgHit = piwdgHitTest(pntAt, c8HitId);
    PublishMouseOverMsg(piwdgHit);
}

tCIDLib::TVoid TIntfEditWnd::PublishMouseOverMsg()
{
    // Get the mouse position and call the other version after adjusting for our window
    PublishMouseOverMsg(facCIDCtrls().pntPointerPos(*this));
}


// We tell the template to update any image references
tCIDLib::TVoid TIntfEditWnd::RefreshImages()
{
    try
    {
        // Refresh the image cache. If it reports any changes, then tell the template to update
        TGraphWndDev gdevCompat(*this);
        if (facCQCIntfEng().bRefreshImgCache(gdevCompat, m_pcivEdit->cuctxToUse().sectUser()))
        {
            TDataSrvClient dsclToUse;
            m_iwdgOrg.RefreshImages(dsclToUse);

            // And force it to redraw
            m_pcivEdit->ForceRedraw();
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        facCQCGKit().ShowError
        (
            *this
            , L"Image Refresh Error"
            , errToCatch.strErrText()
            , errToCatch
        );
    }
}


//
//  If there are any selected items, clear the list and redraw the region that
//  contains them all, to remove the visible selection handles. The caller tells us
//  if we should clear the attribute editor as well, or not. And he tells us whether to
//  to push an undo or not. He may have already pushed it in the context of what is
//  being done.
//
tCIDLib::TVoid
TIntfEditWnd::RemoveSelection(const tCIDLib::TBoolean bClearAttrEd)
{
    //
    //  Create a region that includes all of the selection handles of all of
    //  the selected widgets.
    //
    TGUIRegion grgnRedraw;
    if (bBuildSelHandleRgn(grgnRedraw))
    {
        // Push a selection undo
        m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));

        //
        //  Remove all of the items from the selection list, then redraw. We
        //  have to clear the list first, else they'd get redrawn with
        //  selection marks.
        //
        m_selList.Clear();

        // And redraw the affected parts
        Redraw(grgnRedraw, tCIDCtrls::ERedrawFlags::ImmedChildren);
    }

    //
    //  Update the attribute editor since we affected the selection. But only
    //  if asked to. The caller may know he's going to update it again and we
    //  want to avoid multiple updates.
    //
    if (bClearAttrEd)
        UpdateAttrEd();
}


//
//  This method will go through the selected widgets and change their size to the passed
//  size, but clip any back that would go outside of the available template area.
//
tCIDLib::TVoid TIntfEditWnd::ResizeSelected(const TSize& szNew)
{
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count)
        return;

    //  Push a move undo for the selected widgets
    m_uhEditor.Push(new TIntfMultiPosUndo(iwdgEdit(), m_selList.fcolIds()));

    TGUIRegion grgnUpdate;
    TArea areaOld;
    TArea areaNew;
    const TArea& areaTmpl = iwdgEdit().areaActual();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = iwdgEdit().piwdgFindByDesId(m_selList.c8At(c4Index));
        areaOld = piwdgCur->areaActual();

        // Set up the new size
        areaNew = areaOld;
        areaNew.SetSize(szNew);

        //
        //  If this would take it off the template, clip it back, if it's not
        //  the template itself.
        //
        if (piwdgCur != &iwdgEdit())
        {
            if (areaNew.i4Right() > areaTmpl.i4Right())
                areaNew.i4Right(areaTmpl.i4Right());
            if (areaNew.i4Bottom() > areaTmpl.i4Bottom())
                areaNew.i4Bottom(areaTmpl.i4Bottom());
        }

        //
        //  Add the old and new area to the update region. If one of them
        //  is fully within the other (common), then it won't hurt anything
        //  since it just won't add anything new to the region.
        //
        if (c4Index)
            grgnUpdate.CombineWith(areaNew, tCIDGraphDev::EClipModes::Or);
        else
            grgnUpdate.Set(areaNew);
        grgnUpdate.CombineWith(areaOld, tCIDGraphDev::EClipModes::Or);

        piwdgCur->SetArea(areaNew);
        piwdgCur->SizeChanged(areaNew.szArea(), areaOld.szArea());
    }

    // Redraw the affected parts
    Redraw(grgnUpdate, tCIDCtrls::ERedrawFlags::Full);
}


//
//  We call the public bSave command and display any error returned.
//
tCIDLib::TVoid TIntfEditWnd::Save()
{
    TString strErr;
    if (!bSaveChanges(strErr))
    {
        TErrBox msgbErr
        (
            facCQCIntfEd().strMsg(kIEdMsgs::midWnd_AppName), strErr
        );
        msgbErr.ShowIt(*this);
    }
}


//
//  When the user asks to directly set widget sizes, this is called from the
//  contex menu handler . We have to deal with a few things. We have to
//  get in current size info to initialize the dialog with, and we have to
//  set min/max values.
//
//  1.  If the hit widget is the template itself, then we pass in the template's
//      current size, and set the min to something that wouldn't clip any
//      widgets (or 32 whichever is larger), and a max of 4K.
//
//  2.  Else we have to possibly deal with more than one widget since we operate
//      the selected widgets. So we get the widget with the greatest origin,
//      and use that to set the max (so that it wouldn't push that widget off
//      the template, and set a default minimize.
//
tCIDLib::TVoid TIntfEditWnd::SetWidgetSizes()
{
    //
    //  We just let them enter the new size via a little dialog. If
    //  they commit, we get back a TSize and set it on the template.
    //
    const TSize szTmpl = iwdgEdit().areaActual().szArea();
    TSize szInit;
    TSize szMin;
    TSize szMax;

    //
    //  See if the temlate is the seleced object. It can only ever be
    //  selected by itself.
    //
    TCQCIntfWidget* piwdgFirst = iwdgEdit().piwdgFindByDesId(m_selList.c8At(0));
    const tCIDLib::TBoolean bTmplSel = (piwdgFirst == &iwdgEdit());

    if (bTmplSel)
    {
        TArea areaCur = iwdgEdit().areaActual();
        szInit = areaCur.szArea();
        szMax.Set(4096, 4096);
        szMin.Set(32, 32);
        szMin.TakeLarger(iwdgEdit().szMaxChildExtent());
    }
     else
    {
        szInit = piwdgFirst->areaActual().szArea();
        szMin.Set(kCQCIntfEd_::c4MinSize, kCQCIntfEd_::c4MinSize);
        szMax = szTmpl;

        // Make sure the init isn't bigger than the max
        szInit.TakeSmaller(szMax);
    }

    TSize szNew;
    if (facCIDWUtils().bGetSize(*this
                                , facCQCIntfEd().strMsg(kIEdMsgs::midTitle_GetWidgetSize)
                                , szNew
                                , szInit
                                , szMin
                                , szMax
                                , kCIDLib::False))
    {
        // Resize the selected widgets, clipping back as required
        ResizeSelected(szNew);
    }
}



//
//  Called after a lasso select, to select any widget that either intersect the
//  lasso area or that are fully contained within it. If the Alt key is pressed,
//  we do the fully contained scenario, else just intersects.
//
//  They can ask that we clear the list first, or just add them to the existing
//  selection list. We only clear if we actually hit any widgets.
//
//  We also have to add any group members as well, of any widgets we hit.
//
tCIDLib::TVoid
TIntfEditWnd::SelectContained(  const   TArea&              areaLasso
                                , const tCIDLib::TBoolean   bClearFirst)
{
    // See if we hit any widgets
    tCQCIntfEng::TChildList colHits(tCIDLib::EAdoptOpts::NoAdopt);

    // See if we are doing interects or contains
    tCIDLib::TBoolean bDoContains = facCIDCtrls().bAltShifted();

    const tCIDLib::TCard4 c4ChildCount = iwdgEdit().c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        TCQCIntfWidget& iwdgCur = iwdgEdit().iwdgAt(c4Index);
        if (bDoContains)
        {
            if (areaLasso.bContainsArea(iwdgCur.areaActual()))
                colHits.Add(&iwdgCur);
        }
         else
        {
            if (iwdgCur.areaActual().bIntersects(areaLasso))
                colHits.Add(&iwdgCur);
        }
    }

    if (!colHits.bIsEmpty())
    {
        // Push a selection undo
        m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));

        if (bClearFirst)
            RemoveSelection();

        // And now select the ones we hit, ignoring those locked
        tCQCIntfEng::TChildList colGrp(tCIDLib::EAdoptOpts::NoAdopt);
        const tCIDLib::TCard4 c4HitCount = colHits.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4HitCount; c4Index++)
        {
            TCQCIntfWidget* piwdgCur = colHits[c4Index];

            //
            //  If this one is locked, then if it's in a group that's locked
            //  as a whole, so either way we ignore it.
            //
            if (piwdgCur->bIsLocked())
                continue;

            // Add it to the selection list (and all all group members if a group)
            AddToSelection(piwdgCur, kCIDLib::False, kCIDLib::False);
        }
    }
}


//
//  This will go through all of the selected widgets and, for any that report they
//  support sizing to their contents, ask them to do so.
//
tCIDLib::TVoid TIntfEditWnd::SizeSelectedToContents()
{
    const tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count)
        return;

    // Push a multi-pos undo for the selected widgets
    m_uhEditor.Push(new TIntfMultiPosUndo(iwdgEdit(), m_selList.fcolIds()));

    // Get the template size. We'll force them to stay within this
    const TArea& areaLim = iwdgEdit().areaActual();

    tCIDLib::TBoolean bDoneOne = kCIDLib::False;
    TGUIRegion grgnUpdate;
    TArea areaNew;
    TSize szOld;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));

        // If this guy doesn't support this, then skip it
        if (!piwdgCur->bCanSizeTo())
            continue;

        // Ask the widget what size it would prefer
        TSize szNew;
        piwdgCur->QueryContentSize(szNew);

        // If we got back an empty size, then do nothing
        if (szNew.bAllZero())
            continue;

        //
        //  Add the old and new area to the update region. If one of them
        //  is fully within the other (common), then it won't hurt anything
        //  since it just won't add anything new to the region.
        //
        areaNew = piwdgCur->areaActual();
        if (!bDoneOne)
            grgnUpdate.Set(areaNew);
        else
            grgnUpdate.CombineWith(areaNew, tCIDGraphDev::EClipModes::Or);
        szOld = areaNew.szArea();
        areaNew.SetSize(szNew);
        areaNew.ForceWithin(areaLim, kCIDLib::True);
        grgnUpdate.CombineWith(areaNew, tCIDGraphDev::EClipModes::Or);

        piwdgCur->SetArea(areaNew);
        piwdgCur->SizeChanged(areaNew.szArea(), szOld);

        // Remember we changed at least one
        bDoneOne = kCIDLib::True;
    }

    //
    //  If any changes, then refresh the update region, else trash the undo we pushed
    //  above.
    //
    if (bDoneOne)
        Redraw(grgnUpdate, tCIDCtrls::ERedrawFlags::ImmedChildren);
    else
        m_uhEditor.DiscardTop();
}


//
//  Called when they click and drag on something already selected, and it's not
//  an edge/corner move scenario. So we put ourselves inot move drag mode. We
//  get the starting mouse position.
//
tCIDLib::TVoid TIntfEditWnd::StartMove(const TPoint& pntStart)
{
    //
    //  Shouldn't happen, but just in case the selection is empty or contains
    //  the template.
    //
    CIDAssert
    (
        !m_selList.bIsEmpty() && (m_selList.c8At(0) != iwdgEdit().c8DesId())
        , L"Cannot move an empty selection or template"
    );

    TArea areaUs = areaWndSize();

    // Create a bitmap the size of our window
    TClrPalette palDummy;
    m_bmpDrag = TBitmap
    (
        areaUs.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
        , palDummy
        , kCIDLib::True
    );

    //
    //  Create a memory device over it and copy our current window contents. Make
    //  sure it gets dropped immediatley because we'll force an immediate redraw
    //  below to get the initial outline displayed. It will try to create a device
    //  over this bitmap as well, and it would fail if this device still holds it.
    //
    TGraphWndDev gdevUs(*this);
    {
        TGraphMemDev gdevTrack(gdevUs, m_bmpDrag);
        gdevTrack.CopyBits(gdevUs, areaUs, areaUs);
    }

    //
    //  Get the bounding area of all of the selected widgets. That becomes the
    //  initial drag area.
    //
    m_areaDrag = areaSelection();

    //
    //  We have to figure out the offset of the point relative to the origin of
    //  the bounding box of the selection.
    //
    m_pntDragOfs = pntStart - m_areaDrag.pntOrg();

    //
    //  Figure out the allowable movement. It would be whatever would prevent
    //  the bounding area from going outside of the template area. For the
    //  origin, it's how far the bounding area is from the template origin,
    //  minus how far we are within the bounding area.
    //
    const TArea areaCont = iwdgEdit().areaActual();
    TArea areaConfine
    (
        TPoint
        (
            pntStart.i4X() - m_areaDrag.i4X(), pntStart.i4Y() - m_areaDrag.i4Y()
        )
        , TPoint
          (
            pntStart.i4X() + (areaCont.i4Right() - m_areaDrag.i4Right())
            , pntStart.i4Y() + (areaCont.i4Bottom() - m_areaDrag.i4Bottom())
          )
    );

    // Convert it to screen coordinates
    TArea areaScreenCoord;
    ToScreenCoordinates(areaConfine, areaScreenCoord);
    facCIDCtrls().ConfinePointer(areaScreenCoord);

    // Capture the mouse
    CaptureMouse();

    // It all worked, so set the mode and redraw to get the outline displayed
    m_eDragMode = EDragModes::Move;
    m_pcivEdit->Redraw(m_areaDrag);
}


//
//  Called when they hit an edge or corner of a single selected widget (or the
//  template itself) to start the sizing operation.
//
//  For the template of course we will only ever get called here for bottom/right
//  stuff. For widgets it can be any corner or edge. So we limit in the direction
//  of movement to the edge of the template.
//
//  Because of the limitations of composited windows, we have to draw the selection
//  slash size rectangle during the paint. So we have to create a bitmap, capture our
//  current contents, and during the paint we blit it back to the screen with the
//  rectangle.
//
tCIDLib::TVoid
TIntfEditWnd::StartSize(const   TPoint&                 pntStart
                        , const TCQCIntfWidget&         iwdgTarget
                        , const tCIDCtrls::EWndAreas    eArea)
{
    TArea areaBmp = areaWndSize();

    // Create a bitmap the size of our window
    TClrPalette palDummy;
    m_bmpDrag = TBitmap
    (
        areaBmp.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
        , palDummy
        , kCIDLib::True
    );

    //
    //  Create a memory device over it and copy our current window contents. Make
    //  sure it gets dropped immediateley because we'll force an immediate redraw
    //  below and it will try to create a device over this bitmap as well, and it
    //  would fail if this device still holds it.
    //
    TGraphWndDev gdevUs(*this);
    {
        TGraphMemDev gdevTrack(gdevUs, m_bmpDrag);
        gdevTrack.CopyBits(gdevUs, areaBmp, areaBmp);
    }

    //
    //  We need to limit the area of movement correctly based on what corner
    //  or edge was hit. We start off by setting a container area, then we
    //  can generically set the confinement based on the area.
    //
    const TCQCIntfTemplate& iwdgTmpl = iwdgEdit();
    const tCIDLib::TBoolean bIsTmpl(iwdgTarget.c8DesId() == iwdgTmpl.c8DesId());
    const TArea areaTarget = iwdgTarget.areaActual();
    TArea areaCont = areaBmp;


    //
    //  Start the confinement at the widget target area if a widget, or the bitmap
    //  area if the template.
    //
    TArea areaConfine = bIsTmpl ? areaCont : iwdgTmpl.areaActual();

    // First do the horizontal
    switch(eArea)
    {
        case tCIDCtrls::EWndAreas::Left :
        case tCIDCtrls::EWndAreas::BottomLeft :
        case tCIDCtrls::EWndAreas::TopLeft :
            // Limit is widget right
            areaConfine.i4Right(areaTarget.i4Right());
            break;

        case tCIDCtrls::EWndAreas::Right :
        case tCIDCtrls::EWndAreas::BottomRight :
        case tCIDCtrls::EWndAreas::TopRight :
            // Limit is widget left, make sure the right stays locked
            areaConfine.i4X(areaTarget.i4Left(), kCIDLib::True);
            break;

        default :
            // It's a vertical operation, so left/right limits are widget's
            areaConfine.i4X(areaTarget.i4X());
            areaConfine.i4Right(areaTarget.i4Right());
            break;
    };

    // And the vertical
    switch(eArea)
    {
        case tCIDCtrls::EWndAreas::Top :
        case tCIDCtrls::EWndAreas::TopLeft :
        case tCIDCtrls::EWndAreas::TopRight :
            // Limit is widget bottom
            areaConfine.i4Bottom(areaTarget.i4Bottom());
            break;

        case tCIDCtrls::EWndAreas::Bottom :
        case tCIDCtrls::EWndAreas::BottomLeft :
        case tCIDCtrls::EWndAreas::BottomRight :
            // Limit is widget top, make sure the bottom stays locked
            areaConfine.i4Y(areaTarget.i4Y(), kCIDLib::True);
            break;

        default :
            // It's a horz operation, so top/bottom limits are widget's
            areaConfine.i4Y(areaTarget.i4Y());
            areaConfine.i4Bottom(areaTarget.i4Bottom());
            break;
    };

    // Convert it to screen coordinates
    TArea areaScreenCoord;
    ToScreenCoordinates(areaConfine, areaScreenCoord);
    facCIDCtrls().ConfinePointer(areaScreenCoord);

    // Capture the mouse
    CaptureMouse();

    // Set the initial drag area, which initially is the widget's area
    m_areaDrag = iwdgTarget.areaActual();

    //
    //  Remember the area and store our mode now that it's looking like it
    //  won't fail.
    //
    m_eDragArea = eArea;
    m_eDragMode = EDragModes::Size;

    //
    //  In this case, unlike a select, the area isn't initially empty, so we
    //  need to do an initial draw of the rectangle.
    //
    m_pcivEdit->Redraw(areaTarget);
}


//
//  Puts us into marquee select mode. We can't use the normal helper from
//  CIDCtrls because we are likely to be in a composited top level window.
//  So we have to do it ourself and draw it in the paint.
//
tCIDLib::TVoid TIntfEditWnd::StartMarqueeSel(const TPoint& pntStart)
{
    TArea areaUs = areaWndSize();

    // Create a bitmap the size of our window
    TClrPalette palDummy;
    m_bmpDrag = TBitmap
    (
        areaUs.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
        , palDummy
        , kCIDLib::True
    );

    //
    //  Create a memory device over it and copy our current window contents.
    //  Though we don't force an immediate redraw (initial area is empty so no
    //  need), make sure just in case that we don't let the memory device stick
    //  around. If we should later force a redraw, it would attempt to put
    //  another device around the bitmap and that would fail.
    //
    TGraphWndDev gdevUs(*this);
    {
        TGraphMemDev gdevTrack(gdevUs, m_bmpDrag);
        gdevTrack.CopyBits(gdevUs, areaUs, areaUs);
    }

    // Limit movement to our template area
    TArea areaConfine(areaUs);
    TArea areaScreenCoord;
    ToScreenCoordinates(areaConfine, areaScreenCoord);
    facCIDCtrls().ConfinePointer(areaScreenCoord);

    // Capture the mouse
    CaptureMouse();

    // Set the initial drag area, which initially is empty
    m_areaDrag.Set(pntStart, TSize(0, 0));

    //
    //  Remember the start point which is where we have to invert as they move
    //  before or after it.
    //
    m_pntDragOfs = pntStart;

    // Looks like it worked, so set the mode flag
    m_eDragMode = EDragModes::Select;
}


tCIDLib::TVoid TIntfEditWnd::ToggleTextJustification(const tCIDLib::TBoolean bHorz)
{
    // If none selected, then nothing to do
    tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count)
        return;

    //
    //  Go through though them and see if any implement the font mixin. If not, then
    //  there's nothing to do. To be efficient, remember them as we go and just go
    //  through those at the end, via the font mixin. Our list must be non-adopting
    //  of course!
    //
    TRefVector<TCQCIntfWidget> colKeepers(tCIDLib::EAdoptOpts::NoAdopt);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(c4Index));
        MCQCIntfFontIntf* pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(piwdgCur);
        if (pmiwdgFont)
            colKeepers.Add(piwdgCur);
    }

    // If we didn't find any, then we are done
    c4Count = colKeepers.c4ElemCount();
    if (!c4Count)
        return;

    // Push a full undo with separator
    m_uhEditor.Push(new TIntfFullUndo(iwdgEdit()), kCIDLib::True);

    // For each keeper, toggle the Horz/Vert justification as required
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        MCQCIntfFontIntf* pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(colKeepers[c4Index]);
        if (bHorz)
        {
            tCIDLib::EHJustify eJust = pmiwdgFont->eHJustify();
            if (eJust == tCIDLib::EHJustify::Max)
                eJust = tCIDLib::EHJustify::Min;
            else
                eJust++;
            pmiwdgFont->eHJustify(eJust);
        }
         else
        {
            tCIDLib::EVJustify eJust = pmiwdgFont->eVJustify();
            if (eJust == tCIDLib::EVJustify::Max)
                eJust = tCIDLib::EVJustify::Min;
            else
                eJust++;
            pmiwdgFont->eVJustify(eJust);
        }
    }

    // Build up a region that includes the affected widgets and invalidate it
    TGUIRegion grgnRedraw(colKeepers[0]->areaActual());
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
        grgnRedraw.CombineWith(colKeepers[c4Index]->areaActual(), tCIDGraphDev::EClipModes::Or);
    m_pcivEdit->Redraw(grgnRedraw);
}


//
//  If the undo stack isn't empty, we pull items off the top of the stack until we hit
//  a separator or its empty. For the non-separator ones, we invoke their Undo() method.
//  Once we hit a separator, we look to see if there are any more separators after it.
//  This can happen if they pushed a separator but ended up not actually pushing any undo
//  operations.
//
//  We have to deal with the special case of their being a separate on top of the stack,
//  where someone pushed a separator but never pushed any undos. This can happen easily
//  enough. There can never be more than one in a row since dups are ignored during the
//  push.
//
tCIDLib::TVoid TIntfEditWnd::Undo()
{
    // Makes the logic below simpler
    if (m_uhEditor.bIsEmpty())
        return;

    // If the top of stack is a separator, ignore it
    if (m_uhEditor.pucTop()->bIsA(TUndoCmdSep::clsThis()))
        m_uhEditor.DiscardTop();

    tCQCIntfEng::TDesIdList fcolOrgSel(m_selList.fcolIds());
    while (!m_uhEditor.bIsEmpty())
    {
        // Get the top of the undo stack and invoke it
        TBaseIntfUndo* pucTop = static_cast<TBaseIntfUndo*>(m_uhEditor.pucPop());
        TJanitor<TBaseIntfUndo> janItem(pucTop);

        // If it's a separator, then we are done
        if (pucTop->bIsA(TUndoCmdSep::clsThis()))
            break;

        // Invoke this one's undo
        pucTop->Undo(*m_pwndAttrEd, iwdgEdit());

        // If it's a selection undo, then restore the selection list
        if (pucTop->bIsA(TIntfSelectUndo::clsThis()))
        {
            TIntfSelectUndo* pucSel(static_cast<TIntfSelectUndo*>(pucTop));

            // <TBD> Remember manual initial selection in the undo and put it back
            m_selList.SetList(pucSel->fcolSelected(), kCIDLib::False);
        }
    }

    // Get rid of any more adjacent separators
    while (!m_uhEditor.bIsEmpty())
    {
        TBaseIntfUndo* pucTop = static_cast<TBaseIntfUndo*>(m_uhEditor.pucTop());
        if (!pucTop->bIsA(TUndoCmdSep::clsThis()))
            break;
        m_uhEditor.DiscardTop();
    }

    // Force a redraw
    ForceRepaint();

    //
    //  If a single widget is selected, update the attribute editor just in case. If
    //  it's the same one as before it'll just do an update instead of a full reload.
    //
    //  If we removed any selection, then clear the attribute editor
    //
    const tCIDLib::TCard4 c4SelCount = m_selList.c4Count();
    if (c4SelCount == 0)
    {
        UpdateAttrEd();
    }
     else if (c4SelCount == 1)
    {
        TCQCIntfWidget* piwdgSel = iwdgEdit().piwdgFindByDesId(m_selList.c8At(0));
        if (piwdgSel)
            m_pwndAttrEd->LoadWidgetAttrs(*piwdgSel);
    }

    //
    //  Publish a selection msg if we affected it. Note that collections don't publish
    //  anything on assignment, so our assignment above didn't send anything.
    //
    if (fcolOrgSel != m_selList.fcolIds())
        m_pstopUpdates.Publish(new TIntfEdUpdateTopic(tCQCIntfEng::TDesIdList(m_selList.fcolIds())));

    // Force a validation pass to get the error list updated
    Validate();
}


//
//  If there's a selection, we unlock any selected widgets. If the template is selected
//  or there's no selection, we unload any locked widgets.
//
//  No need to update the attribute editor, since nothing that's locked can be selected.
//
tCIDLib::TVoid TIntfEditWnd::UnlockAll()
{
    TCQCIntfTemplate& iwdgTar = iwdgEdit();

    //
    //  We'll remember the ones we are going to change so we can push an undo. In this
    //  case we can push the undo at the end, to avoid extra work.
    //
    tCQCIntfEng::TDesIdList fcolChanged(iwdgTar.c4ChildCount());

    tCIDLib::TCard4 c4Count = m_selList.c4Count();
    if (!c4Count || (m_pcivEdit->piwdgFindByDesId(m_selList.c8At(0)) == &iwdgEdit()))
    {
        // Nothing selected or the template is selected, so do them all
        c4Count = iwdgTar.c4ChildCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCIntfWidget& iwdgCur = iwdgTar.iwdgAt(c4Index);
            if (iwdgCur.bIsLocked())
            {
                fcolChanged.c4AddElement(iwdgCur.c8DesId());
                iwdgCur.bIsLocked(kCIDLib::False);
            }
        }
    }
     else
    {
        TCQCIntfWidget* piwdgCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            piwdgCur = iwdgTar.piwdgFindByDesId(m_selList.c8At(c4Index));
            if (piwdgCur->bIsLocked())
            {
                fcolChanged.c4AddElement(piwdgCur->c8DesId());
                piwdgCur->bIsLocked(kCIDLib::False);
            }
        }
    }

    m_uhEditor.Push(new TIntfLockUndo(fcolChanged, kCIDLib::True));
}


//
//  If we ended up with a single widget selected, then we have to load it into
//  the attribute editor. If more than one, we have to clear the attribute editor
//  out.
//
tCIDLib::TVoid TIntfEditWnd::UpdateAttrEd()
{
    if (m_selList.bSingleSel())
    {
        // Get the one selected widget
        TCQCIntfWidget* piwdgCur = m_pcivEdit->piwdgFindByDesId(m_selList.c8At(0));
        m_pwndAttrEd->LoadWidgetAttrs(*piwdgCur);
    }
     else
    {
        m_pwndAttrEd->ClearWidgetAttrs();
    }
}


//
//  We do a validation pass on the template and display any errors.
//
tCIDLib::TVoid TIntfEditWnd::Validate()
{
    TCQCIntfTemplate& iwdgTmpl = iwdgEdit();
    tCQCIntfEng::TErrList colErrs;
    iwdgTmpl.eDoValidation(colErrs);
    m_pwndErrList->LoadErrors(colErrs);
}


//
//  When the widget sends us a selection update. We want to do as little as possible
//  to keep it from being blinky and whatnot. This is called to do that work.
//
tCIDLib::TVoid TIntfEditWnd::WdgPalSelUpdate(const tCQCIntfEng::TDesIdList fcolUpdate)
{
    if (fcolUpdate.bIsEmpty())
    {
        // We can optimize here. Just remove any existing selection
        if (!m_selList.bIsEmpty())
        {
            // Push a selection undo
            m_uhEditor.Push(new TIntfSelectUndo(m_selList.fcolIds()));
            RemoveSelection();
        }
    }
     else
    {
        // Put the selection list into block mode for this
        TColBlockModeJan janBlock(&m_selList.fcolIds(), kCIDLib::True);

        const tCQCIntfEng::TDesIdList fcolOrg = m_selList.fcolIds();
        tCQCIntfEng::TDesIdList fcolChanged;

        // Go through our current selections. For any not in the incoming list, remove
        TCQCIntfTemplate& iwdgTmpl = iwdgEdit();
        tCIDLib::TCard4 c4Count = fcolOrg.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const tCIDLib::TCard8 c8CurId = fcolOrg[c4Index];
            if (!fcolUpdate.bElementPresent(c8CurId))
            {
                // Don't use the index since we are removing elements
                m_selList.Remove(c8CurId);
                fcolChanged.c4AddElement(c8CurId);
            }
        }

        // And for any not in our list that are in the incoming, add them
        c4Count = fcolUpdate.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const tCIDLib::TCard8 c8CurId = fcolUpdate[c4Index];
            if (!m_selList.bIsSelected(c8CurId))
            {
//                if (piwdgCur->bIsLocked())
//                    continue;
                m_selList.c4AddNew(c8CurId, kCIDLib::False);
                fcolChanged.c4AddElement(c8CurId);
            }
        }

        //
        //  Go through the affected ones and build up a region and redraw. Note that
        //  some may no longer exist. They were chanegd because they were deleted. So
        //  we have to deal with that.
        //
        c4Count = fcolChanged.c4ElemCount();
        if (c4Count)
        {
            m_uhEditor.Push(new TIntfSelectUndo(fcolOrg));

            TGUIRegion grgnRedraw;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TCQCIntfWidget* piwdgCur = iwdgTmpl.piwdgFindByDesId(fcolChanged[c4Index]);
                if (!piwdgCur)
                    continue;

                if (grgnRedraw.bIsEmpty())
                    grgnRedraw.Set(piwdgCur->areaActual());
                else
                    grgnRedraw.CombineWith(piwdgCur->areaActual(), tCIDGraphDev::EClipModes::Or);
            }

            if (!grgnRedraw.bIsEmpty())
                m_pcivEdit->Redraw(grgnRedraw);
        }
    }

    //
    //  If we ended up with a single widget selected, then load it into the attribute
    //  editor.
    //
    if (m_selList.bSingleSel())
    {
        TCQCIntfWidget* piwdgSel = iwdgEdit().piwdgFindByDesId(m_selList.c8At(0));
        if (piwdgSel)
            m_pwndAttrEd->LoadWidgetAttrs(*piwdgSel);
    }
}
