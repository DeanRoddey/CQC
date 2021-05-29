//
// FILE NAME: CQCIGKit_EditCmdDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/02/2002
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
//  This file implements the standard action editor dialog.
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
#include    "CQCIGKit_EditCmdDlg_.hpp"


// ---------------------------------------------------------------------------
//  Local types and data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIGKit_EditCmdDlg
    {
        // -----------------------------------------------------------------------
        //  The last size/pos of the dialog, which we will use when reloaded.
        //  Default it to the default zero values.
        // -----------------------------------------------------------------------
        TArea   areaLastPos;


        // -----------------------------------------------------------------------
        //  Ids we use in the parameter popup menu for the dynamic content, which has
        //  to be outside of any of the fixed ids.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4 c4InsRTVSub       = 1000;
        constexpr tCIDLib::TCard4 c4InsRTVFirst     = 1001;
        constexpr tCIDLib::TCard4 c4InsRTVLast      = 1399;

        constexpr tCIDLib::TCard4 c4InsActParmSub   = 1400;
        constexpr tCIDLib::TCard4 c4InsActParmFirst = 1401;
        constexpr tCIDLib::TCard4 c4InsActParmLast  = 1408;
    }
}


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCEditCmdDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCQCEditCmdDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCEditCmdDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCEditCmdDlg::TCQCEditCmdDlg() :

    m_bRequired(kCIDLib::False)
    , m_c4CurNesting(0)
    , m_c4CurEvIndex(0)
    , m_colCmdCache(109, TStringKeyOps())
    , m_colTargets(tCIDLib::EAdoptOpts::NoAdopt)
    , m_pcolCurEvOps(nullptr)
    , m_pmcsrcOrg(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndChangeButton(nullptr)
    , m_pwndCmdButton(nullptr)
    , m_pwndCommentButton(nullptr)
    , m_pwndCopyAllButton(nullptr)
    , m_pwndCopyButton(nullptr)
    , m_pwndCutButton(nullptr)
    , m_pwndDelAllButton(nullptr)
    , m_pwndDisableButton(nullptr)
    , m_pwndDnButton(nullptr)
    , m_pwndEvents(nullptr)
    , m_pwndIfButton(nullptr)
    , m_pwndIfElseButton(nullptr)
    , m_pwndOpList(nullptr)
    , m_pwndPasteButton(nullptr)
    , m_pwndPasteAllButton(nullptr)
    , m_pwndReplaceButton(nullptr)
    , m_pwndResetButton(nullptr)
    , m_pwndSaveButton(nullptr)
    , m_pwndUpButton(nullptr)
    , m_strSlash1(L"\\")
    , m_strSlash2(L"\\\\")
{
    // Initialize our per-parameter window pointer arrays
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCKit::c4MaxCmdParms; c4Index++)
    {
        m_apwndPrefs[c4Index] = 0;
        m_apwndTexts[c4Index] = 0;
    }
}

TCQCEditCmdDlg::~TCQCEditCmdDlg()
{
}


// ---------------------------------------------------------------------------
//  TCQCEditCmdDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Note that the command content will be modified directly.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bRun(const  TWindow&                    wndOwner
                    ,       MCQCCmdSrcIntf&             mcsrcEdit
                    , const tCIDLib::TBoolean           bRequired
                    , const TCQCUserCtx&                cuctxToUse
                    ,       tCQCKit::TCmdTarList* const pcolExtraTars)
{
    // Save info for later use
    m_bRequired = bRequired;
    m_cuctxToUse = cuctxToUse;
    m_pmcsrcOrg = &mcsrcEdit;

    // Make a copy of the actions for editing, leaving the original for comparison
    static_cast<MCQCCmdSrcIntf&>(m_csrcEdit) = mcsrcEdit;

    //
    //  Set up the standard targets and load up a collection of them. The global vars
    //  can be unsafe here since we don't really use them to run the action.
    //
    TEvSrvCmdTar        ctarEventSrv;
    TStdFieldCmdTar     ctarFlds;
    TStdVarsTar         ctarGlobals(tCIDLib::EMTStates::Unsafe, kCIDLib::False);
    TStdVarsTar         ctarLocals(tCIDLib::EMTStates::Unsafe, kCIDLib::True);
    TStdMacroCmdTar     ctarMacro;
    TStdSystemCmdTar    ctarSystem;

    m_colTargets.Add(&ctarEventSrv);
    m_colTargets.Add(&ctarFlds);
    m_colTargets.Add(&ctarGlobals);
    m_colTargets.Add(&ctarLocals);
    m_colTargets.Add(&ctarMacro);
    m_colTargets.Add(&ctarSystem);

    // If the user passed in any extra targets, add them
    if (pcolExtraTars)
    {
        const tCIDLib::TCard4 c4ETCount = pcolExtraTars->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ETCount; c4Index++)
            m_colTargets.Add(pcolExtraTars->pobjAt(c4Index));
    }

    //
    //  Loop through all of the events and their commands. If any of the targets referenced
    //  (or their commands) are not found, then convert those steps into comments.
    //
    tCIDLib::TBoolean   bLostCmds = kCIDLib::False;
    TCQCCmd             cmdTmp;
    TString             strComment;

    const tCIDLib::TCard4 c4EventCnt = m_csrcEdit.c4EventCount();
    for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EventCnt; c4EvIndex++)
    {
        // Get the opcode list for this event
        const TString& strEventId = m_csrcEdit.caeiEventAt(c4EvIndex).m_strEventId;
        MCQCCmdSrcIntf::TOpcodeBlock* pcolOps
        (
            m_csrcEdit.pcolOpsForEvent(strEventId)
        );

        const tCIDLib::TCard4 c4OpCount = pcolOps->c4ElemCount();
        for (tCIDLib::TCard4 c4OpIndex = 0; c4OpIndex < c4OpCount; c4OpIndex++)
        {
            TActOpcode& aocCur = pcolOps->objAt(c4OpIndex);

            // If a comment, skip it
            if (aocCur.m_eOpcode == tCQCKit::EActOps::Comment)
                continue;

            if ((aocCur.m_eOpcode == tCQCKit::EActOps::Cmd)
            ||  (aocCur.m_eOpcode == tCQCKit::EActOps::If))
            {
                TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;
                MCQCCmdTarIntf* pmctarCur;
                if (!bFindCmd(ccfgCur, strEventId, cmdTmp, pmctarCur))
                {
                    strComment.LoadFromMsg
                    (
                        kIGKitErrs::errcCmd_TarNotFound
                        , facCQCIGKit()
                        , ccfgCur.strTargetId()
                        , ccfgCur.strCmdId()
                    );

                    // Remember if it was an If before we change it
                    const tCIDLib::TBoolean bWasIf
                    (
                        aocCur.m_eOpcode == tCQCKit::EActOps::If
                    );

                    aocCur.Reset(tCQCKit::EActOps::Comment);
                    ccfgCur.c4ParmCnt(1);
                    ccfgCur.SetParmAt(0, strComment, tCQCKit::ECmdPTypes::Text);
                    bLostCmds = kCIDLib::True;

                    //
                    //  If it's an If or IfElse, then we have to find the
                    //  associated Else and End and convert them as well.
                    //
                    if (bWasIf)
                    {
                        // Find the Else or End
                        tCIDLib::TCard4 c4CondInd = c4FindCondEnd
                        (
                            *pcolOps, c4OpIndex, kCIDLib::False
                        );

                        //
                        //  Convert this one. Remember first if it's an Else,
                        //  since that means we have to find the end still.
                        //
                        TActOpcode& aocFirst = pcolOps->objAt(c4CondInd);
                        const tCIDLib::TBoolean bDoEnd
                        (
                            aocFirst.m_eOpcode == tCQCKit::EActOps::Else
                        );
                        aocFirst.m_eOpcode = tCQCKit::EActOps::Comment;
                        aocFirst.m_ccfgStep.c4ParmCnt(1);
                        aocFirst.m_ccfgStep.SetParmAt(0, strComment, tCQCKit::ECmdPTypes::Text);

                        // If it's an IfElse, then find the End as well
                        if (bDoEnd)
                        {
                            c4CondInd = c4FindCondEnd(*pcolOps, c4CondInd, kCIDLib::True);
                            TActOpcode& aocSec = pcolOps->objAt(c4CondInd);
                            aocSec.m_eOpcode = tCQCKit::EActOps::Comment;
                            aocSec.m_ccfgStep.c4ParmCnt(1);
                            aocSec.m_ccfgStep.SetParmAt(0, strComment, tCQCKit::ECmdPTypes::Text);
                        }
                    }
                }
            }
        }

        //
        //  Flush the command cache between each one since the list of cmds
        //  available changes for each event.
        //
        m_colCmdCache.RemoveAll();
    }

    if (bLostCmds)
    {
        TErrBox msgbErr(facCQCIGKit().strMsg(kIGKitMsgs::midStatus_TarNotFound));
        msgbErr.ShowIt(wndOwner);
    }

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_EdCmd
    );
    return (c4Res == kCQCIGKit::ridDlg_EdCmd_Save);
}


//
//  See header notes. If the caller allows for any sort of relative paths, then
//  this is the base path that relative paths are relative to.
//
tCIDLib::TVoid TCQCEditCmdDlg::SetBasePath(const TString& strToSet)
{
    m_strBasePath = strToSet;
}


// ---------------------------------------------------------------------------
//  TCQCEditCmdDlg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  If they hit escape or the X button. If we return true it allows the exit, so we
//  return the negation of the exit check.
//
tCIDLib::TBoolean TCQCEditCmdDlg::bCancelRequest()
{
    return bCheckExit();
}


