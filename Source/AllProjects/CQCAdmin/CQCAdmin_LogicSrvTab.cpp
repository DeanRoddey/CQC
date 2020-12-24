//
// FILE NAME: CQCAdmin_LogicSrvTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/03/2016
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
//  This file implements a tab for editing the logic server configuration.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_LogicSrvTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TLogicSrvTab, TContentTab)


// ---------------------------------------------------------------------------
//   CLASS: TLServerCfgNewDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TLServerCfgNewDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLServerCfgNewDlg();

        TLServerCfgNewDlg(const TLServerCfgNewDlg&) = delete;

        ~TLServerCfgNewDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TLServerCfgNewDlg& operator=(const TLServerCfgNewDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TCQLSrvFldType*&        pclsftToFill
            , const tCIDLib::TStrList&      colNames
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pcolNames
        //      The caller gives us a list of any current field names, so that we can
        //      check for dups.
        //
        //  m_ppclsftEdit
        //      A pointer to a pointer to the field type object we'll create based on
        //      the selected type.
        //
        //  m_pwndXXX
        //      Typed pointers to those child controls of ours that we want
        //      to keep around for quick access.
        // -------------------------------------------------------------------
        const tCIDLib::TStrList*    m_pcolNames;
        TCQLSrvFldType**            m_ppclsftEdit;
        TRadioButton*               m_pwndBoolean;
        TPushButton*                m_pwndCancel;
        TRadioButton*               m_pwndCardinal;
        TRadioButton*               m_pwndFloat;
        TRadioButton*               m_pwndInteger;
        TEntryField*                m_pwndName;
        TPushButton*                m_pwndSave;
        TRadioButton*               m_pwndString;
        TRadioButton*               m_pwndTime;
        TMultiColListBox*           m_pwndTypeList;
};


