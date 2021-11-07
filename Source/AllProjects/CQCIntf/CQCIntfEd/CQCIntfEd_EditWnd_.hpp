//
// FILE NAME: CQCIntfEd_EditWnd.hpp
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
//  This is the header for the CQCIntfEd_EditWnd.cpp file, which implements a window that
//  provides sufficient interaction with our edit specific view derivative to make the
//  interface engine happy, and provide our editing requirements.
//
//  WE DO NOT allow any operations to partially affect a group, if that command would
//  change the group members. They have to ungroup before they delete or change the Z-order
//  of a group member. This prevents a lot of complexity we'd otherwise have to deal
//  with.
//
//  Undo is a complex issue here, as usual. Our strategy is:
//
//  1.  At the point where the user invokes an operation, we push a separator
//  2.  Each individual operation that changes something pushes an undo as is appropriate
//  3.  Our undo method will invoke Undo for all of the pushed objects down to the next
//      separator.
//  4.  If nothing ends up getting pushed, we end up with a redundant separator that will
//      just get ignored.
//  5.  The undo handler will ignore any pushed undo objects that report themselves as
//      the same as the current top of stack. This avoids pushing redundant objects, which
//      would make it seem like nothing happened when they do an undo.
//  6.  We flush the undo stack when they save
//
//  This is about the only way to maintain any reasonable hope of keeping it coherent
//  and not breaking it
//
// CAVEATS/GOTCHAS:
//
//  1)  We don't have to provide any save menu or accel stuff. The Admin Intf that we are
//      used by will do that.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TAlignListSorter;


// ---------------------------------------------------------------------------
//   CLASS: TIntfSelList
//  PREFIX: sel
//
//  We need fairly tight control over the selection list, and need to know how
//  the first items add to the selection were added and possibly other stuff. So
//  we encapsulate that here. Mostly it's just a list of design time ids, which
//  refer to the selected widgets. But we also care if the first widget selected
//  is selected manually, as opposed to a lasso style selection. This is for
//  alignment reasons.
// ---------------------------------------------------------------------------
class TIntfSelList
{
    public :
        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AppendList(const tCQCIntfEng::TDesIdList& fcolToAdd)
        {
            m_bFirstManual = kCIDLib::False;
            fcolToAdd.bForEach
            (
                [this](const tCIDLib::TCard8 c8Cur)
                {
                    if (!m_fcolIds.bElementPresent(c8Cur))
                        m_fcolIds.c4AddElement(c8Cur);
                    return kCIDLib::True;
                }
            );
        }

        tCIDLib::TBoolean bIsEmpty() const
        {
            return m_fcolIds.bIsEmpty();
        }

        tCIDLib::TBoolean bAlignToFirst() const
        {
            return m_bFirstManual;
        }

        tCIDLib::TBoolean bIsSelected(const tCIDLib::TCard8 c8Id) const
        {
            return m_fcolIds.bElementPresent(c8Id);
        }

        tCIDLib::TBoolean bSingleSel() const
        {
            return m_fcolIds.c4ElemCount() == 1;
        }

        tCIDLib::TCard4
        c4AddNew(const tCIDLib::TCard8 c8New, const tCIDLib::TBoolean bManual)
        {
            if (m_fcolIds.bIsEmpty())
                m_bFirstManual = bManual;
            if (!m_fcolIds.bElementPresent(c8New))
                m_fcolIds.c4AddElement(c8New);
            return m_fcolIds.c4ElemCount();
        }

        tCIDLib::TCard8 c8At(const tCIDLib::TCard4 c4At) const
        {
            return m_fcolIds[c4At];
        }

        tCIDLib::TCard4 c4Count() const
        {
            return m_fcolIds.c4ElemCount();
        }

        const tCQCIntfEng::TDesIdList& fcolIds() const
        {
            return m_fcolIds;
        }

            tCQCIntfEng::TDesIdList& fcolIds()
        {
            return m_fcolIds;
        }

        tCIDLib::TVoid Clear()
        {
            m_bFirstManual = kCIDLib::False;
            m_fcolIds.RemoveAll();
        }

        tCIDLib::TVoid Remove(const tCIDLib::TCard8 c8Id)
        {
            m_fcolIds.bDeleteIfPresent(c8Id);
        }

