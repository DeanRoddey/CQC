//
// FILE NAME: CIDWnd_EaseScroller.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/05/2013
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
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfView;

// ---------------------------------------------------------------------------
//   CLASS: TCIDEaseScrCBD
//  PREFIX: escrcbd
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCIDEaseScrCBD
{
    public :
        // -------------------------------------------------------------------
        //  Constructors Destructor
        // -------------------------------------------------------------------
        TCIDEaseScrCBD(const TCIDEaseScrCBD&) = delete;

        virtual ~TCIDEaseScrCBD();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCIDEaseScrCBD& operator=(const TCIDEaseScrCBD&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCIDEaseScrCBD();
};


// ---------------------------------------------------------------------------
//   CLASS: MCIDEaseScrollerCB
//  PREFIX: escrcb
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCIDEaseScrollerCB
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        MCIDEaseScrollerCB(const MCIDEaseScrollerCB&) = delete;

        virtual ~MCIDEaseScrollerCB();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        MCIDEaseScrollerCB& operator=(const MCIDEaseScrollerCB&) = delete;


        // -------------------------------------------------------------------
        //  Public-virtual method
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid EaseDrawAt
        (
                    TGraphDrawDev&          gdevTar
            , const TArea&                  areaSrc
            , const TArea&                  areaTar
            ,       TCIDEaseScrCBD* const   escrcbdCtx
        ) = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        MCIDEaseScrollerCB();
};


// ---------------------------------------------------------------------------
//   CLASS: TCIDEaseScroller
//  PREFIX: escr
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCIDEaseScroller : public TObject

{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCIDEaseScroller();

        TCIDEaseScroller(const TCIDEaseScroller&) = delete;

        ~TCIDEaseScroller();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCIDEaseScroller& operator=(const TCIDEaseScroller&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Scroll1
        (
                    TGraphDrawDev&          gdevTar
            , const TArea&                  areaTar
            , const TArea&                  areaScroll
            ,       TGraphDrawDev&          gdevBuf
            ,       TGraphDrawDev&          gdevBgn
            ,       TGraphDrawDev&          gdevScroll
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TCard4         c4Power
        );

        tCIDLib::TVoid Scroll2
        (
                    TGraphDrawDev&          gdevTar
            , const TArea&                  areaTar
            , const TArea&                  areaScroll
            ,       TGraphDrawDev&          gdevScroll
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TCard4         c4Power
        );

        tCIDLib::TVoid Scroll3
        (
                    TGraphDrawDev&          gdevTar
            , const TArea&                  areaTar
            , const TArea&                  areaScroll
            ,       TGraphDrawDev&          gdevBuf
            ,       MCIDEaseScrollerCB&     escrcbDraw
            ,       TCIDEaseScrCBD&         escrcbdCtx
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TCard4         c4Power
        );

        tCIDLib::TVoid Scroll4
        (
                    TGraphDrawDev&          gdevTar
            , const TArea&                  areaTar
            ,       TGraphDrawDev&          gdevNew
            ,       TGraphDrawDev&          gdevBgn
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TCard4         c4Power
            , const tCIDLib::TBoolean       bPush
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Scroll1_2
        (
                    TGraphDrawDev&          gdevTar
            , const TArea&                  areaTar
            , const TArea&                  areaScroll
            ,       TGraphDrawDev* const    pgdevBuf
            ,       TGraphDrawDev* const    pgdevBgn
            ,       TGraphDrawDev&          gdevScroll
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TCard4         c4Power
        );


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCIDEaseScroller,TObject)
};

#pragma CIDLIB_POPPACK


