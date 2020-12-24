//
// FILE NAME: CQCIntfEng_WebBrowser.cpp
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
//  This file implements the class that creates and manages an embedded
//  web browser window.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfEng_.hpp"
#include    "CQCIntfEng_WebBrowser.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfWebBrowser,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_WebBrowser
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Format versions
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 1;


        // -----------------------------------------------------------------------
        //  Our capabilities flags
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
            , tCQCIntfEng::ECapFlags::Windowed
            , tCQCIntfEng::ECapFlags::AlwaysOnTop
        );


        // -----------------------------------------------------------------------
        //  The ping timeout period we use
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TEncodedTime     enctPingInt = kCIDLib::enctOneSecond * 5;


        // -----------------------------------------------------------------------
        //  The maximum times we'll try to connect (or reconnect) to the external helper
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4           c4MaxReconnect = 4;
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWebBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfWebBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfWebBrowser::TCQCIntfWebBrowser() :

    TCQCIntfWidget
    (
        CQCIntfEng_WebBrowser::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_WebBrowser)
    )
    , m_bAutoRefresh(kCIDLib::False)
    , m_bSilentMode(kCIDLib::False)
    , m_c4Minutes(1)
    , m_c4RetryCnt(0)
    , m_hwndHelper(kCIDCtrls::hwndInvalid)
    , m_porbcBrowser(nullptr)
{
    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/WebBrowser");
}

