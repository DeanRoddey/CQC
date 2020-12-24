//
// FILE NAME: CQCIntfEng_WebImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2009
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
//  This file implements a web image widget. It just displays an image
//  that is downloaded from a web server.
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
#include    "CQCIntfEng_WebImage.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfWebImg,TCQCIntfWidget)

RTTIDecls(TCQCIntfWebImgBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldWebImg,TCQCIntfWebImgBase)
AdvRTTIDecls(TCQCIntfStaticWebImg,TCQCIntfWebImgBase)
AdvRTTIDecls(TCQCIntfVarWebImg,TCQCIntfWebImgBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_WebImage
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Persistent format version for the old class that has now been
        //  refactored away. We keep it around just to be able to read in already
        //  persisted olf objects.
        //
        //  Version 2 -
        //      Added the 'no cache' option
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2OldFmtVersion = 2;



        // -----------------------------------------------------------------------
        //  Base class persistent format version
        //
        //  Version 2 -
        //      We added the refresh milliseconds to support auto-refresh.
        //
        //  Version 3 - 4.8.1
        //      Added support for 1 or 2 header lines to be configured, which will be
        //      sent along in the HTTP query.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion = 3;


        // -----------------------------------------------------------------------
        //  Field web image persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FldFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  Static web image persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2StaticFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  Variable web image persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2VarFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eStaticCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eFldCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );


        // -----------------------------------------------------------------------
        //  What we pass as the agent in the HTTP request
        // -----------------------------------------------------------------------
        const TString strAgent(L"CQSL Web Image Widget");
    }
}



// ---------------------------------------------------------------------------
//  REFACTORING CODE
//
//  Streaming in code for the old original class that was refactored away.
//  We need this to read in old objects already stored.
// ---------------------------------------------------------------------------
TCQCIntfWebImg::TCQCIntfWebImg() :

    TCQCIntfWidget(CQCIntfEng_WebImage::eStaticCapFlags, L"OldWebImageWidget")
{
}

TCQCIntfWebImg::~TCQCIntfWebImg()
{
}

TSize TCQCIntfWebImg::szDefaultSize() const
{
    return TSize(0,0);
}

tCIDLib::TBoolean TCQCIntfWebImg::bIsSame(const TCQCIntfWidget&) const
{
    return kCIDLib::False;
}

tCIDLib::TVoid
TCQCIntfWebImg::Validate(const TCQCFldCache&, tCQCIntfEng::TErrList&, TDataSrvClient&) const
{
}

tCIDLib::TVoid TCQCIntfWebImg::Update(TGraphDrawDev&, const TArea&, TGUIRegion&)
{
}

tCIDLib::TVoid TCQCIntfWebImg::CopyFrom(const TCQCIntfWidget&)
{
}

tCIDLib::TVoid TCQCIntfWebImg::StreamFrom(TBinInStream& strmToReadFrom)
{
   strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_WebImage::c2OldFmtVersion))
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

    // Stream our stuff
    strmToReadFrom  >> m_c1ConstOpacity
                    >> m_ePlacement
                    >> m_strURL;

    //
    //  If beyond V1, then read in the no cache option, else default it
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_bNoCache;
    else
        m_bNoCache = kCIDLib::False;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWebImgBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfWebImgBase: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfWebImgBase::
TCQCIntfWebImgBase( const   tCQCIntfEng::ECapFlags  eCaps
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCaps, strTypeName)
    , m_bNewImage(kCIDLib::False)
    , m_bNoCache(kCIDLib::True)
    , m_c1ConstOpacity(0xFF)
    , m_c4NewSerialNum(0)
    , m_c4RefreshMSs(0)
    , m_ePlacement(tCIDGraphDev::EPlacement::Center)
    , m_thrDownloader
      (
          facCIDLib().strNextThreadName(L"CQCWebImgDL")
          , TMemberFunc<TCQCIntfWebImgBase>(this, &TCQCIntfWebImgBase::eDownloader)
      )
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until one is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/WebImage");
}

