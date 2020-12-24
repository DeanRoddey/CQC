//
// FILE NAME: CQCIntfEng_WebCamera.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/07/2017
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
//  This file implements the class that creates and manages an external video display
//  helper process.
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


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfWebCamera,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_WebCamera
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Format versions
        //
        //  Version 2 -
        //      Add new mute and volume attributes
        //
        //  Version 3 -
        //      Added a new web URL as an option for the WebRIVA client.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 3;


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
//   CLASS: TCQCIntfWebCamera
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

//
//  We allow for a set of parameters which must be a quoted comma list of key/value
//  pairs. We have to validate these in a few places and it's way too much to want
//  to replicate. Also the editor needs to validate the entered attribute value as
//  well, so we need it publically available.
//
tCIDLib::TBoolean
TCQCIntfWebCamera::bValidateParms(  const   TString&            strToVal
                                    ,       TString&            strErr
                                    ,       tCIDLib::TKVPList&  colToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    tCIDLib::TStrList colVals;
    tCIDLib::TCard4 c4ErrInd;
    bRet = TStringTokenizer::bParseQuotedCommaList(strToVal, colVals, c4ErrInd);
    if (bRet)
    {
        colToFill.RemoveAll();
        TString strVal;
        colVals.bForEachNC
        (
            [&strErr, &strVal, &bRet, &colToFill](TString& strCur)
            {
                if (!strCur.bSplit(strVal, kCIDLib::chEquals))
                {
                    bRet = kCIDLib::False;
                }
                 else
                {
                    // The key can't be empty, the value can
                    strCur.StripWhitespace();
                    bRet = !strCur.bIsEmpty();
                }

                if (bRet)
                {
                    strVal.StripWhitespace();
                    colToFill.objAdd(TKeyValuePair(strCur, strVal));
                }
                return bRet;
            }
        );
    }

    if (!bRet)
        strErr.LoadFromMsg(kIEngErrs::errcVal_BadCamParams, facCQCIntfEng());
    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebCamera: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfWebCamera::TCQCIntfWebCamera() :

    TCQCIntfWidget
    (
        CQCIntfEng_WebCamera::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_WebCamera)
    )
    , m_bMute(kCIDLib::True)
    , m_c4RetryCnt(0)
    , m_c4Volume(100)
    , m_enctNextPing(0)
    , m_enctRestart(0)
    , m_hwndHelper(kCIDCtrls::hwndInvalid)
    , m_porbcCamera(nullptr)
{
    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/WebCamera");
}