// ---------------------------------------------------------------------------
//  TLServerCfgNewDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TLServerCfgNewDlg::TLServerCfgNewDlg() :

    m_ppclsftEdit(nullptr)
    , m_pwndBoolean(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndCardinal(nullptr)
    , m_pwndFloat(nullptr)
    , m_pwndInteger(nullptr)
    , m_pwndName(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndString(nullptr)
    , m_pwndTime(nullptr)
    , m_pwndTypeList(nullptr)
{
}

TLServerCfgNewDlg::~TLServerCfgNewDlg()
{
}


// ---------------------------------------------------------------------------
//  TLServerCfgNewDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TLServerCfgNewDlg::bRunDlg( const   TWindow&            wndOwner
                            ,       TCQLSrvFldType*&    pclsftToFill
                            , const tCIDLib::TStrList&  colNames)
{
    // Save the info for later use
    m_pcolNames = &colNames;
    m_ppclsftEdit = &pclsftToFill;

    // And now we can run it
    const tCIDLib::TCard4 c4Res = c4RunDlg(wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_LSrvNew);
    return (c4Res == kCQCAdmin::ridDlg_LSrvNew_Save);
}


// ---------------------------------------------------------------------------
//  TLServerCfgNewDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TLServerCfgNewDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TDlgBox::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_Bool, m_pwndBoolean);
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_Card, m_pwndCardinal);
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_Float, m_pwndFloat);
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_Int, m_pwndInteger);
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_Name, m_pwndName);
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_Save, m_pwndSave);
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_String, m_pwndString);
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_Time, m_pwndTime);
    CastChildWnd(*this, kCQCAdmin::ridDlg_LSrvNew_TypeList, m_pwndTypeList);

    // And register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TLServerCfgNewDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TLServerCfgNewDlg::eClickHandler);
    m_pwndTypeList->pnothRegisterHandler(this, &TLServerCfgNewDlg::eLBHandler);

    // Set up the column title
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facCQCAdmin.strMsg(kCQCAMsgs::midTab_LSrvNew_TypeListTitle));
    m_pwndTypeList->SetColumns(colCols);

    //
    //  Load up the type list with the types we know about. We set the field config
    //  class name as the user data on each item
    //
    colCols[0] = L"Boolean";
    m_pwndTypeList->c4AddItem(colCols, 0, L"TCQSLLDBool");
    colCols[0] = L"Elapsed Time";
    m_pwndTypeList->c4AddItem(colCols, 0, L"TCQSLLDElapsedTm");
    colCols[0] = L"Math Formula";
    m_pwndTypeList->c4AddItem(colCols, 0, L"TCQSLLDMath");
    colCols[0] = L"Min/Max/Average";
    m_pwndTypeList->c4AddItem(colCols, 0, L"TCQSLLDMinMaxAvg");
    colCols[0] = L"Pattern Format";
    m_pwndTypeList->c4AddItem(colCols, 0, L"TCQSLLDPatFmt");
    colCols[0] = L"Graph";
    m_pwndTypeList->c4AddItem(colCols, 0, L"TCQSLLDGraph");
    colCols[0] = L"On Count";
    m_pwndTypeList->c4AddItem(colCols, 0, L"TCQSLLDOnCounter");
    colCols[0] = L"Running Average";
    m_pwndTypeList->c4AddItem(colCols, 0, L"TCQSLLDRunAvg");

    // Select an initial type as the default
    m_pwndTypeList->SelectByIndex(0, kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TLServerCfgNewDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses
TLServerCfgNewDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_LSrvNew_Cancel)
    {
        EndDlg(kCQCAdmin::ridDlg_LSrvNew_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_LSrvNew_Save)
    {
        TString strName = m_pwndName->strWndText();
        strName.StripWhitespace();

        // The name can't be empty
        if (strName.bIsEmpty())
        {
            TErrBox msgbName(facCQCAdmin.strMsg(kCQCAErrs::errcGen_NoName));
            msgbName.ShowIt(*this);
            m_pwndName->TakeFocus();
            return tCIDCtrls::EEvResponses::Handled;
        }

        // Make sure it's valid for creating a field
        if (!facCQCKit().bIsValidFldName(strName, kCIDLib::True))
        {
            TErrBox msgbName(facCQCAdmin.strMsg(kCQCAErrs::errcGen_BadFldName));
            msgbName.ShowIt(*this);
            m_pwndName->TakeFocus();
            return tCIDCtrls::EEvResponses::Handled;
        }

        // Make sure it's not a dup
        const tCIDLib::TCard4 c4NameCnt = m_pcolNames->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4NameCnt; c4Index++)
        {
            if (m_pcolNames->objAt(c4Index).bCompareI(strName))
            {
                TErrBox msgbName(facCQCAdmin.strMsg(kCQCAErrs::errcGen_DupName), strName);
                msgbName.ShowIt(*this);
                m_pwndName->TakeFocus();
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        // We'll dynamically create an object based on the class type we stored
        const tCIDLib::TCard4 c4Index = m_pwndTypeList->c4CurItem();
        CIDAssert(c4Index != kCIDLib::c4MaxCard, L"No type is selected");
        *m_ppclsftEdit = static_cast<TCQLSrvFldType*>
        (
            TClass::pobjMakeNewOfClass(m_pwndTypeList->strUserDataAt(c4Index))
        );

        // Set the selected name and data type
        (*m_ppclsftEdit)->strFldName(strName);

        if (m_pwndBoolean->bCheckedState())
            (*m_ppclsftEdit)->eType(tCQCKit::EFldTypes::Boolean);
        else if (m_pwndCardinal->bCheckedState())
            (*m_ppclsftEdit)->eType(tCQCKit::EFldTypes::Card);
        else if (m_pwndFloat->bCheckedState())
            (*m_ppclsftEdit)->eType(tCQCKit::EFldTypes::Float);
        else if (m_pwndInteger->bCheckedState())
            (*m_ppclsftEdit)->eType(tCQCKit::EFldTypes::Int);
        else if (m_pwndString->bCheckedState())
            (*m_ppclsftEdit)->eType(tCQCKit::EFldTypes::String);
        else if (m_pwndTime->bCheckedState())
            (*m_ppclsftEdit)->eType(tCQCKit::EFldTypes::Time);
        else
        {
            CIDAssert2(L"Unknown logic server field type");
        }
        EndDlg(kCQCAdmin::ridDlg_LSrvNew_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TLServerCfgNewDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        //
        //  Based on the class type selected, we need to select a default
        //  data type and disable those that are not legal for the type.
        //
        const TString& strType = m_pwndTypeList->strUserDataAt(wnotEvent.c4Index());

        if (strType == L"TCQSLLDBool")
        {
            // It can only be boolean
            m_pwndBoolean->SetCheckedState(kCIDLib::True);
            m_pwndBoolean->SetEnable(kCIDLib::True);

            m_pwndCardinal->SetEnable(kCIDLib::False);
            m_pwndFloat->SetEnable(kCIDLib::False);
            m_pwndInteger->SetEnable(kCIDLib::False);
            m_pwndString->SetEnable(kCIDLib::False);
            m_pwndTime->SetEnable(kCIDLib::False);
        }
         else if (strType == L"TCQSLLDElapsedTm")
        {
            // It can only be time
            m_pwndTime->SetCheckedState(kCIDLib::True);
            m_pwndTime->SetEnable(kCIDLib::True);

            m_pwndBoolean->SetEnable(kCIDLib::False);
            m_pwndCardinal->SetEnable(kCIDLib::False);
            m_pwndFloat->SetEnable(kCIDLib::False);
            m_pwndInteger->SetEnable(kCIDLib::False);
            m_pwndString->SetEnable(kCIDLib::False);
        }
         else if ((strType == L"TCQSLLDMath")
              ||  (strType == L"TCQSLLDMinMaxAvg")
              ||  (strType == L"TCQSLLDRunAvg"))
        {
            // It can only be one of the numerics, start on card
            m_pwndCardinal->SetCheckedState(kCIDLib::True);
            m_pwndCardinal->SetEnable(kCIDLib::True);
            m_pwndFloat->SetEnable(kCIDLib::True);
            m_pwndInteger->SetEnable(kCIDLib::True);

            m_pwndBoolean->SetEnable(kCIDLib::False);
            m_pwndString->SetEnable(kCIDLib::False);
            m_pwndTime->SetEnable(kCIDLib::False);
        }
         else if (strType == L"TCQSLLDPatFmt")
        {
            // It can only be string
            m_pwndString->SetCheckedState(kCIDLib::True);
            m_pwndBoolean->SetEnable(kCIDLib::True);

            m_pwndBoolean->SetEnable(kCIDLib::False);
            m_pwndCardinal->SetEnable(kCIDLib::False);
            m_pwndFloat->SetEnable(kCIDLib::False);
            m_pwndInteger->SetEnable(kCIDLib::False);
            m_pwndTime->SetEnable(kCIDLib::False);
        }
         else if (strType == L"TCQSLLDGraph")
        {
            // It can only be float
            m_pwndFloat->SetCheckedState(kCIDLib::True);
            m_pwndFloat->SetEnable(kCIDLib::True);

            m_pwndBoolean->SetEnable(kCIDLib::False);
            m_pwndCardinal->SetEnable(kCIDLib::False);
            m_pwndInteger->SetEnable(kCIDLib::False);
            m_pwndString->SetEnable(kCIDLib::False);
            m_pwndTime->SetEnable(kCIDLib::False);
        }
         else if (strType == L"TCQSLLDOnCounter")
        {
            // It can only be cardinal
            m_pwndCardinal->SetCheckedState(kCIDLib::True);
            m_pwndCardinal->SetEnable(kCIDLib::True);

            m_pwndBoolean->SetEnable(kCIDLib::False);
            m_pwndFloat->SetEnable(kCIDLib::False);
            m_pwndInteger->SetEnable(kCIDLib::False);
            m_pwndString->SetEnable(kCIDLib::False);
            m_pwndTime->SetEnable(kCIDLib::False);
        }
         else
        {
            TErrBox msgbErr(TString(L"Unknown logic server field type ") + strType);
            msgbErr.ShowIt(*this);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}





// ---------------------------------------------------------------------------
//  CLASS: TLogicSrvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLogicSrvTab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TLogicSrvTab::TLogicSrvTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Config", kCIDLib::True)
    , m_c4NextId(1)
    , m_pwndAddFld(nullptr)
    , m_pwndAddSrcFld(nullptr)
    , m_pwndDelFld(nullptr)
    , m_pwndDelSrcFld(nullptr)
    , m_pwndFldList(nullptr)
    , m_pwndSrcFldDn(nullptr)
    , m_pwndSrcFldUp(nullptr)
    , m_pwndSrcList(nullptr)
{
}

TLogicSrvTab::~TLogicSrvTab()
{
}


// ---------------------------------------------------------------------------
//  TDrvMonTab: Public, inherited methods
// ---------------------------------------------------------------------------

// We have two MC list boxes but only one is set for in-place editing, the field names
tCIDLib::TBoolean
TLogicSrvTab::bIPETestCanEdit(  const   tCIDCtrls::TWndId
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    // Get the current text and set up the attribute
    TString strVal;
    m_pwndFldList->QueryColText(c4IPERow(), c4ColInd, strVal);
    adatFill.Set
    (
        L"Name", L"Name", kCIDMData::strAttrLim_Required, tCIDMData::EAttrTypes::String
    );
    adatFill.SetValueFromText(strVal);

    // Indicate we set the value
    bValueSet = kCIDLib::True;

    return kCIDLib::True;
}


//
tCIDLib::TBoolean
TLogicSrvTab::bIPEValidate( const   TString&            strSrc
                            ,       TAttrData&          adatTar
                            , const TString&            strNewVal
                            ,       TString&            strErrMsg
                            ,       tCIDLib::TCard8&    c8UserId) const
{
    //
    //  Call the base mixin class to do basic validation. If that fails, then
    //  no need to look further.
    //
    if (!MIPEIntf::bIPEValidate(strSrc, adatTar, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    CIDAssert(adatTar.strId() == L"Name", L"Unknown attribute name");

    // Make sure the name is unique and a legal field name
    if (!facCQCKit().bIsValidFldName(strNewVal, kCIDLib::True))
    {
        strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcGen_BadFldName);
        return kCIDLib::False;
    }

    // Make sure it's not a dup
    TString strName;
    const tCIDLib::TCard4 c4FldCnt = m_pwndFldList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FldCnt; c4Index++)
    {
        // Skip the current row
        if (c4Index == c4IPERow())
            continue;

        m_pwndFldList->QueryColText(c4IPERow(), 0, strName);
        if (strName.bCompareI(strNewVal))
        {
            strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcGen_DupName, strName);
            return kCIDLib::False;
        }
    }

    // It passed so set it on the attribute
    adatTar.SetValueFromText(strNewVal);

    return kCIDLib::True;
}


tCIDLib::TVoid
TLogicSrvTab::IPEValChanged(const   tCIDCtrls::TWndId
                            , const TString&            strSrc
                            , const TAttrData&          adatNew
                            , const TAttrData&          adatOld
                            , const tCIDLib::TCard8     )
{
    //
    //  Update the config. We need to get the id of the row, then use that to find the
    //  original field index.
    //
    clsftSelected().strFldName(adatNew.strValue());
}



// ---------------------------------------------------------------------------
//  TLogicSrvTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TLogicSrvTab::CreateTab(        TTabbedWnd&         wndParent
                        , const TString&            strTabText
                        , const TCQLogicSrvCfg&     lscfgEdit)
{
    //
    //  Though the main facility should not let anyone with less than power user log into
    //  this program, check it just in case.
    //
    CIDAssert
    (
        facCQCAdmin.eUserRole() >= tCQCKit::EUserRoles::PowerUser
        , L"This requires power user or higher privileges"
    )

    // Store away the info, and a second copy for comparisons later
    m_lscfgEdit = lscfgEdit;
    m_lscfgOrg = lscfgEdit;

    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TLogicSrvTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TLogicSrvTab::AreaChanged(  const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Resize our child controls if appropriate
    if (bSizeChanged && m_pwndSrcList && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TLogicSrvTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls.
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_LogicSrv, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to fit them to us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    //
    //  Set us to use the standard window background. The dialog description we
    //  used to create the content was set to the main window theme, so all of the
    //  widgets should be fine already.
    //
    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    // Get pointers to our handful of controls and load those that show data
    CastChildWnd(*this, kCQCAdmin::ridTab_LSrv_AddFld, m_pwndAddFld);
    CastChildWnd(*this, kCQCAdmin::ridTab_LSrv_AddSrcFld, m_pwndAddSrcFld);
    CastChildWnd(*this, kCQCAdmin::ridTab_LSrv_DelFld, m_pwndDelFld);
    CastChildWnd(*this, kCQCAdmin::ridTab_LSrv_DelSrcFld, m_pwndDelSrcFld);
    CastChildWnd(*this, kCQCAdmin::ridTab_LSrv_FldList, m_pwndFldList);
    CastChildWnd(*this, kCQCAdmin::ridTab_LSrv_FldConfig, m_pwndPerType);
    CastChildWnd(*this, kCQCAdmin::ridTab_LSrv_SrcFldDn, m_pwndSrcFldDn);
    CastChildWnd(*this, kCQCAdmin::ridTab_LSrv_SrcFldUp, m_pwndSrcFldUp);
    CastChildWnd(*this, kCQCAdmin::ridTab_LSrv_SrcList, m_pwndSrcList);

    // The field list has a single column
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(facCQCAdmin.strMsg(kCQCAMsgs::midTab_LSrv_FldListTitle));
    m_pwndFldList->SetColumns(colCols);

    // Go through the list and load up the defined fields
    const tCIDLib::TCard4 c4Count = m_lscfgEdit.c4FldCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQLSrvFldType* pclsftAt = m_lscfgEdit.pclsftAt(c4Index);
        pclsftAt->c4FldId(m_c4NextId++);
        colCols[0] = pclsftAt->strFldName();
        m_pwndFldList->c4AddItem(colCols, pclsftAt->c4FldId());
    }

    // The source list has two columns
    colCols[0] = L"#   ";
    colCols.objAdd(facCQCAdmin.strMsg(kCQCAMsgs::midTab_LSrv_SrcListTitle));
    m_pwndSrcList->SetColumns(colCols);
    m_pwndSrcList->AutoSizeCol(0, kCIDLib::True);

    // Set our event handlers
    m_pwndAddFld->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);
    m_pwndAddSrcFld->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);
    m_pwndDelFld->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);
    m_pwndDelSrcFld->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);
    m_pwndFldList->pnothRegisterHandler(this, &TLogicSrvTab::eLBHandler);
    m_pwndSrcList->pnothRegisterHandler(this, &TLogicSrvTab::eLBHandler);
    m_pwndSrcFldDn->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);
    m_pwndSrcFldUp->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);

    // Set us as the in place editor on the field list
    m_pwndFldList->SetAutoIPE(this);

    //
    //  Select the 0th field to get everything set up, if we have any incoming. Force
    //  a selection event to make the info get loaded.
    //
    if (c4Count)
        m_pwndFldList->SelectByIndex(0, kCIDLib::True);

    return kCIDLib::True;
}