TCQCIntfWebImgBase::TCQCIntfWebImgBase(const TCQCIntfWebImgBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , m_bNewImage(kCIDLib::False)
    , m_bNoCache(iwdgSrc.m_bNoCache)
    , m_c1ConstOpacity(iwdgSrc.m_c1ConstOpacity)
    , m_c4NewSerialNum(0)
    , m_c4RefreshMSs(iwdgSrc.m_c4RefreshMSs)
    , m_ePlacement(iwdgSrc.m_ePlacement)
    , m_kvalHdr1(iwdgSrc.m_kvalHdr1)
    , m_kvalHdr2(iwdgSrc.m_kvalHdr2)
    , m_strLiveURL(iwdgSrc.m_strLiveURL)
    , m_thrDownloader
      (
          facCIDLib().strNextThreadName(L"CQCWebImgDL")
          , TMemberFunc<TCQCIntfWebImgBase>(this, &TCQCIntfWebImgBase::eDownloader)
      )
{
    // We don't copy over any images or thread or stamp
}

TCQCIntfWebImgBase::~TCQCIntfWebImgBase()
{
    // Stop the thread
    bStopDownload();
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebImgBase: Public operators
// ---------------------------------------------------------------------------
TCQCIntfWebImgBase&
TCQCIntfWebImgBase::operator=(const TCQCIntfWebImgBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);

        m_bNoCache       = iwdgSrc.m_bNoCache;
        m_c1ConstOpacity = iwdgSrc.m_c1ConstOpacity;
        m_c4RefreshMSs   = iwdgSrc.m_c4RefreshMSs;
        m_ePlacement     = iwdgSrc.m_ePlacement;
        m_kvalHdr1       = iwdgSrc.m_kvalHdr1;
        m_kvalHdr2       = iwdgSrc.m_kvalHdr2;
        m_strLiveURL     = iwdgSrc.m_strLiveURL;

        // We don't copy over any image related stuff
        m_bNewImage         = kCIDLib::False;
        m_c4NewSerialNum    = 0;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebImgBase: Public, inherited methods
// ---------------------------------------------------------------------------

// We cannot size to contents since our contents is runtime only
tCIDLib::TBoolean TCQCIntfWebImgBase::bCanSizeTo() const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCIntfWebImgBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfWebImgBase& iwdgOther(static_cast<const TCQCIntfWebImgBase&>(iwdgSrc));

    // Do our own stuff
    return
    (
        (m_bNoCache == iwdgOther.m_bNoCache)
        && (m_c1ConstOpacity == iwdgOther.m_c1ConstOpacity)
        && (m_c4RefreshMSs == iwdgOther.m_c4RefreshMSs)
    );
}


// Nothing to do at this level for now, so just pass it on
tCIDLib::TBoolean
TCQCIntfWebImgBase::bValidateParm(  const   TCQCCmd&        cmdSrc
                                    , const TCQCCmdCfg&     ccfgTar
                                    , const tCIDLib::TCard4 c4Index
                                    , const TString&        strValue
                                    ,       TString&        strErrText)
{
    return TParent::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText);
}


tCIDLib::TVoid TCQCIntfWebImgBase::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfWebImgBase&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfWebImgBase::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
    {
        m_c1ConstOpacity = tCIDLib::TCard1(ccfgToDo.piAt(0).m_strValue.c4Val());
        if (bCanRedraw(strEventId))
            Redraw();
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
    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid
TCQCIntfWebImgBase::Initialize( TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  If we are in RIVA mode, and auto-refresh is enabled, then limit it to
    //  no more than once every 5 seconds.
    //
    if (TFacCQCIntfEng::bRemoteMode() && m_c4RefreshMSs)
    {
        if (m_c4RefreshMSs < 8000)
            m_c4RefreshMSs = 8000;
    }
}


// Start up the downloader thread once we are set up
tCIDLib::TVoid TCQCIntfWebImgBase::PostInit()
{
    m_thrDownloader.Start();
}


tCIDLib::TVoid
TCQCIntfWebImgBase::QueryCmds(          TCollection<TCQCCmd>&   colCmds
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // At this level we provide a little image display control
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetOpacity
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetOpacity)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_Opacity)
        )
    );
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfWebImgBase::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove those common ones that are not applicable to us
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);

    // Set any kept color names that aren't already defaulted correctly
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    colAttrs.objPlace
    (
            L"Web Image Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    colAttrs.objPlace
    (
        L"Refresh MSs (0=Off)"
        , kCQCIntfEng::strAttr_WebImg_RefreshMSs
        , L"Range: 0, 1000000"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetCard(m_c4RefreshMSs);

    colAttrs.objPlace
    (
        L"Opacity"
        , kCQCIntfEng::strAttr_WebImg_Opacity
        , L"Range: 0, 255"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetCard(tCIDLib::TCard4(m_c1ConstOpacity));

    colAttrs.objPlace
    (
        L"No Cache"
        , kCQCIntfEng::strAttr_WebImg_NoCache
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetBool(m_bNoCache);

    TString strLim;
    tCIDGraphDev::FormatEPlacement
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    colAttrs.objPlace
    (
        L"Placement"
        , kCQCIntfEng::strAttr_WebImg_Placement
        , strLim
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(tCIDGraphDev::strXlatEPlacement(m_ePlacement));

    colAttrs.objPlace
    (
        L"HTTP Headers:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    colAttrs.objPlace
    (
        L"Hdr Key 1"
        , kCQCIntfEng::strAttr_WebImg_HdrKey1
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_kvalHdr1.strKey());

    colAttrs.objPlace
    (
        L"Hdr Value 1"
        , kCQCIntfEng::strAttr_WebImg_HdrVal1
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_kvalHdr1.strValue());

    colAttrs.objPlace
    (
        L"Hdr Key 2"
        , kCQCIntfEng::strAttr_WebImg_HdrKey2
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_kvalHdr2.strKey());

    colAttrs.objPlace
    (
        L"Hdr Value 2"
        , kCQCIntfEng::strAttr_WebImg_HdrVal2
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_kvalHdr2.strValue());
}


tCIDLib::TVoid
TCQCIntfWebImgBase::Replace(const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
}



TSize TCQCIntfWebImgBase::szDefaultSize() const
{
    return TSize(240, 192);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfWebImgBase::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Handle our attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_WebImg_NoCache)
        m_bNoCache = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebImg_Opacity)
        m_c1ConstOpacity = tCIDLib::TCard1(adatNew.c4Val());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebImg_Placement)
        m_ePlacement = tCIDGraphDev::eXlatEPlacement(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebImg_RefreshMSs)
        m_c4RefreshMSs = adatNew.c4Val();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebImg_HdrKey1)
        m_kvalHdr1.strKey(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebImg_HdrKey2)
        m_kvalHdr2.strKey(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebImg_HdrVal1)
        m_kvalHdr1.strValue(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_WebImg_HdrVal2)
        m_kvalHdr2.strValue(adatNew.strValue());
}


// Set up default parameter values for some commands
tCIDLib::TVoid TCQCIntfWebImgBase::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    TParent::SetDefParms(ccfgToSet);

    if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
        ccfgToSet.piAt(0).m_strValue = L"255";
}


