//
// FILE NAME: CQCWebRIVA_View.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2009
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
//  This is the header for the CQCWebRIVA_View.cpp file, which provides an implementation
//  of the interface engine's view interface, optimized for our remote viewer needs. In
//  the regular IV the 'view' is a window, but here it is a more abstract thing, just a
//  virtual display that is actually displayed by the clients. We do go through the
//  motions of doing the operations, so that our graphical state is always what the real
//  IV's would be, just in case.
//
//  We actually derive from an intermediate view class, the 'standard view' class which
//  handles a lot of the common functionality that we need, to support a stack of popups
//  and so forth. So we don't have to do all that much here.
//
//  We override some callbacks in order to forward special commands to the RIVA clients.
//  And to handle comamnds that the IV engine forwards to the 'containing application' to
//  handle. In a couple cases we just throw an exception if some methods are called that
//  make no sense within a RIVA environment, i.e. the IV engine should only do them when
//  not in remote mode.
//
//  We override the call that lets the IV get a graphics device to draw on. We return our
//  special graphics device class, which just queues up commands to be sent to the client
//  (after doing them itself, as mentioned above, to insure that our graphics state is the
//  same as if we were drawing it locally, in case any IV code queries the state, or
//  saves/restores it.
//
//  We also handle the calls that deal with capture/release of the pointer and getting the
//  pointer position, since that is actually on the RIVA clients and we just store it here
//  to return in place of real pointer info.
//
//  We override the ShowError() methods and just send a message to the client. We can't
//  do anything but return, so these are async in the RIVA case, not blocking modal popups
//  as in the real IV. That's sort of a problem, but not much we can do about it.
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
//   CLASS: TCQCWebRIVAView
//  PREFIX: civ
// ---------------------------------------------------------------------------
class CQCWEBRIVAEXP TCQCWebRIVAView : public TCQCIntfStdView
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCWebRIVAView
        (
            const   TString&                strTitle
            , const tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
            , const TSize&                  szDevRes
            , const tCQCIntfEng::TGraphIntfDev cptrDev
            ,       TWebRIVAGraphDev*       pgdevToUse
            ,       TWebRIVAWIntf* const    pwrwiOwner
            ,       MCQCIntfAppHandler* const pmiahOwner
            , const TCQCUserCtx&            cuctxToUse
            , const tCIDLib::TFloat8        f8ClientLat
            , const tCIDLib::TFloat8        f8ClientLong

        );

        TCQCWebRIVAView
        (
            const   TCQCIntfTemplate&       iwdgTemplate
            , const TString&                strTitle
            , const tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
            , const TSize&                  szDevRes
            , const tCQCIntfEng::TGraphIntfDev cptrDev
            ,       TWebRIVAGraphDev*       pgdevToUse
            ,       TWebRIVAWIntf* const    pwrwiOwner
            ,       MCQCIntfAppHandler* const pmiahOwner
            , const TCQCUserCtx&            cuctxToUse
            , const tCIDLib::TFloat8        f8ClientLat
            , const tCIDLib::TFloat8        f8ClientLong
        );

        TCQCWebRIVAView(const TCQCWebRIVAView&) = delete;
        TCQCWebRIVAView(TCQCWebRIVAView&&) = delete;

        ~TCQCWebRIVAView();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCWebRIVAView& operator=(const TCQCWebRIVAView&) = delete;
        TCQCWebRIVAView& operator=(TCQCWebRIVAView&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TArea areaDisplayRes() const final;

        TArea areaView() const final;

        tCQCIntfEng::TGraphIntfDev cptrNewGraphDev() const final;

        tCIDLib::TVoid DoActEvent
        (
                    tCQCIntfEng::TIntfCmdEv& iceToDo
        )   final;

        tCIDLib::TVoid DoModalLoop
        (
                    tCIDLib::TBoolean&      bBreakFlag
            , const tCIDLib::TBoolean       bNoEscape
        )   final;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   final;

        tCIDLib::TVoid ForceRedraw() final;

        const TGraphDrawDev& gdevCompat() const final;

        tCIDLib::TVoid InvalidateArea
        (
            const   TArea&                  areaToRedraw
        )   final;

        tCIDLib::TVoid PauseTimers
        (
            const   tCIDLib::TBoolean       bPauseState
        )   final;

        tCIDLib::TVoid PlayWAV
        (
            const   TString&                strToPlay
            , const tCIDLib::EWaitModes     eWait
        )   final;

        tCIDLib::TVoid Redraw() final;

        tCIDLib::TVoid Redraw
        (
            const   TArea&                  areaToRedraw
        )   final;

        tCIDLib::TVoid Redraw
        (
            const   TGUIRegion&             grgnToRedraw
        )   final;

        tCIDLib::TVoid ShowErr
        (
            const   TString&                strTitle
            , const TString&                strText
            , const TError&                 errToShow
        )   final;

        tCIDLib::TVoid ShowMsg
        (
            const   TString&                strTitle
            , const TString&                strText
            , const tCIDLib::TBoolean       bIsError
            , const MFormattable&           mfmtbl1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtbl2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtbl3 = MFormattable::Nul_MFormattable()
        )   final;

        tCIDLib::TVoid TakeFocus() final;

        const TWindow& wndOwner() const final;

        TWindow& wndOwner() final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CaptureMouse();

        TPoint pntTouchPos() const;

        tCIDLib::TVoid ReleaseMouseCapture();

        tCIDLib::TVoid SetClientLocInfo
        (
            const   tCIDLib::TFloat8        f8ClientLat
            , const tCIDLib::TFloat8        f8ClientLong
        );

        tCIDLib::TVoid SetTouchPos
        (
            const   TPoint&                 pntPos
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMouseCaptured
        //      We keep up with whether the client has the mouse captured or not so we
        //      can avoid doing anything that would interrupt that.
        //
        //  m_c4ModalDepth
        //      We don't do modal loops here really, but we have to simulate them since
        //      the interface engine uses them whenever a popop or action is invoked. We
        //      just turn around can call back into the worker thread to simulate this,
        //      and use a janitor to increment this counter so we know if we are in one
        //      or not.
        //
        //      Currently this isn't used, but it's there if needed.
        //
        //  m_cptrRemDev
        //      Views return graphics devices via a counted pointer, because graphics
        //      devices don't support value semantics. But don't want clients having to
        //      clean them up (because sometimes they should not, as is the case for us,
        //      sine we have a single one that is always returned.) So the worker thread
        //      passes it to us via the counted pointer, so it will remain alive until
        //      he destroys us and then destroys that original counted pointer.
        //
        //      We also get a type specific pointer to it, which we store below for our
        //      own use. This counted pointer is a generic type defined at the interface
        //      engine level.
        //
        //  m_f8ClientLat
        //  m_f8ClientLong
        //      In addition to the system wide, server side lat/long info that gets stored
        //      on our parent class, we can also store client side lat/long info that the
        //      RIVA clients pass to us. This can be used in geo-distance calculation action
        //      commands. If none is set, then these are set to defaults to let us know they
        //      were never set.
        //
        //  m_gdevCompat
        //      We create a simple 'compatible' graphics device for handing back any time
        //      the engine asks for a compatible device. We can't use the drawing one
        //      (m_cptrRemDev) one because access to it has to be synchronized, whereas this
        //      one is a const one that is only used to pass to windows APIs that require a
        //      compatible device.
        //
        //  m_pgdevToUse
        //      A type specific pointer to the same remote graphics device as is stored in
        //      m_cptrRemDev. This is for our internal use so that we can call remote viewer
        //      specific methods it provides.
        //
        //  m_pntTouch
        //      The client keeps our worker thread apprised of the ptr pos, which he
        //      passes to us. We then make this available to any widgets that want to
        //      know or our base class.
        //
        //  m_pmiahOwner
        //      The worker thread up in the web server client facility implements the IV
        //      engine's app handler mixin.
        //
        //  m_pwrwiOwner
        //      A back-link to the thread that created us. It's a worker thread that
        //      is serving a remote viewer client, but we see it via our Web RIVA worker
        //      interface mixin, because we have no access to the worker thread class.
        //
        //  m_szDevRes
        //      The full device resolution provided by the calling code. We have to
        //      give this back on demand.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bMouseCaptured;
        tCIDLib::TCard4             m_c4ModalDepth;
        tCQCIntfEng::TGraphIntfDev  m_cptrRemDev;
        TGraphMemDev                m_gdevCompat;
        tCIDLib::TFloat8            m_f8ClientLat;
        tCIDLib::TFloat8            m_f8ClientLong;
        TWebRIVAGraphDev*           m_pgdevToUse;
        MCQCIntfAppHandler*         m_pmiahOwner;
        TPoint                      m_pntTouch;
        TWebRIVAWIntf*              m_pwrwiOwner;
        TSize                       m_szDevRes;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCWebRIVAView,TCQCIntfStdView)
};


#pragma CIDLIB_POPPACK


