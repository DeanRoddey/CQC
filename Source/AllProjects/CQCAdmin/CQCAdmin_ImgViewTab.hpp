//
// FILE NAME: CQCAdmin_ImgViewTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/09/2015
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
//  This is the header for the CQCClient_ImgViewTab.cpp file, which implements a
//  tab window that just contains a scroll area window, within which is a simple
//  static image window. This is used for read-only viewing of image.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TImgViewTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TImgViewTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TImgViewTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TImgViewTab(const TImgViewTab&) = delete;
        TImgViewTab(TImgViewTab&&) = delete;

        ~TImgViewTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TImgViewTab& operator=(const TImgViewTab&) = delete;
        TImgViewTab& operator=(TImgViewTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
            , const TBitmap&                bmpToDisplay
        );


    protected  :
        // -------------------------------------------------------------------
        //  Protected, inherited method
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   final;

        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TVoid Destroyed() final;



    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndScroll
        //      A scroll window that contains the image window and provides the
        //      scrolling.
        //
        //  m_pwndImage
        //      The static image window that displays the image that we are given
        //      to view.
        // -------------------------------------------------------------------
        TScrollArea*    m_pwndScroll;
        TStaticImg*     m_pwndImage;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TImgViewTab, TContentTab)
};