tCIDLib::TVoid
TCQCIntfWebImgBase::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebImgBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the no cache option
tCIDLib::TBoolean TCQCIntfWebImgBase::bNoCache() const
{
    return m_bNoCache;
}

tCIDLib::TBoolean TCQCIntfWebImgBase::bNoCache(const tCIDLib::TBoolean bNoCache)
{
    m_bNoCache = bNoCache;
    return m_bNoCache;
}


// Get/set the constant opacity
tCIDLib::TCard1 TCQCIntfWebImgBase::c1ConstOpacity() const
{
    return m_c1ConstOpacity;
}

tCIDLib::TCard1 TCQCIntfWebImgBase::c1ConstOpacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1ConstOpacity = c1ToSet;
    return m_c1ConstOpacity;
}


// Get/set the refresh millisecs
tCIDLib::TCard4 TCQCIntfWebImgBase::c4RefreshMSs() const
{
    return m_c4RefreshMSs;
}

tCIDLib::TCard4 TCQCIntfWebImgBase::c4RefreshMSs(const tCIDLib::TCard4 c4ToSet)
{
    m_c4RefreshMSs = c4ToSet;
    return m_c4RefreshMSs;
}


// Get/set the image placement setting
tCIDGraphDev::EPlacement TCQCIntfWebImgBase::ePlacement() const
{
    return m_ePlacement;
}

tCIDGraphDev::EPlacement TCQCIntfWebImgBase::ePlacement(const tCIDGraphDev::EPlacement eToSet)
{
    m_ePlacement = eToSet;
    return m_ePlacement;
}


// Get set one of the configurable header line
const TKeyValuePair& TCQCIntfWebImgBase::kvalHdrAt(const tCIDLib::TCard4 c4At) const
{
    CIDAssert(c4At < 2, L"Invalid web image header line index");
    if (c4At == 0)
        return m_kvalHdr1;

    return m_kvalHdr2;
}

tCIDLib::TVoid
TCQCIntfWebImgBase::SetHdrAt(const  TString&        strKey
                            , const TString&        strValue
                            , const tCIDLib::TCard4 c4At)
{
    CIDAssert(c4At < 2, L"Invalid web image header line index");
    if (c4At == 0)
        m_kvalHdr1.Set(strKey, strValue);
    else
        m_kvalHdr2.Set(strKey, strValue);
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebImgBase: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfWebImgBase::ActiveUpdate(const  tCIDLib::TBoolean   bNoRedraw
                                , const TGUIRegion&
                                , const tCIDLib::TBoolean   bInTopLayer)
{
    if (m_bNewImage)
    {
        TString strURL;
        try
        {
            TLocker mtxSync(&m_mtxSync);

            m_bNewImage = kCIDLib::False;
            strURL = m_strLiveURL;

            if (facCQCIntfEng().bLogInfo())
            {
                facCQCIntfEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"New image data available. ImgSz=%(1)"
                    , tCIDLib::ESeverities::Info
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(m_imgData.c4ImageSz())
                );
            }

            // Create the new image
            m_bmpImg = TBitmap(gdevCompat(), m_imgData);

            // Set the new serial number on it
            m_bmpImg.c4SerialNum(m_c4NewSerialNum);

            //
            //  Set the image key on it, so that any RIVA clients will be sent
            //  this key when we draw this image.
            //
            TString strRemId = kCQCKit::strWRIVA_WebPref;
            strRemId.Append(strURL);
            m_bmpImg.strUserData(strRemId);
        }

        catch(TError& errToCatch)
        {
            // Reset the bitmap
            m_bmpImg.Reset();
            m_bmpImg.strUserData(TString::strEmpty());

            if (facCQCIntfEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcWebImg_BitmapCreation
                , strURL
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }

        catch(...)
        {
            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcWebImg_BitmapCreation
                , strURL
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );

            // Reset the bitmap
            m_bmpImg.Reset();
            m_bmpImg.strUserData(TString::strEmpty());
        }

        if (!bNoRedraw && bInTopLayer)
            Redraw(areaActual());
    }
}


tCIDLib::TVoid TCQCIntfWebImgBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion
    ||  (c2FmtVersion > CQCIntfEng_WebImage::c2BaseFmtVersion))
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

    strmToReadFrom  >> m_c1ConstOpacity
                    >> m_ePlacement
                    >> m_bNoCache;

    // If > V1, read in the refresh MSs, else default it to zero for no refresh
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_c4RefreshMSs;
    else
        m_c4RefreshMSs = 0;

    // If > V2 read in the header lines, else make sure they are empty
    if (c2FmtVersion > 2)
    {
        strmToReadFrom >> m_kvalHdr1 >> m_kvalHdr2;
    }
     else
    {
        m_kvalHdr1.Clear();
        m_kvalHdr2.Clear();
    }

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any images and make sure we are in ready state
    m_bNewImage = kCIDLib::False;
    m_c4NewSerialNum = 0;
    m_bmpImg.Reset();
    m_strLiveURL.Clear();
}