// The state always goes back to init and the proxy pointer to null
TCQCIntfWebBrowser::TCQCIntfWebBrowser(const TCQCIntfWebBrowser& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , m_bAutoRefresh(iwdgSrc.m_bAutoRefresh)
    , m_bSilentMode(iwdgSrc.m_bSilentMode)
    , m_c4Minutes(iwdgSrc.m_c4Minutes)
    , m_c4RetryCnt(0)
    , m_hwndHelper(kCIDCtrls::hwndInvalid)
    , m_porbcBrowser(nullptr)
    , m_strInitURL(iwdgSrc.m_strInitURL)
{
}

TCQCIntfWebBrowser::~TCQCIntfWebBrowser()
{
    // Should already be done, but just in case
    Terminate();
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebBrowser: Public operators
// ---------------------------------------------------------------------------

//
//  The browser window is NOT copied. It's purely a runtime attribute, as are
//  the coutner and next refresh time.
//
TCQCIntfWebBrowser&
TCQCIntfWebBrowser::operator=(const TCQCIntfWebBrowser& iwdgSrc)
{
    CIDAssert(m_porbcBrowser == nullptr, L"The web browser window has already been created");

    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);

        m_bAutoRefresh = iwdgSrc.m_bAutoRefresh;
        m_bSilentMode  = iwdgSrc.m_bSilentMode;
        m_c4Minutes    = iwdgSrc.m_c4Minutes;
        m_strInitURL   = iwdgSrc.m_strInitURL;

        // These aren't copied, just reset
        if (m_porbcBrowser)
            delete m_porbcBrowser;
        m_porbcBrowser = nullptr;
        m_c4RetryCnt = 0;
        m_hwndHelper = kCIDCtrls::hwndInvalid;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfWebBrowser::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfWebBrowser& iwdgOther(static_cast<const TCQCIntfWebBrowser&>(iwdgSrc));

    // Do our own stuff
    return
    (
        (m_bAutoRefresh == iwdgOther.m_bAutoRefresh)
        && (m_bSilentMode == iwdgOther.m_bSilentMode)
        && (m_c4Minutes == iwdgOther.m_c4Minutes)
        && (m_strInitURL == iwdgOther.m_strInitURL)
    );
}


tCIDLib::TVoid TCQCIntfWebBrowser::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfWebBrowser&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfWebBrowser::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (!m_porbcBrowser)
        return tCQCKit::ECmdRes::Ok;

    try
    {
        // We just delegate the commands to the browser window we manage
        if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_WebNavOp)
        {
            const TString& strOp = ccfgToDo.piAt(0).m_strValue;

            tWebBrowser::ENavOps eOp;
            if (strOp == L"Back")
                eOp = tWebBrowser::ENavOps::Back;
            else if (strOp == L"Forward")
                eOp = tWebBrowser::ENavOps::Forward;
            else if (strOp == L"Home")
                eOp = tWebBrowser::ENavOps::Home;
            else if (strOp == L"Search")
                eOp = tWebBrowser::ENavOps::Search;
            else if (strOp == L"Reload")
                eOp = tWebBrowser::ENavOps::Reload;
            else if (strOp == L"Stop")
                eOp = tWebBrowser::ENavOps::Stop;
            else
            {
                if (acteTar.pcmdtDebug())
                    acteTar.pcmdtDebug()->ActDebug(L"Unknown naviation operation");
                return tCQCKit::ECmdRes::Ok;
            }

            m_porbcBrowser->DoOp(eOp);
        }
         else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_WebNavURL)
        {
            // If a pre-load, just save it as the initial URL
            if (strEventId == kCQCKit::strEvId_OnPreload)
            {
                m_strInitURL = ccfgToDo.piAt(0).m_strValue;
            }
            else
            {
                m_porbcBrowser->SetURL(ccfgToDo.piAt(0).m_strValue);

                // It worked so we can bump our next ping time
                m_enctNextPing = TTime::enctNow() + CQCIntfEng_WebBrowser::enctPingInt;
            }
        }
         else
        {
            // Not one of ours, so let our parent try it
            return TParent::eDoCmd
            (
                ccfgToDo
                , c4Index
                , strUserId
                , strEventId
                , ctarGlobals
                , bResult
                , acteTar
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid TCQCIntfWebBrowser::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Try to start up the external helper
    StartHelper();
}


tCIDLib::TVoid TCQCIntfWebBrowser::PostInit()
{
    // Get the external widget visible, if we managed to create it
    if (m_porbcBrowser)
    {
        // If we fail to get it set up, then reset and we'll start retrying
        if (!bShowHelper())
            Reset();
    }
}


tCIDLib::TVoid
TCQCIntfWebBrowser::QueryCmds(          TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // And add our commands
    if (eContext == tCQCKit::EActCmdCtx::Standard)
    {
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_WebNavOp
                , facCQCKit().strMsg(kKitMsgs::midCmd_WebNavOp)
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Operation)
                , L"Back, Forward, Home, Search, Reload, Stop"
            )
        );
    }

    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_WebNavURL
            , facCQCKit().strMsg(kKitMsgs::midCmd_WebNavURL)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_URL)
        )
    );
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfWebBrowser::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove any base attributes we don't honor, which is most of them
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_BgnFill);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Border);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Rounding);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Shadow);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Transparent);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Bgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Bgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Do our attributes
    adatTmp.Set(L"Browser Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Auto Refresh"
        , kCQCIntfEng::strAttr_WebBrws_AutoRefresh
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetBool(m_bAutoRefresh);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Silent Mode"
        , kCQCIntfEng::strAttr_WebBrws_Silent
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetBool(m_bSilentMode);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Minutes"
        , kCQCIntfEng::strAttr_WebBrws_Minutes
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetCard(m_c4Minutes);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Init URL"
        , kCQCIntfEng::strAttr_WebBrws_InitURL
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(m_strInitURL);
    colAttrs.objAdd(adatTmp);
}


tCIDLib::TVoid
TCQCIntfWebBrowser::Replace(const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::URL))
    {
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strInitURL, bRegEx, bFull, regxFind
        );
    }
}



TSize TCQCIntfWebBrowser::szDefaultSize() const
{
    return TSize(256, 256);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfWebBrowser::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_WebBrws_AutoRefresh)
        m_bAutoRefresh = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebBrws_InitURL)
        m_strInitURL = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebBrws_Minutes)
        m_c4Minutes = adatNew.c4Val();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebBrws_Silent)
        m_bSilentMode = adatNew.bVal();
}


