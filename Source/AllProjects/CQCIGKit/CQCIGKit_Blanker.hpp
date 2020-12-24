//
// FILE NAME: CQCGKit_Blanker.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/13/2001
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
//  This is the header for the CQCGKit_Blanker.cpp file, which implements the very simple
//  blanker window. This window just goes full screen and fills itself with black.
//
//  We support some optional modes as well. We can display a clock that we move around
//  slowly so as not to burn in the screen. And we can also slowly rotate through a set
//  of images in a pre-defined scope of the image repository, so as to act like a simple
//  picture frame. The EBlankModes enum tells us which mode we are in.
//
//  Any mouse click will close the window. Any keystroke which is not a key mapped key will
//  also close it. If its a defined key mapped key, then the mapped action will be invoked.
//
//  The caller tells passes us his window, and we use that to get the area of the monitor
//  that the window is on. We get the full screen area of the monitor and size to that.
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
//  CLASS: TCQCBlankerWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class CQCIGKITEXPORT TCQCBlankerWnd : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCBlankerWnd
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        TCQCBlankerWnd(const TCQCBlankerWnd&) = delete;

        ~TCQCBlankerWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCBlankerWnd& operator=(const TCQCBlankerWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCIGKit::EBlankModes eMode() const;

        tCQCIGKit::EBlankModes eMode
        (
            const   tCQCIGKit::EBlankModes  eToSet
        );

        tCIDLib::TVoid Create
        (
            const   TWindow&                wndCaller
            , const TString&                strTitle
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChar
        (
            const   tCIDLib::TCh            chKey
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
            , const TWindow* const          pwndChild = nullptr
        )   override;

        tCIDLib::TBoolean bClick
        (
            const   tCIDCtrls::EMouseButts  eButton
            , const tCIDCtrls::EMouseClicks eClickType
            , const TPoint&                 pntAt
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bExtKey
        (
            const   tCIDCtrls::EExtKeys     eExtKey
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
            , const TWindow* const          pwndChild = nullptr
        )   override;

        tCIDLib::TVoid CodeReceived
        (
            const   tCIDLib::TInt4          i4Code
            , const tCIDLib::TCard4         c4Data
        )   override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TBoolean bPaint
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaUpdate
        )   override;

        tCIDLib::TVoid GettingFocus() override;

        tCIDLib::TVoid LosingFocus() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DrawClock
        (
                    TGraphDrawDev&          gdevToUse
            , const tCIDLib::TBoolean       bFillBgn
        );

        tCIDLib::TVoid DrawSlide
        (
                    TGraphDrawDev&          gdevToUse
            , const tCIDLib::TBoolean       bFillBgn
        );

        tCIDLib::EExitCodes eDownloadThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid UpdateContent
        (
            const   tCIDLib::TBoolean       bForce
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaContent
        //      If in a mode that requires we draw something, we keep up with the last
        //      place we displayed it, so we can both redraw it in the same place if we
        //      have to redraw the window and so we can orbit the content around in some
        //      cases, to prevent screen burn.
        //
        //  m_areaMonitor
        //      The area of the monitor we are running on, which provides the limit of
        //      movement for the clock display.
        //
        //  m_bPtrHidden
        //      We have to keep up with hiding/showing the pointer, and there are multiple
        //      places where this might happen. To keep up with it and not hide/show it more
        //      than once, we use this flag.
        //
        //  m_bmpSlide
        //      When the background thread posts to us that it has another image downloaded,
        //      we grab the contents from m_pimgSlide, and update this bitmap. We then use
        //      this do any drawing. That way, the background thread is free to download
        //      another without us having to fight over the image object.
        //
        //  m_c4LastSlideScopeId
        //  m_c4LastSlideIndex
        //      When we fill in m_colSlideImgs, we remember the serial number of the scope,
        //      and we'll periodically check for changes and update our list. 99% of the time
        //      we'll just be told we have the latest stuff.
        //
        //      And we keep up the index of the last slide we showed, just doing a round
        //      robin through them.
        //
        //  m_colSlideImgs
        //      If we are in slide show mode, we query the list of images in the predefined
        //      image repo scope that holds the images we are to use.
        //
        //  m_cuctxToUse
        //      The caller has to provide us with a security context that we need to do
        //      some things. It should be for the currently logged in user.
        //
        //  m_eCorner
        //      If in clock mode, we periodically move it around through the corners
        //      to avoid screen burnin issues. So we just keep rotating though this
        //      every so many minutes.
        //
        //  m_eMode
        //      We support a set of display modes. The code that invokes us tells us which
        //      mode to use. By default it's just a blank black window.
        //
        //  m_gfontDate
        //  m_gfontTime
        //      If we are in clock mode, we set up our fonts so we don't have to keep doing
        //      it iever time we update.
        //
        //  m_pimgSlide
        //      A pointer to the current slide image we are displaying if in slideshow
        //      mode. It's a pointer so that we don't have to force the PNG facility's
        //      headers onto all our clients, and since it's only used if in slide show
        //      mode.
        //
        //      To deal with synchronization issues, the m_bmpSlide is updated to hold the
        //      new slide image when the background thread indicates there is a new one
        //      available. At that point, we know the bgn thread is stopped, so we can
        //      access this without sync. After that we do all drawing from the bitmap.
        //
        //  m_strTitle
        //      Since we can be invoked from a number of different places, we let the
        //      caller provide us a title to display in popups.
        //
        //  m_tmLastUpdate
        //      The time that we last updated our content (started a new image download or
        //      updated the current time.)
        //
        //  m_tmidUpdate
        //      The id of the timer we start to update our clock.
        //
        //  m_thrSlideLoader
        //      If in slideshow mode, this thread is used to download images for display.
        //      When it has downloaded one, it posts a code to this window, and we go grab
        //      it and display it. The update timer just kicks off this thread periodically
        //      to get a new image.
        // -------------------------------------------------------------------
        TArea                   m_areaContent;
        TArea                   m_areaMonitor;
        tCIDLib::TBoolean       m_bPtrHidden;
        TBitmap                 m_bmpSlide;
        tCIDLib::TCard4         m_c4LastSlideScopeId;
        tCIDLib::TCard4         m_c4LastSlideIndex;
        tCIDLib::TStrList       m_colSlideImgs;
        const TCQCUserCtx&      m_cuctxToUse;
        tCQCIGKit::EBlankModes  m_eMode;
        TGUIFont                m_gfontDate;
        TGUIFont                m_gfontTime;
        TPNGImage*              m_pimgSlide;
        TString                 m_strTitle;
        TTime                   m_tmLastUpdate;
        tCIDCtrls::TTimerId     m_tmidUpdate;
        TThread                 m_thrSlideLoader;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCBlankerWnd, TFrameWnd)
};

#pragma CIDLIB_POPPACK