tCIDLib::ESaveRes
TLogicSrvTab::eDoSave(          TString&                strErrMsg
                        , const tCQCAdmin::ESaveModes   eMode
                        ,       tCIDLib::TBoolean&      bChangesSaved)
{
    //
    //  We need a field cache. In this case, we are only using it to validate
    //  existing selected fields, so we don't have to use any per field
    //  filtering as we do elsewhere in this dialog. We just insure that we
    //  get readable fields since all source files must be readable.
    //
    TCQCFldCache cfcInfo;
    cfcInfo.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite));

    tCQLogicSh::TFldList& colFlds = m_lscfgEdit.colFldTypes();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        tCIDLib::TCard4 c4SrcErrId = kCIDLib::c4MaxCard;
        if (!colFlds[c4Index]->bValidate(strErrMsg, c4SrcErrId, cfcInfo))
        {
            // Select the offending field if in save mode
            if (eMode == tCQCAdmin::ESaveModes::Save)
            {
                tCIDLib::TCard4 c4ListInd = m_pwndFldList->c4IdToIndex
                (
                    m_lscfgEdit.pclsftAt(c4Index)->c4FldId()
                );
                m_pwndFldList->SelectByIndex(c4ListInd);

                // If there's a src field specific index, select that
                if (c4SrcErrId != kCIDLib::c4MaxCard)
                    m_pwndSrcList->SelectByIndex(c4SrcErrId);
            }
            return tCIDLib::ESaveRes::Errors;
        }
    }

    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (m_lscfgEdit == m_lscfgOrg)
            return tCIDLib::ESaveRes::NoChanges;
        return tCIDLib::ESaveRes::OK;
    }

    // There are changes, so write our current edit contents out if allowed
    try
    {
        tCQLogicSh::TLogicSrvProxy orbcLogic = facCQLogicSh().orbcLogicSrvProxy();

        tCIDLib::TCard4 c4SerialNum;
        orbcLogic->SetConfig(m_lscfgEdit, c4SerialNum);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErrMsg = errToCatch.strErrText();
        return tCIDLib::ESaveRes::Errors;
    }

    // Store the changes to the org member
    m_lscfgOrg = m_lscfgEdit;
    bChangesSaved = kCIDLib::True;

    // No changes anymore
    return tCIDLib::ESaveRes::NoChanges;
}


// ---------------------------------------------------------------------------
//  TLogicSrvTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when the user asked to add a source field. We need to ask the currently
//  selected field for any field criteria object he'd like us to use, and we then do the
//  standard field selection dialog.
//
tCIDLib::TVoid TLogicSrvTab::AddSrcField()
{
    // Get the selected field. If none, then do nothing
    const tCIDLib::TCard4 c4FldSel = m_pwndFldList->c4CurItem();
    if (c4FldSel == kCIDLib::c4MaxCard)
        return;

    // And get the indicated field, mapping the list window index to the config index
    TCQLSrvFldType& clsftTar = clsftSelected();

    // Make sure he's not maxed out on source fields
    if (clsftTar.c4SrcFldCount() >= clsftTar.c4MaxSrcFields())
    {
        TOkBox msgbAlready(facCQCAdmin.strMsg(kCQCAMsgs::midStatus_MaxSrcFieldsUsed));
        msgbAlready.ShowIt(*this);
        return;
    }

    //
    //  Ask him for a field filter to use and set up a field info cache with
    //  that. The cache adopts it.
    //
    TCQCFldCache cfcInfo;
    cfcInfo.Initialize(clsftTar.pffiltToUse());

    // We can just use the standard field selection dialog for this
    tCQCKit::EDevCats   eDevCat;
    TString             strDriver;
    TString             strField;
    TString             strFullField;
    TString             strMake;
    TString             strModel;
    TString             strNum;
    if (facCQCIGKit().bSelectField(*this, cfcInfo, strDriver, strField))
    {
        //
        //  Make sure it's not the field we are configuring. No circular
        //  references are allowed.
        //
        const TCQCFldDef& flddSrc = cfcInfo.flddFor(strDriver, strField);
        if (strField == clsftTar.strFldName())
        {
            cfcInfo.bDevExists(strDriver, strMake, strModel, eDevCat);
            if (strModel == L"CQSLLogicSrv")
            {
                TErrBox msgbSelfRef(facCQCAdmin.strMsg(kCQCAErrs::errcLSrv_NoSelfRef));
                msgbSelfRef.ShowIt(*this);
                return;
            }
        }

        // Build up the full field name
        strFullField = strDriver;
        strFullField.Append(kCIDLib::chPeriod);
        strFullField.Append(strField);

        //
        //  Add this guy to the source list of the field, then to our list box, if it was
        //  taken. It will be ignored if it's already in the list. We number the fields and
        //  add that as the first column. Make it the new selection.
        //
        if (clsftTar.bAddField(strFullField))
        {
            tCIDLib::TStrList colCols(2);
            colCols.objAdd(TString::strEmpty());
            colCols[0].SetFormatted(clsftTar.c4SrcFldCount());
            colCols.objAdd(strFullField);
            m_pwndSrcList->c4AddItem(colCols, 0, kCIDLib::c4MaxCard, kCIDLib::True);
        }
         else
        {
            TOkBox msgbAlready
            (
                facCQCAdmin.strMsg(kCQCAMsgs::midStatus_FldAlreadyUsed, strFullField)
            );
            msgbAlready.ShowIt(*this);
        }
    }
}