//
//  WE override this so that we can clean up the
//
tCIDLib::TVoid TCQCIntfWebBrowser::Terminate()
{
    // Just call the rest method
    Reset();
}


tCIDLib::TVoid
TCQCIntfWebBrowser::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);
}



// ---------------------------------------------------------------------------
//  TCQCIntfWebBrower: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfWebBrowser::LoadPage(const TString& strToLoad)
{
    // Queue it up to load
    if (m_porbcBrowser)
    {
        try
        {
            m_porbcBrowser->SetURL(strToLoad);

            // It worked so we can bump our next ping time
            m_enctNextPing = TTime::enctNow() + CQCIntfEng_WebBrowser::enctPingInt;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            civOwner().ShowErr(L"Web Browser Widget", L"Could not set URL", errToCatch);
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We have to ping our helper process every so often if we've not sent him a message,
//  else he'll close down.
//
tCIDLib::TVoid
TCQCIntfWebBrowser::ActiveUpdate(const  tCIDLib::TBoolean   bNoRedraw
                                , const TGUIRegion&         grgnClip
                                , const tCIDLib::TBoolean   bInTopLayer)
{
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if (enctCur > m_enctNextPing)
    {
        // Update the time first
        m_enctNextPing = enctCur + CQCIntfEng_WebBrowser::enctPingInt;

        //
        //  If we don't have the browser object, then let's try to recover if we have
        //  not already used up all of our lives.
        //
        if (!m_porbcBrowser && (m_c4RetryCnt < CQCIntfEng_WebBrowser::c4MaxReconnect))
            TryReconnect();

        // If we have the browser, let's try to ping him
        if (m_porbcBrowser)
        {
            try
            {
                m_porbcBrowser->Ping();
            }

            catch(TError& errToCatch)
            {
                if (facCQCIntfEng().bLogWarnings())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                // If a connection loss, reset so we start retrying
                if (m_porbcBrowser->bCheckForLostConnection(errToCatch))
                {
                    facCQCIntfEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcWBrws_LostConn
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strWidgetId()
                    );
                    Reset();
                }
            }
        }
    }
}


//
//  This can happen at viewing time, when the size of the window is changed, which in turn
//  will change our widget's origin.
//
tCIDLib::TVoid
TCQCIntfWebBrowser::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    TParent::AreaChanged(areaNew, areaOld);

    // If the browser was created, then update it's size/pos
    if (m_porbcBrowser)
    {
        try
        {
            // Get our parent view window's position in screen coordinates
            TArea areaPar;
            civOwner().wndOwner().ToScreenCoordinates(civOwner().areaView(), areaPar);

            // Add our area to the screen area, since ours is relative to the window
            TArea areaBrws(areaActual());
            areaBrws += areaPar.pntOrg();

            // And send the new areas to the browser helper
            m_porbcBrowser->SetArea(areaPar, areaBrws);

            // It worked so we can bump our next ping time
            m_enctNextPing = TTime::enctNow() + CQCIntfEng_WebBrowser::enctPingInt;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            // If a connection loss, reset so we start retrying
            if (m_porbcBrowser->bCheckForLostConnection(errToCatch))
            {
                facCQCIntfEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIEngErrs::errcWBrws_LostConn
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strWidgetId()
                );
                Reset();
            }
        }

        catch(...)
        {
        }
    }
}



//
//  Since we manage a real window, we have to react to display state changes
//  and hide/show the window.
//
tCIDLib::TVoid TCQCIntfWebBrowser::DisplayStateChanged()
{
    // If the browser was created, then adjust the display state
    if (m_porbcBrowser)
    {
        try
        {
            m_porbcBrowser->SetVisibility(eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            // If a connection loss, reset so we start retrying
            if (m_porbcBrowser->bCheckForLostConnection(errToCatch))
            {
                facCQCIntfEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIEngErrs::errcWBrws_LostConn
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strWidgetId()
                );
                Reset();
            }
        }

        catch(...)
        {
        }
    }
}


