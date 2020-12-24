//
// FILE NAME: ZWaveLeviC_EdGroupsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/14/2012
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
//  This file implements the groups editor dialog.
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
#include    "ZWaveLeviC_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveEdGroupsDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveEdGroupsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveEdGroupsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveEdGroupsDlg::TZWaveEdGroupsDlg() :

    m_colEdit(kZWaveLeviSh::c4MaxGroupId)
    , m_pwndCancel(nullptr)
    , m_pwndDelete(nullptr)
    , m_pwndDeleteAll(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSave(nullptr)
{
}

TZWaveEdGroupsDlg::~TZWaveEdGroupsDlg()
{

}


// ---------------------------------------------------------------------------
//  TZWaveEdGroupsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TZWaveEdGroupsDlg::bRun(const   TWindow&            wndOwner
                        , const TString&            strMoniker
                        ,       TZWaveDrvConfig&    dcfgEdit)
{
    //
    //  Save the incoming moniker. We won't need the configuration data
    //  again until we get back here so no need to save that.
    //
    m_strMoniker = strMoniker;

    // Make a copy of the current group names
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLeviSh::c4MaxGroupId; c4Index++)
        m_colEdit.objAdd(dcfgEdit.strGroupAt(c4Index));

    //
    //  Run the dialog. If successful and we get a positive response, put
    //  back the modified list.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facZWaveLeviC(), kZWaveLeviC::ridDlg_EdGrps
    );

    if (c4Res == kZWaveLeviC::ridDlg_EdGrps_Save)
    {
        //
        //  Set the group info back on the configuration. He provides a
        //  method for us to do this.
        //
        dcfgEdit.SetGroups(m_colEdit);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TZWaveEdGroupsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWaveEdGroupsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveLeviC::ridDlg_EdGrps_Cancel, m_pwndCancel);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_EdGrps_Delete, m_pwndDelete);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_EdGrps_DeleteAll, m_pwndDeleteAll);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_EdGrps_List, m_pwndList);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_EdGrps_Save, m_pwndSave);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TZWaveEdGroupsDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TZWaveEdGroupsDlg::eLBHandler);
    m_pwndDelete->pnothRegisterHandler(this, &TZWaveEdGroupsDlg::eClickHandler);
    m_pwndDeleteAll->pnothRegisterHandler(this, &TZWaveEdGroupsDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TZWaveEdGroupsDlg::eClickHandler);

    //
    //  Now load up our list. We just load them in order and put the number
    //  before them for the user's convenience.
    //
    LoadGrpList();
    return bRet;
}


// ---------------------------------------------------------------------------
//  TZWaveEdGroupsDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  WE just go through the collection and empty all the strings, then call
//  the list loader to reaload.
//
tCIDLib::TVoid TZWaveEdGroupsDlg::DeleteAllGroups()
{
    TYesNoBox msgbAsk(facZWaveLeviC().strMsg(kZWCMsgs::midQ_DelAllUnits));

    if (msgbAsk.bShowIt(*this))
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLeviSh::c4MaxGroupId; c4Index++)
            m_colEdit[c4Index].Clear();
        LoadGrpList();
    }
}


//
//  Called when they click the remove button. We just clear the name for that
//  one.
//
tCIDLib::TVoid TZWaveEdGroupsDlg::DeleteGroup()
{
    // Shouldn't be possible not to have one selected, but just in case
    const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
    if (c4Index != kCIDLib::c4MaxCard)
    {
        if (!m_colEdit[c4Index].bIsEmpty())
        {
            TYesNoBox msgbAsk
            (
                facZWaveLeviC().strMsg(kZWCMsgs::midQ_DelUnit, m_colEdit[c4Index])
            );

            if (msgbAsk.bShowIt(*this))
            {
                m_colEdit[c4Index].Clear();

                TString strText;
                FmtTextAt(c4Index, strText);
                m_pwndList->c4SetText(c4Index, strText);
            }
        }
    }
}


// Called when they double click on the list, we let them edit the name
tCIDLib::TVoid TZWaveEdGroupsDlg::EditGroup()
{
    // Shouldn't be possible not to have one selected, but just in case
    const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return;

    TString strNew;
    const tCIDLib::TBoolean bRes = facCIDWUtils().bGetText
    (
        *this
        , facZWaveLeviC().strMsg(kZWCMsgs::midPrompt_GroupName)
        , m_colEdit[c4Index]
        , strNew
    );

    if (bRes && !strNew.bIsEmpty())
    {
        //
        //  See if that name is already in the list. If so, and it's not at
        //  the same index, then it's a dup.
        //
        tCIDLib::TCard4 c4FoundAt = 0;
        for (; c4FoundAt < kZWaveLeviSh::c4MaxGroupId; c4FoundAt++)
        {
            if (m_colEdit[c4FoundAt] == strNew)
                break;
        }

        if (c4FoundAt == c4Index)
        {
            // It's the same group, so do nothing
        }
         else if (c4FoundAt != kZWaveLeviSh::c4MaxGroupId)
        {
            // It's a dup, so reject it
            TErrBox msgbDup(facZWaveLeviSh().strMsg(kZWShErrs::errcCfg_DupGroup, strNew));
            msgbDup.ShowIt(*this);
        }
         else
        {
            // It's ok so update
            m_colEdit[c4Index] = strNew;
            FmtTextAt(c4Index, strNew);
            m_pwndList->c4SetText(c4Index, strNew);
        }
    }
}


// Our button click handler
tCIDCtrls::EEvResponses TZWaveEdGroupsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveLeviC::ridDlg_EdGrps_Cancel)
        EndDlg(kZWaveLeviC::ridDlg_EdGrps_Cancel);
    else if (wnotEvent.widSource() == kZWaveLeviC::ridDlg_EdGrps_Delete)
        DeleteGroup();
    else if (wnotEvent.widSource() == kZWaveLeviC::ridDlg_EdGrps_DeleteAll)
        DeleteAllGroups();
    else if (wnotEvent.widSource() == kZWaveLeviC::ridDlg_EdGrps_Save)
        EndDlg(kZWaveLeviC::ridDlg_EdGrps_Save);

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We handle double clicks in the group list, to edit the group.
//
tCIDCtrls::EEvResponses TZWaveEdGroupsDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    // Watch for invoke operations
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        if (wnotEvent.widSource() == kZWaveLeviC::ridDlg_EdGrps_List)
            EditGroup();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  There are few places we have to set up or modify the text displayed for
//  a given item. So we break it out into a helper that will return the
//  formatted text for the group at a particular slot.
//
tCIDLib::TVoid
TZWaveEdGroupsDlg::FmtTextAt(const  tCIDLib::TCard4 c4At
                            ,       TString&        strToFill)
{
    strToFill.Clear();

    if (c4At < 100)
        strToFill.Append(L"  ");
    else if (c4At < 10)
        strToFill.Append(L" ");
    strToFill.AppendFormatted(c4At + 1);
    strToFill.Append(L". ");
    strToFill.Append(m_colEdit[c4At]);
}


//
//  Load up all of the groups (empty or set) into the list. We number them
//  for the user's convenience.
//
tCIDLib::TVoid TZWaveEdGroupsDlg::LoadGrpList()
{
    TString strText;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLeviSh::c4MaxGroupId; c4Index++)
    {
        FmtTextAt(c4Index, strText);
        m_pwndList->c4AddItem(strText, c4Index);
    }
    m_pwndList->SelectByIndex(0);
}


