//
// FILE NAME: CQCIGKit_EditCmdDlg2.cpp
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
//  This file implements some of the overflow from the main file because this dialog
//  is so complex.
//
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
#include    "CQCIGKit_GetCmdDelOpt_.hpp"
#include    "CQCIGKit_CmdReplaceDlg_.hpp"


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIGKit_EditCmdDlg
    {
        // -----------------------------------------------------------------------
        //  The clipboard format we use for cutting action commands to the CB
        // -----------------------------------------------------------------------
        const tCIDLib::TCh* const   pszActListCBFmt = L"FObj:CQCActionList";


        // -----------------------------------------------------------------------
        //  For keeping track of the last settings of the search/replace dialog
        // -----------------------------------------------------------------------
        tCIDLib::TBoolean   bFullMatch = kCIDLib::False;
        tCIDLib::TBoolean   bRegEx = kCIDLib::False;
        tCIDLib::TBoolean   bThisEvent = kCIDLib::True;
        TString      strFindStr;
        TString      strRepStr;
    }
}



// ---------------------------------------------------------------------------
//  TEditCmdDlg: Private, non-virtaul methods
// ---------------------------------------------------------------------------

//
//  When the user asks to change the target of a command, this is called.
//  We see if there are any other targets (non-special ones other than the
//  case of local/global variable targets), and present those as options
//  to select from. If they select one, we update the target op code with
//  the new target. We only get called here for command type opcodes.
//
//  For convenience, in case it's needed, we return true if we actually
//  changed the command, else false.
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bChangeTarget(          TOpBlock&       colOps
                                , const tCIDLib::TCard4 c4Index
                                , const TPoint&         pntAt)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;

    //
    //  Get the current target and it's type. If debugging making sure we
    //  didn't get called with a non-command opcode.
    //
    TActOpcode& aocCur = colOps[c4Index];
    CIDAssert
    (
        aocCur.m_eOpcode == tCQCKit::EActOps::Cmd
        , L"Change target can only be used on command opcodes"
    )

    // Look it up. If debugging, make sure we didn't get a bogus id
    const TString& strOrgTargetId = aocCur.m_ccfgStep.strTargetId();

    MCQCCmdTarIntf* pctarChange = pctarFindById(strOrgTargetId);
    CIDAssert(pctarChange != 0, L"The target of the change was not found");
    const TString& strOrgTargetType = pctarChange->strCmdTargetType();

    // Create a new menu to pop up with the targets
    TPopupMenu menuTars(L"Change Target Menu");
    menuTars.Create();

    // First add a Cancel item with the id of 1000, then a separator
    menuTars.AddActionItem(L"Cancel", L"Cancel", 1000);
    menuTars.AddSeparator();

    //
    //  We need to keep a list of target pointers in the same order that we add them to the
    //  menu (minus 1 for the Cancel item we added above) so that we can get back from
    //  the id of the selected menu item to the target. Can't be more of them that the
    //  current target count.
    //
    const tCIDLib::TCard4 c4TarCnt = m_colTargets.c4ElemCount();
    tCQCKit::TCmdTarList colAvailTars(tCIDLib::EAdoptOpts::NoAdopt, c4TarCnt);

    //
    //  Now add any of the possible candidates. We have one special case, where the
    //  GVar and LVar targets can be interchanged. Otherwise, we just search the
    //  non-special targets for more targets of the same type.
    //
    tCIDLib::TCard4 c4Added = 0;
    if (strOrgTargetId == kCQCKit::strCTarId_GVars)
    {
        // Find the index of the local vars target and add that guy
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarCnt; c4Index++)
        {
            MCQCCmdTarIntf& mctarCur = *m_colTargets[c4Index];
            if (mctarCur.strCmdTargetId() == kCQCKit::strCTarId_LVars)
            {
                menuTars.AddActionItem
                (
                    mctarCur.strCmdTargetName()
                    , mctarCur.strCmdTargetName()
                    , c4Added + 1001
                );
                c4Added++;
                colAvailTars.Add(&mctarCur);
                break;
            }
        }
    }
     else if (strOrgTargetId == kCQCKit::strCTarId_LVars)
    {
        // Find the index of the global vars target and add that guy
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarCnt; c4Index++)
        {
            MCQCCmdTarIntf& mctarCur = *m_colTargets[c4Index];
            if (mctarCur.strCmdTargetId() == kCQCKit::strCTarId_GVars)
            {
                menuTars.AddActionItem
                (
                    mctarCur.strCmdTargetName()
                    , mctarCur.strCmdTargetName()
                    , c4Added + 1
                );
                c4Added++;
                colAvailTars.Add(&mctarCur);
                break;
            }
        }
    }
     else
    {
        // Find any non-special targets of the same type
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarCnt; c4Index++)
        {
            MCQCCmdTarIntf& mctarCur = *m_colTargets[c4Index];
            if (mctarCur.bIsSpecialCmdTar())
                continue;

            // Skip the original target
            if (mctarCur.strCmdTargetId() == strOrgTargetId)
                continue;

            if (mctarCur.strCmdTargetType() != strOrgTargetType)
                continue;

            menuTars.AddActionItem
            (
                mctarCur.strCmdTargetName()
                , mctarCur.strCmdTargetName()
                , c4Added + 1
            );
            c4Added++;
            colAvailTars.Add(&mctarCur);
        }
    }

    // If we got any available targets, then let them choose
    if (c4Added)
    {
        //
        //  Ok, it's loaded, so pop up the menu. The last parms says top align,
        //  which should make it come out aligned with the top of the button
        //  that invoked us.
        //
        tCIDLib::TCard4 c4Res = menuTars.c4Process
        (
            *this, pntAt, tCIDLib::EVJustify::Bottom
        );

        // If they made a selection besides cancel, then return that info
        if (c4Res && (c4Res != 1000))
        {
            // We used 1 based ids, so sub by 1 to get the index
            c4Res--;

            // Update the target to use this new target
            aocCur.m_ccfgStep.SetTarget(*colAvailTars[c4Res]);

            TString strFmt;
            aocCur.FormatTo(strFmt, m_c4CurNesting);

            m_pwndOpList->SetColText(c4Index, 0, strFmt);
            bRes = kCIDLib::True;
        }
    }
    return bRes;
}


//
//  Called to see if the user wants to exit without changing, from the
//  cancel button or close request handler.
//
tCIDLib::TBoolean TCQCEditCmdDlg::bCheckExit() const
{
    // If nothing has changed, then obviously it's ok
    if (m_csrcEdit.bSameCmds(*m_pmcsrcOrg))
        return kCIDLib::True;

    // Else ask
    TYesNoBox msgbAsk
    (
        strWndText(), facCQCKit().strMsg(kKitMsgs::midQ_ExitWithoutSaving)
    );
    return msgbAsk.bShowIt(*this);
}


//
//  See if the passed value (which will be an action parameter) has any
//  replacement tokens. If so, we can't really use the value for accessing
//  the thing referenced since we don't know what the actual value is.
//
//  The passed value will be updated. If it has tokens it will be cleared,
//  else it's left as the original (fully expanded value.)
//
tCIDLib::TBoolean
TCQCEditCmdDlg::bHasTokens(MCQCCmdSrcIntf& mcsrcCmd, TString& strValue)
{
    TCQCCmdRTVSrc* pcrtsExp = mcsrcCmd.pcrtsMakeNew(m_cuctxToUse);
    TJanitor<TCQCCmdRTVSrc> janRTV(pcrtsExp);

    TString strNewVal;
    tCQCKit::ECmdPrepRes eRes = facCQCKit().eStdTokenReplace
    (
        strValue
        , pcrtsExp
        , 0
        , 0
        , strNewVal
        , tCQCKit::ETokRepFlags::None
    );

    //
    //  If it was changed or failed, then clear it and return true that
    //  we have tokens.
    //
    if (eRes != tCQCKit::ECmdPrepRes::Unchanged)
    {
        strValue.Clear();
        return kCIDLib::True;
    }

    // Appears to be a regular expanded value
    return kCIDLib::False;
}


//
//  Calculates the nesting level for the passed index. It might be max card, i.e.
//  there's no current opcode in the list so a query for the current index returned
//  max card. And it might be equal to the current opcode count, if they are inserting
//  at the end past any possible command that could influence the nesting. In either
//  case we return zero.
//
tCIDLib::TCard4
TCQCEditCmdDlg::c4CalcNesting(const tCIDLib::TCard4 c4ForIndex) const
{
    tCIDLib::TCard4 c4RetVal = 0;
    if ((c4ForIndex == kCIDLib::c4MaxCard) || (c4ForIndex == m_pcolCurEvOps->c4ElemCount()))
        return c4RetVal;

    //
    //  If we have any and the for index is beyond the 0th line (which has to be
    //  at the 0th nesting level.
    //
    if (!m_pcolCurEvOps->bIsEmpty() && (c4ForIndex > 0))
    {
        // Start the previous opcode on the 0th one, then loop from 1
        tCQCKit::EActOps ePrevOp = m_pcolCurEvOps->objAt(0).m_eOpcode;
        for (tCIDLib::TCard4 c4Index = 1; c4Index <= c4ForIndex; c4Index++)
        {
            const TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4Index);
            const tCQCKit::EActOps eCurOp = aocCur.m_eOpcode;

            // Based on the previous state, deal with the current opcode
            switch(ePrevOp)
            {
                case tCQCKit::EActOps::Comment :
                case tCQCKit::EActOps::Cmd :
                    //
                    //  If we see an End or Else after a command or comment,
                    //  then the nesting goes down.
                    //
                    if ((eCurOp == tCQCKit::EActOps::End)
                    ||  (eCurOp == tCQCKit::EActOps::Else))
                    {
                        c4RetVal--;
                    }
                    break;

                case tCQCKit::EActOps::Else :
                    //
                    //  If we see a cmd, comment, or an if after an else,
                    //  then the nesting goes up.
                    //
                    if ((eCurOp == tCQCKit::EActOps::Cmd)
                    ||  (eCurOp == tCQCKit::EActOps::Comment)
                    ||  (eCurOp == tCQCKit::EActOps::If))
                    {
                        c4RetVal++;
                    }
                    break;

                case tCQCKit::EActOps::End :
                    // If we send an end or else after an end, then nesting goes down
                    if ((eCurOp == tCQCKit::EActOps::End)
                    ||  (eCurOp == tCQCKit::EActOps::Else))
                    {
                        c4RetVal--;
                    }
                    break;

                case tCQCKit::EActOps::If :
                    // If we see a cmd, comment or If after an If, then it goes up
                    if ((eCurOp == tCQCKit::EActOps::Cmd)
                    ||  (eCurOp == tCQCKit::EActOps::Comment)
                    ||  (eCurOp == tCQCKit::EActOps::If))
                    {
                        c4RetVal++;
                    }
                    break;

                default :
                    // Nothing special for this one
                    break;
            };

            // Now make the current opcode the previous op
            ePrevOp = eCurOp;
        }
    }

    #if CID_DEBUG_ON
    if (c4RetVal > 256)
    {
        TErrBox msgbErr(strWndText(), L"The return value is too large");
        msgbErr.ShowIt(*this);
        c4RetVal = 0;
    }
    #endif
    return c4RetVal;
}