//
//  Figures out the index in the configuration data for the passed field list window index.
//  The window is sorted while the fields in the config data are not. We use the psuedo
//  field id that we assign to each field (and set as the row id on each row in the list
//  window) to find it.
//
tCIDLib::TCard4
TLogicSrvTab::c4ListSelToFld(  const   tCIDLib::TCard4     c4WndIndex
                                    , const tCIDLib::TBoolean   bThrowIfNot) const
{
    if (c4WndIndex == kCIDLib::c4MaxCard)
    {
        if (bThrowIfNot)
        {
            facCQCAdmin.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCAErrs::errcLSrv_NoSelection
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotReady
            );
        }
        return kCIDLib::c4MaxCard;
    }

    const tCIDLib::TCard4 c4Id = m_pwndFldList->c4IndexToId(c4WndIndex);
    tCIDLib::TCard4 c4Index;
    if (!m_lscfgEdit.bFindById(c4Id, c4Index))
    {
        if (bThrowIfNot)
        {
            facCQCAdmin.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCAErrs::errcLSrv_NoSelection
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotReady
            );
        }
        c4Index = kCIDLib::c4MaxCard;
    }
    return c4Index;
}


//
//  The field list window is sorted, but the fields in the actual configuration data are
//  not. So, any time we want to get the configuration data for the selected field, we have
//  to get the index of the selected item, get its row id which is the pseudo field id we
//  assign to all of the fields, then use that to look up the field by its id, and get back
//  the configuration data index.
//
//  We can the use that to return a ref to the correct one. This avoids a lot of grunt work
//  and possible mistakes by keeping this all here.
//
TCQLSrvFldType& TLogicSrvTab::clsftSelected()
{
    const tCIDLib::TCard4 c4CfgIndex = c4ListSelToFld
    (
        m_pwndFldList->c4CurItem(), kCIDLib::True
    );
    return *m_lscfgEdit.pclsftAt(c4CfgIndex);
}