//
//  In our case, if our parent id hidden, we need to hid our camera window since
//  it's not affected by the interface level visibility.
//
tCIDLib::TVoid
TCQCIntfWebBrowser::ParentDisplayStateChanged(const tCQCIntfEng::EDispStates eState)
{
    if (TFacCQCIntfEng::bRemoteMode())
    {
        if (TFacCQCIntfEng::bWebRIVAMode())
        {
        }
    }
     else
    {
        // If the browser was created, then adjust the display state
        if (m_porbcBrowser)
        {
            try
            {
                m_porbcBrowser->SetVisibility(eState != tCQCIntfEng::EDispStates::Hidden);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                // If a connection loss, reset so we start retrying
                if (m_porbcBrowser->bCheckForLostConnection(errToCatch))
                {
                    facCQCIntfEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcWCam_LostConn
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strWidgetId()
                    );
                    Reset();
                }
            }

            catch(...)
            {
            }
        }
    }
}


//
//  This is a special case. Our area, which is relative to our parent view (window) hasn't
//  changed, but the position of the window on the screen has. So in this case, we have to
//  move the browser to keep it in the right position.
//
tCIDLib::TVoid TCQCIntfWebBrowser::ScrPosChanged(const TArea& areaTmplWnd)
{
    // If the browser was created, then update it's size/pos
    if (m_porbcBrowser)
    {
        try
        {
            // Add our area to the template window area, which is already in screen coordinates
            TArea areaNew(areaActual());
            areaNew += areaTmplWnd.pntOrg();

            m_porbcBrowser->SetArea(areaTmplWnd, areaNew);

            // It worked so we can bump our next ping time
            m_enctNextPing = TTime::enctNow() + CQCIntfEng_WebBrowser::enctPingInt;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            // If a connection loss, reset so we start retrying
            if (m_porbcBrowser->bCheckForLostConnection(errToCatch))
            {
                facCQCIntfEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIEngErrs::errcWBrws_LostConn
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strWidgetId()
                );
                Reset();
            }
        }

        catch(...)
        {
        }
    }
}


