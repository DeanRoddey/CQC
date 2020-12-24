//
// FILE NAME: CQCIntfEd_EdView.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/02/2015
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
//  This is the header for the CQCIntfEd_EdView.cpp file, which provides a
//  concrete implementation of the interface engine's view interface. In
//  our case most of the functionality is just dummied out. We just provide
//  enough for the widgets to draw themselves. No actions will ever be run
//  there's no popups, nothing loaded into overlays, no update timers going
//  off, etc..., just a base template.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TIntfEdWindow;
class TWebBrowserWnd;

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfEdView
//  PREFIX: civ
// ---------------------------------------------------------------------------
class TCQCIntfEdView : public TCQCIntfView
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfEdView
        (
                    TIntfEditWnd* const     pwndOwner
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfEdView(const TCQCIntfEdView&) = delete;

        ~TCQCIntfEdView();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfEdView& operator=(const TCQCIntfEdView&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TArea areaDisplayRes() const override;

        TArea areaView() const override;

        tCIDLib::TVoid AsyncCmdCallback
        (
            const   tCQCIntfEng::EAsyncCmds eCmd
        )   override;

        tCIDLib::TVoid AsyncDataCallback
        (
                    TCQCIntfADCB* const     padcbInfo
        )   override;

        tCIDLib::TBoolean bActiveAction() const override;

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

        tCIDLib::TBoolean bLoadOverlay
        (
            const   TString&                strOvrName
            , const TString&                strToLoad
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TBoolean bLoadNewTemplate
        (
                    TDataSrvClient&         dsclToUse
            , const TCQCIntfTemplate&       iwdgToLoad
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TBoolean bHasPopups() const override;

        tCIDLib::TBoolean bInitialize
        (
                    TDataSrvClient&         dsclToUse
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TCard4 c4StackSize() const override;

        tCQCIntfEng::TGraphIntfDev cptrNewGraphDev() const override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid DoActEvent
        (
                    tCQCIntfEng::TIntfCmdEv& iceToDo
        )   override;

        tCIDLib::TVoid DoActiveUpdatePass() override;

        tCIDLib::TVoid DoEventProcessingPass() override;

        tCIDLib::TVoid DoModalLoop
        (
                    tCIDLib::TBoolean&      bBreakFlag
            , const tCIDLib::TBoolean       bNoEscape
        )   override;

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

        tCIDLib::TVoid ForceRedraw() override;

        const TGraphDrawDev& gdevCompat() const override;

        const TCQCIntfTemplate& iwdgTopMost() const override;

        TCQCIntfTemplate& iwdgTopMost() override;

        const TCQCIntfTemplate& iwdgBaseTmpl() const override;

        TCQCIntfTemplate& iwdgBaseTmpl() override;

        tCIDLib::TVoid InvalidateArea
        (
            const   TArea&                  areaToRedraw
        )   override;

        MCQCCmdTarIntf* pmctarFind
        (
            const   TString&                strTarId
            , const tCIDLib::TBoolean       bBaseOnly = kCIDLib::False
        )   override;

        tCIDLib::TVoid PauseTimers
        (
            const   tCIDLib::TBoolean       bPauseState
        )   override;

        tCIDLib::TVoid PlayWAV
        (
            const   TString&                strToPlay
            , const tCIDLib::EWaitModes     eWait
        )   override;

        tCIDLib::TVoid QueryCombinedAreas
        (
                    TArea&                  areaToFill
        )   const override;

        tCIDLib::TVoid Redraw() override;

        tCIDLib::TVoid Redraw
        (
            const   TArea&                  areaToRedraw
        )   override;

        tCIDLib::TVoid Redraw
        (
            const   TGUIRegion&             grgnToRedraw
        )   override;

        tCIDLib::TVoid ScrPosChanged
        (
            const   TArea&                  areaTmplWnd
        )   override;

        tCIDLib::TVoid SetVariable
        (
            const   TString&                strKey
            , const TString&                strValue
        )   override;

        tCIDLib::TVoid ShowErr
        (
            const   TString&                strTitle
            , const TString&                strText
            , const TError&                 errToShow
        )   override;

        tCIDLib::TVoid ShowMsg
        (
            const   TString&                strTitle
            , const TString&                strText
            , const tCIDLib::TBoolean       bIsError
            , const MFormattable&           mfmtbl1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtbl2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtbl3 = MFormattable::Nul_MFormattable()
        )   override;

        tCIDLib::TVoid TakeFocus() override;

        tCIDLib::TVoid Validate() override;

        const TWindow& wndOwner() const override;

        TWindow& wndOwner() override;



        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCancelTracking
        (
                    TCQCIntfContainer&      iwdgParent
            , const tCIDLib::TBoolean       bSendEnd
        );

        tCIDLib::TBoolean bFindGroupMembers
        (
            const   tCIDLib::TCard4         c4GroupId
            ,       tCQCIntfEng::TDesIdList& fcolToFill
            ,       tCIDLib::TCard4&        c4FirstZ
        );

        tCIDLib::TBoolean bFindGroupMembers
        (
            const   tCIDLib::TCard4         c4GroupId
            ,       tCQCIntfEng::TChildList& colToFill
            ,       tCIDLib::TCard4&        c4FirstZ
        );

        tCIDLib::TBoolean bIsTracking() const;

        tCIDLib::TVoid ExitCurrentProcessingLoop();

        TCQCIntfWidget* piwdgFindByDesId
        (
            const   tCIDLib::TCard8         c8ToFind
        );

        const TCQCIntfWidget* piwdgFindByDesId
        (
            const   tCIDLib::TCard8         c8ToFind
        )   const;

        TCQCIntfWidget* piwdgFindByDesId
        (
            const   tCIDLib::TCard8         c8ToFind
            ,       tCIDLib::TCard4&        c4ZOrder
        );

        const TCQCIntfWidget* piwdgFindByName
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bRecurse
        )   const;

        TCQCIntfWidget* piwdgFindByName
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bRecurse
        );

        TCQCIntfWidget* piwdgFindTracking();

        const TCQCIntfWidget* piwdgFindTracking() const;

        TPoint pntTouchPos() const;

        TIntfEditWnd* pwndEditor();

        const TIntfEditWnd* pwndEditor() const;

        const TString& strEdUpdateTopic() const;

        const TString& strTmplPath() const
        {
            return m_iwdgTemplate.strTemplateName();
        }

        const TString& strWdgPalTopic() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_iwdgTemplate
        //      The template we are editing. There's only ever a base template
        //      at edit time.
        //
        //  m_pgdevCompat
        //      A compatible graphics device for our window. This is used by
        //      the underlying engine whenever it needs a temporary compatible
        //      device for things like bitmap creation. We fault it in because
        //      there's a circular relationship between this view and the owning
        //      window. When we get created, the window pointer we get hasn't
        //      been created yet.
        //
        //  m_pwndOwner
        //      The window that owns us gives us a point to him so that we can pass
        //      on incoming calls to him where needed, and get info from him to pass
        //      back.
        // -------------------------------------------------------------------
        TCQCIntfTemplate        m_iwdgTemplate;
        mutable TGraphWndDev*   m_pgdevCompat;
        TIntfEditWnd*           m_pwndOwner;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfEdView,TCQCIntfView)
};


#pragma CIDLIB_POPPACK