tCIDCtrls::EEvResponses TLogicSrvTab::eClickHandler(TButtClickInfo& wnotClick)
{
    if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrv_AddFld)
    {
        // Pass in a list of current names
        TString strName;
        const tCIDLib::TCard4 c4Count = m_pwndFldList->c4ItemCount();
        tCIDLib::TStrList colNames(c4Count);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_pwndFldList->QueryColText(c4Index, 0, strName);
            colNames.objAdd(strName);
        }

        // Get the field type info from the user since we have to create the right type
        TCQLSrvFldType* pclsftNew = nullptr;
        TLServerCfgNewDlg dlgNewFld;
        if (dlgNewFld.bRunDlg(*this, pclsftNew, colNames))
        {
            // Add a new field, giving it the next available pseudo field id
            const tCIDLib::TCard4 c4Id = m_c4NextId++;
            pclsftNew->c4FldId(c4Id);
            m_lscfgEdit.AddField(pclsftNew);

            //
            //  And add it to the list, setting the row id to the pseudo field id. The list
            //  is sorted so we need to get back the index it went into, so that we can
            //  select it.
            //
            tCIDLib::TStrList colCols(1);
            colCols.objAdd(pclsftNew->strFldName());
            const tCIDLib::TCard4 c4Index = m_pwndFldList->c4AddItem
            (
                colCols, c4Id, kCIDLib::c4MaxCard, kCIDLib::True
            );
        }
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrv_AddSrcFld)
    {
        // Call a helper that handles this
        AddSrcField();
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrv_DelFld)
    {
        //
        //  Get the current index and, before we remove it, get the id for that row. This
        //  will be used to find the item in the original config and remove it.
        //
        const tCIDLib::TCard4 c4ListInd = m_pwndFldList->c4CurItem();
        tCIDLib::TCard4 c4Index = c4ListSelToFld(c4ListInd, kCIDLib::False);
        if (c4Index != kCIDLib::c4MaxCard)
        {
            TString strName;
            m_pwndFldList->QueryColText(c4ListInd, 0, strName);
            TYesNoBox msgbDel(facCQCAdmin.strMsg(kCQCAMsgs::midQ_Delete, TString(L"field"), strName));
            if (msgbDel.bShowIt(*this))
            {
                m_pwndFldList->RemoveAt(c4ListInd);
                m_lscfgEdit.DeleteField(c4Index);
            }
        }
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrv_DelSrcFld)
    {
        tCIDLib::TCard4 c4Index = c4ListSelToFld(m_pwndFldList->c4CurItem(), kCIDLib::False);
        const tCIDLib::TCard4 c4SrcInd = m_pwndSrcList->c4CurItem();
        if ((c4Index != kCIDLib::c4MaxCard) && (c4SrcInd != kCIDLib::c4MaxCard))
        {
            TCQLSrvFldType& clsftTar = *m_lscfgEdit.colFldTypes()[c4Index];
            TYesNoBox msgbQ
            (
                facCQCAdmin.strMsg
                (
                    kCQCAMsgs::midQ_Delete
                    , TString(L"source field")
                    , clsftTar.strSrcFldAt(c4SrcInd)
                )
            );

            if (msgbQ.bShowIt(*this))
            {
                clsftTar.RemoveField(c4SrcInd);
                m_pwndSrcList->RemoveAt(c4SrcInd);
                RenumberSrcFlds(c4Index, c4SrcInd);
            }
        }
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrv_SrcFldDn)
    {
        tCIDLib::TCard4 c4Index = c4ListSelToFld(m_pwndFldList->c4CurItem(), kCIDLib::False);
        const tCIDLib::TCard4 c4SrcInd = m_pwndSrcList->c4CurItem();
        if ((c4Index != kCIDLib::c4MaxCard) && (c4SrcInd != kCIDLib::c4MaxCard))
        {
            TCQLSrvFldType& clsftLoad = *m_lscfgEdit.colFldTypes()[c4Index];
            if (clsftLoad.bMoveField(c4SrcInd, kCIDLib::False))
            {
                if (m_pwndSrcList->bMoveDn(kCIDLib::True))
                    RenumberSrcFlds(c4Index, c4SrcInd);
            }
        }
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrv_SrcFldUp)
    {
        tCIDLib::TCard4 c4Index = c4ListSelToFld(m_pwndFldList->c4CurItem(), kCIDLib::False);
        const tCIDLib::TCard4 c4SrcInd = m_pwndSrcList->c4CurItem();
        if ((c4Index != kCIDLib::c4MaxCard) && (c4SrcInd != kCIDLib::c4MaxCard))
        {
            TCQLSrvFldType& clsftLoad = *m_lscfgEdit.colFldTypes()[c4Index];
            if (clsftLoad.bMoveField(c4SrcInd, kCIDLib::True))
            {
                if (m_pwndSrcList->bMoveUp(kCIDLib::True))
                    RenumberSrcFlds(c4Index, c4SrcInd - 1);
            }
        }
    }


    //
    //  These are the field type specific ones. These could only happen if a field is
    //  selected, so we don't have to check that here.
    //
     else if ((wnotClick.widSource() == kCQCAdmin::ridTab_LSrvMMA_Min)
          ||  (wnotClick.widSource() == kCQCAdmin::ridTab_LSrvMMA_Max)
          ||  (wnotClick.widSource() == kCQCAdmin::ridTab_LSrvMMA_Average))
    {
        // A min/max/avg is selected and they changed the type
        TCQSLLDMinMaxAvg& clsftMMA = static_cast<TCQSLLDMinMaxAvg&>(clsftSelected());
        switch(wnotClick.widSource())
        {
            case kCQCAdmin::ridTab_LSrvMMA_Min :
                clsftMMA.eValType(tCQLogicSh::EMMATypes::Minimum);
                break;

            case kCQCAdmin::ridTab_LSrvMMA_Max :
                clsftMMA.eValType(tCQLogicSh::EMMATypes::Maximum);
                break;

            case kCQCAdmin::ridTab_LSrvMMA_Average :
                clsftMMA.eValType(tCQLogicSh::EMMATypes::Average);
                break;

            default :
                CIDAssert2(L"Unknown M/M/A type")
                break;
        };
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrvOnCnt_Inverse)
    {
        TCQSLLDOnCounter& clsftOC = static_cast<TCQSLLDOnCounter&>((clsftSelected()));
        clsftOC.bInverse(wnotClick.bState());
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrvBool_Negate)
    {
        TCQSLLDBool& clsftBool = static_cast<TCQSLLDBool&>((clsftSelected()));

        //
        //  We also need to get the index for the selected source field since
        //  it's his expression we are updating.
        //
        const tCIDLib::TCard4 c4SrcIndex = m_pwndSrcList->c4CurItem();
        if (c4SrcIndex != kCIDLib::c4MaxCard)
        {
            clsftBool.SetNegatedAt(c4SrcIndex, wnotClick.bState());
        }
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrvElTm_Negate)
    {
        TCQSLLDElapsedTm& clsftElTm = static_cast<TCQSLLDElapsedTm&>(clsftSelected());

        //
        //  We also need to get the index for the selected source field since
        //  it's his expression we are updating.
        //
        const tCIDLib::TCard4 c4SrcIndex = m_pwndSrcList->c4CurItem();
        if (c4SrcIndex != kCIDLib::c4MaxCard)
            clsftElTm.SetNegatedAt(c4SrcIndex, wnotClick.bState());
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_LSrvElTm_AutoReset)
    {
        TCQSLLDElapsedTm& clsftElTm = static_cast<TCQSLLDElapsedTm&>(clsftSelected());
        clsftElTm.bAutoReset(wnotClick.bState());
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Handles stuff from our text entry related controls. We just update the related confg on
//  every character, so that we are always up to date.
//
tCIDCtrls::EEvResponses TLogicSrvTab::eEFHandler(TEFChangeInfo& wnotEvent)
{
    TCQLSrvFldType& clsftTar = clsftSelected();

    TEntryField* pwndComp = m_pwndPerType->pwndChildAs<TEntryField>(wnotEvent.widSource());
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvBool_CompVal)
    {
        const tCIDLib::TCard4 c4SrcIndex = m_pwndSrcList->c4CurItem();
        if (c4SrcIndex != kCIDLib::c4MaxCard)
        {
            // Update the comp value on the selected expression
            TCQSLLDBool& clsftBool = static_cast<TCQSLLDBool&>(clsftTar);
            TCQCExpression& exprCur = clsftBool.exprAt(c4SrcIndex);
            exprCur.strCompVal(pwndComp->strWndText());
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvElTm_CompVal)
    {
        const tCIDLib::TCard4 c4SrcIndex = m_pwndSrcList->c4CurItem();
        if (c4SrcIndex != kCIDLib::c4MaxCard)
        {
            // Update the comp value on the selected expression
            TCQSLLDElapsedTm& clsftElTm = static_cast<TCQSLLDElapsedTm&>(clsftTar);
            TCQCExpression& exprCur = clsftElTm.exprAt(c4SrcIndex);
            exprCur.strCompVal(pwndComp->strWndText());
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvOnCnt_CompVal)
    {
        const tCIDLib::TCard4 c4SrcIndex = m_pwndSrcList->c4CurItem();
        if (c4SrcIndex != kCIDLib::c4MaxCard)
        {
            // Update the comp value on the selected expression
            TCQSLLDOnCounter& clsftOC = static_cast<TCQSLLDOnCounter&>(clsftTar);
            TCQCExpression& exprCur = clsftOC.exprAt(c4SrcIndex);
            exprCur.strCompVal(pwndComp->strWndText());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TLogicSrvTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrv_FldList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndDelFld->SetEnable(kCIDLib::False);
            if (m_pwndDelFld->bHasFocus())
                m_pwndAddFld->SetDefButton();

            // Clear the source list as well
            m_pwndSrcList->RemoveAll();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            m_pwndDelFld->SetEnable(kCIDLib::True);

            // Display the info for the newly selected field
            LoadSelField();
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrv_SrcList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndDelSrcFld->SetEnable(kCIDLib::False);
            m_pwndSrcFldDn->SetEnable(kCIDLib::False);
            m_pwndSrcFldUp->SetEnable(kCIDLib::False);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            m_pwndDelSrcFld->SetEnable(kCIDLib::True);
            m_pwndSrcFldDn->SetEnable(kCIDLib::True);
            m_pwndSrcFldUp->SetEnable(kCIDLib::True);
        }

        if ((wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        ||  (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged))
        {
            LoadSrcField(wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared);
        }
    }

    //
    //  These are all per-type specific. For them to occur requires that there be a
    //  field selected, so we don't have to check for that.
    //
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvGraph_Minutes)
    {
        TCQSLLDGraph& clsftGraph = static_cast<TCQSLLDGraph&>(clsftSelected());
        clsftGraph.c4Minutes(wnotEvent.c4Index() + 1);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvOnCnt_Expr)
    {
        const tCIDLib::TCard4 c4SrcIndex = m_pwndSrcList->c4CurItem();
        if (c4SrcIndex != kCIDLib::c4MaxCard)
        {
            TCQSLLDOnCounter& clsftOC = static_cast<TCQSLLDOnCounter&>(clsftSelected());
            TCQCExpression& exprCur = clsftOC.exprAt(c4SrcIndex);
            TComboBox* pwndExpr = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvOnCnt_Expr);
            TEntryField* pwndComp = m_pwndPerType->pwndChildAs<TEntryField>(kCQCAdmin::ridTab_LSrvOnCnt_CompVal);
            StoreExprType(*pwndExpr, *pwndComp, exprCur);
        }
    }
     else if ((wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvBool_Expr)
          ||  (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvBool_LogOp))
    {
        const tCIDLib::TCard4 c4SrcIndex = m_pwndSrcList->c4CurItem();
        TCQSLLDBool& clsftBool = static_cast<TCQSLLDBool&>(clsftSelected());
        if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvBool_Expr)
        {
            if (c4SrcIndex != kCIDLib::c4MaxCard)
            {
                TCQCExpression& exprCur = clsftBool.exprAt(c4SrcIndex);
                TComboBox* pwndExpr = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvBool_Expr);
                TEntryField* pwndComp = m_pwndPerType->pwndChildAs<TEntryField>(kCQCAdmin::ridTab_LSrvBool_CompVal);
                StoreExprType(*pwndExpr, *pwndComp, exprCur);
            }
        }
         else if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvBool_LogOp)
        {
            TComboBox* pwndLogOp = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvBool_LogOp);
            clsftBool.eLogOp(tCQCKit::ELogOps(pwndLogOp->c4CurItem()));
        }
    }
     else if ((wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvElTm_Expr)
          ||  (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvElTm_LogOp))
    {
        const tCIDLib::TCard4 c4SrcIndex = m_pwndSrcList->c4CurItem();
        TCQSLLDElapsedTm& clsftElTm = static_cast<TCQSLLDElapsedTm&>(clsftSelected());
        if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvElTm_Expr)
        {
            if (c4SrcIndex != kCIDLib::c4MaxCard)
            {
                TCQCExpression& exprCur = clsftElTm.exprAt(c4SrcIndex);
                TComboBox* pwndExpr = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvElTm_Expr);
                TEntryField* pwndComp = m_pwndPerType->pwndChildAs<TEntryField>(kCQCAdmin::ridTab_LSrvElTm_CompVal);
                StoreExprType(*pwndExpr, *pwndComp, exprCur);
            }
        }
         else if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvElTm_LogOp)
        {
            TComboBox* pwndLogOp = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvElTm_LogOp);
            clsftElTm.eLogOp(tCQCKit::ELogOps(pwndLogOp->c4CurItem()));
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvRunAvg_Hours)
    {
        TCQSLLDRunAvg& clsftRunAvg = static_cast<TCQSLLDRunAvg&>(clsftSelected());
        clsftRunAvg.c4AvgHours(wnotEvent.c4Index() + 1);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Handles stuff from our text entry related controls. We just update the related config
//  on every character, so that we are always up to date.
//
tCIDCtrls::EEvResponses TLogicSrvTab::eMLEHandler(TMLEChangeInfo& wnotEvent)
{
    // For this to occur, a field has to be selected so we don't have to check for that
    TCQLSrvFldType& clsftTar = clsftSelected();

    TMultiEdit* pwndSrc = m_pwndPerType->pwndChildAs<TMultiEdit>(wnotEvent.widSource());
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvMath_Formula)
    {
        TCQSLLDMath& clsftMath = static_cast<TCQSLLDMath&>(clsftTar);
        clsftMath.strFormula(pwndSrc->strWndText());
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_LSrvPatFmt_Pat)
    {
        TCQSLLDPatFmt& clsftPat = static_cast<TCQSLLDPatFmt&>(clsftTar);
        clsftPat.strPattern(pwndSrc->strWndText());
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called to clean up the per-field type controls. We just delete the generic
//  window that contains them, then recreate it again.
//
tCIDLib::TVoid TLogicSrvTab::DeleteFldCtrls()
{
    // Save the area so we can create it again in the same place
    const TArea areaCont = m_pwndPerType->areaWnd();
    m_pwndPerType->Destroy();
    delete m_pwndPerType;

    m_pwndPerType = new TGenericWnd();
    m_pwndPerType->CreateGenWnd
    (
        *this
        , areaCont
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , kCQCAdmin::ridTab_LSrv_FldConfig
    );

    // We have to get the anchor set back on it as well
    m_pwndPerType->eEdgeAnchor(tCIDCtrls::EEdgeAnchors::MoveBottomSizeRight);
}


//
//  There are a few places where we need to load up a combo box of available expression
//  types. This is a bit of a weird one since we have a separate regular expression
//  expression type, or it can be a statement with its own list. So we put in a 0th
//  reg ex entry, then the statement types.
//
//  JUST BE careful in the list box handlers for these, to account for this weirdness.
//
tCIDLib::TVoid TLogicSrvTab::LoadExprTypes(TComboBox& wndTar)
{
    // Put in the magic reg ex one
    wndTar.c4AddItem(L"Regular Expr", 0);

    //
    //  And the statement ones. We put the statement enum value in as the row id for later
    //  convenience. As per above, none of these will have a value of zero, so we put zero
    //  in as the id for the reg ex one above. We start on the first used statements, not
    //  the min value!
    //
    tCIDLib::ForEachE<tCQCKit::EStatements, tCQCKit::EStatements::FirstUsed>
    (
        [&wndTar](const auto eStmt)
        {
            wndTar.c4AddItem(tCQCKit::strXlatEStatements(eStmt), tCIDLib::c4EnumOrd(eStmt));
            return kCIDLib::True;
        }
    );
}


//
//  When a new field is selected, this is called to load up the info for the newly selected
//  one. We have to load different info for each type of field. So we clean up the old
//  ones and create the new ones.
//
tCIDLib::TVoid TLogicSrvTab::LoadSelField()
{
    DeleteFldCtrls();

    //
    //  First look at it via the base type. Based on that we can figure out what it's type
    //  is and load the right controls to edit it.
    //
    TCQLSrvFldType& clsftLoad = clsftSelected();

    // Load the correct dialog definition
    TDlgDesc dlgdLoad;
    tCIDLib::TBoolean bSuccess = kCIDLib::True;
    if (clsftLoad.bIsA(TCQSLLDBool::clsThis()))
    {
        bSuccess = facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_LSrv_Bool, dlgdLoad);
    }
     else if (clsftLoad.bIsA(TCQSLLDElapsedTm::clsThis()))
    {
        bSuccess = facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_LSrv_ElTm, dlgdLoad);
    }
     else if (clsftLoad.bIsA(TCQSLLDMath::clsThis()))
    {
        bSuccess = facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_LSrv_Math, dlgdLoad);
    }
     else if (clsftLoad.bIsA(TCQSLLDMinMaxAvg::clsThis()))
    {
        bSuccess = facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_LSrv_MMA, dlgdLoad);
    }
     else if (clsftLoad.bIsA(TCQSLLDPatFmt::clsThis()))
    {
        bSuccess = facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_LSrv_PatFmt, dlgdLoad);
    }
     else if (clsftLoad.bIsA(TCQSLLDGraph::clsThis()))
    {
        bSuccess = facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_LSrv_Graph, dlgdLoad);
    }
     else if (clsftLoad.bIsA(TCQSLLDRunAvg::clsThis()))
    {
        bSuccess = facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_LSrv_RunAvg, dlgdLoad);
    }
     else if (clsftLoad.bIsA(TCQSLLDOnCounter::clsThis()))
    {
        bSuccess = facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_LSrv_OnCnt, dlgdLoad);
    }
     else
    {
        CIDAssert2(L"Unknown logic field class type")
    }

    if (!bSuccess)
    {
        return;
    }

    // OK, we can load the controls now
    tCIDCtrls::TWndId widInitFocus;
    m_pwndPerType->PopulateFromDlg(dlgdLoad, widInitFocus);

    //
    //  And set up any of them that are not per-source field settings, i.e. they apply to
    //  the whole logic server field. And also set up handlers for any of the per-field
    //  controls that need them.
    //
    //  And we also do any loading of lists here, whether field or source field specific
    //  generally since it's a set of choices that applies while this field is loaded.
    //
    if (clsftLoad.bIsA(TCQSLLDBool::clsThis()))
    {
        // Load the expression type combo box
        TComboBox* pwndExpr = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvBool_Expr);
        LoadExprTypes(*pwndExpr);
        pwndExpr->SelectByIndex(0);

        // Load the logical ops combo box
        TComboBox* pwndLogOp = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvBool_LogOp);
        tCQCKit::ELogOps eOp = tCQCKit::ELogOps::Min;
        for (; eOp < tCQCKit::ELogOps::Count; eOp++)
            pwndLogOp->c4AddItem(tCQCKit::strAltXlatELogOps(eOp), tCIDLib::c4EnumOrd(eOp));
        pwndLogOp->SelectByIndex(0);

        pwndLogOp->pnothRegisterHandler(this, &TLogicSrvTab::eLBHandler);
        pwndExpr->pnothRegisterHandler(this, &TLogicSrvTab::eLBHandler);
        m_pwndPerType->pwndChildAs<TEntryField>
        (
            kCQCAdmin::ridTab_LSrvBool_CompVal
        )->pnothRegisterHandler(this, &TLogicSrvTab::eEFHandler);

        m_pwndPerType->pwndChildAs<TCheckBox>
        (
            kCQCAdmin::ridTab_LSrvBool_Negate
        )->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);

        // Disable the expression related until we know we have a source field
        pwndExpr->SetEnable(kCIDLib::False);
        m_pwndPerType->SetEnable(kCQCAdmin::ridTab_LSrvBool_CompVal, kCIDLib::False);
        m_pwndPerType->SetEnable(kCQCAdmin::ridTab_LSrvBool_Negate, kCIDLib::False);
    }
     else if (clsftLoad.bIsA(TCQSLLDElapsedTm::clsThis()))
    {
        // Load the expression type combo box
        TComboBox* pwndExpr = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvElTm_Expr);
        LoadExprTypes(*pwndExpr);
        pwndExpr->SelectByIndex(0);

        // Load the logical ops combo box
        TComboBox* pwndLogOp = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvElTm_LogOp);
        tCQCKit::ELogOps eOp = tCQCKit::ELogOps::Min;
        for (; eOp < tCQCKit::ELogOps::Count; eOp++)
            pwndLogOp->c4AddItem(tCQCKit::strAltXlatELogOps(eOp), tCIDLib::c4EnumOrd(eOp));
        pwndLogOp->SelectByIndex(0);

        pwndLogOp->pnothRegisterHandler(this, &TLogicSrvTab::eLBHandler);
        pwndExpr->pnothRegisterHandler(this, &TLogicSrvTab::eLBHandler);
        m_pwndPerType->pwndChildAs<TEntryField>
        (
            kCQCAdmin::ridTab_LSrvElTm_CompVal
        )->pnothRegisterHandler(this, &TLogicSrvTab::eEFHandler);

        m_pwndPerType->pwndChildAs<TCheckBox>
        (
            kCQCAdmin::ridTab_LSrvElTm_Negate
        )->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);

        m_pwndPerType->pwndChildAs<TCheckBox>
        (
            kCQCAdmin::ridTab_LSrvElTm_AutoReset
        )->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);

        // Disable the expression related until we know we have a source field
        pwndExpr->SetEnable(kCIDLib::False);
        m_pwndPerType->SetEnable(kCQCAdmin::ridTab_LSrvElTm_CompVal, kCIDLib::False);
        m_pwndPerType->SetEnable(kCQCAdmin::ridTab_LSrvElTm_Negate, kCIDLib::False);
    }
     else if (clsftLoad.bIsA(TCQSLLDGraph::clsThis()))
    {
        // Load up the list of available minutes
        TCQSLLDGraph& clsftGraph = static_cast<TCQSLLDGraph&>(clsftLoad);
        TComboBox* pwndMins = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvGraph_Minutes);
        TString strMin;
        for (tCIDLib::TCard4 c4Val = 1; c4Val <= kCQLogicSh::c4MaxGraphCol; c4Val++)
        {
            strMin.SetFormatted(c4Val);
            pwndMins->c4AddItem(strMin);
        }
        pwndMins->SelectByIndex(clsftGraph.c4Minutes() - 1);
        pwndMins->pnothRegisterHandler(this, &TLogicSrvTab::eLBHandler);
    }
     else if (clsftLoad.bIsA(TCQSLLDMath::clsThis()))
    {
        TCQSLLDMath& clsftMath = static_cast<TCQSLLDMath&>(clsftLoad);
        TMultiEdit* pwndFormula = m_pwndPerType->pwndChildAs<TMultiEdit>(kCQCAdmin::ridTab_LSrvMath_Formula);

        pwndFormula->strWndText(clsftMath.strFormula());
        pwndFormula->pnothRegisterHandler(this, &TLogicSrvTab::eMLEHandler);
    }
     else if (clsftLoad.bIsA(TCQSLLDMinMaxAvg::clsThis()))
    {
        TCQSLLDMinMaxAvg& clsftMMA = static_cast<TCQSLLDMinMaxAvg&>(clsftLoad);

        TRadioButton* pwndMin = m_pwndPerType->pwndChildAs<TRadioButton>(kCQCAdmin::ridTab_LSrvMMA_Min);
        pwndMin->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);

        TRadioButton* pwndMax = m_pwndPerType->pwndChildAs<TRadioButton>(kCQCAdmin::ridTab_LSrvMMA_Max);
        pwndMax->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);

        TRadioButton* pwndAvg = m_pwndPerType->pwndChildAs<TRadioButton>(kCQCAdmin::ridTab_LSrvMMA_Average);
        pwndAvg->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);

        switch(clsftMMA.eValType())
        {
            case tCQLogicSh::EMMATypes::Minimum :
                pwndMin->SetCheckedState(kCIDLib::True);
                break;

            case tCQLogicSh::EMMATypes::Maximum :
                pwndMax->SetCheckedState(kCIDLib::True);
                break;

            case tCQLogicSh::EMMATypes::Average :
                pwndAvg->SetCheckedState(kCIDLib::True);
                break;
        };
    }
     else if (clsftLoad.bIsA(TCQSLLDPatFmt::clsThis()))
    {
        TCQSLLDPatFmt& clsftPat = static_cast<TCQSLLDPatFmt&>(clsftLoad);
        TMultiEdit* pwndExp = m_pwndPerType->pwndChildAs<TMultiEdit>(kCQCAdmin::ridTab_LSrvPatFmt_Pat);
        pwndExp->strWndText(clsftPat.strPattern());
        pwndExp->pnothRegisterHandler(this, &TLogicSrvTab::eMLEHandler);
    }
     else if (clsftLoad.bIsA(TCQSLLDRunAvg::clsThis()))
    {
        // Load up the list of available hours
        TCQSLLDRunAvg& clsftRunAvg = static_cast<TCQSLLDRunAvg&>(clsftLoad);
        TComboBox* pwndHrs = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvRunAvg_Hours);

        TString strHr;
        for (tCIDLib::TCard4 c4Val = 1; c4Val <= 24; c4Val++)
        {
            strHr.SetFormatted(c4Val);
            pwndHrs->c4AddItem(strHr);
        }
        pwndHrs->SelectByIndex(clsftRunAvg.c4AvgHours() - 1);
        pwndHrs->pnothRegisterHandler(this, &TLogicSrvTab::eLBHandler);
    }
     else if (clsftLoad.bIsA(TCQSLLDOnCounter::clsThis()))
    {
        TCQSLLDOnCounter& clsftOnCnt = static_cast<TCQSLLDOnCounter&>(clsftLoad);
        TCheckBox* pwndInverse = m_pwndPerType->pwndChildAs<TCheckBox>(kCQCAdmin::ridTab_LSrvOnCnt_Inverse);
        pwndInverse->SetCheckedState(clsftOnCnt.bInverse());

        // Load the expression types list
        TComboBox* pwndExpr = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvOnCnt_Expr);
        LoadExprTypes(*pwndExpr);
        pwndExpr->SelectByIndex(0);

        pwndInverse->pnothRegisterHandler(this, &TLogicSrvTab::eClickHandler);
        m_pwndPerType->pwndChildAs<TEntryField>
        (
            kCQCAdmin::ridTab_LSrvOnCnt_CompVal
        )->pnothRegisterHandler(this, &TLogicSrvTab::eEFHandler);
        pwndExpr->pnothRegisterHandler(this, &TLogicSrvTab::eLBHandler);

        // Disable the expression related until we know we have a source field
        pwndExpr->SetEnable(kCIDLib::False);
        m_pwndPerType->SetEnable(kCQCAdmin::ridTab_LSrvOnCnt_CompVal, kCIDLib::False);
    }
     else
    {
        CIDAssert2(L"Unknown logic field class type")
    }

    //
    //  We can load up the list of source fields now that everything is place. This will
    //  cause the first one in the list to be loaded into any per-source field controls we
    //  just created.
    //
    m_pwndSrcList->RemoveAll();
    const tCIDLib::TCard4 c4SrcFldCnt = clsftLoad.c4SrcFldCount();
    if (c4SrcFldCnt)
    {
        const tCQLogicSh::TSrcList& colSrcFlds = clsftLoad.colSrcFields();
        tCIDLib::TStrList colCols(2);
        colCols.objAdd(TString::strEmpty());
        colCols.objAdd(TString::strEmpty());

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcFldCnt; c4Index++)
        {
            colCols[0].SetFormatted(c4Index + 1);
            colCols[1] = colSrcFlds[c4Index];
            m_pwndSrcList->c4AddItem(colCols, 0);
        }
        m_pwndSrcList->SelectByIndex(0, kCIDLib::True);
    }
}


