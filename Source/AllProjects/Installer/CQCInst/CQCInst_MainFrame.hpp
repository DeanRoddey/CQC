//
// FILE NAME: CQCInst_MainFrameWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2001
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
//  This is the header for the CQCInst_MainFrameWnd.cpp file, which
//  implements the main frame window for the program.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TMainFrameWnd : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TMainFrameWnd();

        TMainFrameWnd(const TMainFrameWnd&) = delete;
        TMainFrameWnd(TMainFrameWnd&&) = delete;

        ~TMainFrameWnd();


        // -------------------------------------------------------------------
        // Public operators
        // -------------------------------------------------------------------
        TMainFrameWnd& operator=(const TMainFrameWnd&) = delete;
        TMainFrameWnd& operator=(TMainFrameWnd&&) = delete;


        // -------------------------------------------------------------------
        // Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreateMainWnd();

        tCIDLib::TBoolean bLogin
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            ,       TCQCUserCtx&            cuctxToFill
        );

        tCIDLib::TBoolean bValidatePort
        (
            const   TComboBox&              wndSrc
            ,       TString&                strErrText
        );

        tCIDLib::TBoolean bValidatePort
        (
            const   TComboBox&              wndSrc
            ,       TString&                strErrText
            ,       tCIDLib::TIPPortNum&    ippnFound
        );

        tCIDLib::TIPPortNum ippnExtractPort
        (
            const   TComboBox&              wndSrc
        );

        tCIDLib::TVoid LoadPortOpts
        (
            const   TInstInfoPanel&         panTar
            ,       TComboBox&              wndTarget
            , const tCIDLib::TIPPortNum     ippnCur
        );

        tCIDLib::TVoid SetNavFlags
        (
            const   tCQCInst::ENavFlags     eToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllowShutdown() final;

        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TBoolean bPaint
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaUpdate
        )   final;

        tCIDLib::TVoid DataReceived
        (
            const   tCIDLib::TCard4         c4DataId
            ,       tCIDLib::TVoid* const   pData
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        //
        //  Note that some of these are in the CQCInst_Utils.cpp file ot keep
        //  the main file from becoming too large.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFindOldVer();

        tCIDLib::TCard4 c4GlobalValidation();

        tCIDLib::TVoid CalcAreas
        (
            const   TSize&                  szNew
        );

        tCIDLib::TVoid CheckExit();

        tCIDCtrls::EEvResponses eMFClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TVoid FindNextPanel();

        tCIDLib::TVoid FindPrevPanel();

        tCIDLib::TVoid ReportChanges();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaXXX
        //      We pre-calculate the areas of our various displayed thingies any time
        //      the window size changes.
        //
        //  m_bmpLogo
        //      The logo bitmap that we draw in the upper left corner. We load it from a
        //      file in the same directory as the program.
        //
        //  m_bGotLogin
        //      If true, then m_cuctxAdmin has been set via valid admin login.
        //
        //  m_c4CurPanel
        //      The index of the currently displayed panel, i.e. the index into the
        //      m_colPanels list.
        //
        //  m_c4PanInd_Summary
        //      The only panel we need to know anything about specifically is the summary
        //      panel, so we save it's index. When the next panel would be the summary,
        //      we run all the overall validity checks and force the user back to any
        //      offending ones. The individual panels cannot validate some things since
        //      they can only see their own data.
        //
        //  m_colPanels
        //      A non-adopting ref vector that we put all the panels into so that we can
        //      move back and forth through them.
        //
        //  m_cuctxAdmin
        //      If we've gotten a valid admin login from the user, we store the creds here
        //      for later use.
        //
        //  m_infoInstall
        //      The user provided install info. Its gotten by bGetInstallInfo() and
        //      passed to the main window to be filled in and/or edited.
        //
        //  m_strLastMSEP
        //      This is the last target end point of the MS that we logged into successfully.
        //      If they ask to log in again and it's different, we know we need try again
        //      with the new target. This is only valid if m_bGotLogin is true.
        //
        //  m_pwndBackButton
        //  m_pwndCloseButton
        //  m_pwndNextButton
        //      We create these buttons on the fly, placing them under the content area
        //      to control movement back and forth through the info panels and to bail out.
        //
        //  m_pwndHideIn
        //      We need to hide the unused panels and show them one at a time. So we use
        //      this hidden window to parent them when not in use. This guarantees
        //      they are out of the tab order and never draw anything.
        // -------------------------------------------------------------------
        TArea                       m_areaBackButton;
        TArea                       m_areaCloseButton;
        TArea                       m_areaContent;
        TArea                       m_areaDivider;
        TArea                       m_areaLogo;
        TArea                       m_areaNextButton;
        TBitmap                     m_bmpLogo;
        tCIDLib::TBoolean           m_bGotLogin;
        tCIDLib::TCard4             m_c4CurPanel;
        tCIDLib::TCard4             m_c4PanInd_Summary;
        TRefVector<TInstInfoPanel>  m_colPanels;
        TCQCUserCtx                 m_cuctxAdmin;
        TCQCInstallInfo             m_infoInstall;
        TPushButton*                m_pwndBackButton;
        TPushButton*                m_pwndCloseButton;
        TPushButton*                m_pwndNextButton;
        TGenericWnd*                m_pwndHideIn;
        TString                     m_strLastMSEP;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMainFrameWnd,TFrameWnd)
};

#pragma CIDLIB_POPPACK