//
//  Calc the nesting for an item based on the previous item and the new
//  item.
//
tCIDLib::TCard4
TCQCEditCmdDlg::c4CalcNesting(  const   tCQCKit::EActOps    ePrevOp
                                , const tCIDLib::TCard4     c4PrevIndent
                                , const TActOpcode&         aocNew) const
{
    // If no previous op, then this one is easy
    if (ePrevOp == tCQCKit::EActOps::Count)
        return 0;

    // Based on the previous state, deal with the new opcode
    const tCQCKit::EActOps eCurOp = aocNew.m_eOpcode;
    tCIDLib::TCard4 c4RetVal = c4PrevIndent;
    switch(ePrevOp)
    {
        case tCQCKit::EActOps::Comment :
        case tCQCKit::EActOps::Cmd :
            //
            //  If we see an End or Else after a command or comment,
            //  then the nesting goes down.
            //
            if ((eCurOp == tCQCKit::EActOps::End)
            ||  (eCurOp == tCQCKit::EActOps::Else))
            {
                c4RetVal--;
            }
            break;

        case tCQCKit::EActOps::Else :
            //
            //  If we see a cmd, comment, or an if after an else,
            //  then the nesting goes up.
            //
            if ((eCurOp == tCQCKit::EActOps::Cmd)
            ||  (eCurOp == tCQCKit::EActOps::Comment)
            ||  (eCurOp == tCQCKit::EActOps::If))
            {
                c4RetVal++;
            }
            break;

        case tCQCKit::EActOps::End :
            // If we send an end or else after an end, then nesting goes down
            if ((eCurOp == tCQCKit::EActOps::End)
            ||  (eCurOp == tCQCKit::EActOps::Else))
            {
                c4RetVal--;
            }
            break;

        case tCQCKit::EActOps::If :
            // If we see a cmd, comment or If after an If, then it goes up
            if ((eCurOp == tCQCKit::EActOps::Cmd)
            ||  (eCurOp == tCQCKit::EActOps::Comment)
            ||  (eCurOp == tCQCKit::EActOps::If))
            {
                c4RetVal++;
            }
            break;

        default :
            // Nothing to do here
            break;
    }
    return c4RetVal;
}


//
//  Given an End or Else opcode, finds the matching previous Else or If
//  opcode that started that block. The bUltimate will make it ignore an
//  Else and go on to the If, when searching backwards from an End.
//
tCIDLib::TCard4
TCQCEditCmdDlg::c4FindCondBegin(const   tCIDLib::TCard4     c4End
                                , const tCIDLib::TBoolean   bUltimate) const
{
    CIDAssert(c4End > 0, L"Can't find conditional begin from 0th opcode");

    tCIDLib::TCard4 c4Nesting = 1;
    tCIDLib::TCard4 c4RetVal = c4End - 1;
    while (c4Nesting)
    {
        const TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4RetVal);

        if (aocCur.m_eOpcode == tCQCKit::EActOps::End)
        {
            c4Nesting++;
        }
         else if (aocCur.m_eOpcode == tCQCKit::EActOps::Else)
        {
            //
            //  We ignore Else if we are looking for the ultimate and
            //  we are at a nested level level, i.e. we only care if this
            //  would be the Else that we want to end on.
            //
            if (!bUltimate && (c4Nesting == 1))
                c4Nesting--;
        }
         else if (aocCur.m_eOpcode == tCQCKit::EActOps::If)
        {
            c4Nesting--;
        }

        if (c4Nesting)
        {
            if (!c4RetVal)
            {
                facCQCIGKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kIGKitErrs::errcCmd_UnmatchedCond
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                );
            }
            c4RetVal--;
        }
    }
    return c4RetVal;
}


//
//  Finds the end of the conditional starting at the passed index. We have to
//  deal with nested conditionals. If the bUltimate parm is true, then we
//  go all the way to the End of an If/Else/End, instead of stopping at the
//  Else.
//
//  There's another version below for doing the same sort of search through
//  the original opcode list format. While editing we work within the list
//  window and only save them out when needed, so this one is used during
//  live editing.
//
tCIDLib::TCard4
TCQCEditCmdDlg::c4FindCondEnd(  const   tCIDLib::TCard4     c4Start
                                , const tCIDLib::TBoolean   bUltimate) const
{

    tCIDLib::TCard4 c4Nesting = 1;
    tCIDLib::TCard4 c4RetVal = c4Start + 1;

    const tCIDLib::TCard4 c4OpCount = m_pcolCurEvOps->c4ElemCount();
    while (c4Nesting)
    {
        const TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4RetVal);
        if (aocCur.m_eOpcode == tCQCKit::EActOps::End)
        {
            c4Nesting--;
        }
         else if (aocCur.m_eOpcode == tCQCKit::EActOps::Else)
        {
            //
            //  We ignore Else if we are looking for the ultimate end and
            //  we are at a nested level, i.e. we only care if this would
            //  be the Else that we want to end on.
            //
            if (!bUltimate && (c4Nesting == 1))
                c4Nesting--;
        }
         else if (aocCur.m_eOpcode == tCQCKit::EActOps::If)
        {
            c4Nesting++;
        }

        if (c4Nesting)
        {
            c4RetVal++;
            if (c4RetVal >= c4OpCount)
            {
                facCQCIGKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kIGKitErrs::errcCmd_UnmatchedCond
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                );
            }
        }
    }
    return c4RetVal;
}


//
//  Same as above but searches an opcode list instead of the opcode list
//  window. So this is used before we've loaded the window. The one above
//  is used during editing (where we aren't changing the original list until
//  the user saves.)
//
tCIDLib::TCard4
TCQCEditCmdDlg::c4FindCondEnd(  const   MCQCCmdSrcIntf::TOpcodeBlock& colOps
                                , const tCIDLib::TCard4               c4Start
                                , const tCIDLib::TBoolean             bUltimate) const
{
    const tCIDLib::TCard4 c4OpCount = colOps.c4ElemCount();
    tCIDLib::TCard4 c4Nesting = 1;
    tCIDLib::TCard4 c4RetVal = c4Start + 1;
    while (c4Nesting)
    {
        const TActOpcode& aocCur = colOps[c4RetVal];

        if (aocCur.m_eOpcode == tCQCKit::EActOps::End)
        {
            c4Nesting--;
        }
         else if (aocCur.m_eOpcode == tCQCKit::EActOps::Else)
        {
            //
            //  We ignore Else if we are looking for the ultimate end and
            //  we are at a nested level level, i.e. we only care if this
            //  would be the Else that we want to end on.
            //
            if (!bUltimate && (c4Nesting == 1))
                c4Nesting--;
        }
         else if (aocCur.m_eOpcode == tCQCKit::EActOps::If)
        {
            c4Nesting++;
        }

        if (c4Nesting)
        {
            c4RetVal++;
            if (c4RetVal >= c4OpCount)
            {
                facCQCIGKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kIGKitErrs::errcCmd_UnmatchedCond
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                );
            }
        }
    }
    return c4RetVal;
}


//
//  When they press the Change button, we get the selected command, and see
//  how it can be changed and offer them a menu of options to change it.
//
tCIDLib::TVoid TCQCEditCmdDlg::ChangeCmd(const TPoint& pntAt)
{
    // If no selected command, just return
    const tCIDLib::TCard4 c4OpIndex = m_pwndOpList->c4CurItem();
    if (c4OpIndex == kCIDLib::c4MaxCard)
        return;

    // Let's see what type it is and let them modify it if possible
    TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4OpIndex);

    if ((aocCur.m_eOpcode == tCQCKit::EActOps::If)
    ||  (aocCur.m_eOpcode == tCQCKit::EActOps::Else)
    ||  (aocCur.m_eOpcode == tCQCKit::EActOps::End))
    {
        // Allow them to modify condition commands in various ways
        ChangeConditional(*m_pcolCurEvOps, c4OpIndex, pntAt);
    }
     else if (aocCur.m_eOpcode == tCQCKit::EActOps::Cmd)
    {
        //
        //  It's a command, so let them select a new target, so let's
        //  find any new targets of the same type as the original.
        //
        bChangeTarget(*m_pcolCurEvOps, c4OpIndex, pntAt);
    }
}


//
//  When the user asks to change a command and it's an If or Else, then this
//  is called. We allow them turn an If into an If/Else or to split the Else
//  of an If/Else off into a separate If.
//
tCIDLib::TVoid
TCQCEditCmdDlg::ChangeConditional(          TOpBlock&       colOps
                                    , const tCIDLib::TCard4 c4CurIndex
                                    , const TPoint&         pntAt)
{
    TActOpcode& aocCur = colOps[c4CurIndex];
    CIDAssert
    (
        (aocCur.m_eOpcode == tCQCKit::EActOps::If)
         || (aocCur.m_eOpcode == tCQCKit::EActOps::Else)
         || (aocCur.m_eOpcode == tCQCKit::EActOps::End)
        , L"Change conditional can only be used on If, Else, or End opcodes"
    )

    TActOpcode              aocNew;
    TCQCCmd                 cmdNew;
    const MCQCCmdTarIntf*   pmctarNew;
    TString                 strFmt;

    if ((aocCur.m_eOpcode == tCQCKit::EActOps::If)
    ||  (aocCur.m_eOpcode == tCQCKit::EActOps::Else))
    {
        //
        //  These effectively do the same thing. We are either changing the
        //  target command of an If, or changing an Else into a separate
        //  If. The only difference is that in the latter case we have to
        //  insert a new End command before the current index, to capp
        //  off the original If.
        //
        if (bGetCommand(tCQCKit::ECmdTypes::Conditional, pntAt, cmdNew, pmctarNew))
        {
            // Remember if we need to insert an End before we change the opcode
            const tCIDLib::TBoolean bAddEnd(aocCur.m_eOpcode == tCQCKit::EActOps::Else);

            aocCur.m_eOpcode = tCQCKit::EActOps::If;
            aocCur.m_ccfgStep.SetFrom(cmdNew, *m_pmcsrcOrg, pmctarNew);
            pmctarNew->SetDefParms(aocCur.m_ccfgStep);

            aocCur.FormatTo(strFmt, m_c4CurNesting);
            m_pwndOpList->SetColText(c4CurIndex, 0, strFmt);

            if (bAddEnd)
            {
                // Insert an End command before us to cap off the original If
                aocNew.m_eOpcode = tCQCKit::EActOps::End;
                aocNew.m_ccfgStep.Reset();
                aocNew.m_i4Ofs = 0;
                aocNew.FormatTo(strFmt, m_c4CurNesting);

                colOps.InsertAt(aocNew, c4CurIndex);

                tCIDLib::TStrList colCols(1);
                colCols.objAdd(strFmt);
                m_pwndOpList->c4AddItem(colCols, aocNew.m_c4Id, c4CurIndex);
            }

            //
            //  We need to force a reselect of the current item so that the
            //  parameters for the conditional command will get updated for
            //  the newly selected command.
            //
            m_pwndOpList->SelectByIndex(c4CurIndex, kCIDLib::True);
        }
    }
     else
    {
        //
        //  Insert a new else before the end, but only if the previous conditional
        //  is an If. If the previous one is an Else, then we are already in an
        //  If/Else and there's nothing to do.
        //
        const tCIDLib::TCard4 c4StartInd = c4FindCondBegin(c4CurIndex, kCIDLib::False);
        TActOpcode& aocStart = colOps[c4StartInd];

        if (aocStart.m_eOpcode == tCQCKit::EActOps::If)
        {
            TActOpcode aocNew;
            aocNew.m_eOpcode = tCQCKit::EActOps::Else;
            aocNew.m_ccfgStep.Reset();
            aocNew.m_i4Ofs = 0;
            aocNew.FormatTo(strFmt, m_c4CurNesting);

            colOps.InsertAt(aocNew, c4CurIndex);

            tCIDLib::TStrList colCols(1);
            colCols.objAdd(strFmt);
            m_pwndOpList->c4AddItem(colCols, aocNew.m_c4Id, c4CurIndex);
        }
    }

    // Refresh the list
    m_pwndOpList->ForceRepaint();
}