//
//  This is called when a new source field is selected or the source field list is cleared.
//  We have to look at the current field type and figure out which per-source field controls
//  might exist and load them or clear them, respectively.
//
//  We have a special case to check which is that the source field list is being cleared
//  before the field list was cleared, in which case we need to just delete the field related
//  controls.
//
tCIDLib::TVoid TLogicSrvTab::LoadSrcField(const tCIDLib::TBoolean bClear)
{
    // IF no field selected, then clear the per-type fields and return
    const tCIDLib::TCard4 c4FldListIndex = m_pwndFldList->c4CurItem();
    if (c4FldListIndex == kCIDLib::c4MaxCard)
    {
        DeleteFldCtrls();
        return;
    }

    // We have something, so get a ref to the config
    TCQLSrvFldType& clsftCur = clsftSelected();

    // There should be a source field index if not clearing
    const tCIDLib::TCard4 c4SrcIndex = m_pwndSrcList->c4CurItem();
    if (!bClear && (c4SrcIndex == kCIDLib::c4MaxCard))
    {
        CIDAssert2(L"There is not any source field selected")
    }

    if (clsftCur.bIsA(TCQSLLDBool::clsThis()))
    {
        TEntryField* pwndComp = m_pwndPerType->pwndChildAs<TEntryField>(kCQCAdmin::ridTab_LSrvBool_CompVal);
        TComboBox* pwndLogOp = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvBool_LogOp);
        TComboBox* pwndExpr = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvBool_Expr);
        TCheckBox* pwndNeg = m_pwndPerType->pwndChildAs<TCheckBox>(kCQCAdmin::ridTab_LSrvBool_Negate);

        if (bClear)
        {
            pwndComp->ClearText();
            pwndExpr->SelectByIndex(0);
            pwndLogOp->SelectByIndex(0);
            pwndNeg->SetEnable(kCIDLib::False);

            // Disable the expression oriented ones
            pwndComp->SetEnable(kCIDLib::False);
            pwndExpr->SetEnable(kCIDLib::False);
            pwndNeg->SetEnable(kCIDLib::False);
        }
         else
        {
            TCQSLLDBool& clsftBool = static_cast<TCQSLLDBool&>(clsftCur);

            // Make sure the expression related ones are enabled now
            pwndExpr->SetEnable(kCIDLib::True);
            pwndNeg->SetEnable(kCIDLib::True);

            // Select the logical operator
            pwndLogOp->SelectByIndex(tCIDLib::c4EnumOrd(clsftBool.eLogOp()));

            const TCQCExpression& exprCur = clsftBool.exprAt(c4SrcIndex);
            if (exprCur.bHasCompVal())
                pwndComp->strWndText(exprCur.strCompVal());
            else
                pwndComp->ClearText();
            pwndComp->SetEnable(exprCur.bHasCompVal());

            SelectExpr(*pwndExpr, exprCur);
            pwndNeg->SetCheckedState(exprCur.bNegated());
        }
    }
     else if (clsftCur.bIsA(TCQSLLDElapsedTm::clsThis()))
    {
        TEntryField* pwndComp = m_pwndPerType->pwndChildAs<TEntryField>(kCQCAdmin::ridTab_LSrvElTm_CompVal);
        TComboBox* pwndLogOp = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvElTm_LogOp);
        TComboBox* pwndExpr = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvElTm_Expr);
        TCheckBox* pwndNeg = m_pwndPerType->pwndChildAs<TCheckBox>(kCQCAdmin::ridTab_LSrvElTm_Negate);
        TCheckBox* pwndAutoReset = m_pwndPerType->pwndChildAs<TCheckBox>(kCQCAdmin::ridTab_LSrvElTm_AutoReset);

        if (bClear)
        {
            pwndAutoReset->SetCheckedState(kCIDLib::False);
            pwndNeg->SetCheckedState(kCIDLib::False);
            pwndComp->ClearText();
            pwndLogOp->SelectByIndex(0);
            pwndExpr->SelectByIndex(0);

            // Disable the expression oriented ones
            pwndComp->SetEnable(kCIDLib::False);
            pwndExpr->SetEnable(kCIDLib::False);
            pwndNeg->SetEnable(kCIDLib::False);
        }
         else
        {
            TCQSLLDElapsedTm& clsftElTm = static_cast<TCQSLLDElapsedTm&>(clsftCur);

            // Make sure the expression related ones are enabled now
            pwndExpr->SetEnable(kCIDLib::True);
            pwndNeg->SetEnable(kCIDLib::True);

            // Select the logical operator
            pwndLogOp->SelectByIndex(tCIDLib::c4EnumOrd(clsftElTm.eLogOp()));

            const TCQCExpression& exprCur = clsftElTm.exprAt(c4SrcIndex);
            if (exprCur.bHasCompVal())
                pwndComp->strWndText(exprCur.strCompVal());
            else
                pwndComp->ClearText();
            pwndComp->SetEnable(exprCur.bHasCompVal());

            SelectExpr(*pwndExpr, exprCur);
            pwndNeg->SetCheckedState(exprCur.bNegated());
            pwndAutoReset->SetCheckedState(clsftElTm.bAutoReset());
        }
    }
     else if (clsftCur.bIsA(TCQSLLDOnCounter::clsThis()))
    {
        // Inverse check box isn't involved, it's for the target field
        TEntryField* pwndComp = m_pwndPerType->pwndChildAs<TEntryField>(kCQCAdmin::ridTab_LSrvOnCnt_CompVal);
        TComboBox* pwndExpr = m_pwndPerType->pwndChildAs<TComboBox>(kCQCAdmin::ridTab_LSrvOnCnt_Expr);
        if (bClear)
        {
            pwndComp->ClearText();
            pwndExpr->SelectByIndex(0);

            // Disable the expression oriented ones
            pwndComp->SetEnable(kCIDLib::False);
            pwndExpr->SetEnable(kCIDLib::False);
        }
         else
        {
            TCQSLLDOnCounter& clsftOC = static_cast<TCQSLLDOnCounter&>(clsftCur);

            // Make sure the expression related ones are enabled now
            pwndExpr->SetEnable(kCIDLib::True);

            // Set up the expression for the selected source fields
            const TCQCExpression& exprCur = clsftOC.exprAt(c4SrcIndex);
            if (exprCur.bHasCompVal())
                pwndComp->strWndText(exprCur.strCompVal());
            else
                pwndComp->ClearText();
            pwndComp->SetEnable(exprCur.bHasCompVal());

            pwndExpr->SetEnable(kCIDLib::True);
            SelectExpr(*pwndExpr, exprCur);
        }
    }
}


