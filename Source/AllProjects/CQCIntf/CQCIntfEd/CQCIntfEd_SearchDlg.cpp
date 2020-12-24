//
// FILE NAME: CQCIntfEd_SearchDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/09/2010
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
//  This file implements the search/replace options dialog.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfEd_.hpp"
#include    "CQCIntfEd_SearchDlg_.hpp"



// ----------------------------------------------------------------------------
//  Do our RTTI macros
// ----------------------------------------------------------------------------
RTTIDecls(TIntfSearchDlg,TDlgBox)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCIntfEd_SearchRepDlg
{
    namespace
    {
        // The max previous search/replace strings we'll store in our recall lists
        constexpr tCIDLib::TCard4 c4MaxRecall = 16;
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TIntfSearchDlg
// PREFIX: dlgb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfSearchDlg: Constructors and Destructors
// ---------------------------------------------------------------------------
TIntfSearchDlg::TIntfSearchDlg() :

    m_bTmplSelected(kCIDLib::False)
    , m_pwndActParms(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndChildren(nullptr)
    , m_pwndExpressions(nullptr)
    , m_pwndFldName(nullptr)
    , m_pwndFiltParms(nullptr)
    , m_pwndFindList(nullptr)
    , m_pwndFullMatch(nullptr)
    , m_pwndImgName(nullptr)
    , m_pwndMoniker(nullptr)
    , m_pwndText(nullptr)
    , m_pwndTemplate(nullptr)
    , m_pwndRegEx(nullptr)
    , m_pwndReplace(nullptr)
    , m_pwndRepList(nullptr)
    , m_pwndURL(nullptr)
    , m_pwndVarName(nullptr)
{
}

TIntfSearchDlg::~TIntfSearchDlg()
{
}


// ---------------------------------------------------------------------------
//  TIntfSearchDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TIntfSearchDlg::bRun(           TWindow&                wndOwner
                        ,       tCIDLib::TBoolean&      bFullMatch
                        ,       tCIDLib::TBoolean&      bRegEx
                        ,       TString&                strFindStr
                        ,       TString&                strRepStr
                        ,       tCQCIntfEng::ERepFlags& eFlags
                        , const tCIDLib::TBoolean       bTmplSelected)
{
    // Remember the template selected flag for later use
    m_bTmplSelected = bTmplSelected;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_Repl
    );

    if (c4Res == kCQCIntfEd::ridDlg_Repl_Replace)
    {
        // Copy new values back to the caller's parms, since we are committing
        bFullMatch  = m_bFullMatch;
        bRegEx      = m_bRegEx;
        strFindStr  = m_strFindStr;
        strRepStr   = m_strRepStr;
        eFlags      = m_eFlags;

        // Store these as the most recent values
        s_bLastFull = m_bFullMatch;
        s_bLastRegEx = m_bRegEx;
        s_eLastFlags = m_eFlags;
        UpdateRecallList(m_strRepStr, s_colRepList);
        UpdateRecallList(m_strFindStr, s_colFindList);

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TIntfSearchDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIntfSearchDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed convenience pointers to some of our widgets
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_ActParms, m_pwndActParms);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_Children, m_pwndChildren);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_Expr, m_pwndExpressions);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_FldName, m_pwndFldName);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_FiltParms, m_pwndFiltParms);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_FullMatch, m_pwndFullMatch);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_ImgName, m_pwndImgName);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_Moniker, m_pwndMoniker);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_RegEx, m_pwndRegEx);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_FindStr, m_pwndFindList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_RepStr, m_pwndRepList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_Replace, m_pwndReplace);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_TmplName, m_pwndTemplate);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_Text, m_pwndText);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_URL, m_pwndURL);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Repl_VarName, m_pwndVarName);

    // Load up the last selected values
    LoadLastVals();

    //
    //  Install handlers for the push buttons. The rest just hold what they
    //  hold and we'll check them at the end if necessary.
    //
    m_pwndCancel->pnothRegisterHandler(this, &TIntfSearchDlg::eClickHandler);
    m_pwndReplace->pnothRegisterHandler(this, &TIntfSearchDlg::eClickHandler);

    // Indicate that the dialog box should be made visible now
    return bRet;
}


// ---------------------------------------------------------------------------
//  TIntfSearchDlg: Private, static data
// ---------------------------------------------------------------------------
tCIDLib::TBoolean       TIntfSearchDlg::s_bLastFull;
tCIDLib::TBoolean       TIntfSearchDlg::s_bLastRegEx;
tCQCIntfEng::ERepFlags  TIntfSearchDlg::s_eLastFlags(tCQCIntfEng::ERepFlags::None);
TVector<TString>        TIntfSearchDlg::s_colFindList(CQCIntfEd_SearchRepDlg::c4MaxRecall);
TVector<TString>        TIntfSearchDlg::s_colRepList(CQCIntfEd_SearchRepDlg::c4MaxRecall);