//
//  We ask the parameter entry fields to pass us extended key strokes. If the child
//  window points is not null, then it should be one of the parameter entry fields,
//  so we see if the key is one we care about.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bChar(  const   tCIDLib::TCh        chChar
                        , const tCIDLib::TBoolean   bAlt
                        , const tCIDLib::TBoolean   bCtrl
                        , const tCIDLib::TBoolean   bShift
                        , const TWindow* const      pwndChild)
{

    if (pwndChild)
    {
        // Figure out which one it is
        tCIDLib::TCard4 c4ParmInd = kCIDLib::c4MaxCard;
        const tCIDCtrls::TWndId widSrc = pwndChild->widThis();
        if (widSrc == kCQCIGKit::ridDlg_EdCmd_P1Text)
            c4ParmInd = 0;
        else if (widSrc == kCQCIGKit::ridDlg_EdCmd_P2Text)
            c4ParmInd = 1;
        else if (widSrc == kCQCIGKit::ridDlg_EdCmd_P3Text)
            c4ParmInd = 2;
        else if (widSrc == kCQCIGKit::ridDlg_EdCmd_P4Text)
            c4ParmInd = 3;

        if (c4ParmInd != kCIDLib::c4MaxCard)
        {

            if (!bAlt && bCtrl && !bShift)
            {
                if (chChar == kCIDLib::chLatin_d)
                {
                    InsertDriverMon(c4ParmInd);
                    return kCIDLib::True;
                }
                 else if (chChar == kCIDLib::chLatin_f)
                {
                    InsertField(c4ParmInd);
                    return kCIDLib::True;
                }
            }
        }
    }
    return kCIDLib::False;
}



