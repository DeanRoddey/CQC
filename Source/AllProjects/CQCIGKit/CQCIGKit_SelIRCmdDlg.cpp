//
// FILE NAME: CQCGKit_SelIRCmdDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2006
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
//  This file implements a dialog that let's the user choose an IR model.cmd
//  value.
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
#include    "CQCIGKit_.hpp"
#include    "CQCIGKit_SelIRCmdDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSelIRCmdDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCQCSelIRCmdDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSelIRCmdDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSelIRCmdDlg::TCQCSelIRCmdDlg() :

    m_pwndCancelButton(0)
    , m_pwndCmds(0)
    , m_pwndModels(0)
    , m_pwndSelectButton(0)
{
}

TCQCSelIRCmdDlg::~TCQCSelIRCmdDlg()
{
    // Call our own destroy method
    Destroy();
}


// ---------------------------------------------------------------------------
//  TCQCSelIRCmdDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCSelIRCmdDlg::bRunDlg(const  TWindow&    wndOwner
                        , const TString&    strDriver
                        ,       TString&    strToFill)
{
    //
    //  If there is any incoming value, we break out the model, command, and
    //  zone info and store it for later use. The first two are used for initial
    //  selection, and the zone info is put back onto the selection at the
    //  end.
    //
    m_strInModel = strToFill;
    if (!m_strInModel.bIsEmpty())
    {
        if (!m_strInModel.bSplit(m_strInCmd, kCIDLib::chPeriod))
        {
            // It can't be valid so just reset
            m_strInCmd.Clear();
            m_strInZone.Clear();
        }
         else
        {
            // See if there is any zone info
            m_strInCmd.bSplit(m_strInZone, kCIDLib::chColon);
        }
    }

    //
    //  We need to contact the indicated driver and ask for a list of it's
    //  device models and their commands.
    //
    try
    {
        tCQCKit::TCQCSrvProxy orbcDev = facCQCKit().orbcCQCSrvAdminProxy(strDriver);
        TString strData;
        tCIDLib::TBoolean bRes = orbcDev->bQueryTextVal
        (
            strDriver, kCQCIR::strQueryDevCmdList, TString::strEmpty(), m_strData
        );

        if (!bRes)
        {
            TOkBox msgbOK
            (
                wndOwner.strWndText()
                , facCQCIGKit().strMsg(kIGKitMsgs::midStatus_NoModelsLoaded, strDriver)
            );
            msgbOK.ShowIt(wndOwner);
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this
            , facCQCIGKit().strMsg(kIGKitMsgs::midStatus_CantQueryIRModels, strDriver)
            , errToCatch
        );
        return kCIDLib::False;
    }

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_SelIRCmd
    );

    if (c4Res == kCQCIGKit::ridDlg_SelIRCmd_Select)
    {
        strToFill = tCIDLib::ForceMove(m_strSelection);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCSelIRCmdDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCSelIRCmdDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();;

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelIRCmd_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelIRCmd_Cmds, m_pwndCmds);
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelIRCmd_Models, m_pwndModels);
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelIRCmd_Select, m_pwndSelectButton);

    //
    //  Set up the columns of the two list boxes. We use multi-column list boxes
    //  but with just one column, since the regular list boxes suck.
    //
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_SelIRCmd_ModelPref));
    m_pwndModels->SetColumns(colCols);
    colCols[0] = facCQCIGKit().strMsg(kIGKitMsgs::midDlg_SelIRCmd_CmdPref);
    m_pwndCmds->SetColumns(colCols);

    //
    //  The data we got before the dialog was run is in the form:
    //
    //  Model=xxx
    //  cmd1
    //  cmd2
    //  cmdx
    //
    //  Model=yyy
    //  cmd1
    //  cmdx
    //
    //  So we parse through this and load up our model list. We let the list sort,
    //  and set each item's id to the original index.
    //
    TTextStringInStream strmSrc(&m_strData);
    TString strCurLn;
    tCIDLib::TStrList colEmpty;

    while (!strmSrc.bEndOfStream())
    {
        strmSrc >> strCurLn;

        // If an empty line, we skip it
        strCurLn.StripWhitespace();
        if (strCurLn.bIsEmpty())
            continue;

        // If it starts with Model=, we start a new model
        if (strCurLn.bStartsWith(L"Model="))
        {
            strCurLn.Cut(0, 6);

            //
            //  Add it to the models list box. Store the index into the model list
            //  for this one. Do it before we update the models list so that the
            //  current count is the index.
            //
            colCols[0] = strCurLn;
            m_pwndModels->c4AddItem(colCols, m_colModels.c4ElemCount());

            // And add an entry to each of our data structures
            m_colModels.objAdd(strCurLn);
            m_colCmds.objAdd(colEmpty);
        }
         else if (!m_colCmds.bIsEmpty())
        {
            m_colCmds[m_colModels.c4ElemCount() - 1].objAdd(strCurLn);
        }
    }

    //
    //  Go through the command lists and sort them so we don't have to do it
    //  ever time one is loaded.
    //
    const tCIDLib::TCard4 c4ModelCnt = m_colModels.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ModelCnt; c4Index++)
        m_colCmds[c4Index].Sort(&TString::eComp);

    //
    //  If there's an incoming model and command, select those. Else just keep
    //  the defaults we have.
    //
    LoadCmds(m_strInModel, m_strInCmd);

    // Register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TCQCSelIRCmdDlg::eClickHandler);
    m_pwndModels->pnothRegisterHandler(this, &TCQCSelIRCmdDlg::eLBHandler);
    m_pwndSelectButton->pnothRegisterHandler(this, &TCQCSelIRCmdDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCSelIRCmdDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TCQCSelIRCmdDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_SelIRCmd_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_SelIRCmd_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_SelIRCmd_Select)
    {
        //
        //  The model can be looked at as a string item since the templatized
        //  version we actually used derives from it.
        //
        const tCIDLib::TCard4 c4ModelInd = m_pwndModels->c4CurItem();
        const tCIDLib::TCard4 c4CmdInd = m_pwndCmds->c4CurItem();
        if ((c4ModelInd != kCIDLib::c4MaxCard)
        &&  (c4CmdInd != kCIDLib::c4MaxCard))
        {
            TString strModel;
            m_pwndModels->QueryColText(c4ModelInd, 0, strModel);
            TString strCmd;
            m_pwndCmds->QueryColText(c4CmdInd, 0, strCmd);

            m_strSelection = strModel;
            m_strSelection.Append(kCIDLib::chPeriod);
            m_strSelection.Append(strCmd);

            // If there was incoming zone info, put that back
            if (!m_strInZone.bIsEmpty())
            {
                m_strSelection.Append(kCIDLib::chColon);
                m_strSelection.Append(m_strInZone);
            }
            EndDlg(kCQCIGKit::ridDlg_SelIRCmd_Select);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Load new command lists when we get a new selection in the model list. The
//  model list is the only wired up so we don't have to check for the source
//  list here.
//
tCIDCtrls::EEvResponses TCQCSelIRCmdDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        TString strModel;
        m_pwndModels->QueryColText(wnotEvent.c4Index(), 0, strModel);
        LoadCmds(strModel, TString::strEmpty());
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  If there is such an indicated model, it will find it and load it, and if that has
//  such an indicated command, that will be selected. If either is not found (or empty)
//  the 0th entry (if available) will be taken.
//
tCIDLib::TVoid
TCQCSelIRCmdDlg::LoadCmds(const TString& strModel, const TString& strSelCmd)
{
    // If no models available, do nothing
    if (!m_pwndModels->c4ItemCount())
        return;

    // Try to find the indicated model. If not, take the 0th one
    tCIDLib::TCard4 c4ListInd;
    if (!m_pwndModels->bSelectByText(strModel, c4ListInd))
        c4ListInd = 0;

    // Get the list item's id which is the original model list index
    tCIDLib::TCard4 c4ModelInd = m_pwndModels->c4IndexToId(c4ListInd);

    // Shouldn't happen but check it just in case that we got a valid orignal index
    if (c4ModelInd < m_colCmds.c4ElemCount())
    {
        // Looks like we have it, so let's load the list
        TWndPaintJanitor janDraw(m_pwndCmds);

        // Clear out any current contents
        m_pwndCmds->RemoveAll();

        const tCIDLib::TStrList& colCmds = m_colCmds[c4ModelInd];
        const tCIDLib::TCard4 c4Count = colCmds.c4ElemCount();
        if (c4Count)
        {
            tCIDLib::TStrList colCols(1);
            colCols.objAdd(TString::strEmpty());
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                colCols[0] = colCmds[c4Index];
                m_pwndCmds->c4AddItem(colCols, c4Index);
            }

            // Try to select the incoming command if not empty
            if (!strSelCmd.bIsEmpty())
            {
                tCIDLib::TCard4 c4At;
                if (!m_pwndCmds->bSelectByText(strSelCmd, c4At))
                    m_pwndCmds->SelectByIndex(0);
            }
        }
    }
}