tCIDLib::TVoid TCQCIntfWebImgBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_WebImage::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff
    strmToWriteTo   << m_c1ConstOpacity
                    << m_ePlacement
                    << m_bNoCache

                       // V2 stuff
                    << m_c4RefreshMSs

                       // V3 stuff
                    << m_kvalHdr1
                    << m_kvalHdr2

                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfWebImgBase::Update(         TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();

    // Calculate the target area based on the placement
    TArea areaSrc(TPoint::pntOrigin, m_bmpImg.szBitmap());
    TArea areaTarget;
    const tCIDGraphDev::EBltTypes eBlt = facCIDGraphDev().eCalcPlacement
    (
        areaDraw, areaSrc, areaTarget, m_ePlacement, kCIDLib::False
    );

    // If the target area intersects the invalid area, draw it
    if (areaTarget.bIntersects(areaInvalid))
    {
        m_bmpImg.AdvDrawOn
        (
            gdevTarget
            , areaSrc
            , areaTarget
            , kCIDLib::False
            , 0
            , m_bmpEmpty
            , eBlt
            , m_c1ConstOpacity
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebImgBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Allow the derived class to see what the live URL is
tCIDLib::TVoid TCQCIntfWebImgBase::QueryLiveURL(TString& strToFill) const
{
    TLocker mtxSync(&m_mtxSync);
    strToFill = m_strLiveURL;
}


// For the derived classes to give us a new URL to display
tCIDLib::TVoid TCQCIntfWebImgBase::SetLiveURL(const TString& strToSet)
{
    //
    //  Lock and store the new URL. The download thread will pick this one up the
    //  next round.
    //
    TLocker mtxSync(&m_mtxSync);
    m_strLiveURL = strToSet;
}


// ---------------------------------------------------------------------------
//  TCQCIntfWebImgBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Tries to stop the download if it's active
tCIDLib::TBoolean TCQCIntfWebImgBase::bStopDownload()
{
    if (m_thrDownloader.bIsRunning())
    {
        try
        {
            m_thrDownloader.ReqShutdownSync(15000);
            m_thrDownloader.eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  This thread handles the background downloading of images. When it has
//  the image and has streamed it in, it stores it in a member and exits.
//  The GUI thread will see it and display it.
//
tCIDLib::EExitCodes
TCQCIntfWebImgBase::eDownloader(TThread& thrThis, tCIDLib::TVoid*)
{
    // Now let the GUI thread go
    thrThis.Sync();

    try
    {
        tCIDLib::TKVPList   colInHdrs;
        tCIDLib::TKVPList   colOutHdrs;
        THTTPClient         httpcImg;
        TJPEGImage          imgJPEG;
        TPNGImage           imgPNG;
        THeapBuf            mbufData(0x20000, 0x800000, 0x10000);
        TString             strServerStamp;
        TString             strURL;
        TURL                urlImg;

        //
        //  Set up an empty image we can copy to the bitmap image if something
        //  goes wrong and we want to insure the drawn image is blanked out.
        //
        TPixelArray pixaEmpty
        (
            tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
            , tCIDImage::ERowOrders::TopDown
            , TSize(1, 1)
        );
        pixaEmpty.SetAllAlpha(0);
        TBitmapImage imgEmpty(pixaEmpty);

        // Enable auto-authentication
        httpcImg.bAutoAuth(kCIDLib::True);

        while (kCIDLib::True)
        {
            try
            {
                //
                //  Check the live URL. If it's changed, then we need to update, even
                //  if we are not auto-refreshing.
                //
                tCIDLib::TBoolean bNewURL = kCIDLib::False;
                {
                    TLocker lockrSync(&m_mtxSync);

                    if (strURL != m_strLiveURL)
                    {
                        bNewURL = kCIDLib::True;
                        strURL = m_strLiveURL;
                        strServerStamp.Clear();
                    }

                    if (bNewURL)
                    {
                        // If the new URL is empty, then set an empty image
                        if (strURL.bIsEmpty())
                        {
                            m_imgData = imgEmpty;
                            m_c4NewSerialNum = 0;
                            m_bNewImage = kCIDLib::True;
                        }

                        if (facCQCIntfEng().bLogInfo())
                        {
                            facCQCIntfEng().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , L"Got new URL to display"
                                , strURL
                                , tCIDLib::ESeverities::Info
                                , tCIDLib::EErrClasses::AppStatus
                            );
                        }
                    }
                }

                //
                //  If we are either in auto-refresh mode, or we got a new URL, then
                //  let's do a download. Else sleep a bit, and try again. Break out
                //  if asked to shutdown while sleeping. If the URL is empty of course
                //  we also don't do anything.
                //
                if (strURL.bIsEmpty() || (!bNewURL && !m_c4RefreshMSs))
                {
                    if (!thrThis.bSleep(500))
                        break;

                    continue;
                }

                //
                //
                //  If we have a new URL, then parse it into our URL object. We don't
                //  want to continually reparse every round if in auto-refresh mode,
                //  so we only do it when the URL changes.
                //
                if (bNewURL)
                {
                    try
                    {
                        urlImg.Reset(strURL, tCIDSock::EQualified::Full);
                    }

                    //
                    //  Don't try to use it. Clear our URL and the live URL
                    //  strings and set up an empty default image.
                    //
                    //  Unless the URL changes, we'll stop doing anything at this
                    //  point.
                    //
                    catch(TError& errToCatch)
                    {
                        // Make sure we unlock before doing the logging
                        {
                            TLocker lockrSync(&m_mtxSync);

                            strURL.Clear();
                            m_strLiveURL.Clear();
                            m_imgData = imgEmpty;
                            m_c4NewSerialNum = 0;
                            m_bNewImage = kCIDLib::True;
                        }

                        if (facCQCIntfEng().bShouldLog(errToCatch))
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }
                        continue;
                    }

                    catch(...)
                    {
                        TLocker lockrSync(&m_mtxSync);

                        strURL.Clear();
                        m_strLiveURL.Clear();
                        m_imgData = imgEmpty;
                        m_c4NewSerialNum = 0;
                        m_bNewImage = kCIDLib::True;
                        continue;
                    }
                }

                //
                //  OK, let's give it a shot. Note we allocate a small buffer with a
                //  failure large max size. The HTTP client will reallocate it up
                //  to hold the data as required.
                //
                tCIDLib::TCard4     c4ContLen = 0;
                tCIDNet::EHTTPCodes eCodeType;
                TString             strContType;
                TString             strRepText;
                TString             strUltimateURL;

                //
                //  If we have a server stamp, then generate an if modified header if
                //  caching isn't diabled.
                //
                colInHdrs.RemoveAll();
                if (!m_bNoCache && !strServerStamp.bIsEmpty())
                {
                    colInHdrs.objAdd
                    (
                        TKeyValuePair(THTTPClient::strHdr_IfModifiedSince, strServerStamp)
                    );
                }

                // Add non empty configurable header lines
                if (!m_kvalHdr1.strKey().bIsEmpty())
                    colInHdrs.objAdd(m_kvalHdr1);
                if (!m_kvalHdr2.strKey().bIsEmpty())
                    colInHdrs.objAdd(m_kvalHdr2);

                if (facCQCIntfEng().bLogInfo())
                {
                    facCQCIntfEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Downloading image"
                        , strURL
                        , tCIDLib::ESeverities::Info
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }

                TCIDDataSrcJan janEmpty;
                tCIDLib::TCard4 c4Res = httpcImg.c4SendGetRedir
                (
                    janEmpty
                    , urlImg
                    , TTime::enctNowPlusSecs(8)
                    , CQCIntfEng_WebImage::strAgent
                    , L"image/*"
                    , eCodeType
                    , strRepText
                    , colOutHdrs
                    , strContType
                    , mbufData
                    , c4ContLen
                    , strUltimateURL
                    , colInHdrs
                );

                if (facCQCIntfEng().bLogInfo())
                {
                    facCQCIntfEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Back from download. Res code=%(1), Bytes=%(2)"
                        , tCIDLib::ESeverities::Info
                        , tCIDLib::EErrClasses::AppStatus
                        , TCardinal(c4Res)
                        , TCardinal(c4ContLen)
                    );
                }

                // If we got it, then let's process it, else skip it
                if (c4Res == kCIDNet::c4HTTPStatus_NotModified)
                {
                    if (facCQCIntfEng().bLogInfo())
                    {
                        facCQCIntfEng().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Server reports no change in image data"
                            , tCIDLib::ESeverities::Info
                            , tCIDLib::EErrClasses::AppStatus
                        );
                    }

                    //
                    //  It hasn't been modified, so don't do anything, but get out the
                    //  stamp and save it.
                    //
                    TNetCoreParser::bFindHdrLine
                    (
                        colOutHdrs, THTTPClient::strHdr_LastModified, strServerStamp
                    );
                }
                 else if (c4Res != kCIDNet::c4HTTPStatus_OK)
                {
                    if (facCQCIntfEng().bLogWarnings())
                    {
                        facCQCIntfEng().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kIEngErrs::errcWebImg_DownloadFailed
                            , strURL
                            , tCIDLib::ESeverities::Warn
                            , tCIDLib::EErrClasses::CantDo
                            , TCardinal(c4Res)
                        );
                    }
                }
                 else if (c4ContLen != 0)
                {
                    if (facCQCIntfEng().bLogInfo())
                    {
                        facCQCIntfEng().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Probing image for type info"
                            , tCIDLib::ESeverities::Info
                            , tCIDLib::EErrClasses::AppStatus
                        );
                    }

                    //
                    //  Probe the buffer to see what type of image it is. We don't bother
                    //  to look at the content type since it might not be right.
                    //
                    const tCIDImage::EImgTypes eType = facCIDImgFact().eProbeImg
                    (
                        mbufData, c4ContLen
                    );

                    //
                    //  If an unknown type, ignore it and log if in verbose mode. Else we
                    //  need try to decode it and leave it for the fgn thread.
                    //
                    if (eType == tCIDImage::EImgTypes::Unknown)
                    {
                        if (facCQCIntfEng().bLogWarnings())
                        {
                            facCQCIntfEng().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kIEngErrs::errcWebImg_UnknownImgType
                                , strURL
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::CantDo
                                , strContType
                            );
                        }
                    }
                     else
                    {
                        if (facCQCIntfEng().bLogInfo())
                        {
                            facCQCIntfEng().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , L"Streaming in image data. Type=%(1)"
                                , tCIDLib::ESeverities::Info
                                , tCIDLib::EErrClasses::AppStatus
                                , TCardinal(tCIDLib::c4EnumOrd(eType))
                            );
                        }

                        //
                        //  Try to stream the image in and make sure it's going to work
                        //  before we commit. We stream into a temp first. Make sure
                        //  the stream is cleaned up because we may move the memory buffer
                        //  below.
                        //
                        tCIDLib::TBoolean bGoodImg = kCIDLib::True;
                        TBitmapImage imgBmp;
                        TPNGImage imgPNG;
                        TJPEGImage imgJPEG;
                        {
                            TBinMBufInStream strmSrc(&mbufData, c4ContLen);
                            if (eType == tCIDImage::EImgTypes::Bitmap)
                                strmSrc >> imgBmp;
                            else if (eType == tCIDImage::EImgTypes::JPEG)
                                strmSrc >> imgJPEG;
                            else if (eType == tCIDImage::EImgTypes::PNG)
                                strmSrc >> imgPNG;
                            else
                                bGoodImg = kCIDLib::False;
                        }

                        //
                        //  Add it to our web cache if we are in RIVA mode.
                        //  Otherwise, we only use web server based caching. In
                        //  RIVA mode we have to get it cached so that the RIVA
                        //  client can come back and get it when asked to draw.
                        //
                        if (bGoodImg)
                        {
                            tCIDLib::TCard4 c4SerialNum = 0;
                            if (TFacCQCIntfEng::bRemoteMode())
                            {
                                // We are done with the bufffer, so we can move it.
                                facCQCIntfEng().CacheWebImage
                                (
                                    strURL
                                    , c4ContLen
                                    , tCIDLib::ForceMove(mbufData)
                                    , civOwner().gdevCompat()
                                    , c4SerialNum
                                );
                            }

                            // Now we can lock and move the data in
                            TLocker lockrSync(&m_mtxSync);
                            m_c4NewSerialNum = c4SerialNum;
                            if (eType == tCIDImage::EImgTypes::Bitmap)
                                m_imgData = imgBmp;
                            else if (eType == tCIDImage::EImgTypes::JPEG)
                                m_imgData = imgJPEG;
                            else if (eType == tCIDImage::EImgTypes::PNG)
                                m_imgData = imgPNG;

                            m_bNewImage = kCIDLib::True;
                        }

                        if (facCQCIntfEng().bLogInfo())
                        {
                            facCQCIntfEng().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , L"Image data streamed in successfully"
                                , tCIDLib::ESeverities::Info
                                , tCIDLib::EErrClasses::AppStatus
                            );
                        }
                    }
                }
            }

            catch(TError& errToCatch)
            {
                if (facCQCIntfEng().bLogWarnings())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    facCQCIntfEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcWebImg_ImgExcept
                        , strURL
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                    );
                }
            }

            catch(...)
            {
                if (facCQCIntfEng().bLogFailures())
                {
                    facCQCIntfEng().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcWebImg_ImgSysExcept
                        , strURL
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                    );
                }
            }

            //
            //  If we are in auto-refresh mode, then we wait for the refresh period.
            //  Else we do a short wait then go check for a new URL.
            //
            const tCIDLib::TCard4 c4Sleep = m_c4RefreshMSs ? m_c4RefreshMSs : 500;
            if (!thrThis.bSleep(c4Sleep))
                break;
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcWebImg_InitExcept
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }
    }

    catch(...)
    {
        if (facCQCIntfEng().bLogFailures())
        {
            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcWebImg_InitSysExcept
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }
    }

    return tCIDLib::EExitCodes::Normal;
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldWebImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldWebImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldWebImg::TCQCIntfFldWebImg() :

    TCQCIntfWebImgBase
    (
        CQCIntfEng_WebImage::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_WebImage)
    )
    , MCQCIntfSingleFldIntf
      (
        new TCQCFldFiltTypeName
        (
            tCQCKit::EReqAccess::MReadCWrite, tCQCKit::EFldTypes::String
        )
      )
{
}