tCIDLib::TVoid TCQCIntfWebBrowser::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_WebBrowser::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // Read in our stuff
    strmToReadFrom  >> m_bAutoRefresh
                    >> m_strInitURL
                    >> m_bSilentMode
                    >> m_c4Minutes;

    // There shouldn't be an existing proxy, but destroy it if so
    if (m_porbcBrowser)
    {
        delete m_porbcBrowser;
        m_porbcBrowser = nullptr;
    }

    // And it should all end with an endmarker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfWebBrowser::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_WebBrowser::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // And stream out our stuff and our end marker
    strmToWriteTo   << m_bAutoRefresh
                    << m_strInitURL
                    << m_bSilentMode
                    << m_c4Minutes
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfWebBrowser::Update(         TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    //
    //  If in designer mode, draw a rectangle to show where we would go at viewing time,
    //  and fill the background.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        if (bIsTransparent())
        {
            const tCIDGraphDev::EMixModes eSaveMode = gdevTarget.eMixMode();
            gdevTarget.eMixMode(tCIDGraphDev::EMixModes::XorPen);
            gdevTarget.Stroke(grgnClip, facCIDGraphDev().rgbWhite);
            gdevTarget.eMixMode(eSaveMode);
        }
         else
        {
            TParent::Update(gdevTarget, areaInvalid, grgnClip);
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebBrowser: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Once the helper is created, we can show it. This is split out because, in the initial
//  scenario (i.e. not recovering from a failure of the helper but on initial creation) we
//  do this separately in PostInit.
//
tCIDLib::TBoolean TCQCIntfWebBrowser::bShowHelper()
{
    // Only do this in viewing mode and if not in remote viewer mode.
    if (TFacCQCIntfEng::bDesMode() || TFacCQCIntfEng::bRemoteMode())
        return kCIDLib::True;

    // If we didn't manage to create the external helper, then nothing to do
    if (!m_porbcBrowser)
        return kCIDLib::True;


    //
    //  Convert our area to screen coordinates. And do the same for our containing parent
    //  window.
    //
    TArea areaUs;
    civOwner().wndOwner().ToScreenCoordinates(areaActual(), areaUs);

    TArea areaPar;
    civOwner().wndOwner().ToScreenCoordinates(civOwner().areaView(), areaPar);

    try
    {
        m_porbcBrowser->Initialize
        (
            areaPar
            , areaUs
            , eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden
            , m_strInitURL
        );

        // It worked so we can bump our next ping time
        m_enctNextPing = TTime::enctNow() + CQCIntfEng_WebBrowser::enctPingInt;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCIntfEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcWBrws_ShowFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strWidgetId()
        );
        return kCIDLib::False;
    }

    return kCIDLib::True;
}



//
//  If we find that the external helper is not running, then this is called to reset us
//  and start trying to reload it.
//
tCIDLib::TVoid TCQCIntfWebBrowser::Reset()
{
    if (m_porbcBrowser)
    {
        try
        {
            m_porbcBrowser->Close();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }

        try
        {
            delete m_porbcBrowser;
        }

        catch(...)
        {
        }

        m_porbcBrowser = nullptr;
    }
}


//
//  This is called to try to start up the external helper. It only does soemthing if we
//  are in non-RIVA, non-designer mode.
//
tCIDLib::TVoid TCQCIntfWebBrowser::StartHelper()
{
    // Only do this in viewing mode and if not in remote viewer mode.
    if (TFacCQCIntfEng::bDesMode() || TFacCQCIntfEng::bRemoteMode())
        return;

    //
    //  We want to get the external helper process started. It will be invisible until
    //  it is shown.
    //
    try
    {
        //
        //  Generate an id that we'll pass to the program. This is our connection point to
        //  create the shared resources we need to get ourselves set up.
        //
        TString strUniqueId = L"WebBrowserDat";
        strUniqueId.AppendFormatted(facCIDLib().c4NextId());

        //
        //  Create a shared event, in reset state. The helper will trigger it once he
        //  has the data set up that we need.
        //
        tCIDLib::TBoolean bCreated;
        TResourceName rsnBuf(kCIDLib::pszResCompany, L"CQCWBHelper", strUniqueId);
        TEvent evDataReady
        (
            rsnBuf
            , bCreated
            , tCIDLib::EEventStates::Reset
            , tCIDLib::ECreateActs::CreateIfNew
            , kCIDLib::True
        );

        //
        //  Create the shared memory buffer that we will pass info in with and he will
        //  return info in.
        //
        TSharedMemBuf mbufInfo
        (
            4096
            , 8192
            , rsnBuf
            , bCreated
            , tCIDLib::EMemAccFlags::ReadWrite
            , tCIDLib::ECreateActs::CreateIfNew
        );

        //
        //  Format out the data. We leave a Card4 blank at the start and go back and fill
        //  it in with the size.
        //
        tCIDLib::TCard4 c4Size = 0;
        {
            TBinMBufOutStream strmOut(&mbufInfo);
            strmOut << tCIDLib::TCard4(0)
                    << tCIDLib::EStreamMarkers::StartObject
                    << m_bSilentMode
                    << (m_bAutoRefresh ? m_c4Minutes : 0)
                    << TCardinal(tCIDLib::TCard4(civOwner().wndOwner().hwndThis()), tCIDLib::ERadices::Hex)
                    << tCIDLib::EStreamMarkers::EndObject;

            strmOut.Flush();
            c4Size = strmOut.c4CurPos();
        }
        mbufInfo.PutCard4(c4Size, 0);

        //
        //  Build up the command line. It'll be in the same directory as our library
        //  facility.
        //
        TPathStr pathCmdLine(256UL);
        pathCmdLine = facCQCIntfEng().strPath();
        pathCmdLine.AddLevel(L"CQCWBHelper");
        pathCmdLine.AppendExt(L"exe");


        // Set up the parameters
        tCIDLib::TStrList colParms(4);
        colParms.objAdd(strUniqueId);

        //
        //  These are standard ones that get removed by the core CIDLib stuff. We have to
        //  tell him the name server address, even though he doesn't use it. And the data
        //  directory.
        //
        TString strNSAddr(L"/NSAddr=");
        strNSAddr.Append(TSysInfo::strNSAddr());
        colParms.objAdd(strNSAddr);

        TString strDataDir(L"/DataDir=");
        strDataDir.Append(facCQCKit().strDataDir());
        strDataDir.Append(L"/");
        colParms.objAdd(strDataDir);

        m_extpHelper.Start
        (
            pathCmdLine
            , TString::strEmpty()
            , colParms
            , tCIDLib::EExtProcFlags::None
            , tCIDLib::EExtProcShows::Hidden
        );


        // For debugging, invoke the helper in the debugger
        #if 0
        pathCmdLine = L"devenv.exe /debugexe CQCWBHelper.exe ";
        pathCmdLine.Append(strUniqueId);
        m_extpHelper.SystemEscape(pathCmdLine);
        #endif

        //
        //  Now we wait for him to signal us he has filled in the memory buffer which the
        //  info we need.
        //
        #if CID_DEBUG_ON
        evDataReady.WaitFor(60000);
        #else
        evDataReady.WaitFor(5000);
        #endif

        TOrbObjId ooidServer;
        {
            // The first card4 is the bytes of data to stream
            TBinMBufInStream strmSrc(&mbufInfo, mbufInfo.c4At(0));

            // Skip the size now
            tCIDLib::TCard4 c4Skip;
            strmSrc >> c4Skip;

            strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);
            tCIDLib::TCard4 c4WndHandle;
            strmSrc >> ooidServer
                    >> c4WndHandle;
            strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);

            // Convert the window handle to its real type
            m_hwndHelper = tCIDCtrls::TWndHandle(c4WndHandle);
        }

        //
        //  And let's try to create our proxy now. Tell it no name server binding, since
        //  we aren't using the name server.
        //
        m_porbcBrowser = new TCQCWBHelperClientProxy(ooidServer, TString::strEmpty());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCIntfEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcWBrws_StartFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strWidgetId()
        );
    }
}