//
//  When the opcode list is cleared, or they select the special [end] item,
//  we need to clear out the parameter widgets and disable the entry field
//  and selection button.
//
tCIDLib::TVoid TCQCEditCmdDlg::ClearParmWidgets()
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCKit::c4MaxCmdParms; c4Index++)
    {
        m_apwndPrefs[c4Index]->ClearText();
        m_apwndPrefs[c4Index]->SetEnable(kCIDLib::False);
        m_apwndTexts[c4Index]->ClearText();
        m_apwndTexts[c4Index]->SetEnable(kCIDLib::False);
    }
}


// Clears the oplist for the current event
tCIDLib::TVoid TCQCEditCmdDlg::ClearOpList()
{
    // Don't send an event or it will cause a race condition
    m_pwndOpList->RemoveAll(kCIDLib::False);

    // And then clear the parm widgets manually ourself instead of depending on the event
    ClearParmWidgets();
}


//
//  This is called to copy/cut one or more opcodes to the clipboard. We just flatten
//  them to a memory buffer. We assume here that the caller has selected a valid range
//  of command indices. They can pass max card for either to get the first or last op
//  code.
//
tCIDLib::TVoid
TCQCEditCmdDlg::CopyCut(const   tCIDLib::TBoolean   bCopy
                        , const tCIDLib::TCard4     c4StartInd
                        , const tCIDLib::TCard4     c4EndInd)
{
    //
    //  If no opcodes, then nothing to do.
    //
    tCIDLib::TCard4 c4Count = m_pwndOpList->c4ItemCount();
    if (!c4Count)
        return;

    // Figure out the actual indices
    tCIDLib::TCard4 c4ActStart;
    tCIDLib::TCard4 c4ActEnd;
    if (c4StartInd == kCIDLib::c4MaxCard)
    {
        // They want to just do the current item
        c4ActStart = m_pwndOpList->c4CurItem();
        c4ActEnd = c4ActStart;
    }
     else if (c4EndInd == kCIDLib::c4MaxCard)
    {
        // They want to go from the passed start index, to the end of the list
        c4ActStart = c4StartInd;
        c4ActEnd = c4Count - 1;
    }
     else
    {
        // Just take the actual incoming
        c4ActStart = c4StartInd;
        c4ActEnd = c4EndInd;
    }

    // If the start is at the count, nothing to do
    if (c4ActStart >= c4Count)
        return;

    // If the end is past the count, clip it back
    if (c4ActEnd >= c4Count)
        c4ActEnd = c4Count - 1;

    // Get the opcodes to the clipboard
    OpCodesToClipboard(bCopy, c4ActStart, c4ActEnd);

    // If it was a cut, then remove the items
    if (!bCopy)
    {
        TWndPaintJanitor janPaint(m_pwndOpList);
        for (tCIDLib::TCard4 c4Index = c4ActStart; c4Index <= c4ActEnd; c4Index++)
        {
            //
            //  We just keep removing at the start index, since each time we remove
            //  one, the next one will slide down into this slot.
            //
            m_pcolCurEvOps->RemoveAt(c4ActStart);
            m_pwndOpList->RemoveAt(c4ActStart);
        }

        //
        //  Since the start position will almost always have been the selected item,
        //  get the current item. If none, then we deleted it, and need to put it back
        //  to the action start index, if there are any items left.
        //
        const tCIDLib::TCard4 c4Left = m_pwndOpList->c4ItemCount();
        if (c4Left)
        {
            if (m_pwndOpList->c4CurItem() == kCIDLib::c4MaxCard)
            {
                tCIDLib::TCard4 c4NewSel = c4ActStart;
                if (c4NewSel >= c4Left)
                    c4NewSel = c4Left - 1;
                m_pwndOpList->SelectByIndex(c4NewSel, kCIDLib::True);
            }
        }
    }
}


//
//  This method is called if the user selects delete and is on a conditional
//  opcode (If, Else, While, etc...) The caller already asked if they want to
//  discard the contained commands as well, or just delete the conditional
//  and move the contained commands left (to the nesting level of the cond
//  we are removing.)
//
tCIDLib::TVoid
TCQCEditCmdDlg::DeleteCondOp(const  tCIDLib::TCard4     c4Start
                            , const tCIDLib::TBoolean   bDiscard)
{
    TActOpcode& aocStart = m_pcolCurEvOps->objAt(c4Start);

    #if CID_DEBUG_ON
    if ((aocStart.m_eOpcode != tCQCKit::EActOps::If)
    &&  (aocStart.m_eOpcode != tCQCKit::EActOps::Else))
    {
        TErrBox msgbErr(L"DeleteCondOp() didn't get a condtional opcode index");
        msgbErr.ShowIt(*this);
        return;
    }
    #endif

    // We need to find the extent of the block we are going to process
    tCIDLib::TCard4 c4BeginInd = c4Start;
    tCIDLib::TCard4 c4EndInd = c4FindCondEnd(c4Start, kCIDLib::False);

    //
    //  If we stopped on an Else, then convert it an If, and move the end
    //  index back to the previous command.
    //
    //  Else, if we started on an Else, then convert it to an End for the If
    //  and move the start index forward to remove the stuff from there to
    //  the end.
    //
    //  Else, we had to have started on an If and are going to an End, so
    //  we just keep the begin/end indices.
    //
    TActOpcode& aocEnd = m_pcolCurEvOps->objAt(c4EndInd);
    TString strFmt;
    if (aocEnd.m_eOpcode == tCQCKit::EActOps::Else)
    {
        aocEnd = aocStart;
        aocEnd.FormatTo(strFmt, m_c4CurNesting);

        m_pwndOpList->SetColText(c4EndInd, 0, strFmt);
        if (bDiscard)
            c4EndInd--;
        else
            c4EndInd = c4BeginInd;
    }
     else if (aocStart.m_eOpcode == tCQCKit::EActOps::Else)
    {
        aocStart.Reset(tCQCKit::EActOps::End);
        aocStart.FormatTo(strFmt, m_c4CurNesting);

        m_pwndOpList->SetColText(c4BeginInd, 0, strFmt);
        if (bDiscard)
            c4BeginInd++;
        else
            c4BeginInd = c4EndInd;
    }

    //
    //  Ok, we know the range. If we are going to discard, then we can just dump the
    //  whole set. Else, we need to remove the first and last and then go through them
    //  and reformat them so that they get the new correct nesting level.
    //
    //  We need to insure during all of this that the list window doesn't send out
    //  selection change events. This risks index errors. So we indicate in all of the
    //  remove commands that they should not make another selection. We'll do that at
    //  the end.
    //
    TWndPaintJanitor janWnd(this);
    if (bDiscard)
    {
        //
        //  We just keep removing at the begin index, which is a good way to make sure
        //  we delete the right ones even as the indices are changing. Once we've updated
        //  the collection, then do the list window.
        //
        for (tCIDLib::TCard4 c4Index = c4BeginInd; c4Index <= c4EndInd; c4Index++)
            m_pcolCurEvOps->RemoveAt(c4BeginInd);
        m_pwndOpList->RemoveRange(c4BeginInd, c4EndInd);
    }
     else
    {
        // REMOVE THE END first since that leaves the beginning index still valid
        m_pcolCurEvOps->RemoveAt(c4EndInd);
        m_pwndOpList->RemoveAt(c4EndInd, kCIDLib::False);
        if (c4EndInd != c4BeginInd)
        {
            m_pcolCurEvOps->RemoveAt(c4BeginInd);
            m_pwndOpList->RemoveAt(c4BeginInd, kCIDLib::False);
        }
    }

    //
    //  If there are any items left, we have to make sure we get something reselected,
    //  since we may have deleted the selected item, and we need to reindent everything.
    //
    const tCIDLib::TCard4 c4Left = m_pwndOpList->c4ItemCount();
    if (c4Left)
    {
        ReindentAll(strFmt);
        if (m_pwndOpList->c4CurItem() == kCIDLib::c4MaxCard)
        {
            tCIDLib::TCard4 c4NewSel = c4BeginInd;
            if (c4NewSel >= c4Left)
                c4NewSel = c4Left - 1;
            m_pwndOpList->SelectByIndex(c4NewSel, kCIDLib::True);
        }
    }
}


//
//  Deletes the opcode at the indicated index, if it can be deleted. We cannot
//  delete End opcodes. There are also special cases. If we delete an Else, then
//  we convert the related If/Else into just an If (and we toss all the opcodes
//  it contains.) If we delete an If, then we delete all of the opcodes in the
//  If (and its Else if present.)
//
//  In the case of an If or Else, we ask the user if they want to keep the
//  contained opcodes and move then out to the level of the If, i.e. to just
//  get rid of the If(/Else) and keep the opcodes.
//
tCIDLib::TVoid TCQCEditCmdDlg::DeleteCurOp()
{
    //
    //  Get the currently selected opcode if any. There might not be anything
    //  in the list.
    //
    tCIDLib::TCard4 c4OpIndex = m_pwndOpList->c4CurItem();
    if (c4OpIndex == kCIDLib::c4MaxCard)
        return;

    // Looks like there's something there, so see what we need to do
    TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4OpIndex);

    // If it's an End of a conditional block, we just ignore it
    if (aocCur.m_eOpcode == tCQCKit::EActOps::End)
        return;

    // Else do the right thing based on what was selected
    if ((aocCur.m_eOpcode == tCQCKit::EActOps::If)
    ||  (aocCur.m_eOpcode == tCQCKit::EActOps::Else))
    {
        // Figure out what they want to do
        TGetCmdDelOptDlg dlgGetOpt;
        const tCQCIGKit::ECmdDelOpts eOpt = dlgGetOpt.eRunDlg(*this);
        if (eOpt != tCQCIGKit::ECmdDelOpts::Cancel)
            DeleteCondOp(m_pwndOpList->c4CurItem(), eOpt == tCQCIGKit::ECmdDelOpts::DelAll);
    }
     else if ((aocCur.m_eOpcode == tCQCKit::EActOps::Cmd)
          ||  (aocCur.m_eOpcode == tCQCKit::EActOps::Comment))
    {
        // Make sure they want to do it, and then do delete it
        TYesNoBox msgbAsk(facCQCIGKit().strMsg(kIGKitMsgs::midQ_DeleteCmd));
        if (msgbAsk.bShowIt(*this))
        {
            m_pcolCurEvOps->RemoveAt(c4OpIndex);
            m_pwndOpList->RemoveAt(c4OpIndex);

            // If there are any left, reselect the closest one
            const tCIDLib::TCard4 c4Count = m_pwndOpList->c4ItemCount();
            if (c4Count)
            {
                if (c4OpIndex >= c4Count)
                    c4OpIndex = c4Count - 1;
                m_pwndOpList->SelectByIndex(c4OpIndex);
            }
        }
    }
}