TCQCIntfFldWebImg::TCQCIntfFldWebImg(const TCQCIntfFldWebImg& iwdgSrc) :

    TCQCIntfWebImgBase(iwdgSrc)
    , MCQCIntfSingleFldIntf(iwdgSrc)
{
}

TCQCIntfFldWebImg::~TCQCIntfFldWebImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldWebImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldWebImg&
TCQCIntfFldWebImg::operator=(const TCQCIntfFldWebImg& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfSingleFldIntf::operator=(iwdgSrc);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldWebImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfFldWebImg::bCanAcceptField( const   TCQCFldDef&         flddTest
                                    , const TString&
                                    , const TString&
                                    , const tCIDLib::TBoolean   bStore)
{
    // If it's a readable string field, we can use it
    return (flddTest.eType() == tCQCKit::EFldTypes::String)
           && ((flddTest.eAccess() == tCQCKit::EFldAccess::Read)
           ||  (flddTest.eAccess() == tCQCKit::EFldAccess::ReadWrite));
}


tCIDLib::TBoolean
TCQCIntfFldWebImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldWebImg& iwdgOther(static_cast<const TCQCIntfFldWebImg&>(iwdgSrc));

    // We just add the field mixin
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


tCIDLib::TVoid TCQCIntfFldWebImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldWebImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldWebImg::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LinkToField)
    {
        // Just pass the field name on to our field mixin
        ChangeField(ccfgToDo.piAt(0).m_strValue);
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
    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid
TCQCIntfFldWebImg::Initialize(  TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfFldWebImg::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We just support the changing of the field
    {
        TCQCCmd& cmdLinkToFld = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_LinkToField
                , facCQCKit().strMsg(kKitMsgs::midCmd_LinkToField)
                , tCQCKit::ECmdPTypes::Field
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            )
        );

        cmdLinkToFld.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::MReadCWrite);
    }
}


