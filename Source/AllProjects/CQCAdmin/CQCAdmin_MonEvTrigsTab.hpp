//
// FILE NAME: CQCAdmin_MonEvTrigsTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/06/2016
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
//  This is the header for the CQCClient_MonEvTrigsTab.cpp file, which watches event
//  triggers and displays them, to help users diagnose triggered event issues.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TWebBrowserWnd;

// ---------------------------------------------------------------------------
//  CLASS: TMonEvTrigsTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TMonEvTrigsTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMonEvTrigsTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TMonEvTrigsTab(const TMonEvTrigsTab&) = delete;
        TMonEvTrigsTab(TMonEvTrigsTab&&) = delete;

        ~TMonEvTrigsTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMonEvTrigsTab& operator=(const TMonEvTrigsTab&) = delete;
        TMonEvTrigsTab& operator=(TMonEvTrigsTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
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

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colLines
        //      For reading back in lines from the formatted event.
        //
        //  m_psmsgTmp
        //      A temp pub/sub message to read new triggers into.
        //
        //  m_psasubEvTrigs
        //      An async subscription we set up for CQCKit's event trigger published topic.
        //      We use this to read incoming trigger info.
        //
        //  m_pwndXXX
        //      We get some typed pointers to controls, for convenience. We don't own
        //      them.
        //
        //  m_rgbOrgBgn
        //  m_rgbOrgText
        //      We rotate the bgn/text colors of the text output window for each event.
        //      One set is the original colors of the text out window, so we remember them
        //      so as not to have to get them every time.
        //
        //  m_strCurLine
        //      Fore pulling lines from the formatted event.
        //
        //  m_strmOut
        //  m_strmIn
        //      For formatting out events, IN THIS ORDER for initialization purposes. We
        //      link the input stream to the output, to use to read back in the formatted
        //      content.
        //
        //  m_tmStamp
        //      We format out a time stamp before each trigger we spit out. So we set up
        //      this guy with the desired format and just update it to now each time, then
        //      format it out.
        //
        //  m_tmidMon
        //      we just use a timer to monitor for events. We don't need to do a blocking
        //      operation. We just check for available msgs else we do nothing. So we don't
        //      need the complications of a bgn thread.
        // -------------------------------------------------------------------
        tCIDLib::TStrList       m_colLines;
        TPubSubAsyncSub         m_psasubEvTrigs;
        TPubSubMsg              m_psmsgTmp;
        TTextOutWnd*            m_pwndOutput;
        TPushButton*            m_pwndClear;
        TPushButton*            m_pwndCopy;
        TRGBClr                 m_rgbOrgBgn;
        TRGBClr                 m_rgbOrgText;
        TString                 m_strCurLine;
        TTextStringOutStream    m_strmOut;
        TTextStringInStream     m_strmIn;
        TTime                   m_tmStamp;
        tCIDCtrls::TTimerId     m_tmidMon;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMonEvTrigsTab, TContentTab)
};