//
//  This is called when the user presses the disable button. It's actually
//  a toggle between enable and disable. If the current command is enabled,
//  we disable it, and vice versa.
//
//  Currently we only support doing this on single commands. If it's an
//  If, Else, or End, or a comment of course, we don't do anything.
//
tCIDLib::TVoid TCQCEditCmdDlg::DisableCmds()
{
    //
    //  If nothing selected, do nothing. Shouldn't happen since disable the button in
    //  that case, but just in case.
    //
    const tCIDLib::TCard4 c4OpIndex = m_pwndOpList->c4CurItem();
    if (c4OpIndex == kCIDLib::c4MaxCard)
        return;

    TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4OpIndex);

    // If it's a command type opcode, then we can do it, else ignore
    if (aocCur.m_eOpcode == tCQCKit::EActOps::Cmd)
    {
        if (aocCur.m_bDisabled)
            aocCur.m_bDisabled = kCIDLib::False;
        else
            aocCur.m_bDisabled = kCIDLib::True;

        // Force a reformat and redraw of this one
        TString strFmt;
        aocCur.FormatTo(strFmt, m_c4CurNesting);

        m_pwndOpList->SetColText(c4OpIndex, 0, strFmt);
    }

    // We also need to update the enable/disable button
    if (aocCur.m_bDisabled)
    {
        m_pwndDisableButton->strWndText
        (
            facCQCIGKit().strMsg(kIGKitMsgs::midDlg_EdCmd_Enable)
        );
    }
     else
    {
        m_pwndDisableButton->strWndText
        (
            facCQCIGKit().strMsg(kIGKitMsgs::midDlg_EdCmd_Disable)
        );
    }
}


//
//  Do a search and replace operation.
//
tCIDLib::TVoid TCQCEditCmdDlg::DoReplace()
{
    // Put up the search and replace dialog to get the info we need
    TActCmdReplaceDlg   dlgRep;
    const tCIDLib::TBoolean bRes = dlgRep.bRunDlg
    (
        *this
        , CQCIGKit_EditCmdDlg::bFullMatch
        , CQCIGKit_EditCmdDlg::bRegEx
        , CQCIGKit_EditCmdDlg::bThisEvent
        , CQCIGKit_EditCmdDlg::strFindStr
        , CQCIGKit_EditCmdDlg::strRepStr
    );

    // If they cancel, just return with no action
    if (!bRes)
        return;

    // If in reg ex mode set up the regular expression
    TRegEx regxFind;
    if (CQCIGKit_EditCmdDlg::bRegEx)
        regxFind.SetExpression(CQCIGKit_EditCmdDlg::strFindStr);

    //
    //  Ok, let's do it. We are either doing it for just the currently
    //  selected event, or all events. For the current event we have to
    //  change the stuff currently in the opcode list. For the others we
    //  change them in the command source.
    //
    //  One way or another wa always do the current event, so we'll first
    //  loop through the events and update them in the source, skipping
    //  the current one, then do the current one via the live data.
    //
    const tCIDLib::TCard4 c4EventCnt = m_csrcEdit.c4EventCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4EventCnt; c4Index++)
    {
        // If only doing the current event, skip the others
        if (CQCIGKit_EditCmdDlg::bThisEvent && (c4Index != m_c4CurEvIndex))
            continue;

        const TCQCActEvInfo& caeiCur = m_csrcEdit.caeiEventAt(c4Index);
        MCQCCmdSrcIntf::TOpcodeBlock* pcolOps = m_csrcEdit.pcolOpsForEvent
        (
            caeiCur.m_strEventId
        );

        // For each opcode, ask it to do a replace
        const tCIDLib::TCard4 c4OpCnt = pcolOps->c4ElemCount();
        for (tCIDLib::TCard4 c4OpInd = 0; c4OpInd < c4OpCnt; c4OpInd++)
        {
            TActOpcode& aocCur = pcolOps->objAt(c4OpInd);
            aocCur.m_ccfgStep.bSearchNReplace
            (
                CQCIGKit_EditCmdDlg::strFindStr
                , CQCIGKit_EditCmdDlg::strRepStr
                , CQCIGKit_EditCmdDlg::bRegEx
                , CQCIGKit_EditCmdDlg::bFullMatch
                , regxFind
            );
        }
    }

    //
    //  Let's run through the displayed opcodes and refresh them all. We can just
    //  use the re-indent method to do this.
    //
    TString strFmt;
    ReindentAll(strFmt);

    // Force a reselect of the current item to make the parameter info update
    m_pwndOpList->SelectByIndex(m_pwndOpList->c4CurItem(), kCIDLib::True);
}


//
//  Given a command config object and parameter index, we'll look up the
//  original command, find the param definition, and get the requested field
//  access setting from it and return that value.
//
tCQCKit::EReqAccess
TCQCEditCmdDlg::eFindReqFldAccess(  const   TCQCCmdCfg&     ccfgCur
                                    , const TString&        strEventId
                                    , const tCIDLib::TCard4 c4PInd)
{
    tCQCKit::EReqAccess eRet = tCQCKit::EReqAccess::ReadOrWrite;
    MCQCCmdTarIntf* pmctarCur;
    TCQCCmd cmdSrc;
    if (bFindCmd(ccfgCur, strEventId, cmdSrc, pmctarCur))
        eRet = cmdSrc.cmdpAt(c4PInd).eFldAccess();
    return eRet;
}


// Insert a new command into the list at the current position.
tCIDLib::TVoid
TCQCEditCmdDlg::InsertCmd(  const   TCQCCmd&            cmdNew
                            , const MCQCCmdTarIntf&     mctarNew
                            , const tCIDLib::TBoolean   bInsertAfter)
{
    TActOpcode aocNew(tCQCKit::EActOps::Cmd);

    aocNew.m_ccfgStep.SetFrom(cmdNew, *m_pmcsrcOrg, &mctarNew);
    mctarNew.SetDefParms(aocNew.m_ccfgStep);

    // Call the other version that takes an already built opcode
    InsertCmd(aocNew, bInsertAfter);
}


// Insert a new opcode at the current position
tCIDLib::TVoid
TCQCEditCmdDlg::InsertCmd(const TActOpcode& aocNew, const tCIDLib::TBoolean bInsertAfter)
{
    //
    //  Get info about the opcode before the insert point. That's either the current
    //  command if inserting after, or the one before if not.
    //
    tCIDLib::TCard4         c4Indent = 0;
    tCIDLib::TCard4         c4InsAt = 0;
    const tCIDLib::TCard4   c4OpCnt = m_pcolCurEvOps->c4ElemCount();
    tCQCKit::EActOps        ePrevOp = tCQCKit::EActOps::Count;

    //
    //  If we have any opcodes, then get the current opcode, possibly moving forward
    //  to insert after if asked, and possibly deal with interaction with the previous
    //  opcode.
    //
    if (c4OpCnt)
    {
        // Get the current item, as the initial insert point
        c4InsAt = m_pwndOpList->c4CurItem();

        //
        //  If they asked to insert after, and we aren't already at the end, then
        //  we move forward by one. Note that we check for <= the op count, because
        //  we may be inserting after the last command. So we can go up to the count
        //  as the insert point.
        //
        if (bInsertAfter && (c4InsAt + 1 <= c4OpCnt))
            c4InsAt++;

        // Calc the index level for the target opcode
        c4Indent = c4CalcNesting(c4InsAt);

        //
        //  If more than one opcode and we aren't no the 0th one, need to get the
        //  previous opcode, since that will affect the nesting.
        //
        if ((c4OpCnt > 1) && c4InsAt)
        {
            TActOpcode& aocPrev = m_pcolCurEvOps->objAt(c4InsAt - 1);
            ePrevOp = aocPrev.m_eOpcode;

            // Calc the new index based on the previous and current
            c4Indent = c4CalcNesting(ePrevOp, c4Indent, aocNew);
        }
    }

    //
    //  If the cur opcode is an End or Else, then we are inserting at the end of a
    //  conditional block, and the nesting level is one more than the current nesting
    //  level. Watch for an insert at the end, which won't be a valid index yet.
    //
    if (c4InsAt < c4OpCnt)
    {
        const tCQCKit::EActOps eCurOp = m_pcolCurEvOps->objAt(c4InsAt).m_eOpcode;
        if ((eCurOp == tCQCKit::EActOps::Else) || (eCurOp == tCQCKit::EActOps::End))
            c4Indent++;
    }

    // And now we can insert it
    m_pcolCurEvOps->InsertAt(aocNew, c4InsAt);
    TString strFmt;
    aocNew.FormatTo(strFmt, c4Indent);
    tCIDLib::TStrList colVals(1);
    colVals.objAdd(strFmt);
    m_pwndOpList->c4AddItem(colVals, aocNew.m_c4Id, c4InsAt);

    //
    //  If it has any parameter, then select it in the list. Then move the focus
    //  to the first parameter entry field.
    //
    if (aocNew.m_ccfgStep.c4ParmCnt())
    {
        m_pwndOpList->SelectByIndex(c4InsAt);
        m_apwndTexts[0]->TakeFocus();
    }
}