// Report any monikers we refrence
tCIDLib::TVoid
TCQCIntfFldWebImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldWebImg::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"URL Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldWebImg::RegisterFields(          TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldWebImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocField))
        MCQCIntfSingleFldIntf::ReplaceField(strSrc, strTar, bRegEx, bFull, regxFind);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfFldWebImg::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfFldWebImg::Validate(const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our field mixin
    bValidateField(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldWebImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Our field mixin saw a change in our field, so react to that
tCIDLib::TVoid
TCQCIntfFldWebImg::FieldChanged(        TCQCFldPollInfo&    cfpiAssoc
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        const TCQCFldValue& fvVal = fvCurrent();

        if (fvVal.eFldType() == tCQCKit::EFldTypes::String)
        {
            const TCQCStringFldValue& fvStr = static_cast<const TCQCStringFldValue&>(fvVal);
            SetLiveURL(fvStr.strValue());
        }
         else
        {
            SetLiveURL(TString::strEmpty());
        }
    }
     else
    {
        SetLiveURL(TString::strEmpty());
    }
}


tCIDLib::TVoid TCQCIntfFldWebImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_WebImage::c2FldFmtVersion))
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

    // Stream our parent class' contents
    TParent::StreamFrom(strmToReadFrom);

    // Do any mixins
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // And now ours, though we don't have any of our own right now
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfFldWebImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_WebImage::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // Stream our stuff
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