//
//  The state always goes back to init and the proxy pointer to null, and no need to
//  copy the time stamps.
//
TCQCIntfWebCamera::TCQCIntfWebCamera(const TCQCIntfWebCamera& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , m_bMute(iwdgSrc.m_bMute)
    , m_c4RetryCnt(0)
    , m_c4Volume(iwdgSrc.m_c4Volume)
    , m_enctNextPing(0)
    , m_enctRestart(0)
    , m_hwndHelper(kCIDCtrls::hwndInvalid)
    , m_porbcCamera(nullptr)
    , m_strParams(iwdgSrc.m_strParams)
    , m_strURL(iwdgSrc.m_strURL)
    , m_strWebURL(iwdgSrc.m_strWebURL)
{
}

TCQCIntfWebCamera::~TCQCIntfWebCamera()
{
    // Should already be done, but just in case
    Terminate();
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebCamera: Public operators
// ---------------------------------------------------------------------------

//
//  The browser window is NOT copied. It's purely a runtime attribute, as are
//  the coutner and next refresh time.
//
TCQCIntfWebCamera& TCQCIntfWebCamera::operator=(const TCQCIntfWebCamera& iwdgSrc)
{
    CIDAssert(m_porbcCamera == nullptr, L"The web camera window has already been created");

    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);

        m_bMute     = iwdgSrc.m_bMute;
        m_c4Volume  = iwdgSrc.m_c4Volume;
        m_strParams = iwdgSrc.m_strParams;
        m_strURL    = iwdgSrc.m_strURL;
        m_strWebURL = iwdgSrc.m_strWebURL;

        // These aren't copied, just reset
        if (m_porbcCamera)
            delete m_porbcCamera;
        m_porbcCamera = nullptr;
        m_c4RetryCnt = 0;
        m_hwndHelper = kCIDCtrls::hwndInvalid;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebCamera: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfWebCamera::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    // Do our own stuff, only the persistent bits
    const TCQCIntfWebCamera& iwdgOther(static_cast<const TCQCIntfWebCamera&>(iwdgSrc));
    return
    (
        (m_bMute == iwdgOther.m_bMute)
        && (m_c4Volume == iwdgOther.m_c4Volume)
        && (m_strParams == iwdgOther.m_strParams)
        && (m_strURL == iwdgOther.m_strURL)
        && (m_strWebURL == iwdgOther.m_strWebURL)
    );
}


tCIDLib::TBoolean
TCQCIntfWebCamera::bValidateParm(const  TCQCCmd&        cmdSrc
                                , const TCQCCmdCfg&     ccfgTar
                                , const tCIDLib::TCard4 c4Index
                                , const TString&        strValue
                                ,       TString&        strErrText)
{
    // Let our parent go first
    if (!TParent::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    if ((ccfgTar.strCmdId() == kCQCKit::strCmdId_SetURL)
    ||  ((ccfgTar.strCmdId() == kCQCKit::strCmdId_SetWebURL) && !strValue.bIsEmpty()))
    {
        // If it's the web URL it can be empty!
        if (c4Index == 0)
        {
            try
            {
                // Try to parse the URL. It has to be fully qualified
                TURL urlImg(strValue, tCIDSock::EQualified::Full);
            }

            catch(const TError& errToCatch)
            {
                strErrText = errToCatch.strErrText();
                return kCIDLib::False;
            }
        }
         else if (c4Index == 1)
        {
            tCIDLib::TKVPList colParms;
            if (!bValidateParms(strValue, strErrText, colParms))
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}

tCIDLib::TVoid TCQCIntfWebCamera::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfWebCamera&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfWebCamera::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    try
    {
        if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetMute)
        {
            if (TFacCQCIntfEng::bRemoteMode())
            {
                if (TFacCQCIntfEng::bWebRIVAMode())
                {
                }
            }
             else
            {
                //
                //  Store the last mute state set, so that we can send it again
                //  if we have to reconnect.
                //
                m_bMute = ccfgToDo.piAt(0).m_strValue.bCompareI(kCQCKit::pszFld_True);

                // If we have a camera object proxy, send this to it
                if (m_porbcCamera)
                {
                    try
                    {
                        m_porbcCamera->SetMute(m_bMute);
                    }

                    catch(TError& errToCatch)
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        throw;
                    }
                }
            }
        }
         else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetURL)
        {
            // Update our parameters and URL from the incoming action
            m_strParams = ccfgToDo.piAt(1).m_strValue;
            m_strURL = ccfgToDo.piAt(0).m_strValue;

            //
            //  If in preload, then we just store the info for later use, else let's
            //  process it now.
            //
            if (strEventId != kCQCKit::strEvId_OnPreload)
            {
                try
                {
                    // In native mode, destroy our current helper
                    Reset();

                    // And now start it up again
                    StartHelper();

                    // If we created it, try to show it. If we fail, reset again
                    if (m_porbcCamera)
                    {
                        if (!bShowHelper())
                            Reset();
                    }
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }
            }

            //
            //  If in WebRIVA mode, we just need to send him a new URL to load,
            //  but we only send this one if there's no web URL. We send this
            //  whether in onpreload or not, since he does the creating of the
            //  display element and such, not us.
            //
            if (TFacCQCIntfEng::bRemoteMode())
            {
                if (TFacCQCIntfEng::bWebRIVAMode() && m_strWebURL.bIsEmpty())
                {
                    TString strErr;
                    tCIDLib::TKVPList colParams;
                    if (bValidateParms(m_strParams, strErr, colParams))
                    {
                        try
                        {
                            civOwner().miahOwner().SetRemWidgetURL
                            (
                                m_strUID
                                , tCQCIntfEng::ERIVAWTypes::WebCamera
                                , m_strURL
                                , colParams
                            );
                        }
                        catch(TError& errToCatch)
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            throw;
                        }
                    }
                     else
                    {
                        facCQCIntfEng().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kIEngErrs::errcWCam_BadParams
                            , strErr
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Format
                        );
                    }
                }
            }
        }
         else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetWebURL)
        {
            m_strWebURL = ccfgToDo.piAt(0).m_strValue;

            //
            //  If in WebRIVA mode, and this is not the onpreload, then we will now
            //  invoke this URL. If in preload we just store it for the later load.
            //
            if ((strEventId != kCQCKit::strEvId_OnPreload)
            &&  TFacCQCIntfEng::bRemoteMode()
            &&  TFacCQCIntfEng::bWebRIVAMode())
            {
                // Parse out the parameters to a list
                TString strErr;
                tCIDLib::TKVPList colParams;
                bValidateParms(m_strParams, strErr, colParams);

                civOwner().miahOwner().SetRemWidgetURL
                (
                    m_strUID
                    , tCQCIntfEng::ERIVAWTypes::WebCamera
                    , m_strWebURL
                    , colParams
                );
            }
        }
         else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetVolume)
        {
            try
            {
                if (TFacCQCIntfEng::bRemoteMode())
                {
                    if (TFacCQCIntfEng::bWebRIVAMode())
                    {
                    }
                }
                 else
                {
                    //
                    //  Store the last volume set, so that we can send it again if we have to
                    //  reconnect.
                    //
                    m_c4Volume = ccfgToDo.piAt(0).m_strValue.c4Val();

                    // If we have a camera object proxy, send this to it
                    if (m_porbcCamera)
                        m_porbcCamera->SetVolume(m_c4Volume);
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                throw;
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


tCIDLib::TVoid TCQCIntfWebCamera::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Try to start up the external helper
    StartHelper();
}


tCIDLib::TVoid TCQCIntfWebCamera::PostInit()
{
    if (!TFacCQCIntfEng::bRemoteMode())
    {
        // If we've not started the helper yet try that
        if (!m_porbcCamera)
            StartHelper();

        //
        //  If we managed to create it, then let's make it visible. If we can't
        //  then we'll continue trying later.
        //
        if (m_porbcCamera)
        {
            if (!bShowHelper())
                Reset();
        }
    }
}


tCIDLib::TVoid
TCQCIntfWebCamera::QueryCmds(          TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Only allow this in normal context or preload
    if ((eContext == tCQCKit::EActCmdCtx::Standard)
    ||  (eContext == tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd& cmdSetURL = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetURL
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetURL)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_URL)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Parms)
            )
        );

        // The parameters parameter can be empty
        cmdSetURL.cmdpAt(1).bRequired(kCIDLib::False);
    }

    if ((eContext == tCQCKit::EActCmdCtx::Standard)
    ||  (eContext == tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd& cmdSetWebURL = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetWebURL
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetWebURL)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_URL)
            )
        );

        // This one is optional
        cmdSetWebURL.cmdpAt(0).bRequired(kCIDLib::False);
    }

    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetMute
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetMute)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCKit().strMsg(kKitMsgs::midCmdP_MuteState)
        )
    );

    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetVolume
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetVolume)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Volume)
        )
    );
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfWebCamera::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove any base attributes we don't honor, which is most of them
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Border);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Rounding);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Shadow);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Transparent);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Do our attributes
    colAttrs.objPlace
    (
        L"Camera Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    colAttrs.objPlace
    (
        L"URL"
        , kCQCIntfEng::strAttr_WebCam_URL
        , tCIDMData::EAttrTypes::String
    ).SetString(m_strURL);

    colAttrs.objPlace
    (
        L"WebURL"
        , kCQCIntfEng::strAttr_WebCam_WebURL
        , tCIDMData::EAttrTypes::String
    ).SetString(m_strWebURL);

    colAttrs.objPlace
    (
        L"Params"
        , kCQCIntfEng::strAttr_WebCam_Params
        , tCIDMData::EAttrTypes::String
    ).SetString(m_strParams);

    colAttrs.objPlace
    (
        L"Init Mute"
        , kCQCIntfEng::strAttr_WebCam_InitMute
        , tCIDMData::EAttrTypes::Bool
    ).SetBool(m_bMute);

    colAttrs.objPlace
    (
        L"Init Volume"
        , kCQCIntfEng::strAttr_WebCam_InitVolume
        , L"Range: 0, 100"
        , tCIDMData::EAttrTypes::Card
    ).SetCard(m_c4Volume);
}