        tCIDLib::TVoid
        SetList(const   tCQCIntfEng::TDesIdList&    fcolToSet
                , const tCIDLib::TBoolean           bFirstManual)
        {
            m_bFirstManual = bFirstManual;
            m_fcolIds = fcolToSet;
        }

        tCIDLib::TVoid SetPubTopic(const TString& strTopic)
        {
            m_fcolIds.EnablePubSub(strTopic);
        }

    private :
        // -------------------------------------------------------------------
        //  Private members
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bFirstManual = kCIDLib::False;
        tCQCIntfEng::TDesIdList m_fcolIds;
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfEdWindow
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIntfEditWnd : public TWindow
                     , public MCQCIntfAppHandler
                     , public MIPEIntf
                     , public MPubSubscription
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfEditWnd();

        TIntfEditWnd(const TIntfEditWnd&) = delete;
        TIntfEditWnd(TIntfEditWnd&&) = delete;

        ~TIntfEditWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfEditWnd& operator=(const TIntfEditWnd&) = delete;
        TIntfEditWnd& operator=( TIntfEditWnd&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AsyncCallback
        (
            const   tCQCIntfEng::EAsyncCmds eCmd
            ,       TCQCIntfView&           civSender
        )   final;

        tCIDLib::TVoid AsyncDataCallback
        (
                    TCQCIntfADCB* const     padcbInfo
        )   final;

        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const final;

        tCIDLib::TBoolean bProcessAccel
        (
            const   tCIDLib::TVoid* const   pMsgData
        )   const final;

        tCIDLib::TVoid DismissBlanker
        (
                    TCQCIntfView&           civSender
        )   final;

        tCIDLib::TVoid DoSpecialAction
        (
            const   tCQCIntfEng::ESpecActs  eAct
            , const tCIDLib::TBoolean       bAsync
            ,       TCQCIntfView&           civSender
        )   final;

        tCQCIntfEng::EAppFlags eAppFlags() const final;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   final;

        tCIDLib::TVoid NewTemplate
        (
            const   TCQCIntfTemplate&       iwdgNew
            ,       TCQCIntfView&           civSender
            , const tCIDLib::TCard4         c4StackPos
        )   final;

        MCQCCmdTracer* pmcmdtTrace() final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChanges() const;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        );

        tCIDLib::TVoid CreateEditWnd
        (
                    TScrollArea&            wndParent
            ,       TCQCIntfEdView* const   pcivEdit
            ,       TIntfAttrEdWnd* const   pwndAttrEd
            ,       TIntfErrListWnd* const  pwndErrList
            ,       TIntfWidgetPalTab* const pwndWdgPal
            , const tCIDCtrls::TWndId       widToUse
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        const TString& strEdUpdateTopic() const
        {
            return m_strUpdateTopic;
        }

        const TString& strWdgPalTopic() const
        {
            return m_strWdgPalTopic;
        }

        tCIDLib::TVoid SelectByDesId
        (
            const   tCIDLib::TCard8         c8DesId
        );


    protected :
        // -------------------------------------------------------------------
        //  Declare our friends
        // -------------------------------------------------------------------
        friend class TAlignListSorter;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TBoolean bClick
        (
            const   tCIDCtrls::EMouseButts  eButton
            , const tCIDCtrls::EMouseClicks eClickType
            , const TPoint&                 pntAt
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
        )   final;

        tCIDLib::TBoolean bEraseBgn
        (
                TGraphDrawDev&              gdevToUse
        )   final;

        tCIDLib::TBoolean bExtKey
        (
            const   tCIDCtrls::EExtKeys     eKey
            , const tCIDLib::TBoolean       bAlt
            , const tCIDLib::TBoolean       bCtrl
            , const tCIDLib::TBoolean       bShift
            , const TWindow* const          pwndChild = nullptr
        )   final;

        tCIDLib::TBoolean bMouseMove
        (
            const   TPoint&                 pntAt
            , const tCIDLib::TBoolean       bControlDown
            , const tCIDLib::TBoolean       bShiftDown
        )   final;

        tCIDLib::TBoolean bPaint
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaUpdate
        )   final;