//
//  This is called periodically to give us a chance to update. We just pass
//  it to our field mixin. If the value has changed, he will call us back on
//  our FieldChange() method.
//
tCIDLib::TVoid
TCQCIntfFldWebImg::ValueUpdate(         TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticWebImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticWebImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticWebImg::TCQCIntfStaticWebImg() :

    TCQCIntfWebImgBase
    (
        CQCIntfEng_WebImage::eStaticCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_WebImage)
    )
{
}

//
//  A special ctor used for refactoring. The container class, when reading in
//  the widgets, will watch for the old class and read it in. It'll then call
//  this ctor to let us make use of that old content to create a new one.
//
TCQCIntfStaticWebImg::TCQCIntfStaticWebImg(const TCQCIntfWebImg& iwdgOld) :

    TCQCIntfWebImgBase
    (
        CQCIntfEng_WebImage::eStaticCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_WebImage)
    )
    , m_strURL(iwdgOld.m_strURL)
{
    //
    //  Copy over the widget level stuff. Don't want to copy the base class
    //  level over since it would munge the stuff we passed down above.
    //
    TCQCIntfWidget::operator=(iwdgOld);

    // Call settings on the parent class to store away that stuff
    bNoCache(iwdgOld.m_bNoCache);
    c1ConstOpacity(iwdgOld.m_c1ConstOpacity);
    ePlacement(iwdgOld.m_ePlacement);
}

TCQCIntfStaticWebImg::TCQCIntfStaticWebImg(const TCQCIntfStaticWebImg& iwdgToCopy) :

    TCQCIntfWebImgBase(iwdgToCopy)
    , m_strURL(iwdgToCopy.m_strURL)
{
}

TCQCIntfStaticWebImg::~TCQCIntfStaticWebImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticWebImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticWebImg&
TCQCIntfStaticWebImg::operator=(const TCQCIntfStaticWebImg& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        m_strURL   = iwdgSrc.m_strURL;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticWebImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfStaticWebImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticWebImg& iwdgOther(static_cast<const TCQCIntfStaticWebImg&>(iwdgSrc));

    // Check our stuff
    return (m_strURL == iwdgOther.m_strURL);
}


// Make sure our URL is a fully qualified one
tCIDLib::TBoolean
TCQCIntfStaticWebImg::bValidateParm(const   TCQCCmd&        cmdSrc
                                    , const TCQCCmdCfg&     ccfgTar
                                    , const tCIDLib::TCard4 c4Index
                                    , const TString&        strValue
                                    ,       TString&        strErrText)
{
    // Let our parent go first
    if (!TParent::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    if ((ccfgTar.strCmdId() == kCQCKit::strCmdId_SetImage) && (c4Index == 0))
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
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCIntfStaticWebImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticWebImg&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfStaticWebImg::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetImage)
    {
        // Pass along the URL to the base class to load
        SetLiveURL(ccfgToDo.piAt(0).m_strValue);
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
    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid
TCQCIntfStaticWebImg::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


tCIDLib::TVoid TCQCIntfStaticWebImg::PostInit()
{
    // Call our parent first
    TParent::PostInit();

    //
    //  If we have an initial URL to load, then set it on our parent if not in
    //  designer mode.
    //
    if (!m_strURL.bIsEmpty() && !TFacCQCIntfEng::bDesMode())
        SetLiveURL(m_strURL);
}


tCIDLib::TVoid
TCQCIntfStaticWebImg::QueryCmds(        TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    //
    //  This one will update the image to display. In a pre-load scenario
    //  we just store it as teh initial image to load.
    //
    {
        TCQCCmd& cmdSetImg = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetImage
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetImage)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_URL)
            )
        );

        // The image is optional so that they can blank the image
        cmdSetImg.SetParmReqAt(0, kCIDLib::False);
    }
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfStaticWebImg::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    adatTmp.Set(L"Static Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    // We have a statically configured URL
    adatTmp.Set
    (
        L"URL"
        , kCQCIntfEng::strAttr_WebImg_URL
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(m_strURL);
    colAttrs.objAdd(adatTmp);
}


tCIDLib::TVoid
TCQCIntfStaticWebImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
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
    }
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfStaticWebImg::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_WebImg_URL)
        m_strURL = adatNew.strValue();
}