tCIDLib::TBoolean TCQCEditCmdDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Change, m_pwndChangeButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Cmd, m_pwndCmdButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Copy, m_pwndCopyButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_CopyAll, m_pwndCopyAllButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Comment, m_pwndCommentButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Cut, m_pwndCutButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_DeleteAll, m_pwndDelAllButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Disable, m_pwndDisableButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Events, m_pwndEvents);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_If, m_pwndIfButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_IfElse, m_pwndIfElseButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_MoveDn, m_pwndDnButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_MoveUp, m_pwndUpButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_OpList, m_pwndOpList);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Paste, m_pwndPasteButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_PasteAll, m_pwndPasteAllButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_P1Pref, m_apwndPrefs[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_P1Text, m_apwndTexts[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_P2Pref, m_apwndPrefs[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_P2Text, m_apwndTexts[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_P3Pref, m_apwndPrefs[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_P3Text, m_apwndTexts[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_P4Pref, m_apwndPrefs[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_P4Text, m_apwndTexts[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Replace, m_pwndReplaceButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Reset, m_pwndResetButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdCmd_Save, m_pwndSaveButton);

    // And register our event handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndChangeButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndCmdButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndCopyButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndCopyAllButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndCutButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndCommentButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndDelAllButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndDisableButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndDnButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndEvents->pnothRegisterHandler(this, &TCQCEditCmdDlg::eLBHandler);
    m_pwndIfButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndIfElseButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndOpList->pnothRegisterHandler(this, &TCQCEditCmdDlg::eLBHandler);
    m_pwndPasteButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndPasteAllButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndReplaceButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndResetButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
    m_pwndUpButton->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);

    //
    //  While we are going through these, disable the prefix buttons, since they need to
    //  be disabled until such time as a command is selected. This insure that, if we
    //  have no commands, we'll come up with them disable without a bunch of checking
    //  below.
    //
    //  We also ask the parameter text ones to pass on unused key combos, so that we can
    //  provide some hot keys.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCKit::c4MaxCmdParms; c4Index++)
    {
        m_apwndPrefs[c4Index]->pnothRegisterHandler(this, &TCQCEditCmdDlg::eClickHandler);
        m_apwndTexts[c4Index]->pnothRegisterHandler(this, &TCQCEditCmdDlg::eEFHandler);

        m_apwndPrefs[c4Index]->SetEnable(kCIDLib::False);
        m_apwndTexts[c4Index]->bPropagateKeys(kCIDLib::True);
    }

    // Make sure the command source has some events
    const tCIDLib::TCard4 c4EvCount = m_csrcEdit.c4EventCount();
    if (!c4EvCount)
    {
        TOkBox msgbErr
        (
            strWndText()
            , facCQCKit().strMsg(kKitErrs::errcCmd_NoEvents)
        );
        msgbErr.ShowIt(*this);
        EndDlg(kCQCIGKit::ridDlg_EdCmd_Cancel);
        return kCIDLib::False;
    }

    //
    //  Set up some font stuff here. We want the list box to use a fixed
    //  pitch font to make the code more readable. So get the standard
    //  attrs, and adjust to make it a fixed pitch. This get's us one that
    //  is basically the same size as the standard font for this machine.
    //
    TFontSelAttrs fselFont(TGUIFacility::gfontDefNorm().fselCurrent());
    fselFont.strFaceName(L"Courier");
    fselFont.ePitch(tCIDGraphDev::EFontPitches::Fixed);
    fselFont.eWeight(tCIDGraphDev::EFontWeights::Medium);
    fselFont.eFamily(tCIDGraphDev::EFontFamilies::DontCare);
    m_gfontLB.SetSelAttrs(fselFont);
    m_pwndOpList->SetFont(m_gfontLB);

    //
    //  Load the event list. Find the first one that has some opcodes and select
    //  it, else the 0th one. We set the user data of each one to the index in
    //  the original list so that we can let it sort but still easily map back.
    //
    tCIDLib::TCard4 c4InitSel = kCIDLib::c4MaxCard;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4EvCount; c4Index++)
    {
        const TCQCActEvInfo& caeiCur = m_csrcEdit.caeiEventAt(c4Index);
        m_pwndEvents->InsertItem(kCIDLib::c4MaxCard, caeiCur.m_strName, c4Index);

        if ((c4InitSel == kCIDLib::c4MaxCard)
        &&  m_csrcEdit.c4OpCount(caeiCur.m_strEventId))
        {
            c4InitSel = c4Index;
        }
    }

    // Set the column titles on the opcode list
    tCIDLib::TStrList colColTitles(1);
    colColTitles.objAdd(L"Commands for this Event");
    m_pwndOpList->SetColumns(colColTitles);

    //
    //  If all events were empty, just select the zeroth one. Force an event to be
    //  sent even if we don't change indices, so that the op list gets filled in.
    //
    if (c4InitSel == kCIDLib::c4MaxCard)
        c4InitSel = 0;
    m_pwndEvents->SelectByIndex(c4InitSel, kCIDLib::True);

    //
    //  Before we are made visible, reposition ourself to the previous size
    //  and pos we were at. This will cause a call to AreaChanged which will
    //  adjust the control positions. Only do it if not empty (which means
    //  this is the first run.) If empty, then calculate a center position
    //  for the dialog and put it there.
    //
    if (CQCIGKit_EditCmdDlg::areaLastPos.bIsEmpty())
    {
        TArea areaOwner = pwndFindOwner()->areaWnd();
        TArea areaUs = areaWnd();

        // Center our area in the owner's and move us there
        areaUs.CenterIn(areaOwner);
        SetPos(areaUs.pntOrg());
    }
     else
    {
        // Mmove to the previous area
        SetSizePos(CQCIGKit_EditCmdDlg::areaLastPos, kCIDLib::False);

        //
        //  Do an inital auto-size to get the content resized from our initial size
        //  to the restored area.
        //
        AutoAdjustChildren(areaWndSize(), CQCIGKit_EditCmdDlg::areaLastPos);
    }

    return bRes;
}



tCIDLib::TBoolean
TCQCEditCmdDlg::bShowHelp(  const   tCIDCtrls::TWndId   widCtrl
                            , const TPoint&             )
{
    // Get the master server web server info. If we can't get that, can't do anything
    tCIDLib::TBoolean bSecure = kCIDLib::False;
    tCIDLib::TIPPortNum ippnWS = 0;
    TString strWSHost;
    if (!facCQCKit().bFindHelpWS(strWSHost, ippnWS, bSecure))
    {
        TErrBox msgbErr(L"The Master Server's Web Server information could not be obtained");
        msgbErr.ShowIt(*this);
        return kCIDLib::True;
    }

    //
    //  If there is an action selected in the action list, then le's invoke help for
    //  that guy.
    //
    const tCIDLib::TCard4 c4CmdInd = m_pwndOpList->c4CurItem();
    if (c4CmdInd != kCIDLib::c4MaxCard)
    {
        TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4CmdInd);
        TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;

        //
        //  Depending on what's selected, we will create a help URL and invoke the
        //  browser on it.
        //
        if (aocCur.m_eOpcode == tCQCKit::EActOps::Cmd)
        {
            // Find the target for this command
            TCQCCmd cmdCheck;
            MCQCCmdTarIntf* pmctarCur;
            if (bFindCmd(ccfgCur, m_strCurEventId, cmdCheck, pmctarCur))
            {
                //
                //  OK, let's now build up the URL.
                TString strURL(bSecure ? L"https://" : L"http://");
                strURL.Append(strWSHost);
                strURL.Append(kCIDLib::chColon);
                strURL.AppendFormatted(ippnWS);
                strURL.Append(L"/Web2/CQCDocs/CQCDocs.html?");

                //
                //  The help id tells us the target page to load. If it starts with the
                //  interface widget prefix, which means it's a named widget, then we
                //  send them to the widget documentation. Else, it'll be one of the
                //  standard command targets and it'll be in the action reference part.
                //
                tCIDLib::TBoolean bShowIt = kCIDLib::True;
                TString strHelpRef = pmctarCur->strCmdHelpId();
                if (strHelpRef.bIsEmpty())
                {
                    bShowIt = kCIDLib::False;
                }
                 else
                {
                    //
                    //  We have to break out the last part as the page id and the other part
                    //  is the reference.
                    //
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

                    // Add the command name as a bookmark
                    strURL.Append(L"&bookmark=");
                    strURL.Append(ccfgCur.strName());
                }

                if (bShowIt)
                    TGUIShell::InvokeDefBrowser(*this, strURL);

                // Indicate we handled this so it doesn't propagate
                return kCIDLib::True;
            }
        }
    }

    // Let ot go up-stream
    return kCIDLib::False;
}


tCIDLib::TVoid TCQCEditCmdDlg::Destroyed()
{
    // Remember our last size/pos, then pass it on to our parent
    CQCIGKit_EditCmdDlg::areaLastPos = areaWnd();
    TParent::Destroyed();
}


//
//  We intercept the popup context menus of the entry fields (by setting the 'parent
//  menu' style on them, so that we can provide our own menu.
//
tCIDCtrls::ECtxMenuOpts
TCQCEditCmdDlg::eShowContextMenu(const TPoint& pntAt, const tCIDCtrls::TWndId widSrc)
{
    // If nothing is selected in the action list, then nothing to do
    const tCIDLib::TCard4 c4ActListIndex = m_pwndOpList->c4CurItem();
    if (c4ActListIndex == kCIDLib::c4MaxCard)
        return tCIDCtrls::ECtxMenuOpts::Done;

    //
    //  Figure out which parameter this is for, which also makes sure we are getting
    //  this from a right click on one of the entry fields. If not, it's generally
    //  because the entry field is disabled, and so the context menu invocation goes
    //  straight through to us.
    //
    tCIDLib::TCard4 c4ParmInd = kCIDLib::c4MaxCard;
    if (widSrc == kCQCIGKit::ridDlg_EdCmd_P1Text)
        c4ParmInd = 0;
    else if (widSrc == kCQCIGKit::ridDlg_EdCmd_P2Text)
        c4ParmInd = 1;
    else if (widSrc == kCQCIGKit::ridDlg_EdCmd_P3Text)
        c4ParmInd = 2;
    else if (widSrc == kCQCIGKit::ridDlg_EdCmd_P4Text)
        c4ParmInd = 3;

    if (c4ParmInd == kCIDLib::c4MaxCard)
        return tCIDCtrls::ECtxMenuOpts::Done;

    //
    //  If this parameter is not used, then do nothing. We get the opcode for this command
    //  and see what the parameter count is. If our parameter index is at or beyond that
    //  count, then it's not a used one.
    //
    {

        TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4ActListIndex);
        TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;
        if (c4ParmInd >= ccfgCur.c4ParmCnt())
            return tCIDCtrls::ECtxMenuOpts::Done;
    }

    // Put the focus on this entry field if it's not already
    m_apwndTexts[c4ParmInd]->TakeFocus();

    // Looks ok, so let's set up the menu
    TPopupMenu menuInsert(L"Parm Popup");
    menuInsert.Create(facCQCIGKit(), kCQCIGKit::ridMenu_Parms);

    //
    //  Insert the action parm values into that menu. We just offer 8 of them. They
    //  have standard names.
    //
    {
        TSubMenu menuActParm(menuInsert, kCQCIGKit::ridMenu_Parms_ActParms);
        tCIDLib::TCard4 c4NextId = CQCIGKit_EditCmdDlg::c4InsActParmFirst;
        TString strName;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < 8; c4Index++)
        {
            strName.SetFormatted(c4Index + 1);
            menuActParm.AddActionItem(strName, strName, c4NextId++);
        }
    }

    // Fill in the RTVs submenu
    {
        TSubMenu menuRTVs(menuInsert, kCQCIGKit::ridMenu_Parms_RTVs);
        const tCIDLib::TCard4 c4BaseRTVCnt = MCQCCmdSrcIntf::c4StdRTVCount();
        tCIDLib::TCard4 c4NextId = CQCIGKit_EditCmdDlg::c4InsRTVFirst;

        const tCIDLib::TCard4 c4RTVCnt = m_csrcEdit.c4RTValCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RTVCnt; c4Index++)
        {
            //
            //  When we hit the number of standard ones, then put in a separator
            //  to separate the standard ones from the source specific ones.
            //
            if (c4Index == c4BaseRTVCnt)
                menuRTVs.AddSeparator();

            const TCQCCmdRTVal& crtvCur = m_csrcEdit.crtvAt(c4Index);
            menuRTVs.AddActionItem(crtvCur.strName(), crtvCur.strName(), c4NextId++);
        }
    }

    // Now we can process it
    const tCIDLib::TCard4 c4Cmd = menuInsert.c4Process(*this, pntAt);

    //
    //  If we got something, then get the current selection of the parameter value. This
    //  will be the previous value.
    //
    TString strPrevVal;
    if (c4Cmd)
    {
        if (!m_apwndTexts[c4ParmInd]->bGetSelectedText(strPrevVal))
            strPrevVal.Clear();
    }

    TString strFmt;
    if ((c4Cmd >= CQCIGKit_EditCmdDlg::c4InsActParmFirst)
    &&  (c4Cmd <= CQCIGKit_EditCmdDlg::c4InsActParmLast))
    {
        // It's an action parameter
        TString strFmt = L"%(LVar:CQCActParm_";
        strFmt.AppendFormatted
        (
            (c4Cmd - CQCIGKit_EditCmdDlg::c4InsActParmFirst) + 1
        );
        strFmt.Append(L")");
        m_apwndTexts[c4ParmInd]->InsertText(strFmt, kCIDLib::True);
    }
     else if ((c4Cmd >= CQCIGKit_EditCmdDlg::c4InsRTVFirst)
          &&  (c4Cmd <= CQCIGKit_EditCmdDlg::c4InsRTVLast))
    {
        // It's a runtime value
        const TCQCCmdRTVal& crtvCur = m_csrcEdit.crtvAt
        (
            c4Cmd - CQCIGKit_EditCmdDlg::c4InsRTVFirst
        );
        strFmt = L"%(";
        strFmt.Append(crtvCur.strId());
        strFmt.Append(L")");

        m_apwndTexts[c4ParmInd]->InsertText(strFmt, kCIDLib::True);
    }
     else if (c4Cmd == kCQCIGKit::ridMenu_Parms_DriverMon)
    {
        InsertDriverMon(c4ParmInd);
    }
     else if (c4Cmd == kCQCIGKit::ridMenu_Parms_Field)
    {
        InsertField(c4ParmInd);
    }
     else if (c4Cmd == kCQCIGKit::ridMenu_Parms_Var)
    {
        //
        //  Select a variable and insert it. Pass in any previous value in case
        //  it's a valid variable or reference. We allow by name or value here.
        //
        TString strSel;
        if (bSelectVariable(strPrevVal, strSel, kCIDLib::False))
            m_apwndTexts[c4ParmInd]->InsertText(strSel, kCIDLib::True);
    }
     else if ((c4Cmd == kCQCIGKit::ridMenu_Parms_Copy)
          ||  (c4Cmd == kCQCIGKit::ridMenu_Parms_Cut))
    {
        // Copy any selected text to the clipboard
        m_apwndTexts[c4ParmInd]->bSelectionToClipboard();

        // If cutting, delete the selection
        if (c4Cmd == kCQCIGKit::ridMenu_Parms_Cut)
            m_apwndTexts[c4ParmInd]->InsertText(TString::strEmpty());
    }
     else if (c4Cmd == kCQCIGKit::ridMenu_Parms_Paste)
    {
        // If there's any text no the clipboard, paste it in
        try
        {
            TGUIClipboard gclipTmp(*this);

            TString strPaste;
            if (gclipTmp.bGetAsText(strPaste))
                m_apwndTexts[c4ParmInd]->InsertText(strPaste);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TErrBox msgbErr(L"Action Editor", errToCatch.strErrText());
            msgbErr.ShowIt(*this);
        }
    }
     else if ((c4Cmd == kCQCIGKit::ridMenu_Parms_ImagePath)
          ||  (c4Cmd == kCQCIGKit::ridMenu_Parms_SchEventPath)
          ||  (c4Cmd == kCQCIGKit::ridMenu_Parms_TrgEventPath)
          ||  (c4Cmd == kCQCIGKit::ridMenu_Parms_TemplatePath))
    {
        TString strSelected;
        tCQCRemBrws::EDTypes eDType = tCQCRemBrws::EDTypes::Count;
        if (c4Cmd == kCQCIGKit::ridMenu_Parms_ImagePath)
            eDType = tCQCRemBrws::EDTypes::Image;
        else if (c4Cmd == kCQCIGKit::ridMenu_Parms_SchEventPath)
            eDType = tCQCRemBrws::EDTypes::SchEvent;
        else if (c4Cmd == kCQCIGKit::ridMenu_Parms_TrgEventPath)
            eDType = tCQCRemBrws::EDTypes::TrgEvent;
        else if (c4Cmd == kCQCIGKit::ridMenu_Parms_TemplatePath)
            eDType = tCQCRemBrws::EDTypes::Template;

        const tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
        (
            *this
            , L"Insert File Path"
            , eDType
            , TString::strEmpty()
            , m_cuctxToUse
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strSelected
        );

        if (bRes)
            m_apwndTexts[c4ParmInd]->InsertText(strSelected, kCIDLib::True);
    }
    return tCIDCtrls::ECtxMenuOpts::Done;
}