//
//  Insert a new conditional command into the list at the current location.
//
tCIDLib::TVoid
TCQCEditCmdDlg::InsertCond( const   tCIDLib::TBoolean   bHasElse
                            , const TCQCCmd&            cmdNew
                            , const MCQCCmdTarIntf&     mctarNew
                            , const tCIDLib::TBoolean   bInsertAfter)
{
    TString strFmt;

    TActOpcode aocNew(tCQCKit::EActOps::If);
    aocNew.m_ccfgStep.SetFrom(cmdNew, *m_pmcsrcOrg, &mctarNew);
    mctarNew.SetDefParms(aocNew.m_ccfgStep);

    //
    //  Get the current index. If they asked to insert after, see if we can move
    //  forward. Note that we check for <= the op count, because we can be inserting
    //  after the last opcode. Of course make sure the list isn't currently empty as
    //  well.
    //
    tCIDLib::TCard4         c4Indent = 0;
    tCIDLib::TCard4         c4InsAt = 0;
    const tCIDLib::TCard4   c4OpCnt = m_pcolCurEvOps->c4ElemCount();
    tCQCKit::EActOps        ePrevOp = tCQCKit::EActOps::Count;

    //
    //  If we have any opcodes, then we have to deal with the previous one, since
    //  that affects the nesting level. And we have to get the current index as the
    //  insert point, potentially bumping it up if asked to insert after.
    //
    if (c4OpCnt)
    {
        c4InsAt = m_pwndOpList->c4CurItem();

        //
        //  If asked to insert after, see if we can move up. Note that we can move
        //  up to the actual count, since we may be inserting after the last command
        //  which is both legal and necessary in order to add new commands.
        //
        if (bInsertAfter && (c4InsAt + 1 <= c4OpCnt))
            c4InsAt++;

        // Calc the index level for the target
        c4Indent = c4CalcNesting(c4InsAt);
    }

    //
    //  If the cur opcode is an End or Else, then we are inserting at the end of a
    //  conditional block, and the nesting level is one more than the current nesting
    //  level. Watch for insert at the end, which won't be a valid index yet.
    //
    if (c4InsAt < c4OpCnt)
    {
        const tCQCKit::EActOps eCurOp = m_pcolCurEvOps->objAt(c4InsAt).m_eOpcode;
        if ((eCurOp == tCQCKit::EActOps::Else) || (eCurOp == tCQCKit::EActOps::End))
            c4Indent++;
    }

    // Remember the initial IF index
    const tCIDLib::TCard4 c4IfIndex = c4InsAt;

    //
    //  We have to suppress list box events here, because the handler, once we put
    //  the If in, will try to find the Else and End, which it does as it keeps the
    //  button enable/disable states up to date.
    //
    //  Once we're done we'll select the if. This really only is an issue if the
    //  opcode list is currently empty because in that case the If we insert becomes
    //  the selected item.
    //

    m_pcolCurEvOps->InsertAt(aocNew, c4InsAt);
    aocNew.FormatTo(strFmt, c4Indent);
    tCIDLib::TStrList colVals(1);
    colVals.objAdd(strFmt);
    m_pwndOpList->c4AddItem(colVals, aocNew.m_c4Id, c4InsAt++, kCIDLib::False);

    // If it's an If/Else, then add the else
    if (bHasElse)
    {
        aocNew.m_eOpcode = tCQCKit::EActOps::Else;
        aocNew.m_ccfgStep.Reset();
        aocNew.m_i4Ofs = 0;

        m_pcolCurEvOps->InsertAt(aocNew, c4InsAt);
        aocNew.FormatTo(colVals[0], c4Indent);
        m_pwndOpList->c4AddItem(colVals, aocNew.m_c4Id, c4InsAt++, kCIDLib::False);
    }

    // And finally add an End
    aocNew.m_eOpcode = tCQCKit::EActOps::End;
    aocNew.m_ccfgStep.Reset();
    aocNew.m_i4Ofs = 0;

    m_pcolCurEvOps->InsertAt(aocNew, c4InsAt);
    aocNew.FormatTo(colVals[0], c4Indent);
    m_pwndOpList->c4AddItem(colVals, aocNew.m_c4Id, c4InsAt++, kCIDLib::False);

    //
    //  Select the If in the list. Then move the focus to the first parameter entry
    //  field. Just in case the list was empty, and the if is already selected, force
    //  an event, so that the parameter info and buttons get updated.
    //
    m_pwndOpList->SelectByIndex(c4IfIndex, kCIDLib::True);
    m_apwndTexts[0]->TakeFocus();
}


//
//  Called when the user wants to insert a driver moniker into a parameter, either from the
//  insert popup menu or via hot key.
//
tCIDLib::TVoid TCQCEditCmdDlg::InsertDriverMon(const tCIDLib::TCard4 c4ParmInd)
{
    //
    //  If there's a previous value, see if it's a valid field name. If so, we'll pass it
    //  in as the default. We also need to see if it's potentially a field reference.
    //
    //  If there's any selected text we use that. Else we get the whole text value.
    //
    TString strDriver;
    if (!m_apwndTexts[c4ParmInd]->bGetSelectedText(strDriver))
        strDriver = m_apwndTexts[c4ParmInd]->strWndText();

    if (facCQCIGKit().bSelectDriver(*this, strDriver))
        m_apwndTexts[c4ParmInd]->InsertText(strDriver, kCIDLib::True);
}


//
//  Called when the user wants to insert a field into a parameter, either from the insert
//  popup menu or via hot key.
//
tCIDLib::TVoid TCQCEditCmdDlg::InsertField(const tCIDLib::TCard4 c4ParmInd)
{
    tCIDLib::TBoolean bByValue = kCIDLib::False;

    //
    //  If there's a previous value, see if it's a valid field name. If so, we'll pass it
    //  in as the default. We also need to see if it's potentially a field reference.
    //
    //  If there's any selected text we use that. Else we get the whole text value.
    //
    TString strPrevVal;
    if (!m_apwndTexts[c4ParmInd]->bGetSelectedText(strPrevVal))
        strPrevVal = m_apwndTexts[c4ParmInd]->strWndText();

    //
    //  If the previous value looks reasonable, let's try to convert it into the raw
    //  moniker.field format.
    //
    TString strDriver, strField;
    if (!strPrevVal.bIsEmpty()
    &&   (strPrevVal[0] == kCIDLib::chDollarSign)
    &&   (strPrevVal.c4Length() > 3)
    &&   (strPrevVal[1] == kCIDLib::chOpenParen)
    &&   (strPrevVal.chLast() == kCIDLib::chCloseParen))
    {
        bByValue = kCIDLib::True;
        strPrevVal.Cut(0, 2);
        strPrevVal.DeleteLast();
    }

    if (!facCQCKit().bParseFldName(strPrevVal, strDriver, strField))
    {
        strDriver.Clear();
        strField.Clear();
    }

    // Select a field and insert it
    const tCIDLib::TBoolean bSel = facCQCIGKit().bSelectField
    (
        *this
        , new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite)
        , strDriver
        , strField
        , bByValue
        , kCIDLib::False
    );

    if (bSel)
    {
        TString strFmt;
        if (bByValue)
            strFmt = L"$(";
        strFmt.Append(strDriver);
        strFmt.Append(kCIDLib::chPeriod);
        strFmt.Append(strField);
        if (bByValue)
            strFmt.Append(L")");

        m_apwndTexts[c4ParmInd]->InsertText(strFmt, kCIDLib::True);
    }
}


//
//  Moves the currently selected opcode up or down. If it is a conditional
//  we move the whole block.
//
tCIDLib::TVoid TCQCEditCmdDlg::MoveUpDown(const tCIDLib::TBoolean bUp)
{
    // If we only have one or zero item, then nothing we can do
    const tCIDLib::TCard4 c4Count = m_pwndOpList->c4ItemCount();
    if (c4Count < 2)
        return;

    // Get the current selection
    tCIDLib::TCard4 c4CurInd = m_pwndOpList->c4CurItem();

    //
    //  Simplify by dealing with special cases. If we are at one end, and
    //  we can't move in the direction indicated, then just return.
    //
    if ((bUp && !c4CurInd) || (!bUp && (c4CurInd >= c4Count - 1)))
        return;

    //
    //  Get the opcode out and check the type. If not a conditional, then we
    //  can do the simple scenario.
    //
    const TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4CurInd);
    TString strFmt;
    if ((aocCur.m_eOpcode == tCQCKit::EActOps::Cmd)
    ||  (aocCur.m_eOpcode == tCQCKit::EActOps::Comment))
    {
        // It's the simple scenario of a single opcode
        tCIDLib::TCard4 c4NewInd = c4CurInd;
        if (bUp)
            c4NewInd--;
        else
            c4NewInd++;

        //
        //  Swap the items in the collection, then do a reindent. We don't really
        //  need to re-indent. But it gets the list item's text updated to match
        //  the new order of the items in teh collection.
        //
        m_pcolCurEvOps->SwapItems(c4CurInd, c4NewInd);
        Reindent(c4CurInd, c4NewInd, strFmt);

        // Select the new index
        m_pwndOpList->SelectByIndex(c4NewInd);
    }
     else if (aocCur.m_eOpcode == tCQCKit::EActOps::If)
    {
        //
        //  It's the harder one. So we have to find the end of this block
        //  and move the whole thing up or down. Actually, what we do is
        //  to find the thing on the opposite side of it, and move it in
        //  the opposite direction.
        //
        tCIDLib::TCard4 c4EndInd = c4FindCondEnd(c4CurInd, kCIDLib::True);

        //
        //  We have to do a block based move here.
        //
        //  We check that there are at least three items in the list, since to do
        //  any such move we have to have at least two items. And one of the items
        //  is clearly an if statement which can at the least involve an If and
        //  and End.
        //
        const tCIDLib::TCard4 c4Count = m_pwndOpList->c4ItemCount();
        if ((c4Count > 2) && (bUp || (c4EndInd < c4Count - 1)))
        {
            // Move the items in the collection
            m_pcolCurEvOps->BlockMove(c4CurInd, c4EndInd, bUp);

            // Reformat everything
            ReindentAll(strFmt);

            // Keep the selection on the If
            m_pwndOpList->SelectByIndex(bUp ? c4CurInd - 1 : c4CurInd + 1);
        }
    }
}


//
//  This is called to copy one or more opcodes to the clipboard. We just flatten them to
//  a memory buffer and put that on the clipboard under our own private format type. We
//  also store a formatted text version.
//
tCIDLib::TVoid
TCQCEditCmdDlg::OpCodesToClipboard( const   tCIDLib::TBoolean   bCopy
                                    , const tCIDLib::TCard4     c4StartInd
                                    , const tCIDLib::TCard4     c4EndInd)
{
    // If no opcodes, then nothing to do
    tCIDLib::TCard4 c4Count = m_pwndOpList->c4ItemCount();
    if (!c4Count)
        return;

    // Figure out the actual indices
    tCIDLib::TCard4 c4ActStart;
    tCIDLib::TCard4 c4ActEnd;
    if (c4StartInd == kCIDLib::c4MaxCard)
    {
        // They want to just do the current item
        c4ActStart = m_pwndOpList->c4CurItem();
        c4ActEnd = c4ActStart;
    }
     else if (c4EndInd == kCIDLib::c4MaxCard)
    {
        // They want to go from the passed start index, to the end of the list
        c4ActStart = c4StartInd;
        c4ActEnd = c4Count - 1;
    }
     else
    {
        // Just take the actual incoming
        c4ActStart = c4StartInd;
        c4ActEnd = c4EndInd;
    }

    // If the start is at the count, nothing to do
    if (c4ActStart >= c4Count)
        return;

    // If the end is past the count, clip it back
    if (c4ActEnd >= c4Count)
        c4ActEnd = c4Count - 1;

    // Format the action text and copy to the clipboard
    TString strFmt;
    TTextStringOutStream strmFmt(2048UL);
    strmFmt.eNewLineType(tCIDLib::ENewLineTypes::CRLF);
    for (tCIDLib::TCard4 c4Index = c4ActStart; c4Index <= c4ActEnd; c4Index++)
    {
        //
        //  Get the current command and format it out. We format it out
        //  without the ellipsed text that we used when creating the display
        //  text.
        //
        m_pcolCurEvOps->objAt(c4Index).FormatLongTo(strFmt, kCIDLib::False);
        strmFmt << strFmt << kCIDLib::NewLn;
    }
    strmFmt.Flush();

    //
    //  And do the binary version. We have to flatten the list of opcodes to a memory
    //  buffer and then put it on the clipboard as a flattened object.
    //
    tCIDLib::TCard4 c4BinSz;
    THeapBuf mbufActs(8192);
    {
        TBinMBufOutStream strmOut(&mbufActs);

        c4Count = (c4ActEnd - c4ActStart) + 1;
        strmOut << c4Count << tCIDLib::TCard4(c4Count ^ kCIDLib::c4MaxCard);
        for (tCIDLib::TCard4 c4Index = c4ActStart; c4Index <= c4ActEnd; c4Index++)
        {
            strmOut << m_pcolCurEvOps->objAt(c4Index)
                    << tCIDLib::EStreamMarkers::Frame;
        }

        strmOut.Flush();
        c4BinSz = strmOut.c4CurSize();
    }

    // Create the clipboard and store both formats
    try
    {
        TGUIClipboard gclipTmp(*this);
        gclipTmp.Clear();
        gclipTmp.StoreText(strmFmt.strData());
        gclipTmp.StoreFlatObject(CQCIGKit_EditCmdDlg::pszActListCBFmt, mbufActs, c4BinSz);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this
            , facCQCIGKit().strMsg(kIGKitErrs::errcCmd_ClipError)
            , errToCatch
        );
    }
}


