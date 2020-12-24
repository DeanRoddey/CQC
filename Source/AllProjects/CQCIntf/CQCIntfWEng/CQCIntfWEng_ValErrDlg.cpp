//
// FILE NAME: CQCIntfWEng_ValErrDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/23/2008
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
//  This file implements a dialog that displays validation errors to the
//  user, when a template load has non-fatal errors or when in the designer
//  the user asks for a validation pass.
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
#include    "CQCIntfWEng_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCValErrDlg,TDlgBox)



// ---------------------------------------------------------------------------
//  CLASS: TValListOps
// PREFIX: lbops
//
//  A list box ops derivative tomake it easy to load up the list of errors to
//  a list box.
// ---------------------------------------------------------------------------
class TValListOps : public TListBoxOps
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructors
        // -------------------------------------------------------------------
        TValListOps(const tCQCIntfEng::TErrList* const pcolErrs) :

            m_pcolErrs(pcolErrs)
        {
        }

        TValListOps(const TValListOps&) = delete;

        ~TValListOps() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TValListOps& operator=(const TValListOps&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4RowCount() const override
        {
            return m_pcolErrs->c4ElemCount();
        }

        tCIDLib::TCard4 c4RowId(const tCIDLib::TCard4 c4RowIndex) const override
        {
            return m_pcolErrs->objAt(c4RowIndex).c4ErrId();
        }

        const TString& strRowText(const tCIDLib::TCard4 c4RowIndex) const override
        {
            return m_pcolErrs->objAt(c4RowIndex).strErrText();
        }

        const tCQCIntfEng::TErrList*    m_pcolErrs;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCValErrDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCValErrDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCValErrDlg::TCQCValErrDlg() :

    m_pcolErrs(0)
    , m_pwndCloseButton(0)
    , m_pwndList(0)
    , m_pwndReport(0)
{
}

TCQCValErrDlg::~TCQCValErrDlg()
{
    // Call our own destroy method
    Destroy();
}


// ---------------------------------------------------------------------------
//  TCQCValErrDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCValErrDlg::RunDlg(  const   TWindow&                wndOwner
                        ,       TWindow* const          pwndReport
                        , const tCQCIntfEng::TErrList&  colErrs
                        , const TString&                strTmplName)
{
    // Store the incoming stuff till we can use it
    m_pcolErrs = &colErrs;
    m_pwndReport = pwndReport;
    m_strTmplName = strTmplName;

    c4RunDlg(wndOwner, facCQCIntfWEng(), kCQCIntfWEng::ridDlg_ValErrs);
}


// ---------------------------------------------------------------------------
//  TCQCValErrDlg: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCValErrDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCIntfWEng::ridDlg_ValErrs_Close, m_pwndCloseButton);
    CastChildWnd(*this, kCQCIntfWEng::ridDlg_ValErrs_List, m_pwndList);

    // Set the title bar text which needs to include the template name
    TString strTitle(strWndText());
    strTitle.eReplaceToken(m_strTmplName, L'1');
    strWndText(strTitle);

    // And register our event handlers
    m_pwndCloseButton->pnothRegisterHandler(this, &TCQCValErrDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TCQCValErrDlg::eLBHandler);

    //
    //  Load up the incoming errors. We allow the user to remove them from the
    //  list after they are dealt with, so we store the unique error id as the
    //  user data on each item.
    //
    TValListOps lbopsLoad(m_pcolErrs);
    m_pwndList->LoadList(lbopsLoad);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCValErrDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handles button clicks
tCIDCtrls::EEvResponses
TCQCValErrDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    // We only have one close button, so just exit
    EndDlg(0);
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses
TCQCValErrDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    //
    //  According to whether they double clicked or cursored, we send a
    //  message to the report window if we got one.
    //
    const TCQCIntfValErrInfo* pveiCur = pveiFindById(wnotEvent.c4Id());
    if (pveiCur && m_pwndReport)
    {
        tCIDLib::TInt4 i4Code = 0;
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
            i4Code = facCQCIntfWEng().i4ErrSelCode();
        else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
            i4Code = facCQCIntfWEng().i4ErrInvokeCode();

        if (i4Code)
        {
            if (pveiCur->eType() == tCQCIntfEng::EValErrTypes::State)
            {
                // It's an error in a state

            }
             else if (pveiCur->eType() == tCQCIntfEng::EValErrTypes::Widget)
            {
                // It's an error in a widget, so get the id out
                const tCIDLib::TCard4 c4WidgetId = pveiCur->c4WidgetId();

                //
                //  We send him the widget id. We send the code we were given
                //  to indicate what happened, and we send the widget id as
                //  the value. This is sent async since we take the default
                //  last parameter.
                //
                m_pwndReport->SendCode(i4Code, c4WidgetId);

                // Ok, let's remove this one from the list if an invoke
                if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
                    m_pwndList->RemoveAt(wnotEvent.c4Index());
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Go through the errors and find the one that has this id
const TCQCIntfValErrInfo* TCQCValErrDlg::pveiFindById(const tCIDLib::TCard4 c4Id) const
{
    const TCQCIntfValErrInfo* pveiRet = nullptr;
    const tCIDLib::TCard4 c4Count = m_pcolErrs->c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfValErrInfo& veiCur = m_pcolErrs->objAt(c4Index);
        if (veiCur.c4ErrId() == c4Id)
        {
            pveiRet = &veiCur;
            break;
        }
    }
    return pveiRet;
}