// ---------------------------------------------------------------------------
//  TCQCEditCmdDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We just do whatever checking we can do on the parameters of the passed
//  opcode. We can adjust the parameter values here, such as stripping leading
//  and trailing whitespace.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bCheckParms(        TActOpcode&         aocCheck
                            , const TString&            strEventId
                            ,       tCIDLib::TCard4&    c4BadParmInd)
{
    TCQCCmdCfg& ccfgCheck = aocCheck.m_ccfgStep;

    // Look up the original command info
    TCQCCmd         cmdCheck;
    MCQCCmdTarIntf* pmctarCur;
    if (!bFindCmd(ccfgCheck, strEventId, cmdCheck, pmctarCur))
    {
        TErrBox msgbErr
        (
            strWndText()
            , facCQCIGKit().strMsg(kIGKitMsgs::midStatus_CantFindTarget)
        );
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }

    // Create a standard RTV object for use below
    TCQCCmdRTVSrc   crtsTest(m_cuctxToUse);
    TString         strPRes;
    TString         strErrText;
    TString         strTmp1;
    TString         strTmp2;
    const tCIDLib::TCard4 c4PCount = ccfgCheck.c4ParmCnt();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PCount; c4Index++)
    {
        const TCQCCmdParm& cmdpCur = cmdCheck.cmdpAt(c4Index);
        TString& strPVal = ccfgCheck.piAt(c4Index).m_strValue;
        if (cmdpCur.bRequired() && strPVal.bIsEmpty())
        {
            TErrBox msgbErr
            (
                strWndText()
                , facCQCIGKit().strMsg
                  (
                    kIGKitMsgs::midStatus_RequiredCmdParm
                    , TCardinal(c4Index + 1)
                    , cmdCheck.strName()
                  )
            );
            msgbErr.ShowIt(*this);
            c4BadParmInd = c4Index;
            return kCIDLib::False;
        }

        //
        //  Based on the type we can do some validation, if the parm is not empty
        //  and doesn't have any replacement tokens that we cannot provide. If it
        //  does, then we cannot really evaluate the parm here.
        //
        //  An exception is an expression, in which we know the values won't be
        //  available but we want to test the expression as is.
        //
        const tCQCKit::ECmdPrepRes eRes = facCQCKit().eStdTokenReplace
        (
            strPVal, &crtsTest, 0, 0, strPRes, tCQCKit::ETokRepFlags::TestOnly
        );

        if (eRes == tCQCKit::ECmdPrepRes::Failed)
        {
            TErrBox msgbErr
            (
                strWndText()
                , facCQCIGKit().strMsg
                  (
                    kIGKitMsgs::midStatus_CmdExpansionErr
                    , TCardinal(c4Index + 1)
                    , cmdCheck.strName()
                  )
            );
            msgbErr.ShowIt(*this);
            c4BadParmInd = c4Index;
            return kCIDLib::False;
        }

        //
        //  If we have a valid and there we no tokens in it, let's let the command target
        //  validate the parameter value. We re-evaluate expressions even if they have
        //  not changed, since they reference other things.
        //
        if (!strPVal.bIsEmpty()
        &&  ((eRes == tCQCKit::ECmdPrepRes::Unchanged)
        ||   (cmdpCur.eType() == tCQCKit::ECmdPTypes::Expression)))
        {
            // This shouldn't throw, but we don't want to fall over if so
            tCIDLib::TBoolean bOk = kCIDLib::False;
            try
            {
                bOk = pmctarCur->bValidateParm
                (
                    cmdCheck, ccfgCheck, c4Index, strPVal, strErrText
                );
            }

            catch(TError& errToCatch)
            {
                if (facCQCIGKit().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
                strErrText = errToCatch.strErrText();
            }

            // If it failed, tell the user
            if (!bOk)
            {
                TErrBox msgbErr
                (
                    strWndText()
                    , facCQCKit().strMsg
                      (
                        kKitErrs::errcCmd_BadCmdParm
                        , TCardinal(c4Index + 1)
                        , cmdCheck.strName()
                        , strErrText
                      )
                );
                msgbErr.ShowIt(*this);
                c4BadParmInd = c4Index;
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}


//
//  Each time we need to present a list of available commands, we have to ask the
//  target widget for his list of commands. And we also ahve to do it when validating
//  all the commands, and also to just look up the command figure out the parameters
//  and so forth. There's a lot of overhead involved in doing this over and over.
//
//  So we manage a cache to avoid this. The cache will be flushed when they change
//  events, because the list of possible commands will usually chnage, and we'll just
//  start faulting it back in again as targets are accessed.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bFindCmd(const  TCQCCmdCfg&         ccfgSrc
                        , const TString&            strEventId
                        ,       TCQCCmd&            cmdToFill
                        ,       MCQCCmdTarIntf*&    pmctarCur)
{
    // Get the target id that the command was configured for
    const TString& strTarId = ccfgSrc.strTargetId();

    // See if such a target exists. If not, we failed
    pmctarCur = pctarFindById(strTarId);
    if (!pmctarCur)
        return kCIDLib::False;

    // See if its cmds are in our cache first. If not, add it
    TCmdCache::TPair* pkvalFind = m_colCmdCache.pkobjFindByKey(strTarId);
    if (!pkvalFind)
    {
        // It's not, so add a cache slot and load it up
        pkvalFind = &m_colCmdCache.kobjAdd(strTarId, TCmdList());
        pmctarCur->QueryCmds
        (
            pkvalFind->objValue(), m_csrcEdit.eContextOfEvent(strEventId)
        );
    }

    // And find this command in its list of commands
    TCmdList& colCmds = pkvalFind->objValue();
    const tCIDLib::TCard4 c4Count = colCmds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (colCmds[c4Index].strId() == ccfgSrc.strCmdId())
        {
            cmdToFill = colCmds[c4Index];
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  This is called to select/change a command via a popup menu. It is a multi-level menu
//  with the targets as the top level sub-menus, and the commands available for each
//  target in the drop-over menu.
//
//  The caller tells us what kind of command this should be via the eType parameter. It
//  tells us if we can present only conditionals, only non-conditionals, or either.
//
//  We need to have some way of mapping back to the selected target, based on the menu
//  id chosen by the user. So we have to create a list of target/command pairs that can
//  be referenced at the end to get back to the selected item.
//
//  Since the commands are queried from the targets, there is no big list of them and we
//  cannot go back and ask for one specifically, so we have to copy the commands into the
//  list as well. Kind of inefficient, but this is a human driven action that will happen
//  rarely in computer terms.
//
//  We need to get the targets and their commands into the appropriate order, so we do
//  a sort after we get the list. This is a three level sort, the levels being:
//
//  1. If it's a special target it always comes first
//  2. If the same special/non-special type, then the command target name is used
//  3. If the same command target name, then the command name is used.
//
//  This gets us special ones first, then the non-special ones, and within that the two
//  lists are sorted by target name. And, each target's commands are sorted. The cmd item
//  class' comparator method handles this.
//
class TCmdItem
{
    public :
        static tCIDLib::ESortComps eComp
        (
            const   TCmdItem&               item1
            , const TCmdItem&               item2
        );

        TCmdItem(const  MCQCCmdTarIntf* const   pctarCmd
                , const TCQCCmd&                cmdThis) :

            m_cmdThis(cmdThis)
            , m_pctarCmd(pctarCmd)
        {}

        ~TCmdItem() {}

        TCQCCmd                 m_cmdThis;
        const MCQCCmdTarIntf*   m_pctarCmd;
};

tCIDLib::ESortComps
TCmdItem::eComp(const TCmdItem& item1, const TCmdItem& item2)
{
    const tCIDLib::TBoolean b1Special = item1.m_pctarCmd->bIsSpecialCmdTar();
    const tCIDLib::TBoolean b2Special = item2.m_pctarCmd->bIsSpecialCmdTar();

    // Sort of first on special/non-special target differences
    tCIDLib::ESortComps eComp = tCIDLib::ESortComps::Equal;
    if (!b1Special && b2Special)
        return tCIDLib::ESortComps::FirstGreater;
    else if (b1Special && !b2Special)
        return tCIDLib::ESortComps::FirstLess;

    // If the same, then check the target name
    if (eComp == tCIDLib::ESortComps::Equal)
        eComp = item1.m_pctarCmd->strCmdTargetName().eCompare(item2.m_pctarCmd->strCmdTargetName());

    // If those are the same as well, then sort on command name
    if (eComp == tCIDLib::ESortComps::Equal)
            eComp = item1.m_cmdThis.strName().eCompare(item2.m_cmdThis.strName());

        return eComp;
}


tCIDLib::TBoolean
TCQCEditCmdDlg::bGetCommand(const   tCQCKit::ECmdTypes  eType
                            , const TPoint&             pntAt
                            ,       TCQCCmd&            cmdToFill
                            , const MCQCCmdTarIntf*&    pmctarToFill)
{
    //
    //  Ok, we have to jump through some hoops to get the values in the
    //  menu correctly arranged. First, we want to set up a sorted vector
    //  of target names, so that we can put the targets in sorted order.
    //
    //  But we have to have special targets first. So, we first get all
    //  the special target names, and then all the non-special target
    //  names.
    //
    //  We have a special hack here for now, to only present the global
    //  variables target if it's an OnExit command. Later we need to come
    //  with some better way to deal with filtering on event type. Currently
    //  it's becoming some sort of combinatorial math thing.
    //
    tCIDLib::TCard4 c4TarCnt = m_colTargets.c4ElemCount();
    TVector<TCmdItem> colKeepers;
    {
        TRefVector<const MCQCCmdTarIntf> colByName
        (
            tCIDLib::EAdoptOpts::NoAdopt, c4TarCnt
        );
        tCQCKit::TCmdTarList::TCursor cursTars(&m_colTargets);
        if (cursTars.bReset())
        {
            if (m_strCurEventId == kCQCKit::strEvId_OnExit)
            {
                do
                {
                    const MCQCCmdTarIntf& mctarCur = cursTars.objRCur();
                    if (mctarCur.strCmdTargetId() == kCQCKit::strCTarId_GVars)
                    {
                        colByName.Add(&mctarCur);
                        break;
                    }
                }   while (cursTars.bNext());
                c4TarCnt = 1;
            }
             else
            {
                do
                {
                    const MCQCCmdTarIntf& mctarCur = cursTars.objRCur();
                    if (mctarCur.bIsSpecialCmdTar())
                        colByName.Add(&mctarCur);
                }   while (cursTars.bNext());
                const tCIDLib::TCard4 c4SpecialCnt = colByName.c4ElemCount();

                cursTars.bReset();
                do
                {
                    const MCQCCmdTarIntf& mctarCur = cursTars.objRCur();
                    if (!mctarCur.bIsSpecialCmdTar())
                        colByName.Add(&mctarCur);
                }   while (cursTars.bNext());

                // If we ended up with none, then we are done
                const tCIDLib::TCard4 c4NameCnt = colByName.c4ElemCount();
                if (!c4NameCnt)
                    return kCIDLib::False;
            }
        }

        //
        //  Now loop through the target list and for each one, ask it for its
        //  commands and see if any of them are valid for the type of commands
        //  we want to show. If so, add it to the list.
        //
        TVector<TCQCCmd> colCmds;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarCnt; c4Index++)
        {
            const MCQCCmdTarIntf& mctarCur = *colByName[c4Index];
            colCmds.RemoveAll();
            mctarCur.QueryCmds(colCmds, m_csrcEdit.eContextOfEvent(m_strCurEventId));

            const tCIDLib::TCard4 c4CmdCnt = colCmds.c4ElemCount();
            for (tCIDLib::TCard4 c4CInd = 0; c4CInd < c4CmdCnt; c4CInd++)
            {
                const TCQCCmd& cmdCur = colCmds[c4CInd];
                if (tCIDLib::bAllBitsOn(cmdCur.eType(), eType))
                    colKeepers.objPlace(&mctarCur, cmdCur);
            }
        }
    }

    // If we didn't get any keepers, then just return false now
    if (colKeepers.bIsEmpty())
        return kCIDLib::False;

    // Now we do a sort of the keepers list, see the item class above
    colKeepers.Sort(TCmdItem::eComp);

    // We got some, so create and fill in the menu
    tCIDLib::TBoolean bRes = kCIDLib::False;
    const tCIDLib::TCard4 c4KeeperCnt = colKeepers.c4ElemCount();
    try
    {
        TPopupMenu menuCmds(L"CmdEditorPopup");
        menuCmds.Create();

        //
        //  We have to keep up with the last target we saw so that we can
        //  create a new submenu item each time it changes. We also have
        //  to remember when we see the first non-special target, so we
        //  can insert a separator.
        //
        tCIDLib::TBoolean       bFirstNS = kCIDLib::True;
        const MCQCCmdTarIntf*   pmctarLast = nullptr;

        //
        //  Note that we assign menu items that match the index in our
        //  keeper collection of the item it represents. This way, we
        //  can do a simple lookup at the end if they select one.
        //
        TSubMenu menuSub;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4KeeperCnt; c4Index++)
        {
            TCmdItem& itemCur = colKeepers[c4Index];

            // If this is a new target, start a new submenu
            if (itemCur.m_pctarCmd != pmctarLast)
            {
                // If this is the first non-special, insert a separator
                if (!itemCur.m_pctarCmd->bIsSpecialCmdTar() && bFirstNS)
                {
                    menuCmds.AddSeparator();
                    bFirstNS = kCIDLib::False;
                }

                // Remember this guy as the last one we saw
                pmctarLast = itemCur.m_pctarCmd;

                // Give it the index of it's first slot in the keeper list + 1
                menuCmds.AddSubMenu
                (
                    pmctarLast->strCmdTargetName()
                    , pmctarLast->strCmdTargetName()
                    , c4Index + 1
                );

                // Get this submenu now as the active one
                menuSub.Set(menuCmds, c4Index + 1);
            }

            // Add an item to the current submenu
            menuSub.AddActionItem
            (
                itemCur.m_cmdThis.strName()
                , itemCur.m_cmdThis.strName()
                , c4Index + 1
            );
        }

        //
        //  Ok, it's loaded, so pop up the menu. The last parms says bottom align,
        //  which should make it come out below the point.
        //
        tCIDLib::TCard4 c4Res = menuCmds.c4Process
        (
            *this, pntAt, tCIDLib::EVJustify::Bottom
        );

        // If they made a selection, then return that info
        if (c4Res)
        {
            bRes = kCIDLib::True;

            // We used 1 based ids, so sub by 1 to get the index
            c4Res--;
            TCmdItem& itemSel = colKeepers[c4Res];
            pmctarToFill = itemSel.m_pctarCmd;
            cmdToFill = itemSel.m_cmdThis;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this, facCQCIGKit().strMsg(kIGKitErrs::errcCmd_SelMenu), errToCatch
        );
        bRes = kCIDLib::False;
    }
    return bRes;
}


//
//  Give the path to a template, query the base template info. Return true if we
//  were able to do so.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bQueryTmplInfo( const   TString&                strTmplPath
                                ,       TString&                strNotes
                                ,       tCQCKit::EUserRoles&    eMinRole)
{

    TString strExpPath;
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        //
        //  Expand the template path, which we give back to the caller for any error
        //  msgs and whatnot.
        //
        facCQCKit().bExpandResPath(m_strBasePath, strTmplPath, strExpPath);

        // Get a data server client
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        TDataSrvClient dsclQ;

        tCIDLib::TCard4         c4Sz;
        tCIDLib::TCard4         c4SerialNum;
        tCIDLib::TEncodedTime   enctLast;
        bRes = dsclQ.bQueryTemplateInfo
        (
            strTmplPath
            , c4Sz
            , eMinRole
            , strNotes
            , c4SerialNum
            , enctLast
            , kCIDLib::False
            , m_cuctxToUse.sectUser()
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbErr
        (
            strWndText()
            , facCQCIGKit().strMsg(kIGKitErrs::errcTmpl_TmplQueryInfo, strExpPath)
        );
        msgbErr.ShowIt(*this);
    }
    return bRes;
}



//
//  Allows the user to select a local executable file.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectApp(const TString& strOrgVal, TString& strToFill)
{
    // Set up a file type list
    tCIDLib::TKVPList colTypes;
    colTypes.objPlace(L"Executable File", L"*.exe");

    TString strRealVal(strOrgVal);
    SwizzlePath(strRealVal, kCIDLib::True);

    tCIDLib::TStrList colSel;
    const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
    (
        *this
        , facCQCIGKit().strMsg(kIGKitMsgs::midTitle_SelProgram)
        , strRealVal
        , colSel
        , colTypes
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EFOpenOpts::FileMustExist, tCIDCtrls::EFOpenOpts::FileSystemOnly
          )
    );

    if (bRes)
    {
        strToFill = colSel[0];
        SwizzlePath(strToFill, kCIDLib::False);
    }
    return bRes;
}