//
//  A helper to find a target in our target list by its text id. Note that
//  the numeric ids are not used at design time, only the text ids.
//
MCQCCmdTarIntf* TCQCEditCmdDlg::pctarFindById(const TString& strIdToFind)
{
    const tCIDLib::TCard4 c4Count = m_colTargets.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colTargets[c4Index]->strCmdTargetId() == strIdToFind)
            return m_colTargets[c4Index];
    }
    return 0;
}


//
//  Called when the user does a paste of opcodes.
//
tCIDLib::TVoid TCQCEditCmdDlg::PasteOpcodes(const tCIDLib::TBoolean bPasteAll)
{
    try
    {
        // See if there the data we want is available
        tCIDLib::TCard4 c4Sz;
        THeapBuf        mbufData;
        {
            TGUIClipboard gclipTmp(*this);
            if (!gclipTmp.bIsFormatAvailable(tCIDCtrls::EClipFormats::FlatObj))
                return;

            // Get the data if it's of the type we need
            const tCIDLib::TBoolean bRes = gclipTmp.bGetAsFlatObject
            (
                CQCIGKit_EditCmdDlg::pszActListCBFmt, mbufData, c4Sz
            );

            // If not, then nothing to do
            if (!bRes)
                return;
        }

        // We got something, so try to stream in the opcodes
        TBinMBufInStream strmSrc(&mbufData, c4Sz);
        tCIDLib::TCard4 c4Cnt, c4XORCnt;
        strmSrc >> c4Cnt >> c4XORCnt;

        // It's scrambled somehow, so do nothing
        if (c4Cnt != (c4XORCnt ^ kCIDLib::c4MaxCard))
        {
            facCQCIGKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIGKitErrs::errcCmd_BasePasteData
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
            return;
        }

        //
        //  Get the current item, if any, and figure out the indent level
        //  of the item before starting point. We'll use this to reindent
        //  the items as we add them.
        //
        tCIDLib::TCard4     c4InsAt = 0;
        tCQCKit::EActOps    ePrevOp = tCQCKit::EActOps::Count;
        tCIDLib::TCard4     c4Indent = 0;
        {
            const tCIDLib::TCard4 c4Count = m_pwndOpList->c4ItemCount();
            if (c4Count)
            {
                c4InsAt = m_pwndOpList->c4CurItem();


                //
                //  If Ctrl-shifted, move up if we can. This can put us paste the last
                //  item, so that hey can add paste at the end.
                //
                if (facCIDCtrls().bCtrlShifted())
                    c4InsAt++;
            }
        }

        //
        //  If not the first line, init the previous opcode to the one before
        //  the insert point.
        //
        if (c4InsAt)
        {
            ePrevOp = m_pcolCurEvOps->objAt(c4InsAt - 1).m_eOpcode;
            c4Indent = c4CalcNesting(c4InsAt - 1);
        }

        TActOpcode  aocNew;
        tCIDLib::TStrList colVals(1);
        colVals.objAdd(TString::strEmpty());

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
        {
            strmSrc >> aocNew;
            strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);

            c4Indent = c4CalcNesting(ePrevOp, c4Indent, aocNew);
            aocNew.FormatTo(colVals[0], c4Indent);

            m_pcolCurEvOps->InsertAt(aocNew, c4InsAt);
            m_pwndOpList->c4AddItem(colVals, 0, c4InsAt++);

            ePrevOp = aocNew.m_eOpcode;
        }

        // If there's no current selection, then select the 0th item if we can
        if ((m_pwndOpList->c4CurItem() == kCIDLib::c4MaxCard)
        &&  (m_pwndOpList->c4ItemCount() > 0))
        {
            m_pwndOpList->SelectByIndex(0, kCIDLib::True);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this
            , facCQCIGKit().strMsg(kIGKitMsgs::midStatus_CantPaste)
            , errToCatch
        );
    }
}


//
//  After changes, we need to reindent the opcodes for display. We can do
//  a range or all of them. The all version just calls the range version wtih
//  the full range (minus the [end] item which doesn't need it and doesn't
//  have an opcode.)
//
tCIDLib::TVoid
TCQCEditCmdDlg::Reindent(const  tCIDLib::TCard4 c4BeginInd
                        , const tCIDLib::TCard4 c4EndInd
                        ,       TString&        strFmt)
{
    // Avoid blinkies, disable drawing until it's all done
    TWndPaintJanitor janPaint(this);

    // They could be out of order, so flip if needed
    tCIDLib::TCard4 c4First;
    tCIDLib::TCard4 c4Last;
    if (c4BeginInd > c4EndInd)
    {
        c4First = c4EndInd;
        c4Last = c4BeginInd;
    }
     else
    {
        c4First = c4BeginInd;
        c4Last = c4EndInd;
    }

    for (tCIDLib::TCard4 c4Index = c4First; c4Index <= c4Last; c4Index++)
    {
        TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4Index);
        aocCur.FormatTo(strFmt, c4CalcNesting(c4Index));
        m_pwndOpList->SetColText(c4Index, 0, strFmt);
    }
}

tCIDLib::TVoid TCQCEditCmdDlg::ReindentAll(TString& strFmt)
{
    // It wants an end index, not a count, so we sub 1.
    const tCIDLib::TCard4 c4Count = m_pwndOpList->c4ItemCount();
    if (c4Count > 0)
        Reindent(0, c4Count - 1, strFmt);
}


