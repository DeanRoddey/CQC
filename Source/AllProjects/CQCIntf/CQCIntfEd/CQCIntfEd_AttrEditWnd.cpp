//
// FILE NAME: CQCIntfEd_AttrEditWnd.cpp
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
//  This file implements our derivative of the attribute editor.
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
#include    "CQCIntfEd_EditTBarDlg_.hpp"
#include    "CQCIntfEd_EditExprMapDlg_.hpp"
#include    "CQCIntfEd_EditXlatsDlg_.hpp"
#include    "CQCIntfEd_ImgListDlg_.hpp"
#include    "CQCIntfEd_LiveTileDlg_.hpp"
#include    "CQCIntfEd_TmplHotKeysDlg_.hpp"
#include    "CQCIntfEd_TmplActionsDlg_.hpp"
#include    "CQCIntfEd_TmplStatesDlgs_.hpp"
#include    "CQCIntfEd_EdTrigEvsDlg_.hpp"
#include    "CQCIntfEd_UserLimitsDlg_.hpp"
#include    "CQCIntfEng_Slider.hpp"
#include    "CQCIntfEng_VolKnob.hpp"


// ---------------------------------------------------------------------------
//  CLASS: TIntfAttrEdWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfAttrEdWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfAttrEdWnd::TIntfAttrEdWnd(TCQCIntfEdView* const    pcivToUse
                               , MIPEIntf* const        pmipeToUse) :

    m_pcivToUse(pcivToUse)
{
    // Set the passed IPE handler on our parent
    SetIPEHandler(pmipeToUse);
}

TIntfAttrEdWnd::~TIntfAttrEdWnd()
{
}


// ---------------------------------------------------------------------------
//  TIntfAttrEdWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard8 TIntfAttrEdWnd::c8CurDesId() const
{
    return m_c8DesId;
}


tCIDLib::TVoid TIntfAttrEdWnd::ClearWidgetAttrs()
{
    m_c8DesId = kCIDLib::c8MaxCard;
    RemoveAll();
}


tCIDLib::TVoid TIntfAttrEdWnd::LoadWidgetAttrs(const TCQCIntfWidget& iwdgToLoad)
{
    // Ask the widget for all its attributes
    tCIDMData::TAttrList colAttrs;
    TAttrData adatTmp;
    iwdgToLoad.QueryWdgAttrs(colAttrs, adatTmp);

    m_c8DesId = iwdgToLoad.c8DesId();
    LoadValues(colAttrs, iwdgToLoad.c8DesId());
    SelectByIndex(0);
}



// ---------------------------------------------------------------------------
//  TIntfAttrEdWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We override this so that we can subscribe to the editor window's update topic. We
//  don't want to do that until we are created.
//
tCIDLib::TBoolean TIntfAttrEdWnd::bCreated()
{
    if (!TAttrEditWnd::bCreated())
        return kCIDLib::False;

    SubscribeToTopic(m_pcivToUse->pwndEditor()->strEdUpdateTopic(), kCIDLib::False);
    return kCIDLib::True;
}