// ---------------------------------------------------------------------------
//  TIntfSearchDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TIntfSearchDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_Repl_Replace)
    {
        // Get the options together and check them for validity
        m_bFullMatch    = m_pwndFullMatch->bCheckedState();
        m_bRegEx        = m_pwndRegEx->bCheckedState();
        m_strFindStr   = m_pwndFindList->strWndText();
        m_strRepStr    = m_pwndRepList->strWndText();

        // The find string cannot be empty if reg ex or not full match
        if (m_strFindStr.bIsEmpty() && (m_bRegEx || !m_bFullMatch))
        {
            // Make sure that they really want to remove it
            TErrBox msgError
            (
                facCQCIGKit().strMsg(kIGKitMsgs::midStatus_SearchEmptyStr)
            );
            msgError.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        // If a reg ex, make sure it's valid
        if (m_bRegEx)
        {
            try
            {
                TRegEx regxTest;
                regxTest.SetExpression(m_strFindStr);
            }

            catch(const TError&)
            {
                TErrBox msgError
                (
                    facCQCIGKit().strMsg(kIGKitMsgs::midStatus_BadRegEx)
                );
                msgError.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        m_eFlags = eQueryFlags();

        // Make sure some replace flags are set
        if ((m_eFlags == tCQCIntfEng::ERepFlags::Children)
        ||  (m_eFlags == tCQCIntfEng::ERepFlags::None))
        {
            TErrBox msgError(facCQCIntfEd().strMsg(kIEdMsgs::midStatus_NoReplFlags));
            msgError.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }
        EndDlg(kCQCIntfEd::ridDlg_Repl_Replace);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_Repl_Cancel)
    {
        EndDlg(kCQCIntfEd::ridDlg_Repl_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Goes through all of the query flag check boxes and creates the bitmapped
//  enum value for the actual replace flags.
//
tCQCIntfEng::ERepFlags TIntfSearchDlg::eQueryFlags()
{
    tCQCIntfEng::ERepFlags eRes = tCQCIntfEng::ERepFlags::None;

    if (m_pwndActParms->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::ActionParms);

    if (m_pwndExpressions->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::Expressions);

    if (m_pwndFldName->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::AssocField);

    if (m_pwndFiltParms->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::FilterParms);

    if (m_pwndImgName->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::Image);

    if (m_pwndMoniker->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::AssocDevice);

    if (m_pwndText->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::Caption);

    if (m_pwndTemplate->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::Template);

    if (m_pwndURL->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::URL);

    if (m_pwndVarName->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::AssocVar);

    if (m_pwndChildren->bCheckedState())
        eRes = tCIDLib::eOREnumBits(eRes, tCQCIntfEng::ERepFlags::Children);

    return eRes;
}


//
//  Called on entry to load up the previous selected values last time we were
//  invoked.
//
tCIDLib::TVoid TIntfSearchDlg::LoadLastVals()
{
    // Load the recall lists
    tCIDLib::TCard4 c4ListCnt = s_colFindList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ListCnt; c4Index++)
        m_pwndFindList->c4AddItem(s_colFindList[c4Index], c4Index);

    c4ListCnt = s_colRepList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ListCnt; c4Index++)
        m_pwndRepList->c4AddItem(s_colRepList[c4Index], c4Index);

    // Set the replacement flags
    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::ActionParms))
        m_pwndActParms->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::Expressions))
        m_pwndExpressions->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::AssocField))
        m_pwndFldName->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::FilterParms))
        m_pwndFiltParms->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::Image))
        m_pwndImgName->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::AssocDevice))
        m_pwndMoniker->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::Caption))
        m_pwndText->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::Template))
        m_pwndTemplate->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::URL))
        m_pwndURL->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::AssocVar))
        m_pwndVarName->SetCheckedState(kCIDLib::True);

    if (tCIDLib::bAllBitsOn(s_eLastFlags, tCQCIntfEng::ERepFlags::Children))
        m_pwndChildren->SetCheckedState(kCIDLib::True);

    //
    //  If not in template selected mode, disable the children option to make
    //  it clear it's not relevant.
    //
    if (!m_bTmplSelected)
    {
        m_pwndChildren->SetCheckedState(kCIDLib::False);
        m_pwndChildren->SetEnable(kCIDLib::False);
    }

    // Set the options
    m_pwndFullMatch->SetCheckedState(s_bLastFull);
    m_pwndRegEx->SetCheckedState(s_bLastRegEx);
}


tCIDLib::TVoid
TIntfSearchDlg::UpdateRecallList(const  TString&            strNewVal
                                ,       tCIDLib::TStrList&  colList)
{
    // If the list is full, dump the oldest one
    if (colList.c4ElemCount() == CQCIntfEd_SearchRepDlg::c4MaxRecall)
        colList.bRemoveLast();

    //
    //  If this value is already in the list, move it to the top. If not, add
    //  it as new value.
    //
    tCIDLib::TCard4 c4Index = 0;
    tCIDLib::TCard4 c4ListCnt = colList.c4ElemCount();
    for (; c4Index < c4ListCnt; c4Index++)
    {
        if (colList[c4Index] == strNewVal)
            break;
    }

    if (c4Index == c4ListCnt)
    {
        // It's a new value, so insert this one, making room if needed
        if (colList.c4ElemCount() == CQCIntfEd_SearchRepDlg::c4MaxRecall)
            colList.bRemoveLast();
        colList.InsertAt(strNewVal, 0);
    }
     else if (c4Index)
    {
        //
        //  It's already in the list, and not the first value, so move it to
        //  the top of the list.
        //
        colList.RemoveAt(c4Index);
        colList.InsertAt(strNewVal, 0);
    }
}

