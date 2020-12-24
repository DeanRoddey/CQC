//
// FILE NAME: CQCIntfWEng_View.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/06/2009
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
//  This is the header for the CQCIntfWEng_View.cpp file, which provides a
//  concrete implementation of the interface engine's view interface, based
//  on a real window. So basically we derive from the view class and just
//  implement all of the virtuals in terms of the window and pass on timer,
//  mouse, and keyboard input from the window to the engine.
//
//  Actually we derive from an intermediate class, the 'standard view' class
//  which handles a lot of the common functionality that we need.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfViewWnd;

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWView
//  PREFIX: civ
// ---------------------------------------------------------------------------
class CQCINTFWENGEXPORT TCQCIntfWView : public TCQCIntfStdView
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfWView
        (
                    TCQCIntfViewWnd* const  pwndOwner
            , const TString&                strTitle
            ,       MCQCIntfAppHandler* const pmiahToUse
            , const tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfWView
        (
            const   TCQCIntfTemplate&       iwdgTemplate
            ,       TCQCIntfViewWnd* const  pwndOwner
            , const TString&                strTitle
            ,       MCQCIntfAppHandler* const pmiahToUse
            , const tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TCQCIntfWView();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TArea areaDisplayRes() const override;

        TArea areaView() const override;

        tCQCIntfEng::TGraphIntfDev cptrNewGraphDev() const override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid DoActEvent
        (
                    tCQCIntfEng::TIntfCmdEv& iceToDo
        )   override;

        tCIDLib::TVoid DoModalLoop
        (
                    tCIDLib::TBoolean&      bBreakFlag
            , const tCIDLib::TBoolean       bNoEscape
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

        tCIDLib::TVoid InvalidateArea
        (
            const   TArea&                  areaToRedraw
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

        tCIDLib::TVoid Redraw() override;

        tCIDLib::TVoid Redraw
        (
            const   TArea&                  areaToRedraw
        )   override;

        tCIDLib::TVoid Redraw
        (
            const   TGUIRegion&             grgnToRedraw
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

        const TWindow& wndOwner() const override;

        TWindow& wndOwner() override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCIntfViewWnd& wndOwner2() const;

        TCQCIntfViewWnd& wndOwner2();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pgdevCompat
        //      A compatible graphics device that we have to provide to
        //      the base view class. It's never used to draw, only for places
        //      where a compatible device is required. It's just a device
        //      based on our window.
        //
        //  m_pwndOwner
        //      The window that owns us gives us a point to him so that we
        //      can pass on incoming calls that we are handling from the
        //      engine.
        // -------------------------------------------------------------------
        mutable TGraphWndDev*   m_pgdevCompat;
        TCQCIntfViewWnd*        m_pwndOwner;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfView,TCQCIntfStdView)
};


#pragma CIDLIB_POPPACK

