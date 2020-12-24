//
// FILE NAME: CQCIGKit_EditCmdDlg_.hpp
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
//  This is the header for the CQCGKit_EditCmdDlg.cpp file, which implements the
//  TCQCEditCmdDlg class. This is a fairly complex dialog that allows for editing
//  of CQC actions.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCEditCmdDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCEditCmdDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  We are constantly having to look up commands from the targets to
        //  check parameters and such. In a fairly long action, it would be
        //  hugely inefficient to constantly re-query the commands of a target.
        //  So we fault them into a cache as they are accessed. Our bFindCmd
        //  method handles this, getting them from the cache if there else
        //  looking them up and adding them to the cache.
        //
        //  When we change events the cache has to be flushed since the list
        //  of commands can be different for each event.
        // -------------------------------------------------------------------
        typedef TVector<TCQCCmd>                            TCmdList;
        typedef THashMap<TCmdList,TString,TStringKeyOps>    TCmdCache;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCEditCmdDlg();

        TCQCEditCmdDlg(const TCQCEditCmdDlg&) = delete;

        ~TCQCEditCmdDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCEditCmdDlg& operator=(const TCQCEditCmdDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       MCQCCmdSrcIntf&         mcsrcEdit
            , const tCIDLib::TBoolean       bRequired
            , const TCQCUserCtx&            cuctxToUse
            ,       tCQCKit::TCmdTarList* const pcolExtraTars
        );

        tCIDLib::TVoid SetBasePath
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCancelRequest() override;

        tCIDLib::TBoolean bChar
        (
            const   tCIDLib::TCh            chChar
            , const tCIDLib::TBoolean       bAlt
            , const tCIDLib::TBoolean       bCtrl
            , const tCIDLib::TBoolean       bShift
            , const TWindow* const          pwndChild = nullptr
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bShowHelp
        (
            const   tCIDCtrls::TWndId       widCtrl
            , const TPoint&                 pntMouse
        )   override;

        tCIDLib::TVoid Destroyed() override;

        tCIDCtrls::ECtxMenuOpts eShowContextMenu
        (
            const   TPoint&                 pntAt
            , const tCIDCtrls::TWndId       widSrc
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef MCQCCmdSrcIntf::TOpcodeBlock    TOpBlock;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChangeTarget
        (
                    TOpBlock&               colOps
            , const tCIDLib::TCard4         c4CurIndex
            , const TPoint&                 pntAt
        );

        tCIDLib::TBoolean bCheckExit() const;

        tCIDLib::TBoolean bCheckParms
        (
                    TActOpcode&             aocCheck
            , const TString&                strEventId
            ,       tCIDLib::TCard4&        c4BadParmInd
        );

        tCIDLib::TBoolean bFindCmd
        (
            const   TCQCCmdCfg&             ccfgSrc
            , const TString&                strEventId
            ,       TCQCCmd&                cmdToFill
            ,       MCQCCmdTarIntf*&        pmctarCur
        );

        tCIDLib::TBoolean bHasTokens
        (
                    MCQCCmdSrcIntf&         mcsrcCmd
            ,       TString&                strValue
        );

        tCIDLib::TBoolean bQueryTmplInfo
        (
            const   TString&                strTmplPath
            ,       TString&                strNotes
            ,       tCQCKit::EUserRoles&    eMinRole
        );

        tCIDLib::TBoolean bSelectApp
        (
            const   TString&                strOrgVal
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSelectBool
        (
                    TString&                strToFill
            , const tCIDLib::TCard4         c4ParmInd
        );

        tCIDLib::TBoolean bSelectColor
        (
                    TString&                strToFill
        );

        tCIDLib::TBoolean bSelectEMailAcct
        (
            const   TString&                strOrgVal
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSelectEnum
        (
            const   tCIDLib::TStrList&      colValues
            ,       tCIDLib::TCard4&        c4SelIndex
            , const TPoint&                 pntAt
        );

        tCIDLib::TBoolean bSelectFile
        (
            const   TCQCCmdCfg&             ccfgSrc
            , const TString&                strEventId
            , const tCIDLib::TCard4         c4ParmInd
            , const TString&                strOrgVal
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSelectFldEnum
        (
            const   TCQCFldEnumLimit&       fldlSrc
            , const tCIDLib::TCard4         c4ParmInd
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSelectIV
        (
            const   TString&                strOrgVal
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSelectPath
        (
            const   TString&                strOrgVal
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSelectParmEnum
        (
            const   TCQCCmdCfg&             ccfgSrc
            , const TString&                strEventId
            , const tCIDLib::TCard4         c4ParmInd
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSelectSubAction
        (
            const   MCQCCmdTarIntf&         mctarCur
            , const TString&                strCmdId
            , const tCQCKit::EActCmdCtx     eCmdCtx
            , const TString&                strOrgVal
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSelectTrayMonBinding
        (
            const   TString&                strOrgVal
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSelectVariable
        (
            const   TString&                strOrgVal
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bNameOnly
        );

        tCIDLib::TBoolean bValidate();

        tCIDLib::TCard4 c4FindCondBegin
        (
            const   tCIDLib::TCard4         c4End
            , const tCIDLib::TBoolean       bUltimate
        )   const;

        tCIDLib::TCard4 c4FindCondEnd
        (
            const   tCIDLib::TCard4         c4Start
            , const tCIDLib::TBoolean       bUltimate
        )   const;

        tCIDLib::TCard4 c4FindCondEnd
        (
            const   TOpBlock&               colOps
            , const tCIDLib::TCard4         c4Start
            , const tCIDLib::TBoolean       bUltimate
        )   const;

        tCIDLib::TBoolean bGetCommand
        (
            const   tCQCKit::ECmdTypes      eType
            , const TPoint&                 pntAt
            ,       TCQCCmd&                cmdToFill
            , const MCQCCmdTarIntf*&        pmctarToFill
        );

        tCIDLib::TCard4 c4CalcNesting
        (
            const   tCIDLib::TCard4         c4ForIndex
        )   const;

        tCIDLib::TCard4 c4CalcNesting
        (
            const   tCQCKit::EActOps        ePrevOp
            , const tCIDLib::TCard4         c4PrevIndent
            , const TActOpcode&             aocNew
        )   const;

        tCIDLib::TVoid ChangeCmd
        (
            const   TPoint&                 pntAt
        );

        tCIDLib::TVoid ChangeConditional
        (
                    TOpBlock&               colOps
            , const tCIDLib::TCard4         c4CurIndex
            , const TPoint&                 pntAt
        );

        tCIDLib::TVoid ClearParmWidgets();

        tCIDLib::TVoid ClearOpList();

        tCIDLib::TVoid CopyCut
        (
            const   tCIDLib::TBoolean       bCopy
            , const tCIDLib::TCard4         c4StartInd
            , const tCIDLib::TCard4         c4EndInd
        );

        tCIDLib::TVoid DeleteCondOp
        (
            const   tCIDLib::TCard4         c4Start
            , const tCIDLib::TBoolean       bDiscard
        );

        tCIDLib::TVoid DeleteCurOp();

        tCIDLib::TVoid DisableCmds();

        tCIDLib::TVoid DoReplace();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eEFHandler
        (
                    TEFChangeInfo&          wnotEvent
        );

        tCQCKit::EReqAccess eFindReqFldAccess
        (
            const   TCQCCmdCfg&             ccfgCur
            , const TString&                strEventId
            , const tCIDLib::TCard4         c4PInd
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid InsertCmd
        (
            const   TCQCCmd&                cmdNew
            , const MCQCCmdTarIntf&         mctarNew
            , const tCIDLib::TBoolean       bInsertAfter
        );

        tCIDLib::TVoid InsertCmd
        (
            const   TActOpcode&             aocNew
            , const tCIDLib::TBoolean       bInsertAfter
        );

        tCIDLib::TVoid InsertCond
        (
            const   tCIDLib::TBoolean       bHasElse
            , const TCQCCmd&                cmdNew
            , const MCQCCmdTarIntf&         mctarNew
            , const tCIDLib::TBoolean       bInsertAfter
        );

        tCIDLib::TVoid InsertDriverMon
        (
            const   tCIDLib::TCard4         c4ParmInd
        );

        tCIDLib::TVoid InsertField
        (
            const   tCIDLib::TCard4         c4ParmInd
        );

        tCIDLib::TVoid LoadEventOps
        (
            const   tCIDLib::TCard4         c4LBIndex
        );

        tCIDLib::TVoid MoveUpDown
        (
            const   tCIDLib::TBoolean       bUp
        );

        tCIDLib::TVoid OpCodesToClipboard
        (
            const   tCIDLib::TBoolean       bCopy
            , const tCIDLib::TCard4         c4StartInd
            , const tCIDLib::TCard4         c4EndInd
        );

        MCQCCmdTarIntf* pctarFindById
        (
            const   TString&                strIdToFind
        );

        tCIDLib::TVoid PasteOpcodes
        (
            const   tCIDLib::TBoolean       bPasteAll
        );

        tCIDLib::TVoid Reindent
        (
            const   tCIDLib::TCard4         c4BeginInd
            , const tCIDLib::TCard4         c4EndInd
            ,       TString&                strFmt
        );

        tCIDLib::TVoid ReindentAll
        (
                    TString&                strFmt
        );

        tCIDLib::TVoid SelParmVal
        (
            const   tCIDCtrls::TWndId       widSrc
        );

        tCIDLib::TVoid SwizzlePath
        (
                    TString&                strIO
            , const tCIDLib::TBoolean       bIn
        );

        tCIDLib::TVoid ToggleNegateAt
        (
            const   tCIDLib::TCard4         c4Index
        );

        tCIDLib::TVoid UpdateForSelection();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4CurEvIndex
        //      To make things go faster, we keep the index (in the original list,
        //      not the combo box which is sorted) of the currently selected event.
        //      This way all of the code that constantly accesses this stuff doesn't
        //      have to look it up constantly.
        //
        //  m_colStates
        //      To keep things simpler we maintain some window states to enable
        //      and disable controls appropriately.
        //
        //  m_csrcEdit
        //      We make a copy of the caller's command source content for editing
        //      purposes. We use a class provided by CQCKit for this sort of thing,
        //      which is just a pure wrapper around the command source mixin.
        //
        //      ONLY USE this for action command editing. Don't call any methods on it
        //      otherwise. Use the original for that, m_pmcsrcOrg, since those methods
        //      can be overridden in the actual object that is providing the actions
        //      for editing. So things like creating the runtime value object need to
        //      be done by the original object.
        //
        //  m_gfontLB
        //      We set up the list box to use a fixed font so that everything lines
        //      up better.
        //
        //  m_pcolCurEvOps
        //      We keep this pointed at the currently selected opcode block (the ones
        //      for the currently selected event.) We update this in the combo box
        //      event handler.
        //
        //  m_pmcsrcOrg
        //      A pointer to the original command source, so that we can both get
        //      any changes back to the caller and for checking to see if anything
        //      has changed.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just
        //      want to avoid lots of downcasting when we use them.
        //
        //  m_strBasePath
        //      This is a little bit kludgey, but now that we support relative
        //      template paths it leaves us with the problem where, if we want to
        //      get info about a template referenced in a command, or just verify
        //      it exists, we have to be able to expand it. That means we need the
        //      path of the template the action is embedded in.
        //
        //      So, the intf editor will set this upon invoking the editing of an
        //      action. Else it will be blank and shouldn't come into use since
        //      only within a template action can relative paths be used. If we end
        //      up supporting relative paths elsewhere, then this will have to be
        //      extended to deal with that.
        //
        //  m_strCurEventId
        //      The name of the last selected (current) event, for any error msgs and
        //      such. We update it upon getting a selection event from the combo box.
        //
        //  m_strSlash1
        //  m_strSlash2
        //      When dealing with local paths, we automatically swizzle the path so that
        //      it has double slashes, to avoid escapement issues. So we keep these around
        //      to make that more efficient. For our own server based resources, this isn't
        //      required since we use forward slashes.
        // -------------------------------------------------------------------
        TPushButton*            m_apwndPrefs[kCQCKit::c4MaxCmdParms];
        TEntryField*            m_apwndTexts[kCQCKit::c4MaxCmdParms];
        tCIDLib::TBoolean       m_bRequired;
        tCIDLib::TCard4         m_c4CurEvIndex;
        tCIDLib::TCard4         m_c4CurNesting;
        TCmdCache               m_colCmdCache;
        tCQCKit::TCmdTarList    m_colTargets;
        TBasicCmdSrc            m_csrcEdit;
        TCQCUserCtx             m_cuctxToUse;
        TGUIFont                m_gfontLB;
        TOpBlock*               m_pcolCurEvOps;
        MCQCCmdSrcIntf*         m_pmcsrcOrg;
        TPushButton*            m_pwndCancelButton;
        TPushButton*            m_pwndChangeButton;
        TPushButton*            m_pwndCmdButton;
        TPushButton*            m_pwndCommentButton;
        TPushButton*            m_pwndCopyAllButton;
        TPushButton*            m_pwndCopyButton;
        TPushButton*            m_pwndCutButton;
        TPushButton*            m_pwndDelAllButton;
        TPushButton*            m_pwndDisableButton;
        TPushButton*            m_pwndDnButton;
        TComboBox*              m_pwndEvents;
        TPushButton*            m_pwndIfButton;
        TPushButton*            m_pwndIfElseButton;
        TMultiColListBox*       m_pwndOpList;
        TPushButton*            m_pwndPasteButton;
        TPushButton*            m_pwndPasteAllButton;
        TPushButton*            m_pwndReplaceButton;
        TPushButton*            m_pwndResetButton;
        TPushButton*            m_pwndSaveButton;
        TPushButton*            m_pwndUpButton;
        TString                 m_strBasePath;
        TString                 m_strCurEventId;
        TString                 m_strSlash1;
        TString                 m_strSlash2;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCEditCmdDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