//
//  This is called when the current parameter is a boolean type, and the user
//  presses the select button. The return indicates if they made a selection or not.
//  If so, we return the text formatted version of the value.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectBool(        TString&        strToFill
                            , const tCIDLib::TCard4 c4ParmInd)
{
    //
    //  Calculate where we want the menu to be at (to the right of the value
    //  selection button that caused us to be called.)
    //
    TPoint pntAt;
    ToScreenCoordinates(m_apwndPrefs[c4ParmInd]->areaWnd().pntUR(), pntAt);
    pntAt.Adjust(2, 0);

    //
    //  And now let the user select from this list. We do it via a simple
    //  popup menu.
    //
    tCIDLib::TBoolean bRes = kCIDLib::False;
    TPopupMenu menuBool(L"Bool Value Select");
    try
    {
        // Fill in the menu with the two values
        menuBool.Create();
        menuBool.AddActionItem(L"False", kCQCKit::pszFld_False, 1000);
        menuBool.AddActionItem(L"True", kCQCKit::pszFld_True, 1001);

        // Run the menu and if they select something, give back that value
        tCIDLib::TCard4 c4Res = menuBool.c4Process
        (
            *this, pntAt, tCIDLib::EVJustify::Top, tCIDLib::EHJustify::Left
        );

        // If they made a selection
        if (c4Res == 1000)
        {
            strToFill = kCQCKit::pszFld_False;
            bRes = kCIDLib::True;
        }
         else if (c4Res == 1001)
        {
            strToFill = kCQCKit::pszFld_True;
            bRes = kCIDLib::True;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Never made a selection
        return kCIDLib::False;
    }
    return bRes;
}


//
//  This is called when the current parameter is an color type, and the user
//  presses the select button.
//
tCIDLib::TBoolean TCQCEditCmdDlg::bSelectColor(TString& strToFill)
{
    // Parse any incoming value, else just set to white to start
    TRGBClr rgbVal;
    if (!rgbVal.bParseFromText(strToFill, tCIDLib::ERadices::Dec, L' '))
        rgbVal = facCIDGraphDev().rgbWhite;

    // Pop up the palette window
    if (facCIDWUtils().bSelectColor(*this, rgbVal))
    {
        rgbVal.FormatToText(strToFill, kCIDLib::chSpace);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  This is called when the current parameter is a e-mail account type, and the user presses
//  the select button. We load up the defined e-mail accounts and let them select one.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectEMailAcct(const TString& strOrgVal, TString& strToFill)
{
    //
    //  Try to query all of the e-mail accounts from the installation server. We just need the
    //  names here, not the whole accounts.
    //
    tCIDLib::TStrList colAccounts;
    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        orbcIS->bQueryEMailAccountNames(colAccounts, m_cuctxToUse.sectUser());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbErr
        (
            strWndText(), facCQCIGKit().strMsg(kIGKitErrs::errcCmd_QueryEMAccts)
        );
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }

    // If no accounts defined, say so and return
    const tCIDLib::TCard4 c4Count = colAccounts.c4ElemCount();
    if (!c4Count)
    {
        TOkBox msgbNone
        (
            strWndText(), facCQCIGKit().strMsg(kIGKitMsgs::midStatus_NoEMAccounts)
        );
        msgbNone.ShowIt(*this);
        return kCIDLib::False;
    }

    // Let them select one, using the generic list selection dialog
    const TString strTitle(kIGKitMsgs::midTitle_SelEMailAcct, facCQCIGKit());
    strToFill = strOrgVal;
    return facCIDWUtils().bListSelect(*this, strTitle, colAccounts, strToFill);
}


//
//  This is called when the current parameter is either the value of a field that has
//  an enumerated limit, or the parameter itself has an enumerated limit, and the
//  number of values is fairly small (otherwise a dialog is used.) These both in turn
//  call one that just takes the list of values to select from.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectFldEnum( const   TCQCFldEnumLimit&   fldlSrc
                                , const tCIDLib::TCard4     c4ParmInd
                                ,       TString&            strToFill)
{
    tCIDLib::TStrList colEnumVals;
    fldlSrc.c4QueryValues(colEnumVals);

    //
    //  Calculate where we want the menu to be at (to the right of the value
    //  button that caused us to be called.)
    //
    TPoint pntAt;
    ToScreenCoordinates(m_apwndPrefs[c4ParmInd]->areaWnd().pntUR(), pntAt);
    pntAt.Adjust(2, 0);

    // Call the generic value now
    tCIDLib::TCard4 c4SelIndex;
    if (bSelectEnum(colEnumVals, c4SelIndex, pntAt))
    {
        strToFill = colEnumVals[c4SelIndex];
        return kCIDLib::True;
    }
    return kCIDLib::False;
}

tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectParmEnum(const   TCQCCmdCfg&     ccfgSrc
                                , const TString&        strEventId
                                , const tCIDLib::TCard4 c4ParmInd
                                ,       TString&        strToFill)
{
    // Try to find the command. If not found, just return false
    TCQCCmd cmdSrc;
    MCQCCmdTarIntf* pmctarCur;
    if (!bFindCmd(ccfgSrc, strEventId, cmdSrc, pmctarCur))
        return kCIDLib::False;

    // Ask the parameter for a list of the enum values
    tCIDLib::TKVPList colEnumVals;
    const TCQCCmdParm& cmdpSrc = cmdSrc.cmdpAt(c4ParmInd);
    cmdpSrc.QueryEnumVals(colEnumVals);
    colEnumVals.Sort(TKeyValuePair::eCompKeyI);

    // We ahve to build up a string list of just the display values to pass to the helper
    const tCIDLib::TCard4 c4ValCount = colEnumVals.c4ElemCount();
    tCIDLib::TStrList colSelVals(c4ValCount);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCount; c4Index++)
        colSelVals.objAdd(colEnumVals[c4Index].strKey());

    //
    //  Calculate where we want the menu to be at (to the right of the value
    //  button that caused us to be called.)
    //
    TPoint pntAt;
    ToScreenCoordinates(m_apwndPrefs[c4ParmInd]->areaWnd().pntUR(), pntAt);
    pntAt.Adjust(2, 0);

    //
    //  Call the generic value now, giving it the display values. If they make a selection
    //  store the corresponding internal value.
    //
    tCIDLib::TCard4 c4Index;
    if (bSelectEnum(colSelVals, c4Index, pntAt))
    {
        strToFill = colEnumVals[c4Index].strValue();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}

tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectEnum(const   tCIDLib::TStrList&  colValues
                            ,       tCIDLib::TCard4&    c4SelIndex
                            , const TPoint&             pntAt)
{
    // It's possible there might not be any values available
    if (colValues.bIsEmpty())
        return kCIDLib::False;

    //
    //  And now let the user select from this list. We do it via a simple
    //  popup menu.
    //
    tCIDLib::TBoolean bRes = kCIDLib::False;
    TPopupMenu menuEnum(L"Enum Value Select");
    try
    {
        menuEnum.Create();

        // Fill in the menu with the values
        const tCIDLib::TCard4 c4Count = colValues.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            menuEnum.AddActionItem(TString::strEmpty(), colValues[c4Index], c4Index + 1000);

        // Run the menu and if they select something, give back that value
        tCIDLib::TCard4 c4Res = menuEnum.c4Process
        (
            *this, pntAt, tCIDLib::EVJustify::Top, tCIDLib::EHJustify::Left
        );

        if (c4Res != kCIDLib::c4MaxCard)
        {
            c4SelIndex = c4Res - 1000;
            bRes = kCIDLib::True;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        return kCIDLib::False;
    }
    return bRes;
}


//
//  This is called when the current parameter is an file type, and the
//  user presses the select button. We pop up the standard local file
//  browser variant of the file open dialog.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectFile(const   TCQCCmdCfg&     ccfgSrc
                            , const TString&        strEventId
                            , const tCIDLib::TCard4 c4ParmInd
                            , const TString&        strOrgVal
                            ,       TString&        strToFill)
{
    // Try to find the command. If not found, just return false
    TCQCCmd cmdSrc;
    MCQCCmdTarIntf* pmctarCur;
    if (!bFindCmd(ccfgSrc, strEventId, cmdSrc, pmctarCur))
        return kCIDLib::False;

    // Set up a file type list
    tCIDLib::TKVPList colTypes;
    colTypes.objPlace
    (
        cmdSrc.cmdpAt(c4ParmInd).strExtra(), cmdSrc.cmdpAt(c4ParmInd).strExtra()
    );

    TString strRealVal(strOrgVal);
    SwizzlePath(strRealVal, kCIDLib::True);

    tCIDLib::TStrList colSel;
    const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
    (
        *this
        , facCQCIGKit().strMsg(kIGKitMsgs::midTitle_SelFile)
        , strRealVal
        , colSel
        , colTypes
        , tCIDCtrls::EFOpenOpts::FileMustExist
    );

    if (bRes)
    {
        strToFill = colSel[0];
        SwizzlePath(strToFill, kCIDLib::False);
    }
    return bRes;
}


//
//  Called to allow the user to select a new currently running interface
//  viewer. We just iterate the currently available control interfaces
//  for IVs in the name server and offer up those binding names for
//  selection.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectIV(const TString& strOrgVal, TString& strToFill)
{
    const TString strTitle(kIGKitMsgs::midTitle_SelIntfViewer, facCQCIGKit());

    try
    {
        // Enumerate the scope where the IV's register themselves
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();
        TVector<TNameServerInfo> colList;
        const tCIDLib::TCard4 c4BindCnt = orbcNS->c4EnumObjects
        (
            TIntfCtrlClientProxy::strScope, colList, kCIDLib::False
        );

        // Fill up a list of the binding names
        tCIDLib::TStrList colNames;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4BindCnt; c4Index++)
            colNames.objAdd(colList[c4Index].strNodeName());

        // And we use the generic list selection dialog
        TString strNew = strOrgVal;
        if (facCIDWUtils().bListSelect(*this, strTitle, colNames, strNew))
        {
            // We return the id from the same index
            strToFill = strNew;
            return kCIDLib::True;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this, facCQCIGKit().strMsg(kIGKitErrs::errcCmd_SelectIV), errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::False;
}


//
//  Allows the user to select a local path
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectPath(const TString& strOrgVal, TString& strToFill)
{
    // Set up a file type list
    tCIDLib::TKVPList colTypes;
    colTypes.objPlace(L"Path", L"*.*");

    TString strRealVal(strOrgVal);
    SwizzlePath(strRealVal, kCIDLib::True);

    tCIDLib::TStrList colSel;
    const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
    (
        *this
        , facCQCIGKit().strMsg(kIGKitMsgs::midTitle_SelPath)
        , strRealVal
        , colSel
        , colTypes
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EFOpenOpts::SelectFolders, tCIDCtrls::EFOpenOpts::PathMustExist
          )
    );

    if (bRes)
    {
        strToFill = colSel[0];
        SwizzlePath(strToFill, kCIDLib::False);
    }
    return bRes;
}


//
//  This is called when the current parameter is a target sub-action type,
//  and the user presses the select button. We ask the command target whose
//  command is being configured what sub-actions he has available and
//  present them to the user for selection. We use the simple list dialog
//  to get the selection.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectSubAction(const  MCQCCmdTarIntf&     mctarCur
                                , const TString&            strCmdId
                                , const tCQCKit::EActCmdCtx eCmdCtx
                                , const TString&            strOrgVal
                                ,       TString&            strToFill)
{
    // And ask the source for the list. If none, then just do nothing
    TVector<TString> colNames;
    if (!mctarCur.bQueryTarSubActNames(colNames, strCmdId, eCmdCtx))
        return kCIDLib::False;

    // OK, we have some so do the selection and return his return
    strToFill = strOrgVal;
    return facCIDWUtils().bListSelect
    (
        *this, facCQCIGKit().strMsg(kIGKitMsgs::midTitle_SelAction), colNames, strToFill
    );
}


