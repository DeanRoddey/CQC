//
// FILE NAME: CQCIntfEng_View.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/15/2002
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
//  This is the header for the CQCIntfEng_View.cpp file, which defines an
//  abstract mechanism via which templates get presented to the outside world.
//  It's not always a real window, since we support remoted user interfaces. So we
//  define a view class from which various actual implemenations can be derived.
//  There's also the issue that the designer has to have it's own editor type window
//  while regular viewers need a different type of window that just does display.
//
//  So our whole IV engine works in terms of this abstraction to get drawing
//  devices to draw on, to manage the stack of layered popups, to do things like
//  report errors, get user touch and keyboard input, to post actions to be
//  processed, and to get periodic calls that allow it to keep the widgets updated
//  and so forth.
//
//  To manage the stack of templates we define a template manager class, whch is
//  independent of the viewing mechanism. It's only used by real viewing apps. The
//  designer doesn't use it since each template is dealt with separately.
//
//  So we also create a derivative of the view class that is used by viewer type
//  apps (IV and RIVA server), and provides a lot of code that will be common always
//  to any viewer, whether window based or not.
//
//  We also define a mixin that applications must implement if they want to support
//  certain operations that require their cooperation. And it also allows us to report
//  some things to the application.
//
//  One of the things the app handler is used for is to get the app to call us back
//  asynchronously to do some things that we can't do inline. So we post send it to
//  the application which will queue it up and then call us back on the GUI thread
//  asynchronously. For generic async callbacks we define a simple ADCB (async data
//  call back) class that can be derived from and used to queue up arbitrary data
//  to be passed back asynchronously to be processed. It includes a callback id
//  string to indicate what it is, so that it can be cast back to its actual type.
//  The app will call our AsyncDataCallback method, which will handle the operation
//  or pass it to the appropriate widget.
//
//  Though it's defined in the cpp file, we create a derivative used to allow widgets
//  to load a new template into something besides their direct parent container. This
//  must be done async so that the action has completed and its safe to destroy tye
//  widgets.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfTmplMgr;
class TCQCIntfView;


// ---------------------------------------------------------------------------
//    CLASS: TAsyncDCBBase
//   PREFIX: adcb
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfADCB
{
    public :
        TCQCIntfADCB() = delete;

        TCQCIntfADCB
        (
                    TCQCIntfView* const     pcivCaller
            , const TString&                strId
        );

        TCQCIntfADCB(const TCQCIntfADCB&) = delete;

        virtual ~TCQCIntfADCB();

        TCQCIntfADCB& operator=(const TCQCIntfADCB&) = delete;

        TCQCIntfView*   m_pcivCaller;
        TString         m_strCBId;
};


// ---------------------------------------------------------------------------
//    CLASS: TAncOverLoad
//   PREFIX: adcb
//
//  For sending async ancestor container loading operations
// ---------------------------------------------------------------------------
class TAncOverLoad : public TCQCIntfADCB
{
    public :
        TAncOverLoad
        (
                    TCQCIntfView* const     pcivCB
            , const TCQCIntfContainer&      iwdgStart
            , const TString&                strAncestor
            , const TString&                strTmplToLoad
        );

        ~TAncOverLoad();

        TString     m_strAncestor;
        TString     m_strTarPath;
        TString     m_strTmplToLoad;
};


// ---------------------------------------------------------------------------
//    CLASS: TScrollOverADCB
//   PREFIX: adcb
//
//  For sending async commands that allow a widget to cause an ancestor container to
//  scroll. These do the standard slide type thing, and we only support scrolling
//  left or right (which will be in terms of the width of the overlay) or to
//  home or end.
// ---------------------------------------------------------------------------
class TScrollOverADCB : public TCQCIntfADCB
{
    public :
        TScrollOverADCB
        (
                    TCQCIntfView* const pcivCB
            , const TCQCIntfContainer&  iwdgStart
            , const TString&            strScrollCmd
            , const TString&            strTarget
        );

        ~TScrollOverADCB();

        TString             m_strScrollCmd;
        TString             m_strTarPath;
        TString             m_strTarget;
};


// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfAppHandler
//  PREFIX: miah
//
//  This is a simple mixin class that containing apps should mixin and then
//  use to pass as pointer to itself to the view when it creates it. This lets
//  the view call it back for certain things that only the app knows how to
//  deal with.
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfAppHandler
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MCQCIntfAppHandler();

        MCQCIntfAppHandler(const MCQCIntfAppHandler&) = default;
        MCQCIntfAppHandler(MCQCIntfAppHandler&&) = default;

        ~MCQCIntfAppHandler();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        MCQCIntfAppHandler& operator=(const MCQCIntfAppHandler&) = default;
        MCQCIntfAppHandler& operator=(MCQCIntfAppHandler&&) = default;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid AsyncCallback
        (
            const   tCQCIntfEng::EAsyncCmds eCmd
            ,       TCQCIntfView&           civSender
        ) = 0;

        virtual tCIDLib::TVoid AsyncDataCallback
        (
                    TCQCIntfADCB* const     padcbInfo
        ) = 0;

        virtual tCIDLib::TVoid CreateRemWidget
        (
            const   TString&                strUID
            , const tCQCIntfEng::ERIVAWTypes eType
            , const TArea&                  areaAt
            , const tCIDLib::TKVPList&      colParams
            , const tCIDLib::TBoolean       bInitVis
        );

        virtual tCIDLib::TVoid DestroyRemWidget
        (
            const   TString&                strUID
            , const tCQCIntfEng::ERIVAWTypes eType
        );

        virtual tCIDLib::TVoid DismissBlanker
        (
                    TCQCIntfView&           civSender
        ) = 0;

        virtual tCIDLib::TVoid DoSpecialAction
        (
            const   tCQCIntfEng::ESpecActs  eAct
            , const tCIDLib::TBoolean       bAsync
            ,       TCQCIntfView&           civSender
        ) = 0;

        virtual tCQCIntfEng::EAppFlags eAppFlags() const = 0;

        virtual tCIDLib::TVoid NewTemplate
        (
            const   TCQCIntfTemplate&       iwdgNew
            ,       TCQCIntfView&           civSender
            , const tCIDLib::TCard4         c4StackPos
        ) = 0;

        virtual MCQCCmdTracer* pmcmdtTrace() = 0;

        virtual tCIDLib::TVoid SetRemWidgetVis
        (
            const   TString&                strUID
            , const tCQCIntfEng::ERIVAWTypes eType
            , const tCIDLib::TBoolean       bState
        );

        virtual tCIDLib::TVoid SetRemWidgetURL
        (
            const   TString&                strUID
            , const tCQCIntfEng::ERIVAWTypes eType
            , const TString&                strURL
            , const tCIDLib::TKVPList&      colParams
        );
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfView
//  PREFIX: civ
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfView : public TObject, public MCQCCmdTarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Public Destructor
        // -------------------------------------------------------------------
        TCQCIntfView() = delete;

        TCQCIntfView(const TCQCIntfView&) = delete;
        TCQCIntfView(TCQCIntfView&&) = delete;

        ~TCQCIntfView();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfView& operator=(const TCQCIntfView&) = delete;
        TCQCIntfView& operator=(TCQCIntfView&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual TArea areaDisplayRes() const = 0;

        virtual TArea areaView() const = 0;

        virtual tCIDLib::TVoid AsyncCmdCallback
        (
            const   tCQCIntfEng::EAsyncCmds eCmd
        ) = 0;

        virtual tCIDLib::TVoid AsyncDataCallback
        (
                    TCQCIntfADCB* const     padcbData
        );

        virtual tCIDLib::TBoolean bActiveAction() const = 0;

        virtual tCIDLib::TBoolean bCheckTimeout();

        virtual tCIDLib::TBoolean bCreatePopUp
        (
            const   TString&                strTemplate
            , const tCIDLib::TBoolean       bSlideOut
            , const tCIDLib::EDirs          eDir
            , const TPoint&                 pntOrigin
            , const TString&                strAlignment
            , const tCIDLib::TBoolean       bTOPopup
            , const TString&                strPOParms
        ) = 0;

        virtual tCIDLib::TBoolean bHasPopups() const = 0;

        virtual tCIDLib::TBoolean bInitialize
        (
                    TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        ) = 0;

        virtual tCIDLib::TBoolean bInterceptCmd
        (
            const   TString&                strEventId
            , const TString&                strTargetId
            ,       TCQCCmdCfg&             ccfgToCheck
        )   const;

        virtual tCIDLib::TBoolean bLoadOverlay
        (
            const   TString&                strOvrName
            , const TString&                strToLoad
            ,       tCQCIntfEng::TErrList&  colErrs
        ) = 0;

        virtual tCIDLib::TBoolean bLoadNewTemplate
        (
                    TDataSrvClient&         dsclInit
            , const TCQCIntfTemplate&       iwdgToLoad
            ,       tCQCIntfEng::TErrList&  colErrs
        ) = 0;

        virtual TBitmap& bmpQueryContentUnder
        (
            const   TArea&                  areaInvalid
            , const tCIDLib::TBoolean       bIgnoreTop = kCIDLib::False
        );

        virtual TBitmap& bmpQueryContentUnder
        (
            const   TArea&                  areaInvalid
            , const TCQCIntfWidget* const   pwidgStopAt
            , const tCIDLib::TBoolean       bBefore
        );

        virtual tCIDLib::TCard4 c4StackSize() const = 0;

        virtual tCQCIntfEng::TGraphIntfDev cptrNewGraphDev() const = 0;

        virtual tCIDLib::TVoid Destroyed() = 0;

        virtual tCIDLib::TVoid DoActEvent
        (
                    tCQCIntfEng::TIntfCmdEv& iceToDo
        ) = 0;

        virtual tCIDLib::TVoid DoActiveUpdatePass() = 0;

        virtual tCIDLib::TVoid DoEventProcessingPass() = 0;

        virtual tCIDLib::TVoid DoModalLoop
        (
                    tCIDLib::TBoolean&      bBreakFlag
            , const tCIDLib::TBoolean       bNoEscape
        ) = 0;

        virtual tCIDLib::TVoid DoUpdatePass() = 0;

        virtual tCIDLib::TVoid DrawUnder
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            , const tCIDLib::TBoolean       bIgnoreTop = kCIDLib::False
        ) = 0;

        virtual tCIDLib::TVoid EndBranchReplace
        (
                    TCQCIntfContainer&      iwdgCont
        );

        virtual tCIDLib::TVoid ForceRedraw() = 0;

        virtual const TGraphDrawDev& gdevCompat() const = 0;

        virtual const TCQCIntfTemplate& iwdgTopMost() const = 0;

        virtual TCQCIntfTemplate& iwdgTopMost() = 0;

        virtual const TCQCIntfTemplate& iwdgBaseTmpl() const = 0;

        virtual TCQCIntfTemplate& iwdgBaseTmpl() = 0;

        virtual tCIDLib::TVoid InvalidateArea
        (
            const   TArea&                  areaToRedraw
        ) = 0;

        virtual MCQCCmdTarIntf* pmctarFind
        (
            const   TString&                strTarId
            , const tCIDLib::TBoolean       bBaseOnly = kCIDLib::False
        ) = 0;

        virtual tCIDLib::TVoid PauseTimers
        (
            const   tCIDLib::TBoolean       bPauseState
        ) = 0;

        virtual tCIDLib::TVoid PlayWAV
        (
            const   TString&                strToPlay
            , const tCIDLib::EWaitModes     eWait
        ) = 0;

        virtual tCIDLib::TVoid PostOps
        (
                    TCQCIntfWidget&         iwdgSender
            ,       MCQCCmdSrcIntf&         mcsrcAction
            , const TString&                strEvent
            , const tCIDLib::TCard4         c4HandlerId
            , const tCQCKit::EActOrders     eOrder
        );

        virtual tCIDLib::TVoid PostOps
        (
                    TCQCIntfWidget&         iwdgSender
            ,       MCQCCmdSrcIntf&         mcsrcAction
            , const TString&                strEventId
            , const tCIDLib::TCard4         c4HandlerId
            ,       TCQCIntfActCtx&         iactxTar
            ,       MCQCCmdTarIntf* const   pctarExtra
            , const tCQCKit::EActOrders     eOrder
        );

        virtual tCIDLib::TVoid QueryCombinedAreas
        (
                    TArea&                  areaToFill
        )   const = 0;

        virtual tCIDLib::TVoid Redraw() = 0;

        virtual tCIDLib::TVoid Redraw
        (
            const   TArea&                  areaToRedraw
        ) = 0;

        virtual tCIDLib::TVoid Redraw
        (
            const   TGUIRegion&             grgnToRedraw
        ) = 0;

        virtual tCIDLib::TVoid ScrPosChanged
        (
            const   TArea&                  areaTmplWnd
        )   = 0;

        virtual tCIDLib::TVoid SetVariable
        (
            const   TString&                strKey
            , const TString&                strValue
        );

        virtual tCIDLib::TVoid ShowErr
        (
            const   TString&                strTitle
            , const TString&                strText
            , const TError&                 errToShow
        ) = 0;

        virtual tCIDLib::TVoid ShowMsg
        (
            const   TString&                strTitle
            , const TString&                strText
            , const tCIDLib::TBoolean       bIsError
            , const MFormattable&           mfmtbl1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtbl2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtbl3 = MFormattable::Nul_MFormattable()
        ) = 0;

        virtual tCIDLib::TVoid StartBranchReplace
        (
                    TCQCIntfContainer&      iwdgCont
        );

        virtual tCIDLib::TVoid TakeFocus() = 0;

        virtual tCIDLib::TVoid Validate();

        virtual TWindow& wndOwner() = 0;

        virtual const TWindow& wndOwner() const = 0;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryTarSubActNames
        (
                    TCollection<TString>&   colToFill
            , const TString&                strCmdId
            , const tCQCKit::EActCmdCtx     eCmdCtx
        )   const override;

        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid CmdTarCleanup() override;

        tCIDLib::TVoid CmdTarInitialize
        (
            const   TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoFades() const;

        tCIDLib::TBoolean bDoFades
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bEnableTmplCaching() const;

        tCIDLib::TBoolean bEnableTmplCaching
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsBusy() const;

        tCIDLib::TBoolean bProcessGestEv
        (
            const   tCIDCtrls::EGestEvs     eEv
            , const TPoint&                 pntStart
            , const TPoint&                 pntAt
            , const TPoint&                 pntDelta
            , const tCIDLib::TBoolean       bTwoFinger
        );

        TStdVarsTar& ctarGlobalVars();

        const TCQCUserCtx& cuctxToUse() const
        {
            return m_cuctxToUse;
        }

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
        );

        tCIDLib::TVoid DrawMTextAlpha
        (
                    TGraphDrawDev&          gdevTar
            , const TString&                strText
            , const TArea&                  areaFormat
            , const tCIDLib::EHJustify      eHJustify = tCIDLib::EHJustify::Left
            , const tCIDLib::EVJustify      eVJustify = tCIDLib::EVJustify::Top
            , const tCIDLib::TBoolean       bWordBreak = kCIDLib::True
            , const tCIDLib::TCard1         c1ConstAlpha = 0xFF
        );

        tCIDLib::TVoid DrawTextAlpha
        (
                    TGraphDrawDev&          gdevTar
            , const TString&                strText
            , const tCIDLib::TCard4         c4StartInd
            , const tCIDLib::TCard4         c4Len
            , const TArea&                  areaFormat
            , const tCIDLib::EHJustify      eHJustify = tCIDLib::EHJustify::Left
            , const tCIDLib::EVJustify      eVJustify = tCIDLib::EVJustify::Center
            , const tCIDGraphDev::ETextFmts eFormat = tCIDGraphDev::ETextFmts::None
            , const tCIDLib::TCard1         c1ConstAlpha = 0xFF
            , const TRGBClr&                rgbBgnFill = TRGBClr::Nul_TRGBClr()
        );

        tCIDLib::TVoid DrawTextAlpha
        (
                    TGraphDrawDev&          gdevTar
            , const TString&                strText
            , const TArea&                  areaFormat
            , const tCIDLib::EHJustify      eHJustify = tCIDLib::EHJustify::Left
            , const tCIDLib::EVJustify      eVJustify = tCIDLib::EVJustify::Center
            , const tCIDGraphDev::ETextFmts eFormat = tCIDGraphDev::ETextFmts::None
            , const tCIDLib::TCard1         c1ConstAlpha = 0xFF
            , const TRGBClr&                rgbBgnFill = TRGBClr::Nul_TRGBClr()
        );

        tCIDLib::TVoid DrawTextFX
        (
                    TGraphDrawDev&          gdevTar
            , const TString&                strText
            , const TArea&                  areaDraw
            , const tCIDGraphDev::ETextFX  eEffect
            , const TRGBClr&                rgbClr1
            , const TRGBClr&                rgbClr2
            , const tCIDLib::EHJustify      eHJustify
            , const tCIDLib::EVJustify      eVJustify
            , const tCIDLib::TBoolean       bNoTextWrap
            , const TPoint&                 pntOffset
            , const tCIDLib::TCard1         c1ConstAlpha = 0xFF
        );

        tCQCIntfEng::ESpecActs eAutoBlankMode() const;

        tCQCIntfEng::ESpecActs eAutoBlankMode
        (
            const   tCQCIntfEng::ESpecActs  eToSet
        );

        tCIDLib::TVoid GestInertiaIdle
        (
            const   tCIDLib::TEncodedTime   enctEnd
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
        );

        MCQCIntfAppHandler& miahOwner() const;

        MCQCCmdTracer* pmcmdtTrace();

        tCIDLib::TVoid PerGestOpts
        (
            const   TPoint&                 pntAt
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TBoolean       bTwoFingers
            ,       tCIDCtrls::EGestOpts&   eToSet
            ,       tCIDLib::TFloat4&       f4VScale
        );

        tCIDLib::TVoid ProcessFlick
        (
            const   tCIDLib::EDirs          eDir
            , const TPoint&                 pntStartPos
        );

        tCIDLib::TVoid QueryAppFlags
        (
                    tCIDLib::TBoolean&      bSignageMode
        )   const;

        const TRGBClr& rgbBorder() const;

        const TRGBClr& rgbBorder
        (
            const   TRGBClr&                rgbToSet
        );

        const TString& strTitle() const;

        const TString& strTmplId() const;

        const TString& strBaseTmplName() const;

        const TString& strCurTmplName() const;

        const TCQCSecToken& sectUser() const
        {
            return m_cuctxToUse.sectUser();
        }

        tCIDLib::TVoid UpdateLastActivityStamp();


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TCQCIntfView
        (
            const   TString&                strTitle
            ,       MCQCIntfAppHandler* const pmiahToUse
            , const TCQCUserCtx&            cuctxToUse
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSinceLastActivity
        (
            const   tCIDLib::TEncodedTime   enctCheck
        );

        tCIDLib::TBoolean bTOEventTime
        (
            const   TCQCIntfTemplate&       iwdgToCheck
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckTextBufSz
        (
            const   tCIDLib::TCard4         c4Width
            , const tCIDLib::TCard4         c4Height
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDoFades
        //      We can optionally support popup fade in/out. This is set to
        //      false by default, but the user can set it via a command. It
        //      still won't do anything if in remote viewer mode or possibly
        //      some other conditions. It just says the user wants them if they
        //      are doable.
        //
        //  m_bEnableTmplCaching
        //      Template caching can be set on a per-view mode, which allows us to
        //      support per-session settings in the RIVA server. This is passed in
        //      to the facility class' template query helper to control whether it
        //      uses the cache or not.
        //
        //      We default this to true unless in designer mode, where it's defaulted
        //      to false.
        //
        //  m_bmpTBuf
        //  m_bmpTMask
        //      These are bitmap buffers for use in the fancy text drawing methods
        //      that we provide for widgets to use.
        //
        //  m_ctarGlobalVars
        //      Each window provides global variables for the templates that
        //      run in it. So we declare a standard variables command target
        //      for this purpose. This guy will get passed in during cmd
        //      editing at design time, and when commands are run at viewing
        //      time.
        //
        //  m_enctLastActivity
        //      Used to do popup auto-timeouts and other 'since activity' type
        //      stuff. This is updated any time we get any user input and when
        //      we come back from invoking any action.
        //
        //  m_eAutoBlankMode
        //      The special action we use when we auto-blank. Defaults to
        //      blank and blanker, but can be set to just blanker or blanker
        //      with time display.
        //
        //  m_janGesture
        //      When we get a gesture start, and we get a widget that wants
        //      to accept it. We have to remember that widget and pass the
        //      subsequent gesture events to it until we get an end. But
        //      any of the events could cause the widget to be destroyed.
        //      So we store it in a widget janitor. That way, it'll get
        //      marked as dead, but won't be deleted. So we can be sure it
        //      doesn't get whacked behind our back.
        //
        //  m_pixaTBlur
        //      Used in the fancy text drawing methods.
        //
        //  m_pmiahToUse
        //      The containing app has to provide us with a pointer to
        //      something that implements the app handler interface so that
        //      we can pass on those things that only he can deal with.
        //
        //  m_rgbBorder
        //      If the template doesn't fill the whole area of the view, this
        //      color is used to fill the unused background. We default it to
        //      black, but they can use a command to set it.
        //
        //  m_strTitle
        //      A title given to us to use in any popups we have to do, error
        //      dialogs, etc...
        //
        //  m_strTmplId
        //      For indentification purposes in apps that support more than
        //      one template window at once, we generate a unique id for
        //      ourself and provide a method to access it. It's a formatted
        //      MD5 hash.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bDoFades;
        tCIDLib::TBoolean       m_bEnableTmplCaching;
        TBitmap                 m_bmpTBuf;
        TBitmap                 m_bmpTMask;
        TStdVarsTar             m_ctarGlobalVars;
        const TCQCUserCtx&      m_cuctxToUse;
        tCIDLib::TEncodedTime   m_enctLastActivity;
        tCQCIntfEng::ESpecActs  m_eAutoBlankMode;
        TCQCIntfWdgJanitor      m_janGesture;
        TPixelArray             m_pixaTBlur;
        MCQCIntfAppHandler*     m_pmiahToUse;
        TRGBClr                 m_rgbBorder;
        TString                 m_strTitle;
        TString                 m_strTmplId;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfView,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTimerJan
//  PREFIX: jan
//
//  A little timer to ask the viewer to stop making timer calls, on a scoped
//  basis in a safe way.
// ---------------------------------------------------------------------------
class TCQCIntfTimerJan
{
    public :
        TCQCIntfTimerJan() = delete;

        TCQCIntfTimerJan(TCQCIntfView* const pcivTarget) :

            m_bActive(kCIDLib::True)
            , m_pcivTarget(pcivTarget)
        {
            m_pcivTarget->PauseTimers(kCIDLib::True);
        }

        ~TCQCIntfTimerJan()
        {
            if (m_pcivTarget && m_bActive)
                m_pcivTarget->PauseTimers(kCIDLib::False);
        }

        TCQCIntfTimerJan(const TCQCIntfTimerJan&) = delete;
        TCQCIntfTimerJan(TCQCIntfTimerJan&&) = delete;

        TCQCIntfTimerJan& operator=(const TCQCIntfTimerJan&) = delete;
        TCQCIntfTimerJan& operator=(TCQCIntfTimerJan&&) = delete;

        tCIDLib::TVoid Release()
        {
            if (m_pcivTarget && m_bActive)
            {
                m_pcivTarget->PauseTimers(kCIDLib::False);
                m_bActive = kCIDLib::False;
            }
        }

        tCIDLib::TVoid Reacquire()
        {
            if (m_pcivTarget && !m_bActive)
            {
                m_pcivTarget->PauseTimers(kCIDLib::True);
                m_bActive = kCIDLib::True;
            }
        }


    private :
        tCIDLib::TBoolean   m_bActive;
        TCQCIntfView*       m_pcivTarget;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStdView
//  PREFIX: civ
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStdView : public TCQCIntfView
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TCQCIntfStdView() = delete;

        TCQCIntfStdView(const TCQCIntfStdView&) = delete;
        TCQCIntfStdView( TCQCIntfStdView&&) = delete;

        ~TCQCIntfStdView();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStdView& operator=(const TCQCIntfStdView&) = delete;
        TCQCIntfStdView& operator=(TCQCIntfStdView&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AsyncCmdCallback
        (
            const   tCQCIntfEng::EAsyncCmds eCmd
        )   override;

        tCIDLib::TVoid AsyncDataCallback
        (
                    TCQCIntfADCB* const     padcbData
        )   override;

        tCIDLib::TBoolean bActiveAction() const override;

        tCIDLib::TBoolean bCheckTimeout() override;

        tCIDLib::TBoolean bCreatePopUp
        (
            const   TString&                strTemplate
            , const tCIDLib::TBoolean       bSlideOut
            , const tCIDLib::EDirs          eDir
            , const TPoint&                 pntOrigin
            , const TString&                strAlignment
            , const tCIDLib::TBoolean       bTOPopup
            , const TString&                strPOParms
        )   override;

        tCIDLib::TBoolean bHasPopups() const override;

        tCIDLib::TBoolean bInitialize
        (
                    TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TBoolean bLoadNewTemplate
        (
                    TDataSrvClient&         dsclInit
            , const TCQCIntfTemplate&       iwdgToLoad
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TBoolean bLoadOverlay
        (
            const   TString&                strOvrName
            , const TString&                strToLoad
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        TBitmap& bmpQueryContentUnder
        (
            const   TArea&                  areaInvalid
            , const tCIDLib::TBoolean       bIgnoreTop = kCIDLib::False
        )   override;

        TBitmap& bmpQueryContentUnder
        (
            const   TArea&                  areaInvalid
            , const TCQCIntfWidget* const   pwidgStopAt
            , const tCIDLib::TBoolean       bBefore
        )   override;

        tCIDLib::TCard4 c4StackSize() const override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid DoActiveUpdatePass() override;

        tCIDLib::TVoid DoEventProcessingPass() override;

        tCIDLib::TVoid DoUpdatePass() override;

        tCIDLib::TVoid DrawUnder
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            , const tCIDLib::TBoolean       bIgnoreTop = kCIDLib::False
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid EndBranchReplace
        (
                    TCQCIntfContainer&      iwdgCont
        )   override;

        const TCQCIntfTemplate& iwdgTopMost() const override;

        TCQCIntfTemplate& iwdgTopMost() override;

        const TCQCIntfTemplate& iwdgBaseTmpl() const override;

        TCQCIntfTemplate& iwdgBaseTmpl() override;

        MCQCCmdTarIntf* pmctarFind
        (
            const   TString&                strTarId
            , const tCIDLib::TBoolean       bBaseOnly = kCIDLib::False
        )   override;

        tCIDLib::TVoid PostOps
        (
                    TCQCIntfWidget&         iwdgSender
            ,       MCQCCmdSrcIntf&         mcsrcAction
            , const TString&                strEvent
            , const tCIDLib::TCard4         c4HandlerId
            , const tCQCKit::EActOrders     eOrder
        )   override;

        tCIDLib::TVoid PostOps
        (
                    TCQCIntfWidget&         iwdgSender
            ,       MCQCCmdSrcIntf&         mcsrcAction
            , const TString&                strEventId
            , const tCIDLib::TCard4         c4HandlerId
            ,       TCQCIntfActCtx&         iactxTar
            ,       MCQCCmdTarIntf* const   pctarExtra
            , const tCQCKit::EActOrders     eOrder
        )   override;

        tCIDLib::TVoid QueryCombinedAreas
        (
                    TArea&                  areaToFill
        )   const override;

        tCIDLib::TVoid ScrPosChanged
        (
            const   TArea&                  areaTmplWnd
        )   override;

        tCIDLib::TVoid SetVariable
        (
            const   TString&                strKey
            , const TString&                strValue
        )   override;

        tCIDLib::TVoid StartBranchReplace
        (
                    TCQCIntfContainer&      iwdgCont
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNoChildMouse() const;

        tCIDLib::TBoolean bNoChildMouse
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bQueryFldInfo
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       TCQCFldDef&             flddToFill
        );

        tCIDLib::TBoolean bSpecialKey
        (
            const   tCIDCtrls::EExtKeys     eKey
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
        );

        tCIDLib::TCard4 c4QueryStatus
        (
                    TString&                strBaseTmpl
            ,       TString&                strTopTmpl
        );

        tCIDLib::TVoid DisplayResChanged
        (
            const   TSize&                  szNew
        );

        tCIDLib::TVoid GettingFocus();

        tCIDLib::TVoid HotKey
        (
            const   tCQCIntfEng::EHotKeys   eKey
        );

        const TCQCIntfTemplate& iwdgAt
        (
            const   tCIDLib::TCard4         c4HandlerId
        )   const;

        TCQCIntfTemplate& iwdgAt
        (
            const   tCIDLib::TCard4         c4HandlerId
        );

        tCIDLib::TVoid LosingFocus();

        tCIDLib::TVoid NewSize
        (
            const   TSize&                  szNew
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQCIntfStdView
        (
            const   TString&                strTitle
            ,       MCQCIntfAppHandler* const pmiahToUse
            , const TRGBClr&                rgbBgn
            , const tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
            , const tCIDLib::TBoolean       bCenteredView
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfStdView
        (
            const   TString&                strTitle
            , const TCQCIntfTemplate&       iwdgTemplate
            ,       MCQCIntfAppHandler* const pmiahToUse
            , const TRGBClr&                rgbBgn
            , const tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
            , const tCIDLib::TBoolean       bCenteredView
            , const TCQCUserCtx&            cuctxToUse
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCenteredView
        //      Some derived view classes want the base template centered in
        //      the viewing area. Some want them upper left justified. The
        //      derived ctor tells our ctor which to do. This is only relevant
        //      if dynamic resizing isn't being done.
        //
        //  m_bInEventAction
        //      This is set whenever we are in an event driven action, so that
        //      we can prvent things from happening at the same time.
        //
        //  m_bNoChildMouse
        //      This can be set via a command. We don't use it here but window
        //      based derivatives can, to allow mouse move messages from being
        //      seen by child windows (of which web widgets are the only ones.)
        //      Some ActiveX stuff that loads into them gets confused by these
        //      mouse movement messages for some reason.
        //
        //  m_bmpDBuf
        //      We double buffer all drawing, and this is the overall bitmap
        //      that we output to. It's kept at the same size as the view
        //      itself, so widgets can be drawn at their native x,y positions
        //      as though it were the full view.
        //
        //  m_c4VarUpdate
        //      To optimize the updating of widgets that use variables, this
        //      is bumped any time an action is run, or when a variable is
        //      set. So we only need to do a variable update pass if this is
        //      non-zero. It's mutable because the checker method will check
        //      it and clear it.
        //
        //  m_enctBlankTime
        //      The period of inactivity that will cause the blanker
        //      to be kicked off. Defaults to zero, which means don't do
        //      blanking. It's set via an action command. The client code
        //      that provides the actual view implementation is responsible
        //      for invoking any blanker.
        //
        //  m_f8Lat
        //  m_f8Long
        //      We have to have the latitude/longitude info in order to
        //      determine night time if we get any events (one of the
        //      filters being if it's night or not.) This is passed to us by
        //      the client during ctor, and it doesn't change. The app has to
        //      be restarted to see a change in location.
        //
        //  m_pitmStack
        //      The interface template manager, which manages our stack of
        //      interfaces for us and does a lot of the grunt work. Most any
        //      non-designer app is going to want to support popups, and this
        //      guy mainly does that.
        //
        //  m_psmsgTmp
        //  m_psasubEvTrigs
        //      We subscribe to incoming event triggers, which is enabled on CQCKit
        //      so that he will publish them. We have a temp pub/sub message that we
        //      use to read in event msgs. We use an async subscription wrapper so that
        ///     we can process then when it's clean for us to, using a timer.
        //
        //  m_rgbBgn
        //      We are given a color in the ctor that we are to use for the
        //      overall background color.
        //
        //  m_strTOPopup
        //  m_strTOPopupParms
        //      One of the auto-blanker options is to invoke a popup, either
        //      itself or when the blanker is invoked so it will be there when
        //      the blanker is dismissed. If this is set, then we use it. We
        //      also need to store the parms to pass.
        //
        //  m_szLast
        //      We support minimal redraw, so we have to keep up with the last
        //      size we saw, so we can adjust when we get a new size.
        //
        //  m_tmNoon
        //  m_tmNow
        //  m_tmTmp
        //      These are temps that are used in the timer callback, to do
        //      event handling stuff that is time related.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bCenteredView;
        tCIDLib::TBoolean       m_bInEventAction;
        tCIDLib::TBoolean       m_bNoChildMouse;
        TBitmap                 m_bmpDBuf;
        tCIDLib::TCard4         m_c4VarUpdate;
        tCIDLib::TEncodedTime   m_enctBlankTime;
        tCIDLib::TFloat8        m_f8Lat;
        tCIDLib::TFloat8        m_f8Long;
        TCQCIntfTmplMgr*        m_pitmStack;
        TPubSubMsg              m_psmsgTmp;
        TPubSubAsyncSub         m_psasubEvTrigs;
        TRGBClr                 m_rgbBgn;
        TString                 m_strTOPopup;
        TString                 m_strTOPopupParms;
        TSize                   m_szLast;
        TTime                   m_tmNoon;
        TTime                   m_tmNow;
        TTime                   m_tmTmp;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStdView,TCQCIntfView)
};

#pragma CIDLIB_POPPACK