//
//  If we have lost connection, or never managed to get it, then this is called to try
//  get us going again. It'll only be called so many times then we just stay offline.
//
tCIDLib::TVoid TCQCIntfWebBrowser::TryReconnect()
{
    facCQCIntfEng().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kIEngErrs::errcWBrws_Retrying
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , TCardinal(m_c4RetryCnt + 1)
        , strWidgetId()
    );

    //
    //  If the external program seems to be running, try to stop it. We can't use the
    //  control interface because we already lost connection.
    //
    if (m_extpHelper.bIsRunning())
    {
        try
        {
            facCIDCtrls().SendShutdown(m_hwndHelper);

            // Wait a bit for it to end
            tCIDLib::EExitCodes eToFill;
            m_extpHelper.bWaitForDeath(eToFill, 200);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcWBrws_StopFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strWidgetId()
            );

            // Failed, so bump the retry count and return
            m_c4RetryCnt++;
            return;
        }

        //
        //  If still running, we failed. Note that it might actually still shut down. We don't
        //  wait long above so as to avoid locking up the GUI thread. If so, then it'll be
        //  down by the next time we get called here.
        //
        if (m_extpHelper.bIsRunning())
        {
            m_c4RetryCnt++;
            return;
        }
    }

    // Let's call the start method to try to start a new guy up
    StartHelper();

    // If we didn't get it created, bump the retry count and return
    if (!m_porbcBrowser)
    {
        m_c4RetryCnt++;
        return;
    }

    // Try to display it now. If that fails, it will return and return false
    if (!bShowHelper())
    {
        // Assume a bad failure and reset
        Reset();
        m_c4RetryCnt++;
        return;
    }

    // And now we can reset the retry counter since we got reconnected successfully
    m_c4RetryCnt = 0;
}