tCIDLib::TVoid
TCQCIntfStaticWebImg::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Make sure our URL is parseable if not empty
    if (!m_strURL.bIsEmpty())
    {
        try
        {
            TURL urlImg(m_strURL, tCIDSock::EQualified::Full);
        }

        catch(const TError& errToCatch)
        {
            colErrs.objAdd
            (
                TCQCIntfValErrInfo
                (
                    c4UniqueId(), errToCatch.strErrText(), kCIDLib::False, strWidgetId()
                )
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticWebImg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the URL
const TString& TCQCIntfStaticWebImg::strURL() const
{
    return m_strURL;
}

const TString& TCQCIntfStaticWebImg::strURL(const TString& strToSet)
{
    m_strURL = strToSet;
    return m_strURL;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticWebImg: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfStaticWebImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_WebImage::c2StaticFmtVersion))
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

    // Stream our parent class' contents
    TParent::StreamFrom(strmToReadFrom);

    // And now ours
    strmToReadFrom  >> m_strURL;
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfStaticWebImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_WebImage::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff
    strmToWriteTo   << m_strURL
                    << tCIDLib::EStreamMarkers::EndObject;
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarWebImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarWebImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarWebImg::TCQCIntfVarWebImg() :

    TCQCIntfWebImgBase
    (
        CQCIntfEng_WebImage::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_WebImage)
    )
{
}

TCQCIntfVarWebImg::TCQCIntfVarWebImg(const TCQCIntfVarWebImg& iwdgSrc) :

    TCQCIntfWebImgBase(iwdgSrc)
    , MCQCIntfVarIntf(iwdgSrc)
{
}

TCQCIntfVarWebImg::~TCQCIntfVarWebImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarWebImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarWebImg&
TCQCIntfVarWebImg::operator=(const TCQCIntfVarWebImg& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfVarIntf::operator=(iwdgSrc);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarWebImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVarWebImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarWebImg& iwdgOther(static_cast<const TCQCIntfVarWebImg&>(iwdgSrc));

    // We just add the variable mixin
    return MCQCIntfVarIntf::bSameVariable(iwdgOther);
}


tCIDLib::TVoid TCQCIntfVarWebImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarWebImg&>(iwdgSrc);
}



// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarWebImg::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LinkToVar)
    {
        SetVarInfo(ccfgToDo.piAt(0).m_strValue);
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
    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid
TCQCIntfVarWebImg::Initialize(  TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfVarWebImg::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We just support the changing of the variable
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_LinkToVar
            , facCQCKit().strMsg(kKitMsgs::midCmd_LinkToVar)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );
}


tCIDLib::TVoid
TCQCIntfVarWebImg::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarWebImg::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocVar))
        MCQCIntfVarIntf::ReplaceVar(strSrc, strTar, bRegEx, bFull, regxFind);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfVarWebImg::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarWebImg::Validate(const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarWebImg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Our variable mixin calls us here when we acquire the associated variable,
//  to make sure it has the correct attributes for our use. We can only use
//  variables of string type.
//
tCIDLib::TBoolean TCQCIntfVarWebImg::bCanAcceptVar(const TCQCActVar& varToTest)
{
    return (varToTest.eType() == tCQCKit::EFldTypes::String);
}


tCIDLib::TVoid TCQCIntfVarWebImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_WebImage::c2VarFmtVersion))
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

    // Stream our parent class' contents
    TParent::StreamFrom(strmToReadFrom);

    // Do any mixins
    MCQCIntfVarIntf::ReadInVar(strmToReadFrom);

    // And now ours, though we don't have any of our own right now
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfVarWebImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_WebImage::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // Stream our stuff
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


//
//  We just pass this on to our variable mixin. It's called periodically
//  to allow us to check to see if our variable value has changed. We'll get
//  called back on VarValChanged() or VarInError() if so.
//
tCIDLib::TVoid
TCQCIntfVarWebImg::ValueUpdate(         TCQCPollEngine&
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean   bVarUpdate
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    // If no variables could have changed, don't bother
    if (!bVarUpdate)
        return;

    // Pass it on to our mixin
    VarUpdate(ctarGlobalVars, bNoRedraw);
}



//
//  Our variable interface mixin calls us back here if the variable state
//  changes out of ready state. We just tell the base class to get rid of
//  any current image.
//
tCIDLib::TVoid
TCQCIntfVarWebImg::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    SetLiveURL(TString::strEmpty());
}



//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that we are in ready state, either
//  already were or have just transitioned into it.
//
tCIDLib::TVoid
TCQCIntfVarWebImg::VarValChanged(const  TCQCActVar&         varNew
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        )
{
    if (varNew.eType() == tCQCKit::EFldTypes::String)
    {
        const TCQCStringFldValue& fvStr
                    = static_cast<const TCQCStringFldValue&>(varNew.fvCurrent());
        SetLiveURL(fvStr.strValue());
    }
     else
    {
        // It's not of the right time, so tell the bass class to not show an image
        SetLiveURL(TString::strEmpty());
    }
}