tCIDLib::TVoid
TCQCIntfWebCamera::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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
            strSrc, strTar, m_strURL, bRegEx, bFull, regxFind
        );

        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strWebURL, bRegEx, bFull, regxFind
        );
    }
}


TSize TCQCIntfWebCamera::szDefaultSize() const
{
    return TSize(256, 256);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfWebCamera::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_WebCam_InitMute)
        m_bMute = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebCam_InitVolume)
        m_c4Volume = adatNew.c4Val();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebCam_Params)
        m_strParams = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebCam_URL)
        m_strURL = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebCam_WebURL)
        m_strWebURL = adatNew.strValue();
}


//
//  We override this so that we can clean up the resources created to display the
//  camera content.
//
tCIDLib::TVoid TCQCIntfWebCamera::Terminate()
{
    //
    //  In native mode, we reset our connection to the external helper. In WebRIVA
    //  mode we need to send a command to the client to destroy whatever he created
    //  to display this content.
    //
    if (TFacCQCIntfEng::bRemoteMode())
    {
        if (TFacCQCIntfEng::bWebRIVAMode())
        {
            //
            //  Send a message to destroy whatever the client created. There may be
            //  no owning view if this is just some termporary template object or
            //  it never got loaded.
            //
            TCQCIntfView* pcivTest = pcivOwner();
            if (pcivTest)
            {
                pcivTest->miahOwner().DestroyRemWidget
                (
                    m_strUID, tCQCIntfEng::ERIVAWTypes::WebCamera
                );
            }
        }
    }
     else
    {
        Reset();
    }
}


