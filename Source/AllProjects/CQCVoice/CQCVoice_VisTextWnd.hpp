//
// FILE NAME: CQCVoice_VisTextWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/09/2017
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
//  This is the header for the CQCVoice_VisTextWnd.cpp file, which implements
//  our 'visual text' reply window that is used if we are configured to display our
//  replies as text (possibly in addition to speaking them.)
//
//  So we will receive text to display. When we do, we will be told if we need to wait
//  for spoken text to complete or not (since CQCVoice doesn't start listening again
//  until it does.) If so, then we will display a blinking indicator to make it clear
//  that we are waiting for the spoken text. Else we just display the text.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TCQCVoiceVisTextWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCVoiceVisTextWnd : public TWindow
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCVoiceVisTextWnd();

        ~TCQCVoiceVisTextWnd();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearText();

        tCIDLib::TVoid Create
        (
            const   TWindow&                wndParent
            , const TArea&                  areaInit
        );

        tCIDLib::TVoid ShowText
        (
            const   TString&                strMsg
            , const tCIDLib::TBoolean       bWaitForSpoken
        );

        tCIDLib::TVoid ReadyToListen();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bPaint
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaUpdate
        )   override;

        tCIDLib::TVoid Destroyed();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CalcAreas
        (
            const   TArea&                  areaSrc
        );

        tCIDLib::TVoid DrawSpeaking
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaUpdate
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaSpeaking
        //  m_areaText
        //      We precaculate the areas for the text and speaking indicator, when
        //      the area changes, so that we know
        //
        //  m_bSpeaking
        //      If we are showing the speaking indicator, this is the current state,
        //      so that we know how to draw it if we need to redraw.
        //
        //  m_gfontText
        //      The font we are using to display the text.
        //
        //  m_rgbSpeaking
        //      Our busy indicator fill color.
        //
        //  m_strText
        //      The text we are displaying.
        // -------------------------------------------------------------------
        TArea               m_areaSpeaking;
        TArea               m_areaText;
        tCIDLib::TBoolean   m_bSpeaking;
        TGUIFont            m_gfontText;
        TRGBClr             m_rgbSpeaking;
        TString             m_strText;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCVoiceVisTextWnd, TWindow)
};




// ---------------------------------------------------------------------------
//  CLASS: TCQCVoiceVisTextFrame
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCVoiceVisTextFrame : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCVoiceVisTextFrame();

        ~TCQCVoiceVisTextFrame();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearText();

        tCIDLib::TVoid Create();

        tCIDLib::TVoid ShowText
        (
            const   TString&                strMsg
            , const tCIDLib::TBoolean       bWaitForSpoken
        );

        tCIDLib::TVoid ReadyToListen();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )  override;

        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndClient
        //      The window that we keep in our client area and which does the actual
        //      display. We just pass any incoming calls on to him.
        // -------------------------------------------------------------------
        TCQCVoiceVisTextWnd*   m_pwndClient;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCVoiceVisTextFrame, TFrameWnd)
};