//
//  We let them move the source fields up and down, but we also display the numbers since
//  they often need to refer to the source fields by their numbers. So after adding,
//  removing, or moving a source field, we have to update the numbers.
//
//  We only need to do the ones that have changed, so they give us a starting index.
//
tCIDLib::TVoid
TLogicSrvTab::RenumberSrcFlds(  const   tCIDLib::TCard4 c4Field
                                , const tCIDLib::TCard4 c4StartSrc)
{
    // Disable updates while we do this
    TWndPaintJanitor janDraw(this);

    const tCIDLib::TCard4 c4Count = m_pwndSrcList->c4ItemCount();
    TString strNum;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strNum.SetFormatted(c4Index + 1);
        m_pwndSrcList->SetColText(c4Index, 0, strNum);
    }
}


//
//  There are complications with the expression type setup, and we have to do it from a
//  few places, so it is split out here. See LoadExprTypes() for more details.
//
tCIDLib::TVoid TLogicSrvTab::SelectExpr(TComboBox& wndTar, const TCQCExpression& exprToSel)
{
    //
    //  If a regular expression, select the 0th entry. Else, select the statement'th
    //  entry. Note that we skipped the None entry, but the regular expression entry
    //  took its place, so we can still just use the statement in that case as an
    //  index.
    //
    if (exprToSel.eType() == tCQCKit::EExprTypes::RegEx)
        wndTar.SelectByIndex(0);
    else
        wndTar.SelectByIndex(tCIDLib::c4EnumOrd(exprToSel.eStatement()));
}