tCIDLib::TVoid
TCQCIntfWebCamera::Validate(const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Validate the parameter string
    TString strErr;
    tCIDLib::TKVPList colParms;
    if (!bValidateParms(m_strParams, strErr, colParms))
        colErrs.objAdd(TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::True, strWidgetId()));
}




// ---------------------------------------------------------------------------
//  TCQCIntfWebCamera: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We have to ping our helper process every so often if we've not sent him a message,
//  else he'll close down.
//
tCIDLib::TVoid
TCQCIntfWebCamera::ActiveUpdate(const   tCIDLib::TBoolean   bNoRedraw
                                , const TGUIRegion&         grgnClip
                                , const tCIDLib::TBoolean   bInTopLayer)
{
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if (enctCur > m_enctNextPing)
    {
        // Update the time first
        m_enctNextPing = enctCur + CQCIntfEng_WebCamera::enctPingInt;

        if (!TFacCQCIntfEng::bRemoteMode())
        {
            //
            //  If we don't have the browser object, then let's try to recover if we have
            //  not already used up all of our lives.
            //
            if (!m_porbcCamera && (m_c4RetryCnt < CQCIntfEng_WebCamera::c4MaxReconnect))
                TryReconnect();

            //
            //  If we have the browser, let's try to ping him or to recycle him if we have
            //  hit the shutdown time.
            //
            if (m_porbcCamera)
            {
                try
                {
                    if (enctCur >= m_enctRestart)
                    {
                        //
                        //  Set the restart foward five minutes. If we fail, we'll try again, but
                        //  not too quickly. If it works, the reset time will get set to the
                        //  real time when we start it back up.
                        //
                        m_enctRestart = TTime::enctNowPlusMins(5);
                        Reset();
                        StartHelper();
                    }
                     else
                    {
                        m_porbcCamera->Ping();
                    }
                }

                catch(TError& errToCatch)
                {
                    if (facCQCIntfEng().bLogWarnings())
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    // If a connection loss, reset so we start retrying
                    if (m_porbcCamera->bCheckForLostConnection(errToCatch))
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
            }
        }
    }
}


//
//  This can happen at viewing time, when the size of the window is changed, which in turn
//  will change our widget's origin.
//
tCIDLib::TVoid
TCQCIntfWebCamera::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    TParent::AreaChanged(areaNew, areaOld);

    if (TFacCQCIntfEng::bRemoteMode())
    {
        if (TFacCQCIntfEng::bWebRIVAMode())
        {
            //
            //  Send an updated position to the client. Calculate our position relative
            //  to the template area, and send that.
            //
            TArea areaCam(areaActual());
            areaCam -= civOwner().iwdgBaseTmpl().areaActual().pntOrg();


        }
    }
     else
    {
        // If the browser was created, then update it's size/pos
        if (m_porbcCamera)
        {
            try
            {
                // Get our parent view window's position in screen coordinates
                TArea areaPar;
                civOwner().wndOwner().ToScreenCoordinates(civOwner().areaView(), areaPar);

                // Add our area to the screen area, since ours is relative to the window
                TArea areaNewCam(areaActual());
                areaNewCam += areaPar.pntOrg();

                // And send the new areas to the browser helper
                m_porbcCamera->SetArea(areaPar, areaNewCam);

                // It worked so we can bump our next ping time
                m_enctNextPing = TTime::enctNow() + CQCIntfEng_WebCamera::enctPingInt;
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                // If a connection loss, reset so we start retrying
                if (m_porbcCamera->bCheckForLostConnection(errToCatch))
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
//  Since we manage a real window, we have to react to display state changes
//  and hide/show the window.
//
tCIDLib::TVoid TCQCIntfWebCamera::DisplayStateChanged()
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
        if (m_porbcCamera)
        {
            try
            {
                m_porbcCamera->SetVisibility(eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                // If a connection loss, reset so we start retrying
                if (m_porbcCamera->bCheckForLostConnection(errToCatch))
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
//  In our case, if our parent id hidden, we need to hid our camera window since
//  it's not affected by the interface level visibility.
//
tCIDLib::TVoid
TCQCIntfWebCamera::ParentDisplayStateChanged(const tCQCIntfEng::EDispStates eState)
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
        if (m_porbcCamera)
        {
            try
            {
                m_porbcCamera->SetVisibility(eState != tCQCIntfEng::EDispStates::Hidden);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                // If a connection loss, reset so we start retrying
                if (m_porbcCamera->bCheckForLostConnection(errToCatch))
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
tCIDLib::TVoid TCQCIntfWebCamera::ScrPosChanged(const TArea& areaTmplWnd)
{
    if (TFacCQCIntfEng::bRemoteMode())
    {
        if (TFacCQCIntfEng::bWebRIVAMode())
        {
        }
    }
     else
    {
        // If the browser was created, then update it's size/pos
        if (m_porbcCamera)
        {
            try
            {
                // Add our area to the template window area, which is already in screen coordinates
                TArea areaNew(areaActual());
                areaNew += areaTmplWnd.pntOrg();

                m_porbcCamera->SetArea(areaTmplWnd, areaNew);

                // It worked so we can bump our next ping time
                m_enctNextPing = TTime::enctNow() + CQCIntfEng_WebCamera::enctPingInt;
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                // If a connection loss, reset so we start retrying
                if (m_porbcCamera->bCheckForLostConnection(errToCatch))
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


tCIDLib::TVoid TCQCIntfWebCamera::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_WebCamera::c2FmtVersion))
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
    strmToReadFrom  >> m_strURL
                    >> m_strParams;

    // Do V2 stuff
    if (c2FmtVersion > 1)
    {
        strmToReadFrom >> m_bMute >> m_c4Volume;
    }
     else
    {
        m_bMute = kCIDLib::True;
        m_c4Volume = 100;
    }

    // Do V3 stuff
    if (c2FmtVersion > 2)
        strmToReadFrom >> m_strWebURL;
    else
        m_strWebURL.Clear();

    // There shouldn't be an existing proxy, but destroy it if so
    if (m_porbcCamera)
    {
        delete m_porbcCamera;
        m_porbcCamera = nullptr;
    }

    // And it should all end with an endmarker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfWebCamera::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_WebCamera::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // And stream out our stuff and our end marker
    strmToWriteTo   << m_strURL
                    << m_strParams

                    // V2 stuff
                    << m_bMute
                    << m_c4Volume

                    // V3 stuff
                    << m_strWebURL

                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfWebCamera::Update(         TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    //
    //  If in designer mode, draw a rectangle to show where we would go at viewing time.
    //  Else, just fill the background.
    //
    if (TFacCQCIntfEng::bDesMode() && bIsTransparent())
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


// ---------------------------------------------------------------------------
//  TCQCIntfWebCamera: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Once the helper is created, we can show it. This is split out because, in the initial
//  scenario (i.e. not recovering from a failure of the helper but on initial creation) we
//  do this separately in PostInit.
//
tCIDLib::TBoolean TCQCIntfWebCamera::bShowHelper()
{
    // Only do this in viewing mode and if not in remote viewer mode.
    if (TFacCQCIntfEng::bDesMode() || TFacCQCIntfEng::bRemoteMode())
        return kCIDLib::True;

    // If we didn't manage to create the external helper, then nothing to do
    if (!m_porbcCamera)
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
        m_porbcCamera->Initialize
        (
            areaPar, areaUs, eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden
        );

        // It worked so we can bump our next ping time
        m_enctNextPing = TTime::enctNow() + CQCIntfEng_WebCamera::enctPingInt;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCIntfEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcWCam_ShowFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strWidgetId()
        );
        return kCIDLib::False;
    }

    return kCIDLib::True;
}



//
//  This is used in native mode to reset our connection to the external program.
//  This isn't used in WebRIVA mode.
//
tCIDLib::TVoid TCQCIntfWebCamera::Reset()
{
    if (m_porbcCamera)
    {
        try
        {
            m_porbcCamera->Close();
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
            delete m_porbcCamera;
        }

        catch(...)
        {
        }

        m_porbcCamera = nullptr;
    }
}


//
//  This is called to start up the web camera support. If in riva mode, it sends a command
//  to the WebRIVA client to create the widget. Else, it starts up the external process
//  that hosts the web camera for the regular IV.
//
tCIDLib::TVoid TCQCIntfWebCamera::StartHelper()
{
    // We don't do anything in designer mode
    if (TFacCQCIntfEng::bDesMode())
        return;

    //
    //  Generate an id that we'll pass to the external program or remote viewer. This
    //  is our connection point to create the shared resources we need to get ourselves
    //  set up.
    //
    m_strUID = L"WebCameraDat";
    m_strUID.AppendFormatted(facCIDLib().c4NextId());


    //
    //  If in remote mode, we need to send a message to the client to create an
    //  appropriate video element. For now, we have to deal with the fact that we
    //  still have the old RIVA clients around, and they don't understand this, so
    //  only send it if it's the newer WebRIVA client.
    //
    if (TFacCQCIntfEng::bRemoteMode())
    {
        if (TFacCQCIntfEng::bWebRIVAMode())
        {
            //
            //  Calculate the area of the web cam widget relative to the overall
            //  template.
            //
            TArea areaCam(areaActual());
            areaCam -= civOwner().iwdgBaseTmpl().areaActual().pntOrg();

            // Parse out the parameters to a list
            TString strErr;
            tCIDLib::TKVPList colParams;
            bValidateParms(m_strParams, strErr, colParams);

            civOwner().miahOwner().CreateRemWidget
            (
                m_strUID
                , tCQCIntfEng::ERIVAWTypes::WebCamera
                , areaCam
                , colParams
                , eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden
            );

            // And, if we have a URL, send that
            if (!m_strWebURL.bIsEmpty() || !m_strURL.bIsEmpty())
            {
                civOwner().miahOwner().SetRemWidgetURL
                (
                    m_strUID
                    , tCQCIntfEng::ERIVAWTypes::WebCamera
                    , m_strURL
                    , colParams
                );
            }
        }
    }
     else
    {
        //
        //  Update the reset timer. Even if we don't manage to get it started we go
        //  and reset it. If we do get it started, this insures that the restart timer
        //  gets set correctly.
        //
        m_enctRestart = TTime::enctNowPlusHours(2);

        //
        //  We want to get the external helper process started. It will be invisible
        //  until it is shown.
        //
        try
        {
            //
            //  Create a shared event, in reset state. The helper will trigger it once he
            //  has the data set up that we need.
            //
            tCIDLib::TBoolean bCreated;
            TResourceName rsnBuf(kCIDLib::pszResCompany, L"CQCWCHelper", m_strUID);
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
            //  Format out the data. We leave a Card4 blank at the start and go back
            //  and fill it in with the size.
            //
            tCIDLib::TCard4 c4Size = 0;
            {
                TBinMBufOutStream strmOut(&mbufInfo);
                strmOut << tCIDLib::TCard4(0)
                        << tCIDLib::EStreamMarkers::StartObject
                        << m_strURL
                        << tCIDLib::TCard4(civOwner().wndOwner().hwndThis())
                        << m_strParams
                        << tCIDLib::TBoolean(facCQCIntfEng().eLogThreshold() <= tCIDLib::ESeverities::Warn)
                        << m_bMute
                        << m_c4Volume
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
            pathCmdLine.AddLevel(L"CQCWCHelper");
            pathCmdLine.AppendExt(L"exe");


            // Set up the parameters
            tCIDLib::TStrList colParms(4);
            colParms.objAdd(m_strUID);

            //
            //  These are standard ones that get removed by the core CIDLib stuff. We
            //  have to tell him the name server address, even though he doesn't use
            //  it. And the data directory.
            //
            TString strNSAddr(L"/NSAddr=");
            strNSAddr.Append(TSysInfo::strNSAddr());
            colParms.objAdd(strNSAddr);

            TString strDataDir(L"/DataDir=");
            strDataDir.Append(facCQCKit().strDataDir());
            strDataDir.Append(L"/");
            colParms.objAdd(strDataDir);

            #if 0
            colParms.objAdd(L"/GlobalLogMode=Warn");
            #endif


            // For debugging, invoke the helper in the debugger
            #if 0
                pathCmdLine = L"devenv.exe /debugexe CQCWCHelper.exe ";
                pathCmdLine.Append(m_strUID);
                m_extpHelper.SystemEscape(pathCmdLine);
            #else
                m_extpHelper.Start
                (
                    pathCmdLine
                    , TString::strEmpty()
                    , colParms
                    , tCIDLib::EExtProcFlags::None
                    , tCIDLib::EExtProcShows::Hidden
                );
            #endif

            //
            //  Now we wait for him to signal us he has filled in the memory buffer
            //  which the info we need.
            //
            #if CID_DEBUG_ON
            evDataReady.WaitFor(60000);
            #else
            evDataReady.WaitFor(5000);
            #endif

            TOrbObjId ooidServer;
            {
                // The first card4 is the size of data to stream out
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
            //  And let's try to create our proxy now. Tell it no name server binding,
            //  since we aren't using the name server.
            //
            m_porbcCamera = new TCQCWCHelperClientProxy(ooidServer, TString::strEmpty());
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcWCam_StartFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strWidgetId()
            );
        }
    }
}


//
//  If we have lost connection, or never managed to get it, then this is called to try
//  get us going again. It'll only be called so many times then we just stay offline.
//
tCIDLib::TVoid TCQCIntfWebCamera::TryReconnect()
{
    facCQCIntfEng().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kIEngErrs::errcWCam_Retrying
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
                , kIEngErrs::errcWCam_StopFailed
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
    if (!m_porbcCamera)
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