//
//  We override this to handle visual editing for some specialized types of our own.
//  For some we set the special attribute type and handle them generically. In some
//  cases we look for specific attributes of ours. Else, we pass it on to our parent
//  class.
//
tCIDLib::TBoolean
TIntfAttrEdWnd::bVisEdit(       TAttrData&      adatEdit
                        , const TArea&          areaCellScr
                        , const tCIDLib::TCard8 c8UserId)
{
    tCIDLib::TBoolean bHandled = kCIDLib::True;

    // We do some generically via the special type
    if (adatEdit.strSpecType() == kCQCIntfEng::strAttrType_Expression)
    {
        // These are binary, so stream in the expression for editing
        TCQCExpression exprEdit;
        if (adatEdit.c4Bytes())
        {
            TBinMBufInStream strmSrc(&adatEdit.mbufVal(), adatEdit.c4Bytes());
            strmSrc >> exprEdit;
        }

        //
        //  We are only editing the expression here. The source value to compare with the
        //  expression is the business of the widget that we are editing. They have to
        //  encode some info into the user data of the attribute. It's in the form:
        //
        //      "type", "limits"
        //
        //  So the EFldTypes type and then a limits string (which can be empty but has to
        //  be there.) If not set, then we just pass in string and no limits which makes
        //  it generic.
        //
        TString strLimits;
        tCQCKit::EFldTypes eType = tCQCKit::EFldTypes::String;
        if (!adatEdit.strUserData().bIsEmpty())
        {
            tCIDLib::TCard4 c4ErrInd;
            tCIDLib::TStrList colVals;
            if (TStringTokenizer::bParseQuotedCommaList(adatEdit.strUserData(), colVals, c4ErrInd)
            &&  (colVals.c4ElemCount() == 2))
            {
                eType = tCQCKit::eAltXlatEFldTypes(colVals[0]);
                if (eType != tCQCKit::EFldTypes::Count)
                    strLimits = colVals[1];
                else
                    eType = tCQCKit::EFldTypes::String;
            }
        }

        const tCIDLib::TBoolean bEdit = facCQCGKit().bEditExpression2
        (
            *this, L"Edit Expression", eType, strLimits, exprEdit, kCIDLib::True
        );

        if (bEdit)
        {
            // Stream it back out to the attribute
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                strmTar << exprEdit << kCIDLib::FlushIt;
                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
        }
    }
     else if ((adatEdit.strSpecType() == kCQCIntfEng::strAttrType_Image)
          ||  (adatEdit.strSpecType() == kCQCIntfEng::strAttrType_Template))
    {
        //
        //  If the current value is empty, then we need to fault in a useful
        //  base path. This is a type relative path, so we just pass a
        //  path into the user area.
        //
        TString strInitPath;
        if (adatEdit.strValue().bIsEmpty())
            strInitPath = L"/User";
        else
            strInitPath = adatEdit.strValue();

        //
        //  Set up the data type and title text based on the special type. If it's
        //  a template we pass the base widget's path as the relative path in case
        //  they are doing relative paths.
        //
        tCQCRemBrws::EDTypes eType = tCQCRemBrws::EDTypes::Count;
        const TString* pstrRelPath = nullptr;
        if (adatEdit.strSpecType() == kCQCIntfEng::strAttrType_Image)
        {
            eType = tCQCRemBrws::EDTypes::Image;
        }
         else if (adatEdit.strSpecType() == kCQCIntfEng::strAttrType_Template)
        {
            eType = tCQCRemBrws::EDTypes::Template;
            pstrRelPath = &m_pcivToUse->iwdgBaseTmpl().strTemplateName();

        }
        CIDAssert(eType != tCQCRemBrws::EDTypes::Count, L"Unknown attr special data type");

        TString strTitle(L"Select a ");
        strTitle.Append(facCQCRemBrws().strFlTypeDescr(eType));

        TString strNewPath;
        const tCIDLib::TBoolean bSel = facCQCTreeBrws().bSelectFile
        (
            *this
            , strTitle
            , eType
            , strInitPath
            , m_pcivToUse->cuctxToUse()
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strNewPath
            , nullptr
            , pstrRelPath
        );

        // If they selected something, then store it
        if (bSel)
            adatEdit.SetString(strNewPath);
    }
     else if (adatEdit.strSpecType() == kCQCIntfEng::strAttrType_ImageScope)
    {
        //
        //  If the current value is empty, then we need to fault in a useful
        //  base path.
        //
        TString strInitPath;
        if (adatEdit.strValue().bIsEmpty())
            strInitPath = L"/User";
        else
            strInitPath = adatEdit.strValue();

        // Set up the data type and title text based on the special type
        tCQCRemBrws::EDTypes eType = tCQCRemBrws::EDTypes::Count;
        if (adatEdit.strSpecType() == kCQCIntfEng::strAttrType_ImageScope)
            eType = tCQCRemBrws::EDTypes::Image;
        CIDAssert(eType != tCQCRemBrws::EDTypes::Count, L"Unknown attr special data type");

        TString strTitle(L"Select a Scope");
        TString strNewPath;
        const tCIDLib::TBoolean bSel = facCQCTreeBrws().bSelectFile
        (
            *this
            , strTitle
            , eType
            , strInitPath
            , m_pcivToUse->cuctxToUse()
            , tCQCTreeBrws::EFSelFlags::SelectScopes
            , strNewPath
        );

        // If they selected something, then store it
        if (bSel)
            adatEdit.SetString(strNewPath);
    }
     else if (adatEdit.strSpecType() == kCQCIntfEng::strAttrType_LSGraph)
    {
        //
        //  We'll us a generic list selection dialog. So first let's get the list of
        //  available graphs.
        //
        try
        {
            tCQLogicSh::TLogicSrvProxy orbcLS = facCQLogicSh().orbcLogicSrvProxy();
            tCIDLib::TStrList colGraphs;
            const tCIDLib::TCard4 c4Count = orbcLS->c4QueryGraphFlds(colGraphs);
            if (c4Count)
            {
                TString strSel = adatEdit.strValue();
                const tCIDLib::TBoolean bHandled = facCIDWUtils().bListSelect
                (
                    *this, L"Select a Graph", colGraphs, strSel
                );

                if (bHandled)
                    adatEdit.SetString(strSel);
            }
             else
            {
                TOkBox msgbNone
                (
                    strWndText(), L"No Logic Server graphs are currently defined"
                );
                msgbNone.ShowIt(*this);
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                *this
                , L"An error occurred while finding the available graphs"
                , errToCatch
            );
            return kCIDLib::True;
        }
    }
     else if (adatEdit.strSpecType() == kCQCIntfEng::strAttrType_RepoMoniker)
    {
        //
        //  We'll do the standard driver selection dialog, with a filter that only shows
        //  media repos.
        //
        TString strSel = adatEdit.strValue();
        const tCIDLib::TBoolean bSel = facCQCIGKit().bSelectDriver
        (
            *this
            , strSel
            , tCQCKit::EDevCats::MediaRepository
            , TString::strEmpty()
            , TString::strEmpty()
        );
        if (bSel)
            adatEdit.SetString(strSel);
    }




    //
    //  These are handled via the specific attribute name
    //
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Images)
    {
        // Get the image list interface of the target widget
        TCQCIntfWidget* piwdgTar = m_pcivToUse->piwdgFindByDesId(c8UserId);
        MCQCIntfImgListIntf* pmiwdgImgL = dynamic_cast<MCQCIntfImgListIntf*>(piwdgTar);

        if (!pmiwdgImgL)
        {
            TErrBox msgbErr
            (
                L"Interface Editor"
                , L"The widget does not implement the image list interface"
            );
            msgbErr.ShowIt(*this);
            return kCIDLib::False;
        }

        //
        //  Only do this if there are any images defined. There almost always is,
        //  but in those cases where it's driven by something else, dynamically,
        //  there's a possibility.
        //
        if (pmiwdgImgL->c4ImgKeyCount())
        {
            TImgListSelDlg dlgImgList;
            if (dlgImgList.bRun(*this, *pmiwdgImgL, m_pcivToUse, piwdgTar->areaActual()))
            {
                tCIDLib::TCard4 c4Sz;
                THeapBuf mbufData(kCIDLib::c4Sz_1K);
                {
                    TBinMBufOutStream strmTar(&mbufData);
                    pmiwdgImgL->WriteOutImgList(strmTar);
                    strmTar.Flush();
                    c4Sz = strmTar.c4CurPos();
                }
                adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
            }
        }
    }
     else if (adatEdit.strId() == kCQCKit::strAttr_Actions)
    {
        //
        //  The action content is just a flattened command source. Above we create a
        //  dead simple class that just implements the command source mixin. We can
        //  use this to read in the action, pass it in to be edited, then write it
        //  back out.
        //
        TBasicCmdSrc csrcEdit;
        MCQCCmdSrcIntf::TRTVList colRTVs;
        if (adatEdit.c4Bytes())
        {
            TBinMBufInStream strmSrc(&adatEdit.mbufVal(), adatEdit.c4Bytes());
            strmSrc >> csrcEdit;

            //
            //  There is also a trailing list of RTV definitions which is needed for
            //  editing the action. So read that in, and then set all of them on the
            //  cmd source. This will allow the action editor to present the correct list
            //  of RTVs to the user in the parameter value insertion popup menu.
            //
            strmSrc >> colRTVs;
            strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);

            // We start after the standard ones and then add the rest
            const tCIDLib::TCard4 c4StdCnt = MCQCCmdSrcIntf::c4StdRTVCount();
            const tCIDLib::TCard4 c4SrcCnt = colRTVs.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = c4StdCnt; c4Index < c4SrcCnt; c4Index++)
            {
                const TCQCCmdRTVal& crtvCur = colRTVs[c4Index];
                csrcEdit.AddRTValue(crtvCur.strName(), crtvCur.strId(), crtvCur.eType());
            }
        }

        // Gather up a list of named widgets as the extra action cmd targets
        tCQCKit::TCmdTarList colExtra(tCIDLib::EAdoptOpts::NoAdopt);
        TCQCIntfWidget* piwdgTar = m_pcivToUse->piwdgFindByDesId(c8UserId);
        m_pcivToUse->iwdgBaseTmpl().FindAllTargets(colExtra, kCIDLib::False);

        //
        //  Always add the owning view, since he represents the interface viewer
        //  target (for link operations and such.)
        //
        colExtra.Add(m_pcivToUse);

        //
        //  And invoke the action editor dialog. If the required limit is set, then we
        //  indicate to the editor that it cannot be empty.
        //
        const tCIDLib::TBoolean bRequired(adatEdit.strLimits() == kCIDMData::strAttrLim_Required);
        const tCIDLib::TBoolean bChanged = facCQCIGKit().bEditAction
        (
            *this
            , csrcEdit
            , bRequired
            , m_pcivToUse->cuctxToUse()
            , &colExtra
            , &m_pcivToUse->iwdgBaseTmpl().strTemplateName()
        );

        if (bChanged)
        {
            // Stream the changes back out
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                strmTar << csrcEdit
                        << colRTVs
                        << tCIDLib::EStreamMarkers::Frame
                        << kCIDLib::FlushIt;

                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Field)
    {
        //
        //  We need to access the widget for this, and make double sure it implements
        //  the single field mixin.
        //
        TCQCIntfWidget* piwdgTar = m_pcivToUse->piwdgFindByDesId(c8UserId);
        MCQCIntfSingleFldIntf* pmiwdgFldIntf = dynamic_cast<MCQCIntfSingleFldIntf*>(piwdgTar);

        if (!pmiwdgFldIntf)
        {
            TErrBox msgbErr(L"The widget does not implement the single field interface");
            msgbErr.ShowIt(*this);
            return kCIDLib::False;
        }

        // Parse out the field name components if there's one set
        TString strMoniker;
        TString strFldName;
        if (!adatEdit.strValue().bIsEmpty())
        {
            if (!facCQCKit().bParseFldName(adatEdit.strValue(), strMoniker, strFldName))
            {
                strMoniker.Clear();
                strFldName.Clear();
            }
        }

        //
        //  Get a new selection. We have to provide a field filter which the field
        //  mixin interface provides, based on what it knows are the required attributes
        //  of the widget type it is part of. We have to duplicate it since the field
        //  cache adopts the filter.
        //
        TCQCFldCache cfcFields;
        cfcFields.Initialize(::pDupObject<TCQCFldFilter>(pmiwdgFldIntf->ffiltToUse()));
        const tCIDLib::TBoolean bSel = facCQCIGKit().bSelectField
        (
            *this, cfcFields, strMoniker, strFldName
        );

        if (bSel)
        {
            //
            //  Ask the widget if it will accept this field. Generally it will since it
            //  provided us with a filter, but sometimes it might not filter out perfectly.
            //  Tell it to update with this field info if it accepts, so that it'll update
            //
            //
            const TCQCFldDef& flddSel = cfcFields.flddFor(strMoniker, strFldName);
            const tCIDLib::TBoolean bRet = pmiwdgFldIntf->bCanAcceptField
            (
                flddSel
                , cfcFields.strDevMake(strMoniker)
                , cfcFields.strDevModel(strMoniker)
                , kCIDLib::True
            );

            if (bRet)
            {
                //
                //  Store the selected field back away. Convert it back to the dotted
                //  notation.
                //
                strMoniker.Append(kCIDLib::chPeriod);
                strMoniker.Append(strFldName);
                adatEdit.SetString(strMoniker);

                //
                //  We have to deal with a special case here. If the widget implements
                //  the boolean interface, then we need to update the user data on the
                //  true/false expressions to reflect the type/limits of the newly
                //  selected field. This is used above when the expressions are edited
                //  to let the expression editing dialog provide interactive value
                //  selection.
                //
                TString strInfo;
                if (bAttrExists(kCQCIntfEng::strAttr_Bool_TrueExpr))
                {
                    MCQCIntfBooleanIntf::FormatUserData(flddSel, strInfo);
                    ChangeAttrUserData(kCQCIntfEng::strAttr_Bool_TrueExpr, strInfo);
                }

                if (bAttrExists(kCQCIntfEng::strAttr_Bool_FalseExpr))
                {
                    MCQCIntfBooleanIntf::FormatUserData(flddSel, strInfo);
                    ChangeAttrUserData(kCQCIntfEng::strAttr_Bool_FalseExpr, strInfo);
                }
            }
             else
            {
                // Tell the user we couldn't make the change
                TErrBox msgbErr(L"The target widget could not accept the selected field");
                msgbErr.ShowIt(*this);
            }
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Font_Face)
    {
        // We can pass in the other values for preview purposes
        TString strFontFace = adatFindById(kCQCIntfEng::strAttr_Font_Face).strValue();
        const tCIDLib::TBoolean bSel = facCIDWUtils().bSelectFontFace
        (
            *this
            , strFontFace
            , adatFindById(kCQCIntfEng::strAttr_Font_Height).c4Val()
            , adatFindById(kCQCIntfEng::strAttr_Font_Bold).bVal()
            , adatFindById(kCQCIntfEng::strAttr_Font_Italic).bVal()
        );

        if (bSel)
            adatEdit.SetString(strFontFace);
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Tmpl_HotKeys)
    {
        // These are the hot key actions associated with a template
        tCQCIntfEng::THKActList colEdit;
        if (adatEdit.c4Bytes())
        {
            TBinMBufInStream strmSrc(&adatEdit.mbufVal(), adatEdit.c4Bytes());
            strmSrc >> colEdit;
        }

        // Gather up a list of named widgets as the extra action cmd targets
        tCQCKit::TCmdTarList colExtra(tCIDLib::EAdoptOpts::NoAdopt);
        TCQCIntfWidget* piwdgTar = m_pcivToUse->piwdgFindByDesId(c8UserId);
        m_pcivToUse->iwdgBaseTmpl().FindAllTargets(colExtra, kCIDLib::False);

        //
        //  Always add the owning view, since he represents the interface viewer
        //  target (for link operations and such.)
        //
        colExtra.Add(m_pcivToUse);

        const TString& strTmplPath = m_pcivToUse->iwdgBaseTmpl().strTemplateName();
        TTmplHotKeysDlg dlgEdit;
        if (dlgEdit.bRun(*this, strTmplPath, colEdit, colExtra, m_pcivToUse->cuctxToUse()))
        {
            // They saved, so stream it back out to the attribute
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                strmTar << colEdit << kCIDLib::FlushIt;
                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_TimeText_Format)
    {
        // We have a helper dialog to make this easier
        TString strEdit = adatEdit.strValue();
        if (facCQCIGKit().bEditTimeTokens(*this, strEdit))
            adatEdit.SetString(strEdit);
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Tmpl_TmplActions)
    {
        // These are the template actions associated with a template
        tCQCIntfEng::TActList colEdit;
        if (adatEdit.c4Bytes())
        {
            TBinMBufInStream strmSrc(&adatEdit.mbufVal(), adatEdit.c4Bytes());
            strmSrc >> colEdit;
        }

        // Gather up a list of named widgets as the extra action cmd targets
        tCQCKit::TCmdTarList colExtra(tCIDLib::EAdoptOpts::NoAdopt);
        TCQCIntfWidget* piwdgTar = m_pcivToUse->piwdgFindByDesId(c8UserId);
        m_pcivToUse->iwdgBaseTmpl().FindAllTargets(colExtra, kCIDLib::False);

        //
        //  Always add the owning view, since he represents the interface viewer
        //  target (for link operations and such.)
        //
        colExtra.Add(m_pcivToUse);

        const TString& strTmplPath = m_pcivToUse->iwdgBaseTmpl().strTemplateName();
        TTmplActionsDlg dlgEdit;
        if (dlgEdit.bRun(*this, strTmplPath, colEdit, colExtra, m_pcivToUse->cuctxToUse()))
        {
            // They saved, so stream it back out to the attribute
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                strmTar << colEdit << kCIDLib::FlushIt;
                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
        }
    }
     else if ((adatEdit.strId() == kCQCIntfEng::strAttr_LiveTile_Fld1)
          ||  (adatEdit.strId() == kCQCIntfEng::strAttr_LiveTile_Fld2))
    {
        const TString& strOrgVal = adatEdit.strValue();

        // Parse out the field name components if there's one set
        TString strMoniker;
        TString strFldName;
        if (!strOrgVal.bIsEmpty())
        {
            if (!facCQCKit().bParseFldName(strOrgVal, strMoniker, strFldName))
            {
                strMoniker.Clear();
                strFldName.Clear();
            }
        }

        //
        //  Get a new selection. We don't do any filtering here, so we just pass the
        //  most permissive filter.
        //
        TCQCFldCache cfcFields;
        cfcFields.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite));
        const tCIDLib::TBoolean bSel = facCQCIGKit().bSelectField
        (
            *this, cfcFields, strMoniker, strFldName
        );

        if (bSel)
        {
            // Just put it back into the attribute
            strMoniker.Append(kCIDLib::chPeriod);
            strMoniker.Append(strFldName);
            adatEdit.SetString(strMoniker);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_LiveTile_SrcTemplates)
    {
        // Stream in the data
        TCQCIntfLiveTile::TTmplList colTmpls;
        TCQCIntfLiveTile::ParseSrcTmplsAttr(adatEdit.mbufVal(), adatEdit.c4Bytes(), colTmpls);

        //
        //  Get the current values of the field attributes to pass in so it can use them
        //  for validation.
        //
        TString strFld1 = adatFindById(kCQCIntfEng::strAttr_LiveTile_Fld1).strValue();
        TString strFld2 = adatFindById(kCQCIntfEng::strAttr_LiveTile_Fld2).strValue();

        // If they commit, then stream it back out
        TLiveTileDlg dlgEdit;
        if (dlgEdit.bRun(*this
                        ,colTmpls
                        , strFld1
                        , strFld2
                        , m_pcivToUse->cuctxToUse()
                        , m_pcivToUse->iwdgBaseTmpl().strTemplateName()))
        {
            THeapBuf mbufData(16 * 1024UL);
            const tCIDLib::TCard4 c4Bytes = TCQCIntfLiveTile::c4FmtSrcTmplsAttr
            (
                colTmpls, mbufData
            );
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Bytes);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_MappedImg_Mappings)
    {
        // Stream in the collection
        TCQCIntfMappedImgBase::TKeyList colEdit;
        {
            TBinMBufInStream strmSrc(&adatEdit.mbufVal(), adatEdit.c4Bytes());
            strmSrc >> colEdit;
        }

        TString strInstruct(kIEdMsgs::midDlg_MappedImg_Instruct, facCQCIntfEd());
        TString strTitle(kIEdMsgs::midDlg_MappedImg_Title, facCQCIntfEd());
        TEditExprMapDlg dlgEdit;
        if (dlgEdit.bRun(*this, strTitle, strInstruct, colEdit))
        {
            // Stream the changes back to the
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                strmTar << colEdit << kCIDLib::FlushIt;
                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);

            //
            //  And we need to update the limits on the default key attribute, since it
            //  is a list of the possible keys.
            //
            TString strLims(kCIDMData::strAttrLim_EnumPref);
            const tCIDLib::TCard4 c4Count = colEdit.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                if (c4Index)
                    strLims.Append(kCIDLib::chComma);
                strLims.Append(colEdit[c4Index].objKey());
            }

            ChangeAttrLimits(kCQCIntfEng::strAttr_MappedImg_DefKey, strLims);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Fields)
    {
        //
        //  <TBD>
        //  NOT actually needed yet, but at some point. They are widgets that use the
        //  multi-field mixin interface but they directly set their fields, they don't
        //  let the user select them.
        //
    }
     else if ((adatEdit.strId() == kCQCIntfEng::strAttr_Slider_Limits)
          ||  (adatEdit.strId() == kCQCIntfEng::strAttr_VKnob_Limits))
    {
        //
        //  The limits are stored away for consistency, but in this case, we go back
        //  to the widget to get the current limits since they could have changed if
        //  a new field or variable was selected.
        //
        TCQCIntfWidget* piwdgTar = m_pcivToUse->piwdgFindByDesId(c8UserId);
        tCQCIntfEng::TUserLims fcolLims;
        if (adatEdit.strId() == kCQCIntfEng::strAttr_Slider_Limits)
        {
            TCQCIntfSliderBase* piwdgSlider = static_cast<TCQCIntfSliderBase*>(piwdgTar);
            fcolLims = piwdgSlider->fcolLimits();
        }
         else
        {
            TCQCIntfVolKnobBase* piwdgVKnob = static_cast<TCQCIntfVolKnobBase*>(piwdgTar);
            fcolLims = piwdgVKnob->fcolLimits();
        }

        //
        //  These will only be set on field based widgets. And they will not have been
        //  linked to their fields at design time. We get the field mixin and get the
        //  info in it and use that to query the field definition. From that we get the
        //  limits info.
        //
        MCQCIntfSingleFldIntf* pmiwdgFld = dynamic_cast<MCQCIntfSingleFldIntf*>(piwdgTar);

        TCQCFldDef flddCur;
        if (!facCQCKit().bQueryFldDef(pmiwdgFld->strMoniker(), pmiwdgFld->strFieldName(), flddCur))
        {
            TErrBox msgbErr(L"Information on the associated field could not be queried");
            msgbErr.ShowIt(*this);
            return kCIDLib::True;
        }

        //
        //  We have to get the limits, which is a pain since it could be one of three
        //  different types. But there's a helper for this.
        //
        tCIDLib::TFloat8 f8Min, f8Max;
        const tCIDLib::TBoolean bRes = facCQCKit().bCheckNumRangeFld
        (
            flddCur, kCQCIntfEng::f8MinRange, kCQCIntfEng::f8MaxRange, kCIDLib::True, f8Min, f8Max
        );

        // Invoke the editor dialog. If they save, stream them back out
        TUserLimitsDlg dlgLims;
        if (dlgLims.bRun(*this, fcolLims, tCIDLib::TInt4(f8Min), tCIDLib::TInt4(f8Max)))
        {
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                strmTar << fcolLims << kCIDLib::FlushIt;
                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Tmpl_States)
    {
        // Stream in the state info.
        TCQCIntfStateList istlStates;
        if (adatEdit.c4Bytes())
        {
            TBinMBufInStream strmSrc(&adatEdit.mbufVal(), adatEdit.c4Bytes());
            strmSrc >> istlStates;
        }

        TTmplStatesDlg dlgStates;
        if (dlgStates.bRun(*this, istlStates))
        {
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                strmTar << istlStates << kCIDLib::FlushIt;
                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_ToolBar_Buttons)
    {
        TCQCIntfToolbar* piwdgTar = static_cast<TCQCIntfToolbar*>
        (
            m_pcivToUse->piwdgFindByDesId(c8UserId)
        );

        if (!piwdgTar)
        {
            TErrBox msgbErr(L"Interface Editor", L"The widget could not be found");
            msgbErr.ShowIt(*this);
            return kCIDLib::False;
        }

        //
        //  Invoke the button editor dialog. It directly edits the widget. But, if the user
        //  commits, for undo purposes we have to get the new content into the attribute.
        //
        TEditTBarDlg dlgButts;
        if (dlgButts.bRun(*this, piwdgTar, m_pcivToUse))
        {
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                piwdgTar->StreamButtons(strmTar);
                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Tmpl_TrigEvents)
    {
        //
        //  It's the triggered events for a template. So we just need to invoke
        //  the editing dialog for this.
        //
        TCQCIntfTemplate* piwdgTar = static_cast<TCQCIntfTemplate*>
        (
            m_pcivToUse->piwdgFindByDesId(c8UserId)
        );

        if (!piwdgTar)
        {
            TErrBox msgbErr(L"Interface Editor", L"The target widget could not be found");
            msgbErr.ShowIt(*this);
            return kCIDLib::False;
        }

        TEdTrigEvsDlg dlgTrigEvs;
        if (dlgTrigEvs.bRun(*this, piwdgTar))
        {
            // For undo purposes we need to store the changes
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                strmTar << piwdgTar->colTrgEvents() << kCIDLib::FlushIt;
                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Widget_States)
    {
        // We read in the state name list and invoke an editor dialog
        tCIDLib::TStrList colCurStates;
        if (adatEdit.c4Bytes())
        {
            TBinMBufInStream strmSrc(&adatEdit.mbufVal(), adatEdit.c4Bytes());
            strmSrc >> colCurStates;
        }

        // Get the list of available state names
        tCIDLib::TStrList colAvailStates;
        const TCQCIntfStateList& istlList = m_pcivToUse->iwdgBaseTmpl().istlStates();
        tCIDLib::TCard4 c4StateCnt = istlList.c4StateCount();

        // If there aren't any, just tell the user, else let them select
        if (c4StateCnt)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4StateCnt; c4Index++)
                colAvailStates.objAdd(istlList.istAt(c4Index).strName());

            const tCIDLib::TBoolean bRes = facCIDWUtils().bListMultiSelect2
            (
                *this
                , L"Select Widget States"
                , L"State Name"
                , colAvailStates
                , colCurStates
            );

            if (bRes)
            {
                tCIDLib::TCard4 c4Sz;
                THeapBuf mbufData(kCIDLib::c4Sz_1K);
                {
                    TBinMBufOutStream strmTar(&mbufData);
                    strmTar << colCurStates << kCIDLib::FlushIt;
                    c4Sz = strmTar.c4CurPos();
                }
                adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
            }
        }
         else
        {
            // Make sure this widget has no states defined
            if (!colCurStates.bIsEmpty())
            {
                colCurStates.RemoveAll();
                TBinMBufOutStream strmTar(128UL);
                strmTar << colCurStates << kCIDLib::FlushIt;
                adatEdit.SetMemBuf(strmTar.mbufData(), strmTar.c4CurSize());
            }

            TOkBox msgbNone(L"This template has not defined any states");
            msgbNone.ShowIt(*this);
        }
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Var_Variable)
    {
        //
        //  Pop up the variable cheat sheet, in by name only mode. That means that the
        //  bByValue parameter is just a dummy here.
        //
        tCIDLib::TBoolean bByValue = kCIDLib::False;
        TString strToFill = adatEdit.strValue();
        const tCIDLib::TBoolean bRes = facCQCIGKit().bManageVars
        (
            *this
            , m_pcivToUse->cuctxToUse().strUserName()
            , strToFill
            , bByValue
            , kCIDLib::True
            , m_pcivToUse->cuctxToUse()
        );

        // Put the selected value back into the attribute
        if (bRes)
            adatEdit.SetString(strToFill);
    }
     else if (adatEdit.strId() == kCQCIntfEng::strAttr_Xlats)
    {
        // Stream in the data to edit
        TIEXlatsWrap iewXlats;
        if (adatEdit.c4Bytes())
        {
            TBinMBufInStream strmSrc(&adatEdit.mbufVal(), adatEdit.c4Bytes());
            iewXlats.ReadInXlats(strmSrc);
        }

        TEditXlatsDlg dlgXlat;
        if (dlgXlat.bRun(*this, iewXlats))
        {
            // They saved, so stream it back out to the attribute
            tCIDLib::TCard4 c4Sz;
            THeapBuf mbufData(kCIDLib::c4Sz_1K);
            {
                TBinMBufOutStream strmTar(&mbufData);
                iewXlats.WriteOutXlats(strmTar);
                strmTar.Flush();
                c4Sz = strmTar.c4CurPos();
            }
            adatEdit.SetMemBuf(tCIDLib::ForceMove(mbufData), c4Sz);
        }
    }
     else
    {
        bHandled = TAttrEditWnd::bVisEdit(adatEdit, areaCellScr, c8UserId);
    }

    return bHandled;
}


//
//  We register for the interface engine's published topic for basic changes to the
//  widgets (of which the position is the only one that can change behind our back since
//  we are the means by which the others are changed.) So we just watch for change msgs
//  msgs and update the area if it's for our displayed widget and the area is different.
//  It could be the same since we could have changed it.
//
tCIDLib::TVoid TIntfAttrEdWnd::ProcessPubMsg(const TPubSubMsg& psmsgIn)
{
    try
    {
        // Based on the event, let's update our list
        const TIntfEdUpdateTopic& ieutCur = psmsgIn.objMsgAs<TIntfEdUpdateTopic>();

        if ((ieutCur.m_eType == TIntfEdUpdateTopic::ETypes::Changed)
        &&  (m_c8DesId != kCIDLib::c4MaxCard)
        &&  (ieutCur.m_c8DesId == m_c8DesId))
        {
            //
            //  Find the area or size attribute, depending on whether it's a template
            //  or a child widget.
            //
            if (bAttrExists(kCQCIntfEng::strAttr_Base_Area))
            {
                const TAttrData& adatArea = adatFindById(kCQCIntfEng::strAttr_Base_Area);
                if (adatArea.areaVal() != ieutCur.m_areaPos)
                    UpdateArea(kCQCIntfEng::strAttr_Base_Area, ieutCur.m_areaPos);
            }
             else if (bAttrExists(kCQCIntfEng::strAttr_Base_Size))
            {
                const TAttrData& adatArea = adatFindById(kCQCIntfEng::strAttr_Base_Size);
                if (adatArea.szVal() != ieutCur.m_areaPos.szArea())
                    UpdateSize(kCQCIntfEng::strAttr_Base_Size, ieutCur.m_areaPos.szArea());
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}