//
//  This is called when the current parameter is an tray monitor server binding.
//  We find all the tray monitorsand present a list to the user to select from.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectTrayMonBinding(const TString& strOrgVal, TString& strToFill)
{
    const TString strTitle(kIGKitMsgs::midTitle_SelTrayMon, facCQCIGKit());

    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Enumerate the scope where the app ctrl servers register themselves
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();
        TVector<TNameServerInfo> colList;
        const tCIDLib::TCard4 c4BindCnt = orbcNS->c4EnumObjects
        (
            TAppCtrlClientProxy::strImplScope, colList, kCIDLib::False
        );

        // Fill up a list of the binding names
        tCIDLib::TStrList colNames;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4BindCnt; c4Index++)
            colNames.objAdd(colList[c4Index].strNodeName());

        // And we use the generic list selection dialog
        strToFill = strOrgVal;
        bRet = facCIDWUtils().bListSelect(*this, strTitle, colNames, strToFill);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this, facCQCIGKit().strMsg(kIGKitErrs::errcCmd_SelectTM), errToCatch
        );
        bRet = kCIDLib::False;
    }
    return bRet;
}



//
//  This is called when the current parameter is a variable name type, and the user
//  presses the select button or if he pops up the menu and selects to insert a
//  variable. The dialog will handle figuring out if the original value is variable
//  or variable ref and so forth.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bSelectVariable(const   TString&            strOrgVal
                                ,       TString&            strToFill
                                , const tCIDLib::TBoolean   bNameOnly)
{
    //
    //  The facility class has a helper to present the dialog. Pass in the original
    //  value in case it's a variable or variable ref.
    //
    tCIDLib::TBoolean bByValue = kCIDLib::False;
    strToFill = strOrgVal;
    const tCIDLib::TBoolean bRes = facCQCIGKit().bManageVars
    (
        *this, m_cuctxToUse.strUserName(), strToFill, bByValue, bNameOnly, m_cuctxToUse
    );

    if (bRes)
    {
        strToFill.Clear();

        if (bByValue)
            strToFill = L"%(";
        strToFill.Append(facCQCIGKit().strTopOfCheatSheet());
        if (bByValue)
            strToFill.Append(kCIDLib::chCloseParen);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Handle button clicks from the user. There are lots of them.
//
tCIDCtrls::EEvResponses TCQCEditCmdDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    try
    {
        if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Cancel)
        {
            // If there are changes, confirm that they want to exit
            if (bCheckExit())
                EndDlg(kCQCIGKit::ridDlg_EdCmd_Cancel);
        }
         else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Comment)
        {
            TActOpcode aocNew(tCQCKit::EActOps::Comment);
            aocNew.m_ccfgStep.c4ParmCnt(1);
            aocNew.m_ccfgStep.SetParmAt(0, TString::strEmpty(), tCQCKit::ECmdPTypes::Text);
            InsertCmd(aocNew, facCIDCtrls().bCtrlShifted());
        }
         else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Change)
        {
            TPoint pntAt(m_pwndChangeButton->areaWnd().pntUR());
            pntAt.Adjust(4, 0);
            TWindow::wndDesktop().XlatCoordinates(pntAt, *this);

            ChangeCmd(pntAt);
        }
         else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Cmd)
        {
            //
            //  They want to insert a new command at the current position, so
            //  invoke the menu that let's them select a command. We have to
            //  translate the point to the right of the button that invoked
            //  us into screen coodinates for the menu to pop up at. We tell
            //  it to only present commands that have side effects.
            //
            TCQCCmd cmdNew;
            const MCQCCmdTarIntf* pmctarNew;

            TPoint pntAt(m_pwndCmdButton->areaWnd().pntUR());
            pntAt.Adjust(4, 0);
            TWindow::wndDesktop().XlatCoordinates(pntAt, *this);
            if (bGetCommand(tCQCKit::ECmdTypes::SideEffect, pntAt, cmdNew, pmctarNew))
                InsertCmd(cmdNew, *pmctarNew, facCIDCtrls().bCtrlShifted());
        }
         else if ((wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Copy)
              ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Cut))
        {
            //
            //  We either want to just copy the current single command,
            //  or the contents of an If block. So see what the current
            //  opcode is.
            //
            const tCIDLib::TCard4 c4Start = m_pwndOpList->c4CurItem();
            if (c4Start != kCIDLib::c4MaxCard)
            {
                const tCIDLib::TBoolean bCopy
                (
                    wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Copy
                );

                // Looks like there's something there, so see what we need to do
                TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4Start);

                if (aocCur.m_eOpcode == tCQCKit::EActOps::If)
                {
                    // Find the end of this If statement
                    const tCIDLib::TCard4 c4End = c4FindCondEnd(c4Start, kCIDLib::True);
                    CopyCut(bCopy, c4Start, c4End);
                }
                 else
                {
                    // If it's not an Else or End, then do it
                    if ((aocCur.m_eOpcode != tCQCKit::EActOps::Else)
                    &&  (aocCur.m_eOpcode != tCQCKit::EActOps::End))
                    {
                        CopyCut(bCopy, kCIDLib::c4MaxCard, kCIDLib::c4MaxCard);
                    }
                }
            }
        }
         else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_CopyAll)
        {
            // Shouldn't be empty, but make sure
            if (!m_pcolCurEvOps->bIsEmpty())
                CopyCut(kCIDLib::True, 0, m_pcolCurEvOps->c4ElemCount() - 1);
        }
         else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_DeleteAll)
        {
            // Make absolutely sure
            TYesNoBox msgbAsk
            (
                strWndText() , facCQCIGKit().strMsg(kIGKitMsgs::midQ_DeleteAllCmds)
            );

            if (msgbAsk.bShowIt(*this))
            {
                // Clear the opcodes for the current event
                m_csrcEdit.colOpsAt(m_pwndEvents->c4CurItem()).RemoveAll();
                m_colCmdCache.RemoveAll();

                ClearOpList();
            }
        }
         else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Disable)
        {
            // Call a helper to do this work
            DisableCmds();
        }
         else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Replace)
        {
            DoReplace();
        }
         else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Reset)
        {
            // Nothing to do unless there are changes
            if (!m_pmcsrcOrg->bSameCmds(m_csrcEdit))
            {
                TYesNoBox msgbAsk
                (
                    strWndText(), facCQCIGKit().strMsg(kIGKitMsgs::midQ_ResetAction)
                );

                if (msgbAsk.bShowIt(*this))
                {
                    //
                    //  We just copy back over the commands from the original command
                    //  source, and reload the current event, forcing an event so that
                    //  our handler reloads the opcode list.
                    //
                    static_cast<MCQCCmdSrcIntf&>(m_csrcEdit) = *m_pmcsrcOrg;
                    m_pwndEvents->SelectByIndex(0, kCIDLib::True);
                }
            }
        }
         else if ((wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_If)
              ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_IfElse))
        {
            //
            //  They want to insert a conditional at the current position, so
            //  invoke the menu that let's them select a command. We have to
            //  translate the point to the right of the button that invoked
            //  us into screen coodinates for the menu to pop up at. We tell
            //  it to only present commands that return a conditional value.
            //  They can have side effects, but we don't set that as a
            //  required type flag.
            //
            TCQCCmd cmdNew;
            const MCQCCmdTarIntf* pmctarNew;

            TPoint  pntAt(pwndChildById(wnotEvent.widSource())->areaWnd().pntUR());
            pntAt.Adjust(4, 0);
            TWindow::wndDesktop().XlatCoordinates(pntAt, *this);
            if (bGetCommand(tCQCKit::ECmdTypes::Conditional, pntAt, cmdNew, pmctarNew))
            {
                InsertCond
                (
                    wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_IfElse
                    , cmdNew
                    , *pmctarNew
                    , facCIDCtrls().bCtrlShifted()
                );
            }
        }
         else if ((wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_MoveDn)
              ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_MoveUp))
        {
            // Call a helper to move the selected command up or down
            MoveUpDown(wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_MoveUp);
        }
         else if ((wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Paste)
              ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_PasteAll))
        {
            PasteOpcodes(wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_PasteAll);
        }
         else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Save)
        {
            // If required, make sure it has some commands
            if(m_bRequired && !m_csrcEdit.bHasAnyOps())
            {
                TErrBox msgbErr(facCQCKit().strMsg(kKitErrs::errcCmd_NoActCommands));
                msgbErr.ShowIt(*this);
            }
             else if (!m_pmcsrcOrg->bSameCmds(m_csrcEdit))
            {
                if (bValidate())
                {
                    *m_pmcsrcOrg = m_csrcEdit;
                    EndDlg(kCQCIGKit::ridDlg_EdCmd_Save);
                }
            }
             else
            {
                // Just act as though they cancelled since nothing to save
                EndDlg(kCQCIGKit::ridDlg_EdCmd_Cancel);
            }
        }
         else if ((wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_P1Pref)
              ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_P2Pref)
              ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_P3Pref)
              ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_P4Pref))
        {
            // They want to do visual selection of the parameter value, so try that
            SelParmVal(wnotEvent.widSource());
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this, L"An error occurred while processing user click", errToCatch
        );
    }

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We watch for changes in the parameter entry fields and update the selected
//  command config object on the fly as changes are made, and update the display
//  to show the changes.
//
//  We ellipsis parameters that are long so that they can at least see the start
//  of each parameter.
//
tCIDCtrls::EEvResponses TCQCEditCmdDlg::eEFHandler(TEFChangeInfo& wnotEvent)
{
    //
    //  It's going to be one of the parameter fields, so figure out what parameter
    //  index it is, and update the parameter with the new value of the entry field.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::EEFEvents::Changed)
    {
        //
        //  Get the selected opcode and it's configuration. It's posible that we'll get
        //  called here when entry fields are cleared (when the oplist is emptied), so make
        //  sure that list window item, opcode, and parm index are valid before we use this
        //  one.
        //
        //  Also make sure it's one of the text entry fields, and not one of the prefixes
        //  being updated.
        //
        //  We don't have to deal with comments specially here since we set enough info on
        //  the opcode code's command config object to make all this work (i.e. one parm of
        //  text type.)
        //
        //  To avoid a race condition when cleaning up the opcode list, we check both the
        //  event and opcode index. If either has been emptied, we don't do anything.
        //  This lets us use the current event index as a 'don't respond to EF changes'
        //  flag as well, which helps with some race conditions.
        //
        const tCIDLib::TCard4 c4OpIndex = m_pwndOpList->c4CurItem();
        if ((m_c4CurEvIndex != kCIDLib::c4MaxCard) && (c4OpIndex != kCIDLib::c4MaxCard))
        {
            TString strEventId;
            TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4OpIndex);
            TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;

            tCIDLib::TCard4 c4PInd = kCIDLib::c4MaxCard;
            if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_P1Text)
                c4PInd = 0;
            else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_P2Text)
                c4PInd = 1;
            else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_P3Text)
                c4PInd = 2;
            else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_P4Text)
                c4PInd = 3;

            if (c4PInd < ccfgCur.c4ParmCnt())
            {
                ccfgCur.piAt(c4PInd).m_strValue = m_apwndTexts[c4PInd]->strWndText();

                // Reformat this one and update the list window item with it
                TString strFmt;
                aocCur.FormatTo(strFmt, m_c4CurNesting);
                m_pwndOpList->SetColText(m_pwndOpList->c4CurItem(), 0, strFmt);
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We need to watch for changes in our lists and updated things accordingly.
//
tCIDCtrls::EEvResponses
TCQCEditCmdDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_Events)
    {
        //
        //  DO NOT react to this list being cleared by clearing the opcode list. This
        //  will cause race condition issues.
        //
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            //
            //  The selected event changed so we need to load the opcodes for this one.
            //  We stored the original event index as the item id of each combo box
            //  item, so we just pass that in.
            //
            LoadEventOps(wnotEvent.c4Id());
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdCmd_OpList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::DeleteReq)
        {
            //
            //  The user hit the delete key while on the command list (and with something
            //  currently selected.)
            //
            DeleteCurOp();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Update everything to reflect the newly selected opcode
            UpdateForSelection();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            // If it's an If, then this will toggle it's negation
            ToggleNegateAt(wnotEvent.c4Index());
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            ClearParmWidgets();
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Given the index of an event (in the original event list in the command source)
//  we load up the opcodes for that list.
//
tCIDLib::TVoid TCQCEditCmdDlg::LoadEventOps(const tCIDLib::TCard4 c4EvIndex)
{
    // Store the new event index and event id as the last selected ones
    m_c4CurEvIndex = c4EvIndex;
    m_strCurEventId = m_csrcEdit.strEventIdAt(c4EvIndex);

    // And get a pointer to the current opcodes
    m_pcolCurEvOps = &m_csrcEdit.colOpsAt(c4EvIndex);

    // Clear any existing opcodes and the command cache
    m_colCmdCache.RemoveAll();
    m_pwndOpList->RemoveAll();

    // Only need to do anything if this guy has any opcodes
    const tCIDLib::TCard4 c4OpCnt = m_pcolCurEvOps->c4ElemCount();
    if (c4OpCnt)
    {
        tCIDLib::TStrList colVals(1);
        colVals.objAdd(TString::strEmpty());

        // Initialize the previous op to an invalid value!
        tCQCKit::EActOps ePrevOp = tCQCKit::EActOps::Count;
        tCIDLib::TCard4  c4Nesting = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4OpCnt; c4Index++)
        {
            const TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4Index);

            // Based on the previous state, deal with the current opcode
            const tCQCKit::EActOps eCurOp = aocCur.m_eOpcode;
            switch(ePrevOp)
            {
                case tCQCKit::EActOps::Cmd :
                case tCQCKit::EActOps::Comment :
                    //
                    //  If we see an End or Else after a command or comment,
                    //  then the nesting goes down.
                    //
                    if ((eCurOp == tCQCKit::EActOps::End)
                    ||  (eCurOp == tCQCKit::EActOps::Else))
                    {
                        if (c4Nesting)
                            c4Nesting--;
                    }
                    break;

                case tCQCKit::EActOps::Else :
                    //
                    //  If we see a cmd, comment, or if after an else, then the
                    //  nesting goes up.
                    //
                    if ((eCurOp == tCQCKit::EActOps::Cmd)
                    ||  (eCurOp == tCQCKit::EActOps::Comment)
                    ||  (eCurOp == tCQCKit::EActOps::If))
                    {
                        c4Nesting++;
                    }
                    break;

                case tCQCKit::EActOps::End :
                    // If we send an end or else after an end, then nesting goes down
                    if ((eCurOp == tCQCKit::EActOps::End)
                    ||  (eCurOp == tCQCKit::EActOps::Else))
                    {
                        if (c4Nesting)
                            c4Nesting--;
                    }
                    break;

                case tCQCKit::EActOps::If :
                    // If we see a cmd, comment or If after an If, then it goes up
                    if ((eCurOp == tCQCKit::EActOps::Cmd)
                    ||  (eCurOp == tCQCKit::EActOps::Comment)
                    ||  (eCurOp == tCQCKit::EActOps::If))
                    {
                        c4Nesting++;
                    }
                    break;

                default :
                    // Nothing special to do for this one
                    break;
            };

            // Now make the current opcode the previous op
            ePrevOp = eCurOp;

            // Format this guy and add it to the list
            aocCur.FormatTo(colVals[0], c4Nesting);
            m_pwndOpList->c4AddItem(colVals, aocCur.m_c4Id);
        }
    }

    // Select the 0th entry if there were any
    if (c4OpCnt)
        m_pwndOpList->SelectByIndex(0);
}