        tCIDLib::TBoolean bShowHelp
        (
            const   tCIDCtrls::TWndId       widCtrl
            , const TPoint&                 pntMouse
        )   final;

        tCIDCtrls::ECtxMenuOpts eShowContextMenu
        (
            const   TPoint&                 pntAt
            , const tCIDCtrls::TWndId       widSrc
        )   final;

        tCIDLib::TVoid GettingFocus() final;

        tCIDLib::TVoid LosingFocus() final;

        tCIDLib::TVoid MenuCommand
        (
            const   tCIDLib::TResId         ridItem
            , const tCIDLib::TBoolean       bChecked
            , const tCIDLib::TBoolean       bEnabled
        )   final;

        tCIDLib::TVoid MouseExit() final;

        tCIDLib::TVoid ProcessPubMsg
        (
            const   TPubSubMsg&             psmsgIn
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        //
        //  EDragModes
        //      We have to handle a few different dragging scenarios:
        //
        //      1. Size/marquee select. Both of these just need to drag out a
        //         rectangle from the start point down/right. For a size, we
        //         can only have one widget selected.
        //      2. Widget move. We drag around an irrelegular shape that out
        //         lines all of the selected widgets.
        // -------------------------------------------------------------------
        enum class EDragModes
        {
            None
            , Select
            , Size
            , Move
        };

        // ---------------------------------------------------------------------------
        //   CLASS: TAlignWrapper
        //  PREFIX: aw
        //
        //  A simple helper class used by the alignment tool handler. It greatly
        //  simplifies dealing with fully selected groups, which we treat as a unit for
        //  alignment purposes.
        //
        //  It just contains non-adopting pointers to the widgets. If more than one,
        //  then it's a group.
        // ---------------------------------------------------------------------------
        class TAlignWrapper
        {
            public :
                // -------------------------------------------------------------------
                //  Constructors and Destructor
                // -------------------------------------------------------------------
                TAlignWrapper();

                TAlignWrapper(const TAlignWrapper&) = delete;
                TAlignWrapper(TAlignWrapper&&) = default;

                ~TAlignWrapper() = default;


                // -------------------------------------------------------------------
                //  Public operators=
                // -------------------------------------------------------------------
                TAlignWrapper& operator=(const TAlignWrapper&) = delete;
                TAlignWrapper& operator=(TAlignWrapper&&) = default;


                // -------------------------------------------------------------------
                //  Public, non-virtual methods
                // -------------------------------------------------------------------
                tCIDLib::TVoid Complete();

                tCIDLib::TVoid SetOrg
                (
                    const   TPoint&         pntSrc
                    ,       TIntfEditWnd&   iwdgOwner
                );


                // -------------------------------------------------------------------
                //  Public data members
                //
                //  m_areaFull
                //      The full area covered, either the area of the single widget or the
                //      hull area of the group members.
                //
                //  m_colList
                //      A non-adopting list of the available widgets.
                // -------------------------------------------------------------------
                TArea                   m_areaFull;
                tCQCIntfEng::TChildList m_colList;
        };
        using TAlignList = TRefVector<TAlignWrapper>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        TArea areaSelection();

        tCIDLib::TVoid AddGrpToSelection
        (
            const   tCIDLib::TCard4         c4GroupId
            , const tCIDLib::TBoolean       bClearFirst
        );

        tCIDLib::TVoid AddToSelection
        (
                    TCQCIntfWidget* const   piwdgToAdd
            , const tCIDLib::TBoolean       bClearFirst
            , const tCIDLib::TBoolean       bManualSel
        );

        tCIDLib::TVoid AdjustWidgetAreas
        (
            const   TArea&                  areaOld
            , const TArea&                  areaNew
        );

        tCIDLib::TVoid ApplyAlignTool
        (
            const   TString&                strType
        );

        tCIDLib::TVoid ApplyFontTool
        (
            const   TString&                strToDo
        );

        tCIDLib::TBoolean bAnyGroupedSel() const;

        tCIDLib::TBoolean bAnyUngroupedSel() const;

        tCIDLib::TBoolean bBuildAlignList
        (
                    TAlignList&             colToFill
        );

        tCIDLib::TBoolean bBuildSelHandleRgn
        (
                    TGUIRegion&             grgnToFill
        );

        tCIDLib::TBoolean bGrpFullySelected
        (
            const   tCIDLib::TCard4         c4GroupId
            ,       tCQCIntfEng::TDesIdList& fcolGrpIds
        )   const;

        tCIDLib::TBoolean bIsSelected
        (
            const   tCIDLib::TCard8         c8DesId
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TBoolean bIsSelected
        (
            const   tCIDLib::TCard8         c8DesId
        );

        tCIDLib::TBoolean bPartialGrpSel() const;

        tCIDLib::TBoolean bSingleSelected
        (
            const   tCIDLib::TBoolean       bFullGrpReq = kCIDLib::True
        )   const;

        tCIDLib::TBoolean bSnapArea
        (
                    TArea&                  areaToAdjust
            , const TArea&                  areaLimit
            , const tCIDLib::TBoolean       bForce
        );

        tCIDLib::TBoolean bSnapPoint
        (
                    TPoint&                 pntToAdjust
            , const tCIDLib::TBoolean       bForce
        );

        tCIDLib::TVoid CancelDrag
        (
            const   tCIDLib::TBoolean       bRedraw
        );

        tCIDLib::TVoid ChangeZOrder
        (
            const   tCIDLib::TCard4         c4Cmd
        );

        tCIDLib::TVoid CheckAttrChange
        (
                    TCQCIntfWidget&         iwdgTar
            , const TAttrData&              adatOld
            , const TAttrData&              adatNew
        );

        tCIDLib::TVoid CopyCutSelected
        (
            const   tCIDLib::TBoolean       bCut
        );

        tCIDLib::TVoid CreateNewWidget
        (
            const   tCIDLib::TCard4         c4CmdId
            , const TPoint&                 pntAt
        );

        tCIDLib::TVoid DeleteGrpOpts
        (
                    TCQCIntfWidget* const   piwdgHit
        );

        tCIDLib::TVoid DeleteSelected
        (
            const   tCIDLib::TBoolean       bConfirm
        );

        tCIDLib::TVoid Deselect
        (
            const   TCQCIntfWidget* const   piwdgTar
        );

        tCIDLib::TVoid DeselectGrp
        (
            const   tCIDLib::TCard4         c4GroupId
        );

        tCIDLib::TVoid DoGrouping();

        tCIDLib::TVoid DoSearchAndReplace();

        tCIDLib::TVoid DoUngrouping();

        tCIDLib::TVoid DrawAllSelHandles();

        tCIDLib::TVoid DrawSelHandles
        (
                    TGraphDrawDev&          gdevToUse
            , const TCQCIntfWidget&         iwdgHit
        );

        tCIDLib::TVoid DrawSelHandlesIfSelected
        (
                    TGraphDrawDev&          gdevToUse
            , const tCIDLib::TCard8         c8DesId
        );

        tCIDLib::TVoid DrawUnder
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaUnder
        );

        tCIDLib::TVoid DrillDown
        (
            const   TPoint&                 pntAt
            ,       TCQCIntfWidget&         iwdgHit
        );

        tCIDCtrls::EEvResponses eAttrEditHandler
        (
                    TAttrEditInfo&          wnotInfo
        );

        tCIDCtrls::ESysPtrs ePtrForArea
        (
            const   tCIDCtrls::EWndAreas    eArea
        )   const;

        const TCQCIntfTemplate& iwdgEdit() const;

        TCQCIntfTemplate& iwdgEdit();

        tCIDLib::TVoid LockSelected();

        TCQCIntfWidget* piwdgHitNext
        (
            const   TPoint&                 pntTest
            ,       tCIDLib::TCard8&        c8DesId
        );

        TCQCIntfWidget* piwdgHitTest
        (
            const   TPoint&                 pntTest
            ,       tCIDLib::TCard8&        c8DesId
        );

        tCIDLib::TVoid PasteAttrs
        (
            const   tCIDMData::TAttrList&   colToApply
        );

        tCIDLib::TVoid PasteWidgets
        (
            const   TPoint&                 pntWndRelative
        );

        tCIDLib::TVoid PublishMouseOverMsg
        (
            const   TCQCIntfWidget* const   piwdgUnder
        );

        tCIDLib::TVoid PublishMouseOverMsg
        (
            const   TPoint&                 pntAt
        );

        tCIDLib::TVoid PublishMouseOverMsg();

        tCIDLib::TVoid RefreshImages();

        tCIDLib::TVoid RemoveSelection
        (
            const   tCIDLib::TBoolean       bClearAttrEd = kCIDLib::True
        );

        tCIDLib::TVoid ResizeSelected
        (
            const   TSize&                  szNew
        );

        tCIDLib::TVoid Save();

        tCIDLib::TVoid SelectContained
        (
            const   TArea&                  areaLasso
            , const tCIDLib::TBoolean       bClearFirst
        );

        tCIDLib::TVoid SetWidgetSizes();

        tCIDLib::TVoid SizeSelectedToContents();

        tCIDLib::TVoid StartMove
        (
            const   TPoint&                 pntStart
        );

        tCIDLib::TVoid StartSize
        (
            const   TPoint&                 pntStart
            , const TCQCIntfWidget&         iwdgTarget
            , const tCIDCtrls::EWndAreas    eArea
        );

        tCIDLib::TVoid StartMarqueeSel
        (
            const   TPoint&                 pntStart
        );

        tCIDLib::TVoid ToggleTextJustification
        (
            const   tCIDLib::TBoolean       bHorz
        );

        tCIDLib::TVoid Undo();

        tCIDLib::TVoid UnlockAll();

        tCIDLib::TVoid UpdateAttrEd();

        tCIDLib::TVoid Validate();

        tCIDLib::TVoid WdgPalSelUpdate
        (
            const   tCQCIntfEng::TDesIdList fcolNew
        );


        // -------------------------------------------------------------------
        //  Private static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eAlignComp
        (
            const   TIntfEditWnd::TAlignWrapper&    aw1
            , const TIntfEditWnd::TAlignWrapper&    aw2
            , const tCIDLib::TCard4                 c4UserData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_accelEdit
        //      Our accelerator table, to map hot keys to our menu commands.
        //
        //  m_areaDrag
        //      If the drag mode is not a move, then this is the ongoing area
        //      that we adjust to represent the area between the start point
        //      and the current mouse position.
        //
        //  m_bmpChecker
        //      We fill our background with a checkerboard pattern to help
        //      with dealing with transparent images.
        //
        //  m_bmpDrag
        //      See m_eDragMode member. This is used to greatly reduce overhead
        //      during move, size and select type tracking.
        //
        //  m_c4NextWId
        //      The next design time widget id to assign. We use these to assign
        //      unique ids to widgets for identification purposes during design
        //      time, so that we can avoid using pointers where that would be
        //      dangerous or unworkable. Each editor window is given a new id
        //      when created, and that is set as the template level part of
        //      the id on the template itself. The widgets will be a combination
        //      of the template id and the per widget id.
        //
        //  m_c8LastMouseOver
        //      We have to publish mouse enter/exit msgs has the mouse moves into and
        //      out of widgets. We don't want to do this on every move which would be
        //      hugely inefficient. So we have to keep up with the last widget it was
        //      over. It's initialized to zero which won't be valid, so any initial
        //      check will see a change and send a first msg.
        //
        //      We call private helpers to post these messages and those guys keep this
        //      updated, though we do have to be careful on cuts, pastes, undos, and
        //      keyboard driven moves and sizes to force an update.
        //
        //  m_eDragArea
        //  m_eDragMode
        //      Indicates the drag mode we are in. This is used during mouse
        //      move, paint, and button release to know if we are currently
        //      doing some sort of drag. We also store the window area that was
        //      originally hit, so that we know how to adjust the drag area when
        //      the mouse moves.
        //
        //  m_gpenDrag
        //      A pen that we use for drag operations.
        //
        //  m_iwdgOrg
        //      The original (or last saved) version of the widget, so that we can
        //      check for changes upon request.
        //
        //  m_menuEdit
        //      Our popup menu for applying various operations on selected
        //      widgets.
        //
        //  m_pcivEdit
        //      A pointer to our editor specific view object.
        //
        //  m_pntDragOfs
        //      For a move type drag, we have to remember how far the mouse was
        //      from the origin of the area that contains all of the selected
        //      widgets. This let's us know how to adjust the origin relative to
        //      the mouse moving. For a marquee select, it's the original point,
        //      which is the pivot point where we have to invert our corners.
        //
        //  m_psmsgTmp
        //      To avoid constantly creating and destroying one of these, we just keep
        //      one around to use during the Timer() method.
        //
        //  m_pssubPalChanges
        //      A subscription for the palette window's change msgs, so that we can
        //      respond to user input driven changes from that window.
        //
        //  m_pssubSelChanges
        //      We set up the selection id list (m_fcolSelected) to publish changes
        //      and we subscribe to that. This let's us know any time the selection
        //      list changes so we don't have to have code all over the place dealing
        //      with that. We turn around and publish one of our own msgs.
        //
        //  m_pssubWdgListChanges
        //      We set up the template itself to have its widget collection publish change
        //      info. We subscribe to those changes and re-publish that info in one of our
        //      own msgs.
        //
        //  m_pstopUpdates
        //      We create this topic to publish information that others may need to know
        //      about our template content. We re-publish selection and widget list and
        //      widget change msgs that we get from the above subscriptions, so that we
        //      can present a single, unified update interface.
        //
        //  m_pwndAttrEd
        //      The main window gives us a pointer to the attribute editor so that we
        //      can load attributes into it and clear it out as required. It's actually
        //      a derivative of the base attribute editor window so that it can deal
        //      with visual editing of some of our specialized types.
        //
        //  m_pwndErrList
        //      We get a pointer to the error list window so that we can send it error
        //      info for it to display. We also give him some info for him to post us
        //      back error selection/invocation operations.
        //
        //  m_pwndWdgPal
        //      A pointer to the widet palette that serves our editor window
        //
        //  m_selList
        //      A list of design time ids of the widgets (including possibly the template
        //      itself) that are currently selected. This allows us to display the selection
        //      handles and to do operations on the selected widgets.
        //
        //      Any methods that affect this list have to deal with the fact that if a
        //      single widget is selected, it must be loaded into the attribute editor,
        //      and if no selection the attribute editor must  be emptied.
        //
        //  m_strSelectionTopic
        //      The topic path we set up m_fcolSelected to publish changes on, so that we
        //      can deal with selection changes and pass them on via our update topic.
        //
        //  m_strEdUpdateTopic
        //      The topic id that we create to publish changes. We set this up in the ctor
        //      so it's available immediately, because it may be accessed before we are
        //      created and create teh actual topic.
        //
        //  m_strWdgPalTopic
        //      We create the unique id for the Widget palette window to publish changes
        //      to. Since he exists up above us, we create the id so that we can see it
        //      and he can see it as well (indirectly via our main window.)
        //
        //  m_uhEditor
        //      This is the undo handler for the editor window. This is the standard
        //      undo framework object. We use it to store and get back out undo objects
        //      that we derive from its base undo cmd class.
        // -------------------------------------------------------------------
        TAccelTable             m_accelEdit;
        TArea                   m_areaDrag;
        TBitmap                 m_bmpDrag;
        TBitmap                 m_bmpChecker;
        tCIDLib::TCard4         m_c4NextWId;
        tCIDLib::TCard8         m_c8LastMouseOver;
        tCIDCtrls::EWndAreas    m_eDragArea;
        EDragModes              m_eDragMode;
        TCosmeticPen            m_gpenDrag;
        TCQCIntfTemplate        m_iwdgOrg;
        TPopupMenu              m_menuEdit;
        TCQCIntfEdView*         m_pcivEdit;
        TPoint                  m_pntDragOfs;
        TPubSubTopic            m_pstopUpdates;
        TIntfAttrEdWnd*         m_pwndAttrEd;
        TIntfErrListWnd*        m_pwndErrList;
        TIntfWidgetPalTab*      m_pwndWdgPal;
        TIntfSelList                m_selList;
        TString                 m_strSelectionTopic;
        TString                 m_strUpdateTopic;
        TString                 m_strWdgPalTopic;
        TUndoHandler            m_uhEditor;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfEditWnd,TWindow)
};

#pragma CIDLIB_POPPACK