//
//  The use clicked one of the parameter value selection buttons. So, if the parameter
//  is of a type we can do it for, invoke the correct value selection operation. We
//  get the window if of the button that was clicked, which tells us which parameter.
//
tCIDLib::TVoid TCQCEditCmdDlg::SelParmVal(const tCIDCtrls::TWndId widSrc)
{
    // Figure out the parameter index
    tCIDLib::TCard4 c4PInd = kCIDLib::c4MaxCard;
    if (widSrc == kCQCIGKit::ridDlg_EdCmd_P1Pref)
        c4PInd = 0;
    else if (widSrc == kCQCIGKit::ridDlg_EdCmd_P2Pref)
        c4PInd = 1;
    else if (widSrc == kCQCIGKit::ridDlg_EdCmd_P3Pref)
        c4PInd = 2;
    else if (widSrc == kCQCIGKit::ridDlg_EdCmd_P4Pref)
        c4PInd = 3;

    CIDAssert(c4PInd != kCIDLib::c4MaxCard, L"Unknown parameter index");

    // Get the current opcode
    const tCIDLib::TCard4 c4OpIndex = m_pwndOpList->c4CurItem();
    TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4OpIndex);
    TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;

    // Get the type of the target parameter
    const tCQCKit::ECmdPTypes ePType = ccfgCur.piAt(c4PInd).m_eType;

    //
    //  See if it's a command that writes to a field and lets the user enter
    //  a value. If so, we want to let them browse for the value if the field
    //  is available for us to look it up and know what values to present. So
    //  we have to do some clunky code here because there's nothing in the
    //  actual commands themselves that would tip us off.
    //
    //  If so, we get out the target field info for use below.
    //
    tCIDLib::TBoolean bFldWriteVal = kCIDLib::True;
    TString strFldParm;
    if ((ccfgCur.strCmdId() == kCQCKit::strCmdId_FieldWrite) && (c4PInd == 1))
    {
        strFldParm = ccfgCur.piAt(0).m_strValue;
    }
     else if ((ccfgCur.strCmdId() == kCQCKit::strCmdId_FieldWrite2) && (c4PInd == 2))
    {
        // The field is in moniker/field form
        if (!ccfgCur.piAt(0).m_strValue.bIsEmpty()
        &&  !ccfgCur.piAt(1).m_strValue.bIsEmpty())
        {
            strFldParm = ccfgCur.piAt(0).m_strValue;
            strFldParm.Append(L'.');
            strFldParm.Append(ccfgCur.piAt(1).m_strValue);
        }
    }
     else if ((ccfgCur.strCmdId() == kCQCKit::strCmdId_TimedFldChange)
          &&  ((c4PInd == 1) || (c4PInd == 2)))
    {
        strFldParm = ccfgCur.piAt(0).m_strValue;
    }
     else if (((ccfgCur.strCmdId() == kCQCKit::strCmdId_SafeWaitFieldVal)
          ||   (ccfgCur.strCmdId() == kCQCKit::strCmdId_WaitFieldVal))
          &&  (c4PInd == 1))
    {
        strFldParm = ccfgCur.piAt(0).m_strValue;
    }
     else
    {
        // It's not one we want to treat this way
        bFldWriteVal = kCIDLib::False;
    }

    // Find the command target
    MCQCCmdTarIntf* pctarCur = pctarFindById(ccfgCur.strTargetId());

    // And get the context of the selected event
    const tCQCKit::EActCmdCtx eCmdCtx(m_pmcsrcOrg->eContextOfEvent(m_strCurEventId));

    //
    //  Get a copy of the original value for those methods below that
    //  want to pass it in for selection purposes.
    //
    const TString strOrgVal = ccfgCur.piAt(c4PInd).m_strValue;

    tCIDLib::TBoolean bNewVal = kCIDLib::False;
    TString strNewVal;
    if (bFldWriteVal)
    {
        //
        //  Get the remembered parameter that should have the field name and
        //  get the name out.
        //
        if (strFldParm.bIsEmpty())
            return;

        TString strTarFld;
        try
        {
            // There's something there, so try to expand it
            TCQCCmdRTVSrc* pcrtsExp = m_pmcsrcOrg->pcrtsMakeNew(m_cuctxToUse);
            TJanitor<TCQCCmdRTVSrc> janRTV(pcrtsExp);

            tCQCKit::ECmdPrepRes eRes = facCQCKit().eStdTokenReplace
            (
                strFldParm, pcrtsExp, 0, 0, strTarFld, tCQCKit::ETokRepFlags::None
            );

            if (eRes == tCQCKit::ECmdPrepRes::Unchanged)
                strTarFld = strFldParm;
            else if (eRes == tCQCKit::ECmdPrepRes::Failed)
                strTarFld.Clear();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                *this
                , facCQCIGKit().strMsg
                  (
                    kIGKitMsgs::midStatus_CantExpand, TCardinal(c4PInd + 1)
                  )
                , errToCatch
            );
            return;
        }

        // If it's empty, then it failed, so do nothing.
        if (strTarFld.bIsEmpty())
            return;

        //
        //  Ok, that point we should have a supposedly valid moniker.fld
        //  value, so try to break it out.
        //
        TString strMon;
        TString strFld;
        try
        {
            facCQCKit().ParseFldName(strTarFld, strMon, strFld);
        }

        catch(const TError& errToCatch)
        {
            TErrBox msgbErr(strWndText(), errToCatch.strErrText());
            msgbErr.ShowIt(*this);
            return;
        }

        //
        //  See if we can find this field out there in driver land and query
        //  the field info.
        //
        TCQCFldDef flddTar;
        tCQCKit::TCQCSrvProxy orbcQS;
        try
        {
            orbcQS = facCQCKit().orbcCQCSrvAdminProxy(strMon);
            orbcQS->QueryFieldDef(strMon, strFld, flddTar);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                *this
                , facCQCIGKit().strMsg(kIGKitMsgs::midStatus_CantFindField, strMon, strFld)
                , errToCatch
            );
            return;
        }

        //
        //  If the field name is the same as the IR drivers use for their
        //  command sending field, then see if it's an IR driver. If so,
        //  then we present the IR command selection dialog instead of the
        //  usual value selection dialog.
        //
        tCIDLib::TBoolean bIsIRDrv = kCIDLib::False;
        if (strFld == TFacCQCIR::strFldName_Invoke)
        {
            try
            {
                TCQCDriverObjCfg cqcdcTest;
                if (orbcQS->bQueryDriverInfo(strMon, cqcdcTest))
                {
                    const tCQCKit::EDevCats eDevCat = cqcdcTest.eCategory();
                    bIsIRDrv = (eDevCat == tCQCKit::EDevCats::IRBlaster)
                               || (eDevCat == tCQCKit::EDevCats::IRCombo);
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCGKit().ShowError
                (
                    *this
                    , facCQCIGKit().strMsg(kIGKitMsgs::midStatus_CantGetDrvType, strMon)
                    , errToCatch
                );
            }
        }

        // Get a field limits object for this field, which we need in a few places below
        TCQCFldLimit* pfldlOpt = TCQCFldLimit::pfldlMakeNew(flddTar);
        TJanitor<TCQCFldLimit> janLims(pfldlOpt);

        //
        //  So we special case IR devices, and we special case booleans and enums
        //  with reasonable numbers of values, which we can do as a drop down menu
        //  more conveniently.
        //
        if (bIsIRDrv)
        {
            //
            //  In this case, pass in the original value to be udpated if they make
            //  a selection. This lets it select the original value, and to retain
            //  any zone indicator across a command change.
            //
            strNewVal = strOrgVal;
            bNewVal = facCQCIGKit().bSelectIRCmd(*this, strMon, strNewVal);
        }
         else if (flddTar.eType() == tCQCKit::EFldTypes::Boolean)
        {
            bNewVal = bSelectBool(strNewVal, c4PInd);
        }
         else if ((flddTar.eType() == tCQCKit::EFldTypes::String)
              &&  pfldlOpt->bIsA(TCQCFldEnumLimit::clsThis())
              &&  (static_cast<TCQCFldEnumLimit*>(pfldlOpt)->c4ValCount() <= 24))
        {
            bNewVal = bSelectFldEnum
            (
                *static_cast<TCQCFldEnumLimit*>(pfldlOpt), c4PInd, strNewVal
            );
        }
         else
        {
            //
            //  Not a special case, so try to do a general value selection via a set
            //  of dialog boxes. We ask the field def for the optimal graphical
            //  representation for its field. If none, then we just return.
            //
            if (pfldlOpt->eOptimalRep() == tCQCKit::EOptFldReps::None)
            {
                TOkBox msgbOk
                (
                    strWndText(), facCQCIGKit().strMsg(kIGKitMsgs::midStatus_CantSelectVal)
                );
                msgbOk.ShowIt(*this);
                return;
            }

            //
            //  Invoke a generic field value getter. If the user selects a new value
            //  it'll return true and update strNewVal
            //
            bNewVal = facCQCGKit().bQueryFldValue
            (
                *this
                , flddTar
                , strMon
                , TPoint::pntOrigin
                , strNewVal
                , strOrgVal
            );
        }
    }
     else if (((ccfgCur.strCmdId() == kCQCKit::strCmdId_FieldWrite2)
          ||   (ccfgCur.strCmdId() == kCQCKit::strCmdId_FieldRead2))
          &&  (c4PInd == 1))
    {
        //
        //  In this special case the driver and field are separate parameters, but we
        //  let them still choose both. We'll update both parameters if they make a
        //  change.
        //
        //  So expand the both parameters and make sure they are something we can
        //  handle here at design time.
        //
        TString strDev;
        try
        {
            //
            //  There's something there, so try to expand it. If there are
            //  any tokens it will be cleared.
            //
            strDev = ccfgCur.piAt(0).m_strValue;
            bHasTokens(*m_pmcsrcOrg, strDev);

            strNewVal = ccfgCur.piAt(1).m_strValue;
            bHasTokens(*m_pmcsrcOrg, strNewVal);
        }

        catch(...)
        {
            // Just eat it and give up
            return;
        }

        //
        //  Seems reasonable to try to let them select one. Set the requested
        //  access based on the command we are in.
        //
        tCQCKit::EReqAccess eReqAcc;
        if (ccfgCur.strCmdId() == kCQCKit::strCmdId_FieldWrite2)
            eReqAcc = tCQCKit::EReqAccess::CReadMWrite;
        else
            eReqAcc = tCQCKit::EReqAccess::MReadCWrite;

        bNewVal = facCQCIGKit().bSelectField
        (
            *this, new TCQCFldFilter(eReqAcc), strDev, strNewVal
        );

        //
        //  If they selected, then update both parameters. We have to do the driver
        //  parameter ourself here since the code below doesn't understand more than
        //  the current parameter. We don't have to do the formatting and updating
        //  of the displayed command, since that'll be done below. We have to just
        //  get the parameter and parameter text field updated.
        //
        if (bNewVal)
        {
            ccfgCur.piAt(0).m_strValue = strDev;
            m_apwndTexts[0]->strWndText(strDev);
        }
    }
     else if (((ccfgCur.strCmdId() == L"IntfCmd:InvokePopup") && (c4PInd == 1))
          ||  ((ccfgCur.strCmdId() == L"IntfCmd:InvokePopout") && (c4PInd == 3))
          ||  ((ccfgCur.strCmdId() == L"IntfCmd:SetBlankOpts") && (c4PInd == 2)))
    {
        //
        //  This one we have to use the cmd name directly, because the defined cmd
        //  string exists above this level. We don't have a parm type for interface
        //  notes. It's not worth it given that there are only these two cmds.
        //
        //  But it does mean that if the command name ever changes we get broken
        //  unless someone remembers to come change this.
        //
        //  See if the template path parm is not empty and has no replacement tokens.
        //  If so, see if we can get the template info which has the notes in it.
        //
        TString strTmpl;
        {
            if (ccfgCur.strCmdId() == L"IntfCmd:SetBlankOpts")
                strTmpl = ccfgCur.piAt(1).m_strValue;
            else
                strTmpl = ccfgCur.piAt(0).m_strValue;

            if (strTmpl.bIsEmpty())
                return;

            //
            //  We have to expand it since it could be relative. If it works, take that.
            //  Else keep the original.
            //
            TString strExpPath;
            if (facCQCKit().bExpandResPath(m_strBasePath, strTmpl, strExpPath))
                strTmpl = strExpPath;
        }

        //
        //  If it has any tokens, we can't do this. De-swizzle our temp
        //  copy first, since the editor automatically doubles slashes
        //  in paths.
        //
        if (bHasTokens(*m_pmcsrcOrg, strTmpl))
            return;

        // Try to get the template info for the referenced template
        tCQCKit::EUserRoles eMinRole;
        TString             strNotes;
        if (bQueryTmplInfo(strTmpl, strNotes, eMinRole) && !strNotes.bIsEmpty())
        {
            TOkBox msgbNotes(L"Template Notes", strNotes);
            msgbNotes.ShowIt(*this);
        }
    }
     else
    {
        //
        //  According to the type, present a different type of selection dialog or
        //  menu. We only worry here with types that we can deal with. Others we just
        //  ignore.
        //
        switch(ePType)
        {
            case tCQCKit::ECmdPTypes::TrayMonBinding :
                bNewVal = bSelectTrayMonBinding(strOrgVal, strNewVal);
                break;

            case tCQCKit::ECmdPTypes::Application :
                bNewVal = bSelectApp(strOrgVal, strNewVal);
                break;

            case tCQCKit::ECmdPTypes::Area :
                break;

            case tCQCKit::ECmdPTypes::Boolean :
            {
                // Just show a simple true/false popup list to choose from
                bNewVal = bSelectBool(strNewVal, c4PInd);
                break;
            }

            case tCQCKit::ECmdPTypes::Color :
            {
                strNewVal = strOrgVal;
                bNewVal = bSelectColor(strNewVal);
                break;
            }

            case tCQCKit::ECmdPTypes::Driver :
            {
                strNewVal = strOrgVal;
                bNewVal = facCQCIGKit().bSelectDriver(*this, strNewVal);
                break;
            }

            case tCQCKit::ECmdPTypes::EMailAcct :
                bNewVal = bSelectEMailAcct(strOrgVal, strNewVal);
                break;

            case tCQCKit::ECmdPTypes::Enum :
                // Present a list of the enumerated values to choose from
                bNewVal = bSelectParmEnum(ccfgCur, m_strCurEventId, c4PInd, strNewVal);
                break;

            case tCQCKit::ECmdPTypes::EvMonId :
            case tCQCKit::ECmdPTypes::SchEvId :
            case tCQCKit::ECmdPTypes::TrgEvId :
            {
                tCQCRemBrws::EDTypes eDType = tCQCRemBrws::EDTypes::Count;
                if (ePType == tCQCKit::ECmdPTypes::EvMonId)
                    eDType = tCQCRemBrws::EDTypes::EvMonitor;
                else if (ePType == tCQCKit::ECmdPTypes::SchEvId)
                    eDType = tCQCRemBrws::EDTypes::SchEvent;
                else if (ePType == tCQCKit::ECmdPTypes::TrgEvId)
                    eDType = tCQCRemBrws::EDTypes::TrgEvent;

                bNewVal = facCQCTreeBrws().bSelectFile
                (
                    *this
                    , L"Select an Event"
                    , eDType
                    , strOrgVal
                    , m_cuctxToUse
                    , tCQCTreeBrws::EFSelFlags::SelectItems
                    , strNewVal
                );
                break;
            }

            case tCQCKit::ECmdPTypes::Field :
            {
                //
                //  Look up the original command and use that get the requested
                //  field access for this parameter. We will pass this to the
                //  field selection method in order to only present to the user
                //  fields that are valid for the requested access.
                //
                strNewVal = strOrgVal;
                bNewVal = facCQCIGKit().bSelectField
                (
                    *this
                    , new TCQCFldFilter(eFindReqFldAccess(ccfgCur, m_strCurEventId, c4PInd))
                    , strNewVal
                );
                break;
            }

            case tCQCKit::ECmdPTypes::File :
            {
                bNewVal = bSelectFile(ccfgCur, m_strCurEventId, c4PInd, strOrgVal, strNewVal);
                break;
            }

            case tCQCKit::ECmdPTypes::GlobAction :
            {
                bNewVal = facCQCTreeBrws().bSelectFile
                (
                    *this
                    , L"Select a Global Action"
                    , tCQCRemBrws::EDTypes::GlobalAct
                    , strOrgVal
                    , m_cuctxToUse
                    , tCQCTreeBrws::EFSelFlags::SelectItems
                    , strNewVal
                );
                break;
            };

            case tCQCKit::ECmdPTypes::Host :
                break;

            case tCQCKit::ECmdPTypes::ImgPath :
            {
                bNewVal = facCQCTreeBrws().bSelectFile
                (
                    *this
                    , L"Select an Image"
                    , tCQCRemBrws::EDTypes::Image
                    , strOrgVal
                    , m_cuctxToUse
                    , tCQCTreeBrws::EFSelFlags::SelectItems
                    , strNewVal
                );
                break;
            }

            case tCQCKit::ECmdPTypes::IV :
                bNewVal = bSelectIV(strOrgVal, strNewVal);
                break;

            case tCQCKit::ECmdPTypes::Macro :
            {
                TString strRelPath;
                bNewVal = facCQCTreeBrws().bSelectFile
                (
                    *this
                    , L"Select a Macro"
                    , tCQCRemBrws::EDTypes::Macro
                    , strOrgVal
                    , m_cuctxToUse
                    , tCQCTreeBrws::EFSelFlags::SelectItems
                    , strRelPath
                );

                //
                //  This is going to get us back a relative path. We need a CML class path,
                //  so convert it.
                //
                facCQCRemBrws().RelPathToCMLClassPath(strRelPath, strNewVal);
                break;
            }

            case tCQCKit::ECmdPTypes::Path :
                bNewVal = bSelectPath(strOrgVal, strNewVal);
                break;

            case tCQCKit::ECmdPTypes::Point :
                break;

            case tCQCKit::ECmdPTypes::Number :
                break;

            case tCQCKit::ECmdPTypes::Signed :
                break;

            case tCQCKit::ECmdPTypes::TarSubAction :
                bNewVal = bSelectSubAction
                (
                    *pctarCur, ccfgCur.strCmdId(), eCmdCtx, strOrgVal, strNewVal
                );
                break;

            case tCQCKit::ECmdPTypes::Template :
                bNewVal = facCQCTreeBrws().bSelectFile
                (
                    *this
                    , L"Select a Template"
                    , tCQCRemBrws::EDTypes::Template
                    , strOrgVal
                    , m_cuctxToUse
                    , tCQCTreeBrws::EFSelFlags::SelectItems
                    , strNewVal
                    , nullptr
                    , &m_strBasePath
                );
                break;

            case tCQCKit::ECmdPTypes::Unsigned :
                break;

            case tCQCKit::ECmdPTypes::VarName :
                bNewVal = bSelectVariable(strOrgVal, strNewVal, kCIDLib::False);
                break;

            default :
                // Not one we care about
                break;
        };
    }

    // If a new value was entered, then take it
    if (bNewVal)
    {
        ccfgCur.piAt(c4PInd).m_strValue = strNewVal;

        TString strFmt;
        m_apwndTexts[c4PInd]->strWndText(strNewVal);
        aocCur.FormatTo(strFmt, m_c4CurNesting);
        m_pwndOpList->SetColText(c4OpIndex, 0, strFmt);
    }
}