//
//  This is called on save, to validate the current command info, so that we warn the
//  the user of potential problems. We also calculate the offsets for the jump related
//  opcodes and update them to reflect these calculated offsets.
//
//  To do the offset calcs, we use a stack. We have to count the distance between If/End,
//  If/Else, and Else/End pairs, and they can be nested of course, so the stack allows us
//  to count the opcodes between them. So we push the current index when we see an If so
//  that when we see an Else or End, we can get the index of the related If.
//
tCIDLib::TBoolean TCQCEditCmdDlg::bValidate()
{
    TFundStack<tCIDLib::TCard4> fcolOfs(64);
    TVector<TCQCCmd> colCmds;

    // We see if any of them are questionably large and warn about it
    tCIDLib::TBoolean bLargeWarn = kCIDLib::False;

    //
    //  Go through the list of events, and for each one check its opcodes. This index
    //  is only for the combo box. It's not necessarily in the order of the actual
    //  events. We stored the original index as the per item id. We use this index
    //  instead of just going directly to the original data so that we can move back
    //  to the event that has an error in it and get the offending command selected.
    //
    const tCIDLib::TCard4 c4EventCnt = m_pwndEvents->c4ItemCount();
    for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EventCnt; c4EvIndex++)
    {
        //
        //  Get the opcode list for this event. We stored the original event index
        //  as the id, so that we could let the combo box sort.
        //
        tCIDLib::TCard4 c4OrgIndex = m_pwndEvents->c4CurItemId();

        const TCQCActEvInfo& caeiCur = m_csrcEdit.caeiEventAt(c4OrgIndex);
        const TString& strEvId = caeiCur.m_strEventId;
        MCQCCmdSrcIntf::TOpcodeBlock* pcolOps
        (
            m_csrcEdit.pcolOpsForEvent(strEvId)
        );

        //
        //  Run through all of the opcodes and make sure that any required parms have
        //  something in them. We can't be sure if it's the right stuff, but make sure it's
        //  not empty.
        //
        TCQCCmd cmdTmp;
        MCQCCmdTarIntf* pmctarTmp;
        const tCIDLib::TCard4 c4OpCount = pcolOps->c4ElemCount();
        for (tCIDLib::TCard4 c4OpIndex = 0; c4OpIndex < c4OpCount; c4OpIndex++)
        {
            TActOpcode& aocCur = pcolOps->objAt(c4OpIndex);
            switch(aocCur.m_eOpcode)
            {
                case tCQCKit::EActOps::Cmd :
                {
                    //
                    //  If disabled, don't check it. Of coures make sure anything to do with
                    //  maintaining our validation context is still done, but at this point
                    //  there's none of that in individual commands, just conditionals.
                    //
                    if (!aocCur.m_bDisabled)
                    {
                        //
                        //  Get a list of valid commands for this event, so that we
                        //  can be sure that they haven't pasted some commands in
                        //  that otherwise would not be allowed in this event.
                        //
                        if (!bFindCmd(aocCur.m_ccfgStep, strEvId, cmdTmp, pmctarTmp))
                        {
                            TErrBox msgbErr
                            (
                                facCQCIGKit().strMsg
                                (
                                    kIGKitMsgs::midStatus_BadCmdForEvent
                                    , aocCur.m_ccfgStep.strName()
                                    , caeiCur.m_strName
                                )
                            );
                            msgbErr.ShowIt(*this);
                            m_pwndEvents->SelectByIndex(c4EvIndex);
                            m_pwndOpList->SelectByIndex(c4OpIndex);
                            return kCIDLib::False;
                        }

                        tCIDLib::TCard4 c4BadInd = kCIDLib::c4MaxCard;
                        if (!bCheckParms(aocCur, strEvId, c4BadInd))
                        {
                            m_pwndEvents->SelectByIndex(c4EvIndex);
                            m_pwndOpList->SelectByIndex(c4OpIndex);
                            if (c4BadInd != kCIDLib::c4MaxCard)
                                m_apwndTexts[c4BadInd]->TakeFocus();
                            return kCIDLib::False;
                        }
                    }
                    break;
                }

                case tCQCKit::EActOps::End :
                {
                    //
                    //  Pop the previous If/Else offset off the stack, and go
                    //  back and update it with the difference.
                    //
                    const tCIDLib::TCard4 c4Ofs = fcolOfs.tPop();
                    pcolOps->objAt(c4Ofs).m_i4Ofs = tCIDLib::TInt4(c4OpIndex - c4Ofs);
                    break;
                }

                case tCQCKit::EActOps::Else :
                {
                    //
                    //  Update the If we are for so that it jumps to here, then
                    //  replace the top of stack with the current offset, so
                    //  that the End will see this Else as it's precedent.
                    //
                    const tCIDLib::TCard4 c4IfOfs = fcolOfs.tPop();
                    pcolOps->objAt(c4IfOfs).m_i4Ofs = tCIDLib::TInt4(c4OpIndex - c4IfOfs);
                    fcolOfs.Push(c4OpIndex);
                    break;
                }

                case tCQCKit::EActOps::If :
                {
                    // Check the parms if not disabled
                    if (!aocCur.m_bDisabled)
                    {
                        tCIDLib::TCard4 c4BadInd = kCIDLib::c4MaxCard;
                        if (!bCheckParms(aocCur, strEvId, c4BadInd))
                        {
                            m_pwndEvents->SelectByIndex(c4EvIndex);
                            m_pwndOpList->SelectByIndex(c4OpIndex);
                            if (c4BadInd != kCIDLib::c4MaxCard)
                                m_apwndTexts[c4BadInd]->TakeFocus();
                            return kCIDLib::False;
                        }
                    }

                    // We need to push the current index
                    fcolOfs.Push(c4OpIndex);
                    break;
                }

                default :
                    // We can ignore this one
                    break;
            };
        }

        // If a large count of operations, warn them
        if (c4OpCount > 320)
            bLargeWarn = kCIDLib::True;
    }

    // The stack better be empty
    #if CID_DEBUG_ON
    if (!fcolOfs.bIsEmpty())
    {
        TErrBox msgbErr(L"The offset stack is not empty at the end of validation");
        msgbErr.ShowIt(*this);
    }
    #endif

    if (bLargeWarn)
    {
        TOkBox msgbWarn(facCQCIGKit().strMsg(kIGKitMsgs::midStatus_LargeAction));
        msgbWarn.ShowIt(*this);
    }
    return kCIDLib::True;
}
