//
// FILE NAME: CppDrvTest_TrigMonTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/21/2018
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
//  This is a tab window that goes inserted into the main tabbed window of our
//  interface. This one just displays any received event triggers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TTrigMonTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TTrigMonTab : public TTabWindow, public MIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TTrigMonTab();

        TTrigMonTab(const TTabWindow&) = delete;

        ~TTrigMonTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TTrigMonTab&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetMoniker
        (
            const   TString&                strMoniker
        );


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

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidSrc
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4EvCount
        //      We flip colors every other one, so we just run up this counter and test
        //      whether its even or odd.
        //
        //  m_colLines
        //      For reading back in lines from the formatted event, which we can load
        //      into the text output window.
        //
        //  m_psmsgTmp
        //      A temp pub/sub message to read new triggers into.
        //
        //  m_psasubEvTrigs
        //      An async subscription we set up for CQCKit's event trigger published topic.
        //      We use this to read incoming trigger info.
        //
        //  m_pwndOutput
        //      We just keep this sized slightly within our area. It's used to display
        //      the trigger output.
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
        //  m_strMoniker
        //      We are given the moniker of our driver. This way, we can filter out any
        //      triggers not from our driver, which makes debugging easier.
        //
        //  m_tmStamp
        //      We format out a time stamp before each trigger we spit out. So we set up
        //      this guy with the desired format and just update it to now each time, then
        //      format it out.
        //
        //  m_tmidUpdate
        //      We use a timer to keep our driver info up to date. This is the id of
        //      the timer we create for that.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4EvCount;
        tCIDLib::TStrList       m_colLines;
        TPubSubAsyncSub         m_psasubEvTrigs;
        TPubSubMsg              m_psmsgTmp;
        TTextOutWnd*            m_pwndOutput;
        TRGBClr                 m_rgbOrgBgn;
        TRGBClr                 m_rgbOrgText;
        TString                 m_strCurLine;
        TTextStringOutStream    m_strmOut;
        TTextStringInStream     m_strmIn;
        TString                 m_strMoniker;
        TTime                   m_tmStamp;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTrigMonTab, TTabWindow)
};

#pragma CIDLIB_POPPACK