//
//  When we need to save a new expression selection by the user, this is called since we
//  need to do it from a few places. See LoadExprTypes() above as well. There are special
//  issues here.
//
tCIDLib::TVoid
TLogicSrvTab::StoreExprType(TComboBox& wndSrc, TEntryField& wndComp, TCQCExpression& exprTar)
{
    // Get the selected index from the list
    const tCIDLib::TCard4 c4Sel = wndSrc.c4CurItem();

    //
    //  If it's zero, then they've selected the special regular expression value. Otherwise
    //  it's a statement and the statement enum value is stored in the row.
    //
    if (c4Sel)
    {
        exprTar.eType(tCQCKit::EExprTypes::Statement);
        exprTar.eStatement(tCQCKit::EStatements(wndSrc.c4IndexToId(c4Sel)));
    }
     else
    {
        // There's no statement in this case
        exprTar.eType(tCQCKit::EExprTypes::RegEx);
        exprTar.eStatement(tCQCKit::EStatements::None);
    }

    //
    //  Enable/disable the comparison value field based on whether the expression
    //  needs one.
    //
    const tCIDLib::TBoolean bCompVal = exprTar.bHasCompVal();
    wndComp.SetEnable(bCompVal);
    if (!bCompVal)
        wndComp.ClearText();
}