//
//  When paths are involved, we want to double up the slashes so as to
//  avoid escapement problems. That means that in any of the selection
//  methods we have to replace doubles with singles to pass into the
//  selection dialog (else it won't recognize it), and then the other way
//  around after selection to put it back into the parameter value.
//
tCIDLib::TVoid
TCQCEditCmdDlg::SwizzlePath(TString& strIO, const tCIDLib::TBoolean bIn)
{
    tCIDLib::TCard4 c4At(0);
    if (bIn)
        strIO.bReplaceSubStr(m_strSlash2, m_strSlash1, c4At, kCIDLib::True);
    else
        strIO.bReplaceSubStr(m_strSlash1, m_strSlash2, c4At, kCIDLib::True);
}


//
//  This is called if an item in the opcode list is double clicked. We see if it is
//  an If. If so, we toggle its negation status.
//
tCIDLib::TVoid TCQCEditCmdDlg::ToggleNegateAt(const tCIDLib::TCard4 c4Index)
{
    TActOpcode& aocTar = m_pcolCurEvOps->objAt(c4Index);

    if (aocTar.m_eOpcode == tCQCKit::EActOps::If)
    {
        // Toggle the negate flag
        aocTar.m_bNegate = !aocTar.m_bNegate;

        // And update the display
        TString strFmt;
        aocTar.FormatTo(strFmt, m_c4CurNesting);
        m_pwndOpList->SetColText(c4Index, 0, strFmt);
    }
}


//
//  When a new item is selected in the opcode list, we need to update the parameter
//  info and enable/disable stuff based on the new selection. Note that the paste nd
//  paste all buttons are enabled or disabled based on what's on the clipboard, so
//  we only do it upon entry to the dialog and when something is copied or cut.
//
tCIDLib::TVoid TCQCEditCmdDlg::UpdateForSelection()
{
    const tCIDLib::TCard4 c4CurOpInd = m_pwndOpList->c4CurItem();

    // If nothing selected, just clear the parms and return
    if (c4CurOpInd == kCIDLib::c4MaxCard)
    {
        ClearParmWidgets();
        return;
    }

    // Re-calculate the nesting level for the selected item
    m_c4CurNesting = c4CalcNesting(c4CurOpInd);

    const TActOpcode& aocCur = m_pcolCurEvOps->objAt(c4CurOpInd);

    tCIDLib::TBoolean bOnACmd = kCIDLib::False;
    if ((aocCur.m_eOpcode != tCQCKit::EActOps::Else)
    &&  (aocCur.m_eOpcode != tCQCKit::EActOps::End))
    {
        // Get the command configuration data
        const TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;

        //
        //  It's either an if, cmd, or comment. If a comment, then we
        //  handle it a little specially.
        //
        //
        tCIDLib::TCard4 c4PIndex = 0;
        if (aocCur.m_eOpcode == tCQCKit::EActOps::Comment)
        {
            m_apwndPrefs[0]->strWndText(L"Comment");
            m_apwndTexts[0]->strWndText(ccfgCur.piAt(0).m_strValue);

            m_apwndPrefs[0]->SetEnable(kCIDLib::True);
            m_apwndTexts[0]->SetEnable(kCIDLib::True);
            c4PIndex = 1;
        }
         else
        {
            //
            //  Look up the command target and command so that we can get the
            //  parameter names and types and such.
            //
            TCQCCmd cmdCur;
            MCQCCmdTarIntf* pmctarCur;
            if (bFindCmd(ccfgCur, m_strCurEventId, cmdCur, pmctarCur))
            {
                //
                //  For each of the parameters defined for this command, set up the
                //  parameter widgets. Clear and disable the others.
                //
                const tCIDLib::TCard4 c4Count = ccfgCur.c4ParmCnt();
                for (; c4PIndex < c4Count; c4PIndex++)
                {
                    const TCQCCmdParm& cmdpCur = cmdCur.cmdpAt(c4PIndex);
                    m_apwndPrefs[c4PIndex]->strWndText(cmdpCur.strName());
                    m_apwndPrefs[c4PIndex]->SetEnable(kCIDLib::True);
                    m_apwndTexts[c4PIndex]->strWndText(ccfgCur.piAt(c4PIndex).m_strValue);
                    m_apwndTexts[c4PIndex]->SetEnable(kCIDLib::True);
                }
            }
             else
            {
                //
                //  This should not happen since we converted any missing
                //  targets on entry.
                //
                TErrBox msgbErr
                (
                    strWndText()
                    , L"IntError: The target for this command was not found"
                );
                msgbErr.ShowIt(*this);
            }
        }

        // Empty the unused parameter values
        for (; c4PIndex < kCQCKit::c4MaxCmdParms; c4PIndex++)
        {
            m_apwndPrefs[c4PIndex]->ClearText();
            m_apwndPrefs[c4PIndex]->SetEnable(kCIDLib::False);
            m_apwndTexts[c4PIndex]->ClearText();
            m_apwndTexts[c4PIndex]->SetEnable(kCIDLib::False);
        }

        bOnACmd = kCIDLib::True;
    }
     else
    {
        // Clean all the parameter stuff out
        ClearParmWidgets();

        bOnACmd = kCIDLib::False;
    }

    // If it's not a comment, then enable the change button
    m_pwndChangeButton->SetEnable(aocCur.m_eOpcode != tCQCKit::EActOps::Comment);

    // These are all enabled if we are on a command, else disabled
    m_pwndCopyAllButton->SetEnable(bOnACmd);
    m_pwndCopyButton->SetEnable(bOnACmd);
    m_pwndCutButton->SetEnable(bOnACmd);
    m_pwndDelAllButton->SetEnable(bOnACmd);

    //
    //  Set the move up/down depending on current position. We have a special case if
    //  moving down and the current opcode is an If. We have to find the end of the If
    //  see if that End is the last command. If so, we can't move forward.
    //
    if (aocCur.m_eOpcode == tCQCKit::EActOps::If)
    {
        tCIDLib::TCard4 c4EndOfIf = c4FindCondEnd(c4CurOpInd, kCIDLib::True);
        m_pwndDnButton->SetEnable(c4EndOfIf + 1 != m_pcolCurEvOps->c4ElemCount());
    }
     else
    {
        m_pwndDnButton->SetEnable(bOnACmd && (c4CurOpInd + 1 < m_pcolCurEvOps->c4ElemCount()));
    }
    m_pwndUpButton->SetEnable(bOnACmd && (c4CurOpInd != 0));

    // Deal with the enable/disable button
    if (aocCur.m_eOpcode == tCQCKit::EActOps::Cmd)
    {
        //
        //  Based on the enabled/disabled state of the command, enable and
        //  set the text of the disable/enable button.
        //
        m_pwndDisableButton->SetEnable(kCIDLib::True);
        if (aocCur.m_bDisabled)
        {
            m_pwndDisableButton->strWndText
            (
                facCQCIGKit().strMsg(kIGKitMsgs::midDlg_EdCmd_Enable)
            );
        }
         else
        {
            m_pwndDisableButton->strWndText
            (
                facCQCIGKit().strMsg(kIGKitMsgs::midDlg_EdCmd_Disable)
            );
        }
    }
     else
    {
        // Disable the enable/disable button
        m_pwndDisableButton->SetEnable(kCIDLib::False);
    }
}


