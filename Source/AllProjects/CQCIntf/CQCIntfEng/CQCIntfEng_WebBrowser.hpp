//
// FILE NAME: CQCIntfEng_WebBrowser.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/11/2005
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
//  This is the header for the CQCIntfEng_WebBrowser.cpp file, which implements the
//  TCQCIntfWebBrowser class. It creates and manages a separate process which creates a
//  window and places it over this widget's area, and creates an embedded browser window
//  to fill that area. So it looks like it's part of our window but it's really not. This
//  is done because the embedded browser is a huge piece of crap that's under documented
//  and difficult to make stable over time. This way, we keep it completely outside of
//  our process space, and insure that a new one (in a new process) is created every time.
//
//  There are also a lot of tricky issues with threading and OLE, which is used to embed
//  the browser. Our system cannot provide a clean, single GUI thread scenario that it
//  wants. Even though there is a single GUI thread, we are often getting to it via sync
//  msg messages from background threads running actions. This can be various layers deep.
//  This seems to really confuse it.
//
//  So it's a pain because it's tricky to maintain a separate process window like this,
//  but it's the only thing that is practical under the circumstances.
//
//  The helper will shut down when we tell it to, or if it doesn't get at least a ping from
//  us within 10 seconds. In the normal course of events we are just pinging it regularly
//  using the active update callback. We ping every 5 seconds to be safe.
//
//  If we lose connection to it, we see if it's still running. If not, we'll try to restart
//  it once. If that fails, we give up and it just stays down.
//
//  This guy doesn't support any of the visual styles of a widget. It is just a place
//  holder the browser that will go there.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCWBHelperClientProxy;


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWebBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfWebBrowser : public TCQCIntfWidget
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfWebBrowser();

        TCQCIntfWebBrowser
        (
            const   TCQCIntfWebBrowser&       iwdgToCopy
        );

        ~TCQCIntfWebBrowser();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfWebBrowser& operator=
        (
            const   TCQCIntfWebBrowser&       iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
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

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid PostInit() override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Terminate() override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadPage(const TString& strToLoad);


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const   tCIDLib::TBoolean       bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   override;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   override;

        tCIDLib::TVoid DisplayStateChanged() override;

        tCIDLib::TVoid ParentDisplayStateChanged
        (
            const   tCQCIntfEng::EDispStates eState
        )   final;

        tCIDLib::TVoid ScrPosChanged
        (
            const   TArea&                  areaTmplWnd
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bShowHelper();

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid StartHelper();

        tCIDLib::TVoid TryReconnect();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAutoRefresh
        //  m_bSilentMode
        //      These are configured persistent values that are used to set options on
        //      the browser. They are passed to the external program and makes use of
        //      them.
        //
        //  m_c4Minutes
        //      The number of minutes between each auto-refresh. This is persisted, and
        //      passed to the external helper program.
        //
        //  m_c4RetryCnt
        //      If we lose connection, we'll try to recover a few times before we give up.
        //      Once we get that far, we just stop trying becuase it's probably a waste of
        //      time.
        //
        //  m_enctNextPing
        //      The next time we need to ping our external process, so that he doesn't close
        //      down, thinking we are gone away.
        //
        //  m_extpHelper
        //      The external process object we use to start the helper. This allows us to
        //      check if it's still there or not if we lose connection.
        //
        //  m_hwndHelper
        //      We get back the main frame window handle of the helper.
        //
        //  m_porbcBrowser
        //      A client proxy that we use to talk to the external application that we spawn
        //      of to display the browser.
        //
        //  m_strInitURL
        //      The initial URL to load, if any. Can be empty. This is persisted.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bAutoRefresh;
        tCIDLib::TBoolean           m_bSilentMode;
        tCIDLib::TCard4             m_c4Minutes;
        tCIDLib::TCard4             m_c4RetryCnt;
        tCIDLib::TEncodedTime       m_enctNextPing;
        TExternalProcess            m_extpHelper;
        tCIDCtrls::TWndHandle       m_hwndHelper;
        TCQCWBHelperClientProxy*    m_porbcBrowser;
        TString                     m_strInitURL;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfWebBrowser,TCQCIntfWidget)
        DefPolyDup(TCQCIntfWebBrowser)
};

#pragma CIDLIB_POPPACK



